#ifndef GD_GRAPH_ANALYSIS_UI_H
#define GD_GRAPH_ANALYSIS_UI_H
#include "gd_graph_analysis.h"
#include <stdbool.h>
/* Per-pane/session presentation only; never part of analytical controls/cache keys. */
typedef struct {
    int shared_peers,roots,candidates,ancestors,support,bypass,foundation_peers,foundation_members;
    int reach_peers,reach_targets,witness[2],families,signature,members,bin[2];
    int foundation_sources,reach_sources,overview_sources;
} GD_GraphEvidencePages;
_Static_assert(sizeof(GD_GraphEvidencePages)<=128,"Keep stack-copied pane paging state small");
/* Shared controls/details for cimgui views. Returns retained evidence-pair index,
   or -1. Caller renders view-specific provenance and graph marks. */
int gd_graph_analysis_panel(const GD_Layout *layout, GD_GraphAnalysis *analysis,
    int selected,bool *enabled,GD_GraphLens *lens,bool *branches,int *peer,GD_GraphEvidencePages *pages);
/* Independent layer. Return 1 when selected witness provenance is expanded. */
int gd_graph_paths_panel(const GD_Layout *layout,GD_GraphAnalysis *analysis,GD_PathControls *controls,
    int selected,const unsigned char *declared,int undirected,GD_GraphEvidencePages *pages);
/* Returns 1 when selected-cell source provenance is expanded.
   model may be NULL when layout keys identify derived nodes, not records. */
int gd_graph_overview_panel(const GD_Model *model,const GD_Layout *layout,GD_GraphAnalysis *analysis,
    GD_OverviewControls *controls,int undirected,GD_GraphEvidencePages *pages);
/* Non-NULL metric uses caller's typed link evidence, not generic relation names. */
int gd_graph_overview_panel_metric(const GD_Model *model,const GD_Layout *layout,GD_GraphAnalysis *analysis,
    GD_OverviewControls *controls,int undirected,const char *metric,GD_GraphEvidencePages *pages);
/* Returns chosen family representative node for ordinary source navigation. */
int gd_graph_shapes_panel(const GD_Model *model,const GD_Layout *layout,GD_GraphAnalysis *analysis,
    int selected,bool *enabled,GD_GraphEvidencePages *pages);
int gd_graph_reachability_panel(const GD_Layout *layout,GD_GraphAnalysis *analysis,int selected,
    GD_ReachControls *controls,int undirected,GD_GraphEvidencePages *pages);
/* Independent AND layer; 1 requests source provenance for marked links. */
int gd_graph_foundations_panel(const GD_Model *model,const GD_Layout *layout,GD_GraphAnalysis *analysis,
    int selected,int authored_direction,bool *enabled,int *peer,GD_GraphEvidencePages *pages);
#endif
