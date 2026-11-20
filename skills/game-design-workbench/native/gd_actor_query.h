#ifndef GD_ACTOR_QUERY_H
#define GD_ACTOR_QUERY_H
#include "gd_effect_graph.h"
#define GD_ACTOR_VARIANTS 64
#define GD_ACTOR_WRITES GD_MAX_RULE_WRITES
#define GD_ACTOR_ACTIVATIONS GD_MAX_RULE_EXPANSIONS
#define GD_ACTOR_CHECKS GD_MAX_RULE_COMPARISONS
#define GD_ACTOR_WORK GD_RULE_ANALYSIS_WORK_LIMIT
#define GD_ACTOR_CASE_WORDS ((GD_MAX_RULE_CASES+63)/64)
#define GD_ACTOR_PARTNER_WORDS ((GD_MAX_RULE_PARTICIPANTS+63)/64)
#define GD_ACTOR_EFFECT_WORDS ((GD_ACTOR_WRITES+63)/64)
typedef struct {
    int case_index, participant, axis, value_count; /* participant -1 means declared A. */
    const char *values[GD_ACTOR_VARIANTS]; /* Empty list enumerates only declared enum values. */
    const GD_RuleScenario *fixed; /* No fact on selected participant/axis; no implicit override. */
} GD_ActorSelection;
typedef struct {
    const char *value;
    GD_RuleTruth applicability, context, requirements;
    GD_RuleExclusionCheck exclusion;
    int first_write, write_count, first_activation, activation_count, first_check, check_count;
    int constraints_complete, values_complete, conflicts, uncertain_pairs, checks_omitted, opaque_effects, cycles, work, limited;
    unsigned long long cases[GD_ACTOR_CASE_WORDS], partners[GD_ACTOR_PARTNER_WORDS], effects[GD_ACTOR_EFFECT_WORDS];
} GD_ActorVariant;
typedef struct {
    GD_EffectKind kind; int actor, axis, origin_case, declaration, activation;
    GD_RuleWriteKind declaration_kind; const char *state;
} GD_ActorEffect;
typedef struct { int shared, first_only, second_only; } GD_ActorSetComparison;
typedef struct { GD_ActorSetComparison cases, partners, effects; int complete; } GD_ActorComparison;
typedef struct {
    GD_Allocator allocator; const GD_RuleAnalysis *analysis;
    int case_index, participant, axis, requested, variant_count, write_count, activation_count, check_count, effect_count;
    int work, work_limit, limited, unexamined_variants, vocabulary_closed;
    GD_RuleScenario fixed;
    GD_ActorVariant variants[GD_ACTOR_VARIANTS];
    GD_RuleWrite writes[GD_ACTOR_WRITES]; int write_activations[GD_ACTOR_WRITES],write_effects[GD_ACTOR_WRITES];
    GD_RuleActivation activations[GD_ACTOR_ACTIVATIONS];
    GD_RuleWriteCheck checks[GD_ACTOR_CHECKS]; GD_ActorEffect effects[GD_ACTOR_WRITES];
    GD_RuleScenario inputs; GD_RuleEvaluation evaluation; GD_RuleTrace trace; /* Owned reusable scratch. */
} GD_ActorQuery;
/* All facts/values refer to one explicit root case. Other local participants
   stay distinct, even with same noun. Existing evaluator owns every truth,
   quantity and conflict decision. No scene or Cartesian profile generation.
   Borrow analysis/model/input strings until destruction. Invalid input or
   allocation failure returns NULL with model diagnostic; limits retain prefix. */
GD_ActorQuery *gd_actor_query_build(GD_Model *model, const GD_RuleAnalysis *analysis,
                                  const GD_ActorSelection *selection, int work_limit);
void gd_actor_query_destroy(GD_ActorQuery *query);
int gd_actor_trace_participant(const GD_RuleAnalysis *analysis, const GD_RuleActivation *activation, int participant);
GD_EffectKind gd_actor_write_family(const GD_ActorQuery *query, int variant, const GD_RuleWrite *write);
/* Exact retained concept association; -1 for invalid/unexamined write. */
int gd_actor_write_effect(const GD_ActorQuery *query, int variant, int write);
typedef enum { GD_ACTOR_SET_CASES, GD_ACTOR_SET_PARTNERS, GD_ACTOR_SET_EFFECTS } GD_ActorSet;
/* Membership mask: 1 first, 2 second, 3 shared, 0 neither; -1 invalid.
   Uses same retained not-excluded sets as gd_actor_compare. */
int gd_actor_compare_member(const GD_ActorQuery *query, int first, int second, GD_ActorSet set, int member);
int gd_actor_compare(const GD_ActorQuery *query, int first, int second, GD_ActorComparison *result);
#endif
