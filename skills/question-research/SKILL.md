---
name: question-research
description: "Research one focused question, investigate whether an answer already exists, gather public-web evidence or current data, compare conflicting sources, identify evidence gaps, or build an evidence dossier for a human/LLM judgement. Use for iterative, retained, evidence-linked research; skip a simple fact check, one-source lookup, drafting, and multi-topic campaigns."
---

# Question Research

Build a traceable public-web evidence base for one focused question. Start with a bounded delegated first blush; continue only when it is insufficient or the user wants retained evidence.

## Trigger clarification

Use this only when the request needs iterative discovery, public-source evidence, or current-data gathering. Keep one question boundary; adjacent questions require user approval as a new or broadened inquiry.

## Required materials

Read [`references/index.md`](references/index.md) during inquiry framing. Read [`formats/research-register.md`](formats/research-register.md) and [`formats/research-artifacts.md`](formats/research-artifacts.md) before creating inquiry artifacts. The register is the project-local lineage record; `source-map.md` is the single human-readable source/fetch/review map.

## Workflow

1. **Frame the inquiry.** Restate the focused question and the decision or use it informs. Derive 2–5 coverage subquestions that test the answer from distinct necessary angles; they are lenses for this inquiry, not permission to broaden it. Define what is in and out of the corpus, appropriate source authority, applicable time/geography/version/population boundaries, required freshness, and what would count as sufficient evidence. Use the reference catalogue to select applicable guides, record each selection and reason, and read those guides. Ask one focused clarification only when a missing boundary could materially change the corpus or conclusion; otherwise record the working assumptions and proceed.
   *Done when the question, consumer, coverage subquestions, corpus boundaries, evidence bar, freshness, and selected references are explicit.*

2. **Set up the inquiry location and first-blush brief.** Create a project-relative output directory with `first-blush/` and `discovery/D-001/`. Read the required formats and record the research frame, labels, URL cap, and output paths. Defer the full outcome and register.
   *Done when work has paths, required artifacts, labels, URL cap, lenses, and constraints.*

3. **Run delegated first blush.** Load `delegate-tasks` and launch independent, bounded public-web jobs: an answer analyst using a mid-to-large/high-thinking model that writes `first-blush/answer-analyst.md`, and source discovery writing labelled candidates to `discovery/D-001/source-candidates.csv`. Give each the question, coverage subquestions, corpus boundaries, selected-reference constraints, source labels, URL cap, output path, and required format. Preserve each child result and every candidate observation.
   *Done when the provisional answer and every source candidate are traceable to distinct job artifacts.*

4. **Report the first blush and gate the workflow.** Give the user a concise provisional answer, strongest sources or observations, material caveats, and what deeper work could change. If sufficient, stop and retain the first-blush artifacts. Otherwise, use `ask_user` to obtain permission to enter the full retained-evidence workflow; ask for `evidence-answer` or `evidence-dossier` if it remains unclear.
   *Done when the user has explicitly stopped or authorised deeper research.*

5. **Create and review the register, queue, and source map.** Use the standard register in the output directory and record the approved outcome. Run packaged normalisation and queue scripts with explicit paths. Preserve every observation and merge only exact canonical URLs. Review every queue row, then create `capture-queue-reviewed.csv` with a proposed status, selection reason, and review notes for every source; no source proceeds to capture without that record. Generate `source-map.md` from the index and reviewed queue; regenerate it after capture updates.
   *Done when the register and source map link observations to normalised sources and every capture candidate has a recorded review decision.*

6. **Reconnoitre, then capture bounded evidence and data.** Load `crawl4ai` and make one bounded Crawl4AI reconnaissance fetch for each reviewed seed URL before preparing any capture batch. Inspect its rendered page and retained response to decide whether the seed itself answers the question, relevant content is at explicit linked URLs, or a clearly identified pagination rule needs a bounded listing/detail crawl. Record that rule and bounds in the queue review or run metadata; do not infer a site-wide crawl from the seed. Then capture only the seed, explicit discovered URLs, or an approved paginated sample with `capture-crawl4ai-batch.py`. Retain useful tables/figures as CSV/JSON. Build source-capture lineage and regenerate the source map after each batch. Preserve successes and failures; never handle private, paywalled, or CAPTCHA content.
   *Done when every selected seed has a recorded reconnaissance outcome, and retained captures, structured observations, or explicit failures are linked from the register and source map.*

7. **Review the retained corpus.** Once all selected fetches have a terminal record, load `delegate-tasks` and launch one bounded, no-new-search review job over the retained artifacts, source map, and register. It creates a human-readable `evidence-review.md`: key observations by source ID, support/challenge relationships, contradictions, capture limits, and candidate answers or interpretations. It may not promote a conclusion beyond the evidence.
   *Done when the human can review the fetched corpus without opening every artifact.*

8. **Assess coverage before synthesis.** Compare retained evidence against the coverage dimensions and applicable branch reference. When evidence is thin, ask the user for adjacent questions or permission to broaden the corpus; do not silently widen scope.
   *Done when gaps, counter-evidence, corpus limits, and source disagreement are visible.*

9. **Synthesize from retained artifacts only.** For an `evidence-answer`, distinguish supported claims from unresolved ones. For an `evidence-dossier`, preserve raw observations and opposing interpretations, then offer a clearly labelled proposed judgement. Treat direct reports as reports rather than proof of prevalence, demand, or causality.
   *Done when conclusions, data, interpretation, uncertainty, and artifact links are separate.*

10. **Validate and report.** Run packaged validation with explicit paths, verify local links, and report the outcome, retained evidence, limits, and next review step.
   *Done when another researcher can reproduce the path from question to conclusion or judgement.*

## Packaged scripts

Run these directly from this package with explicit project paths:

- `scripts/normalise-sources.mjs --input-dir DIR --output-dir DIR` — preserve discovery observations, produce canonical source index and merge audit.
- `scripts/prepare-capture-queue.mjs --index FILE --output FILE` — create one review row per indexed source.
- `scripts/prepare-follow-up-batch.mjs --queue FILE --output FILE [--limit N]` — select reviewed Crawl4AI candidates without changing the queue.
- `scripts/capture-crawl4ai-batch.py --batch FILE --output-dir DIR --capture-records FILE` — capture explicit public-page URLs and append every outcome.
- `scripts/build-source-capture-lineage.mjs --index FILE --queue FILE --capture-records FILE --output FILE` — retain every source’s capture status and artifact lineage.
- `scripts/build-source-map.mjs --index FILE --queue FILE --lineage FILE --output FILE` — generate the authoritative human-readable source/fetch/review map.
- `scripts/update-research-register.mjs --register FILE --lineage FILE` — replace only the generated capture-status block in the register.
- `scripts/validate-research-register.mjs --register FILE` — check required register shape and local artifact links.

## Rules

- Keep credentials, personal data, private content, paywall bypasses, and CAPTCHA handling out of the inquiry.
- Preserve raw observations, source type, source context, contradictory evidence, capture failures, and applicable timestamps.
- Evidence labels describe provenance; they do not decide truth.
- Never use a model synthesis as a substitute for retained source evidence.
- Separate observations, interpretations, hypotheses, and recommendations in retained artifacts and reports; test conclusions against relevant alternatives.
- An evidence dossier may be inconclusive; say so rather than forcing a judgement.
- The global package is canonical. Run its scripts directly; do not copy them into projects.
