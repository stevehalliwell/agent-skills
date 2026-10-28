#ifndef GD_GRAPH_ANALYSIS_H
#define GD_GRAPH_ANALYSIS_H
#include "gd_layout.h"

#define GD_GRAPH_ANALYSIS_PAIRS 16384
#define GD_GRAPH_ANALYSIS_HASH 32768
#define GD_GRAPH_ANALYSIS_WORK 16777216

typedef struct { int owner, neighbor, edge, endpoint; } GD_GraphFeature;
typedef struct { int a, b, shared, first_a, first_b; } GD_GraphMatch;
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
    int shown_nodes, shown_edges, feature_count, pair_count, complete, work;
} GD_GraphAnalysis;
/* Zero-initialize once. NULL mask includes all nodes. Inactive edges count only
   when include_inactive is true. Recomputes after topology/display-mask changes;
   returns 1 on recompute, 0 when cached. Complete covers this projection only.
   At most 2*edges sorted occurrences, then bounded inverted-index pair joins.
   Budget/pair exhaustion marks partial; top-k display is not the compute bound. */
int gd_graph_analyze(const GD_Layout *layout, const unsigned char *shown,
                     int include_inactive, GD_GraphAnalysis *analysis);
/* Retained match, or -1. Partial result cannot establish absence of a match. */
int gd_graph_match(const GD_GraphAnalysis *analysis, int node, int peer);
#endif
