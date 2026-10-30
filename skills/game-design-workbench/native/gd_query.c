#include "gd_query.h"
#include <string.h>

static const char *const names[]={"object-bindings","actor-states","effect-contributors","state-coverage"};
static const char *const projections[]={"actors","actors","effects","actors"};
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
static int local_participant(const GD_Rules *rules, int case_index, const char *name) {
    const GD_RuleCase *item=&rules->cases[case_index]; int p,found=-1;
    if (!name) return item->primary_participant;
    for (p=item->first_participant;p<item->first_participant+item->participant_count;++p) if (!strcmp(name,rules->participants[p].name)) { if (found>=0) return -2; found=p; }
    return found;
}
static int text_within(const char *text, size_t maximum) { size_t i=0; if (!text || !*text) return 0; while (i<=maximum && text[i]) ++i; return i<=maximum; }
static int coverage_values(GD_Model *model, GD_Query *query) {
    GD_StateCoverage *coverage=query->state_coverage; const GD_QueryRequest *request=&query->request;
    const GD_ValueDefinition *definition=&model->records[request->axis].axis; const char *allowed=definition->allowed; int i,j;
    if (!request->value_count && query->rules->axes[request->axis].type!=GD_AXIS_ENUM) { gd_error(model,"coverage.values","Numeric coverage requires finite explicit typed values",NULL,0); return 0; }
    if (request->value_count) for (i=0;i<request->value_count;++i) coverage->values[coverage->value_count++].value=request->values[i];
    else for (i=0;i<definition->allowed_count;++i) { for (j=0;j<coverage->value_count;++j) if (!strcmp(allowed,coverage->values[j].value)) break; if (j==coverage->value_count) { if (coverage->value_count==GD_ACTOR_VARIANTS) { gd_error(model,"coverage.values","Declared enum exceeds finite value bound; select explicit values",NULL,0); return 0; } coverage->values[coverage->value_count++].value=allowed; } allowed+=strlen(allowed)+1; }
    for (i=0;i<coverage->value_count;++i) {
        GD_StateCoverageValue *item=&coverage->values[i]; double number=0; allowed=definition->allowed;
        if (!text_within(item->value,GD_MAX_RULE_LINE)) { gd_error(model,"coverage.value","Coverage value requires bounded nonempty native literal",NULL,0); return 0; }
        for (j=0;j<definition->allowed_count;++j) { if (!strcmp(allowed,item->value)) item->declared=1; allowed+=strlen(allowed)+1; }
        if (query->rules->axes[request->axis].type==GD_AXIS_ENUM) { if (!item->declared && model->records[request->axis].axis_closed) { gd_error(model,"coverage.value","Value absent from closed enum vocabulary",NULL,0); return 0; } }
        else if (!gd_control_number(item->value,&number,query->rules->axes[request->axis].type==GD_AXIS_INTEGER)) { gd_error(model,"coverage.value","Numeric value must be finite declared-type literal in axis unit",NULL,0); return 0; }
        for (j=0;j<i;++j) {
            int duplicate;
            if (query->rules->axes[request->axis].type==GD_AXIS_ENUM) duplicate=!strcmp(item->value,coverage->values[j].value);
            else { double previous; gd_control_number(coverage->values[j].value,&previous,query->rules->axes[request->axis].type==GD_AXIS_INTEGER); duplicate=previous==number; }
            if (duplicate) { gd_error(model,"coverage.duplicate_value","Finite coverage list repeats same typed value",NULL,0); return 0; }
        }
    }
    return 1;
}
static int state_coverage(GD_Model *model, GD_Query *query) {
    GD_StateCoverage *coverage; uint64_t all; int i,j;
    query->state_coverage=model->allocator.allocate(model->allocator.context,sizeof(*coverage)); if (!query->state_coverage) { gd_error(model,"coverage.memory","Cannot allocate bounded finite state coverage",NULL,0); return 0; }
    coverage=query->state_coverage; memset(coverage,0,sizeof(*coverage));
    if (!coverage_values(model,query)) return 0;
    coverage->source_incomplete=!query->rules->bindings_complete || !query->rules->axis_definitions_complete || !query->rules->condition_bindings_complete || query->rules->findings_omitted;
    if (query->request.case_index<0) for (i=0;i<query->rules->source_count;++i) if (status_matches(&query->request,&model->records[query->rules->sources[i].record]) && query->rules->sources[i].state!=GD_RULE_PARSED) coverage->source_incomplete=1;
    for (i=0;i<query->rules->case_count;++i) if ((query->request.case_index<0 || query->request.case_index==i) && case_matches(query,i) && !query->rules->cases[i].participant_count) coverage->source_incomplete=1;
    all=coverage->value_count==64 ? UINT64_MAX:(UINT64_C(1)<<coverage->value_count)-1;
    /* Reuse native finite variants, including whole-case guards/exclusions.
       Outcome work is conservative overhead, not a second applicability engine. */
    for (i=0;i<query->binding_count && coverage->value_count;++i) {
        const GD_QueryBinding *binding=&query->bindings[i]; GD_ActorSelection selection={0}; GD_ActorQuery *variants; int remaining=query->request.work_limit-query->work-coverage->work;
        if (remaining<=0) { coverage->limited=1; break; }
        selection.case_index=binding->case_index; selection.participant=binding->participant; selection.axis=query->request.axis; selection.value_count=coverage->value_count;
        for (j=0;j<coverage->value_count;++j) selection.values[j]=coverage->values[j].value;
        variants=gd_actor_query_build(model,query->analysis,&selection,remaining); if (!variants) return 0;
        coverage->work+=variants->work; ++coverage->evaluated_bindings;
        for (j=0;j<variants->variant_count;++j) {
            const GD_ActorVariant *variant=&variants->variants[j]; uint64_t bit=UINT64_C(1)<<j;
            coverage->present[i]|=bit;
            if (variant->applicability==GD_RULE_TRUTH_TRUE) coverage->definite[i]|=bit;
            else if (variant->applicability==GD_RULE_TRUTH_UNKNOWN) coverage->undetermined[i]|=bit;
            if (!variant->limited) coverage->finished[i]|=bit;
        }
        coverage->limited=variants->limited; gd_actor_query_destroy(variants); if (coverage->limited) break;
    }
    for (i=0;i<query->binding_count;++i) {
        const GD_QueryBinding *binding=&query->bindings[i]; coverage->case_unfinished[binding->case_index]|=all^coverage->finished[i];
        coverage->case_definite[binding->case_index]|=coverage->definite[i]&coverage->finished[i];
        coverage->case_undetermined[binding->case_index]|=coverage->undetermined[i]&coverage->finished[i];
        for (j=0;j<coverage->value_count;++j) {
            uint64_t bit=UINT64_C(1)<<j; GD_StateCoverageValue *value=&coverage->values[j];
            if (!(coverage->finished[i]&bit)) { ++value->unfinished_bindings; continue; }
            if (coverage->definite[i]&bit) ++value->definite_bindings; else if (coverage->undetermined[i]&bit) ++value->undetermined_bindings; else if (coverage->present[i]&bit) ++value->excluded_bindings;
        }
    }
    for (i=0;i<query->rules->case_count;++i) if (query->cases[i]) for (j=0;j<coverage->value_count;++j) {
        uint64_t bit=UINT64_C(1)<<j; GD_StateCoverageValue *value=&coverage->values[j];
        if (coverage->case_unfinished[i]&bit) ++value->unfinished_cases;
        if (coverage->case_definite[i]&bit) ++value->definite_cases; else if (coverage->case_undetermined[i]&bit) ++value->undetermined_cases; else if (!(coverage->case_unfinished[i]&bit)) ++value->excluded_cases;
    }
    for (j=0;j<coverage->value_count;++j) {
        GD_StateCoverageValue *value=&coverage->values[j];
        value->no_retained_candidate=value->definite_cases || value->undetermined_cases ? GD_RULE_TRUTH_FALSE : query->limited || coverage->limited || coverage->source_incomplete ? GD_RULE_TRUTH_UNKNOWN : GD_RULE_TRUTH_TRUE;
    }
    return 1;
}
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
        (request->kind==GD_QUERY_STATE_COVERAGE && (request->axis<0 || request->target_id || request->effect_kind>=0 || request->fixed || (request->participant && request->case_index<0))) ||
        (request->kind==GD_QUERY_CONTRIBUTORS && (request->participant || request->fixed || request->value_count>1 || (request->target_id && (request->axis>=0 || request->value_count || request->effect_kind>=0))))) {
        gd_error(model,"query.parameters","Parameters do not match operation; inspect query --list",NULL,0); return NULL;
    }
    if (request->kind==GD_QUERY_STATE_COVERAGE && rules->axes[request->axis].type!=GD_AXIS_ENUM && rules->axes[request->axis].type!=GD_AXIS_INTEGER && rules->axes[request->axis].type!=GD_AXIS_NUMBER) { gd_error(model,"coverage.axis_type","Coverage requires explicit enum, integer or number axis type",NULL,0); return NULL; }
    if (request->kind==GD_QUERY_STATE_COVERAGE && request->participant) {
        p=local_participant(rules,request->case_index,request->participant);
        if (p<0 || (request->noun>=0 && (rules->participants[p].noun_resolution!=GD_RULE_RESOLVED || rules->participants[p].noun_record!=request->noun))) { gd_error(model,"query.binding","Select one exact local participant matching optional object",NULL,0); return NULL; }
        if (!axis_applies(rules,p,request->axis,NULL)) { gd_error(model,"query.axis","Selected axis is not explicitly applicable to selected binding",NULL,0); return NULL; }
    }
    query=model->allocator.allocate(model->allocator.context,sizeof(*query));
    if (!query) { gd_error(model,"query.memory","Cannot allocate bounded query adapter",NULL,0); return NULL; }
    memset(query,0,sizeof(*query)); query->allocator=model->allocator; query->model=model; query->rules=rules; query->analysis=analysis; query->request=*request;
    if (request->kind==GD_QUERY_BINDINGS || request->kind==GD_QUERY_STATE_COVERAGE) {
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
        if (request->kind==GD_QUERY_STATE_COVERAGE && !state_coverage(model,query)) goto failure;
    } else if (request->kind==GD_QUERY_ACTOR_STATES) {
        int participant=local_participant(rules,request->case_index,request->participant);
        GD_ActorSelection selection={0};
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
    if (query->state_coverage) query->allocator.release(query->allocator.context,query->state_coverage);
    gd_effect_query_destroy(query->contributors); gd_effect_graph_destroy(query->effects); gd_resource_graph_destroy(query->resource); gd_actor_query_destroy(query->variants);
    query->allocator.release(query->allocator.context,query);
}
