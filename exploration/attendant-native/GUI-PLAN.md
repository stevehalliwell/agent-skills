# Shared Markdown GUI implementation plan

Status: step 5d implemented, tested and human-confirmed for cross-paragraph/formatting and within-cell preview feedback. Whole-app review found three unresolved rollout blockers; see [GUI-REVIEW.md](GUI-REVIEW.md). Copied-workspace evaluation only. Latest isolated viewer opened separately; previous viewer/unsent work untouched. Installed skill/launcher switch stays last.

## Result

Small Windows-first shared Markdown workspace: collection/file navigation, top document tabs, source editing/preview, clickable front-matter references and selected-text feedback that agents retrieve through ordinary Attendant queries.

Application/core is C99, compilable with CJIT and `zig cc`. Third-party native dependencies may contain C++. Use very recent Dear ImGui/Markdown stack; do not assume existing on-disk versions meet requirement. Ink player's cimgui/raylib pattern is candidate, not proof of CJIT GUI compatibility: its current checked GUI recipe uses Zig. Verify upstream release, Markdown component, matching cimgui generation and dependency ABI/build paths before adopting. Research: [GUI-DEPENDENCIES.md](GUI-DEPENDENCIES.md). A narrow C ABI bridge for unwrapped third-party C++ APIs is allowed; application/core remains C99. No automatic dependency installs/downloads. Preserve GUI-independent CLI build.

## Document and navigation contract

- Markdown files authoritative; SQLite disposable. Save files normally; preparation refreshes projection on next interaction. No DB-mediated save or collaborative merge service.
- Open workspace with narrow collection/file navigation and top document tabs. Reopen same canonical file by focusing existing tab. Opening reference does not replace original tab.
- Read front-matter reference declarations through existing collection/schema rules. Each `depends_on` item opens target document; missing targets show unresolved, not guessed paths.
- Edit source and show rendered Markdown preview. Preserve unrelated source/body bytes where core operates on metadata; human full-document edits save intended text. Dirty-tab save/discard/cancel is explicit. Reload clean buffers after external changes; never silently overwrite dirty work. No locking/CRDT project.
- One comment action works for source and freely selected rendered text, including selections across formatting, paragraphs, tables and code. Missing exact source mapping must not disable preview notes. Use verified source spans where available; otherwise retain rendered sections and highlighted local character ranges. Never label rendered offsets as source positions or reverse-search clipboard text. Freeze composer context without changing source-editor selection.
- Agent reveal/navigation must not steal current selection/focus. Selecting text alone grants no edit permission.

## Free preview selection contract

- Keep one **Comment on selection** action and approved modal border/labels. Drag either direction across rendered pieces; ordinary formatting boundaries are not errors. Existing document/selection capacity limits remain explicit; never silently truncate.
- Capture selected rendered text and all selected rendered sections. Each section includes its rendered text and zero-based/end-exclusive highlighted Unicode-code-point range within that section. Include surrounding unselected text only within affected sections so partial highlights remain clear.
- Map to original Markdown where provenance proves it. Source span may include intervening Markdown markers; do not require rendered bytes to equal raw source slice. Validate stored raw source span/text/hash normally. Keep rendered highlight context distinct from enclosing source span.
- If mapping cannot be proven, send context-only note with target path/hash, rendered sections and local highlight ranges. Explicitly report source anchor unavailable. Never invent a source position or use older source selection.
- Prefer simple Markdown context appended separately from verbatim human comment. Keep existing collection schema: context-only records use `range_start: 0`, `range_end: 0`, `selected_text: ""` as explicit absent-source-anchor state; real anchors remain nonempty and verified. New reader must distinguish this state, open target without selecting guessed source, and retain existing anchored comments unchanged. Ordinary CLI query/update stays usable; older GUI readers may reject new unanchored records rather than guess.
- Shared selection, composer and `--inspect` expose same latest preview context. Preserve it when focus moves to comment input; clear on document switch/reload and invalidate stale revisions. Keep rendered ranges and source ranges explicitly separate.
- Dirty source still needs **Save & Send**. Freeze context from current buffer; bind feedback to successfully saved hash. Failed save sends nothing; failed queue still reports saved-but-not-queued. Reading never resolves feedback and notes grant no edit authority.

Example fallback payload, shown as data rather than guessed source coordinates:

```json
{"sections":[{"text":"First section with bold text.","highlight_start":6,"highlight_end":13,"range_unit":"unicode-codepoint"}]}
```

Highlighted text is `section`. For mapped selection across `plain **bold** text`, retain actual rendered highlight plus corresponding raw Markdown span, including markers inside that span.

Acceptance: actual mouse drags, not only Select All, across plain/bold/italic/link boundaries, wrapped lines, paragraphs, table cells and code sections; reverse drags and partial first/last sections; UTF-8/supplementary ranges; mapped and context-only Send through native/legacy CLI reads and explicit resolution; retained inspection during modal focus; unchanged source caret/files on selection; existing comments and modal styling preserved.

## Agent reveal contract

Incoming reveal updates separate agent marker, never human caret/selection. Default: latest location hint only; no automatic tab open, activation, scrolling or foreground focus. Human **Go** opens/reuses target and scrolls to marker without replacing human selection. Fixed-height hint area must not move editor during interaction.

Control command:

```sh
attendant-gui --project DIR --reveal FILE --line 42
```

- Target already-running matching workspace. No auto-start, project mailbox or session injection. Native CLI stays GUI-independent.
- Full-source line is one-based. Sender reads saved UTF-8 source without preparation/writes and binds marker to exact saved hash. Receiver copies bounded request; never calls source API from message callback. Hint is transient; latest replaces previous.
- Display amber source-line outline/gutter mark separately from cursor, clipped to source widget. Render/scroll only when target buffer is clean and matches saved hash; stale/dirty hints remain readable but cannot receive guessed positions. Human Go never reloads dirty work or moves caret/selection, including original tab when opening another file. Display update waits for pointer/navigation release and pending human Go scroll.
- Scope: project descendants or registered/open external documents, ordinary local drive paths only. Preflight root/target ancestry outside message callback before source read and again before Go. Reparse, UNC, mapped-remote and extended-namespace reveal paths unsupported; ordinary editor/feedback unchanged. Not an atomic filesystem sandbox against concurrent reparse swaps. Full-source lines use LF/CRLF; bare CR unsupported for reveal.
- Versioned fixed-width local-desktop IPC, no source bytes/edit authority. Discovery: 4096 total windows, 16 matching candidates, 2000-ms budget checked before every class filter. Individual delivery 1000 ms; filesystem I/O no hard latency guarantee.
- No matching GUI, ambiguous instances, invalid/outside-workspace path, invalid line or failed delivery returns structured error/nonzero. Success acknowledges delivered hint, not persistence, foreground activation or source change.
- Validate both compilers: real external-process delivery, unchanged active document/selection/scroll/focus, separate visible marker, explicit Go, stale/dirty rejection, no source/projection writes, malformed payload and absent/ambiguous GUI.

## Feedback data contract

One managed collection per workspace: alias `_meta_comments`, default directory `records/_meta_comments`. Create/register on first Send only; browsing/querying must not create it. Existing incompatible alias/directory/schema is an explicit conflict, never overwritten. Compatible configured collection may use another directory; resolve through config.

Proposed v1 schema, finalized here for implementation:

```yaml
---
status: [pending, resolved]
target_path: ""
target_hash: ""
range_start: 0
range_end: 0
range_unit: [unicode-codepoint]
selected_text: ""
---
```

Use normal generated `id`, `name`, `created_at`, `desc`, `tags` metadata. Comment goes in Markdown body, not DB-only storage. `target_path` identifies selected saved file using core's absolute path convention; UI may display workspace-relative path. Built-in `source_path`/`source_hash` continue to identify/hash the comment Markdown itself; target fields must not collide with either. No automatic path rewriting after relocation.

- Source-anchor ranges: zero-based Unicode code-point offsets, end-exclusive, in complete saved Markdown including front matter. Context-only preview notes use explicit absent-anchor state described above; rendered highlight ranges live in context, not these source fields. CR/LF each count as code points. Not UTF-8 byte or UTF-16 code-unit offsets. UI adapters translate explicitly; tests include non-ASCII/supplementary characters and CRLF.
- `target_hash`: lowercase SHA-256 of exact saved UTF-8 file bytes. Verify selected text equals specified source range at submission. Validate integer range bounds and UTF-8 before publishing comment.
- Dirty document offers explicit **Save & Send** or Cancel. Failed save sends nothing. After successful save, create feedback against saved text/hash. Failed comment creation leaves source saved and reports failure; never imply comment was queued.
- Reading does not consume feedback. Agent queries pending records, considers/responds in task context, then explicitly resolves through ordinary `update`. Resolution is not automatic source deletion or blanket approval for unrelated work.
- Feedback can outlive source changes: stored selection remains readable; changed hash marks range stale. Do not silently apply old range to new content.
- Local collection is inbox; no direct Pi/session injection or separate messaging daemon. Reuse core source publication/validation where it fits; do not expose new CLI commands merely for GUI internals. Keep operational failures recoverable in GUI rather than using process-fatal CLI handling.

Example agent loop, against native executable:

```sh
native query -p "$workspace" \
  -s 'SELECT name,source_path,target_path,target_hash,range_start,range_end,range_unit,selected_text FROM _meta_comments WHERE status=:status ORDER BY created_at,name' \
  -P '{"status":"pending"}'
# Open returned comment Markdown to read body, inspect source, respond/act.
native update -p "$workspace" -c _meta_comments \
  -i '[{"name":"COMMENT_FILENAME_STEM","fields":{"status":"resolved"}}]'
```

## Pending reminder contract

Native `query` and `search` already prepare projection. On those commands only, when `_meta_comments` exists, append small sibling field without changing existing result fields:

```json
{
  "columns": ["name"], "rows": [], "returned": 0, "truncated": false,
  "meta_comments": {"pending": 3}
}
```

Also emit `3 _meta_comments are waiting for you.` on stderr when count > 0. Use singular grammar for 1. Include `{ "pending": 0 }` for configured compatible collection with no pending rows; no notice at zero. Omit field entirely when collection absent. Never dump comment bodies as sidecar.

Read count from same prepared SQLite read snapshot as command. Count visible valid projected pending rows, consistent with inbox query. Invalid/blocked records remain exposed through existing diagnostics. If alias exists but status declaration is not exactly `[pending, resolved]`, append `{ "pending": null, "error": "incompatible-schema" }` with concise warning rather than claiming zero. This check recognizes discovery/lifecycle only; step 2 validates complete managed schema before creating feedback.

No inbox creation, writes, FTS construction or extra preparation just to count. No new notices on doctor/validate/help/add-table/create/update/sync in first version. Existing non-writing commands retain behavior. Reading inbox itself does not clear count. Installed `.mjs` need not gain sidecar before native rollout; its ordinary query/update can operate on same collection.

## Git and boundaries

Git context deferred by user. No Git status/diff integration in current delivery; native CLI stays Git-independent. Revisit read-only boundary before adding it: owned-fixture probe confirmed `status` and `diff --no-ext-diff --no-textconv` still execute configured clean filters, which can write files. No auto-commit, discard or Git-based recovery.

Do not expand Unicode search or long-path/UNC qualification now: recorded corpus audit has no affected Unicode classification mismatch, and user deferred Windows edge coverage. No viewer-induced launcher rollout, nested-YAML expansion, full collaboration protocol or new task lifecycle policy.

## Ordered steps and observable acceptance

- [x] **0. Recent GUI dependency compatibility check.** Build/link/render portion passed in [C99 GUI spike](../../spikes/cimgui-richmd-c99/RESULTS.md): pinned ImGui 1.93.0/RichMd 1.0.0, regenerated cimgui, Zig-built DLL, strict C99 Zig host and CJIT host. Actual editing, selection/copy and C link callbacks passed. Strict original-input range export now passes 56 actual selection cases across both hosts, including real source/key hash collisions. Only proven contiguous spans accepted; broader mappings stay disabled with source-editor fallback.
- [x] **1. Native pending-feedback discovery.** Add query/search sidecar and concise notice; compatible/absent/zero/incompatible cases. Ordinary querying must never create collection. Both compilers and relative/absolute configs; unchanged source/hash checks, reading does not consume, resolving clears count, body remains intact. Existing read/CLI/search-mode/real-data suites remain green.
- [x] **2. Managed collection and feedback core.** Canonical schema/usage/template seeds; narrowly allow `_meta_comments` alias without relaxing normal record-name/path validation. First-Send bootstrap uses staged publication and rollback; no conflicting-file overwrite. Create one comment with generated identity, range/text/hash verification and Markdown body. Recoverable API for GUI, tested headlessly with both compilers. Failure tests cover save failure, schema/path conflict, invalid offsets, source change and comment publication after successful save. Ordinary `.mjs` and native query/update read/resolve same records.
- [x] **3. GUI build and read-only workspace.** Prove CJIT/Zig application compile/link with chosen dependency packaging; separate outputs, no CLI dependency on GUI/C++. Open owned fixture workspace, browse records and show document tabs/preview. Source reference clicks open/focus correct tabs; unresolved links visible. Check clean shutdown and bad-file errors without losing workspace.
- [x] **4. Editing and source-selection feedback.** Editable source, undo, save/discard/cancel, source selection and comment composer. Save & Send uses step 2; feedback pending badge/list opens source. End-to-end GUI send → CLI query → explicit resolution. Test UTF-8 offsets/CRLF and stale source markers. Preview-selection mapping follows as part of completing feedback surface, not guessed offsets.
- [x] **5. Complete preview navigation and feedback review.** Strict source-mapped preview selection, Markdown/front-matter links, multiple tabs, pending/resolved views and separate non-focusing agent marker implemented/tested. Explicit Go scrolls only; dirty/stale/unsupported paths cannot receive guessed locations. Git badges/diff deferred by user. No silent external-buffer overwrite.
- [x] **5a. CLI inspection of current viewer focus/selection.** Implemented and runtime-qualified; repeat human review after 5d. `attendant-gui --project DIR --inspect` returns JSON with active file, dirty state and latest human source/preview selection: text and exact full-source Unicode-code-point range when supported. Include unsaved selections; retain selection when focus moves to chat, clear on document switch/reload. Preview context without source mapping must retain rendered text/sections and local highlight ranges, never guessed source coordinates or older source selection; step 5d adds rendered inspection context. Empty selection is successful empty state. No focus, source, feedback or projection changes; absent/ambiguous/unavailable GUI follows reveal control errors. Selection alone grants no edit permission. User accepted contract during implementation.
- [x] **5b. Consistent Comment on selection action.** Implemented; human used shared action successfully with cross-piece preview selections in 5d. One **Comment on selection** button above both views uses shared latest human selection from source or preview. Preserve verified source anchors without making exact mapping a prerequisite for preview notes; step 5d removes exact-mapping gate. Latest preview context must never fall back to older source selection. Document switch/reload clears shared selection. Consistent per-view placement or right-click menu remains fallback only if unification proves unsuitable.
- [x] **5c. Clearly framed comment input.** Editable area has visible local border and **Comment:** heading directly above it; excerpt has separate **Selected source:** label. Empty composer screenshots and four-edge pixel checks pass both compilers. Existing input focus and Save & Send behavior remain intact. Human accepted modal styling.
- [x] **5d. Free preview selection and contextual feedback.** Implemented/tested under contract above: owned rendered sections/local highlights, optional enclosing source span, context-only feedback, inspection v2 and frozen composer. One shared button, modal labels/border and existing anchors preserved. Baseline failure reproduced; actual drags and mapped/context-only native/legacy loops pass both compilers. Human confirmed cross-paragraph/formatting and within-cell selections through two stored comments. Previous viewer/unsent text untouched.
- [ ] **5e. Resolve whole-app review blockers.** Protect drafts across terminal renderer failure, isolate editor undo state across tab lifetimes, and expose partial-workspace diagnostics. [GUI-REVIEW.md](GUI-REVIEW.md) holds evidence, options and acceptance. Recovery strategy remains undecided; review did not authorize automatic source overwrite or launcher rollout.
- [ ] **6. Guidance and integrated acceptance.** Explain sidecar/inbox workflow in CLI help/reference and skill guidance without switching launcher. Test normal queries, comment body inspection, resolution, malformed feedback and absent inbox. Use disposable real-project copies for GUI/CLI loop, source preservation, reference navigation and external edits. Record actual visual/interaction checks separately from headless tests.
- [ ] **7. Rollout gate, then launcher last.** Review correctness coverage, useful additional capabilities and representative performance against existing runner; measure GUI startup/idle/interaction footprint rather than infer. Only after gate, implement/discover/cache compiler build and switch skill launcher. No automatic installs. Final user acceptance remains required before treating GUI as release-ready.

## Checks

- Planning/research checkpoint committed as `26b7894` before implementation. Subsequent `spike/cimgui-richmd-c99` verifies dependency build/link/render through both compilers. Step 0 now includes conservative original-source range proof; broader mapping is not inferred from copy support. Spike documents compatibility patches and teardown/provenance checks; no further broad dependency research needed before step 2.
- Step 1 implemented in `read.h`: query/search sidecar and notice, same read snapshot, no inbox bootstrap or FTS work solely for count. Existing output unchanged when collection absent. Lifecycle-only schema recognition; complete managed-schema/bootstrap validation belongs to step 2.
- `meta-comments-check.mjs`: 11 cases × CJIT/Zig × relative/absolute paths = 44 passing cases. Covers absent/empty/pending/resolved/incompatible, inbox reads, external status edits, hidden/malformed records, non-notifying commands, legacy reuse, source preservation and cold query without FTS.
- Both native builds pass existing read, CLI and 13-case search-mode suites in both config forms; 66 copied-real-data workflow cases and 21 harness/existing-runner tests pass. Evidence: `build/review/meta-comments-results.json`, `build/meta-comments-{results,read-results,cli-results,mode-results}.jsonl`, `build/meta-comments-real-data-results.json`.
- Corrected managed selected-file hash field to `target_hash` during fixture validation: built-in `source_hash` is reserved for comment Markdown, just as built-in `source_path` is. No existing source schema or behavior changed.
- Feedback discovery and GUI spike checkpointed as `f9de9c7` and `340098c`. Active implementation branch: `feat/shared-markdown-gui`. Launcher/release gate unchanged.
- Step 2 implemented through `native-api.h`/`native-api.c`: recoverable single-thread request boundary; source read/optimistic atomic save; full managed-schema validation; Unicode code-point range/text/hash checks; first-Send seeds/comment with config published last. Missing config is created on first Send only. Existing compatible alternate directory reused; no normal filename validation relaxed. No unrelated source correction or projection preparation.
- `api-check.mjs`: 68 cases across CJIT/Zig and relative/absolute configs. Includes stage/seed/comment rollback, external target/config changes after staging, failed save, stale hash, empty comment, supplementary Unicode/CRLF, schema/directory conflicts, ordinary native/`.mjs` read/resolve, body preservation, and saved-but-not-queued behavior. Evidence: `build/api-check-results.json`; owned fixtures: `build/api-check/run-gAGkGQ`. Existing read/CLI/source suites pass both compilers. Independent core review found rollback path ownership could change after publication; cleanup now compares expected bytes under a non-write/delete-shared handle and deletes through that handle. Changed files survive, appear in `written`, and force explicit conflict on retry; focused regression passes.
- Step 3: integrated C99 GUI and source-only workspace discovery implemented under `gui/`. CJIT/Zig production and test hosts compile/link against same verified DLL. Actual D3D previews, Unicode/space project paths, typed-reference button input, tab reuse, visible unresolved targets and unchanged source/no projection passed. Both compilers pass 42 always-on headless document-state checks; two build-provenance guard tests pass. Evidence: `build/gui-check/run-VbtQvZ`, `build/gui-check-results.json`, retained `gui/evidence/windows-x64.json`; details in `gui/README.md`.
- Step 4: source editor/undo/redo, save/reload/close Save/Discard/Cancel, source-selection composer, explicit Save & Send, saved-but-not-queued, pending/resolved views and saved-target selection implemented. `attendant_read_comment()` validates managed anchor without consumption; fresh target hash/text/range must match before selection, and dirty targets stay untouched.
- Independent workspace review found three issues, now fixed/tested: unreadable record blocked whole GUI; post-read stat could accept newer metadata for old bytes; failed Send lost surviving-path evidence. Browse policy now records per-file diagnostics while CLI stays strict; read/save return associated metadata; model-owned failed/surviving paths survive refresh.
- Source-editing checkpoint: 10 fresh real windows/D3D runs, both compilers, real edit/undo/close-Cancel/Send/target-button input, 71-code-point UTF-8/supplementary/CRLF source selection, native and `.mjs` read/resolve/body preservation, failed-queue UI and no other source/config change. Both compilers pass 81 always-on model checks; two provenance guards pass. Evidence: `build/gui-check/run-JNHl9H`, `gui/evidence/windows-x64.json`; 68 core API cases and existing read/CLI/source suites remain green.
- Preview checkpoint: 60 real stack windows, including 54 strict range cases, and 12 integrated GUI windows across CJIT/Zig; 94 always-on model checks per compiler. Exact body feedback `[35,62)` includes BOM/CRLF/supplementary full-source accounting, explicit Save & Send, unchanged editor selection, native/`.mjs` reads and explicit resolution. Maintained deterministic RichMd patch rejects unknown/missing edits; three stack and two GUI provenance guards pass. Independent static review found no actionable mapping/ownership issues; reviewer did not run checks. Evidence: `build/gui-check/run-rnktnp`, `gui/evidence/windows-x64.json`, stack `evidence/source-selection-windows-x64.json`.
- Git integration deferred after user chose to avoid current filter-execution trade-off. Probe: `build/git-readonly-probe-txlWbc`, no live project touched.
- Agent reveal checkpoint: both compilers pass 134 always-on model checks and 26 real GUI windows. Cross-compiler external-process hints preserve document, actual caret/selection/active ID, geometry, scroll and Win32 focus; explicit Go preserves caret/selection across same/new tab. Source-only pixel bounds, dirty/stale rejection, absent/ambiguous instances, malformed payload/arguments, pre-send junction rejection and post-delivery junction replacement pass. Qualification caught transient clip-rect spill and stale rendered-tab activation overwrite; retained inner clip and forced-target guard fix both. Review findings prompted reparse preflight outside callback and deadline/visit checks before class filtering. Follow-up static review found no remaining actionable production issue within inspected scope; reviewer ran no checks. Three GUI provenance guards pass; compiler version probes now succeed for CJIT/Zig. Evidence: `build/gui-check/run-aYn6uC`, `gui/evidence/windows-x64.json`.
- Inspection checkpoint: `--inspect` reports active live buffer and latest human source/preview selection without focus/files/projection/feedback changes. Both compilers pass 168 always-on model checks and 32 real GUI windows. Cross-process clean/dirty exact text/code-point ranges, repeatability, caret/scroll/focus preservation and absent/ambiguous rejection pass. Actual preview keyboard-menu Select All and code-child interactions replace older source ownership; widget contexts report unsupported, never stale source fallback. Renderer ownership serial replaces mouse heuristics without changing C selection POD layout. Stack rerun: 62 windows/56 selection cases; three stack and three GUI guards pass. Independent follow-up static review found no remaining actionable issue; reviewer ran no tests. Evidence: `build/gui-check/run-BenL1m`, `gui/evidence/windows-x64.json`, stack `evidence/source-selection-windows-x64.json`.
- Unified comment checkpoint: one action above source/preview freezes latest exact shared range; empty/unsupported context cannot fall back to old source caret. Source and preview Save & Send loops pass. Actual unsupported markup/code-child button clicks open no composer; exact preview opens correct range, and inspection stays unchanged when comment input owns focus. Two successive 32-window suites pass, with 168 model checks/compiler and three GUI guards. Independent static review found no actionable issue; no reviewer tests. Evidence: `build/gui-check/run-6PM9aB`, `gui/evidence/windows-x64.json`.
- Comment-input checkpoint: local border style, separate source/comment labels and initial input focus pass both compilers. 168 model checks/compiler, 32 real GUI windows and three GUI guards pass. Empty-input screenshots visually inspected; pixel checks verify all four border edges. Independent static review found no actionable issue; reviewer ran no tests. Evidence: `build/gui-check/run-YV43y6`, `gui/evidence/windows-x64.json`.
- Human preview checkpoint: latest isolated viewer PID 28556, owned `build/manual-review/preview-chunks-NtNqUE/review.md`. Two pending comments read back without mutation: `comment-01a18ab5-4750-70c6-9849-ea7c1e5d2a71`, source `[46,250)`, two rendered paragraphs across bold/italic; `comment-01a18ab5-a404-7296-9b3f-84708eae74b0`, source `[318,331)`, rendered `Bold inside` within table cell, local highlight `[0,11)`. Both retain separate human body/context and verified target hash. User confirmed result. This does not qualify remaining dirty-target/resolution or wider UI cases.
- Free-preview checkpoint: 54 integrated real windows, 193 always-on model checks/compiler, 92 embedded API cases, 62 legacy stack windows/56 strict cases, three GUI and three stack guards pass. Actual forward/reverse/partial drags, later paragraphs/italic/link/code/table Unicode, wrapped text and edge auto-scroll pass. Dirty context-only Save & Send binds frozen context to saved hash; native/legacy reads and explicit resolution preserve source/human body. Inspection retains selected rendered text and section JSON during modal focus. Both empty-input screenshots visually inspected; border remains visible.
- Review fixed export-failure ownership and unsafe normal teardown after render-phase exceptions. Actual 2.2-MB quote-heavy paragraph reports preview/unavailable, not older source; nested ID/child/style fault terminates without normal frame teardown or source writes. Terminal failure still loses drafts: whole-app blocker, not universal recovery. Static follow-up found unreachable unavailable label; conditional corrected. Renderer/context framing remains bounded. Evidence: `build/gui-check/run-aYxU5Y`, `gui/evidence/windows-x64.json`, `build/api-check/run-SzSjM1`, stack `evidence/source-selection-windows-x64.json`.
- Independent whole-app static review found draft-loss, reused-editor undo identity and hidden partial-discovery diagnostics. Parent traced cited paths; fixes remain pending. See [GUI-REVIEW.md](GUI-REVIEW.md). No reviewer runtime tests.
- Copied-real-project integrated loop, GUI footprint evidence and launcher change remain incomplete.

## Comments

- Original GUI markup action remained disabled; retained legacy raw host reproduced actual forward plain/bold drag as unsupported. Evidence: `build/free-preview-before.json`. New selectable API avoids exact-byte gate without weakening legacy source proof.
- One early extended-suite reveal check failed `view.active_id == widget && view.caret >= 0`; cause not isolated. Later full suites pass. Inspection control subprocesses now hide sender console; no claim this proves physical focus routing.
- Manual viewer PID 21884 remained running. Protected executable/DLL hashes match original production manifest; isolated build variant prevents replacement. Reading/inspection did not consume user feedback.
- One fixed-coordinate preview-driver run did not complete selection/composer path and hit dirty-buffer shutdown guard. Cause not isolated; direct replay and two later full suites passed. Owned mutable drivers now stop at frame bound and reject incomplete work through final checks instead of hanging. Remaining interactive review is still required.
