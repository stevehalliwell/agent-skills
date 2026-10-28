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
int gd_graph_shapes_panel(const GD_Model *model, const GD_Layout *layout, GD_GraphAnalysis *analysis, int selected, bool *enabled) {
    const GD_GraphShapes *shapes; int i,repeated=0,shown=0,navigate=-1;
    igSeparator(); igCheckbox("Structural shape families layer",enabled);
    if (!*enabled) return -1;
    shapes=gd_graph_model_shapes(model,analysis);
    igTextWrapped("Two exact typed endpoint-neighborhood refinement rounds, starting from node roles. Shape ID / family size are transient per projection. Same family is NOT identity, isomorphism, equivalence or quality.");
    igTextWrapped("Shown graph only; missing declarations can change families. Rare shape is not a design defect. Gold outer outline marks selected family, not shared canonical ingredients.");
    if (!shapes->complete) { igTextWrapped("UNAVAILABLE: shape work limit; intermediate classes are not family proofs."); return -1; }
    for (i=0;i<shapes->family_count;++i) repeated+=shapes->size[i]>1;
    igText("%d shown families | %d repeated families",shapes->family_count,repeated);
    if (igCollapsingHeader_TreeNodeFlags("Discovered shape families",0)) {
        igBeginChild_Str("Shape families",(ImVec2_c){0,85},0,0);
        for (i=0;i<shapes->family_count && shown<128;++i) if (shapes->size[i]>1) {
            char label[192]; int node=shapes->representative[i];
            snprintf(label,sizeof(label),"shape %d | %d nodes | %.80s##shape-%d",i+1,shapes->size[i],layout->nodes[node].label,i);
            if (igButton(label,(ImVec2_c){0,0})) navigate=node;
            ++shown;
        }
        if (repeated>shown) igText("%d further repeated families not listed",repeated-shown);
        igEndChild();
    }
    if (selected<0 || selected>=layout->node_count || !analysis->shown[selected]) return navigate;
    igText("Selection: shape %d / %d nodes",shapes->colors[selected]+1,shapes->size[shapes->colors[selected]]);
    if (!igCollapsingHeader_TreeNodeFlags("Shape signature and members",0)) return navigate;
    igText("Node role tag: %d | typed incident features: %d",shapes->roles[selected],shapes->begin[selected+1]-shapes->begin[selected]);
    for (i=shapes->begin[selected];i<shapes->begin[selected+1] && i<shapes->begin[selected]+32;++i) {
        const GD_ShapeFeature *feature=&shapes->features[i];
        igTextWrapped("%s | endpoint %c | previous-round class %d",gd_graph_relation_name(feature->relation),feature->endpoint ? 'B':'A',feature->color);
    }
    if (shapes->begin[selected+1]-shapes->begin[selected]>32) igTextWrapped("Signature text retains first 32 features; equality compares full exact tuples.");
    shown=0;
    for (i=0;i<layout->node_count && shown<128;++i) if (analysis->shown[i] && shapes->colors[i]==shapes->colors[selected]) {
        igTextUnformatted(model->records[layout->nodes[i].key].reference,NULL); ++shown;
    }
    if (shapes->size[shapes->colors[selected]]>shown) igText("%d further members not listed",shapes->size[shapes->colors[selected]]-shown);
    return navigate;
}
int gd_graph_overview_panel(const GD_Model *model, const GD_Layout *layout, GD_GraphAnalysis *analysis,
                            GD_OverviewControls *controls, int undirected) {
    const GD_GraphOverview *overview; bool enabled=controls->enabled!=0; int i,side;
    igSeparator(); igCheckbox("Adjacency / incidence overview",&enabled); controls->enabled=enabled;
    if (!enabled) return 0;
    overview=gd_graph_overview(analysis,controls,undirected);
    igTextWrapped(undirected ? "Symmetric incidence bins, NOT flow. Mirrored cells do not double total input rows.":"Rows -> columns follow shown arrows, not inferred traffic or causality.");
    igTextWrapped("All shown identities, contiguous projection-index bins; not semantic clusters. Brightness uses log bands of projected link-row count, NOT strength, quantity or flow. Within-bin diagonal entries need not be self-links.");
    igText("%d identities / %d bins / %d input link rows / peak %d",overview->node_count,overview->bins,overview->link_count,overview->peak);
    igTextWrapped("Select inset cell or use bin controls. Gold graph marks are actual contributing endpoints/links, not every member of bin. Missing/filtered declarations stay outside scope.");
    if (!overview->bins) return 0;
    {
        int row=controls->row<0 ? 0:controls->row,column=controls->column<0 ? 0:controls->column,changed;
        changed=igSliderInt("Row bin",&row,0,overview->bins-1,"%d",0);
        changed|=igSliderInt("Column bin",&column,0,overview->bins-1,"%d",0);
        changed|=igButton("Inspect bin cell",(ImVec2_c){0,0});
        if (changed) { controls->row=row; controls->column=column; gd_graph_overview_select(analysis,row,column); }
    }
    if (controls->row<0 || controls->column<0) return 0;
    igText("Cell %d,%d: %d projected link rows",controls->row,controls->column,overview->selected_count);
    for (side=0;side<2;++side) {
        int bin=side ? controls->column:controls->row,begin=overview->start[bin],end=overview->start[bin+1];
        igText("%s bin: source ordinals %d..%d (%d identities)",side ? "Column":"Row",begin,end-1,end-begin);
        for (i=begin;i<end && i<begin+4;++i) {
            int node=overview->nodes[i],key=layout->nodes[node].key;
            igTextWrapped("%s",model && key>=0 && key<model->record_count ? model->records[key].reference:layout->nodes[node].label);
        }
        if (end-begin>4) igText("%d further bin identities",end-begin-4);
    }
    if (!igCollapsingHeader_TreeNodeFlags("Overview cell source evidence",0)) return 0;
    {
        int kinds[64],counts[64]={0},count=0,other=0,j;
        for (i=0;i<analysis->edge_count;++i) if (overview->selected_edges[i]) {
            int kind=analysis->edges[i].relation;
            for (j=0;j<count && kinds[j]!=kind;++j) {}
            if (j==count) { if (count==64) { ++other; continue; } kinds[count++]=kind; }
            ++counts[j];
        }
        for (i=0;i<count;++i) igTextWrapped("%s: %d link rows",gd_graph_relation_name(kinds[i]),counts[i]);
        if (other) igText("%d rows have unlisted kinds",other);
    }
    return 1;
}
