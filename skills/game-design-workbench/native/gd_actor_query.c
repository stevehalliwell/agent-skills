#include "gd_actor_query.h"
#include <string.h>

static int charge(GD_ActorQuery *query, size_t work) {
    if (query->limited || work>(size_t)(query->work_limit-query->work)) { query->limited=1; return 0; }
    query->work+=(int)work; return 1;
}
static void member(unsigned long long *set, int index) { set[index/64]|=1ULL<<(index%64); }
int gd_actor_trace_participant(const GD_RuleAnalysis *analysis, const GD_RuleActivation *activation, int participant) {
    int step;
    if (!analysis || !activation || activation->path_count<1 || activation->path_count>GD_MAX_RULE_EXPANSION_DEPTH || activation->case_index<0 || activation->case_index>=analysis->rules->case_count) return -1;
    {
        const GD_RuleCase *item=&analysis->rules->cases[activation->case_index];
        if (participant<item->first_participant || participant>=item->first_participant+item->participant_count) return -1;
    }
    for (step=activation->path_count-1;step>0;--step) {
        participant=gd_rule_call_participant(analysis,activation->calls[step],participant);
        if (participant<0) return -1;
    }
    return participant;
}
GD_EffectKind gd_actor_write_family(const GD_ActorQuery *query, int variant, const GD_RuleWrite *write) {
    const GD_Rules *rules; GD_ExpressionInput starting;
    if (!query || variant<0 || variant>=query->variant_count || !write) return GD_EFFECT_UNKNOWN;
    if (write->kind==GD_WRITE_STATE) return write->axis>=0 && write->state ? GD_EFFECT_STATE:GD_EFFECT_UNKNOWN;
    if (write->kind==GD_WRITE_PRESERVE) return GD_EFFECT_PRESERVE;
    rules=query->analysis->rules;
    if (write->value.bounds==GD_BOUNDS_OUTSIDE || write->value.state==GD_EXPRESSION_UNIT_ERROR || write->value.state==GD_EXPRESSION_ARITHMETIC_ERROR) return GD_EFFECT_QUANTITY;
    if (write->actor==query->participant && write->axis==query->axis && rules->axes[query->axis].type!=GD_AXIS_ENUM) {
        GD_RulePredicateFact fact={write->actor,write->axis,query->variants[variant].value,GD_RULE_TRUTH_TRUE};
        starting=gd_rule_starting_quantity(rules,write->actor,write->axis,&fact,1);
    } else starting=gd_rule_starting_quantity(rules,write->actor,write->axis,query->fixed.facts,query->fixed.fact_count);
    if (write->value.known && write->value.checks_complete && starting.known && starting.unit_known) {
        /* Comparison avoids subtracting safe integers into an unsafe difference.
           This family is conditional on explicitly supplied starting facts. */
        return write->value.number>starting.value.number ? GD_EFFECT_GAIN:write->value.number<starting.value.number ? GD_EFFECT_LOSS:GD_EFFECT_NO_CHANGE;
    }
    if (write->quantity>=0) {
        int recognized; GD_RuleQuantityValue delta=gd_rule_quantity_delta(rules,write->quantity,&recognized);
        if (recognized && delta.known && delta.checks_complete)
            return delta.number>0 ? GD_EFFECT_GAIN:delta.number<0 ? GD_EFFECT_LOSS:GD_EFFECT_NO_CHANGE;
    }
    return GD_EFFECT_QUANTITY;
}
static int effect_member(GD_ActorQuery *query, int variant, int index) {
    const GD_RuleWrite *write=&query->writes[index]; GD_ActorEffect key; int i;
    memset(&key,0,sizeof(key)); key.kind=gd_actor_write_family(query,variant,write); key.axis=write->axis;
    key.actor=key.origin_case=key.declaration=key.activation=-1;
    if (key.kind==GD_EFFECT_STATE) key.state=write->state;
    if (key.kind==GD_EFFECT_QUANTITY || key.kind==GD_EFFECT_UNKNOWN || key.kind==GD_EFFECT_PRESERVE) {
        key.actor=write->actor; key.origin_case=write->origin_case; key.declaration=write->declaration;
        key.declaration_kind=write->kind; key.activation=query->write_activations[index];
    }
    for (i=0;i<query->effect_count;++i) {
        const GD_ActorEffect *other=&query->effects[i];
        if (!charge(query,1+GD_MAX_RULE_EXPANSION_DEPTH*2+(key.state ? strlen(key.state):0))) return -1;
        if (key.kind!=other->kind || key.actor!=other->actor || key.axis!=other->axis || key.origin_case!=other->origin_case || key.declaration!=other->declaration || key.declaration_kind!=other->declaration_kind) continue;
        if (key.activation>=0) {
            const GD_RuleActivation *a=&query->activations[key.activation],*b=&query->activations[other->activation];
            if (a->path_count!=b->path_count || memcmp(a->path,b->path,(size_t)a->path_count*sizeof(*a->path)) || memcmp(a->calls,b->calls,(size_t)a->path_count*sizeof(*a->calls))) continue;
        }
        if ((!key.state && !other->state) || (key.state && other->state && !strcmp(key.state,other->state))) return i;
    }
    if (query->effect_count==GD_ACTOR_WRITES) { query->limited=1; return -1; }
    query->effects[query->effect_count]=key; return query->effect_count++;
}
GD_ActorQuery *gd_actor_query_build(GD_Model *model, const GD_RuleAnalysis *analysis, const GD_ActorSelection *selection, int work_limit) {
    GD_ActorQuery *query; const GD_Rules *rules; const GD_RuleCase *root; const char *values[GD_ACTOR_VARIANTS];
    int actor,axis,n=0,i,j,edge,applicable=0;
    if (!model) return NULL;
    if (!analysis || analysis->model!=model || !selection) { gd_error(model,"actors.arguments","Actor query requires one matching model/analysis and explicit selection",NULL,0); return NULL; }
    rules=analysis->rules;
    if (selection->case_index<0 || selection->case_index>=rules->case_count || selection->value_count<0 || selection->value_count>GD_ACTOR_VARIANTS || work_limit<0 || work_limit>GD_ACTOR_WORK) {
        gd_error(model,"actors.arguments","Select one parsed case, at most 64 values and bounded work",NULL,0); return NULL;
    }
    root=&rules->cases[selection->case_index]; actor=selection->participant==-1 ? root->primary_participant:selection->participant; axis=selection->axis;
    if (actor<root->first_participant || actor>=root->first_participant+root->participant_count || axis<0 || axis>=model->record_count || !rules->axes[axis].valid || rules->axes[axis].type==GD_AXIS_UNSPECIFIED || rules->participants[actor].noun_resolution!=GD_RULE_RESOLVED) {
        gd_error(model,"actors.binding","Select a declared local participant (default explicit A) and valid applicable axis; no order/global actor fallback",NULL,0); return NULL;
    }
    for (edge=rules->actor_first[rules->participants[actor].noun_record];edge>=0;edge=rules->next_axis[edge])
        if (rules->axis_resolution[edge]==GD_RULE_RESOLVED && model->edges[edge].target==axis) applicable=1;
    if (!applicable) { gd_error(model,"actors.axis","Selected axis is not explicitly declared on selected participant noun",NULL,0); return NULL; }
    if (!selection->value_count && rules->axes[axis].type!=GD_AXIS_ENUM) { gd_error(model,"actors.values","Numeric variants require explicit finite --value literals in declared axis unit",NULL,0); return NULL; }
    if (selection->fixed && (selection->fixed->fact_count<0 || selection->fixed->fact_count>GD_MAX_RULE_CONDITION_ITEMS || selection->fixed->context_count<0 || selection->fixed->context_count>GD_MAX_RULE_CONTEXT_FACTS || selection->fixed->work<0 || selection->fixed->work>GD_RULE_EVALUATION_WORK_LIMIT || selection->fixed->limited)) {
        gd_error(model,"actors.facts","Fixed fact/context input exceeds native scenario bounds",NULL,0); return NULL;
    }
    if (selection->fixed) for (i=0;i<selection->fixed->fact_count;++i) if (selection->fixed->facts[i].participant==actor && selection->fixed->facts[i].axis_record==axis) {
        gd_error(model,"actors.fixed_axis","Fixed facts must not also assign selected variant axis/binding; no silent override",NULL,0); return NULL;
    }
    query=model->allocator.allocate(model->allocator.context,sizeof(*query));
    if (!query) { gd_error(model,"actors.memory","Cannot allocate bounded actor query",NULL,0); return NULL; }
    memset(query,0,sizeof(*query)); query->allocator=model->allocator; query->analysis=analysis;
    for (i=0;i<GD_ACTOR_WRITES;++i) query->write_effects[i]=-1;
    query->case_index=selection->case_index; query->participant=actor; query->axis=axis; query->work_limit=work_limit; query->vocabulary_closed=model->records[axis].axis_closed;
    if (selection->fixed) query->fixed=*selection->fixed;
    if (selection->value_count) { n=selection->value_count; for (i=0;i<n;++i) values[i]=selection->values[i]; }
    else {
        const char *value=model->records[axis].axis.allowed;
        for (i=0;i<model->records[axis].axis.allowed_count;++i) {
            for (j=0;j<n;++j) if (!strcmp(values[j],value)) break;
            if (j==n) values[n++]=value;
            value+=strlen(value)+1;
        }
    }
    query->requested=n;
    if (!charge(query,(size_t)query->fixed.work)) { query->unexamined_variants=n; return query; }
    for (i=0;i<n && !query->limited;++i) {
        GD_RuleEvaluation *evaluation=&query->evaluation; GD_ActorVariant *variant=&query->variants[query->variant_count]; int before,limit;
        if (!values[i] || !values[i][0]) { gd_error(model,"actors.value","Variant requires a nonempty typed literal",NULL,0); goto failure; }
        query->inputs=query->fixed; before=query->inputs.work;
        if (!gd_rule_assume_value(model,rules,selection->case_index,actor,axis,values[i],&query->inputs)) goto failure;
        if (!charge(query,(size_t)(query->inputs.work-before))) break;
        for (j=0;j<i;++j) {
            int duplicate;
            if (!charge(query,strlen(values[i])+strlen(values[j])+1)) break;
            if (rules->axes[axis].type==GD_AXIS_ENUM) duplicate=!strcmp(values[j],values[i]);
            else { double a,b; gd_control_number(values[i],&a,rules->axes[axis].type==GD_AXIS_INTEGER); gd_control_number(values[j],&b,rules->axes[axis].type==GD_AXIS_INTEGER); duplicate=a==b; }
            if (duplicate) { gd_error(model,"actors.duplicate_value","Finite variant list contains the same typed value more than once",NULL,0); goto failure; }
        }
        if (query->limited) break;
        limit=query->work_limit-query->work; if (limit>GD_RULE_EVALUATION_WORK_LIMIT) limit=GD_RULE_EVALUATION_WORK_LIMIT;
        {
            GD_RuleAssumptions inputs={query->inputs.facts,query->inputs.fact_count,query->inputs.contexts,query->inputs.context_count};
            gd_rule_evaluate_bounded(analysis,selection->case_index,inputs,evaluation,limit,&query->trace);
        }
        if (!charge(query,(size_t)evaluation->work)) break;
        if (evaluation->input_valid==0) { gd_error(model,"actors.facts","Contradictory or invalid fixed/variant facts",NULL,0); goto failure; }
        if (evaluation->write_count>GD_ACTOR_WRITES-query->write_count || query->trace.count>GD_ACTOR_ACTIVATIONS-query->activation_count || evaluation->checks_count>GD_ACTOR_CHECKS-query->check_count) { query->limited=1; break; }
        variant->value=values[i]; variant->applicability=evaluation->applicability; variant->context=evaluation->context; variant->requirements=evaluation->requirements; variant->exclusion=evaluation->exclusion;
        variant->first_write=query->write_count; variant->write_count=evaluation->write_count;
        variant->first_activation=query->activation_count; variant->activation_count=query->trace.count;
        variant->first_check=query->check_count; variant->check_count=evaluation->checks_count;
        variant->constraints_complete=evaluation->constraints_complete; variant->values_complete=evaluation->values_complete;
        variant->conflicts=evaluation->conflict_count; variant->uncertain_pairs=evaluation->uncertain_write_pairs; variant->checks_omitted=evaluation->checks_omitted;
        variant->opaque_effects=evaluation->opaque_effects; variant->cycles=evaluation->cycle_count; variant->work=query->inputs.work-before+evaluation->work; variant->limited=evaluation->limited;
        memcpy(query->writes+query->write_count,evaluation->writes,sizeof(*query->writes)*(size_t)evaluation->write_count);
        for (j=0;j<evaluation->write_count;++j) query->write_activations[query->write_count+j]=variant->first_activation+query->trace.write_activation[j];
        query->write_count+=evaluation->write_count;
        memcpy(query->activations+query->activation_count,query->trace.items,sizeof(*query->activations)*(size_t)query->trace.count); query->activation_count+=query->trace.count;
        for (j=0;j<evaluation->checks_count;++j) { GD_RuleWriteCheck check=evaluation->checks[j]; check.first+=variant->first_write; check.second+=variant->first_write; query->checks[query->check_count++]=check; }
        ++query->variant_count; /* Family helper can now inspect this variant's explicit value. */
        for (j=variant->first_activation;j<query->activation_count && !query->limited;++j) {
            const GD_RuleActivation *activation=&query->activations[j]; const GD_RuleCase *item=&rules->cases[activation->case_index]; int p;
            if (!charge(query,1)) break;
            if (activation->applicability==GD_RULE_TRUTH_FALSE) continue;
            member(variant->cases,activation->case_index);
            for (p=item->first_participant;p<item->first_participant+item->participant_count;++p) {
                int mapped;
                if (!charge(query,GD_MAX_RULE_EXPANSION_DEPTH*GD_MAX_RULE_CALL_BINDINGS)) break;
                mapped=gd_actor_trace_participant(analysis,activation,p);
                if (mapped>=root->first_participant && mapped<root->first_participant+root->participant_count && mapped!=actor) member(variant->partners,mapped);
            }
        }
        for (j=variant->first_write;j<query->write_count && !query->limited;++j) { int effect=effect_member(query,query->variant_count-1,j); query->write_effects[j]=effect; if (effect>=0) member(variant->effects,effect); }
        if (evaluation->limited) query->limited=1;
        if (query->limited) variant->limited=1;
    }
    query->unexamined_variants=n-query->variant_count; return query;
failure:
    gd_actor_query_destroy(query); return NULL;
}
void gd_actor_query_destroy(GD_ActorQuery *query) { if (query) query->allocator.release(query->allocator.context,query); }
static GD_ActorSetComparison compare_set(const unsigned long long *a, const unsigned long long *b, int words) {
    GD_ActorSetComparison result={0}; int i,bit;
    for (i=0;i<words;++i) for (bit=0;bit<64;++bit) {
        int first=!!(a[i]&(1ULL<<bit)),second=!!(b[i]&(1ULL<<bit));
        result.shared+=first && second; result.first_only+=first && !second; result.second_only+=!first && second;
    }
    return result;
}
int gd_actor_write_effect(const GD_ActorQuery *query, int variant, int write) {
    const GD_ActorVariant *item;
    if (!query || variant<0 || variant>=query->variant_count) return -1;
    item=&query->variants[variant];
    if (write<item->first_write || write>=item->first_write+item->write_count) return -1;
    return query->write_effects[write];
}
int gd_actor_compare_member(const GD_ActorQuery *query, int first, int second, GD_ActorSet set, int member) {
    const unsigned long long *a,*b; int count;
    if (!query || first<0 || first>=query->variant_count || second<0 || second>=query->variant_count || member<0) return -1;
    if (set==GD_ACTOR_SET_CASES) { a=query->variants[first].cases; b=query->variants[second].cases; count=query->analysis->rules->case_count; }
    else if (set==GD_ACTOR_SET_PARTNERS) { a=query->variants[first].partners; b=query->variants[second].partners; count=query->analysis->rules->participant_count; }
    else if (set==GD_ACTOR_SET_EFFECTS) { a=query->variants[first].effects; b=query->variants[second].effects; count=query->effect_count; }
    else return -1;
    if (member>=count) return -1;
    return ((a[member/64]>>(member%64))&1u) | (((b[member/64]>>(member%64))&1u)<<1);
}
int gd_actor_compare(const GD_ActorQuery *query, int first, int second, GD_ActorComparison *result) {
    if (!query || !result || first<0 || second<0 || first>=query->variant_count || second>=query->variant_count) return 0;
    result->cases=compare_set(query->variants[first].cases,query->variants[second].cases,GD_ACTOR_CASE_WORDS);
    result->partners=compare_set(query->variants[first].partners,query->variants[second].partners,GD_ACTOR_PARTNER_WORDS);
    result->effects=compare_set(query->variants[first].effects,query->variants[second].effects,GD_ACTOR_EFFECT_WORDS);
    result->complete=!query->variants[first].limited && !query->variants[second].limited && query->variants[first].constraints_complete && query->variants[second].constraints_complete && query->variants[first].values_complete && query->variants[second].values_complete; return 1;
}
