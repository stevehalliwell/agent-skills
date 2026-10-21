# GameFlow research references

Status: **two actual local full-article Markdown captures**, plus preserved independent notes. Local retention only; **not publication-cleared**. `.gitkeep` preserved. No framework, workflow, template, or example-game changes.

## Source-to-local-capture mapping

1. **Penelope Sweetser, Daniel Johnson, Peta Wyeth — Revisiting the GameFlow Model with Detailed Heuristics (2013).**
   - [Actual source Markdown](captures/revisiting-gameflow-2013/revisiting-gameflow-2013.md).
   - [Independent source notes](revisiting-gameflow-detailed-heuristics-2013-notes.md), separate from capture.
   - [Publisher article](https://ojs.aut.ac.nz/journal-of-creative-technologies/index.php/JCT/article/view/16); [original online PDF](https://ojs.aut.ac.nz/journal-of-creative-technologies/index.php/JCT/article/download/16/14/).
   - Full article text, abstract through discussion, references, list of tables, and journal/license end matter. Tables 1–5 retained; several heuristic tables become headings/lists rather than original page layout. Some table/category order and line breaks are imperfect. Zero detected figures exported; no visual page facsimiles. Article presents selected heuristics, not all 165 from its separately cited 2012 paper; that is source coverage, not missing conversion text.
   - Copyright 2013 Penelope Sweetser, Daniel Johnson, Peta Wyeth & Colab; CC BY-NC-ND 4.0, as recorded in publisher metadata and PDF end matter. [License](https://creativecommons.org/licenses/by-nc-nd/4.0/). Noncommercial/no-derivatives restrictions apply; this format conversion is retained for user's offline reference, not approved for public redistribution.
2. **Penny Sweetser, Daniel Johnson, Jay Kyburz — Evaluating GameFlow in a Multiplayer Online Strategy Game Under Development (2020).**
   - [Actual source Markdown](captures/gameflow-multiplayer-2020/gameflow-multiplayer-2020.md).
   - [Independent source notes](gameflow-multiplayer-development-2020-notes.md), separate from capture.
   - [DOI](https://doi.org/10.1145/3373017.3373068); [institutional record](https://openresearch-repository.anu.edu.au/items/dba45f73-579a-437d-8f56-463aaac1682c); [original online PDF](https://openresearch-repository.anu.edu.au/bitstreams/230a1d79-5ffd-4572-aee0-e26e50ed435f/download).
   - Full article text, abstract through sections 1–8 and references [1]–[33], including tables 1–10. Opening author/abstract metadata is interleaved by two-column extraction; bullet glyphs, accented names, and copyright symbol show encoding defects. Some reference ordering and paragraph/list boundaries are imperfect. Two local PNGs retained, including Figure 1 game screenshot; no figure OCR or interpretation, no page facsimiles. Figure links resolve.
   - Copyright 2020 held by owner/authors; publication rights licensed to ACM. Source's qualified personal/classroom-copying notice is retained in Markdown. No broad redistribution license established. **Local-only / not-publication-cleared**; third-party figure rights remain subject to source notice.

## Acquisition and verification

Capture date: **2026-10-21**. Both legitimate public PDFs downloaded with curl into isolated temporary scratch, then converted with existing shared Docling v1.30.0 at `http://127.0.0.1:5001`. Born-digital conversion used `--no-ocr --images --no-image-ocr --async`; table structure enabled. No remote OCR, LLM enrichment, dependency/model downloads, or container changes. No PDF binaries retained here; use online PDF links above.

Docling image: `quay.io/docling-project/docling-serve-cpu@sha256:061d35c03611bc15b73d024c8e8387bcf0624279f8b57c16c1567326f214ba56`. Shared service was not restarted, stopped, or reconfigured.

Each capture directory retains original `conversion.json` response, `async-task.json`, and `capture-metadata.json` with source URL, input SHA-256, method, checks, and rights. Readable `.md` is linked above, not buried only in JSON. Neither capture overwrote an existing conversion.

Both results report `status: success`, `errors: []`. Read beginning, middle, and end, including actual methods/results/discussion and full reference endings; confirmed source title/authors/DOI or journal identity. Output is article text, not abstract-only, login, challenge, or error content. Checked relative Markdown and image paths. Original extraction is preserved without handwritten replacement text; fidelity is full-text conversion, not pixel-exact or error-free transcription.

2020 async client hit `[WinError 10048] Only one usage of each socket address (protocol/network address/port) is normally permitted`. Recovered **same task** `2069fc8b-2712-478d-b5b1-c2847a3489f0` through bounded curl polling and result retrieval, then used existing client's image extraction routine. No resubmission or service restart. A transient loopback connection failure cleared on retry.

## Scope and blockers

No acquisition blockers remain for the two substantively read sources above. Existing folder contained no actual full-text conversions to reuse.

Preserved historical leads are **outside this bounded source set**, not silently represented by later articles:

- Original 2005 *GameFlow: A Model for Evaluating Player Enjoyment in Games*, Penelope Sweetser and Peta Wyeth, [DOI](https://doi.org/10.1145/1077246.1077253): prior attempt returned HTTP 406; suggested Values at Play PDF returned HTTP 404. Original full paper was not reviewed. No new acquisition attempted here.
- 2017 *GameFlow in Different Game Genres and Platforms*, [DOI](https://doi.org/10.1145/3034780): previously abstract-only discovery, not full-text reading. Prior search mislabeled the 2020 bitstream; verified captured title and DOI identify 2020 paper. No new acquisition attempted here.
- Other works mentioned only in source bibliographies or background discussion, including 2012 heuristics paper and books, were not chased.

## Comments

Independent notes remain selective interpretation, not source captures or evidence about the workbench example game's players. Local full-text retention does not imply permission to publish these files, tables, or figures.
