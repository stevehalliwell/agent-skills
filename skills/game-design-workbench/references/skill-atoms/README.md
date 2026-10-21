# Skill atoms — retained sources and independent notes

Status: **two actual full-text local captures**, plus existing independent notes. Captured 2026-10-21. Local offline reference only; **not publication-cleared**.

## Source-to-local-capture mapping

### The Chemistry of Game Design — Daniel Cook

- Source: https://lostgarden.com/2021/03/13/the-chemistry-of-game-design-2/
- [Actual article Markdown](captures/chemistry-of-game-design/article.md).
- [Independent source notes](chemistry-of-game-design-source-notes.md), preserved separately.
- Edition: author's 2021-03-13 repost of 2007 essay, with author-declared minor edits and fixed links. Original 2007 edition not independently acquired or compared.
- Fidelity: full repost prose, headings, lists, captions, conclusion, and complete **References And Notes**, through final research-paper link. Not a summary. Site navigation, comments, and related-post widgets excluded.
- Figures: 14 of 15 embedded images retained in `captures/chemistry-of-game-design/images/`. JPEG files use `.jpg` extensions for image-tool compatibility; original source URLs, response MIME, and unchanged hashes recorded in metadata. Markdown points to those local files.
- Real blocker: **Diagram 11: Active Icon** returned HTTP 404. Original remote image URL and caption remain in Markdown. No substitute drawn or copied from another source. Text capture is complete; visual coverage is partial.
- [Capture metadata](captures/chemistry-of-game-design/capture-metadata.json).

### Loops and Arcs — Daniel Cook / Danc

- Source: https://lostgarden.com/2012/04/30/loops-and-arcs/
- [Actual article Markdown](captures/loops-and-arcs/article.md).
- [Independent source notes](loops-and-arcs-source-notes.md), preserved separately.
- Edition: author-site essay dated 2012-04-30.
- Fidelity: full prose, headings, lists, conclusion, and closing “take care, Danc.” No separate references section appears in article body. Site navigation, comments, and related-post widgets excluded.
- Figures: all four diagrams retained locally as PNG. All four embedded WordPress resize URLs returned HTTP 404; the article's own linked original-image URLs succeeded. Markdown embeds local originals while retaining source image links.
- [Capture metadata](captures/loops-and-arcs/capture-metadata.json); [image recovery provenance](captures/loops-and-arcs/image-recovery.json).
- No full-text or figure blocker remains.

## Actual acquisition and verification

1. Read every existing source card and README. Bounded substantive source set: the two essays above. No prior actual Markdown source conversions existed in this folder.
2. Built-in `fetch_content` returned readable complete text for both URLs. Retained its original per-source result as `fetch-result.json` and extracted prose as `fetch-readable-original.md`.
3. Retrieved public source HTML directly with Python standard-library HTTP client; retained unchanged `page.html` and `http-headers.txt` per source.
4. Used existing installed Defuddle CLI, `parse page.html --markdown --json`, without installation. Retained unchanged `defuddle-result.json`. Wrote readable `article.md` from extracted source text, with attribution and rights header. Localized only downloaded image embeds.
5. Defuddle omitted Chemistry's final MDA link and last two reference headings/links. Restored those exact source passages from complete built-in extraction, cross-checked against raw HTML. Original extraction results remain unchanged.
6. Read both resulting articles through beginning, middle, and end. Checked Chemistry's learning-loop and burnout sections, conclusion, and final references; checked Loops' loop/arc definitions, market discussion, conclusion, and signature. Neither is an abstract, challenge page, or error response.
7. Compared raw HTML `entry-content` text segments against each final Markdown after removing punctuation/spacing: **Chemistry 407 segments, Loops 238 segments; no missing segment longer than 40 normalized characters**. This supports coverage, not semantic validation of author's claims.
8. Checked local Markdown destinations and image signatures, HTTP MIME types, and hashes. No OCR, remote enrichment, model download, containers, Docling, installs, or reusable scripts needed. Figures retained as images, not transcribed into text; no claim of diagram-content validation.

## Rights and scope

Copyright remains with Daniel Cook and any other rightsholders. No suitable redistribution license established. Attribution and public accessibility do **not** establish permission to publish. Captures retained for user's requested local offline LLM reference only: **local-only / not-publication-cleared**. Rights uncertainty is not reported as a retrieval blocker.

Existing `.gitkeep`, source cards, and independent notes remain. Notes are interpretation, not replacements for captured source text. Earlier notes-only rights statements describe the earlier pass; capture links and corrected current retention statements now identify actual local sources.

## Excluded leads — not missing captures

- *Project Horseshoe 2007 slides: Smashing the game design atom*: existing README recorded only landing-page retrieval, explicitly not reading presentation. Unread lead; not chased.
- Workshop presentation linked from Chemistry: explicitly unacquired in existing card. Not chased.
- Original 2007 publication: edition-comparison lead, not another substantively reviewed source. Captured author repost instead, with edition limit stated above.
- Books, linked background research, MDA, Tetris PDF, and other bibliography entries: citations within captured article, not independently read sources in assigned cards. Links preserved; no new acquisition sweep.

## Comments

No application, framework, workflow, or sibling-folder edits. No commits, staging, dependency installation, or shared-service changes. This pass retains sources, not empirical validation of Cook's framework or project modeling choices.
