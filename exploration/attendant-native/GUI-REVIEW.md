# Whole-app review

Verdict: copied-workspace evaluation only. Not ready for everyday authoritative-workspace editing or installed-launcher rollout.

Preview-selection slice implemented and tested in `f279dec` and `6317608`. User deferred renderer recovery until a real case needs solving, requested file-path undo isolation and accepted simple workspace-warning/CLI guidance. Review follow-up implements those bounded changes; no recovery architecture or launcher rollout.

## Review disposition

### Deferred: Renderer failure loses drafts

- `apps/gui/main.c` abandons broken render frame, warns user and enters cleanup.
- `gui_workspace_destroy()` frees document buffers. Host cleanup also frees composer draft.
- Abandoning unsafe ImGui frame is correct. Losing still-valid C-owned edits in unrelated tabs is separate problem.
- Current nested-failure test proves controlled termination and unchanged saved source, not draft recovery.

Decision: accept known unsaved-work loss for now; defer recovery until an ordinary failure supplies a real case. Injected exception is qualification evidence, not an observed ordinary trigger. No automatic source save, recovery export or renderer restart added. Recovery is no longer an active prerequisite for this evaluation.

### Fixed: Tab reuse can inherit another document's undo history

- ImGui owns undo; application previously identified source editor by slot/generation only. Closed/reused tabs can reset generation to zero.
- Pinned `imgui_widgets.cpp:4954-4978` preserves undo when widget ID and current bytes match.
- Owned real-window reproduction: edit A to `undo shared`, save/close, open B containing identical bytes in same slot, then Undo. Before fix, B changed to `undo share` using A's character history.
- Source editor ID now includes full file-path bytes through `igImHashData()` plus existing slot/generation. Raw-byte hashing avoids ImGui's special `###` string-ID handling. Ordinary edits/save keep identity; reload still advances generation.
- No custom undo engine or persistent per-file undo stacks. Fresh undo lifetime on every reopening of same path is not promised; file-path ownership is agreed scope.

Both compilers pass same-slot identical-content isolation, literal `###` paths, within-file Undo/Redo and return to A. B's saved bytes remain unchanged.

### Implemented: Workspace issue warning and CLI guidance

- `src/core/core.c` records unreadable/omitted collections. `attendant_read_workspace()` returns successful healthy navigation plus diagnostic count.
- Workspace header now shows count and warns that collections/records may be missing or invalid. Warning reads current navigation snapshot and remains until refreshed diagnostics clear.
- Header directs user to native CLI `doctor -p <workspace>` for read-only details, then manual correction and **Refresh files**. No automatic repairs, source correction or projection preparation.
- Count includes missing configuration during plain-folder browsing; Markdown remains usable. Detailed paths/messages stay in CLI, not new GUI diagnostics API. User accepted this smaller scope.
- Partial discovery itself does not write source. Existing expected-hash save checks remain unchanged; this is not a universal no-corruption guarantee.

Both compilers pass configured collection pointing at regular file: two issues visible, healthy document opens, source/config inventory unchanged. Warning screenshots inspected. Access-denied ACL behavior was not newly qualified.

## Selection result

- Free forward/reverse/partial preview selection across formatting, paragraphs, links, wrapped lines, tables and code.
- Rendered affected sections retain local Unicode-code-point highlights. Optional verified source span may include intervening Markdown syntax.
- Context-only notes use explicit absent anchor: zero bounds and empty `selected_text`; saved target hash remains verified.
- One action freezes owned composer context. Inspector retains latest selection during comment focus. No older source fallback.
- Dirty context-only Save & Send saves intended buffer before binding feedback to saved hash. Human body precedes separate context metadata.
- Export-capacity failure preserves preview ownership. Render-phase failure remains terminal limitation above.

## Validation

Preview checkpoint, before this follow-up:

- CJIT/Zig production and test GUI builds in isolated `build/gui-preview/`.
- 193 always-on model checks per compiler.
- 54 integrated real window/D3D runs: 26 selection scenarios, two controlled render failures and existing editor/reveal/inspection workflows.
- Actual later-paragraph/code/table Unicode expectations, wrapped selection, edge auto-scroll, partial boundaries and forward/reverse equivalence.
- Mapped/context-only feedback, dirty context Save & Send, native/legacy reads and explicit resolution. Oversized 2.2-MB quote-heavy paragraph cannot expose older source selection.
- 92 embedded API cases, including context-only bootstrap/hash/rollback/body/resolution.
- 62 legacy stack windows / 56 strict-selection cases. Three stack and three GUI provenance guards.
- Both empty composer screenshots inspected; labels, collapsed context viewer, editable border and initial input focus retained.

Preview evidence: `build/gui-check/run-aYxU5Y`, `build/api-check/run-SzSjM1`, stack `evidence/source-selection-windows-x64.json`.

Review follow-up: CJIT/Zig production/test builds isolated in `build/gui-review/`; 193 always-on model checks per compiler, 58 real window/D3D runs and three provenance guards pass. Includes new undo and partial-discovery cases plus existing selection/reveal/feedback loops. Before-fix undo failure retained in `build/review-before.log`; final run `build/gui-check/run-97NrzE`, retained `tests/gui/evidence/windows-x64.json`. Native API/legacy stack suites were not rerun; their source did not change.

Independent reviewers traced selected-path fixes and whole-app ownership, save/close, navigation, API, IPC, renderer teardown, source/projection and build paths. Reviews were static: no independent builds/tests or GUI interaction. Parent checked cited ownership/undo/diagnostic paths. No new native CLI defect established in inspected paths; not whole-app correctness certification.

## Layout cleanup validation

- Runtime: `apps/cli/`, `apps/gui/`, `src/`. Tests/fixtures, tools and SQLite experiments separated. Review order in [README.md](README.md#review-map).
- Separate mains; core has no entry point/exclusion macro. GUI API no longer compiles CLI handlers. Four shared directory/record helpers moved into core; test input/state/completion/fault logic moved out of runtime source.
- Source comparison proves unchanged CLI main body, four shared helper bodies, three GUI driver bodies, two feedback-fault bodies and production GUI source after excluding test blocks. Evidence: `build/layout-source-proof.json`.
- CJIT/Zig CLI, API probe and GUI production/test builds pass; GUI isolated in `build/gui-layout/`.
- Existing read/projection, CLI, source, preparation, incremental, feedback-discovery and search-mode suites pass both compilers. Embedded API: 92 cases. GUI: 193 model checks/compiler, 58 real windows. Harness/layout: 18 tests; four provenance guards include extracted `.inc`/fault headers.
- Current-policy `full` SQLite comparison builds. Historical eager-baseline experiment/tuning builds fail against newer shared YAML/rollback ownership fields; documented under `experiments/sqlite/README.md`, not repaired or used for new timing claims.
- Final evidence: `build/layout-*.log`, `build/api-check/run-XLCDUj`, `build/gui-check/run-Emuyvo`, `tests/gui/evidence/windows-x64.json`. Legacy dependency-stack suite and copied-real-data workflows were not rerun. Installed launcher and protected viewer binaries unchanged.

## Remaining gates

1. Agreed review follow-up complete. Renderer draft-loss remains explicitly accepted/deferred, not recovered.
2. Human confirmed two preview comments in separately opened latest build: cross-paragraph bold/italic selection and plain/bold selection within table cell. Continue remaining manual dirty-target/resolution cases; previous viewer was not silently upgraded.
3. Complete copied-real-project GUI/CLI loop and agent guidance.
4. Measure startup, idle, interaction and distribution footprint. No new performance claim from smoke tests.
5. Keep wider glyph/Unicode, long-path, DPI, device-loss, long-run and power-loss limits explicit.
6. Keep Git deferred. Switch installed skill/launcher only after readiness review and rollout approval.

## Comments

- Earlier checkpoint verified manual viewer PID 21884 remained running. This follow-up used separate binaries/owned copies; original `build/gui/` and manual-preview `build/gui-preview/` production binary/DLL hashes still match retained manifests. No close, restart or source edit targeted manual session. Unsent text was not inspected or modified.
- One final-layout suite got `gui-not-running` during repeated inspection after receiver readiness. Same final-source full rerun passed; cause not isolated. Failure retained in `build/layout-gui-check-first-final.log`; final pass in `build/layout-gui-check.log`.
- One early follow-up suite failed existing preview Save & Send exercise. Isolated rerun and later full suites passed; cause not isolated. Retained `build/review-after.log` failure and `build/review-final.log` final pass. No runtime-test claim attributed to static reviewers.
