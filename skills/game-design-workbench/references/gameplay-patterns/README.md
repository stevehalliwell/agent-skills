# Gameplay design patterns — local sources

Status: **3 sources captured in full readable Markdown** on 2026-10-21. Independent notes and `.gitkeep` preserved. No catalog sweep, book acquisition, framework changes, staging, or commits.

## Source-to-local-capture map

1. **Staffan Björk, Sus Lundgren, Jussi Holopainen — Game Design Patterns (2003)**
   - [Actual local paper](captures/game-design-patterns-2003.docling/game-design-patterns-2003.md).
   - [Original public PDF](https://dl.digra.org/index.php/dl/article/download/60/60); [publication record / DOI](https://doi.org/10.26503/dl.v2003i1.60).
   - [Independent source notes](bjork-lundgren-holopainen-2003-notes.md), kept separately.
   - Method: downloaded public PDF to temporary scratch; local Docling v1.30.0, `--no-ocr --images --no-image-ocr --async`. Shared service unchanged. Windows client hit `WinError 10048`; recovered **same** task through curl, without resubmission. [Conversion result](captures/game-design-patterns-2003.docling/conversion.json); [task record](captures/game-design-patterns-2003.docling/async-task.json).
   - Coverage: all 15 PDF pages, including copyright leaf, introduction, model development, structural framework, worked Paper Rock Scissors entry and its references, applications, discussion, conclusion, acknowledgments, and all 48 main references. Docling status `success`, errors `[]`. Beginning/middle/end inspected; independent temporary PDF text extraction checked page boundaries and reference endpoint, not used as replacement capture.
   - Fidelity limits: readable text conversion, not facsimile. Two-column spacing, page furniture, line-break hyphens, entity escapes, and split URLs remain. Detected figure export produced **0 images**; no substantive figure seen in text/layout checks. No page screenshots or diagram interpretation. Temporary original PDF removed after checks; online link above is authoritative.
   - Rights: source copyright © 2003 authors, Utrecht University, and DiGRA; **all rights reserved**. Original notice remains in capture. Retained for user's private offline reference only; **local-only / not-publication-cleared**. Public access is not redistribution permission.

2. **Gameplay Design Patterns wiki — Pattern-Based Methods**
   - [Actual local page](captures/pattern-based-methods.md); [original page](https://gdp.cse.chalmers.se/index.php?title=Pattern-Based_Methods).
   - [Independent source notes](pattern-based-methods-notes.md), kept separately.
   - Method: built-in `fetch_content` readable extraction; copied returned text directly from stored result, not rewritten. Raw HTML separately downloaded. Captured revision **91**, identified in HTML; page undated in readable body. [Raw HTML](captures/pattern-based-methods.html).
   - Coverage: complete article body from opening sentence through **The Question of Orthodoxy**. Includes analysis, design, three original “To be written” subsections, testing, and tacit-knowledge discussion. Article heading sequence checked against raw HTML. No references section or substantive article images present. Navigation/search controls omitted; no linked pages acquired.
   - Rights: wiki maintained by Staffan Björk and Jussi Holopainen; individual page authorship not established. No content license established; copyright/redistribution permission unknown. **Local-only / not-publication-cleared**.

3. **Gameplay Design Patterns wiki — Save-Load Cycles, revision 26347**
   - [Actual local entry](captures/save-load-cycles-revision-26347.md); [original inspected revision](https://gdp.cse.chalmers.se/index.php?title=Save-Load_Cycles&oldid=26347).
   - [Independent source notes](save-load-cycles-notes.md), kept separately.
   - Method: built-in `fetch_content` readable extraction; copied returned text directly from stored result, not rewritten. Raw HTML separately downloaded and revision **26347** confirmed. [Raw HTML](captures/save-load-cycles-revision-26347.html).
   - Coverage: complete article body, including opening definition, examples, use, diegetic/interface/narration aspects, consequences, every relations subsection, history, references, and acknowledgements. Beginning/middle/end and heading sequence checked against HTML. No substantive article images present.
   - Fidelity limits: source's unprocessed `<ref>` / `<references>` markup remains in Markdown; reference text is readable in file. Original typos, placeholders, empty sections, and missing-page/redlink destinations preserved. Wiki navigation omitted. Site-relative article links expanded to original host, not falsely presented as local captures. Linked games/patterns not acquired or verified.
   - Rights: same wiki attribution and unknown content license as above. Entry cites *Patterns in Game Design* (2004); book not acquired. **Local-only / not-publication-cleared**.

## Provenance and checks

- [Original built-in fetch result](captures/wiki-fetch-result.json) preserves both extracted wiki bodies, URLs, and fetch timestamp. Readable `.md` files above remain primary offline entry points.
- [Capture metadata](captures/capture-metadata.json) records methods, URLs, revisions, hashes, Docling image/task, checks, and limits.
- Only this assigned directory received retained artifacts. No existing captures overwritten. `.gitkeep` unchanged.
- Markdown links checked for local destinations; no missing figure files. Wiki article links remain online; linked-source availability not asserted.
- Scope: three substantive sources actually discussed in existing cards. Wiki Main Page was attribution context only. Bibliographic leads, linked games/patterns, missing Reversibility/Reversability entries, guessed catalog URLs, and books excluded. Paper's worked example retained as part of paper, not a fourth source.

## Blockers

None for full-text access or local conversion of this bounded source set. Publication/redistribution rights remain restricted or unestablished; **no publication clearance claimed**. Local retention does not authorize redistribution.
