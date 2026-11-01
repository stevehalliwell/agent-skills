# Shared force layouts — Step 1 baseline

Status: audit, reusable fixtures and initial work budgets complete; awaiting user review before Step 2. Production layout/renderer behavior unchanged. Plan: [shared force layouts](shared-force-layout-plan.md).

Checkpoint before work: `8167ead`. Measurements use unchanged production `gd_layout.c` and `gd_layout_routes.c`, built with Windows CJIT. No GUI launched, restarted or manipulated.

## Footprint audit

Collision must operate in layout coordinates using rendered local bounds, not node centers or semantic band dimensions. Bounds can be asymmetric: labels often sit below node center. Include visible captions/badges; selection outlines may add small fixed padding. Camera zoom does not change layout-space footprints.

- **Interactions:** circle radius 13 or square 24 × 24; centered name at y + 22, font 18. Case-count text belongs to edges, not node footprint. Component grouping is real connectivity; inactive overlay must not merge groups.
- **Skills:** same marker/name plus rank/conflict/unknown caption at y + 43, font 13. Draft marker and active analysis badges add extents. New band number must join measured footprint when implemented. Existing directed preparation measures name width and reserves 120 height, not a measured union of every annotation.
- **Challenges:** marker/name plus entry/reachability caption at y + 43. Existing route-band geometry must remain separate from rendered bounds and Free graph seed.
- **Relationships:** marker/name plus collection/root caption at y + 43. Stable ego arrangement measures approximate label length, then fixes neighborhood positions. New force mode must preserve revealed topology while separating full labels/captions.
- **MDA:** rendered card 210 × 64, centered on displayed position; role/name and optional root text inside. Shared/shape/reach/path annotations extend beyond card. Free graph multiplies layout coordinates by 3 before rendering. Thus body collision bounds are 70 × 21.333333 layout units, not stored 220 × 96 semantic layout dimensions. Normalize all measured bounds by this scale. Long truncated text still needs renderer measurement; byte truncation does not prove text fits.
- **Resources:** rendered card 180 × 48, with role and truncated label. Shared/comparison/shape/reach/path annotations extend above/below. Resource adapter leaves stored node width/height zero; generic routing fallback is not actual card footprint. All 550 Mario-like synthetic nodes sharing one layout group illustrates pressure on point repulsion, not actual-example measurement.
- **Actors/Effects:** card rectangle uses explicit node width/height. Base 250 × 72, weighted effect cards can reach 400 × 172; coverage cards use 440 × 112. Text is clipped inside cards. Native scene has typed nodes, ghosts and wrapped bands; Free graph must retain mappings. Intentional stacked-ghost display remains separate from generic non-overlap behavior.
- **Similarity:** distance-derived positions remain outside force solver scope.

Sources: `native/gd_graph_view.c`, `gd_mda_view.c`, `gd_resource_view.c`, `gd_explorer_query_view.c`, `gd_query_scene.c`, `gd_layout_routes.c` and typed adapters. Paths relative to `skills/game-design-workbench/`.

## Integration findings

- Existing solver is already shared. Extend it rather than duplicate per-view forces.
- Point repulsion uses up to 32 fixed partners per node within group. At capacity: 131072 repulsion checks and 32768 edge-spring visits per step, plus linear node work.
- Edge springs target center distance 110 regardless of card size. Large cards can overlap more as springs converge; stronger repulsion alone is not a footprint contract.
- Center attraction targets fixed initial group centers. Expanding geometry without updating component extents can pull components back into collision. Group packing/separation must be checked alongside node separation.
- Directed solver locks ordering axis and suppresses generic edge springs. Existing constrained modes must retain these semantics.
- General Free graph collision work must consider nearby nodes across groups. Existing sampled intra-group repulsion cannot establish that property.
- Actor/Effect node dragging currently calls directed layout movement, which locks band axis. Layout choice must propagate through input as well as arrangement and rendering.
- Relationships/Challenges and query panes are explicitly excluded from ongoing settling in explorer loop. Adding selector alone would not activate forces.
- MDA coordinate scaling also affects picking, camera and focus calculations; do not remove scale in only one path.

## Reusable baseline

```powershell
./scripts/measure-force-layout.ps1 -Steps 120 > baseline-120.json
./scripts/measure-force-layout.ps1 -Steps 600 > baseline-600.json
```

Runner compiles only existing solver/routing and `native/test_layout_baseline.c`. No dependencies, renderer or project-data inputs. Default executable: ignored `native/build/force-baseline/test-layout-baseline.exe`; optional output directory keeps other builds untouched. JSON stdout; failure exits nonzero.

Ten deterministic synthetic fixtures cover components, MDA-scale cards, dense Resource-like incidence, query-card hubs, long labels, coincident nodes, overlapping groups, cycles/parallel edges/self-loops, dense links and full capacity. These are geometry proxies, not loaded Mario/crate scenes or visual acceptance.

Metrics:

- Exact all-node-pair footprint intersections before/after, including cross-group pairs. Touching edges do not count as overlap.
- Maximum minimum-axis penetration in layout units.
- Exact strict interior centerline crossing count only for at most 256 edges. Shared-endpoint pairs, self-loops, inactive edges and collinear overlaps excluded. Larger graphs report null, not zero. This is not rendered-curve/label obstruction measurement.
- Solver CPU step time excludes geometry metric scans, arrangement and determinism rerun. Windows `clock()` resolution can report zero for short batches; no zero-cost inference.
- Rebuild and repeat each fixture, comparing final position hashes. Finite positions/velocities required. Same-build determinism only; no cross-compiler promise.
- Self-check known crossing, shared endpoints, inactive edges, self-loops, overlap, touching and crossing-count omission. Reject invalid step counts.

## Measured baseline

Overlap pairs: initial / after 120 steps / after 600 steps.

- interaction-components, 48 nodes: **36 / 27 / 27**.
- mda-card-hub, 64 nodes: **90 / 34 / 51**. Conservative 70 × 32 layout-unit body/badge proxy; not exact renderer text union.
- resource-card-ring, 550 nodes: **7904 / 8607 / 8744**.
- query-card-hub, 96 nodes: **1276 / 2242 / 2301**.
- long-label-ring, 64 nodes: **515 / 226 / 248**.
- coincident-cards, 48 nodes: **1128 / 75 / 53**.
- cross-group-cards, 48 nodes: **768 / 139 / 27**.
- cycle-multiedges, 32 nodes: **144 / 49 / 44**.
- dense-cards, 64 nodes / 256 edges: **294 / 159 / 160**. Centerline crossings: **896 / 276 / 284**.
- capacity, 4096 nodes / 32768 edges: **175399 / 186160 / 543285**. Crossing count omitted.

More settling does not reliably remove footprint overlap. Some point-layout fixtures improve while large-card/large-component fixtures get worse. This is a baseline, not a failure of yet-unimplemented collision behavior.

Local solver mean per step: capacity **2.158333 ms** at 120 steps, **2.166667 ms** at 600 steps. Resource-like fixture **0.216667 ms** / **0.225 ms**. Single local runs, synthetic topology, no GUI/render cost, not latency guarantees. Existing `GD_Layout`: **1867848 bytes**; node: **64 bytes**.

## Initial engineering budgets

Internal starting caps for next two slices; tune from measured implementation, not new user-facing options. Do not promise a frame deadline from these caps.

### Step 2 — footprint separation

- Preserve existing node/edge capacities: 4096 / 32768.
- Separate global broad repulsion from local footprint collision. Use deterministic bounded neighborhood candidate generation, not exhaustive node pairs each display step.
- Cap local candidate enumeration/narrow-phase work at **65536 visits/checks per display step**. Count enumeration work too; no hidden unbounded dense-bucket scan. Broad-phase preparation must have separately bounded node/index work.
- Use a deterministic continuation/fairness policy if cap prevents scanning all candidates. Expose budget exhaustion in tests/diagnostics; never claim complete clearance from partial work.
- At most one bounded separation pass per display step initially. Re-run over frames; pointer-held node stays controlled by user. Existing pause behavior remains explicit.
- Scratch allocation ceiling **1 MiB per active layout operation**, owner-managed, not large stack frames or shared mutable scratch across panes. Measure actual memory; avoid reserving worst-case scratch in every unvisited pane.
- Reject non-finite inputs/results and retain last-good layout on preparation/allocation failure. Separation must not corrupt selection, source IDs or analytical caches.
- Measure exact overlaps offline after 120 and 600 steps. Require zero overlap on feasible small fixtures; report residual counts and exhaustion for dense/capacity fixtures. Add measured font/annotation integration before claiming real-view clearance.

### Step 3 — crossing reduction

- Separate budget: **8192 edge-pair candidate visits/checks per display step**, including enumeration. No all-edge-pair inner loop in production.
- Exclude shared endpoints, self-loops and duplicate/collinear cases from strict crossing objective. Preserve all authored edges and endpoint identities.
- Crossing improvement is secondary to node clearance and stable settling. Candidate moves must not undo Step 2 clearance or violate existing constrained modes.
- Exact offline crossing oracle restricted to fixtures with at most 256 edges. Capacity run verifies bounded work, finite state and determinism, not exact global crossing optimum.
- At capacity, exhaustive node pairs would be **8386560**; edge pairs **536854528**. These are offline comparison ceilings, not acceptable per-frame loops.

## Checks and review boundary

- CJIT build succeeds; both 120-step and 600-step runs pass all ten finite-state/determinism checks and metric self-checks.
- Invalid executable arguments `0`, `601`, `xyz` reject with exit 2.
- JSON parsed successfully with Node. `git diff --check` clean except existing LF/CRLF conversion notices.
- Evidence: ignored `native/build/control-work/force-baseline-{120,600}.json`.
- No solver, renderer, GUI, IPC or project-data behavior changed. No full compiler matrix or live readability acceptance claimed.
- Step 2 waits for requested progress review. Neighborhood seed anchor/component policy remains Step 4 refinement, not a blocker for Step 2.
