# Capture metadata

- Date: 2026-10-21 UTC.
- Scope: two existing substantive article sources; no link following.
- Readable extraction: built-in `fetch_content`, response `mvhcyp1b9uwacr`, both full results retrieved with `get_search_content`.
- Raw retention: `curl -L --fail --max-time 60 -sS -D <headers> <original-url> -o <html>`; both commands succeeded.
- Robots: fetched https://www.raphkoster.com/robots.txt before raw acquisition. `User-agent: *` disallows only `/calendar/action~posterboard/`, `/calendar/action~agenda/`, `/calendar/action~oneday/`, `/calendar/action~month/`, `/calendar/action~week/`, `/calendar/action~stream/`. Neither source path disallowed.
- Rights: copyright original author/publisher and quoted contributors. No open license established. Local-only reference; not publication-cleared.

## Files

- `geek-fun.html`, `geek-fun.headers.txt`: https://www.raphkoster.com/games/essays/geek-fun-isnt-frivolous/ ; [readable full text](../../koster-2005-geek-fun-full-text.md).
- `fun-in-class.html`, `fun-in-class.headers.txt`: https://www.raphkoster.com/2008/10/13/fun-in-class-in-sex/ ; [readable full text](../../koster-2008-fun-in-class-full-text.md).

Raw HTML includes surrounding site material; readable Markdown isolates article text. Source responses retained unchanged, including replacement punctuation present in raw HTML. Readable extraction has legible punctuation. No screenshots or image downloads; article `entry-content` elements have zero image elements.

## Verification

- 2005: opening E3 invasion; middle practice, positive feedback, Chutes and Ladders; ending games growing up and saving world. Publication note retained.
- 2008: opening attributed Teaching Game Design quotation; middle disconnected concepts and static; ending complementary models and blind-men/elephant parable. All outbound article references retained.
- Compared complete Markdown article word-token sequences to raw HTML text using Python standard-library HTML parser, excluding Markdown link targets and normalizing non-ASCII punctuation: exact match, 1,230 and 759 tokens respectively. No summary substitution, challenge text, or abstract-only capture.
- Neither article has a separate references section beyond inline links. No PDF or video source in bounded set.
- No container, shared Docling operation, OCR, remote enrichment, dependency install, or script file created.
