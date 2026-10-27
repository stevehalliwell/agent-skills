#include "gd_rule_analysis.h"
#include <assert.h>
#include <math.h>
#include <string.h>

static int space(unsigned char c) { return c==' ' || c=='\t'; }
static int span_equal(const char *text, size_t length, const char *value) {
    return value && !strncmp(text,value,length) && !value[length];
}
static int compile_charge(GD_RuleAnalysis *analysis, size_t amount) {
    if (analysis->compile_limited) return 0;
    if (amount>(size_t)(GD_RULE_ANALYSIS_WORK_LIMIT-analysis->compile_work)) { analysis->compile_limited=1; return 0; }
    analysis->compile_work+=(int)amount; return 1;
}
static int evaluation_charge(GD_RuleEvaluation *result, size_t amount) {
    if (result->limited) return 0;
    if (amount>(size_t)(GD_RULE_EVALUATION_WORK_LIMIT-result->work)) {
        result->limited=1; result->constraints_complete=result->values_complete=0; return 0;
    }
    result->work+=(int)amount; return 1;
}
static void call_error(GD_Model *model, const GD_Rules *rules, int case_index, int line, const char *code, const char *message) {
    gd_error(model,code,message,model->records[rules->sources[rules->cases[case_index].source].record].source_path,line);
}
static int participant_named(const GD_Rules *rules, int case_index, const char *text, size_t length) {
    const GD_RuleCase *item=&rules->cases[case_index]; int p;
    for (p=item->first_participant;p<item->first_participant+item->participant_count;++p)
        if (span_equal(text,length,rules->participants[p].name)) return p;
    return -1;
}
int gd_rule_case_find(const GD_Model *model, const GD_Rules *rules, const char *name) {
    const char *separator; size_t length; int c, found=-1;
    if (!model || !rules || !name) return -1;
    separator=strchr(name,'#'); length=separator ? (size_t)(separator-name):strlen(name);
    for (c=0;c<rules->case_count;++c) {
        const GD_RuleCase *item=&rules->cases[c]; const GD_Record *record=&model->records[rules->sources[item->source].record];
        if (!span_equal(name,length,record->reference) || (separator && strcmp(separator+1,item->name))) continue;
        if (found>=0) return -2;
        found=c;
    }
    return found;
}
typedef struct { const char *name; size_t length; int caller, used; } MapSpec;
static void compile_call(GD_Model *model, GD_RuleAnalysis *analysis, int case_index, int participant_index,
                         const char *text, size_t length) {
    const GD_Rules *rules=analysis->rules;
    const GD_RuleParticipant *participant=&rules->participants[participant_index];
    const char *cursor=text, *end=text+length, *reference, *filter=NULL;
    size_t reference_length, filter_length=0;
    MapSpec maps[GD_MAX_RULE_CALL_BINDINGS]; int map_count=0, explicit_map=0, i,c,p,source=-1,call_index,before;
    GD_RuleCall *call;
    if (!compile_charge(analysis,(length+1)*(GD_MAX_RECORDS+GD_MAX_RULE_CALL_BINDINGS))) return;
    if (analysis->call_count==GD_MAX_RULE_CALLS) { analysis->compile_limited=1; return; }
    call_index=analysis->call_count++; call=&analysis->calls[call_index]; memset(call,0,sizeof(*call));
    call->caller_case=case_index; call->caller=participant_index; call->line=participant->lines[6];
    call->text=text; call->length=length; call->target_source=-1; call->resolution=GD_RULE_UNDETERMINED;
    call->next=analysis->call_first[participant_index]; analysis->call_first[participant_index]=call_index;
    while (cursor<end && space((unsigned char)*cursor)) ++cursor;
    reference=cursor; while (cursor<end && !space((unsigned char)*cursor) && *cursor!='#') ++cursor;
    reference_length=(size_t)(cursor-reference);
    if (cursor<end && *cursor=='#') { filter=++cursor; while (cursor<end && !space((unsigned char)*cursor)) ++cursor; filter_length=(size_t)(cursor-filter); }
    if (!reference_length || !memchr(reference,'/',reference_length) || (filter && !filter_length)) goto syntax;
    for (i=0;i<rules->source_count;++i) if (span_equal(reference,reference_length,model->records[rules->sources[i].record].reference)) {
        if (source>=0) { call->resolution=GD_RULE_AMBIGUOUS; goto invalid_reference; } source=i;
    }
    if (source<0) {
        int declared=0;
        const char *slash=memchr(reference,'/',reference_length);
        for (i=0;i<model->collection_count;++i) if (span_equal(reference,(size_t)(slash-reference),model->collections[i].alias)) declared=1;
        call->resolution=declared ? GD_RULE_MISSING:GD_RULE_UNLOADED;
        if (declared) goto invalid_reference;
        return;
    }
    call->target_source=source;
    call->first_case=rules->sources[source].first_case; call->case_count=rules->sources[source].case_count;
    if (filter) {
        int found=-1;
        for (c=call->first_case;c<call->first_case+call->case_count;++c) if (span_equal(filter,filter_length,rules->cases[c].name)) found=c;
        if (found<0) { call->resolution=GD_RULE_MISSING; goto invalid_reference; }
        call->first_case=found; call->case_count=1;
    }
    if (!call->case_count) return;
    while (cursor<end && space((unsigned char)*cursor)) ++cursor;
    if (cursor<end) {
        if ((size_t)(end-cursor)<5 || memcmp(cursor,"with",4) || !space((unsigned char)cursor[4])) goto syntax;
        cursor+=4; explicit_map=1;
        while (cursor<end) {
            const char *name,*finish,*caller,*caller_end;
            while (cursor<end && space((unsigned char)*cursor)) ++cursor;
            name=cursor; while (cursor<end && *cursor!='=' && *cursor!=',') ++cursor; finish=cursor;
            while (finish>name && space((unsigned char)finish[-1])) --finish;
            if (cursor==end || *cursor!='=' || finish==name || map_count==GD_MAX_RULE_CALL_BINDINGS) goto syntax;
            caller=++cursor; while (caller<end && space((unsigned char)*caller)) ++caller;
            cursor=caller; while (cursor<end && *cursor!=',') ++cursor; caller_end=cursor;
            while (caller_end>caller && space((unsigned char)caller_end[-1])) --caller_end;
            if (caller==caller_end) goto syntax;
            for (i=0;i<map_count;++i) if (maps[i].length==(size_t)(finish-name) && !memcmp(maps[i].name,name,maps[i].length)) goto syntax;
            if (!compile_charge(analysis,(size_t)rules->cases[case_index].participant_count*((size_t)(caller_end-caller)+1))) return;
            maps[map_count]=(MapSpec){name,(size_t)(finish-name),participant_named(rules,case_index,caller,(size_t)(caller_end-caller)),0};
            if (maps[map_count].caller<0) goto binding;
            ++map_count;
            if (cursor<end) { ++cursor; if (cursor==end) goto syntax; }
        }
        if (!map_count) goto syntax;
    }
    before=analysis->binding_count; call->first_binding=before;
    for (c=call->first_case;c<call->first_case+call->case_count;++c) {
        const GD_RuleCase *callee=&rules->cases[c];
        if (!explicit_map && callee->participant_count!=1) goto binding_rollback;
        for (p=callee->first_participant;p<callee->first_participant+callee->participant_count;++p) {
            const GD_RuleParticipant *parameter=&rules->participants[p]; int caller=participant_index;
            if (!compile_charge(analysis,1)) { analysis->binding_count=before; return; }
            if (explicit_map) {
                caller=-1;
                for (i=0;i<map_count;++i) {
                    if (!compile_charge(analysis,maps[i].length+1)) { analysis->binding_count=before; return; }
                    if (span_equal(maps[i].name,maps[i].length,parameter->name)) { caller=maps[i].caller; maps[i].used=1; }
                }
                if (caller<0) goto binding_rollback;
            }
            if (parameter->noun_resolution==GD_RULE_RESOLVED && rules->participants[caller].noun_resolution==GD_RULE_RESOLVED && parameter->noun_record!=rules->participants[caller].noun_record) goto type_rollback;
            if (analysis->binding_count==GD_MAX_RULE_BINDINGS) { analysis->compile_limited=1; analysis->binding_count=before; return; }
            analysis->bindings[analysis->binding_count++]=(GD_RuleBinding){p,caller};
        }
    }
    for (i=0;i<map_count;++i) if (!maps[i].used) goto binding_rollback;
    call->binding_count=analysis->binding_count-before; call->resolution=GD_RULE_RESOLVED; return;
type_rollback:
    analysis->binding_count=before; call->resolution=GD_RULE_WRONG_TYPE;
    call_error(model,rules,case_index,call->line,"rules.invocation_type","Explicit callee/caller noun types differ; no inferred transformation or name/type-based mapping"); return;
binding_rollback:
    analysis->binding_count=before;
binding:
    call->resolution=GD_RULE_MISSING;
    call_error(model,rules,case_index,call->line,"rules.invocation_binding","Map every selected callee participant explicitly to a caller label; shorthand requires exactly one participant in every selected case"); return;
syntax:
    call->resolution=GD_RULE_UNAVAILABLE;
    call_error(model,rules,case_index,call->line,"rules.invocation_syntax","Expected ALIAS/NAME[#CASE] optionally followed by with Callee=Caller, Other=OtherCaller; separate calls with semicolons"); return;
invalid_reference:
    call_error(model,rules,case_index,call->line,"rules.invocation_reference","Invocation must identify one supplied rule source and existing selected case; never infer missing rules");
}
GD_RuleAnalysis *gd_analyze_rules(GD_Model *model, const GD_Rules *rules) {
    GD_RuleAnalysis *analysis; int c,p;
    assert(model && rules);
    analysis=model->allocator.allocate(model->allocator.context,sizeof(*analysis));
    if (!analysis) { gd_error(model,"rules.analysis_memory","Cannot allocate bounded rule analysis",NULL,0); return NULL; }
    memset(analysis,0,sizeof(*analysis)); analysis->allocator=model->allocator; analysis->model=model; analysis->rules=rules; analysis->requested_case=-1;
    for (p=0;p<GD_MAX_RULE_PARTICIPANTS;++p) analysis->call_first[p]=-1;
    for (p=0;p<GD_MAX_RECORDS;++p) analysis->verb_first[p]=-1;
    for (p=0;p<model->edge_count;++p) if (model->edges[p].field==GD_VERB_REF || model->edges[p].field==GD_VERBS) {
        int source=model->edges[p].source;
        analysis->verb_next[p]=analysis->verb_first[source]; analysis->verb_first[source]=p;
    }
    for (c=0;c<rules->case_count && !analysis->compile_limited;++c) {
        const GD_RuleCase *item=&rules->cases[c];
        for (p=item->first_participant;p<item->first_participant+item->participant_count && !analysis->compile_limited;++p) {
            const char *cursor=rules->participants[p].causes;
            if (!cursor) continue;
            while (*cursor && !analysis->compile_limited) {
                const char *start=cursor,*end;
                while (*cursor && *cursor!=';') ++cursor; end=cursor;
                while (end>start && space((unsigned char)end[-1])) --end;
                compile_call(model,analysis,c,p,start,(size_t)(end-start));
                if (*cursor==';') { ++cursor; if (!*cursor) call_error(model,rules,c,rules->participants[p].lines[6],"rules.invocation_syntax","Trailing empty invocation is not a rule"); }
            }
            analysis->calls_complete[p]=(unsigned char)!analysis->compile_limited;
        }
    }
    if (analysis->compile_limited) gd_error(model,"rules.invocation_limit","Invocation work/call/binding limit exceeded; remaining expansion unavailable",NULL,0);
    return analysis;
}
void gd_rule_analysis_destroy(GD_RuleAnalysis *analysis) {
    if (analysis) analysis->allocator.release(analysis->allocator.context,analysis);
}
static GD_RuleTruth truth_and(GD_RuleTruth a, GD_RuleTruth b) {
    return a==GD_RULE_TRUTH_FALSE || b==GD_RULE_TRUTH_FALSE ? GD_RULE_TRUTH_FALSE:
        a==GD_RULE_TRUTH_TRUE && b==GD_RULE_TRUTH_TRUE ? GD_RULE_TRUTH_TRUE:GD_RULE_TRUTH_UNKNOWN;
}
static GD_RuleTruth truth_not(GD_RuleTruth a) { return a==GD_RULE_TRUTH_TRUE ? GD_RULE_TRUTH_FALSE:a==GD_RULE_TRUTH_FALSE ? GD_RULE_TRUTH_TRUE:GD_RULE_TRUTH_UNKNOWN; }
const char *gd_rule_truth_name(GD_RuleTruth truth) { return truth==GD_RULE_TRUTH_TRUE ? "true":truth==GD_RULE_TRUTH_FALSE ? "false":"unknown"; }
const char *gd_rule_relation_name(GD_RuleRelation relation) {
    return relation==GD_RULE_RELATION_EQUIVALENT ? "equivalent":relation==GD_RULE_RELATION_REDUNDANT ? "redundant":
        relation==GD_RULE_RELATION_COMPATIBLE ? "compatible_overlap":relation==GD_RULE_RELATION_CONFLICT ? "conflict":
        relation==GD_RULE_RELATION_DISJOINT ? "disjoint":"undetermined";
}
static int local_facts(GD_RuleEvaluation *result, GD_RuleAssumptions inputs, int participant, GD_RulePredicateFact *facts) {
    int i,count=0,actor=result->actor_map[participant];
    for (i=0;i<inputs.fact_count;++i) {
        if (!evaluation_charge(result,1)) break;
        if (inputs.facts[i].participant==actor) { facts[count]=inputs.facts[i]; facts[count++].participant=participant; }
    }
    return count;
}
static GD_RuleTruth condition_value(const GD_RuleAnalysis *analysis, GD_RuleEvaluation *result,
                                    const GD_RuleCondition *condition, GD_RuleAssumptions inputs) {
    GD_RulePredicateFact facts[GD_MAX_RULE_CONDITION_ITEMS]; int count;
    if (condition->state!=GD_CONDITION_PARSED) return GD_RULE_TRUTH_UNKNOWN;
    count=local_facts(result,inputs,condition->participant,facts);
    if (!evaluation_charge(result,gd_rule_condition_work(analysis->rules,condition,count))) return GD_RULE_TRUTH_UNKNOWN;
    return gd_rule_condition_value(analysis->rules,condition,facts,count);
}
static GD_RuleTruth context_value(const GD_RuleAnalysis *analysis, GD_RuleEvaluation *result, const GD_RuleContext *context, GD_RuleAssumptions inputs) {
    GD_RuleTruth found=GD_RULE_TRUTH_UNKNOWN; int i,seen=0;
    (void)analysis;
    if (context->state!=GD_CONDITION_PARSED || !context->bindings_complete) return found;
    if (context->unrestricted) return GD_RULE_TRUTH_TRUE;
    for (i=0;i<inputs.context_count;++i) {
        const GD_RuleContextFact *fact=&inputs.contexts[i];
        if (!evaluation_charge(result,context->operator_length+2)) return GD_RULE_TRUTH_UNKNOWN;
        if (fact->left!=result->actor_map[context->left] || fact->right!=result->actor_map[context->right] || fact->axis!=context->target.axis_record ||
            !span_equal(context->target.state,strlen(context->target.state),fact->value)) continue;
        if (seen && found!=fact->truth) return GD_RULE_TRUTH_UNKNOWN;
        found=fact->truth; seen=1;
    }
    return found;
}
static GD_RuleTruth axis_scope(const GD_RuleAnalysis *analysis, GD_RuleEvaluation *result, int participant_index) {
    const GD_Rules *rules=analysis->rules;
    const GD_RuleParticipant *participant=&rules->participants[participant_index];
    const GD_RuleCondition *condition=&participant->required_condition;
    int edge,i;
    if (participant->noun_resolution!=GD_RULE_RESOLVED || participant->other_axes_mode==GD_RULE_OTHER_INVALID) return GD_RULE_TRUTH_UNKNOWN;
    if (participant->other_axes_mode==GD_RULE_OTHER_ANY || (condition->token_count==1 && rules->condition_tokens[condition->first_token].operation==GD_RULE_OP_ANY)) return GD_RULE_TRUTH_TRUE;
    if (!rules->actor_axes_complete[participant->noun_record]) return GD_RULE_TRUTH_UNKNOWN;
    for (edge=rules->actor_first[participant->noun_record];edge>=0;edge=rules->next_axis[edge]) {
        int axis=analysis->model->edges[edge].target,mentioned=0;
        if (!evaluation_charge(result,1) || rules->axis_resolution[edge]!=GD_RULE_RESOLVED) return GD_RULE_TRUTH_UNKNOWN;
        for (i=condition->first_token;i<condition->first_token+condition->token_count;++i) {
            const GD_RuleConditionToken *token=&rules->condition_tokens[i];
            if (!evaluation_charge(result,1)) return GD_RULE_TRUTH_UNKNOWN;
            if ((token->operation==GD_RULE_OP_STATE || token->operation==GD_RULE_OP_COMPARE) && token->target.axis_record==axis && token->target.resolution==GD_RULE_RESOLVED) mentioned=1;
        }
        if (!mentioned) return GD_RULE_TRUTH_UNKNOWN;
    }
    return GD_RULE_TRUTH_TRUE;
}
static GD_RuleTruth case_value(const GD_RuleAnalysis *analysis, GD_RuleEvaluation *result, int case_index,
                              GD_RuleAssumptions inputs, int root) {
    const GD_RuleCase *item=&analysis->rules->cases[case_index];
    GD_RuleTruth context=context_value(analysis,result,&item->ordered_context,inputs);
    GD_RuleTruth positive=item->participant_count ? GD_RULE_TRUTH_TRUE:GD_RULE_TRUTH_UNKNOWN;
    GD_RuleExclusionCheck exclusions={GD_EXCLUSION_ABSENT,0,0,-1,0,0};
    int p,unknown=0,work=result->work;
    for (p=item->first_participant;p<item->first_participant+item->participant_count;++p) {
        const GD_RuleParticipant *participant=&analysis->rules->participants[p];
        GD_RuleTruth required=condition_value(analysis,result,&participant->required_condition,inputs);
        if ((participant->required_condition.state==GD_CONDITION_PARSED && !participant->required_condition.bindings_complete) ||
            (participant->skip_condition.state==GD_CONDITION_PARSED && !participant->skip_condition.bindings_complete)) result->constraints_complete=0;
        positive=truth_and(positive,truth_and(required,axis_scope(analysis,result,p)));
        if (participant->skip_if) {
            GD_RuleTruth guard;
            ++exclusions.guard_count;
            if (exclusions.state==GD_EXCLUSION_EXCLUDED) continue;
            guard=condition_value(analysis,result,&participant->skip_condition,inputs);
            if (result->limited) { unknown=1; continue; }
            ++exclusions.checked_count;
            if (guard==GD_RULE_TRUTH_TRUE) { exclusions.state=GD_EXCLUSION_EXCLUDED; exclusions.excluded_by=p; }
            else if (guard==GD_RULE_TRUTH_UNKNOWN) unknown=1;
        }
    }
    if (item->unknown_conditions) positive=truth_and(positive,GD_RULE_TRUTH_UNKNOWN);
    if (exclusions.state!=GD_EXCLUSION_EXCLUDED && exclusions.guard_count) exclusions.state=unknown ? GD_EXCLUSION_UNKNOWN:GD_EXCLUSION_NOT_EXCLUDED;
    exclusions.work=result->work-work; exclusions.limited=result->limited;
    if (root) { result->context=context; result->requirements=positive; result->exclusion=exclusions; }
    if (exclusions.state==GD_EXCLUSION_EXCLUDED) return GD_RULE_TRUTH_FALSE;
    return truth_and(truth_and(positive,context),exclusions.state==GD_EXCLUSION_UNKNOWN ? GD_RULE_TRUTH_UNKNOWN:GD_RULE_TRUTH_TRUE);
}
static size_t bounded_length(const char *text, size_t limit) {
    size_t length=0;
    if (!text) return limit+1;
    while (length<=limit && text[length]) ++length;
    return length;
}
static int assumptions_valid(const GD_RuleAnalysis *analysis, GD_RuleEvaluation *result, GD_RuleAssumptions inputs) {
    int i,j; double numbers[GD_MAX_RULE_CONDITION_ITEMS];
    const GD_RuleCase *root=&analysis->rules->cases[result->root_case];
    if (inputs.fact_count<0 || inputs.fact_count>GD_MAX_RULE_CONDITION_ITEMS || (inputs.fact_count && !inputs.facts) ||
        inputs.context_count<0 || inputs.context_count>GD_MAX_RULE_CONTEXT_FACTS || (inputs.context_count && !inputs.contexts)) return 0;
    for (i=0;i<inputs.fact_count;++i) {
        const GD_RulePredicateFact *fact=&inputs.facts[i];
        size_t length=bounded_length(fact->value,GD_MAX_RULE_LINE);
        if (!evaluation_charge(result,length+1)) return -1;
        if (!length || length>GD_MAX_RULE_LINE || fact->participant<0 || fact->participant>=analysis->rules->participant_count ||
            fact->axis_record<0 || fact->axis_record>=analysis->model->record_count || analysis->model->records[fact->axis_record].role!=GD_AXIS ||
            fact->truth<GD_RULE_TRUTH_UNKNOWN || fact->truth>GD_RULE_TRUTH_TRUE) return 0;
        {
            int edge,member=0;
            if (fact->participant<root->first_participant || fact->participant>=root->first_participant+root->participant_count ||
                analysis->rules->participants[fact->participant].noun_resolution!=GD_RULE_RESOLVED || !analysis->rules->axes[fact->axis_record].valid || analysis->rules->axes[fact->axis_record].type==GD_AXIS_UNSPECIFIED) return 0;
            for (edge=analysis->rules->actor_first[analysis->rules->participants[fact->participant].noun_record];edge>=0;edge=analysis->rules->next_axis[edge]) {
                if (!evaluation_charge(result,1)) return -1;
                if (analysis->rules->axis_resolution[edge]==GD_RULE_RESOLVED && analysis->model->edges[edge].target==fact->axis_record) member=1;
            }
            if (!member) return 0;
        }
        if (analysis->rules->axes[fact->axis_record].type==GD_AXIS_ENUM && analysis->model->records[fact->axis_record].axis_closed) {
            const GD_ValueDefinition *definition=&analysis->model->records[fact->axis_record].axis;
            const char *value=definition->allowed; int k,found=0;
            for (k=0;k<definition->allowed_count;++k) {
                size_t size=bounded_length(value,GD_MAX_FRONTMATTER);
                if (!evaluation_charge(result,size+length+2)) return -1;
                if (!strcmp(value,fact->value)) found=1;
                value+=size+1;
            }
            if (!found) return 0;
        }
        if (analysis->rules->axes[fact->axis_record].type==GD_AXIS_INTEGER || analysis->rules->axes[fact->axis_record].type==GD_AXIS_NUMBER) {
            if (!evaluation_charge(result,GD_RULE_NUMERIC_FACT_WORK)) return -1;
            if (length>=128 || !gd_control_number(fact->value,&numbers[i],analysis->rules->axes[fact->axis_record].type==GD_AXIS_INTEGER)) return 0;
        }
        for (j=0;j<i;++j) {
            const GD_RulePredicateFact *other=&inputs.facts[j];
            if (!evaluation_charge(result,length+2)) return -1;
            if (fact->participant!=other->participant || fact->axis_record!=other->axis_record) continue;
            if (!strcmp(fact->value,other->value) && fact->truth!=GD_RULE_TRUTH_UNKNOWN && other->truth!=GD_RULE_TRUTH_UNKNOWN && fact->truth!=other->truth) return 0;
            if (analysis->rules->axes[fact->axis_record].type==GD_AXIS_ENUM && fact->truth==GD_RULE_TRUTH_TRUE && other->truth==GD_RULE_TRUTH_TRUE && strcmp(fact->value,other->value)) return 0;
            if (analysis->rules->axes[fact->axis_record].type==GD_AXIS_INTEGER || analysis->rules->axes[fact->axis_record].type==GD_AXIS_NUMBER) {
                if (numbers[i]!=numbers[j] && fact->truth==GD_RULE_TRUTH_TRUE && other->truth==GD_RULE_TRUTH_TRUE) return 0;
                if (numbers[i]==numbers[j] && fact->truth!=other->truth && fact->truth!=GD_RULE_TRUTH_UNKNOWN && other->truth!=GD_RULE_TRUTH_UNKNOWN) return 0;
            }
        }
    }
    for (i=0;i<inputs.context_count;++i) {
        const GD_RuleContextFact *fact=&inputs.contexts[i];
        size_t length=bounded_length(fact->value,GD_MAX_RULE_LINE);
        if (!evaluation_charge(result,length+1)) return -1;
        if (!length || length>GD_MAX_RULE_LINE || fact->left<0 || fact->right<0 || fact->left>=analysis->rules->participant_count || fact->right>=analysis->rules->participant_count ||
            fact->axis<0 || fact->axis>=analysis->model->record_count || analysis->model->records[fact->axis].role!=GD_AXIS || !analysis->rules->axes[fact->axis].valid || analysis->rules->axes[fact->axis].type!=GD_AXIS_ENUM ||
            fact->truth<GD_RULE_TRUTH_UNKNOWN || fact->truth>GD_RULE_TRUTH_TRUE || fact->left<root->first_participant || fact->right<root->first_participant ||
            fact->left>=root->first_participant+root->participant_count || fact->right>=root->first_participant+root->participant_count) return 0;
        if (analysis->model->records[fact->axis].axis_closed) {
            const GD_ValueDefinition *definition=&analysis->model->records[fact->axis].axis;
            const char *state=definition->allowed; int k,found=0;
            for (k=0;k<definition->allowed_count;++k) {
                size_t size=bounded_length(state,GD_MAX_FRONTMATTER);
                if (!evaluation_charge(result,size+length+2)) return -1;
                if (!strcmp(state,fact->value)) found=1;
                state+=size+1;
            }
            if (!found) return 0;
        }
        for (j=0;j<i;++j) {
            const GD_RuleContextFact *other=&inputs.contexts[j];
            if (!evaluation_charge(result,length+2)) return -1;
            if (fact->left==other->left && fact->right==other->right && fact->axis==other->axis && !strcmp(fact->value,other->value) &&
                fact->truth!=GD_RULE_TRUTH_UNKNOWN && other->truth!=GD_RULE_TRUTH_UNKNOWN && fact->truth!=other->truth) return 0;
        }
    }
    return 1;
}
static const char *current_state(GD_RuleEvaluation *result, GD_RuleAssumptions inputs, int actor, int axis) {
    const char *value=NULL; int i;
    for (i=0;i<inputs.fact_count;++i) {
        const GD_RulePredicateFact *fact=&inputs.facts[i];
        if (!evaluation_charge(result,1)) return NULL;
        if (fact->participant==actor && fact->axis_record==axis && fact->truth==GD_RULE_TRUTH_TRUE) value=fact->value;
    }
    if (!value) return NULL;
    for (i=0;i<inputs.fact_count;++i) {
        const GD_RulePredicateFact *fact=&inputs.facts[i];
        size_t length=bounded_length(value,GD_MAX_RULE_LINE);
        if (!evaluation_charge(result,length+2)) return NULL;
        if (fact->participant==actor && fact->axis_record==axis && fact->truth!=GD_RULE_TRUTH_TRUE && !strcmp(value,fact->value)) return NULL;
    }
    return value;
}
static GD_RuleWrite *new_write(GD_RuleEvaluation *result, int actor, int axis, int origin_case, int participant,
                               int line, int conditional, const int *path, int depth) {
    GD_RuleWrite *write;
    if (!evaluation_charge(result,(size_t)depth+1)) return NULL;
    if (result->write_count==GD_MAX_RULE_WRITES) { result->limited=1; result->constraints_complete=result->values_complete=0; return NULL; }
    write=&result->writes[result->write_count++]; memset(write,0,sizeof(*write));
    write->actor=actor; write->axis=axis; write->origin_case=origin_case; write->origin_participant=participant;
    write->line=line; write->quantity=-1; write->conditional=conditional; write->path_count=depth;
    memcpy(write->path,path,(size_t)depth*sizeof(*path));
    if (actor<0 || axis<0 || conditional) result->constraints_complete=0;
    if (actor<0 || axis<0) result->opaque_effects=1;
    return write;
}
static void case_writes(const GD_RuleAnalysis *analysis, GD_RuleEvaluation *result, int case_index, int conditional,
                        GD_RuleAssumptions inputs, const int *path, int depth) {
    const GD_Rules *rules=analysis->rules; const GD_RuleCase *item=&rules->cases[case_index]; int p,i;
    if (item->unknown_effects) { result->constraints_complete=0; result->opaque_effects=1; }
    for (p=item->first_participant;p<item->first_participant+item->participant_count && !result->limited;++p) {
        const GD_RuleParticipant *participant=&rules->participants[p];
        if (participant->causes && !analysis->calls_complete[p]) { result->constraints_complete=0; result->opaque_effects=1; }
        if (participant->becomes && !participant->state_effect_count) { result->constraints_complete=0; result->opaque_effects=1; }
        for (i=participant->first_state_effect;i<participant->first_state_effect+participant->state_effect_count;++i) {
            const GD_RuleStateEffect *effect=&rules->state_effects[i];
            GD_RuleWrite *write=new_write(result,result->actor_map[p],effect->target.resolution==GD_RULE_RESOLVED ? effect->target.axis_record:-1,case_index,p,effect->line,conditional,path,depth);
            if (!write) return;
            write->kind=GD_WRITE_STATE; write->state=effect->target.resolution==GD_RULE_RESOLVED ? effect->target.state:NULL;
            if (!write->state) result->constraints_complete=result->values_complete=0;
        }
        for (i=participant->first_assignment;i<participant->first_assignment+participant->assignment_count;++i) {
            const GD_RuleAssignment *assignment=&rules->assignments[i];
            GD_RulePredicateFact facts[GD_MAX_RULE_CONDITION_ITEMS];
            int count=local_facts(result,inputs,p,facts), index=assignment->quantity;
            GD_RuleWrite *write=new_write(result,result->actor_map[p],assignment->target.resolution==GD_RULE_RESOLVED ? assignment->target.axis_record:-1,case_index,p,assignment->line,conditional,path,depth);
            if (!write) return;
            write->kind=GD_WRITE_QUANTITY; write->quantity=index;
            if (index>=0 && rules->quantities[index].state==GD_CONDITION_PARSED) {
                const GD_RuleQuantity *quantity=&rules->quantities[index];
                if (!evaluation_charge(result,(size_t)(quantity->token_count+1)*GD_RULE_NUMERIC_TOKEN_WORK+(size_t)count*GD_RULE_NUMERIC_FACT_WORK)) return;
                write->value=gd_rule_quantity_value(rules,index,facts,count);
                if (!quantity->bindings_complete || !write->value.checks_complete || gd_expression_has_unknown(rules->quantity_tokens+quantity->first_token,quantity->token_count)) result->constraints_complete=0;
            } else result->constraints_complete=0;
            if (!write->value.known) result->values_complete=0;
            if (write->value.state==GD_EXPRESSION_ARITHMETIC_ERROR || write->value.state==GD_EXPRESSION_UNIT_ERROR) result->constraints_complete=0;
        }
        if (participant->preserve && !participant->preservation_count) result->constraints_complete=0;
        for (i=participant->first_preservation;i<participant->first_preservation+participant->preservation_count;++i) {
            GD_RuleDeferredPreservation *promise;
            if (!evaluation_charge(result,(size_t)depth+1)) return;
            if (result->deferred_count==GD_MAX_RULE_PRESERVATIONS) { result->limited=1; result->constraints_complete=0; return; }
            promise=&result->deferred[result->deferred_count++];
            promise->actor=result->actor_map[p]; promise->preservation=i; promise->origin_case=case_index; promise->origin_participant=p;
            promise->conditional=conditional; promise->path_count=depth; memcpy(promise->path,path,(size_t)depth*sizeof(*path));
        }
    }
}
static void preserve_axis(const GD_RuleAnalysis *analysis, GD_RuleEvaluation *result, const GD_RuleDeferredPreservation *promise,
                           int axis, GD_RuleAssumptions inputs) {
    const GD_RulePreservation *source=&analysis->rules->preservations[promise->preservation];
    GD_RuleWrite *write; int i,conditional=promise->conditional;
    if (source->kind==GD_PRESERVE_OTHER) {
        if (result->opaque_effects) conditional=1;
        for (i=0;i<result->write_count;++i) {
            const GD_RuleWrite *candidate=&result->writes[i];
            if (!evaluation_charge(result,1)) return;
            if (candidate->kind==GD_WRITE_PRESERVE || candidate->actor!=promise->actor || candidate->axis!=axis) continue;
            if (!candidate->conditional) return;
            conditional=1; /* Candidate may write; remaining-axis promise is conditional too. */
        }
    }
    write=new_write(result,promise->actor,axis,promise->origin_case,promise->origin_participant,source->line,conditional,promise->path,promise->path_count);
    if (!write) return;
    write->kind=GD_WRITE_PRESERVE;
    if (analysis->rules->axes[axis].type==GD_AXIS_ENUM) {
        write->state=current_state(result,inputs,promise->actor,axis);
        if (!write->state) result->values_complete=0;
    } else if (analysis->rules->axes[axis].type==GD_AXIS_INTEGER || analysis->rules->axes[axis].type==GD_AXIS_NUMBER) {
        GD_ExpressionInput starting;
        if (!evaluation_charge(result,(size_t)(inputs.fact_count+1)*GD_RULE_NUMERIC_FACT_WORK)) return;
        starting=gd_rule_starting_quantity(analysis->rules,promise->actor,axis,inputs.facts,inputs.fact_count);
        write->value.state=starting.known ? GD_EXPRESSION_VALUE:GD_EXPRESSION_UNKNOWN;
        write->value.number=starting.value.number; write->value.known=starting.known; write->value.checks_complete=starting.unit_known;
        if (!evaluation_charge(result,GD_RULE_NUMERIC_FACT_WORK)) return;
        if (starting.known) write->value.bounds=gd_rule_quantity_bounds(analysis->rules,axis,starting.value.number);
        if (!starting.known) result->values_complete=0;
    } else result->constraints_complete=result->values_complete=0;
}
static void whole_case_preservation(const GD_RuleAnalysis *analysis, GD_RuleEvaluation *result, GD_RuleAssumptions inputs) {
    int i,edge;
    for (i=0;i<result->deferred_count && !result->limited;++i) {
        const GD_RuleDeferredPreservation *promise=&result->deferred[i];
        const GD_RulePreservation *source=&analysis->rules->preservations[promise->preservation];
        if (promise->actor<0) { result->constraints_complete=0; continue; }
        if (source->kind==GD_PRESERVE_AXIS) {
            if (source->target.resolution==GD_RULE_RESOLVED) preserve_axis(analysis,result,promise,source->target.axis_record,inputs);
            else result->constraints_complete=0;
        } else {
            const GD_RuleParticipant *actor=&analysis->rules->participants[promise->actor];
            if (actor->noun_resolution!=GD_RULE_RESOLVED) { result->constraints_complete=0; continue; }
            if (!analysis->rules->actor_axes_complete[actor->noun_record]) result->constraints_complete=0;
            ++result->generation;
            for (edge=analysis->rules->actor_first[actor->noun_record];edge>=0;edge=analysis->rules->next_axis[edge]) {
                int axis=analysis->model->edges[edge].target;
                if (!evaluation_charge(result,1)) break;
                if (analysis->rules->axis_resolution[edge]!=GD_RULE_RESOLVED) { result->constraints_complete=0; continue; }
                if (result->seen[axis]==result->generation) continue;
                result->seen[axis]=result->generation;
                preserve_axis(analysis,result,promise,axis,inputs);
            }
        }
    }
}
static GD_RuleTruth compatible_writes(const GD_RuleAnalysis *analysis, GD_RuleEvaluation *budget, const GD_RuleWrite *a, const GD_RuleWrite *b) {
    const GD_Rules *rules=analysis->rules;
    if (a->conditional || b->conditional || a->axis<0 || b->axis<0 || a->value.state==GD_EXPRESSION_ARITHMETIC_ERROR ||
        a->value.state==GD_EXPRESSION_UNIT_ERROR || b->value.state==GD_EXPRESSION_ARITHMETIC_ERROR || b->value.state==GD_EXPRESSION_UNIT_ERROR) return GD_RULE_TRUTH_UNKNOWN;
    if (a->state && b->state) {
        size_t length=bounded_length(a->state,GD_MAX_FRONTMATTER);
        if (length>GD_MAX_FRONTMATTER || !evaluation_charge(budget,length+2)) return GD_RULE_TRUTH_UNKNOWN;
        return span_equal(a->state,length,b->state) ? GD_RULE_TRUTH_TRUE:GD_RULE_TRUTH_FALSE;
    }
    if (a->value.known && b->value.known && a->value.state==GD_EXPRESSION_VALUE && b->value.state==GD_EXPRESSION_VALUE)
        return a->value.number==b->value.number ? GD_RULE_TRUTH_TRUE:GD_RULE_TRUTH_FALSE;
    if (a->kind==GD_WRITE_PRESERVE && b->kind==GD_WRITE_PRESERVE) return GD_RULE_TRUTH_TRUE;
    if (a->quantity>=0 && b->quantity>=0) {
        const GD_RuleQuantity *first=&rules->quantities[a->quantity], *second=&rules->quantities[b->quantity];
        if (first->bindings_complete && second->bindings_complete && first->state==GD_CONDITION_PARSED && second->state==GD_CONDITION_PARSED) {
            if (!evaluation_charge(budget,(size_t)(first->token_count+second->token_count+1)*GD_RULE_NUMERIC_TOKEN_WORK)) return GD_RULE_TRUTH_UNKNOWN;
            if (gd_expressions_equal(rules->quantity_tokens+first->first_token,first->token_count,rules->quantity_tokens+second->first_token,second->token_count)) return GD_RULE_TRUTH_TRUE;
        }
    }
    if (a->kind==GD_WRITE_PRESERVE || b->kind==GD_WRITE_PRESERVE) {
        int index=a->kind==GD_WRITE_PRESERVE ? b->quantity:a->quantity;
        if (index>=0 && rules->quantities[index].bindings_complete && gd_expression_is_starting_value(rules->quantity_tokens+rules->quantities[index].first_token,rules->quantities[index].token_count)) return GD_RULE_TRUTH_TRUE;
    }
    return GD_RULE_TRUTH_UNKNOWN;
}
static void write_checks(const GD_RuleAnalysis *analysis, GD_RuleEvaluation *result) {
    int i,j;
    for (i=0;i<result->write_count && !result->limited;++i) for (j=0;j<i;++j) {
        const GD_RuleWrite *a=&result->writes[i], *b=&result->writes[j]; GD_RuleTruth compatible;
        if (!evaluation_charge(result,1)) break;
        if (a->actor<0 || a->axis<0 || a->actor!=b->actor || a->axis!=b->axis) continue;
        compatible=compatible_writes(analysis,result,a,b);
        if (result->limited) break;
        ++result->checked_write_pairs;
        if (compatible==GD_RULE_TRUTH_FALSE) ++result->conflict_count;
        else if (compatible==GD_RULE_TRUTH_UNKNOWN) ++result->uncertain_write_pairs;
        if (compatible==GD_RULE_TRUTH_TRUE) continue;
        if (result->checks_count<GD_MAX_ERRORS) result->checks[result->checks_count++]=(GD_RuleWriteCheck){j,i,truth_not(compatible)};
        else ++result->checks_omitted;
    }
}
static void effect_scope(const GD_RuleAnalysis *analysis, GD_RuleEvaluation *result) {
    const GD_RuleCase *root=&analysis->rules->cases[result->root_case]; int p,edge,i;
    for (p=root->first_participant;p<root->first_participant+root->participant_count && !result->limited;++p) {
        const GD_RuleParticipant *participant=&analysis->rules->participants[p];
        if (participant->noun_resolution!=GD_RULE_RESOLVED) { result->constraints_complete=0; continue; }
        if (!analysis->rules->actor_axes_complete[participant->noun_record]) result->constraints_complete=0;
        for (edge=analysis->rules->actor_first[participant->noun_record];edge>=0;edge=analysis->rules->next_axis[edge]) {
            int axis=analysis->model->edges[edge].target,covered=0;
            if (!evaluation_charge(result,1)) break;
            if (analysis->rules->axis_resolution[edge]!=GD_RULE_RESOLVED) { result->constraints_complete=0; continue; }
            for (i=0;i<result->write_count;++i) {
                if (!evaluation_charge(result,1)) break;
                if (result->writes[i].actor==p && result->writes[i].axis==axis && !result->writes[i].conditional) covered=1;
            }
            if (!covered) result->constraints_complete=0;
        }
    }
}
typedef struct { int case_index, entered, conditional, participant, call, next_case; } ExpansionFrame;
void gd_rule_evaluate(const GD_RuleAnalysis *analysis, int case_index, GD_RuleAssumptions inputs, GD_RuleEvaluation *result) {
    ExpansionFrame stack[GD_MAX_RULE_EXPANSION_DEPTH]; int path[GD_MAX_RULE_EXPANSION_DEPTH],depth=1,p;
    if (!result) return;
    memset(result,0,offsetof(GD_RuleEvaluation,writes)); result->root_case=case_index; result->input_valid=-1; result->exclusion.excluded_by=-1;
    result->constraints_complete=result->values_complete=1; result->deferred_count=0; result->generation=0;
    memset(result->seen,0,sizeof(result->seen));
    if (!analysis || case_index<0 || case_index>=analysis->rules->case_count) { result->input_valid=0; result->constraints_complete=0; return; }
    if (!evaluation_charge(result,GD_MAX_RECORDS)) return;
    result->input_valid=assumptions_valid(analysis,result,inputs);
    if (result->input_valid!=1) { result->constraints_complete=result->values_complete=0; return; }
    result->assumptions=inputs;
    for (p=analysis->rules->cases[case_index].first_participant;p<analysis->rules->cases[case_index].first_participant+analysis->rules->cases[case_index].participant_count;++p) result->actor_map[p]=p;
    stack[0]=(ExpansionFrame){case_index,0,0,0,-1,0};
    while (depth && !result->limited) {
        ExpansionFrame *frame=&stack[depth-1];
        const GD_RuleCase *item=&analysis->rules->cases[frame->case_index];
        if (!evaluation_charge(result,1)) break;
        if (!frame->entered) {
            GD_RuleTruth applicability;
            if (result->expansions==GD_MAX_RULE_EXPANSIONS) { result->limited=1; result->constraints_complete=0; break; }
            ++result->expansions; path[depth-1]=frame->case_index;
            applicability=case_value(analysis,result,frame->case_index,inputs,depth==1);
            if (depth==1) result->applicability=applicability;
            if (applicability==GD_RULE_TRUTH_FALSE) { --depth; continue; }
            frame->conditional=frame->conditional || applicability!=GD_RULE_TRUTH_TRUE;
            if (frame->conditional) result->constraints_complete=0;
            case_writes(analysis,result,frame->case_index,frame->conditional,inputs,path,depth);
            frame->entered=1; frame->participant=item->first_participant;
            frame->call=item->participant_count ? analysis->call_first[frame->participant]:-1;
        }
        if (frame->participant>=item->first_participant+item->participant_count) { --depth; continue; }
        if (frame->call<0) {
            ++frame->participant; frame->next_case=0;
            if (frame->participant<item->first_participant+item->participant_count) frame->call=analysis->call_first[frame->participant];
            continue;
        }
        {
            const GD_RuleCall *call=&analysis->calls[frame->call]; int child,i,cycle=0;
            if (call->resolution!=GD_RULE_RESOLVED || !call->case_count) { result->constraints_complete=0; result->opaque_effects=1; frame->call=call->next; frame->next_case=0; continue; }
            if (frame->next_case==call->case_count) { frame->call=call->next; frame->next_case=0; continue; }
            child=call->first_case+frame->next_case++;
            for (i=0;i<depth;++i) if (stack[i].case_index==child) cycle=1;
            if (cycle) { ++result->cycle_count; result->constraints_complete=0; result->opaque_effects=1; continue; }
            if (depth==GD_MAX_RULE_EXPANSION_DEPTH) { result->limited=1; result->constraints_complete=0; break; }
            for (p=analysis->rules->cases[child].first_participant;p<analysis->rules->cases[child].first_participant+analysis->rules->cases[child].participant_count;++p) result->actor_map[p]=-1;
            for (i=call->first_binding;i<call->first_binding+call->binding_count;++i) {
                if (!evaluation_charge(result,1)) break;
                if (analysis->bindings[i].callee>=analysis->rules->cases[child].first_participant &&
                    analysis->bindings[i].callee<analysis->rules->cases[child].first_participant+analysis->rules->cases[child].participant_count)
                    result->actor_map[analysis->bindings[i].callee]=result->actor_map[analysis->bindings[i].caller];
            }
            stack[depth]=(ExpansionFrame){child,0,frame->conditional,0,-1,0}; ++depth;
        }
    }
    if (result->applicability!=GD_RULE_TRUTH_FALSE) {
        whole_case_preservation(analysis,result,inputs); effect_scope(analysis,result); write_checks(analysis,result);
    }
    if (result->conflict_count || result->uncertain_write_pairs || result->cycle_count) result->constraints_complete=0;
    if (result->limited) result->constraints_complete=result->values_complete=0;
}
typedef struct { int first_actor,second_actor,axis,count; const char *values[GD_MAX_ENUM_VALUES]; } BranchAxis;
static int comparison_charge(GD_RuleAnalysis *analysis, size_t amount) {
    if (analysis->comparison_limited) return 0;
    if (amount>(size_t)(GD_RULE_ANALYSIS_WORK_LIMIT-analysis->comparison_work)) { analysis->comparison_limited=1; return 0; }
    analysis->comparison_work+=(int)amount; return 1;
}
static int actor_alignment(GD_RuleAnalysis *analysis, GD_RuleComparisonResult *comparison, int *map) {
    const GD_Rules *rules=analysis->rules; const GD_RuleCase *a=&rules->cases[comparison->first_case], *b=&rules->cases[comparison->second_case];
    int p,q,k,matched=0;
    for (k=0;k<2;++k) {
        const GD_RuleCase *item=k ? b:a;
        for (p=item->first_participant;p<item->first_participant+item->participant_count;++p) {
            if (rules->participants[p].noun_resolution!=GD_RULE_RESOLVED) return 0;
            for (q=item->first_participant;q<p;++q) {
                if (!comparison_charge(analysis,1)) return 0;
                if (rules->participants[p].noun_record==rules->participants[q].noun_record) return 0;
            }
        }
    }
    for (p=b->first_participant;p<b->first_participant+b->participant_count;++p) {
        map[p]=p;
        for (q=a->first_participant;q<a->first_participant+a->participant_count;++q) {
            if (!comparison_charge(analysis,1)) return 0;
            if (rules->participants[p].noun_record==rules->participants[q].noun_record) { map[p]=q; ++matched; }
        }
    }
    comparison->actor_scope_equal=matched==a->participant_count && matched==b->participant_count;
    return matched>0;
}
static int comparison_context(GD_RuleAnalysis *analysis, GD_RuleComparisonResult *comparison, const int *map) {
    const GD_RuleContext *a=&analysis->rules->cases[comparison->first_case].ordered_context;
    const GD_RuleContext *b=&analysis->rules->cases[comparison->second_case].ordered_context;
    int source_a=analysis->rules->sources[analysis->rules->cases[comparison->first_case].source].record;
    int source_b=analysis->rules->sources[analysis->rules->cases[comparison->second_case].source].record;
    int i,j,common=0;
    if (analysis->verb_first[source_a]>=0 && analysis->verb_first[source_b]>=0) {
        for (i=analysis->verb_first[source_a];i>=0;i=analysis->verb_next[i]) for (j=analysis->verb_first[source_b];j>=0;j=analysis->verb_next[j]) {
            size_t length=bounded_length(analysis->model->edges[i].reference,GD_MAX_RULE_LINE);
            if (!comparison_charge(analysis,length+1)) return 0;
            if (span_equal(analysis->model->edges[i].reference,length,analysis->model->edges[j].reference)) common=1;
        }
        if (!common) { comparison->reason="different_authored_action_links; joint occurrence not established"; return 0; }
    }
    if (a->state!=GD_CONDITION_PARSED || b->state!=GD_CONDITION_PARSED || !a->bindings_complete || !b->bindings_complete) return 0;
    if (a->unrestricted || b->unrestricted) { comparison->context_scope_equal=a->unrestricted && b->unrestricted; return 1; }
    if (!comparison_charge(analysis,a->operator_length+2)) return 0;
    if (a->left!=map[b->left] || a->right!=map[b->right] || a->target.axis_record!=b->target.axis_record || strcmp(a->target.state,b->target.state)) return 0;
    comparison->context_scope_equal=1; return 1;
}
static int branch_axes(GD_RuleAnalysis *analysis, GD_RuleComparisonResult *comparison, const int *map, BranchAxis *variables, int *count) {
    const GD_Rules *rules=analysis->rules; int side,p,edge,i,k;
    *count=0; comparison->vocabularies_closed=1;
    for (side=0;side<2;++side) {
        const GD_RuleCase *item=&rules->cases[side ? comparison->second_case:comparison->first_case];
        for (p=item->first_participant;p<item->first_participant+item->participant_count;++p) {
            int noun=rules->participants[p].noun_record,actor=side ? map[p]:p;
            if (!rules->actor_axes_complete[noun]) comparison->vocabularies_closed=0;
            for (edge=rules->actor_first[noun];edge>=0;edge=rules->next_axis[edge]) {
                int axis=analysis->model->edges[edge].target,found=-1;
                const GD_ValueDefinition *definition;
                const char *value;
                if (!comparison_charge(analysis,1)) return 0;
                if (rules->axis_resolution[edge]!=GD_RULE_RESOLVED) { comparison->vocabularies_closed=0; continue; }
                if (!rules->axes[axis].valid || rules->axes[axis].type==GD_AXIS_UNSPECIFIED) { comparison->vocabularies_closed=0; continue; }
                if (rules->axes[axis].type!=GD_AXIS_ENUM) { comparison->numeric_unexamined=1; continue; }
                definition=&analysis->model->records[axis].axis;
                if (!analysis->model->records[axis].axis_closed) comparison->vocabularies_closed=0;
                for (i=0;i<*count;++i) if (variables[i].axis==axis && (variables[i].first_actor==actor || (variables[i].first_actor<0 && variables[i].second_actor==p))) found=i;
                if (found<0) {
                    if (*count==GD_MAX_RULE_WITNESS_FACTS || !definition->allowed_count) return 0;
                    found=(*count)++; memset(&variables[found],0,sizeof(variables[found]));
                    variables[found].first_actor=variables[found].second_actor=-1; variables[found].axis=axis;
                    value=definition->allowed;
                    for (k=0;k<definition->allowed_count;++k) {
                        size_t length=bounded_length(value,GD_MAX_FRONTMATTER);
                        if (!comparison_charge(analysis,length+1)) return 0;
                        /* Repeated declarations are one enum value, never extra states. */
                        for (i=0;i<variables[found].count;++i) {
                            if (!comparison_charge(analysis,length+1)) return 0;
                            if (!strcmp(variables[found].values[i],value)) break;
                        }
                        if (i==variables[found].count) variables[found].values[variables[found].count++]=value;
                        value+=length+1;
                    }
                }
                if (side) variables[found].second_actor=p; else variables[found].first_actor=p;
            }
        }
    }
    return 1;
}
static void save_witness(GD_RuleComparisonResult *comparison, const BranchAxis *variables, int count, int branch) {
    int i;
    comparison->witness_count=0;
    for (i=0;i<count;++i) {
        int selected=branch%variables[i].count; branch/=variables[i].count;
        comparison->witness[comparison->witness_count++]=(GD_RuleWitnessFact){variables[i].first_actor>=0 ? variables[i].first_actor:variables[i].second_actor,variables[i].axis,variables[i].values[selected]};
    }
}
static int branch_facts(const BranchAxis *variables, int count, int branch, GD_RulePredicateFact *a, int *count_a, GD_RulePredicateFact *b, int *count_b) {
    int i,j; *count_a=*count_b=0;
    for (i=0;i<count;++i) {
        int selected=branch%variables[i].count; branch/=variables[i].count;
        for (j=0;j<variables[i].count;++j) {
            if ((variables[i].first_actor>=0 && *count_a==GD_MAX_RULE_CONDITION_ITEMS) || (variables[i].second_actor>=0 && *count_b==GD_MAX_RULE_CONDITION_ITEMS)) return 0;
            if (variables[i].first_actor>=0) a[(*count_a)++]=(GD_RulePredicateFact){variables[i].first_actor,variables[i].axis,variables[i].values[j],j==selected ? GD_RULE_TRUTH_TRUE:GD_RULE_TRUTH_FALSE};
            if (variables[i].second_actor>=0) b[(*count_b)++]=(GD_RulePredicateFact){variables[i].second_actor,variables[i].axis,variables[i].values[j],j==selected ? GD_RULE_TRUTH_TRUE:GD_RULE_TRUTH_FALSE};
        }
    }
    return 1;
}
static int numeric_scope(const GD_Rules *rules, int case_index) {
    const GD_RuleCase *item=&rules->cases[case_index]; int p,k,i;
    for (p=item->first_participant;p<item->first_participant+item->participant_count;++p) for (k=0;k<2;++k) {
        const GD_RuleCondition *condition=k ? &rules->participants[p].skip_condition:&rules->participants[p].required_condition;
        for (i=condition->first_token;i<condition->first_token+condition->token_count;++i) if (rules->condition_tokens[i].operation==GD_RULE_OP_COMPARE) return 1;
    }
    return 0;
}
static void compare_pair(GD_RuleAnalysis *analysis, GD_RuleComparisonResult *comparison) {
    const GD_Rules *rules=analysis->rules;
    BranchAxis variables[GD_MAX_RULE_WITNESS_FACTS]; int variable_count=0,map[GD_MAX_RULE_PARTICIPANTS];
    GD_RulePredicateFact first_facts[GD_MAX_RULE_CONDITION_ITEMS],second_facts[GD_MAX_RULE_CONDITION_ITEMS];
    GD_RuleContextFact first_context,second_context;
    GD_RuleAssumptions first_inputs={first_facts,0,NULL,0},second_inputs={second_facts,0,NULL,0};
    const GD_RuleContext *context_a=&rules->cases[comparison->first_case].ordered_context,*context_b=&rules->cases[comparison->second_case].ordered_context;
    int total=1,overflow=0,branch,i,j,same_gates=1,same_writes=1,conflict=0,unknown=0;
    comparison->reason="participant_binding_unknown_or_multiple_same_type_locals"; comparison->unexamined_branches=-1;
    if (!actor_alignment(analysis,comparison,map)) return;
    comparison->reason="ordered_context_overlap_not_established; no geometry or symmetry inference";
    if (!comparison_context(analysis,comparison,map)) return;
    comparison->reason="state_vocabulary_empty_or_dimension_limit";
    if (!branch_axes(analysis,comparison,map,variables,&variable_count)) return;
    for (i=0;i<variable_count;++i) {
        if (total>2147483647/variables[i].count) overflow=1;
        else if (!overflow) total*=variables[i].count;
    }
    if (!context_a->unrestricted) { first_context=(GD_RuleContextFact){context_a->left,context_a->right,context_a->target.axis_record,context_a->target.state,GD_RULE_TRUTH_TRUE}; first_inputs.contexts=&first_context; first_inputs.context_count=1; }
    if (!context_b->unrestricted) { second_context=(GD_RuleContextFact){context_b->left,context_b->right,context_b->target.axis_record,context_b->target.state,GD_RULE_TRUTH_TRUE}; second_inputs.contexts=&second_context; second_inputs.context_count=1; }
    comparison->outcomes_complete=1;
    comparison->numeric_unexamined=comparison->numeric_unexamined || numeric_scope(rules,comparison->first_case) || numeric_scope(rules,comparison->second_case);
    comparison->reason="checked_declared_enum_product; hypothetical instances, not reachable scene evidence";
    for (branch=0;branch<GD_MAX_RULE_BRANCHES && (overflow || branch<total);++branch) {
        GD_RuleEvaluation *a=&analysis->first,*b=&analysis->second;
        int branch_unknown=0,branch_same=1,branch_conflict=0;
        if (GD_RULE_ANALYSIS_WORK_LIMIT-analysis->comparison_work<2*GD_RULE_EVALUATION_WORK_LIMIT) { analysis->comparison_limited=1; break; }
        if (!branch_facts(variables,variable_count,branch,first_facts,&first_inputs.fact_count,second_facts,&second_inputs.fact_count)) { comparison->reason="scenario_fact_limit"; break; }
        gd_rule_evaluate(analysis,comparison->first_case,first_inputs,a); gd_rule_evaluate(analysis,comparison->second_case,second_inputs,b);
        if (a->limited || b->limited || a->input_valid!=1 || b->input_valid!=1) { comparison->reason="scenario_evaluation_limit_or_invalid_assumptions"; comparison_charge(analysis,(size_t)a->work+b->work); break; }
        ++comparison->checked_branches;
        if (a->applicability==GD_RULE_TRUTH_UNKNOWN || b->applicability==GD_RULE_TRUTH_UNKNOWN) { ++comparison->undetermined_branches; unknown=1; }
        if (a->applicability!=b->applicability) same_gates=0;
        if (a->applicability!=GD_RULE_TRUTH_TRUE || b->applicability!=GD_RULE_TRUTH_TRUE) { comparison_charge(analysis,(size_t)a->work+b->work); continue; }
        ++comparison->overlap_branches;
        if (a->opaque_effects || b->opaque_effects || a->conflict_count || b->conflict_count || a->uncertain_write_pairs || b->uncertain_write_pairs) branch_unknown=1;
        for (i=0;i<a->write_count && !a->limited;++i) {
            const GD_RuleWrite *left=&a->writes[i]; int found=0;
            if (left->kind==GD_WRITE_QUANTITY && !left->value.known) comparison->numeric_unexamined=1;
            for (j=0;j<b->write_count;++j) {
                const GD_RuleWrite *right=&b->writes[j]; GD_RuleTruth compatible;
                if (!evaluation_charge(a,1)) break;
                if (left->actor<0 || left->axis<0 || right->actor<0 || left->actor!=map[right->actor] || left->axis!=right->axis) continue;
                compatible=compatible_writes(analysis,a,left,right);
                if (a->limited) break;
                if (compatible==GD_RULE_TRUTH_FALSE) branch_conflict=1;
                if (compatible==GD_RULE_TRUTH_FALSE && !conflict) {
                    conflict=1; comparison->conflict_first=i; comparison->conflict_second=j; comparison->conflict_a=*left; comparison->conflict_b=*right; save_witness(comparison,variables,variable_count,branch);
                }
                if (compatible==GD_RULE_TRUTH_UNKNOWN) branch_unknown=1;
                if (compatible==GD_RULE_TRUTH_TRUE) found=1;
                if (left->origin_case==right->origin_case && left->line==right->line) comparison->shared_derivation=1;
            }
            if (!found) branch_same=0;
        }
        for (j=0;j<b->write_count && !a->limited;++j) {
            const GD_RuleWrite *right=&b->writes[j]; int found=0;
            if (right->kind==GD_WRITE_QUANTITY && !right->value.known) comparison->numeric_unexamined=1;
            for (i=0;i<a->write_count;++i) {
                const GD_RuleWrite *left=&a->writes[i];
                if (!evaluation_charge(a,1)) break;
                if (left->actor>=0 && right->actor>=0 && left->actor==map[right->actor] && left->axis==right->axis && compatible_writes(analysis,a,left,right)==GD_RULE_TRUTH_TRUE) found=1;
            }
            if (!found) branch_same=0;
        }
        comparison_charge(analysis,(size_t)a->work+b->work);
        if (a->limited) { --comparison->checked_branches; --comparison->overlap_branches; comparison->reason="write_comparison_work_limit"; break; }
        if (branch_unknown) { ++comparison->undetermined_branches; unknown=1; }
        else if (!branch_conflict) { ++comparison->compatible_branches; if (branch_same) ++comparison->matching_branches; }
        if (!branch_same) same_writes=0;
        if (!a->constraints_complete || !b->constraints_complete) comparison->outcomes_complete=0;
    }
    comparison->unexamined_branches=overflow ? -1:total-comparison->checked_branches;
    if (conflict) comparison->relation=GD_RULE_RELATION_CONFLICT;
    else if (unknown || comparison->unexamined_branches || analysis->comparison_limited || !comparison->vocabularies_closed) comparison->relation=GD_RULE_RELATION_UNDETERMINED;
    else if (!comparison->overlap_branches) comparison->relation=GD_RULE_RELATION_DISJOINT;
    else if (same_gates && same_writes && comparison->actor_scope_equal && comparison->context_scope_equal)
        comparison->relation=comparison->numeric_unexamined || !comparison->outcomes_complete ? GD_RULE_RELATION_REDUNDANT:GD_RULE_RELATION_EQUIVALENT;
    else comparison->relation=GD_RULE_RELATION_COMPATIBLE;
}
void gd_rule_compare(GD_RuleAnalysis *analysis) {
    int a,b;
    if (!analysis) return;
    analysis->compared=1; analysis->comparison_count=analysis->pair_count=analysis->comparison_work=analysis->comparison_limited=0;
    analysis->pairs_unexamined=analysis->rules->case_count*(analysis->rules->case_count-1)/2;
    for (a=0;a<analysis->rules->case_count && !analysis->comparison_limited;++a) for (b=a+1;b<analysis->rules->case_count && !analysis->comparison_limited;++b) {
        GD_RuleComparisonResult *comparison;
        if (analysis->comparison_count==GD_MAX_RULE_COMPARISONS) { analysis->comparison_limited=1; break; }
        comparison=&analysis->comparisons[analysis->comparison_count++]; memset(comparison,0,sizeof(*comparison));
        comparison->first_case=a; comparison->second_case=b; comparison->conflict_first=comparison->conflict_second=-1;
        compare_pair(analysis,comparison); ++analysis->pair_count; --analysis->pairs_unexamined;
    }
    analysis->first.assumptions=(GD_RuleAssumptions){0}; analysis->second.assumptions=(GD_RuleAssumptions){0};
}
static int scenario_error(GD_Model *model, const char *message) { gd_error(model,"rules.scenario",message,NULL,0); return 0; }
static int scenario_charge(GD_Model *model, GD_RuleScenario *scenario, size_t amount) {
    if (scenario->limited) return 0;
    if (amount>(size_t)(GD_RULE_EVALUATION_WORK_LIMIT-scenario->work)) { scenario->limited=1; return scenario_error(model,"Scenario input work limit exceeded; no partial assumption adopted"); }
    scenario->work+=(int)amount; return 1;
}
static int scenario_axis(GD_Model *model, GD_RuleScenario *scenario, const char *reference, size_t length) {
    int i,found=-1;
    if (!length || length>257 || !memchr(reference,'/',length)) return -1;
    for (i=0;i<model->record_count;++i) {
        if (!scenario_charge(model,scenario,length+1)) return -1;
        if (model->records[i].role==GD_AXIS && span_equal(reference,length,model->records[i].reference)) { if (found>=0) return -1; found=i; }
    }
    return found;
}
static int scenario_fact(GD_Model *model, GD_RuleScenario *scenario, GD_RulePredicateFact fact) {
    int i;
    size_t length=bounded_length(fact.value,GD_MAX_RULE_LINE);
    for (i=0;i<scenario->fact_count;++i) {
        if (!scenario_charge(model,scenario,length+1)) return 0;
        if (fact.participant==scenario->facts[i].participant && fact.axis_record==scenario->facts[i].axis_record && fact.truth==scenario->facts[i].truth && !strcmp(fact.value,scenario->facts[i].value)) return 1;
    }
    if (scenario->fact_count==GD_MAX_RULE_CONDITION_ITEMS) return scenario_error(model,"Scenario exceeds 256 facts after explicit enum-current-value expansion");
    scenario->facts[scenario->fact_count++]=fact; return 1;
}
int gd_rule_assume(GD_Model *model, const GD_Rules *rules, int case_index, const char *text, GD_RuleScenario *scenario) {
    const char *colon,*equals,*value; int actor,axis,edge,member=0,before;
    size_t length;
    if (!model || !rules || !scenario || !text) return 0;
    before=scenario->fact_count; length=bounded_length(text,GD_MAX_RULE_LINE);
    if (case_index<0 || case_index>=rules->case_count || length>GD_MAX_RULE_LINE || !(colon=strchr(text,':')) || !(equals=strchr(colon+1,'=')) || !equals[1])
        return scenario_error(model,"--assume needs LOCAL:ALIAS/AXIS=VALUE in selected case; numeric VALUE uses declared axis unit");
    if (!scenario_charge(model,scenario,(size_t)rules->cases[case_index].participant_count*((size_t)(colon-text)+1))) return 0;
    actor=participant_named(rules,case_index,text,(size_t)(colon-text)); axis=scenario_axis(model,scenario,colon+1,(size_t)(equals-colon-1)); value=equals+1;
    if (actor<0 || axis<0 || rules->participants[actor].noun_resolution!=GD_RULE_RESOLVED || !rules->axes[axis].valid)
        return scenario_error(model,"Assumption requires one declared local participant and unique loaded valid canonical axis");
    for (edge=rules->actor_first[rules->participants[actor].noun_record];edge>=0;edge=rules->next_axis[edge]) {
        if (!scenario_charge(model,scenario,1)) return 0;
        if (rules->axis_resolution[edge]==GD_RULE_RESOLVED && model->edges[edge].target==axis) member=1;
    }
    if (!member) return scenario_error(model,"Assumption axis must be explicitly applicable to participant noun; loading a definition alone does not establish membership");
    if (rules->axes[axis].type==GD_AXIS_ENUM) {
        const GD_ValueDefinition *definition=&model->records[axis].axis; const char *state=definition->allowed; int i,known=0;
        for (i=0;i<definition->allowed_count;++i) {
            size_t size=bounded_length(state,GD_MAX_FRONTMATTER);
            if (!scenario_charge(model,scenario,size+length+2)) goto rollback;
            if (!strcmp(state,value)) known=1;
            if (size<=GD_MAX_RULE_LINE && !scenario_fact(model,scenario,(GD_RulePredicateFact){actor,axis,state,!strcmp(state,value) ? GD_RULE_TRUTH_TRUE:GD_RULE_TRUTH_FALSE})) goto rollback;
            state+=size+1;
        }
        if (!known && model->records[axis].axis_closed) { scenario_error(model,"Value is absent from closed enum vocabulary"); goto rollback; }
        if (!known && !scenario_fact(model,scenario,(GD_RulePredicateFact){actor,axis,value,GD_RULE_TRUTH_TRUE})) goto rollback;
    } else if (rules->axes[axis].type==GD_AXIS_INTEGER || rules->axes[axis].type==GD_AXIS_NUMBER) {
        double number;
        if (!gd_control_number(value,&number,rules->axes[axis].type==GD_AXIS_INTEGER)) return scenario_error(model,"Numeric assumption must be finite declared-type literal; no units/coercion/rounding in fact text");
        if (!scenario_fact(model,scenario,(GD_RulePredicateFact){actor,axis,value,GD_RULE_TRUTH_TRUE})) goto rollback;
    } else return scenario_error(model,"Assumption needs enum/integer/number axis type; unspecified type stays unknown");
    return 1;
rollback:
    scenario->fact_count=before; return 0;
}
int gd_rule_assume_context(GD_Model *model, const GD_Rules *rules, int case_index, const char *text, GD_RuleTruth truth, GD_RuleScenario *scenario) {
    const char *cursor=text,*parts[3],*dot,*value=NULL; size_t lengths[3],length; int count=0,left,right,axis=-1,i;
    if (!model || !rules || !scenario || !text) return 0;
    length=bounded_length(text,GD_MAX_RULE_LINE);
    if (case_index<0 || case_index>=rules->case_count || length>GD_MAX_RULE_LINE) return scenario_error(model,"Context assumption needs one bounded selected-case triple");
    while (*cursor) {
        if (space((unsigned char)*cursor)) { ++cursor; continue; }
        if (count==3) return scenario_error(model,"--context/--context-not needs LOCAL ALIAS/AXIS.STATE LOCAL");
        parts[count]=cursor; while (*cursor && !space((unsigned char)*cursor)) ++cursor; lengths[count]=(size_t)(cursor-parts[count]); ++count;
    }
    if (count!=3 || !scenario_charge(model,scenario,(size_t)rules->cases[case_index].participant_count*(length+1))) return scenario_error(model,"--context/--context-not needs LOCAL ALIAS/AXIS.STATE LOCAL");
    left=participant_named(rules,case_index,parts[0],lengths[0]); right=participant_named(rules,case_index,parts[2],lengths[2]);
    dot=memchr(parts[1],'.',lengths[1]);
    if (dot) {
        const GD_ValueDefinition *definition;
        axis=scenario_axis(model,scenario,parts[1],(size_t)(dot-parts[1]));
        if (axis>=0 && rules->axes[axis].type==GD_AXIS_ENUM && rules->axes[axis].valid) {
            const char *state; definition=&model->records[axis].axis; state=definition->allowed;
            for (i=0;i<definition->allowed_count;++i) {
                size_t size=bounded_length(state,GD_MAX_FRONTMATTER);
                if (!scenario_charge(model,scenario,size+lengths[1]+1)) return 0;
                if (span_equal(dot+1,lengths[1]-(size_t)(dot-parts[1])-1,state)) value=state;
                state+=size+1;
            }
        }
    } else {
        const GD_RuleContext *context=&rules->cases[case_index].ordered_context;
        if (context->bindings_complete && context->target.state && span_equal(parts[1],lengths[1],context->target.state)) { axis=context->target.axis_record; value=context->target.state; }
    }
    if (left<0 || right<0 || axis<0 || !value) return scenario_error(model,"Context needs exact local endpoints and canonical declared enum operator, or selected case's known bare operator; no global guesses");
    if (scenario->context_count==GD_MAX_RULE_CONTEXT_FACTS) return scenario_error(model,"Scenario exceeds 64 context facts");
    scenario->contexts[scenario->context_count++]=(GD_RuleContextFact){left,right,axis,value,truth}; return 1;
}
static void json_member(FILE *output, const char *key, const char *value) {
    fputc(',',output); gd_json_string(output,key); fputc(':',output); gd_json_string(output,value);
}
static void case_json(FILE *output, const GD_RuleAnalysis *analysis, int index) {
    const GD_RuleCase *item=&analysis->rules->cases[index];
    const GD_Record *record=&analysis->model->records[analysis->rules->sources[item->source].record];
    fputs("{\"record\":",output); gd_json_string(output,record->reference);
    json_member(output,"record_id",record->id); json_member(output,"case",item->name);
    json_member(output,"status",record->status); json_member(output,"coverage",record->coverage); json_member(output,"source_path",record->source_path);
    fprintf(output,",\"line\":%d}",item->line);
}
static void actor_json(FILE *output, const GD_RuleAnalysis *analysis, int index) {
    if (index<0) { fputs("null",output); return; }
    fputs("{\"participant\":",output); gd_json_string(output,analysis->rules->participants[index].name);
    json_member(output,"noun",analysis->rules->participants[index].noun); fprintf(output,",\"snapshot_index\":%d}",index);
}
static void write_json(FILE *output, const GD_RuleAnalysis *analysis, const GD_RuleWrite *write) {
    int i;
    fputs("{\"actor\":",output); actor_json(output,analysis,write->actor);
    json_member(output,"axis",write->axis>=0 ? analysis->model->records[write->axis].reference:NULL);
    json_member(output,"kind",write->kind==GD_WRITE_STATE ? "state":write->kind==GD_WRITE_QUANTITY ? "quantity":"preserve");
    json_member(output,"state",write->state);
    fputs(",\"number\":",output); if (write->value.known && isfinite(write->value.number)) fprintf(output,"%.17g",write->value.number); else fputs("null",output);
    json_member(output,"numeric_state",write->value.state==GD_EXPRESSION_VALUE ? "resolved":write->value.state==GD_EXPRESSION_UNIT_ERROR ? "unit_error":
        write->value.state==GD_EXPRESSION_ARITHMETIC_ERROR ? "arithmetic_error":"unknown");
    json_member(output,"bounds",write->value.bounds==GD_BOUNDS_WITHIN ? "within_declared":write->value.bounds==GD_BOUNDS_OUTSIDE ? "outside_declared":
        write->value.bounds==GD_BOUNDS_UNDECLARED ? "not_declared":"unknown");
    fputs(",\"expression\":",output);
    if (write->quantity>=0) gd_json_string_n(output,analysis->rules->quantities[write->quantity].text,analysis->rules->quantities[write->quantity].length); else fputs("null",output);
    fprintf(output,",\"conditional\":%s,\"line\":%d,\"origin\":",write->conditional ? "true":"false",write->line); case_json(output,analysis,write->origin_case);
    fputs(",\"via\":[",output);
    for (i=0;i<write->path_count;++i) { if (i) fputc(',',output); case_json(output,analysis,write->path[i]); }
    fputs("]}",output);
}
static void evaluation_json(FILE *output, const GD_RuleAnalysis *analysis, const GD_RuleEvaluation *result) {
    int i;
    fputs("{\"case\":",output); case_json(output,analysis,result->root_case);
    json_member(output,"input_state",result->input_valid==1 ? "typed_assumptions":result->input_valid==0 ? "invalid":"unexamined");
    fputs(",\"assumptions\":[",output);
    for (i=0;i<result->assumptions.fact_count;++i) {
        const GD_RulePredicateFact *fact=&result->assumptions.facts[i]; if (i) fputc(',',output);
        fputs("{\"actor\":",output); actor_json(output,analysis,fact->participant); json_member(output,"axis",analysis->model->records[fact->axis_record].reference);
        json_member(output,"value",fact->value); json_member(output,"truth",gd_rule_truth_name(fact->truth)); fputc('}',output);
    }
    fputs("],\"context_assumptions\":[",output);
    for (i=0;i<result->assumptions.context_count;++i) {
        const GD_RuleContextFact *fact=&result->assumptions.contexts[i]; if (i) fputc(',',output);
        fputs("{\"left\":",output); actor_json(output,analysis,fact->left); fputs(",\"right\":",output); actor_json(output,analysis,fact->right);
        json_member(output,"axis",analysis->model->records[fact->axis].reference); json_member(output,"value",fact->value); json_member(output,"truth",gd_rule_truth_name(fact->truth)); fputc('}',output);
    }
    fputc(']',output);
    json_member(output,"applicability",gd_rule_truth_name(result->applicability)); json_member(output,"context",gd_rule_truth_name(result->context));
    json_member(output,"requirements",gd_rule_truth_name(result->requirements));
    json_member(output,"exclusion",result->exclusion.state==GD_EXCLUSION_ABSENT ? "absent":result->exclusion.state==GD_EXCLUSION_EXCLUDED ? "excluded":
        result->exclusion.state==GD_EXCLUSION_NOT_EXCLUDED ? "not_excluded":"unknown");
    fprintf(output,",\"guards\":%d,\"checked_guards\":%d,\"excluded_by\":",result->exclusion.guard_count,result->exclusion.checked_count);
    actor_json(output,analysis,result->exclusion.excluded_by);
    fprintf(output,",\"constraints_complete\":%s,\"values_complete\":%s,\"expansions\":%d,\"cycles\":%d,\"work\":%d,\"limited\":%s,\"conflicts\":%d,\"uncertain_overlap_pairs\":%d,\"checked_overlap_pairs\":%d,\"writes\":[",
        result->constraints_complete ? "true":"false",result->values_complete ? "true":"false",result->expansions,result->cycle_count,result->work,result->limited ? "true":"false",
        result->conflict_count,result->uncertain_write_pairs,result->checked_write_pairs);
    for (i=0;i<result->write_count;++i) { if (i) fputc(',',output); write_json(output,analysis,&result->writes[i]); }
    fputs("],\"write_checks\":[",output);
    for (i=0;i<result->checks_count;++i) {
        const GD_RuleWriteCheck *check=&result->checks[i]; if (i) fputc(',',output);
        fprintf(output,"{\"first_write\":%d,\"second_write\":%d,\"conflict\":",check->first,check->second); gd_json_string(output,gd_rule_truth_name(check->conflict)); fputc('}',output);
    }
    fprintf(output,"],\"write_checks_omitted\":%d}",result->checks_omitted);
}
void gd_write_rule_analysis(FILE *output, const GD_RuleAnalysis *analysis) {
    int i,j;
    fprintf(output,"{\"invocation_work\":%d,\"invocation_limited\":%s,\"calls\":[",analysis->compile_work,analysis->compile_limited ? "true":"false");
    for (i=0;i<analysis->call_count;++i) {
        const GD_RuleCall *call=&analysis->calls[i]; if (i) fputc(',',output);
        fputs("{\"caller\":",output); case_json(output,analysis,call->caller_case); fputs(",\"participant\":",output); actor_json(output,analysis,call->caller);
        fputs(",\"text\":",output); gd_json_string_n(output,call->text,call->length); json_member(output,"resolution",gd_rule_resolution_name(call->resolution));
        fprintf(output,",\"line\":%d,\"selected_cases\":%d,\"bindings\":[",call->line,call->case_count);
        for (j=0;j<call->binding_count;++j) {
            const GD_RuleBinding *binding=&analysis->bindings[call->first_binding+j]; if (j) fputc(',',output);
            fputs("{\"callee\":",output); actor_json(output,analysis,binding->callee); fputs(",\"caller\":",output); actor_json(output,analysis,binding->caller); fputc('}',output);
        }
        fputs("]}",output);
    }
    fputs("],\"scenario\":",output); if (analysis->requested_case>=0) evaluation_json(output,analysis,&analysis->scenario); else fputs("null",output);
    fprintf(output,",\"compared\":%s,\"compared_pairs\":%d,\"pairs_unexamined\":%d,\"comparison_work\":%d,\"comparison_limited\":%s,\"comparisons\":[",
        analysis->compared ? "true":"false",analysis->pair_count,analysis->pairs_unexamined,analysis->comparison_work,analysis->comparison_limited ? "true":"false");
    for (i=0;i<analysis->comparison_count;++i) {
        const GD_RuleComparisonResult *comparison=&analysis->comparisons[i]; if (i) fputc(',',output);
        fputs("{\"first\":",output); case_json(output,analysis,comparison->first_case); fputs(",\"second\":",output); case_json(output,analysis,comparison->second_case);
        json_member(output,"relation",gd_rule_relation_name(comparison->relation)); json_member(output,"scope",comparison->reason);
        fprintf(output,",\"checked_branches\":%d,\"overlap_branches\":%d,\"matching_branches\":%d,\"compatible_branches\":%d,\"undetermined_branches\":%d,\"unexamined_known_branches\":",
            comparison->checked_branches,comparison->overlap_branches,comparison->matching_branches,comparison->compatible_branches,comparison->undetermined_branches);
        if (comparison->unexamined_branches<0) fputs("null",output); else fprintf(output,"%d",comparison->unexamined_branches);
        fprintf(output,",\"closed_vocabularies\":%s,\"complete_outcome_scope\":%s,\"numeric_domains_unexamined\":%s,\"shared_derivation\":%s,\"independent_corroboration\":false,\"same_actor_scope\":%s,\"same_context_scope\":%s,\"witness\":[",
            comparison->vocabularies_closed ? "true":"false",comparison->outcomes_complete ? "true":"false",comparison->numeric_unexamined ? "true":"false",comparison->shared_derivation ? "true":"false",
            comparison->actor_scope_equal ? "true":"false",comparison->context_scope_equal ? "true":"false");
        for (j=0;j<comparison->witness_count;++j) {
            const GD_RuleWitnessFact *fact=&comparison->witness[j]; if (j) fputc(',',output);
            fputs("{\"actor\":",output); actor_json(output,analysis,fact->actor); json_member(output,"axis",analysis->model->records[fact->axis].reference); json_member(output,"value",fact->value); fputc('}',output);
        }
        fputs("],\"conflicting_writes\":",output);
        if (comparison->relation==GD_RULE_RELATION_CONFLICT) { fputc('[',output); write_json(output,analysis,&comparison->conflict_a); fputc(',',output); write_json(output,analysis,&comparison->conflict_b); fputc(']',output); }
        else fputs("null",output);
        fputc('}',output);
    }
    fprintf(output,"],\"limits\":{\"calls\":%d,\"bindings\":%d,\"bindings_per_call\":%d,\"depth\":%d,\"expansions\":%d,\"writes\":%d,\"context_facts\":%d,\"pairs\":%d,\"branches_per_pair\":%d,\"dimensions\":%d,\"evaluation_work\":%d,\"analysis_work\":%d},\"scope\":",
        GD_MAX_RULE_CALLS,GD_MAX_RULE_BINDINGS,GD_MAX_RULE_CALL_BINDINGS,GD_MAX_RULE_EXPANSION_DEPTH,GD_MAX_RULE_EXPANSIONS,GD_MAX_RULE_WRITES,GD_MAX_RULE_CONTEXT_FACTS,GD_MAX_RULE_COMPARISONS,
        GD_MAX_RULE_BRANCHES,GD_MAX_RULE_WITNESS_FACTS,GD_RULE_EVALUATION_WORK_LIMIT,GD_RULE_ANALYSIS_WORK_LIMIT);
    gd_json_string(output,"Read-only conditional proposals and bounded logical comparison, all supplied source statuses retained. One simultaneous transition reads immutable starting facts; no implicit source winner, actor inheritance, repair, sequential mutation or actual gameplay execution. Context requires supplied relation truth; no geometry/symmetry inferred. Comparison aligns uniquely typed participant variables hypothetically, never invocation bindings or observed instances. Different ordered predicates/action links remain undetermined. Enumerated known enum values are hypothetical branches, not initial values or reachable scene evidence. Open states, missing information, numeric domains and limits prevent universal equivalence. Reference-derived agreement is not independent corroboration. Unknown is not false; unspecified is not unchanged. No frequency, learning, feeling or quality claim.");
    fputc('}',output);
}
