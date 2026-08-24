#!/usr/bin/env python
"""Validate the structural requirements of an OKF v0.2 knowledge bundle.

Usage:
  python validate_okf.py PATH/TO/okf
  python validate_okf.py PATH/TO/okf --drift --json

Install the parser once for this skill:
  python -m pip install -r requirements.txt

Exit codes: 0 conformant, 1 conformance errors, 2 invalid invocation/dependency.
Broken links and drift are warnings unless --strict-links or --strict-drift is used.
"""
from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from dataclasses import dataclass, asdict
from datetime import date, datetime
from pathlib import Path
from typing import Any
from urllib.parse import urlparse

try:
    import yaml
except ImportError:
    print("PyYAML is required. Install it with: python -m pip install -r requirements.txt", file=sys.stderr)
    raise SystemExit(2)

FRONTMATTER = re.compile(r"\A---[ \t]*\r?\n(.*?)\r?\n---[ \t]*(?:\r?\n|\Z)", re.DOTALL)
LINK = re.compile(r"(?<!!)\[[^\]]+\]\(([^)]+)\)")
DATE_HEADING = re.compile(r"^## (\d{4}-\d{2}-\d{2})\s*$", re.MULTILINE)


@dataclass
class Finding:
    level: str
    path: str
    message: str


def yaml_block(path: Path, text: str) -> tuple[dict[str, Any] | None, str | None]:
    match = FRONTMATTER.match(text.lstrip("\ufeff"))
    if not match:
        return None, "missing YAML frontmatter delimited by ---"
    try:
        data = yaml.safe_load(match.group(1))
    except yaml.YAMLError as exc:
        return None, f"invalid YAML frontmatter: {exc.problem or str(exc)}"
    if not isinstance(data, dict):
        return None, "frontmatter must be a YAML mapping"
    return data, None


def local_resource(resource: Any, bundle: Path) -> Path | None:
    if not isinstance(resource, str) or not resource:
        return None
    parsed = urlparse(resource)
    if parsed.scheme and parsed.scheme != "file":
        return None
    candidate = Path(parsed.path if parsed.scheme == "file" else resource)
    return candidate if candidate.is_absolute() else bundle / candidate


def git_last_change(path: Path, root: Path) -> datetime | None:
    try:
        result = subprocess.run(
            ["git", "-C", str(root), "log", "-1", "--format=%aI", "--", str(path)],
            check=False, capture_output=True, text=True,
        )
    except OSError:
        return None
    stamp = result.stdout.strip()
    if result.returncode or not stamp:
        return None
    try:
        return datetime.fromisoformat(stamp.replace("Z", "+00:00"))
    except ValueError:
        return None


def validate(bundle: Path, check_drift: bool) -> list[Finding]:
    findings: list[Finding] = []
    concepts: dict[Path, tuple[str, dict[str, Any]]] = {}
    markdown_files = sorted(path for path in bundle.rglob("*.md") if path.is_file())
    if not markdown_files:
        return [Finding("error", ".", "bundle contains no Markdown files")]

    for path in markdown_files:
        relative = path.relative_to(bundle)
        try:
            text = path.read_text(encoding="utf-8")
        except (OSError, UnicodeError) as exc:
            findings.append(Finding("error", str(relative), f"Markdown file must be readable UTF-8: {exc}"))
            continue
        if path.name == "index.md":
            data, error = yaml_block(path, text)
            if data is not None:
                if relative.parent != Path("."):
                    findings.append(Finding("error", str(relative), "only the bundle-root index.md may have frontmatter"))
                elif set(data) != {"okf_version"}:
                    findings.append(Finding("error", str(relative), "index.md frontmatter may contain only okf_version"))
            elif not error.startswith("missing"):
                findings.append(Finding("error", str(relative), error))
            continue
        if path.name == "log.md":
            if text.lstrip("\ufeff").startswith("---"):
                findings.append(Finding("error", str(relative), "log.md must not begin with a frontmatter delimiter"))
            headings = DATE_HEADING.findall(text)
            try:
                parsed_dates = [date.fromisoformat(value) for value in headings]
                if parsed_dates != sorted(parsed_dates, reverse=True):
                    findings.append(Finding("error", str(relative), "log date headings must be newest first"))
            except ValueError:
                findings.append(Finding("error", str(relative), "log date headings must use YYYY-MM-DD"))
            continue

        data, error = yaml_block(path, text)
        if error:
            findings.append(Finding("error", str(relative), error))
            continue
        concept_type = data.get("type")
        if not isinstance(concept_type, str) or not concept_type.strip():
            findings.append(Finding("error", str(relative), "concept frontmatter requires a non-empty string type"))
        concepts[path] = (text, data)

    for path, (text, data) in concepts.items():
        relative = path.relative_to(bundle)
        for target in LINK.findall(text):
            target = target.split("#", 1)[0].strip()
            if not target or urlparse(target).scheme or target.startswith("#"):
                continue
            destination = (bundle / target.lstrip("/")) if target.startswith("/") else (path.parent / target)
            if not destination.resolve().exists():
                findings.append(Finding("warning", str(relative), f"broken concept link: {target}"))
        if check_drift:
            resource = local_resource(data.get("resource"), bundle)
            generated = data.get("generated")
            generated_at = generated.get("at") if isinstance(generated, dict) else None
            if resource and resource.exists() and isinstance(generated_at, str):
                try:
                    concept_time = datetime.fromisoformat(generated_at.replace("Z", "+00:00"))
                    source_time = git_last_change(resource, bundle)
                    if source_time and source_time > concept_time:
                        findings.append(Finding("warning", str(relative), f"resource changed after generated.at: {resource}"))
                except ValueError:
                    findings.append(Finding("warning", str(relative), "generated.at is not an ISO 8601 datetime; drift could not be checked"))
    return findings


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("bundle", type=Path, help="path to the OKF bundle root")
    parser.add_argument("--drift", action="store_true", help="compare local resources to Git history")
    parser.add_argument("--strict-links", action="store_true", help="treat broken-link warnings as errors")
    parser.add_argument("--strict-drift", action="store_true", help="treat drift warnings as errors")
    parser.add_argument("--json", action="store_true", help="write findings as JSON")
    args = parser.parse_args()
    bundle = args.bundle.resolve()
    if not bundle.is_dir():
        parser.error(f"bundle directory does not exist: {bundle}")
    findings = validate(bundle, args.drift or args.strict_drift)
    if args.json:
        print(json.dumps([asdict(finding) for finding in findings], indent=2))
    else:
        for finding in findings:
            print(f"{finding.level.upper():7} {finding.path}: {finding.message}")
        if not findings:
            print("OKF bundle is conformant.")
    errors = [f for f in findings if f.level == "error"]
    if args.strict_links:
        errors += [f for f in findings if f.level == "warning" and "broken concept link" in f.message]
    if args.strict_drift:
        errors += [f for f in findings if f.level == "warning" and "resource changed" in f.message]
    return 1 if errors else 0


if __name__ == "__main__":
    raise SystemExit(main())
