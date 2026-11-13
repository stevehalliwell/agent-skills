# Resource readability and shared Free graph defaults

## Review evidence

- Prior control-review checkpoint: `01de149`; earlier implementation checkpoint `60331e0`. Both local, unpushed.
- Existing Mario viewer PID3884, session `3884-c734fb8b8e31108d`. Resources has550 identities. Native arrangement, paused, no canvas filters or distance focus.
- Framed exact `case:interactions%2Fbump-brick-small#small-bump`: one selected item, three direct neighbors, zoom1.20000005, no context clipping.
- User reports local area looks fine. Current overlap defect not reproduced. Do not assume historical Resource overlap persists.
- User finds connected nodes farther apart than unrelated nodes; asks for visible bounds everywhere and shared Neighborhood behavior in all Free/force graphs. Native/Neighborhood distinction not useful in reviewed presentation; user prefers Neighborhood only if no useful distinction remains.

## Requested result

- Use shared Neighborhood preparation consistently across Free/force graphs. Preserve fixed/Grid/constrained layouts and exact native restoration for those modes.
- Enable existing whole-graph workspace bounds by default for every Free/force view. User chose one enclosing rectangle per graph, not per-component outlines.
- Investigate connected-neighborhood spacing versus unrelated component placement. Retain exact graph identities, declared edges, counts and evidence; do not invent grouping semantics, causality or flow.
- Remove Native initial-arrangement choice outright. User explicitly requests no migration/compatibility alias: product unreleased. Remove redundant arrangement selector; use Neighborhood for all Free/force preparation. Fixed native/Grid/band layout modes remain.
- Preserve shared bounded solver, render-only smoothing, typed selection, independent panes, drafts, explicit analytical Run and last-good failure behavior. No implicit query Run or source edits.

## Implementation boundary

- Apply common defaults/preparation to unified and standalone Free/force viewers. Seed once on explicit geometry preparation/rearrangement, not every frame, camera change or filter change.
- Existing bounded Neighborhood seeder and workspace sizing remain shared. Keep20% extra width/height and render-only smoothing. No per-view solver or layout budget widening.
- First review new defaults before tuning additional group spacing. Local overlap was not reproduced; connected-neighborhood spacing remains review target, not a proven global distance invariant.
- Normal startup Actor/Effect preparation may build default canvas; never consume prior pending analytical inputs.

## Implementation

- `gd_layout_prepare_free()` composes existing bounded Neighborhood seeder and optional workspace setup. No alternate solver or seeder. Fixed layouts keep geometry and disable workspace.
- Remove public `initial_arrangement` field/selector, internal pane seed choice and Actor/Effect seed-choice parameter. Query scene placement now accepts workspace preference instead; Free always seeds Neighborhood. No migration alias.
- Unified pane preparation and query spatial units share preparation. Rearrangement defers seeding until existing measured footprints are remapped; preserve compound stacks and typed scene metadata. Candidate allocation failure retains previous layout/result.
- Standalone Skills/Challenges/Relationships/Verb usage and MDA prepare candidate geometry before swap. Resource startup seeds before its existing120 solver steps; unified Resources uses same order. No startup/iteration budget increase.
- Shared bounds renderer covers unified and standalone Free graphs; Fit now frames selections or shown node footprints, excluding workspace container; [selection framing follow-up](selection-framing.md). Workspace defaults true in pane creation, Reset view and Reset all. Unsupported fixed/Grid/similarity views keep preference but do not draw/enforce workspace. Existing bounds toggle remains for explicit override.
- Workspace width/height still include20% extra room. No smoothing feedback, per-frame reseed, automatic camera/resize/filter reseed or analytical Run.

## Checks

- Warning-free CJIT core, adapter and production graph-viewer builds in `native/build/neighborhood-defaults-work/`.
- Core145/145 pass. Added common-preparation checks for bounded/unbounded Free setup, invalid preferences, allocation failure and fixed-pose preservation. Native query/cloud restoration and compound behavior remain covered.
- Final focused Node13/13 pass; both examples' source hashes unchanged. Includes Find/default IPC/new shared-default checks and deliberately enabled deep presentation/layout/refresh regressions because shared preparation changed. No widened thresholds/timeouts.
- Shared-UX checks Crate404 / Mario968: all supported Free panes have default bounds; ordinary Free layouts match shared preparation at pane aspect, with Resource120 steps; unsupported layouts remain unbounded. Reset/draft/history tests remain intact.
- Resource deep probes: Crate30 nodes /17 groups, Mario550 /168; zero initial overlaps and zero after600 more steps. Mario still hits contact work limits and has measurable raw motion. Not a convergence or pixel-readability proof.
- Final default Node Crate315.7035ms / Mario1018.3239ms including source hashes; combined1334.7915ms. Sub1-second budget remains open; no guaranteed latency claim.
- Existing private ordinary-layout legacy/Neighborhood comparison probes remain diagnostic only, not product controls. Removed obsolete Effects Native-seed trials: optional research now17 configurations /85 records, with Effects weighted/unweighted × workspace off/on. Samples0/120/600/4096 and256-step motion window unchanged. Research not rerun; historical19-configuration results are not current measurements.
- Artifacts: `build-core.log`, `build-adapter.log`, `build-graph.log`, `core.log`, `final-checks.log`, `crate-ux.json`, `mario-ux.json`.
- User authorized replacement of old review viewer after inspection showed no modified records, Undo/Redo steps or pending rank text. Verified PID/path, closed PID3884 through normal window-close request, then opened Mario Resources PID43208 / session `43208-e528fcf3cb81b5a6` from new build. Fitted whole workspace for boundary/packing review; no OS input or forced termination. Subsequent island-separation review in PID21672 accepted updated Mario Resources presentation; user authorized checkpoint. [Island implementation, measurements and acceptance](shared-force-layout-islands.md). Broader every-view/standalone pointer behavior and final compiler/platform matrix remain unverified.
