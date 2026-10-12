#!/usr/bin/env python3
"""Pass one host provider credential to owned test containers through stdin.

Usage: python <skill-dir>/scripts/pass-auth.py CONTAINER [CONTAINER ...]
Requires containers labelled purpose=herdr-pi-proof and private tmpfs agent dirs.
Never prints credentials, writes host credential copies, or puts secrets in argv.
Stdout is JSON; failures return exit 1. OAuth tokens need 5 minutes remaining to
reduce refresh-token reuse during this short test. This is not a credential proxy.
"""
import argparse
import json
import os
from pathlib import Path
import subprocess
import sys
import time

AGENT_DIR = "/home/node/.pi/agent"
WRITER = """
const fs = require('node:fs');
let input = '';
process.stdin.setEncoding('utf8');
process.stdin.on('data', chunk => input += chunk);
process.stdin.on('end', () => {
  try {
    const auth = JSON.parse(input);
    const target = '/home/node/.pi/agent/auth.json';
    fs.writeFileSync(target + '.tmp', JSON.stringify(auth), {mode: 0o600});
    fs.renameSync(target + '.tmp', target);
  } catch (_) { process.exitCode = 1; }
});
"""


def transfer(containers, source, provider, allow_root=False):
    # Validate every target before opening host credentials or modifying a child.
    env = dict(os.environ, MSYS_NO_PATHCONV="1")
    for name in containers:
        result = subprocess.run(["docker", "inspect", name], capture_output=True, env=env)
        if result.returncode:
            raise ValueError("Target container unavailable: " + name)
        info = json.loads(result.stdout)[0]
        if (info.get("Config", {}).get("Labels") or {}).get("purpose") != "herdr-pi-proof":
            raise ValueError("Target is not an owned Pi test container: " + name)
        if not info.get("State", {}).get("Running"):
            raise ValueError("Target container is not running: " + name)
        if AGENT_DIR not in (info.get("HostConfig", {}).get("Tmpfs") or {}):
            raise ValueError("Target lacks temporary credential storage: " + name)
        allowed_users = ("node", "1000:1000", "0:0") if allow_root else ("node", "1000:1000")
        if info.get("Config", {}).get("User") not in allowed_users:
            raise ValueError("Unexpected target container user; guest-root handoff requires --allow-root: " + name)
    try:
        credential = json.loads(source.read_text(encoding="utf-8"))[provider]
    except (OSError, ValueError, KeyError):
        raise ValueError("Cannot load selected provider from host auth.json") from None
    if not isinstance(credential, dict) or credential.get("type") not in ("oauth", "api_key"):
        raise ValueError("Unsupported provider credential type")
    if credential["type"] == "oauth":
        expires = credential.get("expires")
        if not isinstance(expires, (int, float)) or expires < time.time() * 1000 + 300000:
            raise ValueError("Host OAuth credential needs refresh before this short test")
        if not all(isinstance(credential.get(key), str) and credential[key] for key in ("access", "refresh")):
            raise ValueError("Host OAuth credential is incomplete")
    elif not isinstance(credential.get("key"), str) or not credential["key"] or credential["key"].startswith("!"):
        raise ValueError("Only literal API keys can be transferred by this helper")
    payload = json.dumps({provider: credential}).encode("utf-8")
    completed = []
    for name in containers:
        result = subprocess.run(["docker", "exec", "-i", name, "node", "-e", WRITER],
                                input=payload, capture_output=True, env=env)
        if result.returncode:
            raise ValueError("Credential handoff failed for " + name + "; prior targets: " + ", ".join(completed))
        completed.append(name)
    return {"status": "provisioned", "provider": provider, "containers": completed,
            "storage": "private runtime tmpfs", "host_auth_modified": False}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("containers", nargs="+")
    parser.add_argument("--provider", default="openai")
    parser.add_argument("--source", type=Path, default=Path.home() / ".pi/agent/auth.json")
    parser.add_argument("--allow-root", action="store_true", help="Permit explicit 0:0 guest-root profile; default rejects root targets")
    args = parser.parse_args()
    try:
        result = transfer(args.containers, args.source, args.provider, args.allow_root)
    except (ValueError, OSError, subprocess.SubprocessError):
        # Suppress raw exceptions; they may include credential-bearing input.
        print(json.dumps({"status": "error", "code": "handoff_failed",
                          "message": "Check owned running targets, private tmpfs storage, provider credential and OAuth expiry."}))
        return 1
    print(json.dumps(result))
    return 0


if __name__ == "__main__":
    sys.exit(main())
