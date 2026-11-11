# Free graph coverage audit and missing modes

## Result

Four missing Free modes added after spring-spacing checkpoint `1e52e61`:

1. Actor usage cloud in unified explorer.
2. Standalone Skills viewer.
3. Standalone Challenges viewer.
4. Standalone Relationships viewer, including focused relationships reached from matrix graph.

Existing layouts/defaults retained. Shared solver unchanged. No query Run, source writes or new dependencies. User authorized separate usage-cloud review viewer after checks. User accepted reviewed usage-cloud Free presentation and authorized commit. Standalone GUI review and broader every-view acceptance remain open.

## Audit

Unified explorer's nine projections:

- Skills: depth/rank and Free already present.
- Challenges: route bands and Free already present.
- Relationships: stable neighborhood and Free already present.
- Verb usage: Free, noun/verb columns and Grid already present.
- MDA: role bands and Free already present.
- Resources: already a Free incidence graph, not missing a solver.
- Actors: interaction map already weighted Free; explicit state/coverage scenes already support Native/Free. Usage cloud was fixed-only.
- Effects: Native contributor context and Free already present.
- Noun interactions: Grid and Free already present.

Standalone entry points:

- Matrix already offers `--layout graph` and graph/columns GUI choice. Table-only entry remains available.
- MDA already offers role bands/Free. Resources already uses Free incidence.
- `view skill-atoms` switches Skills/Challenges/Relationships but previously exposed only native layouts and suppressed Challenges/Relationships settling. Added all three Free choices.

Auxiliary canvases:

- Similarity canvas is a numerical distance embedding, not a topology graph. Changing its coordinates or inventing springs would change analytical meaning. Leave unchanged.
- Adjacency/incidence insets, case grids, evidence lists and rule/binding inspectors are not missing graph solvers. Retain these analytical presentations.
- Structural question overlays use existing source graphs. No additional result graph or inferred edges introduced.

## Actor usage cloud

- Keep Native fixed actor ring and central concept grid, including existing coordinates and usage-derived card sizes.
- Add **Layout: Fixed actor-ring cloud / Free graph**; IPC presentation `layout: native | free` while `actor_overview: usage`.
- Free starts from same cloud pose for Native seed, or shared Neighborhood seed. All actors, axes and noun concepts may move; no fixed ring or pinned roots in Free.
- Existing Initial arrangement, Pause/Space, workspace bounds and render-only One Euro controls work in Free. Fixed Native bypasses settling, dragging, bounds and smoothing while keeping preferences.
- Use same private query-scene spatial projection, weighted springs, footprint separation and last-good allocation behavior. No new solver, evaluator or scene topology.
- Geometry switches retain scene/query identity, pending/applied inputs, selection, counts and evidence. Native restores prepared cloud pose after movement. Explicit rearrange resumes settling, matching existing query-layout behavior.
- Interaction map remains automatically Free; generic layout control stays disabled there. IPC now reports its actual `free` layout instead of misleading `native` value.

## Standalone Skills, Challenges and Relationships

- Add Free graph alongside depth/rank, route bands and stable neighborhood.
- Reuse same native seed preparation as unified explorer, then enable shared Free solver. Keep source band/rank/root annotations; Free does not lock data roots or axes.
- Preserve each standalone projection's layout choice while switching Explore mode. Skills retains depth/rank basis independently of Free.
- Expose Pause and Space for new settling modes; native stable Challenges/Relationships remain non-settling. Fit, dragging, draft/Undo and source navigation use existing paths.
- CLI: `view skill-atoms ... --mode skills|challenges|relationships --layout graph`. Default remains prerequisites/native; `prerequisites` and `rank` remain accepted. Table/columns and matrix-pair overlay remain rejected for this entry.
- Standalone views retain existing unsmoothed rendering and existing camera behavior. No new standalone IPC endpoint, workspace control or seed selector added. Unified versions already provide those presentation controls.

`gd_graph_view.c::prepare_layout()` now owns shared native-seed-to-Free conversion for both entry paths. Remove old duplicate unified-only conversion. No per-frame allocation or new layout scratch; standalone remembered choices use three integers.

## Verification

- CJIT CLI, core, explorer adapter and graph builds pass without compiler warnings.
- Core 145/145. Extended overview test exercises both Free seeds, all-node dragging, 600 raw steps, finite geometry, footprint containment, three allocation failure stages, invalid seed/aspect, exact Native-coordinate restoration and unchanged typed metadata/counts/evidence.
- Focused integration 6/6. Both real examples retain source hashes and release owned memory. Production GUI/IPC operations cover cloud switching, allocation rollback, pending/applied separation, selected-node retention, Neighborhood, bounds, render-filter availability and Native restoration. Empty Crate actor cloud also supported without inferred actors.
- Standalone CLI regression covers default/prerequisites/rank/graph and rejected table/columns across all three modes using headless executable. Accepted choices reach existing `view.unavailable` boundary without opening a window; invalid choices remain `cli.arguments`.
- Shared seed preparation and Free root movement exercised through existing unified adapter checks. Standalone GUI choice/remembered-mode behavior compiled and traced, not real-pointer tested.
- No new analytical interpretation or source modification. Full compiler matrix remains final Step 6.
- Authorized separate Mario review viewer opened: PID `35648`, session `35648-2693a87c34c3bc7e`, executable `native/build/missing-force-work/gd-view.exe`. Confirmed Actors / Usage cloud / Free / Neighborhood, bounds/filter on, unpaused, Fit applied. Guarded presentation-only IPC; no analytical Run. No existing window replaced or closed. Logs/configuration retained beside binary.

User reviewed usage-cloud Free viewer, said "looks good" and authorized commit. Acceptance covers reviewed presentation, not unperformed pointer checks or standalone GUI review. Do not claim CPU checks prove readability or pointer behavior.

Next recommendation: selection continuity across Grid/Free and presentation changes, followed by consistent selection framing. Preserve accepted layouts; leave visual polish for final UX pass.
