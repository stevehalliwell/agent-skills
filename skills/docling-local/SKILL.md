---
name: docling-local
description: "Convert PDFs, scanned documents, DOCX, PPTX, XLSX, HTML, images, and similar local files to Markdown using a local Dockerized Docling server. Use when users ask to turn a document into .md, OCR a scan, preserve tables/layout, or extract document figures/page images without cloud APIs or Python packages installed on the host. Skip for web-page extraction or when Docker is unavailable."
compatibility: "Docker Desktop/Engine running locally; one-time pull of quay.io/docling-project/docling-serve-cpu:v1.30.0 (~4.4 GB)."
---

# Local Docling

Convert local documents → project-local Markdown, optionally with extracted PNGs, through one persistent local Docker service.

## Trigger clarification

Use this only for local document conversion. It does not fetch URLs. Confirm the input file, output location, whether images are wanted, and OCR language when it is not apparent. Default: Markdown beside the source in `<stem>.docling/`, OCR enabled, extracted figure images enabled for PDFs and image documents, with each image's second-pass OCR text immediately below its Markdown image link.

Done when the source, output, and requested artifacts are explicit.

## Required read

Read [`../docker-local/SKILL.md`](../docker-local/SKILL.md) before using or starting the service.

## Workflow

1. Check Docker, the required image, and the local server.
   ```bash
   image='quay.io/docling-project/docling-serve-cpu:v1.30.0'
   docker info >/dev/null
   if ! docker image inspect "$image" >/dev/null 2>&1; then
     echo "required Docling image is not installed: $image" >&2
     exit 1
   fi
   required_image_id="$(docker image inspect --format '{{.Id}}' "$image")"
   container_image_id="$(docker container inspect --format '{{.Image}}' docling-local 2>/dev/null || true)"
   if [ -n "$container_image_id" ] && [ "$container_image_id" != "$required_image_id" ]; then
     echo "docling-local uses a different image ID" >&2
     exit 2
   fi
   docker ps --filter name='^/docling-local$' --format '{{.Names}}' | grep -qx docling-local
   ```
   - If Docker is unavailable, state its error; do not install host Python packages as a fallback.
   - If the required image is absent, state its exact name, version, and one-time ~4.4 GB download, then ask approval to install it. Do not substitute `latest` or another locally cached image.
   - If the image exists but the server is absent, start it in step 3 without pulling again. If a named `docling-local` container exists with a different image ID, report it and ask approval before replacing it.
   - Done when Docker, image, and server are present, or the installation approval/blocker is explicit.

2. Install the required image only when it is absent and the user approves.
   ```bash
   image='quay.io/docling-project/docling-serve-cpu:v1.30.0'
   docker pull "$image"
   docker image inspect "$image" --format '{{index .RepoDigests 0}}'
   ```
   - Report the resolved digest after the pull.
   - Done when this exact version is locally available.

3. Start the one-server-per-machine container when it is absent, or start its stopped instance when it uses the required image.
   ```bash
   image='quay.io/docling-project/docling-serve-cpu:v1.30.0'
   if docker container inspect docling-local >/dev/null 2>&1; then
     docker start docling-local
   else
     docker run -d --name docling-local --restart unless-stopped \
       --publish 127.0.0.1:5001:5001 \
       --env DOCLING_SERVE_ENABLE_REMOTE_SERVICES=false \
       --env DOCLING_SERVE_LOAD_MODELS_AT_BOOT=true \
       "$image"
   fi
   ```
   - Wait for `curl --fail --silent http://127.0.0.1:5001/health` to succeed (allow up to two minutes for model loading). On failure, show `docker logs --tail 50 docling-local` and report the blocker.
   - Capture the resolved image identity: `docker image inspect "$image" --format '{{index .RepoDigests 0}}'`.
   - Keep the port loopback-only. Never add API keys, enable remote services, or expose it on the network.
   - Done when `/health` succeeds and the resolved digest is available.

4. Convert with the bundled, dependency-free client.
   ```bash
   python3 <path-to-docling-local-skill>/scripts/convert.py \
     /absolute/path/to/source.pdf \
     --output-dir /absolute/path/to/source.docling \
     --images
   ```
   - For a document that may exceed the synchronous request limit, add `--async`. The client submits one local task, polls it every 10 seconds, and writes `async-task.json` with the task ID and final status before processing its result. It waits up to two hours by default; use `--async-timeout 0` only when an unbounded local wait is intended.
   ```bash
   python3 <path-to-docling-local-skill>/scripts/convert.py \
     /absolute/path/to/large-source.pdf \
     --output-dir /absolute/path/to/large-source.docling \
     --images --async
   ```
   - Use `--no-images` only when the user wants text/Markdown alone. Use `--no-image-ocr` when images are wanted but their text should not be appended; this avoids one additional local conversion per image.
   - Add `--ocr-lang en` (or one repeatable option per language) when known; leave unspecified for Docling defaults. Add `--no-ocr` only for reliable born-digital documents when speed matters.
   - The client writes `<stem>.md`, optionally `images/`, `conversion.json`, and `image-ocr.json`; async runs also write `async-task.json`. It requests embedded images from the local service, writes them as local files, changes their Markdown links to relative paths, then OCRs each extracted image and writes successful results immediately below its image link.
   - Do not overwrite a non-empty output directory without user approval.
   - Done when the command exits successfully and the Markdown is non-empty.

5. Check and report output.
   - Verify Markdown exists and, when requested, report the extracted-image count. Report document status/errors from `conversion.json` rather than silently accepting a partial conversion.
   - State source, Markdown path, image directory (if any), OCR setting, and any conversion limitation.
   - Done when the user can open the artifacts or has a precise blocker.

## Rules

- The service is local and the standard pipeline uses local OCR/layout/table models; do not select VLM, picture-description, formula-enrichment, or any remote/API option unless the user explicitly asks to revisit that policy. Vision interpretation is not presently handled: the skill does not describe images or convert diagrams to Mermaid. A future opt-in local vision-model stage could add reviewable descriptions or diagram representations while retaining the original image.
- The Docker image is intentionally persistent per machine. Conversion outputs remain project-local; the source file is uploaded to loopback only and is not mounted into the container.
- The required image is `quay.io/docling-project/docling-serve-cpu:v1.30.0`. Before a deliberate version update, show the intended tag/digest and ask approval; do not use `latest` or auto-pull during ordinary conversion.
- Image export means rasterized detected figures and, when supported, page images—not necessarily the original embedded binary assets byte-for-byte.
