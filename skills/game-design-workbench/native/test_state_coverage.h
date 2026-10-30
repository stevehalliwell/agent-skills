#include "gd_query.h"
TEST(state_coverage_whole_cases_local_hypotheses_and_limits,
    GD_Model *model=model_create(); GD_Rules *rules; GD_RuleAnalysis *analysis; GD_Query *query; GD_QueryRequest request={0}; int i;
    TEST_ASSERT(model && setup("state-coverage")); collections[1].role=GD_AXIS; collections[2].role=GD_RULE;
    TEST_ASSERT(write_record(1,"Form","---\nvalue_type: enum\nallowed_values: [small, big, unused]\nclosed: true\n---\n"));
    TEST_ASSERT(write_record(0,"unit","---\naxes: [actions/Form]\n---\n"));
    TEST_ASSERT(write_record(2,"rules","---\nstatus: accepted\n---\n```gd-rule\n"
        "case small:\n    target: A\n    context: ANY\n    A:\n        noun: things/unit\n        requires: Form.small\n        other_axes: ANY\n        becomes: Form.unused\n"
        "case pair:\n    target: B\n    context: ANY\n    A:\n        noun: things/unit\n        requires: Form.big\n        other_axes: ANY\n    B:\n        noun: things/unit\n        requires: Form.big\n        other_axes: ANY\n```\n"));
    TEST_ASSERT(gd_load(model,collections,3)); rules=gd_read_rules(model); analysis=gd_analyze_rules(model,rules);
    TEST_ASSERT(rules && analysis && !model->error_count);
    request.kind=GD_QUERY_STATE_COVERAGE; request.case_index=request.noun=request.effect_kind=-1; request.axis=find_record(model,"actions/Form"); request.work_limit=GD_ACTOR_WORK;
    query=gd_query_build(model,rules,analysis,&request);
    TEST_ASSERT(query && query->state_coverage && !query->state_coverage->limited && !query->state_coverage->source_incomplete && query->binding_count==3);
    TEST_ASSERT(query->state_coverage->value_count==3 && query->state_coverage->evaluated_bindings==3);
    TEST_ASSERT(query->state_coverage->values[0].definite_bindings==1 && query->state_coverage->values[0].definite_cases==1 && query->state_coverage->values[0].excluded_cases==1);
    TEST_ASSERT(query->state_coverage->values[1].undetermined_bindings==2 && query->state_coverage->values[1].undetermined_cases==1 && !query->state_coverage->values[1].definite_cases);
    TEST_ASSERT(query->state_coverage->values[2].excluded_bindings==3 && query->state_coverage->values[2].excluded_cases==2 && query->state_coverage->values[2].no_retained_candidate==GD_RULE_TRUTH_TRUE);
    TEST_ASSERT(query->state_coverage->values[0].no_retained_candidate==GD_RULE_TRUTH_FALSE && query->state_coverage->values[1].no_retained_candidate==GD_RULE_TRUTH_FALSE);
    TEST_ASSERT(query->state_coverage->work<=request.work_limit-query->work);
    printf("finite coverage owned storage: %llu bytes; query adapter: %llu bytes\n",(unsigned long long)sizeof(*query->state_coverage),(unsigned long long)sizeof(*query));
    gd_query_destroy(query);
    {
        GD_Allocator allocator=model->allocator; RouteFailure failure; int before=model->error_count;
        for (i=0;i<3;++i) {
            failure=(RouteFailure){allocator,i}; model->allocator=(GD_Allocator){&failure,route_allocate,route_release};
            TEST_ASSERT(!gd_query_build(model,rules,analysis,&request)); model->allocator=allocator;
            TEST_ASSERT(model->error_count>before); model->error_count=before;
        }
    }
    request.work_limit=1024; query=gd_query_build(model,rules,analysis,&request);
    TEST_ASSERT(query && query->state_coverage->limited && query->binding_count==3);
    for (i=0;i<3;++i) TEST_ASSERT(query->state_coverage->values[i].unfinished_bindings==3 && query->state_coverage->values[i].no_retained_candidate==GD_RULE_TRUTH_UNKNOWN && !query->state_coverage->values[i].definite_cases && !query->state_coverage->values[i].undetermined_cases);
    gd_query_destroy(query);
    request.work_limit=0; query=gd_query_build(model,rules,analysis,&request);
    TEST_ASSERT(query && query->limited && !query->binding_count && query->state_coverage->values[2].no_retained_candidate==GD_RULE_TRUTH_UNKNOWN); gd_query_destroy(query);
    request.work_limit=GD_ACTOR_WORK; request.status_count=1; request.statuses[0]="absent"; query=gd_query_build(model,rules,analysis,&request);
    TEST_ASSERT(query && !query->binding_count && !query->limited && query->state_coverage->values[2].no_retained_candidate==GD_RULE_TRUTH_TRUE); gd_query_destroy(query);
    request.status_count=0; rules->bindings_complete=0; query=gd_query_build(model,rules,analysis,&request);
    TEST_ASSERT(query && query->state_coverage->source_incomplete && query->state_coverage->values[2].no_retained_candidate==GD_RULE_TRUTH_UNKNOWN); gd_query_destroy(query);
    gd_rule_analysis_destroy(analysis); gd_rules_destroy(rules); gd_destroy(model);
)
