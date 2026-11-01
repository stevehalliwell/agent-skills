#ifndef GD_QUERY_H
#define GD_QUERY_H
#include "gd_actor_query.h"
#include "gd_effect_query.h"
typedef enum { GD_QUERY_BINDINGS, GD_QUERY_ACTOR_STATES, GD_QUERY_CONTRIBUTORS, GD_QUERY_STATE_COVERAGE, GD_QUERY_COUNT } GD_QueryKind;
/* Case/noun/axis/effect_kind use -1 for no selector; zero work is valid.
   Status all must stand alone. Fixed inputs and strings remain borrowed. */
typedef struct {
    GD_QueryKind kind;
    int case_index, noun, axis, effect_kind, value_count, status_count, work_limit;
    const char *participant, *target_id, *values[GD_ACTOR_VARIANTS], *statuses[GD_MAX_CONTROL_FILTERS];
    const GD_RuleScenario *fixed;
} GD_QueryRequest;
typedef struct { int case_index, participant; } GD_QueryBinding;
typedef struct {
    const char *value;
    int declared, definite_bindings, undetermined_bindings, excluded_bindings, unfinished_bindings;
    int definite_cases, undetermined_cases, excluded_cases, unfinished_cases;
    GD_RuleTruth no_retained_candidate; /* Unknown when absence cannot be checked. */
} GD_StateCoverageValue;
/* Each row is an independent local-binding hypothesis, never simultaneous
   state assignment across cases/participants. Masks use finite-value indices. */
typedef struct {
    int value_count, evaluated_bindings, work, limited, source_incomplete;
    GD_StateCoverageValue values[GD_ACTOR_VARIANTS];
    uint64_t present[GD_MAX_RULE_PARTICIPANTS], definite[GD_MAX_RULE_PARTICIPANTS];
    uint64_t undetermined[GD_MAX_RULE_PARTICIPANTS], finished[GD_MAX_RULE_PARTICIPANTS];
    uint64_t case_definite[GD_MAX_RULE_CASES], case_undetermined[GD_MAX_RULE_CASES], case_unfinished[GD_MAX_RULE_CASES];
} GD_StateCoverage;
typedef struct {
    GD_Allocator allocator; const GD_Model *model; const GD_Rules *rules; const GD_RuleAnalysis *analysis;
    GD_QueryRequest request;
    int binding_count, work, limited, unexamined_bindings, excluded_roots;
    GD_QueryBinding bindings[GD_MAX_RULE_PARTICIPANTS];
    unsigned char cases[GD_MAX_RULE_CASES], activation_mask[GD_EFFECT_NODES];
    GD_StateCoverage *state_coverage;
    GD_ActorQuery *variants; GD_ResourceGraph *resource; GD_EffectGraph *effects; GD_EffectQuery *contributors;
} GD_Query;
const char *gd_query_name(GD_QueryKind kind);
const char *gd_query_projection(GD_QueryKind kind);
int gd_query_kind(const char *name); /* -1 unknown; no natural-language interpretation. */
int gd_query_record(const GD_Model *model, const char *reference, GD_Role role); /* Exact: -1 absent, -2 ambiguous. */
/* Model/rules/analysis/input strings/fixed scenario must outlive result.
   Owns bounded child queries only. Retains limit-marked partial construction;
   NULL denotes invalid arguments or allocation failure, not a zero-match result. */
GD_Query *gd_query_build(GD_Model *model, const GD_Rules *rules, const GD_RuleAnalysis *analysis, const GD_QueryRequest *request);
void gd_query_destroy(GD_Query *query);
int gd_query_fields(GD_QueryKind kind, const char *fields);
int gd_write_query_catalogue(FILE *output, int json);
int gd_write_query(FILE *output, const GD_Model *model, const GD_Query *query, int json, const char *fields);
/* Serialization only: no implicit all-variant comparison evaluation. */
int gd_write_query_retained(FILE *output,const GD_Model *model,const GD_Query *query,int json,const char *fields);
int gd_write_query_selection(FILE *output,const GD_Model *model,const GD_Query *query,GD_EffectQuery *selected,int json,const char *fields);
#endif
