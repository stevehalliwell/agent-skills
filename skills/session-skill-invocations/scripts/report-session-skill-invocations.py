#!/usr/bin/env python3
"""Report explicit and model-initiated Pi skill loads from saved JSONL sessions.

Usage:
  python report-session-skill-invocations.py
  python report-session-skill-invocations.py --format table
  python report-session-skill-invocations.py --sessions-dir PATH --session-glob 'project/*.jsonl'

CSV is written to stdout by default, one row per detected load. Diagnostics and
summaries are written to stderr. Exit status is nonzero only for invalid arguments or
an unreadable configured directory.
"""

from __future__ import annotations

import argparse
import csv
import json
import fnmatch
import os
import re
import statistics
import sys
from datetime import datetime
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Iterator

AGENT_DIR = Path(os.environ.get("PI_CODING_AGENT_DIR", Path.home() / ".pi" / "agent")).expanduser()
DEFAULT_SESSIONS_DIR = Path(os.environ.get("PI_CODING_AGENT_SESSION_DIR", AGENT_DIR / "sessions")).expanduser()


@dataclass(frozen=True)
class Skill:
    name: str
    path: Path
    content: str


def text_content(content: Any) -> str | None:
    if isinstance(content, str):
        return content
    if isinstance(content, list):
        return "".join(block.get("text", "") for block in content if isinstance(block, dict) and block.get("type") == "text")
    return None


def skill_name(content: str, fallback: str) -> str:
    if content.startswith("---\n"):
        for line in content.splitlines()[1:]:
            if line == "---":
                break
            if line.startswith("name:"):
                return line.partition(":")[2].strip().strip('"\'') or fallback
    return fallback


def discover_skills(skill_dirs: list[Path]) -> list[Skill]:
    skills: list[Skill] = []
    seen: set[Path] = set()
    for directory in skill_dirs:
        if not directory.is_dir():
            continue
        for path in directory.rglob("SKILL.md"):
            path = path.resolve()
            if path in seen:
                continue
            seen.add(path)
            try:
                content = path.read_text(encoding="utf-8")
            except OSError as error:
                print(f"warning: cannot read skill {path}: {error}", file=sys.stderr)
                continue
            skills.append(Skill(skill_name(content, path.parent.name), path, content))
    return skills


SKILL_COMMAND = re.compile(r"^/skill:([a-z0-9][a-z0-9-]*)(?:\s.*)?$", re.DOTALL)
SKILL_WRAPPER = re.compile(r'^<skill name="([^"]+)" location="([^"]+)">')


def user_skill_invocation(content: str, skills: list[Skill]) -> tuple[str, str | None] | None:
    command = SKILL_COMMAND.fullmatch(content.strip())
    if command:
        name = command.group(1)
        known_skill = next((skill for skill in skills if skill.name == name), None)
        return name, str(known_skill.path) if known_skill else None

    wrapper = SKILL_WRAPPER.match(content)
    if wrapper:
        return wrapper.group(1), wrapper.group(2)

    for skill in skills:
        # Older Pi versions persisted the complete skill, optionally followed by User: arguments.
        if content == skill.content or content.startswith(skill.content.rstrip() + "\n\nUser:"):
            return skill.name, str(skill.path)
    return None


def skill_from_read_path(read_path: str, skills: list[Skill]) -> tuple[str, str] | None:
    if Path(read_path).name.casefold() != "skill.md":
        return None
    known_skill = next((skill for skill in skills if Path(read_path).resolve() == skill.path), None)
    return (known_skill.name, str(known_skill.path)) if known_skill else (Path(read_path).parent.name, read_path)


def json_lines(path: Path) -> Iterator[tuple[int, dict[str, Any]]]:
    try:
        with path.open(encoding="utf-8") as file:
            for number, line in enumerate(file, 1):
                try:
                    entry = json.loads(line)
                except json.JSONDecodeError:
                    print(f"warning: invalid JSON at {path}:{number}", file=sys.stderr)
                    continue
                if isinstance(entry, dict):
                    yield number, entry
    except OSError as error:
        print(f"warning: cannot read session {path}: {error}", file=sys.stderr)


def session_files(sessions_dir: Path, patterns: list[str]) -> list[Path]:
    matches: set[Path] = set()
    for path in sessions_dir.rglob("*.jsonl"):
        relative_path = path.relative_to(sessions_dir).as_posix()
        if any(fnmatch.fnmatchcase(relative_path, pattern) for pattern in patterns):
            matches.add(path)
    return sorted(matches)


def report(session_paths: list[Path], skills: list[Skill]) -> Iterator[dict[str, Any]]:
    for session_file in session_paths:
        session_id = None
        user_invocations: dict[str, str] = {}
        for line_number, entry in json_lines(session_file):
            if entry.get("type") == "session":
                session_id = entry.get("id")
                continue
            if entry.get("type") != "message":
                continue
            message = entry.get("message")
            if not isinstance(message, dict):
                continue

            if message.get("role") == "user":
                content = text_content(message.get("content"))
                if content is None:
                    continue
                invocation = user_skill_invocation(content, skills)
                if invocation:
                    name, skill_file = invocation
                    entry_id = entry.get("id")
                    if isinstance(entry_id, str):
                        user_invocations[entry_id] = name
                    yield {"conversation_hash": session_id, "timestamp": entry.get("timestamp"), "jsonl_line": line_number, "skill_name": name, "loaded_by": "user", "session_file": str(session_file), "skill_file": skill_file}

            if message.get("role") == "assistant":
                for block in message.get("content", []):
                    if not isinstance(block, dict) or block.get("type") != "toolCall" or block.get("name") != "read":
                        continue
                    arguments = block.get("arguments")
                    if not isinstance(arguments, dict):
                        continue
                    read_path = arguments.get("path")
                    if not isinstance(read_path, str):
                        continue
                    loaded_skill = skill_from_read_path(read_path, skills)
                    if not loaded_skill:
                        continue
                    name, skill_file = loaded_skill
                    if user_invocations.get(entry.get("parentId")) == name:
                        continue
                    yield {"conversation_hash": session_id, "timestamp": entry.get("timestamp"), "jsonl_line": line_number, "skill_name": name, "loaded_by": "agent", "session_file": str(session_file), "skill_file": skill_file}


def conversation_messages(session_paths: list[Path]) -> tuple[dict[str, int], dict[str, list[int]]]:
    messages_by_date: dict[str, int] = {}
    lines_by_session: dict[str, list[int]] = {}
    for session_file in session_paths:
        message_lines: list[int] = []
        for line_number, entry in json_lines(session_file):
            message = entry.get("message")
            if not isinstance(message, dict) or message.get("role") not in ("user", "assistant"):
                continue
            message_lines.append(line_number)
            timestamp = entry.get("timestamp")
            if isinstance(timestamp, str):
                date = timestamp[:10]
                messages_by_date[date] = messages_by_date.get(date, 0) + 1
        lines_by_session[str(session_file)] = message_lines
    return messages_by_date, lines_by_session


def write_csv(path: Path, fieldnames: list[str], rows: list[dict[str, Any]]) -> None:
    with path.open("w", newline="", encoding="utf-8") as file:
        writer = csv.DictWriter(file, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(rows)


def write_analysis(output_dir: Path, records: list[dict[str, Any]], session_paths: list[Path], skills: list[Skill]) -> None:
    output_dir.mkdir(parents=True, exist_ok=True)
    messages_by_date, lines_by_session = conversation_messages(session_paths)
    total_messages = sum(messages_by_date.values())

    summary: dict[tuple[str, str], list[dict[str, Any]]] = {}
    over_time: dict[tuple[str, str, str], list[dict[str, Any]]] = {}
    for record in records:
        source_key = (record["skill_name"], record["loaded_by"])
        summary.setdefault(source_key, []).append(record)
        timestamp = record.get("timestamp")
        if isinstance(timestamp, str):
            over_time.setdefault((timestamp[:10], *source_key), []).append(record)

    summary_rows = [{
        "skill_name": name,
        "loaded_by": loaded_by,
        "loads": len(group),
        "sessions_with_loads": len({record["conversation_hash"] for record in group}),
        "all_conversation_messages": total_messages,
        "loads_per_100_messages": round(100 * len(group) / total_messages, 4) if total_messages else 0,
    } for (name, loaded_by), group in sorted(summary.items())]
    write_csv(output_dir / "skill-usage-summary.csv", list(summary_rows[0]) if summary_rows else ["skill_name", "loaded_by", "loads", "sessions_with_loads", "all_conversation_messages", "loads_per_100_messages"], summary_rows)

    over_time_rows = [{
        "date": date,
        "skill_name": name,
        "loaded_by": loaded_by,
        "loads": len(group),
        "all_conversation_messages": messages_by_date.get(date, 0),
        "loads_per_100_messages": round(100 * len(group) / messages_by_date[date], 4) if messages_by_date.get(date) else 0,
    } for (date, name, loaded_by), group in sorted(over_time.items())]
    write_csv(output_dir / "skill-usage-over-time.csv", list(over_time_rows[0]) if over_time_rows else ["date", "skill_name", "loaded_by", "loads", "all_conversation_messages", "loads_per_100_messages"], over_time_rows)

    proximity_rows: list[dict[str, Any]] = []
    by_session: dict[str, list[dict[str, Any]]] = {}
    for record in records:
        by_session.setdefault(record["session_file"], []).append(record)
    for session_file, group in by_session.items():
        ordered = sorted(group, key=lambda record: record["jsonl_line"])
        message_lines = lines_by_session.get(session_file, [])
        for previous, current in zip(ordered, ordered[1:]):
            previous_timestamp, current_timestamp = previous.get("timestamp"), current.get("timestamp")
            seconds = None
            if isinstance(previous_timestamp, str) and isinstance(current_timestamp, str):
                seconds = round((datetime.fromisoformat(current_timestamp.replace("Z", "+00:00")) - datetime.fromisoformat(previous_timestamp.replace("Z", "+00:00"))).total_seconds(), 3)
            messages_between = sum(previous["jsonl_line"] < line < current["jsonl_line"] for line in message_lines)
            proximity_rows.append({
                "conversation_hash": current["conversation_hash"], "previous_skill_name": previous["skill_name"], "previous_loaded_by": previous["loaded_by"], "skill_name": current["skill_name"], "loaded_by": current["loaded_by"], "previous_timestamp": previous_timestamp, "timestamp": current_timestamp, "seconds_since_previous_load": seconds, "messages_between_loads": messages_between,
            })
    write_csv(output_dir / "skill-proximity.csv", ["conversation_hash", "previous_skill_name", "previous_loaded_by", "skill_name", "loaded_by", "previous_timestamp", "timestamp", "seconds_since_previous_load", "messages_between_loads"], proximity_rows)

    proximity_summary: dict[tuple[str, str, str, str], list[dict[str, Any]]] = {}
    for row in proximity_rows:
        proximity_summary.setdefault((row["previous_skill_name"], row["previous_loaded_by"], row["skill_name"], row["loaded_by"]), []).append(row)
    proximity_summary_rows = []
    for key, group in sorted(proximity_summary.items()):
        seconds = [row["seconds_since_previous_load"] for row in group if row["seconds_since_previous_load"] is not None]
        messages = [row["messages_between_loads"] for row in group]
        proximity_summary_rows.append({"previous_skill_name": key[0], "previous_loaded_by": key[1], "skill_name": key[2], "loaded_by": key[3], "pairs": len(group), "median_seconds_between_loads": statistics.median(seconds) if seconds else None, "median_messages_between_loads": statistics.median(messages) if messages else None})
    write_csv(output_dir / "skill-proximity-summary.csv", ["previous_skill_name", "previous_loaded_by", "skill_name", "loaded_by", "pairs", "median_seconds_between_loads", "median_messages_between_loads"], proximity_summary_rows)

    loads_by_skill: dict[str, list[dict[str, Any]]] = {}
    for record in records:
        loads_by_skill.setdefault(record["skill_name"], []).append(record)
    candidate_rows: list[dict[str, Any]] = []
    for skill in skills:
        loads = loads_by_skill.get(skill.name, [])
        if not loads:
            candidate_rows.append({"candidate_type": "unused_skill", "skills": skill.name, "loads": 0, "sessions": 0, "close_pairs": None, "pair_share": None, "evidence": "No detected user or agent loads.", "recommendation": "Review removal, rename, or trigger description."})
        elif len(loads) <= 3:
            candidate_rows.append({"candidate_type": "rare_skill", "skills": skill.name, "loads": len(loads), "sessions": len({record["conversation_hash"] for record in loads}), "close_pairs": None, "pair_share": None, "evidence": "Three or fewer detected loads.", "recommendation": "Review whether to remove, rename, or improve triggering guidance."})

    close_pairs: dict[tuple[str, str], int] = {}
    for row in proximity_rows:
        seconds = row["seconds_since_previous_load"]
        if row["previous_skill_name"] == row["skill_name"] or row["messages_between_loads"] > 5 or (seconds is not None and seconds > 3600):
            continue
        pair = (row["previous_skill_name"], row["skill_name"])
        close_pairs[pair] = close_pairs.get(pair, 0) + 1
    for (previous_name, name), pairs in sorted(close_pairs.items()):
        smaller_load_count = min(len(loads_by_skill.get(previous_name, [])), len(loads_by_skill.get(name, [])))
        pair_share = pairs / smaller_load_count if smaller_load_count else 0
        if pairs >= 5 and pair_share >= 0.5:
            candidate_rows.append({"candidate_type": "frequent_close_pair", "skills": f"{previous_name} -> {name}", "loads": smaller_load_count, "sessions": None, "close_pairs": pairs, "pair_share": round(pair_share, 4), "evidence": "Consecutive loads within five messages and one hour.", "recommendation": "Review shared prerequisites, direct cross-reference, overlapping scope, or trigger guidance; do not merge without workflow review."})
    write_csv(output_dir / "skill-review-candidates.csv", ["candidate_type", "skills", "loads", "sessions", "close_pairs", "pair_share", "evidence", "recommendation"], candidate_rows)


def main() -> int:
    parser = argparse.ArgumentParser(description="Report Pi skill loads from saved sessions.")
    parser.add_argument("--sessions-dir", type=Path, default=DEFAULT_SESSIONS_DIR, help="Session directory (default: %(default)s)")
    parser.add_argument("--session-glob", action="append", default=[], metavar="GLOB", help="Session-file glob relative to --sessions-dir; repeatable. '*' matches all sessions.")
    parser.add_argument("--skills-dir", type=Path, action="append", default=[], help="Skill directory to match; repeatable (default: global Pi skills)")
    parser.add_argument("--analysis-dir", type=Path, help="Write usage and proximity CSV analyses to this directory")
    parser.add_argument("--format", choices=("csv", "jsonl", "table"), default="csv", help="Output format (default: csv)")
    args = parser.parse_args()

    sessions_dir = args.sessions_dir.expanduser()
    if not sessions_dir.is_dir():
        parser.error(f"sessions directory does not exist: {sessions_dir}")

    session_paths = session_files(sessions_dir, args.session_glob or ["*"])
    skill_dirs = [directory.expanduser() for directory in args.skills_dir] or [AGENT_DIR / "skills", Path.home() / ".agents" / "skills"]
    skills = discover_skills(skill_dirs)
    records = list(report(session_paths, skills))

    if args.analysis_dir:
        write_analysis(args.analysis_dir.expanduser(), records, session_paths, skills)

    if args.format == "jsonl":
        for record in records:
            print(json.dumps(record, separators=(",", ":")))
    elif args.format == "csv":
        writer = csv.DictWriter(sys.stdout, fieldnames=["conversation_hash", "timestamp", "jsonl_line", "skill_name", "loaded_by", "session_file", "skill_file"])
        writer.writeheader()
        writer.writerows(records)
    else:
        print("conversation_hash\ttimestamp\tjsonl_line\tskill_name\tloaded_by")
        for record in records:
            print(f"{record['conversation_hash']}\t{record['timestamp']}\t{record['jsonl_line']}\t{record['skill_name']}\t{record['loaded_by']}")

    print(f"scanned {len(session_paths)} sessions; matched {len(records)} skill loads across {len(skills)} skills", file=sys.stderr)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
