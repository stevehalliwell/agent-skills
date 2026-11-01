# Shared force layouts — Step 2 separation

Status: reviewed in Mario Resource viewer; user reports substantial readability improvement. Exhaustive live readability acceptance remains open. No crossing-reduction work or new layout modes in this slice. [Plan](shared-force-layout-plan.md); [prior baseline](shared-force-layout-baseline.md).

## Changed behavior

Free force layouts use axis-aligned local footprint bounds, including asymmetric labels/captions. Renderer bounds are independent of semantic band/card dimensions.

- Shared solver projects intersecting rectangles along smaller escape axis, with 8 layout-unit clearance. Full containment uses escape distance, not intersection width.
- Active-edge springs account for footprint size rather than always pulling large cards toward 110-unit center spacing. Authored edges/meaning unchanged.
- Free component seed extents account for footprints. Existing column/directed modes retain their positioning constraints and do not run new projection pass.
- Resource presentation now groups actual connected components of retained incidence edges, rather than placing every node in one group. No nodes, relations, pools or identities merged; isolated nodes remain separate components. This addresses initial word-cloud compression without inventing flow.
- Renderer measures node labels/captions and shared analysis badges into footprint union. Graph markers reserve outline room. MDA normalizes displayed coordinates by its 3x Free graph scale. Resource initial settling reserves body/default badge bounds before first draw.
- Query-card dimensions provide initial footprints, but Actor/Effect ongoing force behavior remains later slice. Existing query bands and intentional ghost stacks unchanged.
- Held node does not move under separation. Pause remains explicit. Changing annotations while paused updates bounds but does not silently resume settling.
- Explorer discloses bounded separation work/movement limits. Camera fit is not evidence of clearance.

## Work and ownership

- Sweep sorted snapshot x-intervals; narrow phase reads live rectangles. Refresh sorted snapshot between passes so changes do not invalidate broad-phase order within a pass. In-place heapsort gives bounded O(n log n) sorting without library allocation or quadratic worst case.
- At most 16 passes and 65536 candidate visits/checks per display step. One pass did not clear contact chains in measurements; extra passes retain same total candidate cap. Complete sweeps use stable order near rest; only work exhaustion rotates starting row.
- At most 20 layout units of separation movement per node per step, plus existing ordinary movement capped at 5; allow floating-point rounding. This prevents hundreds-unit correction jumps on dense fixtures.
- A small over-relaxation margin reduces repeated spring compression. Compression velocity is removed only on corrected axis.
- `collision_limited` means work, pass or correction movement was bounded; it does not count residual overlaps. It may remain set even when rectangles do not overlap.
- Scratch lives in owner-allocated layout: 98304 bytes interval scratch plus 16384 bytes movement allowance. No new heap allocation in frame solver, global mutable scratch or large stack workspace.
- Node storage: 80 bytes, previously 64. Layout: 2048088 bytes, previously 1867848; increase 180240 bytes per allocated layout. Unvisited panes do not allocate layouts.
- Footprint setter rejects non-finite, empty/reversed or out-of-range bounds, retaining previous bounds. Coordinate range: +/-1048576 layout units. Existing caller-owned replacement/failure behavior retained.

## Verification

CJIT only. Separate output directories preserve running viewer.

- Native core: **133/133**. Five new tests cover asymmetric/invalid bounds, cross-group projection, pointer ownership, full containment, actual component grouping/inactive links, fixed-layout behavior and collision work cap/continuation.
- Both examples: production IPC adapter passes **254 IPC + 2178 control + 105 configuration + 192 presentation/result + 10 feedback** checks. Additional checks include pure renderer-footprint union/scale and default Resource layout clearance/bounded work. Owned allocations released; source data unchanged.
- Targeted Node: **21 passed, zero failed/skipped**.
- Protocol fixture: **90 cases + 3 writer + 115 capture** checks.
- CLI, graph viewer and explorer helper compile. No native window launched or existing viewer manipulated.
- Baseline runner passes ten finite/deterministic fixtures at 120 and 600 steps. Candidate checks never exceed 65536. Actual position displacement measured separately from velocity to avoid hiding correction motion.

### Loaded-example Resource geometry

These are CPU checks of initial body/default-badge footprints, not font-rendered screenshots. First observation follows startup's 120 settling steps; second follows 600 additional steps in independent layout copy.

- Crate: **30 nodes / 17 components**, zero intersections at both observations. Last 32-step displacement RMS **0.000226**, maximum **0.000508** layout units.
- Mario: **550 nodes / 168 components**, zero intersections at both observations. Last 32-step displacement RMS **0.130038**, maximum **1.621199** layout units. Bounded flag remains set; no global-clearance guarantee inferred.
- Separate graph draw code still measures exact current text/badge unions when renderer runs. Live review must inspect those full footprints and camera readability.

### Adversarial synthetic limits

After 600 total steps, exact footprint-intersection counts:

- Interaction components **0**, MDA card hub **0**, long labels **0**, cycle/multiedges **0**.
- Forced single-group 550-card ring **180**, query-card hub **16**, coincident cards **1**, deliberately overlapping groups **1**, dense 64-card graph **3**.
- Capacity 4096 nodes / 32768 edges **80387**; work cap active. Prior point-only solver retained **543285** at same step count, but initial component extents also changed, so this is combined seed/solver improvement, not isolated projection effect.
- Dense fixtures can retain overlap and motion under finite budgets. Do not claim universal no-overlap or settled geometry. Further tuning remains a visible review limit, not a hidden success criterion.
- Local 600-step solver-only means: forced single-group Resource ring **1.97 ms/step**, capacity **6.44 ms/step**, versus prior point-only **0.22 ms** and **2.16 ms**. More geometry work costs CPU. Single local runs exclude rendering/metrics; not latency guarantees.
- No edge-crossing reduction implemented; crossing counts may improve or worsen as rectangles separate.

Evidence under `skills/game-design-workbench/native/build/control-work/`: ignored `force-{core,node,crate-geometry,plumber-geometry}.log`, `force-adapter-{crate,plumber}.json`, `force-protocol.json`, `force-separation-{120,600}.json` and `build-force-*.log`.

## Progress review

Review build: `skills/game-design-workbench/native/build/force-work-ui/gd-view.exe`; CLI: `skills/game-design-workbench/native/build/force-work/gd.exe`. Build not launched.

Inspect Interactions Free graph, MDA Free graph and Resources, especially pause/resume, long labels, badges, dragging and group separation. These are existing layouts; Skills/Challenges/Relationships and Actor/Effect Free graph choices still come later. Full compiler matrix remains final Step 6.

User reviewed separate Mario Resources viewer (PID 46488) and authorized commit plus Step 3 crossing reduction. Existing viewers preserved. Later packing follow-up recorded in plan/todo: neighborhood clusters too vertical; aim to match main-view aspect ratio.
