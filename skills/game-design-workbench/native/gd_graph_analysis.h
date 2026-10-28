#ifndef GD_GRAPH_ANALYSIS_H
#define GD_GRAPH_ANALYSIS_H
#include "gd_layout.h"

#define GD_GRAPH_ANALYSIS_PAIRS 16384
#define GD_GRAPH_ANALYSIS_HASH 32768
#define GD_GRAPH_ANALYSIS_WORK 16777216

typedef enum { GD_GRAPH_ANY, GD_GRAPH_SAME_ROLE, GD_GRAPH_CHANGED_ROLE, GD_GRAPH_LENS_COUNT } GD_GraphLens;
extern const char *const gd_graph_lens_names[GD_GRAPH_LENS_COUNT];
typedef struct { int owner, neighbor, edge, endpoint, relation, owner_end; } GD_GraphFeature;
typedef struct { int a, b, shared, first_a, first_b; } GD_GraphMatch;
/* Neighbor nodes have one class; link rows may combine classes from their two
   incident roles (for example a direct A--B link). Other-role means shared
   identity without qualifying role, not a unique ingredient. */
enum { GD_GRAPH_SHARED=1, GD_GRAPH_A_ONLY=2, GD_GRAPH_B_ONLY=4, GD_GRAPH_OTHER_ROLE=8 };
typedef struct {
    int valid, a, b, shared, a_only, b_only, other_role;
    unsigned char nodes[GD_MAX_RECORDS], edges[GD_MAX_PAIRS];
    int first_a[GD_MAX_RECORDS], first_b[GD_MAX_RECORDS];
} GD_GraphComparison;
typedef struct {
    unsigned char ancestors[GD_MAX_RECORDS], edges[GD_MAX_PAIRS];
    int arcs[64], retained, length, ancestor_count, node;
    int candidates[GD_MAX_RECORDS], candidate_count, candidate;
} GD_PathSelection;
typedef struct {
    unsigned char edges[GD_MAX_PAIRS];
    int arcs[64], retained, length, target, candidate, fork, rejoin;
    int valid, complete, found, work, limit;
} GD_PathBypass;
typedef struct {
    int out_head[GD_MAX_RECORDS], in_head[GD_MAX_RECORDS];
    int out_next[2*GD_MAX_PAIRS], in_next[2*GD_MAX_PAIRS];
    int order[GD_MAX_RECORDS], number[GD_MAX_RECORDS+1], dominator[GD_MAX_RECORDS+1];
    int cursor[GD_MAX_RECORDS], stack[GD_MAX_RECORDS], parent_arc[GD_MAX_RECORDS];
    int dominated[GD_MAX_RECORDS], incoming[GD_MAX_RECORDS];
    unsigned char roots[GD_MAX_RECORDS], reached[GD_MAX_RECORDS];
    int valid, complete, undirected, root_count, reachable_count, work, limit;
    GD_PathSelection selection;
    GD_PathBypass bypass;
} GD_GraphPaths;
/* Presentation keys, not node selection. Remap across source reload. */
typedef struct { int keys[GD_MAX_RECORDS], count; int enabled, declared, lost, compare; } GD_PathControls;
#define GD_CLOSURE_WORDS ((GD_MAX_RECORDS+63)/64)
/* Strict transitive closure. Self reachability is retained only as cycle
   evidence. Completed rows are exact for this shown typed projection;
   boundary seeds describe missing source dependencies, not inferred edges.
   About 2.7 MiB per cache, owned by view scratch, never a model snapshot. */
typedef struct {
    uint64_t rows[GD_MAX_RECORDS][GD_CLOSURE_WORDS];
    int head[GD_MAX_RECORDS], next[2*GD_MAX_PAIRS], queue[GD_MAX_RECORDS];
    int direct[GD_MAX_RECORDS], count[GD_MAX_RECORDS], users[GD_MAX_RECORDS];
    unsigned char eligible[GD_MAX_RECORDS], boundary[GD_MAX_RECORDS];
    unsigned char finished[GD_MAX_RECORDS], unknown[GD_MAX_RECORDS], cycle[GD_MAX_RECORDS];
    int valid, complete, relation, reverse, undirected, work, limit, row_count;
    uint64_t projected[GD_MAX_PAIRS];
    int projection_valid, projected_count, projected_direction;
    int selected, peer_count, peers[GD_MAX_RECORDS], shared[GD_MAX_RECORDS];
    GD_GraphComparison comparison;
} GD_GraphClosure;
/* Caller-allocated scratch/result; not for a small thread stack. Owns no strings
   or model pointers. All node/edge indices refer to the current layout. Two
   incident links establish a shared neighbor, not equivalent semantic roles.
   Cache compares topology/masks exactly; camera/positions never affect scope. */
typedef struct {
    GD_GraphFeature features[GD_MAX_PAIRS*2];
    GD_GraphMatch pairs[GD_GRAPH_ANALYSIS_PAIRS];
    int slots[GD_GRAPH_ANALYSIS_HASH];
    int matched[GD_MAX_RECORDS], neighbors[GD_MAX_RECORDS], incident[GD_MAX_RECORDS];
    int keys[GD_MAX_RECORDS];
    unsigned char shown[GD_MAX_RECORDS];
    GD_LayoutEdge edges[GD_MAX_PAIRS];
    int initialized, include_inactive, node_count, edge_count;
    GD_GraphLens lens;
    GD_GraphComparison comparison;
    GD_GraphPaths paths;
    GD_GraphClosure foundations;
    int shown_nodes, shown_edges, feature_count, pair_count, complete, work;
} GD_GraphAnalysis;
/* Zero-initialize once. NULL mask includes all nodes. Inactive edges count only
   when include_inactive is true. Cached analysis->shown may serve as mask for
   a lens-only update. Recomputes after topology/tag/lens/display-mask changes;
   returns 1 on recompute, 0 when cached. Complete covers this projection only.
   At most 2*edges sorted occurrences, then bounded inverted-index pair joins.
   Budget/pair exhaustion marks partial; top-k display is not the compute bound.
   Role = projection relation tag + incident endpoint. Same-role requires at
   least one equal role; changed-role requires at least one unequal role.
   Alternatives may satisfy both lenses. Counts still deduplicate neighbors. */
int gd_graph_analyze(const GD_Layout *layout, const unsigned char *shown,
                     int include_inactive, GD_GraphLens lens, GD_GraphAnalysis *analysis);
/* Retained match, or -1. Partial result cannot establish absence of a match. */
int gd_graph_match(const GD_GraphAnalysis *analysis, int node, int peer);
/* Exact one-hop comparison over the shown projection, even when global pair
   discovery stopped early. A is selected node, B its peer; order matters.
   Visits bounded feature/node/link arrays and uses binary role lookups. No
   new discovery, source inference or global query-budget mutation. Returns
   NULL for invalid/hidden/same nodes. Borrow until next compare/analyze call. */
const GD_GraphComparison *gd_graph_compare(GD_GraphAnalysis *analysis, int a, int b);
/* Rooted path domination over current shown projection. Undirected mode walks
   incidences both ways; it does not infer flow. Virtual start has index
   node_count and is never a source record. No shown roots means unavailable.
   Work limit (0..GD_GRAPH_ANALYSIS_WORK) charges DFS arcs, pass visits,
   predecessor scans and parent walks; exhausted iterations are NOT proofs.
   Results exclude unreachable nodes; dominated counts exclude self. */
const GD_GraphPaths *gd_graph_paths(GD_GraphAnalysis *analysis, const unsigned char *roots, int undirected, int work_limit);
const GD_PathSelection *gd_graph_path_selection(GD_GraphAnalysis *analysis, int selected);
/* Candidates are nonmandatory nodes on selected DFS support path, closest
   first. -1 uses selection.candidate. A separate bounded vertex-avoiding BFS
   retains a second path under identical starts/scope. No opportunity/learning
   semantics. complete=0 means no witness conclusion. F/R delimit two witness
   branches; fork=-1 denotes separate starts via virtual root, not authored fork.
   Borrow until next bypass/selection/path recomputation or analyze invalidation. */
const GD_PathBypass *gd_graph_path_bypass(GD_GraphAnalysis *analysis, int selected, int candidate, int work_limit);
/* One typed relation. reverse follows b->a; undirected follows both endpoint
   directions (connections, not flow). NULL eligible includes shown nodes;
   NULL boundary means no external source-boundary facts. Cache is invalidated
   with graph analysis; pass its foundations member. Work charges queue visits
   and arcs; partial rows never establish absence. Counts/users are lower bounds
   on exhaustion. No knowledge, mastery, source or path-budget mutation. */
const GD_GraphClosure *gd_graph_closure(GD_GraphAnalysis *analysis, const unsigned char *eligible,
    const unsigned char *boundary, int relation, int reverse, int undirected, int work_limit);
int gd_graph_closure_contains(const GD_GraphClosure *closure, int root, int node);
/* Pair branches cover entire strict closures, not just one supporting path.
   Completed A/B rows required. b=-1 is unpaired A. Exact local results survive
   partial global discovery. Borrow until next selection/closure/analyze. */
const GD_GraphComparison *gd_graph_closure_compare(GD_GraphAnalysis *analysis, int a, int b);
/* Automatically choose first source-order shared peer when old peer no longer
   qualifies. NULL on unavailable local row; no selected-node re-rooting. */
const GD_GraphComparison *gd_graph_closure_choose(GD_GraphAnalysis *analysis, int a, int *peer);
/* Skills: reverse projected prerequisite->dependent. Relationships: forward
   authored dependent->prerequisite. Other projections return NULL. Source
   boundaries include unresolved/wrong-role/hidden/missing projected links. */
const GD_GraphClosure *gd_graph_foundations(const GD_Model *model, GD_GraphAnalysis *analysis, int authored_direction);
/* Sorted unique keys; 1 added, 0 duplicate, -1 capacity. No allocation. */
int gd_graph_path_start(GD_PathControls *controls, int key);
const GD_GraphPaths *gd_graph_paths_prepare(GD_GraphAnalysis *analysis, const GD_PathControls *controls,
                                         const unsigned char *declared, int undirected);
/* Remap source-record keys by unique qualified reference before old snapshot
   destruction. Missing/ambiguous starts increment lost; never guess identity. */
void gd_graph_paths_remap(GD_PathControls *controls, const GD_Model *old, const GD_Model *fresh);
#endif
