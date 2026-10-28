#include "gd_graph_analysis_ui.h"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_NO_EXPORT
#include "cimgui.h"
#include <stdio.h>

int gd_graph_reachability_panel(const GD_Layout *layout, GD_GraphAnalysis *analysis, int selected,
                                GD_ReachControls *controls, int undirected) {
    const GD_GraphClosure *closure; const GD_GraphComparison *comparison;
    bool enabled=controls->enabled!=0; int kinds[64],count=0,i,j,choice=0,shown=0;
    const char *names[64];
    igSeparator(); igCheckbox("Typed multi-hop layer",&enabled); controls->enabled=enabled;
    if (!enabled) return 0;
    if (controls->relation>=0) kinds[count++]=controls->relation;
    for (i=0;i<analysis->edge_count && count<64;++i) {
        const GD_LayoutEdge *edge=&analysis->edges[i];
        if (!analysis->shown[edge->a] || !analysis->shown[edge->b] || (!edge->active && !analysis->include_inactive)) continue;
        for (j=0;j<count && kinds[j]!=edge->relation;++j) {}
        if (j==count) kinds[count++]=edge->relation;
    }
    if (count==64) igTextWrapped("Kind menu retains first 64 kinds plus current selection within that limit; other source links remain visible.");
    if (!count) { igTextWrapped("Unavailable: no shown relation kind to traverse."); return 0; }
    for (i=0;i<count;++i) { names[i]=gd_graph_relation_name(kinds[i]); if (kinds[i]==controls->relation) choice=i; }
    if (igCombo_Str_arr("Traverse one kind",&choice,names,count,-1) || controls->relation<0) controls->relation=kinds[choice];
    if (!undirected) { int reverse=controls->reverse; if (igCombo_Str("Traversal",&reverse,"Shown arrows\0Against shown arrows\0",-1)) controls->reverse=reverse; }
    closure=gd_graph_reachability_prepare(analysis,controls,undirected);
    igTextWrapped(undirected ? "Same-kind incidence connections, NOT flow, ancestry or requirements.":"Same-kind shown directed reachability, NOT causality, mastery or prerequisite necessity.");
    igTextWrapped("T: other shown origins reaching this identity. Purple: shared reached identity; blue: A-reached only; amber: B-reached only. Roots excluded. Missing/filtered declarations can change wider conclusions.");
    if (!closure->complete) igTextWrapped("PARTIAL: work limit. T counts/peer discovery are lower bounds; unfinished rows cannot prove absence.");
    comparison=gd_graph_closure_choose_into(analysis,&analysis->reachability,selected,&controls->peer);
    if (!comparison) { igTextWrapped("Select a shown node with completed local traversal."); return 0; }
    igText("%d reached identities | %d shared-target peers",closure->count[selected],closure->peer_count);
    if (igCollapsingHeader_TreeNodeFlags("Typed reachability peers",0)) {
        igBeginChild_Str("Typed peers",(ImVec2_c){0,85},0,0);
        for (i=0;i<closure->peer_count && shown<128;++i,++shown) {
            int node=closure->peers[i]; char label[192];
            snprintf(label,sizeof(label),"%.96s | %d shared targets##typed-%d",layout->nodes[node].label,closure->shared[node],node);
            if (igButton(label,(ImVec2_c){0,0})) controls->peer=node;
        }
        if (closure->peer_count>shown) igText("%d further peers not listed",closure->peer_count-shown);
        igEndChild();
    }
    comparison=gd_graph_closure_choose_into(analysis,&analysis->reachability,selected,&controls->peer);
    if (comparison->b>=0) igTextWrapped("T-A: %s | T-B: %s | shared %d / A-only %d / B-only %d",layout->nodes[selected].label,layout->nodes[comparison->b].label,comparison->shared,comparison->a_only,comparison->b_only);
    if (!igCollapsingHeader_TreeNodeFlags("Typed reachability evidence",0)) return 0;
    {
        int targets[64],target_count=0; const char *labels[64]; int target_choice=0;
        for (i=0;i<layout->node_count && target_count<64;++i) if (comparison->nodes[i]) {
            if (i==controls->target) target_choice=target_count;
            labels[target_count]=layout->nodes[i].label; targets[target_count++]=i;
        }
        if (target_count) {
            igCombo_Str_arr("Witness target",&target_choice,labels,target_count,-1); controls->target=targets[target_choice];
            for (j=0;j<2;++j) {
                int arcs[64],length=gd_graph_closure_path(analysis,closure,j,controls->target,arcs,64),retained=length<64 ? length:64;
                if (length<0) continue;
                igText("%c witness: %d links; last %d listed",j ? 'B':'A',length,retained);
                for (i=retained-1;i>=0;--i) {
                    const GD_LayoutEdge *edge=&analysis->edges[arcs[i]/2]; int reverse=arcs[i]%2;
                    igTextWrapped("%s %s %s | %s | source key %d%s",layout->nodes[reverse ? edge->b:edge->a].label,undirected ? "--":"->",layout->nodes[reverse ? edge->a:edge->b].label,gd_graph_relation_name(edge->relation),edge->key,!undirected && reverse ? " | against shown arrow":"");
                }
            }
        }
        if (comparison->shared+comparison->a_only+comparison->b_only>target_count) igTextWrapped("Target menu retains first 64 reached identities; graph flags and counts cover whole closure.");
    }
    return 1;
}
