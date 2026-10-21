# Bartle sources — local captures

Status: **two sources captured** for local offline reference. Existing independent notes and `.gitkeep` preserved. No bibliographic leads, books, framework sweep, talk audio, or new sources acquired.

Acquisition date: 2026-10-21. Exact timestamps, methods, checks, rights, image identities and PDF hash: [capture metadata](captures/acquisition.json).

## Source-to-capture mapping

### Richard A. Bartle — Hearts, Clubs, Diamonds, Spades: Players Who Suit MUDs (1996)

- **Actual source:** [complete article Markdown](captures/players-who-suit-muds-1996/players-who-suit-muds-1996.md).
- Independent commentary, not source text: [existing source notes](players-who-suit-muds-1996-notes.md).
- Original: https://mud.co.uk/richard/hcds.htm. Existing mirror is same work, not another source.
- Method: built-in readable fetch inspected; bounded, one-URL Crawl4AI extraction retained. Pinned image and `check_robots_txt=true`; robots endpoint returned HTTP 404. Crawl returned `success=true`, HTTP 200. Isolated loopback container removed; no link following.
- Fidelity: full article, beginning through references and copyright footer. Verified abstract/preface, taxonomy, ASCII interest graph, all 16 interaction cases, dynamics, summary, endnotes, and final Whitlock bibliography entries. Not abstract-only or challenge content.
- Figures: all five directly embedded GIFs retained locally. Crawl omitted endnote 2 graphical figure; restored its image link from original HTML before References. Decorative images localized too. ASCII graphs remain text. Source footnote/reference links still target original online anchors; local Markdown does not recreate HTML anchor IDs. Historical external bibliography links may be dead; not crawled.
- Raw evidence: [HTTP HTML](captures/players-who-suit-muds-1996/page.html), [rendered HTML](captures/players-who-suit-muds-1996/rendered.html), [unaltered crawl result](captures/players-who-suit-muds-1996/response.json), robots response and headers beside them.
- Rights: copyright Richard A. Bartle. Endnote permits unchanged redistribution **for academic purposes** only. General/open redistribution not established. **Local-only; not publication-cleared.**

### Richard A. Bartle — A Game Designer’s View of Gamification (19 June 2012)

- **Actual source:** [Docling slide Markdown with seven local figures](captures/game-designers-view-of-gamification-2012/game-designers-view-of-gamification-2012.md).
- **Text coverage companion:** [all 18 slide text layers as Markdown](captures/game-designers-view-of-gamification-2012/game-designers-view-of-gamification-2012-text-layer.md). Mechanical extraction, not notes or paraphrase. Read both for accessible text plus diagrams.
- Independent commentary, not source text: [existing source notes](game-designers-view-of-gamification-2012-notes.md).
- Original PDF: https://mud.co.uk/richard/GamificationSummit.pdf. Author index: https://mud.co.uk/richard/presntns.htm. Index is provenance/navigation, not a separate substantive work.
- Method: downloaded public PDF to temporary scratch; existing shared Docling service at `http://127.0.0.1:5001`, asynchronous conversion, `do_ocr=false`, embedded images, no image OCR. Same submitted task polled with curl; final `success`, errors `[]`. Shared service not changed. No remote OCR, enrichment, model/package download or service restart.
- Fidelity: complete slide-deck coverage across Markdown and retained figures, **not a fully searchable transcription of every diagram**. Docling places several whole slides inside figures; their headings/body text disappear from its Markdown text. `pdftotext -layout` companion preserves all 18 page text layers, including New partition #1–#4. Source font extraction yields replacement bullet glyphs, irregular capitalization and some duplicate lines. Diagram labels embedded as pixels remain image-only; no OCR or invented descriptions.
- Checks: opening title/date/author; middle partition/utility slides; last contribution/conclusion; 18 text-layer pages; seven exported images; full New partition #1 figure visually checked; every local Markdown figure path resolves. Slides are not talk speech; audio/captions/speaker notes not acquired.
- Raw evidence: [Docling result](captures/game-designers-view-of-gamification-2012/conversion.json), task submission/status JSON, and PDF text-layer check beside Markdown. Original PDF removed after conversion/check; use online PDF above, not a nonexistent local PDF.
- Rights: no open license or general redistribution permission established. Attribution: Richard A. Bartle, University of Essex. Goth photograph credited on slide to `gothsuptrees.net`; third-party rights not cleared. **Local-only; not publication-cleared.**

## Limits and blockers

- No full-text access blockers. No source replaced with another summary.
- Article text complete; slide text/visual limits stated above. No claim of perfect OCR or complete machine-readable diagram text.
- Public access and local retention do not grant publication rights. No staging, commits, publication, or changes outside assigned folder.
