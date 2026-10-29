/* Native domain checks. Build/run through test.ps1. Fixtures stay in ignored build/. */
#include "gd.h"
#include "gd_rule_analysis.h"
#include "gd_resource_graph.h"
#include "gd_skill_unlocks.h"
#include "gd_layout.h"
#include "gd_graph_analysis.h"
#include "gd_diagnostics.h"
#include <limits.h>
#include <math.h>
#include <time.h>
#define TEST_ENABLE 1
#include "vendor/proto-headers/test.h"

static unsigned char backing[64 * 1024 * 1024];
static free_list *storage;
static char root[GD_MAX_PATH];
static char folders[3][GD_MAX_PATH];
static GD_Collection collections[3];

static void *allocate(void *context, size_t size) {
    arena_result result = free_list_alloc(context, size);
    return result.error == arena_error_ok ? result.ptr : NULL;
}
static void release(void *context, void *memory) { free_list_free(context, memory); }
static GD_Model *model_create(void) {
    GD_Allocator allocator = { storage, allocate, release };
    return gd_create(allocator);
}
static int setup(const char *name) {
    const char *aliases[] = {"things", "actions", "cases"};
    char directory[GD_MAX_PATH];
    int i;
    if (snprintf(directory, sizeof(directory), "%s/%s", root, name) >= (int)sizeof(directory) ||
        !file_directory_create(directory, 1)) return 0;
    for (i = 0; i < 3; ++i) {
        if (snprintf(folders[i], sizeof(folders[i]), "%s/%s", directory, aliases[i]) >= (int)sizeof(folders[i]) ||
            !file_directory_create(folders[i], 1)) return 0;
        collections[i].alias = aliases[i]; collections[i].directory = folders[i]; collections[i].role = (GD_Role)i;
    }
    return 1;
}
static int write_record(int collection, const char *name, const char *text) {
    char path[GD_MAX_PATH];
    if (snprintf(path, sizeof(path), "%s/%s.md", folders[collection], name) >= (int)sizeof(path)) return 0;
    return file_write_bytes(cstring_from_raw(path), text, strlen(text)) == file_error_ok;
}
static int find_record(GD_Model *model, const char *reference) {
    int i;
    for (i = 0; i < model->record_count; ++i) if (!strcmp(model->records[i].reference, reference)) return i;
    return -1;
}
static int has_error(GD_Model *model, const char *code) {
    int i;
    for (i = 0; i < model->error_count; ++i) if (!strcmp(model->errors[i].code, code)) return 1;
    return 0;
}
TEST(rule_outcomes,
    GD_Model *model=model_create(); GD_Rules *rules; GD_RuleAnalysis *analysis; GD_ResourceGraph *graph;
    GD_RuleEvaluation *result; GD_RulePredicateFact facts[4]; GD_RuleContextFact context;
    GD_RuleAssumptions inputs={facts,3,NULL,0}; int root_case,life,form,amount,hero,budget,i,found,all_count,other_count;
    TEST_ASSERT(model && setup("rule-outcomes")); collections[1].role=GD_AXIS; collections[2].role=GD_RULE;
    TEST_ASSERT(write_record(1,"Life","---\nvalue_type: enum\nallowed_values: [alive, dead]\nclosed: true\n---\nLife."));
    TEST_ASSERT(write_record(1,"Form","---\nvalue_type: enum\nallowed_values: [small, tall]\nclosed: true\n---\nForm."));
    TEST_ASSERT(write_record(1,"Amount","---\nvalue_type: integer\nunit: life\nminimum: 0\nmaximum: 9\n---\nBudget."));
    TEST_ASSERT(write_record(1,"Contact","---\nvalue_type: enum\nallowed_values: [sideContacts]\nclosed: true\n---\nOperator vocabulary."));
    TEST_ASSERT(write_record(0,"hero","---\naxes: [actions/Life, actions/Form]\n---\nActor."));
    TEST_ASSERT(write_record(0,"lives","---\naxes: [actions/Amount]\n---\nActor."));
    TEST_ASSERT(write_record(0,"walker","---\naxes: [actions/Life]\n---\nActor."));
    TEST_ASSERT(write_record(2,"helper","```gd-rule\ncase small:\n    context: ANY\n    Victim:\n        noun: things/hero\n        requires: alive AND small\n        other_axes: ANY\n        becomes: dead\n        preserve: OTHER\n    Reserve:\n        noun: things/lives\n        requires: ANY\n        set:\n            Amount: this - 1[life]\n        preserve: OTHER\ncase named:\n    context: ANY\n    Victim:\n        noun: things/hero\n        requires: alive AND small\n        other_axes: ANY\n        becomes: dead\n        preserve: Form, OTHER\n    Reserve:\n        noun: things/lives\n        requires: ANY\n        set:\n            Amount: this - 1[life]\ncase loop:\n    context: ANY\n    Again:\n        noun: things/hero\n        requires: ANY\n        causes: cases/helper#loop\n        preserve: ALL\n```"));
    TEST_ASSERT(write_record(2,"root","---\ncontext_axes: [actions/Contact]\n---\n```gd-rule\ncase direct:\n    context: ANY\n    Hero:\n        noun: things/hero\n        requires: alive AND small\n        other_axes: ANY\n        becomes: dead, tall\n        preserve: OTHER\n    Budget:\n        noun: things/lives\n        requires: ANY\n        set:\n            Amount: this - 1[life]\n        preserve: OTHER\ncase composed:\n    context: ANY\n    Hero:\n        noun: things/hero\n        requires: alive AND small\n        other_axes: ANY\n        becomes: tall\n        causes: cases/helper#small with Victim=Hero, Reserve=Budget\n        preserve: OTHER\n    Budget:\n        noun: things/lives\n        requires: ANY\n        preserve: OTHER\ncase named-conflict:\n    context: ANY\n    Hero:\n        noun: things/hero\n        requires: alive AND small\n        other_axes: ANY\n        becomes: tall\n        causes: cases/helper#named with Victim=Hero, Reserve=Budget\n        preserve: OTHER\n    Budget:\n        noun: things/lives\n        requires: ANY\ncase guard:\n    Hero:\n        noun: things/hero\n        skip_if: ANY\ncase context:\n    context: Hero sideContacts Target\n    Hero:\n        noun: things/hero\n        requires: ANY\n        preserve: ALL\n    Target:\n        noun: things/walker\n        requires: ANY\n        preserve: ALL\n```"));
    gd_load(model,collections,3); rules=gd_read_rules(model); TEST_ASSERT(rules && !model->error_count);
    graph=gd_resource_graph_build(model,rules); TEST_ASSERT(graph && !graph->limited && !graph->preservation_unexamined && graph->composition_unexamined>0);
    found=all_count=other_count=0; for (i=0;i<graph->relation_count;++i) if (graph->relations[i].kind==GD_RESOURCE_PRESERVATION) {
        const GD_ResourceRelation *edge=&graph->relations[i];
        const GD_RulePreservation *promise=&rules->preservations[edge->declaration];
        if (promise->kind==GD_PRESERVE_AXIS) {
            ++found; TEST_ASSERT(graph->nodes[edge->facet].axis==find_record(model,"actions/Form") && !edge->conditional);
        } else {
            TEST_ASSERT(edge->conditional==(promise->kind==GD_PRESERVE_OTHER));
            if (promise->kind==GD_PRESERVE_ALL) ++all_count; else ++other_count;
        }
    }
    TEST_ASSERT(found==1 && all_count>0 && other_count>0); gd_resource_graph_destroy(graph);
    analysis=gd_analyze_rules(model,rules); TEST_ASSERT(analysis && !model->error_count && analysis->call_count==3);
    graph=gd_resource_graph_build(model,rules); TEST_ASSERT(graph && gd_resource_graph_bind_calls(graph,analysis));
    TEST_ASSERT(graph->calls==analysis && graph->calls_bound && graph->composition_unexamined==0);
    found=0; for (i=0;i<graph->relation_count;++i) if (graph->relations[i].kind==GD_RESOURCE_INVOCATION) {
        const GD_ResourceRelation *edge=&graph->relations[i]; const GD_RuleCall *call=&analysis->calls[edge->declaration];
        ++found; TEST_ASSERT(graph->nodes[edge->activation].case_index==call->caller_case);
        TEST_ASSERT(graph->nodes[edge->facet].case_index>=call->first_case && graph->nodes[edge->facet].case_index<call->first_case+call->case_count);
        TEST_ASSERT(call->binding_count>0 && edge->line==call->line);
    }
    TEST_ASSERT(found==3 && !gd_resource_graph_bind_calls(graph,analysis)); gd_resource_graph_destroy(graph);
    life=find_record(model,"actions/Life"); form=find_record(model,"actions/Form"); amount=find_record(model,"actions/Amount");
    root_case=gd_rule_case_find(model,rules,"cases/root#composed"); TEST_ASSERT(root_case>=0);
    hero=rules->cases[root_case].first_participant; budget=hero+1;
    facts[0]=(GD_RulePredicateFact){hero,life,"alive",GD_RULE_TRUTH_TRUE}; facts[1]=(GD_RulePredicateFact){hero,form,"small",GD_RULE_TRUTH_TRUE};
    facts[2]=(GD_RulePredicateFact){budget,amount,"3",GD_RULE_TRUTH_TRUE};
    result=&analysis->scenario; gd_rule_evaluate(analysis,root_case,inputs,result);
    TEST_ASSERT(result->input_valid==1 && result->applicability==GD_RULE_TRUTH_TRUE && !result->limited);
    TEST_ASSERT(result->constraints_complete && result->values_complete && result->expansions==2 && result->write_count==3 && !result->conflict_count);
    found=0; for (i=0;i<result->write_count;++i) if (result->writes[i].axis==amount) { ++found; TEST_ASSERT(result->writes[i].value.number==2 && result->writes[i].value.known); }
    TEST_ASSERT(found==1 && !strcmp(facts[0].value,"alive") && !strcmp(facts[1].value,"small") && !strcmp(facts[2].value,"3"));
    root_case=gd_rule_case_find(model,rules,"cases/root#named-conflict"); hero=rules->cases[root_case].first_participant;
    facts[0].participant=facts[1].participant=hero; facts[2].participant=hero+1;
    gd_rule_evaluate(analysis,root_case,inputs,result);
    TEST_ASSERT(result->applicability==GD_RULE_TRUTH_TRUE && result->conflict_count==1 && !result->constraints_complete);
    root_case=gd_rule_case_find(model,rules,"cases/root#guard"); inputs.fact_count=0;
    gd_rule_evaluate(analysis,root_case,inputs,result);
    TEST_ASSERT(result->applicability==GD_RULE_TRUTH_FALSE && result->exclusion.state==GD_EXCLUSION_EXCLUDED && result->write_count==0);
    root_case=gd_rule_case_find(model,rules,"cases/root#context"); hero=rules->cases[root_case].first_participant;
    context=(GD_RuleContextFact){hero+1,hero,find_record(model,"actions/Contact"),"sideContacts",GD_RULE_TRUTH_TRUE}; inputs.contexts=&context; inputs.context_count=1;
    gd_rule_evaluate(analysis,root_case,inputs,result); TEST_ASSERT(result->context==GD_RULE_TRUTH_UNKNOWN && result->applicability==GD_RULE_TRUTH_UNKNOWN);
    context.left=hero; context.right=hero+1;
    gd_rule_evaluate(analysis,root_case,inputs,result); TEST_ASSERT(result->context==GD_RULE_TRUTH_TRUE && result->applicability==GD_RULE_TRUTH_TRUE);
    context.truth=GD_RULE_TRUTH_FALSE;
    gd_rule_evaluate(analysis,root_case,inputs,result); TEST_ASSERT(result->applicability==GD_RULE_TRUTH_FALSE);
    root_case=gd_rule_case_find(model,rules,"cases/helper#loop"); hero=rules->cases[root_case].first_participant; inputs.context_count=0;
    gd_rule_evaluate(analysis,root_case,inputs,result); TEST_ASSERT(result->cycle_count==1 && !result->constraints_complete && result->expansions==1);
    facts[0]=(GD_RulePredicateFact){hero,life,"alive",GD_RULE_TRUTH_TRUE}; facts[1]=(GD_RulePredicateFact){hero,life,"dead",GD_RULE_TRUTH_TRUE}; inputs.fact_count=2;
    gd_rule_evaluate(analysis,root_case,inputs,result); TEST_ASSERT(result->input_valid==0 && result->applicability==GD_RULE_TRUTH_UNKNOWN);
    facts[0]=(GD_RulePredicateFact){hero,amount,"3",GD_RULE_TRUTH_TRUE}; inputs.fact_count=1;
    gd_rule_evaluate(analysis,root_case,inputs,result); TEST_ASSERT(result->input_valid==0); /* Axis not owned by Hero. */
    facts[0].participant=0;
    gd_rule_evaluate(analysis,root_case,inputs,result); TEST_ASSERT(result->input_valid==0); /* Participant outside root case. */
    root_case=gd_rule_case_find(model,rules,"cases/root#context"); hero=rules->cases[root_case].first_participant;
    context=(GD_RuleContextFact){hero,hero+1,find_record(model,"actions/Contact"),"unlisted",GD_RULE_TRUTH_TRUE}; inputs.fact_count=0; inputs.context_count=1;
    gd_rule_evaluate(analysis,root_case,inputs,result); TEST_ASSERT(result->input_valid==0); /* Closed context vocabulary. */
    gd_rule_analysis_destroy(analysis); gd_rules_destroy(rules); gd_destroy(model);
)
TEST(rule_quantities,
    GD_Model *model=model_create(); GD_Rules *rules;
    GD_Collection domains[4]; char directory[GD_MAX_PATH], path[GD_MAX_PATH];
    GD_RulePredicateFact facts[5]; GD_RuleQuantityValue value; GD_ResourceGraph *graph;
    int amount,life, first,second,other,recognized;
    const char *control="---\nvalue_type: integer\nvalue: 1\nunit: energy\n---\nConfigured cost, not starting energy.";
    TEST_ASSERT(model && setup("rule-quantities")); collections[1].role=GD_AXIS; collections[2].role=GD_RULE;
    TEST_ASSERT(write_record(1,"Amount","---\nvalue_type: integer\nunit: energy\nminimum: 0\nmaximum: 3\n---\nEnergy."));
    TEST_ASSERT(write_record(1,"Life","---\nvalue_type: enum\nallowed_values: [alive, dead]\nclosed: true\n---\nLife."));
    TEST_ASSERT(write_record(1,"Form","---\nvalue_type: enum\nallowed_values: [small, tall]\nclosed: true\n---\nForm."));
    TEST_ASSERT(write_record(0,"hero","---\naxes: [actions/Amount, actions/Life, actions/Form]\n---\nActor."));
    TEST_ASSERT(write_record(2,"numeric","```gd-rule\ncase numeric:\n    Actor:\n        noun: things/hero\n"
        "        requires: alive AND (Amount >= (1[energy] + 1[energy]) AND Amount < 4[energy]) AND Amount > 0.5[energy] AND {controls/policy}\n"
        "        skip_if: Amount <= 0[energy]\n        becomes: {controls/form}\n        set:\n            Amount: this - {controls/cost}\n"
        "            Amount: this + 1[energy]\n            Amount: -1[energy]\n"
        "    Other:\n        noun: things/hero\n        requires: ANY\n        set:\n            Amount: this + 1[energy]\n```\n"));
    TEST_ASSERT(snprintf(directory,sizeof(directory),"%s/numeric-controls",root)<(int)sizeof(directory) && file_directory_create(directory,1));
    TEST_ASSERT(snprintf(path,sizeof(path),"%s/cost.md",directory)<(int)sizeof(path));
    TEST_ASSERT(file_write_bytes(cstring_from_raw(path),control,strlen(control))==file_error_ok);
    TEST_ASSERT(snprintf(path,sizeof(path),"%s/policy.md",directory)<(int)sizeof(path));
    { const char *text="---\nvalue_type: boolean\nvalue: true\n---\nPolicy.";
      TEST_ASSERT(file_write_bytes(cstring_from_raw(path),text,strlen(text))==file_error_ok); }
    TEST_ASSERT(snprintf(path,sizeof(path),"%s/form.md",directory)<(int)sizeof(path));
    { const char *text="---\nvalue_type: enum\nvalue: small\nallowed_values: [small, tall]\n---\nConfigured form, not an actor initialization.";
      TEST_ASSERT(file_write_bytes(cstring_from_raw(path),text,strlen(text))==file_error_ok); }
    memcpy(domains,collections,sizeof(collections)); domains[3]=(GD_Collection){"controls",directory,GD_CONTROL};
    TEST_ASSERT(gd_load(model,domains,4)); rules=gd_read_rules(model);
    TEST_ASSERT(rules && !model->error_count && rules->quantity_bindings_complete && rules->controls && rules->controls->resolved_count==3 && rules->effect_bindings_complete);
    TEST_ASSERT(rules->assignment_count==4 && rules->quantity_count==8);
    graph=gd_resource_graph_build(model,rules);
    TEST_ASSERT(graph && !graph->limited && graph->node_count>=3 && graph->relation_count>=8);
    { int i,writes=0,requirements=0,exclusions=0;
      for (i=0;i<graph->relation_count;++i) {
          const GD_ResourceRelation *edge=&graph->relations[i];
          TEST_ASSERT(graph->nodes[edge->activation].kind==GD_RESOURCE_CASE && graph->nodes[edge->facet].kind==GD_RESOURCE_FACET);
          if (edge->kind==GD_RESOURCE_QUANTITY_WRITE) ++writes;
          if (edge->kind==GD_RESOURCE_REQUIREMENT) ++requirements;
          if (edge->kind==GD_RESOURCE_EXCLUSION) ++exclusions;
      }
      TEST_ASSERT(writes==4 && requirements>=3 && exclusions==1); }
    gd_resource_graph_destroy(graph);
    TEST_ASSERT(rules->state_effect_count==1 && !strcmp(rules->state_effects[0].target.state,"small"));
    TEST_ASSERT(rules->state_effects[0].target.axis_record==find_record(model,"actions/Form"));
    amount=find_record(model,"actions/Amount"); life=find_record(model,"actions/Life");
    facts[0]=(GD_RulePredicateFact){0,amount,"3",GD_RULE_TRUTH_TRUE};
    facts[1]=(GD_RulePredicateFact){0,life,"alive",GD_RULE_TRUTH_TRUE};
    first=rules->assignments[0].quantity; second=rules->assignments[1].quantity; other=rules->assignments[3].quantity;
    TEST_ASSERT(gd_rule_quantity_value(rules,first,NULL,0).state==GD_EXPRESSION_UNKNOWN);
    value=gd_rule_quantity_delta(rules,first,&recognized);
    TEST_ASSERT(recognized && value.known && value.checks_complete && value.number==-1);
    value=gd_rule_quantity_delta(rules,second,&recognized);
    TEST_ASSERT(recognized && value.known && value.number==1);
    value=gd_rule_quantity_delta(rules,rules->assignments[2].quantity,&recognized);
    TEST_ASSERT(!recognized && !value.known);
    value=gd_rule_quantity_value(rules,second,facts,2);
    TEST_ASSERT(value.state==GD_EXPRESSION_VALUE && value.known && value.number==4 && value.bounds==GD_BOUNDS_OUTSIDE);
    value=gd_rule_quantity_value(rules,first,facts,2);
    TEST_ASSERT(value.state==GD_EXPRESSION_VALUE && value.number==2 && value.bounds==GD_BOUNDS_WITHIN);
    TEST_ASSERT(gd_rule_quantity_value(rules,other,facts,2).state==GD_EXPRESSION_UNKNOWN);
    TEST_ASSERT(rules->quantities[rules->assignments[2].quantity].inspection.number==-1);
    TEST_ASSERT(rules->quantities[rules->assignments[2].quantity].inspection.bounds==GD_BOUNDS_OUTSIDE);
    TEST_ASSERT(gd_rule_condition_value(rules,&rules->participants[0].required_condition,facts,2)==GD_RULE_TRUTH_TRUE);
    TEST_ASSERT(gd_rule_case_exclusions(rules,0,facts,2).state==GD_EXCLUSION_NOT_EXCLUDED);
    facts[2]=(GD_RulePredicateFact){0,amount,"2",GD_RULE_TRUTH_TRUE};
    TEST_ASSERT(gd_rule_condition_value(rules,&rules->participants[0].required_condition,facts,3)==GD_RULE_TRUTH_UNKNOWN);
    facts[0].value="0";
    TEST_ASSERT(gd_rule_case_exclusions(rules,0,facts,2).state==GD_EXCLUSION_EXCLUDED);
    facts[0].value="2.5";
    TEST_ASSERT(gd_rule_quantity_value(rules,first,facts,2).state==GD_EXPRESSION_UNKNOWN);
    facts[0].value="3"; facts[0].participant=1;
    TEST_ASSERT(gd_rule_quantity_value(rules,other,facts,2).number==4 && !gd_sources_changed(model));
    gd_rules_destroy(rules); gd_destroy(model);
)
TEST(rule_numeric_comparisons,
    const char *operators[]={"==","!=","<","<=",">",">="}, *values[]={"2","3","4"};
    const GD_RuleTruth expected[6][3]={{1,2,1},{2,1,2},{2,1,1},{2,2,1},{1,1,2},{1,2,2}};
    int i,j;
    for (i=0;i<6;++i) {
        GD_Model *model=model_create(); GD_Rules *rules; GD_RulePredicateFact fact;
        char name[64],body[512];
        snprintf(name,sizeof(name),"rule-numeric-comparison-%d",i);
        TEST_ASSERT(model && setup(name)); collections[1].role=GD_AXIS; collections[2].role=GD_RULE;
        TEST_ASSERT(write_record(1,"Amount","---\nvalue_type: integer\nunit: life\nmaximum: 1\n---\nBound does not constrain comparison thresholds."));
        TEST_ASSERT(write_record(0,"hero","---\naxes: [actions/Amount]\n---\nActor."));
        snprintf(body,sizeof(body),"```gd-rule\ncase test:\n    A:\n        noun: things/hero\n        requires: Amount %s 3[life]\n```\n",operators[i]);
        TEST_ASSERT(write_record(2,"comparison",body) && gd_load(model,collections,3)); rules=gd_read_rules(model);
        TEST_ASSERT(rules && !model->error_count && rules->condition_bindings_complete);
        for (j=0;j<3;++j) {
            fact=(GD_RulePredicateFact){0,find_record(model,"actions/Amount"),values[j],GD_RULE_TRUTH_TRUE};
            TEST_ASSERT(gd_rule_condition_value(rules,&rules->participants[0].required_condition,&fact,1)==expected[i][j]);
        }
        TEST_ASSERT(rules->quantities[0].inspection.bounds==GD_BOUNDS_UNKNOWN);
        gd_rules_destroy(rules); gd_destroy(model);
    }
)
TEST(rule_condition_logic,
    GD_Model *model = model_create(); GD_Rules *rules; int life, protection, a, b, owner;
    GD_RulePredicateFact facts[5]; GD_RuleExclusionCheck exclusion;
    const GD_RuleTruth conjunction[3][3] = {{0,1,0},{1,1,1},{0,1,2}};
    const GD_RuleTruth disjunction[3][3] = {{0,0,2},{0,1,2},{2,2,2}};
    const GD_RuleTruth negation[3] = {0,2,1};
    TEST_ASSERT(model && setup("rule-condition-logic")); collections[1].role = GD_AXIS; collections[2].role = GD_RULE;
    TEST_ASSERT(write_record(1,"Life","---\nvalue_type: enum\nallowed_values: [alive, dead]\nclosed: true\n---\nLife."));
    TEST_ASSERT(write_record(1,"Form","---\nvalue_type: enum\nallowed_values: [small, tall]\n---\nForm."));
    TEST_ASSERT(write_record(1,"Protection","---\nvalue_type: enum\nallowed_values: [protected, unprotected]\n---\nProtection."));
    TEST_ASSERT(write_record(0,"hero","---\naxes: [actions/Life, actions/Form, actions/Protection]\n---\nSame noun, distinct participants."));
    TEST_ASSERT(write_record(2,"contact","```gd-rule\ncase contact:\n"
        "    First:\n        noun: things/hero\n        requires: alive AND (tall OR small) AND NOT protected\n        skip_if: dead\n        other_axes: ANY\n"
        "    Second:\n        noun: things/hero\n        requires: alive\n        pools:\n            Life: resources/player-life\n            actions/Form: resources/player-form\n            Unknown: resources/unknown\n"
        "    Unset:\n        noun: things/hero\n        other_axes: ANY\n"
        "    Constant:\n        requires: ANY\n"
        "    UnknownNot:\n        noun: things/hero\n        requires: NOT protected\n"
        "    KnownOr:\n        noun: things/hero\n        requires: ANY OR protected\n"
        "    KnownAnd:\n        noun: things/hero\n        requires: NOT ANY AND protected\n"
        "    TruthAnd:\n        noun: things/hero\n        requires: alive AND protected\n"
        "    TruthOr:\n        noun: things/hero\n        requires: alive OR protected\n"
        "    TruthNot:\n        noun: things/hero\n        requires: NOT protected\n"
        "case no-guard:\n    Actor:\n        noun: things/hero\n        requires: ANY\n"
        "case always-skip:\n    Actor:\n        noun: things/hero\n        skip_if: ANY\n"
        "case never-skip:\n    Actor:\n        noun: things/hero\n        skip_if: NOT ANY\n```\n"));
    TEST_ASSERT(gd_load(model,collections,3)); rules = gd_read_rules(model);
    TEST_ASSERT(rules && !model->error_count && rules->condition_bindings_complete && rules->condition_count == 13);
    life = find_record(model,"actions/Life"); protection = find_record(model,"actions/Protection");
    TEST_ASSERT(rules->pool_count == 3 && rules->pools[0].target.resolution == GD_RULE_RESOLVED && rules->pools[0].target.axis_record == life);
    TEST_ASSERT(rules->pools[1].target.resolution == GD_RULE_RESOLVED && rules->pools[1].target.axis_record == find_record(model,"actions/Form"));
    TEST_ASSERT(rules->pools[2].target.resolution == GD_RULE_MISSING && rules->pools[2].target.axis_record == -1);
    facts[0] = (GD_RulePredicateFact){0,life,"alive",GD_RULE_TRUTH_TRUE};
    facts[1] = (GD_RulePredicateFact){0,find_record(model,"actions/Form"),"tall",GD_RULE_TRUTH_TRUE};
    facts[2] = (GD_RulePredicateFact){0,find_record(model,"actions/Form"),"small",GD_RULE_TRUTH_FALSE};
    facts[3] = (GD_RulePredicateFact){0,protection,"protected",GD_RULE_TRUTH_FALSE};
    facts[4] = (GD_RulePredicateFact){0,life,"dead",GD_RULE_TRUTH_FALSE};
    exclusion = gd_rule_case_exclusions(rules,0,facts,5);
    TEST_ASSERT(exclusion.state == GD_EXCLUSION_NOT_EXCLUDED && exclusion.guard_count == 1 && exclusion.checked_count == 1 && exclusion.excluded_by == -1);
    TEST_ASSERT(gd_rule_case_exclusions(rules,0,NULL,0).state == GD_EXCLUSION_UNKNOWN);
    TEST_ASSERT(gd_rule_case_exclusions(rules,-1,facts,5).state == GD_EXCLUSION_UNKNOWN);
    TEST_ASSERT(gd_rule_case_exclusions(rules,2,NULL,0).state == GD_EXCLUSION_EXCLUDED);
    TEST_ASSERT(gd_rule_case_exclusions(rules,3,NULL,0).state == GD_EXCLUSION_NOT_EXCLUDED);
    TEST_ASSERT(gd_rule_condition_value(rules,&rules->participants[0].required_condition,facts,5) == GD_RULE_TRUTH_TRUE);
    TEST_ASSERT(gd_rule_condition_value(rules,&rules->participants[0].skip_condition,facts,5) == GD_RULE_TRUTH_FALSE);
    facts[4].truth = GD_RULE_TRUTH_TRUE;
    exclusion = gd_rule_case_exclusions(rules,0,facts,5);
    TEST_ASSERT(exclusion.state == GD_EXCLUSION_EXCLUDED && exclusion.excluded_by == 0 && !exclusion.limited);
    exclusion = gd_rule_case_exclusions(rules,1,facts,5);
    TEST_ASSERT(exclusion.state == GD_EXCLUSION_ABSENT && exclusion.guard_count == 0 && exclusion.checked_count == 0 && !exclusion.work);
    TEST_ASSERT(gd_rule_condition_value(rules,&rules->participants[0].skip_condition,facts,5) == GD_RULE_TRUTH_TRUE);
    facts[4].truth = GD_RULE_TRUTH_FALSE;
    facts[3].truth = GD_RULE_TRUTH_UNKNOWN;
    TEST_ASSERT(gd_rule_condition_value(rules,&rules->participants[0].required_condition,facts,5) == GD_RULE_TRUTH_UNKNOWN);
    facts[0].truth = GD_RULE_TRUTH_FALSE;
    TEST_ASSERT(gd_rule_condition_value(rules,&rules->participants[0].required_condition,facts,5) == GD_RULE_TRUTH_FALSE);
    facts[0].truth = GD_RULE_TRUTH_TRUE;
    TEST_ASSERT(gd_rule_condition_value(rules,&rules->participants[1].required_condition,facts,5) == GD_RULE_TRUTH_UNKNOWN);
    facts[0].participant = 1;
    TEST_ASSERT(gd_rule_condition_value(rules,&rules->participants[1].required_condition,facts,5) == GD_RULE_TRUTH_TRUE);
    TEST_ASSERT(rules->participants[1].skip_condition.state == GD_CONDITION_UNSPECIFIED);
    TEST_ASSERT(rules->participants[2].other_axes_mode == GD_RULE_OTHER_ANY && rules->participants[2].required_condition.state == GD_CONDITION_UNSPECIFIED);
    TEST_ASSERT(gd_rule_condition_value(rules,&rules->participants[2].required_condition,NULL,0) == GD_RULE_TRUTH_UNKNOWN);
    TEST_ASSERT(gd_rule_condition_value(rules,&rules->participants[3].required_condition,NULL,0) == GD_RULE_TRUTH_TRUE);
    TEST_ASSERT(gd_rule_condition_value(rules,&rules->participants[4].required_condition,NULL,0) == GD_RULE_TRUTH_UNKNOWN);
    TEST_ASSERT(gd_rule_condition_value(rules,&rules->participants[5].required_condition,NULL,0) == GD_RULE_TRUTH_TRUE);
    TEST_ASSERT(gd_rule_condition_value(rules,&rules->participants[6].required_condition,NULL,0) == GD_RULE_TRUTH_FALSE);
    for (owner = 7; owner < 10; ++owner) for (a = 0; a < 3; ++a) for (b = 0; b < 3; ++b) {
        facts[0] = (GD_RulePredicateFact){owner,life,"alive",(GD_RuleTruth)a};
        facts[1] = (GD_RulePredicateFact){owner,protection,"protected",(GD_RuleTruth)b};
        TEST_ASSERT(gd_rule_condition_value(rules,&rules->participants[owner].required_condition,facts,2) ==
                    (owner == 7 ? conjunction[a][b]:owner == 8 ? disjunction[a][b]:negation[b]));
    }
    facts[0] = (GD_RulePredicateFact){1,life,"alive",GD_RULE_TRUTH_TRUE};
    facts[1] = (GD_RulePredicateFact){1,life,"alive",GD_RULE_TRUTH_FALSE};
    TEST_ASSERT(gd_rule_condition_value(rules,&rules->participants[1].required_condition,facts,2) == GD_RULE_TRUTH_UNKNOWN);
    TEST_ASSERT(gd_rule_condition_value(rules,&rules->participants[1].required_condition,NULL,1) == GD_RULE_TRUTH_UNKNOWN);
    TEST_ASSERT(gd_rule_condition_value(rules,&rules->participants[1].required_condition,facts,GD_MAX_RULE_CONDITION_ITEMS+1) == GD_RULE_TRUTH_UNKNOWN);
    TEST_ASSERT(gd_rule_condition_value(rules,&rules->participants[1].required_condition,facts,-1) == GD_RULE_TRUTH_UNKNOWN);
    TEST_ASSERT(!gd_sources_changed(model)); gd_rules_destroy(rules); gd_destroy(model);
)
TEST(rule_exclusion_limits,
    GD_Model *model = model_create(); GD_Rules *rules;
    GD_RulePredicateFact facts[GD_MAX_RULE_CONDITION_ITEMS]; GD_RuleExclusionCheck result;
    const int guards = GD_MAX_RULE_CONDITION_TOKENS/(GD_MAX_RULE_CONDITION_ITEMS-1);
    size_t used; int i, j, life, symbol_work;
    TEST_ASSERT(model && setup("rule-exclusion-limits")); collections[1].role = GD_AXIS; collections[2].role = GD_RULE;
    TEST_ASSERT(write_record(1,"Life","---\nvalue_type: enum\nallowed_values: [alive, dead]\nclosed: true\n---\nLife."));
    TEST_ASSERT(write_record(0,"hero","---\naxes: [actions/Life]\n---\nActor."));
    used = (size_t)snprintf(model->input,sizeof(model->input),"```gd-rule\ncase guards:\n");
    for (i = 0; i < guards; ++i) {
        used += (size_t)snprintf(model->input+used,sizeof(model->input)-used,"    A%d:\n        noun: things/hero\n        skip_if: ",i);
        for (j = 0; j < GD_MAX_RULE_CONDITION_ITEMS/2; ++j)
            used += (size_t)snprintf(model->input+used,sizeof(model->input)-used,"%sdead",j ? " OR ":"");
        TEST_ASSERT(used+6 < sizeof(model->input)); model->input[used++] = '\n';
    }
    strcpy(model->input+used,"```\n"); TEST_ASSERT(write_record(2,"guards",model->input));
    TEST_ASSERT(gd_load(model,collections,3)); rules = gd_read_rules(model);
    TEST_ASSERT(rules && !model->error_count && rules->cases[0].skip_if_count == guards);
    life = find_record(model,"actions/Life"); symbol_work = rules->symbol_work;
    for (i = 0; i < GD_MAX_RULE_CONDITION_ITEMS; ++i)
        facts[i] = (GD_RulePredicateFact){i,life,"dead",GD_RULE_TRUTH_FALSE};
    result = gd_rule_case_exclusions(rules,0,facts,GD_MAX_RULE_CONDITION_ITEMS);
    TEST_ASSERT(result.state == GD_EXCLUSION_UNKNOWN && result.limited && result.guard_count == guards);
    TEST_ASSERT(result.checked_count > 0 && result.checked_count < guards && result.work <= GD_RULE_EVALUATION_WORK_LIMIT && result.excluded_by == -1);
    facts[0].truth = GD_RULE_TRUTH_TRUE;
    result = gd_rule_case_exclusions(rules,0,facts,GD_MAX_RULE_CONDITION_ITEMS);
    TEST_ASSERT(result.state == GD_EXCLUSION_EXCLUDED && result.excluded_by == 0 && result.checked_count == 1 && !result.limited);
    facts[0].truth = GD_RULE_TRUTH_UNKNOWN; facts[1].truth = GD_RULE_TRUTH_TRUE;
    result = gd_rule_case_exclusions(rules,0,facts,GD_MAX_RULE_CONDITION_ITEMS);
    TEST_ASSERT(result.state == GD_EXCLUSION_EXCLUDED && result.excluded_by == 1 && result.checked_count == 2 && !result.limited);
    TEST_ASSERT(gd_rule_case_exclusions(rules,0,NULL,1).state == GD_EXCLUSION_UNKNOWN);
    TEST_ASSERT(gd_rule_case_exclusions(rules,0,facts,GD_MAX_RULE_CONDITION_ITEMS+1).state == GD_EXCLUSION_UNKNOWN);
    TEST_ASSERT(rules->symbol_work == symbol_work && !gd_sources_changed(model));
    gd_rules_destroy(rules); gd_destroy(model);
)
TEST(rule_effect_bindings,
    GD_Model *model = model_create(); GD_Rules *rules; int life, form, time, amount;
    TEST_ASSERT(model && setup("rule-effect-bindings")); collections[1].role = GD_AXIS; collections[2].role = GD_RULE;
    TEST_ASSERT(write_record(1,"Life","---\nvalue_type: enum\nallowed_values: [alive, dead, dead]\nclosed: true\n---\nLife."));
    TEST_ASSERT(write_record(1,"Form","---\nvalue_type: enum\nallowed_values: [small, tall]\n---\nForm."));
    TEST_ASSERT(write_record(1,"Time","---\nvalue_type: number\nunit: s\n---\nTime."));
    TEST_ASSERT(write_record(1,"Amount","---\nvalue_type: integer\nunit: life\n---\nAmount."));
    TEST_ASSERT(write_record(0,"hero","---\naxes: [actions/Life, actions/Form, actions/Time, actions/Life]\n---\nHero."));
    TEST_ASSERT(write_record(0,"lives","---\naxes: [actions/Amount]\n---\nSeparate noun."));
    TEST_ASSERT(write_record(2,"contact","```gd-rule\ncase contact:\n    First:\n        noun: things/hero\n        becomes: small, Form.small, Life.dead\n        set:\n            Time: {controls/window}\n    Second:\n        noun: things/hero\n        preserve: ALL\n    Lives:\n        noun: things/lives\n        set:\n            Amount: this + 1[life]\n```\n"));
    TEST_ASSERT(gd_load(model,collections,3)); rules = gd_read_rules(model);
    TEST_ASSERT(rules && !model->error_count && rules->effect_bindings_complete && rules->state_effect_count == 3);
    life = find_record(model,"actions/Life"); form = find_record(model,"actions/Form");
    time = find_record(model,"actions/Time"); amount = find_record(model,"actions/Amount");
    TEST_ASSERT(rules->participants[0].state_effect_count == 3 && !rules->participants[1].state_effect_count);
    TEST_ASSERT(rules->state_effects[0].target.axis_record == form && rules->state_effects[1].target.axis_record == form);
    TEST_ASSERT(rules->state_effects[2].target.axis_record == life && !strcmp(rules->state_effects[2].target.state,"dead"));
    TEST_ASSERT(rules->state_effects[0].target.resolution == GD_RULE_RESOLVED && rules->state_effects[0].length == 5);
    TEST_ASSERT(!strncmp(rules->state_effects[0].text,"small",5) && !strcmp(rules->participants[0].becomes,"small, Form.small, Life.dead"));
    TEST_ASSERT(rules->assignments[0].target.resolution == GD_RULE_RESOLVED && rules->assignments[0].target.axis_record == time);
    TEST_ASSERT(rules->assignments[1].target.resolution == GD_RULE_RESOLVED && rules->assignments[1].target.axis_record == amount);
    TEST_ASSERT(!strcmp(rules->assignments[1].expression,"this + 1[life]"));
    TEST_ASSERT(rules->symbol_work > 0 && rules->symbol_work <= GD_RULE_SYMBOL_WORK_LIMIT && !gd_sources_changed(model));
    gd_rules_destroy(rules); gd_destroy(model);
)
TEST(rule_axis_definitions,
    GD_Model *model = model_create(); GD_Rules *rules; int life, amount, hero, isolated, empty, null_axes;
    TEST_ASSERT(model && setup("rule-axis-definitions")); collections[1].role = GD_AXIS; collections[2].role = GD_RULE;
    TEST_ASSERT(write_record(1,"Life","---\nvalue_type: enum\nallowed_values: [alive, dead, alive]\nclosed: false\n---\nKnown states, open set."));
    TEST_ASSERT(write_record(1,"Amount","---\nvalue_type: integer\nunit: life\nminimum: -0\nmaximum: 99\n---\nQuantity, not initial value."));
    TEST_ASSERT(write_record(1,"Time","---\nvalue_type: number\nunit: s\n---\nNo invented numeric bounds."));
    TEST_ASSERT(write_record(1,"Unknown","---\nvalue_type: unspecified\n---\nIncomplete definition allowed."));
    TEST_ASSERT(write_record(0,"hero","---\naxes: [actions/Life, actions/Time, actions/Life]\n---\nActor."));
    TEST_ASSERT(write_record(0,"lives","---\naxes: [actions/Amount]\n---\nPool stays a noun."));
    TEST_ASSERT(write_record(0,"isolated","Actor with unspecified axes."));
    TEST_ASSERT(write_record(0,"empty","---\naxes: []\n---\nNo axes currently declared."));
    TEST_ASSERT(write_record(0,"null","---\naxes: null\n---\nExplicitly unspecified."));
    TEST_ASSERT(write_record(2,"rule","```gd-rule\ncase contact:\n    A:\n        noun: things/hero\n    B:\n        noun: things/hero\n    Lives:\n        noun: things/lives\n```\n"));
    TEST_ASSERT(gd_load(model,collections,3)); rules = gd_read_rules(model); TEST_ASSERT(rules && !model->error_count);
    life = find_record(model,"actions/Life"); amount = find_record(model,"actions/Amount"); hero = find_record(model,"things/hero");
    isolated = find_record(model,"things/isolated"); empty = find_record(model,"things/empty"); null_axes = find_record(model,"things/null");
    TEST_ASSERT(rules->axis_count == 4 && rules->actor_count == 5 && !rules->axis_definitions_complete && !rules->bindings_complete);
    TEST_ASSERT(rules->axes[life].type == GD_AXIS_ENUM && rules->axes[life].valid && !model->records[life].axis_closed);
    TEST_ASSERT(model->records[life].axis.allowed_count == 3 && model->records[life].axis_closed_line == 4);
    TEST_ASSERT(rules->axes[amount].type == GD_AXIS_INTEGER && rules->axes[amount].unit_known && rules->axes[amount].valid);
    TEST_ASSERT(!strcmp(model->records[amount].axis.minimum,"-0") && !model->records[amount].axis_closed_line);
    TEST_ASSERT(rules->actor_axes_complete[hero] && !rules->actor_axes_complete[isolated] && rules->actor_axes_complete[empty] && !rules->actor_axes_complete[null_axes]);
    TEST_ASSERT(model->records[null_axes].axes_line == 2 && !model->records[null_axes].axes_declared);
    TEST_ASSERT(model->records[empty].axes_declared && rules->actor_first[empty] == -1);
    TEST_ASSERT(rules->participants[0].noun_resolution == GD_RULE_RESOLVED && rules->participants[0].noun_record == hero);
    TEST_ASSERT(rules->participants[1].noun_record == hero && strcmp(rules->participants[0].name,rules->participants[1].name));
    TEST_ASSERT(rules->next_axis[rules->next_axis[rules->actor_first[hero]]] >= 0); /* Duplicate declaration retained. */
    gd_rules_destroy(rules); gd_destroy(model);
)
TEST(rule_axis_errors,
    const char *definitions[] = {
        "value_type: boolean\n", "value_type: integer\nunit: life\nminimum: 1.5\n",
        "value_type: number\nunit: s\nminimum: 2\nmaximum: 1\n", "value_type: number\nunit: s+life\n",
        "value_type: enum\nunit: s\n", "value_type: integer\nunit: life\nclosed: true\n",
        "value_type: enum\nallowed_values: [alive, null]\n", "value_type: enum\nallowed_values: [\"\"]\n",
        "value_type: enum\nclosed: null\n", "value_type: enum\nclosed: false\nclosed: true\n",
        "value_type: integer\nunit: life\nminimum: 9007199254740992\n", "value_type: number\nminimum: \"0\"\n"
    };
    const char *codes[] = {"rules.axis_type","rules.axis_bound","rules.axis_bounds","rules.axis_unit","rules.axis_shape",
        "rules.axis_shape","rules.axis_value_type","rules.axis_value_empty","rules.axis_closed_type","input.duplicate_field","rules.axis_bound","rules.axis_field_type"};
    int i;
    for (i = 0; i < (int)(sizeof(definitions)/sizeof(definitions[0])); ++i) {
        GD_Model *model = model_create(); GD_Rules *rules; char name[64], text[1024];
        snprintf(name,sizeof(name),"rule-axis-error-%d",i);
        TEST_ASSERT(model && setup(name)); collections[1].role = GD_AXIS; collections[2].role = GD_RULE;
        snprintf(text,sizeof(text),"---\n%s---\nDefinition.",definitions[i]); TEST_ASSERT(write_record(1,"Bad",text));
        (void)gd_load(model,collections,3); rules = gd_read_rules(model);
        TEST_ASSERT(rules && has_error(model,codes[i]) && !rules->axis_definitions_complete && !rules->bindings_complete);
        TEST_ASSERT(model->errors[0].line > 0);
        gd_rules_destroy(rules); gd_destroy(model);
    }
)
TEST(rule_axis_references,
    GD_Model *model = model_create(); GD_Rules *rules; int hero, edge, count = 0;
    TEST_ASSERT(model && setup("rule-axis-references")); collections[1].role = GD_AXIS; collections[2].role = GD_RULE;
    TEST_ASSERT(write_record(1,"Life","---\nvalue_type: enum\nallowed_values: [alive, dead]\nclosed: true\n---\nStates."));
    TEST_ASSERT(write_record(0,"hero","---\naxes: [actions/Life, actions/absent, foreign/Life, things/hero]\n---\nActor."));
    TEST_ASSERT(write_record(2,"rule","```gd-rule\ncase refs:\n    A:\n        noun: things/hero\n    Missing:\n        noun: things/absent\n    Unloaded:\n        noun: foreign/actor\n    Wrong:\n        noun: actions/Life\n    Unknown:\n        requires: ANY\n```\n"));
    TEST_ASSERT(gd_load(model,collections,3)); rules = gd_read_rules(model); TEST_ASSERT(rules && !model->error_count);
    TEST_ASSERT(rules->axis_definitions_complete && !rules->bindings_complete);
    hero = find_record(model,"things/hero"); edge = rules->actor_first[hero];
    TEST_ASSERT(rules->axis_resolution[edge] == GD_RULE_RESOLVED); edge = rules->next_axis[edge];
    TEST_ASSERT(rules->axis_resolution[edge] == GD_RULE_MISSING); edge = rules->next_axis[edge];
    TEST_ASSERT(rules->axis_resolution[edge] == GD_RULE_UNLOADED); edge = rules->next_axis[edge];
    TEST_ASSERT(rules->axis_resolution[edge] == GD_RULE_WRONG_ROLE && rules->next_axis[edge] == -1);
    TEST_ASSERT(rules->participants[0].noun_resolution == GD_RULE_RESOLVED);
    TEST_ASSERT(rules->participants[1].noun_resolution == GD_RULE_MISSING);
    TEST_ASSERT(rules->participants[2].noun_resolution == GD_RULE_UNLOADED);
    TEST_ASSERT(rules->participants[3].noun_resolution == GD_RULE_WRONG_ROLE);
    TEST_ASSERT(rules->participants[4].noun_resolution == GD_RULE_UNSPECIFIED);
    for (edge = 0; edge < rules->finding_count; ++edge) if (!strcmp(rules->findings[edge].code,"rules.axis_reference")) ++count;
    TEST_ASSERT(count == 3);
    gd_rules_destroy(rules); gd_destroy(model);
)
TEST(rule_axis_incomplete,
    GD_Model *model = model_create(); GD_Rules *rules;
    TEST_ASSERT(model && setup("rule-axis-incomplete")); collections[1].role = GD_AXIS; collections[2].role = GD_RULE;
    TEST_ASSERT(write_record(1,"Time","---\nvalue_type: number\n---\nUnit not yet known."));
    TEST_ASSERT(write_record(1,"Open","---\nvalue_type: enum\nallowed_values: []\n---\nOpen, no invented states."));
    TEST_ASSERT(write_record(1,"ClosedEmpty","---\nvalue_type: enum\nallowed_values: []\nclosed: true\n---\nExplicit empty domain, not a parse failure."));
    TEST_ASSERT(write_record(0,"actor","---\naxes: [actions/Time]\n---\nActor."));
    TEST_ASSERT(gd_load(model,collections,3)); rules = gd_read_rules(model);
    TEST_ASSERT(rules && !model->error_count && !rules->axis_definitions_complete && rules->bindings_complete);
    TEST_ASSERT(rules->axes[find_record(model,"actions/Time")].valid && !rules->axes[find_record(model,"actions/Time")].unit_known);
    TEST_ASSERT(!model->records[find_record(model,"actions/Open")].axis_closed && model->records[find_record(model,"actions/ClosedEmpty")].axis_closed);
    gd_rules_destroy(rules); gd_destroy(model);
)
TEST(rule_axis_limits,
    {
        GD_Model *model = model_create(); GD_Rules *rules; size_t used; int i;
        TEST_ASSERT(model && setup("rule-axis-value-limit")); collections[1].role = GD_AXIS; collections[2].role = GD_RULE;
        used = (size_t)snprintf(model->input,sizeof(model->input),"---\nvalue_type: enum\nallowed_values: [");
        for (i = 0; i <= GD_MAX_ENUM_VALUES; ++i)
            used += (size_t)snprintf(model->input+used,sizeof(model->input)-used,"%sv%d",i ? ", ":"",i);
        strcpy(model->input+used,"]\n---\nValues."); TEST_ASSERT(write_record(1,"Many",model->input));
        TEST_ASSERT(!gd_load(model,collections,3)); rules = gd_read_rules(model);
        TEST_ASSERT(rules && has_error(model,"rules.axis_value_limit") && !rules->axis_definitions_complete);
        gd_rules_destroy(rules); gd_destroy(model);
    }
    {
        GD_Model *model = model_create(); GD_Rules *rules; int i;
        TEST_ASSERT(model && setup("rule-axis-finding-limit")); collections[1].role = GD_AXIS; collections[2].role = GD_RULE;
        for (i = 0; i <= GD_MAX_ERRORS; ++i) { char name[32]; snprintf(name,sizeof(name),"actor-%d",i); TEST_ASSERT(write_record(0,name,"Axes unspecified.")); }
        TEST_ASSERT(gd_load(model,collections,3)); rules = gd_read_rules(model);
        TEST_ASSERT(rules && !model->error_count && rules->finding_count == GD_MAX_ERRORS && rules->findings_omitted == 1 && !rules->bindings_complete);
        gd_rules_destroy(rules); gd_destroy(model);
    }
    {
        GD_Model *model = model_create(); GD_Rules *rules; int actor;
        TEST_ASSERT(model && setup("rule-axis-ambiguous")); collections[1].role = GD_AXIS; collections[2].role = GD_RULE;
        TEST_ASSERT(write_record(1,"a","---\nname: Life\nvalue_type: enum\n---\nAxis A."));
        TEST_ASSERT(write_record(1,"b","---\nname: Life\nvalue_type: enum\n---\nAxis B."));
        TEST_ASSERT(write_record(0,"a","---\nname: hero\naxes: [actions/Life]\n---\nActor A."));
        TEST_ASSERT(write_record(0,"b","---\nname: hero\naxes: [actions/Life]\n---\nActor B."));
        TEST_ASSERT(write_record(2,"refs","```gd-rule\ncase refs:\n    Ambiguous:\n        noun: things/hero\n    Missing:\n        noun: things/absent\n```\n"));
        TEST_ASSERT(!gd_load(model,collections,3)); rules = gd_read_rules(model);
        TEST_ASSERT(rules && has_error(model,"input.identity") && rules->participants[0].noun_resolution == GD_RULE_AMBIGUOUS);
        TEST_ASSERT(rules->participants[1].noun_resolution == GD_RULE_UNAVAILABLE);
        actor = find_record(model,"things/hero"); TEST_ASSERT(rules->axis_resolution[rules->actor_first[actor]] == GD_RULE_AMBIGUOUS);
        TEST_ASSERT(!rules->axis_definitions_complete && !rules->bindings_complete);
        gd_rules_destroy(rules); gd_destroy(model);
    }
)
TEST(rules_declarations,
    GD_Model *model = model_create();
    GD_Rules *rules;
    TEST_ASSERT(model && setup("rules-declarations"));
    collections[2].role = GD_RULE;
    TEST_ASSERT(write_record(0,"hero","Hero."));
    TEST_ASSERT(write_record(2,"contact",
        "---\nname: contact\nstatus: accepted\ncoverage: defined\n---\n# Intent\n\n```gd-rule\n"
        "case powered-side-contact:\n    context: Player sideContacts Walker\n\n"
        "    Player:\n        noun: things/hero\n        requires: alive AND (tall OR spark) AND NOT protected\n"
        "        other_axes: ANY\n        becomes: small, protected\n        pools:\n            Amount: resources/wallet\n        set:\n"
        "            ProtectionTime: {control-vars/safety-window}\n            Amount: this + 1[life]\n"
        "            Amount: this + 2[life]\n        preserve: OTHER\n"
        "    Walker:\n        noun: things/hero\n        requires: alive\n"
        "    Lives:\n        requires: ANY\n        preserve: ALL\n"
        "case unknown-result:\n    unknown_conditions: Unspecified placement predicate.\n    unknown_effects: Missing rebound magnitude.\n    Player:\n        skip_if: dead OR (hidden AND NOT revealed)\n"
        "        causes: mechanics/power-loss\n```\n"));
    TEST_ASSERT(write_record(2,"legacy","Legacy prose remains unknown."));
    TEST_ASSERT(write_record(2,"empty","```gd-rule\n# Design not filled in.\n```\n"));
    TEST_ASSERT(gd_load(model,collections,3));
    TEST_ASSERT(model->rule_mode && !model->cell_count && !model->component_count);
    rules = gd_read_rules(model);
    TEST_ASSERT(rules && !model->error_count);
    TEST_ASSERT(rules->source_count == 3 && rules->case_count == 2 && rules->participant_count == 4 && rules->assignment_count == 3);
    TEST_ASSERT(rules->sources[0].state == GD_RULE_PARSED && rules->sources[0].line == 8);
    TEST_ASSERT(rules->sources[1].state == GD_RULE_EMPTY && rules->sources[2].state == GD_RULE_ABSENT);
    TEST_ASSERT(rules->cases[0].line == 9 && rules->cases[0].context_line == 10 && rules->participants[0].line == 12);
    TEST_ASSERT(!strcmp(rules->cases[0].context,"Player sideContacts Walker"));
    TEST_ASSERT(rules->context_count == 1 && !rules->context_bindings_complete);
    TEST_ASSERT(rules->cases[0].ordered_context.state == GD_CONDITION_PARSED);
    TEST_ASSERT(rules->cases[0].ordered_context.left == 0 && rules->cases[0].ordered_context.right == 1);
    TEST_ASSERT(rules->cases[0].ordered_context.target.resolution == GD_RULE_UNDETERMINED);
    TEST_ASSERT(rules->cases[1].ordered_context.state == GD_CONDITION_UNSPECIFIED && rules->cases[1].ordered_context.left == -1);
    TEST_ASSERT(!strcmp(rules->participants[0].requires,"alive AND (tall OR spark) AND NOT protected"));
    TEST_ASSERT(rules->pool_count == 1 && rules->participants[0].pool_count == 1);
    TEST_ASSERT(!strcmp(rules->pools[0].axis,"Amount") && !strcmp(rules->pools[0].reference,"resources/wallet"));
    TEST_ASSERT(rules->pools[0].line == rules->participants[0].lines[8] + 1);
    TEST_ASSERT(!strcmp(rules->assignments[0].expression,"{control-vars/safety-window}"));
    TEST_ASSERT(!strcmp(rules->assignments[1].expression,"this + 1[life]") && !strcmp(rules->assignments[2].expression,"this + 2[life]"));
    TEST_ASSERT(!rules->participants[1].preserve && !rules->participants[1].becomes && !rules->participants[1].other_axes);
    TEST_ASSERT(!rules->participants[2].noun && !strcmp(rules->participants[2].requires,"ANY"));
    TEST_ASSERT(!rules->cases[1].context && !rules->participants[3].requires && !rules->participants[3].noun);
    TEST_ASSERT(!strcmp(rules->participants[3].causes,"mechanics/power-loss"));
    TEST_ASSERT(!strcmp(rules->cases[1].unknown_conditions,"Unspecified placement predicate.") && !strcmp(rules->cases[1].unknown_effects,"Missing rebound magnitude."));
    TEST_ASSERT(rules->cases[1].unknown_conditions_line>rules->cases[1].line && rules->cases[1].unknown_effects_line>rules->cases[1].unknown_conditions_line);
    gd_rules_destroy(rules); gd_destroy(model);
)
TEST(resource_unloaded_call,
    GD_Model *model=model_create(); GD_Rules *rules; GD_RuleAnalysis *analysis; GD_ResourceGraph *graph;
    TEST_ASSERT(model && setup("resource-unloaded-call")); collections[2].role=GD_RULE;
    TEST_ASSERT(write_record(2,"source","```gd-rule\ncase source:\n    Actor:\n        causes: external/action\n        preserve: ALL\n```\n"));
    TEST_ASSERT(gd_load(model,collections,3)); rules=gd_read_rules(model); TEST_ASSERT(rules && !model->error_count);
    analysis=gd_analyze_rules(model,rules); TEST_ASSERT(analysis && !model->error_count && analysis->call_count==1);
    TEST_ASSERT(analysis->calls[0].resolution==GD_RULE_UNLOADED);
    graph=gd_resource_graph_build(model,rules); TEST_ASSERT(graph && graph->composition_unexamined==1 && graph->preservation_unexamined==1);
    TEST_ASSERT(gd_resource_graph_bind_calls(graph,analysis) && graph->composition_unexamined==1 && !graph->relation_count);
    gd_resource_graph_destroy(graph); gd_rule_analysis_destroy(analysis); gd_rules_destroy(rules); gd_destroy(model);
)
TEST(rule_expression_delta,
    GD_Model *model = model_create(); GD_ControlToken tokens[GD_MAX_EXPRESSION_TOKENS];
    GD_Expression expression = {tokens,0,GD_MAX_EXPRESSION_TOKENS};
    GD_ExpressionSite site = {0,1,GD_EXPRESSION_ASSIGNMENT}; GD_ExpressionInput starting = {0}, delta;
    const char *texts[] = {"this", "this - 100[coin]", "this + -2[coin]", "this - -3[coin]", "this + 0[coin]", "min(this + 1[coin], 9[coin])", "this + 1[life]", "this + this", "this * 2 + 1[coin]"};
    const double expected[] = {0,-100,-2,3,0}; int i, recognized; GD_ExpressionState state;
    TEST_ASSERT(model && setup("rule-expression-delta")); collections[2].role=GD_RULE;
    TEST_ASSERT(write_record(2,"source","Source.")); TEST_ASSERT(gd_load(model,collections,3));
    site.record=find_record(model,"cases/source"); TEST_ASSERT(site.record>=0);
    starting.value.kind=GD_LITERAL_NUMBER; starting.unit_known=1;
    TEST_ASSERT(gd_control_unit("coin",4,&starting.value.unit));
    for (i=0;i<9;++i) {
        expression.count=0; TEST_ASSERT(gd_compile_expression(model,&expression,texts[i],strlen(texts[i]),site));
        state=gd_expression_delta(tokens,expression.count,NULL,&starting,1,&recognized,&delta);
        if (i<5) TEST_ASSERT(recognized && state==GD_EXPRESSION_VALUE && delta.known && delta.unit_known && delta.value.number==expected[i]);
        else if (i==6) TEST_ASSERT(recognized && state==GD_EXPRESSION_UNIT_ERROR);
        else TEST_ASSERT(!recognized && state==GD_EXPRESSION_UNKNOWN);
    }
    gd_destroy(model);
)
TEST(rule_pool_resolution,
    GD_Model *model = model_create(); GD_Rules *rules; GD_ResourceGraph *graph; GD_Collection inputs[4];
    char folder[GD_MAX_PATH], path[GD_MAX_PATH];
    TEST_ASSERT(model && setup("rule-pool-resolution")); collections[1].role = GD_AXIS; collections[2].role = GD_RULE;
    TEST_ASSERT(snprintf(folder,sizeof(folder),"%s/rule-pool-resolution/resources",root) < (int)sizeof(folder));
    TEST_ASSERT(file_directory_create(folder,1));
    TEST_ASSERT(snprintf(path,sizeof(path),"%s/wallet.md",folder) < (int)sizeof(path));
    { const char *text = "---\nnoun: things/coin\n---\nWallet.";
      TEST_ASSERT(file_write_bytes(cstring_from_raw(path),text,strlen(text)) == file_error_ok); }
    TEST_ASSERT(write_record(0,"hero","---\naxes: [actions/Amount]\n---\nHero."));
    TEST_ASSERT(write_record(1,"Amount","---\nvalue_type: integer\nunit: coin\n---\nAmount."));
    TEST_ASSERT(write_record(2,"trade","```gd-rule\ncase trade:\n    Actor:\n        noun: things/hero\n        pools:\n            Amount: resources/wallet\n            actions/Amount: resources/missing\n            Amount: other/wallet\n            Amount: things/hero\n        set:\n            Amount: this - 1[coin]\n    Other:\n        noun: things/hero\n        pools:\n            Amount: resources/wallet\n        set:\n            Amount: this + 1[coin]\n    Third:\n        noun: things/hero\n        pools:\n            Amount: resources/wallet\n        set:\n            Amount: this + 2[coin]\n    Missing:\n        noun: things/hero\n        pools:\n            Amount: resources/absent\n        set:\n            Amount: this - 1[coin]\n    Unloaded:\n        noun: things/hero\n        pools:\n            Amount: other/absent\n        set:\n            Amount: this - 1[coin]\n```\n"));
    memcpy(inputs,collections,sizeof(collections)); inputs[3] = (GD_Collection){"resources",folder,GD_RESOURCE};
    TEST_ASSERT(gd_load(model,inputs,4)); rules = gd_read_rules(model); TEST_ASSERT(rules && !model->error_count);
    TEST_ASSERT(rules->pool_count == 8 && !rules->pool_bindings_complete);
    TEST_ASSERT(rules->pools[0].resource_resolution == GD_RULE_RESOLVED && rules->pools[0].target.resolution == GD_RULE_RESOLVED);
    TEST_ASSERT(!strcmp(model->records[rules->pools[0].resource_record].resource_noun,"things/coin"));
    TEST_ASSERT(rules->pools[1].resource_resolution == GD_RULE_MISSING);
    TEST_ASSERT(rules->pools[2].resource_resolution == GD_RULE_UNLOADED);
    TEST_ASSERT(rules->pools[3].resource_resolution == GD_RULE_WRONG_ROLE);
    TEST_ASSERT(rules->pools[0].identity_resolution == GD_RULE_AMBIGUOUS && rules->pools[3].identity_resolution == GD_RULE_AMBIGUOUS);
    TEST_ASSERT(rules->pools[4].identity_resolution == GD_RULE_RESOLVED && rules->pools[5].identity_resolution == GD_RULE_RESOLVED);
    graph=gd_resource_graph_build(model,rules);
    TEST_ASSERT(graph && !graph->limited && graph->node_count==8 && graph->relation_count==8 && graph->unresolved_facets==3);
    TEST_ASSERT(graph->nodes[0].kind==GD_RESOURCE_ROLE && graph->nodes[0].record==find_record(model,"resources/wallet"));
    TEST_ASSERT(graph->relations[1].kind==GD_RESOURCE_ROLE_BINDING && graph->relations[1].activation==0);
    TEST_ASSERT(graph->relations[0].facet!=graph->relations[2].facet && graph->relations[2].facet==graph->relations[3].facet);
    TEST_ASSERT(graph->relations[0].delta_recognized && graph->relations[0].delta.known && graph->relations[0].delta.number==-1);
    TEST_ASSERT(graph->relations[2].delta_recognized && graph->relations[2].delta.known && graph->relations[2].delta.number==1);
    TEST_ASSERT(graph->nodes[graph->relations[4].activation].kind==GD_RESOURCE_ROLE && graph->nodes[graph->relations[4].activation].resolution==GD_RULE_MISSING);
    TEST_ASSERT(graph->nodes[graph->relations[6].activation].kind==GD_RESOURCE_ROLE && graph->nodes[graph->relations[6].activation].resolution==GD_RULE_UNLOADED);
    gd_resource_graph_destroy(graph);
    TEST_ASSERT(!gd_sources_changed(model)); gd_rules_destroy(rules); gd_destroy(model);
)
TEST(rules_fence_scope,
    GD_Model *model = model_create();
    GD_Rules *rules;
    TEST_ASSERT(model && setup("rules-fence-scope")); collections[2].role = GD_RULE;
    TEST_ASSERT(write_record(2,"quoted",
        "````markdown\r\n```gd-rule\r\ncase hidden:\r\n```\r\n````\r\n"
        "   ~~~ gd-rule\r\ncase real:\r\n    context: ANY\r\n    Actor:\r\n"
        "        noun: things/actor\r\n        becomes: unknown\r\n   ~~~~\r\n"));
    TEST_ASSERT(write_record(2,"z-duplicate","```gd-rule\ncase real:\n```\n"));
    TEST_ASSERT(gd_load(model,collections,3)); rules = gd_read_rules(model);
    TEST_ASSERT(rules && !model->error_count && rules->case_count == 2);
    TEST_ASSERT(!strcmp(rules->cases[0].name,"real") && rules->cases[0].line == 7);
    TEST_ASSERT(!strcmp(rules->cases[1].name,"real") && rules->cases[0].source != rules->cases[1].source);
    TEST_ASSERT(!strcmp(rules->cases[0].context,"ANY") && rules->participants[0].lines[4] == 11);
    gd_rules_destroy(rules); gd_destroy(model);
)
TEST(rules_invalid_records,
    const char *inputs[] = {
        "```gd-rule\ncase bad:\n    A:\n        noun: things/a\n        noun: things/b\n```\n",
        "```gd-rule\ncase bad:\n    A:\n    A:\n```\n",
        "```gd-rule\ncase bad:\ncase bad:\n```\n",
        "```gd-rule\ncase bad:\n    A:\n        mystery: wrong\n```\n",
        "```gd-rule\ncase bad:\n    A:\n        requires:\n```\n",
        "```gd-rule\ncase bad:\n    A:\n          noun: things/a\n```\n",
        "```gd-rule\ncase bad:\n\tA:\n```\n",
        "```gd-rule\ncase bad:\n    A:\n        set: Amount: 1\n```\n",
        "```gd-rule\ncase bad:\n    A:\n        set:\n            Amount:\n```\n",
        "```gd-rule\ncase bad:\n    A:\n        pools:\n            Amount: resources/valid\n            Other: bad\n```\n",
        "```gd-rule\ncase bad:\n    context: ANY\n    context: ANY\n```\n",
        "```gd-rule\ncase bad:\n    A:\n        noun: things/a\n",
        "```gd-rule\ncase bad:\n```\n```gd-rule\ncase another:\n```\n",
        "```gd-rule\n    A:\n```\n"
    };
    const char *codes[] = {"rules.duplicate_field","rules.duplicate_participant","rules.duplicate_case","rules.field",
        "rules.value","rules.indent","rules.indent","rules.assignment","rules.assignment","rules.pool","rules.duplicate_field",
        "rules.unclosed_block","rules.multiple_blocks","rules.case"};
    int i;
    for (i = 0; i < (int)(sizeof(inputs)/sizeof(inputs[0])); ++i) {
        GD_Model *model = model_create(); GD_Rules *rules; char name[64];
        snprintf(name,sizeof(name),"rules-invalid-%d",i);
        TEST_ASSERT(model && setup(name)); collections[2].role = GD_RULE;
        TEST_ASSERT(write_record(2,"a-good","```gd-rule\ncase good:\n    Actor:\n        becomes: alive\n```\n"));
        TEST_ASSERT(write_record(2,"bad",inputs[i]));
        TEST_ASSERT(write_record(2,"z-good","```gd-rule\ncase later:\n    Actor:\n        preserve: OTHER\n```\n"));
        TEST_ASSERT(gd_load(model,collections,3)); rules = gd_read_rules(model);
        TEST_ASSERT(rules && has_error(model,codes[i]));
        TEST_ASSERT(rules->case_count == 2 && rules->participant_count == 2 && !rules->assignment_count && !rules->pool_count);
        TEST_ASSERT(rules->sources[1].state == GD_RULE_INVALID && !rules->sources[1].case_count);
        TEST_ASSERT(!strcmp(rules->cases[0].name,"good") && !strcmp(rules->cases[1].name,"later"));
        TEST_ASSERT(!strcmp(rules->participants[0].becomes,"alive") && !strcmp(rules->participants[1].preserve,"OTHER"));
        TEST_ASSERT(model->errors[0].line > 0);
        gd_rules_destroy(rules); gd_destroy(model);
    }
)
TEST(rules_limits,
    int mode;
    const char *codes[] = {"rules.case_limit","rules.participant_limit","rules.assignment_limit","rules.line_limit"};
    for (mode = 0; mode < 4; ++mode) {
        GD_Model *model = model_create(); GD_Rules *rules; char name[64];
        size_t used; int i, count = mode == 0 ? GD_MAX_RULE_CASES : mode == 1 ? GD_MAX_RULE_PARTICIPANTS : GD_MAX_RULE_ASSIGNMENTS;
        snprintf(name,sizeof(name),"rules-limit-%d",mode);
        TEST_ASSERT(model && setup(name)); collections[2].role = GD_RULE;
        used = (size_t)snprintf(model->input,sizeof(model->input),"```gd-rule\n");
        if (mode) used += (size_t)snprintf(model->input+used,sizeof(model->input)-used,"case big:\n");
        if (mode == 2) used += (size_t)snprintf(model->input+used,sizeof(model->input)-used,"    Actor:\n        set:\n");
        if (mode == 3) {
            memset(model->input+used,'x',GD_MAX_RULE_LINE+1); used += GD_MAX_RULE_LINE+1;
            model->input[used++] = '\n';
        } else for (i = 0; i <= count; ++i)
            used += (size_t)snprintf(model->input+used,sizeof(model->input)-used,
                mode == 0 ? "case c%d:\n" : mode == 1 ? "    A%d:\n" : "            Amount: this + %d[life]\n",i);
        TEST_ASSERT(used+5 < sizeof(model->input));
        strcpy(model->input+used,"```\n");
        TEST_ASSERT(write_record(2,"limit",model->input));
        TEST_ASSERT(gd_load(model,collections,3)); rules = gd_read_rules(model);
        TEST_ASSERT(rules && has_error(model,codes[mode]));
        TEST_ASSERT(!rules->case_count && !rules->participant_count && !rules->assignment_count && !rules->text_used);
        gd_rules_destroy(rules); gd_destroy(model);
    }
    {
        GD_Model *model = model_create(); GD_Rules *rules; size_t used; int i, j;
        TEST_ASSERT(model && setup("rules-text-limit")); collections[2].role = GD_RULE;
        for (i = 0; i < 6; ++i) {
            char name[32]; snprintf(name,sizeof(name),"text-%d",i);
            used = (size_t)snprintf(model->input,sizeof(model->input),"```gd-rule\ncase text:\n    Actor:\n        set:\n");
            for (j = 0; j < 120; ++j) {
                used += (size_t)snprintf(model->input+used,sizeof(model->input)-used,"            Amount: ");
                TEST_ASSERT(used+7501+5 < sizeof(model->input));
                memset(model->input+used,'x',7500); used += 7500; model->input[used++] = '\n';
            }
            strcpy(model->input+used,"```\n"); TEST_ASSERT(write_record(2,name,model->input));
        }
        TEST_ASSERT(gd_load(model,collections,3)); rules = gd_read_rules(model);
        /* Reader retains the four complete records. Their deliberate 7500-byte
           nonsense RHS also receives independent quantity-syntax diagnostics. */
        TEST_ASSERT(rules && has_error(model,"rules.text_limit") && has_error(model,"rules.quantity_syntax"));
        TEST_ASSERT(rules->source_count == 6 && rules->case_count == 4 && rules->assignment_count == 480);
        TEST_ASSERT(rules->sources[4].state == GD_RULE_INVALID && rules->sources[5].state == GD_RULE_INVALID);
        TEST_ASSERT(rules->text_used < GD_RULE_TEXT_CAPACITY && strlen(rules->assignments[479].expression) == 7500);
        gd_rules_destroy(rules); gd_destroy(model);
    }
)
TEST(empty,
    GD_Model *model = model_create();
    TEST_ASSERT(model && setup("empty"));
    TEST_ASSERT(gd_load(model, collections, 3));
    TEST_ASSERT(model->record_count == 0 && model->cell_count == 0);
    TEST_ASSERT(!gd_sources_changed(model));
    gd_destroy(model);
)
TEST(matrix,
    GD_Model *model = model_create();
    int box, wall, push;
    TEST_ASSERT(model && setup("matrix"));
    TEST_ASSERT(write_record(0, "box", "---\nname: box\nstatus: accepted\n---\nBox."));
    TEST_ASSERT(write_record(0, "wall", "Wall."));
    TEST_ASSERT(write_record(1, "push", "Push."));
    TEST_ASSERT(write_record(1, "pull", "---\nstatus: rejected\n---\nPull."));
    TEST_ASSERT(write_record(2, "first", "---\nstatus: accepted\ncoverage: defined\nnouns: [things/box, things/box]\nverb: actions/push\ncontext_nouns: [things/wall]\n---\nFirst condition."));
    TEST_ASSERT(write_record(2, "second", "---\nstatus: accepted\ncoverage: defined\nnouns: [things/box]\nverb: actions/push\n---\nOther condition."));
    TEST_ASSERT(write_record(2, "excluded", "---\nstatus: accepted\ncoverage: excluded\nnouns: [things/wall]\nverb: actions/push\n---\nNot available."));
    TEST_ASSERT(write_record(2, "proposal", "---\nstatus: proposed\ncoverage: defined\nnouns: [things/box, things/wall]\nverb: actions/pull\n---\nAlternative."));
    TEST_ASSERT(gd_load(model, collections, 3));
    TEST_ASSERT(model->cell_count == 4 && model->pair_count == 5 && !model->unplaced_cases);
    box = find_record(model, "things/box"); wall = find_record(model, "things/wall"); push = find_record(model, "actions/push");
    TEST_ASSERT(box >= 0 && wall >= 0 && push >= 0);
    TEST_ASSERT(model->connectivity[box].partners == 1 && model->connectivity[box].cases == 3);
    TEST_ASSERT(model->connectivity[wall].partners == 0 && model->connectivity[wall].cases == 2);
    TEST_ASSERT(model->connectivity[push].partners == 1 && model->connectivity[push].active_cases == 2);
    TEST_ASSERT(model->component_count == 3);
    TEST_ASSERT(model->component_of[box] == model->component_of[push]);
    TEST_ASSERT(model->component_of[wall] != model->component_of[box]);
    TEST_ASSERT(model->components[model->component_of[box]].edge_count == 1);
    TEST_ASSERT(!gd_sources_changed(model));
    TEST_ASSERT(write_record(0, "new", "New node."));
    TEST_ASSERT(gd_sources_changed(model));
    {
        GD_Model *fresh = model_create();
        TEST_ASSERT(fresh && gd_load(fresh, collections, 3));
        TEST_ASSERT(fresh->record_count == model->record_count + 1);
        gd_destroy(fresh);
        TEST_ASSERT(write_record(0, "new", "---\nname: [unsafe]\n---\n"));
        fresh = model_create();
        TEST_ASSERT(fresh && !gd_load(fresh, collections, 3));
        TEST_ASSERT(model->cell_count == 4 && model->connectivity[box].partners == 1);
        TEST_ASSERT(!strcmp(model->records[box].body, "Box."));
        gd_destroy(fresh);
    }
    gd_destroy(model);
)
TEST(unsafe_input,
    GD_Model *model = model_create();
    TEST_ASSERT(model && setup("unsafe"));
    TEST_ASSERT(write_record(0, "duplicate", "---\nname: box\nname: other\n---\n"));
    TEST_ASSERT(write_record(1, "invalid", "---\nname: [nested]\n---\n"));
    TEST_ASSERT(write_record(2, "broken", "---\nnouns: [broken\n---\n"));
    TEST_ASSERT(write_record(0, "escaped-nul", "---\nname: \"box\\0hidden\"\n---\n"));
    TEST_ASSERT(write_record(0, "merge", "---\n<<: {name: box}\n---\n"));
    TEST_ASSERT(!gd_load(model, collections, 3));
    TEST_ASSERT(has_error(model, "input.duplicate_field") && has_error(model, "input.field_type") && has_error(model, "input.yaml"));
    TEST_ASSERT(has_error(model, "input.field_nul") && has_error(model, "input.yaml_merge"));
    TEST_ASSERT(model->record_count == 0 && model->edge_count == 0);
    gd_destroy(model);
)
TEST(ambiguous_endpoints,
    GD_Model *model = model_create();
    TEST_ASSERT(model && setup("ambiguous"));
    TEST_ASSERT(write_record(0, "one", "---\nname: box\n---\n"));
    TEST_ASSERT(write_record(0, "two", "---\nname: box\n---\n"));
    TEST_ASSERT(write_record(1, "push", "Push."));
    TEST_ASSERT(write_record(2, "case", "---\nnouns: [things/box]\nverb: actions/push\n---\n"));
    TEST_ASSERT(!gd_load(model, collections, 3));
    TEST_ASSERT(has_error(model, "input.identity") && has_error(model, "matrix.endpoint"));
    TEST_ASSERT(model->cell_count == 0 && model->unplaced_cases == 1);
    gd_destroy(model);
)
TEST(text_types,
    GD_Model *model = model_create();
    int record;
    TEST_ASSERT(model && setup("types"));
    TEST_ASSERT(write_record(0, "fallback", "---\nname: !!str null\nid: null\nrank: -1\nmetadata: {ignored: [one, two]}\n---\nMeaning preserved."));
    TEST_ASSERT(write_record(1, "numeric", "---\nname: 123\n---\n"));
    TEST_ASSERT(!gd_load(model, collections, 3));
    TEST_ASSERT(has_error(model, "input.field_type"));
    record = find_record(model, "things/null");
    TEST_ASSERT(record >= 0 && !model->records[record].id);
    TEST_ASSERT(strstr(model->records[record].body, "Meaning preserved."));
    gd_destroy(model);
)
TEST(missing_endpoint,
    GD_Model *model = model_create();
    TEST_ASSERT(model && setup("missing"));
    TEST_ASSERT(write_record(0, "box", "Box."));
    TEST_ASSERT(write_record(1, "push", "Push."));
    TEST_ASSERT(write_record(2, "case", "---\nnouns: [things/missing]\nverb: things/box\n---\n"));
    TEST_ASSERT(!gd_load(model, collections, 3));
    TEST_ASSERT(has_error(model, "matrix.endpoint"));
    TEST_ASSERT(!model->cell_count && model->unplaced_cases == 1);
    gd_destroy(model);
)
TEST(input_limits,
    GD_Model *model = model_create();
    char path[GD_MAX_PATH];
    TEST_ASSERT(model && setup("limits"));
    memset(model->input, 'x', sizeof(model->input));
    snprintf(path, sizeof(path), "%s/oversized.md", folders[0]);
    TEST_ASSERT(file_write_bytes(cstring_from_raw(path), model->input, sizeof(model->input)) == file_error_ok);
    snprintf(path, sizeof(path), "%s/binary.md", folders[0]);
    TEST_ASSERT(file_write_bytes(cstring_from_raw(path), "x\0y", 3) == file_error_ok);
    memcpy(model->input, "---\nignored: ", 13);
    snprintf(path, sizeof(path), "%s/frontmatter.md", folders[1]);
    TEST_ASSERT(file_write_bytes(cstring_from_raw(path), model->input, GD_MAX_FRONTMATTER + 32) == file_error_ok);
    TEST_ASSERT(!gd_load(model, collections, 3));
    TEST_ASSERT(has_error(model, "input.read_limit") && has_error(model, "input.utf8") && has_error(model, "input.frontmatter"));
    gd_destroy(model);
)
TEST(component_chain,
    GD_Model *model = model_create(), *reordered = model_create();
    GD_Collection reversed[3];
    int a, b, c, x, y, i;
    TEST_ASSERT(model && reordered && setup("chain"));
    TEST_ASSERT(write_record(0, "a", "A.")); TEST_ASSERT(write_record(0, "b", "B.")); TEST_ASSERT(write_record(0, "c", "C."));
    TEST_ASSERT(write_record(1, "x", "X.")); TEST_ASSERT(write_record(1, "y", "Y."));
    TEST_ASSERT(write_record(2, "ax", "---\nstatus: accepted\ncoverage: defined\nnouns: [things/a]\nverb: actions/x\n---\n"));
    TEST_ASSERT(write_record(2, "bx", "---\nstatus: accepted\ncoverage: defined\nnouns: [things/b]\nverb: actions/x\n---\n"));
    TEST_ASSERT(write_record(2, "by", "---\nstatus: accepted\ncoverage: defined\nnouns: [things/b]\nverb: actions/y\n---\n"));
    TEST_ASSERT(gd_load(model, collections, 3));
    a = find_record(model, "things/a"); b = find_record(model, "things/b"); c = find_record(model, "things/c");
    x = find_record(model, "actions/x"); y = find_record(model, "actions/y");
    TEST_ASSERT(model->component_count == 2 && model->component_of[a] == model->component_of[b]);
    TEST_ASSERT(model->component_of[a] == model->component_of[x] && model->component_of[x] == model->component_of[y]);
    TEST_ASSERT(model->component_of[c] != model->component_of[a]);
    TEST_ASSERT(model->components[model->component_of[a]].edge_count == 3);
    reversed[0] = collections[2]; reversed[1] = collections[1]; reversed[2] = collections[0];
    TEST_ASSERT(gd_load(reordered, reversed, 3));
    TEST_ASSERT(reordered->component_count == model->component_count);
    for (i = 0; i < model->record_count; ++i) TEST_ASSERT(model->component_of[i] == reordered->component_of[i]);
    gd_destroy(reordered); gd_destroy(model);
)
static GD_Layout test_layout;
TEST(layout_modes,
    GD_Layout *layout = &test_layout;
    float first_x, first_y, other_x;
    int i;
    memset(layout, 0, sizeof(*layout)); layout->node_count = 3; layout->edge_count = 2; layout->group_count = 1;
    layout->nodes[0] = (GD_LayoutNode){.key=0, .label="zeta", .degree=1};
    layout->nodes[1] = (GD_LayoutNode){.key=1, .label="alpha", .side=1, .degree=2};
    layout->nodes[2] = (GD_LayoutNode){.key=2, .label="beta", .degree=3};
    layout->edges[0] = (GD_LayoutEdge){0,1,0,1,0}; layout->edges[1] = (GD_LayoutEdge){1,2,1,1,0};
    TEST_ASSERT(gd_layout_arrange(layout, GD_LAYOUT_COLUMNS, GD_SORT_NAME));
    TEST_ASSERT(layout->order[0] == 1 && layout->order[1] == 2 && layout->order[2] == 0);
    first_x = layout->nodes[0].x; first_y = layout->nodes[0].y;
    gd_layout_step(layout, -1);
    TEST_ASSERT(layout->nodes[0].x == first_x && layout->nodes[0].y == first_y && layout->tick == 0);
    TEST_ASSERT(gd_layout_arrange(layout, GD_LAYOUT_COLUMNS, GD_SORT_PARTNERS));
    TEST_ASSERT(layout->order[0] == 2 && layout->order[1] == 1 && layout->order[2] == 0);
    TEST_ASSERT(gd_layout_arrange(layout, GD_LAYOUT_FORCE, GD_SORT_NAME));
    layout->nodes[0].x = 100; layout->nodes[0].y = 200; other_x = layout->nodes[1].x;
    gd_layout_step(layout, 0);
    TEST_ASSERT(layout->nodes[0].x == 100 && layout->nodes[0].y == 200 && layout->nodes[1].x != other_x);
    TEST_ASSERT(gd_layout_arrange(layout, GD_LAYOUT_FORCE, GD_SORT_NAME));
    for (i = 0; i < 120; ++i) gd_layout_step(layout, -1);
    first_x = layout->nodes[0].x; first_y = layout->nodes[0].y;
    TEST_ASSERT(isfinite(first_x) && isfinite(first_y));
    TEST_ASSERT(gd_layout_arrange(layout, GD_LAYOUT_FORCE, GD_SORT_NAME));
    for (i = 0; i < 120; ++i) gd_layout_step(layout, -1);
    TEST_ASSERT(layout->nodes[0].x == first_x && layout->nodes[0].y == first_y);
    layout->edges[0].a = 99;
    TEST_ASSERT(!gd_layout_arrange(layout, GD_LAYOUT_FORCE, GD_SORT_NAME));
)
TEST(layout_capacity,
    GD_Layout *layout = &test_layout;
    clock_t start;
    int i;
    memset(layout, 0, sizeof(*layout)); layout->node_count = GD_MAX_RECORDS; layout->edge_count = GD_MAX_PAIRS; layout->group_count = 1;
    for (i = 0; i < layout->node_count; ++i) { layout->nodes[i].label = "node"; layout->nodes[i].key = i; layout->nodes[i].side = i%2; }
    for (i = 0; i < layout->edge_count; ++i) layout->edges[i] = (GD_LayoutEdge){i%GD_MAX_RECORDS, (i+1)%GD_MAX_RECORDS, i, 1,0};
    TEST_ASSERT(gd_layout_arrange(layout, GD_LAYOUT_FORCE, GD_SORT_NAME));
    start = clock();
    for (i = 0; i < 10; ++i) gd_layout_step(layout, -1);
    printf("layout capacity probe: 4096 nodes / 32768 edges / 10 steps: %.1f ms\n", 1000.0*(clock()-start)/CLOCKS_PER_SEC);
    for (i = 0; i < layout->node_count; ++i) TEST_ASSERT(isfinite(layout->nodes[i].x) && isfinite(layout->nodes[i].y));
    layout->node_count = GD_MAX_RECORDS+1;
    TEST_ASSERT(!gd_layout_arrange(layout, GD_LAYOUT_FORCE, GD_SORT_NAME));
)
static int setup_skills(const char *name) {
    if (!setup(name)) return 0;
    collections[0].role = GD_SKILL; collections[0].alias = "atoms";
    return 1;
}
static int has_finding(const GD_Model *model, const char *code) {
    int i;
    for (i = 0; i < model->finding_count; ++i) if (!strcmp(model->findings[i].code, code)) return 1;
    return 0;
}
TEST(skill_unlocks,
    GD_Model *model=model_create(); GD_SkillUnlocks *query; unsigned char known[GD_MAX_RECORDS]={0};
    int understanding,jump,fall,method,other,broken,cycle;
    TEST_ASSERT(model && setup_skills("skill-unlocks"));
    TEST_ASSERT(write_record(0,"understanding","---\nprerequisites: []\nmechanics: [rules/contact]\n---\nUnderstanding."));
    TEST_ASSERT(write_record(0,"jump","---\nprerequisites: []\n---\nAim a jump."));
    TEST_ASSERT(write_record(0,"fall","---\nprerequisites: []\n---\nControl a fall."));
    TEST_ASSERT(write_record(0,"method","---\nstatus: proposed\nprerequisites: [atoms/understanding, atoms/jump, atoms/jump]\nmechanics: [rules/contact]\n---\nJump method."));
    TEST_ASSERT(write_record(0,"other","---\nprerequisites: [atoms/understanding, atoms/fall]\nmechanics: [rules/contact]\n---\nFall method."));
    TEST_ASSERT(write_record(0,"broken","---\nprerequisites: [atoms/absent]\n---\nUnknown ancestry."));
    TEST_ASSERT(write_record(0,"cycle","---\nprerequisites: [atoms/cycle]\n---\nCycle."));
    TEST_ASSERT(gd_load(model,collections,1));
    understanding=find_record(model,"atoms/understanding"); jump=find_record(model,"atoms/jump"); fall=find_record(model,"atoms/fall");
    method=find_record(model,"atoms/method"); other=find_record(model,"atoms/other"); broken=find_record(model,"atoms/broken"); cycle=find_record(model,"atoms/cycle");
    query=allocate(storage,sizeof(*query)); TEST_ASSERT(query);
    known[understanding]=known[jump]=1;
    TEST_ASSERT(gd_skill_unlocks(model,known,understanding,query));
    TEST_ASSERT(query->eligibility[method]==GD_SKILL_PREREQUISITES_SUPPLIED && !query->known[method] && query->supplied[method]==2);
    TEST_ASSERT(query->eligibility[other]==GD_SKILL_NEEDS_MORE && query->missing[other]==1);
    TEST_ASSERT(query->eligibility[broken]==GD_SKILL_UNRESOLVED && query->eligibility[cycle]==GD_SKILL_UNRESOLVED);
    TEST_ASSERT(query->eligibility[fall]==GD_SKILL_NO_PREREQUISITES && !query->known[fall]);
    TEST_ASSERT(query->descendants[method] && query->descendants[other] && query->comparison_count==2);
    memset(known,0,sizeof(known)); known[method]=1;
    TEST_ASSERT(gd_skill_unlocks(model,known,method,query) && query->missing[method]==2 && !query->known[jump]);
    TEST_ASSERT(query->ancestors[understanding] && query->ancestors[jump] && !query->limited);
    TEST_ASSERT(!gd_skill_unlocks(model,known,model->record_count,query));
    TEST_ASSERT(gd_skill_unlocks(model,NULL,-1,query) && query->eligibility[method]==GD_SKILL_NEEDS_MORE);
    { GD_Draft *draft=gd_draft_create(model); TEST_ASSERT(draft);
      memset(known,0,sizeof(known)); known[understanding]=known[jump]=1;
      TEST_ASSERT(gd_draft_prerequisite(draft,method,"atoms/fall",1)==1);
      TEST_ASSERT(gd_skill_unlocks(draft->model,known,method,query) && query->eligibility[method]==GD_SKILL_NEEDS_MORE);
      TEST_ASSERT(gd_draft_undo(draft)==1);
      TEST_ASSERT(gd_skill_unlocks(draft->model,known,method,query) && query->eligibility[method]==GD_SKILL_PREREQUISITES_SUPPLIED);
      TEST_ASSERT(gd_skill_unlocks(draft->model,query->known,method,query) && query->known[understanding]);
      gd_draft_destroy(draft);
    }
    TEST_ASSERT(!gd_sources_changed(model)); release(storage,query); gd_destroy(model);
)
TEST(skill_unlock_limits,
    GD_Model *model=model_create(); GD_SkillUnlocks *query;
    int i; static char shared[129];
    TEST_ASSERT(model); memset(shared,'x',128); shared[128]=0;
    model->skill_mode=1; model->record_count=2; model->edge_count=1000;
    model->records[0].role=model->records[1].role=GD_SKILL;
    model->records[0].reference="atoms/first"; model->records[1].reference="atoms/second";
    for(i=0;i<1000;++i) { model->edges[i].source=i<500 ? 0:1; model->edges[i].field=GD_MECHANICS; model->edges[i].reference=shared; }
    query=allocate(storage,sizeof(*query)); TEST_ASSERT(query);
    TEST_ASSERT(gd_skill_unlocks(model,NULL,0,query) && query->limited && query->work<=GD_SKILL_QUERY_WORK);
    TEST_ASSERT(query->comparison_count==GD_SKILL_COMPARISONS && query->comparisons_omitted>0);
    release(storage,query); gd_destroy(model);
)
TEST(skill_chain,
    GD_Model *model = model_create();
    GD_Layout *layout = &test_layout;
    int root_node, zero, branch, merge, join, late, unranked, isolated, rank_mode, horizontal, i, step;
    TEST_ASSERT(model && setup_skills("skill-chain"));
    TEST_ASSERT(write_record(0, "root", "---\nstatus: rejected\nrank: -4\nprerequisites: []\n---\nRoot body stays unchanged."));
    TEST_ASSERT(write_record(0, "zero", "---\nrank: 0\nprerequisites: [atoms/root]\n---\n"));
    TEST_ASSERT(write_record(0, "branch", "---\nrank: null\nprerequisites: [atoms/root]\n---\n"));
    TEST_ASSERT(write_record(0, "merge", "---\nrank: -1\nprerequisites: [atoms/zero, atoms/branch, atoms/zero]\n---\n"));
    TEST_ASSERT(write_record(0, "late", "---\nrank: 3\n---\n"));
    TEST_ASSERT(write_record(0, "join", "---\nrank: 2\nprerequisites: [atoms/merge, atoms/late]\n---\n"));
    TEST_ASSERT(write_record(0, "unranked", "---\nrank: null\n---\n"));
    TEST_ASSERT(write_record(0, "unranked-child", "---\nprerequisites: [atoms/unranked]\n---\n"));
    TEST_ASSERT(write_record(0, "isolated", "---\nstatus: proposed\nrank: 9007199254740991\n---\n"));
    TEST_ASSERT(gd_load(model, collections, 1));
    root_node = find_record(model, "atoms/root"); zero = find_record(model, "atoms/zero"); branch = find_record(model, "atoms/branch");
    merge = find_record(model, "atoms/merge"); join = find_record(model, "atoms/join"); late = find_record(model, "atoms/late");
    unranked = find_record(model, "atoms/unranked"); isolated = find_record(model, "atoms/isolated");
    TEST_ASSERT(model->record_count == 9 && model->component_count == 3 && model->skill_link_count == 7);
    TEST_ASSERT(model->skills[root_node].depth == 0 && !model->skills[root_node].has_minimum && model->skills[root_node].display_rank == -4);
    TEST_ASSERT(model->records[zero].has_rank && model->records[zero].rank == 0 && model->skills[zero].minimum_rank == -4);
    TEST_ASSERT(!model->records[branch].has_rank && model->skills[branch].has_display && model->skills[branch].display_rank == -4);
    TEST_ASSERT(model->skills[merge].depth == 2 && model->skills[merge].prerequisite_count == 2 && model->skills[merge].declared_prerequisites == 3);
    TEST_ASSERT(model->skills[merge].rank_conflict && model->skills[merge].display_rank == 0 && model->records[merge].rank == -1);
    TEST_ASSERT(model->skills[join].depth == 3 && model->skills[join].display_rank == 3 && model->skills[join].rank_conflict && model->records[join].rank == 2);
    TEST_ASSERT(model->skills[late].depth == 0 && model->skills[late].display_rank == 3);
    TEST_ASSERT(!model->skills[unranked].has_display && !model->skills[unranked].has_minimum && !model->skills[unranked].incomplete);
    TEST_ASSERT(model->skills[isolated].display_rank == INT64_C(9007199254740991));
    TEST_ASSERT(!model->skill_incomplete && model->finding_count == 2 && has_finding(model, "skill.rank_conflict"));
    for (rank_mode = 0; rank_mode < 2; ++rank_mode) for (horizontal = 0; horizontal < 2; ++horizontal) {
        TEST_ASSERT(gd_skill_layout(model, layout, rank_mode, horizontal, GD_SORT_NAME));
        TEST_ASSERT(layout->node_count == 9 && layout->edge_count == 7);
        /* Routed long links, rather than distinct root columns, must avoid
           false incidence with the late independent root. */
        if (rank_mode) for (i=0;i<layout->edge_count;++i) if (layout->edges[i].a!=late && layout->edges[i].b!=late)
            TEST_ASSERT(gd_layout_route_distance(layout,i,layout->nodes[late].x,layout->nodes[late].y)>20);
        for (step = 0; step < 120; ++step) {
            /* Pointer drag attempts to move a prerequisite past descendants,
               including paused input (before stepping) and held-node settling. */
            gd_layout_move_node(layout, step%layout->node_count, 100000, 100000);
            for (i = 0; i < layout->edge_count; ++i) {
                const GD_LayoutNode *a = &layout->nodes[layout->edges[i].a], *b = &layout->nodes[layout->edges[i].b];
                TEST_ASSERT(horizontal ? a->x < b->x : a->y < b->y);
            }
            gd_layout_step(layout, step%layout->node_count);
            for (i = 0; i < layout->edge_count; ++i) {
                const GD_LayoutNode *a = &layout->nodes[layout->edges[i].a], *b = &layout->nodes[layout->edges[i].b];
                TEST_ASSERT(horizontal ? a->x < b->x : a->y < b->y);
            }
        }
        TEST_ASSERT(gd_skill_layout(model, layout, rank_mode, horizontal, GD_SORT_PARTNERS));
    }
    TEST_ASSERT(!gd_sources_changed(model) && !strcmp(model->records[root_node].body, "Root body stays unchanged."));
    TEST_ASSERT(!strcmp(model->records[root_node].status, "rejected") && !model->records[branch].has_rank);
    {
        GD_Model *fresh = model_create();
        TEST_ASSERT(fresh && write_record(0, "zero", "---\nrank: 4\nprerequisites: [atoms/root]\n---\n"));
        TEST_ASSERT(gd_sources_changed(model) && gd_load(fresh, collections, 1));
        TEST_ASSERT(fresh->skills[merge].display_rank == 4 && fresh->records[merge].rank == -1 && fresh->records[join].rank == 2);
        TEST_ASSERT(fresh->skills[join].display_rank == 4 && fresh->skills[late].display_rank == 3 && !fresh->records[branch].has_rank);
        TEST_ASSERT(model->records[zero].rank == 0 && model->skills[join].display_rank == 3);
        gd_destroy(fresh);
        fresh = model_create();
        TEST_ASSERT(fresh && write_record(0, "zero", "---\nrank: 0\nprerequisites: [atoms/root]\n---\n"));
        TEST_ASSERT(gd_load(fresh, collections, 1) && fresh->skills[join].display_rank == 3 && fresh->records[join].rank == 2);
        gd_destroy(fresh);
    }
    gd_destroy(model);
)
TEST(skill_incomplete,
    GD_Model *model = model_create();
    int a, b, tail, self, missing, child, independent, wrong, i, rank_mode, horizontal;
    TEST_ASSERT(model && setup_skills("skill-incomplete"));
    collections[1].role = GD_RELATED; collections[1].alias = "context";
    TEST_ASSERT(write_record(1, "not-a-skill", "Context only."));
    TEST_ASSERT(write_record(0, "a", "---\nrank: 2\nprerequisites: [atoms/b, atoms/independent]\n---\n"));
    TEST_ASSERT(write_record(0, "b", "---\nprerequisites: [atoms/a]\n---\n"));
    TEST_ASSERT(write_record(0, "tail", "---\nrank: 9\nprerequisites: [atoms/a]\n---\n"));
    TEST_ASSERT(write_record(0, "self", "---\nprerequisites: [atoms/self]\n---\n"));
    TEST_ASSERT(write_record(0, "missing", "---\nprerequisites: [atoms/absent]\n---\n"));
    TEST_ASSERT(write_record(0, "child", "---\nprerequisites: [atoms/missing]\n---\n"));
    TEST_ASSERT(write_record(0, "independent", "---\nrank: 0\nmechanics: [context/not-loaded]\n---\n"));
    TEST_ASSERT(write_record(0, "wrong", "---\nprerequisites: [context/not-a-skill]\n---\n"));
    TEST_ASSERT(gd_load(model, collections, 2));
    a = find_record(model, "atoms/a"); b = find_record(model, "atoms/b"); tail = find_record(model, "atoms/tail");
    self = find_record(model, "atoms/self"); missing = find_record(model, "atoms/missing"); child = find_record(model, "atoms/child");
    independent = find_record(model, "atoms/independent"); wrong = find_record(model, "atoms/wrong");
    TEST_ASSERT(model->skills[a].cycle && model->skills[b].cycle && model->skills[self].cycle);
    TEST_ASSERT(!model->skills[tail].cycle && model->skills[tail].incomplete && model->skills[tail].depth == -1);
    TEST_ASSERT(model->skills[missing].incomplete && model->skills[child].incomplete && model->skills[wrong].incomplete);
    TEST_ASSERT(model->skills[missing].declared_prerequisites == 1 && model->skills[missing].prerequisite_count == 0);
    TEST_ASSERT(!model->skills[independent].incomplete && model->skills[independent].depth == 0);
    TEST_ASSERT(!model->skills[a].has_display && !model->skills[tail].has_minimum && model->records[tail].rank == 9);
    TEST_ASSERT(model->skill_link_count == 6 && model->skill_incomplete);
    TEST_ASSERT(has_finding(model, "skill.prerequisite_cycle") && has_finding(model, "skill.placement_unknown"));
    TEST_ASSERT(has_finding(model, "skill.prerequisite_missing") && has_finding(model, "skill.prerequisite_target"));
    TEST_ASSERT(model->skills[missing].layout_layer < model->skills[child].layout_layer);
    TEST_ASSERT(model->skills[a].layout_layer == model->skills[b].layout_layer && model->skills[a].layout_layer < model->skills[tail].layout_layer);
    for (rank_mode = 0; rank_mode < 2; ++rank_mode) for (horizontal = 0; horizontal < 2; ++horizontal) {
        TEST_ASSERT(gd_skill_layout(model, &test_layout, rank_mode, horizontal, GD_SORT_NAME));
        TEST_ASSERT(test_layout.edge_count == model->skill_link_count);
        for (i = 0; i < test_layout.edge_count; ++i) {
            const GD_LayoutNode *p = &test_layout.nodes[test_layout.edges[i].a], *d = &test_layout.nodes[test_layout.edges[i].b];
            const GD_SkillState *before = &model->skills[p->key], *after = &model->skills[d->key];
            if (before->cycle && after->cycle && before->layout_layer == after->layout_layer) continue;
            TEST_ASSERT(horizontal ? p->x < d->x : p->y < d->y);
        }
    }
    gd_destroy(model);
)
TEST(skill_rank_types,
    const struct { const char *value; int valid, ranked; int64_t expected; } samples[] = {
        {"null",1,0,0}, {"!!null 'null'",1,0,0}, {"!!null 'wrong'",0,0,0}, {"!!null \"null\\0hidden\"",0,0,0}, {"-9007199254740991",1,1,-INT64_C(9007199254740991)}, {"0",1,1,0},
        {"9007199254740991",1,1,INT64_C(9007199254740991)}, {"!!int '2'",1,1,2},
        {"!!str null",0,0,0}, {"'2'",0,0,0}, {"true",0,0,0}, {"1.5",0,0,0},
        {"9007199254740992",0,0,0}, {"-9007199254740992",0,0,0}, {"[1]",0,0,0},
        {"{n: 2}",0,0,0}, {"!!float 2",0,0,0}, {"!!int \"2\\0\"",0,0,0}
    };
    int i;
    TEST_ASSERT(setup_skills("skill-rank-types"));
    for (i = 0; i < (int)(sizeof(samples)/sizeof(samples[0])); ++i) {
        GD_Model *model = model_create(); char source[512];
        TEST_ASSERT(model);
        snprintf(source, sizeof(source), "---\nrank: %s\n---\n", samples[i].value);
        TEST_ASSERT(write_record(0, "value", source));
        TEST_ASSERT(gd_load(model, collections, 1) == samples[i].valid);
        if (samples[i].valid) {
            TEST_ASSERT(model->record_count == 1 && model->records[0].has_rank == samples[i].ranked);
            if (samples[i].ranked) TEST_ASSERT(model->records[0].rank == samples[i].expected);
        } else TEST_ASSERT(has_error(model, "skill.rank_type") && model->record_count == 0);
        gd_destroy(model);
    }
    {
        GD_Model *model = model_create();
        TEST_ASSERT(model && write_record(0, "value", "---\nrank: 1\nrank: 2\n---\n"));
        TEST_ASSERT(!gd_load(model, collections, 1) && has_error(model, "input.duplicate_field")); gd_destroy(model);
    }
)
TEST(skill_capacity,
    int phase, i;
    for (phase = 0; phase < 2; ++phase) {
        GD_Model *model = model_create(); clock_t start;
        TEST_ASSERT(model);
        model->record_count = GD_MAX_RECORDS; model->edge_count = GD_MAX_EDGES;
        for (i = 0; i < GD_MAX_RECORDS; ++i) model->records[i].role = GD_SKILL;
        for (i = 0; i < GD_MAX_EDGES; ++i) {
            int child = i < GD_MAX_RECORDS-1 ? i+1 : 1+i%(GD_MAX_RECORDS-1);
            model->edges[i].field = GD_PREREQUISITES; model->edges[i].source = child;
            model->edges[i].target = i < GD_MAX_RECORDS-1 ? i : (i/(GD_MAX_RECORDS-1))%child;
        }
        if (phase) { model->edges[GD_MAX_EDGES-1].source = 0; model->edges[GD_MAX_EDGES-1].target = GD_MAX_RECORDS-1; }
        start = clock(); gd_analyze_skills(model);
        printf("skill capacity probe: 4096 nodes / 32768 declarations / %s: %.1f ms\n", phase ? "cycle" : "chain", 1000.0*(clock()-start)/CLOCKS_PER_SEC);
        if (!phase) TEST_ASSERT(model->skills[GD_MAX_RECORDS-1].depth == GD_MAX_RECORDS-1 && !model->skill_incomplete);
        else {
            for (i = 0; i < GD_MAX_RECORDS; ++i) TEST_ASSERT(model->skills[i].cycle && model->skills[i].incomplete && model->skills[i].depth == -1);
            TEST_ASSERT(model->finding_count == GD_MAX_ERRORS && model->findings_omitted == GD_MAX_RECORDS-GD_MAX_ERRORS);
        }
        gd_destroy(model);
    }
)
static int draft_fixture(const char *name, GD_Model *model) {
    return setup_skills(name) &&
        write_record(0, "root", "---\nrank: 1\n---\nRoot body.") &&
        write_record(0, "child", "---\nrank: null\nprerequisites: [atoms/root, atoms/root]\nmechanics: [rules/keep]\n---\nChild body.") &&
        write_record(0, "leaf", "---\nrank: 2\nprerequisites: [atoms/child]\n---\nLeaf body.") && gd_load(model, collections, 1);
}
TEST(draft_ranks,
    GD_Model *base = model_create(); GD_Draft *draft; int parent, child, leaf;
    TEST_ASSERT(base && draft_fixture("draft-ranks", base));
    parent = find_record(base,"atoms/root"); child = find_record(base,"atoms/child"); leaf = find_record(base,"atoms/leaf");
    draft = gd_draft_create(base); TEST_ASSERT(draft && !draft->modified_count);
    TEST_ASSERT(gd_draft_rank(draft,parent,"4") == 1 && draft->modified_count == 1);
    TEST_ASSERT(draft->model->records[parent].rank == 4 && base->records[parent].rank == 1);
    TEST_ASSERT(!draft->model->records[child].has_rank && draft->model->records[leaf].rank == 2);
    TEST_ASSERT(draft->model->skills[leaf].display_rank == 4 && draft->model->skills[leaf].rank_conflict);
    TEST_ASSERT(!draft->rank_changed[child] && !draft->rank_changed[leaf] && !gd_sources_changed(base));
    TEST_ASSERT(!strcmp(draft->model->records[child].body,"Child body."));
    TEST_ASSERT(gd_draft_rank(draft,parent,"1.5") == -1 && gd_draft_rank(draft,parent,"9007199254740992") == -1);
    TEST_ASSERT(draft->history_count == 1 && gd_draft_rank(draft,parent,"4") == 0);
    TEST_ASSERT(gd_draft_rank(draft,parent,"-9007199254740991") == 1);
    TEST_ASSERT(gd_draft_rank(draft,parent,"0") == 1 && draft->model->skills[leaf].display_rank == 2);
    TEST_ASSERT(gd_draft_rank(draft,parent,"null") == 1 && !draft->model->records[parent].has_rank);
    TEST_ASSERT(gd_draft_rank(draft,parent,"9007199254740991") == 1);
    while (draft->history_count) TEST_ASSERT(gd_draft_undo(draft) == 1);
    TEST_ASSERT(!draft->modified_count && draft->model->records[parent].rank == 1 && !gd_draft_undo(draft));
    TEST_ASSERT(draft->model->finding_count == base->finding_count && !gd_sources_changed(base));
    gd_draft_destroy(draft); gd_destroy(base);
)
TEST(draft_connections,
    GD_Model *base = model_create(); GD_Draft *draft; int parent, child, leaf, count;
    TEST_ASSERT(base && draft_fixture("draft-connections",base));
    parent = find_record(base,"atoms/root"); child = find_record(base,"atoms/child"); leaf = find_record(base,"atoms/leaf");
    draft = gd_draft_create(base); TEST_ASSERT(draft);
    TEST_ASSERT(gd_draft_prerequisite(draft,parent,"atoms/child",1) == 1);
    TEST_ASSERT(draft->model->skills[parent].cycle && draft->model->skills[child].cycle && draft->model->skills[leaf].incomplete);
    TEST_ASSERT(!draft->model->skills[leaf].cycle && draft->modified_count == 1 && !draft->prerequisites_changed[child]);
    TEST_ASSERT(gd_draft_undo(draft) && !draft->model->skill_incomplete && !draft->modified_count);
    TEST_ASSERT(gd_draft_prerequisite(draft,parent,"atoms/root",1) == 1 && draft->model->skills[parent].cycle);
    TEST_ASSERT(gd_draft_undo(draft) && !draft->modified_count);
    TEST_ASSERT(gd_draft_prerequisite(draft,child,"atoms/root",0) == 1 && !draft->model->skills[child].prerequisite_count);
    TEST_ASSERT(gd_draft_prerequisite(draft,child,"atoms/root",1) == 1 && !draft->modified_count);
    TEST_ASSERT(gd_draft_undo(draft) && gd_draft_undo(draft));
    TEST_ASSERT(!memcmp(draft->model->edges,base->edges,base->edge_count*sizeof(base->edges[0])));
    TEST_ASSERT(gd_draft_prerequisite(draft,child,"atoms/unknown",1) == -1 && !draft->modified_count);
    count = base->edge_count; TEST_ASSERT(!gd_sources_changed(base));
    TEST_ASSERT(draft->model->edge_count == count && base->skills[child].declared_prerequisites == 2);
    gd_draft_destroy(draft); gd_destroy(base);
)
TEST(draft_limits,
    GD_Model *base = model_create(); GD_Draft *draft; int parent, child, i;
    TEST_ASSERT(base && draft_fixture("draft-limits",base));
    parent = find_record(base,"atoms/root"); child = find_record(base,"atoms/child");
    draft = gd_draft_create(base); TEST_ASSERT(draft);
    for (i = 0; i < GD_MAX_DRAFT_EDITS; ++i) TEST_ASSERT(gd_draft_rank(draft,parent,i%2 ? "1":"0") == 1);
    TEST_ASSERT(draft->history_count == GD_MAX_DRAFT_EDITS && !draft->modified_count);
    TEST_ASSERT(gd_draft_rank(draft,parent,"4") == -1 && draft->model->records[parent].rank == 1);
    for (i = 0; i < GD_MAX_DRAFT_EDITS; ++i) TEST_ASSERT(gd_draft_undo(draft) == 1);
    TEST_ASSERT(!draft->modified_count && !draft->history_count && !gd_sources_changed(base));
    gd_draft_destroy(draft);
    base->edge_count = GD_MAX_EDGES;
    for (i = 0; i < GD_MAX_EDGES; ++i) base->edges[i] = (GD_Edge){child,parent,GD_PREREQUISITES,base->records[parent].reference,0};
    gd_recompute_skills(base); draft = gd_draft_create(base); TEST_ASSERT(draft);
    TEST_ASSERT(gd_draft_prerequisite(draft,child,"atoms/leaf",1) == -1 && draft->model->edge_count == GD_MAX_EDGES);
    TEST_ASSERT(gd_draft_prerequisite(draft,child,"atoms/root",0) == 1 && !draft->model->edge_count);
    TEST_ASSERT(gd_draft_undo(draft) && draft->model->edge_count == GD_MAX_EDGES && !draft->modified_count);
    gd_draft_destroy(draft); gd_destroy(base);
)
TEST(draft_report,
    GD_Model *base = model_create(); GD_Draft *draft; int parent; char path[GD_MAX_PATH];
    TEST_ASSERT(base && draft_fixture("draft-report",base));
    parent = find_record(base,"atoms/root"); draft = gd_draft_create(base); TEST_ASSERT(draft);
    TEST_ASSERT(gd_draft_rank(draft,parent,"4") == 1);
    TEST_ASSERT(gd_draft_prerequisite(draft,parent,"atoms/leaf",1) == 1);
    TEST_ASSERT(write_record(0,"root","---\nrank: 9\n---\nExternal body."));
    snprintf(path,sizeof(path),"%s/draft-changes.json",root);
    TEST_ASSERT(gd_draft_finish(draft,path) == 0 && draft->modified_count == 1);
    TEST_ASSERT(gd_draft_finish(draft,path) == -1 && draft->modified_count == 1);
    snprintf(path,sizeof(path),"%s/no-folder/report.json",root);
    TEST_ASSERT(gd_draft_finish(draft,path) == -1 && draft->model->records[parent].rank == 4);
    snprintf(path,sizeof(path),"%s/not-a-report.md",root);
    TEST_ASSERT(gd_draft_finish(draft,path) == -1);
    TEST_ASSERT(write_record(0,"root","---\nrank: invalid\n---\nInvalid external state."));
    snprintf(path,sizeof(path),"%s/draft-unavailable.json",root);
    TEST_ASSERT(gd_draft_finish(draft,path) == 2 && draft->model->records[parent].rank == 4);
    TEST_ASSERT(write_record(0,"root","---\nname: renamed\nrank: null\n---\nRenamed external state."));
    snprintf(path,sizeof(path),"%s/draft-identity.json",root);
    TEST_ASSERT(gd_draft_finish(draft,path) == 2);
    TEST_ASSERT(write_record(0,"root","---\nrank: 1\n---\nRoot body."));
    TEST_ASSERT(!gd_sources_changed(base));
    while (draft->history_count) TEST_ASSERT(gd_draft_undo(draft));
    snprintf(path,sizeof(path),"%s/draft-empty.json",root);
    TEST_ASSERT(!draft->modified_count && gd_draft_finish(draft,path) == 0 && !gd_sources_changed(base));
    gd_draft_destroy(draft); gd_destroy(base);
)
static int route_setup(const char *name) {
    if (!setup(name)) return 0;
    collections[0].alias = "atoms"; collections[0].role = GD_SKILL;
    collections[1].alias = "challenges"; collections[1].role = GD_CHALLENGE;
    collections[2].alias = "context"; collections[2].role = GD_RELATED;
    return 1;
}
static GD_RouteCheck *route_check(GD_Model *model, GD_Routes *routes, const char *challenge, const char *skill) {
    int i, c = find_record(model,challenge), s = find_record(model,skill);
    for (i = 0; i < routes->check_count; ++i) if (routes->checks[i].challenge == c && routes->checks[i].skill == s) return &routes->checks[i];
    return NULL;
}
TEST(routes_semantics,
    GD_Model *model = model_create(); GD_Routes *routes; GD_RouteCheck *check; int i, witnessed = 0;
    TEST_ASSERT(model && route_setup("routes-semantics"));
    TEST_ASSERT(write_record(0,"a","---\nprerequisites: [atoms/b]\nrank: -1\n---\nA."));
    TEST_ASSERT(write_record(0,"b","---\nrank: null\n---\nB."));
    TEST_ASSERT(write_record(1,"entry","---\nentry: true\nintroduces: [atoms/b]\nnext: [challenges/practice, challenges/bypass]\n---\nEntry."));
    TEST_ASSERT(write_record(1,"practice","---\npractises: [atoms/a, atoms/a]\nstatus: rejected\nnext: [challenges/demand]\n---\nPractice."));
    TEST_ASSERT(write_record(1,"bypass","---\nrequires: [atoms/a]\nnext: [challenges/demand]\n---\nUse is not teaching."));
    TEST_ASSERT(write_record(1,"demand","---\nintroduces: [atoms/a]\nrequires: [atoms/a, atoms/a]\ntests_mastery: [atoms/a]\nnext: []\n---\nDemand."));
    TEST_ASSERT(write_record(1,"isolated","---\nrequires: [atoms/a]\n---\nNo declared route."));
    TEST_ASSERT(gd_load(model,collections,2)); routes = gd_analyze_routes(model); TEST_ASSERT(routes);
    check = route_check(model,routes,"challenges/demand","atoms/a");
    TEST_ASSERT(check && check->coverage == GD_SUPPORT_SOME && check->direct_requires && check->direct_mastery && check->current_introduction);
    TEST_ASSERT(route_check(model,routes,"challenges/demand","atoms/b")->coverage == GD_SUPPORT_ALL);
    TEST_ASSERT(route_check(model,routes,"challenges/bypass","atoms/a")->coverage == GD_SUPPORT_ABSENT);
    TEST_ASSERT(route_check(model,routes,"challenges/isolated","atoms/a")->coverage == GD_SUPPORT_UNKNOWN);
    for (i = 0; i < routes->witness_count; ++i) {
        GD_RouteWitness *w = &routes->witnesses[i];
        if (&routes->checks[w->check] == check && !w->supported) { TEST_ASSERT(w->length == 3 && w->path[1] == find_record(model,"challenges/bypass")); witnessed = 1; }
    }
    TEST_ASSERT(witnessed && model->edge_count == 14 && routes->check_count == 6 && routes->routes_complete && !gd_sources_changed(model));
    gd_routes_destroy(routes); gd_destroy(model);
)
TEST(routes_types_unknown,
    const char *bad[] = {"'true'","0","null","yes","[true]","!!str true","!!bool \"true\\0\""};
    int i; char text[512]; GD_Model *model; GD_Routes *routes;
    TEST_ASSERT(route_setup("routes-types")); TEST_ASSERT(write_record(0,"a","A."));
    for (i = 0; i < (int)(sizeof(bad)/sizeof(bad[0])); ++i) {
        model = model_create(); snprintf(text,sizeof(text),"---\nentry: %s\n---\nBad.",bad[i]);
        TEST_ASSERT(write_record(1,"entry",text)); TEST_ASSERT(!gd_load(model,collections,2)); gd_destroy(model);
    }
    TEST_ASSERT(write_record(1,"entry","---\nentry: !!bool 'true'\nrequires: [atoms/a]\nnext: [challenges/missing, context/wrong]\nintroduces: [atoms/missing]\n---\nEntry."));
    TEST_ASSERT(write_record(2,"wrong","Wrong role.")); model = model_create(); TEST_ASSERT(gd_load(model,collections,3));
    routes = gd_analyze_routes(model); TEST_ASSERT(routes && !routes->routes_complete && !routes->requirements_complete);
    TEST_ASSERT(routes->checks[0].coverage == GD_SUPPORT_UNKNOWN && routes->checks[0].unsupported_route && !routes->checks[0].supported_route);
    TEST_ASSERT(model->finding_count >= 4 && model->findings[0].line > 0 && !model->error_count);
    gd_routes_destroy(routes); gd_destroy(model);
    TEST_ASSERT(write_record(1,"entry","---\nentry: false\nrequires: [atoms/a]\nnext: null\nintroduces: null\npractises: []\n---\nNo entries."));
    model = model_create(); TEST_ASSERT(gd_load(model,collections,2)); routes = gd_analyze_routes(model);
    TEST_ASSERT(routes && !routes->entry_count && routes->checks[0].coverage == GD_SUPPORT_UNKNOWN && !routes->checks[0].unsupported_route);
    gd_routes_destroy(routes); gd_destroy(model);
    TEST_ASSERT(write_record(1,"entry","---\nentry: true\nentry: false\n---\nDuplicate."));
    model = model_create(); TEST_ASSERT(!gd_load(model,collections,2) && has_error(model,"input.duplicate_field")); gd_destroy(model);
    TEST_ASSERT(write_record(1,"entry","---\nentry: true\nnext: challenges/entry\n---\nScalar route."));
    model = model_create(); TEST_ASSERT(!gd_load(model,collections,2)); gd_destroy(model);
    TEST_ASSERT(write_record(0,"a","---\nprerequisites: [atoms/b]\n---\nA."));
    TEST_ASSERT(write_record(0,"b","---\nprerequisites: [atoms/a, atoms/missing]\n---\nB."));
    TEST_ASSERT(write_record(1,"entry","---\nentry: true\nintroduces: [atoms/a]\nrequires: [atoms/a]\nnext: [challenges/entry]\n---\nRevisit."));
    model = model_create(); TEST_ASSERT(gd_load(model,collections,2)); routes = gd_analyze_routes(model);
    TEST_ASSERT(routes && routes->routes_complete && !routes->requirements_complete && routes->check_count == 2);
    TEST_ASSERT(route_check(model,routes,"challenges/entry","atoms/a")->coverage == GD_SUPPORT_SOME);
    TEST_ASSERT(route_check(model,routes,"challenges/entry","atoms/b")->coverage == GD_SUPPORT_ABSENT);
    TEST_ASSERT(routes->checks[0].supported_route && routes->checks[0].unsupported_route);
    gd_routes_destroy(routes); gd_destroy(model);
)
static void route_edge(GD_Model *model, int source, int target, GD_Field field) {
    GD_Edge *edge = &model->edges[model->edge_count++];
    edge->source = source; edge->target = target; edge->field = field; edge->reference = model->records[target].reference; edge->line = 3;
}
TEST(routes_walk_oracle,
    GD_Model *model = model_create(); unsigned seed = 713; int trial, i, j, k;
    const char *names[] = {"atoms/a","challenges/a","challenges/b","challenges/c","challenges/d"};
    TEST_ASSERT(model); model->record_count = 5; model->route_mode = model->skill_mode = 1;
    for (i = 0; i < 5; ++i) { model->records[i].role = i ? GD_CHALLENGE:GD_SKILL; model->records[i].reference = names[i]; }
    for (trial = 0; trial < 96; ++trial) {
        unsigned char reach[8][8] = {{0}}, support[4], entry[4]; GD_Routes *routes;
        model->edge_count = model->finding_count = model->findings_omitted = 0;
        for (i = 0; i < 4; ++i) {
            seed = seed*1664525u+1013904223u; support[i] = (seed>>28)&1; entry[i] = (seed>>29)&1;
            model->records[i+1].entry = entry[i]; route_edge(model,i+1,0,GD_REQUIRES);
            if (support[i]) route_edge(model,i+1,0,trial&1 ? GD_INTRODUCES:GD_PRACTISES);
        }
        for (i = 0; i < 4; ++i) for (j = 0; j < 4; ++j) {
            seed = seed*1664525u+1013904223u;
            if ((seed>>28)%3 == 0) {
                route_edge(model,i+1,j+1,GD_NEXT);
                reach[2*i][2*j+support[i]] = 1; reach[2*i+1][2*j+1] = 1;
            }
        }
        for (i = 0; i < 8; ++i) reach[i][i] = 1;
        for (k = 0; k < 8; ++k) for (i = 0; i < 8; ++i) for (j = 0; j < 8; ++j) reach[i][j] |= reach[i][k] && reach[k][j];
        routes = gd_analyze_routes(model); TEST_ASSERT(routes && routes->check_count == 4);
        for (i = 0; i < 4; ++i) {
            int yes = 0, no = 0; GD_RouteCheck *check = &routes->checks[i];
            for (j = 0; j < 4; ++j) if (entry[j]) { no |= reach[2*j][2*i]; yes |= reach[2*j][2*i+1]; }
            TEST_ASSERT(check->supported_route == yes && check->unsupported_route == no);
            TEST_ASSERT(check->coverage == (!yes && !no ? GD_SUPPORT_UNKNOWN : yes ? no ? GD_SUPPORT_SOME:GD_SUPPORT_ALL:GD_SUPPORT_ABSENT));
        }
        gd_routes_destroy(routes);
    }
    gd_destroy(model);
)
typedef struct { GD_Allocator parent; int remaining; } RouteFailure;
static void *route_allocate(void *context, size_t size) {
    RouteFailure *failure = context;
    return failure->remaining-- > 0 ? failure->parent.allocate(failure->parent.context,size):NULL;
}
static void route_release(void *context, void *memory) { RouteFailure *failure = context; failure->parent.release(failure->parent.context,memory); }
TEST(routes_limits,
    GD_Model *model = model_create(); GD_Routes *routes; int i; char names[385][32]; GD_Allocator original; RouteFailure failure;
    TEST_ASSERT(model); model->route_mode = model->skill_mode = 1;
    for (i = 0; i < 385; ++i) {
        snprintf(names[i],sizeof(names[i]),i<256 ? "atoms/s%d":"challenges/c%d",i);
        model->records[i].reference = names[i]; model->records[i].role = i<256 ? GD_SKILL:GD_CHALLENGE;
        if (i > 0 && i < 256) route_edge(model,i,i-1,GD_PREREQUISITES);
        if (i >= 256) route_edge(model,i,255,GD_REQUIRES);
    }
    model->records[256].entry = 1; model->record_count = 384; --model->edge_count;
    routes = gd_analyze_routes(model); TEST_ASSERT(routes && routes->check_count == GD_MAX_ROUTE_CHECKS && !model->error_count);
    TEST_ASSERT(routes->witness_count == GD_MAX_ROUTE_WITNESSES && routes->witnesses_omitted > 0 && model->findings_omitted > 0);
    gd_routes_destroy(routes); model->record_count = 385; ++model->edge_count; model->finding_count = model->findings_omitted = 0;
    routes = gd_analyze_routes(model); TEST_ASSERT(routes && has_error(model,"scaffolding.check_limit") && routes->check_count == GD_MAX_ROUTE_CHECKS);
    gd_routes_destroy(routes);
    model->error_count = 0; model->finding_count = model->findings_omitted = 0; model->edge_count = 0; model->record_count = 301;
    model->records[0].role = GD_SKILL;
    for (i = 1; i < 301; ++i) { model->records[i].role = GD_CHALLENGE; model->records[i].entry = i==1; route_edge(model,i,0,GD_REQUIRES); if (i>1) route_edge(model,i-1,i,GD_NEXT); }
    route_edge(model,1,0,GD_INTRODUCES);
    routes = gd_analyze_routes(model); TEST_ASSERT(routes && !model->error_count);
    TEST_ASSERT(routes->witnesses[0].full_length == 300 && routes->witnesses[0].length == GD_MAX_ROUTE_PATH && routes->witnesses[0].path[GD_MAX_ROUTE_PATH-1] == 300);
    gd_routes_destroy(routes);
    original = model->allocator; failure.parent = original; failure.remaining = 0;
    model->allocator.context = &failure; model->allocator.allocate = route_allocate; model->allocator.release = route_release;
    TEST_ASSERT(!gd_analyze_routes(model) && has_error(model,"input.memory"));
    model->error_count = 0; failure.remaining = 1; routes = gd_analyze_routes(model);
    TEST_ASSERT(routes && has_error(model,"input.memory")); gd_routes_destroy(routes);
    model->allocator = original; gd_destroy(model);
)
static int mda_setup(const char *name, GD_Collection *result, char *system_folder) {
    int i;
    if (!setup(name)) return 0;
    collections[0].role = GD_MDA_MECHANIC; collections[1].role = GD_MDA_DYNAMIC; collections[2].role = GD_MDA_AESTHETIC;
    for (i = 0; i < 3; ++i) result[i] = collections[i];
    if (snprintf(system_folder,GD_MAX_PATH,"%s/%s/arrangements",root,name) >= GD_MAX_PATH || !file_directory_create(system_folder,1)) return 0;
    result[3].alias = "arrangements"; result[3].directory = system_folder; result[3].role = GD_MDA_SYSTEM; return 1;
}
static void mda_edge(GD_Model *model, int source, int target, GD_Field field) {
    GD_Edge *edge = &model->edges[model->edge_count++];
    edge->source = source; edge->target = target; edge->reference = model->records[target].reference; edge->field = field; edge->line = model->edge_count;
}
TEST(mda_traces,
    GD_Model *model = model_create(); GD_Mda *mda; GD_Collection inputs[4]; GD_Layout *layout;
    char directory[GD_MAX_PATH], path[GD_MAX_PATH]; int push, intent, i, nodes[GD_MAX_MDA_PATH], links[GD_MAX_MDA_PATH], full, count;
    const char *pack = "---\nstatus: accepted\nmechanics: [things/push]\n---\nConstructed arrangement, not dynamics.";
    TEST_ASSERT(model && mda_setup("mda-traces",inputs,directory));
    TEST_ASSERT(write_record(0,"push","---\nstatus: accepted\nnouns: [unloaded/crate]\ncontrol-vars: [unloaded/force]\n---\nRule, not measured emotion."));
    TEST_ASSERT(write_record(0,"isolated","---\nstatus: rejected\n---\nNo declared association."));
    TEST_ASSERT(write_record(1,"drift","---\nstatus: proposed\nmechanics: [things/push, things/push]\nsystems: [arrangements/pack]\nexperiments: [unloaded/experiment]\n---\nHypothesis only."));
    TEST_ASSERT(write_record(2,"intent","---\nstatus: accepted\ndynamics: [actions/drift]\nexperiments: [unloaded/experiment]\n---\nLocal intent, not whole player story."));
    TEST_ASSERT(write_record(2,"unlinked","---\nstatus: rejected\ndynamics: []\n---\nNo basis declared."));
    snprintf(path,sizeof(path),"%s/pack.md",directory); TEST_ASSERT(file_write_bytes(cstring_from_raw(path),pack,strlen(pack)) == file_error_ok);
    TEST_ASSERT(gd_load(model,inputs,4)); mda = gd_analyze_mda(model);
    TEST_ASSERT(mda && mda->node_count == 6 && mda->link_count == 5 && mda->graph_complete && mda->gap_count == 2);
    push = gd_mda_find(model,"things/push"); intent = gd_mda_find(model,"cases/intent");
    TEST_ASSERT(push >= 0 && intent >= 0 && mda->nodes[push].aesthetic == intent);
    TEST_ASSERT(!strcmp(model->records[gd_mda_find(model,"things/isolated")].status,"rejected"));
    TEST_ASSERT(gd_mda_query(model,mda,push,GD_MDA_FORWARD) && mda->selected_count == 4 && mda->gap_count == 0);
    count = gd_mda_path(mda,intent,nodes,links,GD_MAX_MDA_PATH,&full);
    TEST_ASSERT(count == 3 && full == 3 && nodes[0] == push && nodes[2] == intent);
    for (i = 0; i+1 < count; ++i) {
        const GD_Edge *edge = &model->edges[mda->links[links[i]].edge];
        TEST_ASSERT(edge->source == nodes[i+1] && edge->target == nodes[i]);
    }
    snprintf(path,sizeof(path),"%s/mda-forward.json",root);
    { FILE *output = fopen(path,"wb"); TEST_ASSERT(output && gd_write_mda_result(output,model,mda,1,NULL)); TEST_ASSERT(!fclose(output)); }
    TEST_ASSERT(gd_mda_query(model,mda,intent,GD_MDA_BACKWARD) && mda->selected_count == 4);
    TEST_ASSERT(gd_mda_query(model,mda,push,GD_MDA_BACKWARD) && mda->selected_count == 1);
    TEST_ASSERT(gd_mda_query(model,mda,push,GD_MDA_BOTH) && mda->selected_count == 4);
    layout = model->allocator.allocate(model->allocator.context,sizeof(*layout));
    TEST_ASSERT(layout && gd_mda_layout(model,mda,layout,1) && layout->node_count == 6 && layout->edge_count == 5);
    for (i = 0; i < layout->edge_count; ++i) {
        const GD_Edge *edge = &model->edges[mda->links[layout->edges[i].key].edge];
        TEST_ASSERT(layout->nodes[layout->edges[i].a].key == edge->source && layout->nodes[layout->edges[i].b].key == edge->target);
        TEST_ASSERT(layout->edges[i].relation==(int)edge->field);
        TEST_ASSERT(layout->nodes[layout->edges[i].a].layer > layout->nodes[layout->edges[i].b].layer);
    }
    TEST_ASSERT(gd_mda_layout(model,mda,layout,0) && layout->mode == GD_LAYOUT_FORCE && layout->node_count == 6);
    TEST_ASSERT(!gd_sources_changed(model) && !strcmp(model->records[gd_mda_find(model,"actions/drift")].body,"Hypothesis only."));
    model->allocator.release(model->allocator.context,layout); gd_mda_destroy(mda); gd_destroy(model);
)
TEST(mda_resolution,
    GD_Model *model; GD_Mda *mda; GD_Collection inputs[4]; char directory[GD_MAX_PATH]; int i, unloaded = 0, missing = 0, wrong = 0;
    TEST_ASSERT(mda_setup("mda-resolution",inputs,directory));
    TEST_ASSERT(write_record(0,"push","Rule."));
    TEST_ASSERT(write_record(0,"other","Other rule."));
    TEST_ASSERT(write_record(1,"drift","---\nmechanics: [things/push, things/missing, cases/intent]\nsystems: [arrangements/pack]\n---\nDeclared but incomplete."));
    TEST_ASSERT(write_record(2,"intent","---\ndynamics: [actions/drift]\n---\nIntent."));
    model = model_create(); TEST_ASSERT(model && gd_load(model,inputs,3)); mda = gd_analyze_mda(model);
    TEST_ASSERT(mda && !mda->graph_complete && !model->error_count);
    for (i = 0; i < mda->link_count; ++i) { unloaded += mda->links[i].resolution == GD_MDA_UNLOADED; missing += mda->links[i].resolution == GD_MDA_MISSING; wrong += mda->links[i].resolution == GD_MDA_WRONG_ROLE; }
    TEST_ASSERT(unloaded == 1 && missing == 1 && wrong == 1);
    TEST_ASSERT(!strcmp(gd_mda_trace(mda,gd_mda_find(model,"things/other")),"unknown"));
    TEST_ASSERT(!strcmp(gd_mda_trace(mda,gd_mda_find(model,"things/push")),"declared_trace"));
    TEST_ASSERT(gd_mda_query(model,mda,gd_mda_find(model,"things/other"),GD_MDA_BOTH) && mda->selected_count == 1 && mda->finding_count == 1);
    TEST_ASSERT(!strcmp(mda->findings[0].code,"mda.trace_unknown"));
    TEST_ASSERT(!gd_mda_query(model,mda,-2,GD_MDA_BOTH) && !mda->selected_count && !mda->query_complete);
    gd_mda_destroy(mda); gd_destroy(model);
    TEST_ASSERT(write_record(0,"duplicate","---\nname: push\n---\nAmbiguous identity."));
    model = model_create(); TEST_ASSERT(model && !gd_load(model,inputs,3) && has_error(model,"input.identity"));
    mda = gd_analyze_mda(model); TEST_ASSERT(mda && mda->node_count == 5 && gd_mda_find(model,"things/push") == -2);
    for (i = 0; i < mda->link_count; ++i) {
        const char *reference = model->edges[mda->links[i].edge].reference;
        if (!strcmp(reference,"things/push")) TEST_ASSERT(mda->links[i].resolution == GD_MDA_AMBIGUOUS);
        if (!strcmp(reference,"things/missing")) TEST_ASSERT(mda->links[i].resolution == GD_MDA_UNAVAILABLE);
    }
    TEST_ASSERT(!mda->graph_complete && mda->selected_count == 5);
    gd_mda_destroy(mda); gd_destroy(model);
)
TEST(mda_types,
    GD_Model *model; GD_Mda *mda; GD_Collection inputs[4]; char directory[GD_MAX_PATH]; int i;
    const char *bad[] = {"mechanics: scalar", "systems: 7", "experiments: [false]", "mechanics: []\nmechanics: []", "systems: {bad: shape}", "systems: [!!str \"escaped\\0nul\"]"};
    TEST_ASSERT(mda_setup("mda-types",inputs,directory));
    TEST_ASSERT(write_record(0,"rule","Rule.")); TEST_ASSERT(write_record(2,"intent","---\ndynamics: null\n---\nNo basis declared."));
    for (i = 0; i < 6; ++i) {
        char text[256]; snprintf(text,sizeof(text),"---\n%s\n---\nInvalid typed field.",bad[i]);
        TEST_ASSERT(write_record(1,"broken",text)); model = model_create();
        TEST_ASSERT(model && !gd_load(model,inputs,3) && model->error_count);
        mda = gd_analyze_mda(model); TEST_ASSERT(mda && !mda->graph_complete && mda->node_count == 2 && mda->selected_count == 2);
        gd_mda_destroy(mda); gd_destroy(model);
    }
    TEST_ASSERT(write_record(1,"broken","---\nmechanics: null\nsystems: []\nexperiments: null\n---\nEmpty declarations are not parse errors."));
    model = model_create(); TEST_ASSERT(model && gd_load(model,inputs,3)); mda = gd_analyze_mda(model);
    TEST_ASSERT(mda && mda->graph_complete && mda->node_count == 3 && !mda->link_count && mda->gap_count == 3);
    gd_mda_destroy(mda); gd_destroy(model);
)
TEST(mda_oracle,
    GD_Model *model = model_create(); unsigned random = 7813; int trial, i, j, k, direction, root_index;
    char names[12][24];
    TEST_ASSERT(model); model->mda_mode = 1; model->record_count = 12;
    for (i = 0; i < 12; ++i) {
        snprintf(names[i],sizeof(names[i]),"test/r%d",i); model->records[i].reference = names[i]; model->records[i].name = names[i]+5;
        model->records[i].role = i < 3 ? GD_MDA_MECHANIC:i < 5 ? GD_MDA_SYSTEM:i < 9 ? GD_MDA_DYNAMIC:GD_MDA_AESTHETIC;
    }
    for (trial = 0; trial < 64; ++trial) {
        GD_Mda *mda; int authored[12][12] = {{0}};
        model->edge_count = 0;
        for (i = 0; i < 12; ++i) for (j = 0; j < 12; ++j) {
            GD_Role a = model->records[i].role, b = model->records[j].role;
            int allowed = (a == GD_MDA_SYSTEM && b == GD_MDA_MECHANIC) || (a == GD_MDA_DYNAMIC && (b == GD_MDA_MECHANIC || b == GD_MDA_SYSTEM)) || (a == GD_MDA_AESTHETIC && b == GD_MDA_DYNAMIC);
            random = random*1664525u+1013904223u;
            if (allowed && ((random>>24)&3) == 0) {
                mda_edge(model,i,j,b == GD_MDA_MECHANIC ? GD_MECHANICS:b == GD_MDA_SYSTEM ? GD_SYSTEMS:GD_DYNAMICS); authored[i][j] = 1;
                if ((random&7) == 0) mda_edge(model,i,j,b == GD_MDA_MECHANIC ? GD_MECHANICS:b == GD_MDA_SYSTEM ? GD_SYSTEMS:GD_DYNAMICS);
            }
        }
        mda = gd_analyze_mda(model); TEST_ASSERT(mda && mda->graph_complete);
        for (direction = 0; direction < 3; ++direction) {
            int distance[12][12];
            for (i = 0; i < 12; ++i) for (j = 0; j < 12; ++j) distance[i][j] = i == j ? 0:((direction != GD_MDA_FORWARD && authored[i][j]) || (direction != GD_MDA_BACKWARD && authored[j][i])) ? 1:13;
            for (k = 0; k < 12; ++k) for (i = 0; i < 12; ++i) for (j = 0; j < 12; ++j)
                if (distance[i][k]+distance[k][j] < distance[i][j]) distance[i][j] = distance[i][k]+distance[k][j];
            for (root_index = 0; root_index < 12; ++root_index) {
                TEST_ASSERT(gd_mda_query(model,mda,root_index,(GD_MdaDirection)direction));
                for (i = 0; i < 12; ++i) {
                    int nodes[GD_MAX_MDA_PATH], links[GD_MAX_MDA_PATH], full, length;
                    TEST_ASSERT(mda->selected[i] == (distance[root_index][i] < 13));
                    length = gd_mda_path(mda,i,nodes,links,GD_MAX_MDA_PATH,&full);
                    TEST_ASSERT(full == (distance[root_index][i] < 13 ? distance[root_index][i]+1:0) && length == full);
                    if (!length) continue;
                    TEST_ASSERT(nodes[0] == root_index && nodes[length-1] == i);
                    for (j = 0; j+1 < length; ++j) {
                        const GD_Edge *edge = &model->edges[mda->links[links[j]].edge];
                        TEST_ASSERT((direction != GD_MDA_FORWARD && edge->source == nodes[j] && edge->target == nodes[j+1]) || (direction != GD_MDA_BACKWARD && edge->target == nodes[j] && edge->source == nodes[j+1]));
                    }
                }
            }
        }
        for (k = 0; k < 12; ++k) for (i = 0; i < 12; ++i) for (j = 0; j < 12; ++j) authored[i][j] |= authored[i][k] && authored[k][j];
        for (i = 0; i < 12; ++i) {
            int has_aesthetic = model->records[i].role == GD_MDA_AESTHETIC;
            for (j = 9; j < 12; ++j) has_aesthetic |= authored[j][i];
            TEST_ASSERT((mda->nodes[i].aesthetic >= 0) == has_aesthetic);
        }
        gd_mda_destroy(mda);
    }
    gd_destroy(model);
)
TEST(mda_limits,
    GD_Model *model = model_create(); GD_Mda *mda; GD_Allocator original; RouteFailure failure;
    static char names[GD_MAX_RECORDS][24]; int i, nodes[GD_MAX_MDA_PATH], links[GD_MAX_MDA_PATH], full, count;
    TEST_ASSERT(model); model->mda_mode = 1; model->record_count = GD_MAX_RECORDS;
    for (i = 0; i < GD_MAX_RECORDS; ++i) {
        snprintf(names[i],sizeof(names[i]),"test/r%d",i); model->records[i].reference = names[i]; model->records[i].name = names[i]+5;
        model->records[i].role = i%2 ? GD_MDA_DYNAMIC:GD_MDA_MECHANIC;
    }
    for (i = 1; i < GD_MAX_RECORDS; i += 2) { mda_edge(model,i,i-1,GD_MECHANICS); if (i+1 < GD_MAX_RECORDS) mda_edge(model,i,i+1,GD_MECHANICS); }
    while (model->edge_count < GD_MAX_EDGES) mda_edge(model,1,0,GD_MECHANICS);
    mda = gd_analyze_mda(model); TEST_ASSERT(mda && mda->node_count == GD_MAX_RECORDS && mda->link_count == GD_MAX_EDGES);
    TEST_ASSERT(mda->gap_count == GD_MAX_RECORDS && mda->finding_count == GD_MAX_ERRORS && mda->findings_omitted == GD_MAX_RECORDS-GD_MAX_ERRORS);
    TEST_ASSERT(gd_mda_query(model,mda,0,GD_MDA_BOTH) && mda->selected_count == GD_MAX_RECORDS);
    count = gd_mda_path(mda,GD_MAX_RECORDS-1,nodes,links,GD_MAX_MDA_PATH,&full);
    TEST_ASSERT(full == GD_MAX_RECORDS && count == GD_MAX_MDA_PATH && nodes[0] == GD_MAX_RECORDS-GD_MAX_MDA_PATH && nodes[count-1] == GD_MAX_RECORDS-1);
    TEST_ASSERT(gd_mda_query(model,mda,0,GD_MDA_FORWARD) && mda->selected_count == 2 && !mda->findings_omitted);
    original = model->allocator; failure.parent = original; failure.remaining = 0;
    model->allocator.context = &failure; model->allocator.allocate = route_allocate; model->allocator.release = route_release;
    TEST_ASSERT(!gd_analyze_mda(model) && has_error(model,"input.memory"));
    model->allocator = original; gd_mda_destroy(mda); gd_destroy(model);
)
static void control_definition(GD_Model *model, int index, const char *reference, const char *type, const char *value, const char *expression, const char *unit) {
    GD_Record *record=&model->records[index];
    memset(record,0,sizeof(*record)); record->role=GD_CONTROL; record->reference=reference;
    record->name=strchr(reference,'/')+1; record->source_path=reference;
    record->control.type=type; record->control.value=value; record->control.expression=expression; record->control.unit=unit;
    record->control.kind=value ? GD_LITERAL_NUMBER:GD_LITERAL_NULL;
}
TEST(control_units_numbers,
    GD_Unit unit; double number;
    TEST_ASSERT(gd_control_unit("m/s",3,&unit) && unit.count==2 && unit.terms[1].power==-1);
    TEST_ASSERT(gd_control_unit("m*s^-1",6,&unit) && unit.count==2 && unit.terms[1].power==-1);
    TEST_ASSERT(gd_control_unit("energy/energy",13,&unit) && !unit.count);
    TEST_ASSERT(gd_control_unit("1",1,&unit) && !unit.count);
    TEST_ASSERT(!gd_control_unit("",0,&unit) && !gd_control_unit("m/",2,&unit) && !gd_control_unit("m^33",4,&unit));
    TEST_ASSERT(!gd_control_unit("m/(s*s)",7,&unit) && !gd_control_unit("m m",3,&unit));
    TEST_ASSERT(gd_control_number("-9007199254740991",&number,1));
    TEST_ASSERT(!gd_control_number("9007199254740992",&number,1));
    TEST_ASSERT(!gd_control_number("9007199254740991.1",&number,1));
    TEST_ASSERT(!gd_control_number("1.00000000000000000001",&number,1));
    TEST_ASSERT(!gd_control_number("1e-999",&number,0) && !gd_control_number("nan",&number,0));
    TEST_ASSERT(gd_control_number("1.250e2",&number,1) && number==125);
    TEST_ASSERT(!gd_control_number(".1",&number,1) && !gd_control_number("1.2.3",&number,0));
)
TEST(control_formulas,
    GD_Model *model=model_create(); GD_Controls *controls; int i;
    const char *expressions[]={"1+2*3","(1+2)*3","--2 + -3","min(3, 2)","max(-3,-2)","clamp(8,2,4)","abs(-3)","floor(-2.1)","ceil(-2.1)","round(-2.5)","round(2.5)","round(0.49999999999999994)","1e2 + .5", "-(min(2,max(3,4)))"};
    double expected[]={7,9,-1,2,-2,4,3,-3,-2,-3,3,0,100.5,-2};
    TEST_ASSERT(model); model->record_count=1; model->control_mode=1;
    control_definition(model,0,"config/value","number",NULL,NULL,"1");
    for (i=0;i<14;++i) {
        model->records[0].control.expression=expressions[i]; model->error_count=model->errors_omitted=0;
        controls=gd_resolve_controls(model,-1,NULL,0);
        TEST_ASSERT(controls && controls->complete && controls->values[0].number==expected[i]); gd_controls_destroy(controls);
    }
    model->records[0].control.expression="1[s] + 0.2[s]"; model->records[0].control.unit="s";
    controls=gd_resolve_controls(model,-1,NULL,0); TEST_ASSERT(controls && controls->complete && fabs(controls->values[0].number-1.2)<1e-15); gd_controls_destroy(controls);
    gd_destroy(model);
)
TEST(control_reference_syntax,
    GD_Model *model=model_create(); GD_Controls *controls; int i;
    const char *invalid[]={"${config/a}","{config/a} + ${config/a}","$ {config/a}","{}","{config/a","{config/a}}","{{config/a}}"};
    TEST_ASSERT(model); model->record_count=2;
    control_definition(model,0,"config/a","number","2",NULL,"s");
    control_definition(model,1,"config/b","number",NULL,"({config/a} + {config/a}) / 2","s");
    model->records[1].control.lines[2]=17;
    controls=gd_resolve_controls(model,-1,NULL,0);
    TEST_ASSERT(controls && controls->complete && controls->values[1].number==2 && controls->tokens[0].offset==1);
    gd_controls_destroy(controls);
    for (i=0;i<7;++i) {
        model->error_count=model->errors_omitted=0; model->records[1].control.expression=invalid[i];
        controls=gd_resolve_controls(model,-1,NULL,0);
        TEST_ASSERT(controls && !controls->complete && model->error_count);
        if (i<2) {
            TEST_ASSERT(has_error(model,"controls.reference_syntax") && model->errors[0].line==17);
            TEST_ASSERT(!strcmp(model->errors[0].path,"config/b") && strstr(model->errors[0].message,"{ALIAS/NAME}"));
        }
        gd_controls_destroy(controls);
    }
    model->error_count=model->errors_omitted=0; model->records[1].control.expression="{config/missing}";
    controls=gd_resolve_controls(model,-1,NULL,0);
    TEST_ASSERT(controls && !controls->complete && has_error(model,"controls.reference"));
    gd_controls_destroy(controls); gd_destroy(model);
)
TEST(control_errors,
    GD_Model *model=model_create(); GD_Controls *controls; int i;
    const char *invalid[]={"1/0","clamp(1,3,2)","1[s]+2[ms]","min(1,2,3)","pow(2,3)","1;system(2)","()","min()","1+","1e309","2[bad/]","1,2","(1,2)","1(2)","1[s]+1","1e-999"};
    TEST_ASSERT(model); model->record_count=1; control_definition(model,0,"config/value","number",NULL,NULL,"1");
    for (i=0;i<16;++i) {
        model->error_count=model->errors_omitted=0; model->records[0].control.expression=invalid[i];
        controls=gd_resolve_controls(model,-1,NULL,0); TEST_ASSERT(controls && !controls->complete && model->error_count); gd_controls_destroy(controls);
    }
    model->error_count=0; model->records[0].control.type="integer"; model->records[0].control.expression="3 / 2";
    controls=gd_resolve_controls(model,-1,NULL,0); TEST_ASSERT(controls && !controls->complete && has_error(model,"controls.type")); gd_controls_destroy(controls);
    model->error_count=0; model->records[0].control.expression="round(3/2)";
    controls=gd_resolve_controls(model,-1,NULL,0); TEST_ASSERT(controls && controls->complete && controls->values[0].number==2); gd_controls_destroy(controls);
    model->error_count=0; model->records[0].control.expression="9007199254740991+1-1";
    controls=gd_resolve_controls(model,-1,NULL,0); TEST_ASSERT(controls && !controls->complete); gd_controls_destroy(controls);
    model->error_count=0; model->records[0].control.expression=NULL; model->records[0].control.value="1";
    model->records[0].control.kind=GD_LITERAL_NUMBER; model->records[0].control.minimum="2";
    controls=gd_resolve_controls(model,-1,NULL,0); TEST_ASSERT(controls && !controls->complete && has_error(model,"controls.range")); gd_controls_destroy(controls);
    model->error_count=0; model->records[0].control.minimum=NULL; model->records[0].control.type="boolean";
    controls=gd_resolve_controls(model,-1,NULL,0); TEST_ASSERT(controls && !controls->complete && has_error(model,"controls.type")); gd_controls_destroy(controls);
    gd_destroy(model);
)
TEST(control_loading,
    GD_Model *model=model_create(); GD_Controls *controls; int flag,mode;
    TEST_ASSERT(model && setup("control-loading")); collections[0].role=GD_CONTROL; collections[0].alias="config";
    TEST_ASSERT(write_record(0,"a","---\nvalue_type: number\nvalue: 2\nunit: m/s\n---\n"));
    TEST_ASSERT(write_record(0,"b","---\nvalue_type: number\nvalue: null\nexpression: '1[m] / {config/a}'\nunit: s\n---\n"));
    TEST_ASSERT(write_record(0,"flag","---\nvalue_type: boolean\nvalue: false\n---\n"));
    TEST_ASSERT(write_record(0,"mode","---\nvalue_type: enum\nvalue: '=value'\nallowed_values: ['=value', 'other', '']\n---\n"));
    TEST_ASSERT(gd_load(model,collections,1)); controls=gd_resolve_controls(model,-1,NULL,0);
    flag=gd_control_find(model,"config/flag"); mode=gd_control_find(model,"config/mode");
    TEST_ASSERT(controls && controls->complete && controls->resolved_count==4 && controls->values[gd_control_find(model,"config/b")].number==0.5);
    TEST_ASSERT(controls->values[flag].kind==GD_LITERAL_BOOLEAN && !controls->values[flag].number && !strcmp(controls->values[mode].string,"=value"));
    TEST_ASSERT(model->records[mode].control.allowed_count==3 && !gd_sources_changed(model));
    gd_controls_destroy(controls); gd_destroy(model);
    model=model_create(); TEST_ASSERT(model && setup("control-null-enum")); collections[0].role=GD_CONTROL;
    TEST_ASSERT(write_record(0,"bad","---\nvalue_type: enum\nvalue: x\nallowed_values: [null]\n---\n"));
    TEST_ASSERT(!gd_load(model,collections,1) && has_error(model,"controls.enum_value")); gd_destroy(model);
    model=model_create(); TEST_ASSERT(model && setup("control-bad-metadata")); collections[0].role=GD_CONTROL;
    TEST_ASSERT(write_record(0,"duplicate","---\nvalue: 2\nvalue: 3\n---\n"));
    TEST_ASSERT(write_record(0,"unit","---\nunit: []\n---\n"));
    TEST_ASSERT(write_record(0,"enum","---\nallowed_values: [2]\n---\n"));
    TEST_ASSERT(write_record(0,"bounds","---\nminimum: false\n---\n"));
    TEST_ASSERT(write_record(0,"alias","---\nvalue: &number 2\nunit: *number\n---\n"));
    TEST_ASSERT(write_record(0,"finite","---\nvalue: .inf\n---\n"));
    TEST_ASSERT(write_record(0,"expression","---\nexpression: [1]\n---\n"));
    TEST_ASSERT(!gd_load(model,collections,1) && !model->record_count && model->error_count>=7 && has_error(model,"input.duplicate_field")); gd_destroy(model);
)
TEST(control_overrides_scope,
    GD_Model *model=model_create(); GD_Controls *controls; GD_ControlOverride override={"config/a","4"};
    TEST_ASSERT(model); model->record_count=3;
    control_definition(model,0,"config/a","number","2",NULL,"m/s");
    control_definition(model,1,"config/b","number",NULL,"1[m]/{config/a}","s");
    control_definition(model,2,"config/unknown","unspecified",NULL,NULL,NULL);
    controls=gd_resolve_controls(model,1,&override,1);
    TEST_ASSERT(controls && controls->complete && controls->selected_count==2 && controls->values[1].number==0.25);
    TEST_ASSERT(controls->overridden[0] && !strcmp(model->records[0].control.value,"2") && !controls->selected[2]); gd_controls_destroy(controls);
    override.reference="config/b"; override.value="0.75";
    controls=gd_resolve_controls(model,1,&override,1);
    TEST_ASSERT(controls && controls->complete && controls->selected_count==1 && controls->values[1].number==0.75 && !strcmp(model->records[1].control.expression,"1[m]/{config/a}")); gd_controls_destroy(controls);
    controls=gd_resolve_controls(model,-1,NULL,0); TEST_ASSERT(controls && !controls->complete && controls->resolved_count==2); gd_controls_destroy(controls); gd_destroy(model);
)
TEST(control_selection,
    GD_Model *model=model_create(); GD_Controls *controls; GD_ControlFilter filter={0}; GD_ControlSelection selection;
    int a,b,c,d;
    TEST_ASSERT(model && setup("control-selection")); collections[0].role=GD_CONTROL; collections[0].alias="config";
    TEST_ASSERT(write_record(0,"a","---\nstatus: accepted\nvalue_type: number\nvalue: 2\nunit: m\n---\n"));
    TEST_ASSERT(write_record(0,"b","---\nstatus: proposed\nvalue_type: number\nvalue: 3\nunit: m\n---\n"));
    TEST_ASSERT(write_record(0,"c","---\nstatus: accepted\nvalue_type: number\nexpression: '{config/a} + {config/b}'\nunit: m\n---\n"));
    TEST_ASSERT(write_record(0,"d","---\nstatus: rejected\nvalue_type: boolean\nvalue: false\n---\n"));
    TEST_ASSERT(gd_load(model,collections,1));
    a=gd_control_find(model,"config/a"); b=gd_control_find(model,"config/b"); c=gd_control_find(model,"config/c"); d=gd_control_find(model,"config/d");
    filter.root=-1; filter.accepted_default=1;
    TEST_ASSERT(gd_select_controls(model,&filter,&selection) && selection.roots[a] && selection.roots[c] && !selection.roots[b] && !selection.allowed[d]);
    controls=gd_resolve_control_set(model,&selection,NULL,0);
    TEST_ASSERT(controls && !controls->complete && has_error(model,"controls.status")); gd_controls_destroy(controls); model->error_count=0;
    filter.status_count=1; filter.statuses[0]="all";
    TEST_ASSERT(gd_select_controls(model,&filter,&selection)); controls=gd_resolve_control_set(model,&selection,NULL,0);
    TEST_ASSERT(controls && controls->complete && controls->selected_count==4 && controls->values[c].number==5); gd_controls_destroy(controls);
    filter.status_count=0; filter.include_count=1; filter.include[0]="c";
    TEST_ASSERT(gd_select_controls(model,&filter,&selection)); controls=gd_resolve_control_set(model,&selection,NULL,0);
    TEST_ASSERT(controls && controls->complete && controls->selected_count==3 && controls->requested[c] && !controls->requested[a] && !controls->requested[b]); gd_controls_destroy(controls);
    filter.exclude_count=1; filter.exclude[0]="config/c";
    TEST_ASSERT(gd_select_controls(model,&filter,&selection)); controls=gd_resolve_control_set(model,&selection,NULL,0);
    TEST_ASSERT(controls && controls->complete && !controls->selected_count); gd_controls_destroy(controls);
    filter.exclude_count=0; filter.include[0]="b"; filter.status_count=1; filter.statuses[0]="accepted";
    TEST_ASSERT(gd_select_controls(model,&filter,&selection) && !selection.roots[b]);
    filter.include[0]="missing"; TEST_ASSERT(!gd_select_controls(model,&filter,&selection) && has_error(model,"controls.selection"));
    gd_destroy(model);
)
TEST(control_limits,
    GD_Model *model=model_create(); GD_Controls *controls; GD_Allocator original; RouteFailure failure;
    static char names[GD_MAX_RECORDS][32],expressions[GD_MAX_RECORDS][64]; int i;
    TEST_ASSERT(model); model->record_count=1;
    {
        char too_many[601];
        for (i=0;i<599;++i) too_many[i]=i%2 ? '+':'1'; too_many[599]=0;
        control_definition(model,0,"config/value","number",NULL,too_many,"1");
        controls=gd_resolve_controls(model,-1,NULL,0);
        TEST_ASSERT(controls && !controls->complete && has_error(model,"controls.expression_limit")); gd_controls_destroy(controls);
        model->error_count=model->errors_omitted=0;
    }
    model->record_count=GD_MAX_RECORDS;
    for (i=0;i<GD_MAX_RECORDS;++i) {
        snprintf(names[i],sizeof(names[i]),"config/n%04d",i);
        if (i) snprintf(expressions[i],sizeof(expressions[i]),"{config/n%04d}+1",i-1);
        control_definition(model,i,names[i],"integer",i ? NULL:"0",i ? expressions[i]:NULL,"1");
    }
    controls=gd_resolve_controls(model,GD_MAX_RECORDS-1,NULL,0);
    TEST_ASSERT(controls && controls->complete && controls->selected_count==GD_MAX_RECORDS && controls->values[GD_MAX_RECORDS-1].number==GD_MAX_RECORDS-1); gd_controls_destroy(controls);
    model->records[0].control.value=NULL; model->records[0].control.kind=GD_LITERAL_NULL; model->records[0].control.expression="{config/n4095}";
    controls=gd_resolve_controls(model,-1,NULL,0);
    TEST_ASSERT(controls && !controls->complete && !controls->resolved_count && has_error(model,"controls.cycle") && model->errors_omitted==GD_MAX_RECORDS-GD_MAX_ERRORS); gd_controls_destroy(controls);
    model->error_count=model->errors_omitted=0; original=model->allocator; failure.parent=original; failure.remaining=0;
    model->allocator.context=&failure; model->allocator.allocate=route_allocate; model->allocator.release=route_release;
    TEST_ASSERT(!gd_resolve_controls(model,-1,NULL,0) && has_error(model,"input.memory")); model->allocator=original; gd_destroy(model);
)
TEST(skill_progression_modes,
    GD_Model *base=model_create(); GD_Draft *draft; GD_Routes *routes; GD_Layout *layout;
    GD_Focus focus;
    int a,b,demand,entry,isolated,i;
    TEST_ASSERT(base && route_setup("explorer-modes"));
    TEST_ASSERT(write_record(0,"a","---\nrank: 0\n---\nA."));
    TEST_ASSERT(write_record(0,"b","---\nrank: null\n---\nB."));
    TEST_ASSERT(write_record(1,"entry","---\nentry: true\nintroduces: [atoms/a]\nnext: [challenges/demand, challenges/demand]\n---\nEntry."));
    TEST_ASSERT(write_record(1,"demand","---\nrequires: [atoms/a]\nnext: [challenges/entry, challenges/demand]\n---\nDemand."));
    TEST_ASSERT(write_record(1,"isolated","---\nnext: [atoms/a, challenges/missing]\n---\nIsolated."));
    TEST_ASSERT(gd_load(base,collections,2)); draft=gd_draft_create(base); TEST_ASSERT(draft && draft->model->route_mode);
    layout=base->allocator.allocate(base->allocator.context,sizeof(*layout)); TEST_ASSERT(layout);
    a=find_record(base,"atoms/a"); b=find_record(base,"atoms/b"); demand=find_record(base,"challenges/demand"); entry=find_record(base,"challenges/entry"); isolated=find_record(base,"challenges/isolated");
    routes=gd_analyze_routes(draft->model); TEST_ASSERT(routes && route_check(draft->model,routes,"challenges/demand","atoms/a")->coverage==GD_SUPPORT_UNKNOWN);
    gd_routes_destroy(routes);
    TEST_ASSERT(gd_draft_prerequisite(draft,a,"atoms/b",1)==1 && gd_draft_rank(draft,a,"-2")==1);
    TEST_ASSERT(gd_focus_reset(draft->model,&focus,a));
    for (i=0;i<6;++i) {
        TEST_ASSERT(gd_focus_layout(draft->model,&focus,layout,GD_SORT_NAME) && focus.visible[b]);
        TEST_ASSERT(gd_skill_layout(draft->model,layout,i%2,i%2,GD_SORT_NAME) && layout->node_count==2);
        { int edge; for (edge=0;edge<layout->edge_count;++edge) TEST_ASSERT(layout->edges[edge].relation==GD_PREREQUISITES); }
        TEST_ASSERT(gd_route_layout(draft->model,layout,i%2,GD_SORT_PARTNERS) && layout->node_count==3 && layout->edge_count==4);
        { int edge; for (edge=0;edge<layout->edge_count;++edge) TEST_ASSERT(layout->edges[edge].relation==GD_NEXT); }
        TEST_ASSERT(layout->nodes[0].key>=0 && draft->modified_count==1 && draft->history_count==2 && draft->model->records[a].rank==-2);
    }
    for (i=0;i<layout->node_count;++i) {
        if (layout->nodes[i].key==entry) TEST_ASSERT(layout->nodes[i].layer==0);
        if (layout->nodes[i].key==demand) TEST_ASSERT(layout->nodes[i].layer==1);
        if (layout->nodes[i].key==isolated) TEST_ASSERT(layout->nodes[i].layer==-1);
    }
    routes=gd_analyze_routes(draft->model);
    TEST_ASSERT(routes && route_check(draft->model,routes,"challenges/demand","atoms/b") && routes->checks[1].skill>=0);
    TEST_ASSERT(base->records[a].rank==0 && !draft->model->records[b].has_rank && !gd_sources_changed(base));
    gd_routes_destroy(routes);
    TEST_ASSERT(gd_draft_undo(draft) && gd_draft_undo(draft) && draft->modified_count==0);
    TEST_ASSERT(gd_focus_layout(draft->model,&focus,layout,GD_SORT_NAME) && focus.visible[b]);
    for (i=0;i<layout->edge_count;++i) TEST_ASSERT(draft->model->edges[layout->edges[i].key].field!=GD_PREREQUISITES);
    TEST_ASSERT(gd_route_layout(draft->model,layout,0,GD_SORT_NAME));
    base->allocator.release(base->allocator.context,layout); gd_draft_destroy(draft); gd_destroy(base);
)
TEST(route_layout_capacity,
    GD_Model *model=model_create(); GD_Layout *layout; int i;
    TEST_ASSERT(model); layout=model->allocator.allocate(model->allocator.context,sizeof(*layout)); TEST_ASSERT(layout);
    TEST_ASSERT(gd_route_layout(model,layout,0,GD_SORT_NAME) && !layout->node_count);
    model->record_count=GD_MAX_RECORDS;
    for (i=0;i<GD_MAX_RECORDS;++i) { model->records[i].role=GD_CHALLENGE; model->records[i].name="node"; model->records[i].reference="challenges/node"; model->records[i].entry=i==0; }
    for (i=0;i<GD_MAX_RECORDS-1;++i) route_edge(model,i,i+1,GD_NEXT);
    while (model->edge_count<GD_MAX_EDGES) route_edge(model,GD_MAX_RECORDS-1,0,GD_NEXT);
    TEST_ASSERT(gd_route_layout(model,layout,1,GD_SORT_NAME) && layout->node_count==GD_MAX_RECORDS && layout->edge_count==GD_MAX_EDGES);
    TEST_ASSERT(layout->nodes[GD_MAX_RECORDS-1].layer==GD_MAX_RECORDS-1 && layout->nodes[0].layer==0);
    model->allocator.release(model->allocator.context,layout); gd_destroy(model);
)
static GD_Model *import_model(void) {
    GD_Model *model=model_create();
    if (!model) return NULL;
    model->record_count=4;
    control_definition(model,0,"config/a","number","2",NULL,"s");
    control_definition(model,1,"config/b","number","3",NULL,"s");
    control_definition(model,2,"config/c","number",NULL,"{config/a}+{config/b}","s");
    control_definition(model,3,"config/mode","enum","plain",NULL,NULL);
    model->records[3].control.kind=GD_LITERAL_STRING;
    model->records[3].control.allowed="plain\0\xe9\x9b\xaa\0\xf0\x9f\x98\x80\0line\n\"quoted\"\0";
    model->records[3].control.allowed_count=4;
    return model;
}
TEST(control_import_roundtrip,
    GD_Model *model=import_model(); GD_ControlImport *plan; int i;
    const char *formats[]={"json","csv","ini"};
    const char *data[]={"{\"a\":4,\"c\":5}","key,type,value,unit\r\na,number,4,s\r\nc,number,5,s\r\n","[controls]\na=4\nc=5\n"};
    TEST_ASSERT(model);
    for (i=0;i<3;++i) {
        plan=gd_plan_control_import(model,NULL,data[i],strlen(data[i]),formats[i],"game");
        TEST_ASSERT(plan && plan->complete && plan->changed_count==1 && plan->derived_count==1);
        TEST_ASSERT(plan->current->values[2].number==5 && plan->candidate->values[2].number==7 && !plan->inputs[1].changed && !strcmp(model->records[0].control.value,"2"));
        gd_control_import_destroy(plan);
    }
    {
        const char *matching="{\"a\":4,\"c\":7}";
        plan=gd_plan_control_import(model,NULL,matching,strlen(matching),"json","game");
        TEST_ASSERT(plan && plan->complete && plan->inputs[1].changed && !plan->inputs[1].conflict); gd_control_import_destroy(plan);
    }
    {
        const char *mismatch="{\"a\":4,\"c\":6}";
        plan=gd_plan_control_import(model,NULL,mismatch,strlen(mismatch),"json","game");
        TEST_ASSERT(plan && !plan->complete && plan->inputs[1].conflict && has_error(model,"controls.import_formula")); gd_control_import_destroy(plan); model->error_count=0;
    }
    {
        GD_ControlSelection selection; const char *changed="{\"a\":4}";
        memset(&selection,0,sizeof(selection)); selection.roots[0]=1;
        for (i=0;i<4;++i) selection.allowed[i]=1;
        model->records[2].control.maximum="6";
        plan=gd_plan_control_import(model,&selection,changed,strlen(changed),"json","game");
        TEST_ASSERT(plan && !plan->complete && has_error(model,"controls.range") && plan->candidate->selected[2]); gd_control_import_destroy(plan);
    }
    gd_destroy(model);
)
TEST(control_import_legacy_dependent,
    GD_Model *model=import_model(); GD_ControlImport *plan; GD_ControlSelection selection;
    const char *changed="{\"a\":4}"; int i;
    TEST_ASSERT(model); memset(&selection,0,sizeof(selection)); selection.roots[0]=1;
    for (i=0;i<4;++i) selection.allowed[i]=1;
    model->records[2].control.expression="${config/a}+{config/b}";
    plan=gd_plan_control_import(model,&selection,changed,strlen(changed),"json","game");
    TEST_ASSERT(plan && !plan->complete && plan->candidate && plan->candidate->selected[2]);
    TEST_ASSERT(has_error(model,"controls.reference_syntax") && !strcmp(model->records[0].control.value,"2"));
    gd_control_import_destroy(plan); gd_destroy(model);
)
TEST(control_import_strings,
    GD_Model *model=import_model(); GD_ControlImport *plan; int i;
    const char *data[]={"{\"mode\":\"\\u96ea\"}","{\"mode\":\"\\ud83d\\ude00\"}","key,type,value,unit\nmode,enum,\"line\n\"\"quoted\"\"\",\n","[controls]\nmode=\"line\\n\\\"quoted\\\"\"\n"};
    const char *formats[]={"json","json","csv","ini"};
    const char *expected[]={"\xe9\x9b\xaa","\xf0\x9f\x98\x80","line\n\"quoted\"","line\n\"quoted\""};
    TEST_ASSERT(model);
    for (i=0;i<4;++i) {
        plan=gd_plan_control_import(model,NULL,data[i],strlen(data[i]),formats[i],"game");
        TEST_ASSERT(plan && plan->complete && plan->changed_count==1 && !strcmp(plan->candidate->values[3].string,expected[i])); gd_control_import_destroy(plan);
    }
    gd_destroy(model);
)
TEST(control_import_failures,
    GD_Model *model=import_model(); GD_ControlImport *plan; int i;
    const char *invalid[]={"{\"a\":2,\"a\":3}","{\"absent\":1}","{\"a\":\"2\"}","{\"a\":01}","{\"a\":.5}","{\"a\":+1}","{\"a\":1e309}","{\"a\":true}","{\"a\":null}","{\"a\":1,}","{a:1}","{\"a\":[]}","{\"a\":1}extra","{\"mode\":\"\\u0000\"}","{\"mode\":\"\\ud800\"}","{\"mode\":\"\\udc00\"}","{\"mode\":\"\\x61\"}","{\"mode\":\"not-allowed\"}"};
    TEST_ASSERT(model);
    for (i=0;i<18;++i) {
        model->error_count=model->errors_omitted=0;
        plan=gd_plan_control_import(model,NULL,invalid[i],strlen(invalid[i]),"json","game");
        TEST_ASSERT(plan && !plan->complete && model->error_count); gd_control_import_destroy(plan);
    }
    model->error_count=0;
    TEST_ASSERT(!gd_plan_control_import(model,NULL,"{}",GD_MAX_FILE+1,"json","game") && has_error(model,"controls.import_input"));
    model->error_count=0;
    {
        GD_Allocator original=model->allocator; RouteFailure failure;
        failure.parent=original; failure.remaining=1; model->allocator.context=&failure; model->allocator.allocate=route_allocate; model->allocator.release=route_release;
        plan=gd_plan_control_import(model,NULL,"{}",2,"json","game");
        TEST_ASSERT(plan && !plan->complete && !plan->current && has_error(model,"input.memory")); gd_control_import_destroy(plan); model->allocator=original;
    }
    gd_destroy(model);
)
TEST(graph_analysis_projection,
    GD_Layout *layout=allocate(storage,sizeof(*layout)); GD_GraphAnalysis *analysis=allocate(storage,sizeof(*analysis));
    unsigned seed=37; int run,i,a,b,k,mode; unsigned char shown[8];
    TEST_ASSERT(layout && analysis); memset(layout,0,sizeof(*layout)); memset(analysis,0,sizeof(*analysis));
    layout->node_count=8; layout->edge_count=12;
    for (i=0;i<8;++i) { layout->nodes[i].key=100+i; layout->nodes[i].label="node"; }
    for (run=0;run<32;++run) {
        for (i=0;i<12;++i) {
            GD_LayoutEdge *edge=&layout->edges[i];
            seed=seed*1664525u+1013904223u; edge->a=(int)((seed>>16)%8);
            seed=seed*1664525u+1013904223u; edge->b=(int)((seed>>16)%8);
            edge->key=i; edge->active=i%3!=0;
        }
        for (mode=0;mode<4;++mode) {
            int adjacent[8][8]={{0}},incident[8]={0},nodes=0,edges=0;
            GD_LayoutEdge before[12]; memcpy(before,layout->edges,sizeof(before));
            for (i=0;i<8;++i) { shown[i]=(unsigned char)(mode<2 || i%3!=0); nodes+=shown[i]; }
            for (i=0;i<12;++i) {
                const GD_LayoutEdge *edge=&layout->edges[i];
                if ((!edge->active && !(mode%2)) || !shown[edge->a] || !shown[edge->b]) continue;
                adjacent[edge->a][edge->b]=adjacent[edge->b][edge->a]=1;
                ++edges; ++incident[edge->a]; if (edge->a!=edge->b) ++incident[edge->b];
            }
            gd_graph_analyze(layout,shown,mode%2,GD_GRAPH_ANY,analysis);
            TEST_ASSERT(analysis->complete && analysis->shown_nodes==nodes && analysis->shown_edges==edges);
            TEST_ASSERT(!memcmp(before,layout->edges,sizeof(before)));
            for (a=0;a<8;++a) {
                int matches=0,neighbors=0;
                for (b=0;b<8;++b) {
                    int common=0,pair;
                    if (a==b) continue;
                    neighbors+=adjacent[a][b];
                    for (k=0;k<8;++k) common+=adjacent[a][k] && adjacent[b][k];
                    pair=gd_graph_match(analysis,a,b);
                    TEST_ASSERT((pair>=0)==(common>0));
                    if (pair>=0) {
                        const GD_GraphMatch *match=&analysis->pairs[pair];
                        const GD_GraphFeature *left=&analysis->features[match->first_a],*right=&analysis->features[match->first_b];
                        TEST_ASSERT(match->shared==common && left->neighbor==right->neighbor);
                        TEST_ASSERT(adjacent[left->owner][left->neighbor] && adjacent[right->owner][right->neighbor]);
                        TEST_ASSERT(left->edge>=0 && left->edge<12 && right->edge>=0 && right->edge<12);
                        ++matches;
                    }
                }
                TEST_ASSERT(analysis->matched[a]==matches && analysis->neighbors[a]==neighbors && analysis->incident[a]==incident[a]);
            }
            layout->nodes[0].x+=1; layout->nodes[0].y-=2; ++layout->tick;
            TEST_ASSERT(!gd_graph_analyze(layout,shown,mode%2,GD_GRAPH_ANY,analysis));
        }
    }
    layout->edges[0].key+=100; TEST_ASSERT(gd_graph_analyze(layout,shown,1,GD_GRAPH_ANY,analysis));
    release(storage,analysis); release(storage,layout);
)
TEST(graph_analysis_roles,
    GD_Layout *layout=allocate(storage,sizeof(*layout)); GD_GraphAnalysis *analysis=allocate(storage,sizeof(*analysis));
    unsigned seed=91; int run,mode,lens,i,a,b,n,r,s,x,y; unsigned char shown[6];
    TEST_ASSERT(layout && analysis); memset(layout,0,sizeof(*layout)); memset(analysis,0,sizeof(*analysis));
    layout->node_count=6; layout->edge_count=16;
    for (i=0;i<6;++i) layout->nodes[i].key=i;
    for (run=0;run<32;++run) {
        for (i=0;i<16;++i) {
            GD_LayoutEdge *edge=&layout->edges[i];
            seed=seed*1664525u+1013904223u; edge->a=(int)((seed>>16)%6);
            seed=seed*1664525u+1013904223u; edge->b=(int)((seed>>16)%6);
            seed=seed*1664525u+1013904223u; edge->relation=(int)((seed>>16)%4);
            edge->key=i; edge->active=i%3!=0;
        }
        for (mode=0;mode<4;++mode) {
            unsigned char roles[6][6][4][2]={{{{0}}}};
            for (i=0;i<6;++i) shown[i]=(unsigned char)(mode<2 || i%3!=0);
            for (i=0;i<16;++i) {
                const GD_LayoutEdge *edge=&layout->edges[i];
                if ((!edge->active && !(mode%2)) || !shown[edge->a] || !shown[edge->b]) continue;
                roles[edge->a][edge->b][edge->relation][0]=1;
                roles[edge->b][edge->a][edge->relation][1]=1;
            }
            for (lens=0;lens<GD_GRAPH_LENS_COUNT;++lens) {
                gd_graph_analyze(layout,shown,mode%2,(GD_GraphLens)lens,analysis);
                TEST_ASSERT(analysis->complete && analysis->lens==(GD_GraphLens)lens);
                for (a=0;a<6;++a) for (b=a+1;b<6;++b) {
                    int common=0,pair,counts[16]={0},saved_work=analysis->work;
                    const GD_GraphComparison *comparison=gd_graph_compare(analysis,a,b);
                    TEST_ASSERT((comparison!=NULL)==(shown[a] && shown[b]));
                    for (n=0;n<6;++n) {
                        int qualifies=0,has_a=0,has_b=0;
                        for (r=0;r<4;++r) for (x=0;x<2;++x) { has_a|=roles[a][n][r][x]; has_b|=roles[b][n][r][x]; }
                        for (r=0;r<4;++r) for (s=0;s<4;++s) for (x=0;x<2;++x) for (y=0;y<2;++y)
                            if (roles[a][n][r][x] && roles[b][n][s][y] &&
                                (lens==GD_GRAPH_ANY || (lens==GD_GRAPH_SAME_ROLE ? r==s && x==y:r!=s || x!=y))) qualifies=1;
                        common+=qualifies;
                        if (comparison) {
                            int category=has_a && has_b ? (qualifies ? GD_GRAPH_SHARED:GD_GRAPH_OTHER_ROLE):
                                has_a ? GD_GRAPH_A_ONLY:has_b ? GD_GRAPH_B_ONLY:0;
                            TEST_ASSERT(comparison->nodes[n]==category); ++counts[category];
                        }
                    }
                    if (comparison) {
                        TEST_ASSERT(comparison->a==a && comparison->b==b && comparison->shared==common);
                        TEST_ASSERT(comparison->a_only==counts[GD_GRAPH_A_ONLY] && comparison->b_only==counts[GD_GRAPH_B_ONLY] && comparison->other_role==counts[GD_GRAPH_OTHER_ROLE]);
                        for (i=0;i<16;++i) {
                            const GD_LayoutEdge *edge=&layout->edges[i]; unsigned flags=0;
                            if ((edge->active || mode%2) && shown[edge->a] && shown[edge->b]) for (x=0;x<2;++x) {
                                int owner=x ? edge->b:edge->a,neighbor=x ? edge->a:edge->b,other,exists=0,qualifies=0;
                                if (owner!=a && owner!=b) continue;
                                other=owner==a ? b:a;
                                for (r=0;r<4;++r) for (s=0;s<2;++s) if (roles[other][neighbor][r][s]) {
                                    exists=1;
                                    if (lens==GD_GRAPH_ANY || (lens==GD_GRAPH_SAME_ROLE ? r==edge->relation && s==x:r!=edge->relation || s!=x)) qualifies=1;
                                }
                                flags|=!exists ? (owner==a ? GD_GRAPH_A_ONLY:GD_GRAPH_B_ONLY):qualifies ? GD_GRAPH_SHARED:GD_GRAPH_OTHER_ROLE;
                            }
                            TEST_ASSERT(comparison->edges[i]==flags);
                        }
                        TEST_ASSERT(gd_graph_compare(analysis,a,b)==comparison && analysis->work==saved_work);
                        comparison=gd_graph_compare(analysis,b,a);
                        TEST_ASSERT(comparison->a_only==counts[GD_GRAPH_B_ONLY] && comparison->b_only==counts[GD_GRAPH_A_ONLY] && comparison->shared==common);
                    }
                    pair=gd_graph_match(analysis,a,b);
                    TEST_ASSERT((pair>=0)==(common>0));
                    if (pair>=0) {
                        const GD_GraphMatch *match=&analysis->pairs[pair];
                        const GD_GraphFeature *left=&analysis->features[match->first_a],*right=&analysis->features[match->first_b];
                        int same=left->relation==right->relation && left->endpoint==right->endpoint;
                        TEST_ASSERT(match->shared==common && left->neighbor==right->neighbor);
                        TEST_ASSERT(roles[left->owner][left->neighbor][left->relation][left->endpoint] && roles[right->owner][right->neighbor][right->relation][right->endpoint]);
                        TEST_ASSERT(lens==GD_GRAPH_ANY || (lens==GD_GRAPH_SAME_ROLE ? same:!same));
                    }
                }
                TEST_ASSERT(!gd_graph_analyze(layout,shown,mode%2,(GD_GraphLens)lens,analysis));
            }
        }
    }
    layout->edges[0].relation=(layout->edges[0].relation+1)%4;
    TEST_ASSERT(gd_graph_analyze(layout,shown,1,GD_GRAPH_CHANGED_ROLE,analysis));
    TEST_ASSERT(gd_graph_analyze(layout,analysis->shown,analysis->include_inactive,GD_GRAPH_ANY,analysis));
    layout->node_count=3; layout->edge_count=4;
    layout->edges[0]=(GD_LayoutEdge){0,2,0,1,GD_MECHANICS};
    layout->edges[1]=(GD_LayoutEdge){1,2,1,1,GD_MECHANICS};
    layout->edges[2]=(GD_LayoutEdge){1,2,2,1,GD_RESOURCES}; layout->edges[3]=layout->edges[2];
    for (lens=0;lens<GD_GRAPH_LENS_COUNT;++lens) {
        int pair; gd_graph_analyze(layout,NULL,1,(GD_GraphLens)lens,analysis); pair=gd_graph_match(analysis,0,1);
        TEST_ASSERT(pair>=0 && analysis->pairs[pair].shared==1 && analysis->neighbors[1]==1 && analysis->incident[1]==3);
    }
    layout->edge_count=2; layout->edges[1].relation=GD_MECHANICS; layout->edges[1].a=2; layout->edges[1].b=1;
    gd_graph_analyze(layout,NULL,1,GD_GRAPH_SAME_ROLE,analysis); TEST_ASSERT(gd_graph_match(analysis,0,1)<0);
    gd_graph_analyze(layout,NULL,1,GD_GRAPH_CHANGED_ROLE,analysis); TEST_ASSERT(gd_graph_match(analysis,0,1)>=0);
    release(storage,analysis); release(storage,layout);
)
/* Independent removal/reachability oracle, deliberately not dominator-tree math. */
static void path_oracle(const GD_Layout *layout, const unsigned char *shown, const unsigned char *roots,
                        int inactive, int undirected, int removed, unsigned char *reached) {
    int queue[8],front=0,back=0,i;
    assert(layout->node_count<=8); memset(reached,0,8);
    for (i=0;i<layout->node_count;++i) if (roots[i] && shown[i] && i!=removed) { reached[i]=1; queue[back++]=i; }
    while (front<back) {
        int node=queue[front++],side;
        for (i=0;i<layout->edge_count;++i) for (side=0;side<(undirected ? 2:1);++side) {
            const GD_LayoutEdge *edge=&layout->edges[i]; int a=side ? edge->b:edge->a,b=side ? edge->a:edge->b;
            if (a!=node || b==removed || (!edge->active && !inactive) || !shown[b] || reached[b]) continue;
            reached[b]=1; queue[back++]=b;
        }
    }
}
TEST(graph_paths,
    GD_Layout *layout=allocate(storage,sizeof(*layout)); GD_GraphAnalysis *analysis=allocate(storage,sizeof(*analysis));
    unsigned seed=817; int run,mode,choice,undirected,i,j,v;
    unsigned char shown[8],roots[8],reached[8],without[8][8];
    TEST_ASSERT(layout && analysis); memset(layout,0,sizeof(*layout)); memset(analysis,0,sizeof(*analysis));
    layout->node_count=8; layout->edge_count=24;
    for (i=0;i<8;++i) layout->nodes[i].key=100+i;
    for (run=0;run<64;++run) {
        for (i=0;i<24;++i) {
            seed=seed*1664525u+1013904223u; layout->edges[i].a=(int)((seed>>16)%8);
            seed=seed*1664525u+1013904223u; layout->edges[i].b=(int)((seed>>16)%8);
            layout->edges[i].key=i; layout->edges[i].active=i%3!=0;
        }
        for (mode=0;mode<4;++mode) {
            for (i=0;i<8;++i) shown[i]=(unsigned char)(mode<2 || i%3!=0);
            gd_graph_analyze(layout,shown,mode%2,GD_GRAPH_ANY,analysis);
            for (choice=0;choice<3;++choice) for (undirected=0;undirected<2;++undirected) {
                const GD_GraphPaths *paths; int count=0,global_work=analysis->work;
                for (i=0;i<8;++i) roots[i]=(unsigned char)(choice==0 ? i==0:choice==1 ? i%2==0:0);
                path_oracle(layout,shown,roots,mode%2,undirected,-1,reached);
                for (i=0;i<8;++i) path_oracle(layout,shown,roots,mode%2,undirected,i,without[i]);
                paths=gd_graph_paths(analysis,roots,undirected,GD_GRAPH_ANALYSIS_WORK);
                TEST_ASSERT(paths->complete && analysis->work==global_work);
                for (i=0;i<8;++i) {
                    int expected=0;
                    TEST_ASSERT(paths->reached[i]==reached[i]); count+=reached[i];
                    for (j=0;j<8;++j) expected+=j!=i && reached[j] && !without[i][j];
                    TEST_ASSERT(paths->dominated[i]==expected);
                }
                TEST_ASSERT(paths->reachable_count==count);
                for (v=0;v<8;++v) {
                    const GD_PathSelection *selection=gd_graph_path_selection(analysis,v); int node=v;
                    if (!reached[v]) { TEST_ASSERT(!selection && paths->dominator[v]<0); continue; }
                    TEST_ASSERT(selection && selection->length==selection->retained);
                    for (i=0;i<8;++i) TEST_ASSERT(selection->ancestors[i]==(i!=v && reached[v] && !without[i][v]));
                    for (i=0;i<selection->retained;++i) {
                        int arc=selection->arcs[i]; const GD_LayoutEdge *edge=&layout->edges[arc/2];
                        TEST_ASSERT((undirected || arc%2==0) && shown[edge->a] && shown[edge->b] && (edge->active || mode%2));
                        TEST_ASSERT(node==(arc%2 ? edge->a:edge->b) && selection->edges[arc/2]); node=arc%2 ? edge->b:edge->a;
                    }
                    TEST_ASSERT(roots[node] && shown[node]);
                    {
                        int support_position[8],support_root=node,c,expected=0,path_work=paths->work;
                        for (i=0;i<8;++i) support_position[i]=-1;
                        for (node=v,i=0;;++i) { int arc=paths->parent_arc[node]; support_position[node]=i; if (arc<0) break; node=arc%2 ? layout->edges[arc/2].b:layout->edges[arc/2].a; }
                        for (i=0;i<8;++i) expected+=support_position[i]>0 && !selection->ancestors[i];
                        TEST_ASSERT(selection->candidate_count==expected);
                        for (c=0;c<selection->candidate_count;++c) {
                            int candidate=selection->candidates[c],position[8],work; unsigned char expected_edges[24]={0}; const GD_PathBypass *bypass;
                            TEST_ASSERT(without[candidate][v] && support_position[candidate]>0);
                            bypass=gd_graph_path_bypass(analysis,v,candidate,0); TEST_ASSERT(bypass && !bypass->complete && !bypass->found);
                            bypass=gd_graph_path_bypass(analysis,v,candidate,GD_GRAPH_ANALYSIS_WORK);
                            TEST_ASSERT(bypass && bypass->complete && bypass->found && bypass->length==bypass->retained);
                            for (i=0;i<8;++i) position[i]=-1;
                            node=v; position[node]=0;
                            for (i=0;i<bypass->retained;++i) {
                                int arc=bypass->arcs[i]; const GD_LayoutEdge *edge=&layout->edges[arc/2];
                                TEST_ASSERT((undirected || arc%2==0) && shown[edge->a] && shown[edge->b] && (edge->active || mode%2));
                                TEST_ASSERT(node==(arc%2 ? edge->a:edge->b) && bypass->edges[arc/2]); expected_edges[arc/2]=1; node=arc%2 ? edge->b:edge->a;
                                TEST_ASSERT(node!=candidate && position[node]<0); position[node]=i+1;
                            }
                            TEST_ASSERT(!memcmp(expected_edges,bypass->edges,sizeof(expected_edges)));
                            TEST_ASSERT(undirected || paths->incoming[bypass->rejoin]>=2);
                            TEST_ASSERT(roots[node] && shown[node] && support_position[bypass->rejoin]<support_position[candidate]);
                            if (bypass->fork>=0) TEST_ASSERT(support_position[bypass->fork]>support_position[candidate] && position[bypass->fork]>position[bypass->rejoin]);
                            else TEST_ASSERT(node!=support_root);
                            for (i=0;i<8;++i) if (i!=bypass->fork && i!=bypass->rejoin && support_position[i]>support_position[bypass->rejoin] && (bypass->fork<0 || support_position[i]<support_position[bypass->fork]))
                                TEST_ASSERT(position[i]<0 || position[i]<position[bypass->rejoin] || (bypass->fork>=0 && position[i]>position[bypass->fork]));
                            work=bypass->work; TEST_ASSERT(gd_graph_path_bypass(analysis,v,candidate,GD_GRAPH_ANALYSIS_WORK)==bypass && bypass->work==work);
                            TEST_ASSERT(paths->complete && paths->work==path_work && analysis->work==global_work);
                        }
                        TEST_ASSERT(!gd_graph_path_bypass(analysis,v,v,GD_GRAPH_ANALYSIS_WORK) && !gd_graph_path_bypass(analysis,v,-2,GD_GRAPH_ANALYSIS_WORK));
                        if (!selection->candidate_count) TEST_ASSERT(!gd_graph_path_bypass(analysis,v,-1,GD_GRAPH_ANALYSIS_WORK));
                    }
                }
                TEST_ASSERT(gd_graph_paths(analysis,roots,undirected,GD_GRAPH_ANALYSIS_WORK)==paths);
                TEST_ASSERT(!memcmp(analysis->edges,layout->edges,24*sizeof(layout->edges[0])));
            }
        }
    }
    release(storage,analysis); release(storage,layout);
)
TEST(graph_bypass_ordering,
    GD_Layout *layout=allocate(storage,sizeof(*layout)); GD_GraphAnalysis *analysis=allocate(storage,sizeof(*analysis));
    unsigned char roots[13]={1},shown[13]; const GD_PathBypass *bypass; int i;
    const int edges[][2]={{0,10},{0,3},{10,11},{11,12},{12,1},{1,2},{1,4},{2,3},{3,6},{3,1},{6,7},{7,8},{8,9},{9,5},{4,5}};
    TEST_ASSERT(layout && analysis); memset(layout,0,sizeof(*layout)); memset(analysis,0,sizeof(*analysis));
    layout->node_count=13; layout->edge_count=15;
    for (i=0;i<13;++i) layout->nodes[i].key=i;
    for (i=0;i<15;++i) layout->edges[i]=(GD_LayoutEdge){edges[i][0],edges[i][1],i,1,0};
    gd_graph_analyze(layout,NULL,1,GD_GRAPH_ANY,analysis); gd_graph_paths(analysis,roots,0,GD_GRAPH_ANALYSIS_WORK);
    bypass=gd_graph_path_bypass(analysis,5,2,GD_GRAPH_ANALYSIS_WORK);
    /* Support: 0,10,11,12,1,2,3,6,7,8,9,5. Bypass: 0,3,1,4,5.
       Shared node 3 follows C on support but PRECEDES F=1 on bypass. */
    TEST_ASSERT(bypass && bypass->found && bypass->fork==1 && bypass->rejoin==5 && bypass->length==4);
    TEST_ASSERT(bypass->edges[1] && bypass->edges[9] && bypass->edges[6] && bypass->edges[14]);
    analysis->paths.selection.candidate=2;
    TEST_ASSERT(gd_graph_path_bypass(analysis,5,-1,GD_GRAPH_ANALYSIS_WORK)==bypass && bypass->candidate==2);
    layout->node_count=4; layout->edge_count=3;
    layout->edges[0]=(GD_LayoutEdge){0,2,0,1,0}; layout->edges[1]=(GD_LayoutEdge){2,3,1,1,0}; layout->edges[2]=(GD_LayoutEdge){1,3,2,1,0}; roots[1]=1;
    gd_graph_analyze(layout,NULL,1,GD_GRAPH_ANY,analysis); gd_graph_paths(analysis,roots,0,GD_GRAPH_ANALYSIS_WORK);
    bypass=gd_graph_path_bypass(analysis,3,2,1); TEST_ASSERT(bypass && !bypass->complete && !bypass->found && !bypass->edges[2]);
    bypass=gd_graph_path_bypass(analysis,3,2,GD_GRAPH_ANALYSIS_WORK);
    TEST_ASSERT(bypass && bypass->found && bypass->fork==-1 && bypass->rejoin==3 && bypass->length==1 && bypass->edges[2]);
    memset(shown,1,sizeof(shown)); shown[1]=0; gd_graph_analyze(layout,shown,1,GD_GRAPH_ANY,analysis); gd_graph_paths(analysis,roots,0,GD_GRAPH_ANALYSIS_WORK);
    TEST_ASSERT(!gd_graph_path_bypass(analysis,3,2,GD_GRAPH_ANALYSIS_WORK));
    release(storage,analysis); release(storage,layout);
)
TEST(graph_path_limits,
    GD_Layout *layout=allocate(storage,sizeof(*layout)); GD_GraphAnalysis *analysis=allocate(storage,sizeof(*analysis));
    unsigned char roots[GD_MAX_RECORDS]={0}; GD_PathControls controls={0}; const GD_GraphPaths *paths; const GD_PathSelection *selection; int i;
    TEST_ASSERT(layout && analysis); memset(layout,0,sizeof(*layout)); memset(analysis,0,sizeof(*analysis));
    layout->node_count=GD_MAX_RECORDS; layout->edge_count=GD_MAX_RECORDS-1;
    for (i=0;i<layout->node_count;++i) layout->nodes[i].key=GD_MAX_RECORDS-i;
    for (i=0;i<layout->edge_count;++i) layout->edges[i]=(GD_LayoutEdge){i,i+1,i,1,0};
    roots[0]=1; gd_graph_analyze(layout,NULL,1,GD_GRAPH_ANY,analysis);
    paths=gd_graph_paths(analysis,roots,0,0); TEST_ASSERT(!paths->complete && !paths->work && !gd_graph_path_selection(analysis,0));
    paths=gd_graph_paths(analysis,roots,0,32); TEST_ASSERT(!paths->complete && paths->work==32);
    paths=gd_graph_paths(analysis,roots,0,GD_MAX_RECORDS*3);
    TEST_ASSERT(!paths->complete && paths->dominator[GD_MAX_RECORDS-1]==GD_MAX_RECORDS-2 && !gd_graph_path_selection(analysis,GD_MAX_RECORDS-1));
    paths=gd_graph_paths(analysis,roots,0,GD_GRAPH_ANALYSIS_WORK);
    TEST_ASSERT(paths->complete && paths->dominated[0]==GD_MAX_RECORDS-1 && paths->dominator[GD_MAX_RECORDS-1]==GD_MAX_RECORDS-2);
    selection=gd_graph_path_selection(analysis,GD_MAX_RECORDS-1);
    TEST_ASSERT(selection && selection->retained==64 && selection->length==GD_MAX_RECORDS-1 && selection->ancestor_count==GD_MAX_RECORDS-1);
    TEST_ASSERT(!gd_graph_paths_prepare(analysis,&controls,NULL,0));
    TEST_ASSERT(gd_graph_path_start(&controls,GD_MAX_RECORDS)==1 && !gd_graph_path_start(&controls,GD_MAX_RECORDS)); controls.enabled=controls.declared=1;
    TEST_ASSERT(!gd_graph_paths_prepare(analysis,&controls,NULL,0)->root_count);
    TEST_ASSERT(gd_graph_paths_prepare(analysis,&controls,roots,0)->root_count==1); controls.declared=0;
    TEST_ASSERT(gd_graph_paths_prepare(analysis,&controls,NULL,0)->root_count==1);
    layout->nodes[0].x=70; TEST_ASSERT(!gd_graph_analyze(layout,NULL,1,GD_GRAPH_ANY,analysis) && analysis->paths.valid);
    layout->edges[0].active=0; TEST_ASSERT(gd_graph_analyze(layout,NULL,0,GD_GRAPH_ANY,analysis) && !analysis->paths.valid);
    TEST_ASSERT(gd_graph_paths_prepare(analysis,&controls,NULL,0)->reachable_count==1);
    layout->edge_count=GD_MAX_RECORDS;
    for (i=0;i<layout->edge_count-1;++i) layout->edges[i]=(GD_LayoutEdge){i,i+1,i,1,0};
    layout->edges[GD_MAX_RECORDS-1]=(GD_LayoutEdge){0,2,GD_MAX_RECORDS-1,1,0};
    gd_graph_analyze(layout,NULL,1,GD_GRAPH_ANY,analysis); paths=gd_graph_paths(analysis,roots,0,GD_GRAPH_ANALYSIS_WORK);
    {
        const GD_PathBypass *bypass=gd_graph_path_bypass(analysis,GD_MAX_RECORDS-1,-1,32);
        TEST_ASSERT(bypass && !bypass->complete && bypass->work==32 && !bypass->found);
        bypass=gd_graph_path_bypass(analysis,GD_MAX_RECORDS-1,-1,GD_GRAPH_ANALYSIS_WORK);
        TEST_ASSERT(bypass && bypass->complete && bypass->found && bypass->candidate==1 && bypass->fork==0 && bypass->rejoin==2);
        TEST_ASSERT(bypass->retained==64 && bypass->length==GD_MAX_RECORDS-2 && bypass->edges[GD_MAX_RECORDS-1] && !bypass->edges[0] && !bypass->edges[1]);
        layout->nodes[0].y=80; TEST_ASSERT(!gd_graph_analyze(layout,NULL,1,GD_GRAPH_ANY,analysis) && analysis->paths.bypass.valid);
        TEST_ASSERT(gd_graph_path_selection(analysis,3)->candidate_count==1 && !analysis->paths.bypass.valid);
        TEST_ASSERT(!gd_graph_path_bypass(analysis,3,0,GD_GRAPH_ANALYSIS_WORK));
    }
    layout->edge_count=GD_MAX_PAIRS;
    for (i=0;i<layout->edge_count;++i) layout->edges[i]=(GD_LayoutEdge){0,i%(GD_MAX_RECORDS-1)+1,i,1,0};
    memset(roots,1,sizeof(roots)); gd_graph_analyze(layout,NULL,1,GD_GRAPH_ANY,analysis);
    paths=gd_graph_paths(analysis,roots,1,GD_GRAPH_ANALYSIS_WORK);
    TEST_ASSERT(!analysis->complete && paths->complete && paths->root_count==GD_MAX_RECORDS && paths->incoming[0]==GD_MAX_RECORDS-1);
    for (i=0;i<GD_MAX_RECORDS;++i) TEST_ASSERT(!paths->dominated[i] && paths->dominator[i]==GD_MAX_RECORDS);
    memset(roots,0,sizeof(roots)); roots[0]=1; layout->edges[GD_MAX_PAIRS-1].a=1; layout->edges[GD_MAX_PAIRS-1].b=2;
    gd_graph_analyze(layout,NULL,1,GD_GRAPH_ANY,analysis); gd_graph_paths(analysis,roots,0,GD_GRAPH_ANALYSIS_WORK);
    {
        const GD_PathBypass *bypass=gd_graph_path_bypass(analysis,2,1,GD_GRAPH_ANALYSIS_WORK);
        TEST_ASSERT(!analysis->complete && bypass && bypass->complete && bypass->found && bypass->fork==0 && bypass->rejoin==2 && bypass->length==1);
    }
    layout->node_count=layout->edge_count=0; gd_graph_analyze(layout,NULL,1,GD_GRAPH_ANY,analysis);
    TEST_ASSERT(!gd_graph_paths(analysis,NULL,0,0)->root_count && !gd_graph_path_selection(analysis,0));
    controls.count=0;
    for (i=0;i<GD_MAX_RECORDS;++i) TEST_ASSERT(gd_graph_path_start(&controls,i)==1);
    TEST_ASSERT(gd_graph_path_start(&controls,INT_MAX)==-1 && !gd_graph_path_start(&controls,0));
    {
        GD_Model *old=model_create(),*fresh=model_create(); TEST_ASSERT(old && fresh);
        old->record_count=3; fresh->record_count=2;
        old->records[0].reference="records/one"; old->records[1].reference="records/two"; old->records[2].reference="records/gone";
        fresh->records[0].reference="records/two"; fresh->records[1].reference="records/one";
        controls.count=controls.lost=0; controls.enabled=controls.declared=controls.compare=1;
        for (i=0;i<3;++i) gd_graph_path_start(&controls,i);
        gd_graph_paths_remap(&controls,old,fresh);
        TEST_ASSERT(controls.count==2 && controls.keys[0]==0 && controls.keys[1]==1 && controls.lost==1 && controls.enabled && controls.declared && controls.compare);
        controls.count=controls.lost=0; fresh->record_count=3; fresh->records[2].reference="records/one";
        for (i=0;i<3;++i) gd_graph_path_start(&controls,i);
        gd_graph_paths_remap(&controls,old,fresh); TEST_ASSERT(controls.count==1 && controls.keys[0]==0 && controls.lost==2);
        fresh->records[1].reference="records/two";
        gd_graph_paths_remap(&controls,fresh,old); TEST_ASSERT(!controls.count && controls.lost==3);
        gd_destroy(fresh); gd_destroy(old);
    }
    release(storage,analysis); release(storage,layout);
)
TEST(graph_comparison,
    GD_Layout *layout=allocate(storage,sizeof(*layout)); GD_GraphAnalysis *analysis=allocate(storage,sizeof(*analysis));
    const GD_GraphComparison *comparison; int i; unsigned char shown[6]={1,1,1,0,1,1};
    TEST_ASSERT(layout && analysis); memset(layout,0,sizeof(*layout)); memset(analysis,0,sizeof(*analysis));
    TEST_ASSERT(!gd_graph_compare(analysis,0,1));
    layout->node_count=6; layout->edge_count=6;
    for (i=0;i<6;++i) layout->nodes[i].key=i;
    layout->edges[0]=(GD_LayoutEdge){0,2,0,1,1}; layout->edges[1]=(GD_LayoutEdge){1,2,1,1,1};
    layout->edges[2]=(GD_LayoutEdge){0,3,2,1,1}; layout->edges[3]=(GD_LayoutEdge){1,4,3,1,1};
    layout->edges[4]=(GD_LayoutEdge){0,5,4,1,1}; layout->edges[5]=(GD_LayoutEdge){1,5,5,1,2};
    gd_graph_analyze(layout,NULL,1,GD_GRAPH_SAME_ROLE,analysis); comparison=gd_graph_compare(analysis,0,1);
    TEST_ASSERT(comparison && comparison->shared==1 && comparison->other_role==1 && comparison->a_only==1 && comparison->b_only==1);
    TEST_ASSERT(comparison->edges[0]==GD_GRAPH_SHARED && comparison->edges[2]==GD_GRAPH_A_ONLY && comparison->edges[3]==GD_GRAPH_B_ONLY && comparison->edges[4]==GD_GRAPH_OTHER_ROLE);
    TEST_ASSERT(!gd_graph_compare(analysis,-1,1) && !gd_graph_compare(analysis,0,6) && !gd_graph_compare(analysis,1,1));
    layout->nodes[0].x=40; TEST_ASSERT(!gd_graph_analyze(layout,NULL,1,GD_GRAPH_SAME_ROLE,analysis) && analysis->comparison.valid);
    layout->edges[6]=(GD_LayoutEdge){0,1,6,1,1}; layout->edge_count=7;
    TEST_ASSERT(gd_graph_analyze(layout,NULL,1,GD_GRAPH_SAME_ROLE,analysis) && !analysis->comparison.valid);
    comparison=gd_graph_compare(analysis,0,1);
    TEST_ASSERT(comparison->a_only==2 && comparison->b_only==2 && comparison->edges[6]==(GD_GRAPH_A_ONLY|GD_GRAPH_B_ONLY));
    gd_graph_analyze(layout,shown,1,GD_GRAPH_ANY,analysis); comparison=gd_graph_compare(analysis,0,1);
    TEST_ASSERT(comparison->shared==2 && !comparison->other_role && comparison->a_only==1 && comparison->b_only==2 && !comparison->edges[2]);
    TEST_ASSERT(!gd_graph_compare(analysis,0,3));
    layout->node_count=3; layout->edge_count=GD_MAX_PAIRS;
    for (i=0;i<layout->edge_count;++i) layout->edges[i]=(GD_LayoutEdge){i%2,2,i,1,i/2};
    gd_graph_analyze(layout,NULL,1,GD_GRAPH_SAME_ROLE,analysis); comparison=gd_graph_compare(analysis,0,1);
    TEST_ASSERT(analysis->feature_count==2*GD_MAX_PAIRS && comparison->shared==1 && !comparison->a_only && !comparison->other_role);
    for (i=0;i<layout->edge_count;++i) TEST_ASSERT(comparison->edges[i]==GD_GRAPH_SHARED);
    release(storage,analysis); release(storage,layout);
)
TEST(graph_analysis_limits,
    GD_Layout *layout=allocate(storage,sizeof(*layout)); GD_GraphAnalysis *analysis=allocate(storage,sizeof(*analysis)); int i;
    TEST_ASSERT(layout && analysis); memset(layout,0,sizeof(*layout)); memset(analysis,0,sizeof(*analysis));
    layout->node_count=GD_MAX_RECORDS; layout->edge_count=GD_MAX_RECORDS-1;
    for (i=0;i<GD_MAX_RECORDS;++i) layout->nodes[i].key=i;
    for (i=0;i<layout->edge_count;++i) layout->edges[i]=(GD_LayoutEdge){0,i+1,i,1,0};
    TEST_ASSERT(gd_graph_analyze(layout,NULL,1,GD_GRAPH_ANY,analysis));
    TEST_ASSERT(!analysis->complete && analysis->pair_count==GD_GRAPH_ANALYSIS_PAIRS && analysis->work<=GD_GRAPH_ANALYSIS_WORK);
    TEST_ASSERT(analysis->neighbors[0]==GD_MAX_RECORDS-1 && analysis->incident[0]==GD_MAX_RECORDS-1 && !analysis->matched[0]);
    TEST_ASSERT(gd_graph_match(analysis,GD_MAX_RECORDS-2,GD_MAX_RECORDS-1)<0);
    { int work=analysis->work; const GD_GraphComparison *comparison=gd_graph_compare(analysis,GD_MAX_RECORDS-2,GD_MAX_RECORDS-1);
      TEST_ASSERT(comparison && comparison->shared==1 && !comparison->a_only && !comparison->other_role && analysis->work==work); }
    layout->edge_count=(GD_MAX_RECORDS-1)*2;
    for (i=0;i<layout->edge_count;++i) layout->edges[i]=(GD_LayoutEdge){0,i/2+1,i,1,i};
    TEST_ASSERT(gd_graph_analyze(layout,NULL,1,GD_GRAPH_SAME_ROLE,analysis));
    TEST_ASSERT(!analysis->complete && !analysis->pair_count && analysis->work==GD_GRAPH_ANALYSIS_WORK);
    TEST_ASSERT(analysis->neighbors[0]==GD_MAX_RECORDS-1 && analysis->incident[0]==(GD_MAX_RECORDS-1)*2);
    { const GD_GraphComparison *comparison=gd_graph_compare(analysis,GD_MAX_RECORDS-2,GD_MAX_RECORDS-1);
      TEST_ASSERT(comparison && !comparison->shared && comparison->other_role==1 && !comparison->a_only && analysis->work==GD_GRAPH_ANALYSIS_WORK); }
    layout->edge_count=GD_MAX_PAIRS;
    for (i=0;i<layout->edge_count;++i) layout->edges[i]=(GD_LayoutEdge){i%GD_MAX_RECORDS,i%GD_MAX_RECORDS,i,1,0};
    TEST_ASSERT(gd_graph_analyze(layout,NULL,1,GD_GRAPH_ANY,analysis) && analysis->complete && !analysis->pair_count);
    TEST_ASSERT(analysis->feature_count==GD_MAX_RECORDS*2 && analysis->shown_edges==GD_MAX_PAIRS);
    for (i=0;i<GD_MAX_RECORDS;++i) TEST_ASSERT(!analysis->neighbors[i] && !analysis->matched[i] && analysis->incident[i]==GD_MAX_PAIRS/GD_MAX_RECORDS);
    layout->node_count=layout->edge_count=0;
    TEST_ASSERT(gd_graph_analyze(layout,NULL,0,GD_GRAPH_ANY,analysis) && analysis->complete && !analysis->feature_count && !analysis->shown_nodes && !analysis->shown_edges);
    release(storage,analysis); release(storage,layout);
)
TEST(graph_input_links,
    GD_Model *model=model_create(); GD_Draft *draft; GD_Focus focus; int encounter, energy, jump;
    GD_Layout *layout=allocate(storage,sizeof(*layout)); GD_LinkIndex *index=allocate(storage,sizeof(*index));
    TEST_ASSERT(model && layout && index && setup("graph-input-links"));
    collections[0].role=GD_SKILL; collections[0].alias="atoms";
    collections[1].role=GD_CHALLENGE; collections[1].alias="steps";
    collections[2].role=GD_RELATED; collections[2].alias="context";
    TEST_ASSERT(write_record(0,"jump","---\nstatus: proposed\n---\nJump."));
    TEST_ASSERT(write_record(1,"encounter","---\nentry: true\nrequires: [atoms/jump]\nmechanics: [context/rule]\ninteractions: [context/contact]\nnouns: [context/noun]\nverbs: [context/verb]\nresources: [context/energy]\n---\nEncounter."));
    TEST_ASSERT(write_record(2,"rule","---\nmechanics: [context/rule]\n---\nSelf link."));
    TEST_ASSERT(write_record(2,"contact","Contact.")); TEST_ASSERT(write_record(2,"noun","Noun."));
    TEST_ASSERT(write_record(2,"verb","Verb.")); TEST_ASSERT(write_record(2,"start","Start.")); TEST_ASSERT(write_record(2,"limit","Limit."));
    TEST_ASSERT(write_record(2,"energy","---\nnoun: context/noun\ninitial_control: context/start\ncapacity_control: context/limit\n---\nEnergy."));
    TEST_ASSERT(gd_load(model,collections,3) && model->edge_count==10);
    encounter=find_record(model,"steps/encounter"); energy=find_record(model,"context/energy"); jump=find_record(model,"atoms/jump");
    gd_index_links(model,index);
    TEST_ASSERT(encounter>=0 && energy>=0 && jump>=0 && index->outgoing[jump]==-1);
    TEST_ASSERT(!strcmp(model->records[energy].resource_noun,"context/noun"));
    TEST_ASSERT(gd_focus_reset(model,&focus,encounter) && gd_focus_layout(model,&focus,layout,GD_SORT_NAME));
    TEST_ASSERT(layout->node_count==7 && layout->edge_count==8 && focus.boundary_count==2);
    TEST_ASSERT(gd_focus_expand(model,&focus,energy) && gd_focus_layout(model,&focus,layout,GD_SORT_NAME));
    TEST_ASSERT(layout->node_count==9 && layout->edge_count==10 && !focus.boundary_count);
    draft=gd_draft_create(model); TEST_ASSERT(draft);
    TEST_ASSERT(gd_draft_prerequisite(draft,jump,"atoms/jump",1)==1);
    gd_index_links(draft->model,index);
    TEST_ASSERT(index->outgoing[jump]>=0 && draft->model->edges[index->outgoing[jump]].field==GD_PREREQUISITES);
    TEST_ASSERT(gd_draft_undo(draft)==1); gd_index_links(draft->model,index);
    TEST_ASSERT(index->outgoing[jump]==-1);
    TEST_ASSERT(gd_focus_reset(draft->model,&focus,energy) && gd_focus_layout(draft->model,&focus,layout,GD_SORT_NAME));
    TEST_ASSERT(focus.visible[find_record(model,"context/start")] && focus.visible[find_record(model,"context/limit")]);
    TEST_ASSERT(!gd_sources_changed(model)); gd_draft_destroy(draft); release(storage,index); release(storage,layout); gd_destroy(model);
)
TEST(resource_input_links,
    GD_Model *model=model_create(); GD_Focus focus; GD_Layout *layout=allocate(storage,sizeof(*layout)); int resource;
    const char *fields[]={"noun","initial_control","capacity_control"};
    const char *invalid[]={"[]","{}","true","42","\"\"","[controls/start]"}; int f,v;
    TEST_ASSERT(model && layout && setup("resource-input-links")); collections[1].role=GD_RESOURCE; collections[2].role=GD_RULE;
    TEST_ASSERT(write_record(0,"coin","Coin.")); TEST_ASSERT(write_record(2,"source","Rule source."));
    TEST_ASSERT(write_record(1,"wallet","---\nnoun: things/coin\ninitial_control: controls/start\ncapacity_control: controls/limit\n---\nWallet."));
    TEST_ASSERT(gd_load(model,collections,3) && model->edge_count==3);
    resource=find_record(model,"actions/wallet"); TEST_ASSERT(resource>=0);
    TEST_ASSERT(!strcmp(model->records[resource].resource_noun,"things/coin"));
    TEST_ASSERT(model->edges[0].field==GD_RESOURCE_NOUN && model->edges[0].line==2);
    TEST_ASSERT(model->edges[1].field==GD_INITIAL_CONTROL && model->edges[1].line==3 && model->edges[1].target==-1);
    TEST_ASSERT(model->edges[2].field==GD_CAPACITY_CONTROL && model->edges[2].line==4 && model->edges[2].target==-1);
    TEST_ASSERT(!strcmp(gd_field_name(GD_RESOURCE_NOUN),"noun") && !strcmp(gd_field_name(GD_CAPACITY_CONTROL),"capacity_control"));
    TEST_ASSERT(gd_focus_reset(model,&focus,resource) && gd_focus_layout(model,&focus,layout,GD_SORT_NAME));
    TEST_ASSERT(layout->node_count==2 && layout->edge_count==1 && focus.boundary_count==2);
    TEST_ASSERT(!gd_sources_changed(model)); gd_destroy(model);
    for (f=0;f<3;++f) for (v=0;v<6;++v) {
        char text[512]; model=model_create(); TEST_ASSERT(model);
        snprintf(text,sizeof(text),"---\n%s: %s\n---\nInvalid.",fields[f],invalid[v]);
        TEST_ASSERT(write_record(1,"wallet",text));
        TEST_ASSERT(!gd_load(model,collections,3) && model->record_count==2 && !model->edge_count);
        gd_destroy(model);
    }
    model=model_create(); TEST_ASSERT(model);
    TEST_ASSERT(write_record(1,"wallet","---\nnoun: null\ninitial_control: null\ncapacity_control: null\n---\nUnspecified."));
    TEST_ASSERT(gd_load(model,collections,3) && !model->edge_count);
    release(storage,layout); gd_destroy(model);
)
TEST(focused_relationships,
    GD_Model *model=model_create(); GD_Focus focus; int i;
    GD_Layout *layout=allocate(storage,sizeof(*layout)); GD_LinkIndex *index=allocate(storage,sizeof(*index));
    const char *references[]={"things/incoming","skills/root","mechanics/neighbor","cases/farther","things/hidden","cases/isolated"};
    TEST_ASSERT(model && layout && index);
    model->record_count=6; model->edge_count=7;
    for (i=0;i<6;++i) { model->records[i].reference=references[i]; model->records[i].status=i%2 ? "rejected":"accepted"; }
    model->edges[0]=(GD_Edge){0,1,GD_MECHANICS,references[1],10};
    model->edges[1]=(GD_Edge){1,2,GD_MECHANICS,references[2],11};
    model->edges[2]=(GD_Edge){2,3,GD_MECHANICS,references[3],12};
    model->edges[3]=model->edges[1];
    model->edges[4]=(GD_Edge){1,1,GD_PREREQUISITES,references[1],14};
    model->edges[5]=(GD_Edge){1,-1,GD_MECHANICS,"missing/unknown",15};
    model->edges[6]=(GD_Edge){4,5,GD_MECHANICS,references[5],16};
    gd_index_links(model,index);
    TEST_ASSERT(index->outgoing[1]==1 && index->next_outgoing[1]==3 && index->next_outgoing[3]==4);
    TEST_ASSERT(index->next_outgoing[4]==5 && index->next_outgoing[5]==-1);
    TEST_ASSERT(index->incoming[1]==0 && index->next_incoming[0]==4 && index->next_incoming[4]==-1);
    TEST_ASSERT(index->next_incoming[5]==-1);
    TEST_ASSERT(gd_focus_reset(model,&focus,1) && gd_focus_layout(model,&focus,layout,GD_SORT_NAME));
    TEST_ASSERT(layout->node_count==3 && layout->edge_count==4 && focus.boundary_count==2);
    TEST_ASSERT(focus.visible[0] && focus.visible[1] && focus.visible[2] && !focus.visible[3] && !focus.visible[4] && !focus.visible[5]);
    TEST_ASSERT(focus.expanded[1] && !focus.expanded[0] && !focus.expanded[2]);
    TEST_ASSERT(layout->edges[0].key==0 && layout->edges[1].key==1 && layout->edges[2].key==3 && layout->edges[3].key==4);
    TEST_ASSERT(layout->nodes[layout->edges[0].a].key==0 && layout->nodes[layout->edges[0].b].key==1);
    TEST_ASSERT(gd_focus_expand(model,&focus,2) && gd_focus_layout(model,&focus,layout,GD_SORT_PARTNERS));
    TEST_ASSERT(layout->node_count==4 && layout->edge_count==5 && focus.boundary_count==1);
    TEST_ASSERT(gd_focus_reset(model,&focus,1));
    model->edges[1].target=5; model->edges[1].reference=references[5]; gd_index_links(model,index);
    TEST_ASSERT(gd_focus_layout(model,&focus,layout,GD_SORT_NAME) && focus.visible[5] && !focus.visible[4] && !focus.visible[3]);
    model->edges[1].target=-1; gd_index_links(model,index);
    TEST_ASSERT(gd_focus_layout(model,&focus,layout,GD_SORT_NAME) && focus.visible[5]); /* Retain revealed isolate. */
    TEST_ASSERT(!gd_focus_reset(model,&focus,6) && focus.root==1);
    TEST_ASSERT(!gd_focus_expand(model,&focus,-1));
    TEST_ASSERT(gd_focus_reset(model,&focus,-1) && gd_focus_layout(model,&focus,layout,GD_SORT_NAME));
    TEST_ASSERT(!layout->node_count && !layout->edge_count && !focus.boundary_count);
    release(storage,index); release(storage,layout); gd_destroy(model);
)
TEST(focused_relationship_capacity,
    GD_Model *model=model_create(); GD_Focus focus; int i;
    GD_Layout *layout=allocate(storage,sizeof(*layout)); GD_LinkIndex *index=allocate(storage,sizeof(*index));
    TEST_ASSERT(model && layout && index);
    model->record_count=GD_MAX_RECORDS; model->edge_count=GD_MAX_EDGES;
    for (i=0;i<GD_MAX_RECORDS;++i) model->records[i].reference="same visible label; distinct record indices";
    for (i=0;i<GD_MAX_EDGES;++i) model->edges[i]=(GD_Edge){0,i%GD_MAX_RECORDS,GD_MECHANICS,"reference",i};
    gd_index_links(model,index);
    { int outgoing=0, incoming=0, edge;
      for (edge=index->outgoing[0];edge>=0;edge=index->next_outgoing[edge]) ++outgoing;
      for (edge=index->incoming[0];edge>=0;edge=index->next_incoming[edge]) ++incoming;
      TEST_ASSERT(outgoing==GD_MAX_EDGES && incoming==GD_MAX_EDGES/GD_MAX_RECORDS);
    }
    TEST_ASSERT(gd_focus_reset(model,&focus,0) && gd_focus_layout(model,&focus,layout,GD_SORT_PARTNERS));
    TEST_ASSERT(layout->node_count==GD_MAX_RECORDS && layout->edge_count==GD_MAX_EDGES && !focus.boundary_count);
    for (i=0;i<layout->edge_count;++i) TEST_ASSERT(layout->edges[i].key==i);
    gd_layout_step(layout,-1);
    for (i=0;i<layout->node_count;++i) TEST_ASSERT(isfinite(layout->nodes[i].x) && isfinite(layout->nodes[i].y));
    release(storage,index); release(storage,layout); gd_destroy(model);
)
TEST(diagnostic_projection,
    GD_Model *model=model_create(); GD_DiagnosticItem item;
    GD_Finding findings[2]={{"skill.example","Investigate declared prerequisites","prerequisites","atoms/b",0,7},{"mda.question","Unbound question",NULL,NULL,-1,0}};
    GD_DiagnosticSet set;
    TEST_ASSERT(model);
    model->record_count=2; model->records[0].reference="atoms/a"; model->records[0].source_path="design/a.md";
    model->records[1].reference="atoms/b"; model->records[1].source_path="design/b.md";
    gd_error(model,"operation.example","Broken operation","design/b.md",9);
    set=(GD_DiagnosticSet){model,findings,2,3,NULL};
    TEST_ASSERT(gd_diagnostic_count(&set)==3 && gd_diagnostic_item(&set,0,&item));
    TEST_ASSERT(item.kind==GD_DIAGNOSTIC_FINDING && item.record==0 && item.line==7 && !strcmp(item.path,"design/a.md"));
    TEST_ASSERT(gd_diagnostic_matches(&item,GD_DIAGNOSTIC_ALL,"PREREQUISITES",-1));
    TEST_ASSERT(gd_diagnostic_matches(&item,GD_DIAGNOSTIC_FINDING,"ATOMS/B",0));
    TEST_ASSERT(gd_diagnostic_matches(&item,GD_DIAGNOSTIC_ALL,"DESIGN/A.MD",-1));
    TEST_ASSERT(!gd_diagnostic_matches(&item,GD_DIAGNOSTIC_ERROR,"",-1) && !gd_diagnostic_matches(&item,GD_DIAGNOSTIC_ALL,"",-2));
    TEST_ASSERT(!gd_diagnostic_matches(&item,GD_DIAGNOSTIC_ALL,"",1));
    TEST_ASSERT(gd_diagnostic_item(&set,1,&item) && item.record==-1 && !item.path && !item.record_reference);
    TEST_ASSERT(gd_diagnostic_item(&set,2,&item) && item.kind==GD_DIAGNOSTIC_ERROR && item.record==1 && item.line==9);
    model->records[0].source_path="design/b.md";
    TEST_ASSERT(gd_diagnostic_item(&set,2,&item) && item.record==-1);
    TEST_ASSERT(!gd_diagnostic_item(&set,-1,&item) && !gd_diagnostic_item(&set,3,&item));
    set.finding_count=0; model->error_count=0;
    TEST_ASSERT(!gd_diagnostic_count(&set));
    gd_destroy(model);
)
TEST(diagnostic_retention,
    GD_Model *model=model_create(); GD_DiagnosticErrors *snapshot; GD_DiagnosticItem item; GD_DiagnosticSet set;
    GD_Error errors[GD_MAX_ERRORS+1]; char message[4100], path[64]="design/rejected.md", code[140]; int i;
    TEST_ASSERT(model);
    memset(message,'x',4094); memcpy(message+4094,"\xf0\x9f\x98\x80",5);
    memset(code,'a',sizeof(code)-1); code[sizeof(code)-1]=0;
    for (i=0;i<GD_MAX_ERRORS+1;++i) errors[i]=(GD_Error){code,message,path,42};
    snapshot=gd_diagnostics_capture(model->allocator,errors,GD_MAX_ERRORS+1,2);
    TEST_ASSERT(snapshot && snapshot->count==GD_MAX_ERRORS && snapshot->omitted==3 && snapshot->truncated[0]);
    TEST_ASSERT(strlen(snapshot->errors[0].code)==128 && strlen(snapshot->errors[0].message)==4094);
    TEST_ASSERT(gd_valid_utf8((const unsigned char *)snapshot->errors[0].message,4094));
    message[0]='z'; code[0]='b'; path[0]='X';
    TEST_ASSERT(snapshot->errors[0].message[0]=='x' && snapshot->errors[0].code[0]=='a' && !strcmp(snapshot->errors[0].path,"design/rejected.md"));
    model->record_count=1; model->records[0].reference="atoms/old"; model->records[0].source_path="design/rejected.md";
    set=(GD_DiagnosticSet){model,NULL,0,0,snapshot};
    TEST_ASSERT(gd_diagnostic_count(&set)==GD_MAX_ERRORS && gd_diagnostic_item(&set,0,&item) && item.kind==GD_DIAGNOSTIC_RELOAD && item.record==0 && item.truncated && item.line==42);
    for (i=0;i<GD_MAX_ERRORS;++i) {
        model->findings[i]=(GD_Finding){"question","Retained domain question",NULL,NULL,0,0};
        gd_error(model,"operation","Retained error",NULL,0);
    }
    set.findings=model->findings; set.finding_count=GD_MAX_ERRORS;
    TEST_ASSERT(gd_diagnostic_count(&set)==3*GD_MAX_ERRORS && gd_diagnostic_item(&set,3*GD_MAX_ERRORS-1,&item) && item.kind==GD_DIAGNOSTIC_RELOAD);
    TEST_ASSERT(gd_diagnostic_item(&set,GD_MAX_ERRORS,&item) && item.kind==GD_DIAGNOSTIC_ERROR);
    set.findings=NULL; set.finding_count=0; model->error_count=0;
    gd_diagnostics_destroy(snapshot);
    {
        char long_path[4100], prefix[GD_MAX_PATH];
        memset(long_path,'p',sizeof(long_path)-1); long_path[sizeof(long_path)-1]=0;
        memcpy(prefix,long_path,GD_MAX_PATH-1); prefix[GD_MAX_PATH-1]=0;
        errors[0]=(GD_Error){"input.path","Path too long",long_path,0};
        snapshot=gd_diagnostics_capture(model->allocator,errors,1,0);
        TEST_ASSERT(snapshot && snapshot->path_truncated[0]);
        model->records[0].source_path=prefix; set.rejected=snapshot;
        TEST_ASSERT(gd_diagnostic_item(&set,0,&item) && item.record==-1); /* Shortened path is not an identity. */
        gd_diagnostics_destroy(snapshot);
    }
    snapshot=gd_diagnostics_capture(model->allocator,NULL,0,INT_MAX);
    TEST_ASSERT(snapshot && snapshot->omitted==INT_MAX && snapshot->count==1 && !strcmp(snapshot->errors[0].code,"view.reload") && !snapshot->errors[0].path && !snapshot->truncated[0]);
    gd_diagnostics_destroy(snapshot);
    TEST_ASSERT(!gd_diagnostics_capture(model->allocator,errors,GD_MAX_ERRORS+1,INT_MAX));
    {
        RouteFailure failure; GD_Allocator allocator=model->allocator;
        failure.parent=allocator; failure.remaining=0; allocator.context=&failure; allocator.allocate=route_allocate; allocator.release=route_release;
        TEST_ASSERT(!gd_diagnostics_capture(allocator,errors,1,0));
    }
    gd_destroy(model);
)
TEST(rules_allocation_failure,
    GD_Model *model = model_create();
    RouteFailure failure;
    GD_Allocator allocator;
    TEST_ASSERT(model);
    allocator = model->allocator;
    failure.parent = allocator; failure.remaining = 0;
    model->allocator.context = &failure; model->allocator.allocate = route_allocate; model->allocator.release = route_release;
    TEST_ASSERT(!gd_read_rules(model) && has_error(model,"rules.memory"));
    model->allocator = allocator;
    {
        GD_Rules *rules=gd_read_rules(model);
        TEST_ASSERT(rules);
        model->allocator.context=&failure; model->allocator.allocate=route_allocate; model->allocator.release=route_release;
        TEST_ASSERT(!gd_analyze_rules(model,rules) && has_error(model,"rules.analysis_memory"));
        model->allocator=allocator; gd_rules_destroy(rules);
    }
    gd_destroy(model);
)
#include "test_graph_closure.h"
#include "test_graph_drawing.h"
#include "test_layout_focus.h"
#include "test_layout_interaction.h"
#include "test_matrix_grid.h"
#include "test_resource_output.h"
#include "test_graph_reachability.h"
#include "test_graph_shapes.h"
#include "test_graph_overview.h"
#include "test_explorer.h"
int main(int argc, char **argv) {
    int failed = 0;
    if (argc != 2 || strlen(argv[1]) >= sizeof(root)) { fputs("Usage: test-core NEW-FIXTURE-DIRECTORY\n", stderr); return 2; }
    strcpy(root, argv[1]);
    { char *p; for (p = root; *p; ++p) if (*p == '\\') *p = '/'; }
    if (!file_directory_create(root, 1)) { fputs("Fixture directory must be new\n", stderr); return 2; }
    storage = free_list_from_buffer(backing, sizeof(backing));
    if (!storage) return 2;
    failed += test_rule_outcomes();
    failed += test_rule_quantities(); failed += test_rule_numeric_comparisons();
    failed += test_rule_condition_logic(); failed += test_rule_exclusion_limits(); failed += test_rule_effect_bindings();
    failed += test_rule_axis_definitions(); failed += test_rule_axis_errors(); failed += test_rule_axis_references(); failed += test_rule_axis_incomplete(); failed += test_rule_axis_limits();
    failed += test_rules_declarations(); failed += test_resource_unloaded_call(); failed += test_rule_expression_delta(); failed += test_rule_pool_resolution(); failed += test_rules_fence_scope(); failed += test_rules_invalid_records(); failed += test_rules_limits(); failed += test_rules_allocation_failure();
    failed += test_matrix_grid(); failed += test_resource_output_limits();
    failed += test_empty(); failed += test_matrix(); failed += test_unsafe_input(); failed += test_ambiguous_endpoints();
    failed += test_text_types(); failed += test_missing_endpoint();
    failed += test_input_limits();
    failed += test_component_chain(); failed += test_layout_modes(); failed += test_layout_capacity();
    failed += test_skill_unlock_limits(); failed += test_skill_unlocks(); failed += test_skill_chain(); failed += test_skill_incomplete(); failed += test_skill_rank_types(); failed += test_skill_capacity();
    failed += test_draft_ranks(); failed += test_draft_connections(); failed += test_draft_limits(); failed += test_draft_report();
    failed += test_routes_semantics(); failed += test_routes_types_unknown(); failed += test_routes_walk_oracle(); failed += test_routes_limits();
    failed += test_mda_traces(); failed += test_mda_resolution(); failed += test_mda_types(); failed += test_mda_oracle(); failed += test_mda_limits();
    failed += test_control_units_numbers(); failed += test_control_formulas(); failed += test_control_reference_syntax(); failed += test_control_errors();
    failed += test_control_loading(); failed += test_control_overrides_scope(); failed += test_control_selection(); failed += test_control_limits();
    failed += test_control_import_roundtrip(); failed += test_control_import_legacy_dependent(); failed += test_control_import_strings(); failed += test_control_import_failures();
    failed += test_skill_progression_modes(); failed += test_route_layout_capacity();
    failed += test_graph_analysis_projection(); failed += test_graph_analysis_roles(); failed += test_graph_comparison(); failed += test_graph_paths(); failed += test_graph_path_limits(); failed += test_graph_bypass_ordering(); failed += test_graph_analysis_limits();
    failed += test_graph_overview_oracle(); failed += test_graph_overview_limits();
    failed += test_explorer_loading(); failed += test_explorer_allocation_failures();
    failed += test_graph_shapes_oracle(); failed += test_graph_shapes_limits();
    failed += test_typed_reachability_controls();
    failed += test_layout_settling(); failed += test_layout_distance_focus(); failed += test_layout_interaction(); failed += test_directed_drawing_order(); failed += test_directed_drawing_routes();
    failed += test_graph_closure_oracle(); failed += test_graph_closure_limits(); failed += test_graph_foundations_adapter();
    failed += test_graph_input_links(); failed += test_resource_input_links();
    failed += test_focused_relationships(); failed += test_focused_relationship_capacity();
    failed += test_diagnostic_projection(); failed += test_diagnostic_retention();
    printf("native domain/layout/draft/route/MDA/control/rule checks: %d/92 passed\n", 92-failed);
    return failed ? 1 : 0;
}
