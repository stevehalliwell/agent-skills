#include "gd_explorer_controls.h"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_NO_EXPORT
#include "cimgui.h"
#include "gd_explorer_loaded_pickers.h"
#include "gd_graph_analysis_draw.h"
#include <math.h>
#include <string.h>

void gd_explorer_query_destroy(GD_ExplorerQueryState *state) {
    if (!state) return;
    gd_query_scene_destroy(state->scene); gd_effect_query_destroy(state->selected); gd_query_destroy(state->query);
    if (state->empty) state->allocator.release(state->allocator.context,state->empty);
    state->allocator.release(state->allocator.context,state);
}
static int query_place(GD_ExplorerContext *context,GD_QueryScene *scene,GD_LayoutMode mode) {
    float aspect=1;
    if(isfinite(context->viewport.width) && isfinite(context->viewport.height) && context->viewport.width>0 && context->viewport.height>0) {
        float measured=context->viewport.width/context->viewport.height;if(isfinite(measured) && measured>0) aspect=measured;
    }
    if(!gd_query_scene_place(scene,mode,context->pane->workspace_bounds,aspect))return 0;
    return scene->layout.mode!=GD_LAYOUT_FORCE || gd_query_scene_islands(scene,context->pane->island_separation);
}
int gd_explorer_query_arrange(GD_ExplorerContext *context,GD_LayoutMode mode) {
    GD_ExplorerPane *pane=context->pane;
    if(!pane->query_state || !pane->query_state->scene || !query_place(context,pane->query_state->scene,mode)) return 0;
    pane->layout_choice=mode;pane->selection.held=-1;pane->paused=false;
    pane->fit=pane->view==GD_EXPLORER_EFFECTS || pane->query_state->applied.coverage ? GD_EXPLORER_FIT_QUERY_READABLE:1;return 1;
}
static int focus_result(GD_ExplorerContext *context,GD_ExplorerQueryState *state, int target) {
    GD_EffectSelection selection=state->query->contributors->selection; GD_EffectQuery *query; GD_QueryScene *scene; int i,matches=0;
    selection.target=target; query=gd_effect_query_build(state->query->effects,selection,GD_EFFECT_WORK); if (!query) return 0;
    scene=gd_query_scene_effect_context(state->query,query,state->applied.weighted,GD_EFFECT_WORK);
    if (!scene) { gd_effect_query_destroy(query); return 0; }
    for (i=0;i<scene->layout.node_count;++i) if (scene->nodes[i].source==target && scene->nodes[i].kind==GD_QUERY_NODE_EFFECT) ++matches;
    if (matches!=1) { gd_query_scene_destroy(scene); gd_effect_query_destroy(query); return 0; }
    if(state->scene && !query_place(context,scene,(GD_LayoutMode)context->pane->layout_choice)) {gd_query_scene_destroy(scene);gd_effect_query_destroy(query);return 0;}
    gd_query_scene_destroy(state->scene); gd_effect_query_destroy(state->selected); state->selected=query; state->scene=scene; return 1;
}
static GD_ExplorerQueryState *build_state(GD_ExplorerContext *context, const GD_ExplorerQueryOptions *options, int allow_empty, const char *primary_value) {
    GD_Model *model=gd_explorer_model(context->project,context->pane->view); GD_Allocator allocator=context->project->allocator;
    GD_ExplorerQueryState *state=allocator.allocate(allocator.context,sizeof(*state)); GD_QueryRequest request={0}; char *cursor,*item;
    int i,errors=model->error_count,omitted=model->errors_omitted,actor=context->pane->view==GD_EXPLORER_ACTORS;
    int inventory=actor && allow_empty && !options->coverage && (options->root_case<0 || options->axis<0);
    if (!state) { snprintf(context->pane->query_message,sizeof(context->pane->query_message),"Cannot allocate query state; last good query retained."); return NULL; }
    memset(state,0,sizeof(*state)); state->allocator=allocator; state->applied=*options;
    if (actor && allow_empty && options->coverage && options->axis<0) {
        state->empty=allocator.allocate(allocator.context,sizeof(*state->empty));
        if (!state->empty) goto failure;
        memset(state->empty,0,sizeof(*state->empty)); return state;
    }
    request.kind=inventory ? GD_QUERY_BINDINGS:actor ? (options->coverage ? GD_QUERY_STATE_COVERAGE:GD_QUERY_ACTOR_STATES):GD_QUERY_CONTRIBUTORS; request.noun=actor && options->coverage ? options->noun:-1; request.effect_kind=-1; request.work_limit=GD_EFFECT_WORK;
    request.case_index=actor && !inventory ? options->root_case:-1; request.axis=actor && !inventory ? options->axis:-1;
    if (options->accepted) { request.status_count=1; request.statuses[0]="accepted"; }
    if (actor && options->coverage) {
        if (*options->facts || *options->contexts) { snprintf(context->pane->query_message,sizeof(context->pane->query_message),"Coverage uses independent hypotheses, not fixed facts/context. Clear those inputs first."); goto failure; }
        if (options->participant>=0) {
            if (options->root_case<0 || options->root_case>=context->project->rules->case_count || options->participant>=context->project->rules->participant_count) goto failure;
            { const GD_RuleCase *root=&context->project->rules->cases[options->root_case]; if (options->participant<root->first_participant || options->participant>=root->first_participant+root->participant_count) goto failure; }
            request.participant=context->project->rules->participants[options->participant].name;
        }
        memcpy(state->value_tokens,options->values,sizeof(state->value_tokens)); cursor=state->value_tokens;
        while ((item=gd_picker_next_item(&cursor,options->literal_values))) { if (request.value_count==GD_ACTOR_VARIANTS) { snprintf(context->pane->query_message,sizeof(context->pane->query_message),"At most 64 finite values."); goto failure; } request.values[request.value_count++]=item; }
    } else if (actor && !inventory) {
        if (options->root_case<0 || options->root_case>=context->project->rules->case_count || options->participant<0 || options->participant>=context->project->rules->participant_count) { snprintf(context->pane->query_message,sizeof(context->pane->query_message),"Choose an explicit root case, declared participant and applicable axis."); goto failure; }
        { const GD_RuleCase *root=&context->project->rules->cases[options->root_case]; if (options->participant<root->first_participant || options->participant>=root->first_participant+root->participant_count) { snprintf(context->pane->query_message,sizeof(context->pane->query_message),"Participant must belong to selected root case; no same-name fallback."); goto failure; } }
        request.participant=context->project->rules->participants[options->participant].name;
        memcpy(state->value_tokens,options->values,sizeof(state->value_tokens)); cursor=state->value_tokens;
        while ((item=gd_picker_next_item(&cursor,options->literal_values))) { if (request.value_count==GD_ACTOR_VARIANTS) { snprintf(context->pane->query_message,sizeof(context->pane->query_message),"At most 64 finite values."); goto failure; } request.values[request.value_count++]=item; }
        memcpy(state->fact_tokens,options->facts,sizeof(state->fact_tokens)); cursor=state->fact_tokens;
        while ((item=gd_picker_next_item(&cursor,options->literal_assumptions))) if (!gd_rule_assume(model,context->project->rules,options->root_case,item,&state->fixed)) goto failure;
        memcpy(state->context_tokens,options->contexts,sizeof(state->context_tokens)); cursor=state->context_tokens;
        while ((item=gd_picker_next_item(&cursor,options->literal_assumptions))) { int positive=*item!='!'; if (!positive) ++item; if (!gd_rule_assume_context(model,context->project->rules,options->root_case,item,positive,&state->fixed)) goto failure; }
        if (state->fixed.fact_count || state->fixed.context_count) request.fixed=&state->fixed;
    }
    state->query=gd_query_build(model,context->project->rules,context->project->calls,&request); if (!state->query) goto failure;
    if (state->query->variants) {
        if (primary_value) {
            for (i=0;i<state->query->variants->variant_count;++i) if (!strcmp(primary_value,state->query->variants->variants[i].value)) break;
            if (i==state->query->variants->variant_count) { snprintf(context->pane->query_message,sizeof(context->pane->query_message),"Primary typed value disappeared; last good state retained. Reset query to accept new scope."); goto failure; }
            state->applied.primary=i;
        } else if (state->applied.primary<0 || state->applied.primary>=state->query->variants->variant_count) state->applied.primary=state->query->variants->variant_count ? 0:-1;
        if (state->applied.compare>=state->query->variants->variant_count) state->applied.compare=-1;
    } else if (actor) state->applied.primary=state->applied.compare=-1;
    if (state->query->effects) {
        int best=-1;
        for (i=0;i<state->query->effects->node_count;++i) if (state->query->effects->nodes[i].kind!=GD_EFFECT_ACTIVATION && (best<0 || state->query->contributors->incoming[i]>state->query->contributors->incoming[best])) best=i;
        if (best>=0 && !focus_result(context,state,best)) goto failure;
    }
    if (!state->scene) state->scene=inventory ? gd_query_scene_overview(state->query,(GD_ActorOverviewMode)context->pane->actor_overview,GD_EFFECT_WORK):gd_query_scene_build(state->query,(GD_QueryGhostMode)state->applied.ghosts,state->applied.primary,state->applied.weighted,state->applied.include_excluded,GD_EFFECT_WORK);
    if (!state->scene || (state->applied.compare>=0 && !gd_query_scene_compare(state->scene,state->applied.primary,state->applied.compare))) goto failure;
    if(!query_place(context,state->scene,(GD_LayoutMode)context->pane->layout_choice)) goto failure;
    model->error_count=errors; model->errors_omitted=omitted; return state;
failure:
    if (model->error_count>errors) snprintf(context->pane->query_message,sizeof(context->pane->query_message),"%s: %s. Last good query retained.",model->errors[errors].code,model->errors[errors].message);
    else if (!*context->pane->query_message) snprintf(context->pane->query_message,sizeof(context->pane->query_message),"Cannot prepare bounded query/drawing; last good query retained.");
    model->error_count=errors; model->errors_omitted=omitted; gd_explorer_query_destroy(state); return NULL;
}
int gd_explorer_query_overview(GD_ExplorerContext *context,int mode) {
    GD_ExplorerPane *pane=context->pane;GD_ExplorerQueryState *state=pane->query_state,*fresh=NULL;GD_QueryScene *scene;int old=pane->actor_overview,record=gd_explorer_query_record(pane),i;
    if(pane->view!=GD_EXPLORER_ACTORS || mode<GD_ACTOR_INTERACTIONS || mode>GD_ACTOR_USAGE || !state) return 0;
    if(state->query && state->query->request.kind==GD_QUERY_BINDINGS) {
        scene=gd_query_scene_overview(state->query,(GD_ActorOverviewMode)mode,GD_EFFECT_WORK);
        if(!scene || !query_place(context,scene,(GD_LayoutMode)pane->layout_choice)) {gd_query_scene_destroy(scene);return 0;}
        gd_query_scene_destroy(state->scene);state->scene=scene;
    } else {
        GD_ExplorerQueryOptions defaults;gd_explorer_form_defaults(GD_FORM_QUERY,&defaults);
        pane->actor_overview=mode;fresh=build_state(context,&defaults,1,NULL);pane->actor_overview=old;
        if(!fresh) return 0;
        gd_explorer_query_destroy(state);pane->query_state=state=fresh;
    }
    pane->actor_overview=mode;pane->layout=&state->scene->layout;pane->selection=(GD_ExplorerSelection){-1,-1,0,-1,-1};
    for(i=0;i<pane->layout->node_count;++i) if(gd_query_scene_record(state->scene,i)==record) pane->selection.node=i;
    if(record>=0 && pane->selection.node<0) pane->selection.linked=record; /* Keep source inspection when definition is absent from new canvas. */
    memset(pane->match_nodes,0,sizeof(pane->match_nodes));memset(pane->match_edges,0,sizeof(pane->match_edges));*pane->find_reference=0;pane->visible_matches=pane->hidden_matches=0;
    pane->fit=1;pane->paused=false;++pane->revision;
    snprintf(pane->query_message,sizeof(pane->query_message),"Actor overview changed. Pending query inputs retained; no analytical Run or source writes.");return 1;
}
/* First canvas uses default applied scope. Staged inputs require explicit Run. */
int gd_explorer_query_prepare(GD_ExplorerContext *context) {
    GD_ExplorerPane *pane=context->pane;
    if (!pane->query_state) {
        GD_ExplorerQueryOptions defaults; int pristine; bool literal_values=pane->query_options.literal_values,literal_assumptions=pane->query_options.literal_assumptions;
        gd_explorer_form_defaults(GD_FORM_QUERY,&defaults); pristine=gd_explorer_form_equal(GD_FORM_QUERY,&pane->query_options,&defaults);
        pane->query_state=build_state(context,&defaults,1,NULL);
        if (pane->query_state && pristine) {
            pane->query_options=pane->query_state->applied;
            pane->query_options.literal_values=literal_values; pane->query_options.literal_assumptions=literal_assumptions;
        }
    }
    if (!pane->query_state) return 0;
    pane->layout=pane->query_state->scene ? &pane->query_state->scene->layout:pane->query_state->empty;
    if (pane->view==GD_EXPLORER_EFFECTS && pane->fit==1) pane->fit=GD_EXPLORER_FIT_QUERY_READABLE;
    return 1;
}
int gd_explorer_query_run(GD_ExplorerContext *context) {
    GD_ExplorerPane *pane=context->pane; GD_ExplorerQueryState *fresh;
    *pane->query_message=0; fresh=build_state(context,&pane->query_options,pane->view==GD_EXPLORER_ACTORS && !pane->query_options.coverage && pane->query_options.root_case<0 && pane->query_options.axis<0,NULL); if (!fresh) return 0;
    if (pane->structure_state && ((pane->structure_state->result && pane->structure_state->result->actor_scene) || (pane->structure_state->picker_context && pane->structure_state->picker_context->actor_scene)) && fresh->query->request.kind!=GD_QUERY_BINDINGS) {
        GD_ExplorerPane candidate=*pane; GD_ExplorerContext next=*context;
        candidate.query_state=fresh; candidate.structure_state=NULL; next.pane=&candidate;
        if (!gd_explorer_structure_remap(&next,context->project,pane)) { snprintf(pane->query_message,sizeof(pane->query_message),"%s",*candidate.query_message ? candidate.query_message:"Cannot prepare dependent structural result; last good query retained."); gd_explorer_query_destroy(fresh); return 0; }
        gd_explorer_structure_destroy(pane->structure_state); pane->structure_state=candidate.structure_state;
    }
    if (fresh->query->request.kind==GD_QUERY_BINDINGS && pane->structure_state && ((pane->structure_state->result && pane->structure_state->result->actor_scene) || (pane->structure_state->picker_context && pane->structure_state->picker_context->actor_scene))) { gd_explorer_structure_destroy(pane->structure_state); pane->structure_state=NULL; }
    gd_explorer_query_destroy(pane->query_state); pane->query_state=fresh; pane->query_options=fresh->applied; pane->layout=&fresh->scene->layout;
    pane->selection=(GD_ExplorerSelection){-1,-1,0,-1,-1}; pane->fit=pane->view==GD_EXPLORER_EFFECTS || fresh->applied.coverage ? GD_EXPLORER_FIT_QUERY_READABLE:1; pane->prepared=1; pane->revision=1;
    snprintf(pane->query_message,sizeof(pane->query_message),"Query applied. Values are assumptions; no source writes or final outcome proof."); return 1;
}
static int remap_record(const GD_Model *old, const GD_Model *fresh, int index) {
    int mapped;
    if (index<0) return -1;
    mapped=gd_explorer_record(fresh,old->records[index].reference);
    if (mapped<0 || old->records[index].role!=fresh->records[mapped].role || (!!old->records[index].id!=!!fresh->records[mapped].id) || (old->records[index].id && strcmp(old->records[index].id,fresh->records[mapped].id))) return -1;
    return mapped;
}
static int remap_axis(const GD_Model *old, const GD_Model *fresh, int index) {
    int mapped=remap_record(old,fresh,index);
    if (mapped<0 || strcmp(old->records[index].axis.type ? old->records[index].axis.type:"",fresh->records[mapped].axis.type ? fresh->records[mapped].axis.type:"") || strcmp(old->records[index].axis.unit ? old->records[index].axis.unit:"",fresh->records[mapped].axis.unit ? fresh->records[mapped].axis.unit:"")) return -1;
    return mapped;
}
int gd_explorer_query_remap(GD_ExplorerContext *context, const GD_ExplorerProject *old, const GD_ExplorerPane *prior) {
    GD_ExplorerPane *pane=context->pane; GD_ExplorerQueryOptions options=prior->query_options; const char *value=NULL,*comparison=NULL;
    const GD_Model *before=gd_explorer_model(old,prior->view); const GD_Model *fresh=gd_explorer_model(context->project,pane->view);
    int i,root=-1,participant=-1;
    if (prior->query_state && !gd_explorer_form_equal(GD_FORM_QUERY,&prior->query_options,&prior->query_state->applied)) { snprintf(pane->query_message,sizeof(pane->query_message),"Run pending query inputs before refreshing; last applied result retained."); return 0; }
    if (options.root_case>=0) {
        const GD_RuleCase *source=&old->rules->cases[options.root_case]; int record=old->rules->sources[source->source].record,mapped=remap_record(before,fresh,record);
        if (mapped<0) return 0;
        for (i=0;i<context->project->rules->case_count;++i) { const GD_RuleCase *candidate=&context->project->rules->cases[i]; if (context->project->rules->sources[candidate->source].record==mapped && !strcmp(source->name,candidate->name)) { if (root>=0) return 0; root=i; } }
        if (root<0) return 0;
        for (i=source->first_participant;i<source->first_participant+source->participant_count;++i) {
            const GD_RuleParticipant *actor=&old->rules->participants[i]; const GD_RuleCase *destination=&context->project->rules->cases[root]; int j,match=-1;
            for (j=destination->first_participant;j<destination->first_participant+destination->participant_count;++j) if (!strcmp(actor->name,context->project->rules->participants[j].name)) { if (match>=0) return 0; match=j; }
            if (match<0 || actor->noun_resolution!=context->project->rules->participants[match].noun_resolution) return 0;
            if (actor->noun_resolution==GD_RULE_RESOLVED) { if (remap_record(before,fresh,actor->noun_record)!=context->project->rules->participants[match].noun_record) return 0; }
            else if (strcmp(actor->noun ? actor->noun:"",context->project->rules->participants[match].noun ? context->project->rules->participants[match].noun:"")) return 0;
        }
        if (options.participant>=0) {
            const GD_RuleCase *candidate=&context->project->rules->cases[root];
            for (i=candidate->first_participant;i<candidate->first_participant+candidate->participant_count;++i) if (!strcmp(old->rules->participants[options.participant].name,context->project->rules->participants[i].name)) { if (participant>=0) return 0; participant=i; }
            if (participant<0) return 0;
        }
    }
    options.root_case=root; options.participant=participant;
    if (options.axis>=0) { options.axis=remap_axis(before,fresh,options.axis); if (options.axis<0) return 0; }
    if (options.noun>=0) { options.noun=remap_record(before,fresh,options.noun); if (options.noun<0) return 0; }
    if (prior->query_state) {
        for (i=0;i<prior->query_state->fixed.fact_count;++i) if (remap_axis(before,fresh,prior->query_state->fixed.facts[i].axis_record)<0) return 0;
        for (i=0;i<prior->query_state->fixed.context_count;++i) if (remap_axis(before,fresh,prior->query_state->fixed.contexts[i].axis)<0) return 0;
    }
    if (prior->query_state && prior->query_state->query && prior->query_state->query->variants && prior->query_state->applied.primary>=0) value=prior->query_state->query->variants->variants[prior->query_state->applied.primary].value;
    if (prior->query_state && prior->query_state->query && prior->query_state->query->variants && prior->query_state->applied.compare>=0) comparison=prior->query_state->query->variants->variants[prior->query_state->applied.compare].value;
    pane->query_options=options; *pane->query_message=0; pane->query_state=build_state(context,&options,1,value); if (!pane->query_state) return 0;
    pane->query_options=pane->query_state->applied; pane->layout=pane->query_state->scene ? &pane->query_state->scene->layout:pane->query_state->empty;
    if (comparison) {
        const GD_ActorQuery *variants=pane->query_state->query->variants;
        if (!variants) return 0;
        for (i=0;i<variants->variant_count;++i) if (!strcmp(comparison,variants->variants[i].value)) break;
        if (i==variants->variant_count || !gd_query_scene_compare(pane->query_state->scene,pane->query_state->applied.primary,i)) return 0;
        pane->query_options.compare=pane->query_state->applied.compare=i;
    }
    if (prior->query_state && prior->query_state->selected) {
        const GD_Query *previous=prior->query_state->query; const GD_EffectNode *goal=&previous->effects->nodes[prior->query_state->selected->selection.target];
        int mapped=gd_effect_node_remap(previous->effects,prior->query_state->selected->selection.target,pane->query_state->query->effects);
        if (goal->axis>=0 && (mapped<0 || remap_axis(before,fresh,goal->axis)!=pane->query_state->query->effects->nodes[mapped].axis)) return 0;
        if (goal->relation>=0) { int source_case=previous->resource->relations[goal->relation].source_case,record=previous->rules->sources[previous->rules->cases[source_case].source].record; if (remap_record(before,fresh,record)<0) return 0; }
        if (mapped<0 || !focus_result(context,pane->query_state,mapped)) return 0;
        pane->layout=&pane->query_state->scene->layout;
    }
    if (prior->query_state && prior->query_state->query && prior->query_state->query->effects && prior->selection.node>=0 && prior->query_state->scene) {
        int source=prior->query_state->scene->nodes[prior->selection.node].source;
        int mapped=gd_effect_node_remap(prior->query_state->query->effects,source,pane->query_state->query->effects);
        int source_record=gd_query_scene_record(prior->query_state->scene,prior->selection.node);
        if (source_record>=0 && remap_record(before,fresh,source_record)<0) mapped=-1;
        if (mapped>=0) for (i=0;i<pane->layout->node_count;++i) if (pane->query_state->scene->nodes[i].source==mapped) pane->selection.node=i;
    }
    if (prior->query_state && prior->query_state->query && prior->query_state->query->request.kind==GD_QUERY_BINDINGS && prior->selection.node>=0) {
        int source=gd_query_scene_record(prior->query_state->scene,prior->selection.node),mapped=remap_record(before,fresh,source);
        if (mapped>=0) for (i=0;i<pane->layout->node_count;++i) if (gd_query_scene_record(pane->query_state->scene,i)==mapped) pane->selection.node=i;
        snprintf(pane->query_message,sizeof(pane->query_message),"Actor overview reloaded; exact selected definition %s.",pane->selection.node>=0 ? "retained":"cleared (missing/replaced)"); return 1;
    }
    if (prior->view==GD_EXPLORER_ACTORS && prior->selection.node>=0) snprintf(pane->query_message,sizeof(pane->query_message),pane->query_options.coverage ? "Coverage selectors retained; native coverage recomputed; transient inspection cleared after reload.":"Root, binding, axis, primary value and facts retained; transient inspection cleared after reload.");
    return 1;
}
int gd_explorer_query_record(const GD_ExplorerPane *pane) {
    if (pane->selection.linked>=0) return pane->selection.linked;
    if(pane->query_state && pane->query_state->scene && pane->selection.cell>=0 && pane->selection.cell<pane->query_state->scene->layout.edge_count) {
        const GD_QueryScene *scene=pane->query_state->scene; const GD_QuerySceneEdge *edge=&scene->edges[pane->selection.cell]; int source_case=-1;
        if(scene->overview) return edge->kind==GD_QUERY_EDGE_AXIS ? scene->query->model->edges[edge->source].source:scene->query->rules->sources[scene->query->rules->cases[edge->source].source].record;
        if(scene->query->effects) source_case=scene->query->resource->relations[scene->query->effects->links[edge->source].relation].source_case;
        else if(edge->kind==GD_QUERY_EDGE_EFFECT && scene->query->variants) source_case=scene->query->variants->writes[edge->source].origin_case;
        if(source_case>=0) return scene->query->rules->sources[scene->query->rules->cases[source_case].source].record;
    }
    return pane->query_state && pane->query_state->scene ? gd_query_scene_record(pane->query_state->scene,pane->selection.node):-1;
}
int gd_explorer_query_compare(GD_ExplorerContext *context) {
    GD_ExplorerQueryState *state=context->pane->query_state;
    if(!state || !state->query || !state->query->variants || state->applied.primary<0 || state->applied.compare<0) return 0;
    state->comparison_first=state->applied.primary; state->comparison_second=state->applied.compare;
    state->comparison_ready=gd_actor_compare(state->query->variants,state->comparison_first,state->comparison_second,&state->comparison); return state->comparison_ready;
}
int gd_explorer_query_display(GD_ExplorerContext *context) {
    GD_ExplorerPane *pane=context->pane; GD_ExplorerQueryState *state=pane->query_state; GD_QueryScene *fresh; int i,kind=-1,source=-1,variant=-1;
    if (!state || !state->query) return 0;
    if(state->scene && state->scene->overview) return gd_explorer_query_overview(context,pane->actor_overview);
    fresh=state->selected ? gd_query_scene_effect_context(state->query,state->selected,pane->query_options.weighted,GD_EFFECT_WORK):gd_query_scene_build(state->query,(GD_QueryGhostMode)pane->query_options.ghosts,pane->query_options.primary,pane->query_options.weighted,pane->query_options.include_excluded,GD_EFFECT_WORK);
    if (fresh && pane->query_options.compare>=0 && !gd_query_scene_compare(fresh,pane->query_options.primary,pane->query_options.compare)) { gd_query_scene_destroy(fresh); fresh=NULL; }
    if(fresh && !query_place(context,fresh,(GD_LayoutMode)pane->layout_choice)) {gd_query_scene_destroy(fresh);fresh=NULL;}
    if (!fresh) { snprintf(pane->query_message,sizeof(pane->query_message),"Cannot allocate display candidate; previous drawing retained."); return 0; }
    if (pane->selection.node>=0 && pane->selection.node<state->scene->layout.node_count) { const GD_QuerySceneNode *node=&state->scene->nodes[pane->selection.node]; kind=node->kind; source=node->source; variant=node->variant; }
    gd_query_scene_destroy(state->scene); state->scene=fresh; pane->layout=&fresh->layout; pane->selection=(GD_ExplorerSelection){-1,-1,0,-1,-1};
    for (i=0;i<fresh->layout.node_count;++i) if ((int)fresh->nodes[i].kind==kind && fresh->nodes[i].source==source && fresh->nodes[i].variant==variant) pane->selection.node=i;
    state->applied.ghosts=pane->query_options.ghosts; state->applied.primary=pane->query_options.primary; state->applied.compare=pane->query_options.compare; state->applied.weighted=pane->query_options.weighted; state->applied.include_excluded=pane->query_options.include_excluded;
    pane->fit=pane->view==GD_EXPLORER_EFFECTS || state->applied.coverage ? GD_EXPLORER_FIT_QUERY_READABLE:1; return 1;
}
int gd_explorer_query_select_result(GD_ExplorerContext *context, int target) {
    GD_ExplorerPane *pane=context->pane; GD_ExplorerQueryState *state=pane->query_state; int i;
    if (!state || !state->query || !state->query->effects || target<0 || target>=state->query->effects->node_count || state->query->effects->nodes[target].kind==GD_EFFECT_ACTIVATION || !focus_result(context,state,target)) return 0;
    pane->layout=&state->scene->layout; pane->selection=(GD_ExplorerSelection){-1,-1,0,-1,-1};
    for (i=0;i<pane->layout->node_count;++i) if (state->scene->nodes[i].source==target) pane->selection.node=i;
    pane->fit=GD_EXPLORER_FIT_QUERY_READABLE; return 1;
}
static void coverage_summary(const GD_Query *query) {
    const GD_StateCoverage *coverage=query->state_coverage; const GD_Record *axis=&query->model->records[query->request.axis];
    igTextWrapped("Applied coverage axis: %s | %s | unit %s",axis->reference,axis->axis.type,axis->axis.unit && *axis->axis.unit ? axis->axis.unit:"unspecified");
    igTextWrapped("Applied root statuses: %s",query->request.status_count ? query->request.statuses[0]:"all");
    igTextWrapped("Bounds: %s .. %s | declared values %d | closed %s",axis->axis.minimum ? axis->axis.minimum:"unspecified",axis->axis.maximum ? axis->axis.maximum:"unspecified",axis->axis.allowed_count,axis->axis_closed ? "yes":"no");
    igTextWrapped("Applied object: %s | case: %s | local participant: %s",query->request.noun>=0 ? query->model->records[query->request.noun].display_name:"all",query->request.case_index>=0 ? query->rules->cases[query->request.case_index].name:"all",query->request.participant ? query->request.participant:"all explicit bindings (not default A)");
    igTextWrapped("%d bindings; %d evaluated; %d unexamined; %d excluded roots",query->binding_count,coverage->evaluated_bindings,query->unexamined_bindings,query->excluded_roots);
    igTextWrapped("%d finite values | inventory work %d | hypothesis work %d | query %s | source %s",coverage->value_count,query->work,coverage->work,query->limited || coverage->limited ? "unfinished":"checked",coverage->source_incomplete ? "incomplete":"checked");
    igTextWrapped("Cards separate deduplicated case counts from local-binding counts. Unfinished cases may overlap known memberships. Unknown guards remain candidates. No retained candidate is nullable checked absence, not a defect or universal gameplay proof. Numeric values are finite samples, not a range.");
}
void gd_explorer_query_controls(GD_ExplorerContext *context) {
    GD_ExplorerPane *pane=context->pane; GD_ExplorerQueryOptions *options=&pane->query_options; const GD_Rules *rules=context->project->rules; GD_Model *model=gd_explorer_model(context->project,pane->view); int i;
    igTextWrapped("Native query scope, not camera bounds. Local binding identities stay distinct. Whole guards/side effects remain source-backed; no executable recipe or final-outcome proof.");
    if (pane->view==GD_EXPLORER_ACTORS) {
        int inventory=pane->query_state && pane->query_state->query && pane->query_state->query->request.kind==GD_QUERY_BINDINGS;
        if (inventory) {
            const GD_QueryScene *scene=pane->query_state->scene; int selected=gd_explorer_query_record(pane);
            if (selected>=0 && model->records[selected].role!=GD_NOUN) selected=-1;
            igTextWrapped("Actor overview: explicit is_actor:true only on perimeter/map. %d unclassified nouns are not actors by default. Scope %s; missing connections are not proof of absence.",gd_query_scene_overview_unclassified(scene),gd_query_scene_overview_incomplete(scene) ? "incomplete":"loaded declarations");
            igTextWrapped(pane->actor_overview==GD_ACTOR_USAGE ? "Used noun concepts and own declared axes. Concept size: distinct connected actors. Unclassified noun concepts labelled separately. Select nodes to highlight usages. Native keeps fixed actor ring; Free lets all actors and concepts move with shared settling and dragging.":"Links count distinct interaction records sharing actor types within a case. Thicker links have stronger bounded attraction. Not direct contact, runtime frequency or applicability proof.");
            if (gd_explorer_picker_begin(context,"Browse object / actor",selected>=0 ? model->records[selected].display_name:NULL)) {
                for (i=0;i<model->record_count;++i) if(model->records[i].role==GD_NOUN) { if (gd_explorer_picker_record_item(context,&model->records[i],TextFormat("%s | %s",model->records[i].display_name,model->records[i].reference),i==selected)) { int n;pane->selection=(GD_ExplorerSelection){-1,-1,0,i,-1};for(n=0;n<scene->layout.node_count;++n) if(scene->nodes[n].source==i) {pane->selection.node=n;pane->selection.linked=-1;break;} } }
                gd_explorer_picker_end();
            }
            if (selected>=0) {
                const GD_Record *record=&model->records[selected];
                igTextWrapped("Actor classification: %s. Classification does not restrict noun queries.",record->is_actor_line ? (record->is_actor ? "actor":"non-actor"):"unclassified");
            }
            igTextWrapped("Browsing does not change pending analysis. Choose a case/local occurrence explicitly to set analysis context.");
            if (options->root_case>=0) igTextWrapped("Pending analysis: %s / %s / %s",rules->cases[options->root_case].name,options->participant>=0 ? rules->participants[options->participant].name:"no binding",options->axis>=0 ? model->records[options->axis].name:"choose axis");
            if (selected>=0 && gd_explorer_picker_begin(context,"Inspect states in case / local binding",options->participant>=0 && rules->participants[options->participant].noun_resolution==GD_RULE_RESOLVED && rules->participants[options->participant].noun_record==selected ? TextFormat("%s / %s",rules->cases[options->root_case].name,rules->participants[options->participant].name):NULL)) {
                const GD_Query *query=pane->query_state->query;
                for (i=0;i<query->binding_count;++i) { const GD_QueryBinding *binding=&query->bindings[i]; const GD_RuleParticipant *participant=&rules->participants[binding->participant]; const GD_RuleCase *item=&rules->cases[binding->case_index]; if (participant->noun_resolution==GD_RULE_RESOLVED && participant->noun_record==selected) { igPushID_Int(i); if (gd_explorer_picker_item(context,TextFormat("%s / %s | %s",item->name,participant->name,model->records[rules->sources[item->source].record].reference),options->participant==binding->participant,0)) { gd_explorer_query_root(options,rules,binding->case_index); gd_explorer_query_participant(options,rules,binding->participant); } igPopID(); } }
                gd_explorer_picker_end();
            }
        } else if (igButton("Return to actor overviews (keep query inputs)",(ImVec2_c){0,0})) {
            gd_explorer_query_overview(context,pane->actor_overview); return;
        }
        if (gd_explorer_form_widget(GD_FORM_QUERY,options,"coverage",NULL)) gd_explorer_query_question(options,options->coverage);
        igTextWrapped("Changing question resets pending selectors/values/facts, not drawn result. Run to replace result.");
        const GD_RuleCase *root=options->root_case>=0 ? &rules->cases[options->root_case]:NULL;
        if (options->coverage) {
            igTextWrapped("Each binding independently receives each finite value. Other bindings/context stay unknown; no same-noun simultaneous assignment or default A.");
            if (gd_explorer_picker_begin(context,"Object filter",options->noun>=0 ? model->records[options->noun].display_name:"All explicitly declared nouns")) {
                if (gd_explorer_picker_item(context,"All explicitly declared nouns",options->noun<0,0)) options->noun=-1;
                for (i=0;i<model->record_count;++i) if (model->records[i].role==GD_NOUN && gd_explorer_picker_record_item(context,&model->records[i],model->records[i].display_name,i==options->noun)) options->noun=i;
                gd_explorer_picker_end();
            }
        }
        if ((!inventory || options->coverage) && gd_explorer_picker_begin(context,"Root case",root ? TextFormat("%s#%s",model->records[rules->sources[root->source].record].reference,root->name):options->coverage ? "All loaded root cases":"Choose source case")) {
            if (options->coverage && gd_explorer_picker_item(context,"All loaded root cases",options->root_case<0,0)) options->root_case=options->participant=-1;
            for (i=0;i<rules->case_count;++i) { const GD_RuleCase *candidate=&rules->cases[i]; igPushID_Int(i); if (gd_explorer_picker_item(context,TextFormat("%s#%s",model->records[rules->sources[candidate->source].record].reference,candidate->name),i==options->root_case,0)) { gd_explorer_query_root(options,rules,i); } igPopID(); }
            gd_explorer_picker_end();
        }
        root=options->root_case>=0 ? &rules->cases[options->root_case]:NULL;
        if (root && !inventory && gd_explorer_picker_begin(context,"Local participant",options->participant>=0 ? rules->participants[options->participant].name:options->coverage ? "All explicit local bindings":"A unspecified; choose declared binding")) {
            if (options->coverage && gd_explorer_picker_item(context,"All explicit local bindings",options->participant<0,0)) options->participant=-1;
            for (i=root->first_participant;i<root->first_participant+root->participant_count;++i) if (gd_explorer_picker_item(context,TextFormat("%s%s",rules->participants[i].name,i==root->primary_participant ? " [authored A]":""),i==options->participant,0)) { gd_explorer_query_participant(options,rules,i); }
            gd_explorer_picker_end();
        }
        if (options->coverage && gd_explorer_picker_begin(context,"Typed axis",options->axis>=0 ? model->records[options->axis].reference:NULL)) {
            for (i=0;i<model->record_count;++i) if (model->records[i].role==GD_AXIS && gd_explorer_picker_item(context,model->records[i].reference,i==options->axis,0)) gd_explorer_query_axis(options,rules,i);
            gd_explorer_picker_end();
        }
        if (!options->coverage && options->participant>=0 && rules->participants[options->participant].noun_resolution==GD_RULE_RESOLVED && gd_explorer_picker_begin(context,"Typed axis",options->axis>=0 ? model->records[options->axis].reference:NULL)) {
            int edge; for (edge=rules->actor_first[rules->participants[options->participant].noun_record];edge>=0;edge=rules->next_axis[edge]) if (rules->axis_resolution[edge]==GD_RULE_RESOLVED) { int axis=model->edges[edge].target; if (gd_explorer_picker_item(context,model->records[axis].reference,axis==options->axis,0)) gd_explorer_query_axis(options,rules,axis); } gd_explorer_picker_end();
        }
        if (options->coverage) igTextWrapped("Blank values use declared enums; numeric axes need explicit finite samples. Each binding is an independent hypothesis, not a simultaneous world state.");
        if (options->axis>=0 && rules->axes[options->axis].type==GD_AXIS_ENUM && gd_explorer_picker_begin(context,"Compare finite values",*options->values ? options->values:"All declared values")) {
            const GD_ValueDefinition *axis=&model->records[options->axis].axis; const char *value=axis->allowed;
            if (gd_explorer_picker_item(context,"All declared values",!*options->values,1)) { *options->values=0; options->literal_values=true; }
            for (i=0;i<axis->allowed_count;++i,value+=strlen(value)+1) if (gd_explorer_picker_item(context,value,gd_picker_contains(options->values,value),1)) {
                if (!options->literal_values && *options->values) snprintf(pane->query_message,sizeof(pane->query_message),"Clear raw samples before using value picker; prior inputs retained.");
                else { options->literal_values=true; gd_explorer_picker_toggle(context,options->values,sizeof(options->values),value,GD_ACTOR_VARIANTS); }
            }
            gd_explorer_picker_end();
        }
        if (options->axis>=0 && (rules->axes[options->axis].type==GD_AXIS_NUMBER || rules->axes[options->axis].type==GD_AXIS_INTEGER)) {
            const GD_ValueDefinition *axis=&model->records[options->axis].axis;
            igTextWrapped("Sample type %s | unit %s | declared bounds %s .. %s",axis->type,axis->unit && *axis->unit ? axis->unit:"unspecified",axis->minimum ? axis->minimum:"unspecified",axis->maximum ? axis->maximum:"unspecified");
            char value[64]; double *sample=&pane->assumption_form.sample; igInputDouble("Finite numeric sample",sample,0,0,"%.17g",0);
            if (igButton("Add numeric sample",(ImVec2_c){0,0})) { if (!isfinite(*sample) || (rules->axes[options->axis].type==GD_AXIS_INTEGER && floor(*sample)!=*sample)) snprintf(pane->query_message,sizeof(pane->query_message),"Sample must be finite and match declared integer/number type."); else if (!options->literal_values && *options->values) snprintf(pane->query_message,sizeof(pane->query_message),"Clear raw samples before using sample builder."); else { options->literal_values=true; snprintf(value,sizeof(value),"%.17g",*sample); if (!gd_picker_contains(options->values,value)) gd_explorer_picker_toggle(context,options->values,sizeof(options->values),value,GD_ACTOR_VARIANTS); } }
        }
        gd_explorer_picker_lines(context,"Selected finite samples (blank: declared values)",options->values,sizeof(options->values));
        if (igTreeNode_Str("Advanced: raw finite samples")) { gd_explorer_form_widget(GD_FORM_QUERY,options,"literal_values",NULL); gd_explorer_form_widget(GD_FORM_QUERY,options,"values",NULL); igTreePop(); }
        if (!options->coverage) gd_explorer_picker_assumptions(context,options->root_case,options->facts,sizeof(options->facts),options->contexts,sizeof(options->contexts),&options->literal_assumptions);
        else if (*options->facts || *options->contexts) { igTextWrapped("Fixed assumptions are incompatible with independent coverage. Clear them explicitly before Run."); gd_explorer_picker_lines(context,"Fixed state assumptions",options->facts,sizeof(options->facts)); gd_explorer_picker_lines(context,"Fixed context assumptions",options->contexts,sizeof(options->contexts)); }
    }
    gd_explorer_form_widget(GD_FORM_QUERY,options,"accepted",pane->view==GD_EXPLORER_ACTORS ? (options->coverage ? "Accepted root cases only":"Accepted root only (dependencies stay labelled)"):"Accepted sources at every placement ancestor");
    gd_explorer_action_button(context,GD_ACTION_QUERY_RUN);
    if (pane->query_state && !gd_explorer_form_equal(GD_FORM_QUERY,options,&pane->query_state->applied)) igTextWrapped("Pending inputs differ from applied result. Run before refresh; last good result remains visible.");
    if(pane->query_state && pane->query_state->query && pane->query_state->query->state_coverage)coverage_summary(pane->query_state->query);
    if (*pane->query_message) igTextWrapped("%s",pane->query_message);
}
void gd_explorer_query_display_controls(GD_ExplorerContext *context) {
    GD_ExplorerPane *pane=context->pane;GD_ExplorerQueryOptions *options=&pane->query_options;GD_Model *model=gd_explorer_model(context->project,pane->view);int i,display=0;
    if (pane->query_state && pane->query_state->query) {
        GD_Query *query=pane->query_state->query;
        if (query->variants) {
            display|=gd_explorer_form_widget(GD_FORM_QUERY,options,"ghosts",NULL);
            if (query->variants->variant_count && gd_explorer_picker_begin(context,"Primary state",options->primary>=0 && options->primary<query->variants->variant_count ? query->variants->variants[options->primary].value:NULL)) { for (i=0;i<query->variants->variant_count;++i) if (gd_explorer_picker_item(context,query->variants->variants[i].value,i==options->primary,0)) { options->primary=i; display=1; } gd_explorer_picker_end(); }
            display|=gd_explorer_form_widget(GD_FORM_QUERY,options,"include_excluded",NULL);
            igText("%d/%d retained variants | work %d | %s",query->variants->variant_count,query->variants->requested,query->variants->work,query->variants->limited ? "LIMITED":"bounded check");
            if (query->variants->variant_count && gd_explorer_picker_begin(context,"Compare state",options->compare>=0 && options->compare<query->variants->variant_count ? query->variants->variants[options->compare].value:"No comparison")) { if (gd_explorer_picker_item(context,"No comparison",options->compare<0,0)) { options->compare=-1; display=1; } for (i=0;i<query->variants->variant_count;++i) if (gd_explorer_picker_item(context,query->variants->variants[i].value,i==options->compare,0)) { options->compare=i; display=1; } gd_explorer_picker_end(); }
            if (pane->query_state->applied.compare>=0) {
                GD_ActorComparison value; int first=pane->query_state->applied.primary,second=pane->query_state->applied.compare;
                gd_actor_compare(query->variants,first,second,&value);
                igTextWrapped("%s vs %s | %s. Green shared, blue first only, pink second only, muted neither. Retained not-excluded sets, not final equivalence or compatible placements.",query->variants->variants[first].value,query->variants->variants[second].value,value.complete ? "closed retained scope":"incomplete scope");
                igTextWrapped("Cases shared/first/second %d/%d/%d; partners %d/%d/%d; concepts %d/%d/%d.",value.cases.shared,value.cases.first_only,value.cases.second_only,value.partners.shared,value.partners.first_only,value.partners.second_only,value.effects.shared,value.effects.first_only,value.effects.second_only);
            }
        } else if (query->contributors) {
            int selected=pane->query_state->selected ? pane->query_state->selected->selection.target:-1;
            if (gd_explorer_picker_begin(context,"Result / contributor context",selected>=0 ? TextFormat("%s %s%s%s",query->effects->nodes[selected].axis>=0 ? model->records[query->effects->nodes[selected].axis].reference:"unresolved",gd_effect_kind_name(query->effects->nodes[selected].kind),query->effects->nodes[selected].value ? ": ":"",query->effects->nodes[selected].value ? query->effects->nodes[selected].value:""):"No retained result")) {
                for (i=0;i<query->effects->node_count;++i) if (query->effects->nodes[i].kind!=GD_EFFECT_ACTIVATION) { const GD_EffectNode *node=&query->effects->nodes[i]; igPushID_Int(i); if (gd_explorer_picker_item(context,TextFormat("%s %s%s%s | %d direct | #%d",node->axis>=0 ? model->records[node->axis].reference:"unresolved",gd_effect_kind_name(node->kind),node->value ? ": ":"",node->value ? node->value:"",query->contributors->incoming[i],i),i==selected,0)) gd_explorer_query_select_result(context,i); igPopID(); } gd_explorer_picker_end();
            }
            if (igSmallButton("Frame selected result / readable")) pane->fit=GD_EXPLORER_FIT_QUERY_READABLE;
            igTextWrapped("Initial framing keeps labels readable; pan/zoom for full context. F fits all, possibly hiding small labels. Camera never changes query scope.");
            if (pane->query_state->selected) igTextWrapped("%d direct target contributors. Drawing includes exact ancestors and all their direct side effects. Initial result uses largest retained direct count, not a quality ranking; choose any result above. Wrapped rows are display only, not time.",pane->query_state->selected->count);
            display|=gd_explorer_form_widget(GD_FORM_QUERY,options,"weighted",NULL);
            igTextWrapped("Counts: direct target-labelled declarations only; calls/context copies/ghosts excluded. Dotted calls are mapped instantiation, not temporal steps.");
            igText("%d contributors | source incomplete %s | query limit %s",query->contributors->count,query->contributors->source_incomplete ? "yes":"no",query->contributors->limited ? "yes":"no");
        }
        if (!query->variants && query->request.kind==GD_QUERY_ACTOR_STATES) igTextWrapped("Root excluded by status scope; no variant facts evaluated. Change inputs and Run to inspect.");
        if (igSmallButton(gd_explorer_action_info(GD_ACTION_QUERY_DISPLAY)->label)) display=1;
        if (display) gd_explorer_action_run(context,GD_ACTION_QUERY_DISPLAY);
    }
    if (*pane->query_message) igTextWrapped("%s",pane->query_message);
}
static Color node_color(const GD_QuerySceneNode *node) {
    Color color=node->kind==GD_QUERY_NODE_EFFECT ? (Color){90,190,170,255}:node->kind==GD_QUERY_NODE_VARIANT ? (Color){139,160,232,255}:node->kind==GD_QUERY_NODE_ACTIVATION ? (Color){218,159,105,255}:(Color){125,171,215,255};
    if (node->applicability==GD_RULE_TRUTH_FALSE) color=(Color){125,132,145,255}; if (node->ghost) color.a=85; return color;
}
const char *gd_explorer_query_node_label(const GD_ExplorerContext *context,int node) {
    const GD_ExplorerQueryState *state=context->pane->query_state;const GD_QuerySceneNode *metadata;
    if(!state || !state->scene || node<0 || node>=state->scene->layout.node_count)return "";
    metadata=&state->scene->nodes[node];
    return metadata->ghost && metadata->kind==GD_QUERY_NODE_VARIANT ? state->query->variants->variants[metadata->variant].value:context->pane->layout->nodes[node].label;
}
void gd_explorer_query_draw(GD_ExplorerContext *context) {
    GD_ExplorerPane *pane=context->pane; GD_ExplorerQueryState *state=pane->query_state; GD_QueryScene *scene=state ? state->scene:NULL;GD_Layout *layout=gd_explorer_display_layout(context); int i,pass;unsigned char highlighted[GD_MAX_RECORDS]={0};
    if(scene && scene->overview && pane->selection.node>=0) {highlighted[pane->selection.node]=1;for(i=0;i<scene->layout.edge_count;++i) {const GD_LayoutEdge *e=&scene->layout.edges[i];if(e->a==pane->selection.node || e->b==pane->selection.node) highlighted[e->a]=highlighted[e->b]=1;}}
    if (!scene) { DrawText(pane->query_options.coverage ? "Choose typed axis and finite values; apply coverage. Case/object optional.":"Choose source case, local participant and typed axis; apply query.",(int)context->viewport.x+24,(int)context->viewport.y+24,18,(Color){170,188,209,255}); return; }
    if(scene->overview && !scene->layout.node_count) {DrawText("No retained actors. Explicit noun is_actor: true is required; review scope and limits.",(int)context->viewport.x+24,(int)context->viewport.y+24,18,(Color){170,188,209,255});return;}
    BeginScissorMode((int)context->viewport.x,(int)context->viewport.y,(int)context->viewport.width,(int)context->viewport.height); BeginMode2D(pane->camera);
    for (i=0;i<layout->edge_count;++i) {
        const GD_LayoutEdge *edge=&layout->edges[i]; const GD_QuerySceneEdge *metadata=&scene->edges[i]; GD_LayoutRoute route; int j,marked=state->selected && state->selected->highlighted[metadata->source];
        float thickness=1.5f;
        if(scene->overview) {marked=pane->selection.cell==i || pane->selection.node==edge->a || pane->selection.node==edge->b;thickness=1.5f+fminf(5,log2f((float)gd_query_scene_overview_count(scene,i)));}
        Color color=marked ? (Color){65,227,195,255}:metadata->kind==GD_QUERY_EDGE_INVOCATION || metadata->kind==GD_QUERY_EDGE_FRONTIER ? (Color){180,160,222,160}:(Color){120,151,175,110};
        if(scene->overview && pane->selection.node>=0 && !marked) color.a=35;
        gd_layout_route(layout,i,&route);
        for (j=1;j<route.count;++j) {
            Vector2 a={route.points[j-1].x,route.points[j-1].y},b={route.points[j].x,route.points[j].y};
            if (metadata->kind==GD_QUERY_EDGE_INVOCATION || metadata->kind==GD_QUERY_EDGE_FRONTIER) { int segment; for (segment=0;segment<12;segment+=2) DrawLineEx((Vector2){a.x+(b.x-a.x)*segment/12,a.y+(b.y-a.y)*segment/12},(Vector2){a.x+(b.x-a.x)*(segment+1)/12,a.y+(b.y-a.y)*(segment+1)/12},(marked ? 3:1.5f)/pane->camera.zoom,color); }
            else DrawLineEx(a,b,(marked ? thickness+1.5f:thickness)/pane->camera.zoom,color);
        }
        if(scene->overview && metadata->kind!=GD_QUERY_EDGE_AXIS && pane->camera.zoom*14>=8 && (marked || gd_query_scene_overview_mode(scene)==GD_ACTOR_INTERACTIONS)) {
            const GD_LayoutNode *a=&layout->nodes[edge->a],*b=&layout->nodes[edge->b];
            DrawText(TextFormat("%d records",gd_query_scene_overview_count(scene,i)),(int)((a->x+b->x)/2),(int)((a->y+b->y)/2),14,(Color){210,224,230,255});
        }
        if (route.count>1 && (metadata->kind==GD_QUERY_EDGE_EFFECT || metadata->kind==GD_QUERY_EDGE_INVOCATION)) {
            Vector2 a={route.points[route.count-2].x,route.points[route.count-2].y},b={route.points[route.count-1].x,route.points[route.count-1].y}; float dx=b.x-a.x,dy=b.y-a.y,length=hypotf(dx,dy),size=7/pane->camera.zoom;
            if (length>2*size) { Vector2 tip={a.x+dx*0.7f,a.y+dy*0.7f}; dx/=length; dy/=length; DrawTriangle(tip,(Vector2){tip.x-dx*size-dy*size/2,tip.y-dy*size+dx*size/2},(Vector2){tip.x-dx*size+dy*size/2,tip.y-dy*size-dx*size/2},color); }
        }
        if (metadata->kind==GD_QUERY_EDGE_EFFECT && pane->camera.zoom*14>=8) {
            const char *target=NULL; const GD_LayoutNode *a=&layout->nodes[edge->a],*b=&layout->nodes[edge->b];
            if (state->query->effects) { int actor=state->query->effects->links[metadata->source].participant; if (actor>=0) target=context->project->rules->participants[actor].name; }
            else { int actor=state->query->variants->writes[metadata->source].actor; if (actor>=0) target=context->project->rules->participants[actor].name; }
            if (target) {
                const char *expression=NULL,*unit=NULL;
                if (state->query->effects) { const GD_ResourceRelation *declaration=&state->query->resource->relations[state->query->effects->links[metadata->source].relation]; if (declaration->kind==GD_RESOURCE_QUANTITY_WRITE) { const GD_RuleAssignment *assignment=&context->project->rules->assignments[declaration->declaration]; expression=assignment->expression; if (assignment->target.axis_record>=0) unit=state->query->model->records[assignment->target.axis_record].axis.unit; } }
                else { const GD_RuleWrite *write=&state->query->variants->writes[metadata->source]; if (write->kind==GD_WRITE_QUANTITY) { expression=context->project->rules->assignments[write->declaration].expression; if (write->axis>=0) unit=state->query->model->records[write->axis].axis.unit; } }
                DrawText(expression && (pane->selection.cell==i || pane->selection.node==edge->a) ? TextFormat("%.24s: %.40s [%s]",target,expression,unit ? unit:"unit unknown"):target,(int)(a->x+(b->x-a->x)*0.12f),(int)(a->y+a->height/2+8+(b->y-a->y)*0.06f),14,(Color){222,224,204,255});
            }
        }
    }
    for (pass=0;pass<2;++pass) for (i=0;i<layout->node_count;++i) {
        const GD_LayoutNode *node=&layout->nodes[i]; const GD_QuerySceneNode *metadata=&scene->nodes[i]; Color color=node_color(metadata); Rectangle card={node->x-node->width/2,node->y-node->height/2,node->width,node->height};
        if ((pass==0)!=!!metadata->ghost) continue;
        if(scene->overview) {const GD_Record *record=&scene->query->model->records[metadata->source];color=record->role==GD_AXIS ? (Color){230,180,100,255}:!record->is_actor_line ? (Color){190,140,220,255}:record->is_actor ? (Color){125,171,215,255}:(Color){90,190,170,255};if(pane->selection.node>=0 && !highlighted[i]) color.a=90;}
        if (scene->query->variants && state->applied.compare>=0) { color=metadata->comparison==3 ? (Color){90,220,160,255}:metadata->comparison==1 ? (Color){105,170,245,255}:metadata->comparison==2 ? (Color){235,135,200,255}:(Color){104,112,128,100}; if (metadata->ghost && metadata->variant!=state->applied.compare) color.a=85; }
        DrawRectangleRounded(card,0.14f,5,(Color){27,37,53,metadata->ghost ? 120:245}); DrawRectangleRoundedLinesEx(card,0.14f,5,(i==pane->selection.node ? 3:1.5f)/pane->camera.zoom,i==pane->selection.node ? (Color){255,218,120,255}:color);
        if (pane->camera.zoom*16>=8) { Vector2 screen=GetWorldToScreen2D((Vector2){card.x+8,card.y+6},pane->camera); Rectangle clip=GetCollisionRec((Rectangle){screen.x,screen.y,(card.width-16)*pane->camera.zoom,(card.height-12)*pane->camera.zoom},context->viewport); if (clip.width<=0 || clip.height<=0) continue; BeginScissorMode((int)clip.x,(int)clip.y,(int)clip.width,(int)clip.height); int record=gd_query_scene_record(scene,i);if(metadata->kind==GD_QUERY_NODE_BINDING || metadata->kind==GD_QUERY_NODE_PARTNER){const GD_RuleParticipant *binding=&scene->query->rules->participants[metadata->source];record=binding->noun_resolution==GD_RULE_RESOLVED ? binding->noun_record:-1;}gd_noun_draw_text(record>=0 ? &scene->query->model->records[record]:NULL,gd_explorer_query_node_label(context,i),(Vector2){card.x+10,card.y+(metadata->ghost && metadata->kind==GD_QUERY_NODE_VARIANT ? card.height-22:9)},16,1,color); if(scene->overview && metadata->incoming) DrawText(TextFormat("%d connected actors",metadata->incoming),(int)card.x+10,(int)(card.y+card.height-23),14,color); if (state->query->effects && metadata->kind==GD_QUERY_NODE_EFFECT) DrawText(TextFormat("%d direct declarations%s",metadata->incoming,state->query->contributors->limited ? " (partial)":""),(int)card.x+10,(int)(card.y+card.height-23),14,color); EndScissorMode(); BeginScissorMode((int)context->viewport.x,(int)context->viewport.y,(int)context->viewport.width,(int)context->viewport.height); }
    }
    EndMode2D(); EndScissorMode();
    if (pane->camera.zoom*16<8) DrawText("Labels hidden below 8 screen pixels; zoom to inspect.",(int)context->viewport.x+12,(int)context->viewport.y+12,15,(Color){185,200,219,255});
}
void gd_explorer_query_input(GD_ExplorerContext *context) {
    GD_ExplorerPane *pane=context->pane; Vector2 mouse=GetMousePosition(),world; int i;
    if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) pane->selection.held=-1;
    if (!CheckCollisionPointRec(mouse,context->viewport) || igGetIO_Nil()->WantCaptureMouse) { pane->selection.held=-1; return; }
    world=GetScreenToWorld2D(mouse,pane->camera);
    if (GetMouseWheelMove()) { Vector2 after; pane->camera.zoom=fmaxf(0.002f,fminf(4,pane->camera.zoom*powf(1.15f,GetMouseWheelMove()))); after=GetScreenToWorld2D(mouse,pane->camera); pane->camera.target.x+=world.x-after.x; pane->camera.target.y+=world.y-after.y; }
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) || IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) { Vector2 delta=GetMouseDelta(); pane->camera.target.x-=delta.x/pane->camera.zoom; pane->camera.target.y-=delta.y/pane->camera.zoom; }
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        const GD_Layout *shown=gd_explorer_display_layout(context);int hit=gd_query_scene_hit_layout(pane->query_state->scene,shown,world.x,world.y);
        if (hit>=0) {
            pane->selection=(GD_ExplorerSelection){-1,hit,0,-1,hit};
            gd_layout_pickup_node(pane->layout,hit,world.x,world.y);
            if (pane->query_state && pane->query_state->query && pane->query_state->query->effects) {
                GD_ExplorerQueryState *state=pane->query_state; int source=state->scene->nodes[hit].source;
                if (state->query->effects->nodes[source].kind!=GD_EFFECT_ACTIVATION && !gd_explorer_query_select_result(context,source)) snprintf(pane->query_message,sizeof(pane->query_message),"Cannot allocate contributor selection/drawing; previous result retained.");
            }
        } else for (i=0;i<pane->layout->edge_count;++i) if (gd_layout_route_distance(shown,i,world.x,world.y)<=7/pane->camera.zoom) { pane->selection=(GD_ExplorerSelection){i,-1,0,-1,-1}; break; }
    }
    if (pane->selection.held>=0 && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) gd_query_scene_drag(pane->query_state->scene,pane->selection.held,world.x,world.y);
}
static void case_details(GD_ExplorerContext *context, int index) {
    const GD_Rules *rules=context->project->rules; const GD_RuleCase *item=&rules->cases[index]; const GD_Record *record=&rules->model->records[rules->sources[item->source].record]; int i;
    igTextWrapped("%s | status %s | %s",record->reference,record->status ? record->status:"unspecified",record->source_path);
    igTextWrapped("Case %s | line %d | authored A %s | context %s",item->name,item->line,item->primary_target ? item->primary_target:"unspecified",item->context ? item->context:"unspecified");
    if (item->unknown_conditions) igTextWrapped("Unknown conditions: %s",item->unknown_conditions); if (item->unknown_effects) igTextWrapped("Unknown effects: %s",item->unknown_effects);
    for (i=item->first_participant;i<item->first_participant+item->participant_count && i<item->first_participant+64;++i) { const GD_RuleParticipant *actor=&rules->participants[i]; igTextWrapped("%s (%s): requires %s | excludes %s | other axes %s | becomes %s | preserves %s | calls %s",actor->name,gd_rule_noun_label(rules,actor),actor->requires ? actor->requires:"unspecified",actor->skip_if ? actor->skip_if:"none",actor->other_axes ? actor->other_axes:"unspecified",actor->becomes ? actor->becomes:"none",actor->preserve ? actor->preserve:"none",actor->causes ? actor->causes:"none"); }
    if (item->participant_count>64) igText("%d more participants in full source/native CLI; inspection list capped at 64.",item->participant_count-64);
}
void gd_explorer_query_source(GD_ExplorerContext *context, int edge) {
    GD_ExplorerQueryState *state=context->pane->query_state; const GD_QuerySceneEdge *item; const GD_Rules *rules=context->project->rules;
    if (!state || !state->scene || edge<0 || edge>=state->scene->layout.edge_count) return;
    item=&state->scene->edges[edge];
    if(state->scene->overview) {
        if(item->kind==GD_QUERY_EDGE_AXIS) {const GD_Edge *reference=&state->query->model->edges[item->source];igTextWrapped("Declared axis usage: %s | line %d | %s",state->query->model->records[reference->source].reference,reference->line,reference->reference);return;}
        igTextWrapped("%d distinct interaction records; same-case participation, not direct modification or frequency. Conditional/excluded declarations stay evidence, not proven applicability.",gd_query_scene_overview_count(state->scene,edge));
        {int c,start,pages=(rules->case_count+63)/64;if(pages<1) return;if(state->coverage_page>=pages) state->coverage_page=pages-1;
         igSliderInt("Interaction evidence source-slot page",&state->coverage_page,0,pages-1,"%d",0);start=state->coverage_page*64;
         igTextWrapped("Scanning case slots %d..%d of %d; page may contain no matches. Full declarations remain in query result/source.",start,start+64<rules->case_count ? start+64:rules->case_count,rules->case_count);
         for(c=start;c<start+64 && c<rules->case_count;++c) if(gd_query_scene_overview_case(state->scene,edge,c)) {igPushID_Int(c);case_details(context,c);igPopID();}}
        return;
    }
    if (state->query->effects) {
        const GD_EffectLink *link=&state->query->effects->links[item->source]; const GD_ResourceRelation *declaration=&state->query->resource->relations[link->relation];
        igTextWrapped("%s | line %d | concrete target %s",item->kind==GD_QUERY_EDGE_EFFECT ? "Direct conditional effect":"Mapped invocation, not temporal sequence",declaration->line,link->participant>=0 ? rules->participants[link->participant].name:"unresolved");
        case_details(context,declaration->source_case);
        if (declaration->kind==GD_RESOURCE_QUANTITY_WRITE) igTextWrapped("Literal quantity: %s",rules->assignments[declaration->declaration].expression);
    } else if (item->kind==GD_QUERY_EDGE_EFFECT) {
        const GD_RuleWrite *write=&state->query->variants->writes[item->source]; igTextWrapped("Target %s | origin line %d | %s",write->actor>=0 ? rules->participants[write->actor].name:"unresolved",write->line,write->conditional ? "conditional":"checked proposal, not final outcome"); case_details(context,write->origin_case);
        if (write->kind==GD_WRITE_QUANTITY) igTextWrapped("Expression %s | result %s%g | unit/bounds/checks remain native",rules->assignments[write->declaration].expression,write->value.known ? "":"unknown; payload ",write->value.number);
    } else igTextWrapped("Context/assumption association, not temporal execution. Variant %d.",item->variant);
}
static void coverage_details(GD_ExplorerContext *context, int value_index) {
    GD_ExplorerQueryState *state=context->pane->query_state; const GD_Query *query=state->query; const GD_StateCoverage *coverage=query->state_coverage;
    const GD_StateCoverageValue *value=&coverage->values[value_index]; uint64_t bit=UINT64_C(1)<<value_index; int i,start,end,pages=(query->binding_count+63)/64;
    igTextWrapped("Finite hypothesis: %s | %s",value->value,value->declared ? "declared vocabulary":"explicit requested sample");
    igTextWrapped("Cases: definite %d | undetermined %d | excluded %d | unfinished %d",value->definite_cases,value->undetermined_cases,value->excluded_cases,value->unfinished_cases);
    igTextWrapped("Bindings: definite %d | undetermined %d | excluded %d | unfinished %d",value->definite_bindings,value->undetermined_bindings,value->excluded_bindings,value->unfinished_bindings);
    igTextWrapped("No retained candidate: %s. Unknown means absence was not checked; false includes retained undetermined guards.",value->no_retained_candidate==GD_RULE_TRUTH_UNKNOWN ? "unknown":value->no_retained_candidate==GD_RULE_TRUTH_TRUE ? "checked absence":"false");
    if (pages) igSliderInt("Binding evidence page",&state->coverage_page,0,pages-1,"%d",0);
    start=state->coverage_page*64; end=start+64; if (end>query->binding_count) end=query->binding_count;
    igText("Local bindings %d..%d of %d; rows are independent hypotheses",start,end,query->binding_count);
    for (i=start;i<end;++i) {
        const GD_QueryBinding *binding=&query->bindings[i]; const GD_RuleCase *root=&query->rules->cases[binding->case_index]; const GD_RuleParticipant *participant=&query->rules->participants[binding->participant];
        const char *truth=!(coverage->present[i]&bit) ? "unavailable":coverage->definite[i]&bit ? "definite":coverage->undetermined[i]&bit ? "undetermined":"excluded";
        igPushID_Int(i);
        if (igTreeNode_Str(TextFormat("%s#%s / %s: %s [%s]",query->model->records[query->rules->sources[root->source].record].reference,root->name,participant->name,truth,coverage->finished[i]&bit ? "examined":"unfinished"))) { igTextWrapped("Object: %s; value applies only this local binding. Whole root guards/context remain native; other bindings stay unknown.",gd_rule_noun_label(query->rules,participant)); case_details(context,binding->case_index); igTreePop(); }
        igPopID();
    }
}
void gd_explorer_query_details(GD_ExplorerContext *context) {
    GD_ExplorerPane *pane=context->pane; GD_ExplorerQueryState *state=pane->query_state; int record=gd_explorer_query_record(pane),selected=pane->selection.node,i;
    if (state && state->scene && selected>=0 && selected<state->scene->layout.node_count) {
        const GD_QuerySceneNode *item=&state->scene->nodes[selected]; igTextWrapped("%s",state->scene->layout.nodes[selected].label);
        if(state->scene->overview) igTextWrapped("Source-backed overview definition; %d connected actors. Positions and shared case counts are not applicability, runtime instances or reachable states.",item->incoming);
        if (item->kind==GD_QUERY_NODE_OBJECT) {
            int edge,axes=0; const GD_Model *model=state->query->model;
            if(!state->scene->overview) igTextWrapped("Loaded object definition; %d exact local-binding occurrences. No applicability or runtime-instance claim.",item->incoming);
            for (edge=state->query->rules->actor_first[item->source];edge>=0;edge=state->query->rules->next_axis[edge]) {
                const GD_Edge *reference=&model->edges[edge]; ++axes;
                if (state->query->rules->axis_resolution[edge]==GD_RULE_RESOLVED) { const GD_Record *axis=&model->records[reference->target]; igTextWrapped("%s | type %s | unit %s | %d declared values",axis->reference,axis->axis.type ? axis->axis.type:"unspecified",axis->axis.unit && *axis->axis.unit ? axis->axis.unit:"unspecified",axis->axis.allowed_count); }
                else igTextWrapped("Unresolved declared axis: %s",reference->reference);
            }
            if (!axes) igTextWrapped("No retained axis references. Inspect source to distinguish unknown inventory from explicitly empty inventory.");
        } else if (state->query->state_coverage) igTextWrapped("Aggregate independent binding hypotheses; card is not one case applicability verdict."); else if(!state->scene->overview) igTextWrapped("Applicability %s | %s",gd_rule_truth_name(item->applicability),item->ghost ? "ghost context, not additional declaration":"native context");
        if (state->query->state_coverage) coverage_details(context,item->source);
        if (state->query->effects) {
            char id[GD_EFFECT_ID_CAPACITY]; const GD_EffectNode *node=&state->query->effects->nodes[item->source];
            if (gd_effect_node_id(state->query->effects,item->source,id,sizeof(id))) { igTextWrapped("Exact typed ID: %s",id); if (igSmallButton("Copy typed ID")) SetClipboardText(id); }
            if (node->kind==GD_EFFECT_ACTIVATION) { const GD_ResourceNode *activation=&state->query->resource->nodes[node->activation]; case_details(context,activation->case_index); igText("Mapped instance %s | depth %d | frontier %s | pool conflict %s",activation->instance ? "yes":"no",activation->depth,activation->frontier ? "yes":"no",activation->binding_conflict ? "yes":"no"); }
            else igText("%d direct declarations in shown contributor context; not gameplay frequency.",item->incoming);
        } else if (item->kind==GD_QUERY_NODE_ACTIVATION) case_details(context,state->query->variants->activations[item->source].case_index);
        if (igTreeNode_Str("Incident guards/effects and exact source declarations")) { for (i=0;i<pane->layout->edge_count;++i) if (pane->layout->edges[i].a==selected || pane->layout->edges[i].b==selected) { igPushID_Int(i); gd_explorer_query_source(context,i); igPopID(); } igTreePop(); }
    }
    if (state && state->selected && igTreeNode_Str("Selected result: exact contributor paths")) {
        for (i=0;i<state->selected->count && i<64;++i) { int path[GD_MAX_RULE_EXPANSION_DEPTH],count=gd_effect_contributor_path(state->selected,i,path,GD_MAX_RULE_EXPANSION_DEPTH),step; const GD_EffectLink *link=&state->query->effects->links[state->selected->links[i]]; igTextWrapped("Target: %s",link->participant>=0 ? context->project->rules->participants[link->participant].name:"unresolved"); for (step=0;step<count;++step) case_details(context,state->query->resource->nodes[state->query->effects->nodes[path[step]].activation].case_index); }
        if (state->selected->count>64) igText("%d further contributor paths remain highlighted; use native CLI for all rows.",state->selected->count-64);
        igTreePop();
    }
    if (pane->selection.cell>=0) gd_explorer_query_source(context,pane->selection.cell);
    if (record>=0) { GD_Model *model=gd_explorer_model(context->project,pane->view); const GD_Record *source=&model->records[record]; gd_noun_ui_text(source,source->display_name);igTextWrapped("%s | status %s | %s",source->reference,source->status ? source->status:"unspecified",source->source_path); if (igSmallButton("Copy source path")) SetClipboardText(source->source_path); if (igTreeNode_Str("Full authored source (other effects retained)")) { igPushTextWrapPos(0); igTextUnformatted(source->body,NULL); igPopTextWrapPos(); igTreePop(); } }
}
