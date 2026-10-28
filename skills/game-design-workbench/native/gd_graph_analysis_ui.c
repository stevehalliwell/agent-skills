#include "gd_graph_analysis_ui.h"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_NO_EXPORT
#include "cimgui.h"
#include <stdio.h>

int gd_graph_analysis_panel(const GD_Layout *layout, GD_GraphAnalysis *analysis,
                            int selected, bool *enabled, GD_GraphLens *lens, int *peer) {
    enum { DISPLAY_MATCHES=128 };
    int i,pair,shown=0;
    igSeparator(); igCheckbox("Shared neighbors layer",enabled);
    if (!*enabled) return -1;
    { int choice=*lens;
      if (igCombo_Str_arr("Neighbor lens",&choice,gd_graph_lens_names,GD_GRAPH_LENS_COUNT,-1)) {
          *lens=(GD_GraphLens)choice;
          gd_graph_analyze(layout,analysis->shown,analysis->include_inactive,*lens,analysis);
          *peer=-1;
      } }
    igTextWrapped("S: nodes sharing a neighbor. Thick purple: example links. Not equivalence.");
    if (*lens!=GD_GRAPH_ANY) igTextWrapped("Incident role = link kind + endpoint side, not inferred flow. Alternative links can satisfy both role lenses.");
    igText("Scope: %d shown nodes / %d shown links",analysis->shown_nodes,analysis->shown_edges);
    if (!analysis->complete) igTextWrapped("PARTIAL: join/pair limit reached. Counts are lower bounds; missing matches are not disproved.");
    if (selected<0 || selected>=layout->node_count || !analysis->shown[selected]) {
        igTextWrapped("Select a node to inspect automatically discovered matches."); return -1;
    }
    igText("Selection: %d other neighbors / %d incident link rows",analysis->neighbors[selected],analysis->incident[selected]);
    igText("%d%s nodes sharing a neighbor",analysis->matched[selected],analysis->complete ? "":"+ retained");
    if (gd_graph_match(analysis,selected,*peer)<0) {
        *peer=-1;
        for (i=0;i<analysis->pair_count;++i) {
            const GD_GraphMatch *match=&analysis->pairs[i];
            if (match->a==selected || match->b==selected) { *peer=match->a==selected ? match->b:match->a; break; }
        }
    }
    if (!igCollapsingHeader_TreeNodeFlags("Shared-neighbor evidence",0)) return -1;
    igBeginChild_Str("Shared neighbor matches",(ImVec2_c){0,85},0,0);
    for (i=0;i<analysis->pair_count;++i) {
        const GD_GraphMatch *match=&analysis->pairs[i]; int other; char label[192];
        if (match->a!=selected && match->b!=selected) continue;
        other=match->a==selected ? match->b:match->a;
        if (*peer<0) *peer=other;
        snprintf(label,sizeof(label),"%.96s | %d%s shared##shared-%d",layout->nodes[other].label,match->shared,analysis->complete ? "":"+",other);
        if (igButton(label,(ImVec2_c){0,0})) *peer=other;
        if (++shown==DISPLAY_MATCHES) break;
    }
    if (analysis->matched[selected]>shown) igText("%d more retained matches not listed",analysis->matched[selected]-shown);
    if (!shown) igTextUnformatted(analysis->complete ? "No shared-neighbor match in this projection.":"No retained match; result is partial.",NULL);
    igEndChild(); pair=gd_graph_match(analysis,selected,*peer);
    if (pair>=0) {
        const GD_GraphMatch *match=&analysis->pairs[pair];
        const GD_GraphFeature *feature=&analysis->features[match->first_a];
        igTextWrapped("Example shared node: %s | %d other shown neighbors",layout->nodes[feature->neighbor].label,analysis->neighbors[feature->neighbor]);
        igTextWrapped("Both matches have a shown link incident to this same node. View-specific evidence follows; directions and roles may differ.");
    }
    return pair;
}
