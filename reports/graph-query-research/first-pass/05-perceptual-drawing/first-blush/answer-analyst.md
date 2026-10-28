# Perceptual graph drawing — first blush

## Provisional answer

Use task-specific drawings, not one universal force layout. Prioritize five techniques:

1. Keep dependency/route flow consistent, with explicit exceptional links.
2. Preserve traceable paths; avoid false node incidence, not merely crossings.
3. Encode relation meaning and uncertainty redundantly; reserve room for readable identities.
4. Make spatial groups truthful and labelled.
5. Treat dense-graph overview as a different representation problem, not a zoom-to-fit problem.

Strongest empirical evidence concerns abstract path tracing and density trade-offs, not game-design decisions. All proposed changes below are **adaptation hypotheses**, not measured improvements in this workbench. No topology establishes learning, mastery, quality, causality or physical reachability.

Mode: bounded orientation/source discovery. Eight unique candidate URLs; two full research PDFs, three research abstracts, two official guidance pages, one inaccessible PDF. Material passages and access limits: [evidence-extracts.md](evidence-extracts.md). No figures copied, application run, code change or usability test.

## Repository boundary

Paths below are repository-relative; line ranges were actually read. `N` means `skills/game-design-workbench/native/`. `C` means `skills/game-design-workbench/examples/crate-puzzle/game-design/`. `P` means `skills/game-design-workbench/examples/little-plumber-guy-nes/game-design/`.

- Skill prerequisites are AND sets, not alternative routes. Reverse links derive unlocks; authored rank is not depth. Sources: `C/skill-atoms/.schema.md:1–11`, `.usage.md:1–9`. Native consumes prerequisites/mechanics specifically: `N/gd_core.c:380–384`.
- Skill SCCs, incomplete ancestry and display layers already exist: `N/gd_skills.c:31–56,58–150,161–194`. Do not propose those as new analyses. Missing ancestry does not become a valid root; descendants of cycles are not necessarily cyclic.
- Challenge schema declares more links than current challenge reader consumes. Schema: `P/challenges/.schema.md:1–19`; reader: `N/gd_core.c:380–384` consumes skill-role fields and `next`, not all challenge mechanics/noun/resource fields. Do not build new drawing semantics on assumed reader coverage.
- Challenge layout already uses shortest entry-hop bands, not topological teaching order: `N/gd_routes.c:207–246`. Existing directed layout keeps parent-based lateral anchors and separate root lanes: `N/gd_layout.c:48–99`.
- Resource preview uses force layout, straight lines, colour-coded relations and 23-byte truncated canvas labels: `N/gd_resource_view.c:30–42,140–157`. It displays declarations, not computed routes/composed outcomes; source path/line and incident declarations appear in details, without source navigation: `N/README.md:34–46`, `N/gd_resource_view.c:56–101`.
- Operation caps are 4,096 records and 32,768 edges/pairs, not author-data limits: `N/gd.h:7–13`. Existing layout is not evidence that this many records remain readable.

## 1. Flow-aligned layers, with visible exceptions

**Question:** Which prerequisites converge on this skill? Where do routes branch/rejoin without suggesting false sequence?

**Concrete drawing hypothesis:** Keep crate `push`, `plan-push`, `coordinate` chain aligned in one lane; bring `route-switch` into `coordinate` from a second lane. Both prerequisite arrows must remain visibly required, ideally with an `ALL` cue rather than an unlabeled fork that looks like OR. Display `coordinate`'s authored rank 2 separately from derived placement; never move source rank silently. Declarations: `C/skill-atoms/push.md:8–10`, `plan-push.md:8–10`, `coordinate.md:8–11`, `route-switch.md:8–10`.

For plumber `stair-fork`, arrange `high-stair` and `low-path` as parallel branches converging toward `final-gap`; branch semantics follow `next`, not prerequisite semantics. `P/challenges/stair-fork.md:9–11,29–31` and `high-stair.md:9–10,29–31` support this illustrative branch/rejoin. Keep layout band labels explicit: shortest entry hops are not physical distance, time, authored rank or guaranteed chronology. Cycle/backlink routes need side channels or return arcs, not deletion to force a DAG. No cyclic example was established in records read.

**Method/evidence:** Graphviz `dot` supplies authoritative layered-method precedent [E6]. Pohl et al. explicitly compare force, orthogonal and hierarchical layouts by task accuracy/time [E3], but retrieved abstract does not prove a winner. Layered approach here follows known relation semantics, not an asserted experimental victory.

**Data and visible result:** Prerequisite AND sets, ranks, `next`, entries, SCC/incomplete flags and layers already exist/are consumed. Lateral ordering and feedback-channel routing derive from those links; no new author field needed. Drawing must preserve every declared edge. Existing skill link/source inspection can verify prerequisite declarations; challenge route/source inspection can verify `next`. Expose feedback marker and `UNKNOWN ANCESTRY`/`CYCLE` text on graph, not only inspector.

**Limits/failures:** Same-rank prerequisites remain valid; sorting solely by rank can invert semantics. Entry-hop bands can place a long-route edge backwards or within a band. A side channel is not evidence of a bad route. ALL cues are proposed presentation, not added domain nodes. Missing reference markers must not invent loaded target records.

**Scale/confidence:** SCC/topological traversal uses linear graph passes before sorting; existing code already performs these. Crossing reduction/feedback routing adds work and must be bounded; no timing measurements. Confidence: high semantic fit, moderate drawing hypothesis, low empirical transfer to this corpus.

## 2. Optimize path continuity and avoid false incidence

**Question:** Can reader trace a route or prerequisite chain without accidentally switching edges or mistaking a crossing for connection?

**Concrete drawing hypothesis:** Keep plumber stair branches smooth and visually separate until real rejoin. Draw `bonus-return` → `shell-lane` as one-way connection, not a symmetric corridor or implied back edge. The record explicitly denies an arrow back to cellar entrance: `P/challenges/bonus-return.md:9–10,27–29`. This is authored design, not measured traversal.

Give long edges clearance around unrelated nodes and label boxes. Offset parallel/opposing edges; use a genuine junction only at a real shared endpoint. Preserve arrow tips outside node silhouettes. Do not remove a crossing by making important paths excessively zigzagged. Orthogonal routing is an option, not automatically the best choice; extra bends can harm continuity.

**Method/evidence:** Ware et al. [E1] studied 43 CS/IS students on 42-node spring-layout shortest-path tasks, paths length 3–5. Continuity and crossings on task path mattered; total drawing crossings did not significantly predict response time in their model. Seven drawings were excluded because edges under nodes looked attached. This directly challenges “minimize all crossings first.” Figures 2, 4–6 in linked PDF illustrate relevant issues.

**Data and visible result:** Endpoints and direction exist; geometric node/label bounds and routing channels are drawing-derived. Current general renderer draws straight segments and arrowheads: `N/gd_graph_view.c:160–185`; current layout's root-lane heuristic already tries to avoid unrelated nodes lying on long edges: `N/gd_layout.c:79–85`. Improvements must change paths on canvas; existing edge identity/source lookup should still reach original declaration after path acquires bends.

**Limits/failures:** Smooth shared trunks can imply nonexistent shared semantics or hide multiplicity. Graph-wide bundling is not recommended for prerequisite tracing in this pass. Ware's regression is task-specific; do not reuse its numeric time coefficients. Abstract spring drawings do not prove a universal routing style for directed game graphs.

**Scale/confidence:** Naive every-edge/every-node collision testing scales as E×V; use bounded spatial candidate tests if explored. Routing and label avoidance can conflict with stable placement. No implementation claim or measured performance. Confidence: moderate/high on false-incidence risk; moderate on proposed route geometry.

## 3. Redundant relation encodings and readable identities

**Question:** Does line mean condition, write, preservation, invocation or role binding? Can meaning survive missing colour perception and dense labels?

**Concrete drawing hypothesis:** For crate `free-push`, distinguish Energy Amount condition from quantity write even when both connect same facet/case. Label them `requires` and `set`; use distinct patterns/ports plus colour. In `insufficient-energy`, show `preserve` rather than a flow/spending arrow. Rules: `C/interactions/free-push.md:32–45`, `low-energy-push.md:33–45`. Role binding must read `binding`, not imply resource transfer. Do not fabricate shared `player-energy` identity: these two rule blocks name Energy actors but do not explicitly bind their pools. `C/resources/player-energy.md:3–11,16` supplies pool metadata, not enough to equate every similarly named rule participant.

Use distinct case/facet/role silhouettes or category text. Current resource nodes already include category text, but relation lines rely on colour and straight segments; legend alone does not disambiguate coincident lines. Keep skill `CYCLE`, `UNKNOWN ANCESTRY` and rank-conflict text: these redundant warnings already exist (`N/gd_graph_view.c:217–224`).

For labels, reserve measured text bounds; allow wrapped identity labels with an explicit truncation cue. Keep exact qualified identity accessible on inspection. Avoid interpreting node size as importance unless metric and legend are explicit. Font family/size optimum remains unknown.

**Method/evidence:** W3C guidance requires visible alternatives to colour [E7]; adapted to desktop, not a WCAG compliance claim. Holten/van Wijk establishes need to evaluate directed-edge representations [E4], but abstract does not substantiate search-result claim that tapered edges win. Do not replace arrows on that basis.

**Data and visible result:** Relation kind, source line, actor/facet and case identity already exist. Resource graph retains binding identity cautiously (`N/gd_resource_graph.c:31–69`) and relation kind/source line (`:72–83`). Patterns, ports and label placement derive from retained data; no schema change. Resource detail panel can corroborate source path/line and relation kind, but no source-opening workflow currently exists. Proposed new direction marks must be keyed to relation semantics, never blindly facet → case for every relation.

**Limits/failures:** Too many patterns are another code to memorize. A visually identical `Amount` label across unbound actors can falsely imply one pool. Byte-truncated labels can collide or split UTF-8; current resource canvas uses `%.23s` (`N/gd_resource_view.c:153–154`). Typography/collision proposals lack direct comparative evidence in retained corpus.

**Scale/confidence:** Relation styles are small per-edge rendering work; text measurement and collision placement can dominate dense views. No pixel-size or frame-rate promise. Confidence: high accessibility rationale; moderate semantic display fit; low on exact label layout.

## 4. Truthful spatial grouping and alignment

**Question:** Are nearby nodes genuinely related, or merely pushed together by layout?

**Concrete drawing hypothesis:** In crate prerequisites, visually distinguish the `push` → `plan-push` lane from `read-switch` → `route-switch`, then show convergence at `coordinate`. Do not enclose those lanes as disconnected components: `coordinate` links both. Aligning ALL inputs helps reveal conjunction; labelled light regions may distinguish relation families, but do not claim these are learned mental categories. For resources, separate rule-case-local facets unless explicit pool binding supports identity. Never merge all `Energy`/`Amount` names into one attractive hub.

**Method/evidence:** IBM-hosted original abstract [E5] reports people can recover masked cluster structure and use edges to delineate groups. This is user-created layout behavior, not measured speed/accuracy improvement from our enclosures. Spatial proximity communicates relationship; using it without provenance risks a false message.

**Data and visible result:** Weak prerequisite components already exist; do not equate them with mechanic families. Mechanic links are consumed for skills, so a separate labelled mechanic grouping is possible, but multi-membership needs explicit handling. Actor/case identities and explicit bindings already exist in resource graph. Show group title and basis on canvas; retain original cross-group edges and declaration inspection. No inferred design taxonomy required.

**Limits/failures:** Multi-membership, overlap and heavy boundary decoration can hide cross-links. Spatial separation can overstate weak differences; geometric closeness is not similarity score. Overview grouping must not change underlying graph. Source abstract omits graph/participant counts, so no quantitative spacing prescription.

**Scale/confidence:** Group traversal and stable ordering are tractable bounded preprocessing; packing overlapping groups and labels needs additional design. Dense cross-group edges may erase benefit. Confidence: moderate perceptual rationale, low on measurable benefit for these fixtures.

## 5. Dense overview: preserve task fit, reject universal node-count cutoff

**Question:** When does graph stop supporting actual reading even if every node fits viewport?

**Concrete drawing hypothesis:** Keep sparse crate prerequisite paths as node-link. For a future dense prerequisite/actor-incidence corpus, consider a labelled adjacency/incidence matrix overview alongside a node-link path drawing. Matrix blocks could reveal shared prerequisite neighborhoods; node-link detail retains paths and AND semantics. Current native matrix represents noun/verb declared cases, NOT an existing prerequisite adjacency matrix (`N/README.md:87–94`). New projection would be work, not a hidden existing feature.

At distant zoom, drawing should show labelled structure/group counts rather than illegibly shrinking every label. Exact on-demand overview/detail interaction belongs to sibling visual-exploration lane; this lane's concern is the geometry and truthful aggregation. No new aggregate metrics or hidden-edge policy selected here.

**Method/evidence:** Ghoniem et al. [E2] compare seven tasks, 36 CS participants, nine random undirected graphs of 20/50/100 nodes. Matrices helped several search/adjacency tasks as complexity increased. Detailed path results have dense-case exceptions to abstract's node-link advantage. Figure 1 compares same 50-node/400-edge graph in both drawings.

**Data and visible result:** Adjacency derives from resolved declarations; matrix cell must retain direction, type and original declaration identities. Unknown endpoints need an explicit unresolved list/boundary indicator, not silent zero cells. Resource incidence may distinguish facets/cases but must not merge unbound actor identities. Sparse topology can remain sparse in storage, even if full matrix drawing is infeasible.

**Limits/failures:** No universal “switch above 20 nodes” rule. Study used neato, undirected synthetic graphs, alphabetical matrix order, interaction and training. Both representations struggled with some path tasks. Matrix can obscure conjunction, paths and labels; grouping can conceal important exceptions. Attractive overview is not demonstrated comprehension.

**Scale/confidence:** Full matrix has V² cells; 4,096 records already imply 16,777,216 possible cells, regardless of sparse edges. Beyond native bounds, virtualization/aggregation and retained identities matter more than drawing all cells. No data cap recommended. Confidence: high that density/task trade-off matters; low on switch threshold and exact overview for this corpus.

## Prioritized follow-up

1. Compare drawings on actual tasks: identify both prerequisites of `coordinate`; trace plumber branch/rejoin and one-way bonus return; distinguish crate energy condition/write/preservation. Measure correctness and time, separately from preference. No experiment run in this lane.
2. Retrieve full directed-edge and task-layout studies before selecting tapered arrows, orthogonal routing, or a claimed empirical layout winner. Purchase mirror remains HTTP 403; locate authorized accessible original in deeper pass.
3. Find empirical label-placement/typography and aggregation evidence. Retained corpus does not settle font choice, label priority, enclosure style, motion stability or semantic zoom.
4. Examine larger directed, cyclic and typed/multirelation graphs with comparable tasks; current empirical examples stop at 100 nodes. Preserve incomplete ancestry and unbound-pool semantics while testing scale.

## Checks, gaps and handoff

- Read brief/assignment completely before discovery; selected mandatory AI-assisted guide. Formats read; no full register, capture queue or extra agents created.
- Journal appended during work; CSV preserves repeated observations and eight-URL ceiling. Only lane folder modified. No code/app/source-record changes or browser automation.
- Material public claims have exact passages and page/section locations in companion extracts. Research abstracts remain abstract-only; no fabricated sample sizes or winners.
- Repository citations restricted to read ranges. Example claims describe authored records, not player measurements. No screenshots assessed as usability evidence.
- Main overlap for synthesis: sibling visual-exploration lane owns selection/filter/overlay/explanation workflow; this lane owns path geometry, grouping cues, label legibility and redundant encodings.
- Deeper pass could change edge encoding, task-layout choice and density strategy. It is less likely to overturn need for truthful direction, explicit uncertainty and non-colour-only meaning. No exhaustive evidence answer claimed.

## Comments

No current blocker to bounded orientation. Evidence gaps and one HTTP 403 remain; no access bypass attempted. Recommendations are shortlist candidates for human review, not implementation approval.
