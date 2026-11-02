# Shared force layouts

Status: Steps 1/2 reviewed and retained. Step 3 position/force crossing experiments did not work well enough and were removed. User authorized checkpoint and continuation to Step 4. Revisit crossings before final review; [research retained for that gate](directed-crossing-algorithms/first-blush/recommendation.md), no replacement selected. User reviewed Mario Resources: substantial readability improvement; clusters too vertical. Additive expansion requested after joint IPC tour. Current task authority: root `todo.md`, Step 2. Live readability acceptance remains open. [Step 1 findings, measurements and budgets](shared-force-layout-baseline.md); [Step 2 implementation and limits](shared-force-layout-separation.md); [rejected position trial](shared-force-layout-crossings.md); [force trial and limits](shared-force-layout-force-uncrossing.md).

## Result

Offer common free force-directed presentation across graph-backed projections. Preserve existing layouts. Use MDA Free graph and grouped Interactions Free graph as visual references.

- Skills: add Free graph. Show computed band number on node without locking free placement to band.
- Challenges: add Free graph. Seed from current directed arrangement, then allow force-directed separation and movement.
- Relationships: retain stable neighborhood layout; add force-directed neighborhood with overlap avoidance.
- Resources: improve existing force graph so nodes and labels separate rather than forming unreadable word cloud.
- Actors: retain native query bands/cards; add free-floating graph. Shared elements and actual edges should produce useful groupings, with grouped Interactions as reference.
- Effects: same additive free-floating presentation as Actors.
- Existing MDA/Interactions free layouts participate in shared solver improvements. Preserve their useful visual behavior; verify regressions explicitly.
- Similarity distance map remains distinct. Do not replace feature-distance coordinates with force placement.

## Source audit (before implementation)

- `native/gd_layout.{h,c}` already provides shared node/edge/group storage, arrangement and fixed-step force solver. Current force solver applies center attraction, point repulsion, active-edge springs, damping and speed cap.
- Repulsion checks at most 32 fixed partners per node, within supplied group only. It does not test rendered rectangles/label footprints. Thus force repulsion is not node-overlap prevention.
- Current step contains no explicit edge-crossing reduction. Directed arrangement/focus have specialized clearance repairs; these are not a general collision/crossing solver.
- Interactions assigns connected components as groups. MDA, Resource and native query scenes currently use one group; query scenes do not run ongoing force settling in explorer.
- Resources performs 120 initial force steps and starts paused. More initial point-repulsion steps alone do not establish readable non-overlap.
- Skills supplies prerequisite/rank bands. Challenges supplies route bands. Relationships seeds force storage, then replaces geometry with stable ego arrangement and disables ongoing settling.
- Actor/Effect query scenes use shared `GD_Layout`, but own typed scene metadata, node sizes, selection, contributors and ghost semantics. Alternate placement must preserve these mappings.
- Drawn footprints differ: text/circles, MDA cards and sized query cards. Collision geometry must match rendering in layout coordinates; MDA free display currently applies separate coordinate scaling.

## Shared technical direction

Refactor/extend existing shared layout mechanism, not one solver per projection. Keep typed adapters responsible for topology, exact identities, displayed labels, node footprints, band metadata and initial placement. Shared algorithm owns geometry, forces and bounded work, not domain evaluation.

- Preserve authored arrows and relation types. Springs and crossing heuristics never infer execution, causation or new edges.
- Use actual shared elements; do not merge distinct actor occurrences, variants, ghosts, pools or effect placements because labels match.
- Preserve meaningful disconnected groups. No invented community/classification semantics from visual clusters.
- Separate node/label overlap handling from broad repulsion. Include nearby nodes across group boundaries; group bounds must account for node footprints.
- Add bounded crossing-reduction effort. Crossing reduction is a heuristic, not a zero-crossing guarantee; some graphs cannot be drawn without crossings.
- Reconcile attraction and collision forces so separation does not create endless jitter or collapse back under springs.
- Keep deterministic bounded work, finite coordinates, safe allocation failure and interactive drag/pause behavior. Avoid exhaustive all-edge-pairs work per frame at 4096 nodes / 32768 edges.
- Layout switch retains exact selection/case context, source inspection and applied results. No hidden Run, source writes or draft/Undo changes.
- GUI/IPC expose same layout choices and shared transitions. Free positioning must not silently change question scope or feature data.
- Free graph allows all nodes, including data roots, to move. Authored arrows and band annotations remain metadata, not position constraints. Existing constrained layouts retain current root/band behavior. Selection focus must not silently re-pin free-mode roots.
- Free graph exposes shared `Initial arrangement: Native / Neighborhood`, with Native default. Native retains each view's starting arrangement, including Challenge directed seed. Neighborhood changes initial geometry of already-shown nodes, not neighborhood scope or analysis. Changing seed repositions shown nodes, resets velocities and resumes settling; selection, applied results, filters, draft and Undo remain intact. GUI and CLI expose same operation. Anchor choice and component placement need technical refinement; no fabricated root for cycles or disconnected groups.
- Existing constrained layouts remain available. Any shared solver changes must respect their ordering constraints.

## Delivery slices

1. **Complete; reviewed.** Specify collision/crossing work budgets and measured acceptance fixtures from existing limits. Audit current renderer footprints and reference layouts before solver changes. Evidence in linked Step 1 baseline.
2. **Complete; reviewed in Mario Resource viewer.** Shared footprint-aware separation, renderer unions/scaling, footprint-sized Free graph component spacing and actual Resource component grouping. CJIT core 133/133, Node 21/21; loaded-example initial Resource footprints have zero intersections after startup and 600 further steps. Adversarial dense fixtures still retain bounded overlap/motion; user reports substantial readability improvement in Mario Resources, not exhaustive every-view acceptance.
3. **Failed experiments removed; crossing work deferred until before final review.** Position corrections were jittery/not clearly better; force fallback and branch rotation also did not work well enough. Remove experimental runtime/workspace/tests; restore reviewed `ed08a0c` overlap code exactly. CJIT core 133/133, Node 21/21, both example IPC suites and ten deterministic 120/600-step fixtures pass. Historical trial measurements and algorithm orientation retained, not adopted. User authorized checkpoint and Steps 4/5 continuation. Existing viewers untouched.
4. Add shared Native/Neighborhood initial-arrangement choice and geometry-only restart for Free graphs, plus Skills, Challenges and Relationships layout choices, band annotations and selection-preserving transitions through shared GUI/IPC controls. Captured follow-up: current neighborhood clusters are too vertical; aim to match main-view aspect ratio. Refine packing behavior in this later slice, not crossing reduction.
5. Add Actor/Effect free placement without changing retained query topology, typed identities or contributor evidence.
6. Revisit failed edge-crossing approaches and algorithm research **before final review**; record a new bounded trial or explicitly retained limits. Then run CJIT-only checks and coordinate live visual review on both examples. Keep full compiler matrix at final Step 6.

## Acceptance

- Every requested new layout is additional; original layouts remain selectable.
- Skills retains meaningful band annotation while nodes move freely. Band annotation follows retained prerequisite-depth/authored-rank basis; exact UI wording remains implementation detail.
- Challenge Free graph begins from directed arrangement and separates without losing route identities.
- Relationships keeps same selected/revealed neighborhood when switching stable/free placement.
- Resource labels/cards separate on representative dense scopes; compare directly with current word-cloud failure.
- Actor/Effect shared elements attract connected structure without fabricated identity merging. Switching back restores valid native presentation and exact inspection context.
- Measure node-footprint intersections, crossing counts and settling/work on deterministic fixtures before/after. Include long labels, coincident nodes, separate components, cycles, shared hubs, multiedges and dense graphs. Record unresolvable constraints and budget exhaustion rather than claiming universal no-overlap/no-crossing.
- Dragged node remains controlled by pointer; layout toggle, pause, selection focus and readable framing do not fight each other.
- Camera fit does not conceal overlap problems by shrinking text. Selection framing remains separate tracked task.
- Live usefulness/readability must be reviewed; CPU geometry tests alone are insufficient.

## Open

- Neighborhood seed anchor choice and disconnected-component placement need technical refinement. Preserve selection without treating an anchor as authored root or changing graph scope.
- Crossing reduction remains unsolved and deferred, not a blocker for Steps 4/5. Revisit before final review without silently adopting fixed ranks, new dependencies or another uncrossing heuristic.
- Step 1 sets initial measured-fixture work caps. Collision implementation validated in Step 2; any future crossing implementation needs its own budget/quality checks. No latency guarantee chosen.
