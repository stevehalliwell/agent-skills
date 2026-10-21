# ZPD and scaffolding: local research references

Status: **three discussed sources have actual full local Markdown captures**: two newly acquired, one existing conversion verified and reused. Independent source cards, notes, existing PDF, figures and `.gitkeep` preserved. No framework/workflow changes, staging, commits or publication.

Acquisition and verification date: **2026-10-21**. Purpose: local offline LLM reference. Captures reproduce source text; source cards below remain independent notes, not substitutes.

## Source-to-local-capture mapping

### Chaiklin (2003) — captured

- Source: Seth Chaiklin, *The Zone of Proximal Development in Vygotsky's Analysis of Learning and Instruction*, chapter in *Vygotsky's Educational Theory in Cultural Context*, Cambridge University Press, pp. 39–64.
- **[Actual local manuscript Markdown](captures/chaiklin-2003/chaiklin-2003.md)**.
- Separate [source card and independent notes](chaiklin-2003-zpd-source-notes.md).
- Reading copy: https://blogs.ubc.ca/vygotsky/files/2013/11/chaiklin.zpd_.pdf
- Method: curl public PDF download to temporary scratch; shared local Docling Serve v1.30.0; born-digital text with OCR disabled, no image export, no remote enrichment. Sync request returned HTTP 504 after queue delay; recovered **same** successful task from service logs via curl, no resubmission or service change.
- Coverage: complete available manuscript from opening question and conceptual critique through imitation/assessment, concluding comment, author note, full references and endnote. Not verified identical to final published chapter. No substantive figure referenced; no images exported. Bibliography spacing, italics and reading layout imperfect.
- Rights: no redistribution license established. Copyright remains with respective rights holders. **Local-only; not publication-cleared.** University hosting is not permission to redistribute.
- Raw result: [conversion.json](captures/chaiklin-2003/conversion.json), `success`, errors `[]`. Temporary PDF removed; no redundant binary retained.

### Van de Pol, Volman and Beishuizen (2010) — captured

- Source: *Scaffolding in Teacher–Student Interaction: A Decade of Research*, *Educational Psychology Review* 22, 271–296. DOI: https://doi.org/10.1007/s10648-010-9127-6
- **[Actual full local article Markdown](captures/van-de-pol-2010/van-de-pol-2010.md)**.
- Separate [source card and independent notes](van-de-pol-2010-scaffolding-review-source-notes.md).
- Original publisher PDF: https://link.springer.com/content/pdf/10.1007/s10648-010-9127-6.pdf
- Method: built-in static HTML fetch began at “Profile of the research” and lacked beginning. Used complete public publisher PDF instead: curl temporary download, local Docling, OCR disabled, table structure enabled, figure export without image OCR. One async submission, bounded 15-second curl polling, `success`, errors `[]`.
- Coverage: title, abstract, introduction, method, review results, effectiveness and measurement sections, conclusions/discussion, appendices, complete reference list ending with Yelland and Masters. Checked beginning/middle/end, not file size alone. Figure 1 [local image](captures/van-de-pol-2010/images/van-de-pol-2010-004.png) visually inspected. All 24 exported image links resolve; other 23 are small publisher decorations. Complex appendix tables retain text but flattened/repeated headings and imperfect alignment; consult online PDF for exact tables. No image OCR or added diagram interpretation.
- Rights: © The Author(s) 2010. PDF explicitly states **Creative Commons Attribution Noncommercial License**. Exact version not established from extracted notice; do not infer one. Attribution retained. **Local-only retention; not publication-cleared** for any planned redistribution without checking applicable terms.
- Raw result: [conversion.json](captures/van-de-pol-2010/conversion.json); [submission](captures/van-de-pol-2010/async-task.json); [final status](captures/van-de-pol-2010/status.json). Temporary PDF removed.

### Van de Pol, Volman, Oort and Beishuizen (2015) — reused

- Source: *The effects of scaffolding in the classroom: support contingency and student independent working time in relation to student achievement, task effort and appreciation of support*, *Instructional Science* 43, 615–641. DOI: https://doi.org/10.1007/s11251-015-9351-z
- **[Existing full local article Markdown](van-de-pol-2015-docling/van-de-pol-2015-effects-of-scaffolding.md)**, verified and reused without duplication or overwriting.
- Separate [source card and independent notes](van-de-pol-2015-source-notes.md); existing [local publisher PDF](van-de-pol-et-al-2015-effects-of-scaffolding.pdf) preserved.
- Original method, recorded in existing conversion header: publisher PDF via curl; local Docling v1.30.0, OCR disabled, table extraction enabled, figures without image OCR. Initial async client socket exhaustion recovered with curl from same task. Prior record reports success and no errors; original service payload was not retained. No fresh conversion claimed.
- Verification: read opening abstract/introduction, methods and participant details, analyses/results including significant and null interactions, discussion, limitations, conclusion, license statement and references through Wood et al. (1978). Four figure paths resolve: `images/van-de-pol-2015-018.png`, `-019.png`, `-022.png`, `-025.png` under conversion folder. Captions may be separated from figures; Table 2 is badly flattened; grouped headings, statistical symbols and affiliations have extraction defects. Full textual coverage does not mean facsimile-quality tables or symbols. Use retained PDF for exact numerical claims.
- Rights: © The Author(s) 2015; [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/), present in article's Open Access notice. Attribution and conversion changes retained. No publication performed.

## Acquisition records and checks

- [Acquisition metadata](captures/acquisition.json): original URLs, PDF signatures and SHA-256 hashes for new downloads, date, result locations and resolved Docling image digest.
- Shared endpoint `http://127.0.0.1:5001` healthy; existing image `quay.io/docling-project/docling-serve-cpu:v1.30.0`, digest recorded in acquisition metadata. Shared service never stopped, restarted or reconfigured. No dependency/model downloads, remote OCR, LLM enrichment, new scripts or spawned agents.
- New raw Docling payloads retained alongside readable Markdown, including status/errors/confidence. Markdown provenance headers identify source, method, rights and limitations.
- Verified source identity and substantive beginning/middle/end passages; no challenge/error/abstract-only capture accepted. Checked every local Markdown/image link in three captures; no missing targets. `.gitkeep` preserved.
- No Crawl4AI container needed in this run: public PDFs supplied complete text after incomplete HTML extraction. Existing captures not overwritten. Raw publisher HTML not retained; PDF-to-Markdown result is retained.

## Bounded inventory and real access limits

Three sources above are substantive sources actually inspected in original source cards. No blockers remain for those three local-text captures.

Wood, Bruner and Ross, *The Role of Tutoring in Problem Solving* (1976), is **not directly read/captured**: original README records publisher HTTP 403 and one isolated Crawl4AI HTTP 500 fallback. Keep that historical blocker visible; this run did not retry or replace it with a note. Direct Vygotsky writings, other bibliographic leads, and books were not acquired or chased. Open University PDF candidate previously returned HTTP 403 and is not a captured source. These are not counted among three completed captures.

## Evidence boundary

Sources concern development, tutoring and classroom learning. They provide concepts and counterchecks, not evidence of this example game's player experience, developmental status, mastery, mandatory route or successful transfer.
