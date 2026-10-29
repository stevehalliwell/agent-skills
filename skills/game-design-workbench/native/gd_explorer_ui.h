#ifndef GD_EXPLORER_UI_H
#define GD_EXPLORER_UI_H
#include "gd_explorer.h"
#include "gd_graph_analysis_ui.h"
#include "gd_layout_focus.h"
#include "gd_matrix_grid.h"
#include "gd_diagnostics_ui.h"
#include "raylib.h"

typedef struct { int cell, node, interaction, linked, held; } GD_ExplorerSelection;
enum { GD_EXPLORER_FIT_GRID_READABLE=4 };
typedef struct { int record, has_rank; int64_t rank; char rank_text[64], filter[129]; } GD_ExplorerSkillEditor;
typedef struct {
    int record, link, held;
    bool roles[4], only_scope, paused, only_path;
    char filter[129];
    unsigned char path_nodes[GD_MAX_RECORDS], path_links[GD_MAX_EDGES];
} GD_ExplorerMdaDisplay;
typedef struct {
    GD_ExplorerView view;
    GD_Layout *layout;
    Camera2D camera,regular_camera;
    GD_LayoutFocus *distance_focus;
    bool distance_focus_enabled;
    uint64_t distance_focus_revision;
    Vector2 right_origin;
    int right_started;
    GD_ExplorerSelection selection;
    GD_MatrixGrid grid; /* Caller-owned presentation, never typed snapshot data. */
    GD_ExplorerSkillEditor editor;
    GD_ExplorerMdaDisplay mda;
    GD_Focus focus;
    GD_DiagnosticBrowser diagnostics;
    GD_PathControls paths;
    GD_ReachControls reach;
    GD_OverviewControls overview;
    GD_GraphLens lens;
    bool shared, branches, foundations, shapes, paused, overlay;
    int peer, foundation_peer, layout_choice, sort, horizontal, rank, witness;
    int prepared, fit, navigate, focus_root, focus_expand, inspected_record;
    unsigned revision;
    char filter[129], find_reference[260];
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
} GD_ExplorerContext;

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

int gd_explorer_resource_prepare(GD_ExplorerContext *context);
void gd_explorer_resource_draw(GD_ExplorerContext *context, const GD_ExplorerEvidence *evidence);
void gd_explorer_resource_details(GD_ExplorerContext *context);
void gd_explorer_resource_source(GD_ExplorerContext *context, int edge);
#endif
