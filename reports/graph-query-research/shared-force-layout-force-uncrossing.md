# Shared force layouts — force-based uncrossing trial

Status: historical failed trial, removed at user's request. Position corrections and force-based uncrossing did not work well enough; neither adopted. Reviewed footprint separation retained unchanged from `ed08a0c`. Revisit crossing algorithms before final review; proceed with later layout work meanwhile. Metrics below describe removed trial, not current code. [Research orientation](directed-crossing-algorithms/first-blush/recommendation.md); [plan](shared-force-layout-plan.md); [previous position trial](shared-force-layout-crossings.md).

## Mechanism

- Detect strict crossings between active edge centerlines. Exclude self-loops, shared endpoints, collinear overlap and endpoint-only contacts. Does not optimize rendered Bezier/loop routes directly.
- Add uncrossing forces before ordinary velocity integration. Do not apply direct uncrossing position corrections. Existing damping and 5-unit ordinary speed cap remain.
- Favor lower-valence endpoint around more-connected junction. Blend escape-normal force with tangential force around that junction: move away from crossing while encouraging branch swing. Junction stays free under ordinary forces; never pin data roots.
- For a detachable branch, spread common angular force through its actual connected members around junction. Collect whole branch first. Another attachment back to junction makes branch ambiguous; retain endpoint-only force. No authored parent links, artificial edges or semantic hierarchy added.
- Average contributions per node and cap force-vector magnitude at **2**. Smooth target with **0.2** follow factor; decay when crossings disappear. Fade force with crossing depth near grazing contact rather than toggle full-strength kicks.
- Run footprint separation last. Do not disable uncrossing just because separation reports partial work. Collision projection still has its own finite limits, not a universal clearance guarantee.
- Preserve selection, applied results, scope, typed identities, draft/Undo, pointer-held node and pause. Existing directed/column modes unchanged.

Tree-parent-like goal is geometric branch swing, not data reparenting or a promised tidy-tree layout. Highly shared/cyclic neighborhoods often use endpoint-only force.

## Bounds and ownership

- At most **8192** candidate/branch visits per step, plus O(node count) reset/smoothing loops. Pair-offset rounds distribute work across edges rather than holding early edge fixed for many frames.
- Cache active adjacency during arrangement; rebuild with topology. Branch BFS uses owner-held queue/stamps, no recursion. Work exhaustion keeps endpoint contribution but never spreads force through incomplete branch traversal.
- **409600 bytes** crossing workspace plus **114688 bytes** existing collision workspace = **524288 bytes** total, below agreed **1 MiB** operation scratch cap.
- Layout size **2457720 bytes**: increase **409632** versus committed Step 2, **409608** versus position variant. Lazy pane allocation retained. No new frame heap allocation or large stack arrays.
- Diagnostics: `crossing_hits` counts detected visits, not distinct analytical crossings; `crossing_forced_nodes` includes decaying force; `crossing_branches` counts multi-node spreading passes, not leaf-only cases. `crossing_limited` reports work cap, not crossing count.

## CJIT checks

- Core **138/138**. Five force tests cover integrated uncrossing, held-node ownership, excluded pairs, constrained modes, force-only velocity changes, decay, whole-branch propagation, ambiguous-cycle fallback, partial-search fallback, work cap/continuation and scratch size. Old position-correction assertions removed.
- Original 64-node/4096-step settling thresholds unchanged and pass. Constant-force prototype produced late kicks; depth fade fixes that regression. Current last-120-step velocity RMS **0.000070**, maximum **0.000600**. Velocity is not actual positional displacement; fixture harness separately measures positions.
- Node **21 passed**, zero failed/skipped. Both production IPC adapters pass **254 IPC + 2178 control + 105 configuration + 196 presentation/result + 10 feedback** checks each. Allocations released; example sources unchanged.
- Protocol **90 + 3 writer + 115 capture** checks. CLI, graph viewer and CPU explorer helper compile into separate `native/build/uncross-force*` directories. No alternate compiler matrix before final Step 6.
- Twelve synthetic fixtures remain finite/deterministic at **120/600** steps; force work never exceeds **8192**. Earlier variant source preserved in ignored `native/build/control-work/position-uncrossing-source/`.

## Measurements

### Synthetic results at 600 steps

Compare against position variant, which defers dense cases and retains Step 2 geometry there. Mixed outcomes; do not claim globally better layouts.

- Crossed independent pair and crossed four-node cycle: **0** crossings and **0** footprint intersections, as with position variant. Without either uncrossing variant, both retain **1** crossing. Current final-tail position RMS **0.010868 / 0.015450** layout units.
- Coincident cards: crossings **23 to 12**, intersections **1 to 1**. Overlapping groups: crossings **109 to 93**, intersections **1 to 1**.
- Dense 64-node/256-edge case: crossings **458 to 458**, intersections **3 to 3**; displacement RMS **1.719159 to 1.113788**.
- Cycle/multiedges: crossings **32 to 32**, intersections **0 to 1**; displacement RMS **0.484277 to 0.085289**.
- Long labels: crossings **14 to 14**, intersections **0 to 0**, but displacement RMS worsens **0.468517 to 1.417051**.
- Forced single-group Resource ring: intersections worsen **180 to 247**. Capacity: **80387 to 82578**. Exact crossing totals omitted above 256 edges. These remain visible stress limits, not claimed passes for universal spacing.
- Interaction-component, MDA-hub and query-hub references have no uncrossing activity and retain prior geometry. No zero-crossing, convergence or latency guarantee.

### Loaded examples

CPU initial/reserved footprints, not font-rendered screenshots. Observe after 120 settling steps, then after 600 more steps. All six projection probes have **zero footprint intersections** at both observations.

- Mario Interactions Free: **38 nodes / 26 edges / 16 groups**; late position RMS **0.058504**, maximum **0.156983** layout units. Uncrossing forces active.
- Mario MDA Free: **44 nodes / 74 edges**; RMS **0.037251**, maximum **0.085804** layout units. Uncrossing forces active.
- Mario Resources: **550 nodes / 496 edges / 168 groups**; RMS **0.130038**, maximum **1.621199**. **No detected centerline crossings/uncrossing activity** during measured steps; motion unchanged from earlier variants. It is not a useful default scope for judging uncrossing effect.
- Crate Interactions: RMS **0.000272**, maximum **0.000458**. Crate MDA Free: **0.087354 / 0.202192**, forces active. Crate Resources: **0.000226 / 0.000508**, forces inactive.
- No multi-node branch spreading occurred in these default projection probes; their active forces are endpoint/junction turns. Whole-branch behavior is verified by dedicated branched-tree unit fixture. Do not claim example graphs are trees.

Evidence under `skills/game-design-workbench/native/build/control-work/`: `uncross-force-{120,600}.json`, `uncross-force-{core,node,crate-geometry,plumber-geometry}.log`, adapter/protocol JSON and `build-uncross-force-*.log`. Earlier `crossing-*` and `force-separation-*` evidence retained. Timings local and exclude rendering/metrics; not frame-time guarantees.

## Review boundary

Trial build: `skills/game-design-workbench/native/build/uncross-force-ui/gd-view.exe`, launched separately as PID **22044** in Mario **MDA Free graph** for meaningful force activity, not unchanged Resource baseline. Shared IPC set layout `free`, settling unpaused and Fit; successful replies confirm no source/draft change. Existing windows not manipulated. Setup receipts: `uncross-force-review-{layout,settling}.json` under evidence directory.

Visual review boundary was crossing improvement, branch/junction behavior, jitter, pause/drag and actual text/badge clearance. No acceptance inferred; user chose algorithm research fallback.

User ended both experiments and authorized removal, checkpoint commit and continuation to layout Step 4. Research retained as orientation only; no replacement adopted.

## Removal checkpoint

- Restore `gd_layout.{c,h}`, core/presentation/baseline tests and measurement script exactly to reviewed `ed08a0c` versions. Remove private crossing implementation and white-box test headers. No crossing-specific workspace, fields, runtime calls or test hooks remain.
- Preserve collision helper, renderer footprints, footprint-sized springs/component spacing and actual Resource component groups unchanged. Ordinary force-directed layout remains.
- CJIT core **133/133**; Node **21/21**, zero skipped; protocol **90 + 3 writer + 115 capture**. Both examples: **254 IPC + 2178 controls + 105 configuration + 192 presentation + 10 feedback** checks.
- Ten baseline fixtures pass at **120/600** steps, finite/deterministic. `GD_Layout` restored to **2048088 bytes**. Code/test/script diff against `ed08a0c` is empty.
- Both Resource footprint probes retain **0** intersections initially and after **600** additional steps. Crate tail RMS/max **0.000226 / 0.000508**; Mario **0.130038 / 1.621199** layout units/step. Bounded dense-motion limits remain, not a universal clearance guarantee.
- Fresh ignored builds: `native/build/overlap-checkpoint` and `native/build/overlap-checkpoint-ui`. Evidence: `native/build/control-work/overlap-checkpoint-*`. No viewer launched, rebuilt in place, closed or manipulated; no alternate compiler checks before final Step 6.
- Revisit failed approaches and retained algorithm research **before final review**. Decide a new bounded trial or explicitly retain limits then; this is not an immediate implementation blocker for Steps 4/5.
