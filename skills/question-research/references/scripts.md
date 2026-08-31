# Packaged scripts

Run these directly from this package with explicit project paths:

- `scripts/normalise-sources.mjs --input-dir DIR --output-dir DIR` — preserve discovery observations, produce canonical source index and merge audit.
- `scripts/prepare-capture-queue.mjs --index FILE --output FILE` — create one review row per indexed source.
- `scripts/prepare-follow-up-batch.mjs --queue FILE --output FILE [--limit N]` — select reviewed Crawl4AI candidates without changing the queue.
- `scripts/capture-crawl4ai-batch.py --batch FILE --output-dir DIR --capture-records FILE` — capture explicit public-page URLs and append every outcome.
- `scripts/build-source-capture-lineage.mjs --index FILE --queue FILE --capture-records FILE --output FILE` — retain every source’s capture status and artifact lineage.
- `scripts/build-source-map.mjs --index FILE --queue FILE --lineage FILE --output FILE` — generate the authoritative human-readable source/fetch/review map.
- `scripts/update-research-register.mjs --register FILE --lineage FILE` — replace only the generated capture-status block in the register.
- `scripts/validate-research-register.mjs --register FILE` — check required register shape and local artifact links.
