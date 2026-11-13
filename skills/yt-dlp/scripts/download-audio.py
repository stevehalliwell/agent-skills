#!/usr/bin/env python3
"""Download one original audio stream; leave conversion to FFmpeg.

Requires yt-dlp on PATH or installed in this Python environment.
Stdout: JSON {"path": absolute_path} or {"error": {"code", "message"}}.
Diagnostics: stderr. Exit 0: verified file; 1: runtime failure; 2: usage error.
"""
import argparse
import importlib.util
import json
from pathlib import Path
import shutil
import subprocess
import sys
from urllib.parse import urlsplit


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("url", help="Public HTTP(S) video URL (quote it in the shell)")
    parser.add_argument("--output-dir", required=True, type=Path, help="Download directory; created if absent")
    args = parser.parse_args()
    if urlsplit(args.url).scheme not in ("http", "https") or not urlsplit(args.url).netloc:
        parser.error("url must be an HTTP(S) URL")

    executable = shutil.which("yt-dlp")
    if executable:
        command = [executable]
    elif importlib.util.find_spec("yt_dlp") is not None:
        command = [sys.executable, "-m", "yt_dlp"]
    else:
        raise RuntimeError("Install yt-dlp on PATH or run: python -m pip install yt-dlp")

    directory = args.output_dir.expanduser().resolve()
    directory.mkdir(parents=True, exist_ok=True)
    # Ignore user configuration so cookies, playlists, and postprocessing stay opt-in.
    command += [
        "--ignore-config", "--no-playlist", "--no-overwrites", "--no-progress",
        "--encoding", "utf-8", "-f", "bestaudio", "-P", str(directory),
        "-o", "%(id)s.%(ext)s", "--print", "after_move:filepath", "--", args.url,
    ]
    result = subprocess.run(command, stdout=subprocess.PIPE, text=True, encoding="utf-8")
    if result.returncode:
        raise RuntimeError(f"yt-dlp exited with code {result.returncode}; see stderr. No automatic cookie retry.")
    paths = result.stdout.strip().splitlines()
    if len(paths) != 1:
        raise RuntimeError("Expected one downloaded file path from yt-dlp.")
    path = Path(paths[0]).resolve()
    if not path.is_file() or path.stat().st_size == 0:
        raise RuntimeError("yt-dlp did not produce a non-empty audio file.")
    print(json.dumps({"path": str(path)}, ensure_ascii=False))


if __name__ == "__main__":
    try:
        main()
    except (OSError, RuntimeError, ValueError) as error:
        message = str(error)
        print(f"download: {message}", file=sys.stderr)
        print(json.dumps({"error": {"code": "download_failed", "message": message}}))
        sys.exit(1)
