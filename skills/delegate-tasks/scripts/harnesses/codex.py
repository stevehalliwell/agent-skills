"""Codex CLI adapter for agent-job.

The generic runner owns lifecycle; this module only validates Codex settings and
constructs one synchronous `codex exec` command.
"""
from __future__ import annotations
import json
from pathlib import Path
import shutil
import subprocess


def command(base: Path, task: dict, result: Path) -> list[str]:
    exe = shutil.which("codex")
    if not exe:
        raise RuntimeError("codex binary is not on PATH")
    model = task.get("model", "gpt-5.6-terra")
    thinking = task.get("thinking", "medium")
    web_search = bool(task.get("web_search", False))
    ephemeral = bool(task.get("ephemeral", True))
    sandbox = task.get("sandbox", "workspace-write")
    if sandbox not in {"workspace-write", "danger-full-access"}:
        raise ValueError(
            f"Codex sandbox must be 'workspace-write' or 'danger-full-access', got {sandbox!r}"
        )

    catalogue = subprocess.run([exe, "debug", "models"], capture_output=True)
    stdout = catalogue.stdout.decode("utf-8", errors="replace") if catalogue.stdout else ""
    stderr = catalogue.stderr.decode("utf-8", errors="replace") if catalogue.stderr else ""
    if catalogue.returncode != 0:
        raise RuntimeError("could not query Codex model catalogue: " + (stderr.strip() or stdout.strip()))
    try:
        available = {
            item["slug"]: {level["effort"] for level in item.get("supported_reasoning_levels", [])}
            for item in json.loads(stdout)["models"]
        }
    except (KeyError, TypeError, json.JSONDecodeError) as exc:
        raise RuntimeError(f"Codex returned an invalid model catalogue: {exc}") from exc
    if model not in available:
        raise ValueError(f"Codex model is unavailable: {model}")
    if thinking not in available[model]:
        raise ValueError(f"Codex thinking level {thinking!r} is unavailable for {model}")

    cmd = [exe, "--sandbox", sandbox, "--ask-for-approval", "never"]
    if web_search:
        cmd.append("--search")
    cmd.append("exec")
    if ephemeral:
        cmd.append("--ephemeral")
    return cmd + [
        "--skip-git-repo-check", "-C", str(base), "--model", str(model),
        "-c", f"model_reasoning_effort={thinking}", "--output-last-message", str(result), "--json", "-",
    ]
