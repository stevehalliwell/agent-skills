# Actor/Effect Free placement — Step 5

Status: implementation checkpoint authorized; visually reviewed, not accepted. Follows reviewed Step 4 checkpoint `a99c028`. User chose to keep stacked ghosts together. Pause here; improve default visual communication on resume.

## Result

Add native/free placement to unified Actor/Effect panes. Retain native cards/bands/wrapped contributor rows. Free uses shared solver and Native/Neighborhood initial arrangements, not another evaluator or per-view solver.

```text
presentation.layout: native | free
presentation.initial_arrangement: native | neighborhood
```

- Native remains default. Arrangement control applies only to Free scene.
- Layout changes preserve applied query, pending inputs, exact selected node/link, typed identities, contributor target/evidence, comparison, filters, draft and Undo.
- No query Run/Display, contributor rebuild, source refresh or analysis during geometry changes.
- Native restores retained scene's starting coordinates, not geometry derived from pending display/query inputs.
- Free allows roots and other spatial elements to move on both axes. Preserve source arrows, widths/weights, excluded state, partial-result flags and exact typed node/edge metadata.
- Run, Display, contributor retarget and refresh prepare selected geometry on candidate before replacing prior scene. Failure retains last-good query/display/selection and layout settings.
- Existing native ghost display and hit priority remain unchanged. Free preserves intentional stacks as defined below.

## Source grounding

- `GD_QueryScene` owns inline `GD_Layout`, labels and typed node/edge metadata; borrows query and optional effect scope. Pane layout points into scene. Ordinary graph replacement must not free that inline pointer or replace scene solely to reposition it.
- `gd_query_scene.c:build_scene()` creates directed geometry, optionally wraps Effects context, then offsets stacked variant cards by 26 units. Other variant-scoped cards retain separate positions and ghost styling.
- `gd_explorer_query_prepare()` preserves cold pending inputs by preparing default applied scope. Keep this behavior.
- `build_state()`/`focus_result()`, `gd_explorer_query_display()`, `gd_explorer_query_select_result()` and remap paths each create/replace scenes. Geometry must be applied atomically at those ownership boundaries.
- Query rendering/hit testing uses scene node order, typed metadata, card dimensions and reverse painter priority. Geometry cannot merge typed variant/activation/effect identities or change direct contributor counts.

## Stacked ghosts

Keep applied stacked variant cards together in Free. Settling or dragging any member moves visual stack as a unit, including while paused. Variant identities, individual exposed-strip selection and evidence remain distinct. Other ghost-styled activation cards remain independent; only intentionally overlapping variant cards form compound.

Retain native pose on scene. A private spatial layout maps typed scene nodes to geometric units and uses shared force solver; singleton units remain one-to-one. Compound footprint unions member cards with native offsets. Folded duplicate assumption springs must not give ghost copies extra convergence weight. Typed scene nodes/edges and contributor counts remain untouched. No new game-domain identities.

## Visual review and continuation

User reviewed Mario in new viewer PID 15832 from `native/build/query-free-work-ui/gd-view.exe`. Opened at Actors / Free graph / Neighborhood and Fit through guarded IPC; no source/draft change or OS focus automation.

Findings:
- Effects pulls into centre and vibrates against surrounding cards. Default composition and settling need work.
- Actors shows grid of root nodes with no visible connecting edges. Default view does not communicate useful relationships.
- Both views need more visually communicative defaults. Mechanical/CPU success is not visual acceptance.

User requested record findings, commit checkpoint and stop for later pickup. No further layout correction attempted.

Next slice: investigate default applied scenes and rendered geometry for Actors/Effects, then refine a visual-communication approach before coding. Distinguish absent topology from hidden/misleading scope and presentation. Diagnose Effects centre attraction, footprint clearance and persistent motion with evidence; no cause or replacement algorithm selected. Do not fabricate edges or automatically apply staged query inputs to make a default look connected. Preserve native layouts and agreed stacked-ghost movement.

## Technical constraints

- Reuse shared solver; no crossing logic or dependency.
- 4096 nodes / 32768 edges; bounded finite preparation and stepping, no per-frame allocations.
- Preserve shared operation-scratch ceiling. Measure and disclose any additional owned scene/geometry storage separately.
- Geometry candidates and native-pose storage have explicit owner lifetimes. Never copy scene label pointers into storage that outlives scene.
- Clear velocities/resume settling after successful arrangement change. Pause/drag must not fight intended ghost presentation.
- Invalid input, work exhaustion or allocation failure leaves old scene and pending/applied state intact; no mutation retry.
- CJIT only until final Step 6. User review required before final acceptance; CPU checks do not prove readability.

## Acceptance

- Same retained query and typed scene evidence before/after native/free and seed changes; exact selected representation retained.
- Pending query/display edits stay pending through geometry changes.
- Native restoration matches initial applied scene, including wrapped Effects context and native ghost offsets.
- Free uses actual links/shared elements without identity merging or ghost-generated contributor weights.
- Selected ghost policy remains usable under settling, pause, primary/ghost dragging, Display and native restoration.
- Empty/unavailable scene, partial results, sized cards, comparison, contributor retarget and every allocation-stage failure checked.
- Fresh build directory; do not rebuild running review executable. No window launch/manipulation without authorization.

## Implementation and verification

- `gd_query_scene_free.h` owns private spatial projection; `gd_query_scene.c` captures/restores native pose and frees private geometry with scene. Shared scene still owns typed inline layout; no independent free of pane's borrowed layout pointer.
- `gd_explorer_query_view.c` applies geometry at scene-candidate boundaries and routes dragging through compound map. Explorer shell handles layout/arrangement controls, Pause/Space and shared stepping. IPC uses same transitions.
- Source/typed nodes and edges remain one-to-one in scene. Private units fold only applied stacked variant cards. Duplicate folded springs cannot add extra weight; canonical parallel source springs remain. Hash insertion capped at 262144 visits; exhausted candidate fails atomically.
- `GD_Layout` remains 2048088 bytes. Scene becomes 3375320 bytes (32808-byte native pose/ownership increase). Free scene adds 2064472 owned bytes for spatial layout/map; native scenes have no private spatial allocation. Old/candidate spatial layouts coexist during guarded switch.
- Temporary edge table 786432 bytes, released before arranging/seeding. Neighborhood workspace 524288 bytes; collision scratch remains 114688 bytes. Conservative operation-scratch bound 901120 bytes, below 1MiB. Owned scene/candidate storage counted separately. No per-frame allocation.
- CJIT core **136/136**; Node **21/21**, zero skipped. Both example adapters: **254 IPC +2232 controls +105 configuration +10 feedback**; presentation **562 Crate /1953 Mario**. Graph, CLI and protocol helper build. No warnings/errors in retained build logs.
- Core checks use real Actor all-variant/stacked/empty-limited scenes and weighted Effects contributor context: deterministic Native/Neighborhood, finite settling, native restoration, both-axis stack dragging, held-unit invariance, exposed-strip hit before drag, typed metadata/count/label preservation, invalid options and every private allocation stage.
- Adapter checks cover geometry-only query/scope/selection/pending preservation, both choices, native restore, every Free-switch/Display candidate allocation stage, explicit Display/Run in Free and Effects retarget in Free. Existing cache/draft/reset/refresh tests remain. Does not establish exhaustive Free-specific refresh/control or real-pointer acceptance.
- Evidence: `native/build/control-work/query-free-{core,node,crate-puzzle,little-plumber-guy-nes}.*`. Fresh outputs: `native/build/query-free-work{,-ui}`. Authorized review viewer subsequently launched; findings above.

## Comments

Test snapshots use host heap rather than consuming shared 64MiB fixture arena alongside retained scenes and old/candidate spatial layouts. Ghost hit check runs after settling, before deliberate drag into other cards: reverse-painter selection correctly gives a covering opaque card priority during temporary overlap. No change to hit priority or claim of instant post-drag clearance.

One overlapping helper build/Node run hit Windows DLL lock; reran builds/adapters/Node sequentially. No user viewer or review executable touched. Temporary diagnostics removed.

Crossing reduction remains unresolved and must be revisited before final review. Actors' finite assumptions and Effects' declaration associations are not temporal simulation or causal verdicts.
