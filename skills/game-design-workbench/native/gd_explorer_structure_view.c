#include "gd_explorer_ui.h"
#include "gd_resource_routes.h"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_NO_EXPORT
#include "cimgui.h"
#include <ctype.h>
#include <string.h>

static void defaults(GD_ExplorerStructureForm *form) { memset(form,0,sizeof(*form)); form->work_limit=GD_GRAPH_ANALYSIS_WORK; }
static int equal_form(const GD_ExplorerStructureForm *a, const GD_ExplorerStructureForm *b) {
    return a->kind==b->kind && a->direction==b->direction && a->lens==b->lens && a->work_limit==b->work_limit && a->include_inactive==b->include_inactive &&
        !strcmp(a->from,b->from) && !strcmp(a->to,b->to) && !strcmp(a->avoid,b->avoid) && !strcmp(a->scope_root,b->scope_root) && !strcmp(a->starts,b->starts) && !strcmp(a->relations,b->relations) && !strcmp(a->statuses,b->statuses);
}
int gd_explorer_structure_pending(const GD_ExplorerPane *pane) {
    GD_ExplorerStructureForm empty; const GD_ExplorerStructureState *state=pane->structure_state;
    if (!state) return 0;
    defaults(&empty); return !equal_form(&state->pending,state->has_applied ? &state->applied:&empty);
}
void gd_explorer_structure_destroy(GD_ExplorerStructureState *state) {
    if (!state) return;
    gd_structure_result_destroy(state->result); state->allocator.release(state->allocator.context,state);
}
static GD_ExplorerStructureState *empty_state(GD_Allocator allocator) {
    GD_ExplorerStructureState *state=allocator.allocate(allocator.context,sizeof(*state));
    if (state) { memset(state,0,sizeof(*state)); state->allocator=allocator; defaults(&state->pending); defaults(&state->applied); }
    return state;
}
/* Lists are UI parameter boundaries only: one literal per line, no expression
   language. Tokens live beside the result, not in a temporary pane copy. */
static int lines(char *text, const char **values, int capacity) {
    int count=0; char *p=text;
    while (*p) {
        char *end=strchr(p,'\n'),*tail; if (end) *end=0;
        while (*p && isspace((unsigned char)*p)) ++p;
        tail=p+strlen(p); while (tail>p && isspace((unsigned char)tail[-1])) *--tail=0;
        if (*p) { if (count==capacity) return -1; values[count++]=p; }
        if (!end) break; p=end+1;
    }
    return count;
}
static GD_ExplorerStructureState *build_state(const GD_ExplorerProject *project, GD_ExplorerView view, const GD_ExplorerStructureForm *form, char *message, size_t size) {
    GD_ExplorerStructureState *state=empty_state(project->allocator); GD_Model *model=gd_explorer_model(project,view); GD_StructureOptions options={0}; int errors=model->error_count,omitted=model->errors_omitted;
    if (!state) { snprintf(message,size,"Cannot allocate structural question inputs; last result retained"); return NULL; }
    state->pending=state->applied=*form;
    memcpy(state->start_tokens,form->starts,sizeof(state->start_tokens)); memcpy(state->relation_tokens,form->relations,sizeof(state->relation_tokens)); memcpy(state->status_tokens,form->statuses,sizeof(state->status_tokens));
    options.question=gd_structure_name((GD_StructureKind)form->kind); options.projection=gd_projection_name(view);
    options.from=*form->from ? state->applied.from:NULL; options.to=*form->to ? state->applied.to:NULL; options.avoid=*form->avoid ? state->applied.avoid:NULL; options.scope_root=*form->scope_root ? state->applied.scope_root:NULL;
    options.root_count=lines(state->start_tokens,options.roots,GD_STRUCTURE_SELECTORS); options.relation_count=lines(state->relation_tokens,options.relations,GD_STRUCTURE_KINDS); options.status_count=lines(state->status_tokens,options.statuses,GD_STRUCTURE_SELECTORS);
    options.direction=(GD_StructureDirection)form->direction; options.lens=(GD_GraphLens)form->lens; options.include_inactive=form->include_inactive; options.work_limit=form->work_limit;
    if (options.root_count<0 || options.relation_count<0 || options.status_count<0) gd_error(model,"query.parameters","Repeated question inputs exceed native count bound",NULL,0);
    else state->result=gd_structure_result_build(model,project,&options);
    if (!state->result) snprintf(message,size,"%s: %s",model->error_count>errors ? model->errors[model->error_count-1].code:"query.memory",model->error_count>errors ? model->errors[model->error_count-1].message:"Cannot build structural result; last result retained");
    model->error_count=errors; model->errors_omitted=omitted;
    if (!state->result) { gd_explorer_structure_destroy(state); return NULL; }
    state->has_applied=1; state->revision=project->revision; return state;
}
int gd_explorer_structure_prepare(GD_ExplorerContext *context) {
    GD_ExplorerStructureState *state=context->pane->structure_state;
    if (state && state->result && gd_explorer_family(context->pane->view)==GD_EXPLORER_SKILL_DATA && state->revision!=context->project->revision) {
        gd_structure_result_destroy(state->result); state->result=NULL;
        snprintf(context->pane->query_message,sizeof(context->pane->query_message),"Draft projection changed; run question again. Inputs retained.");
    }
    return 1;
}
int gd_explorer_structure_run(GD_ExplorerContext *context) {
    GD_ExplorerPane *pane=context->pane; GD_ExplorerStructureState *candidate; GD_ExplorerStructureForm form;
    if (!gd_projection_name(pane->view)) return 0;
    if (pane->structure_state) form=pane->structure_state->pending; else defaults(&form);
    candidate=build_state(context->project,pane->view,&form,pane->query_message,sizeof(pane->query_message));
    if (!candidate) return 0;
    gd_explorer_structure_destroy(pane->structure_state); pane->structure_state=candidate; *pane->query_message=0; return 1;
}
static int same_record(const GD_Record *a, const GD_Record *b) { return a->role==b->role && !strcmp(a->reference,b->reference) && ((!a->id && !b->id) || (a->id && b->id && !strcmp(a->id,b->id))); }
static int same_node(const GD_StructureResult *old, int a, const GD_StructureResult *fresh, int b) {
    int old_key,new_key;
    if (a<0 || b<0) return a==b;
    old_key=old->layout->nodes[a].key; new_key=fresh->layout->nodes[b].key;
    if (old->view==GD_EXPLORER_RESOURCES) return gd_resource_node_remap(old->project->resources,old_key,fresh->project->resources)==new_key;
    return same_record(&gd_explorer_model(old->project,old->view)->records[old_key],&gd_explorer_model(fresh->project,fresh->view)->records[new_key]);
}
static int same_selection(const GD_StructureResult *old, const GD_StructureResult *fresh) {
    const GD_StructureRequest *a=&old->query->request,*b=&fresh->query->request; int i;
    if (!same_node(old,a->source,fresh,b->source) || !same_node(old,a->target,fresh,b->target) || !same_node(old,a->avoid,fresh,b->avoid) || a->root_count!=b->root_count) return 0;
    for (i=0;i<a->root_count;++i) if (!same_node(old,a->roots[i],fresh,b->roots[i])) return 0;
    if (old->options.scope_root) {
        GD_Model *before=gd_explorer_model(old->project,old->view),*after=gd_explorer_model(fresh->project,fresh->view); int x=gd_explorer_record(before,old->options.scope_root),y=gd_explorer_record(after,fresh->options.scope_root);
        if (x<0 || y<0 || !same_record(&before->records[x],&after->records[y])) return 0;
    }
    return 1;
}
int gd_explorer_structure_remap(GD_ExplorerContext *context, const GD_ExplorerProject *old, const GD_ExplorerPane *prior) {
    const GD_ExplorerStructureState *state=prior->structure_state; GD_ExplorerStructureState *candidate,*temporary=NULL; const GD_StructureResult *before;
    if (!state) return 1;
    if (gd_explorer_structure_pending(prior)) { snprintf(context->pane->query_message,sizeof(context->pane->query_message),"Refresh blocked: pending structural question inputs. Run or clear first."); return 0; }
    if (!state->has_applied) { candidate=empty_state(context->project->allocator); if (!candidate) return 0; context->pane->structure_state=candidate; return 1; }
    candidate=build_state(context->project,context->pane->view,&state->applied,context->pane->query_message,sizeof(context->pane->query_message)); if (!candidate) return 0;
    before=state->result;
    /* Inactive skill panes can retain a result whose borrowed draft strings
       were replaced. Rebuild from current old project before reading identity. */
    if (!before || (gd_explorer_family(prior->view)==GD_EXPLORER_SKILL_DATA && state->revision!=old->revision)) {
        temporary=build_state(old,prior->view,&state->applied,context->pane->query_message,sizeof(context->pane->query_message));
        before=temporary ? temporary->result:NULL;
    }
    if (!before || !same_selection(before,candidate->result)) {
        gd_explorer_structure_destroy(temporary); gd_explorer_structure_destroy(candidate);
        snprintf(context->pane->query_message,sizeof(context->pane->query_message),"Refresh blocked: structural selector identity changed or unavailable. Clear question first."); return 0;
    }
    gd_explorer_structure_destroy(temporary); candidate->page=state->page; context->pane->structure_state=candidate; return 1;
}
static void navigate(GD_ExplorerContext *context, int record, int resource_node) {
    GD_ExplorerPane *pane=context->pane; if (record<0) return;
    if (pane->view==GD_EXPLORER_MDA) { pane->mda.record=record; pane->mda.link=-1; }
    else if (pane->view==GD_EXPLORER_RESOURCES) { pane->selection.node=resource_node; pane->selection.cell=-1; pane->selection.linked=resource_node<0 ? record:-1; }
    else pane->navigate=record;
}
static int node_id(const GD_StructureResult *result, int index, char *text, size_t capacity) {
    int key=result->layout->nodes[index].key;
    if (result->view==GD_EXPLORER_RESOURCES) return gd_resource_node_id(result->project->resources,key,-1,text,capacity);
    { int count=snprintf(text,capacity,"%s",gd_explorer_model(result->project,result->view)->records[key].reference); return count>=0 && count<(int)capacity; }
}
static void node_row(GD_ExplorerContext *context, int index) {
    GD_ExplorerStructureState *state=context->pane->structure_state; const GD_StructureResult *result=state->result; const GD_StructureQuery *query=result->query;
    const GD_StructureNode *counts=&query->nodes[index]; int key=result->layout->nodes[index].key,record=gd_projection_record(result->project,result->view,key); char identity[GD_MAX_PATH];
    igPushID_Int(index);
    if (igSmallButton(TextFormat("#%d %s",index,result->layout->nodes[index].label))) navigate(context,record,result->view==GD_EXPLORER_RESOURCES ? key:-1);
    if (query->request.incidence) igText("%d incident rows | %d distinct partners | %d kinds",counts->incident,counts->partners,counts->kinds);
    else { igText("in %d | out %d | %d partners | %d kinds",counts->incoming,counts->outgoing,counts->partners,counts->kinds); igText("%d incident source rows",counts->incident); }
    if (query->request.source>=0 || query->request.target>=0) igText("Retained evidence member: %s",query->marked_nodes[index] ? "yes":"no");
    if (query->request.kind==GD_STRUCTURE_RELATED && query->analysis.comparison.valid) { unsigned flags=query->analysis.comparison.nodes[index]; igText("Partner membership: %s",flags==GD_GRAPH_SHARED ? "shared":flags==GD_GRAPH_A_ONLY ? "first only":flags==GD_GRAPH_B_ONLY ? "second only":flags==GD_GRAPH_OTHER_ROLE ? "other role":"neither"); }
    if (query->request.kind==GD_STRUCTURE_REACHABILITY && query->roots[index]) igText("%d reached targets | row %s | cycle %s",query->analysis.reachability.count[index],query->analysis.reachability.finished[index] ? "finished":"unfinished",!query->analysis.reachability.finished[index] ? "unknown":query->analysis.reachability.cycle[index] ? "yes":"no");
    if (query->request.kind==GD_STRUCTURE_DOMINATORS || query->request.kind==GD_STRUCTURE_BYPASS) {
        if (query->analysis.paths.complete) igText("reachable %s | dominates %d other nodes | mandatory for target %s",query->analysis.paths.reached[index] ? "yes":"no",query->analysis.paths.dominated[index],query->request.target<0 || query->analysis.paths.selection.node!=query->request.target ? "unavailable":query->analysis.paths.selection.ancestors[index] ? "yes":"no");
        else igTextUnformatted("Dominator/start evidence unfinished",NULL);
    }
    if (query->request.kind==GD_STRUCTURE_SHAPES && query->analysis.shapes.complete) igText("Transient family %d | %d members",query->analysis.shapes.colors[index],query->analysis.shapes.size[query->analysis.shapes.colors[index]]);
    if (node_id(result,index,identity,sizeof(identity))) {
        if (igSmallButton("Use as from")) snprintf(state->pending.from,sizeof(state->pending.from),"%s",identity);
        igSameLine(0,-1); if (igSmallButton("Use as to")) snprintf(state->pending.to,sizeof(state->pending.to),"%s",identity);
        if (state->pending.kind==GD_STRUCTURE_BYPASS) { igSameLine(0,-1); if (igSmallButton("Use as avoid")) snprintf(state->pending.avoid,sizeof(state->pending.avoid),"%s",identity); }
    } else igTextWrapped("Native selector identity unavailable; identity controls omitted.");
    if (record>=0 && igTreeNode_Str("Source identity")) { const GD_Record *source=&gd_explorer_model(result->project,result->view)->records[record]; igTextWrapped("%s | %s | %s",source->reference,source->id ? source->id:"ID unspecified",source->status ? source->status:"status unspecified"); igTextWrapped("%s",source->source_path); igTreePop(); }
    igPopID();
}
static void link_row(GD_ExplorerContext *context, int index, int reverse) {
    const GD_StructureResult *result=context->pane->structure_state->result; const GD_LayoutEdge *edge=&result->layout->edges[index]; int source=gd_projection_edge_record(result->project,result->view,edge->key);
    igPushID_Int(index); igTextWrapped("link #%d: %s%s%s | %s%s%s",index,result->layout->nodes[edge->a].label,result->query->request.incidence ? " -- ":" -> ",result->layout->nodes[edge->b].label,gd_graph_relation_name(edge->relation),result->query->request.incidence ? " | undirected incidence":"",reverse ? " | reverse to stored endpoints":"");
    if (source>=0 && igSmallButton("Inspect declaring source")) navigate(context,source,-1);
    if (result->view==GD_EXPLORER_MATRIX && igTreeNode_Str("Loaded matrix source cases")) {
        const GD_Model *model=gd_explorer_model(result->project,result->view); const GD_Cell *cell=&model->cells[edge->key]; int i;
        igText("%d loaded cases: source context, not filtered contributor count",cell->pair_count);
        for (i=0;i<cell->pair_count && i<64;++i) { int record=model->pairs[cell->first_pair+i].interaction; igPushID_Int(i); if (igSmallButton(model->records[record].reference)) navigate(context,record,-1); igPopID(); }
        if (cell->pair_count>64) igText("%d further cases in native result",cell->pair_count-64); igTreePop();
    }
    igPopID();
}
static void witness(GD_ExplorerContext *context, const char *label, const int *arcs, int retained, int length) {
    const GD_StructureQuery *query=context->pane->structure_state->result->query; int i;
    igPushID_Str(label); igText("%s: length %d | %d trailing arcs retained",label,length,retained);
    for (i=retained-1;i>=0;--i) link_row(context,arcs[i]/2,(arcs[i]%2) ^ (query->request.direction==GD_STRUCTURE_BACKWARD));
    igPopID();
}
static void result_panel(GD_ExplorerContext *context) {
    GD_ExplorerStructureState *state=context->pane->structure_state; const GD_StructureResult *result=state->result; const GD_StructureQuery *query=result->query; const GD_GraphAnalysis *analysis=&query->analysis; int i,count=0,indices[GD_MAX_RECORDS];
    igSeparator(); igText("Applied: %s [%s]",gd_structure_name(query->request.kind),gd_projection_name(result->view));
    igTextWrapped("Applied from: %s",result->options.from ? result->options.from:"not selected (projection-wide context)");
    if (result->options.to) igTextWrapped("Applied to: %s",result->options.to);
    if (result->options.avoid) igTextWrapped("Applied avoid: %s",result->options.avoid);
    if (igTreeNode_Str("Applied scope parameters")) {
        igText("Direction: %s | include inactive: %s",query->request.incidence ? "undirected incidence":query->request.direction==GD_STRUCTURE_FORWARD ? "forward":query->request.direction==GD_STRUCTURE_BACKWARD ? "backward":"both",query->request.include_inactive ? "yes":"no");
        igText("Partner lens: %s",query->request.lens==GD_GRAPH_ANY ? "any":query->request.lens==GD_GRAPH_SAME_ROLE ? "same-role":"changed-role");
        if (result->options.scope_root) igTextWrapped("Scope root: %s",result->options.scope_root);
        for (i=0;i<result->options.root_count;++i) igTextWrapped("Start: %s",result->options.roots[i]);
        for (i=0;i<result->options.relation_count;++i) igTextWrapped("Relation: %s",result->options.relations[i]);
        for (i=0;i<result->options.status_count;++i) igTextWrapped("Status: %s",result->options.statuses[i]);
        igTreePop();
    }
    igText("%d nodes | %d rows | %d isolates",query->shown_nodes,query->shown_edges,query->isolated);
    igText("Search %s | available %s | source %s",query->complete ? "complete":"unfinished",query->available ? "yes":"no",result->source_incomplete ? "incomplete":"checked");
    igText("search work %d / %d | selector work %d",query->work,query->request.work_limit,result->selector_work);
    if (query->request.kind==GD_STRUCTURE_RELATED && analysis->comparison.valid) igText("shared %d | first only %d | second only %d",analysis->comparison.shared,analysis->comparison.a_only,analysis->comparison.b_only);
    if (igTreeNodeEx_Str("Answer members and source rows",ImGuiTreeNodeFlags_DefaultOpen)) {
        int nodes=0,links=0;
        igTextWrapped("Retained native marks: support/context union, not the dominator set. Context counts below remain projection-wide; marks identify selected answer members. No query overlay changes canvas scope.");
        for (i=0;i<analysis->node_count;++i) if (query->marked_nodes[i] && nodes++<64) { int key=result->layout->nodes[i].key,source=gd_projection_record(result->project,result->view,key); igPushID_Int(i); if (igSmallButton(TextFormat("#%d %s",i,result->layout->nodes[i].label))) navigate(context,source,result->view==GD_EXPLORER_RESOURCES ? key:-1); igPopID(); }
        if (nodes>64) igText("%d further marked nodes in native result",nodes-64);
        for (i=0;i<analysis->edge_count;++i) if (query->marked_edges[i] && links++<64) link_row(context,i,0);
        if (links>64) igText("%d further marked source rows in native result",links-64);
        if (!nodes && !links) igTextWrapped("No retained selection-specific marks. Inspect context counts, operation parameters and completeness; not a gameplay absence proof.");
        igTreePop();
    }
    if (igTreeNode_Str("Nodes, direct counts and source navigation")) {
        for (i=0;i<analysis->node_count;++i) if (analysis->shown[i]) indices[count++]=i;
        if (state->page<0) state->page=0; if (state->page*64>=count) state->page=count ? (count-1)/64:0;
        if (igSmallButton("Previous 64") && state->page) --state->page;
        igSameLine(0,-1); if (igSmallButton("Next 64") && (state->page+1)*64<count) ++state->page;
        igText("Page %d | %d source-backed entries",state->page+1,count);
        for (i=state->page*64;i<count && i<(state->page+1)*64;++i) node_row(context,indices[i]);
        igTreePop();
    }
    if (igTreeNode_Str("Native witnesses and members")) {
        if (query->request.kind==GD_STRUCTURE_REACHABILITY && query->request.source>=0 && query->request.target>=0) { int arcs[64],length=gd_graph_closure_path(analysis,&analysis->reachability,0,query->request.target,arcs,64); witness(context,"Support suffix",arcs,length<0 ? 0:length<64 ? length:64,length); }
        if (query->request.kind==GD_STRUCTURE_REACHABILITY) {
            int roots=0;
            for (i=0;i<analysis->node_count;++i) if (query->roots[i]) {
                int member,shown=0; if (roots++>=64) continue; igPushID_Int(i);
                if (igTreeNode_Str(TextFormat("Start #%d: %s",i,result->layout->nodes[i].label))) {
                    igText("Row %s | %d retained targets",analysis->reachability.finished[i] ? "finished":"unfinished",analysis->reachability.count[i]);
                    for (member=0;member<analysis->node_count;++member) if (gd_graph_closure_contains(&analysis->reachability,i,member) && shown++<64) { int key=result->layout->nodes[member].key,source=gd_projection_record(result->project,result->view,key); igPushID_Int(member); if (igSmallButton(TextFormat("#%d %s",member,result->layout->nodes[member].label))) navigate(context,source,result->view==GD_EXPLORER_RESOURCES ? key:-1); igPopID(); }
                    if (shown>64) igText("%d further retained targets in native result",shown-64); igTreePop();
                }
                igPopID();
            }
            if (roots>64) igText("%d further starts in native result",roots-64);
        }
        if (query->request.kind==GD_STRUCTURE_DOMINATORS || query->request.kind==GD_STRUCTURE_BYPASS) {
            const GD_GraphPaths *paths=&analysis->paths;
            if (query->request.target>=0 && paths->selection.node==query->request.target) witness(context,"Support suffix",paths->selection.arcs,paths->selection.retained,paths->selection.length);
            else igTextUnformatted("Support suffix unavailable: no retained selected target",NULL);
            if (query->request.kind==GD_STRUCTURE_BYPASS) { const GD_PathBypass *bypass=&paths->bypass; if (!bypass->valid) igTextWrapped("Native bypass unavailable: incomplete starts or selection is not a nonmandatory support-path candidate. Not proof of no alternative."); else { igText("Bypass %s | found %s",bypass->complete ? "finished":"unfinished",bypass->found ? "yes":"no"); if (bypass->found) witness(context,"Avoiding suffix",bypass->arcs,bypass->retained,bypass->length); } }
        }
        if (query->request.kind==GD_STRUCTURE_RELATED && !analysis->comparison.valid) { for (i=0;i<analysis->pair_count && i<64;++i) igText("#%d / #%d: %d shared",analysis->pairs[i].a,analysis->pairs[i].b,analysis->pairs[i].shared); if (analysis->pair_count>64) igText("%d further pairs in native result",analysis->pair_count-64); }
        if (query->request.kind==GD_STRUCTURE_ADJACENCY) { int shown=0; const GD_GraphOverview *overview=&analysis->overview; igText("%d bins | %d input rows (not flow)",overview->bins,overview->link_count); for (i=0;i<overview->bins*overview->bins;++i) if (overview->counts[i] && shown++<64) { if (igTreeNode_Str(TextFormat("row bin %d / column bin %d: %d rows",i/overview->bins,i%overview->bins,overview->counts[i]))) { igTextUnformatted("First native source example (not complete cell provenance)",NULL); if (overview->first[i]>=0) link_row(context,overview->first[i],0); igTreePop(); } } if (shown>64) igText("%d further nonempty cells in native result",shown-64); }
        igTextWrapped("Indices are snapshot-local. Arrows remain authored; support is structural, never temporal execution. Lists cap at 64; native CLI retains complete bounded evidence.");
        igTreePop();
    }
}
void gd_explorer_structure_controls(GD_ExplorerContext *context) {
    GD_ExplorerPane *pane=context->pane; GD_ExplorerStructureState *state; GD_ExplorerStructureForm *form; int i;
    if (!gd_projection_name(pane->view) || !igCollapsingHeader_TreeNodeFlags("Built-in structural questions",0)) return;
    gd_explorer_structure_prepare(context);
    if (!pane->structure_state) pane->structure_state=empty_state(context->project->allocator);
    if (!(state=pane->structure_state)) { igTextWrapped("Cannot allocate question inputs; other pane state retained."); return; }
    form=&state->pending;
    igTextWrapped("Result scope: canonical loaded projection plus explicit filters below, not camera or current canvas filters/revealed neighborhood. Result tables navigate existing source; no query overlay or gameplay proof.");
    if (igBeginCombo("Question",gd_structure_name((GD_StructureKind)form->kind),0)) { for (i=0;i<GD_STRUCTURE_COUNT;++i) if (igSelectable_Bool(gd_structure_name((GD_StructureKind)i),form->kind==i,0,(ImVec2_c){0,0})) form->kind=i; igEndCombo(); }
    igInputText("From identity",form->from,sizeof(form->from),0,NULL,NULL); igInputText("To identity",form->to,sizeof(form->to),0,NULL,NULL);
    if (form->kind==GD_STRUCTURE_BYPASS) igInputText("Avoid identity",form->avoid,sizeof(form->avoid),0,NULL,NULL);
    if (pane->view==GD_EXPLORER_RELATIONSHIPS) igInputText("Scope root (one hop; blank all)",form->scope_root,sizeof(form->scope_root),0,NULL,NULL);
    if (igTreeNode_Str("Filters and bounded parameters")) {
        igInputTextMultiline("Starts (one per line)",form->starts,sizeof(form->starts),(ImVec2_c){0,55},0,NULL,NULL);
        igInputTextMultiline("Relation names (one per line)",form->relations,sizeof(form->relations),(ImVec2_c){0,55},0,NULL,NULL);
        igInputTextMultiline("Statuses (one per line)",form->statuses,sizeof(form->statuses),(ImVec2_c){0,45},0,NULL,NULL);
        if (igBeginCombo("Direction",form->direction==GD_STRUCTURE_FORWARD ? "forward":form->direction==GD_STRUCTURE_BACKWARD ? "backward":"both",0)) { const char *names[]={"forward","backward","both"}; for (i=0;i<3;++i) if (igSelectable_Bool(names[i],form->direction==i,0,(ImVec2_c){0,0})) form->direction=i; igEndCombo(); }
        if (igBeginCombo("Partner lens",form->lens==GD_GRAPH_ANY ? "any":form->lens==GD_GRAPH_SAME_ROLE ? "same-role":"changed-role",0)) { const char *names[]={"any","same-role","changed-role"}; for (i=0;i<3;++i) if (igSelectable_Bool(names[i],form->lens==i,0,(ImVec2_c){0,0})) form->lens=i; igEndCombo(); }
        igCheckbox("Include inactive structural rows",&form->include_inactive); igInputInt("Search work ceiling",&form->work_limit,0,0,0);
        igTextWrapped("Supply only parameters supported by question; unsupported combinations fail without replacing last result. Blank Relationships path relations exclude next; all explicitly includes continuation. Lists trim whitespace; 64 items / 8192 combined bytes each."); igTreePop();
    }
    if (igButton("Run named question",(ImVec2_c){0,0})) gd_explorer_structure_run(context);
    igSameLine(0,-1); if (igButton("Clear question",(ImVec2_c){0,0})) { GD_ExplorerStructureState *fresh=empty_state(context->project->allocator); if (fresh) { gd_explorer_structure_destroy(pane->structure_state); pane->structure_state=fresh; *pane->query_message=0; } else snprintf(pane->query_message,sizeof(pane->query_message),"Cannot clear question; last result/inputs retained"); }
    if (*pane->query_message) igTextWrapped("%s",pane->query_message);
    if (gd_explorer_structure_pending(pane)) igTextWrapped("Inputs pending; displayed result still uses applied parameters. Run or clear before refresh.");
    if (pane->structure_state->result) result_panel(context);
}
