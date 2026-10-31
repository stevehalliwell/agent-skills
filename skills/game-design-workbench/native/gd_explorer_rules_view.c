#include "gd_explorer_controls.h"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_NO_EXPORT
#include "cimgui.h"
#include "gd_explorer_loaded_pickers.h"
#include <string.h>

static GD_ExplorerRuleState *empty_state(GD_Allocator allocator,const GD_ExplorerRuleForm *form) {
    GD_ExplorerRuleState *state=allocator.allocate(allocator.context,sizeof(*state)); if (!state) return NULL;
    memset(state,0,sizeof(*state)); state->allocator=allocator; state->pending=state->applied=*form; state->row=state->peer_write=-1; return state;
}
void gd_explorer_rules_destroy(GD_ExplorerRuleState *state) {
    if (state) { gd_named_destroy(state->result); gd_query_destroy(state->bindings); state->allocator.release(state->allocator.context,state); }
}
static char *line(char **cursor) {
    char *start=*cursor,*end; size_t length;
    while (start && *start) {
        end=strchr(start,'\n'); if (end) { *end=0; *cursor=end+1; } else *cursor=NULL;
        length=strlen(start); if (length && start[length-1]=='\r') start[--length]=0;
        if (length) return start; start=*cursor;
    }
    return NULL;
}
static GD_Query *binding_query(GD_Model *model,const GD_ExplorerProject *project,GD_ExplorerRuleState *state) {
    const GD_ExplorerRuleForm *form=&state->applied; GD_QueryRequest request={0}; char *cursor,*item;
    request.kind=GD_QUERY_BINDINGS; request.case_index=request.noun=request.axis=request.effect_kind=-1; request.work_limit=form->work_limit; request.participant=*form->participant ? form->participant:NULL;
    if (*form->object) { request.noun=gd_query_record(model,form->object,GD_NOUN); if (request.noun<0) { gd_error(model,"query.object","Object must identify one unique loaded noun",form->object,0); return NULL; } }
    if (*form->axis) { request.axis=gd_query_record(model,form->axis,GD_AXIS); if (request.axis<0) { gd_error(model,"query.axis","Axis must identify one unique loaded typed axis",form->axis,0); return NULL; } }
    if (*form->case_name) { request.case_index=gd_rule_case_find(model,project->rules,form->case_name); if (request.case_index<0) { gd_error(model,"query.case","Case must identify one unique parsed source#case",form->case_name,0); return NULL; } }
    memcpy(state->status_tokens,form->statuses,sizeof(state->status_tokens)); cursor=state->status_tokens;
    while ((item=line(&cursor))) { if (request.status_count==GD_MAX_CONTROL_FILTERS) { gd_error(model,"query.status","At most 64 explicit statuses; all must stand alone",NULL,0); return NULL; } request.statuses[request.status_count++]=item; }
    return gd_query_build(model,project->rules,project->calls,&request);
}
static GD_ExplorerRuleState *build(GD_ExplorerContext *context,const GD_ExplorerRuleForm *form) {
    GD_ExplorerRuleState *state=empty_state(context->project->allocator,form); GD_NamedRequest request={0};
    GD_Model *model=context->project->models[GD_EXPLORER_RULE_DATA]; GD_Error saved[GD_MAX_ERRORS]; int before=model->error_count,omitted=model->errors_omitted; char *cursor,*item;
    if (!state) { snprintf(context->pane->rule_message,sizeof(context->pane->rule_message),"Cannot allocate rule candidate; last good result retained."); return NULL; }
    request.kind=(GD_NamedKind)form->kind; request.projection="rules"; request.work_limit=form->work_limit; request.case_name=*state->applied.case_name ? state->applied.case_name:NULL;
    memcpy(state->fact_tokens,form->facts,sizeof(state->fact_tokens)); cursor=state->fact_tokens;
    while ((item=line(&cursor))) { if (request.assumption_count==GD_NAMED_SELECTORS) goto selectors; request.assumptions[request.assumption_count++]=item; }
    memcpy(state->context_tokens,form->contexts,sizeof(state->context_tokens)); cursor=state->context_tokens;
    while ((item=line(&cursor))) { int negative=*item=='!'; if (request.context_count==GD_NAMED_SELECTORS) goto selectors; request.contexts[request.context_count]=item+negative; request.context_truths[request.context_count++]=negative ? GD_RULE_TRUTH_FALSE:GD_RULE_TRUTH_TRUE; }
    /* Native reader owns diagnostic slots. Restore all slots after capturing a
       candidate failure, matching named adapter's single-threaded boundary. */
    memcpy(saved,model->errors,sizeof(saved)); if (form->bindings) state->bindings=binding_query(model,context->project,state); else state->result=gd_named_build(model,context->project,&request);
    if (!state->result && !state->bindings) snprintf(context->pane->rule_message,sizeof(context->pane->rule_message),"%s: %s. Last good rule result retained.",model->error_count>before ? model->errors[before].code:"query.memory",model->error_count>before ? model->errors[before].message:"Cannot prepare native rule workspace");
    memcpy(model->errors,saved,sizeof(saved)); model->error_count=before; model->errors_omitted=omitted;
    if (!state->result && !state->bindings) { gd_explorer_rules_destroy(state); return NULL; }
    if (state->result && state->result->root>=0) state->marked_cases[state->result->root]=1;
    return state;
selectors:
    snprintf(context->pane->rule_message,sizeof(context->pane->rule_message),"At most 64 literal facts and 64 context triples; no partial inputs adopted."); gd_explorer_rules_destroy(state); return NULL;
}
int gd_explorer_rules_run(GD_ExplorerContext *context) {
    GD_ExplorerRuleState *fresh;
    if (!context || !context->pane || !context->pane->rule_state || !context->project || !context->project->models[GD_EXPLORER_RULE_DATA]) return 0;
    *context->pane->rule_message=0; fresh=build(context,&context->pane->rule_state->pending); if (!fresh) return 0;
    gd_explorer_rules_destroy(context->pane->rule_state); context->pane->rule_state=fresh; return 1;
}
int gd_explorer_rules_remap(GD_ExplorerContext *context,const GD_ExplorerProject *old,const GD_ExplorerPane *prior) {
    const GD_ExplorerRuleState *before=prior->rule_state; GD_ExplorerRuleState *fresh; const GD_Model *a,*b; int i;
    if (!before) return 1;
    if (!gd_explorer_form_equal(GD_FORM_RULES,&before->pending,&before->applied)) { snprintf(context->pane->rule_message,sizeof(context->pane->rule_message),"Apply or clear pending rule inputs before refresh."); return 0; }
    a=old->models[GD_EXPLORER_RULE_DATA]; b=context->project->models[GD_EXPLORER_RULE_DATA];
    /* Conservative unchanged-reader guard, not a durable actor ID. No persisted
       native array offsets: every result is rebuilt from exact request strings. */
    if (before->result || before->bindings) {
        if (!a || !b || a->source_digest!=b->source_digest || a->collection_count!=b->collection_count) goto changed;
        for (i=0;i<a->collection_count;++i) if (a->collections[i].role!=b->collections[i].role || strcmp(a->collections[i].alias,b->collections[i].alias) || strcmp(a->collections[i].directory,b->collections[i].directory)) goto changed;
    }
    fresh=before->result || before->bindings ? build(context,&before->applied):empty_state(context->project->allocator,&before->applied);
    if (!fresh) { if (!*context->pane->rule_message) snprintf(context->pane->rule_message,sizeof(context->pane->rule_message),"Cannot allocate refreshed rule state; prior snapshot retained."); return 0; }
    context->pane->rule_state=fresh; return 1;
changed:
    snprintf(context->pane->rule_message,sizeof(context->pane->rule_message),"Rule reader context changed. Clear rule inspection before accepting new source context; last good snapshot retained."); return 0;
}
int gd_explorer_rules_marked(const GD_ExplorerContext *context,int case_index) {
    const GD_ExplorerRuleState *state=context && context->pane ? context->pane->rule_state:NULL;
    return state && ((state->result && state->result->project==context->project) || (state->bindings && state->bindings->model==context->project->models[GD_EXPLORER_RULE_DATA] && state->bindings->analysis==context->project->calls)) && case_index>=0 && case_index<context->project->rules->case_count && state->marked_cases[case_index];
}
static void mark_write(GD_ExplorerRuleState *state,const GD_RuleWrite *write) {
    int i; const GD_Rules *rules=state->result->project->rules;
    if (write->origin_case>=0 && write->origin_case<rules->case_count) state->marked_cases[write->origin_case]=1;
    for (i=0;i<write->path_count && i<GD_MAX_RULE_EXPANSION_DEPTH;++i) if (write->path[i]>=0 && write->path[i]<rules->case_count) state->marked_cases[write->path[i]]=1;
}
static void choose(GD_ExplorerRuleState *state,int row,int peer) {
    const GD_RuleAnalysis *analysis=state->result ? state->result->analysis:NULL; memset(state->marked_cases,0,sizeof(state->marked_cases)); state->row=row; state->peer_write=peer; state->axis_page=0;
    if (state->bindings) { if (row>=0 && row<state->bindings->binding_count) state->marked_cases[state->bindings->bindings[row].case_index]=1; return; }
    if (state->result->request.kind==GD_NAMED_OUTCOMES) {
        state->marked_cases[state->result->root]=1;
        if (row>=0 && row<analysis->scenario.write_count) mark_write(state,&analysis->scenario.writes[row]);
        if (peer>=0 && peer<analysis->scenario.write_count) mark_write(state,&analysis->scenario.writes[peer]);
    } else if (row>=0 && row<analysis->comparison_count) { const GD_RuleComparisonResult *pair=&analysis->comparisons[row]; state->marked_cases[pair->first_case]=state->marked_cases[pair->second_case]=1; if (pair->conflict_first>=0 && pair->conflict_second>=0) { mark_write(state,&pair->conflict_a); mark_write(state,&pair->conflict_b); } }
}
static void source(GD_ExplorerContext *context,int case_index) {
    GD_ExplorerPane *pane=context->pane; const GD_ResourceGraph *graph=context->project->resources; const GD_Rules *rules=context->project->rules; int i;
    if (case_index<0 || case_index>=rules->case_count) return;
    pane->selection=(GD_ExplorerSelection){.cell=-1,.node=-1,.interaction=0,.linked=rules->sources[rules->cases[case_index].source].record,.held=-1};
    for (i=0;i<graph->node_count;++i) if (graph->nodes[i].kind==GD_RESOURCE_CASE && !graph->nodes[i].instance && graph->nodes[i].case_index==case_index) { pane->selection.node=i; pane->selection.linked=-1; break; }
    if (pane->structure_state) pane->structure_state->similarity_canvas=false;
}
static void case_details(GD_ExplorerContext *context,int case_index) {
    const GD_Rules *rules=context->project->rules; const GD_Model *model=context->project->models[GD_EXPLORER_RULE_DATA]; const GD_RuleCase *item; const GD_Record *record; int i;
    if (case_index<0 || case_index>=rules->case_count) return; item=&rules->cases[case_index]; record=&model->records[rules->sources[item->source].record];
    igPushID_Int(case_index); igTextWrapped("%s#%s | line %d | status %s | target %s",record->reference,item->name,item->line,record->status ? record->status:"unspecified",item->primary_target ? item->primary_target:"unspecified");
    igTextWrapped("Context: %s | unknown conditions: %s | unknown effects: %s",item->context ? item->context:"unspecified",item->unknown_conditions ? item->unknown_conditions:"none declared",item->unknown_effects ? item->unknown_effects:"none declared");
    igTextWrapped("Case-level becomes %s | preserves %s | calls %s",item->default_effects[0] ? item->default_effects[0]:"unspecified",item->default_effects[1] ? item->default_effects[1]:"unspecified",item->default_effects[2] ? item->default_effects[2]:"none");
    for (i=item->first_participant;i<item->first_participant+item->participant_count && i<item->first_participant+64;++i) { const GD_RuleParticipant *actor=&rules->participants[i]; igTextWrapped("%s: noun %s | requires %s | excludes %s | becomes %s | preserves %s | calls %s",actor->name,actor->noun ? actor->noun:"unspecified",actor->requires ? actor->requires:"unspecified",actor->skip_if ? actor->skip_if:"none",actor->becomes ? actor->becomes:"unspecified",actor->preserve ? actor->preserve:"unspecified",actor->causes ? actor->causes:"none"); }
    if (item->participant_count>64) igTextWrapped("%d further local participant rows omitted from compact card; full native source remains below.",item->participant_count-64);
    if (igCollapsingHeader_TreeNodeFlags("Full source Markdown body (all cases in record)",0)) igTextWrapped("%s",record->body ? record->body:"no body");
    if (igButton("Inspect source declaration (ordinary canvas)",(ImVec2_c){0,0})) source(context,case_index);
    igTextWrapped("%s",record->source_path); igPopID();
}
static void proposal_details(GD_ExplorerContext *context,const GD_RuleWrite *write) {
    const GD_Rules *rules=context->project->rules; const GD_Model *model=context->project->models[GD_EXPLORER_RULE_DATA]; int i;
    igPushID_Ptr(write);
    igTextWrapped("Target local %s | canonical axis %s | origin local %s | line %d | %s proposal",write->actor>=0 && write->actor<rules->participant_count ? rules->participants[write->actor].name:"unresolved",write->axis>=0 && write->axis<model->record_count ? model->records[write->axis].reference:"unresolved",write->origin_participant>=0 && write->origin_participant<rules->participant_count ? rules->participants[write->origin_participant].name:"unspecified",write->line,write->conditional ? "conditional":"checked");
    if (write->kind==GD_WRITE_QUANTITY) { igTextWrapped("Expression %s",write->declaration>=0 && write->declaration<rules->assignment_count ? rules->assignments[write->declaration].expression:"unavailable"); if (write->value.known) igTextWrapped("Native value %.17g | unit %s | checks complete %s",write->value.number,write->axis>=0 && model->records[write->axis].axis.unit ? model->records[write->axis].axis.unit:"unspecified",write->value.checks_complete ? "yes":"no"); else igTextWrapped("Native numeric value unknown; no guessed payload."); }
    else igTextWrapped("%s %s",write->kind==GD_WRITE_PRESERVE ? "Preservation proposal":"State proposal",write->state ? write->state:"unspecified");
    for (i=0;i<write->path_count && i<GD_MAX_RULE_EXPANSION_DEPTH;++i) if (write->path[i]>=0 && write->path[i]<rules->case_count) { const GD_RuleCase *item=&rules->cases[write->path[i]]; igTextWrapped("Native source path %d: %s#%s",i,model->records[rules->sources[item->source].record].reference,item->name); }
    case_details(context,write->origin_case); igPopID();
}
static void binding_details(GD_ExplorerContext *context,GD_ExplorerRuleState *state) {
    const GD_Query *query=state->bindings; const GD_Rules *rules=query->rules; const GD_Model *model=query->model; int i,start,count=query->binding_count;
    igTextWrapped("Applied object-bindings: object %s | case %s | local %s | axis %s | statuses %s",*state->applied.object ? state->applied.object:"all",*state->applied.case_name ? state->applied.case_name:"all",*state->applied.participant ? state->applied.participant:"all",*state->applied.axis ? state->applied.axis:"all",*state->applied.statuses ? state->applied.statuses:"all");
    igTextWrapped("%d explicit local bindings | unexamined %d | excluded roots %d | work %d/%d | limited %s | native source findings %d (+%d omitted). Declarations, not applicability or merged runtime actors.",count,query->unexamined_bindings,query->excluded_roots,query->work,query->request.work_limit,query->limited ? "yes":"no",rules->finding_count,rules->findings_omitted);
    igTextWrapped("Purple source-case outlines follow selected row on ordinary Resource canvas. Facets/call placements retain Resource meaning; case outline is not an applicability claim.");
    if (gd_explorer_similarity_active(context)) igTextWrapped("Source inspection returns to ordinary canvas; similarity geometry remains unchanged.");
    if (count>64) igSliderInt("Binding page",&state->page,0,(count-1)/64,"%d",0); if (state->page<0 || state->page*64>=count) state->page=0; start=state->page*64;
    for (i=start;i<count && i<start+64;++i) { const GD_QueryBinding *binding=&query->bindings[i]; const GD_RuleCase *item=&rules->cases[binding->case_index]; const GD_RuleParticipant *actor=&rules->participants[binding->participant];
        igPushID_Int(i); if (igSelectable_Bool(TextFormat("%s#%s / %s | %s",model->records[rules->sources[item->source].record].reference,item->name,actor->name,actor->noun ? actor->noun:"unspecified noun"),state->row==i,0,(ImVec2_c){0,0})) choose(state,i,-1); igPopID();
    }
    if (state->row>=0 && state->row<count) { const GD_QueryBinding *binding=&query->bindings[state->row]; const GD_RuleParticipant *actor=&rules->participants[binding->participant];
        igTextWrapped("Local %s | noun %s | resolution %s | declaration line %d",actor->name,actor->noun ? actor->noun:"unspecified",gd_rule_resolution_name(actor->noun_resolution),actor->line);
        if (actor->noun_resolution==GD_RULE_RESOLVED) { int edge; const GD_Record *record=&model->records[actor->noun_record]; igTextWrapped("Canonical noun %s | id %s | explicit axes %s",record->reference,record->id ? record->id:"none",record->axes_declared ? "declared":"unspecified");
            int total=0,index=0;
            for (edge=rules->actor_first[actor->noun_record];edge>=0;edge=rules->next_axis[edge]) ++total;
            if (total>16) igSliderInt("Declared axis page",&state->axis_page,0,(total-1)/16,"%d",0);
            for (edge=rules->actor_first[actor->noun_record];edge>=0;edge=rules->next_axis[edge],++index) if (index/16==state->axis_page) {
                igPushID_Int(edge); igTextWrapped("Axis reference %s | %s | source line %d",model->edges[edge].reference,gd_rule_resolution_name(rules->axis_resolution[edge]),model->edges[edge].line);
                if (rules->axis_resolution[edge]==GD_RULE_RESOLVED) { const GD_Record *axis=&model->records[model->edges[edge].target]; const char *value=axis->axis.allowed; int j;
                    igTextWrapped("%s | id %s | type %s | unit %s | bounds %s .. %s | closed %s",axis->reference,axis->id ? axis->id:"none",axis->axis.type ? axis->axis.type:"unspecified",axis->axis.unit ? axis->axis.unit:"unspecified",axis->axis.minimum ? axis->axis.minimum:"unspecified",axis->axis.maximum ? axis->axis.maximum:"unspecified",axis->axis_closed ? "yes":"no");
                    if (igCollapsingHeader_TreeNodeFlags("Declared enum values",0)) for (j=0;j<axis->axis.allowed_count;++j) { igTextWrapped("%s",value); value+=strlen(value)+1; }
                }
                igPopID();
            }
        }
        case_details(context,binding->case_index);
    }
}
int gd_explorer_rules_inputs(GD_ExplorerContext *context) {
    if (!context->pane->rule_state) {
        GD_ExplorerRuleForm initial; gd_explorer_form_defaults(GD_FORM_RULES,&initial);
        context->pane->rule_state=empty_state(context->project->allocator,&initial);
    }
    return context->pane->rule_state!=NULL;
}
void gd_explorer_rules_controls(GD_ExplorerContext *context) {
    GD_ExplorerPane *pane=context->pane; GD_ExplorerRuleState *state; GD_ExplorerRuleForm *form; const GD_Rules *rules=context->project->rules; const GD_Model *model=context->project->models[GD_EXPLORER_RULE_DATA]; int i;
    if (!igCollapsingHeader_TreeNodeFlags("Object bindings / rule checks [native]",0)) return;
    form=gd_explorer_form_pending(context,GD_FORM_RULES,1); state=pane->rule_state;
    if (!form) { igTextWrapped("Cannot allocate rule controls."); return; }
    igTextWrapped("Whole supplied rules reader, independent of Resource routes, graph filters, camera and finite Actor hypotheses. Native evaluator/comparator only. Proposals are simultaneous, not execution or a conflict winner.");
    { int mode=form->bindings ? 2:form->kind==GD_NAMED_CONSISTENCY ? 1:0;
      if (igCombo_Str("Native inspection",&mode,gd_explorer_rule_modes,-1)) { gd_explorer_rule_mode(form,mode); }
    }
    { char saved_query_message[256]; memcpy(saved_query_message,pane->query_message,sizeof(saved_query_message)); *pane->query_message=0;
    if (form->bindings) {
        int before=gd_explorer_picker_case_index(model,rules,form->case_name),root,local=-1;
        gd_explorer_picker_record(context,"Object definition",GD_NOUN,form->object,sizeof(form->object));
        root=gd_explorer_picker_case(context,"Binding source case",form->case_name,sizeof(form->case_name),1); if (root!=before) *form->participant=0;
        if (root>=0) { const GD_RuleCase *item=&rules->cases[root]; for (i=item->first_participant;i<item->first_participant+item->participant_count;++i) if (!strcmp(rules->participants[i].name,form->participant)) local=i; local=gd_explorer_picker_local(context,"Local binding",root,local,1); if (local>=0) snprintf(form->participant,sizeof(form->participant),"%s",rules->participants[local].name); else *form->participant=0; }
        else igTextWrapped("Choose a source case to filter one local binding. Blank case keeps all cases; Advanced also permits cross-case exact-name filters.");
        gd_explorer_picker_record(context,"Axis definition",GD_AXIS,form->axis,sizeof(form->axis)); gd_explorer_picker_statuses(context,model,form->statuses,sizeof(form->statuses));
        if (igTreeNode_Str("Advanced: raw binding selectors")) { gd_explorer_form_widget(GD_FORM_RULES,form,"object",NULL); gd_explorer_form_widget(GD_FORM_RULES,form,"case_name","Source#case"); gd_explorer_form_widget(GD_FORM_RULES,form,"participant",NULL); gd_explorer_form_widget(GD_FORM_RULES,form,"axis",NULL); gd_explorer_form_widget(GD_FORM_RULES,form,"statuses",NULL); igTreePop(); }
        gd_explorer_form_widget(GD_FORM_RULES,form,"work_limit","Binding work ceiling 0..67108864");
    } else if (form->kind==GD_NAMED_OUTCOMES) {
        int before=gd_explorer_picker_case_index(model,rules,form->case_name),root=gd_explorer_picker_case(context,"Scenario source case",form->case_name,sizeof(form->case_name),form->kind==GD_NAMED_CONSISTENCY);
        if (root!=before) *form->facts=*form->contexts=0;
        gd_explorer_picker_assumptions(context,root,form->facts,sizeof(form->facts),form->contexts,sizeof(form->contexts),NULL);
        if (igTreeNode_Str("Advanced: raw source case")) { gd_explorer_form_widget(GD_FORM_RULES,form,"case_name",NULL); igTreePop(); }
        gd_explorer_form_widget(GD_FORM_RULES,form,"work_limit","Scenario work ceiling 0..16777216");
    } else igTextWrapped("Fixed native comparison ceiling %d; at most %d pairs / %d branches. Unexamined scope stays explicit.",GD_RULE_ANALYSIS_WORK_LIMIT,GD_MAX_RULE_COMPARISONS,GD_MAX_RULE_BRANCHES);
    if (*pane->query_message) snprintf(pane->rule_message,sizeof(pane->rule_message),"%s",pane->query_message); memcpy(pane->query_message,saved_query_message,sizeof(saved_query_message)); }
    if (gd_explorer_action_button(context,GD_ACTION_RULES_RUN)) { state=pane->rule_state; form=&state->pending; }
    igSameLine(0,8); if (gd_explorer_action_button(context,GD_ACTION_RULES_CLEAR)) return;
    if (*pane->rule_message) igTextWrapped("%s",pane->rule_message);
    if (!gd_explorer_form_equal(GD_FORM_RULES,&state->pending,&state->applied)) igTextWrapped("Pending inputs differ; evidence and declaration outlines remain last applied result.");
    if (state->bindings) { binding_details(context,state); return; }
    if (!state->result) return;
    { const GD_RuleAnalysis *analysis=state->result->analysis; int count=state->result->request.kind==GD_NAMED_OUTCOMES ? analysis->scenario.write_count:analysis->comparison_count,start;
      igTextWrapped("Applied %s | owned result/workspace %llu bytes + controls %llu bytes; borrowed reader excluded.",gd_named_name(state->result->request.kind),(unsigned long long)state->result->owned_bytes,(unsigned long long)sizeof(*state));
      if (igCollapsingHeader_TreeNodeFlags("Applied rule assumptions (not pending edits)",0)) { igTextWrapped("Facts: %s",*state->applied.facts ? state->applied.facts:"none supplied"); igTextWrapped("Contexts: %s",*state->applied.contexts ? state->applied.contexts:"none supplied"); }
      { int marked=0,present=0,visible=0; const GD_ResourceGraph *graph=context->project->resources;
        for (i=0;i<rules->case_count;++i) if (state->marked_cases[i]) ++marked;
        for (i=0;i<pane->layout->node_count;++i) { const GD_ResourceNode *node=&graph->nodes[pane->layout->nodes[i].key]; if (node->kind==GD_RESOURCE_CASE && !node->instance && gd_explorer_rules_marked(context,node->case_index)) { ++present; if (context->analysis->shown[i]) ++visible; } }
        igTextWrapped("Marked declarations %d | retained in Resource canvas %d | shown %d | Resource construction limited %s. Select proposal/pair for source outlines; display gaps do not erase native result.",marked,present,visible,graph->limited ? "yes":"no");
      }
      igTextWrapped("Purple outlines mark original case declarations and native proposal source paths, not distinct call placements or Resource flow/applicability. Hidden declarations remain in result. Use source inspector for full exact calls/maps/effects.");
      if (gd_explorer_similarity_active(context)) igTextWrapped("Ordinary declaration outlines are not shown on similarity canvas. Source inspection button returns to ordinary canvas without changing either layout.");
      if (state->result->request.kind==GD_NAMED_OUTCOMES) { const GD_RuleEvaluation *value=&analysis->scenario;
          igTextWrapped("Root truth %s | context %s | requirements %s | input %s | constraints %s | values %s",gd_rule_truth_name(value->applicability),gd_rule_truth_name(value->context),gd_rule_truth_name(value->requirements),value->input_valid==1 ? "valid":value->input_valid==0 ? "invalid":"unfinished",value->constraints_complete ? "complete":"incomplete",value->values_complete ? "complete":"incomplete");
          igTextWrapped("Exclusions %s | checked guards %d/%d | exclusion work %d | limited %s",value->exclusion.state==GD_EXCLUSION_ABSENT ? "absent":value->exclusion.state==GD_EXCLUSION_EXCLUDED ? "excluded":value->exclusion.state==GD_EXCLUSION_NOT_EXCLUDED ? "not excluded":"unknown",value->exclusion.checked_count,value->exclusion.guard_count,value->exclusion.work,value->exclusion.limited ? "yes":"no");
          igTextWrapped("Proposals %d | conflicts %d | uncertain pairs %d | omitted checks %d | opaque effects %d | expansions %d | cycles %d | work %d/%d | limited %s",value->write_count,value->conflict_count,value->uncertain_write_pairs,value->checks_omitted,value->opaque_effects,value->expansions,value->cycle_count,value->work,value->work_limit,value->limited ? "yes":"no");
          case_details(context,state->result->root);
      } else igTextWrapped("Checked pairs %d | retained %d | unexamined %d | work %d/%d | limited %s. Role alignment is hypothetical, not shared runtime identity. Conflict is existential; equivalence bounded by native checked vocabulary/branches.",analysis->pair_count,analysis->comparison_count,analysis->pairs_unexamined,analysis->comparison_work,GD_RULE_ANALYSIS_WORK_LIMIT,analysis->comparison_limited ? "yes":"no");
      if (count>64) igSliderInt("Rule result page",&state->page,0,(count-1)/64,"%d",0); if (state->page<0 || state->page*64>=count) state->page=0; start=state->page*64;
      for (i=start;i<count && i<start+64;++i) { igPushID_Int(i); if (state->result->request.kind==GD_NAMED_OUTCOMES) { const GD_RuleWrite *write=&analysis->scenario.writes[i]; if (igSelectable_Bool(TextFormat("%d: %s | %s | line %d",i,write->kind==GD_WRITE_STATE ? "state":write->kind==GD_WRITE_QUANTITY ? "quantity":"preserve",write->axis>=0 ? model->records[write->axis].reference:"unknown axis",write->line),state->row==i,0,(ImVec2_c){0,0})) choose(state,i,-1); }
          else { const GD_RuleComparisonResult *pair=&analysis->comparisons[i]; if (igSelectable_Bool(TextFormat("%d: %s / %s | %s",i,rules->cases[pair->first_case].name,rules->cases[pair->second_case].name,gd_rule_relation_name(pair->relation)),state->row==i,0,(ImVec2_c){0,0})) choose(state,i,-1); } igPopID(); }
      if (state->row>=0 && state->row<count) {
          if (state->result->request.kind==GD_NAMED_OUTCOMES) { proposal_details(context,&analysis->scenario.writes[state->row]); if (state->peer_write>=0 && state->peer_write<count) proposal_details(context,&analysis->scenario.writes[state->peer_write]); }
          else { const GD_RuleComparisonResult *pair=&analysis->comparisons[state->row];
              igTextWrapped("Reason %s | checked %d | overlap %d | matching %d | compatible %d | undetermined %d | unexamined %d",pair->reason ? pair->reason:"native result",pair->checked_branches,pair->overlap_branches,pair->matching_branches,pair->compatible_branches,pair->undetermined_branches,pair->unexamined_branches);
              igTextWrapped("Closed vocabularies %s | outcomes complete %s | numeric unexamined %s | actor scope equal %s | context scope equal %s | shared derivation %s",pair->vocabularies_closed ? "yes":"no",pair->outcomes_complete ? "yes":"no",pair->numeric_unexamined ? "yes":"no",pair->actor_scope_equal ? "yes":"no",pair->context_scope_equal ? "yes":"no",pair->shared_derivation ? "yes":"no");
              for (i=0;i<pair->witness_count;++i) { const GD_RuleWitnessFact *fact=&pair->witness[i]; igTextWrapped("Native witness: %s | %s = %s",fact->actor>=0 && fact->actor<rules->participant_count ? rules->participants[fact->actor].name:"unresolved role",fact->axis>=0 && fact->axis<model->record_count ? model->records[fact->axis].reference:"unresolved axis",fact->value); }
              case_details(context,pair->first_case); case_details(context,pair->second_case);
              if (pair->conflict_first>=0 && pair->conflict_second>=0) { igTextWrapped("Native existential conflicting proposals (no conflict winner):"); proposal_details(context,&pair->conflict_a); proposal_details(context,&pair->conflict_b); }
          }
      }
      if (state->result->request.kind==GD_NAMED_OUTCOMES && analysis->scenario.checks_count && igCollapsingHeader_TreeNodeFlags("Checked simultaneous proposal pairs",0)) for (i=0;i<analysis->scenario.checks_count;++i) { const GD_RuleWriteCheck *check=&analysis->scenario.checks[i]; igPushID_Int(i); if (igSelectable_Bool(TextFormat("%d / %d: conflict %s",check->first,check->second,gd_rule_truth_name(check->conflict)),false,0,(ImVec2_c){0,0})) choose(state,check->first,check->second); igPopID(); }
    }
}
