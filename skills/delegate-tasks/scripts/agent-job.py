#!/usr/bin/env python3
"""Launch bounded, self-contained Pi or Codex task batches with durable artifacts.

`run --background` preflights and launches every harness as an independent,
detached process. Status, waiting, deadlines, and cancellation reconcile durable
task manifests without depending on a launcher or scheduler process.
"""
from __future__ import annotations
import argparse, datetime as dt, json, os, signal, subprocess, sys, threading, time, uuid
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
from typing import Any
from harnesses import codex, pi

DEFAULT_DEADLINE = 20 * 60
POLL = 0.5
GRACE = 10
TERMINAL = {"completed", "failed", "timeout", "cancelled", "preflight_failed", "incomplete", "parent_exited"}


def now(): return dt.datetime.now(dt.timezone.utc).isoformat().replace("+00:00", "Z")
def run_id(): return dt.datetime.now(dt.timezone.utc).strftime("%Y%m%dT%H%M%SZ") + "-" + uuid.uuid4().hex[:8]
def dump(path: Path, value: Any):
    path.parent.mkdir(parents=True, exist_ok=True); temp = path.with_name("." + path.name + "." + uuid.uuid4().hex + ".tmp")
    temp.write_text(json.dumps(value, indent=2, sort_keys=True) + "\n", encoding="utf-8"); temp.replace(path)
def load(path: Path):
    value = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(value, dict): raise ValueError(f"{path} must contain a JSON object")
    return value
def folder(raw: str):
    value = Path(raw).expanduser().resolve()
    if value == Path(value.anchor): raise ValueError("job folder must not be a filesystem root")
    return value
def identity(pid: int):
    if pid <= 0: return None
    try: os.kill(pid, 0)
    except ProcessLookupError: return None
    except PermissionError: pass
    except OSError: return None
    # Creation time prevents a reused PID being mistaken for the parent on Unix hosts.
    # `ps` is not a reliable PID probe under Windows/MSYS, where kill(0) is enough.
    if os.name == "nt": return str(pid)
    out = subprocess.run(["ps", "-p", str(pid), "-o", "lstart="], capture_output=True, text=True).stdout.strip()
    return out or None
def process_options():
    # Windows has no Unix process groups. CREATE_NEW_PROCESS_GROUP gives the
    # child a distinct process group; taskkill /T terminates its descendants.
    return {"start_new_session": True} if os.name != "nt" else {"creationflags": subprocess.CREATE_NEW_PROCESS_GROUP}
def detached_process_options():
    # Background harnesses have independent lifecycle manifests. On Windows,
    # a new process group avoids launcher control while CREATE_NO_WINDOW keeps
    # routine background harnesses from flashing a console window.
    if os.name != "nt": return {"start_new_session": True}
    return {"creationflags": subprocess.CREATE_NEW_PROCESS_GROUP | subprocess.CREATE_NO_WINDOW}
def kill_group(proc: subprocess.Popen):
    if os.name == "nt":
        subprocess.run(["taskkill", "/PID", str(proc.pid), "/T", "/F"], capture_output=True, check=False)
    else:
        try: os.killpg(proc.pid, signal.SIGTERM)
        except ProcessLookupError: return
    try: proc.wait(timeout=GRACE)
    except subprocess.TimeoutExpired:
        if os.name != "nt":
            try: os.killpg(proc.pid, signal.SIGKILL)
            except ProcessLookupError: pass
        proc.wait()
def rel(base: Path, path: Path): return str(path.relative_to(base))
def terminal(status): return status in TERMINAL
def parsed_time(value): return dt.datetime.fromisoformat(value.replace("Z", "+00:00"))

def scaffold(base: Path):
    base.mkdir(parents=True, exist_ok=True); (base / "tasks").mkdir(exist_ok=True); (base / "runs").mkdir(exist_ok=True); (base / ".agent-jobs").mkdir(exist_ok=True)
    prompt = base / "tasks" / "task.md"
    if not prompt.exists(): prompt.write_text("# Delegated task\n\n## Goal\n\nDescribe the self-contained task.\n\n## Inputs and constraints\n\nList relevant files, decisions, allowed changes, and verification.\n\n## Deliverable\n\nState the required result.\n", encoding="utf-8")
    batch = base / "batch.json"
    if not batch.exists(): dump(batch, {"deadline": DEFAULT_DEADLINE, "defaults": {"harness": "pi", "thinking": "medium"}, "tasks": [{"name": "task", "prompt": "tasks/task.md"}]})
    print(json.dumps({"job_folder": str(base), "batch": str(batch), "status": "scaffolded"}))

def safe_child(base: Path, raw: str, label: str):
    p = (base / raw).resolve()
    if base not in p.parents and p != base: raise ValueError(f"{label} must be inside the job folder: {raw}")
    return p

HARNESS_ADAPTERS = {"pi": pi, "codex": codex}

def adapter_command(base: Path, task: dict, result: Path):
    """Dispatch one resolved task to its harness-specific command builder."""
    try:
        return HARNESS_ADAPTERS[task["harness"]].command(base, task, result)
    except KeyError as exc:
        raise ValueError(f"unsupported harness: {task['harness']!r}") from exc

def effective_task(base: Path, defaults: dict, raw: dict):
    task = {**defaults, **raw}
    name = task.get("name")
    if not isinstance(name, str) or not name or any(x in name for x in "/\\") or name in {".", ".."}: raise ValueError("each task needs a safe unique name")
    task["name"] = name
    if not isinstance(task.get("prompt"), str): raise ValueError(f"task {name} needs a prompt path")
    prompt = safe_child(base, task["prompt"], "prompt")
    if not prompt.is_file() or not prompt.read_text(encoding="utf-8").strip(): raise ValueError(f"task {name} has no non-empty prompt: {prompt}")
    task["deadline"] = float(task.get("deadline", defaults.get("deadline", DEFAULT_DEADLINE)))
    if task["deadline"] <= 0: raise ValueError(f"task {name} deadline must be positive")
    task["harness"] = str(task.get("harness", "pi"))
    return task

def task_run(base: Path, batch_run: Path, task: dict, stop: threading.Event):
    name = task["name"]; work = batch_run / name; work.mkdir(parents=True, exist_ok=True)
    prompt, result, events, stderr = safe_child(base, task["prompt"], "prompt"), work / "result.md", work / "events.log", work / "stderr.log"
    execution_prompt = prompt
    if task["harness"] == "codex":
        # The broad Codex sandbox is a host-level capability. Keep the writable
        # boundary explicit in every child context, regardless of its setting.
        execution_prompt = work / "execution-prompt.md"
        execution_prompt.write_text(
            f"You may write only inside this job folder: {base}. Do not read, modify, or create files outside it, even if your sandbox permits broader host access.\n\n"
            + prompt.read_text(encoding="utf-8"), encoding="utf-8"
        )
    manifest_path = work / "manifest.json"
    started = time.monotonic()
    m = {"name": name, "harness": task["harness"], "status": "starting", "started_at": now(), "ended_at": None, "exit_code": None, "deadline_seconds": task["deadline"], "artifacts": {"prompt": rel(base,prompt), "execution_prompt": rel(base,execution_prompt), "result": rel(batch_run,result), "events": rel(batch_run,events), "stderr": rel(batch_run,stderr)}, "requested": {k: task.get(k) for k in ("model","thinking","web_search","ephemeral","sandbox","skills","exclude_skills","extensions","exclude_extensions") if k in task}}
    try: cmd = adapter_command(base, task, result)
    except Exception as exc:
        m.update(status="preflight_failed", ended_at=now(), elapsed_seconds=round(time.monotonic() - started, 3), error=str(exc)); dump(manifest_path,m); return m
    m["command"] = cmd; dump(manifest_path,m)
    with execution_prompt.open("rb") as inp, events.open("wb") as out, stderr.open("wb") as err:
        proc = subprocess.Popen(cmd, cwd=base, stdin=inp, stdout=out, stderr=err, **process_options())
        m.update(status="running", pid=proc.pid, process_group=proc.pid); dump(manifest_path,m)
        until = time.monotonic() + task["deadline"]
        status = None
        while status is None:
            if stop.is_set(): kill_group(proc); status = "cancelled"; break
            if time.monotonic() >= until: kill_group(proc); status = "timeout"; break
            try:
                code = proc.wait(timeout=POLL)
                if task["harness"] == "pi" and code == 0:
                    result.write_bytes(events.read_bytes())
                status = "completed" if code == 0 and result.is_file() else ("failed" if code else "incomplete")
            except subprocess.TimeoutExpired: pass
    m.update(status=status, ended_at=now(), exit_code=proc.returncode, elapsed_seconds=round(time.monotonic() - started, 3))
    if status == "incomplete": m["error"] = "harness exited successfully without a result artifact"
    dump(manifest_path,m); return m

def run_batch(base: Path, rid: str, parent=None, initial_status="running"):
    config = load(base / "batch.json"); raw_tasks = config.get("tasks")
    if not isinstance(raw_tasks, list) or not raw_tasks: raise ValueError("batch.json requires a non-empty tasks array")
    defaults = {**{"deadline": config.get("deadline", DEFAULT_DEADLINE)}, **config.get("defaults", {})};
    if not isinstance(defaults, dict): raise ValueError("defaults must be an object")
    tasks = [effective_task(base, defaults, x) for x in raw_tasks if isinstance(x, dict)]
    if len(tasks) != len(raw_tasks) or len({t['name'] for t in tasks}) != len(tasks): raise ValueError("tasks must be objects with unique names")
    limit = len(tasks)
    batch_run = base / "runs" / rid; batch_run.mkdir(parents=True, exist_ok=True)
    manifest_path = batch_run / "manifest.json"
    previous = load(manifest_path) if manifest_path.is_file() else {}
    manifest = {"run_id":rid,"status":initial_status,"started_at":previous.get("started_at", now()),"ended_at":None,"parent":parent,"worker_pid":previous.get("worker_pid"),"tasks":{}}
    dump(manifest_path, manifest); stop = threading.Event()
    old_handlers = {}
    def cancelled(_sig, _frame): stop.set()
    if threading.current_thread() is threading.main_thread():
        for sig in (signal.SIGINT, signal.SIGTERM):
            old_handlers[sig] = signal.signal(sig, cancelled)
    try:
        with ThreadPoolExecutor(max_workers=min(limit,len(tasks))) as pool:
            futures = {pool.submit(task_run,base,batch_run,t,stop): t["name"] for t in tasks}
            while futures:
                for future in list(futures):
                    if future.done():
                        name=futures.pop(future)
                        try: manifest["tasks"][name]=future.result()
                        except Exception as exc: manifest["tasks"][name]={"name":name,"status":"failed","error":str(exc),"ended_at":now()}
                        dump(manifest_path,manifest)
                latest = load(manifest_path)
                if latest.get("status") == "cancelling": stop.set()
                if parent and identity(parent["pid"]) != parent["identity"]: stop.set()
                time.sleep(POLL)
    finally:
        for sig, handler in old_handlers.items(): signal.signal(sig, handler)
    statuses = [x["status"] for x in manifest["tasks"].values()]
    manifest["ended_at"] = now()
    manifest["status"] = "cancelled" if stop.is_set() else ("completed" if statuses and all(s=="completed" for s in statuses) else "completed_with_failures")
    dump(manifest_path,manifest); dump(base / ".agent-jobs" / "current.json", {"run_id":rid,"status":manifest["status"],"updated_at":manifest["ended_at"]})
    return manifest

def current(base: Path):
    return load(base / ".agent-jobs" / "current.json")["run_id"]
def manifest(base: Path, rid=None):
    rid = rid or current(base); path=base/"runs"/rid/"manifest.json"
    if not path.is_file(): raise ValueError(f"run does not exist: {rid}")
    return load(path), path

def launch_detached_task(base: Path, batch_run: Path, task: dict):
    """Start one harness directly and persist all state needed for later reconciliation."""
    name=task["name"]; work=batch_run/name; work.mkdir(parents=True, exist_ok=True)
    prompt=safe_child(base,task["prompt"],"prompt"); result=work/"result.md"; events=work/"events.log"; stderr=work/"stderr.log"
    execution_prompt=prompt
    if task["harness"]=="codex":
        execution_prompt=work/"execution-prompt.md"
        execution_prompt.write_text(
            f"You may write only inside this job folder: {base}. Do not read, modify, or create files outside it, even if your sandbox permits broader host access.\n\n"
            + prompt.read_text(encoding="utf-8"), encoding="utf-8"
        )
    started=now()
    manifest={"name":name,"harness":task["harness"],"status":"starting","started_at":started,"ended_at":None,"exit_code":None,"deadline_seconds":task["deadline"],"artifacts":{"prompt":rel(base,prompt),"execution_prompt":rel(base,execution_prompt),"result":rel(batch_run,result),"events":rel(batch_run,events),"stderr":rel(batch_run,stderr)},"requested":{k:task.get(k) for k in ("model","thinking","web_search","ephemeral","sandbox","skills","exclude_skills","extensions","exclude_extensions") if k in task}}
    path=work/"manifest.json"
    try: cmd=adapter_command(base,task,result)
    except Exception as exc:
        manifest.update(status="preflight_failed",ended_at=now(),error=str(exc)); dump(path,manifest); return manifest
    manifest["command"]=cmd; dump(path,manifest)
    # Pi's final stdout is its result. Codex emits JSON events while writing its
    # final message itself to result.md.
    with execution_prompt.open("rb") as inp, (result if task["harness"]=="pi" else events).open("wb") as out, stderr.open("wb") as err:
        try: proc=subprocess.Popen(cmd,cwd=base,stdin=inp,stdout=out,stderr=err,**detached_process_options())
        except OSError as exc:
            manifest.update(status="failed_to_start",ended_at=now(),error=str(exc)); dump(path,manifest); return manifest
    manifest.update(status="running",pid=proc.pid,process_group=proc.pid,process_identity=identity(proc.pid)); dump(path,manifest)
    return manifest

def reconcile(base: Path, rid: str):
    batch,path=manifest(base,rid); changed=False; batch_run=base/"runs"/rid
    for name, summary in batch.get("tasks",{}).items():
        task_path=batch_run/name/"manifest.json"
        if not task_path.is_file(): continue
        task=load(task_path)
        if not terminal(task.get("status")):
            result=batch_run/name/"result.md"
            # Both harnesses write their final response only on completion.
            # Prefer that durable signal over an unreliable Windows PID probe.
            if result.is_file():
                task.update(status="completed",ended_at=now(),error=None)
            else:
                expired=dt.datetime.now(dt.timezone.utc) >= parsed_time(task["started_at"]) + dt.timedelta(seconds=task["deadline_seconds"])
                alive=isinstance(task.get("pid"),int) and identity(task["pid"]) is not None
                if expired and alive:
                    subprocess.run(["taskkill","/PID",str(task["pid"]),"/T","/F"],capture_output=True,check=False) if os.name=="nt" else None
                    task.update(status="timeout",ended_at=now(),error="deadline elapsed; process terminated")
                elif not alive:
                    task.update(status="failed",ended_at=now(),error="process exited without a result artifact")
                else: continue
            dump(task_path,task); changed=True
        if summary != task: batch["tasks"][name]=task; changed=True
    statuses=[task.get("status") for task in batch.get("tasks",{}).values()]
    if statuses and all(terminal(status) for status in statuses):
        final="completed" if all(status=="completed" for status in statuses) else "completed_with_failures"
        if batch.get("status") != final: batch.update(status=final,ended_at=now()); changed=True
    if changed: dump(path,batch); dump(base/".agent-jobs"/"current.json",{"run_id":rid,"status":batch["status"],"updated_at":now()})
    return batch

def start(args):
    base=folder(args.job_folder)
    if not (base/"batch.json").is_file(): raise ValueError("missing batch.json; run scaffold first")
    rid=run_id(); (base/"runs"/rid).mkdir(parents=True)
    if not args.background:
        result=run_batch(base,rid); print(json.dumps({"run_id":rid,"status":result["status"],"manifest":str(base/"runs"/rid/"manifest.json")})); return 0 if result["status"]=="completed" else 1
    config=load(base/"batch.json"); raw_tasks=config.get("tasks")
    if not isinstance(raw_tasks,list) or not raw_tasks: raise ValueError("batch.json requires a non-empty tasks array")
    defaults={**{"deadline":config.get("deadline",DEFAULT_DEADLINE)},**config.get("defaults",{})}
    tasks=[effective_task(base,defaults,item) for item in raw_tasks if isinstance(item,dict)]
    if len(tasks)!=len(raw_tasks) or len({task["name"] for task in tasks})!=len(tasks): raise ValueError("tasks must be objects with unique names")
    batch={"run_id":rid,"status":"launching","started_at":now(),"ended_at":None,"launch_mode":"detached","tasks":{}}
    dump(base/"runs"/rid/"manifest.json",batch)
    for task in tasks: batch["tasks"][task["name"]]=launch_detached_task(base,base/"runs"/rid,task); dump(base/"runs"/rid/"manifest.json",batch)
    failures=[name for name,task in batch["tasks"].items() if task["status"] in {"preflight_failed","failed_to_start"}]
    batch["status"]="launch_failed" if failures else "launched"; batch["launch_failures"]=failures or None
    dump(base/"runs"/rid/"manifest.json",batch); dump(base/".agent-jobs"/"current.json",{"run_id":rid,"status":batch["status"],"updated_at":now()})
    print(json.dumps({"run_id":rid,"status":batch["status"],"launch_failures":failures,"manifest":str(base/"runs"/rid/"manifest.json")})); return 1 if failures else 0

def models(args):
    if args.harness != "pi": raise ValueError(f"model listing is not implemented for harness: {args.harness}")
    return pi.list_models()

def status(args):
    base=folder(args.job_folder); m,_=manifest(base,args.run)
    if m.get("launch_mode")=="detached": m=reconcile(base,m["run_id"])
    else:
        worker=m.get("worker_pid")
        if isinstance(worker,int): m["worker_alive"]=identity(worker) is not None
    print(json.dumps(m,indent=2,sort_keys=True)); return 0
def wait(args):
    base=folder(args.job_folder); rid=args.run or current(base)
    while True:
        m,path=manifest(base,rid)
        if m.get("launch_mode")=="detached": m=reconcile(base,rid)
        if m.get("status") in {"completed","completed_with_failures","cancelled","runner_failed","launch_failed"}: print(json.dumps(m,indent=2,sort_keys=True)); return 0 if m["status"]=="completed" else 1
        worker = m.get("worker_pid")
        if isinstance(worker, int) and identity(worker) is None:
            m.update(status="runner_failed", ended_at=now(), error="batch worker exited before writing a terminal manifest")
            dump(path, m)
            continue
        time.sleep(POLL)
def result(args):
    base=folder(args.job_folder); m,_=manifest(base,args.run)
    for name,t in m.get("tasks",{}).items():
        print(f"## {name} ({t.get('status')})")
        target=base/"runs"/m["run_id"]/name/"result.md"
        if target.is_file(): print(target.read_text(encoding="utf-8").rstrip())
    return 0

def cancel(args):
    base=folder(args.job_folder); m,path=manifest(base,args.run)
    if m.get("launch_mode")=="detached":
        for name, task in m.get("tasks",{}).items():
            if task.get("status")=="running" and isinstance(task.get("pid"),int):
                if os.name=="nt": subprocess.run(["taskkill","/PID",str(task["pid"]),"/T","/F"],capture_output=True,check=False)
                task.update(status="cancelled",ended_at=now()); dump(base/"runs"/m["run_id"]/name/"manifest.json",task)
        m["tasks"]={name:load(base/"runs"/m["run_id"]/name/"manifest.json") for name in m.get("tasks",{})}
        m.update(status="cancelled",ended_at=now(),cancel_requested_at=now()); dump(path,m)
        print(json.dumps({"run_id":m["run_id"],"status":"cancelled","cancelled":True})); return 0
    if m.get("status") in {"completed", "completed_with_failures", "cancelled", "runner_failed", "launch_failed"}: 
        print(json.dumps({"run_id":m["run_id"],"status":m["status"],"cancelled":False})); return 0
    # Request cancellation through durable state; the scheduler observes it,
    # terminates every task group, and writes a terminal all-settled manifest.
    m["status"] = "cancelling"; m["cancel_requested_at"] = now(); dump(path, m)
    print(json.dumps({"run_id":m["run_id"],"status":"cancelling","cancelled":True})); return 0

def parser():
    p=argparse.ArgumentParser(description=__doc__); s=p.add_subparsers(dest="op",required=True)
    x=s.add_parser("scaffold");x.add_argument("job_folder")
    x=s.add_parser("models", help="print a harness's available model catalogue");x.add_argument("harness", choices=sorted(HARNESS_ADAPTERS))
    for op in ("run","start"): 
        x=s.add_parser(op);x.add_argument("job_folder");x.add_argument("--background",action="store_true")
    for op in ("status","wait","result", "cancel"):
        x=s.add_parser(op);x.add_argument("job_folder");x.add_argument("--run")
    x=s.add_parser("_run");x.add_argument("job_folder");x.add_argument("run_id");x.add_argument("--parent-pid",type=int);x.add_argument("--parent-identity")
    return p

def main():
    a=parser().parse_args()
    try:
        if a.op=="scaffold": scaffold(folder(a.job_folder)); return 0
        if a.op=="models": return models(a)
        if a.op in {"run","start"}:  return start(a)
        if a.op=="status": return status(a)
        if a.op=="wait": return wait(a)
        if a.op=="result": return result(a)
        if a.op=="cancel": return cancel(a)
        if a.op=="_run":
            parent = {"pid":a.parent_pid,"identity":a.parent_identity} if a.parent_pid is not None and a.parent_identity else None
            run_batch(folder(a.job_folder),a.run_id,parent); return 0
    except (OSError,RuntimeError,ValueError,json.JSONDecodeError) as e: print(f"agent-job: {e}",file=sys.stderr); return 2
if __name__=="__main__": raise SystemExit(main())
