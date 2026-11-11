# Actor/Effect Free placement — Step 5

Status: checkpoint `a2537f2` committed after guided review. Actor overviews and noun interactions implemented; UX feedback retained in `todo.md`. Effects crowding/motion reproduced below, not fixed or visually accepted. User chose to keep stacked ghosts together.

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

Next slice: refine confirmed Actor interaction-map default below against source representation before coding. Diagnose Effects centre attraction, footprint clearance and persistent motion separately; no cause or replacement algorithm selected for Effects. Do not fabricate edges or automatically apply staged query inputs to make a default look connected. Preserve existing queried layouts and agreed stacked-ghost movement.

### Confirmed Actor overview directions

Replace list-like default inventory canvas with weighted actor interaction map; add separate actor usage-cloud view. User accepted both angles and record-level interaction counting. Existing explicit Actor queries retain their current purpose and behavior. These are new content projections, not merely different placements of existing query scenes.

#### Actor interaction map — default

Question: which actors share the most authored interaction content?

- Show all qualifying actor types, including isolates. Use explicit noun `is_actor: true`, not all nouns. Lives is a supporting resource concept, not an actor. Missing classification stays unclassified, never silently false; overviews must disclose unclassified scope.
- Connect actor types through sourced interaction evidence. Count each distinct interaction record once per actor-type pair, not once per conditional case. Retain contributing records/cases and conditions for link inspection.
- Draw larger counts with thicker links. Default force graph uses stronger attraction for higher weights; preserve readable node separation rather than collapsing related types into overlapping piles.
- Proximity expresses shared interaction declarations, not similarity, runtime frequency, gameplay importance or proven execution.

#### Actor usage cloud — separate view

Question: which nouns/concepts and axes connect actors across the design?

- Place all qualifying actors around an outer ring. Place used non-actor nouns/concepts and axes in the middle; actors are not duplicated as central concept nodes by implication.
- Link actors to central nodes through declared interaction/usage evidence, not incidental prose mentions. Lives can appear centrally without being classified as an actor.
- Grow central node size with connection count, like a word cloud. Keep actors on ring; central nodes may settle spatially. Actor proximity on ring does not encode relatedness.
- Distinguish actors, central nouns and axes visually. Bound node sizes to preserve readability.
- Selecting actor highlights its usages; selecting central concept highlights actors using it. Link inspection exposes contributing declarations and conditions.
- Central size uses distinct connected actors (breadth), labelled explicitly. Interaction-record count remains link evidence, not the size metric.

#### Preserved behavior and open refinement

- Explicit case/state queries replace overview with existing query analysis. Do not implicitly Run queries or consume pending inputs to populate overviews. Preserve source browsing, drafts/Undo and last-good failure behavior.
- Actor-wide state-machine approximation remains separate deferred work in `todo.md`.
- Confirmed extraction: explicit same-case participation in interaction records only, with record-deduplicated pair counts. No cross-case pairing, inference from prose or automatic referenced-mechanic/call expansion. Cloud connects actor to other non-actor/unclassified noun participants in those cases and to that actor's own declared axes; unclassified concepts remain visibly distinct. Shared-case use does not mean direct modification. Existing source statuses/coverage remain evidence, not gameplay frequency or applicability proof.
- Implementation slice: bounded log-scaled widths/attraction and concept sizes; deterministic record-order outer ring and central packed concept cloud. Cloud is a fixed overview, not another physics solver; central settling is optional future polish. Overview switching must preserve pending inputs and existing case/state query behavior.
- Open: classify remaining example nouns under agreed project policy. Self-type participation does not create an inter-type pair; no inferred runtime instances.

### Actor classification foundation — implemented

- Optional `is_actor` lives on canonical noun; no actor collection or duplicate state list. Axes retain state vocabularies/types and applicability; rules retain conditions/changes.
- Native model retains boolean plus declaration line; missing line means unclassified. Supplied null, strings, numbers, collections and duplicate fields fail loading. Existing queries accept all noun classifications.
- Participant query JSON exposes `is_actor: true | false | null`; unresolved nouns also report null with separate resolution status. Inventory inspector shows classification without filtering inventory.
- Shipped noun schema/usage and both example contracts updated. Attendant lacks optional typed booleans: schema declares optional/untyped field without default, native reader enforces semantic type. Generic schema validation alone is insufficient.
- Mario Hero/Walker explicitly true; Lives explicitly false, updated through Attendant. Other nouns remain unclassified, not guessed from prose. Overview implementation follows below.
- Checks: CJIT core 138/138; focused query tests 4/4; full native Node suite 62 passed, zero failed, one Mac-only socket test skipped. Includes Windows transport and both CPU production adapters. CLI/graph/explorer/protocol fixtures build in separate `native/build/actor-classification-work/`. Strict Attendant validation: Mario 225 records and Crate 67, zero diagnostics/corrections. Mario binding query confirms Hero/Walker true and Lives false.
- Evidence in that build directory: `build-core.log`, `native-node.log`, `attendant-update.json`, `validate-{crate-puzzle,little-plumber-guy-nes}.json`, `mario-bindings.json`. Running review viewer untouched; no new GUI launched. New inspector text compiled and exercised by CPU adapters, not visually accepted; Mac build not rerun for this slice.

### Actor overviews — implemented, visual review pending

- Default interaction map and separate usage cloud follow confirmed extraction above. Rule snapshots normalize record roles; extraction checks restored original collection role, not normalized `GD_Record.role`. Only bound interaction collections contribute pairs. Mechanics remain inspectable query evidence, never automatic pair expansion.
- Shared weighted solver multiplies spring force by `1 + min(3, log2(record_count))`; unweighted callers retain existing behavior. Width and concept dimensions use bounded log scales. Usage uses fixed ring/grid; no additional solver or per-frame allocations.
- New owned metadata: 131,084 bytes per overview; construction scratch: 446,464 bytes, released before Free placement. Existing scene/native-pose and Free geometry ownership remain unchanged. Construction uses existing 4096-node/32768-link and charged-work limits; partial scopes are labelled.
- Shared presentation `actor_overview: interactions | usage` works from overview or analytical scene. Pending Query inputs never run implicitly. Switching preserves source selection even when definition is absent from new canvas. Allocation failures retain previous scene/mode. Generic layout unavailable during overview; interaction map retains Native/Neighborhood, Pause and dragging. Usage is fixed, without dragging/settling.
- All nouns remain searchable and queryable regardless of classification. Current Mario map has Hero/Walker; Lives remains non-actor concept in usage. Remaining Mario nouns and all Crate nouns stay unclassified. Empty overview explains explicit classification requirement.
- Retained IPC `result --projection actors --form actor-overview` exposes nodes, counts and scope disclosures, with first contributing source per link. Whole-result bounded byte paging only; nonempty `--fields` rejected. Existing native Query result retains full cases/bindings; GUI edge inspector pages matching cases/conditions. Typed definition references and analytical scene identities remain separate.
- Checks: CJIT core **140/140**; native Node **62 passed, zero failed, one Mac-only socket skip**. CPU production adapters cover both examples without source writes/leaks. New regressions cover repeated bindings/cases, once-per-record pair counts, no cross-case/self/concept-concept links, isolates, non-actor/unclassified concepts, own-axis deduplication, connected-actor sizing, fixed pose, weighted spring behavior, source-kind exclusion, work limits, allocation failures, retained JSON and overview switching from analytical state without applying pending inputs.
- CLI, protocol fixture, graph and production adapter compile in `native/build/actor-overview-work/`. No new GUI launched; existing review viewer/drafts untouched. Visual acceptance, remaining classifications and Effects crowding/motion diagnosis remain open. Full compiler matrix remains final Step 6.

### Effects motion diagnosis — checkpoint follow-up

- Added CPU production-adapter probe over both loaded examples. Run 4096 steps per Native/Neighborhood seed; measure final 256 steps. Restore native pose before existing geometry/evidence retention checks. Probe asserts finite coordinates and collision work bounds, not settling acceptance.
- Mario default contributor context: 44 nodes / 68 edges. Native: RMS **2.452279**, max step **22.421452**, centroid drift **85.536084**, **4** final overlaps. Neighborhood: RMS **0.692194**, max **3.810456**, drift **27.339965**, **0** overlaps. Both report collision limits. Motion units are world units/step; drift spans final 256 steps.
- Crate default context: 3 nodes / 2 edges. Native/Neighborhood RMS **0.000059 / 0.000370**, max **0.000076 / 0.000531**, drift **0.001630 / 0.035670**. No overlaps or collision limits. Dense Mario failure is not universal.
- Tested Effects-only soft gaps 250 and 500. Both remove final Mario Native overlaps but retain motion: Native RMS **1.001501 / 0.201472**, drift **5.027779 / 15.159576**. Larger gap with Neighborhood reaches RMS **0.027914**, max **0.219240**, drift **0.393799**, still reports collision limit. This does not fix Native or establish readable composition.
- Constant 450-unit radial spring trial does not resolve Native motion. Relative-compression velocity projection reduces centroid drift but worsens Effects RMS and Resource motion. Removed all trial runtime changes; do not ship these as fixes. Original solver, spring settings and defaults remain unchanged.
- Root cause remains unresolved. User chose Resources/Verb usage comparison before further tuning; matched measurements now complete. Effects has one connected component, no parallel springs and a 25-neighbor hub; unweighted display alone still moves. Next isolate dense-hub contact/pass/movement limits. [Matched findings and next repair investigation](shared-force-layout-reference-review.md). No new solver/default policy selected.
- Checks: CJIT adapter rebuilt; focused CLI/both-example production-adapter tests **5/5**. Separate baseline probes repeated against both private example copies. Source hashes and owned allocations preserved. Checkpoint core **142/142**; no subsequent core-runtime changes. Existing review viewer untouched; no new window or compiler-matrix run.
- Evidence: `native/build/effect-spacing-work/{mario-baseline,mario-gap250,mario-gap500,mario-radial450,mario-relative,mario-final,crate-final}.{log,json}`, `build-final.log`, `adapter-final.log`. Effects-only probe subsequently consolidated into reusable matched reference diagnostic; see comparison report. Only tests/diagnostics and documentation changed after checkpoint; no additional commit made.

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
