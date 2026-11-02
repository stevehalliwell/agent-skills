# Shared Free layouts — Step 4

Status: shared seeding reviewed in Mario Resources and committed as `549d7bd`. Skills/Challenges/Relationships Free choices reviewed in separate Mario viewer PID 25244/session `25244-0e683f145cbe05a2`; user accepted result and authorized checkpoint/Step 5. Checkpoint `21df372` removed crossing experiments and retained reviewed overlap solver. Crossing reduction remains unsolved; revisit before final review.

## Contract

- Add Free graph without removing current layouts. Actor/Effect delivery remains Step 5.
- Shared **Initial arrangement: Native / Neighborhood**. Native is default.
- Choosing arrangement repositions nodes, clears velocity and resumes settling. Preserve typed identities, selection, scope, filters, applied analysis, drafts and Undo. No hidden analytical Run.
- User chose graph-derived centres per connected component, independent of selection, and compact neighborhoods rather than explicit hop-distance rings. Centre is initial geometry only, never pinned.
- Regular Free settling/dragging allows all nodes, including data roots, to move. Existing constrained modes retain band/root rules. Root movement passes CPU checks; rendered annotation/drag acceptance remains pending visual review.
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
- Temporary seed workspace: 524288 bytes. Combined with existing 114688-byte collision scratch: 638976 bytes. Explicit geometry commands now hold 162056-byte candidate-pane snapshot on heap rather than Windows stack; combined scratch is 801032 bytes, below 1 MiB. Candidate layout (2048088 bytes) and optional owned distance-focus state are separate. No persistent layout-size increase or per-frame allocation.
- GUI uses current main canvas aspect ratio. Unknown, zero, non-finite or unusable viewport falls back to deterministic 1:1 ratio. Public helper rejects invalid ratio.
- Empty graph succeeds with zero scene extent. Invalid geometry/capacity/topology or allocation failure retains complete prior layout.
- Packing is initial geometry, not rigid cluster ownership. Ordinary forces can move disconnected components afterward, especially when adapter semantic groups span multiple components.

Shared GUI/IPC path:

```text
form: presentation
initial_arrangement: native | neighborhood
```

- Control applies immediately to prepared Skills/Challenges/Relationships/Interactions/MDA/Resources Free graphs. Native bands/stable Relationships, columns/grid and similarity canvases cannot apply it or silently switch layout mode. Remember choice when temporarily using native layout.
- Prepare candidate from adapter Native arrangement; reuse matching measured footprints before Neighborhood seed. Native restores adapter's current starting arrangement, including Resource startup settling.
- Seed changes and `view.rearrange` require unchanged retained keys, edges, groups and bands. Reject changes rather than hiding source refresh inside geometry command. Native/Free switches with same retained graph and band basis preserve analysis cache; changing band basis can still invalidate shown-layer cache. No hidden analytical Run.
- Commit only complete candidate. Failed seed allocation retains layout pointer, arrangement, pause state and retained analysis. Existing selection remap remains shared with layout changes.
- Distance-focus remains separate presentation feature, not hub selection policy. While enabled, it owns transient pose. Free graphs no longer apply implicit directed data-root anchors; native constrained modes retain them. No live pointer/animation acceptance claimed.

## Added Free choices

Unified explorer GUI and IPC share these `layout` keys:

```text
skills:        depth | rank | free
challenges:    bands | free
relationships: native | free
```

- Defaults remain depth, declared route bands and stable Relationships neighborhood. Existing Matrix/MDA keys unchanged. Legacy standalone viewers retain existing choices.
- `gd_explorer_graph_prepare()` reuses native adapters, then changes only layout mode to Free. Native seed retains directed/ego coordinates; no force-circle reseed or ranking replacement. Neighborhood uses shared seed helper.
- Skills remembers last depth/rank basis across Free. Renderer adds `Band N` or `Unplaced` beside existing rank/warning caption; panel names remembered basis. Labels are metadata, not fixed coordinates or authored-rank values.
- Relationships keeps revealed/expanded sets, root, boundary and selected record across mode/seed changes. Candidate preparation suppresses automatic replay of expanded flags, then restores flags; mode change cannot reveal newer neighbors. Explicit expansion/reset still adds records. Authored arrows, duplicates and shared nodes stay intact.
- Shared `layout_settles()` controls GUI Pause, Space and stepping. Challenge/Relationships native layouts remain stable; Free modes resume ordinary solver. Shared drag helper moves Free roots on both axes.
- Pure geometry mode switch preserves matching retained graph-analysis cache. Candidate pane/layout/focus ownership commits only after success; allocation-stage failures retain prior mode, geometry, pause and analysis.

## Verification

CJIT only; final compiler matrix remains Step 6. No windows used during automated verification. User-authorized Mario Resources review launched separate PID 25444/session `25444-74095c64c513f5ad`; Neighborhood selected and canvas fitted through IPC. User accepted result. Existing viewers untouched.

- Core 136/136, including seed metadata/topology preservation, deterministic reseed, asymmetric footprints, cycles/shared junctions, duplicate/self/inactive links, isolated components, wide/tall packing, empty graph, exact capacity and atomic invalid-input/allocation failure.
- Both production adapters pass: 254 IPC, 2232 controls, 105 configuration and 10 feedback each; presentation checks 509 Crate / 1778 Mario. Added native-coordinate preservation, remembered rank/depth basis, expanded Relationships scope, Free root movement/held invariance, settling availability, unchanged analysis bytes across Native/Free switch and every candidate-allocation stage.
- Node 21/21, zero skipped. Protocol reused verified fixture: 90 cases + 3 writer + 115 capture checks. Graph/helper builds pass without launch.
- Ten Native geometry fixtures at 120 steps match overlap checkpoint in all non-timing fields. Existing solver geometry and layout size remain unchanged.
- Seeding tests moved large saved test-pane snapshot to heap. New rank-mode test path then exposed production candidate-pane stack cost; shared transition now allocates candidate pane on owner allocator and cleans it on every outcome. Allocation-stage sweep passes without leaks. No runtime stack limit raised.
- Seeding evidence: ignored `native/build/control-work/seed-*`, builds `native/build/seed-work{,-ui}`. Free-choice evidence: `native/build/control-work/free-views-*`, builds `native/build/free-views-work{,-ui}`. Running PID 25444/executable untouched.

CPU checks do not prove rendered readability, stable final aspect or complete root/drag acceptance. GUI widget and IPC use same transition. Resources seed and new Skills/Challenges/Relationships Free choices have user visual acceptance. This does not establish exhaustive control/animation acceptance.

## Review boundary

- Step 4 visual review accepted; checkpoint and Actors/Effects continuation authorized.
- Automated checks launched no windows. User-authorized review launched separate `native/build/free-views-work-ui/gd-view.exe`, PID 25244; Skills Free/Neighborhood selected and fitted through IPC. Other viewers untouched.
- Actors/Effects remain Step 5. Full compiler matrix remains final Step 6. Revisit crossings before final review.

## Comments

- Research recommendation is not adopted. Neither removed crossing experiment returned.
- Orientation and historical measurements remain linked from shared layout plan.
