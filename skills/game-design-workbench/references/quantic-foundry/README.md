# Quantic Foundry local sources

Status: **3/3 inventoried sources captured** on 2026-10-21. Full main-page prose retained as actual source Markdown, with seven substantive local PNG figures. No full-text access blockers. Existing independent notes and `.gitkeep` preserved.

**LOCAL ONLY / NOT PUBLICATION CLEARED.** Copyright 2015–2025 Quantic Foundry, LLC, as stated in page footers. No open license or redistribution permission established. Local retention serves offline reference; it does not establish permission to publish or redistribute source text or images. Earlier notes-only decision did not establish a full-text access blocker.

## Source-to-capture mapping

1. **Nick Yee, “How We Created the Gamer Motivation Profile” (2015-06-18).**
   - [Actual local article](captures/2026-10-21/how-we-created-the-gamer-motivation-profile/article.md).
   - [Original](https://quanticfoundry.com/2015/06/18/how-we-created-the-gamer-motivation-profile/).
   - [Independent source notes](2015-profile-development-source-notes.md), kept separately.
   - Full main article: opening personal examples and measurement questions, approach, all five groups, observations/next steps, final companion-article/profile links. Two local figures: literature-review table and initial motivations table.
   - Source's Social prose says “Completion” for competition; capture preserves source typo. Literature table retained as image, not transcribed. Bibliographic names in that table are not additional acquisition targets.
2. **Nick Yee, “How We Developed The Gamer Motivation Profile v2” (2015-07-20).**
   - [Actual local article](captures/2026-10-21/how-we-developed-the-gamer-motivation-profile-v2/article.md).
   - [Original](https://quanticfoundry.com/2015/07/20/how-we-developed-the-gamer-motivation-profile-v2/).
   - [Independent source notes](2015-profile-v2-source-notes.md), kept separately.
   - Full main article: opening development/sample discussion, regional factor analysis, visualization, all 12 factors, version changes, hierarchy, updated-profile closing paragraphs. Dendrogram retained locally.
   - Source typos, including “Discovery factory,” “ways or progressing,” and Immersion's “3 underlying motivations,” preserved rather than silently corrected.
3. **Quantic Foundry / Nick Yee, “Gamer Motivation Model” (publication date unknown; modification metadata 2016-10-17).**
   - [Actual local page](captures/2026-10-21/gamer-motivation-model/article.md).
   - [Original](https://quanticfoundry.com/gamer-motivation-model/).
   - [Independent source notes](gamer-motivation-model-source-notes.md), kept separately.
   - Full main-page prose: profile invitation, motivation clusters, bigger picture, game audience profiles. All four embedded substantive slide images retained: `Slide1.png`, `Slide2.png`, `Slide3.png`, `Slide5.png`. Page does not embed Slide4; no invented missing-slide blocker.
   - Figures include a partial report screenshot, model table, motivation map, and Europa Universalis audience example. Screenshot's visible crop is source's own crop; linked interactive report is not captured.

## Method and retained evidence

- Acquisition: 2026-10-21 UTC. Built-in `fetch_content` returned HTTP 403 for all three inventoried URLs. One bounded local Crawl4AI batch returned three substantive HTTP 200 successes with robots checking enabled. No link following, login, CAPTCHA handling, access-control bypass, LLM enrichment, OCR, or model/package downloads.
- Image: `unclecode/crawl4ai@sha256:bd36741e7bdd35ddc1a05d9183e1d6d8cefb61dd640d944a25d026b76e917690`. Unique isolated container, random token, loopback-only port; only this run's container removed afterward.
- [Request](captures/2026-10-21/request.json), [original batch response](captures/2026-10-21/batch-response.json), [run metadata](captures/2026-10-21/run-metadata.json).
- Each source directory also contains unchanged `response.json`, extracted `page.html`, and Crawl4AI `page.md`. These retain rendered page output, including navigation/comments/footer. `article.md` is discoverable reading copy, mechanically sliced from `page.md`, not a summary. Only added attribution/rights header, removed page chrome/comments, and localized substantive image URLs.
- Seven directly embedded PNGs downloaded from public WordPress image host using curl. Host robots checked first; [robots response](captures/2026-10-21/image-host-robots.txt) and [download mapping](captures/2026-10-21/image-downloads.json) retained. Full-size linked image variants used instead of resized thumbnails.
- No PDFs or talks in bounded substantive source set. Docling and transcript tools not needed; shared Docling service untouched. Terms-of-use page, sample notes, related articles, reports, surveys, and bibliography-only leads were not followed.

## Fidelity and verification

- Full main-page prose and headings, not abstracts or summaries. Checked beginning/middle/end against raw Markdown and rendered HTML; no challenge/login/error bodies. Source identity and publication/modification distinction checked.
- Reading copies retain exact extracted article wording. No replacement characters found in reading-copy bodies; earlier notes' punctuation warning concerns earlier retrieval, not this capture.
- Main-article prose compared mechanically with raw extraction after reversing image localization. All local Markdown links/figure paths checked. Seven PNG signatures and dimensions checked; all figures opened and visually confirmed substantive/readable.
- Figures remain images, **not searchable Markdown transcriptions**. Text-only LLM ingestion will miss image-only detail; include linked PNGs with a vision-capable reader. Charts' spatial relationships are not converted to text. This is a conversion limit, not a blocked source.
- Reading copies omit decorative hero images, navigation, sharing controls, author bio, comments, and site footer; untrimmed captures retain their extracted text/URLs. Remote outgoing links remain online references, not local captures. No claim of preserving whole websites or linked instruments.
- No application, framework, workflow, sibling-folder, Git index, or configuration changes. No commits or publication.
