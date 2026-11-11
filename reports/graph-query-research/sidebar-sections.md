# Standard sidebar sections

## Goal

First pass of search/filter/scope clarity: standardize UI structure across nine unified workbench views. Standalone legacy preview windows are outside this slice. Group related controls into collapsible sections; remember open/closed state. Matching sections share one preference across views: collapsing Find in Verb usage keeps it collapsed in MDA. Selections, cameras, filters and queries remain independent. Fold preferences survive workbench restarts in separate UI-only settings outside authored game-design records. Do not persist selections, queries or other analytical state. User specifically requests a dedicated Find selected in another view section containing its controls.

## Preserved behavior

- Preserve existing actions, native domain semantics, typed identities, selections/evidence, independent pane cameras/filters, pending/applied inputs, drafts/Undo and last-good failures.
- Folding changes presentation only. Do not Run, clear results, change filters, reveal hidden data or change CLI availability.
- Keep required preparation/cache/token updates outside conditional drawing. Closed sections must not stop necessary work or create extra work.
- No source-record edits, solver changes, viewer launches/replacement, or broader Reset/Clear/pending-application redesign in this pass.

## Structure

Projection selector stays visible. Use stable section names/order across views; include only applicable sections. Retain domain-specific controls/subfolds rather than force false equivalence.

1. View & layout: presentation/layout/camera actions, including Focus selection.
2. Find selected in another view: destination, match summary/list and related navigation controls.
3. Display & filters: canvas visibility and retained Actor/Effect display controls. Distinguish list-only search by explicit reach; native Display does not Run analytical inputs.
4. Analysis: native query controls/results and relevant analytical subfolds.
5. Browse shown nodes: list and list-only search.
6. Selection & evidence: current selection, source/evidence inspection.
7. Sources & changes: refresh, draft/Undo/report controls.
8. Diagnostics.

Keep important stale/error/unsaved and limited-drawing warnings visible even when related section collapsed. Fold preferences are not analytical state; Reset current view should not erase them.

## Grounding

`gd_explorer_view.c` draws shared sidebar in owner loop. Projection switch, Find, camera, layout, domain controls, refresh/drafts and browsing are currently interleaved. Existing Analysis layers, knowledge, rules and source details use individual ImGui headers/tree nodes. `setup` disables ImGui INI persistence via `IniFilename=NULL`.

Common sidebar drawing reorganized in owner loop. MDA traversal/root now under Analysis; role/reference/query/witness visibility under Display & filters. Actor/Effect retained display controls extracted from analytical inputs into same display section, using unchanged native Display action. Similarity keeps analytical geometry and receives common View Fit/Focus controls. General ImGui persistence remains disabled.

## Preferences

- Defaults open: View & layout, Display & filters, Analysis, Selection & evidence. Other common sections closed. Hide unsupported sections without resetting their preference.
- One viewer-owned eight-bit state, separate from all panes. Reset, reload, rearrange and query operations do not reset it.
- Persist only eight named flags with `version=1`; bounded512-byte reader, atomic temporary-file replacement on actual toggle. No per-frame file I/O.
- Per-user `game-design-workbench/sidebar-folds.ini`: Windows `%LOCALAPPDATA%`; macOS `~/Library/Application Support`; Linux `$XDG_CONFIG_HOME` or `~/.config`. Never fall back to project/source folders. Keep general ImGui INI persistence disabled.
- Missing file uses defaults. Invalid/unreadable file preserves session values and reports warning; save failure retains session values and old file. No source/query failure inferred.
- Load at launch, save on toggle. Across running viewer instances, last save wins; no live cross-window synchronization.
- Persist common section folds only. Native per-record evidence trees/subfolds retain existing behavior; no record-specific inspection state saved.

## Acceptance

- Matching actions live in same named sections/order where applicable; Find controls collapse together.
- Switching views retains agreed fold memory without touching destination selection/camera/filter/query state.
- Closing/reopening sections preserves staged inputs, results, evidence, drafts and action availability.
- New preferences instance reloads flags from disk. Skipped/minimized windows do not save false closed states.
- CPU UI/state checks plus authorized live review; no broad visual acceptance inferred from headless checks.

## Implementation and checks

- `gd_sidebar.c/.h`: shared section catalogue, defaults and isolated per-user preference I/O. Windows UTF-8 paths use wide APIs; macOS/Linux use native paths. Versioned named flags only, no source content.
- Common header helper uses Cimgui current-window getter, ignores skipped windows and saves only actual toggle events. Owner-loop preparation/analysis/matching/input/IPC remain independent of fold drawing.
- Warning-free CJIT CLI, explorer adapter and graph builds in `native/build/sidebar-sections-work/`. No viewer launched/replaced or user settings written during checks.
- Crate production adapter passes, including34 sidebar checks,25 feedback checks, existing presentation/configuration tests, unchanged pane snapshots and source hashes. Sidebar tests use private build-directory fixture only, removed afterward.
- Mario production adapter completes under separate180-second diagnostic allowance:99.058 seconds,34 sidebar checks,25 feedback checks,286 adapter checks,2145 presentation checks, source hashes unchanged; helper ownership checks pass.
- **Default focused suite does not pass timing gate:**4 tests pass; Mario subtest and parent fail existing90-second timeout. No timeout or assertion widened. Earlier full suite passed before final skipped-window hardening; that is not a pass for final build. Do not report6/6 for current code.
- Artifacts: `build.log`, `adapter.log`, `adapter-timeout.log`, `mario-diagnostic.json`, `mario-diagnostic.log`, `mario-diagnostic-summary.json`; earlier Crate probes in `crate.json`/`crate.log`.

## Live review setup

User authorized separate Mario viewer. Opened `native/build/sidebar-sections-work/gd-view.exe`, PID37632, session `37632-49e9a9433090d86f`, MDA bands, enemy-contact selected and readable Focus applied. Existing viewers untouched; no OS input posted. Artifacts: `review-existing.json`, `review-pid.txt`, `review-session.txt`, `review-configure.jsonl`, stdout/stderr logs.

User accepted reviewed folds/standardization presentation. Acceptance does not establish unperformed restart, every-view or pointer checks. Default90-second integration timing gate remains open; macOS/Linux execution and final compiler matrix remain unperformed. Changes uncommitted. Next recommended: complete search/filter/scope wording, preserving behavior, then refine pending/applied/display consistency.

## Comments

CJIT headers lack `WC_ERR_INVALID_CHARS`; use existing project's guarded Windows API constant instead of dropping strict conversion. Initial private-path save check exposed backslash handling; normalize directory walk. First skipped-window guard read context internals and faulted; use exported Cimgui window getter instead. Removed only known test-owned crash fixture. Final Crate/Mario diagnostics pass skipped-window and persistence checks; default Mario timing gate remains open.
