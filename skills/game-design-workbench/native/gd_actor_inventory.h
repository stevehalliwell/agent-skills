#ifndef GD_ACTOR_INVENTORY_H
#define GD_ACTOR_INVENTORY_H
#include "gd_rule_analysis.h"

#define GD_ACTOR_INVENTORY_NODES 4096
#define GD_ACTOR_INVENTORY_LINKS 32768
#define GD_ACTOR_INVENTORY_WORK (64 * 1024 * 1024)

typedef enum { GD_ACTOR_CASE, GD_ACTOR_LOCAL_BINDING, GD_ACTOR_NOUN_DEFINITION,
               GD_ACTOR_AXIS_DEFINITION } GD_ActorInventoryKind;
typedef enum { GD_ACTOR_CONTAINS_BINDING, GD_ACTOR_DECLARED_NOUN, GD_ACTOR_DECLARED_AXIS,
               GD_ACTOR_DECLARED_CONTEXT_AXIS, GD_ACTOR_INVOKES, GD_ACTOR_MAPS_BINDING } GD_ActorInventoryLinkKind;
typedef struct {
    GD_ActorInventoryKind kind;
    int record, case_index, participant; /* Immutable native indices; -1 when inapplicable. */
} GD_ActorInventoryNode;
typedef struct {
    GD_ActorInventoryLinkKind kind;
    int from, to, declaration, binding;
    /* declaration: participant, model edge, or compiled call, selected by kind.
       binding: exact compiled substitution row for MAPS_BINDING; otherwise -1. */
} GD_ActorInventoryLink;
typedef struct {
    GD_Allocator allocator;
    const GD_RuleAnalysis *source; /* Borrows analysis, rules/model and their strings until destroy. */
    int node_count, link_count, work, work_limit, limited, source_incomplete;
    int unexamined_records, unexamined_sources, unexamined_cases, unexamined_participants, unexamined_edges, unexamined_calls;
    int unresolved_nouns, unresolved_axes, unresolved_calls, incomplete_sources, incomplete_call_owners;
    int definition_nodes[GD_MAX_RECORDS], case_nodes[GD_MAX_RULE_CASES];
    int participant_nodes[GD_MAX_RULE_PARTICIPANTS]; /* -1 when not retained. */
    GD_ActorInventoryNode nodes[GD_ACTOR_INVENTORY_NODES];
    GD_ActorInventoryLink links[GD_ACTOR_INVENTORY_LINKS];
} GD_ActorInventory;

/* Declaration inventory, never evaluated applicability or a global actor graph.
   Each case/local participant stays distinct even when noun definitions match.
   Include loaded noun/axis definitions (including unused definitions), parsed
   cases and bindings, declared noun axes/context axes, compiled calls and exact
   immediate substitutions. No state matching, inferred initial states or pool
   joins. Parallel declarations remain parallel. Bounded partial result owns
   retained prefix; source incompleteness and construction limits are separate.
   NULL: invalid argument or allocation failure. Failed build owns nothing. */
GD_ActorInventory *gd_actor_inventory_build(GD_Allocator allocator, const GD_RuleAnalysis *source, int work_limit);
void gd_actor_inventory_destroy(GD_ActorInventory *inventory);
const char *gd_actor_inventory_kind_name(GD_ActorInventoryKind kind);
const char *gd_actor_inventory_link_name(GD_ActorInventoryLinkKind kind);
/* Exact declaring line; 0 when unavailable. Noun field differs from binding header. */
int gd_actor_inventory_link_line(const GD_ActorInventory *inventory, int link);
/* Stable length-framed opaque identity: kind, qualified source reference, source
   ID, case name, local participant name. Not a source path or runtime actor ID.
   No snapshot offsets; overlong output fails with empty text, never truncates. */
int gd_actor_inventory_node_id(const GD_ActorInventory *inventory, int node, char *text, size_t size);
#endif
