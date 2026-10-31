/* Unified-shell IPC adapter. Included after existing navigation/selection helpers. */
#ifndef GD_EXPLORER_VIEWER_H
#define GD_EXPLORER_VIEWER_H
#include "gd_viewer.h"
#include "gd_actor_identity.h"
static const char *const viewer_projections[GD_EXPLORER_VIEWS]={"skills","challenges","relationships","matrix","mda","resources","actors","effects"};
typedef struct {
    GD_ExplorerContext *context; GD_ExplorerPane *panes; GD_ViewerServer *server;
    int *active,*stale; uint64_t reload_generation;
    GD_ViewerRequest hint; int has_hint;
    GD_ViewerRequest pending_hint; int hint_pending;
    uint64_t hint_generation,pending_hint_generation;
    unsigned hint_revision,pending_hint_revision;
    const GD_Collection *collections; int collection_count;
    char toast[128]; double toast_until; int human_go;
} ExplorerViewer;
static int viewer_hint_current(const ExplorerViewer *viewer) {
    return !*viewer->hint.id || (viewer->hint_generation==viewer->reload_generation && viewer->hint_revision==viewer->context->project->revision);
}
static int viewer_interacting(void) {
    return igGetIO_Nil()->WantTextInput || IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT) || IsMouseButtonDown(MOUSE_BUTTON_MIDDLE) ||
        IsKeyDown(KEY_ENTER) || IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_TAB) || IsKeyDown(KEY_UP) || IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_RIGHT);
}
static void viewer_apply_hint(ExplorerViewer *viewer) {
    if (!viewer->hint_pending || viewer_interacting()) return;
    viewer->hint=viewer->pending_hint; viewer->has_hint=1; viewer->hint_pending=0;
    viewer->hint_generation=viewer->pending_hint_generation; viewer->hint_revision=viewer->pending_hint_revision;
}
static void viewer_member(GD_ViewerWriter *w, const char *key, const char *value) {
    gd_viewer_quote(w,key); gd_viewer_write(w,":"); gd_viewer_quote(w,value);
}
static void viewer_form(GD_ViewerWriter *w, GD_ExplorerForm form, const void *values) {
    int i,count; const GD_ExplorerControl *fields=gd_explorer_form_fields(form,&count);
    gd_viewer_write(w,"{");
    for (i=0;i<count;++i) {
        const GD_ExplorerControl *field=&fields[i]; const char *value=(const char *)values+field->offset;
        if (i) gd_viewer_write(w,","); gd_viewer_quote(w,field->name); gd_viewer_write(w,":");
        if (field->type==GD_CONTROL_INT) gd_viewer_write(w,"%d",*(const int *)value);
        else if (field->type==GD_CONTROL_BOOL) gd_viewer_write(w,"%s",*(const bool *)value?"true":"false");
        else gd_viewer_quote(w,value);
    }
    gd_viewer_write(w,"}");
}
#include "gd_explorer_viewer_nodes.h"
static void viewer_selection(GD_ViewerWriter *w, GD_ExplorerContext *c) {
    GD_ExplorerPane *p=c->pane; GD_Model *model=gd_explorer_model(c->project,p->view);
    ViewerIdentities ids; char id[GD_MAX_PATH]; int record=p->prepared?selected_record(c):-1,node=viewer_selected_node(c),available;
    viewer_identities(c,&ids); available=viewer_canvas_id(c,node,&ids,id,sizeof(id));
    gd_viewer_write(w,"{\"source\":");
    if (record>=0 && record<model->record_count) {
        gd_viewer_write(w,"{"); viewer_member(w,"reference",model->records[record].reference); gd_viewer_write(w,","); viewer_member(w,"path",model->records[record].source_path);
        gd_viewer_write(w,",\"body_line\":%d}",model->records[record].body_line);
    } else gd_viewer_write(w,"null");
    gd_viewer_write(w,",\"native_id\":"); gd_viewer_quote(w,available?id:NULL);
    gd_viewer_write(w,",\"kind\":"); gd_viewer_quote(w,node>=0 && node<viewer_canvas_count(c)?viewer_canvas_kind(c,node):NULL);
    gd_viewer_write(w,",\"snapshot_local\":{\"node\":%d,\"cell\":%d,\"case_offset\":%d,\"mda_link\":%d},\"identity_available\":%s,\"identity_limited\":%s}",node,p->selection.cell,p->selection.interaction,p->mda.link,available?"true":"false",ids.limited?"true":"false");
}
static void viewer_resource_id(GD_ViewerWriter *w, const GD_ResourceGraph *graph, int node) {
    char id[GD_MAX_PATH]="";
    if (graph && node>=0) gd_resource_node_id(graph,node,-1,id,sizeof(id));
    gd_viewer_quote(w,*id?id:NULL);
}
static void viewer_query_selectors(GD_ViewerWriter *w, const GD_ExplorerProject *project, const GD_ExplorerQueryOptions *options) {
    const GD_Rules *rules=project->rules; const GD_Model *model=project->models[GD_EXPLORER_RULE_DATA];
    char qualified[GD_MAX_PATH]=""; int record;
    if (rules && options->root_case>=0 && options->root_case<rules->case_count) {
        const GD_RuleCase *item=&rules->cases[options->root_case];
        record=rules->sources[item->source].record;
        if (snprintf(qualified,sizeof(qualified),"%s#%s",model->records[record].reference,item->name)>=(int)sizeof(qualified)) *qualified=0;
    }
    gd_viewer_write(w,"{"); viewer_member(w,"case",*qualified?qualified:NULL);
    gd_viewer_write(w,","); viewer_member(w,"participant",rules && options->participant>=0 && options->participant<rules->participant_count?rules->participants[options->participant].name:NULL);
    gd_viewer_write(w,","); viewer_member(w,"axis",model && options->axis>=0 && options->axis<model->record_count?model->records[options->axis].reference:NULL);
    gd_viewer_write(w,","); viewer_member(w,"noun",model && options->noun>=0 && options->noun<model->record_count?model->records[options->noun].reference:NULL); gd_viewer_write(w,"}");
}
static void viewer_key(GD_ViewerWriter *w, GD_ExplorerContext *c, int key) {
    GD_Model *model=gd_explorer_model(c->project,c->pane->view);
    if (c->pane->view==GD_EXPLORER_RESOURCES) viewer_resource_id(w,c->project->resources,key);
    else gd_viewer_quote(w,!query_view(c->pane->view) && key>=0 && key<model->record_count?model->records[key].reference:NULL);
}
static void viewer_node_id(GD_ViewerWriter *w, GD_ExplorerContext *c, int node) {
    const GD_Layout *layout=c->pane->layout;
    viewer_key(w,c,layout && node>=0 && node<layout->node_count?layout->nodes[node].key:-1);
}
static void viewer_layers(GD_ViewerWriter *w, GD_ExplorerContext *c) {
    GD_ExplorerPane *p=c->pane; GD_Model *model=gd_explorer_model(c->project,p->view); int i,emitted=0;
    gd_viewer_write(w,"{\"available\":%s,\"shared\":%s,\"lens\":%d,\"branches\":%s,\"foundations\":%s,\"shapes\":%s,\"overlay\":%s,\"peer_node\":%d,\"foundation_peer_node\":%d,\"paths\":{\"enabled\":%s,\"declared\":%s,\"lost\":%d,\"compare\":%d,\"starts\":[",
        !query_view(p->view)&&!grid_view(p)?"true":"false",p->shared?"true":"false",p->lens,p->branches?"true":"false",p->foundations?"true":"false",p->shapes?"true":"false",p->overlay?"true":"false",p->peer,p->foundation_peer,p->paths.enabled?"true":"false",p->paths.declared?"true":"false",p->paths.lost,p->paths.compare);
    for (i=0;i<p->paths.count;++i) { if (i) gd_viewer_write(w,","); viewer_key(w,c,p->paths.keys[i]); }
    gd_viewer_write(w,"]},\"reach\":{\"enabled\":%s,\"relation\":%d,\"reverse\":%d,\"peer_node\":%d,\"target_node\":%d},\"overview\":{\"enabled\":%s,\"row\":%d,\"column\":%d},\"relationship_root\":",
        p->reach.enabled?"true":"false",p->reach.relation,p->reach.reverse,p->reach.peer,p->reach.target,p->overview.enabled?"true":"false",p->overview.row,p->overview.column);
    viewer_key(w,c,p->view==GD_EXPLORER_RELATIONSHIPS?p->focus.root:-1);
    gd_viewer_write(w,",\"expanded_records\":[");
    if (p->view==GD_EXPLORER_RELATIONSHIPS) for (i=0;i<model->record_count;++i) if (p->focus.expanded[i]) { if (emitted++) gd_viewer_write(w,","); gd_viewer_quote(w,model->records[i].reference); }
    gd_viewer_write(w,"],\"hypothetically_known\":["); emitted=0;
    if (is_graph(p->view)) for (i=0;i<model->record_count;++i) if (p->known[i]) { if (emitted++) gd_viewer_write(w,","); gd_viewer_quote(w,model->records[i].reference); }
    gd_viewer_write(w,"],\"peer_id\":"); viewer_node_id(w,c,p->peer);
    gd_viewer_write(w,",\"foundation_peer_id\":"); viewer_node_id(w,c,p->foundation_peer);
    gd_viewer_write(w,",\"reach_peer_id\":"); viewer_node_id(w,c,p->reach.peer);
    gd_viewer_write(w,",\"reach_target_id\":"); viewer_node_id(w,c,p->reach.target);
    gd_viewer_write(w,"}");
}
static void viewer_inspect(ExplorerViewer *viewer, GD_ViewerWriter *w, const char *fields) {
    GD_ExplorerContext *c=viewer->context; GD_ExplorerContext item=*c; int v,i;
    gd_viewer_write(w,"{\"ok\":true,\"protocol\":1,"); viewer_member(w,"session",gd_viewer_session(viewer->server));
    gd_viewer_write(w,","); viewer_member(w,"projection",viewer_projections[c->pane->view]);
    gd_viewer_write(w,",\"reload_generation\":\"%llu\",\"draft_revision\":%u,\"stale\":%s,\"draft\":{\"modified_records\":%d,\"undo_steps\":%d,\"rank_input_pending\":%s}",
        (unsigned long long)viewer->reload_generation,c->project->revision,*viewer->stale?"true":"false",c->project->draft?c->project->draft->modified_count:0,c->project->draft?c->project->draft->history_count:0,gd_explorer_rank_pending(c->project,viewer->panes)?"true":"false");
    if (gd_viewer_field(fields,"selection")) { gd_viewer_write(w,",\"selection\":"); viewer_selection(w,c); }
    if (fields && *fields && gd_viewer_field(fields,"source")) { gd_viewer_write(w,",\"source\":"); viewer_source(c,w); }
    if (gd_viewer_field(fields,"snapshots")) {
    gd_viewer_write(w,",\"snapshots\":[");
    for (i=0,v=0;v<GD_EXPLORER_FAMILIES;++v) if (c->project->models[v]) {
        GD_Model *model=c->project->models[v]; int j;
        if (i++) gd_viewer_write(w,",");
        gd_viewer_write(w,"{\"family\":%d,\"source_digest\":\"%016llx\",\"collections\":[",v,(unsigned long long)model->source_digest);
        for (j=0;j<model->collection_count;++j) {
            if (j) gd_viewer_write(w,",");
            gd_viewer_write(w,"{"); viewer_member(w,"alias",model->collections[j].alias); gd_viewer_write(w,","); viewer_member(w,"directory",model->collections[j].directory); gd_viewer_write(w,",\"role\":%d}",model->collections[j].role);
        }
        gd_viewer_write(w,"]}");
    }
    gd_viewer_write(w,"]");
    }
    if (gd_viewer_field(fields,"panes")) {
    gd_viewer_write(w,",\"panes\":[");
    for (v=0;v<GD_EXPLORER_VIEWS;++v) {
        GD_ExplorerPane *p=&viewer->panes[v];
        if (v) gd_viewer_write(w,",");
        gd_viewer_write(w,"{"); viewer_member(w,"projection",viewer_projections[v]);
        gd_viewer_write(w,",\"available\":%s,\"prepared\":%s,\"layout\":%d,\"sort\":%d,\"horizontal\":%d,\"paused\":%s,\"camera\":{\"x\":%.9g,\"y\":%.9g,\"zoom\":%.9g},\"focus_layout\":%s,\"focus_auto_fit\":%s,",
            c->project->available&(1u<<v)?"true":"false",p->prepared?"true":"false",p->layout_choice,p->sort,p->horizontal,p->paused?"true":"false",(double)p->camera.target.x,(double)p->camera.target.y,(double)p->camera.zoom,p->distance_focus_enabled?"true":"false",p->focus_auto_fit?"true":"false");
        viewer_member(w,"filter",p->filter); gd_viewer_write(w,","); viewer_member(w,"find_reference",*p->find_reference?p->find_reference:NULL);
        gd_viewer_write(w,",\"visible_matches\":%d,\"hidden_matches\":%d,\"selection\":",p->visible_matches,p->hidden_matches);
        item.pane=p;
        if (c->project->available&(1u<<v)) viewer_selection(w,&item); else gd_viewer_write(w,"null");
        gd_viewer_write(w,",\"layers\":"); if (c->project->available&(1u<<v)) viewer_layers(w,&item); else gd_viewer_write(w,"null");
        gd_viewer_write(w,",\"mda_query\":");
        if (v==GD_EXPLORER_MDA && c->project->mda) { gd_viewer_write(w,"{\"root\":"); viewer_key(w,&item,c->project->mda->root); gd_viewer_write(w,",\"direction\":%d}",c->project->mda->direction); } else gd_viewer_write(w,"null");
        gd_viewer_write(w,",\"rank_editor\":{\"record\":%d,",p->editor.record); viewer_member(w,"text",p->editor.rank_text); gd_viewer_write(w,"}");
        gd_viewer_write(w,",\"mda_display\":{\"only_scope\":%s,\"only_path\":%s,\"roles\":[%s,%s,%s,%s],",p->mda.only_scope?"true":"false",p->mda.only_path?"true":"false",p->mda.roles[0]?"true":"false",p->mda.roles[1]?"true":"false",p->mda.roles[2]?"true":"false",p->mda.roles[3]?"true":"false");
        viewer_member(w,"filter",p->mda.filter); gd_viewer_write(w,"}");
        gd_viewer_write(w,",\"resource_query\":{\"from\":"); viewer_resource_id(w,c->project->resources,p->resource_from);
        gd_viewer_write(w,",\"to\":"); viewer_resource_id(w,c->project->resources,p->resource_to);
        gd_viewer_write(w,",\"witness\":%d,\"accepted_only\":%s,\"only_witness\":%s,\"work_limit\":%d,\"result_available\":%s}",p->resource_witness,p->resource_accepted?"true":"false",p->resource_only_witness?"true":"false",p->resource_work_limit,p->resource_routes?"true":"false");
        gd_viewer_write(w,",\"query\":{\"pending\":"); viewer_form(w,GD_FORM_QUERY,&p->query_options);
        gd_viewer_write(w,",\"pending_selectors\":"); viewer_query_selectors(w,c->project,&p->query_options);
        gd_viewer_write(w,",\"applied\":"); if (p->query_state) viewer_form(w,GD_FORM_QUERY,&p->query_state->applied); else gd_viewer_write(w,"null");
        gd_viewer_write(w,",\"applied_selectors\":"); if (p->query_state) viewer_query_selectors(w,c->project,&p->query_state->applied); else gd_viewer_write(w,"null"); gd_viewer_write(w,"}");
        gd_viewer_write(w,",\"structure\":");
        if (p->structure_state) {
            gd_viewer_write(w,"{\"has_applied\":%s,\"pending\":",p->structure_state->has_applied?"true":"false"); viewer_form(w,GD_FORM_STRUCTURE,&p->structure_state->pending);
            gd_viewer_write(w,",\"applied\":"); if (p->structure_state->has_applied) viewer_form(w,GD_FORM_STRUCTURE,&p->structure_state->applied); else gd_viewer_write(w,"null");
            gd_viewer_write(w,",\"similarity\":{\"active\":%s,\"row\":%d,\"peer\":%d,\"camera\":{\"x\":%.9g,\"y\":%.9g,\"zoom\":%.9g}}}",gd_explorer_similarity_active(&item)?"true":"false",p->structure_state->similarity_row,p->structure_state->similarity_peer,(double)p->structure_state->similarity_camera.target.x,(double)p->structure_state->similarity_camera.target.y,(double)p->structure_state->similarity_camera.zoom);
        } else gd_viewer_write(w,"null");
        gd_viewer_write(w,",\"rules\":");
        if (p->rule_state) {
            gd_viewer_write(w,"{\"pending\":"); viewer_form(w,GD_FORM_RULES,&p->rule_state->pending); gd_viewer_write(w,",\"applied\":"); if (p->rule_state->result || p->rule_state->bindings) viewer_form(w,GD_FORM_RULES,&p->rule_state->applied); else gd_viewer_write(w,"null"); gd_viewer_write(w,"}");
        } else gd_viewer_write(w,"null");
        gd_viewer_write(w,"}");
    }
    gd_viewer_write(w,"]");
    }
    if (gd_viewer_field(fields,"agent_hint")) {
    gd_viewer_write(w,",\"agent_hint\":");
    if (viewer->has_hint) { gd_viewer_write(w,"{"); viewer_member(w,"projection",viewer->hint.projection); gd_viewer_write(w,","); viewer_member(w,"reference",*viewer->hint.reference?viewer->hint.reference:NULL); gd_viewer_write(w,","); viewer_member(w,"id",*viewer->hint.id?viewer->hint.id:NULL); gd_viewer_write(w,",\"stale\":%s}",viewer_hint_current(viewer)?"false":"true"); }
    else gd_viewer_write(w,"null");
    gd_viewer_write(w,",\"pending_agent_hint\":");
    if (viewer->hint_pending) { gd_viewer_write(w,"{"); viewer_member(w,"projection",viewer->pending_hint.projection); gd_viewer_write(w,","); viewer_member(w,"reference",*viewer->pending_hint.reference?viewer->pending_hint.reference:NULL); gd_viewer_write(w,","); viewer_member(w,"id",*viewer->pending_hint.id?viewer->pending_hint.id:NULL); gd_viewer_write(w,"}"); }
    else gd_viewer_write(w,"null");
    }
    gd_viewer_write(w,",\"limits\":{\"derived_navigation\":true,\"node_catalogue\":true,\"selected_source_body\":true,\"application_controls\":false,\"application_actions\":true,\"source_refresh\":true,\"events\":false,\"source_writes\":false,\"selection_owner\":\"application selection; agent hints separate\",\"control_indices\":\"snapshot-local only; not durable selectors\",\"evidence_tables\":\"not exported; use native queries with inspected selectors\"}}");
}
static int viewer_projection(const char *name) {
    int v; for (v=0;v<GD_EXPLORER_VIEWS;++v) if (!strcmp(name,viewer_projections[v])) return v; return -1;
}
static void viewer_toast(ExplorerViewer *viewer, const char *message, const char *subject) {
    if (viewer->human_go) { *viewer->toast=0; return; }
    int written=subject ? snprintf(viewer->toast,sizeof(viewer->toast),"%s %s",message,subject):snprintf(viewer->toast,sizeof(viewer->toast),"%s",message);
    if (written<0 || (size_t)written>=sizeof(viewer->toast)) snprintf(viewer->toast,sizeof(viewer->toast),"CLI activity");
    viewer->toast_until=igGetTime()+2.5;
}
static void viewer_feedback(ExplorerViewer *viewer) {
    if (viewer->has_hint) {
        bool open=true;
        igSetNextWindowPos((ImVec2_c){20,viewer->context->viewport.y+viewer->context->viewport.height-180},ImGuiCond_FirstUseEver,(ImVec2_c){0,0});
        igSetNextWindowSize((ImVec2_c){360,160},ImGuiCond_FirstUseEver);
        if (igBegin("Agent hint",&open,ImGuiWindowFlags_NoFocusOnAppearing|ImGuiWindowFlags_NoSavedSettings)) {
            int current=viewer_hint_current(viewer);
            igBeginDisabled(!current);
            if (igSmallButton("Go")) snprintf(viewer->hint.command,sizeof(viewer->hint.command),"navigate");
            igEndDisabled(); igSameLine(0,-1);
            if (igSmallButton("Dismiss")) open=false;
            igTextWrapped("%s | %s",viewer->hint.projection,*viewer->hint.id?viewer->hint.id:viewer->hint.reference);
            if (!current) igTextWrapped("Hint predates source/draft changes. Request a fresh hint.");
        }
        igEnd();
        if (!open) { viewer->has_hint=0; viewer->hint_pending=0; }
    }
    if (*viewer->toast && igGetTime()<viewer->toast_until) {
        igSetNextWindowPos((ImVec2_c){20,20},ImGuiCond_Always,(ImVec2_c){0,0}); igSetNextWindowBgAlpha(0.85f);
        if (igBegin("CLI feedback",NULL,ImGuiWindowFlags_NoDecoration|ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_NoFocusOnAppearing|ImGuiWindowFlags_NoInputs))
            igTextUnformatted(viewer->toast,NULL);
        igEnd();
    }
}
static void viewer_actions(GD_ExplorerContext *c, GD_ViewerWriter *w) {
    int action,written=0;
    gd_viewer_write(w,"{\"ok\":true,"); viewer_member(w,"projection",viewer_projections[c->pane->view]); gd_viewer_write(w,",\"actions\":[");
    for (action=0;action<GD_ACTION_COUNT;++action) {
        const GD_ExplorerActionInfo *info=gd_explorer_action_info((GD_ExplorerAction)action);
        if (!info || !(info->views&(1u<<c->pane->view))) continue;
        if (written++) gd_viewer_write(w,","); gd_viewer_write(w,"{"); viewer_member(w,"name",info->name);
        gd_viewer_write(w,","); viewer_member(w,"label",info->label);
        gd_viewer_write(w,",\"available\":%s}",gd_explorer_action_available(c,(GD_ExplorerAction)action)?"true":"false");
    }
    gd_viewer_write(w,"],\"parameter_mutation\":false,\"source_draft_edits\":false}");
}
static void viewer_dispatch(void *data, const GD_ViewerRequest *request, GD_ViewerWriter *w) {
    ExplorerViewer *viewer=data; GD_ExplorerContext *c=viewer->context,target=*c;
    int v=viewer_projection(request->projection),node=-1;
    if (!strcmp(request->command,"inspect")) { viewer_inspect(viewer,w,request->fields); return; }
    if (!strcmp(request->command,"refresh")) {
        char message[GD_MAX_PATH+512]; int updated;
        if (viewer_interacting()) { gd_viewer_error(w,"viewer.busy","Refresh blocked by active text/pointer/navigation-key input; no buffers discarded"); return; }
        if (!viewer->collections || viewer->collection_count<1) { gd_viewer_error(w,"viewer.refresh","Original collection bindings unavailable; no reload attempted"); return; }
        updated=refresh_sources(c,viewer->panes,viewer->collections,viewer->collection_count,message,sizeof(message));
        if (updated>=0) *viewer->stale=!updated;
        if (updated<=0) { gd_viewer_error(w,updated<0?"viewer.busy":"viewer.refresh",message); return; }
        ++viewer->reload_generation; viewer_toast(viewer,"CLI reloaded sources",NULL);
        gd_viewer_write(w,"{\"ok\":true,\"reloaded\":true,\"reload_generation\":\"%llu\",\"focus_changed\":false,\"draft_discarded\":false}",(unsigned long long)viewer->reload_generation); return;
    }
    if ((!strcmp(request->command,"nodes") || !strcmp(request->command,"actions")) && !*request->projection) v=c->pane->view;
    if (v<0 || !(c->project->available&(1u<<v))) { gd_viewer_error(w,"viewer.projection","Projection is unknown or unavailable with loaded bindings"); return; }
    target.pane=&viewer->panes[v];
    if (!strcmp(request->command,"nodes")) { viewer_nodes(viewer,request,&target,w); return; }
    if (!strcmp(request->command,"actions")) { viewer_actions(&target,w); return; }
    if (!strcmp(request->command,"action")) {
        int action;
        if (viewer_interacting() || gd_explorer_rank_pending(c->project,viewer->panes)) { gd_viewer_error(w,"viewer.busy","Action blocked by active input or pending rank edit; nothing applied"); return; }
        for (action=0;action<GD_ACTION_COUNT;++action) if (!strcmp(request->action,gd_explorer_action_info((GD_ExplorerAction)action)->name)) break;
        if (action==GD_ACTION_COUNT || !gd_explorer_action_available(&target,(GD_ExplorerAction)action)) { gd_viewer_error(w,"viewer.action","Action unknown or unavailable for target pane/result; inspect viewer actions"); return; }
        if (!gd_explorer_action_run(&target,(GD_ExplorerAction)action)) {
            const char *message=action==GD_ACTION_RULES_RUN || action==GD_ACTION_RULES_CLEAR ? target.pane->rule_message:target.pane->query_message;
            gd_viewer_error(w,"viewer.action",*message?message:"Native action failed; last good result retained"); return;
        }
        c->analysis->initialized=0; viewer_toast(viewer,"CLI action in",gd_explorer_view_names[v]);
        gd_viewer_write(w,"{\"ok\":true,\"executed\":true,"); viewer_member(w,"action",request->action);
        gd_viewer_write(w,","); viewer_member(w,"projection",viewer_projections[v]); gd_viewer_write(w,","); viewer_member(w,"active_projection",viewer_projections[c->pane->view]);
        gd_viewer_write(w,",\"focus_changed\":false,\"source_draft_changed\":false}"); return;
    }
    if (*request->reference && gd_explorer_record(gd_explorer_model(c->project,(GD_ExplorerView)v),request->reference)<0) {
        gd_viewer_error(w,"viewer.reference","Exact qualified source reference is absent or ambiguous; no approximate match"); return;
    }
    if (*request->id) {
        if (!viewer_canvas_current(&target)) { gd_viewer_error(w,"viewer.stale_canvas","Destination canvas is unprepared or from older draft revision; explicitly open projection first"); return; }
        node=viewer_find_node(&target,request->id,w); if (node<0) return;
    }
    if (!strcmp(request->command,"hint")) {
        viewer->pending_hint=*request; viewer->hint_pending=1;
        viewer->pending_hint_generation=viewer->reload_generation; viewer->pending_hint_revision=c->project->revision;
        gd_viewer_write(w,"{\"ok\":true,\"hint_updated\":true,\"display_pending\":true,\"persistent\":false,\"focus_changed\":false}"); return;
    }
    if (viewer_interacting() || gd_explorer_rank_pending(c->project,viewer->panes)) {
        gd_viewer_error(w,"viewer.busy","Navigation blocked by text input, pointer interaction or pending rank input; nothing applied"); return;
    }
    if (!*request->id && !pane_prepare(&target)) { gd_viewer_error(w,"viewer.prepare","Cannot prepare destination; active context retained"); return; }
    if (*request->reference && gd_explorer_similarity_active(&target)) { gd_viewer_error(w,"viewer.unsupported","Source Find navigation on similarity canvas is unsupported; use its exact native ID"); return; }
    if (*request->id && !viewer_select_node(&target,node,w)) return;
    /* Agent guard rejects pending edits; do not invoke human edit-commit path. */
    if (v!=c->pane->view || *request->reference || *request->id) viewer_toast(viewer,"CLI redirected view to",gd_explorer_view_names[v]);
    enter_view(c,viewer->panes,v,*request->reference?request->reference:NULL);
    *viewer->active=v;
    if (*request->reference) { matches(c); fit_view(c,1); c->pane->fit=0; }
    gd_viewer_write(w,"{\"ok\":true,\"focus_changed\":false,\"selection_changed\":%s,",*request->id?"true":"false"); viewer_member(w,"projection",viewer_projections[v]);
    gd_viewer_write(w,",\"visible_matches\":%d,\"hidden_matches\":%d,\"source_only\":%s}",c->pane->visible_matches,c->pane->hidden_matches,*request->id?"false":"true");
}
#endif
