#ifndef GD_RULE_ANALYSIS_H
#define GD_RULE_ANALYSIS_H
#include "gd.h"

#define GD_MAX_RULE_CALLS 8192
#define GD_MAX_RULE_BINDINGS 32768
#define GD_MAX_RULE_CALL_BINDINGS 64
#define GD_MAX_RULE_EXPANSION_DEPTH 16
#define GD_MAX_RULE_EXPANSIONS 4096
#define GD_MAX_RULE_WRITES 8192
#define GD_MAX_RULE_CONTEXT_FACTS 64
#define GD_MAX_RULE_COMPARISONS 4096
#define GD_MAX_RULE_BRANCHES 128
#define GD_MAX_RULE_WITNESS_FACTS 16
#define GD_RULE_ANALYSIS_WORK_LIMIT (64 * 1024 * 1024)

typedef struct { int left, right, axis; const char *value; GD_RuleTruth truth; } GD_RuleContextFact;
typedef struct {
    const GD_RulePredicateFact *facts; int fact_count;
    const GD_RuleContextFact *contexts; int context_count;
} GD_RuleAssumptions;
typedef struct {
    int fact_count, context_count, work, limited;
    GD_RulePredicateFact facts[GD_MAX_RULE_CONDITION_ITEMS];
    GD_RuleContextFact contexts[GD_MAX_RULE_CONTEXT_FACTS];
} GD_RuleScenario;
/* Query input strings/model values borrowed until scenario evaluation finishes. */
int gd_rule_assume(GD_Model *model, const GD_Rules *rules, int case_index, const char *text, GD_RuleScenario *scenario);
int gd_rule_assume_context(GD_Model *model, const GD_Rules *rules, int case_index, const char *text, GD_RuleTruth truth, GD_RuleScenario *scenario);
typedef struct { int callee, caller; } GD_RuleBinding;
typedef struct {
    int caller_case, caller, target_source, first_case, case_count, first_binding, binding_count, line, next;
    const char *text; size_t length;
    GD_RuleResolution resolution;
} GD_RuleCall;
typedef enum { GD_WRITE_STATE, GD_WRITE_QUANTITY, GD_WRITE_PRESERVE } GD_RuleWriteKind;
typedef struct {
    int actor, axis, origin_case, origin_participant, line, quantity, conditional;
    GD_RuleWriteKind kind;
    const char *state;
    GD_RuleQuantityValue value;
    int path_count, path[GD_MAX_RULE_EXPANSION_DEPTH];
} GD_RuleWrite;
typedef struct { int first, second; GD_RuleTruth conflict; } GD_RuleWriteCheck;
typedef struct {
    int actor, preservation, origin_case, origin_participant, conditional, path_count;
    int path[GD_MAX_RULE_EXPANSION_DEPTH];
} GD_RuleDeferredPreservation;
typedef struct {
    int root_case, input_valid, work, limited, expansions, cycle_count;
    int constraints_complete, values_complete, write_count, conflict_count, uncertain_write_pairs, opaque_effects;
    int checked_write_pairs, checks_count, checks_omitted;
    GD_RuleTruth applicability, context, requirements;
    GD_RuleExclusionCheck exclusion;
    GD_RuleAssumptions assumptions; /* Borrowed validated input arrays/text until reporting finishes. */
    GD_RuleWrite writes[GD_MAX_RULE_WRITES];
    GD_RuleWriteCheck checks[GD_MAX_ERRORS];
    /* Private bounded scratch, reset by counts; not published actor state. */
    int actor_map[GD_MAX_RULE_PARTICIPANTS], deferred_count;
    GD_RuleDeferredPreservation deferred[GD_MAX_RULE_PRESERVATIONS];
    int seen[GD_MAX_RECORDS], generation;
} GD_RuleEvaluation;
typedef enum { GD_RULE_RELATION_UNDETERMINED, GD_RULE_RELATION_EQUIVALENT, GD_RULE_RELATION_REDUNDANT,
               GD_RULE_RELATION_COMPATIBLE, GD_RULE_RELATION_CONFLICT, GD_RULE_RELATION_DISJOINT } GD_RuleRelation;
typedef struct {
    int actor, axis; const char *value;
} GD_RuleWitnessFact;
typedef struct {
    int first_case, second_case, checked_branches, overlap_branches, matching_branches, compatible_branches, undetermined_branches, unexamined_branches;
    int vocabularies_closed, outcomes_complete, numeric_unexamined, shared_derivation, actor_scope_equal, context_scope_equal;
    const char *reason;
    GD_RuleRelation relation;
    int witness_count;
    GD_RuleWitnessFact witness[GD_MAX_RULE_WITNESS_FACTS];
    int conflict_first, conflict_second;
    GD_RuleWrite conflict_a, conflict_b; /* Witness expansion, not mutable scratch indices. */
} GD_RuleComparisonResult;
typedef struct {
    GD_Allocator allocator;
    const GD_Model *model; const GD_Rules *rules;
    int call_count, binding_count, compile_work, compile_limited;
    GD_RuleCall calls[GD_MAX_RULE_CALLS];
    GD_RuleBinding bindings[GD_MAX_RULE_BINDINGS];
    int call_first[GD_MAX_RULE_PARTICIPANTS];
    unsigned char calls_complete[GD_MAX_RULE_PARTICIPANTS];
    int verb_first[GD_MAX_RECORDS], verb_next[GD_MAX_EDGES];
    int requested_case, compared, comparison_count, pair_count, pairs_unexamined, comparison_work, comparison_limited;
    GD_RuleComparisonResult comparisons[GD_MAX_RULE_COMPARISONS];
    GD_RuleEvaluation scenario, first, second;
} GD_RuleAnalysis;

/* Compile explicit invocation mappings; borrowed model/rule snapshots remain immutable.
   Allocator owns analysis only. Caller destroys analysis before borrowed snapshots. */
GD_RuleAnalysis *gd_analyze_rules(GD_Model *model, const GD_Rules *rules);
void gd_rule_analysis_destroy(GD_RuleAnalysis *analysis);
void gd_rule_evaluate(const GD_RuleAnalysis *analysis, int case_index,
                      GD_RuleAssumptions assumptions, GD_RuleEvaluation *result);
void gd_rule_compare(GD_RuleAnalysis *analysis);
int gd_rule_case_find(const GD_Model *model, const GD_Rules *rules, const char *name);
const char *gd_rule_truth_name(GD_RuleTruth truth);
const char *gd_rule_relation_name(GD_RuleRelation relation);
void gd_write_rule_analysis(FILE *output, const GD_RuleAnalysis *analysis);
int gd_write_rule_report(FILE *output, const GD_Model *model, const GD_Rules *rules,
                         const GD_RuleAnalysis *analysis, int json, const char *fields);
#endif
