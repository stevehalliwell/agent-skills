#include "gd_explorer_ui.h"
#include "gd_resource_routes.h"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_NO_EXPORT
#include "cimgui.h"
#include <ctype.h>
#include <string.h>

static void defaults(GD_ExplorerStructureForm *form) { memset(form,0,sizeof(*form)); form->work_limit=GD_GRAPH_ANALYSIS_WORK; }
static int equal_form(const GD_ExplorerStructureForm *a, const GD_ExplorerStructureForm *b) {
    return a->kind==b->kind && a->direction==b->direction && a->lens==b->lens && a->work_limit==b->work_limit && a->include_inactive==b->include_inactive && a->actor_scope==b->actor_scope &&
        !strcmp(a->from,b->from) && !strcmp(a->to,b->to) && !strcmp(a->avoid,b->avoid) && !strcmp(a->scope_root,b->scope_root) && !strcmp(a->starts,b->starts) && !strcmp(a->relations,b->relations) && !strcmp(a->statuses,b->statuses) && !strcmp(a->features,b->features) && !strcmp(a->entries,b->entries);
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
    if (state) { memset(state,0,sizeof(*state)); state->allocator=allocator; defaults(&state->pending); defaults(&state->applied); state->similarity_row=state->similarity_peer=state->comparison_first=state->comparison_second=-1; }
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
static GD_ExplorerStructureState *build_state(const GD_ExplorerProject *project, GD_ExplorerView view, const GD_ExplorerStructureForm *form, const GD_Query *actor_query, char *message, size_t size) {
    GD_ExplorerStructureState *state=empty_state(project->allocator); GD_Model *model=gd_explorer_model(project,view); GD_StructureOptions options={0}; int errors=model->error_count,omitted=model->errors_omitted;
    if (!state) { snprintf(message,size,"Cannot allocate structural question inputs; last result retained"); return NULL; }
    state->pending=state->applied=*form;
    memcpy(state->start_tokens,form->starts,sizeof(state->start_tokens)); memcpy(state->relation_tokens,form->relations,sizeof(state->relation_tokens)); memcpy(state->status_tokens,form->statuses,sizeof(state->status_tokens));
    options.question=gd_structure_question_name(form->kind); options.projection=view==GD_EXPLORER_ACTORS && form->actor_scope ? "actors":gd_projection_name(view);
    if (form->actor_scope && form->kind!=GD_STRUCTURE_DECLARATIONS) options.actor_query=actor_query;
    options.from=*form->from ? state->applied.from:NULL; options.to=*form->to ? state->applied.to:NULL; options.avoid=*form->avoid ? state->applied.avoid:NULL; options.scope_root=*form->scope_root ? state->applied.scope_root:NULL;
    options.root_count=lines(state->start_tokens,options.roots,GD_STRUCTURE_SELECTORS); options.relation_count=lines(state->relation_tokens,options.relations,GD_STRUCTURE_KINDS); options.status_count=lines(state->status_tokens,options.statuses,GD_STRUCTURE_SELECTORS);
    options.direction=(GD_StructureDirection)form->direction; options.lens=(GD_GraphLens)form->lens; options.include_inactive=form->include_inactive; options.work_limit=form->work_limit;
    if (form->kind==GD_STRUCTURE_COUNT+1) {
        const char *specs[GD_SIMILARITY_FEATURES]; int i;
        memcpy(state->feature_tokens,form->features,sizeof(state->feature_tokens)); memcpy(state->entry_tokens,form->entries,sizeof(state->entry_tokens));
        state->similarity_selection.features=state->feature_names; state->similarity_selection.weights=state->feature_weights; state->similarity_selection.entries=state->entry_ids; state->similarity_selection.work_limit=form->work_limit;
        state->similarity_selection.feature_count=lines(state->feature_tokens,specs,GD_SIMILARITY_FEATURES); state->similarity_selection.entry_count=lines(state->entry_tokens,state->entry_ids,GD_STRUCTURE_SELECTORS);
        if (state->similarity_selection.feature_count<0 || state->similarity_selection.entry_count<0) gd_error(model,"query.parameters","Similarity feature/entry list exceeds native bound",NULL,0);
        for (i=0;i<state->similarity_selection.feature_count;++i) {
            if (!gd_similarity_feature_spec(specs[i],state->feature_ids[i],sizeof(state->feature_ids[i]),&state->feature_weights[i])) { gd_error(model,"query.similarity_feature","Feature needs native ID optionally =finite nonnegative literal weight",NULL,0); break; }
            state->feature_names[i]=state->feature_ids[i];
        }
        options.similarity=&state->similarity_selection;
    }
    if (options.root_count<0 || options.relation_count<0 || options.status_count<0) gd_error(model,"query.parameters","Repeated question inputs exceed native count bound",NULL,0);
    else if (model->error_count==errors) state->result=gd_structure_result_build(model,project,&options);
    if (!state->result) snprintf(message,size,"%s: %s",model->error_count>errors ? model->errors[model->error_count-1].code:"query.memory",model->error_count>errors ? model->errors[model->error_count-1].message:"Cannot build structural result; last result retained");
    model->error_count=errors; model->errors_omitted=omitted;
    if (!state->result) { gd_explorer_structure_destroy(state); return NULL; }
    if (state->result->similarity && state->result->similarity->table_complete && state->result->similarity_first_node>=0) {
        int i; for (i=0;i<state->result->similarity->entry_count;++i) {
            if (state->result->similarity->nodes[i]==state->result->similarity_first_node) state->similarity_row=i;
            if (state->result->similarity->nodes[i]==state->result->similarity_second_node) state->similarity_peer=i;
        }
    }
    state->has_applied=1; state->revision=project->revision; state->similarity_canvas=form->kind==GD_STRUCTURE_COUNT+1; state->similarity_fit=true; return state;
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
    candidate=build_state(context->project,pane->view,&form,pane->query_state ? pane->query_state->query:NULL,pane->query_message,sizeof(pane->query_message));
    if (!candidate) return 0;
    gd_explorer_structure_destroy(pane->structure_state); pane->structure_state=candidate; *pane->query_message=0; return 1;
}
static int same_record(const GD_Record *a, const GD_Record *b) {
    return a->role==b->role && !strcmp(a->reference,b->reference) && ((!a->id && !b->id) || (a->id && b->id && !strcmp(a->id,b->id))) &&
        (a->role!=GD_AXIS || (!strcmp(a->axis.type ? a->axis.type:"",b->axis.type ? b->axis.type:"") && !strcmp(a->axis.unit ? a->axis.unit:"",b->axis.unit ? b->axis.unit:"")));
}
static int same_node(const GD_StructureResult *old, int a, const GD_StructureResult *fresh, int b) {
    int old_key,new_key;
    if (a<0 || b<0) return a==b;
    old_key=old->layout->nodes[a].key; new_key=fresh->layout->nodes[b].key;
    if (old->view==GD_EXPLORER_RESOURCES) return gd_resource_node_remap(old->project->resources,old_key,fresh->project->resources)==new_key;
    if (old->effects || fresh->effects) {
        const GD_EffectNode *x,*y; int first=-1,second=-1,depth;
        const GD_Model *before=gd_explorer_model(old->project,old->view),*after=gd_explorer_model(fresh->project,fresh->view);
        if (!old->effects || !fresh->effects || gd_effect_node_remap(old->effects,old_key,fresh->effects)!=new_key) return 0;
        x=&old->effects->nodes[old_key]; y=&fresh->effects->nodes[new_key];
        if ((x->axis<0)!=(y->axis<0)) return 0;
        if (x->axis>=0) { const GD_Record *a=&before->records[x->axis],*b=&after->records[y->axis]; if (!same_record(a,b) || strcmp(a->axis.type ? a->axis.type:"",b->axis.type ? b->axis.type:"") || strcmp(a->axis.unit ? a->axis.unit:"",b->axis.unit ? b->axis.unit:"")) return 0; }
        if (x->kind==GD_EFFECT_ACTIVATION) { first=x->activation; second=y->activation; }
        else if (x->relation>=0) { first=old->project->resources->relations[x->relation].activation; second=fresh->project->resources->relations[y->relation].activation; }
        for (depth=0;first>=0 && depth<=GD_MAX_RULE_EXPANSION_DEPTH;++depth) {
            const GD_ResourceNode *a=&old->project->resources->nodes[first],*b=&fresh->project->resources->nodes[second];
            int source=gd_explorer_resource_record(old->project->resources,first),target=gd_explorer_resource_record(fresh->project->resources,second);
            if (source<0 || target<0 || !same_record(&before->records[source],&after->records[target]) || strcmp(before->records[source].body ? before->records[source].body:"",after->records[target].body ? after->records[target].body:"") || a->instance!=b->instance) return 0;
            if (!a->instance) return 1;
            first=a->parent; second=b->parent; if (second<0) return 0;
        }
        return first<0;
    }
    if (old->actor_scene || fresh->actor_scene) {
        char before[GD_MAX_PATH],after[GD_MAX_PATH]; const GD_QuerySceneNode *x,*y; const GD_ActorQuery *p,*q; int first=-1,second=-1,i;
        if (!old->actor_scene || !fresh->actor_scene || !gd_structure_result_node_id(old,old_key,before,sizeof(before)) || !gd_structure_result_node_id(fresh,new_key,after,sizeof(after)) || strcmp(before,after)) return 0;
        x=&old->actor_scene->nodes[old_key]; y=&fresh->actor_scene->nodes[new_key]; p=old->options.actor_query->variants; q=fresh->options.actor_query->variants;
        if (x->kind==GD_QUERY_NODE_ACTIVATION) { first=x->source; second=y->source; }
        if (x->kind==GD_QUERY_NODE_EFFECT) { first=p->effects[x->source].activation; second=q->effects[y->source].activation; }
        if ((first<0)!=(second<0)) return 0;
        if (first>=0) {
            const GD_RuleActivation *a=&p->activations[first],*b=&q->activations[second];
            if (a->path_count!=b->path_count) return 0;
            for (i=0;i<a->path_count;++i) {
                const GD_Record *source=&p->analysis->model->records[p->analysis->rules->sources[p->analysis->rules->cases[a->path[i]].source].record],*target=&q->analysis->model->records[q->analysis->rules->sources[q->analysis->rules->cases[b->path[i]].source].record];
                if (!same_record(source,target) || strcmp(source->body ? source->body:"",target->body ? target->body:"")) return 0;
            }
        }
        return 1;
    }
    if (old->inventory || fresh->inventory) {
        char before[GD_MAX_PATH],after[GD_MAX_PATH]; const GD_ActorInventoryNode *x,*y;
        if (!old->inventory || !fresh->inventory || !gd_structure_result_node_id(old,old_key,before,sizeof(before)) || !gd_structure_result_node_id(fresh,new_key,after,sizeof(after)) || strcmp(before,after)) return 0;
        x=&old->inventory->nodes[old_key]; y=&fresh->inventory->nodes[new_key];
        if ((x->record<0)!=(y->record<0) || (x->record>=0 && !same_record(&old->inventory->source->model->records[x->record],&fresh->inventory->source->model->records[y->record]))) return 0;
        if (x->participant>=0 && y->participant>=0) {
            const GD_RuleParticipant *p=&old->project->rules->participants[x->participant],*q=&fresh->project->rules->participants[y->participant];
            if (p->noun_resolution!=q->noun_resolution || (!p->noun)!=(!q->noun) || (p->noun && strcmp(p->noun,q->noun))) return 0;
            if (p->noun_resolution==GD_RULE_RESOLVED && q->noun_resolution==GD_RULE_RESOLVED && !same_record(&old->inventory->source->model->records[p->noun_record],&fresh->inventory->source->model->records[q->noun_record])) return 0;
        }
        return 1;
    }
    return same_record(&gd_explorer_model(old->project,old->view)->records[old_key],&gd_explorer_model(fresh->project,fresh->view)->records[new_key]);
}
static int same_similarity(const GD_StructureResult *old,const GD_StructureResult *fresh) {
    const GD_SimilarityProjection *a=old->similarity,*b=fresh->similarity; int i,j;
    if (!a || !b) return a==b;
    if (a->map && a->map->available && (!b->map || !b->map->available)) return 0;
    if (a->feature_count!=b->feature_count) return 0;
    for (i=0;i<a->feature_count;++i) if (a->features[i].name && b->features[i].name) {
        const GD_SimilarityFeature *x=&a->features[i],*y=&b->features[i];
        if (strcmp(x->name,y->name) || x->kind!=y->kind || strcmp(x->unit ? x->unit:"",y->unit ? y->unit:"")) return 0;
    }
    if (old->options.similarity && old->options.similarity->entry_count && a->table_complete && b->table_complete) {
        if (a->entry_count!=b->entry_count) return 0;
        for (i=0;i<a->entry_count;++i) {
            int found=-1;
            for (j=0;j<b->entry_count;++j) if (!strcmp(a->ids[i],b->ids[j])) { if (found>=0) return 0; found=j; }
            if (found<0 || !same_node(old,a->nodes[i],fresh,b->nodes[found])) return 0;
            { int x=gd_structure_result_record(old,old->layout->nodes[a->nodes[i]].key),y=gd_structure_result_record(fresh,fresh->layout->nodes[b->nodes[found]].key);
              const GD_Model *before=gd_explorer_model(old->project,old->view),*after=gd_explorer_model(fresh->project,fresh->view);
              if (x>=0 && y>=0 && before->records[x].role==GD_AXIS && (strcmp(before->records[x].axis.type ? before->records[x].axis.type:"",after->records[y].axis.type ? after->records[y].axis.type:"") || strcmp(before->records[x].axis.unit ? before->records[x].axis.unit:"",after->records[y].axis.unit ? after->records[y].axis.unit:""))) return 0;
            }
        }
    }
    return 1;
}
static int same_selection(const GD_StructureResult *old, const GD_StructureResult *fresh) {
    const GD_StructureRequest *a=&old->query->request,*b=&fresh->query->request; int i;
    if (!same_similarity(old,fresh) || !same_node(old,old->similarity_first_node,fresh,fresh->similarity_first_node) || !same_node(old,old->similarity_second_node,fresh,fresh->similarity_second_node)) return 0;
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
    candidate=build_state(context->project,context->pane->view,&state->applied,context->pane->query_state ? context->pane->query_state->query:NULL,context->pane->query_message,sizeof(context->pane->query_message)); if (!candidate) return 0;
    before=state->result;
    /* Inactive skill panes can retain a result whose borrowed draft strings
       were replaced. Rebuild from current old project before reading identity. */
    if (!before || (gd_explorer_family(prior->view)==GD_EXPLORER_SKILL_DATA && state->revision!=old->revision)) {
        temporary=build_state(old,prior->view,&state->applied,prior->query_state ? prior->query_state->query:NULL,context->pane->query_message,sizeof(context->pane->query_message));
        before=temporary ? temporary->result:NULL;
    }
    if (!before || !same_selection(before,candidate->result)) {
        gd_explorer_structure_destroy(temporary); gd_explorer_structure_destroy(candidate);
        snprintf(context->pane->query_message,sizeof(context->pane->query_message),"Refresh blocked: structural selector identity changed or unavailable. Clear question first."); return 0;
    }
    gd_explorer_structure_destroy(temporary); candidate->page=state->page;
    candidate->similarity_canvas=state->similarity_canvas; candidate->similarity_camera=state->similarity_camera; candidate->similarity_camera_ready=state->similarity_camera_ready; candidate->similarity_fit=state->similarity_fit || !state->similarity_camera_ready;
    /* Ad-hoc point inspection resets. Explicit pair re-resolves from IDs;
       never reinterpret old row offsets after source replacement. */
    context->pane->structure_state=candidate; return 1;
}
static void navigate(GD_ExplorerContext *context, int record, int resource_node) {
    GD_ExplorerPane *pane=context->pane; if (record<0) return;
    if (pane->view==GD_EXPLORER_MDA) { pane->mda.record=record; pane->mda.link=-1; }
    else if (pane->view==GD_EXPLORER_RESOURCES) { pane->selection.node=resource_node; pane->selection.cell=-1; pane->selection.linked=resource_node<0 ? record:-1; }
    else if (pane->view==GD_EXPLORER_ACTORS || pane->view==GD_EXPLORER_EFFECTS) { pane->selection.node=-1; pane->selection.cell=-1; pane->selection.linked=record; }
    else pane->navigate=record;
}
void gd_explorer_structure_source_node(GD_ExplorerContext *context,int node) {
    const GD_StructureResult *result=context->pane->structure_state ? context->pane->structure_state->result:NULL; int key;
    if (!result || node<0 || node>=result->layout->node_count) return;
    key=result->layout->nodes[node].key; navigate(context,gd_structure_result_record(result,key),result->view==GD_EXPLORER_RESOURCES ? key:-1);
}
static int node_id(const GD_StructureResult *result, int index, char *text, size_t capacity) {
    return gd_structure_result_node_id(result,result->layout->nodes[index].key,text,capacity);
}
static void node_row(GD_ExplorerContext *context, int index) {
    GD_ExplorerStructureState *state=context->pane->structure_state; const GD_StructureResult *result=state->result; const GD_StructureQuery *query=result->query;
    const GD_StructureNode *counts=&query->nodes[index]; int key=result->layout->nodes[index].key,record=gd_structure_result_record(result,key); char identity[GD_MAX_PATH];
    igPushID_Int(index);
    if (igSmallButton(TextFormat("#%d %s",index,result->layout->nodes[index].label))) navigate(context,record,result->view==GD_EXPLORER_RESOURCES ? key:-1);
    if (query->request.incidence) igText("%d incident rows | %d distinct partners | %d kinds",counts->incident,counts->partners,counts->kinds);
    else { igText("in %d | out %d | %d partners | %d kinds",counts->incoming,counts->outgoing,counts->partners,counts->kinds); igText("%d incident source rows",counts->incident); }
    if (result->effects) { const GD_EffectNode *item=&result->effects->nodes[key]; igTextWrapped("%s | %s%s%s",gd_effect_kind_name(item->kind),item->axis>=0 ? gd_explorer_model(result->project,result->view)->records[item->axis].reference:"unresolved axis",item->value ? " = ":"",item->value ? item->value:""); }
    if (result->actor_scene) { const GD_QuerySceneNode *item=&result->actor_scene->nodes[key]; igText("%s | variant %d | applicability %s",gd_query_scene_node_kind_name(item->kind),item->variant,gd_rule_truth_name(item->applicability)); }
    if (result->inventory) { const GD_ActorInventoryNode *item=&result->inventory->nodes[key]; igText("%s | native case %d | local binding %d",gd_actor_inventory_kind_name(item->kind),item->case_index,item->participant); }
    if (query->request.source>=0 || query->request.target>=0) igText("Retained evidence member: %s",query->marked_nodes[index] ? "yes":"no");
    if (gd_structure_comparison(query)) { unsigned flags=gd_structure_comparison(query)->nodes[index]; igText("%s membership: %s",query->request.kind==GD_STRUCTURE_RELATED_PATHS ? "Reached-node":"Partner",flags==GD_GRAPH_SHARED ? "shared":flags==GD_GRAPH_A_ONLY ? "first only":flags==GD_GRAPH_B_ONLY ? "second only":flags==GD_GRAPH_OTHER_ROLE ? "other role":"neither"); }
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
    const GD_StructureResult *result=context->pane->structure_state->result; const GD_LayoutEdge *edge=&result->layout->edges[index]; int source=gd_structure_result_edge_record(result,edge->key);
    igPushID_Int(index); igTextWrapped("link #%d: %s%s%s | %s%s%s",index,result->layout->nodes[edge->a].label,result->query->request.incidence ? " -- ":" -> ",result->layout->nodes[edge->b].label,gd_graph_relation_name(edge->relation),result->query->request.incidence ? " | undirected incidence":"",reverse ? " | reverse to stored endpoints":"");
    if (result->effects) { const GD_EffectLink *item=&result->effects->links[edge->key]; igText("Native relation %d | line %d | %s %s",item->relation,result->project->resources->relations[item->relation].line,item->kind==GD_EFFECT_DECLARATION ? "effect target":"call owner",item->participant>=0 ? result->project->rules->participants[item->participant].name:"none"); }
    if (result->actor_scene) { const GD_QuerySceneEdge *item=&result->actor_scene->edges[edge->key]; igText("Native %s row %d | variant %d",gd_query_scene_edge_kind_name(item->kind),item->source,item->variant); }
    if (result->inventory) { const GD_ActorInventoryLink *item=&result->inventory->links[edge->key]; igText("Native declaration %d | compiled binding %d | line %d",item->declaration,item->binding,gd_actor_inventory_link_line(result->inventory,edge->key)); }
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
static void related_path_rows(GD_ExplorerContext *context) {
    GD_ExplorerStructureState *state=context->pane->structure_state;
    const GD_StructureResult *result=state->result; const GD_StructureQuery *query=result->query;
    const GD_GraphClosure *closure=&query->analysis.reachability; const GD_GraphComparison *comparison=gd_structure_comparison(query);
    int i,shown=0;
    igTextWrapped("Strict reached-node sets through selected typed kinds, not an ordered-path grammar. Unselected kinds stop traversal; Relationships excludes next by default. Shared endpoints do not prove equivalent roles, learning, applicability or outcomes.");
    if (!comparison) { igTextWrapped("Selected closure rows unfinished or unavailable; no absence claim."); return; }
    igText("%d checked peers share reached nodes (finished rows only)",closure->peer_count);
    for (i=0;i<closure->peer_count && i<64;++i) {
        int peer=closure->peers[i]; char identity[GD_MAX_PATH]; igPushID_Int(peer);
        igText("#%d %s | %d shared",peer,result->layout->nodes[peer].label,closure->shared[peer]);
        if (node_id(result,peer,identity,sizeof(identity)) && igSmallButton("Compare this peer")) snprintf(state->pending.to,sizeof(state->pending.to),"%s",identity);
        igPopID();
    }
    if (closure->peer_count>64) igText("%d further peers in native result",closure->peer_count-64);
    for (i=0;i<closure->row_count;++i) if (comparison->nodes[i]) {
        if (shown++>=64) continue;
        igPushID_Int(i);
        if (igTreeNode_Str(TextFormat("Reached member #%d: %s",i,result->layout->nodes[i].label))) {
            int side;
            igText("Membership: %s",comparison->nodes[i]==GD_GRAPH_SHARED ? "shared":comparison->nodes[i]==GD_GRAPH_A_ONLY ? "first only":"second only");
            for (side=0;side<2;++side) {
                int arcs[64],length=gd_graph_closure_path(&query->analysis,closure,side,i,arcs,64);
                witness(context,side ? "Second root support":"First root support",arcs,length<0 ? 0:length<64 ? length:64,length);
            }
            igTreePop();
        }
        igPopID();
    }
    if (shown>64) igText("%d further members in native result",shown-64);
}
static void finding_rows(GD_ExplorerContext *context, const char *label, const GD_Finding *findings, int count, int omitted) {
    int i; if (!igTreeNode_Str(label)) return;
    igText("%d retained findings; %d omitted by native reader",count,omitted);
    for (i=0;i<count && i<64;++i) { const GD_Finding *finding=&findings[i]; igPushID_Int(i); igTextWrapped("%s: %s | %s | line %d",finding->code,finding->message,finding->reference ? finding->reference:"unspecified",finding->line); if (finding->record>=0 && igSmallButton("Inspect finding source")) navigate(context,finding->record,-1); igPopID(); }
    if (count>64) igText("%d further findings in native result",count-64); igTreePop();
}
static void gap_panel(GD_ExplorerContext *context) {
    GD_ExplorerStructureState *state=context->pane->structure_state; const GD_StructureResult *result=state->result; const GD_DeclarationGaps *gaps=result->gaps;
    const GD_Model *model=gd_explorer_model(result->project,result->view); int i,total=gaps->rows-gaps->counts[GD_DECLARATION_RESOLVED],pages=(total+63)/64,ordinal=0;
    igTextWrapped("Whole loaded reader diagnostic context, not graph/canvas filters. Only consumed explicit declarations and native findings; no new validation rules or required metadata inferred.");
    igTextWrapped("%d declared references: %d resolved; %d missing; %d unloaded; %d ambiguous",gaps->rows,gaps->counts[GD_DECLARATION_RESOLVED],gaps->counts[GD_DECLARATION_MISSING],gaps->counts[GD_DECLARATION_UNLOADED],gaps->counts[GD_DECLARATION_AMBIGUOUS]);
    igTextWrapped("Inventory checked; source %s. Domain/type findings below remain separate from identity resolution. Empty list is not a complete design or missing-metadata proof.",result->source_incomplete ? "incomplete":"checked within loaded scope");
    if (state->page>=pages) state->page=pages ? pages-1:0;
    if (pages) igSliderInt("Declaration gap page",&state->page,0,pages-1,"%d",0);
    for (i=0;i<gaps->rows;++i) if (gaps->identities[i]!=GD_DECLARATION_RESOLVED) {
        const GD_Edge *edge=&model->edges[i]; if (ordinal++<state->page*64 || ordinal>state->page*64+64) continue;
        igPushID_Int(i); igTextWrapped("%s | %s: %s | %s | line %d",model->records[edge->source].reference,gd_field_name(edge->field),edge->reference,gd_declaration_identity_name((GD_DeclarationIdentity)gaps->identities[i]),edge->line);
        if (igSmallButton("Inspect declaring source")) navigate(context,edge->source,-1); igPopID();
    }
    igText("%d identity gaps; 64 rows/page",total);
    finding_rows(context,"Model native findings",model->findings,model->finding_count,model->findings_omitted);
    if (result->view==GD_EXPLORER_MDA) finding_rows(context,"MDA native findings",result->project->mda->findings,result->project->mda->finding_count,result->project->mda->findings_omitted);
    if (result->view==GD_EXPLORER_RESOURCES || result->inventory || result->actor_scene || result->effects) finding_rows(context,"Rule native findings",result->project->rules->findings,result->project->rules->finding_count,result->project->rules->findings_omitted);
}
static void result_panel(GD_ExplorerContext *context) {
    GD_ExplorerStructureState *state=context->pane->structure_state; const GD_StructureResult *result=state->result; const GD_StructureQuery *query=result->query; const GD_GraphAnalysis *analysis=&query->analysis; int i,count=0,indices[GD_MAX_RECORDS];
    igSeparator(); igText("Applied: %s [%s]",gd_structure_name(query->request.kind),result->options.projection);
    if (result->gaps) { gap_panel(context); return; }
    if (result->effects) { const GD_EffectGraph *graph=result->effects; igTextWrapped("Effect scope: whole loaded declaration concepts and concrete mapped placements, not selected contributor/weight display or camera. Links retain targets/proposals; no state-matching edges, recipe, execution, frequency or final-outcome proof."); igText("Construction %d/%d%s | unexamined nodes %d / relations %d | unknown effects %d",graph->work,graph->work_limit,graph->limited ? " [LIMITED]":"",graph->unexamined_source_nodes,graph->unexamined_relations,graph->unknown_effects); }
    if (result->actor_scene) {
        const GD_ActorQuery *source=result->options.actor_query->variants; const GD_RuleCase *root=&result->project->rules->cases[source->case_index]; int i;
        igTextWrapped("Finite actor scope: %s#%s | local %s | %s. All retained hypotheses/excluded checks; independent of ghosts, camera and revealed neighborhood. Links are evidence associations, not execution order or feasibility proof.",gd_explorer_model(result->project,result->view)->records[result->project->rules->sources[root->source].record].reference,root->name,result->project->rules->participants[source->participant].name,gd_explorer_model(result->project,result->view)->records[source->axis].reference);
        igText("Native hypotheses %d | fixed facts %d | contexts %d | topology work %d/%d%s",source->variant_count,source->fixed.fact_count,source->fixed.context_count,result->actor_scene->work,result->actor_scene->work_limit,result->actor_scene->limited ? " [LIMITED]":"");
        if (igTreeNode_Str("Applied finite hypotheses")) { for (i=0;i<source->variant_count;++i) igText("%s | %s | proposals %d%s",source->variants[i].value,gd_rule_truth_name(source->variants[i].applicability),source->variants[i].write_count,source->variants[i].limited ? " [LIMITED]":""); igTreePop(); }
    }
    if (result->inventory) {
        const GD_ActorInventory *inventory=result->inventory;
        igTextWrapped("Inventory-wide declarations. Not finite state hypotheses, applicability, concrete expanded placements or execution. Shared noun definitions never merge local bindings.");
        igText("Construction %s | work %d / %d",inventory->limited ? "limited":"finished",inventory->work,inventory->work_limit);
        igTextWrapped("Unexamined: %d records, %d sources, %d cases, %d bindings, %d reference rows, %d calls",inventory->unexamined_records,inventory->unexamined_sources,inventory->unexamined_cases,inventory->unexamined_participants,inventory->unexamined_edges,inventory->unexamined_calls);
        igTextWrapped("Checked unknowns: %d noun refs, %d axis refs, %d calls; %d incomplete sources, %d incomplete call owners",inventory->unresolved_nouns,inventory->unresolved_axes,inventory->unresolved_calls,inventory->incomplete_sources,inventory->incomplete_call_owners);
    }
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
    if (gd_structure_comparison(query)) { const GD_GraphComparison *comparison=gd_structure_comparison(query); igText("shared %d | first only %d | second only %d",comparison->shared,comparison->a_only,comparison->b_only); }
    if (igTreeNodeEx_Str("Answer members and source rows",ImGuiTreeNodeFlags_DefaultOpen)) {
        int nodes=0,links=0;
        igTextWrapped("Retained native marks: support/context union, not the dominator set. Context counts below remain projection-wide; marks identify selected answer members. No query overlay changes canvas scope.");
        for (i=0;i<analysis->node_count;++i) if (query->marked_nodes[i] && nodes++<64) { int key=result->layout->nodes[i].key,source=gd_structure_result_record(result,key); igPushID_Int(i); if (igSmallButton(TextFormat("#%d %s",i,result->layout->nodes[i].label))) navigate(context,source,result->view==GD_EXPLORER_RESOURCES ? key:-1); igPopID(); }
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
        if (query->request.kind==GD_STRUCTURE_RELATED_PATHS) related_path_rows(context);
        if (query->request.kind==GD_STRUCTURE_REACHABILITY && query->request.source>=0 && query->request.target>=0) { int arcs[64],length=gd_graph_closure_path(analysis,&analysis->reachability,0,query->request.target,arcs,64); witness(context,"Support suffix",arcs,length<0 ? 0:length<64 ? length:64,length); }
        if (query->request.kind==GD_STRUCTURE_REACHABILITY) {
            int roots=0;
            for (i=0;i<analysis->node_count;++i) if (query->roots[i]) {
                int member,shown=0; if (roots++>=64) continue; igPushID_Int(i);
                if (igTreeNode_Str(TextFormat("Start #%d: %s",i,result->layout->nodes[i].label))) {
                    igText("Row %s | %d retained targets",analysis->reachability.finished[i] ? "finished":"unfinished",analysis->reachability.count[i]);
                    for (member=0;member<analysis->node_count;++member) if (gd_graph_closure_contains(&analysis->reachability,i,member) && shown++<64) { int key=result->layout->nodes[member].key,source=gd_structure_result_record(result,key); igPushID_Int(member); if (igSmallButton(TextFormat("#%d %s",member,result->layout->nodes[member].label))) navigate(context,source,result->view==GD_EXPLORER_RESOURCES ? key:-1); igPopID(); }
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
    if (!gd_projection_name(pane->view) || !igCollapsingHeader_TreeNodeFlags("Built-in structural / similarity questions",0)) return;
    gd_explorer_structure_prepare(context);
    if (!pane->structure_state) pane->structure_state=empty_state(context->project->allocator);
    if (!(state=pane->structure_state)) { igTextWrapped("Cannot allocate question inputs; other pane state retained."); return; }
    form=&state->pending;
    if (pane->view==GD_EXPLORER_ACTORS) {
        igCombo_Str("Analysis scope##structure",&form->actor_scope,"Declaration inventory\0State-specific (applied variants)\0",-1);
        if (form->actor_scope) igTextWrapped("Uses last applied native actor-states query, including fixed facts/context. Apply variants first; coverage is not a finite topology. Display ghosts/primary/compare/excluded controls do not filter this scope. Replace old-scope selectors before Run.");
        else igTextWrapped("Declaration scope includes unused noun/axis definitions, distinct local bindings and authored calls. Does not evaluate guards or merge same-noun participants.");
    }
    if (pane->view==GD_EXPLORER_EFFECTS) igTextWrapped("Scope: whole loaded Effect declarations plus question filters; independent of selected result/contributor context, weighting and camera. Concepts are associations; direct counts are not runtime frequency.");
    else if (pane->view==GD_EXPLORER_ACTORS && !form->actor_scope) igTextWrapped("Scope: inventory-wide declarations plus filters below. Case/participant/axis/values/facts above, ghost display and camera do not restrict this inventory. Tables inspect source; no applicability or execution proof.");
    else if (pane->view!=GD_EXPLORER_ACTORS) igTextWrapped("Result scope: canonical loaded projection plus explicit filters below, not camera or current canvas filters/revealed neighborhood. Result tables navigate existing source; no query overlay or gameplay proof.");
    if (igBeginCombo("Question",gd_structure_question_name(form->kind),0)) {
        for (i=0;i<GD_STRUCTURE_QUESTIONS;++i) if (igSelectable_Bool(gd_structure_question_name(i),form->kind==i,0,(ImVec2_c){0,0})) {
            if (i==GD_STRUCTURE_DECLARATIONS) { int scope=form->actor_scope; defaults(form); form->actor_scope=scope; }
            if (i>=GD_STRUCTURE_COUNT) {
                form->direction=GD_STRUCTURE_FORWARD; form->lens=GD_GRAPH_ANY; *form->starts=*form->avoid=0;
                if (i==GD_STRUCTURE_COUNT) *form->from=*form->to=*form->features=*form->entries=0;
                if (form->work_limit==GD_GRAPH_ANALYSIS_WORK) form->work_limit=GD_SIMILARITY_WORK;
            } else *form->features=*form->entries=0;
            form->kind=i;
        }
        igEndCombo();
    }
    if (form->kind>=GD_STRUCTURE_COUNT) igTextWrapped("Similarity keeps explicit status/relation/scope filters. Selection clears traversal starts/avoid/direction/lens; discovery also clears inspected pair/features/entries. Feature directions are native endpoints, not execution. Geometry is separate from ordinary layout.");
    if (form->kind==GD_STRUCTURE_DECLARATIONS) igTextWrapped("Declaration inventory uses whole loaded reader context. Selecting it clears pending graph filters; no source changes or missing-metadata rules.");
    else {
    if (form->kind!=GD_STRUCTURE_COUNT) { igInputText(form->kind==GD_STRUCTURE_COUNT+1 ? "Inspect first (optional)":"From identity",form->from,sizeof(form->from),0,NULL,NULL); igInputText(form->kind==GD_STRUCTURE_COUNT+1 ? "Inspect second (optional)":"To identity",form->to,sizeof(form->to),0,NULL,NULL); }
    if (form->kind==GD_STRUCTURE_COUNT+1) {
        igInputTextMultiline("Features ID[=WEIGHT], one per line",form->features,sizeof(form->features),(ImVec2_c){0,75},0,NULL,NULL);
        igInputTextMultiline("Exact entry IDs (blank all shown)",form->entries,sizeof(form->entries),(ImVec2_c){0,65},0,NULL,NULL);
        igTextWrapped("Discover with similarity-features first. Default weight 1; literal weights only. Up to 256 features and 64 explicit IDs; blank entries selects all up to 4096 shown nodes. Lists each retain 8192 bytes. Missing even zero-weight cells exclude rows; source context stays intact.");
    }
    if (form->kind==GD_STRUCTURE_BYPASS) igInputText("Avoid identity",form->avoid,sizeof(form->avoid),0,NULL,NULL);
    if (pane->view==GD_EXPLORER_RELATIONSHIPS) igInputText("Scope root (one hop; blank all)",form->scope_root,sizeof(form->scope_root),0,NULL,NULL);
    if (igTreeNode_Str("Filters and bounded parameters")) {
        if (form->kind<GD_STRUCTURE_COUNT) igInputTextMultiline("Starts (one per line)",form->starts,sizeof(form->starts),(ImVec2_c){0,55},0,NULL,NULL);
        igInputTextMultiline("Relation names (one per line)",form->relations,sizeof(form->relations),(ImVec2_c){0,55},0,NULL,NULL);
        igInputTextMultiline("Statuses (one per line)",form->statuses,sizeof(form->statuses),(ImVec2_c){0,45},0,NULL,NULL);
        if (form->kind<GD_STRUCTURE_COUNT && igBeginCombo("Direction",form->direction==GD_STRUCTURE_FORWARD ? "forward":form->direction==GD_STRUCTURE_BACKWARD ? "backward":"both",0)) { const char *names[]={"forward","backward","both"}; for (i=0;i<3;++i) if (igSelectable_Bool(names[i],form->direction==i,0,(ImVec2_c){0,0})) form->direction=i; igEndCombo(); }
        if (form->kind<GD_STRUCTURE_COUNT && igBeginCombo("Partner lens",form->lens==GD_GRAPH_ANY ? "any":form->lens==GD_GRAPH_SAME_ROLE ? "same-role":"changed-role",0)) { const char *names[]={"any","same-role","changed-role"}; for (i=0;i<3;++i) if (igSelectable_Bool(names[i],form->lens==i,0,(ImVec2_c){0,0})) form->lens=i; igEndCombo(); }
        igCheckbox("Include inactive structural rows",&form->include_inactive); igInputInt("Search work ceiling",&form->work_limit,0,0,0);
        igTextWrapped("Supply only parameters supported by question; unsupported combinations fail without replacing last result. Blank Relationships path relations exclude next; all explicitly includes continuation. Lists trim whitespace; 64 items / 8192 combined bytes each."); igTreePop();
    }
    }
    if (igButton("Run named question",(ImVec2_c){0,0})) gd_explorer_structure_run(context);
    igSameLine(0,-1); if (igButton("Clear question",(ImVec2_c){0,0})) { GD_ExplorerStructureState *fresh=empty_state(context->project->allocator); if (fresh) { gd_explorer_structure_destroy(pane->structure_state); pane->structure_state=fresh; *pane->query_message=0; } else snprintf(pane->query_message,sizeof(pane->query_message),"Cannot clear question; last result/inputs retained"); }
    if (*pane->query_message) igTextWrapped("%s",pane->query_message);
    if (gd_explorer_structure_pending(pane)) igTextWrapped("Inputs pending; displayed result still uses applied parameters. Run or clear before refresh.");
    if (pane->structure_state->result) {
        if (gd_structure_question_index(pane->structure_state->result->options.question)>=GD_STRUCTURE_COUNT) {
            gd_explorer_similarity_controls(context);
            if (igTreeNode_Str("Native canonical context tables")) { result_panel(context); igTreePop(); }
        } else result_panel(context);
    }
}
