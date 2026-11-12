#ifndef GD_EXPLORER_UI_H
#define GD_EXPLORER_UI_H
#include "gd_explorer.h"
#include "gd_text_match.h"
#include "gd_graph_analysis_ui.h"
#include "gd_layout_focus.h"
#include "gd_matrix_grid.h"
#include "gd_resource_routes.h"
#include "gd_query_scene.h"
#include "gd_structure_cli.h"
#include "gd_similarity_projection.h"
#include "gd_named_query.h"
#include "gd_diagnostics_ui.h"
#include "raylib.h"

typedef struct { int cell, node, interaction, linked, held; } GD_ExplorerSelection;
typedef enum { GD_FOCUS_NONE, GD_FOCUS_FRAMED, GD_FOCUS_PARTIAL, GD_FOCUS_HIDDEN, GD_FOCUS_UNREPRESENTED, GD_FOCUS_UNPLACED } GD_ExplorerFocusStatus;
typedef struct {
    GD_ExplorerFocusStatus status;
    int selected,hidden,context;
    bool clipped,hidden_link;
    float zoom;
} GD_ExplorerFocusResult;
enum { GD_EXPLORER_FIT_GRID_READABLE=4, GD_EXPLORER_FIT_RESOURCE_WITNESS=5, GD_EXPLORER_FIT_QUERY_READABLE=6, GD_EXPLORER_FIT_SELECTION=7 };
typedef struct { int record, has_rank; int64_t rank; char rank_text[64], filter[129]; } GD_ExplorerSkillEditor;
typedef struct {
    int record, link, held;
    bool roles[4], only_scope, paused, only_path;
    char filter[129];
    unsigned char path_nodes[GD_MAX_RECORDS], path_links[GD_MAX_EDGES];
} GD_ExplorerMdaDisplay;
typedef struct {
    int root_case,participant,axis,noun,ghosts,primary,compare;
    bool coverage,accepted,weighted,include_excluded,literal_values,literal_assumptions;
    char values[GD_MAX_RULE_LINE+1],facts[GD_MAX_RULE_LINE+1],contexts[GD_MAX_RULE_LINE+1];
} GD_ExplorerQueryOptions;
typedef struct {
    GD_Allocator allocator; GD_ExplorerQueryOptions applied;
    GD_Query *query; GD_QueryScene *scene; GD_Layout *empty;
    GD_EffectQuery *selected;
    int coverage_page;
    GD_ActorComparison comparison; int comparison_first,comparison_second,comparison_ready;
    GD_RuleScenario fixed;
    char value_tokens[GD_MAX_RULE_LINE+1],fact_tokens[GD_MAX_RULE_LINE+1],context_tokens[GD_MAX_RULE_LINE+1];
} GD_ExplorerQueryState;
typedef struct {
    int kind,direction,lens,work_limit,actor_scope;
    bool include_inactive;
    char from[GD_MAX_PATH],to[GD_MAX_PATH],avoid[GD_MAX_PATH],scope_root[GD_MAX_PATH];
    char starts[GD_MAX_RULE_LINE+1],relations[GD_MAX_RULE_LINE+1],statuses[GD_MAX_RULE_LINE+1];
    char features[GD_MAX_RULE_LINE+1],entries[GD_MAX_RULE_LINE+1];
} GD_ExplorerStructureForm;
typedef struct {
    GD_Allocator allocator; GD_ExplorerStructureForm pending,applied;
    GD_StructureResult *result,*picker_context; unsigned revision,picker_revision; int has_applied,page,picker_scope;
    char start_tokens[GD_MAX_RULE_LINE+1],relation_tokens[GD_MAX_RULE_LINE+1],status_tokens[GD_MAX_RULE_LINE+1];
    char feature_tokens[GD_MAX_RULE_LINE+1],entry_tokens[GD_MAX_RULE_LINE+1],feature_ids[GD_SIMILARITY_FEATURES][128];
    const char *feature_names[GD_SIMILARITY_FEATURES],*entry_ids[GD_STRUCTURE_SELECTORS]; double feature_weights[GD_SIMILARITY_FEATURES];
    GD_SimilaritySelection similarity_selection;
    Camera2D similarity_camera; bool similarity_canvas,similarity_fit,similarity_camera_ready;
    int similarity_row,similarity_peer,similarity_page,comparison_first,comparison_second,comparison_available,comparison_work;
    double comparison_distance,comparison_components[GD_SIMILARITY_FEATURES];
    int similarity_member_pages[GD_SIMILARITY_FEATURES];
} GD_ExplorerStructureState;
typedef struct {
    int kind,work_limit; bool bindings;
    char case_name[GD_MAX_PATH],facts[GD_MAX_RULE_LINE+1],contexts[GD_MAX_RULE_LINE+1];
    char object[GD_MAX_PATH],participant[GD_MAX_PATH],axis[GD_MAX_PATH],statuses[GD_MAX_RULE_LINE+1];
} GD_ExplorerRuleForm;
typedef struct {
    GD_Allocator allocator; GD_ExplorerRuleForm pending,applied; GD_NamedQuery *result; GD_Query *bindings;
    char fact_tokens[GD_MAX_RULE_LINE+1],context_tokens[GD_MAX_RULE_LINE+1],status_tokens[GD_MAX_RULE_LINE+1];
    int page,row,peer_write,axis_page; unsigned char marked_cases[GD_MAX_RULE_CASES];
} GD_ExplorerRuleState;
typedef struct {
    const GD_ExplorerProject *project; int root,participant,axis,value,left,right,context_axis,context_value; bool context_false;
    double number,sample;
} GD_ExplorerAssumptionForm;
typedef struct {
    int from,to,work_limit; bool accepted;
} GD_ExplorerResourceForm;
/* Value snapshot only; pane/domain state remains authoritative. */
typedef struct {
    int layout,initial_arrangement,actor_overview,noun_row,noun_column,noun_case_page,sort,direction,lens,peer,foundation_peer,reach_relation,reach_peer,reach_target;
    int focus_root,mda_root,mda_direction,resource_witness,route_witness,similarity_row,similarity_peer;
    int overview_row,overview_column,coverage_page,structure_page,rules_page,rules_axis_page,similarity_page,bypass_candidate;
    bool paused,workspace_bounds,visual_filter,overlay,distance_focus,focus_auto_fit,shared,branches,foundations,shapes;
    bool paths_enabled,paths_declared,paths_compare,reach_enabled,reach_reverse,overview_enabled;
    bool only_scope,only_path,role0,role1,role2,role3,only_witness,similarity_canvas,known;
    char filter[129],mda_filter[129],list_search[129];
} GD_ExplorerPresentation;
typedef struct {
    GD_ExplorerView view;
    GD_Layout *layout;
    Camera2D camera,regular_camera;
    GD_ExplorerFocusResult focus_result; /* Last camera action, not analytical input. */
    bool focus_camera_override; /* Explicit framing wins next automatic distance-layout fit. */
    GD_LayoutFocus *distance_focus;
    bool distance_focus_enabled,focus_auto_fit;
    uint64_t distance_focus_revision;
    const GD_Layout *find_hold_layout;
    int find_hold_node; /* Camera-only Find must not implicitly refocus layout. */
    Vector2 right_origin;
    int right_started;
    GD_ExplorerSelection selection;
    GD_ExplorerQueryOptions query_options;
    GD_ExplorerQueryState *query_state;
    GD_ExplorerStructureState *structure_state;
    GD_ExplorerRuleState *rule_state;
    char rule_message[256];
    char picker_search[129];
    GD_ExplorerAssumptionForm assumption_form;
    char query_message[256];
    GD_ResourceRoutes *resource_routes;
    GD_ExplorerResourceForm resource_pending,resource_applied;
    int resource_witness;
    bool resource_only_witness;
    char resource_message[160];
    GD_MatrixGrid grid; /* Caller-owned presentation, never typed snapshot data. */
    int noun_case_page;
    GD_ExplorerSkillEditor editor;
    GD_ExplorerMdaDisplay mda;
    GD_Focus focus;
    GD_DiagnosticBrowser diagnostics;
    GD_PathControls paths;
    GD_ReachControls reach;
    GD_OverviewControls overview;
    GD_GraphLens lens;
    bool shared, branches, foundations, shapes, paused, workspace_bounds, visual_filter, overlay;
    int peer, foundation_peer, layout_choice, initial_arrangement, actor_overview, sort, horizontal, rank, witness;
    int prepared, fit, navigate, focus_root, focus_expand, inspected_record;
    unsigned revision;
    char filter[129], list_search[129], find_reference[260];
    unsigned char known[GD_MAX_RECORDS];
    unsigned char match_nodes[GD_MAX_RECORDS], match_edges[GD_MAX_EDGES];
    int visible_matches, hidden_matches;
} GD_ExplorerPane;
typedef struct {
    const GD_GraphPaths *paths;
    const GD_PathBypass *bypass;
    const GD_GraphClosure *foundations, *reachability;
    const GD_GraphComparison *foundation_pair, *reach_pair;
    const GD_GraphShapes *shapes;
    const GD_GraphOverview *overview;
} GD_ExplorerEvidence;
typedef struct {
    GD_ExplorerProject *project;
    GD_ExplorerPane *pane;
    GD_GraphAnalysis *analysis; /* One active-view scratch; never snapshot storage. */
    GD_SkillUnlocks *query;
    Rectangle viewport;
    uint64_t scope_revision;
    GD_Layout *display_layout; /* Borrowed render-only pose; never a solver target. */
    const struct GD_RenderPose *render_pose; /* Optional viewer-owned pose for camera actions only; IPC positions remain raw. */
    GD_ExplorerPane *panes; /* Optional shell-owned panes, never a snapshot. */
    int *active_view;
    struct GD_SelectionHistory *history;
    struct GD_ExplorerFind *finds; /* Optional shell-owned transient Find state. */
    struct GD_Sidebar *sidebar; /* Optional shell-owned shared fold preferences. */
} GD_ExplorerContext;
int gd_explorer_reset_all(GD_ExplorerContext *context);
int gd_explorer_history_available(const GD_ExplorerContext *context,int direction);
int gd_explorer_history_move(GD_ExplorerContext *context,int direction);
int gd_explorer_draft_history(GD_ExplorerContext *context,int redo);
static inline GD_Layout *gd_explorer_display_layout(const GD_ExplorerContext *context) {return context->display_layout ? context->display_layout:context->pane->layout;}

/* Camera only. Context viewport is drawable canvas; no allocation/evaluation. */
int gd_explorer_focus_available(const GD_ExplorerContext *context);
int gd_explorer_focus_selection(GD_ExplorerContext *context);
void gd_explorer_focus_camera(Camera2D *camera,Rectangle viewport,Rectangle selected,Rectangle context,Vector2 primary,GD_ExplorerFocusResult *result);
int gd_explorer_similarity_focus(GD_ExplorerContext *context);

int gd_explorer_graph_prepare(GD_ExplorerContext *context);
void gd_explorer_graph_input(GD_ExplorerContext *context);
int gd_explorer_graph_record(const GD_ExplorerContext *context);
int gd_explorer_graph_details(GD_ExplorerContext *context);
int gd_explorer_graph_commit(GD_ExplorerContext *context);
int gd_explorer_graph_select(GD_ExplorerContext *context, const GD_ExplorerSelection *choice);
void gd_explorer_graph_draw(GD_ExplorerContext *context, const GD_ExplorerEvidence *evidence);
void gd_explorer_graph_source(GD_ExplorerContext *context, int edge);
void gd_explorer_graph_unlocks(GD_ExplorerContext *context);

int gd_explorer_mda_prepare(GD_ExplorerContext *context);
void gd_explorer_mda_mark(GD_ExplorerContext *context);
int gd_explorer_mda_visible(GD_ExplorerContext *context, int record);
void gd_explorer_mda_input(GD_ExplorerContext *context);
void gd_explorer_mda_draw(GD_ExplorerContext *context, const GD_ExplorerEvidence *evidence);
void gd_explorer_mda_details(GD_ExplorerContext *context);
void gd_explorer_mda_source(GD_ExplorerContext *context, int edge);

int gd_explorer_query_prepare(GD_ExplorerContext *context);
int gd_explorer_query_run(GD_ExplorerContext *context);
int gd_explorer_query_display(GD_ExplorerContext *context);
int gd_explorer_query_arrange(GD_ExplorerContext *context,GD_LayoutMode mode,int neighborhood);
int gd_explorer_query_overview(GD_ExplorerContext *context,int mode);
int gd_explorer_query_select_result(GD_ExplorerContext *context, int target);
int gd_explorer_query_compare(GD_ExplorerContext *context);
void gd_explorer_query_destroy(GD_ExplorerQueryState *state);
int gd_explorer_query_remap(GD_ExplorerContext *context, const GD_ExplorerProject *old, const GD_ExplorerPane *prior);
void gd_explorer_query_controls(GD_ExplorerContext *context);
void gd_explorer_query_display_controls(GD_ExplorerContext *context);
const char *gd_explorer_query_node_label(const GD_ExplorerContext *context,int node);
void gd_explorer_query_draw(GD_ExplorerContext *context);
void gd_explorer_query_input(GD_ExplorerContext *context);
void gd_explorer_query_details(GD_ExplorerContext *context);
void gd_explorer_query_source(GD_ExplorerContext *context, int edge);
int gd_explorer_query_record(const GD_ExplorerPane *pane);

int gd_explorer_structure_prepare(GD_ExplorerContext *context);
int gd_explorer_structure_inputs(GD_ExplorerContext *context);
int gd_explorer_structure_run(GD_ExplorerContext *context);
int gd_explorer_structure_clear(GD_ExplorerContext *context);
int gd_explorer_structure_pending(const GD_ExplorerPane *pane);
int gd_explorer_structure_remap(GD_ExplorerContext *context, const GD_ExplorerProject *old, const GD_ExplorerPane *prior);
void gd_explorer_structure_destroy(GD_ExplorerStructureState *state);
void gd_explorer_structure_controls(GD_ExplorerContext *context);
void gd_explorer_structure_source_node(GD_ExplorerContext *context,int node);
int gd_explorer_similarity_active(const GD_ExplorerContext *context);
/* Exact current placed entry; select/frame only, no feature/query changes. */
int gd_explorer_similarity_select(GD_ExplorerContext *context, int row);
int gd_explorer_similarity_compare(GD_ExplorerContext *context);
void gd_explorer_similarity_controls(GD_ExplorerContext *context);
void gd_explorer_similarity_input(GD_ExplorerContext *context);
void gd_explorer_similarity_draw(GD_ExplorerContext *context);

void gd_explorer_rules_destroy(GD_ExplorerRuleState *state);
int gd_explorer_rules_inputs(GD_ExplorerContext *context);
int gd_explorer_rules_run(GD_ExplorerContext *context);
int gd_explorer_rules_remap(GD_ExplorerContext *context,const GD_ExplorerProject *old,const GD_ExplorerPane *prior);
void gd_explorer_rules_controls(GD_ExplorerContext *context);
void gd_explorer_rules_choose(GD_ExplorerRuleState *state,int row,int peer);
int gd_explorer_rules_marked(const GD_ExplorerContext *context,int case_index);

int gd_explorer_resource_prepare(GD_ExplorerContext *context);
void gd_explorer_resource_draw(GD_ExplorerContext *context, const GD_ExplorerEvidence *evidence);
void gd_explorer_resource_details(GD_ExplorerContext *context);
int gd_explorer_resource_run(GD_ExplorerContext *context);
void gd_explorer_resource_controls(GD_ExplorerContext *context);
void gd_explorer_resource_source(GD_ExplorerContext *context, int edge);
#endif
