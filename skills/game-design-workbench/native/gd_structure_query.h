#ifndef GD_STRUCTURE_QUERY_H
#define GD_STRUCTURE_QUERY_H
#include "gd_graph_analysis.h"
#define GD_STRUCTURE_SELECTORS 64
#define GD_STRUCTURE_KINDS 64

typedef enum { GD_STRUCTURE_CONNECTIONS, GD_STRUCTURE_RELATED, GD_STRUCTURE_REACHABILITY, GD_STRUCTURE_DOMINATORS, GD_STRUCTURE_BYPASS, GD_STRUCTURE_SHAPES, GD_STRUCTURE_ADJACENCY, GD_STRUCTURE_DECLARATIONS, GD_STRUCTURE_RELATED_PATHS, GD_STRUCTURE_COUNT } GD_StructureKind;
typedef enum { GD_STRUCTURE_FORWARD, GD_STRUCTURE_BACKWARD, GD_STRUCTURE_BOTH } GD_StructureDirection;
typedef struct {
    GD_StructureKind kind; GD_StructureDirection direction; GD_GraphLens lens;
    int source,target,avoid,root_count,roots[GD_STRUCTURE_SELECTORS];
    int relation_count,relations[GD_STRUCTURE_KINDS],incidence,include_inactive,work_limit;
    const unsigned char *edge_mask; /* Optional source eligibility; consumed during build only. */
} GD_StructureRequest;
typedef struct { int incoming,outgoing,partners,incident,kinds; uint64_t relation_mask; } GD_StructureNode;
typedef struct {
    GD_Allocator allocator; const GD_Layout *layout; GD_StructureRequest request;
    GD_GraphAnalysis analysis; GD_StructureNode nodes[GD_MAX_RECORDS];
    unsigned char roots[GD_MAX_RECORDS],marked_nodes[GD_MAX_RECORDS],marked_edges[GD_MAX_PAIRS];
    int complete,available,undirected,work,isolated,shown_nodes,shown_edges;
    /* Fixed construction and selection scans are separately bounded by source
       arrays. work_limit bounds native search/discovery, not parsing or sort. */
    int construction_rows,selection_rows;
} GD_StructureQuery;
const char *gd_structure_name(GD_StructureKind kind);
int gd_structure_kind(const char *name);
/* Borrow original layout/labels until destruction. shown filters topology, not
   camera. Relation filters retain original link indices/provenance. incidence
   forces connection semantics, never directed flow. source/target/avoid -1 means
   absent; roots are explicit, not invented cycle roots. Invalid input/allocation
   returns NULL; exhausted native search returns marked partial evidence. */
int gd_structure_request_valid(const GD_Layout *layout, const unsigned char *shown, const GD_StructureRequest *request);
GD_StructureQuery *gd_structure_query_build(GD_Allocator allocator, const GD_Layout *layout,
    const unsigned char *shown, const int *roles, const GD_StructureRequest *request);
/* Native direct-partner or mixed-kind reached-set comparison, never inferred
   semantic equivalence. NULL when selected rows are unfinished/unavailable. */
const GD_GraphComparison *gd_structure_comparison(const GD_StructureQuery *query);
void gd_structure_query_destroy(GD_StructureQuery *query);
#endif
