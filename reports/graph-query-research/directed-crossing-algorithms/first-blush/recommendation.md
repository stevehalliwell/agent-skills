# Directed graph crossing minimization — orientation

Status: historical bounded orientation; no replacement adopted. User subsequently removed unsuccessful crossing experiments, retained node-overlap separation and deferred crossing work until before final review. Steps 4/5 proceed; proposal below is a future revisit candidate, not active implementation. Research pass changed no code, installed no dependency and manipulated no viewer.

## Recommendation

Evaluate **Sugiyama-style layered ordering with crossing-scored refinement**, not another continuous uncrossing force. First candidate: median/barycenter sweeps, retain best scored ordering, then bounded adjacent transposition. Use **Graphviz dot as an offline reference**, not an assumed drop-in dependency.

This targets ordering directly. Between two fixed parallel ranks, edges cross when endpoint orders disagree. Spreading nodes without changing those orders cannot remove that inversion. This is a geometric explanation, not proof of the cause of every rejected layout.

## Algorithms worth testing

1. **Median / barycenter layer sweeps.** Reorder each rank from neighbor positions, alternating sweep direction. Gansner et al. describe both heuristics; ELK exposes both. Compare them rather than assuming median always wins. [Primary passages P-002/P-003](checked-passages.md).
2. **Adjacent transposition / greedy switch.** After a sweep, test neighboring nodes in a rank and swap when relevant crossings decrease. Score both neighboring ranks where applicable; preserve best complete ordering. This adds an explicit crossing objective absent from our current helper. [Gansner paper, section 3](https://graphviz.org/documentation/TSE93.pdf); [ELK phase overview](https://eclipse.dev/elk/blog/posts/2025/25-08-21-layered.html).
3. **Sifting / global sifting.** Follow-up when local swaps plateau. Sifting can examine longer insertion moves rather than only immediately improving adjacent swaps. OGDF documents available modules, but detailed mechanics, costs and original global-sifting paper need further review before implementation. [Discovered modules and limits](../discovery/D-001/source-candidates.csv); [analyst interpretation](answer-analyst.md).
4. **Reingold–Tilford / Buchheim tidy tree.** Matches branch/parent arrangement for true trees. Not a general solution for shared children or cyclic directed graphs. A layout-only tree backbone would need an explicit policy retaining every non-tree edge and original identity. No authored reparenting. [Official D3 tree reference](https://d3js.org/d3-hierarchy/tree).

## Full pipeline matters

Gansner's paper and ELK documentation separate these concerns:

1. Handle cycles in a transient layout representation; keep original displayed arrow direction.
2. Assign ranks, or retain required semantic bands.
3. Represent long edges through intermediate ranks with layout-only virtual nodes/segments.
4. Optimize node/virtual-node/port ordering using scored sweeps and local refinement.
5. Assign coordinates while preserving order and spacing complete rendered footprints.
6. Route actual edges through reserved space; validate rendered crossings and obstacle clearance.

Do not discard long-edge route geometry and then assume a single center-to-center line retains the optimized result. Ranking, ordering, coordinate assignment and routing optimize different things. Some crossings remain unavoidable under graph or presentation constraints; ranked crossing minimization is NP-complete in general [P-002]. No zero-crossing guarantee.

## Existing code is a partial starting point

`skills/game-design-workbench/native/gd_layout_directed.h:directed_order()` already runs **four alternating barycenter-style passes**, with prior-order ties. It averages normalized neighbor ranks across existing layers. Coordinate placement already uses semantic card dimensions.

Missing in inspected helper: crossing counter, retained best ordering, scored adjacent transposition/sifting and intermediate virtual-node treatment. This is not simply “add barycenter sorting.” Free force mode is separate and does not use this directed ordering.

Smallest investigation surface: compare existing ordering against scored best-order retention plus bounded transpose on existing directed fixtures. Then assess long edges and renderer routes. No code work authorized by this recommendation alone.

## References and adoption boundaries

- **Graphviz dot:** directed layout reference; original paper explains ordering, rectangular boxes, cycles and routing. Official C library guide provides native integration lead, but no CJIT/ABI, allocation, deployment, current thread-safety or license compatibility established. Prefer an external baseline first.
- **ELK Layered:** feature reference for ports, labels, loops, multi-edges, routing and model-order controls. Java/JavaScript implementation is not native C.
- **Dagre:** accessible JavaScript reference with explicit node dimensions and rectangular edge intersection/control points. Not a native C dependency or quantified scale guarantee.
- **OGDF:** C++ reference with pluggable median/barycenter, transpose and sifting. License exceptions and C++ build/wrapper need review before reuse. Do not copy implementation code without that review.

Detailed source passages, URLs and source-specific cautions: [parent checks](checked-passages.md), [independent analyst](answer-analyst.md), [discovery observations](../discovery/D-001/source-candidates.csv). Source discovery is not an exhaustive literature review.

## Material trade-offs still open

- **Layered versus Free graph:** retained ranks impose structure, unlike freely drifting roots. A ranked initial seed can remain manually movable, but later unrestricted forces/dragging can undo its crossing reduction. Do not silently replace Free graph or existing band semantics. Non-layered planarization and upward-planarization remain unreviewed alternative leads; this pass does not establish layered layout as the only solution.
- **Footprints:** pass full asymmetric label/caption/badge bounds, not only card sizes. Preserve anchor-to-box-center offsets and MDA's normalized units.
- **Work and memory:** naive dummy expansion depends on total edge rank spans, not only real node count. It may exceed fixed capacities or <=1 MiB operation scratch. Bound expansion, sweeps, crossing scores and routing separately; current per-frame caps do not establish a one-shot layout budget.
- **Stability:** deterministic input/ties/seed differs from small-edit stability. Prior positions/model order help constrain motion but may prevent crossing improvements. Interactive options are not a guarantee.
- **Metrics:** current synthetic crossing measurement is strict centerlines and limited to <=256 edges. Compare actual routed crossings, node/label intersections, movement, total work and scratch before claiming improvement.

No reviewed source proves acceptable quality/runtime at **4096 nodes / 32768 edges** under our constraints. Local comparison could change the proposed candidate or favor an external engine. Full retained-evidence research, version/license audit and benchmarks remain separate follow-ups, not silently started here.

## First-pass checks

- Two independent Pi / `openai/gpt-6.1-sol` medium jobs completed: source discovery **176.8 s**, analyst **206.7 s**, measured from child prompt to final message. Both results inspected; panes left available without changing user focus.
- Discovery CSV: **26 observations / 10 unique URLs**, required fields validated; repeated observations preserved. Original Sugiyama DOI returned HTTP 406; no access workaround attempted.
- Parent checked six selected primary references and existing directed/Free dispatch source. Research Markdown local links and discovery CSV validated; recorded target union checked at 21 URLs. `git diff --check` passed, apart from existing informational LF-to-CRLF warning for `todo.md`. No compile or viewer checks needed for documentation-only research.
- Stop at orientation. Full register, normalized queue, source-map and systematic capture deferred; first-pass artifacts retain trails and bounded passages.

## Comments

Retrieval deviations: parent's initial `includeContent` search automatically fetched 11 URLs before six selected reviews, exceeding its planned six-fetch subcap. Across recorded parent/child target lists, 21 distinct URLs remain within overall 24-URL cap; redirects/canonical equivalents not normalized. Analyst's GitHub fetches created tool-managed temporary clones; PDF fetches created temporary extraction caches. No cloned code installed or executed. Caches left under tool management; no project source changes. These deviations prevent claiming strict no-clone/per-parent-fetch compliance.
