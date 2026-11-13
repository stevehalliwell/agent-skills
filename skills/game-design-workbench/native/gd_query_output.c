#include "gd_query.h"
#include "gd_structure_cli.h"
#include "gd_named_query.h"
#include <math.h>
#include <string.h>

static void member(FILE *out, const char *key, const char *value) { fputc(',',out); gd_json_string(out,key); fputc(':',out); gd_json_string(out,value); }
static int selected(const char *fields, const char *key) {
    size_t size=strlen(key); const char *p=fields;
    if (!p) return 1;
    while (*p) { const char *end=strchr(p,','); size_t length=end ? (size_t)(end-p):strlen(p); if (length==size && !memcmp(p,key,size)) return 1; if (!end) break; p=end+1; }
    return 0;
}
int gd_query_fields(GD_QueryKind kind, const char *fields) {
    const char *p=fields;
    if (kind<0 || kind>=GD_QUERY_COUNT) return 0;
    if (!p) return 1;
    if (!*p) return 0;
    while (*p) {
        const char *end=strchr(p,','); size_t size=end ? (size_t)(end-p):strlen(p); char key[32]; int valid;
        if (!size || size>=sizeof(key)) return 0;
        memcpy(key,p,size); key[size]=0;
        valid=!strcmp(key,"cases") || !strcmp(key,"findings") || (kind==GD_QUERY_BINDINGS && !strcmp(key,"bindings")) ||
            (kind==GD_QUERY_ACTOR_STATES && (!strcmp(key,"variants") || !strcmp(key,"comparisons"))) ||
            (kind==GD_QUERY_CONTRIBUTORS && (!strcmp(key,"concepts") || !strcmp(key,"contributors"))) ||
            (kind==GD_QUERY_STATE_COVERAGE && (!strcmp(key,"axis_definition") || !strcmp(key,"state_values") || !strcmp(key,"state_bindings")));
        if (!valid) return 0;
        if (!end) return 1; p=end+1; if (!*p) return 0;
    }
    return 1;
}
static void source_case(FILE *out, const GD_Query *query, int index) {
    const GD_RuleCase *item=&query->rules->cases[index]; const GD_Record *record=&query->model->records[query->rules->sources[item->source].record]; char qualified[GD_MAX_PATH];
    int count=snprintf(qualified,sizeof(qualified),"%s#%s",record->reference,item->name);
    fputs("{\"record\":",out); gd_json_string(out,record->reference); member(out,"record_id",record->id); member(out,"case",item->name);
    member(out,"qualified",count>=0 && count<(int)sizeof(qualified) ? qualified:NULL); member(out,"source_path",record->source_path); member(out,"status",record->status); member(out,"coverage",record->coverage); fprintf(out,",\"line\":%d,\"snapshot_case_index\":%d}",item->line,index);
}
static void actor(FILE *out, const GD_Query *query, int index) {
    const GD_RuleParticipant *item;
    if (index<0 || index>=query->rules->participant_count) { fputs("null",out); return; }
    item=&query->rules->participants[index]; fputs("{\"participant\":",out); gd_json_string(out,item->name); member(out,"noun",item->noun);
    fputs(",\"is_actor\":",out);
    if (item->noun_resolution==GD_RULE_RESOLVED && query->model->records[item->noun_record].is_actor_line)
        fputs(query->model->records[item->noun_record].is_actor ? "true":"false",out);
    else fputs("null",out);
    member(out,"noun_resolution",gd_rule_resolution_name(item->noun_resolution)); member(out,"noun_reference",item->noun_resolution==GD_RULE_RESOLVED ? query->model->records[item->noun_record].reference:NULL); member(out,"noun_record_id",item->noun_resolution==GD_RULE_RESOLVED ? query->model->records[item->noun_record].id:NULL); fprintf(out,",\"line\":%d,\"snapshot_participant_index\":%d}",item->line,index);
}
static void selectors(FILE *out, const GD_Query *query) {
    const GD_QueryRequest *request=&query->request; int i;
    fputs("{\"case\":",out); if (request->case_index>=0) source_case(out,query,request->case_index); else fputs("null",out);
    member(out,"participant",query->variants ? query->rules->participants[query->variants->participant].name:request->participant);
    member(out,"object",request->noun>=0 ? query->model->records[request->noun].reference:NULL);
    member(out,"axis",request->axis>=0 ? query->model->records[request->axis].reference:NULL); member(out,"to",request->target_id);
    member(out,"effect",request->effect_kind>=0 ? gd_effect_kind_name((GD_EffectKind)request->effect_kind):NULL);
    fputs(",\"values\":[",out); for (i=0;i<request->value_count;++i) { if (i) fputc(',',out); gd_json_string(out,request->values[i]); }
    fputs("],\"statuses\":[",out); for (i=0;i<request->status_count;++i) { if (i) fputc(',',out); gd_json_string(out,request->statuses[i]); }
    fprintf(out,"],\"query_work_limit\":%d,\"fixed_facts\":[",request->work_limit);
    if (request->fixed) for (i=0;i<request->fixed->fact_count;++i) {
        const GD_RulePredicateFact *fact=&request->fixed->facts[i]; if (i) fputc(',',out);
        fputs("{\"actor\":",out); actor(out,query,fact->participant); member(out,"axis",fact->axis_record>=0 && fact->axis_record<query->model->record_count ? query->model->records[fact->axis_record].reference:NULL);
        member(out,"value",fact->value); member(out,"truth",gd_rule_truth_name(fact->truth)); fputc('}',out);
    }
    fputs("],\"fixed_context\":[",out);
    if (request->fixed) for (i=0;i<request->fixed->context_count;++i) {
        const GD_RuleContextFact *fact=&request->fixed->contexts[i]; if (i) fputc(',',out);
        fputs("{\"left\":",out); actor(out,query,fact->left); fputs(",\"right\":",out); actor(out,query,fact->right);
        member(out,"axis",fact->axis>=0 && fact->axis<query->model->record_count ? query->model->records[fact->axis].reference:NULL); member(out,"value",fact->value); member(out,"truth",gd_rule_truth_name(fact->truth)); fputc('}',out);
    }
    fputs("]}",out);
}
static void effect_id(FILE *out, const GD_EffectGraph *graph, int index) {
    char id[GD_EFFECT_ID_CAPACITY]; gd_json_string(out,gd_effect_node_id(graph,index,id,sizeof(id)) ? id:NULL);
}
static void activation(FILE *out, const GD_Query *query, int index) {
    if(ferror(out))return;
    const GD_EffectGraph *graph=query->effects; const GD_ResourceNode *node=&query->resource->nodes[graph->nodes[index].activation]; const GD_RuleCase *item=&query->rules->cases[node->case_index]; int p;
    fputs("{\"id\":",out); effect_id(out,graph,index); fputs(",\"source\":",out); source_case(out,query,node->case_index);
    fprintf(out,",\"instantiated\":%s,\"binding_conflict\":%s,\"frontier\":%s,\"mapping\":[",node->instance ? "true":"false",node->binding_conflict ? "true":"false",node->frontier ? "true":"false");
    for (p=item->first_participant;p<item->first_participant+item->participant_count && !ferror(out);++p) {
        int mapped=gd_resource_instance_actor(query->resource,graph->nodes[index].activation,p);
        if (p!=item->first_participant) fputc(',',out);
        fputs("{\"template\":",out); actor(out,query,p); fputs(",\"concrete\":",out); actor(out,query,mapped); fputc('}',out);
    }
    fputc(']',out);
    if (node->instance) {
        const GD_RuleCall *call=&query->analysis->calls[node->call];
        fputs(",\"call_source\":",out); source_case(out,query,call->caller_case); fprintf(out,",\"call_line\":%d,\"call_ordinal\":%d,\"depth\":%d",call->line,node->ordinal,node->depth);
    }
    fputc('}',out);
}
static void variant(FILE *out, const GD_Query *query, int index) {
    if(ferror(out))return;
    const GD_ActorQuery *variants=query->variants; const GD_ActorVariant *item=&variants->variants[index]; int i;
    fprintf(out,"{\"index\":%d,\"value\":",index); gd_json_string(out,item->value); member(out,"applicability",gd_rule_truth_name(item->applicability));
    member(out,"context",gd_rule_truth_name(item->context)); member(out,"requirements",gd_rule_truth_name(item->requirements));
    fprintf(out,",\"exclusion_state\":%d,\"guards\":%d,\"checked_guards\":%d,\"constraints_complete\":%s,\"values_complete\":%s,\"limited\":%s,\"opaque_effects\":%d,\"cycles\":%d,\"conflicts\":%d,\"uncertain_pairs\":%d,\"evaluation_work\":%d,\"activations\":[",item->exclusion.state,item->exclusion.guard_count,item->exclusion.checked_count,item->constraints_complete ? "true":"false",item->values_complete ? "true":"false",item->limited ? "true":"false",item->opaque_effects,item->cycles,item->conflicts,item->uncertain_pairs,item->work);
    for (i=item->first_activation;i<item->first_activation+item->activation_count && !ferror(out);++i) {
        const GD_RuleActivation *entry=&variants->activations[i]; int step;
        if (i!=item->first_activation) fputc(',',out);
        fprintf(out,"{\"index\":%d,\"snapshot_activation_index\":%d,\"source\":",i-item->first_activation,i); source_case(out,query,entry->case_index); member(out,"applicability",gd_rule_truth_name(entry->applicability)); member(out,"local_applicability",gd_rule_truth_name(entry->local_applicability));
        fputs(",\"via\":[",out);
        for (step=0;step<entry->path_count && !ferror(out);++step) {
            if (step) fputc(',',out); fputs("{\"source\":",out); source_case(out,query,entry->path[step]);
            if (entry->calls[step]>=0) { const GD_RuleCall *call=&query->analysis->calls[entry->calls[step]]; fprintf(out,",\"call_snapshot_index\":%d,\"call_line\":%d",entry->calls[step],call->line); fputs(",\"call_text\":",out); gd_json_string_n(out,call->text,call->length); }
            fputc('}',out);
        }
        fputs("]}",out);
    }
    fputs("],\"proposals\":[",out);
    for (i=item->first_write;i<item->first_write+item->write_count && !ferror(out);++i) {
        const GD_RuleWrite *write=&variants->writes[i];
        if (i!=item->first_write) fputc(',',out);
        fprintf(out,"{\"index\":%d,\"activation\":%d,\"declaration_index\":%d,\"family\":",i-item->first_write,variants->write_activations[i]-item->first_activation,write->declaration);
        gd_json_string(out,gd_effect_kind_name(gd_actor_write_family(variants,index,write))); fputs(",\"affected\":",out); actor(out,query,write->actor); member(out,"declared_unit",write->axis>=0 ? query->model->records[write->axis].axis.unit:NULL);
        fputs(",\"proposal\":",out); gd_write_rule_proposal(out,query->analysis,write); fputc('}',out);
    }
    fputs("],\"write_checks\":[",out);
    for (i=item->first_check;i<item->first_check+item->check_count && !ferror(out);++i) {
        const GD_RuleWriteCheck *check=&variants->checks[i]; if (i!=item->first_check) fputc(',',out);
        fprintf(out,"{\"first_proposal\":%d,\"second_proposal\":%d,\"conflict\":",check->first-item->first_write,check->second-item->first_write); gd_json_string(out,gd_rule_truth_name(check->conflict)); fputc('}',out);
    }
    fprintf(out,"],\"checks_omitted\":%d,\"partners\":[",item->checks_omitted);
    {
        int comma=0;
        for (i=0;i<query->rules->participant_count && !ferror(out);++i) if (item->partners[i/64] & (1ULL<<(i%64))) { if (comma++) fputc(',',out); actor(out,query,i); }
    }
    fputs("],\"effect_concepts\":[",out);
    {
        int comma=0;
        for (i=0;i<variants->effect_count && !ferror(out);++i) if (item->effects[i/64] & (1ULL<<(i%64))) {
            const GD_ActorEffect *effect=&variants->effects[i];
            if (comma++) fputc(',',out); fprintf(out,"{\"index\":%d,\"kind\":",i); gd_json_string(out,gd_effect_kind_name(effect->kind));
            member(out,"axis",effect->axis>=0 ? query->model->records[effect->axis].reference:NULL); member(out,"value",effect->state);
            if (effect->origin_case>=0) { fputs(",\"declaration_source\":",out); source_case(out,query,effect->origin_case); fprintf(out,",\"declaration_index\":%d,\"declaration_kind\":%d,\"snapshot_activation_index\":%d",effect->declaration,effect->declaration_kind,effect->activation); }
            fputc('}',out);
        }
    }
    fputs("]}",out);
}
static void comparison_set(FILE *out, GD_ActorSetComparison value, const unsigned long long *first, const unsigned long long *second, int count) {
    int category,i;
    fprintf(out,"{\"shared\":%d,\"first_only\":%d,\"second_only\":%d",value.shared,value.first_only,value.second_only);
    for (category=0;category<3;++category) {
        int comma=0; fputs(category==0 ? ",\"shared_members\":[":category==1 ? ",\"first_only_members\":[":",\"second_only_members\":[",out);
        for (i=0;i<count;++i) {
            int a=!!(first[i/64] & (1ULL<<(i%64))),b=!!(second[i/64] & (1ULL<<(i%64)));
            if (category==0 ? a && b:category==1 ? a && !b:!a && b) { if (comma++) fputc(',',out); fprintf(out,"%d",i); }
        }
        fputc(']',out);
    }
    fputc('}',out);
}
static int concept_matches(const GD_Query *query, int index) {
    const GD_EffectSelection *selection=&query->contributors->selection; const GD_EffectNode *node=&query->effects->nodes[index];
    return node->kind!=GD_EFFECT_ACTIVATION && (selection->target<0 || selection->target==index) && (selection->axis<0 || selection->axis==node->axis) &&
        (selection->kind<0 || selection->kind==(int)node->kind) && (!selection->value || (node->value && !strcmp(selection->value,node->value)));
}
static void contributor(FILE *out, const GD_Query *query, int row) {
    if(ferror(out))return;
    const GD_EffectQuery *result=query->contributors; const GD_EffectLink *link=&query->effects->links[result->links[row]]; const GD_ResourceRelation *relation=&query->resource->relations[link->relation];
    const GD_EffectNode *node=&query->effects->nodes[link->to]; int path[GD_MAX_RULE_EXPANSION_DEPTH],count=gd_effect_contributor_path(result,row,path,GD_MAX_RULE_EXPANSION_DEPTH),i;
    fputs("{\"result\":",out); effect_id(out,query->effects,link->to); fputs(",\"affected\":",out); actor(out,query,link->participant);
    fputs(",\"template_target\":",out); actor(out,query,relation->participant);
    fprintf(out,",\"line\":%d,\"declaration_index\":%d,\"source_relation_index\":%d,\"declaration_conditional\":%s,\"delta_recognized\":%s,\"delta\":",relation->line,relation->declaration,link->relation,relation->conditional ? "true":"false",relation->delta_recognized ? "true":"false");
    if (relation->delta.known && relation->delta.checks_complete && isfinite(relation->delta.number)) fprintf(out,"%.17g",relation->delta.number); else fputs("null",out);
    member(out,"declared_unit",node->axis>=0 ? query->model->records[node->axis].axis.unit:NULL);
    if (relation->kind==GD_RESOURCE_QUANTITY_WRITE) member(out,"expression",query->rules->assignments[relation->declaration].expression);
    fputs(",\"via\":[",out);
    for (i=0;i<count && !ferror(out);++i) { if (i) fputc(',',out); activation(out,query,path[i]); }
    fputs("]}",out);
}
int gd_write_query_catalogue(FILE *out, int json) {
    int i;
    static const char *const parameters[]={"--object, --case, --participant, --axis, --status", "--case and --axis required; --participant (default authored A), --object, --value (repeatable), --assume, --context, --context-not, --status", "--to opaque result ID OR --axis/--value/--effect; --object, --case (root placement), --status", "--axis required; --object, --case, --participant (requires --case), --value (repeatable; required for numeric axis), --status"};
    if (!json) { for (i=0;i<GD_QUERY_COUNT;++i) fprintf(out,"%s [%s]: %s\n",gd_query_name((GD_QueryKind)i),gd_query_projection((GD_QueryKind)i),parameters[i]); return gd_write_named_catalogue(out,0,0) && gd_write_structure_catalogue(out,0,0); }
    fputs("{\"schema_version\":1,\"ok\":true,\"data\":{\"questions\":[",out);
    for (i=0;i<GD_QUERY_COUNT;++i) {
        if (i) fputc(',',out); fputs("{\"name\":",out); gd_json_string(out,gd_query_name((GD_QueryKind)i)); member(out,"projection",gd_query_projection((GD_QueryKind)i)); member(out,"parameters",parameters[i]);
        fputs(",\"required_bindings\":{\"any_of\":[\"interactions\",\"mechanics\"],\"all_of\":[",out); if (i==GD_QUERY_ACTOR_STATES || i==GD_QUERY_STATE_COVERAGE) fputs("\"nouns\",\"axes\"",out); fputs("]},\"required_parameters\":[",out);
        if (i==GD_QUERY_ACTOR_STATES) fputs("\"--case\",\"--axis\"",out); else if (i==GD_QUERY_STATE_COVERAGE) fputs("\"--axis\"",out);
        fputs("],\"optional_parameters\":[\"--object\",\"--status\",\"--work-limit\"",out);
        fputs(i==GD_QUERY_BINDINGS ? ",\"--case\",\"--participant\",\"--axis\"":i==GD_QUERY_ACTOR_STATES ? ",\"--participant\",\"--value\",\"--assume\",\"--context\",\"--context-not\"":i==GD_QUERY_STATE_COVERAGE ? ",\"--case\",\"--participant\",\"--value\"":",\"--case\",\"--axis\",\"--value\",\"--effect\",\"--to\"",out);
        fputs("],\"repeatable_parameters\":[\"--status\"",out); if (i==GD_QUERY_ACTOR_STATES) fputs(",\"--value\",\"--assume\",\"--context\",\"--context-not\"",out); else if (i==GD_QUERY_STATE_COVERAGE) fputs(",\"--value\"",out);
        fputs("],\"fields\":[\"cases\",\"findings\"",out);
        fputs(i==GD_QUERY_BINDINGS ? ",\"bindings\"":i==GD_QUERY_ACTOR_STATES ? ",\"variants\",\"comparisons\"":i==GD_QUERY_STATE_COVERAGE ? ",\"axis_definition\",\"state_values\",\"state_bindings\"": ",\"concepts\",\"contributors\"",out);
        fputs("],\"ui_available\":true",out);
        member(out,"ui_mode",i==GD_QUERY_BINDINGS ? "resources":i==GD_QUERY_ACTOR_STATES || i==GD_QUERY_STATE_COVERAGE ? "actors":"effects");
        member(out,"ui_scope",i==GD_QUERY_STATE_COVERAGE ? "Actors finite state coverage mode: typed axis/finite values; optional object/case/local binding; all or accepted root cases; aggregate value cards and exact independent-binding inspection":i==GD_QUERY_BINDINGS ? "Selection > Inspect bindings in current pane, or Resources > Object bindings / rule checks: explicit typed context staging, separate Run; exact object/case/local/axis/status/work filters, native rows and full source; not applicability":i==GD_QUERY_ACTOR_STATES ? "explicit root/local participant/axis/finite values/fixed facts/context; all or accepted root; selected pair and ghosts":"result picker; exact contributors/ancestors/side effects; all or accepted placement ancestors; direct-count weighting");
        fputs(",\"ui_unavailable_parameters\":[",out);
        if (i!=GD_QUERY_BINDINGS) fputs("\"arbitrary --status list\",\"--work-limit\"",out);
        if (i!=GD_QUERY_STATE_COVERAGE && i!=GD_QUERY_BINDINGS) fputs(",\"--object\"",out);
        if (i==GD_QUERY_CONTRIBUTORS) fputs(",\"--case placement filter\",\"--axis/--value/--effect filters (choose result instead)\"",out);
        if (i==GD_QUERY_STATE_COVERAGE) fputs(",\"explicit values beyond 8192 combined input bytes\",\"explicit value literals containing semicolon or newline separators\"",out);
        fprintf(out,"],\"ipc_available\":false,\"work_limit_max\":%d}",GD_EFFECT_WORK);
    }
    gd_write_named_catalogue(out,1,GD_QUERY_COUNT);
    gd_write_structure_catalogue(out,1,GD_QUERY_COUNT+GD_NAMED_COUNT);
    fputs("]},\"coverage\":{\"scope\":\"implemented_builtin_catalogue_only\",\"complete\":true,\"source_data_loaded\":false},\"errors\":[],\"breadcrumbs\":[\"gd query --question NAME --projection NAME with explicit collection bindings; query --list advertises only implemented operations\"]}\n",out); return !ferror(out);
}
static int write_query(FILE *out, const GD_Model *model, const GD_Query *query, int json, const char *fields,int compare) {
    int i,j,comma=0,complete=model->error_count==0 && model->errors_omitted==0 && query!=NULL;
    int limited=query && (query->limited || (query->state_coverage && query->state_coverage->limited) || (query->variants && query->variants->limited) || (query->resource && query->resource->limited) || (query->effects && query->effects->limited) || (query->contributors && query->contributors->limited));
    if (query && (query->limited || (query->state_coverage && (query->state_coverage->limited || query->state_coverage->source_incomplete)))) complete=0;
    if (query && query->variants) { if (query->variants->limited) complete=0; for (i=0;i<query->variants->variant_count;++i) if (!query->variants->variants[i].constraints_complete || !query->variants->variants[i].values_complete) complete=0; }
    if (query && query->contributors && (query->contributors->limited || query->contributors->source_incomplete || query->resource->binding_conflicts || query->resource->identity_collisions)) complete=0;
    if (query && (!query->rules->bindings_complete || query->rules->findings_omitted)) complete=0;
    if (!json) {
        if (query) {
            fprintf(out,"%s [%s]\n",gd_query_name(query->request.kind),gd_query_projection(query->request.kind));
            for (i=0;i<query->binding_count;++i) { const GD_QueryBinding *binding=&query->bindings[i]; const GD_RuleCase *item=&query->rules->cases[binding->case_index]; fprintf(out,"%s#%s :: %s (%s)%s\n",model->records[query->rules->sources[item->source].record].reference,item->name,query->rules->participants[binding->participant].name,query->rules->participants[binding->participant].noun ? query->rules->participants[binding->participant].noun:"[unspecified noun]",binding->participant==item->primary_participant ? " [A]":""); }
            if (query->variants) for (i=0;i<query->variants->variant_count;++i) fprintf(out,"Value %s: applicability %s; %d proposals; %d conflicts; constraints %s; limited %s\n",query->variants->variants[i].value,gd_rule_truth_name(query->variants->variants[i].applicability),query->variants->variants[i].write_count,query->variants->variants[i].conflicts,query->variants->variants[i].constraints_complete ? "complete":"incomplete",query->variants->variants[i].limited ? "yes":"no");
            if (query->state_coverage) for (i=0;i<query->state_coverage->value_count;++i) { const GD_StateCoverageValue *value=&query->state_coverage->values[i]; fprintf(out,"Value %s: %d definite cases; %d undetermined cases; %d excluded cases; %d unfinished cases; no retained candidate %s%s\n",value->value,value->definite_cases,value->undetermined_cases,value->excluded_cases,value->unfinished_cases,gd_rule_truth_name(value->no_retained_candidate),value->declared ? " [declared]":" [requested hypothesis]"); }
            if (query->contributors) for (i=0;i<query->effects->node_count;++i) if (concept_matches(query,i)) { char id[GD_EFFECT_ID_CAPACITY]; fprintf(out,"%s: %d direct target-labelled declarations\n",gd_effect_node_id(query->effects,i,id,sizeof(id)) ? id:"[identity unavailable]",query->contributors->incoming[i]); }
        }
        for (i=0;i<model->error_count;++i) fprintf(out,"%s: %s\n",model->errors[i].code,model->errors[i].message);
        fprintf(out,"Scope %s. Conditional structural evidence only; no executable sequence, global runtime actor or final outcome proof.\n",complete ? "checked":"incomplete"); return !ferror(out);
    }
    fprintf(out,"{\"schema_version\":1,\"operation\":\"query\",\"ok\":%s,\"question\":",model->error_count || model->errors_omitted ? "false":"true"); gd_json_string(out,query ? gd_query_name(query->request.kind):NULL); member(out,"projection",query ? gd_query_projection(query->request.kind):NULL);
    fputs(",\"selectors\":",out); if (query) selectors(out,query); else fputs("null",out);
    fputs(",\"data\":{",out);
    if (query) {
        if (query->request.kind==GD_QUERY_BINDINGS && selected(fields,"bindings")) {
            fputs("\"bindings\":[",out); comma=1;
            for (i=0;i<query->binding_count && !ferror(out);++i) {
                const GD_QueryBinding *binding=&query->bindings[i]; const GD_RuleParticipant *participant=&query->rules->participants[binding->participant]; int edge,axes=0;
                if (i) fputc(',',out); fputs("{\"source\":",out); source_case(out,query,binding->case_index); fputs(",\"actor\":",out); actor(out,query,binding->participant);
                fprintf(out,",\"primary\":%s,\"axes_complete\":%s,\"axes\":[",binding->participant==query->rules->cases[binding->case_index].primary_participant ? "true":"false",participant->noun_resolution==GD_RULE_RESOLVED && query->rules->actor_axes_complete[participant->noun_record] ? "true":"false");
                if (participant->noun_resolution==GD_RULE_RESOLVED) for (edge=query->rules->actor_first[participant->noun_record];edge>=0 && !ferror(out);edge=query->rules->next_axis[edge]) {
                    const GD_Edge *item=&model->edges[edge]; if (axes++) fputc(',',out); fputs("{\"reference\":",out); gd_json_string(out,item->reference); member(out,"resolution",gd_rule_resolution_name(query->rules->axis_resolution[edge])); member(out,"resolved_reference",query->rules->axis_resolution[edge]==GD_RULE_RESOLVED ? model->records[item->target].reference:NULL);
                    if (query->rules->axis_resolution[edge]==GD_RULE_RESOLVED) { const GD_Record *record=&model->records[item->target]; const char *value=record->axis.allowed; int n; member(out,"value_type",record->axis.type); member(out,"unit",record->axis.unit); member(out,"minimum",record->axis.minimum); member(out,"maximum",record->axis.maximum); fprintf(out,",\"definition_valid\":%s,\"unit_known\":%s,\"closed\":%s,\"values\":[",query->rules->axes[item->target].valid ? "true":"false",query->rules->axes[item->target].unit_known ? "true":"false",record->axis_closed ? "true":"false"); for (n=0;n<record->axis.allowed_count;++n) { if (n) fputc(',',out); gd_json_string(out,value); value+=strlen(value)+1; } fputc(']',out); }
                    fprintf(out,",\"line\":%d}",item->line);
                }
                fputs("]}",out);
            }
            fputc(']',out);
        }
        if (query->state_coverage && selected(fields,"axis_definition")) {
            const GD_Record *axis=&model->records[query->request.axis]; const char *value=axis->axis.allowed;
            if (comma++) fputc(',',out); fputs("\"axis_definition\":{\"reference\":",out); gd_json_string(out,axis->reference);
            member(out,"record_id",axis->id); member(out,"source_path",axis->source_path); member(out,"status",axis->status); member(out,"value_type",axis->axis.type); member(out,"unit",axis->axis.unit); member(out,"minimum",axis->axis.minimum); member(out,"maximum",axis->axis.maximum);
            fprintf(out,",\"closed\":%s,\"definition_valid\":%s,\"unit_known\":%s,\"allowed_values\":[",axis->axis_closed ? "true":"false",query->rules->axes[query->request.axis].valid ? "true":"false",query->rules->axes[query->request.axis].unit_known ? "true":"false");
            for (i=0;i<axis->axis.allowed_count;++i) { if (i) fputc(',',out); gd_json_string(out,value); value+=strlen(value)+1; } fputs("]}",out);
        }
        if (query->state_coverage && selected(fields,"state_values")) {
            if (comma++) fputc(',',out); fputs("\"state_values\":[",out);
            for (i=0;i<query->state_coverage->value_count;++i) {
                const GD_StateCoverageValue *value=&query->state_coverage->values[i]; if (i) fputc(',',out); fputs("{\"value\":",out); gd_json_string(out,value->value);
                fprintf(out,",\"declared\":%s,\"definite_bindings\":%d,\"undetermined_bindings\":%d,\"excluded_bindings\":%d,\"unfinished_bindings\":%d,\"definite_cases\":%d,\"undetermined_cases\":%d,\"excluded_cases\":%d,\"unfinished_cases\":%d,\"no_retained_candidate\":",value->declared ? "true":"false",value->definite_bindings,value->undetermined_bindings,value->excluded_bindings,value->unfinished_bindings,value->definite_cases,value->undetermined_cases,value->excluded_cases,value->unfinished_cases);
                fputs(value->no_retained_candidate==GD_RULE_TRUTH_UNKNOWN ? "null":value->no_retained_candidate==GD_RULE_TRUTH_TRUE ? "true":"false",out); fputc('}',out);
            }
            fputc(']',out);
        }
        if (query->state_coverage && selected(fields,"state_bindings")) {
            if (comma++) fputc(',',out); fputs("\"state_bindings\":[",out);
            for (i=0;i<query->binding_count;++i) {
                if (i) fputc(',',out); fputs("{\"source\":",out); source_case(out,query,query->bindings[i].case_index); fputs(",\"actor\":",out); actor(out,query,query->bindings[i].participant); fputs(",\"values\":[",out);
                for (j=0;j<query->state_coverage->value_count;++j) {
                    const GD_StateCoverage *coverage=query->state_coverage; uint64_t bit=UINT64_C(1)<<j;
                    if (j) fputc(',',out); fprintf(out,"{\"value_index\":%d,\"examined\":%s,\"applicability\":",j,coverage->finished[i]&bit ? "true":"false");
                    gd_json_string(out,!(coverage->present[i]&bit) ? NULL:gd_rule_truth_name(coverage->definite[i]&bit ? GD_RULE_TRUTH_TRUE:coverage->undetermined[i]&bit ? GD_RULE_TRUTH_UNKNOWN:GD_RULE_TRUTH_FALSE)); fputc('}',out);
                }
                fputs("]}",out);
            }
            fputc(']',out);
        }
        if (query->request.kind==GD_QUERY_ACTOR_STATES && selected(fields,"variants")) { if (comma++) fputc(',',out); fputs("\"variants\":[",out); if (query->variants) for (i=0;i<query->variants->variant_count && !ferror(out);++i) { if (i) fputc(',',out); variant(out,query,i); } fputc(']',out); }
        if (compare && query->request.kind==GD_QUERY_ACTOR_STATES && selected(fields,"comparisons")) {
            int pairs=0; if (comma++) fputc(',',out); fputs("\"comparisons\":[",out);
            if (query->variants) for (i=0;i<query->variants->variant_count;++i) for (j=i+1;j<query->variants->variant_count;++j) { GD_ActorComparison value; gd_actor_compare(query->variants,i,j,&value); if (pairs++) fputc(',',out); fprintf(out,"{\"first\":%d,\"second\":%d,\"complete\":%s,\"cases\":",i,j,value.complete ? "true":"false"); comparison_set(out,value.cases,query->variants->variants[i].cases,query->variants->variants[j].cases,query->rules->case_count); fputs(",\"partners\":",out); comparison_set(out,value.partners,query->variants->variants[i].partners,query->variants->variants[j].partners,query->rules->participant_count); fputs(",\"effects\":",out); comparison_set(out,value.effects,query->variants->variants[i].effects,query->variants->variants[j].effects,query->variants->effect_count); fputc('}',out); }
            fputc(']',out);
        }
        if (query->effects && selected(fields,"concepts")) {
            int count=0; if (comma++) fputc(',',out); fputs("\"concepts\":[",out);
            for (i=0;i<query->effects->node_count;++i) if (concept_matches(query,i)) { const GD_EffectNode *node=&query->effects->nodes[i]; if (count++) fputc(',',out); fputs("{\"id\":",out); effect_id(out,query->effects,i); member(out,"kind",gd_effect_kind_name(node->kind)); member(out,"axis",node->axis>=0 ? model->records[node->axis].reference:NULL); member(out,"value",node->value); fprintf(out,",\"incoming\":%d}",query->contributors->incoming[i]); }
            fputc(']',out);
        }
        if (query->effects && selected(fields,"contributors")) { if (comma++) fputc(',',out); fputs("\"contributors\":[",out); for (i=0;i<query->contributors->count && !ferror(out);++i) { if (i) fputc(',',out); contributor(out,query,i); } fputc(']',out); }
        if (selected(fields,"cases")) { int count=0; if (comma++) fputc(',',out); fputs("\"cases\":[",out); for (i=0;i<query->rules->case_count && !ferror(out);++i) if (query->cases[i]) { if (count++) fputc(',',out); fprintf(out,"{\"index\":%d,\"declaration\":",i); gd_write_rule_case(out,model,query->rules,i); fputc('}',out); } fputc(']',out); }
        if (selected(fields,"findings")) { if (comma++) fputc(',',out); fputs("\"findings\":[",out); for (i=0;i<query->rules->finding_count;++i) { const GD_Finding *finding=&query->rules->findings[i]; if (i) fputc(',',out); fputs("{\"code\":",out); gd_json_string(out,finding->code); member(out,"message",finding->message); member(out,"record",finding->record>=0 ? model->records[finding->record].reference:NULL); fprintf(out,",\"line\":%d}",finding->line); } fputc(']',out); }
    }
    fprintf(out,"},\"coverage\":{\"complete\":%s,\"read_only\":true,\"scope\":",complete ? "true":"false");
    gd_json_string(out,"Loaded explicit definitions and mapped placements, not camera clipping. Variant axis values apply only to selected local binding; other fixed facts retain explicit targets. Actor status filters select root definitions; invoked dependencies remain labelled. Effect filters require every placement ancestor in scope. Shared results do not imply sufficient recipes, temporal state-matching, runtime actor identity, resource-pool identity, reachable inventory or final outcomes. Comparison/incoming counts exclude ghost copies; counts are not gameplay frequency, importance or design quality. Comparison member indices refer to cases, partners and effect_concepts in this response; indices are snapshot-local. Fixed facts are requested assumptions; excluded roots and unexamined variants do not establish their validity.");
    fprintf(out,",\"applicability_evaluated\":%s,\"final_outcomes_proven\":false,\"paths_are\":\"mapped_instantiation_not_temporal_sequence\",\"limited\":%s,\"inventory_work\":%d,\"unexamined_bindings_upper_bound\":%d,\"excluded_roots\":%d",query && (query->variants || query->state_coverage) ? "true":"false",limited ? "true":"false",query ? query->work:0,query ? query->unexamined_bindings:0,query ? query->excluded_roots:0);
    if (query && query->variants) fprintf(out,",\"requested_variants\":%d,\"retained_variants\":%d,\"unexamined_variants\":%d,\"variant_work\":%d,\"variant_limited\":%s,\"vocabulary_closed\":%s",query->variants->requested,query->variants->variant_count,query->variants->unexamined_variants,query->variants->work,query->variants->limited ? "true":"false",query->variants->vocabulary_closed ? "true":"false");
    if (query && query->state_coverage) fprintf(out,",\"state_coverage_work\":%d,\"state_coverage_limited\":%s,\"state_query_complete\":%s,\"state_source_incomplete\":%s,\"state_evaluated_bindings\":%d,\"state_scope\":\"Independent finite value hypothesis on each selected local binding; whole root-case guards/exclusions, not outcome feasibility. Other bindings/context stay unknown. Case counts deduplicate local bindings; counts are not partner breadth or gameplay frequency. Null absence means unchecked; missing retained candidates are inspection hints, not design defects. Numeric values are samples, never exhaustive ranges. Child outcome work can conservatively limit matching coverage. Input validation and fixed bounded aggregation are separate from matching work budget.\"",query->state_coverage->work,query->state_coverage->limited ? "true":"false",query->limited || query->state_coverage->limited ? "false":"true",query->state_coverage->source_incomplete ? "true":"false",query->state_coverage->evaluated_bindings);
    if (query && query->resource) fprintf(out,",\"resource_limited\":%s,\"resource_nodes\":%d,\"resource_relations\":%d,\"resource_work\":%d,\"resource_unexamined_cases\":%d,\"composition_unexamined\":%d,\"calls_bound\":%s,\"projection_limited\":%s,\"projection_unexamined_nodes\":%d,\"projection_unexamined_relations\":%d,\"retained_contributors\":%d",query->resource->limited ? "true":"false",query->resource->node_count,query->resource->relation_count,query->resource->work,query->resource->unexamined_cases,query->resource->composition_unexamined,query->resource->calls_bound ? "true":"false",query->effects->limited ? "true":"false",query->effects->unexamined_source_nodes,query->effects->unexamined_relations,query->contributors->count);
    if (query && query->contributors) fprintf(out,",\"contributor_work\":%d,\"contributor_limited\":%s,\"source_incomplete\":%s,\"unexamined_links\":%d,\"unresolved_targets\":%d,\"excluded_scope\":%d,\"excluded_targets\":%d,\"binding_conflicts\":%d",query->contributors->work,query->contributors->limited ? "true":"false",query->contributors->source_incomplete ? "true":"false",query->contributors->unexamined_links,query->contributors->unresolved_targets,query->contributors->excluded_scope,query->contributors->excluded_target,query->resource->binding_conflicts);
    if (query && query->resource) fprintf(out,",\"cycle_frontiers\":%d,\"depth_frontiers\":%d,\"identity_collisions\":%d,\"preservation_unexamined\":%d,\"unknown_case_coverage\":%d",query->resource->cycle_frontiers,query->resource->depth_frontiers,query->resource->identity_collisions,query->resource->preservation_unexamined,query->resource->unknown_case_coverage);
    if (query && query->state_coverage) fprintf(out,",\"state_fixed_bounds\":{\"local_bindings\":%d,\"summary_incidences\":%d,\"axis_text_bytes\":%d,\"literal_bytes\":%d,\"owned_bytes\":%llu}",GD_MAX_RULE_PARTICIPANTS,(GD_MAX_RULE_PARTICIPANTS+GD_MAX_RULE_CASES)*GD_ACTOR_VARIANTS,GD_MAX_FRONTMATTER,GD_MAX_RULE_LINE,(unsigned long long)sizeof(*query->state_coverage));
    fprintf(out,",\"findings_scope\":\"whole_loaded_rules_snapshot\",\"limits\":{\"variant_count\":%d,\"writes\":%d,\"activations\":%d,\"effect_nodes\":%d,\"effect_links\":%d,\"work_per_construction_stage\":%d}},\"errors\":[",GD_ACTOR_VARIANTS,GD_ACTOR_WRITES,GD_ACTOR_ACTIVATIONS,GD_EFFECT_NODES,GD_EFFECT_LINKS,GD_EFFECT_WORK);
    for (i=0;i<model->error_count;++i) { const GD_Error *error=&model->errors[i]; if (i) fputc(',',out); fputs("{\"code\":",out); gd_json_string(out,error->code); member(out,"message",error->message); member(out,"source_path",error->path); fprintf(out,",\"line\":%d}",error->line); }
    fprintf(out,"],\"errors_omitted\":%d,\"breadcrumbs\":[\"gd query --list\",\"Use object-bindings to inspect qualified --case/--participant and declared --axis selectors; use effect-contributors concepts for opaque --to IDs\"]}\n",model->errors_omitted);
    return !ferror(out);
}
int gd_write_query(FILE *out,const GD_Model *model,const GD_Query *query,int json,const char *fields) { return write_query(out,model,query,json,fields,1); }
int gd_write_query_retained(FILE *out,const GD_Model *model,const GD_Query *query,int json,const char *fields) { return write_query(out,model,query,json,fields,0); }
int gd_write_query_selection(FILE *out,const GD_Model *model,const GD_Query *query,GD_EffectQuery *selected,int json,const char *fields) {
    GD_Query *view; int written; char id[GD_EFFECT_ID_CAPACITY];
    if(!selected) return gd_write_query_retained(out,model,query,json,fields);
    view=query->allocator.allocate(query->allocator.context,sizeof(*view)); if(!view) return 0;
    *view=*query; view->contributors=selected;
    if(selected->selection.target>=0 && gd_effect_node_id(query->effects,selected->selection.target,id,sizeof(id))) view->request.target_id=id;
    written=write_query(out,model,view,json,fields,0);query->allocator.release(query->allocator.context,view);return written;
}
