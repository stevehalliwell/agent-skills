# Self-determination reference captures

Status: **three sources captured locally**. Actual source text retained alongside existing independent notes. Acquisition and verification date: **2026-10-21**.

**LOCAL-ONLY / NOT PUBLICATION-CLEARED.** Public access does not establish redistribution permission. No permissive republication license verified. Do not publish these captures without separate rights review. Original authors and publishers retain rights.

## Source-to-local-capture mapping

1. **Ryan and Deci (2000), *Self-Determination Theory and the Facilitation of Intrinsic Motivation, Social Development, and Well-Being*.** *American Psychologist*, 55(1), 68–78. [DOI](https://doi.org/10.1037/0003-066X.55.1.68).
   - **[Actual full-article Markdown](captures/ryan-deci-2000/ryan-deci-2000.md)**; [local figures](captures/ryan-deci-2000/images/); [raw Docling result](captures/ryan-deci-2000/conversion.json); [task record](captures/ryan-deci-2000/async-task.json).
   - [Existing independent notes](ryan-deci-2000-foundations-notes.md), kept separately; not source capture.
   - [Original public PDF](https://www.selfdeterminationtheory.org/SDT/documents/2000_RyanDeci_SDT.pdf). Downloaded to temporary scratch, converted locally through existing Docling service. Born-digital text, `--no-ocr --images --no-image-ocr --async`.
   - Coverage: full 11-page article through final Williams et al. reference. Abstract, body, section headings, conclusions, references, two author portraits and Figure 1 retained. Three exported images; Figure 1 visually checked. Conversion reports `success`, no errors.
   - Fidelity: full article coverage, not a diplomatic transcription. Original text layer has spelling, spacing, hyphenation and symbol defects; HTML entities and occasional heading errors remain. Figure text retained visually without second-pass OCR. Running headers/page numbers need not survive extraction.
   - Rights: copyright 2000 American Psychological Association; no permissive redistribution license verified. Local-only, not publication-cleared.

2. **Ryan, Rigby, and Przybylski (2006), *The Motivational Pull of Video Games: A Self-Determination Theory Approach*.** *Motivation and Emotion*. [DOI](https://doi.org/10.1007/s11031-006-9051-8).
   - **[Actual full-article Markdown](captures/ryan-rigby-przybylski-2006/ryan-rigby-przybylski-2006.md)**; [local images](captures/ryan-rigby-przybylski-2006/images/); [raw Docling result](captures/ryan-rigby-przybylski-2006/conversion.json); [task record](captures/ryan-rigby-przybylski-2006/async-task.json).
   - [Existing independent notes](ryan-rigby-przybylski-2006-video-games-notes.md), kept separately; not source capture.
   - [Original public PDF](https://selfdeterminationtheory.org/SDT/documents/2006_RyanRigbyPrzybylski_MandE.pdf). Same local Docling settings as source 1. Windows client hit `WinError 10048` while polling; recovered **same task** `5ba07656-8c41-4d8c-a4e4-a9cc949c2a70` through bounded slow curl polling and result retrieval. No resubmission or service restart.
   - Coverage: full 17-page supplied article, abstract through all four studies, general discussion and final Yi reference. Tables retained in Markdown. Docling reports `success`, no errors; ten detected image exports retained, some publisher marks rather than substantive figures.
   - Fidelity: spacing, broken compounds, symbol damage, table ordering and numeric formatting can be imperfect. Two model equations on PDF page 11 were **not decoded** by Docling. Added linked [original page 11 image](captures/ryan-rigby-przybylski-2006/images/source-page-11.png), rendered at 2× with existing local `pypdfium2` in shared Docling container, no installs or OCR. Equations therefore retained visually, not machine-readable math. Raw conversion result remains unchanged. No formula enrichment or handwritten reconstruction.
   - Rights: copyright Springer Science + Business Media, LLC 2006; no permissive redistribution license verified. Local-only, not publication-cleared.

3. **Official Self-Determination Theory website, *The Theory*.** No individual byline or publication/update date established.
   - **[Actual complete article Markdown](captures/official-theory-overview/the-theory.md)**; [original raw HTML](captures/official-theory-overview/page.html); [HTTP headers](captures/official-theory-overview/http-headers.txt).
   - [Existing independent notes](official-theory-overview-notes.md), kept separately; not source capture.
   - [Original page](https://selfdeterminationtheory.org/the-theory/). Built-in `fetch_content` readable extraction, response ID `mvhcyni1d8kg9w`; source text saved without summarization. Title retained from raw HTML. Raw HTML and headers captured separately with curl.
   - Coverage: opening SDT definition, organismic viewpoint, all six mini-theories, other topics, applications and all four closing references. All **27 nonempty article heading/paragraph blocks** match retained Markdown after whitespace/markup normalization. No substantive article figures found in raw HTML; branding/navigation images not downloaded.
   - Fidelity: full article text; site navigation/footer excluded from Markdown but preserved in raw HTML. Inline spacing/Markdown emphasis normalized. Original malformed Publications URL retained as source defect; use [working publications page](https://selfdeterminationtheory.org/publications/) if needed. External links remain online, not recursively captured.
   - Rights: official SDT site content, no permissive reuse license established; individual authorship/date unknown. Local-only, not publication-cleared.

## Verification and provenance

- [Verification record](captures/verification.json): PDF hashes, conversion statuses, source-page text samples, headings, end references, image-link checks and HTML block comparison.
- Compared PDF extraction with independently read temporary `pdftotext` output. Sampled beginning/middle/later positions **on every page**, not just file size. All 33 samples match for 2000 paper. For 2006 paper, 49 of 51 match; remaining two concern undecoded model equation and reordered table labels. Checked those regions directly; original equation page retained as image, Table 9 remains present as Markdown.
- Verified article openings, substantive middle sections, conclusions/general discussion and complete final references. No challenge, login, error-page or abstract-only capture accepted.
- All local Markdown and figure links checked. Images use relative paths. Source PDFs were scratch inputs only and removed after verification; online originals linked above. No stale local PDF links.
- Docling endpoint: `http://127.0.0.1:5001`; image `quay.io/docling-project/docling-serve-cpu:v1.30.0`; resolved digest `sha256:061d35c03611bc15b73d024c8e8387bcf0624279f8b57c16c1567326f214ba56`. Reused shared service without restart, stop or reconfiguration. No remote OCR, LLM enrichment, model/package downloads, new tools, or dependency installs.
- Built-in HTML fetch was usable; Crawl4AI not needed. No videos or talks in bounded source set. Bibliographic-only papers and books not acquired.
- Existing `.gitkeep`, source notes and their conceptual content preserved. No preexisting full captures found, so none reused or overwritten.

## Blockers

No full-text access blocker. Remaining fidelity limits: PDF text-layer damage, imperfect table/symbol extraction, and image-only equations. Redistribution rights remain unverified; this is a local reference acquisition, not publication approval.
