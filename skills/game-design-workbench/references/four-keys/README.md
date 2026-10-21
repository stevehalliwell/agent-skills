# Four Keys to Fun: local research references

Status: **two actual source captures retained**, plus original independent notes. Acquired 2026-10-21 for private offline LLM reference. **LOCAL-ONLY / NOT-PUBLICATION-CLEARED.** Public access does not establish redistribution rights.

## Source-to-local-capture mapping

1. **Nicole Lazzaro, *Why We Play Games: Four Keys to More Emotion in Player Experiences* (2004).**
   - [Actual full-document Markdown](captures/why-we-play-games/why_we_play_games.md).
   - [Original public PDF](https://media.gdcvault.com/gdc04/slides/why_we_play_games.pdf). Existing notes identify this as 46-page illustrated report, despite `/slides/` URL.
   - Method: public PDF downloaded to temporary scratch; shared local Docling v1.30.0, async, `--no-ocr --images --no-image-ocr`. Result: `success`, no reported errors. [Original conversion response](captures/why-we-play-games/conversion.json), [terminal task status](captures/why-we-play-games/async-task.json), [HTTP headers](captures/report-http-headers.txt).
   - WinError 10048 interrupted client polling. Recovered **same task** through bounded slow `curl` polling and result retrieval; no resubmission or service lifecycle changes.
   - Full-document coverage: opening title/introduction/method, original four-key overview, detailed sections 1a–4g, conclusion, Appendix A methodology, Appendix B participant table, references, acknowledgements and final copyright notice. Read beginning/middle/end and section progression; not abstract-only or challenge content.
   - All **66 figure links** resolve to retained PNGs in [images/](captures/why-we-play-games/images/). Images are detected raster figures, not guaranteed original embedded binaries. No OCR, image OCR, visual interpretation or page-perfect audit. Some captions/read order and table layout are imperfect; special-character mojibake such as `Â®`, `â€¦` remains in raw conversion. Text remains readable; no handwritten replacement of source text.
   - PDF not retained after conversion. Use online original above; Markdown and figures are offline deliverables.
   - Rights: © 2004 XEODesign, Inc., All Rights Reserved. Player photos separately © 2004 XEODesign, Inc., All Rights Reserved. Trademarks belong to their holders. No redistribution grant established.

2. **Nicole Lazzaro / XEODesign, *The 4 Keys 2 Fun* (author overview; webpage publication date unknown).**
   - [Actual complete article-body Markdown](captures/author-overview/the-4-keys-2-fun.md).
   - [Original author page](https://www.nicolelazzaro.com/the4-keys-to-fun/).
   - Method: usable built-in `fetch_content` readable extraction saved as Markdown, title retained, whitespace/list formatting normalized. `curl` retained [raw HTML](captures/author-overview/page.html) and [HTTP headers](captures/author-overview/http-headers.txt). Checked article text against HTML.
   - Full article body includes opening “Emotion and the Fun of Games,” all four associations, middle author/research discussion, productivity-software paragraph and closing contact details. Navigation, unrelated sidebar blog excerpts, styles and scripts omitted from readable Markdown; retained in raw HTML.
   - [Source thumbnail](captures/author-overview/4_keys_thumb.png) retained locally and Markdown image link localized. Linked full-size poster is outside substantive source set and not acquired; poster hyperlink remains external. Other source hyperlinks remain online. No browser rendering or OCR. Raw HTML has some replacement glyphs; readable extraction preserves punctuation.
   - Rights: no reuse license established. Author/site content retained with attribution, local-only and not publication-cleared.

[Acquisition metadata](captures/acquisition.json) records URLs, PDF checksum, methods, task ID, resolved Docling digest, rights and limits. No existing actual captures were present to reuse; neither new capture overwrote another capture.

## Existing source cards and independent notes

Preserved separately; these are synthesis, **not** source captures:

- [2004 report notes](lazzaro-2004-why-we-play-games-notes.md) — research method, original labels, examples and evidence limits.
- [Author overview notes](lazzaro-four-keys-author-overview-notes.md) — later labels, emotion associations and terminology bridge.
- `.gitkeep` preserved unchanged.

## Bounds, checks and blockers

- Read every existing card and README. Substantive bounded inventory: report and author overview above. Author-hosted PDF is alternate/abstract lead, not a third inspected source; GDC report supplies actual research text. No new source sweep, books, bibliography-only leads, contact-only slides or poster acquisition.
- No full-text blockers for either inventoried source. Earlier author-hosted PDF access failure remains historical context, not failure of GDC report capture.
- Checked source identity, opening/middle/closing text, conclusion/references and Docling status; checked all local Markdown links and all exported figure paths. Completeness claim concerns document/article-body coverage, not exact typography or visual reconstruction.
- No remote OCR, LLM enrichment, package/model downloads, service restarts, commits, staging or publication. No application/framework/workflow changes. Writes confined to this folder; temporary input PDF removed.

## Comments

Earlier notes-only retention reflected a redistribution concern. Current local captures do not imply permission to redistribute. Original independent notes remain intact apart from local-capture links and corrected retention statements.
