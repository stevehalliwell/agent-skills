# Working-memory research references

Status: **2 substantive sources captured locally**. Actual source text, not replacement summaries. Existing independent notes and `.gitkeep` preserved.

Capture date: 2026-10-21 (UTC). Scope: sources substantively discussed in existing two source cards. No books, bibliographic leads, new framework survey, or application changes.

## Source-to-local-capture mapping

### Miller (1956) — captured

- Source: George A. Miller, *The Magical Number Seven, Plus or Minus Two: Some Limits on our Capacity for Processing Information*. *Psychological Review* 63, 81–97. DOI: https://doi.org/10.1037/h0043158.
- **Actual local article:** [miller-1956.md](captures/miller-1956/miller-1956.md).
- Online HTML edition: https://psychclassics.yorku.ca/Miller/ — Christopher D. Green, *Classics in the History of Psychology*, York University.
- Separate independent card: [miller-1956-chunking-source-notes.md](miller-1956-chunking-source-notes.md). Card's selective coverage describes notes, not full capture.
- Method: built-in readable fetch confirmed article content. One bounded Crawl4AI request retained complete HTML edition as Markdown, raw [HTML](captures/miller-1956/page.html), and original [response JSON](captures/miller-1956/response.json). Robots checking enabled; no link following or LLM enrichment. Ten explicitly linked GIFs downloaded separately after robots check; `/robots.txt` returned HTTP 404, with no published restrictions. Isolated loopback container removed after run.
- Fidelity: full HTML article text, section labels, footnotes, references 1–20, and received date. Nine figures and Table 1 retained in [images/](captures/miller-1956/images/), linked locally from Markdown. Exact section labels promoted to Markdown headings; reversible UTF-8/Latin-1 mojibake corrected for Külpe/über. Original response/HTML remain unchanged. No paraphrasing.
- Checks: HTTP 200 and Crawl4AI `success: true`; opening “My problem is that I have been persecuted by an integer”; middle immediate-memory/recoding discussion; closing seven discussion, Summary, footnotes, Woodworth reference 20, and “Received May 4, 1955.” All 10 local image links resolve; all GIF signatures checked; final figure visually inspected. No challenge/error/abstract-only content.
- Limits: fidelity to HTML edition, not independently collated against journal page scans. Existing page markers and typographic/transcription quirks retained. Some source links/footnote anchors remain online; article text and figure content work offline. Figure text remains in image pixels, not OCR. No remote media dependency for the 10 article images.
- Rights: original article attribution above; HTML edition credited to Green/York. No general redistribution license established. Article footnote's U.S. Government reproduction permission is not blanket publication permission. **LOCAL-ONLY / NOT PUBLICATION-CLEARED**, including figures and retained raw artifacts.
- [Capture metadata](captures/miller-1956/capture-metadata.json) records image digest, configuration, normalization, and timestamp.

### Cowan (2001) — captured

- Source: Nelson Cowan, *The magical number 4 in short-term memory: A reconsideration of mental storage capacity*. *Behavioral and Brain Sciences* 24(1), target article pp. 87–114, followed by peer commentaries and author response. DOI: https://doi.org/10.1017/S0140525X01003922.
- **Actual local full-document conversion:** [cowan-bbs-2001.md](captures/cowan-2001/cowan-bbs-2001.md).
- Author-hosted original PDF: https://memory.psych.missouri.edu/assets/doc/articles/2001/cowan-bbs-2001.pdf.
- Separate independent card: [cowan-2001-capacity-source-notes.md](cowan-2001-capacity-source-notes.md). Card's selective coverage describes notes, not conversion scope.
- Method: public author-hosted PDF downloaded to temporary scratch; all 99 PDF pages submitted once to existing local Docling Serve v1.30.0 at `http://127.0.0.1:5001`. Born-digital conversion used `--no-ocr --images --no-image-ocr --async`. No remote OCR, formula/LLM enrichment, package/model download, or shared-service lifecycle change. Original Windows client hit `WinError 10048`; same task recovered through bounded slow curl status polling and result retrieval, not resubmitted. Existing client's image extraction localized embedded figures.
- Fidelity: full-document conversion, including target article sections 1–5, Open Peer Commentary, author response R1–R5, and shared references through Zhang & Simon (1985). Twenty-one detected figures retained in [images/](captures/cowan-2001/images/). No intentional article/commentary/reference truncation.
- Checks: original [conversion result](captures/cowan-2001/conversion.json) reports `status: success`, `errors: []`; [async task](captures/cowan-2001/async-task.json) reports success. Opening title/author/abstract and Introduction checked; middle commentary body checked; target conclusion, commentary heading, response headings, and final alphabetical references checked. All 21 Markdown image paths resolve; first figure visually inspected as Cowan's attention/memory diagram. Checks establish broad full-document coverage, not character-by-character proof against all 99 pages.
- Limits: automatic PDF layout conversion; multi-column reading order, line-break hyphenation, formulas, table structure, entities such as `&amp;`, and occasional accented-character mojibake can remain imperfect. Running headings can appear inside references. One formula's square brackets escaped in Markdown to prevent a false local hyperlink; formula text otherwise unchanged. No OCR of figure labels, no image descriptions, no page-image archive, and no guarantee every graphical element was detected. Use original PDF for precise layout/symbol checks. Some running headers in original copy say 2000; bibliographic publication year remains 2001.
- Rights: Cambridge University Press copyright; no suitable redistribution license established. Author hosting is not republication permission. **LOCAL-ONLY / NOT PUBLICATION-CLEARED**, including commentary text, figures, and raw conversion result.
- Temporary original PDF not retained; Markdown is deliverable. Original online PDF above remains available for layout checks. [Capture metadata](captures/cowan-2001/capture-metadata.json) records PDF SHA-256, page-object count, local service image digest, task identity, and timestamp.

## Inventory boundaries and blockers

- Captured: Miller 1956 and Cowan 2001. Reused full conversions: none existed in assigned folder. Blocked substantive sources: **none**.
- Old README mentioned an optional Cowan 2010 PMC follow-up and failed PubMed extraction. Neither had substantive source discussion in cards or retained full text. These were discovery/access attempts, not additional sources to acquire. No unread/bibliographic-only lead followed.
- Earlier PMC attempt failed anti-bot protection; earlier PubMed result was unusably short. Neither failure is treated as article capture or bypassed. Cowan 2001 author PDF supplies requested source directly.
- `.gitkeep`, both independent cards, and original research substance preserved. No commits, staging, publication, worktrees, tools/scripts, or sibling-folder edits.

## Comments

These captures replace prior notes-only retention policy, not independent notes themselves. Public access supports requested local offline retention; it does not establish redistribution rights. Keep captures and raw artifacts out of publication unless rights are separately cleared.
