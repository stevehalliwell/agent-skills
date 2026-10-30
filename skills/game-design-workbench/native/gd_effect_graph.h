#ifndef GD_EFFECT_GRAPH_H
#define GD_EFFECT_GRAPH_H
#include "gd_resource_graph.h"

#define GD_EFFECT_NODES 4096
#define GD_EFFECT_LINKS 32768
#define GD_EFFECT_WORK (64 * 1024 * 1024)
#define GD_EFFECT_ID_CAPACITY (2 * GD_MAX_PATH)

typedef enum { GD_EFFECT_ACTIVATION, GD_EFFECT_STATE, GD_EFFECT_GAIN, GD_EFFECT_LOSS,
               GD_EFFECT_NO_CHANGE, GD_EFFECT_QUANTITY, GD_EFFECT_PRESERVE, GD_EFFECT_UNKNOWN } GD_EffectKind;
typedef enum { GD_EFFECT_DECLARATION, GD_EFFECT_INVOCATION, GD_EFFECT_CALL_FRONTIER } GD_EffectLinkKind;
typedef struct {
    GD_EffectKind kind;
    int activation, axis, relation; /* Resource node/record/relation indices; -1 when inapplicable. */
    const char *value; /* Borrowed canonical named state, never an inferred value. */
} GD_EffectNode;
typedef struct {
    GD_EffectLinkKind kind;
    int from, to, relation, participant; /* Root-context effect target or call owner. */
} GD_EffectLink;
typedef struct {
    GD_Allocator allocator;
    const GD_ResourceGraph *source; /* Source, rules, calls and model outlive projection. */
    int node_count, link_count, work, work_limit, limited, source_incomplete;
    int unexamined_source_nodes, unexamined_relations, unknown_effects;
    int activations[GD_RESOURCE_GRAPH_NODES];
    GD_EffectNode nodes[GD_EFFECT_NODES];
    GD_EffectLink links[GD_EFFECT_LINKS];
} GD_EffectGraph;

/* Individual result concepts, not whole-outcome equivalence or Resource supply
   identity. Reuse concrete declarations/maps; never evaluate a second rule model.
   Conditions remain attached to original activations. No state-matching edges.
   NULL means allocation/argument failure; bounded partial results expose limits. */
GD_EffectGraph *gd_effect_graph_build(const GD_ResourceGraph *source, int work_limit);
void gd_effect_graph_destroy(GD_EffectGraph *graph);
const char *gd_effect_kind_name(GD_EffectKind kind);
/* Count direct logical effect links only. Optional activation mask indexes this
   graph's nodes, not Resource nodes. Repeated declarations/targets stay distinct.
   Output has node_count entries and must not alias graph or mask storage. */
void gd_effect_incoming(const GD_EffectGraph *graph, const unsigned char *activation_mask, int *restrict counts);
/* Stable opaque identity. Caller supplies bounded output storage; return 0
   rather than truncate on overflow. GD_EFFECT_ID_CAPACITY is a display default. */
int gd_effect_node_id(const GD_EffectGraph *graph, int node, char *text, size_t size);
/* Exact source placement checks supplement opaque IDs. -1 means absent,
   ambiguous or not representable within display identity capacity. */
int gd_effect_node_remap(const GD_EffectGraph *old, int node, const GD_EffectGraph *fresh);
#endif
