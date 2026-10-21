# Theory of Fun: local source captures

Status: **2 substantive sources captured in full readable Markdown**. Existing independent notes and `.gitkeep` preserved. No full-text access blockers.

## Source-to-local-capture mapping

- Raph Koster, *Geek Fun Isn’t Frivolous* (2005-05-15): [actual full text](koster-2005-geek-fun-full-text.md); [independent source notes](koster-2005-geek-fun-source-notes.md). Complete author-hosted essay, including introductory publication note and historical newspaper link. Newspaper/syndicated editions not independently captured.
- Raph Koster, *Fun in class, in sex?* (2008-10-13): [actual full text](koster-2008-fun-in-class-full-text.md); [independent source notes](koster-2008-fun-in-class-source-notes.md). Complete authored post, including opening attributed quotation, both questions, response, and outbound references. Comments not included in readable capture.

## Method and checks

Captured **2026-10-21** with built-in `fetch_content` readable HTML extraction. Inspected complete returned text using `get_search_content` (response ID `mvhcyp1b9uwacr`). Saved source text verbatim, with separate provenance headers; no handwritten replacement summaries. Removed empty trailing “Post navigation” heading from 2008 extraction.

Retained original HTML responses and HTTP headers with `curl` in [captures/2026-10-21/](captures/2026-10-21/). [Capture metadata](captures/2026-10-21/metadata.md) records bounds, robots result, files, and checks. No previous full Markdown conversions existed to reuse.

Verified titles and author/source identity; inspected beginning, middle, and ending of both articles, not just abstracts or snippets. Compared every article word token against separately fetched raw HTML: all 1,230 tokens in 2005 capture and all 759 tokens in 2008 capture matched after removing Markdown link targets and normalizing punctuation. This supplements direct full-text inspection; size alone was not used as proof. HTML confirms neither article body contains image elements. Both conclusions and all body reference links retained. Local Markdown links checked. Outbound references remain online; not recursively captured or checked for current availability.

Readable captures omit site navigation, sharing controls, and comments. No substantive article headings were omitted. Source spelling retained. HTML response contains some replacement punctuation; readable extraction preserves legible punctuation. Raw HTML remains unchanged. No PDF, OCR, captions, figures, or Docker services needed. No packages installed; no shared service changed.

## Rights and local use

Copyright remains with original author/publisher and quoted contributors. No suitable open license or redistribution permission established. **Local-only offline reference; not publication-cleared.** Public accessibility does not establish redistribution rights. Retention here does not authorize publication. Attribution and original URLs appear in each capture. Existing source notes remain independent commentary, not substitutes for source text.

## Bounded inventory and exclusions

Read every existing source note and README in this folder. Two articles above are substantive sources actually discussed. Other URLs in existing inventory are contextual/bibliographic, not additional substantive sources:

- https://www.raphkoster.com/gaming/theoryoffun.shtml — introduction to 2003 presentation; prior inventory explicitly says presentation itself was not extracted/read. Slides omit spoken explanation. No talk capture claimed or newly pursued.
- https://www.theoryoffun.com/ — official book site; book remains bibliographic and excluded.
- https://www.raphkoster.com/2012/09/06/gdconline-revisiting-a-theory-of-fun/ — announcement only; prior inventory explicitly says linked presentation was not read. No slide acquisition claimed.
- Historical newspaper link and discussions cited inside 2008 post remain references, not newly added acquisition targets. Opening quotation remains attributed to its linked source.

## Comments

Earlier notes-only rights statements described initial acquisition, not current retention. Corrected those statements minimally and linked new full captures. Findings and independent notes otherwise preserved. No framework/application changes, source replacement, commits, or publication.
