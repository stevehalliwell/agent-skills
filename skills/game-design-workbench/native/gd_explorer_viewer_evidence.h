/* Retained owner state only. Pages never prepare canvases or run analysis. */
#ifndef GD_EXPLORER_VIEWER_EVIDENCE_H
#define GD_EXPLORER_VIEWER_EVIDENCE_H
#include "gd_diagnostics.h"
static void viewer_row_key(GD_ExplorerContext *c,const char *kind,int row,char *out,size_t capacity) {
    uint64_t hash=UINT64_C(14695981039346656037);
    hash=presentation_hash(hash,kind,strlen(kind)); hash=presentation_hash(hash,&c->pane->view,sizeof(c->pane->view)); hash=presentation_hash(hash,&row,sizeof(row));
    snprintf(out,capacity,"%s:%016llx",kind,(unsigned long long)hash);
}
static void viewer_record_layers(ExplorerViewer *viewer,GD_ExplorerContext *c) {
    if(!overview_available(c) || !presentation_cache(c)) return;
    viewer->layer_view=c->pane->view; viewer_control_token(viewer,c,GD_FORM_PRESENTATION,viewer->layer_token,sizeof(viewer->layer_token));
}
static int viewer_layers_current(ExplorerViewer *viewer,GD_ExplorerContext *c) {
    char token[64]; viewer_control_token(viewer,c,GD_FORM_PRESENTATION,token,sizeof(token));
    return overview_available(c) && presentation_cache(c) && *viewer->layer_token && viewer->layer_view==c->pane->view && !strcmp(token,viewer->layer_token);
}
static GD_DiagnosticSet viewer_diagnostic_set(GD_ExplorerContext *c) {
    GD_DiagnosticSet set={0}; set.model=gd_explorer_model(c->project,c->pane->view); set.rejected=c->pane->diagnostics.rejected;
    if(c->pane->view==GD_EXPLORER_MDA) {set.findings=c->project->mda->findings;set.finding_count=c->project->mda->finding_count;set.findings_omitted=c->project->mda->findings_omitted;}
    else if(c->pane->view>=GD_EXPLORER_RESOURCES) {set.findings=c->project->rules->findings;set.finding_count=c->project->rules->finding_count;set.findings_omitted=c->project->rules->findings_omitted;}
    else {set.findings=set.model->findings;set.finding_count=set.model->finding_count;set.findings_omitted=set.model->findings_omitted;}
    return set;
}
static int viewer_rule_rows(const GD_ExplorerPane *p,int checks) {
    const GD_ExplorerRuleState *s=p->rule_state;if(!s)return 0;
    if(!checks)return gd_explorer_rules_row_count(s);
    if(s->bindings)return 0;
    return s->result && s->result->analysis && s->result->request.kind==GD_NAMED_OUTCOMES ? s->result->analysis->scenario.checks_count:0;
}
static int viewer_evidence_match(const GD_ExplorerContext *c,const GD_ViewerRequest *request,int kind,int source_slot,const GD_DiagnosticSet *diagnostics,GD_DiagnosticItem *diagnostic) {
    if(kind==10)return gd_query_scene_overview_case(c->pane->query_state->scene,c->pane->selection.cell,source_slot);
    if(kind==9)return gd_noun_grid_case(c->project->noun_grid,c->pane->selection.cell,source_slot);
    if(kind==6)return c->pane->known[source_slot]!=0;
    if(kind==4){GD_DiagnosticItem temporary;if(!diagnostic)diagnostic=&temporary;
        if(!gd_diagnostic_item(diagnostics,source_slot,diagnostic))return 0;
        return !*request->reference || (diagnostic->record_reference && !strcmp(request->reference,diagnostic->record_reference));
    }
    return 1;
}
static void viewer_evidence(ExplorerViewer *viewer,const GD_ViewerRequest *request,GD_ExplorerContext *c,GD_ViewerWriter *w) {
    GD_ExplorerPane *p=c->pane; GD_Model *model=gd_explorer_model(c->project,p->view); GD_DiagnosticSet diagnostics=viewer_diagnostic_set(c);
    int total=0,matching=0,ordinal=0,start=(int)request->offset,i,written=0,kind=0; char token[64]; const char *form=request->form;
    if(!*form) {
        gd_viewer_write(w,"{\"ok\":true,\"forms\":[\"links\",\"cells\",\"cases\",\"diagnostics\",\"starts\",\"known\",\"rule-rows\",\"rule-checks\",\"noun-cases\",\"actor-cases\"],\"selection_forms\":[\"source\",\"link\",\"cell\",\"case\",\"diagnostic\",\"rule\",\"rule.check\"],\"selection_guard\":\"selection_token from evidence, or presentation control_token\",\"offset_units\":\"matching-retained-rows\",\"source_slot\":\"native snapshot metadata, not a write selector\",\"native_results\":[\"query\",\"structure\",\"rules\",\"bindings\",\"resource\",\"mda\",\"challenges\",\"skills\",\"layers\",\"comparison\",\"actor-overview\",\"noun-grid\"],\"result_command\":\"viewer result --projection VIEW --form FAMILY; base64 byte pages of native JSON\",\"layers_require\":\"view.analyze or current GUI layer cache; no hidden evaluation\"}"); return;
    }
    if(!strcmp(form,"links")) {kind=1;total=p->prepared && p->layout && !gd_explorer_similarity_active(c) ? p->layout->edge_count:0;}
    else if(!strcmp(form,"cells")) {kind=2;total=p->view==GD_EXPLORER_MATRIX ? model->cell_count:0;}
    else if(!strcmp(form,"cases")) {kind=3;total=p->view==GD_EXPLORER_MATRIX && p->selection.cell>=0 ? model->cells[p->selection.cell].pair_count:0;}
    else if(!strcmp(form,"diagnostics")) {kind=4;total=gd_diagnostic_count(&diagnostics);}
    else if(!strcmp(form,"starts")) {kind=5;total=p->paths.count;}
    else if(!strcmp(form,"known")) {kind=6;total=p->view==GD_EXPLORER_SKILLS ? model->record_count:0;}
    else if(!strcmp(form,"rule-rows")) {kind=7;total=viewer_rule_rows(p,0);}
    else if(!strcmp(form,"rule-checks")) {kind=8;total=viewer_rule_rows(p,1);}
    else if(!strcmp(form,"noun-cases")) {kind=9;total=p->view==GD_EXPLORER_NOUNS && p->selection.cell>=0 ? c->project->rules->case_count:0;}
    else if(!strcmp(form,"actor-cases")){kind=10;total=p->view==GD_EXPLORER_ACTORS && p->query_state && p->query_state->scene && p->query_state->scene->overview && p->selection.cell>=0 && p->selection.cell<p->query_state->scene->layout.edge_count ? c->project->rules->case_count:0;}
    else {gd_viewer_error(w,"viewer.evidence","Unknown page family; read viewer evidence catalogue or use viewer result for native result tables");return;}
    for(i=0;i<total;++i)matching+=viewer_evidence_match(c,request,kind,i,&diagnostics,NULL);
    viewer_control_token(viewer,c,GD_FORM_PRESENTATION,token,sizeof(token));
    gd_viewer_write(w,"{\"ok\":true,"); viewer_member(w,"projection",viewer_projections[p->view]); gd_viewer_write(w,",");viewer_member(w,"form",form);
    gd_viewer_write(w,",");viewer_member(w,"selection_token",token); gd_viewer_write(w,",\"source_slots\":%d,\"total_rows\":%d,\"offset\":%d,\"offset_units\":\"matching-retained-rows\",\"rows\":[",total,matching,start);
    for(i=0;i<total && written<(int)request->limit;++i) {
        char key[64]; GD_DiagnosticItem diagnostic;
        if(!viewer_evidence_match(c,request,kind,i,&diagnostics,&diagnostic))continue;
        if(ordinal++<start)continue;
        if(written++) gd_viewer_write(w,","); viewer_row_key(c,kind==1 ? "link":kind==2 ? "cell":kind==4 ? "diagnostic":kind==7 ? "rule":kind==8 ? "check":"row",i,key,sizeof(key));
        gd_viewer_write(w,"{\"source_slot\":%d,",i);viewer_member(w,"key",key);
        if(kind==1) {
            const GD_LayoutEdge *edge=&p->layout->edges[i]; char id[GD_MAX_PATH];
            gd_viewer_write(w,",\"from\":");gd_viewer_quote(w,presentation_node_id(c,edge->a,id,sizeof(id)) ? id:NULL);
            gd_viewer_write(w,",\"to\":");gd_viewer_quote(w,presentation_node_id(c,edge->b,id,sizeof(id)) ? id:NULL);
            gd_viewer_write(w,",\"relation\":");gd_viewer_quote(w,gd_graph_relation_name(edge->relation));
            gd_viewer_write(w,",\"active\":%s,\"visible\":%s,\"native_source_key\":%d",edge->active ? "true":"false",visible(c,edge->a) && visible(c,edge->b) ? "true":"false",edge->key);
            if(p->view==GD_EXPLORER_ACTORS && p->query_state && p->query_state->scene->overview) {
                const GD_QueryScene *scene=p->query_state->scene;const GD_QuerySceneEdge *metadata=&scene->edges[i];
                gd_viewer_write(w,",\"interaction_record_count\":%d,\"evidence_kind\":",metadata->kind==GD_QUERY_EDGE_AXIS ? 0:gd_query_scene_overview_count(scene,i));gd_viewer_quote(w,gd_query_scene_edge_kind_name(metadata->kind));
            }
            if(p->view==GD_EXPLORER_NOUNS) {gd_viewer_write(w,",\"case_count\":%d,\"metric\":\"distinct declared cases\",\"evidence_form\":\"noun-cases\"",c->project->noun_grid->pairs[edge->key].count);}
            else if(p->view==GD_EXPLORER_MATRIX) {const GD_Cell *cell=&model->cells[edge->key];gd_viewer_write(w,",\"noun\":");gd_viewer_quote(w,model->records[cell->noun].reference);gd_viewer_write(w,",\"verb\":");gd_viewer_quote(w,model->records[cell->verb].reference);gd_viewer_write(w,",\"case_count\":%d",cell->pair_count);}
            else if(p->view==GD_EXPLORER_RESOURCES) {const GD_ResourceRelation *relation=&c->project->resources->relations[edge->key];gd_viewer_write(w,",\"line\":%d,\"source_case_index\":%d,\"participant_index\":%d",relation->line,relation->source_case,relation->mapped_participant);}
            else if(p->view==GD_EXPLORER_MDA) {const GD_Edge *source=&model->edges[c->project->mda->links[edge->key].edge];gd_viewer_write(w,",\"source\":");gd_viewer_quote(w,model->records[source->source].reference);gd_viewer_write(w,",\"line\":%d,\"field\":",source->line);gd_viewer_quote(w,gd_field_name(source->field));}
            else if(p->view==GD_EXPLORER_SKILLS && edge->key>=0 && edge->key<model->skill_link_count) {gd_viewer_write(w,",\"source\":");gd_viewer_quote(w,model->records[model->skill_links[edge->key].dependent].reference);}
            else if(!query_view(p->view) && edge->key>=0 && edge->key<model->edge_count) {const GD_Edge *source=&model->edges[edge->key];gd_viewer_write(w,",\"source\":");gd_viewer_quote(w,model->records[source->source].reference);gd_viewer_write(w,",\"line\":%d,\"field\":",source->line);gd_viewer_quote(w,gd_field_name(source->field));}
        } else if(kind==2) {const GD_Cell *cell=&model->cells[i];gd_viewer_write(w,",\"noun\":");gd_viewer_quote(w,model->records[cell->noun].reference);gd_viewer_write(w,",\"verb\":");gd_viewer_quote(w,model->records[cell->verb].reference);gd_viewer_write(w,",\"case_count\":%d,\"active\":%s,\"selected\":%s",cell->pair_count,cell->active ? "true":"false",p->selection.cell==i ? "true":"false");}
        else if(kind==3) {const GD_Cell *cell=&model->cells[p->selection.cell];const GD_Record *record=&model->records[model->pairs[cell->first_pair+i].interaction];gd_viewer_write(w,",\"reference\":");gd_viewer_quote(w,record->reference);gd_viewer_write(w,",\"path\":");gd_viewer_quote(w,record->source_path);gd_viewer_write(w,",\"status\":");gd_viewer_quote(w,record->status);gd_viewer_write(w,",\"selected\":%s",p->selection.interaction==i ? "true":"false");}
        else if(kind==4) {gd_viewer_write(w,",\"kind\":%d,\"line\":%d,\"truncated\":%s,",diagnostic.kind,diagnostic.line,diagnostic.truncated ? "true":"false");viewer_member(w,"code",diagnostic.code);gd_viewer_write(w,",");viewer_member(w,"message",diagnostic.message);gd_viewer_write(w,",");viewer_member(w,"path",diagnostic.path);gd_viewer_write(w,",");viewer_member(w,"reference",diagnostic.record_reference);}
        else if(kind==5) {int node=-1,j;char id[GD_MAX_PATH];if(p->layout)for(j=0;j<p->layout->node_count;++j)if(p->layout->nodes[j].key==p->paths.keys[i])node=j;gd_viewer_write(w,",\"id\":");gd_viewer_quote(w,node>=0 && presentation_node_id(c,node,id,sizeof(id)) ? id:NULL);}
        else if(kind==7) {gd_viewer_write(w,",\"selected\":%s,\"row_kind\":",p->rule_state->row==i ? "true":"false");gd_viewer_quote(w,p->rule_state->bindings ? "binding":p->rule_state->result->request.kind==GD_NAMED_OUTCOMES ? "proposal":"comparison");gd_viewer_write(w,",\"evidence\":\"same slot in retained bindings/rules native result; exact rule key plus selection_token required\"");}
        else if(kind==9 || kind==10) {const GD_Rules *rules=c->project->rules;const GD_RuleCase *item=&rules->cases[i];const GD_Record *source=&model->records[rules->sources[item->source].record];int participant;gd_viewer_write(w,",");viewer_member(w,"source",source->reference);gd_viewer_write(w,",");viewer_member(w,"case",item->name);gd_viewer_write(w,",");viewer_member(w,"context",item->context);gd_viewer_write(w,",\"line\":%d,\"participants\":[",item->line);for(participant=item->first_participant;participant<item->first_participant+item->participant_count;++participant){const GD_RuleParticipant *binding=&rules->participants[participant];if(participant>item->first_participant)gd_viewer_write(w,",");gd_viewer_write(w,"{");viewer_member(w,"local",binding->name);gd_viewer_write(w,",");viewer_member(w,"noun",binding->noun);gd_viewer_write(w,",");viewer_member(w,"label",binding->noun_resolution==GD_RULE_RESOLVED ? model->records[binding->noun_record].display_name:"[Unresolved noun]");gd_viewer_write(w,",");viewer_member(w,"requires",binding->requires);gd_viewer_write(w,",");viewer_member(w,"skip_if",binding->skip_if);gd_viewer_write(w,"}");}gd_viewer_write(w,"]");}
        else if(kind==8) {const GD_RuleWriteCheck *check=&p->rule_state->result->analysis->scenario.checks[i];gd_viewer_write(w,",\"first_proposal_slot\":%d,\"second_proposal_slot\":%d,\"conflict\":",check->first,check->second);gd_viewer_quote(w,gd_rule_truth_name(check->conflict));}
        else {gd_viewer_write(w,",\"reference\":");gd_viewer_quote(w,model->records[i].reference);}
        gd_viewer_write(w,"}");
    }
    gd_viewer_write(w,"],\"returned_rows\":%d,\"next_offset\":",written);if(start+written<matching)gd_viewer_write(w,"%d",start+written);else gd_viewer_write(w,"null");
    gd_viewer_write(w,",\"scope\":\"matching retained rows; source_slot and guarded key retain native identity; no frozen transaction or source completeness proof\",\"producer_omissions\":");
    if(kind==4)gd_viewer_write(w,"%d",diagnostics.findings_omitted+model->errors_omitted+(diagnostics.rejected ? diagnostics.rejected->omitted:0));else gd_viewer_write(w,"null");
    if(kind==9 && c->project->noun_grid)gd_viewer_write(w,",\"producer_limited\":%s,\"source_incomplete\":%s",c->project->noun_grid->limited ? "true":"false",c->project->noun_grid->incomplete ? "true":"false");
    if(kind==10 && p->query_state && p->query_state->scene && p->query_state->scene->overview)gd_viewer_write(w,",\"producer_limited\":%s,\"overview_incomplete\":%s",p->query_state->scene->limited ? "true":"false",gd_query_scene_overview_incomplete(p->query_state->scene) ? "true":"false");
    if(kind==4)gd_viewer_write(w,",\"findings_omitted\":%d,\"loaded_errors_omitted\":%d,\"rejected_errors_omitted\":%d,\"capture_failed\":%s",diagnostics.findings_omitted,model->errors_omitted,diagnostics.rejected ? diagnostics.rejected->omitted:0,p->diagnostics.capture_failed ? "true":"false");
    gd_viewer_write(w,"}");
}
static int viewer_select(GD_ExplorerContext *c,const GD_ViewerRequest *request,GD_ViewerWriter *w) {
    GD_ExplorerPane *p=c->pane; GD_Model *model=gd_explorer_model(c->project,p->view); int i,found=-1; char key[64];
    if(!viewer_canvas_current(c)) {gd_viewer_error(w,"viewer.stale_canvas","Explicitly open current projection first");return 0;}
    if(!strcmp(request->form,"rule") || !strcmp(request->form,"rule.check")) {
        int checks=!strcmp(request->form,"rule.check");for(i=0;i<viewer_rule_rows(p,checks);++i){viewer_row_key(c,checks ? "check":"rule",i,key,sizeof(key));if(!strcmp(key,request->id)){if(found>=0)goto ambiguous;found=i;}}
        if(found>=0){if(checks){const GD_RuleWriteCheck *check=&p->rule_state->result->analysis->scenario.checks[found];gd_explorer_rules_choose(p->rule_state,check->first,check->second);}else gd_explorer_rules_choose(p->rule_state,found,-1);return 1;}
    }
    else if(!strcmp(request->form,"source")) {found=gd_explorer_record(model,request->id);if(found>=0){if(p->view==GD_EXPLORER_MDA){p->mda.record=found;p->mda.link=-1;}else p->selection=(GD_ExplorerSelection){-1,-1,0,found,-1};return 1;}}
    else if(!strcmp(request->form,"link")) {for(i=0;i<p->layout->edge_count;++i){viewer_row_key(c,"link",i,key,sizeof(key));if(!strcmp(key,request->id)){if(found>=0)goto ambiguous;found=i;}}
        if(found>=0){if(p->view==GD_EXPLORER_NOUNS){const GD_NounPair *pair=&c->project->noun_grid->pairs[p->layout->edges[found].key];noun_grid_choose(c,pair->a,pair->b);return 1;}if(p->view==GD_EXPLORER_MDA){p->mda.link=p->layout->edges[found].key;p->mda.record=model->edges[c->project->mda->links[p->mda.link].edge].source;}else if(p->view==GD_EXPLORER_MATRIX)p->selection=(GD_ExplorerSelection){p->layout->edges[found].key,-1,0,-1,-1};else if(query_view(p->view)) p->selection=(GD_ExplorerSelection){found,-1,0,-1,-1};
            else if(p->view==GD_EXPLORER_RESOURCES) {const GD_ResourceRelation *relation=&c->project->resources->relations[p->layout->edges[found].key];int source=c->project->rules->sources[c->project->rules->cases[relation->source_case].source].record;p->selection=(GD_ExplorerSelection){p->layout->edges[found].key,-1,0,source,-1};}
            else p->selection=(GD_ExplorerSelection){p->layout->edges[found].key,-1,0,-1,-1};return 1;}}
    else if(!strcmp(request->form,"cell") && p->view==GD_EXPLORER_MATRIX) {
        int noun=-1,verb=-1;
        if(*request->reference){noun=gd_explorer_record(model,request->reference);verb=gd_explorer_record(model,request->id);if(noun>=0 && verb>=0 && model->records[noun].role==GD_NOUN && model->records[verb].role==GD_VERB){found=gd_matrix_grid_cell(model,noun,verb);p->grid.noun=noun;p->grid.verb=verb;p->selection=(GD_ExplorerSelection){found,-1,0,-1,-1};return 1;}}
        else {for(i=0;i<model->cell_count;++i){viewer_row_key(c,"cell",i,key,sizeof(key));if(!strcmp(key,request->id)){if(found>=0)goto ambiguous;found=i;}}
            if(found>=0){p->grid.noun=model->cells[found].noun;p->grid.verb=model->cells[found].verb;p->selection=(GD_ExplorerSelection){found,-1,0,-1,-1};return 1;}}
    } else if(!strcmp(request->form,"case") && p->view==GD_EXPLORER_MATRIX && p->selection.cell>=0) {
        const GD_Cell *cell=&model->cells[p->selection.cell];for(i=0;i<cell->pair_count;++i)if(!strcmp(model->records[model->pairs[cell->first_pair+i].interaction].reference,request->id)){if(found>=0)goto ambiguous;found=i;}
        if(found>=0){p->selection.interaction=found;p->selection.linked=-1;return 1;}
    } else if(!strcmp(request->form,"diagnostic")) {
        GD_DiagnosticSet set=viewer_diagnostic_set(c); GD_DiagnosticItem item; for(i=0;i<gd_diagnostic_count(&set);++i){viewer_row_key(c,"diagnostic",i,key,sizeof(key));if(!strcmp(key,request->id) && gd_diagnostic_item(&set,i,&item) && !item.truncated && item.record>=0){if(found>=0)goto ambiguous;found=item.record;}}
        if(found>=0){p->selection=(GD_ExplorerSelection){-1,-1,0,found,-1};if(p->view==GD_EXPLORER_MDA){p->mda.record=found;p->mda.link=-1;}return 1;}
    }
    gd_viewer_error(w,"viewer.selection","Exact typed selection absent/unavailable; inspect evidence or controls, no approximation");return 0;
ambiguous:gd_viewer_error(w,"viewer.ambiguous_id","Selection key matches multiple retained rows; nothing selected");return 0;
}
#endif
