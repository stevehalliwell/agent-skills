#ifndef GD_RESOURCE_ROUTES_H
#define GD_RESOURCE_ROUTES_H
#include "gd_resource_graph.h"
#define GD_RESOURCE_ROUTE_WITNESSES 64
#define GD_RESOURCE_ROUTE_STEPS 64 /* At most 32 case activations. */
#define GD_RESOURCE_ROUTE_WORK (16 * 1024 * 1024)
typedef enum { GD_RESOURCE_ENABLE, GD_RESOURCE_DEBIT, GD_RESOURCE_CREDIT, GD_RESOURCE_STATE_PROPOSAL } GD_ResourceArcKind;
typedef struct { int from,to,relation,next; GD_ResourceArcKind kind; } GD_ResourceArc;
typedef struct { int count,unresolved,proposed; int nodes[GD_RESOURCE_ROUTE_STEPS+1],arcs[GD_RESOURCE_ROUTE_STEPS]; } GD_ResourceWitness;
/* Caller-owned scratch/results, never source state. Direct guarded witnesses,
   not executable plans. Unknown-direction writes and invocation expansion remain
   explicit frontiers. Graph/snapshots must outlive scratch/results. */
typedef struct {
    GD_Allocator allocator; const GD_ResourceGraph *graph;
    int from,to,from_bindings,to_bindings,accepted_only,queried,work,work_limit,limited,depth_limited,cycle_edges;
    int arc_count,witness_count,unknown_writes,unexpanded_calls,excluded_cases;
    int selector_matches,selector_candidates[GD_RESOURCE_ROUTE_WITNESSES];
    int head[GD_RESOURCE_GRAPH_NODES],first_relation[GD_RESOURCE_GRAPH_NODES];
    int case_head[GD_RESOURCE_GRAPH_NODES],case_next[GD_RESOURCE_GRAPH_RELATIONS];
    unsigned char eligible[GD_MAX_RULE_CASES],case_unresolved[GD_MAX_RULE_CASES],goals[GD_RESOURCE_GRAPH_NODES];
    unsigned char marked_nodes[GD_RESOURCE_GRAPH_NODES],marked_relations[GD_RESOURCE_GRAPH_RELATIONS],side_relations[GD_RESOURCE_GRAPH_RELATIONS];
    GD_ResourceArc arcs[GD_RESOURCE_GRAPH_RELATIONS];
    GD_ResourceWitness witnesses[GD_RESOURCE_ROUTE_WITNESSES];
} GD_ResourceRoutes;
GD_ResourceRoutes *gd_resource_routes_create(GD_Allocator allocator);
void gd_resource_routes_destroy(GD_ResourceRoutes *routes);
/* Exact opaque ID wins; qualified source/pool reference can return ambiguous.
   Returns node, -1 absent, -2 ambiguous, -3 work limit. Candidate IDs retained. */
int gd_resource_select(GD_ResourceRoutes *routes, const GD_ResourceGraph *graph, const char *selector);
int gd_resource_query(GD_ResourceRoutes *routes, const GD_ResourceGraph *graph, int from, int to, int accepted_only, int work_limit);
void gd_resource_route_mark(GD_ResourceRoutes *routes, int witness);
const char *gd_resource_arc_name(GD_ResourceArcKind kind);
/* Resource report overload uses same schema and declaration serializer. */
int gd_write_resource_query(FILE *output, const GD_Model *model, const GD_ResourceGraph *graph, const GD_ResourceRoutes *routes, int json, const char *fields);
#endif
