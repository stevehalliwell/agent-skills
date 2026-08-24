#!/usr/bin/env python3
"""Convert one local document through a local Docling Serve container.

Requires Python 3 standard library only. The service must be listening on loopback;
no document is sent to an external service.

Usage:
  convert.py SOURCE --output-dir OUTPUT [--images|--no-images] [--ocr-lang LANG] [--async]
"""

from __future__ import annotations

import argparse
import base64
import json
import mimetypes
from pathlib import Path
import re
import shutil
import sys
import time
import uuid
from urllib.parse import quote
from urllib.error import HTTPError, URLError
from urllib.request import Request, urlopen

DEFAULT_ENDPOINT = "http://127.0.0.1:5001/v1/convert/file"
ASYNC_STATUS_TERMINAL = {"failure", "partial_success", "skipped", "success"}
DATA_IMAGE_RE = re.compile(r"data:image/([A-Za-z0-9.+-]+);base64,([A-Za-z0-9+/=]+)")
EXTENSIONS = {"jpeg": "jpg", "svg+xml": "svg", "x-icon": "ico"}


def multipart_body(fields: list[tuple[str, str]], source: Path) -> tuple[bytes, str]:
    boundary = f"----docling-local-{uuid.uuid4().hex}"
    chunks: list[bytes] = []

    def add(value: str | bytes) -> None:
        chunks.append(value if isinstance(value, bytes) else value.encode("utf-8"))

    for name, value in fields:
        add(f"--{boundary}\r\n")
        add(f'Content-Disposition: form-data; name="{name}"\r\n\r\n')
        add(value)
        add("\r\n")

    mime = mimetypes.guess_type(source.name)[0] or "application/octet-stream"
    add(f"--{boundary}\r\n")
    add(f'Content-Disposition: form-data; name="files"; filename="{source.name}"\r\n')
    add(f"Content-Type: {mime}\r\n\r\n")
    chunks.append(source.read_bytes())
    add("\r\n")
    add(f"--{boundary}--\r\n")
    return b"".join(chunks), boundary


def request_json(request: Request, endpoint: str, timeout: float) -> dict:
    try:
        with urlopen(request, timeout=timeout) as response:
            return json.load(response)
    except HTTPError as exc:
        detail = exc.read().decode("utf-8", errors="replace")
        raise RuntimeError(f"Docling request failed ({exc.code}): {detail}") from exc
    except URLError as exc:
        raise RuntimeError(f"Cannot reach local Docling service at {endpoint}: {exc.reason}") from exc


def convert(source: Path, fields: list[tuple[str, str]], endpoint: str) -> dict:
    body, boundary = multipart_body(fields, source)
    request = Request(
        endpoint,
        data=body,
        method="POST",
        headers={"Accept": "application/json", "Content-Type": f"multipart/form-data; boundary={boundary}"},
    )
    return request_json(request, endpoint, 1800)


def convert_async(source: Path, fields: list[tuple[str, str]], endpoint: str, poll_interval: float, timeout: float) -> tuple[dict, dict]:
    suffix = "/v1/convert/file"
    if not endpoint.endswith(suffix):
        raise RuntimeError(f"async mode requires an endpoint ending in {suffix}: {endpoint}")
    task = convert(source, fields, f"{endpoint}/async")
    task_id = task.get("task_id")
    if not task_id:
        raise RuntimeError(f"Docling async submission returned no task ID: {task}")

    base_url = endpoint[: -len(suffix)]
    status_url = f"{base_url}/v1/status/poll/{quote(str(task_id), safe='')}"
    result_url = f"{base_url}/v1/result/{quote(str(task_id), safe='')}"
    deadline = time.monotonic() + timeout if timeout else None
    while True:
        remaining = deadline - time.monotonic() if deadline else poll_interval
        if deadline and remaining <= 0:
            raise RuntimeError(f"Docling async task {task_id} did not finish within {timeout:g} seconds")
        wait = min(poll_interval, remaining) if deadline else poll_interval
        status = request_json(Request(f"{status_url}?wait={wait:g}", headers={"Accept": "application/json"}), status_url, wait + 30)
        task["last_status"] = status
        if status.get("task_status") in ASYNC_STATUS_TERMINAL:
            result = request_json(Request(result_url, headers={"Accept": "application/json"}), result_url, 60)
            return result, task


def save_images(markdown: str, images_dir: Path, stem: str) -> tuple[str, list[tuple[str, Path]]]:
    images_dir.mkdir(parents=True, exist_ok=True)
    extracted: list[tuple[str, Path]] = []

    def replace(match: re.Match[str]) -> str:
        mime_subtype, encoded = match.groups()
        try:
            image = base64.b64decode(encoded, validate=True)
        except ValueError:
            return match.group(0)
        extension = EXTENSIONS.get(mime_subtype, mime_subtype)
        name = f"{stem}-{len(extracted) + 1:03d}.{extension}"
        path = images_dir / name
        path.write_bytes(image)
        extracted.append((name, path))
        return f"images/{name}"

    converted = DATA_IMAGE_RE.sub(replace, markdown)
    if not extracted:
        images_dir.rmdir()
    return converted, extracted


def append_image_ocr(markdown: str, name: str, text: str) -> str:
    marker = f"](images/{name})"
    replacement = f"](images/{name})\n\n**Image OCR**\n\n{text.strip()}"
    if marker not in markdown:
        raise ValueError(f"cannot find Markdown link for images/{name}")
    return markdown.replace(marker, replacement, 1)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path, help="Local input document")
    parser.add_argument("--output-dir", required=True, type=Path)
    parser.add_argument("--endpoint", default=DEFAULT_ENDPOINT)
    parser.add_argument("--ocr-lang", action="append", default=[], metavar="LANG")
    parser.add_argument("--no-ocr", action="store_true")
    image_group = parser.add_mutually_exclusive_group()
    image_group.add_argument("--images", dest="images", action="store_true", default=True)
    image_group.add_argument("--no-images", dest="images", action="store_false")
    parser.add_argument("--no-image-ocr", action="store_true", help="Do not OCR each extracted image")
    parser.add_argument("--async", dest="async_mode", action="store_true", help="Submit conversion as a Docling task and poll for its result")
    parser.add_argument("--poll-interval", type=float, default=10, metavar="SECONDS", help="Async status poll interval (default: 10)")
    parser.add_argument("--async-timeout", type=float, default=7200, metavar="SECONDS", help="Async wait limit; use 0 to wait indefinitely (default: 7200)")
    parser.add_argument("--overwrite", action="store_true", help="Allow a non-empty output directory")
    args = parser.parse_args()

    source = args.source.expanduser().resolve()
    output = args.output_dir.expanduser().resolve()
    if args.poll_interval <= 0:
        parser.error("--poll-interval must be greater than zero")
    if args.async_timeout < 0:
        parser.error("--async-timeout must be zero or greater")
    if not source.is_file():
        parser.error(f"source is not a readable file: {source}")
    if output.exists() and any(output.iterdir()) and not args.overwrite:
        parser.error(f"output directory is non-empty: {output} (pass --overwrite to replace named output files)")
    output.mkdir(parents=True, exist_ok=True)
    if args.overwrite:
        shutil.rmtree(output / "images", ignore_errors=True)
        for path in (output / f"{source.stem}.md", output / "conversion.json", output / "async-task.json", output / "image-ocr.json"):
            path.unlink(missing_ok=True)

    ocr_fields = [("ocr_lang", lang) for lang in args.ocr_lang]
    main_fields = [
        ("to_formats", "md"),
        ("do_ocr", str(not args.no_ocr).lower()),
        ("do_table_structure", "true"),
        ("include_images", str(args.images).lower()),
        ("include_page_images", "false"),
        ("image_export_mode", "embedded" if args.images else "placeholder"),
        *ocr_fields,
    ]
    try:
        if args.async_mode:
            payload, task = convert_async(source, main_fields, args.endpoint, args.poll_interval, args.async_timeout)
            (output / "async-task.json").write_text(json.dumps(task, indent=2), encoding="utf-8")
        else:
            payload = convert(source, main_fields, args.endpoint)
    except RuntimeError as exc:
        print(exc, file=sys.stderr)
        return 1

    (output / "conversion.json").write_text(json.dumps(payload, indent=2), encoding="utf-8")
    document = payload.get("document") or {}
    markdown = document.get("md_content") or ""
    if not markdown:
        print("Docling returned no Markdown; see conversion.json", file=sys.stderr)
        return 1

    extracted: list[tuple[str, Path]] = []
    if args.images:
        markdown, extracted = save_images(markdown, output / "images", source.stem)

    image_ocr_results: list[dict] = []
    if extracted and not args.no_image_ocr:
        image_fields = [
            ("to_formats", "md"),
            ("do_ocr", "true"),
            ("do_table_structure", "false"),
            ("include_images", "false"),
            ("include_page_images", "false"),
            ("image_export_mode", "placeholder"),
            *ocr_fields,
        ]
        for name, image_path in extracted:
            try:
                image_payload = convert(image_path, image_fields, args.endpoint)
                image_text = (image_payload.get("document") or {}).get("md_content") or ""
                image_text = image_text.replace("<!-- image -->", "").strip()
                status = image_payload.get("status", "unknown")
                result = {"image": f"images/{name}", "status": status, "text": image_text, "errors": image_payload.get("errors", [])}
                if status in {"success", "partial_success"} and image_text.strip():
                    markdown = append_image_ocr(markdown, name, image_text)
                    result["included_in_markdown"] = True
                else:
                    result["included_in_markdown"] = False
                image_ocr_results.append(result)
            except (RuntimeError, ValueError) as exc:
                image_ocr_results.append({"image": f"images/{name}", "status": "failure", "errors": [str(exc)], "included_in_markdown": False})
        (output / "image-ocr.json").write_text(json.dumps(image_ocr_results, indent=2), encoding="utf-8")

    md_path = output / f"{source.stem}.md"
    md_path.write_text(markdown, encoding="utf-8")
    status = payload.get("status", "unknown")
    included = sum(result.get("included_in_markdown", False) for result in image_ocr_results)
    failures = sum(result.get("status") not in {"success", "partial_success"} for result in image_ocr_results)
    print(json.dumps({"status": status, "markdown": str(md_path), "images": len(extracted), "image_ocr": included, "image_ocr_failures": failures, "output": str(output)}))
    if status not in {"success", "partial_success"}:
        print(f"Docling status: {status}; errors: {payload.get('errors', [])}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
