# Discovery journal — directed crossing algorithms

Append-only. Question/frame: [first-pass brief](first-pass-brief.md).

## D-001 setup

- User rejected force fallback and requested established algorithms minimizing crossing lines in directed graphs.
- Mode: bounded orientation. Focus on combinatorial ordering/hierarchical layout, practical engines, footprints/cycles/stability and limitations.
- Source authority: primary papers and official implementation documentation. AI output remains provisional until checked.
- Delegated source discovery will append searches, source trails, retained/rejected leads and reasons here. Parent will append primary-source checks after discovery child finishes to avoid overlapping writes.


## D-001 bounded discovery — chronological trail

1. Read first-pass brief and existing journal. Discovery-only child; no delegation, parent workflow, code edits, installs, clones, crawls or GUI operations.
2. Enabled web tools. Ran three search queries without page-content fetching:
   - `Graphviz dot algorithm Gansner Koutsofios North Vo 1993 drawing directed graphs median transpose virtual nodes`
   - `ELK Layered algorithm crossing minimization cycle breaking node sizes interactive stability`
   - `Dagre wiki layout algorithm crossing minimization OGDF Sugiyama sifting`
   - Search response: `mvykq8j48hbk0r`. Search synthesis treated as discovery signal, not evidence.
3. First fetch batch: Graphviz TSE93 paper; ELK Layered overview; ELK Layered algorithm reference; OGDF SugiyamaLayout; OGDF crossing-minimization index; official Dagre wiki. All six succeeded. Response `mvykqe7drye00y`. Retrieved relevant sections from stored content.
4. Followed OGDF index to SiftingHeuristic and GlobalSifting; followed ELK original Sugiyama DOI; fetched official Graphviz dot page. Three succeeded; DOI failed with `HTTP 406: Not Acceptable`. Response `mvykr1h83fa325`. No alternate host, access workaround or crawl attempted. Retained inaccessible original-paper lead.
5. Stopped at 10 unique fetch targets: nine successful pages/PDF and one inaccessible DOI. Stored-content retrieval did not add URLs. Wrote 26 observation rows to `discovery/D-001/source-candidates.csv`; repeated URLs intentionally preserve separate observations. All retained provenance labels are primary; labels do not imply correctness, accessibility or fitness.

### Source trails and exclusions

- Search led directly to official Graphviz-hosted Gansner et al. paper. Preferred it over ResearchGate copy, libguide and dot manual; those remain unfetched leads, not evidence.
- ELK overview directly names five phases, barycenter/median sweeps, greedy switch and phase-specific interactive settings. Original Sugiyama DOI was inaccessible. Original-paper title in CSV is bibliographic lead, not verified paper text.
- ELK constraining-model blog, individual option pages and ElkLayered.java appeared in search/links. Deferred to preserve cap. Need direct checks before claiming exact fixed-order/band configuration.
- OGDF index led to sifting module pages. GlobalSifting cites original `Global k-Level Crossing Reduction`, Bachmaier, Brandenburg, Brunner, Huebner, JGAA 15(5), 2011, pp. 631–659. Original not fetched. Sifting class page confirms availability/strategies, not detailed insertion-search mechanics or complexity.
- Search surfaced DeepWiki Dagre phase descriptions. Rejected as supporting evidence: official wiki available; third-party generated explanation unnecessary. Official wiki establishes algorithm lineage, not exact current crossing-order implementation.
- Dagre Recommended Reading points to Junger/Mutzel two-layer exact-versus-heuristic comparison, Barth et al. bilayer crossing counting, Brandes/Koepf coordinate assignment and Sander compound graphs. Retained as unfetched bibliographic trails; do not quote original papers through wiki as if directly reviewed.
- Graphviz dot page exposes rank, ordering, mclimit, nslimit and nslimit1 leads. Individual controls not fetched; index alone does not establish numerical work bounds.

### Exact supporting passages reviewed

Graphviz paper: <https://graphviz.org/documentation/TSE93.pdf>, section 3, PDF printed pages 13–16:

> “The vertex order within ranks determines the edge crossings in the layout, so a good ordering is one with few crossings.”

> “Heuristics are appropriate since minimizing edge crossings in layouts of ranked graphs is NP-complete, even for only two ranks [EMW].”

> “After rank assignment, edges between nodes more than one rank apart are replaced by chains of unit length edges between temporary or ‘‘virtual’’ nodes.”

> “The barycenter method defines the weight of v as the average of elements in P. The median method defines the weight of v as the median of elements in P.”

PDF extraction adds spaces around punctuation in some formulas; the preceding two-sentence definition normalizes only that extraction whitespace. Other quotes retain wording.

> “transpose repeatedly exchanges adjacent vertices on the same rank if this decreases the number of crossings.”

Paper section 2.1:

> “Of course these edges are only reversed internally; arrowheads in the drawing show the original direction.”

ELK overview: <https://eclipse.dev/elk/blog/posts/2025/25-08-21-layered.html>:

> “The layered algorithm or Sugiyama algorithm is in its implementation in ELK divided into five phases with intermediate processing slots: Cycle Breaking, Layer Assignment, Crossing Minimization, Node Placement, and Edge Routing.”

> “INTERACTIVE: This strategy uses previously calculated node positions to determine the ordering of nodes in a layer”

> “Additionally, one can greedily post-process the ordering to reduce edge crossings using the greedy switch heuristic.”

OGDF SugiyamaLayout: <https://ogdf.github.io/doc/ogdf/classogdf_1_1_sugiyama_layout.html>:

> “Determines whether the transpose step is performed after each 2-layer crossing minimization; this step tries to reduce the number of crossings by switching neighbored nodes on a layer.”

> “Each repetition (except for the first) starts with randomly permuted nodes on each layer. Deterministic behaviour can be achieved by setting runs to 1.”

OGDF GlobalSifting: <https://ogdf.github.io/doc/ogdf/classogdf_1_1_global_sifting.html>:

> “This option determines, how many times the global sifting is repeated. Each repetition starts from permutation returned by the previous one. The first repetition starts from random permutation.”

Dagre wiki: <https://github.com/dagrejs/dagre/wiki>:

> “Dagre requires only very basic information to lay out graphs, such as the dimensions of nodes.”

> “We made some adjustments to get tighter graphs when node and edges sizes vary greatly.”

### Limits for parent synthesis

- Evidence supports established discrete layered ordering pipeline and optional local refinement, not an adoption decision or benchmark winner.
- No measured time/memory evidence at 4096 nodes / 32768 edges or <=1 MiB scratch. Dummy expansion and repeated crossing counts need explicit accounting. Paper's 24 outer iterations do not bound inner transpose work.
- Previous-position strategies are stability controls, not proof of mental-map preservation. Determinism and incremental stability differ.
- Bounding rectangles are supported conceptually; asymmetric badges/captions need caller-computed footprints and anchor handling. No direct evidence for project-specific shared identities, ghosts, Undo or MDA unit scaling.
- Layered algorithms constrain nodes to ranks; compatibility with unconstrained freely movable roots remains unproved. Given-level OGDF call is useful retained-band lead, not complete integration contract.
- Crossing minimization orders nodes/dummies/ports; routed-curve crossings can differ from current straight-centerline metric. Routing and coordinate assignment remain separate phases.

## Parent parallel trail — appended after child completion

1. Queried `site.graphviz.org dot layered directed graphs edge crossings mincross`, `site.eclipse.dev elk layered crossing minimization layer sweep cycle breaking greedy switch`, and `site.graphviz.org theory gansner drawing directed graphs dot median transpose`. Search response `mvykpxhujis8t9`; background content response `mvykpxhuldbvte`. Search accidentally requested `includeContent`, automatically fetching 11 URLs. This exceeds planned parent six-fetch subcap; no claim of strict subcap compliance.
2. Selected/reviewed Graphviz dot, Gansner paper, ELK phase overview and ELK Layered reference. Fetch response `mvykq35eln65zs`. Read extracted PDF sections 2.1, 3 and 4; retained bounded quotations in `first-blush/checked-passages.md`.
3. Read local `gd_layout_directed.h` completely and searched directed/Free dispatch in `gd_layout.c`. Found existing four-pass barycenter-style ordering; no crossing score, best-order retention or adjacent transposition in inspected helper. This narrows recommendation away from simply adding an already-present heuristic.
4. Queried `site.ogdf.github.io doc SugiyamaLayout SiftingHeuristic GlobalSifting upward planarization` and `site.d3js.org d3 hierarchy tree Reingold Tilford Buchheim`, without content fetching. Search response `mvykrf787h4a4f`. Selected OGDF SiftingHeuristic and D3 tree, fetch response `mvykrn3vgyekoy`. Tree lead tests user's earlier branch-parent analogy; retained limitation to genuine trees or explicit layout-only backbones. Upward planarization remains an unreviewed alternate lead, not recommendation.
5. Inspected both completed child artifacts. Discovery: Pi/gpt-6.1-sol medium, 176.8 seconds; analyst: same harness/model/thinking, 206.7 seconds. Durations from saved child prompt/final-message timestamps. No child failure or pending approval. Kept workflow-created panes available, user focus unchanged.
6. Validated discovery CSV: 26 observations, 10 unique URLs, required fields populated. Preserved duplicate observations and inaccessible DOI. Central recommendation supported by overlapping independently checked primary sources; not by search synthesis alone.
7. Recorded first-pass recommendation and gaps in `first-blush/recommendation.md`. Stop at orientation; no systematic capture/register or next algorithm implementation started. Existing source trials and GUI state untouched.

## Comments

Recorded target lists contain 21 distinct URLs across parent/children before redirect/canonical normalization, within overall 24-target cap. Parent fetch subcap exceeded through automatic content fetching. Analyst reports tool-managed GitHub clones and PDF caches despite no-clone instruction; no cloned code installed/executed. Caches left under tool management. Full source-map/register workflow remains deferred, not falsely marked complete.
