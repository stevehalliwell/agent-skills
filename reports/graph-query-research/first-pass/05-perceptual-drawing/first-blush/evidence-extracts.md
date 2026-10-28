# Perceptual drawing — bounded evidence extracts

First-pass orientation, not systematic review. Public fetches performed 2026-10-28. Eight unique candidate URLs; repeated observations retained in CSV. Exact extracts below come from fetched text, not search summaries. Primary labels identify provenance, not reliability. No third-party figures copied.

## E1 — Path continuity, crossings and occlusion

- Source: Ware, Purchase, Colpoys and McGill, *Cognitive measurements of graph aesthetics* (2002), primary research, publicly hosted typeset proof: https://www.cs.kent.edu/~jmaletic/cs63903/papers/Ware02.pdf
- Access: PDF extracted, nine pages read. Proof includes placeholder pagination/metadata and editorial query page; cite title/authors and internal PDF pages rather than treating proof metadata as final bibliographic authority.
- Location: PDF p.4, “The diagrams”: “There were 42 nodes in the graph, and the number of edges on each node was randomly varied between one and five.”
- Location: PDF p.4, “Experimental task”: “For any given graph drawing, the shortest path between the two highlighted nodes was between three and five.”
- Location: PDF p.5, “Subjects”: “The 43 subjects were second and third year Computer Science and Information Systems students at the University of Queensland.”
- Location: PDF p.5, “Results”: “Seven graph drawings were eliminated from the analysis because of an unexpected perceptual error, where an edge passing under a node, looked as if it were two edges attached to that node.”
- Location: PDF p.6, “Discussion”: “The practical consequence is that it may be worth allowing for an occasional crossing in a graph layout if it reduces the bendiness of paths.”
- Location: PDF pp.6–7, discussion spanning page break: “For shortest path tasks, we have shown here that it is the number of edges that cross the shortest path itself that is important, rather than the total number of edges crossing in the drawing.”
- Location: PDF p.6: “More studies are required to determine the robustness of our findings of this study for other than spring layout graphs and for different graph sizes”.
- Paraphrase: study models correct-trial response times on abstract spring-layout shortest-path tasks. Continuity and crossings on task path matter; total drawing crossings did not significantly predict response time in this model.
- Inference: preserve route continuity and avoid false node incidence before minimizing global crossings at any cost.
- Limits: regression over generated drawings, not a universal causal law or benchmark for authored directed graphs. Do not reuse seconds-per-crossing coefficients for this application. Acute-angle concerns appear in theoretical motivation; this study does not establish a universal right-angle routing prescription. Figures 2, 4–6 illustrate issues in source PDF.

## E2 — Density and representation counter-evidence

- Source: Ghoniem, Fekete and Castagliola, *On the readability of graphs using node-link and matrix-based representations: a controlled experiment and statistical analysis* (2005), primary research, author PDF: https://mohammad.ghoniem.info/research/ivs-usability-2005.pdf
- Access: extracted 22-page PDF; methods, task results, discussion and conclusion read.
- Location: printed p.117, “The data”: “graphs of three different sizes (20 vertices, 50 vertices and 100 vertices) with three different link densities (0.2, 0.4 and 0.6) for each size”. Same section: “we opted for random undirected graphs”. Density definition is paper-specific, not ordinary edge/node ratio.
- Location: printed p.117, “The population”: “The population consisted of 36 subjects, all of whom had previously seen a node-link representation of graphs.” Population described as postgraduate students and confirmed CS researchers.
- Location: printed p.118, implementation: “Given that we are dealing with undirected graphs, we did not display any arrows at the end points of the links”.
- Location: printed p.128, “Finding a path between two nodes”: “The matrix-based representation performs very poorly for this task except for very dense graphs where it outperforms the node-link representation”.
- Location: printed p.129, path discussion: “However, matrices outperform them about large and dense graphs.”
- Location: printed p.134, conclusion: “Path-related tasks remain difficult on both representations and require an appropriate interaction that helps perform them.”
- Paraphrase: matrix and node-link are complementary; sorted matrices help several adjacency/search tasks as density/size increase. Path tracing usually favors node-link for sparse examples, with explicit dense-case exceptions.
- Limits: nine random graph instances, 20–100 vertices, neato layout, trained CS population, 45-second question limit; picking/highlighting available and endpoints preselected for two-node tasks. Not evidence for a universal 20-node cutoff or 4,096-node usability. Abstract's “consistently” favorable path claim must be qualified by detailed results. Source Figure 1 directly juxtaposes 50 vertices/400 edges; linked only, not reproduced.

## E3 — Task-matched comparison, without a retrieved winner

- Source: Pohl, Schmitt and Diehl (2009), primary research publication abstract: https://doi.org/10.2312/COMPAESTH/COMPAESTH09/049-056
- Location: Abstract: “a user study comparing the readability of force-directed, orthogonal, and hierarchical graph layouts.”
- Exact extract: “Based on the correctness of answers and the related response time we evaluated for each task which layout is better suited.”
- Paraphrase: task-specific evaluation exists; eye tracking used to investigate explanations.
- Limits: only abstract retrieved. Graph sizes, population, specific task winners and effect sizes unresolved. No claim that hierarchical layout empirically wins our tasks.

## E4 — Directed-edge cues: retain issue, not search-result ranking

- Source: Holten and van Wijk, CHI 2009, primary university publication abstract: https://research.tue.nl/en/publications/a-user-study-on-visualizing-directed-edges-in-graphs/
- Location: opening abstract: “Although this representation is intuitive, it is not guaranteed that a user is able to determine edge direction as quickly and unambiguously as possible; alternative representations that exhibit less occlusion and visual clutter might be better suited.”
- Exact extract: “we developed five additional directed-edge representations using combinations of shape and color.”
- Paraphrase: study compared direction representations using speed and accuracy.
- Limits: abstract only; sample, graph sizes and winner unavailable. Search synthesis claimed tapered edges perform best; that claim is NOT adopted. Follow-up must inspect paper before recommending replacement for arrowheads.

## E5 — Spatial organization, not measured comprehension improvement

- Source: *Perceptual organization in user-generated graph layouts*, primary institutional research abstract, dated 01 Nov 2008: https://research.ibm.com/publications/perceptual-organization-in-user-generated-graph-layouts
- Location: Abstract: “we asked users to explicitly manipulate nodes in a network diagram to create layouts that they felt best captured the relationships in the data.”
- Exact extract: “observers are able to recover cluster structure, that the distance between clusters is inversely related to the strength of the clustering, and that users exhibit the tendency to use edges to visually delineate perceptual groups.”
- Inference: proximity/group boundaries are meaningful cues and can mislead when groups have no declared/derived basis.
- Limits: abstract lacks participant and graph counts. Layout-production behavior, not proof of faster reading, learning, or superiority of enclosures. Do not infer universal spacing law or assume force-separated groups are game-design families.

## E6 — Layered drawing method

- Source: Graphviz official `dot` documentation: https://www.graphviz.org/docs/layouts/dot/
- Location: introduction: “The layout algorithm aims edges in the same direction (top to bottom, or left to right) and then attempts to avoid edge crossings and reduce edge length.”
- Location: Attributes: “constraint – If false, the edge is not used in ranking the nodes.” Also documents `rank`, `rankdir`, `ordering`, `ranksep` and crossing-minimizer controls.
- Observation: page embeds a directed-cluster SVG diagram and links manual/user guide. Diagram link: https://www.graphviz.org/Gallery/directed/cluster.svg (not fetched separately, not another candidate).
- Interpretation: useful reference method for layers, branch ordering and feedback-edge treatment; not selection of Graphviz dependency.
- Limits: documentation describes algorithm aims, not guaranteed optimality, comprehension benefit or performance. Page version not pinned; implementation/API adoption needs version-specific follow-up. No application tool installed or run.

## E7 — Redundant meaning encodings

- Source: W3C WAI, *Understanding Success Criterion 1.4.1: Use of Color*, official explanatory accessibility guidance for WCAG 2.2: https://www.w3.org/WAI/WCAG22/Understanding/use-of-color.html
- Location: Success Criterion: “Color is not used as the only visual means of conveying information, indicating an action, prompting a response, or distinguishing a visual element.”
- Location: In Brief: “Use information in addition to color, such as shape or text, to convey meaning.”
- Location: Intent note: “This criterion requires a visible alternative to color.”
- Recommendation/adaptation: add pattern/text/shape for relation and warning meanings in desktop graph; legend alone does not differentiate same-shaped edges when hue unavailable.
- Limits: web accessibility guidance, not empirical graph-task comparison or desktop compliance audit. Does not specify optimal font, graph label density or node size. No contrast conformance claimed.

## E8 — Inaccessible original lead

- Source candidate: Purchase (1997), original research mirror: https://scispace.com/pdf/which-aesthetic-has-the-greatest-effect-on-human-3iusjoe7xq.pdf
- Fetch: HTTP 403 Forbidden. No passage retained. No bypass or environment change attempted.
- Status: unverified original lead. Ware's discussion reports and challenges earlier crossing evidence, but cannot substitute for reading Purchase when making claims about its detailed methods.

## Comments

No empirical source located/read for optimal typography, label collision algorithms, animation stability, overview aggregation, or actor/resource-specific layout. Those remain design hypotheses or follow-up gaps. No source supports inferring mastery, quality, learning or actual gameplay feasibility from topology.
