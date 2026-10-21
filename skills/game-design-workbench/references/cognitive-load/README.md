# Cognitive-load local sources

Status: **two actual article captures available**. Existing 2019 conversion reused; 1988 access gap resolved through publicly hosted PDF. No books or bibliography-only leads acquired. Existing independent notes and `.gitkeep` preserved.

## Source inventory and local captures

1. **John Sweller, Jeroen J. G. van Merriënboer, Fred Paas (2019), *Cognitive Architecture and Instructional Design: 20 Years Later*.** *Educational Psychology Review*, 31, 261–292; published online 22 January 2019. [Original DOI](https://doi.org/10.1007/s10648-019-09465-5).
   - Actual source text: [2019 full-document Markdown](sweller-2019.docling/sweller-van-merrienboer-paas-2019.md).
   - Method/date: existing publisher-PDF Docling conversion, 2026-10-21; verified/reused 2026-10-21, not duplicated. OCR and image export disabled. No redundant PDF restored.
   - Coverage: abstract/introduction, theory/history, instructional effects, measurement, future directions, conclusions, Open Access statement, complete bibliography through Youssef-Shalala et al. Full article text coverage, not facsimile or word-perfect transcription.
   - Limits: Table 1 rows/columns merge; spacing, hyphenation, superscripts and typography have artifacts. Figure 1 caption/discussion retained; original publisher PNG now retained and linked beside caption. Other image placeholders are comments, not broken links. Consult [original online PDF](https://link.springer.com/content/pdf/10.1007/s10648-019-09465-5.pdf) for exact table layout and figure. PDF binary no longer retained locally.
   - Rights: © The Author(s) 2019, [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/), explicitly stated in article. Attribution, license link and conversion change notice retained. No publication performed.
2. **John Sweller (1988), *Cognitive Load During Problem Solving: Effects on Learning*.** *Cognitive Science*, 12(2), 257–285. [Original DOI](https://doi.org/10.1207/s15516709cog1202_4).
   - Actual source text: [1988 full-document Markdown](captures/sweller-1988.docling/sweller-1988.md).
   - Download: [publicly hosted PDF](https://andymatuschak.org/files/papers/Sweller%20-%201988%20-%20Cognitive%20load%20during%20problem%20solving.pdf), curl, 2026-10-21. Host robots.txt returned HTTP 200, `User-agent: *`, `crawl-delay: 10`, no disallow. No login, paywall, CAPTCHA or permission controls bypassed.
   - Method: temporary PDF converted through existing shared loopback Docling service, `--no-ocr --images --no-image-ocr --async`. No OCR, remote enrichment or model downloads. Three detected figures exported to [images/](captures/sweller-1988.docling/images/). Temporary PDF removed after conversion.
   - Coverage: article opening; expert/novice distinctions; learning and schema acquisition; production-system model; experimental method/results; theoretical/practical implications; conclusions; references ending with Voss, Vesonder and Spilich (1980). Full-document extraction, not summary or abstract-only content.
   - Limits: paragraph and line-order defects split/reorder some sentences; table/equation extraction and typography are imperfect. Wiley download/terms footers repeat. Three figure raster exports have no OCR or interpretation. Not word-perfect or layout-faithful; use linked source PDF for exact quotations/equations.
   - Rights: John Sweller credited. Wiley terms/download footer retained. No explicit open license or definitive copyright-holder statement identified in extracted text. Public hosting does **not** establish redistribution permission. **Local-only / not-publication-cleared.** Retained for user's offline reference, not approved for distribution.

## Existing cards and independent notes

- [2019 independent reading notes](sweller-2019-reading-notes.md): selective authored notes, not source capture or additional source. Existing substantive content preserved; stale PDF link and 1988 access status corrected.
- No separate source-card files existed in this folder. Original README and notes discussed only the two articles above as acquisition targets. Citations inside the captured review are bibliography-only leads, not additional acquisition scope.
- `.gitkeep` preserved.

## Conversion and verification records

### Reused 2019 conversion

Original acquisition record: publisher HTML retrieved with `fetch_content`, PDF downloaded with curl, then installed Docling client used asynchronously with `--no-ocr --no-images`, 600-second timeout. Reported status `success`, errors `[]`, processing time 115.69146989800004 seconds. Original transient task/response artifacts were already removed; no new claim of rerunning conversion.

- Source PDF SHA-256 recorded by original acquisition: `47ce1d814f9b84c0a37daf7f6d81f7995a125b569acf4cf9d7d91d145c97f619`.
- Read entire retained Markdown in bounded chunks. Checked opening abstract/introduction, middle 4C/ID/instructional-effects/measurement sections, concluding paragraphs, license, and bibliography end. Text is substantive article prose, not challenge/login page.
- Original acquisition spot-checked element interactivity, knowledge dependence, germane-load revision, split attention, expertise reversal and measurement limits against publisher HTML. Current pass verifies retained coverage, not independent line-by-line transcription accuracy.

### 2019 Figure 1 supplement

- Acquired 2026-10-21 without reconverting article or downloading PDF. Built-in `fetch_content` retrieved [publisher figure page](https://link.springer.com/article/10.1007/s10648-019-09465-5/figures/1), identifying Figure 1, its caption and original PNG URL.
- Downloaded [publisher PNG](https://media.springernature.com/full/springer-static/image/art%3A10.1007%2Fs10648-019-09465-5/MediaObjects/10648_2019_9465_Fig1_HTML.png) directly with curl. Media host robots.txt returned HTTP 200, `User-agent: *`, `Allow: /`.
- Local image: [10648_2019_9465_Fig1_HTML.png](sweller-2019.docling/images/10648_2019_9465_Fig1_HTML.png), PNG, 1406 × 1015 pixels. SHA-256: `9060e379491697d03b754123d14f70306113ea4ee71f45a1b195ce93dcf0427f`.
- Opened image and verified four labeled components, diagram and readable text; not challenge/error content. Replaced only existing placeholder immediately after Figure 1 caption with local image link. Source prose preserved. No OCR, image transformation or interpretation added.
- Attribution: Sweller, van Merriënboer and Paas (2019), Figure 1, adapted from van Merriënboer and Kirschner (2018a), as credited in original caption. Article's CC BY 4.0 statement retained; no separate figure exception displayed on fetched figure page. No publication performed; public availability alone is not independent clearance for any underlying third-party material.

### New 1988 conversion

- [Raw conversion result](captures/sweller-1988.docling/conversion.json) preserves original service Markdown with embedded image data; readable Markdown beside it has extracted local figure paths and added provenance header.
- [Terminal task status](captures/sweller-1988.docling/async-task.json): same task `b686e068-fcb8-4992-961e-8cf0860fee6a`. Python client encountered `WinError 10048` while polling; recovered **same submitted task** through curl, bounded 20-second polling. No repeat submission or service restart.
- Conversion status `success`, errors `[]`, processing time 38.9476055959999 seconds.
- Temporary PDF SHA-256: `e4915f345091729347dfc1e6cc0af771f0be824675d27680510055424085486b`.
- Checked source identity/abstract, model and Table 2 in middle, empirical headings, implications/conclusions, and references through final Voss entry. Checked all three local figure links exist. Actual source text retained without handwritten repair or replacement summary.

Both conversions use `quay.io/docling-project/docling-serve-cpu:v1.30.0`, resolved digest `quay.io/docling-project/docling-serve-cpu@sha256:061d35c03611bc15b73d024c8e8387bcf0624279f8b57c16c1567326f214ba56`. Shared service at `http://127.0.0.1:5001` was not stopped, restarted or reconfigured.

## Access failures and remaining limits

- Publisher 1988 PDF endpoint `https://onlinelibrary.wiley.com/doi/epdf/10.1207/s15516709cog1202_4` returned HTTP 403 in current pass. Earlier acquisition also recorded publisher `fetch_content` HTTP 403 and one robots-enabled isolated Crawl4AI fallback returning HTTP 500. Those failures are not full-text captures.
- Public PDF host supplied actual full article, resolving source-text blocker; redistribution clearance remains unknown. No Crawl4AI container needed in current pass.
- No remaining source-text acquisition blocker for bounded two-source inventory. 2019 Figure 1 visual gap resolved; both conversions retain stated layout/text fidelity limits. Educational evidence is not direct gameplay validation.
