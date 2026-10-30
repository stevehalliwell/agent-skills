#ifndef GD_EFFECT_QUERY_H
#define GD_EFFECT_QUERY_H
#include "gd_effect_graph.h"
typedef struct {
    int target, root_case, noun, axis, kind; /* -1 means no corresponding filter. */
    const char *value; /* Optional exact canonical named state. */
    const unsigned char *activation_mask; /* Optional Effect node mask; every ancestor must pass. */
} GD_EffectSelection;
typedef struct {
    GD_Allocator allocator; const GD_EffectGraph *graph; GD_EffectSelection selection;
    int count, work, work_limit, limited, source_incomplete, unexamined_links, unresolved_targets;
    int excluded_scope, excluded_target;
    int links[GD_EFFECT_LINKS], incoming[GD_EFFECT_NODES], incoming_call[GD_EFFECT_NODES];
    unsigned char context[GD_EFFECT_NODES], highlighted[GD_EFFECT_LINKS];
} GD_EffectQuery;
/* Retained direct declarations plus their exact mapped-instantiation context.
   No state matching, exhaustive paths, executable sequence or effect flattening.
   Actor filter is a declared noun type, never a global runtime actor identity.
   Source graph, selection value and mask must outlive result. NULL on invalid args/allocation. */
GD_EffectQuery *gd_effect_query_build(const GD_EffectGraph *graph, GD_EffectSelection selection, int work_limit);
void gd_effect_query_destroy(GD_EffectQuery *query);
/* Source-order direct contributor row -> root-to-activation Effect node path.
   Return count; 0 for invalid input/capacity, never truncate. Caller owns output. */
int gd_effect_contributor_path(const GD_EffectQuery *query, int row, int *nodes, int capacity);
/* Exact opaque ID lookup: -1 missing, -2 ambiguous, -3 work/argument limit. */
int gd_effect_select(const GD_EffectGraph *graph, const char *id, int work_limit);
#endif
