# Evidence extracts — structure/dependencies

Bounded orientation, observed 2026-10-28. Exact passages below came from fetched source content, not search summaries. NetworkX pages identify themselves as 3.7 documentation; no NetworkX dependency is proposed. Provenance labels describe origin, not truth. Source claims are not measured game-design outcomes.

## S1 — Rooted dominators

- URL: https://networkx.org/documentation/stable/reference/algorithms/generated/networkx.algorithms.dominance.immediate_dominators.html
- Provenance: primary, official API documentation. Read entire fetched page, response `mvrebm1kpapfj9`, index 0.
- Location: Returns / Notes / Examples.
- Exact: “A dict containing the immediate dominators of each node reachable from `start`, except for `start` itself.”
- Exact: “The immediate dominators are the parents of their corresponding nodes in the dominator tree.”
- Exact: “We can obtain the immediate post-dominators by reversing the graph before calling `immediate_dominators`.”
- Paraphrase: rooted directed analysis; unreachable nodes are not assigned a result.
- Adaptation hypothesis: an artificial entry connected to all declared entries can support graph-wide mandatory-prefix discovery. Reverse from a selected exit for exit-conditioned convergence. Artificial nodes must stay visually distinct.
- Limits: API contract supplies no game reachability, gates, traffic or learning model. This page does not benchmark complexity or explain all-path semantics in detail; algorithm-paper verification remains follow-up.

## S2 — Betweenness, including counter-evidence

- URL: https://networkx.org/documentation/stable/reference/algorithms/generated/networkx.algorithms.centrality.betweenness_centrality.html
- Provenance: primary, official API documentation. Read fetched definition, parameters, notes, examples and backend caveats; response `mvrebm1kpapfj9`, index 1.
- Location: opening definition.
- Exact: “Betweenness centrality of a node” [formula variable omitted here] “is the sum of the fraction of all-pairs shortest paths that pass through” [formula variable omitted here]. These are two bounded text fragments, not a continuous sentence quotation.
- Location: parameters `weight`.
- Exact: “Weights are used to calculate weighted shortest paths, so they are interpreted as distances.”
- Location: Notes.
- Exact: “For approximate betweenness calculations, set `k` to the number of sampled nodes (‘pivots’) used as sources to estimate the betweenness values.”
- Exact: “For weighted graphs the edge weights must be greater than zero. Zero edge weights can produce an infinite number of equal length paths between pairs of nodes.”
- Paraphrase: this is shortest-path mediation, not mandatory-node detection. Sampling changes exact results to estimates.
- Inference: optional longer practice routes can receive little mediation weight without being unimportant to design. No player-choice distribution is encoded by this metric.
- Limits: no benchmark retained; do not convert arbitrary relationship strength into distance silently. Backend normalization warnings on page reinforce version sensitivity but are not a tested local issue.

## S3 — SCCs

- URL: https://networkx.org/documentation/stable/reference/algorithms/generated/networkx.algorithms.components.strongly_connected_components.html
- Provenance: primary, official API documentation. Entire fetched page read; response `mvrebm1kpapfj9`, index 2.
- Location: opening / Returns / Notes.
- Exact: “Generate nodes in strongly connected components of graph.”
- Exact: “A generator of sets of nodes, one for each strongly connected component of G.”
- Exact: “Iterative version of Tarjan’s algorithm using the improvements described by Tarjan and Zwick in their survey”.
- Paraphrase: directed connectedness partition can be computed without recursive traversal in this implementation.
- Adaptation hypothesis: collapsible route-loop regions, retaining raw edges; skill SCCs remain prerequisite conflicts, not an OR-choice cluster.
- Limits: SCC membership is not a thematic community. Local code uses iterative Kosaraju rather than this documented Tarjan implementation.

## S4 — Ancestry

- URL: https://networkx.org/documentation/stable/reference/algorithms/generated/networkx.algorithms.dag.ancestors.html
- Provenance: primary, official API documentation. Entire fetched page read; response `mvrec6talpxjoj`, index 1.
- Location: opening / example note.
- Exact: “Returns all nodes having a path to `source` in `G`.”
- Exact: “The `source` node is not an ancestor of itself, but can be included manually”.
- Paraphrase: graph reachability supplies transitive foundations if edges point prerequisite to dependent.
- Inference: under authored AND semantics, every resolved ancestor is a declared necessary foundation. Under route alternatives, the same ancestry set means possible predecessors, not all necessary visits.
- Limits: ancestry alone does not validate cycles, incomplete references, learning or solvability.

## S5 — Cuts

- URL: https://networkx.org/documentation/stable/reference/algorithms/generated/networkx.algorithms.connectivity.cuts.minimum_node_cut.html
- Provenance: primary, official API documentation. Entire fetched page read; response `mvrec6talpxjoj`, index 2.
- Location: opening / Notes.
- Exact: “If source and target nodes are provided, this function returns the set of nodes of minimum cardinality that, if removed, would destroy all paths among source and target in G.”
- Exact: “This is a flow based implementation of minimum node cut.”
- Exact: “A minimum node cut of a digraph separates an ordered pair of nodes, and the two orders need not agree, so both are considered”.
- Paraphrase: answers structural disconnection; direction and endpoints matter. Page recommends reusing flow data structures for repeated local cuts.
- Recommendation: defer general all-pair cuts behind simpler graph-wide dominators and explicit removal witnesses.
- Limits: smallest-cardinality cut is not cheapest edit, intended bottleneck, gameplay difficulty or dependency-AND evaluation.

## S6 — Modularity limitation

- URL: https://arxiv.org/abs/physics/0607100
- Provenance: primary, author preprint abstract, v2 dated 2006-07-14; journal reference PNAS 104(1), 36–41 (2007). Fetched abstract and metadata read, response `mvrecgk1999o1f`. Full paper not read.
- Location: Abstract.
- Exact: “We find that modularity optimization may fail to identify modules smaller than a scale which depends on the total number L of links of the network and on the degree of interconnectedness of the modules, even in cases where modules are unambiguously defined.”
- Paraphrase: authors report a resolution limit for modularity optimization.
- Inference: a single modularity partition should not be presented as the game's natural mechanic families. This is not evidence that all community algorithms fail.
- Limits: no local or heterogeneous-game-graph test; abstract alone does not justify selecting an alternative algorithm or numerical resolution parameter.

## Explicit fetch failures

- https://pmc.ncbi.nlm.nih.gov/articles/PMC1765466/ — readable fetch reported “Page appears to be JavaScript-rendered (content loads dynamically)”. No source passage retained; search summary not evidence.
- https://pubmed.ncbi.nlm.nih.gov/17190818/ — same failure. No source passage retained.
- No bypass, browser automation, tool install or environment change attempted. Public arXiv abstract was sufficient for bounded limitation claim; full-paper recovery deferred, not silently completed.

## Repository evidence locations

Paths below are relative to repository root. All cited ranges were actually read with file reader. Local statements in analyst report are observations/inferences, not public-source empirical results.

- `skills/game-design-workbench/examples/crate-puzzle/game-design/skill-atoms/.usage.md:1-9`: AND sets; reverse unlocks; no implicit learning; ranks distinct from depth.
- `skills/game-design-workbench/examples/crate-puzzle/game-design/skill-atoms/.schema.md:1-11`: declared prerequisites/mechanics/rank.
- `skills/game-design-workbench/examples/crate-puzzle/game-design/challenges/.schema.md:1-21`: route, skill roles and extra context fields.
- `skills/game-design-workbench/examples/crate-puzzle/game-design/challenges/.usage.md:1-3`: authored intent, not learning/reachability.
- `skills/game-design-workbench/examples/crate-puzzle/game-design/challenges/move-demo.md:8-18`, `push-room.md:8-23`, `switch-practice.md:8-24`, `combined-room.md:8-22`: entry, fork, optional support, terminal demand.
- `skills/game-design-workbench/examples/crate-puzzle/game-design/skill-atoms/coordinate.md:8-13`, `plan-push.md:8-12`, `push.md:8-12`, `route-switch.md:8-12`: two AND branches and transitive foundations.
- `skills/game-design-workbench/examples/little-plumber-guy-nes/game-design/skill-atoms/jump-stomp.md:7-35`, `fall-stomp.md:7-35`, `stomp-walker.md:7-25`: proposed method alternatives, accepted shared understanding, evidence not mastery.
- `skills/game-design-workbench/examples/little-plumber-guy-nes/game-design/skill-atoms/.usage.md:1-9`: same AND/method semantics; schema also read completely.
- `skills/game-design-workbench/native/gd_core.c:361-387`: role-specific consumption; challenge reader does not consume all schema context links.
- `skills/game-design-workbench/native/gd_skills.c:19-150`: resolved-pair deduplication, degree counts, iterative SCC, incomplete propagation and condensed relative layers.
- `skills/game-design-workbench/native/gd_routes.c:36-61,64-108,110-130,145-200`: validation, prerequisite demand expansion, two-state route walk, witness coverage.
- `skills/game-design-workbench/native/README.md:159-200,222-268,422-430`: skill semantics, existing exploration, route consumer and reader bounds. README read through line 467 in two calls; first call truncated at 358, remainder explicitly read.
