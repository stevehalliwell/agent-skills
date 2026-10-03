# Packaged scripts

Execute these bundled resources directly with explicit project paths; do not copy them into projects. Resolve `<skill-dir>` from the loaded question-research skill. JavaScript resources use Node.js; the capture client uses Python, Docker, curl, and the pinned Crawl4AI image. Follow the loaded Crawl4AI/Docker service and installation-approval rules before capture. Report invocation failures directly; no generic runtime availability probes are required.

- `node <skill-dir>/scripts/normalise-sources.mjs --input-dir DIR --output-dir DIR` — preserve discovery observations, produce the canonical source index and merge audit.
- `node <skill-dir>/scripts/prepare-capture-queue.mjs --index FILE --output FILE` — create one review row per indexed source.
- `node <skill-dir>/scripts/prepare-follow-up-batch.mjs --queue FILE --output FILE [--limit N]` — select reviewed Crawl4AI candidates without changing the queue. The limit bounds this batch, not the inquiry's evidence coverage.
- `python <skill-dir>/scripts/capture-crawl4ai-batch.py --batch FILE --output-dir DIR --capture-records FILE` — capture explicit public-page URLs and append every outcome; no link following, and the output directory must not exist.
- `node <skill-dir>/scripts/build-source-capture-lineage.mjs --index FILE --queue FILE --capture-records FILE --output FILE` — retain every source's capture status and artifact lineage.
- `node <skill-dir>/scripts/build-source-map.mjs --index FILE --queue FILE --lineage FILE --output FILE` — generate the authoritative human-readable source/fetch/review map.
- `node <skill-dir>/scripts/update-research-register.mjs --register FILE --lineage FILE` — replace only the generated capture-status block in the register.
- `node <skill-dir>/scripts/validate-research-register.mjs --register FILE` — check the required register shape and local artifact links.

Check exit status and generated artifacts after each operation. Correct supported input/lineage defects and rerun affected checks. Preserve capture failures and report unresolved blockers rather than silently dropping sources or retrying inaccessible content.
