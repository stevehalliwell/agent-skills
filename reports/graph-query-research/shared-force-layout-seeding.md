# Shared Free layouts — Step 4

Status: first implementation slice reviewed in Mario Resources and approved for commit. Checkpoint `21df372` removed crossing experiments and retained reviewed overlap solver. Shared seeding now covers existing Interactions, MDA and Resources Free graphs. Skills/Challenges/Relationships Free choices remain next slice. Crossing reduction remains unsolved; revisit before final review.

## Contract

- Add Free graph without removing current layouts. Actor/Effect delivery remains Step 5.
- Shared **Initial arrangement: Native / Neighborhood**. Native is default.
- Choosing arrangement repositions nodes, clears velocity and resumes settling. Preserve typed identities, selection, scope, filters, applied analysis, drafts and Undo. No hidden analytical Run.
- User chose graph-derived centres per connected component, independent of selection, and compact neighborhoods rather than explicit hop-distance rings. Centre is initial geometry only, never pinned.
- All Free nodes, including data roots, must move. Existing constrained modes retain band/root rules. Full Free-mode/root/annotation acceptance remains pending expansion slice.
- Keep renderer footprints and MDA coordinate normalization. Pack toward main canvas aspect ratio. No reseed merely from selection, zoom or resize.
- No crossing-specific logic, new dependency, authored-data changes or per-frame allocation.

## Delivered first slice

`gd_layout_seed.h`, exposed through `gd_layout.h/.c`:

1. Validate prepared Free layout, capacities, order permutation, groups, finite footprint bounds and edge endpoints before changing geometry.
2. Build undirected adjacency from active non-self edges. Preserve original edge directions, types, duplicates and self-links.
3. Find transient connected components without changing adapter semantic groups. Pick highest distinct-neighbor-degree hub; break ties using retained native order. Parallel edges do not inflate degree.
4. Traverse breadth-first from hub. Place traversal order on compact rectangular spiral, with footprint-sized cells and 16-unit clearance. This is not fixed hop-distance banding or a crossing objective.
5. Shelf-pack component footprints toward canvas width/height ratio, with 32-unit component spacing. Unequal components can dominate aspect; no optimal packing claim.
6. Commit positions, zero velocities, recompute semantic-group bounds and reset settling/collision counters. Preserve node/edge metadata, order, membership and ordinary force rules.

Limits:

- 4096 nodes, 32768 edges. Fixed workspace initialization plus linear edge/adjacency/BFS/packing work; no recursion or all-pairs seed pass.
- Temporary seed workspace: 524288 bytes. Combined with existing 114688-byte collision scratch: 638976 bytes, below 1 MiB operation-scratch budget. Existing candidate-layout ownership is separate: 2048088 bytes. No persistent layout-size increase.
- GUI uses current main canvas aspect ratio. Unknown, zero, non-finite or unusable viewport falls back to deterministic 1:1 ratio. Public helper rejects invalid ratio.
- Empty graph succeeds with zero scene extent. Invalid geometry/capacity/topology or allocation failure retains complete prior layout.
- Packing is initial geometry, not rigid cluster ownership. Ordinary forces can move disconnected components afterward, especially when adapter semantic groups span multiple components.

Shared GUI/IPC path:

```text
form: presentation
initial_arrangement: native | neighborhood
```

- Control applies immediately to prepared Interactions/MDA/Resources Free graphs. Bands/columns/grid and similarity canvases cannot apply it or silently switch layout mode. Remember choice when temporarily using constrained layout.
- Prepare candidate from adapter Native arrangement; reuse matching measured footprints before Neighborhood seed. Native restores adapter's current starting arrangement, including Resource startup settling.
- Seed changes and `view.rearrange` require unchanged retained topology. Reject changed topology rather than hiding source refresh inside geometry command. Preserve retained analysis cache; do not run analysis again.
- Commit only complete candidate. Failed seed allocation retains layout pointer, arrangement, pause state and retained analysis. Existing selection remap remains shared with layout changes.
- Distance-focus remains existing separate presentation feature, not hub selection policy. Do not claim this slice completes all-root movement or new-mode annotation acceptance.

## Verification

CJIT only; final compiler matrix remains Step 6. No windows used during automated verification. User-authorized Mario Resources review launched separate PID 25444/session `25444-74095c64c513f5ad`; Neighborhood selected and canvas fitted through IPC. User accepted result. Existing viewers untouched.

- Core 136/136, including seed metadata/topology preservation, deterministic reseed, asymmetric footprints, cycles/shared junctions, duplicate/self/inactive links, isolated components, wide/tall packing, empty graph, exact capacity and atomic invalid-input/allocation failure.
- Production GUI/IPC adapter checks cover both shipped examples: default restoration, selection independence, retained analysis bytes, read-only unavailable controls, unknown-viewport fallback, no resize/discovery reseed and candidate-allocation failure.
- Ten Native geometry fixtures at 120 steps match overlap checkpoint in all non-timing fields. Existing solver geometry and layout size remain unchanged.
- Large existing test-pane snapshot moved from stack to owned temporary heap after expanded test hit Windows stack overflow. No runtime stack limit raised.
- Final Node/protocol/helper counts recorded in root `todo.md` after verification. Evidence: ignored `native/build/control-work/seed-*`; builds: `native/build/seed-work{,-ui}`.

CPU checks do not prove rendered readability, stable final aspect or complete root/drag acceptance. GUI widget and IPC use same transition; no live GUI interaction trial claimed.

## Next slice

- Add Skills Free graph while preserving depth/rank keys, remembered basis and visible band annotation without fixed band coordinates.
- Add Challenges Free graph with current directed arrangement as Native seed.
- Add Relationships Free graph without expanding revealed scope or losing root context.
- Retain all current native layouts. Trace selection, root constraints and routing before each adapter change.
- Stop for Step 4 review before Actors/Effects. Revisit crossings before final review.

## Comments

- Research recommendation is not adopted. Neither removed crossing experiment returned.
- Orientation and historical measurements remain linked from shared layout plan.
