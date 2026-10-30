# Whole-app review

Verdict: copied-workspace evaluation only. Not ready for everyday authoritative-workspace editing or installed-launcher rollout.

Preview-selection slice implemented and tested in `f279dec` and `6317608`. Whole-app review found three unresolved blockers. Review did not authorize or implement recovery architecture, editor-lifetime changes or diagnostic UI.

## Blockers

### P1: Renderer failure loses drafts

- `gui/main.c:875-881` abandons broken render frame, warns user and enters cleanup.
- `gui/main.c:931` and `gui/workspace.c:93-98` free all document buffers. Composer draft also freed during cleanup.
- Abandoning unsafe ImGui frame is correct. Losing still-valid C-owned edits in unrelated tabs is separate problem.
- Current nested-failure test proves controlled termination and unchanged saved source, not draft recovery.

Required result: retain exact dirty-tab and unsent-comment bytes without automatically overwriting authoritative Markdown. Options: restart renderer while retaining C model, or bounded recovery export outside broken frame. Recovery location, ownership, retention and failure reporting need explicit design.

Acceptance: multiple dirty tabs plus unsent comment; inject nested render failure; recover exact bytes; saved source unchanged. Report export/restart failures, never claim successful recovery without evidence.

### P2: Tab reuse can inherit another document's undo history

- `gui/workspace.c:122` zeroes reused tab; `:158` zeroes closed tab. Generation can return to zero.
- `gui/main.c:360` derives editor ID from slot/generation. Tab ID at `:562` also uses slot.
- Pinned `imgui_widgets.cpp:4954-4978` preserves undo state when widget ID and current bytes match.
- Static reproduction: edit A from X to Y, save/close A, open B containing Y into same slot, activate editor, Undo. Matching ID/bytes can recycle A's history into B. Saving B then passes its ordinary expected-hash check.

Recommended fix: fresh workspace-wide editor-instance token per opening/reload. Keep token through ordinary edits/save. Include token in widget ID. Do not confuse preview text revision with editor lifetime.

Acceptance: real GUI close/reuse, identical-content documents, Undo/Redo, reopening same path and reload. Verify B never receives A's edits. Static path confirmed; runtime reproduction not run during review.

### P2: Partial workspace discovery is invisible

- `projection.c:635` records unreadable collection and returns without its documents.
- `native-api.c:148-149` returns successful workspace plus diagnostic count.
- GUI copies count but `gui/main.c:87-124` navigation does not display diagnostics. Workspace shell likewise lacks warning.
- Per-record `[invalid]` cannot explain collection omitted before any document row exists.

Required result: visible partial-workspace warning with affected paths/messages while healthy collections remain usable. Expose structured diagnostics if count alone cannot explain missing content. Distinguish unconfigured-folder browsing from broken configured collection.

Acceptance: inaccessible configured collection plus healthy collection; retain healthy navigation and visibly explain omitted collection. No source correction or projection preparation from browsing.

## Selection result

- Free forward/reverse/partial preview selection across formatting, paragraphs, links, wrapped lines, tables and code.
- Rendered affected sections retain local Unicode-code-point highlights. Optional verified source span may include intervening Markdown syntax.
- Context-only notes use explicit absent anchor: zero bounds and empty `selected_text`; saved target hash remains verified.
- One action freezes owned composer context. Inspector retains latest selection during comment focus. No older source fallback.
- Dirty context-only Save & Send saves intended buffer before binding feedback to saved hash. Human body precedes separate context metadata.
- Export-capacity failure preserves preview ownership. Render-phase failure remains terminal limitation above.

## Validation

Parent ran final source state through:

- CJIT/Zig production and test GUI builds in isolated `build/gui-preview/`.
- 193 always-on model checks per compiler.
- 54 integrated real window/D3D runs: 26 selection scenarios, two controlled render failures and existing editor/reveal/inspection workflows.
- Actual later-paragraph/code/table Unicode expectations, wrapped selection, edge auto-scroll, partial boundaries and forward/reverse equivalence.
- Mapped/context-only feedback, dirty context Save & Send, native/legacy reads and explicit resolution. Oversized 2.2-MB quote-heavy paragraph cannot expose older source selection.
- 92 embedded API cases, including context-only bootstrap/hash/rollback/body/resolution.
- 62 legacy stack windows / 56 strict-selection cases. Three stack and three GUI provenance guards.
- Both empty composer screenshots inspected; labels, collapsed context viewer, editable border and initial input focus retained.

Evidence: `build/gui-check/run-aYxU5Y`, `gui/evidence/windows-x64.json`, `build/api-check/run-SzSjM1`, stack `evidence/source-selection-windows-x64.json`.

Independent reviewers traced selected-path fixes and whole-app ownership, save/close, navigation, API, IPC, renderer teardown, source/projection and build paths. Reviews were static: no independent builds/tests or GUI interaction. Parent checked cited ownership/undo/diagnostic paths. No new native CLI defect established in inspected paths; not whole-app correctness certification.

## Remaining gates

1. Resolve blockers above; recovery strategy remains undecided.
2. Human confirmed two preview comments in separately opened latest build: cross-paragraph bold/italic selection and plain/bold selection within table cell. Continue remaining manual dirty-target/resolution cases; previous viewer was not silently upgraded.
3. Complete copied-real-project GUI/CLI loop and agent guidance.
4. Measure startup, idle, interaction and distribution footprint. No new performance claim from smoke tests.
5. Keep wider glyph/Unicode, long-path, DPI, device-loss, long-run and power-loss limits explicit.
6. Keep Git deferred. Switch installed skill/launcher only after readiness review and rollout approval.

## Comments

Protected manual viewer PID 21884 remained running. Original production executable/DLL hashes still match their retained manifest. Implementation/testing used separate binaries and owned copies; no close, restart or source edit targeted manual session. Unsent text was not inspected or modified.
