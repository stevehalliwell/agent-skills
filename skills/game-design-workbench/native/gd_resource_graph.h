#ifndef GD_RESOURCE_GRAPH_H
#define GD_RESOURCE_GRAPH_H
#include "gd_rule_analysis.h"

#define GD_RESOURCE_GRAPH_NODES 4096
#define GD_RESOURCE_GRAPH_RELATIONS 32768
#define GD_RESOURCE_GRAPH_WORK (64 * 1024 * 1024)

typedef enum { GD_RESOURCE_CASE, GD_RESOURCE_FACET, GD_RESOURCE_ROLE } GD_ResourceNodeKind;
typedef enum { GD_RESOURCE_REQUIREMENT, GD_RESOURCE_EXCLUSION, GD_RESOURCE_STATE_WRITE, GD_RESOURCE_QUANTITY_WRITE, GD_RESOURCE_ROLE_BINDING, GD_RESOURCE_PRESERVATION, GD_RESOURCE_INVOCATION } GD_ResourceRelationKind;
typedef struct {
    GD_ResourceNodeKind kind;
    int case_index, participant, axis, pool, record;
    GD_RuleResolution resolution;
    /* A pool reference selects a shared facet only with a resolved axis and
       non-conflicting binding. Text is borrowed from the rules snapshot. */
    const char *pool_reference;
} GD_ResourceNode;
typedef struct {
    GD_ResourceRelationKind kind;
    int facet, activation, participant, declaration, line, delta_recognized, conditional;
    GD_RuleQuantityValue delta; /* Signed change only; no claim about final bounds. */
    /* declaration indexes condition_tokens, state_effects, assignments, pools,
       preservations or compiled calls. ROLE_BINDING activation indexes a role
       node; INVOCATION facet indexes a callee case, not a resource facet.
       Raw conditions remain attached to their participants. */
} GD_ResourceRelation;
typedef struct {
    GD_Allocator allocator;
    const GD_Model *model;
    const GD_Rules *rules;
    const GD_RuleAnalysis *calls; /* Borrowed after call binding. */
    int node_count, relation_count, work, limited, unexamined_cases, unexamined_roles;
    int composition_unexamined, preservation_unexamined, unresolved_facets, unknown_case_coverage, calls_bound;
    GD_ResourceNode nodes[GD_RESOURCE_GRAPH_NODES];
    GD_ResourceRelation relations[GD_RESOURCE_GRAPH_RELATIONS];
} GD_ResourceGraph;

/* Partial direct declarations only. No context, invocation expansion,
   routes or production direction. ALL/OTHER enumerate explicitly declared
   actor axes; OTHER relations remain conditional on whole-case effects.
   Relations retain whole original conditions; they are not independent AND
   requirements or guarantees of an applicable outcome. Borrow model/rules unchanged until graph destruction. */
GD_ResourceGraph *gd_resource_graph_build(GD_Model *model, const GD_Rules *rules);
/* Add explicit compiled call/case edges, retaining mapping and guard provenance.
   No call is unconditional and no mapped effect is flattened into its caller.
   Call once; analysis must outlive graph and match its model/rules snapshots. */
int gd_resource_graph_bind_calls(GD_ResourceGraph *graph, const GD_RuleAnalysis *analysis);
void gd_resource_graph_destroy(GD_ResourceGraph *graph);
/* Read-only retained incidence report, not directed routes/composed outcomes.
   Valid graph from builder/bind_calls; borrowed snapshots must remain alive. */
int gd_write_resource_result(FILE *output, const GD_Model *model, const GD_ResourceGraph *graph, int json, const char *fields);
#ifdef GD_WITH_VIEW
int gd_resource_view(GD_Model *model, GD_ResourceGraph *graph, int frames, const char *screenshot);
#endif
#endif
