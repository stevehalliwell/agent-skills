#ifndef GD_GRAPH_ANALYSIS_UI_H
#define GD_GRAPH_ANALYSIS_UI_H
#include "gd_graph_analysis.h"
#include <stdbool.h>
/* Shared controls/details for cimgui views. Returns retained evidence-pair index,
   or -1. Caller renders view-specific provenance and graph marks. */
int gd_graph_analysis_panel(const GD_Layout *layout, GD_GraphAnalysis *analysis,
                            int selected, bool *enabled, GD_GraphLens *lens, int *peer);
#endif
