# Shared force layouts — Step 3 crossing reduction

Status: historical failed position-correction trial; user rejected live result as jittery/not clearly better. Both this and subsequent force trial were removed; overlap separation retained. [Force trial and removal checks](shared-force-layout-force-uncrossing.md). Revisit crossing work before final review; Steps 4/5 may proceed. Reviewed Step 2 committed as `ed08a0c`. [Plan](shared-force-layout-plan.md); [separation baseline](shared-force-layout-separation.md).

## Behavior

Shared Free force step now tries bounded centerline uncrossing after footprint separation.

- Defer when footprint separation reports partial work. Node clearance takes priority; a bounded separation pass is not permission to sacrifice spacing.
- Visit edge pairs with deterministic continuation across steps. Exclude inactive edges, self-loops, shared endpoints, collinear overlap and endpoint-only contacts from strict crossing metric.
- For a crossing, try up to four endpoints, nearest escape first, excluding pointer-held node. Move perpendicular to other edge's centerline, at most 10 layout units per step, toward its opposite side plus 8-unit margin. At most one move accepted per step.
- Require complete destination footprint clearance against every node, including other groups, with 8-unit gap.
- Check all active edges incident to moved node against all edges before and after trial. Accept fewer crossings, or equal count with lower summed crossing penetration depth. This supports gradual escape without teleporting distant endpoints. Penetration depth is minimum endpoint-to-opposite-line distance for a strict crossing.
- Restore trial position on rejection or work exhaustion. Do not change velocities until acceptance. No partial/sample validation accepted.
- Ordinary forces on later steps may reintroduce crossings; monotonic improvement applies only to accepted move at current geometry, not entire simulation trajectory.
- Preserve exact node/edge identities, arrow directions, relation metadata, scope, selection, applied analysis, drafts and Undo. No new authored edges or flow inference.
- Directed/column modes do not run heuristic. Pause remains explicit. Existing viewers left untouched; separate review build launched for user-authorized visual trial (PID 36924, Mario Resources).

This is straight-centerline geometry, not routed-curve optimization. MDA Beziers, loop bows and label/edge routing still need visual review. No global or zero-crossing guarantee.

## Bounds and limits

- At most **8192** charged visits per step, including candidate enumeration, node-clearance checks, incident-edge discovery and before/after edge-pair checks.
- Minimum validation cost that already exceeds remaining budget rejects proposal immediately. Later exhaustion also rejects and restores trial position.
- No new heap allocation, large stack arrays, broad-phase scratch or dependency. Layout adds **24 bytes** including alignment: **2048112 bytes** total. Frame scratch remains owner-held separation scratch from Step 2.
- `crossing_checks`, `crossing_moves`, pair continuation and `crossing_limited` are layout diagnostics, not analytical evidence or new CLI controls.
- `crossing_limited` reports separation deferral or bounded crossing work. It is not a crossing count or proof that crossings exist.
- Separation's partial flag is conservative: it can remain set even when body rectangles do not overlap. Such layouts also defer crossing work. Dense scopes may receive no crossing moves; improving this requires stronger clearance completion, not dropping validation.
- Large affected neighborhoods may not fit full scoring budget. Skip rather than infer improvement from samples. No full-sweep turnaround or frame-time guarantee.

## Verification

CJIT only; full compiler matrix remains final Step 6.

- Core **137/137**. Four added tests cover progressive uncrossing and integrated force behavior, pointer ownership, identities, 10-unit motion bound, node obstacles, excluded intersections, constrained modes, trial rollback on exhausted after-score, 8192-visit cap and pair continuation.
- Both production IPC adapters: **254 IPC + 2178 control + 105 configuration + 192 presentation/result + 10 feedback** checks each. Resource probe checks both work caps. Owned allocations released; example sources unchanged.
- Node **21 passed**, zero failed/skipped. Protocol **90 cases + 3 writer + 115 capture** checks.
- CLI, graph viewer and explorer helper compile into separate `native/build/crossing-work*` directories. No rebuild of running review executable.
- Baseline runner now includes **12** fixtures. All finite and deterministic at **120/600** steps; work caps respected.

### Comparison against committed Step 2 solver

Compile `gd_layout.c` from `ed08a0c` with same current fixture harness/header and unchanged collision helper. This isolates crossing behavior, including two new fixtures, without changing initial footprints or seeds. Frozen temporary source/executable live only in ignored build directory.

- **Crossed independent pair:** 1 crossing remains after 120 and 600 steps without heuristic; **0** with heuristic. Zero footprint intersections both versions. **16** accepted incremental moves total; last 32-step displacement RMS at 600: **0.010259** layout units.
- **Crossed four-node cycle:** same **1 to 0** crossing result, zero footprint intersections. **16** accepted moves; final-tail RMS **0.010263** layout units.
- **All ten earlier fixtures:** exact final positions, overlap counts and crossing counts retained at both observations. Interaction/MDA reference fixtures remain at zero intersections/crossings. Dense fixtures defer crossing work while separation stays partial, preserving known Step 2 overlap limits rather than trading spacing for fewer lines.
- Crossed-pair maximum charged work **12**, crossed-cycle **33**. Isolated capacity test with resolved/default separation flag exhausts exactly **8192**, then continues from next pair. Capacity force fixture itself defers crossing because separation is partial.

### Loaded-example Resource regression

CPU initial reserved footprints, not font-rendered screenshots. Observations after startup's 120 steps and 600 further steps:

- Crate: **30 nodes / 17 groups**, **zero** intersections at both observations. Final-tail RMS **0.000226**, maximum **0.000508** layout units.
- Mario: **550 nodes / 168 groups**, **zero** intersections at both observations. Final-tail RMS **0.130038**, maximum **1.621199** layout units.
- Positions/motion match Step 2 probes. Mario's partial separation flag continues to defer crossing work. This slice does not promise fewer lines in already crowded Resource clusters.

Evidence under `skills/game-design-workbench/native/build/control-work/`: `crossing-{core,node,crate-geometry,plumber-geometry}.log`, `crossing-adapter-{crate,plumber}.json`, `crossing-protocol.json`, `crossing-{before-,}{120,600}.json` and `build-crossing-*.log`.

## Progress review

New build: `skills/game-design-workbench/native/build/crossing-work-ui/gd-view.exe`. Launched separately as PID 36924 in Mario Resources. Step 2 viewer remains independent. Conditional fallback order recorded in plan: current corrections, then forces if unsatisfactory, then online research if both fail.

Position trial rejected; force fallback also unsuccessful. User authorized removal of both experiments, checkpoint commit and continuation to Step 4. Revisit [established directed crossing algorithms](directed-crossing-algorithms/first-blush/recommendation.md) before final review; no replacement selected. Aspect-ratio packing follow-up remains there: neighborhood clusters currently too vertical; aim to match main-view aspect ratio. No packing, new layout modes or seed-control work included here. Experimental code/tests removed; this report preserves prior variant evidence, not current implementation.
