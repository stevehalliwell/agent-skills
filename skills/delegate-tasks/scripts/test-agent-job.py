#!/usr/bin/env python3
"""Local integration tests for agent-job.py; does not start Pi or Codex."""
from __future__ import annotations
import importlib.util, json, os, sys, tempfile, threading, time
from unittest.mock import patch
from pathlib import Path

RUNNER = Path(__file__).with_name("agent-job.py")
spec = importlib.util.spec_from_file_location("agent_job", RUNNER)
agent_job = importlib.util.module_from_spec(spec); assert spec and spec.loader; spec.loader.exec_module(agent_job)

# Synthetic harnesses exercise scheduler behavior without model credentials.
def synthetic_command(base, task, result):
    kind = task["harness"]
    if kind in {"test-complete", "codex"}: return [sys.executable, "-c", f"from pathlib import Path; Path(r'{result}').write_text('complete')"]
    if kind == "test-fail": return [sys.executable, "-c", "import sys; print('fail'); sys.exit(7)"]
    if kind == "test-sleep": return [sys.executable, "-c", "import time; time.sleep(5); print('late')"]
    raise ValueError(f"unsupported harness: {kind!r}")
agent_job.adapter_command = synthetic_command

def write_batch(base: Path, tasks, **extra):
    (base / "tasks").mkdir(parents=True, exist_ok=True)
    for task in tasks: (base / task["prompt"]).write_text("Do the test task.", encoding="utf-8")
    (base / "batch.json").write_text(json.dumps({"tasks": tasks, **extra}), encoding="utf-8")

def wait_manifest(base: Path, limit=10):
    deadline=time.monotonic()+limit
    while time.monotonic()<deadline:
        state=agent_job.load(base/".agent-jobs"/"current.json")
        m=agent_job.load(base/"runs"/state["run_id"]/"manifest.json")
        if m["status"] in {"completed","completed_with_failures","cancelled","runner_failed"}: return m
        time.sleep(.1)
    raise AssertionError("background batch did not finish")

def main():
    # Codex sandbox selection is part of the recorded task contract. Validate it
    # without invoking the real Codex binary or model catalogue.
    class Completed:
        returncode = 0
        stdout = b'{"models":[{"slug":"gpt-5.6-terra","supported_reasoning_levels":[{"effort":"medium"}]}]}'
        stderr = b""
    result = Path("result.md")
    with patch.object(agent_job.codex.shutil, "which", return_value="codex"), patch.object(agent_job.codex.subprocess, "run", return_value=Completed()):
        safe = agent_job.codex.command(Path("job"), {"sandbox":"workspace-write"}, result)
        broad = agent_job.codex.command(Path("job"), {"sandbox":"danger-full-access"}, result)
        assert safe[:3] == ["codex", "--sandbox", "workspace-write"], safe
        assert broad[:3] == ["codex", "--sandbox", "danger-full-access"], broad
        try: agent_job.codex.command(Path("job"), {"sandbox":"read-only"}, result)
        except ValueError: pass
        else: raise AssertionError("invalid Codex sandbox was accepted")
    with tempfile.TemporaryDirectory() as temp:
        base=Path(temp)/"job"; agent_job.scaffold(base)
        tasks=[
            {"name":"good","prompt":"tasks/good.md","harness":"test-complete"},
            {"name":"bad","prompt":"tasks/bad.md","harness":"test-fail"},
            {"name":"slow","prompt":"tasks/slow.md","harness":"test-sleep","deadline":.2},
        ]; write_batch(base,tasks)
        m=agent_job.run_batch(base,"blocking")
        assert m["status"] == "completed_with_failures", m
        assert m["tasks"]["good"]["status"] == "completed", m
        assert m["tasks"]["bad"]["status"] == "failed", m
        assert m["tasks"]["slow"]["status"] == "timeout", m
        # Direct background worker uses the current Python process as its parent.
        rid="background"; (base/"runs"/rid).mkdir(parents=True)
        thread=threading.Thread(target=agent_job.run_batch,args=(base,rid,{"pid":os.getpid(),"identity":agent_job.identity(os.getpid())}),daemon=True)
        thread.start(); time.sleep(.1)
        m=agent_job.load(base/"runs"/rid/"manifest.json")
        assert m["status"] == "running", m
        thread.join(10); assert not thread.is_alive()
        m=agent_job.load(base/"runs"/rid/"manifest.json")
        assert m["status"] == "completed_with_failures", m
        # A changed parent identity represents a dead/reused launcher PID; all
        # active children must be cancelled before the batch is terminal.
        write_batch(base, [{"name":"orphan","prompt":"tasks/orphan.md","harness":"test-sleep","deadline":5}])
        m=agent_job.run_batch(base,"parent-exited",{"pid":os.getpid(),"identity":"not-this-process"})
        assert m["status"] == "cancelled", m
        assert m["tasks"]["orphan"]["status"] == "cancelled", m
        # Durable cancellation must make wait observe a terminal batch, rather
        # than killing only the scheduler and leaving a stale running manifest.
        write_batch(base, [{"name":"cancel","prompt":"tasks/cancel.md","harness":"test-sleep","deadline":5}])
        rid="cancelled"; run=base/"runs"/rid; run.mkdir(parents=True)
        thread=threading.Thread(target=agent_job.run_batch,args=(base,rid),daemon=True); thread.start(); time.sleep(.1)
        class Args: job_folder=str(base); run=rid
        agent_job.cancel(Args())
        thread.join(10); assert not thread.is_alive()
        m=agent_job.load(run/"manifest.json")
        assert m["status"] == "cancelled", m
        assert m["tasks"]["cancel"]["status"] == "cancelled", m
        # Codex receives the runner-enforced job-folder write boundary.
        write_batch(base, [{"name":"boundary","prompt":"tasks/boundary.md","harness":"codex"}])
        m=agent_job.run_batch(base,"codex-boundary")
        boundary = (base/"runs"/"codex-boundary"/"boundary"/"execution-prompt.md").read_text(encoding="utf-8")
        assert f"You may write only inside this job folder: {base}." in boundary, boundary
        assert m["tasks"]["boundary"]["artifacts"]["execution_prompt"] == str(Path("runs")/"codex-boundary"/"boundary"/"execution-prompt.md"), m
        # Background mode launches harnesses directly; no scheduler/worker PID
        # may be required for a later wait to reconcile their result artifacts.
        write_batch(base, [{"name":"detached","prompt":"tasks/detached.md","harness":"test-complete"}])
        class BackgroundArgs: job_folder=str(base); background=True; parent_pid=None
        assert agent_job.start(BackgroundArgs()) == 0
        detached_rid=agent_job.current(base)
        class WaitArgs: job_folder=str(base); run=detached_rid
        assert agent_job.wait(WaitArgs()) == 0
        detached=agent_job.load(base/"runs"/detached_rid/"manifest.json")
        assert detached["launch_mode"] == "detached", detached
        assert detached["tasks"]["detached"]["status"] == "completed", detached
        # Detached Codex jobs retain same write boundary as blocking jobs.
        write_batch(base, [{"name":"detached-codex","prompt":"tasks/detached-codex.md","harness":"codex"}])
        assert agent_job.start(BackgroundArgs()) == 0
        detached_codex_rid=agent_job.current(base)
        WaitArgs.run=detached_codex_rid
        assert agent_job.wait(WaitArgs()) == 0
        boundary=(base/"runs"/detached_codex_rid/"detached-codex"/"execution-prompt.md").read_text(encoding="utf-8")
        assert f"You may write only inside this job folder: {base}." in boundary, boundary
    print("agent-job scheduler integration tests: OK")
if __name__ == "__main__": main()
