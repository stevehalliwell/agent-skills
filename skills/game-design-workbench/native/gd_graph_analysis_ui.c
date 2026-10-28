#include "gd_graph_analysis_ui.h"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_NO_EXPORT
#include "cimgui.h"
#include <stdio.h>

int gd_graph_analysis_panel(const GD_Layout *layout, GD_GraphAnalysis *analysis,
                            int selected, bool *enabled, GD_GraphLens *lens, bool *branches, int *peer) {
    enum { DISPLAY_MATCHES=128 };
    int i,pair,shown=0,evidence;
    igSeparator(); igCheckbox("Shared neighbors layer",enabled);
    if (!*enabled) return -1;
    { int choice=*lens;
      if (igCombo_Str_arr("Neighbor lens",&choice,gd_graph_lens_names,GD_GRAPH_LENS_COUNT,-1)) {
          *lens=(GD_GraphLens)choice;
          gd_graph_analyze(layout,analysis->shown,analysis->include_inactive,*lens,analysis);
          *peer=-1;
      } }
    igCheckbox("Compare shared / unique branches",branches);
    igTextWrapped(*branches ? "One-hop branches: purple shared; blue A only; amber B only; gray other role; white mixed links. S badges remain graph-wide. Not equivalence.":
        "S: nodes sharing a neighbor. Thick purple: example links. Not equivalence.");
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
    evidence=igCollapsingHeader_TreeNodeFlags("Shared-neighbor evidence",0);
    if (evidence) {
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
    igEndChild();
    }
    pair=gd_graph_match(analysis,selected,*peer);
    if (*branches && pair>=0) {
        const GD_GraphComparison *comparison=gd_graph_compare(analysis,selected,*peer);
        igTextWrapped("A: %s | B: %s",layout->nodes[selected].label,layout->nodes[*peer].label);
        igText("Shared: %d | A only: %d | B only: %d",comparison->shared,comparison->a_only,comparison->b_only);
        igText("Shared identity, other role: %d",comparison->other_role);
        if (!analysis->complete) igTextWrapped("Exact branches for this shown pair; global discovery remains partial.");
    }
    if (!evidence) return -1;
    if (pair>=0) {
        const GD_GraphMatch *match=&analysis->pairs[pair];
        const GD_GraphFeature *feature=&analysis->features[match->first_a];
        igTextWrapped("Example shared node: %s | %d other shown neighbors",layout->nodes[feature->neighbor].label,analysis->neighbors[feature->neighbor]);
        igTextWrapped("Both matches have a shown link incident to this same node. View-specific evidence follows; directions and roles may differ.");
    }
    return pair;
}
int gd_graph_paths_panel(const GD_Layout *layout, GD_GraphAnalysis *analysis, GD_PathControls *controls,
                         int selected, const unsigned char *declared, int undirected) {
    const GD_GraphPaths *paths; const GD_PathSelection *selection; bool enabled=controls->enabled!=0;
    int i,node;
    igSeparator(); igCheckbox("Mandatory paths layer",&enabled); controls->enabled=enabled;
    if (!enabled) return 0;
    if (declared) { bool authored=controls->declared!=0; igCheckbox("Use authored challenge entries",&authored); controls->declared=authored; }
    if (!controls->declared) {
        if (selected>=0 && selected<layout->node_count && igButton("Add selection as analysis start",(ImVec2_c){0,0})) {
            if (gd_graph_path_start(controls,layout->nodes[selected].key)>=0) controls->lost=0;
        }
        if (igButton("Clear analysis starts",(ImVec2_c){0,0})) controls->count=controls->lost=0;
        igText("Configured starts: %d (hidden starts excluded)",controls->count);
    }
    if (controls->lost) igTextWrapped("Reload removed %d unavailable analysis starts. Recheck start set.",controls->lost);
    paths=gd_graph_paths_prepare(analysis,controls,declared,undirected);
    igTextWrapped(undirected ? "Undirected incidence paths, NOT flow. D: other nodes for which every connection from starts includes this node.":
        "Directed shown paths. D: other nodes for which every path from starts includes this node. J: distinct incoming neighbors; join/loop candidate.");
    igTextUnformatted("D-: unreachable | D?: unavailable",NULL);
    igTextWrapped("White triangle: analysis start. Pink: one supporting path. !: mandatory ancestor of selection. These are not traffic or importance scores.");
    igText("Virtual start (analysis-only): %d shown starts",paths->root_count);
    { int shown=0;
      for (i=0;i<layout->node_count && shown<8;++i) if (paths->roots[i]) { igTextWrapped("Start: %s",layout->nodes[i].label); ++shown; }
      if (paths->root_count>shown) igText("%d further shown starts",paths->root_count-shown); }
    if (!paths->root_count) { igTextWrapped("Unavailable: choose shown analysis starts; node inspection does not change starts."); return 0; }
    if (!paths->complete) { igTextWrapped("UNAVAILABLE: path work limit reached. Intermediate dominators are not proofs."); return 0; }
    igText("Reachable: %d / %d shown nodes",paths->reachable_count,analysis->shown_nodes);
    igTextWrapped("Shown graph only; missing declarations or filtered paths can change wider conclusions.");
    selection=gd_graph_path_selection(analysis,selected);
    if (!selection) { igTextWrapped("Selection has no path from shown starts, or no node selected."); return 0; }
    igText("Selection: D%d | %d mandatory ancestors",paths->dominated[selected],selection->ancestor_count);
    if (!igCollapsingHeader_TreeNodeFlags("Mandatory-path evidence",0)) return 0;
    igTextWrapped("Mandatory ancestors, closest first (derived, not necessarily adjacent source links):");
    for (i=0,node=paths->dominator[selected];node!=analysis->node_count && i<64;node=paths->dominator[node],++i)
        igTextUnformatted(layout->nodes[node].label,NULL);
    if (selection->ancestor_count>i) igText("%d further mandatory ancestors",selection->ancestor_count-i);
    igTextWrapped("One support path: %d links, last %d retained. Support links are NOT all mandatory.",selection->length,selection->retained);
    for (i=selection->retained-1;i>=0;--i) {
        int arc=selection->arcs[i]; const GD_LayoutEdge *edge=&analysis->edges[arc/2];
        igTextWrapped("%s %s %s",layout->nodes[arc%2 ? edge->b:edge->a].label,undirected ? "--":"->",layout->nodes[arc%2 ? edge->a:edge->b].label);
    }
    return 1;
}
