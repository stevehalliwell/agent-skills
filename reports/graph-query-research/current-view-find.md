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

- Current bare F shortcut performs Fit without modifier check. Consume Ctrl+F before that path and ensure modified F does not also Fit.
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

Implementation pending. Sidebar changes remain uncommitted; diagnostics runtime reduction is a separate captured task in todo.md.
