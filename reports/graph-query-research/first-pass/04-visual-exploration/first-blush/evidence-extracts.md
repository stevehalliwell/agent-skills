# Bounded evidence extracts — visual exploration

Read date: 2026-10-28. Labels describe provenance, not truth. Exact extracts below come from fetched content, not search summaries. PDF text extraction can change typography. Report adaptations remain hypotheses.

## E1 — Start at relevant context, not mandatory overview

- URL: https://research.ibm.com/publications/search-show-context-expand-on-demand-supporting-large-graph-exploration-with-degree-of-interest
- Provenance: primary; original-publication abstract, IEEE TVCG, 01 Nov 2009. Full paper not read.
- Location: Abstract, opening and method sentences.
- Exact extract: “there are many situations where such an overview is not relevant or practical for users, as analyzing the global structure may not be related to the main task of the users that have semi-specific information needs.”
- Exact extract: “we advocate an interaction model that allows users to remotely browse the immediate context graph around a specific node of interest.”
- Exact extract: “We demonstrate the effectiveness of our approach with an exploration of a dense online database containing over 3 million legal citations.”
- Paraphrase: authors propose local contextual exploration and degree-of-interest extraction.
- Limit: abstract alone; demonstration is not evidence that our game designers benefit, nor a native implementation/performance estimate. Explicit expansion without automatic degree-of-interest ranking is our narrower adaptation.

## E2 — Select versus hide; graph-linked results

- URL: https://manual.cytoscape.org/en/3.10.3_a/Finding_and_Filtering_Nodes_and_Edges.html
- Provenance: primary; official Cytoscape 3.10.3 documentation.
- Location: §10.4, Interactive Filter Application Mode.
- Exact extract: “You also have the option to use the filter to show only the selected nodes by checking the **show** button. The **select** button is checked by default and simply selects nodes that pass the filter.”
- Location: §10.3, cyChart.
- Exact extract: “Selection in the chart will reciprocally select nodes or edges in the network.”
- Interpretation: one result set can drive graph selection while leaving context visible; hiding is a distinct action.
- Limit: documented behavior, not controlled comprehension result. cyChart is a chart/network link; do not claim this passage proves arbitrary table-row reciprocal selection.

## E3 — Scale can change interaction timing

- URL: same Cytoscape filtering chapter as E2.
- Provenance: primary; official software documentation.
- Location: §10.3, Histograms.
- Exact extract: “in large networks this will over-stress the computer and become sluggish in the interface.”
- Exact extract: “there is a checkbox in the header of **cyChart** to control whether selection is recomputed whenever the mouse moves, or occurs only at the end of your drag.”
- Location: §10.4, Node Adjacency Transformer.
- Exact extract: “Note that pressing the **Apply** button repeatedly may cause the selection to continuously expand.”
- Interpretation: explicit apply/release and visibly bounded expansion are supported alternatives to always-live filtering.
- Limit: no network-size threshold or benchmark retained. No performance number transfers to C99 explorer.

## E4 — Attribute/result state can visibly affect graph

- URL: https://manual.cytoscape.org/en/stable/Node_and_Edge_Column_Data.html
- Provenance: primary; official software documentation. Search identified 3.10.5; fetched body has no embedded version. Treat `stable` as version-moving, observed 2026-10-28.
- Location: chapter opening.
- Exact extract: “These column data can then be visualized in a user-defined way by setting up a mapping from columns to network properties (colors, shapes, and so on).”
- Location: Table Panel subsection.
- Exact extract: “To display columns for only selected nodes/edges, click the **Change Table Mode** button”.
- Interpretation: graph encodings plus selected-result detail table are documented implementation patterns.
- Limit: mapping itself does not prove that a particular color/shape supports comprehension. Perceptual encoding choice belongs to sibling drawing lane. Do not copy Cytoscape's editable-table/source mutation behavior.

## E5 — Reversible context-preserving filtering and typed expansion

- URL: https://neo4j.com/docs/aura//explore/explore-visual-tour/scene-interactions/
- Provenance: primary; official vendor documentation. Page describes Bloom in Aura Explore docs; version not pinned, observed 2026-10-28.
- Location: Filtering.
- Exact extract: “When a filter is applied, all filtered elements are greyed out in the Scene, they are still visible but you cannot interact with them.”
- Exact extract: “You can also completely remove the filtered elements from the Scene, by using the `Dismiss Filtered elements`.”
- Exact extract: “Filters are **not** saved, so when the scene is cleared so is the filter panel.”
- Location: Expand nodes.
- Exact extract: “The right-click context menu provides additional options to expand selectively along a specific relationship type and direction”.
- Interpretation: dim, dismiss and expansion are distinct operations. Our adaptation should preserve source inspection even for dimmed support context; it need not copy non-interactivity.
- Limit: vendor behavior claim, not effectiveness evidence. No dependency selection implied. Current scene is not whole author corpus.

## E6 — A single visible path is not all explanations

- URL: same Neo4j chapter as E5.
- Provenance: primary; official vendor documentation.
- Location: Shortest path callout.
- Exact extract: “Bloom searches for shortest paths within 20 hops and shows the first shortest path found by the database.”
- Interpretation: useful challenge to treating one path as exhaustive explanation. Native opportunity witnesses use different semantics; do not replace them with arbitrary shortest paths.
- Limit: product-specific 20-hop bound, not a proposed workbench limit or author-data restriction. Path may omit meaningful alternatives.

## E7 — Meaningful exploration history, not every click

- URL: https://www.cs.tufts.edu/comp/250VA/papers/dunne2012graphtrail.pdf
- Provenance: primary; Dunne, Riche, Lee, Metoyer, Robertson, CHI 2012, GraphTrail.
- Location: p.1668, Visual Exploration History.
- Exact extract: “A parent link can be moused over to highlight the entire chain of actions used to get to the subset in the chart, all the way to a root chart showing all nodes of one type.”
- Location: p.1671, Exploration History Model.
- Exact extract: “The analysis history we keep and display to the user is not the complete history of their actions – instead it is a subset of meaningful actions necessary to understand the displayed analysis.”
- Interpretation: query/filter/expansion states with visible provenance are a narrower candidate than a full interaction event log.
- Limit: GraphTrail history connects aggregate views, not source declarations. Workbench needs separate analysis-action history and graph-support paths; neither substitutes for edit undo.

## E8 — Original study scope and counter-evidence

- URL: same GraphTrail PDF as E7.
- Provenance: primary; original qualitative study report, not independent replication.
- Location: p.1668, Participants and Data.
- Exact extract: “This dataset forms a network and consists of 12,838 nodes from 24 node types, linked by 18,927 edges with 35 types.”
- Location: p.1670, Participants and Tasks.
- Exact extract: “We recruited ten researchers and interns (eight males, two females) who were familiar with the field of HCI from our institution and asked them to analyze the CHI paper data.”
- Location: p.1670, Results, Phase 1.
- Exact extract: “It took most participants approximately half the session to fully grasp the operation and its result.”
- Location: p.1671, Scalability.
- Exact extract: “Our participants usually created analyses of 20-30 charts at most, starting a new file for a new exploration.”
- Exact extract: “having a large exploration history introduces a layout management cost for the user.”
- Location: p.1671, More Chart Types and Interactions.
- Exact extract: “On multiple occasions, they would spot an interesting link pattern using the hybrid bar chart, analyze it further in a matrix, and present it in their report using a node-link diagram.”
- Interpretation: complement graph with results/history; do not replace graph with a universal aggregate canvas. Typed pivots and unbounded history can impose learning/management costs.
- Limits: three-month archaeology deployment plus ten-person qualitative HCI lab study; not game designers, no causal benchmark for our proposed controls. Paper also says brushing/linking charts would improve comparisons; do not report that as an implemented GraphTrail feature.

## Access failures and unverified leads

- https://pubmed.ncbi.nlm.nih.gov/19834159/ — fetch tool reported JavaScript-rendered content. No full abstract read from this endpoint; not evidence used above.
- https://aviz.fr/wiki/uploads/Teaching2014/SearchShowContext.pdf — fetch failed. Full text remains unread. IBM author/publisher abstract is bounded replacement, not equivalent full-paper access.
- No environment/config changes, authenticated fetch, paywall/CAPTCHA workarounds or browser automation. No need to recover failed pages once accessible bounded substitute sufficed.

## Repository evidence locations

Paths relative to repository root; all listed ranges actually read. These are authored declarations/code observations, not player evidence.

- `skills/game-design-workbench/native/README.md:136-148`: focused relationships, raw direction, consumed-field/boundary limits and draft preservation.
- `skills/game-design-workbench/native/README.md:217-225`: hypothetical known assumptions, comparisons, bounded query, current graph markers.
- `skills/game-design-workbench/native/README.md:236-244`: mode-switch and draft/reload constraints.
- `skills/game-design-workbench/native/README.md:256-268`: route opportunity semantics, bypass/support example, check/witness/path caps and scale model.
- `skills/game-design-workbench/native/README.md:373-380`: edit markers, undo and dirty-refresh guard.
- `skills/game-design-workbench/native/gd_core.c:350-394`: role-specific native reader; challenge role does not consume all schema-declared context fields.
- `skills/game-design-workbench/native/gd_focus.c:7-51`: explicit incoming/outgoing expansion, visibility and boundary counting.
- `skills/game-design-workbench/native/gd_graph_view.c:114-125,163-164,198-207`: retained witness membership changes actual graph appearance.
- `skills/game-design-workbench/native/gd_graph_view.c:283-307`: route check inspection, supported/unsupported witness controls, suffix warning and clear highlight.
- `skills/game-design-workbench/native/gd_graph_view.c:439-482`: assumption panel, query completeness and source-lined shared-link candidate inspection.
- `skills/game-design-workbench/native/gd_graph_view.c:678-694,720-734`: known/chain graph markers, dirty refresh block, known identity remap and witness reset.
- `skills/game-design-workbench/examples/crate-puzzle/game-design/challenges/.schema.md:1-19` and `.usage.md:1-3`: declared field inventory and intent boundary.
- `skills/game-design-workbench/examples/crate-puzzle/game-design/challenges/move-demo.md:7-30`, `push-room.md:7-33`, `switch-practice.md:7-33`, `combined-room.md:7-32`: actual route/skill-role declarations and intent caveats.
- `skills/game-design-workbench/examples/crate-puzzle/game-design/skill-atoms/route-switch.md:7-26`: read-switch prerequisite, not measured mastery.
- `skills/game-design-workbench/examples/little-plumber-guy-nes/game-design/skill-atoms/.schema.md:1-11` and `.usage.md:1-9`: AND prerequisites and hypothetical known constraint.
- `skills/game-design-workbench/examples/little-plumber-guy-nes/game-design/skill-atoms/jump-stomp.md:7-34`, `fall-stomp.md:7-34`, `stomp-walker.md:7-24`: proposed variants, shared rule, different prerequisites and evidence sought.
