# Flow — local source captures

Two substantive sources inventoried from every source card and this README. Retention checked 2026-10-21. Actual source text below; cards and independent notes remain separate. `.gitkeep` preserved. No framework sweep, books, or bibliography-only acquisition.

## Source → local capture

1. **Mihaly Csikszentmihalyi, *Flow, the secret to happiness* (TED2004, February 2004). Captured.**
   - [Actual English captions, readable Markdown](csikszentmihalyi-2004-ted-captions.md).
   - [Raw caption segments, language, video ID, millisecond timing](captures/csikszentmihalyi-2004-ted-captions.json).
   - [Independent source notes / source card](csikszentmihalyi-2004-ted-source-notes.md), not a transcript.
   - Source: [official TED YouTube upload](https://www.youtube.com/watch?v=fXIeFJCqsPs); [TED talk page](https://www.ted.com/talks/mihaly_csikszentmihalyi_flow_the_secret_to_happiness).
   - Method/date: existing `youtube-transcript-download/scripts/download.mjs`, `--lang en --format json`, 2026-10-21. All returned segments rendered as timestamped Markdown without changing wording or order. Transcript-to-prose skill consulted; no prose rewrite needed for this readable caption capture. No media downloaded.
   - Coverage/fidelity: full returned English caption track, 256 segments, 00:12.160–18:50.160. Checked opening World War II account, middle composer/attention discussion, later experience-sampling model, closing mandate, thanks and applause. Not another summary. No independent audio check; no slide-only text or images; opening non-captioned seconds not represented. JSON retains original text and timings.
   - Rights: caption republication permission not established; TED page previously reported all rights reserved. Copyright remains with respective rights holders. **Local-only offline reference; not publication-cleared.** No claim that attribution authorizes redistribution.
2. **Cowley et al., *Flow Experiences During Visuomotor Skill Acquisition Reflect Deviation From a Power-Law Learning Curve, but Not Overall Level of Skill* (2019). Reused.**
   - [Existing full article Markdown](cowley-2019-docling/cowley-2019-visuomotor-flow.md).
   - [Independent research notes / source card](cowley-2019-research-notes.md).
   - Source: [DOI](https://doi.org/10.3389/fpsyg.2019.01126); [original online publisher PDF](https://public-pages-files-2025.frontiersin.org/journals/psychology/articles/10.3389/fpsyg.2019.01126/pdf).
   - Method/date: existing Docling conversion acquired 2026-10-21; verified/reused same date, not duplicated or resubmitted. Full Markdown already exists; original PDF no longer retained locally. Stale local PDF link corrected to publisher URL.
   - Coverage/fidelity: main article through references, conflict statement and copyright; seven extracted raster images in `cowley-2019-docling/images/`. These include publisher graphics, not seven research figures. Supplements, data and code not acquired. No image OCR or manual figure interpretation. Mathematical notation, table layout, hyphenation and reading order can differ from PDF. Ethics/author contributions interrupt reference list in conversion; remaining references follow supplementary-material heading.
   - Checks: read full conversion, not just abstract. Confirmed opening title/authors/abstract/introduction, middle methods and results/tables, end limitations/conclusion, references through Wolf et al. (2015), and copyright statement. All seven image links resolve; PNG signatures verified. Source attribution and DOI retained.
   - Rights: copyright © 2019 Cowley, Palomäki, Tammi, Frantsi, Inkilä, Lehtonen, Pölönen, Vepsäläinen and Lappi; [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/). Attribution, original journal citation and conversion changes recorded in capture. No author endorsement implied.

Talk's person-relative experience-sampling model and experiment's questions about reference levels during learning are complementary, not identical. Neither establishes this repository's example game's player experience. Neither is GameFlow.

## Existing conversion provenance

Historical acquisition record, not new service calls in this retention pass:

- Discovery used `web_search`; static retrieval used `fetch_content` and `get_search_content`.
- TED static fetch incomplete. One robots-enforced Crawl4AI fetch recovered metadata but no transcript; official YouTube captions supplied substantive source. Earlier notes-only decision superseded by actual local caption retention above.
- Historical Crawl4AI image: `unclecode/crawl4ai@sha256:bd36741e7bdd35ddc1a05d9183e1d6d8cefb61dd640d944a25d026b76e917690`. Isolated loopback `crawl4ai-flow-*` container removed after use. No new container needed here.
- Publisher PDF downloaded with `curl` after HTML license verification; converted by existing shared Docling at `http://127.0.0.1:5001`.
- Docling image: `quay.io/docling-project/docling-serve-cpu:v1.30.0`, digest `sha256:061d35c03611bc15b73d024c8e8387bcf0624279f8b57c16c1567326f214ba56`.
- Async task `02473e4f-9120-4374-a5e4-399bc06b928e`. Windows client hit `[WinError 10048]`; same task recovered using bounded `curl` polling, not resubmitted. Recorded final status `success`, errors `[]`.
- Born-digital extraction: OCR disabled, table structure enabled, embedded images enabled, page images and image OCR disabled. Existing helper extracted seven PNGs. Raw conversion response stayed in temporary scratch and is no longer retained here; Markdown/images are retained. No new Docling call, service restart, enrichment or model download in this pass.

## Access limits and excluded leads

No full-text blocker among the two substantive retained sources. Caption and conversion fidelity limits remain above.

Prior README recorded these **unread leads**, not additional substantive source cards. Preserve blockers without chasing or representing them as read:

- *Play and Intrinsic Rewards* (1975), DOI `10.1177/002216787501500306`: publisher returned HTTP 403; no full text acquired. Also bibliographic in Cowley article.
- *The Flow Engine Framework* (2018), https://pmc.ncbi.nlm.nih.gov/articles/PMC5973526/: static fetch unusable; prior robots-enforced browser attempt reported anti-bot protection; publisher alternate failed. Not acquired or represented as read. No further anti-bot retries.
- *Flow* (1990) remains book bibliography only; not acquired.

## Comments

Retained source cards and independent notes are not substitutes for captures. Both substantive sources now have discoverable actual local Markdown. Rights uncertainty restricts publication claims, not this requested local retention. No commits, staging, dependency installs, reusable tools, or sibling-folder edits.
