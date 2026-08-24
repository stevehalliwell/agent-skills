#!/usr/bin/env python3
"""Capture explicitly listed public URLs with Crawl4AI and append capture records.

Usage: python3 capture-crawl4ai-batch.py --batch FILE --output-dir DIR --capture-records FILE
The batch requires queue_id,url. No link following occurs and output-dir must not exist.
"""
import argparse, csv, datetime, json, os, re, secrets, subprocess, time
from pathlib import Path
from urllib.request import Request, urlopen

IMAGE = "unclecode/crawl4ai@sha256:bd36741e7bdd35ddc1a05d9183e1d6d8cefb61dd640d944a25d026b76e917690"
HEADERS = ["queue_id", "status", "method", "artifact_path", "captured_at", "detail"]
def command(*args): return subprocess.run(args, check=False, text=True, capture_output=True)
def get(url, token):
    with urlopen(Request(url, headers={"Authorization": f"Bearer {token}"}), timeout=10) as response: return response.read()
def insert_base(html, url):
    base = f'<base href="{url}">'
    return re.sub(r"(<head[^>]*>)", r"\1" + base, html, count=1, flags=re.I) if re.search(r"<head[^>]*>", html, re.I) else base + html
def stamp(): return datetime.datetime.now(datetime.UTC).isoformat()
def append_records(path, rows):
    path.parent.mkdir(parents=True, exist_ok=True)
    exists = path.exists() and path.stat().st_size > 0
    with path.open("a", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=HEADERS)
        if not exists: writer.writeheader()
        writer.writerows(rows)
def rel(record_path, artifact): return os.path.relpath(artifact, record_path.parent)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--batch", required=True); parser.add_argument("--output-dir", required=True); parser.add_argument("--capture-records", required=True)
    args = parser.parse_args(); batch, output, record_path = Path(args.batch), Path(args.output_dir), Path(args.capture_records)
    with batch.open(newline="") as handle: rows = list(csv.DictReader(handle))
    if not rows or any(not row.get("queue_id") or not row.get("url", "").startswith(("http://", "https://")) for row in rows): raise SystemExit("Each batch row needs queue_id and HTTP(S) url.")
    if output.exists(): raise SystemExit(f"Refusing to overwrite existing output: {output}")
    output.mkdir(parents=True); token, port = secrets.token_hex(24), 12000 + (os.getpid() % 30000)
    container = f"question-research-crawl-{os.getpid()}-{secrets.token_hex(4)}"; captured = []
    metadata = {"batch": str(batch), "image": IMAGE, "urls": [row["url"] for row in rows], "link_following": False, "started_at": stamp()}
    try:
        started = command("docker", "run", "-d", "--rm", "--name", container, "-e", f"CRAWL4AI_API_TOKEN={token}", "-p", f"127.0.0.1:{port}:11235", "--shm-size=1g", IMAGE)
        if started.returncode: raise RuntimeError(f"Docker start failed: {started.stderr.strip()}")
        for _ in range(30):
            try: get(f"http://127.0.0.1:{port}/health", token); break
            except Exception: time.sleep(1)
        else: raise RuntimeError("Crawl4AI health check did not succeed within 30 seconds")
        request = {"urls": [row["url"] for row in rows], "crawler_config": {"cache_mode": "BYPASS", "check_robots_txt": True, "wait_for_images": True}}
        (output / "request.json").write_text(json.dumps(request, indent=2) + "\n")
        response = command("curl", "--fail-with-body", "-sS", f"http://127.0.0.1:{port}/crawl", "-H", f"Authorization: Bearer {token}", "-H", "Content-Type: application/json", "--data", "@" + str(output / "request.json"))
        (output / "response.json").write_text(response.stdout)
        if response.returncode: raise RuntimeError(f"Crawl request failed: {response.stderr.strip()}")
        pages = json.loads(response.stdout); pages = pages.get("results", pages if isinstance(pages, list) else [])
        by_url = {page.get("url"): page for page in pages}
        for row in rows:
            page = by_url.get(row["url"]); page_dir = output / row["queue_id"]; page_dir.mkdir(parents=True, exist_ok=True)
            (page_dir / "response.json").write_text(json.dumps(page or {"success": False, "error": "No correlated response"}, indent=2) + "\n")
            item = {"queue_id": row["queue_id"], "status": "failed", "method": "crawl4ai", "artifact_path": "", "captured_at": stamp(), "detail": ""}
            if page and page.get("success"):
                html = page.get("html", ""); markdown = page.get("markdown", {}); markdown = markdown.get("raw_markdown", "") if isinstance(markdown, dict) else str(markdown)
                (page_dir / "page.html").write_text(html); (page_dir / "view.html").write_text(insert_base(html, page.get("url", row["url"]))); (page_dir / "page.md").write_text(markdown)
                item.update({"status": "captured", "artifact_path": rel(record_path, page_dir / "page.md"), "detail": "Rendered public page."})
            else: item["detail"] = (page or {}).get("error_message", "Crawl response marked unsuccessful")
            captured.append(item)
    except Exception as error:
        captured = [{"queue_id": row["queue_id"], "status": "failed", "method": "crawl4ai", "artifact_path": "", "captured_at": stamp(), "detail": str(error)} for row in rows]
        metadata["error"] = str(error)
    finally:
        metadata.update({"completed_at": stamp(), "captured": sum(item["status"] == "captured" for item in captured), "failed": sum(item["status"] == "failed" for item in captured)})
        (output / "run-metadata.json").write_text(json.dumps(metadata, indent=2) + "\n"); command("docker", "rm", "-f", container); append_records(record_path, captured)
    print(json.dumps({"output": str(output), "capture_records": str(record_path), "captured": metadata["captured"], "failed": metadata["failed"]}, indent=2))
    return 2 if metadata["failed"] else 0
if __name__ == "__main__": raise SystemExit(main())
