# Game grammar / meaningful play — local source

Source supports meaningful-play concepts. Noun–verb matrix, case model, ownership, procedure, and worked example are independently authored workbench applications; no claim the book prescribes them. See [application guide](../../frameworks/game-grammar.md).

## Source and reading copies

Katie Salen and Eric Zimmerman, *Rules of Play: Game Design Fundamentals*, chapter 3, **Meaningful Play**, Unit 1: Core Concepts, printed pp. **31–37**. Seven-page teaching scan. Original edition/year not independently established from an edition/copyright page in this excerpt; the course URL's AY2006 is not a book publication date.

- [Full local chapter reading copy](captures/meaningful-play/meaningful-play.md): primary reading format, OCR/layout repairs checked visually against all seven rendered source pages. Source text, not an independent summary or workbench specification.
- [Original PDF, unchanged](captures/meaningful-play/sz-unit11.pdf).
- [Raw Docling Markdown](captures/meaningful-play/docling/sz-unit11.md), [conversion metadata](captures/meaningful-play/docling/conversion.json), [async task record](captures/meaningful-play/docling/async-task.json), [image-OCR record](captures/meaningful-play/docling/image-ocr.json), [extracted opening photograph](captures/meaningful-play/docling/images/sz-unit11-001.png).
- [Public Georgia Tech teaching copy](https://sites.cc.gatech.edu/classes/AY2006/cs4455_spring/documents/sz-unit11.pdf), fetched and inspected 2026-10-21.
- [MIT Press book listing](https://mitpress.mit.edu/9780262240451/rules-of-play/): bibliographic lead; direct URL validation again returned HTTP 403 on 2026-10-21. Not evidence of full-book access or redistribution permission.

PDF: **1,125,711 bytes**; SHA-256 **`3495aed9c786ee71c0cfe1bcf73f9f2150ed3ca68f33df98fbd5988a5a548441`**, checked 2026-10-21T04:08:39Z. Downloaded directly from the explicit public URL, without authentication, bypass, crawl, or publisher-site access workaround.

## Inspected coverage

| Printed page | Actual source content | Workbench use and limit |
| --- | --- | --- |
| 31 | Chapter-opening keywords, photograph, attributed Huizinga passage. | Context; neither independent reading of *Homo Ludens* nor verified quotation history. Source PDF retains marginal photo credit; extracted image has no separate publication clearance. |
| 32–33 | Play, meaning, choices, player/system interaction and context; examples across forms of play. | Do not equate object/rule inventory with experienced meaning or limit all play to formal state transitions. |
| 33–34 | Two Kinds of Meaningful Play: action/outcome relation versus authors' evaluative discernible/integrated criterion. Gross-Out includes other players' responses. | Distinguish descriptive relation from a proposed design aim; neither is a validated quality score. No noun–verb matrix prescribed. |
| 34–35 | Discernable: perceivable outcomes, Rouse excerpt, off-screen events, hunger, asteroid examples. | Compare rule, presentation, and interpretation. Not every source prediction about irritation/frustration is an observed universal response. |
| 35 | Integrated: later consequences, chess, decathlon, cumulative score, delayed narrative significance. | Trace significance without requiring immediate disclosure, points for every action, or a complete future-outcome display. |
| 36 | Formal, social, cultural meanings; transition to later chapters; notes. | Cases/matrix do not exhaust social/cultural meaning. Later chapters and quoted books not read here. |
| 37 | Summary definitions and their relation. | Crosscheck the core concepts, not a universal review checklist or empirical test instrument. |

## Capture and fidelity

Initial built-in PDF extraction returned seven pages but only **120 characters**, insufficient for a source reading. No definitions or application claims were inferred from that failed extraction.

Converted the downloaded local scan through existing loopback Docling service, OCR language `en`, extracted images enabled, asynchronous client. Required image `quay.io/docling-project/docling-serve-cpu:v1.30.0`, digest **`sha256:061d35c03611bc15b73d024c8e8387bcf0624279f8b57c16c1567326f214ba56`**. Server remained unchanged. Result **success**, no reported errors, **one extracted image**, no additional image-OCR text. The image is a photograph, not a conceptual diagram requiring transcription.

Raw Markdown had scan/OCR and two-column reading-order defects. It is retained unchanged with original conversion metadata. A separate checked reading copy repairs:

- Fragmented vertical chapter title and missing words in two Huizinga-interpretation bullets, p. 32.
- Misplaced sentence/choice paragraph between columns on p. 33; missing Gross-Out response text on p. 34.
- Integration paragraph text incorrectly inserted into the Rouse quote at the p. 34–35 transition; restored its actual placement and the interrupted quote.
- Notes/body order on p. 36, Roman-numeral III, first-person I/1/l confusion, and duplicate summary bullet markers.
- Local image path, headings/spacing, and source/provenance introduction. Preserves the source's *discernable* spelling and Huizinga/Rouse attributions rather than silently substitute independent sources.

For visual checking, reused the bundled client's existing local API function with JSON export, `do_ocr=false`, `do_table_structure=false`, `include_page_images=true`, `include_images=false`, and embedded image mode. Examined all seven rendered pages, printed 31–37. Temporary page PNGs are inspection aids; original PDF remains the retained page authority. This per-request render did not reconfigure/restart the service, install packages, or enable remote processing.

Manual checks improve reading fidelity; they do not prove typographic identity or remove copyright/scan limitations. The PDF preserves full page layout, notes, marginal credit, and formatting. Consult it for quotation-critical work. The chapter's use of its cited works does not establish those works' full contents or editions.

## Rights and disposition

Copyrighted book chapter and photograph, encountered on a public teaching site. No redistribution licence or permission established. Public download and local OCR do not establish publication rights; neither course hosting nor a citation grants them.

Original PDF, extracted photograph, raw conversion, and checked reading copy are **local-only/not-publication-cleared**. Independently authored guide is not a substitute licence for those assets. Do not claim whole-book review or reproduce later chapters, instruments, or unrelated content. Keep these restrictions in the final whole-branch/history review before any push/completion; no push or publication performed.
