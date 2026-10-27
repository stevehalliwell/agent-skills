#ifndef GD_H
#define GD_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/* Operation limits, not limits on the author's Markdown project. */
#define GD_MAX_DIRECTORY_ENTRIES 65536
#define GD_MAX_RECORDS 4096
#define GD_MAX_EDGES 32768
#define GD_MAX_PAIRS 32768
#define GD_MAX_COLLECTIONS 16
#define GD_MAX_ERRORS 128
#define GD_MAX_PATH 4096
#define GD_MAX_FILE (1024 * 1024)
#define GD_MAX_FRONTMATTER (64 * 1024)
#define GD_TEXT_CAPACITY (16 * 1024 * 1024)

typedef enum { GD_NOUN, GD_VERB, GD_CASE, GD_RELATED, GD_SKILL, GD_CHALLENGE,
               GD_MDA_MECHANIC, GD_MDA_SYSTEM, GD_MDA_DYNAMIC, GD_MDA_AESTHETIC, GD_CONTROL, GD_RULE, GD_AXIS } GD_Role;
typedef enum { GD_NOUNS, GD_VERB_REF, GD_CONTEXT, GD_MECHANICS,
               GD_CONTROLS, GD_RESOURCES, GD_VERBS, GD_PREREQUISITES,
               GD_INTRODUCES, GD_PRACTISES, GD_REQUIRES, GD_TESTS_MASTERY, GD_NEXT,
               GD_SYSTEMS, GD_DYNAMICS, GD_EXPERIMENTS, GD_AXES } GD_Field;
typedef struct { const char *alias; const char *directory; GD_Role role; } GD_Collection;
#define GD_MAX_ENUM_VALUES 64
typedef enum { GD_LITERAL_NULL, GD_LITERAL_NUMBER, GD_LITERAL_BOOLEAN, GD_LITERAL_STRING } GD_LiteralKind;
typedef struct {
    const char *type, *value, *expression, *unit, *minimum, *maximum;
    const char *allowed; /* allowed_count consecutive NUL-terminated strings in model text. */
    int allowed_count, lines[7];
    GD_LiteralKind kind;
} GD_ValueDefinition;
typedef GD_ValueDefinition GD_ControlDefinition; /* Existing control API name retained. */
typedef struct {
    const char *reference, *name, *id, *description, *status, *coverage;
    GD_ControlDefinition control;
    GD_ValueDefinition axis; /* No control value/expression or implicit actor initial value. */
    const char *source_path, *body;
    GD_Role role;
    int collection, load_index, has_rank, rank_line, entry, entry_line, body_line;
    int axis_closed, axis_closed_line, axes_declared, axes_line;
    int64_t rank;
} GD_Record;
typedef struct { int source, target; GD_Field field; const char *reference; int line; } GD_Edge;
typedef struct { int noun, verb, interaction; } GD_Pair;
typedef struct { int noun, verb, first_pair, pair_count, active; } GD_Cell;
typedef struct { int partners, cases, active_cases; } GD_Connectivity;
typedef struct { int anchor, first_member, member_count, edge_count; } GD_Component;
typedef enum { GD_VIEW_TABLE, GD_VIEW_GRAPH, GD_VIEW_COLUMNS, GD_VIEW_PREREQUISITES, GD_VIEW_RANK } GD_ViewLayout;
typedef enum { GD_EXPLORE_SKILLS, GD_EXPLORE_CHALLENGES, GD_EXPLORE_RELATIONSHIPS } GD_ExplorerMode;
typedef struct { int prerequisite, dependent; } GD_SkillLink;
typedef struct {
    int depth, layout_layer, cycle, incomplete, declared_prerequisites, prerequisite_count, dependent_count;
    int has_minimum, has_display, rank_conflict;
    int64_t minimum_rank, display_rank;
} GD_SkillState;
typedef struct { const char *code, *message, *field, *reference; int record, line; } GD_Finding;
typedef struct { const char *code, *message, *path; int line; } GD_Error;
typedef struct {
    void *context;
    void *(*allocate)(void *context, size_t size);
    void (*release)(void *context, void *memory);
} GD_Allocator;
typedef struct GD_Model GD_Model;

struct GD_Model {
    GD_Allocator allocator;
    GD_Collection collections[GD_MAX_COLLECTIONS];
    GD_Record records[GD_MAX_RECORDS];
    GD_Edge edges[GD_MAX_EDGES];
    GD_Pair pairs[GD_MAX_PAIRS];
    GD_Cell cells[GD_MAX_PAIRS];
    GD_Connectivity connectivity[GD_MAX_RECORDS];
    GD_Component components[GD_MAX_RECORDS];
    int component_of[GD_MAX_RECORDS], component_members[GD_MAX_RECORDS];
    int component_count;
    GD_SkillState skills[GD_MAX_RECORDS];
    GD_SkillLink skill_links[GD_MAX_EDGES];
    GD_Finding findings[GD_MAX_ERRORS];
    int skill_mode, route_mode, mda_mode, control_mode, rule_mode, skill_link_count, finding_count, findings_omitted, skill_incomplete;
    GD_Error errors[GD_MAX_ERRORS];
    int collection_count, record_count, edge_count, pair_count, cell_count;
    int error_count, errors_omitted, unplaced_cases;
    size_t text_used;
    uint64_t source_digest;
    char text[GD_TEXT_CAPACITY];
    char input[GD_MAX_FILE + 1];
};

GD_Model *gd_create(GD_Allocator allocator);
void gd_destroy(GD_Model *model);
/* Call once on a fresh model. Explicit folders only; immediate, non-hidden
   .md files. No schemas/registry. Collection strings must outlive the model. */
int gd_load(GD_Model *model, const GD_Collection *collections, int count);
int gd_sources_changed(GD_Model *model);
int gd_valid_utf8(const unsigned char *text, size_t size);
/* Reads bounded UTF-8 text into model->input; does not alter retained records. */
int gd_read_text(GD_Model *model, const char *path, size_t *size);
/* Analysis implementation; use recompute for an edited skill snapshot. */
void gd_analyze_skills(GD_Model *model);
void gd_recompute_skills(GD_Model *model);

#define GD_MAX_ROUTE_CHECKS 32768
#define GD_MAX_ROUTE_WITNESSES 128
#define GD_MAX_ROUTE_PATH 256
typedef enum { GD_SUPPORT_UNKNOWN, GD_SUPPORT_ALL, GD_SUPPORT_SOME, GD_SUPPORT_ABSENT } GD_Support;
typedef struct {
    int challenge, skill, demanded_by, demand_line;
    GD_Field demand_field;
    int direct_requires, direct_mastery, prerequisite, current_introduction;
    int supported_route, unsupported_route;
    GD_Support coverage;
} GD_RouteCheck;
typedef struct {
    int check, supported, full_length, length;
    int path[GD_MAX_ROUTE_PATH]; /* Complete path or explicit trailing suffix. */
} GD_RouteWitness;
typedef struct {
    GD_Allocator allocator;
    int entry_count, challenge_count, routes_complete, requirements_complete;
    int check_count, witness_count, witnesses_omitted;
    unsigned char reachable[GD_MAX_RECORDS];
    GD_RouteCheck checks[GD_MAX_ROUTE_CHECKS];
    GD_RouteWitness witnesses[GD_MAX_ROUTE_WITNESSES];
} GD_Routes;
/* Owned result borrows model identities. Model must outlive result and remain
   immutable afterward. Call once; appends bounded findings/errors during analysis.
   Never writes input records. */
GD_Routes *gd_analyze_routes(GD_Model *model);
void gd_routes_destroy(GD_Routes *routes);
const char *gd_support_name(GD_Support support);
int gd_write_routes_result(FILE *output, const GD_Model *model, const GD_Routes *routes,
                           int json, const char *fields);

#define GD_MAX_MDA_PATH 64
typedef enum { GD_MDA_BOTH, GD_MDA_FORWARD, GD_MDA_BACKWARD } GD_MdaDirection;
typedef enum { GD_MDA_RESOLVED, GD_MDA_UNLOADED, GD_MDA_MISSING, GD_MDA_WRONG_ROLE, GD_MDA_AMBIGUOUS, GD_MDA_UNAVAILABLE } GD_MdaResolution;
typedef struct { int edge; GD_MdaResolution resolution; } GD_MdaLink;
typedef struct { int basis_declared, basis_resolved, aesthetic; } GD_MdaNode;
typedef struct {
    GD_Allocator allocator;
    int node_count, link_count, graph_complete, root, selected_count, query_complete;
    GD_MdaDirection direction;
    GD_MdaNode nodes[GD_MAX_RECORDS];
    GD_MdaLink links[GD_MAX_EDGES];
    int outgoing[GD_MAX_RECORDS], incoming[GD_MAX_RECORDS];
    int next_out[GD_MAX_EDGES], next_in[GD_MAX_EDGES];
    int distance[GD_MAX_RECORDS], parent[GD_MAX_RECORDS], parent_link[GD_MAX_RECORDS];
    unsigned char selected[GD_MAX_RECORDS];
    int gaps[GD_MAX_RECORDS], gap_count, finding_count, findings_omitted;
    GD_Finding findings[GD_MAX_ERRORS];
} GD_Mda;
/* Analysis owns indices; model identities/text must remain immutable and alive.
   Query changes only transient scope/findings, never source or loaded records. */
int gd_mda_role(GD_Role role);
const char *gd_mda_role_name(GD_Role role);
const char *gd_mda_resolution_name(GD_MdaResolution resolution);
const char *gd_mda_direction_name(GD_MdaDirection direction);
GD_Mda *gd_analyze_mda(GD_Model *model);
void gd_mda_destroy(GD_Mda *mda);
int gd_mda_find(const GD_Model *model, const char *reference);
int gd_mda_query(const GD_Model *model, GD_Mda *mda, int root, GD_MdaDirection direction);
/* Path returns retained node count; full_length preserves original length.
   Links[k] joins nodes[k] to nodes[k+1]; a truncated result is a trailing suffix. */
int gd_mda_path(const GD_Mda *mda, int target, int *nodes, int *links, int capacity, int *full_length);
const char *gd_mda_basis(const GD_Model *model, const GD_Mda *mda, int record);
const char *gd_mda_trace(const GD_Mda *mda, int record);
int gd_write_mda_result(FILE *output, const GD_Model *model, const GD_Mda *mda, int json, const char *fields);

#define GD_MAX_UNIT_TERMS 16
#define GD_MAX_CONTROL_TOKENS 32768
#define GD_MAX_EXPRESSION_TOKENS 256
#define GD_MAX_EXPRESSION_STACK 128
typedef struct { char symbol[33]; int power; } GD_UnitTerm;
typedef struct { int count; GD_UnitTerm terms[GD_MAX_UNIT_TERMS]; } GD_Unit;
typedef struct {
    GD_LiteralKind kind;
    double number;
    const char *string;
    GD_Unit unit;
} GD_ControlValue;
typedef struct { const char *reference, *value; } GD_ControlOverride;
typedef struct {
    int operation, target, source, offset; /* Private bytecode; target>=0 only for references. */
    double number;
    const char *unit;
    int unit_length;
} GD_ControlToken;
typedef struct {
    GD_Allocator allocator;
    int selected_count, resolved_count, token_count, complete;
    unsigned char selected[GD_MAX_RECORDS], requested[GD_MAX_RECORDS], state[GD_MAX_RECORDS], overridden[GD_MAX_RECORDS];
    GD_ControlValue values[GD_MAX_RECORDS];
    int first[GD_MAX_RECORDS], count[GD_MAX_RECORDS];
    GD_ControlToken tokens[GD_MAX_CONTROL_TOKENS];
} GD_Controls;
/* Unit symbols are opaque. No implicit scale conversions. Errors return 0. */
int gd_control_unit(const char *text, size_t length, GD_Unit *unit);
int gd_control_number(const char *text, double *value, int integer);
int gd_control_find(const GD_Model *model, const char *reference);
int gd_control_find_name(const GD_Model *model, const char *name);
#define GD_MAX_CONTROL_FILTERS 64
typedef struct {
    const char *statuses[GD_MAX_CONTROL_FILTERS], *include[GD_MAX_CONTROL_FILTERS], *exclude[GD_MAX_CONTROL_FILTERS];
    int status_count, include_count, exclude_count, accepted_default, root; /* root=-1 means no root restriction. */
} GD_ControlFilter;
typedef struct { unsigned char roots[GD_MAX_RECORDS], allowed[GD_MAX_RECORDS]; } GD_ControlSelection;
/* Explicit names are bare stable keys or qualified references. Exclusions win.
   Status restrictions apply to dependencies; name filters only select output. */
int gd_select_controls(GD_Model *model, const GD_ControlFilter *filter, GD_ControlSelection *selection);
GD_Controls *gd_resolve_control_set(GD_Model *model, const GD_ControlSelection *selection,
                                   const GD_ControlOverride *overrides, int count);
/* Owned result borrows immutable model metadata and override enum strings.
   root=-1 selects all; otherwise resolve root and required dependencies only.
   Overrides are temporary, typed values in the control's declared unit. */
GD_Controls *gd_resolve_controls(GD_Model *model, int root,
                                const GD_ControlOverride *overrides, int count);
void gd_controls_destroy(GD_Controls *controls);
/* export_format: NULL for inspection; otherwise json/csv/ini, raw values only.
   A failed resolution never emits a partial game-facing export. */
int gd_write_controls_result(FILE *output, const GD_Model *model, const GD_Controls *controls,
                             int json, const char *fields, const char *export_format);

typedef struct {
    int record, line, changed, derived, conflict;
    GD_LiteralKind kind;
    const char *text, *type, *unit;
} GD_ControlInput;
typedef struct {
    GD_Allocator allocator;
    GD_Controls *current, *candidate;
    int count, changed_count, derived_count, complete;
    GD_ControlInput inputs[GD_MAX_RECORDS];
    size_t text_used;
    char text[GD_MAX_FILE+1];
} GD_ControlImport;
/* Read-only plan against a fresh current export. Inputs must fit the same file
   bound as source. Result owns candidate/current, borrows immutable model. */
GD_ControlImport *gd_plan_control_import(GD_Model *model, const GD_ControlSelection *selection,
                                        const char *input, size_t size, const char *format, const char *path);
void gd_control_import_destroy(GD_ControlImport *plan);
int gd_write_control_import(FILE *output, const GD_Model *model, const GD_ControlImport *plan, int json, const char *fields);

/* Rule declarations, explicit applicability, state predicate logic and effect bindings.
   Boolean evaluation consumes explicit predicate facts, not a gameplay runtime.
   No quantitative expression evaluation or invocation expansion.
   Result owns copied declaration text; borrows immutable model/provenance. */
#define GD_MAX_RULE_CASES 4096
#define GD_MAX_RULE_PARTICIPANTS 8192
#define GD_MAX_RULE_ASSIGNMENTS 8192
#define GD_MAX_RULE_STATE_EFFECTS 8192
#define GD_MAX_RULE_CONDITION_TOKENS 65536
#define GD_MAX_RULE_CONDITION_ITEMS 256
#define GD_MAX_RULE_CONDITION_DEPTH 64
#define GD_RULE_SYMBOL_WORK_LIMIT (16 * 1024 * 1024)
#define GD_RULE_TEXT_CAPACITY (4 * 1024 * 1024)
#define GD_MAX_RULE_LINE 8192
typedef enum { GD_RULE_ABSENT, GD_RULE_PARSED, GD_RULE_EMPTY, GD_RULE_INVALID } GD_RuleSourceState;
typedef struct { int record, first_case, case_count, line; GD_RuleSourceState state; } GD_RuleSource;
typedef struct { const char *name, *context; int source, line, context_line, first_participant, participant_count; } GD_RuleCase;
typedef enum { GD_RULE_UNSPECIFIED, GD_RULE_RESOLVED, GD_RULE_UNLOADED, GD_RULE_MISSING,
               GD_RULE_WRONG_ROLE, GD_RULE_AMBIGUOUS, GD_RULE_UNAVAILABLE,
               GD_RULE_UNDETERMINED, GD_RULE_WRONG_TYPE, GD_RULE_INVALID_DEFINITION } GD_RuleResolution;
typedef struct { GD_RuleResolution resolution; int axis_record; const char *state; } GD_RuleTarget;
typedef enum { GD_AXIS_UNSPECIFIED, GD_AXIS_ENUM, GD_AXIS_INTEGER, GD_AXIS_NUMBER } GD_AxisType;
typedef struct { GD_AxisType type; int valid, unit_known; } GD_RuleAxis;
typedef enum { GD_CONDITION_UNSPECIFIED, GD_CONDITION_PARSED, GD_CONDITION_INVALID,
               GD_CONDITION_UNSUPPORTED, GD_CONDITION_LIMITED } GD_RuleConditionState;
typedef enum { GD_RULE_OP_STATE, GD_RULE_OP_ANY, GD_RULE_OP_NOT, GD_RULE_OP_AND, GD_RULE_OP_OR } GD_RuleConditionOp;
typedef enum { GD_RULE_TRUTH_UNKNOWN, GD_RULE_TRUTH_FALSE, GD_RULE_TRUTH_TRUE } GD_RuleTruth;
typedef enum { GD_RULE_OTHER_UNSPECIFIED, GD_RULE_OTHER_ANY, GD_RULE_OTHER_INVALID } GD_RuleOtherAxes;
typedef struct {
    GD_RuleConditionState state;
    int first_token, token_count, bindings_complete, participant; /* Participant index in this snapshot. */
} GD_RuleCondition;
typedef struct {
    GD_RuleConditionOp operation;
    const char *text;
    size_t length;
    int offset; /* Byte offset within the raw condition, not Markdown column. */
    GD_RuleTarget target;
} GD_RuleConditionToken;
typedef struct { int participant, axis_record; const char *state; GD_RuleTruth truth; } GD_RulePredicateFact;
typedef struct {
    const char *name, *noun, *requires, *skip_if, *other_axes, *becomes, *preserve, *causes;
    int line, lines[8], first_assignment, assignment_count, noun_record;
    int first_state_effect, state_effect_count;
    GD_RuleCondition required_condition, skip_condition;
    GD_RuleOtherAxes other_axes_mode;
    GD_RuleResolution noun_resolution;
    unsigned fields;
} GD_RuleParticipant;
typedef struct { const char *axis, *expression; int line; GD_RuleTarget target; } GD_RuleAssignment;
typedef struct { const char *text; size_t length; int line; GD_RuleTarget target; } GD_RuleStateEffect;
typedef struct {
    GD_Allocator allocator;
    int source_count, case_count, participant_count, assignment_count;
    int axis_count, actor_count, axis_definitions_complete, bindings_complete, finding_count, findings_omitted;
    int state_effect_count, effect_bindings_complete, symbol_work;
    int condition_count, condition_token_count, condition_bindings_complete;
    GD_RuleAxis axes[GD_MAX_RECORDS]; /* Indexed by model record, not compact axis index. */
    int actor_first[GD_MAX_RECORDS], next_axis[GD_MAX_EDGES];
    unsigned char actor_axes_complete[GD_MAX_RECORDS];
    GD_RuleResolution axis_resolution[GD_MAX_EDGES];
    GD_Finding findings[GD_MAX_ERRORS];
    GD_RuleSource sources[GD_MAX_RECORDS];
    GD_RuleCase cases[GD_MAX_RULE_CASES];
    GD_RuleParticipant participants[GD_MAX_RULE_PARTICIPANTS];
    GD_RuleAssignment assignments[GD_MAX_RULE_ASSIGNMENTS];
    GD_RuleStateEffect state_effects[GD_MAX_RULE_STATE_EFFECTS];
    GD_RuleConditionToken condition_tokens[GD_MAX_RULE_CONDITION_TOKENS];
    size_t text_used;
    char text[GD_RULE_TEXT_CAPACITY];
} GD_Rules;
GD_Rules *gd_read_rules(GD_Model *model);
/* Internal phase: call once per fresh rule result; diagnostics append to model. */
void gd_resolve_rule_axes(GD_Model *model, GD_Rules *rules);
void gd_resolve_rule_logic(GD_Model *model, GD_Rules *rules);
/* Pure predicate evaluation. Missing, conflicting or unresolved facts remain unknown.
   Facts use snapshot participant/axis indices. This does not establish case applicability. */
GD_RuleTruth gd_rule_condition_value(const GD_Rules *rules, const GD_RuleCondition *condition,
                                    const GD_RulePredicateFact *facts, int count);
const char *gd_rule_condition_state_name(GD_RuleConditionState state);
const char *gd_rule_condition_op_name(GD_RuleConditionOp operation);
const char *gd_rule_resolution_name(GD_RuleResolution resolution);
void gd_rules_destroy(GD_Rules *rules);
int gd_write_rules_result(FILE *output, const GD_Model *model, const GD_Rules *rules, int json, const char *fields);

#define GD_MAX_DRAFT_EDITS 4096
typedef struct {
    int record, prerequisite_edit, has_rank, count;
    int64_t rank;
    const char *reference;
} GD_DraftEdit;
typedef struct {
    const GD_Model *baseline; /* Borrowed; must outlive draft and its model. */
    GD_Model *model;         /* Owned analysis, records/edges borrow baseline text. */
    int modified_count, history_count;
    unsigned char rank_changed[GD_MAX_RECORDS], prerequisites_changed[GD_MAX_RECORDS];
    GD_DraftEdit history[GD_MAX_DRAFT_EDITS];
    const char *scratch_a[GD_MAX_EDGES], *scratch_b[GD_MAX_EDGES];
    char message[256];
} GD_Draft;
GD_Draft *gd_draft_create(const GD_Model *baseline);
void gd_draft_destroy(GD_Draft *draft);
/* 1 changed, 0 no-op, -1 rejected without mutation. Rank: null or signed decimal. */
int gd_draft_rank(GD_Draft *draft, int record, const char *text);
int gd_draft_prerequisite(GD_Draft *draft, int record, const char *reference, int enabled);
int gd_draft_undo(GD_Draft *draft);
/* Current disk is separately loaded. Writer adds explicit identity errors there. */
int gd_write_draft_result(FILE *output, const GD_Draft *draft, GD_Model *current);
/* Finish: 0 complete, 2 partial with errors, -1 output failed (retain draft).
   Optional .json destination is created exclusively; never replaces an existing file. */
int gd_draft_finish(GD_Draft *draft, const char *output_path);
int gd_active_case(const GD_Record *record);
const char *gd_field_name(GD_Field field);
void gd_error(GD_Model *model, const char *code, const char *message,
              const char *path, int line);
void gd_json_string(FILE *output, const char *value);
void gd_json_string_n(FILE *output, const char *value, size_t size);
/* operation: 0 matrix, 1 connectivity, 2 skill-atoms.
   fields: comma-separated data keys, or NULL for command default. */
int gd_write_result(FILE *output, const GD_Model *model, int operation,
                    int json, const char *fields);

#ifdef GD_WITH_VIEW
int gd_view(GD_Model *model, int frames, const char *screenshot);
#ifdef GD_WITH_GRAPH
int gd_mda_view(GD_Model *model, GD_Mda *mda, int frames, const char *screenshot, int bands);
int gd_graph_view(GD_Model *model, int frames, const char *screenshot, GD_ViewLayout layout, int other_pairs, int horizontal, const char *output_path, GD_ExplorerMode explorer_mode, const char *focus_from);
#endif
int gd_save_screenshot(const char *path);
#endif
#endif
