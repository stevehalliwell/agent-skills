# Research Register Format

Create one project-local `research-register.md` for each focused question. Keep captured files in a sibling directory and link them with project-relative paths. The register is a maintained lineage view, not a replacement for raw discovery or capture artifacts. `source-map.md` is the single human-readable source/fetch/review mapping document; do not maintain a competing source list.

```markdown
---
question_id: QR-001
question: <one focused question>
outcome: evidence-answer # or evidence-dossier
status: discovery # discovery | capture | coverage-review | synthesis | review | complete | inconclusive
created_at: <ISO-8601 timestamp>
updated_at: <ISO-8601 timestamp>
---

# <question>

## Intent and boundary

- **Decision or use:** <who will use this and why>
- **Coverage subquestions:** <2–5 distinct lenses needed to answer the focused question; not new inquiries>
- **In scope:** <specific population, product, period, geography, or technical context>
- **Out of scope:** <claims and adjacent questions excluded>
- **Source authority:** <source types appropriate to the question>
- **Freshness:** <required observation period, version, or `not time-sensitive`>
- **Sufficient evidence:** <what would permit synthesis; include counter-evidence expectation>
- **Selected references:** <[guide](path) — selection reason; or `none`>
- **Coverage dimensions:** <dimensions required before synthesis, including counter-evidence>
- **Outcome:** <evidence-answer or evidence-dossier; why>

## First blush

- **Provisional answer:** <short answer from bounded delegated first blush>
- **Decision:** <stopped here | continued to retained-evidence workflow>
- **Artifacts:** [answer analyst](first-blush/answer-analyst.md), [source candidates](discovery/D-001/source-candidates.csv)
- **What deeper work could change:** <specific uncertainty or `not applicable`>

## Discovery batches

| Batch | Method | Scope | Artifact | Observations | Limits |
| --- | --- | --- | --- | --- | --- |
| D-001 | delegated public-web discovery | <lens> | [source candidates](discovery/D-001/source-candidates.csv) | 0 | <limit> |

## Source lineage

- **Authoritative source map:** [source map](source-map.md)

| Source ID | Canonical URL | Discovery observations | Source type | Evidence label | Capture status | Retained artifact | Limits / notes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| SRC-001 | <URL> | D-001:2 | official documentation | primary | captured | [page](captures/SRC-001/page.md) | <scope, volatility, or failure> |

Evidence labels: `primary`, `expert-secondary`, `reported-secondary`, `community-report`, or `discovery-signal`. Labels describe provenance; they do not decide truth.

## Evidence matrix

| Evidence ID | Claim, observation, or data point | Supports / challenges | Source IDs | Evidence kind | Observed at | Scope and limits |
| --- | --- | --- | --- | --- | --- | --- |
| E-001 | <verifiable statement or direct observation> | supports <proposition> | SRC-001 | direct statement | <ISO-8601 or n/a> | <limits> |

Evidence kinds: `direct statement`, `measured data`, `reported experience`, `expert interpretation`, `derived comparison`, and `absence/gap`. For volatile metrics, link a retained CSV/JSON artifact and record the metric definition, value, and observation time.

## Conflicts and gaps

- **Conflict:** <competing evidence and source IDs>
- **Gap:** <coverage dimension not met and why>
- **Capture failure:** <source ID, attempt, and result>

## Synthesis

### Evidence answer

- **Supported answer:** <only when evidence supports it>
- **Unresolved:** <what evidence cannot establish>

### Evidence dossier

- **Candidate interpretations:** <competing readings of retained evidence>
- **Proposed judgement:** <explicit interpretation, not externally established fact>
- **Human review required:** <reviewer and question>

## Validation

- <normalisation, queue, capture, and link checks with commands/results>
```

## Capture records

Every Crawl4AI capture batch must append rows to `capture-records.csv`; a failed capture is still a row.

```csv
queue_id,status,method,artifact_path,captured_at,detail
CQ-0001,captured,crawl4ai,crawls/CQ-0001/page.md,2026-01-01T00:00:00Z,Public page captured
CQ-0002,failed,crawl4ai,,2026-01-01T00:00:00Z,Robots denied capture
```

Run `build-source-capture-lineage.mjs` with the normalised index, reviewed queue, and this file. Then run `update-research-register.mjs` to replace only its generated source/capture-status block in the register.

## Required companion artifacts

- `discovery/<batch>/source-candidates.csv` — one row per discovery observation; never delete leads during normalisation.
- `source-observations.csv`, `source-index.csv`, and `dedup-report.json` — generated normalisation artifacts.
- `source-map.md` — generated authoritative human-readable mapping of each source’s origin, rationale, queue state, capture status, artifact, and failure notes.
- `capture-queue.csv` and, when reviewed, `capture-queue-reviewed.csv` — every indexed source remains represented.
- `capture-records.csv` and `source-capture-lineage.csv` — source-to-capture status, failures, and artifact paths.
- `captures/` and `crawls/` — retained source artifacts and failure metadata.
- `evidence-review.md` — delegated no-new-search review of all retained artifacts, with source IDs and unresolved limits.
- `data/` — retained CSV/JSON extracts for current figures, rankings, prices, catalogue samples, or other structured observations.

Use `QR-###`, `D-###`, `SRC-####`, and `E-###` identifiers. IDs are stable within the inquiry; reruns append rather than renumber existing entries.
