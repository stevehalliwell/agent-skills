#ifndef GD_GRAPH_ANALYSIS_UI_H
#define GD_GRAPH_ANALYSIS_UI_H
#include "gd_graph_analysis.h"
#include <stdbool.h>
/* Shared controls/details for cimgui views. Returns retained evidence-pair index,
   or -1. Caller renders view-specific provenance and graph marks. */
int gd_graph_analysis_panel(const GD_Layout *layout, GD_GraphAnalysis *analysis,
                            int selected, bool *enabled, GD_GraphLens *lens, bool *branches, int *peer);
/* Independent layer. Return 1 when selected witness provenance is expanded. */
int gd_graph_paths_panel(const GD_Layout *layout, GD_GraphAnalysis *analysis, GD_PathControls *controls,
                         int selected, const unsigned char *declared, int undirected);
/* Returns 1 when selected-cell source provenance is expanded. */
int gd_graph_overview_panel(const GD_Model *model, const GD_Layout *layout, GD_GraphAnalysis *analysis,
                            GD_OverviewControls *controls, int undirected);
/* Returns chosen family representative node for ordinary source navigation. */
int gd_graph_shapes_panel(const GD_Model *model, const GD_Layout *layout, GD_GraphAnalysis *analysis,
                          int selected, bool *enabled);
int gd_graph_reachability_panel(const GD_Layout *layout, GD_GraphAnalysis *analysis, int selected,
                                GD_ReachControls *controls, int undirected);
/* Independent AND layer; 1 requests source provenance for marked links. */
int gd_graph_foundations_panel(const GD_Model *model, const GD_Layout *layout, GD_GraphAnalysis *analysis,
                              int selected, int authored_direction, bool *enabled, int *peer);
#endif
