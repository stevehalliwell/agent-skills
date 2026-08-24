# Research Artifact Formats

Use these project-local paths under one inquiry output directory. The CSV files are machine-readable audit records; `source-map.md` is the single authoritative human-readable view of what each source is, why it was added, and its fetch/review state.

## Discovery candidate CSV

Every delegated discovery batch writes `discovery/D-###/source-candidates.csv`. Keep one row per observation; do not remove duplicates before normalisation.

```csv
url,title,source_type,evidence_label,why_retain
SOURCE_URL,Example report,official documentation,primary,Direct statement relevant to the question
```

Required fields: `url`, `title`, `source_type`, `evidence_label`, and `why_retain`. Extra, question-specific columns are preserved in `source-observations.csv`.

## Capture queue and batches

`capture-queue.csv` represents every normalised source. Human review creates `capture-queue-reviewed.csv` by adding `proposed_status`, `selection_reason`, and `review_notes`; it must retain every row. Statuses include `crawl4ai-candidate`, `keep-as-discovery`, and `review-needed`.

`prepare-follow-up-batch.mjs` produces an explicit batch with:

```csv
queue_id,source_id,url,title,capture_reason,thread_page_range
CQ-0001,SRC-0001,SOURCE_URL,Example report,Primary source relevant to the question,
```

Use this shape for Crawl4AI batches. It never broadens URLs or edits the reviewed queue.

## Capture records

Every capture script appends to one `capture-records.csv`, including failed and skipped attempts:

```csv
queue_id,status,method,artifact_path,captured_at,detail
CQ-0001,captured,crawl4ai,crawls/capture-001/CQ-0001/page.md,2026-01-01T00:00:00Z,Public page captured
CQ-0002,failed,crawl4ai,,2026-01-01T00:01:00Z,Robots denied capture
```

`artifact_path` is relative to the directory holding `capture-records.csv`, so it can be linked by `source-map.md` and the register.

## Source map

Run `build-source-map.mjs` after queue review and each capture/lineage update. It overwrites `source-map.md` from the retained index, queue, and lineage files. Do not maintain a competing hand-written source list; put human judgement in the queue review fields or register instead.
