# Directed crossing algorithms — independent first blush

## Result and scope

Provisional orientation: evaluate **discrete within-band ordering with an explicit crossing objective**, followed by footprint-aware coordinate assignment and routing. Do not continue treating repulsion or branch rotation as the main crossing optimizer. This is an algorithmic direction for investigation, not dependency selection or architectural adoption.

Important correction: local directed mode already has barycenter-style ordering. Missing pieces worth investigating are adjacent-rank treatment of long edges, best-order retention, crossing-scored transpose or sifting, and renderer-compatible routing—not simply “add Sugiyama.”

Read first-pass brief and selected local source read-only. Reviewed 8 unique public URLs, listed below. No local benchmark, compilation, installation, GUI work, or source edits. Direct corrections and continuously settling branch forces remain rejected trials, not accepted solutions.

## Evidence: bounded source passages

Quoted text is source-reported evidence, not a local performance claim. PDFs have extraction whitespace normalized. Sources accessed during this research session; live documentation/repository versions were not pinned.

### S1 — Original dot algorithm paper

Gansner, Koutsofios, North, Vo, *A Technique for Drawing Directed Graphs*, 1993. Official Graphviz-hosted paper: https://graphviz.org/documentation/TSE93.pdf

- §1.4: “The first pass places the nodes in discrete ranks. The second sets the order of nodes within ranks to avoid edge crossings. The third sets the actual layout coordinates of nodes. The final pass finds the spline control points for edges.”
- §2.1: “Of course these edges are only reversed internally; arrowheads in the drawing show the original direction.”
- §3: “After rank assignment, edges between nodes more than one rank apart are replaced by chains of unit length edges between temporary or ‘virtual’ nodes.”
- §3: “Heuristics are appropriate since minimizing edge crossings in layouts of ranked graphs is NP-complete, even for only two ranks.”
- §3: “The barycenter method defines the weight of v as the average of elements in P. The median method defines the weight of v as the median of elements in P.” Here P contains adjacent-rank ordinal positions, not physical coordinates.
- §3, Figure 3-1 remarks: “At each iteration, if the number of crossings improves, the new ordering is saved.”
- §3: “transpose repeatedly exchanges adjacent vertices on the same rank if this decreases the number of crossings.”
- §1.2 defines input node dimensions as “Size of bounding box of a node v.”
- §4.2: “Using the auxiliary graph also permits the specification of ‘node ports,’ or edge endpoints offset in the X direction from the center of the node.”
- §4.3: “the graph with ‘virtual’ nodes added has V + D nodes and E + D edges, where D is the number of ‘virtual nodes.’”

Scope cautions: paper's optimal ranking/coordinate subproblems do not make the entire drawing crossing-optimal. Its historical timing and quality results are not evidence for this application. Paper reports median's advantage in its setting; that does not establish median as universally best under our ports, bands, or stability constraints.

### S2 — ELK Layered official algorithm reference

https://eclipse.dev/elk/reference/algorithms/org-eclipse-elk-layered.html

- Description: “Arranges as many edges as possible into one direction by placing nodes into subsequent layers.”
- Description: “This implementation supports different routing styles (straight, orthogonal, splines); if orthogonal routing is selected, arbitrary port constraints are respected”.
- Supported graph features explicitly include “Self Loops”, “Multi Edges”, “Edge Labels”, “Ports”, and “Compound”.
- Option entries list `CrossingMinimizationStrategy.LAYER_SWEEP`, `GreedySwitchType.TWO_SIDED`, `CycleBreakingStrategy.GREEDY`, and randomization seed `1`.
- Other listed controls include “Consider Model Order”, “Force Node Model Order”, “Semi-Interactive Crossing Minimization”, node size constraints, node label padding, port labels, and individual spacing.

Scope cautions: option presence is not a guarantee of edit stability, exact minimization, or arbitrary-port support in every routing style. No work or memory guarantee for our limits appears on this page.

### S3 — Dagre official project wiki

https://github.com/dagrejs/dagre/wiki

- Opening: “Dagre is a JavaScript library that makes it easy to lay out directed graphs on the client-side.”
- Design priorities: “Dagre must be able to draw medium sized graphs quickly, potentially at the cost of not being able to adopt more optimal or exact algorithms.”
- Node input: “width - how wide the node should be in pixels” and “height - how tall the node should be in pixels”.
- Output: “Edges get a points property that includes control point coordinates for the edge with points where the edge intersects with the node, assuming a rectangular shape”.
- Recommended reading: “we use the network simplex algorithm for ranking.”
- Recommended reading identifies its bilayer crossing counter as `O(|E| log |V_small|)`.
- License: “Dagre is licensed under the terms of the MIT License.”

Scope cautions: “medium sized” is not a quantified 4096-node/32768-edge guarantee. Counter complexity is not whole-layout complexity. Wiki exposes rectangle intersections, not an ELK-equivalent arbitrary constrained-port contract.

### S4 — OGDF SugiyamaLayout official API

https://ogdf.github.io/doc/ogdf/classogdf_1_1_sugiyama_layout.html

- Optional parameters: “Each repetition (except for the first) starts with randomly permuted nodes on each layer. Deterministic behaviour can be achieved by setting runs to 1.”
- Transpose description: “this step tries to reduce the number of crossings by switching neighbored nodes on a layer.”
- Module options list `LongestPathRanking` and `BarycenterHeuristic` as defaults for ranking and crossing minimization.
- `setRanking`: “if the graph is not acyclic, the ranking module computes an acyclic subgraph.”

Scope caution: documented deterministic configuration does not establish small-edit stability. API also exposes multiple runs and thread controls; comparison settings must be explicit.

### S5 — OGDF sifting reference

https://ogdf.github.io/doc/ogdf/classogdf_1_1_sifting_heuristic.html

- Description: “The sifting heuristic for 2-layer crossing minimization.”
- Strategy enumeration: `LeftToRight`, `DescDegree`, `Random`.

Scope caution: this page establishes an available implementation, but does not provide full pseudocode, asymptotic bounds, or a quality guarantee. Sifting mechanics below are technical orientation, not a quotation of an audited implementation.

### S6 — Graphviz official C library guide

https://graphviz.org/pdf/libguide.pdf

- §2: “Figure 1 gives a template for the basic library use of Graphviz, in this instance using the dot hierarchical layout.”
- Figure 1 contains `gvLayout (gvc, G, "dot");` and `gvFreeLayout(gvc, g);`.
- §1: “N.B. Using Graphviz as a library is not thread-safe.”

Scope caution: guide dated August 21, 2014. Thread-safety warning is historical official guidance; current version-specific guarantees, deployment, plugin requirements, allocation behavior, and license terms need separate verification.

### S7 — OGDF official repository README

https://github.com/ogdf/ogdf

- “OGDF is a self-contained C++ library for graph algorithms”.
- License: “GNU General Public License v2 or v3, with special exceptions.”

Scope caution: exceptions and bundled third-party terms were not audited. This is not permission to copy code into the C99 solver or a license-suitability conclusion.

### S8 — ELK official repository README

https://github.com/eclipse-elk/elk

- “The layout algorithms are plain Java and can thus also be used outside of Eclipse.”
- README describes elkjs as “JavaScript library transpiled from ELK's Java sources”.

Scope caution: ELK license text was not reviewed within this URL budget. Java/JavaScript availability does not establish a native C interface.

## Local observations

Paths below are repository-relative, for locating evidence rather than links.

- `skills/game-design-workbench/native/gd_layout_directed.h:11–36`: `directed_order` executes four passes. It normalizes ordinal positions within each group/layer, averages neighboring positions through `sum/count`, alternates which layer side contributes, then sorts. Ties preserve `previous` order. Inactive edges, cross-group edges, negative layers, and same-layer edges are excluded.
- This is barycenter-style ordering across existing semantic bands. It can include neighbors separated by multiple layers. It is not the exact adjacent-rank dummy-expanded sweep described in S1. In the inspected function, no crossing counter, best-order snapshot, transpose, or sifting step appears.
- Same file, `directed_arrange`: uses width/height-dependent layer and band spans after ordering. Existing footprint-aware packing should not be described as absent.
- Same file, `directed_clearance`: limited straight-link clearance moves within existing order; comment explicitly says “not an obstacle solver.” It does not supply complete route geometry.
- `skills/game-design-workbench/native/gd_layout.c`: directed arrange dispatches to `directed_arrange`; continuous `crossing_forces` runs only for `GD_LAYOUT_FORCE`.
- `skills/game-design-workbench/native/test_layout_baseline.c`: source search shows strict interior centerline crossing measurement and crossing availability only when edge count is at most 256. This is an inspected test limitation, not a measurement of current visual quality.

Trace limit: this pass did not audit every renderer, input adapter, port position, or identity mapping. Brief supplies asymmetric badges/captions, coordinate normalization, and preservation requirements; those contracts remain unverified locally here.

## Interpretation: what changes, and why

### Pipeline and ordering

1. Build a transient layout graph. Preserve source identity, shared nodes, authored directions, and original edges.
2. Handle cycles for layout only, then assign ranks—or retain existing semantic bands where required. Strict increasing rank on every edge is impossible around a directed cycle. Internally reversed feedback edges must still render their original arrowheads. SCC grouping is another possible presentation, not permission to merge records or infer a gameplay loop.
3. Normalize long edges with virtual nodes on intervening ranks. These reserve ordering/route space without creating domain records. Same-layer edges, self-loops, parallel edges, and disconnected components need explicit treatment.
4. Alternate downward/upward median or barycenter sweeps. Score complete candidate orders and retain best encountered order. Add transpose: accept adjacent swaps that reduce relevant crossings, respecting fixed groups/order constraints. Both neighboring layers matter when judging an interior rank.
5. Consider sifting as a stronger optional search: move one vertex through candidate positions and retain its best insertion position. A sequence can reach a better position despite an initially worsening adjacent move, unlike strict improving-only transpose. It costs more search; do not assume it fits existing per-frame work caps.
6. Assign coordinates without destroying order, using rectangular extents and port offsets. Route actual edges, then validate route crossings and obstacle clearance.

Why order can matter more than repulsion: for two edges between fixed parallel ranks, a crossing corresponds to reversed endpoint order. Moving endpoints farther apart while preserving those orders does not remove that inversion. A discrete reorder can remove it directly. Force repulsion mainly handles spatial separation; competing springs, collisions, and anchored bands can prevent the large coordinated move needed to change order. This explains a plausible mechanism, not a proven diagnosis of either rejected experiment.

### Directed ranks versus movable roots

A retained layered layout gives up one degree of freedom: nodes share discrete flow-axis ranks and move chiefly along the cross-axis. Roots become source-side rank members, not freely drifting anchors. Placement communicates directed progression, may enlarge depth/width, and can place shared downstream nodes between parents rather than inside one branch.

Existing bands already impose much of that contract. Reordering within them can preserve semantics. A fully ranked replacement for Free graph changes interaction and presentation semantics; it is not an invisible force-solver fix. One-shot layered initialization followed by free dragging is conceivable, but its crossing result is not invariant once dragging or forces change ordering/ranks. No mode conversion is adopted here.

### Rectangles, labels, ports, and routing

Use complete rendered footprint: marker/card, label, caption, badges, margins, and asymmetric overhang. If input uses symmetric rectangles, carry an offset between footprint center and semantic anchor. Apply MDA display/layout scaling consistently before spacing or clearance comparisons.

Ordering is only part of readability. Long-edge virtual chains must influence routing; if renderer discards them and draws a single center-to-center segment, intermediate-band clearance can disappear. Conversely, routing may remove node intersections without changing centerline crossing count. Port order can produce crossings even when node order looks good. Validate rendered routes, node/label obstacles, overlaps, and arrow readability separately from abstract bilayer crossings. ELK's documented constrained-port support makes it a useful capability reference, not a dependency recommendation.

### Stability and bounded work

Determinism means same input/settings gives same output. Edit stability means small input changes cause limited movement. Neither implies the other. Proposed evaluation controls: canonical typed-identity input order, deterministic cycle handling, stable tie-breaking, fixed traversal/restart order and seed where relevant, explicit pass/work limits, and best-so-far snapshots. Prior-order tie-breaking or movement penalties may improve edit stability but trade off crossing reduction; forced old order can prevent improvement entirely.

Exact versus heuristic: ranked crossing minimization is NP-complete [S1]. Exact search can be useful as a small-fixture oracle; it is not a justified whole-graph interactive plan. Median/barycenter, transpose, and sifting are heuristics, not zero-crossing guarantees. Ranking and coordinate optimization solve different objectives.

At 4096 nodes / 32768 edges, naive all-edge-pair testing has 536854528 unordered pairs. Bilayer inversion counting offers a better scoring route [S3], but shared endpoints and port order require careful definitions. Expanded size depends on rank spans: for positive spans, naive dummy count is `D = sum(span(edge) - 1)`, potentially O(E times number_of_ranks), not O(V). A dense crossing-delta matrix or many restart snapshots can also exceed 1 MiB.

Bound sweeps, candidate swaps/insertions, dummy storage, and routing separately. Existing 65536 collision / 8192 uncrossing visits per step cannot be translated into a full-layout guarantee. No reviewed source establishes runtime, memory, or acceptable visual quality at the stated scale. A one-shot or resumable layout may have different budgets, but that needs an explicit later contract.

## Practical references and native C fit

- **Graphviz dot:** strongest directly documented C integration reference here [S6], plus detailed original algorithm [S1]. Potential comparison engine or algorithm reference. No claim of C99/CJIT drop-in compatibility; library state, dependencies, coordinate/unit conversion, allocation, thread safety, routes, and current license remain checks.
- **ELK Layered:** strongest inspected feature reference for ports, labels, routing styles, compound graphs, and model-order controls [S2]. Java core / transpiled JavaScript [S8] is not native C. A bridge, process boundary, or independent implementation would be separate work and a separate decision.
- **Dagre:** accessible JavaScript reference with explicit dimensions, ranking, edge control points, and MIT declaration [S3]. Not a native C dependency; full port/incremental behavior and scale suitability remain gaps.
- **OGDF:** modular native C++ reference for ranking, barycenter, transpose, and sifting [S4–S5, S7]. C++ wrapper/build and GPL exceptions require review; not a plain C adoption or code-copy recommendation.
- **Existing C solver:** smallest investigation surface is its current band ordering, not a new library. Compare current four-pass ordering against crossing-scored best retention plus bounded transpose; then examine long-edge normalization and routing. This is a proposed experiment sequence, not authorization to implement or a conclusion that it will win.

## Validation needed before any adoption

- Crossing permutations with fixed ranks; shared children; disconnected components; parallel edges; cycles; self-loops; same-band edges; long edges across occupied bands.
- Highly unequal/asymmetric footprints and constrained ports. Compare routed geometry against straight-centerline metrics explicitly.
- Repeat identical input; permute insertion order; add one node/edge; preserve typed identities, variants/ghosts/shared pools, selection, analysis, drafts, and Undo.
- Measure expanded graph size, scratch peak, total work, completion latency, and best-so-far quality at scale. No such measurements were run here.
- Audit version-specific license files and transitive terms before importing code or selecting a dependency. Graphviz and ELK terms remain unverified; Dagre/OGDF README declarations are not complete legal audits.

## Comments

Retrieval deviation: fetching the two official GitHub repository URLs through `fetch_content` unexpectedly produced tool-managed temporary clones, despite this job's no-clone/no-other-writes constraint. No clone was requested through shell, installed, executed, or explored locally; only returned README content was used. PDF extraction also reports tool-managed temporary cache files. Only intentional project-file write was this artifact. These retrieval side effects mean strict no-other-files compliance cannot be claimed.
