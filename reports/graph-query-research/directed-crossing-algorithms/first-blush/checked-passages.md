# Parent checks — bounded primary-source passages

Orientation evidence only. Not exhaustive capture, benchmark or license review. Source identifiers P-001..P-006 local to this artifact. Original pages fetched through Pi web tools; bounded passages retained below. No full retained-evidence workflow authorized.

## P-001 — Graphviz dot

URL: https://graphviz.org/docs/layouts/dot/
Fetch response: `mvykq35eln65zs`, URL 0.

Direct quotation, opening description:
> The layout algorithm aims edges in the same direction (top to bottom, or left to right) and then attempts to avoid edge crossings and reduce edge length.

Source-supported paraphrase: dot targets directed hierarchical layout, not arbitrary force equilibrium. Page lists rank constraints, ordering and mincross iteration controls. No quality/runtime guarantee for our inputs.

## P-002 — Gansner et al., A Technique for Drawing Directed Graphs

URL: https://graphviz.org/documentation/TSE93.pdf
Fetch response: `mvykq35eln65zs`, URL 1; extracted 33-page PDF read locally, especially sections 2.1, 3 and 4.

Direct quotations:
- Section 2.1, p. 6: “Of course these edges are only reversed internally; arrowheads in the drawing show the original direction.”
- Section 3, p. 13: “The vertex order within ranks determines the edge crossings in the layout, so a good ordering is one with few crossings.”
- Section 3, p. 13: “Heuristics are appropriate since minimizing edge crossings in layouts of ranked graphs is NP-complete, even for only two ranks”.
- Section 3, p. 14: “transpose repeatedly exchanges adjacent vertices on the same rank if this decreases the number of crossings.”
- Section 4, p. 17: “The first step assigns X coordinates to all nodes (including virtual nodes), subject to the order within ranks already determined.”

Source-supported paraphrase:
- Split long edges with layout-only virtual nodes on intermediate ranks before crossing ordering.
- Initialize using graph traversal; alternate weighted-median sweeps and local transpositions; retain best counted ordering, not merely latest ordering. Paper uses a finite outer iteration count.
- Compute coordinates after ordering, preserving separation of rectangular node boxes; route splines afterward.
- Section 4.3 makes explicit that virtual nodes enlarge the auxiliary problem. Do not assume ordinary expansion fits our fixed capacities.

Limits: historical algorithm description, not verification of current Graphviz internals. Paper-reported comparative percentages/theoretical bounds not extrapolated to our graphs or to general median variants.

## P-003 — ELK Layered overview

URL: https://eclipse.dev/elk/blog/posts/2025/25-08-21-layered.html
Fetch response: `mvykq35eln65zs`, URL 2.

Direct quotations:
- Opening: “Cycle Breaking, Layer Assignment, Crossing Minimization, Node Placement, and Edge Routing.”
- Crossing Minimization: “Additionally, one can greedily post-process the ordering to reduce edge crossings using the greedy switch heuristic”.
- Cycle Breaking, GREEDY: “uses random decisions on ties”.

Source-supported paraphrase: implementation offers barycenter and median layer sweeps, interactive ordering from previous positions, model-order-aware cycle breaking and separate coordinate/routing phases. Avoid assuming determinism from the algorithm family alone: strategy, tie policy and seed matter. Documentation's interactive options do not prove layout stability on our refresh/filter sequences.

## P-004 — ELK Layered reference

URL: https://eclipse.dev/elk/reference/algorithms/org-eclipse-elk-layered.html
Fetch response: `mvykq35eln65zs`, URL 3.

Direct quotation, Description:
> This implementation supports different routing styles (straight, orthogonal, splines); if orthogonal routing is selected, arbitrary port constraints are respected

Source-supported paraphrase: supports self-loops, multi-edges, edge labels, ports and compound/cross-hierarchy edges. These capabilities make it a useful feature-rich reference, not proof of compatibility with our C ABI, operation scratch budget or semantic identity contract.

## P-005 — OGDF SiftingHeuristic

URL: https://ogdf.github.io/doc/ogdf/classogdf_1_1_sifting_heuristic.html
Fetch response: `mvykrn3vgyekoy`, URL 0.

Direct quotation, class description:
> The sifting heuristic for 2-layer crossing minimization.

Direct observation: API offers LeftToRight, DescDegree and Random strategies. This confirms an implemented layered crossing-minimization alternative. Page alone does not establish resource bounds or improvement on our examples.

## P-006 — D3 tree

URL: https://d3js.org/d3-hierarchy/tree
Fetch response: `mvykrn3vgyekoy`, URL 1.

Direct quotation, opening:
> The tree layout produces tidy node-link diagrams of trees using the Reingold–Tilford “tidy” algorithm, improved to run in linear time by Buchheim et al.

Interpretation: relevant to requested parent/branch arrangement only when input is a real tree or a separately disclosed layout-only tree backbone. Not a general solution for shared-parent or cyclic graphs. Other edges must remain visible; do not invent source hierarchy or drop cross-links. Page does not establish arbitrary asymmetric card-footprint support.

## Local code observation — not external evidence

Read `skills/game-design-workbench/native/gd_layout_directed.h`, functions `directed_order()` and `directed_arrange()`.

- Existing constrained directed arrangement already runs four alternating mean-neighbor reorder passes.
- Orders by group/layer, normalized neighbor rank average and prior-order tie-break.
- Considers neighbors across different layers directly; no layout-only intermediate virtual-node chain in this helper.
- No crossing-count objective, retained best ordering or adjacent-swap refinement in this helper.
- Coordinate assignment then spaces semantic card dimensions within existing layers/groups.
- Free force integration is separate: `gd_layout.c` calls `crossing_forces()` only in `GD_LAYOUT_FORCE`.

Inference: full ranked ordering/refinement is a meaningful next research direction, but calling barycenter sorting wholly new would be wrong. Applying a ranked seed and then unrestricted force relaxation could undo its ordering; preserving the outcome needs an explicit layout policy, not a silently changed Free graph contract.
