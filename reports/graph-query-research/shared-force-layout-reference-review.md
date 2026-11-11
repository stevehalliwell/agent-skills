# Reference-layout comparison

Status: source audit and matched adapter-footprint measurements complete after checkpoint `a2537f2`. Effects remains unresolved; next repair target is dense-hub contact handling, not missing grouping/seeding. No solver, default or authored-data change selected.

## Goal and boundaries

Find which Resources and Verb usage behaviors can improve Effects and later Free graph cleanup. User identifies Resources Neighborhood and Verb usage Noun / verb columns and Free graph as useful references.

- Preserve existing layouts, typed identities, selections, query/evidence scope, pending/applied inputs, drafts/Undo and last-good failure behavior.
- Reuse shared bounded layout machinery. No new evaluator, invented edges, automatic analytical Run, crossing algorithm or dependency.
- Keep classification suffix/style, count-link styling and reverse inset highlighting in recorded UX work.
- Compare first. Approval to investigate does not adopt shared defaults or a new spring model.
- CJIT only. Leave current viewer untouched; no window launch.

## Findings

### Same solver, different presentation pipelines

All movable Free graphs use `gd_layout_step_weighted()` in [gd_layout.c](../../skills/game-design-workbench/native/gd_layout.c). Query scenes call it through private spatial projection; ordinary graphs call it directly. Shared GUI loop advances one step per active, unpaused frame in [gd_explorer_view.c](../../skills/game-design-workbench/native/gd_explorer_view.c).

Resources has no separate successful force solver to copy. Its distinct preparation, grouping and rendered footprints are candidates for comparison, not proof of causation.

### Resources

[gd_resource_layout()](../../skills/game-design-workbench/native/gd_projection.c) assigns real active-edge connected components before Free arrangement. Each component gets its own attraction centre. Default footprints include card body, shared-count row and outline: local bounds `(-106,-43,106,45)`.

[gd_explorer_resource_prepare()](../../skills/game-design-workbench/native/gd_resource_view.c) advances 120 startup steps. Renderer later measures annotation unions. Default pane is paused. Choosing Neighborhood or entering Free resumes settling through shared presentation controls; do not assume accepted Neighborhood review was paused.

Mario CPU probe has 550 nodes / 168 groups. After startup plus 600 further steps, no footprint overlaps remain, but final-32-step RMS is **0.130038**, max **1.621199**, with collision-limit flag. This is not evidence of complete convergence, nor a direct comparison with Effects' longer measurement window.

### Verb usage: Noun / verb columns

[gd_matrix_layout()](../../skills/game-design-workbench/native/gd_layout.c) retains noun/verb sides and source-model connected components. Columns mode uses stable side-by-side placement and does not settle. This directly serves a noun-to-verb reading task; usefulness is not evidence of better physics.

Keep columns as reference for readable role distinction and source tracing. Do not transplant noun/verb categories into Effects or constrain Free roots by implication.

### Verb usage: Free graph

Same model topology/component groups, ordinary shared solver and Native/Neighborhood control. [gd_graph_view.c](../../skills/game-design-workbench/native/gd_graph_view.c) draws compact noun circles / verb squares, then adds measured labels, captions and analysis badges to footprints. It does not use Effects' multiline cards.

Initial headless adapter bounds and live renderer-measured bounds differ. A CPU comparison must name which bounds it uses; do not equate either with visual readability automatically.

### Effects

[gd_query_scene.c](../../skills/game-design-workbench/native/gd_query_scene.c) builds typed activation/effect cards, wraps native contributor rows and enlarges result cards by direct-declaration count. Default selection is highest-incoming result, not a random or simplest case.

[gd_query_scene_free.h](../../skills/game-design-workbench/native/gd_query_scene_free.h) projects cards into private spatial units. Intentional variant stacks fold into compounds; typed evidence stays separate. It preserves canonical parallel source springs, suppressing only folded ghost duplication. Native starts from retained wrapped coordinates without Resource-style startup settling. Neighborhood uses the same shared seeder.

Private layout uses one group for current scene. This differs from Resources' component groups, but selected contributor context may already be connected. Count actual components before treating group policy as a defect.

[Effects diagnosis](shared-force-layout-query.md#effects-motion-diagnosis--checkpoint-follow-up) records both examples and rejected trials. Native still moves after 4096 steps; adding 120 startup steps cannot by itself establish a fix. Large spring gaps, constant radial target and relative-velocity correction did not resolve that failure. All runtime trials removed.

### Neighborhood is already shared

[gd_layout_seed_neighborhood()](../../skills/game-design-workbench/native/gd_layout_seed.h) derives distinct-neighbor hubs, places footprint-sized compact neighborhoods and shelf-packs transient components toward canvas aspect ratio. It preserves adapter groups and ordinary force rules. Initial component packing is not rigid ownership during later settling.

Do not reimplement Neighborhood per view or mistake seed availability for an agreed default policy.

## Matched measurement results

Built reusable `native/test_layout_reference.h` into existing CPU production adapter. It replaces earlier Effects-only probe, not production solver. Both examples have 11 topology records and 44 motion samples: Native/Neighborhood for four views, Resource startup control, and existing unweighted Effects display control.

Protocol: aspect **1.4**; samples at **0, 120, 600, 4096** diagnostic steps. Motion window is latest **256** steps, or all **120** steps at first moving sample. Resource raw Native and Native with **120** startup steps are separate runs. All four views use rendered-world scale **1**. Bounds are **adapter-only**, not GPU/font/annotation measurements. No user view/pause state changed.

Mario final-window results, zero startup steps:

| View | Seed | RMS | Max step | Centroid drift | Overlaps |
|---|---|---:|---:|---:|---:|
| Verb usage | Native | 0.203590 | 1.411897 | 4.316076 | 0 |
| Verb usage | Neighborhood | 0.004444 | 0.006937 | 0.856520 | 0 |
| Resources | Native | 0.099441 | 1.620711 | 0.032238 | 0 |
| Resources | Neighborhood | 0.007337 | 0.157727 | 0.040247 | 0 |
| Effects | Native | 2.452279 | 22.421452 | 85.536084 | 4 |
| Effects | Neighborhood | 0.692194 | 3.810456 | 27.339965 | 0 |
| Noun interactions | Native | 0.015804 | 0.051053 | 0.514894 | 0 |
| Noun interactions | Neighborhood | 0.001575 | 0.003082 | 0.100815 | 0 |

RMS/max units: world units/step. Drift spans final 256 steps. Different graphs and footprints prevent interpreting this as a controlled solver bake-off or rendered usefulness ranking.

### What comparison rules out

- **Missing component grouping:** Effects has **1** adapter group and **1** independently counted connected component. Resources has **168 / 168**, Verb usage **16 / 16**, nouns **4 / 4**. Reassigning Effects to connected components would not split this context.
- **Duplicate springs:** Effects has **68** active non-self springs and **68** unique neighbor pairs; maximum parallel multiplicity **1**. Ghost/parallel deduplication is not this reproducer's cause. Resources has multiplicity up to **4**, yet much lower motion.
- **Missing Neighborhood support:** all four run same seeder. Effects remains mobile with it.
- **Startup alone:** Resource Native after 120 startup plus 4096 diagnostic steps has RMS **0.099004**, versus **0.099441** without startup. Effects Native has no overlaps at step **120**, then **2** at step **600** and **4** at step **4096**. Early overlap clearance does not prove stable settling.
- **Larger result card alone:** existing unweighted Effects display retains exact typed nodes/edges/labels, fixes card bounds to **250 × 72**, but Mario still moves. Native RMS **2.256873**, max **15.000309**, drift **31.324624**; Neighborhood RMS **0.561593**, max **4.098877**, drift **7.157293**. Neither has final overlaps. Disabling size weighting is not a complete repair. This does not rule out deferred genuinely compact/selection-dependent UX.

### Supported next focus

Effects has largest distinct-neighbor hub: **25**, versus Resources **11**, Verb usage **9**, nouns **20**. Effects baseline footprints range **250–362.810547** wide and **72–119.004395** high. Dense fan-in plus rectangular contact handling is a candidate, not yet a proven sole cause; average degree alone does not explain failure.

Every final Mario Effects step reports a collision limit, but maximum candidate checks are only **6710** Native / **4464** Neighborhood, below shared **65536** cap. Raising that candidate cap is not supported as an Effects fix. Investigate remaining separation-pass/movement limits and repeated spring/contact compression. Do not assume which remaining limit fires without instrumentation.

Resources reaches candidate cap in both seeds; that is a separate scalability condition. Neighborhood improves occupied extent from roughly **1566 × 26004** to **4794 × 4823** in this adapter case. This supports retaining its packing as reference, not copying a new solver.

All Crate reference runs finish without overlaps or collision-limit flags; final RMS is at most **0.002878**. Failure is not universal. Noun regression control retains prior measured behavior.

### Checks and evidence

- CJIT adapter build passes. Focused CLI/both-example production-adapter checks **5/5**; source hashes unchanged and owned allocations released.
- Source pane/layout and typed scene checksums remain unchanged across private diagnostic runs. Unweighted control additionally checks exact typed nodes, edges and labels against original.
- Node regressions validate 11 topology profiles, sample/window counts, component/degree identities, finite metrics, work bounds and Resource startup consistency. Generic ring-window results match independent existing noun probe for RMS/max/drift in both seeds/examples.
- Native adapter test allowance raised from **30 s** to **90 s**: measured complete Mario run takes roughly **29–31 s** with added bounded diagnostics. This is a test budget, not a runtime change or performance guarantee.
- Evidence: `native/build/layout-reference-work/measurements.json` (**110** records), `{mario,crate}.{json,log}`, `build-final.log`, `adapter-final.log`. JSON diagnostics use `layout-reference ` prefix on stderr; existing stdout contract unchanged.
- Passing checks validate diagnostic integrity and ownership, not Effects settling or human usability. No new window, new commit, authored-data edit or final compiler matrix.

## Options and trade-offs

1. **Copy Resources' pause/startup behavior.** Small change, but can hide motion rather than solve it. Startup alone cannot explain failure after 4096 steps. Not recommended as Effects fix.
2. **Make Neighborhood universal default.** Existing mechanism, no new solver. Improves measured Mario Effects relative to Native but still moves. Changes reviewed default policy and may hide Native defects. Not adopted.
3. **Use matched traces to isolate dense-hub contact behavior — recommended.** Comparison eliminates grouping, duplication, startup-only and size-weighting-only fixes for this reproducer. Keep production runtime until a focused contact repair passes reference regressions.
4. **Change shared force/contact model now.** May address dense-graph behavior, but risks accepted layouts and lacks isolated cause. Defer until matched evidence identifies a justified change point.

## Next bounded repair investigation

Target shared rectangular contact behavior under dense fan-in, not another default/seed or larger global candidate budget. Reuse existing `query-card-hub` and `dense-cards` fixtures in `native/test_layout_baseline.c`; retain real 44-node / 68-edge Effects reproducer.

- Distinguish exhausted separation passes from exhausted per-node movement. Preserve current hard budgets while measuring.
- Isolate repeated spring/contact compression before choosing correction. No freeze/pause or hidden seed switch presented as convergence repair.
- Candidate must reduce real Mario Effects motion without new overlaps, retain finite/work bounds and preserve both-example source/query/selection ownership checks.
- Re-run matched reference traces and existing noun motion regressions before retaining any shared change. Source grouping, typed identities and all layouts stay intact.
- Compact/selection-dependent cards remain UX review work. No new force model or default policy chosen by this comparison.

## Follow-on trial

User chose workspace bounds first, then render-only filtering. Optional containment prevents escape but increases Mario Effects motion and overlap. Default remains off; no solver-convergence claim. Shared reference diagnostic now includes bounded profiles; original comparison artifacts remain unchanged. [Trial contract, measurements and review boundary](shared-force-layout-workspace.md).

## Comments

Earlier diagnosis evidence remains in `native/build/effect-spacing-work/`. Current evidence is under `native/build/layout-reference-work/`. Comparison adds test diagnostics only; production runtime unchanged. Scope approval concerns investigation, not adoption of architecture or policy.
