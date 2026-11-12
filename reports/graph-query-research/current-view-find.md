# Current-view Find

## Confirmed behavior

- Ctrl+F opens/focuses shared Find UI for current unified view.
- Search current view's shown items, including off-screen items. Exclude items hidden by filters/scope; do not search other projections or all loaded records.
- Search displayed names/labels and available source references. No whole-source Markdown/prose search or inferred associations.
- Typing selects and frames first match immediately. User chose live navigation rather than highlight-only preview.
- Enter selects/frames next match; Shift+Enter selects/frames previous match. Result click selects/frames that match. Escape closes Find, retaining last selection/camera.
- Reuse readable Focus selection policy. Framing changes camera only; no geometry, filter, query scope or analytical input change. Do not Run.
- Empty term or no matches: no new selection/camera change; show clear count/empty feedback rather than revealing hidden data.
- Keep list-only search, canvas filters, current-view Find and analytical scope distinct in labels/help.

## Technical constraints

- Ctrl+F takes precedence over bare F Fit, including when a text field owns keyboard input. Typing F must not Fit.
- Use existing typed selection/picking/visibility contracts for ordinary graphs, Grid headers/pairs and separate similarity entries. Do not use ordinary graph topology for similarity.
- Preserve pending edits and existing human-input guards. Opening Find must not silently commit or discard rank/draft edits.
- Preserve independent pane state on ordinary projection switches. Live navigation occurs from explicit Find input, not automatic projection entry.
- Search text/navigation state is transient; existing restart persistence remains restricted to fold preferences.
- No source writes, automatic queries, selection-history implementation, performance-budget widening or viewer replacement.

## Acceptance

- Ctrl+F opens Find without triggering whole-view Fit.
- Example: type walker; first shown match becomes selected/framed as term changes. Enter/Shift+Enter cycles matches at readable scale.
- Off-screen match remains eligible. Filtered match does not appear or become visible.
- No matches and empty input do not clear current selection or frame unrelated content.
- Retained query results/inputs, filters, geometry and last-good state unchanged by search; selection/source/camera may change through live Find as chosen.
- Verify native typed targets across all nine unified views and Grid/similarity modes; report unsupported cases rather than invent links or scopes.

## Implementation

- `native/gd_explorer_find.h`: shared transient bar above sidebar folds. Per-view text/open state; no restart persistence or automatic navigation on view entry.
- Literal ASCII-case-insensitive substring search; other UTF-8 bytes exact. Search rendered label, source display name and available source reference. Query renderer and Find share ghost-variant label accessor.
- Scan current prepared node/entry order. Include off-screen items; exclude filtered/unplaced items through existing canvas visibility rules. Grid indexes named headers, not numeric cells or invented pair labels. Existing pair selection survives empty/no-match input.
- Bound text to128 UTF-8 bytes and matches to `GD_MAX_RECORDS`. Show first6 result rows with source references; buttons and Enter/Shift+Enter cycle all matches. Matcher uses linear prefix scan; no query application, per-search allocation or cached result indices.
- Use direct typed selection plus existing readable Focus. Do not reuse IPC Effect navigation: that action may rebuild contributor context. Find keeps retained query/scene and raw geometry unchanged.
- Suspend selection-driven distance-layout refocus for Find-selected node on same layout. Keep setting enabled and existing animation/solver behavior unchanged. Canvas click, another native selection or layout replacement resumes normal follow behavior. Escape retains last selection/camera and does not resume layout refocus.
- Pending rank edit blocks opening/navigation with explanation. Active drag/dialog blocks Ctrl+F. Other pending analytical inputs and draft/Undo state stay unchanged. Close remains available when navigation is blocked.

## Verification

- Warning-free CJIT CLI, graph viewer and explorer adapter builds. Automated checks opened no viewer and posted no OS input.
- Focused `--find-helpers GAME-DESIGN-DIRECTORY` adapter mode runs source-read-only Find checks without unrelated layout-motion diagnostics. JSON stdout; nonzero status on failure. Existing `--ipc-helpers` and90-second gate unchanged.
- Crate214/Mario230 native Find checks pass; both example source hashes unchanged. Cover nine views, typed Actor/ghost variants, Grid headers and pair retention, retained similarity, off-screen/filtered/unplaced targets, empty/no matches, wrapping, pending query/rank inputs, draft history and allocation-free navigation.
- Headless ImGui frames verify typing, repeated Enter, Shift+Enter and Escape. Raw layout and borrowed One Euro state remain byte-identical across Find actions.
- Final Node run: Find parent plus two example subtests pass in2.469 seconds. Three no-window unified/standalone entry tests also pass; combined run3.650 seconds. These are not the full IPC/layout-diagnostics suite.
- Artifacts: `native/build/current-find-work/`: `build-final.log`, `final-checks.log`, `crate-find.json`, `mario-find.json`.

## Review outcome

- User authorized separate Mario viewer PID10752 from `current-find-work/gd-view.exe`, starting in MDA. Existing viewers not replaced; no OS input posted by agent.
- Supplied review checklist covers Ctrl+F focus/no Fit, live matching, Enter/Shift+Enter/result selection, empty/no matches, Escape retention, off-screen/filtered items, Grid headers, paused Free geometry and typing guards.
- User reports reviewed presentation looks good and authorizes checkpoint. Reviewed Find presentation accepted; individual checklist steps and unperformed modes are not inferred.
- No every-mode, restart, macOS/Linux or final compiler-matrix acceptance claimed.
- Sidebar checkpoint `3294ee8` and Find checkpoint `b222c43` committed, not pushed. At Find checkpoint, default90-second integration timing gate remained open. [Current diagnostics runtime work](diagnostics-runtime.md) tracks subsequent research split and under1-second target.
