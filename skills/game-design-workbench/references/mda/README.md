# MDA retained sources

Status: **two actual local full-text captures**. Existing independent notes and `.gitkeep` preserved. Capture date: **2026-10-21**. Bounded inventory: 2004 MDA paper and LeBlanc's author homepage. No new framework sweep.

## Source-to-local-capture mapping

1. **Robin Hunicke, Marc LeBlanc, Robert Zubek (2004), *MDA: A Formal Approach to Game Design and Game Research*.**
   - **[Full local Markdown](captures/hunicke-leblanc-zubek-2004/MDA.md)**; [seven local figures](captures/hunicke-leblanc-zubek-2004/images/).
   - [Author-hosted original PDF](https://www.cs.northwestern.edu/~hunicke/MDA.pdf); [AAAI publication record](https://aaai.org/papers/ws04-04-001-mda-a-formal-approach-to-game-design-and-game-research/).
   - Method: public PDF downloaded to temporary scratch with curl; converted through existing shared Docling Serve v1.30.0 at `http://127.0.0.1:5001`, using bundled client with `--images --no-ocr --no-image-ocr --async`. No service changes, remote enrichment, model downloads, or additional OCR.
   - Fidelity: complete extracted paper text, abstract through conclusions and five reference entries; seven detected figures exported. Not a summary. Automated extraction retains spacing/list artifacts and loses some apostrophes/quotation marks. Diagram labels remain readable in images but have no separate OCR transcript. Original pagination/layout not preserved.
   - Checks: read beginning (abstract/introduction), middle (aesthetics, feedback models, mechanics/tuning), end (three AI passes, conclusions, references); verified all seven image paths. Inspected designer/player diagram and Monopoly feedback diagram. `conversion.json` reports `success`, no errors; async task's `last_status.task_status` is `success`.
   - [Capture provenance, source hash and image identity](captures/hunicke-leblanc-zubek-2004/capture-metadata.json); [original conversion result](captures/hunicke-leblanc-zubek-2004/conversion.json); [async task result](captures/hunicke-leblanc-zubek-2004/async-task.json).
   - PDF was temporary input, not retained. Use original online PDF link above; local Markdown and figures are retained.

2. **Marc LeBlanc (undated), *8KindsOfFun.com: The collected game design rants of Marc "MAHK" LeBlanc*.**
   - **[Full local Markdown](captures/leblanc-8kindsoffun/page.md)**; [original URL](https://8kindsoffun.com/).
   - Method: built-in `fetch_content` failed. One isolated loopback Crawl4AI `/crawl` request, pinned image, `cache_mode: BYPASS`, `check_robots_txt: true`, one URL, no link following or LLM enrichment. Request returned HTTP 200 and `success: true`; only this run's temporary container removed.
   - Fidelity: full returned homepage text, including eight kinds, Audio and Video links, Papers, Game Design Workshop, all Lectures, and closing prose. Removed three malformed layout-table delimiter fragments from readable Markdown; source words unchanged. Raw extraction remains in response JSON. Linked slides, books, talks, archives, media, and external pages are not locally captured. Page is undated; HTTP last-modified `2024-03-19` is not a publication date.
   - Checks: compared raw HTML with Markdown; beginning taxonomy, middle workshop/provenance and final lecture/closing sections present. Not a challenge page, error page, or search snippet. Link targets remain original online sources, not claims of local availability. Source's malformed Sissyfight/mailto links remain unchanged.
   - [Raw response](captures/leblanc-8kindsoffun/response.json), [HTML](captures/leblanc-8kindsoffun/page.html), [HTML with base URL](captures/leblanc-8kindsoffun/view.html), [request](captures/leblanc-8kindsoffun/request.json), [capture metadata](captures/leblanc-8kindsoffun/capture-metadata.json).

## Independent notes — separate from captures

- [MDA paper source notes](hunicke-leblanc-zubek-2004-mda-source-notes.md).
- [LeBlanc author-site notes](leblanc-eight-kinds-of-fun-author-site-notes.md).

Notes remain selective independent interpretation, not substitutes for retained source text. Lecture slides, recordings, workshop archives, and *Tools for Creating Dramatic Game Dynamics* were only unread/bibliographic leads in existing notes; excluded from acquisition. AAAI record is publication metadata for the same paper, not a third substantive source.

## Rights and blockers

**Local-only retention; not publication-cleared.** Authors and original hosts attributed above. Copyright remains with respective rights holders; no explicit redistribution/adaptation license established for either source. Public access does not establish publication permission. Do not treat these captures as authorized redistribution or commit/publish them without separate rights review.

No full-text access blockers for these two sources. Missing permission for republication is a rights limit, not a reason to replace offline captures with summaries. No login, paywall, CAPTCHA, robots, or permission controls bypassed.

## Comments

Previous acquisition retained only notes and discarded conversions. This pass adds actual source captures without removing notes. No application, framework, workflow, sibling folder, Git index, or dependency changes made.
