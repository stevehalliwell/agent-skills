"""Pi CLI adapter for agent-job.

Pi children have no discovered resources. This module resolves only explicitly
named or path-based skill/extension allowlist entries and builds one Pi command.
"""
from __future__ import annotations
from pathlib import Path
import shutil
import subprocess


def list_models() -> int:
    """Print Pi's currently available model catalogue verbatim."""
    exe = shutil.which("pi")
    if not exe:
        raise RuntimeError("pi binary is not on PATH")
    return subprocess.run([exe, "--list-models"], check=False).returncode


def _resource_path(base: Path, raw: str, label: str) -> Path:
    raw_path = Path(raw).expanduser()
    candidates = [raw_path] if raw_path.is_absolute() else [base / raw_path]
    project = next((parent for parent in (base, *base.parents) if (parent / ".git").exists()), None)
    roots = [Path.home() / ".pi" / "agent"]
    if project:
        roots.append(project / ".pi")
    for root in roots:
        if label == "skill":
            candidates += [root / "skills" / raw, root / "skills" / f"{raw}.md"]
        else:
            candidates += [root / "extensions" / raw, root / "extensions" / f"{raw}.ts", root / "extensions" / f"{raw}.js", root / "extensions" / f"{raw}.mjs"]
    for candidate in candidates:
        path = candidate.resolve()
        if path.exists():
            return path
    raise ValueError(f"unknown {label} {raw!r}; name a global/project resource or an existing path")


def _selected(values: object, blocked: object) -> list[str]:
    items = values if isinstance(values, list) else []
    denied = {str(item) for item in (blocked if isinstance(blocked, list) else [])}
    return [str(item) for item in items if str(item) not in denied and Path(str(item)).name not in denied]


def command(base: Path, task: dict, _result: Path) -> list[str]:
    exe = shutil.which("pi")
    if not exe:
        raise RuntimeError("pi binary is not on PATH")
    cmd = [
        exe, "--no-session", "--no-skills", "--no-extensions", "--no-context-files",
        "--no-prompt-templates", "--no-themes", "--no-approve",
    ]
    for raw in _selected(task.get("skills"), task.get("exclude_skills")):
        cmd += ["--skill", str(_resource_path(base, raw, "skill"))]
    for raw in _selected(task.get("extensions"), task.get("exclude_extensions")):
        cmd += ["--extension", str(_resource_path(base, raw, "extension"))]
    if task.get("model"):
        cmd += ["--model", str(task["model"])]
    if task.get("thinking"):
        cmd += ["--thinking", str(task["thinking"])]
    prompt = (base / task["prompt"]).resolve()
    if base not in prompt.parents and prompt != base:
        raise ValueError(f"prompt must be inside the job folder: {task['prompt']}")
    return cmd + ["--print", "@" + str(prompt)]
