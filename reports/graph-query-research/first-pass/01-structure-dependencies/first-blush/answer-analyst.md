# Structure/dependencies — first-blush orientation

## Provisional answer

Prioritize **AND-aware transitive foundations**, **rooted route dominators**, and **existing route-opportunity coverage surfaced graph-wide**. Use **components/SCC regions** as structural scaffolding. Keep **degree/betweenness** as explicitly labelled exploration, not an importance score. General minimum cuts and modularity communities should not lead the shortlist.

These are recommendations and adaptation hypotheses, not measured usability or player-learning results. Eight unique public candidate URLs, nine observations; six successfully read sources, two fetch failures. Public extracts and repository locations: [evidence-extracts.md](evidence-extracts.md). No implementation, benchmark, experiment rerun or graph-output verification performed.

## Semantic boundary and available data

Two graphs must remain separate:

- Skills: declarations point dependent to prerequisite; analytical arrows reverse to prerequisite → dependent. Every prerequisite in the array is necessary under authored AND semantics. A path is not an alternative acquisition plan.
- Routes: challenge `next` links are alternatives. A predecessor reachable by some path is not necessarily mandatory. Cycles can represent revisits; analyses must not enumerate infinitely many walks.
- Heterogeneous context: mechanics, skill roles and route edges can share records but must not silently become one interchangeable network. Always show projection, direction, statuses, supplied folders and unresolved references.

**Observed local boundary:** crate skill/challenge schemas declare the required skill and route fields; native reader consumes them. Challenge schema also declares mechanics/interactions/nouns/verbs/resources, but challenge-role native parsing only accepts skill-role links and `next`, plus `entry`. Loading related folders does not make those extra schema fields consumed links. See `native/gd_core.c:361-387`, crate `challenges/.schema.md:1-21` and `skill-atoms/.usage.md:1-9`, under `skills/game-design-workbench/`.

All examples below are authored fixtures, not reverse-engineered game facts. Unknown gates, physical geometry, actual play probabilities and observed understanding remain genuinely missing. Status `proposed` must stay visible for plumber method examples.

## 1. Shared foundations and AND convergence — first priority

**Question:** Which foundations feed several methods or combinations, including indirect dependencies hidden by local degree?

**Concrete observation:** crate `coordinate` requires **both** `plan-push` and `route-switch`; `plan-push` requires `push`, which requires `move`; `route-switch` requires `read-switch`. These declarations create five distinct ancestors of `coordinate`. They do not offer a choice between planning and switch routing. Plumber `jump-stomp` and `fall-stomp` both name accepted `stomp-walker`, adding different second prerequisites; both methods remain proposed. The shared foundation does not imply either method teaches the other.

**Method/evidence:** ancestor/descendant traversal, using prerequisite → dependent projection. S4 defines ancestry as all nodes having a path to a node. Count distinct downstream dependents and challenge demands reached through these chains; retain AND groups at each combination. Run graph-wide rather than requiring selected pairs. Rooted path drill-down explains each count.

**Data:** prerequisite/mechanic fields already consumed; reverse adjacency and distinct transitive counts derivable. Challenge demand closure already exists in `gd_routes.c:64-108`. No new authored unlock list needed. Optional learning evidence remains absent, not derivable.

**Visible adaptation hypothesis:** node badges show direct versus transitive dependent counts, not “importance”; highlight shared `stomp-walker` and both method branches. At `coordinate`, draw an AND brace on the two incoming prerequisite links. Selecting badge highlights supporting chains, with source-field/line declarations and statuses. No fabricated OR connector between methods.

**Limits/failures:** counts depend on authored decomposition, loaded scope and statuses. Cycles or missing ancestry make necessity/completeness claims unsafe; report known reachability plus incomplete boundary, not independent roots. Shared mechanic is evidence of shared declaration, not a prerequisite.

**Scale/confidence:** adjacency traversal is O(V+E) per root; all-root counts by repeated traversal are O(V(V+E)), an engineering bound rather than measured timing. Consider streamed top findings before full closure storage; bitsets on SCC condensation trade memory for repeated-query speed. High confidence in semantics; medium confidence in graph-wide UX value.

Repository anchors: crate `skill-atoms/{coordinate:8-13,plan-push:8-12,push:8-12,route-switch:8-12}.md`; plumber `skill-atoms/{jump-stomp:7-35,fall-stomp:7-35,stomp-walker:7-25}.md`, all beneath their example `game-design/` folders.

## 2. Route dominators and exit-conditioned convergence — second priority

**Question:** Which challenges necessarily precede a reachable challenge in the declared route model? Where do alternatives reconverge?

**Concrete inference from read declarations:** crate has `move-demo → push-room`; `push-room → combined-room` or `push-room → switch-practice → combined-room`. Therefore `push-room` is unavoidable before `combined-room`, while `switch-practice` is bypassable. `combined-room` is convergence for these two authored branches. This is a hand-checked structural inference, not a run of a dominator implementation.

**Method/evidence:** compute rooted dominator tree; connect a visually distinguished synthetic source to every declared entry when needed. Dominator means a node lies on every source-to-target path; S1 documents rooted reachable results. Post-dominators use graph reversal from a selected exit (S1). With multiple exits, connect declared selected exits to a synthetic sink. Label results as exit-conditioned; nontermination and other exits are outside that claim.

**Data:** `entry` and resolved `next` are consumed already. Domination, alternatives and convergence are derivable. Desired exit policy is not a current field contract; fixture's terminal `combined-room` can be a selected analysis endpoint, not a new authored exit declaration.

**Visible adaptation hypothesis:** toggle a dominator-tree overlay without removing original route edges. Halo mandatory-prefix nodes; show both branches and the bypass on selection. Highlight every declaration needed to inspect result, and expose synthetic nodes in legend. Existing source panel can inspect `next` lines; tree must not replace route graph silently.

**Cuts alternative:** S5 documents minimum-node cuts as minimum-cardinality disconnecting sets, using flows and ordered endpoints. Useful later for “which two alternatives jointly carry connectivity?” but more costly and pair/policy-sensitive than graph-wide dominators. Never substitute undirected articulation points: on the undirected triangle `push-room/switch-practice/combined-room`, `push-room` is not an articulation for those three alone, yet is a required prefix from entry to combined-room.

**Limits/failures:** unresolved route links can invalidate an all-path assertion. Return “known projection only/unknown complete domination” rather than authoritative bottleneck. Domination does not imply forced real-world play, traffic, difficulty or a design problem. Do not apply route dominators to prerequisite AND semantics.

**Scale/confidence:** single rooted analysis avoids named all-pair queries. No implementation complexity or speed claim validated in this pass; original algorithm and C99 fit need follow-up. Repeated removal BFS is a simple O(V(V+E)) reference oracle for a future bounded test, not a recommended production algorithm. Medium-high conceptual confidence; medium implementation confidence.

Anchors: crate `challenges/move-demo.md:8-18`, `push-room.md:8-23`, `switch-practice.md:8-24`, `combined-room.md:8-22`.

## 3. Alternative-route opportunity coverage — strongest existing base

**Question:** Does every arrival at a declared demand have earlier introduction/practice, or only some routes?

**Observed capability:** native `gd_routes.c:64-130,145-200` expands AND demands and uses two-state BFS (support not seen/seen) instead of enumerating routes. It distinguishes support strictly before arrival. README documents `all_routes`, `some_routes`, `absent`, `unknown`, and supported/unsupported witnesses. This is already implemented, not a novel recommendation.

**Concrete inference:** crate `switch-practice` introduces `route-switch`, but direct `push-room → combined-room` bypasses it. Thus prior route-switch opportunity can differ between the two authored routes. `combined-room` also requires `plan-push`; the four read challenge declarations provide no earlier introduction/practice of that skill. Current-challenge `coordinate` introduction is not prior support. These are declarations, not evidence players cannot solve room.

**Data:** already consumed route and skill-role fields plus transitive prerequisites. Existing check/witness results available; graph-wide filter and aggregation are derivable. Missing reference handling already conservatively marks route claims unknown (`gd_routes.c:36-61`).

**Visible adaptation hypothesis:** overview marks demand nodes with some/absent/unknown badges; selecting badge colors supported and bypass witnesses differently, retaining line styles and legend. Preserve existing truncated-witness warnings; show retained suffix as suffix, not whole route. Existing Challenges explorer already draws witness highlights; improvement is discovery across findings, not claiming a new route engine.

**Limits/failures:** per-skill opportunities do not establish whole-player readiness or mastery. Different skills' positive witnesses need not prove one route has every desired prior opportunity. Loops/revisits must not erase unsupported first arrival. Incomplete references and unencoded gates remain unknown.

**Scale/confidence:** current README describes work bounded by `(challenges + demanded skills) * (records + relationships)`, check cap 32,768, witness cap 128 and 256-node suffixes; these are implementation limits, not author-data constraints or measured speed. High confidence in read implementation; UX benefit untested. Parent should avoid duplicating current feature in shortlist as entirely new.

## 4. Components, isolates and SCC regions before thematic clustering

**Question:** What is disconnected in a chosen projection? Which directed regions are mutually reachable, and which nodes merely depend downstream on a cycle?

**Observed base:** native skills code already deduplicates resolved prerequisite pairs, computes weak groups and iterative Kosaraju SCCs, marks cycles separately from affected descendants, and builds condensed layers (`gd_skills.c:19-150`; README skill section). S3 documents directed SCC extraction, but its Tarjan implementation is not local code.

**Concrete adaptation hypothesis:** keep crate `coordinate`'s two branches in one prerequisite component, even though their mechanics differ. Provide collapsible SCC route-loop regions and singleton shelves; do not call one-way chains communities. No route cycle or isolate is asserted for these four read crate challenges. Their simple acyclic example provides no empirical cycle-case validation.

**Data:** existing prerequisite groups/cycle flags; route SCC grouping derivable from consumed `next`. Thematic grouping would require an explicit typed projection and resolution policy; no new labels should be authored automatically.

**Visible result:** hulls labelled “weak prerequisite component” or “mutually reachable route region,” expandable to show all raw links. Isolates remain nodes; unresolved outbound links remain boundary declarations, not erased isolation evidence. Click region to inspect internal cycle edges and sourced incoming/outgoing declarations.

**Counter-evidence:** S6's author abstract reports modularity optimization can miss small well-defined modules depending on overall network size and interconnection. Therefore defer a single “natural design clusters” view; even an attractive partition could conceal authored small families. This does not refute all clustering techniques. Shared ubiquitous mechanics could merge otherwise distinct skill groups if edge roles are mixed.

**Scale/confidence:** SCC/weak traversal can be linear with adjacency; already present for skills. No modularity runtime or quality benchmark acquired. High confidence in structural interpretation, lower confidence in thematic community value. Cross-lane overlap: similarity lane owns families; visual lane owns interaction evaluation.

## 5. Degree/betweenness as inspection aids, not pivotality verdicts

**Question:** Which records connect many declarations, or mediate many shortest structural paths, worth inspecting?

**Method/evidence:** distinguish typed distinct in/out degree from raw declaration count. S2 defines betweenness by shortest paths, with optional sampled source approximation. Native skill code already has distinct direct prerequisite/dependent counts (`gd_skills.c:44-55`); full betweenness was not established as a current capability.

**Misleading attractive metric, actual fixture:** crate's direct `push-room → combined-room` route is shorter than its optional switch-practice detour. For entry-to-combined-room shortest paths, `switch-practice` contributes nothing, despite being the authored opportunity for route-switch introduction on the longer route. A low shortest-path mediation contribution therefore cannot mean dispensable teaching or low design value. Also, with default endpoints excluded, terminal `combined-room` has no node-betweenness contribution in this acyclic four-node route graph while remaining the explicit combined demand. No numerical scores were computed.

**Data:** typed resolved edges already available; centrality derivable after projection/status policy. Player traffic probabilities and intended pedagogical value are missing, not estimable from these counts alone.

**Visible adaptation hypothesis:** optional size overlay explicitly labelled “distinct direct dependents” or “shortest-path betweenness”; keep fixed selectable node hit targets and a legend. Click node to display counted neighbors or sampled supporting source-target paths. Pair with dominance/bypass overlay to expose counterexamples. Source declarations remain accessible. Do not name overlay “importance.”

**Limits/failures:** raw duplicates inflate degree unless deduplicated; mixed role graphs inflate common mechanic hubs; shortest routes omit detours, gates and human choices. Weights in S2 mean distances and must be positive, not arbitrary confidence values. Sampling must display seed/sample count and approximation status.

**Scale/confidence:** full all-source shortest-path mediation has substantially more traversal work than degree; S2 offers sampling but no locally validated performance guarantee. Defer exact whole-graph centrality until question and projection warrant cost; do not impose current 4,096-record/32,768-edge loader bounds as data-authoring policy. Medium confidence as exploration; high confidence that it cannot establish mandatory play or learning.

## Prioritized follow-up questions

1. Does graph-wide foundation discovery plus clear AND braces reveal useful combinations beyond current selected-root exploration? Evaluate with actual larger authored graphs, not generated topology alone.
2. What entry/exit and incomplete-reference policy makes route dominator results unambiguous? Verify algorithm semantics with original reference and multi-entry/cycle/unknown cases before coding.
3. Which proposed overview adds information beyond existing route findings/witnesses and diagnostics browser? Avoid redundant capability labels.
4. Are apparent clusters stable across typed projections/status filters and modularity resolution choices? Full paper and competing community methods could change the low-priority recommendation.
5. Beyond native bounds, choose measured budgets for per-root traversals, result retention and interactive overlays. No timings available in this pass.

## Checks, gaps and deeper-pass decision

- Brief, assignment, selected AI-assisted guide and required artifact formats read before discovery. Both example skill schemas/usages, crate challenge schema/usage, actual referenced records, native reader/analysis code and complete native README read.
- Public discovery stopped at eight candidate URLs; six sources read, two explicit failures. Duplicate betweenness observation preserved as ninth CSV row. No search snippet treated as source passage.
- Fixture reasoning checked by reading declarations only; no algorithm execution, performance tests, browser automation, code edits, shared-file edits or extra agents.
- Biggest gap: no empirical usefulness test for these proposed overlays; no original dominator paper or full modularity paper read. No claim that a graph metric predicts learning, mastery, traffic or quality.
- Deeper pass could change ordering among dominators, cuts and centrality, or identify a suitable typed community method. It is unlikely to remove the basic need to separate AND dependencies from alternative routes. Parent approval required before deeper retained-evidence campaign.

## Comments

All local paths in evidence locations are repository-relative for parent synthesis. Public sources provide method contracts/limitations, not demonstrated game-design application evidence. Application and visual-comprehension siblings should assess those separate questions.
