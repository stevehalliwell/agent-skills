#include "gd_query.h"
#include <string.h>

static const char *const names[]={"object-bindings","actor-states","effect-contributors"};
static const char *const projections[]={"actors","actors","effects"};
const char *gd_query_name(GD_QueryKind kind) { return kind>=0 && kind<GD_QUERY_COUNT ? names[kind]:NULL; }
const char *gd_query_projection(GD_QueryKind kind) { return kind>=0 && kind<GD_QUERY_COUNT ? projections[kind]:NULL; }
int gd_query_kind(const char *name) { int i; if (name) for (i=0;i<GD_QUERY_COUNT;++i) if (!strcmp(name,names[i])) return i; return -1; }
int gd_query_record(const GD_Model *model, const char *reference, GD_Role role) {
    int i,found=-1;
    if (!model || !reference) return -1;
    for (i=0;i<model->record_count;++i) if (model->records[i].role==role && !strcmp(model->records[i].reference,reference)) { if (found>=0) return -2; found=i; }
    return found;
}
static int status_matches(const GD_QueryRequest *request, const GD_Record *record) {
    int i;
    if (!request->status_count || !strcmp(request->statuses[0],"all")) return 1;
    for (i=0;i<request->status_count;++i) if (record->status && !strcmp(request->statuses[i],record->status)) return 1;
    return 0;
}
static int case_matches(const GD_Query *query, int index) {
    return status_matches(&query->request,&query->model->records[query->rules->sources[query->rules->cases[index].source].record]);
}
static int charge(GD_Query *query, int amount) {
    if (amount>query->request.work_limit-query->work) { query->limited=1; return 0; }
    query->work+=amount; return 1;
}
static int axis_applies(const GD_Rules *rules, int participant, int axis, GD_Query *budget) {
    int edge; const GD_RuleParticipant *actor=&rules->participants[participant];
    if (actor->noun_resolution!=GD_RULE_RESOLVED) return 0;
    for (edge=rules->actor_first[actor->noun_record];edge>=0;edge=rules->next_axis[edge]) {
        if (budget && !charge(budget,1)) return -1;
        if (rules->axis_resolution[edge]==GD_RULE_RESOLVED && rules->model->edges[edge].target==axis) return 1;
    }
    return 0;
}
static int text_within(const char *text, size_t maximum) { size_t i=0; if (!text || !*text) return 0; while (i<=maximum && text[i]) ++i; return i<=maximum; }
GD_Query *gd_query_build(GD_Model *model, const GD_Rules *rules, const GD_RuleAnalysis *analysis, const GD_QueryRequest *request) {
    GD_Query *query; int i,p;
    if (!model) return NULL;
    if (!rules || rules->model!=model || !request || request->kind<0 || request->kind>=GD_QUERY_COUNT ||
        request->case_index<-1 || request->case_index>=rules->case_count || request->noun<-1 || request->noun>=model->record_count ||
        (request->noun>=0 && model->records[request->noun].role!=GD_NOUN) || request->axis<-1 || request->axis>=model->record_count ||
        (request->axis>=0 && (model->records[request->axis].role!=GD_AXIS || !rules->axes[request->axis].valid)) ||
        request->effect_kind<-1 || request->effect_kind==GD_EFFECT_ACTIVATION || request->effect_kind>GD_EFFECT_UNKNOWN || request->value_count<0 || request->value_count>GD_ACTOR_VARIANTS ||
        request->status_count<0 || request->status_count>GD_MAX_CONTROL_FILTERS || request->work_limit<0 || request->work_limit>GD_EFFECT_WORK || (request->fixed && (request->fixed->fact_count<0 || request->fixed->fact_count>GD_MAX_RULE_CONDITION_ITEMS || request->fixed->context_count<0 || request->fixed->context_count>GD_MAX_RULE_CONTEXT_FACTS))) {
        gd_error(model,"query.arguments","Invalid typed query, snapshot, selector or native bound",NULL,0); return NULL;
    }
    for (i=0;i<request->status_count;++i) if (!text_within(request->statuses[i],128) || (request->status_count>1 && !strcmp(request->statuses[i],"all"))) {
        gd_error(model,"query.status","Status values must be nonempty; all must stand alone",NULL,0); return NULL;
    }
    for (i=0;i<request->value_count;++i) if (!text_within(request->values[i],GD_MAX_RULE_LINE)) { gd_error(model,"query.value","Values require nonempty bounded literals",NULL,0); return NULL; }
    if (request->participant && !text_within(request->participant,128)) { gd_error(model,"query.binding","Participant selector exceeds native bound",NULL,0); return NULL; }
    if (request->kind!=GD_QUERY_BINDINGS && (!analysis || analysis->model!=model || analysis->rules!=rules)) { gd_error(model,"query.analysis","Query requires matching compiled native rule analysis",NULL,0); return NULL; }
    if ((request->kind==GD_QUERY_BINDINGS && (request->value_count || request->target_id || request->effect_kind>=0 || request->fixed)) ||
        (request->kind==GD_QUERY_ACTOR_STATES && (request->target_id || request->effect_kind>=0 || request->case_index<0 || request->axis<0)) ||
        (request->kind==GD_QUERY_CONTRIBUTORS && (request->participant || request->fixed || request->value_count>1 || (request->target_id && (request->axis>=0 || request->value_count || request->effect_kind>=0))))) {
        gd_error(model,"query.parameters","Parameters do not match operation; inspect query --list",NULL,0); return NULL;
    }
    query=model->allocator.allocate(model->allocator.context,sizeof(*query));
    if (!query) { gd_error(model,"query.memory","Cannot allocate bounded query adapter",NULL,0); return NULL; }
    memset(query,0,sizeof(*query)); query->allocator=model->allocator; query->model=model; query->rules=rules; query->analysis=analysis; query->request=*request;
    if (request->kind==GD_QUERY_BINDINGS) {
        for (i=0;i<rules->case_count && !query->limited;++i) {
            const GD_RuleCase *item=&rules->cases[i];
            if (!charge(query,1)) break;
            if ((request->case_index>=0 && request->case_index!=i) || !case_matches(query,i)) { ++query->excluded_roots; continue; }
            for (p=item->first_participant;p<item->first_participant+item->participant_count;++p) {
                const GD_RuleParticipant *participant=&rules->participants[p];
                if (!charge(query,1)) break;
                if (request->noun>=0 && (participant->noun_resolution!=GD_RULE_RESOLVED || participant->noun_record!=request->noun)) continue;
                if (request->participant && strcmp(request->participant,participant->name)) continue;
                if (request->axis>=0) { int applicable=axis_applies(rules,p,request->axis,query); if (applicable<0) break; if (!applicable) continue; }
                query->bindings[query->binding_count++]=(GD_QueryBinding){i,p}; query->cases[i]=1;
            }
        }
        if (query->limited) query->unexamined_bindings=rules->participant_count; /* Conservative upper bound, not an exact missing-match count. */
    } else if (request->kind==GD_QUERY_ACTOR_STATES) {
        const GD_RuleCase *item=&rules->cases[request->case_index]; int participant=item->primary_participant;
        GD_ActorSelection selection={0};
        if (request->participant) {
            participant=-1;
            for (p=item->first_participant;p<item->first_participant+item->participant_count;++p) if (!strcmp(request->participant,rules->participants[p].name)) { if (participant>=0) { participant=-2; break; } participant=p; }
        }
        if (participant<0 || (request->noun>=0 && (rules->participants[participant].noun_resolution!=GD_RULE_RESOLVED || rules->participants[participant].noun_record!=request->noun))) {
            gd_error(model,"query.binding","Select one exact local participant or authored A matching optional object; no same-noun/order fallback",NULL,0); goto failure;
        }
        if (!axis_applies(rules,participant,request->axis,NULL)) { gd_error(model,"query.axis","Selected axis is not explicitly applicable to selected binding",NULL,0); goto failure; }
        if (!case_matches(query,request->case_index)) { query->excluded_roots=1; return query; }
        selection.case_index=request->case_index; selection.participant=participant; selection.axis=request->axis; selection.value_count=request->value_count; selection.fixed=request->fixed;
        for (i=0;i<request->value_count;++i) selection.values[i]=request->values[i];
        query->variants=gd_actor_query_build(model,analysis,&selection,request->work_limit); if (!query->variants) goto failure;
        query->cases[request->case_index]=1;
        for (i=0;i<query->variants->activation_count;++i) query->cases[query->variants->activations[i].case_index]=1;
    } else {
        GD_EffectSelection selection={-1,request->case_index,request->noun,request->axis,request->effect_kind,NULL,query->activation_mask};
        if (request->axis>=0 && ((request->effect_kind==GD_EFFECT_STATE && rules->axes[request->axis].type!=GD_AXIS_ENUM) ||
            ((request->effect_kind==GD_EFFECT_GAIN || request->effect_kind==GD_EFFECT_LOSS || request->effect_kind==GD_EFFECT_NO_CHANGE || request->effect_kind==GD_EFFECT_QUANTITY) && rules->axes[request->axis].type!=GD_AXIS_INTEGER && rules->axes[request->axis].type!=GD_AXIS_NUMBER))) {
            gd_error(model,"query.effect_type","Effect family incompatible with selected typed axis",NULL,0); goto failure;
        }
        if (request->value_count) {
            const GD_ValueDefinition *definition; const char *value; int known=0;
            if (request->axis<0 || rules->axes[request->axis].type!=GD_AXIS_ENUM || (request->effect_kind>=0 && request->effect_kind!=GD_EFFECT_STATE) || !request->values[0] || !*request->values[0]) {
                gd_error(model,"query.value","Effect --value needs one named value and enum --axis",NULL,0); goto failure;
            }
            definition=&model->records[request->axis].axis; value=definition->allowed;
            for (i=0;i<definition->allowed_count;++i) { if (!strcmp(value,request->values[0])) known=1; value+=strlen(value)+1; }
            if (!known && model->records[request->axis].axis_closed) { gd_error(model,"query.value","Value absent from closed axis vocabulary",NULL,0); goto failure; }
            selection.value=request->values[0]; selection.kind=GD_EFFECT_STATE;
        }
        query->resource=gd_resource_graph_build(model,rules);
        if (!query->resource) { if (!model->error_count) gd_error(model,"query.memory","Cannot allocate Resource source for effect query",NULL,0); goto failure; }
        /* False also means retained bounded partial construction, not allocation
           failure. Matching snapshot/one-bind preconditions are established here. */
        (void)gd_resource_graph_bind_calls(query->resource,analysis);
        query->effects=gd_effect_graph_build(query->resource,GD_EFFECT_WORK);
        if (!query->effects) { gd_error(model,"query.memory","Cannot construct effect projection",NULL,0); goto failure; }
        if (request->target_id) {
            selection.target=gd_effect_select(query->effects,request->target_id,GD_EFFECT_WORK);
            if (selection.target<0) { gd_error(model,selection.target==-2 ? "query.selector_ambiguous":selection.target==-3 ? "query.selector_limit":"query.selector_missing","Effect ID missing, ambiguous or limited; inspect concepts with same bindings",NULL,0); goto failure; }
            if (query->effects->nodes[selection.target].kind==GD_EFFECT_ACTIVATION) { gd_error(model,"query.selector_type","Contributor target must be result concept, not activation",NULL,0); goto failure; }
        }
        for (i=0;i<query->effects->node_count;++i) if (query->effects->nodes[i].kind==GD_EFFECT_ACTIVATION) query->activation_mask[i]=(unsigned char)case_matches(query,query->resource->nodes[query->effects->nodes[i].activation].case_index);
        query->contributors=gd_effect_query_build(query->effects,selection,request->work_limit);
        if (!query->contributors) { gd_error(model,"query.memory","Cannot allocate effect contributor query",NULL,0); goto failure; }
        for (i=0;i<query->effects->node_count;++i) if (query->contributors->context[i]) query->cases[query->resource->nodes[query->effects->nodes[i].activation].case_index]=1;
    }
    return query;
failure:
    gd_query_destroy(query); return NULL;
}
void gd_query_destroy(GD_Query *query) {
    if (!query) return;
    gd_effect_query_destroy(query->contributors); gd_effect_graph_destroy(query->effects); gd_resource_graph_destroy(query->resource); gd_actor_query_destroy(query->variants);
    query->allocator.release(query->allocator.context,query);
}
