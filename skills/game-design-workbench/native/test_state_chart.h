#include "gd_query.h"
TEST(state_chart_retains_native_proposals_and_allocation_failures,
    GD_Model *model=model_create(); GD_Rules *rules; GD_RuleAnalysis *analysis; GD_Query *query;
    GD_QueryRequest request={0}; int i,j,transitions=0;
    TEST_ASSERT(model && setup("state-chart")); collections[1].role=GD_AXIS; collections[2].role=GD_RULE;
    TEST_ASSERT(write_record(1,"State","---\nvalue_type: enum\nallowed_values: [ready, spent]\nclosed: true\n---\n"));
    TEST_ASSERT(write_record(0,"unit","---\naxes: [actions/State]\n---\n"));
    TEST_ASSERT(write_record(2,"rules","```gd-rule\ncase use:\n    target: A\n    context: ANY\n    A:\n        noun: things/unit\n        requires: State.ready\n        other_axes: ANY\n        becomes: State.spent\n    B:\n        noun: things/unit\n        requires: ANY\n        preserve: State\n```\n"));
    TEST_ASSERT(gd_load(model,collections,3)); rules=gd_read_rules(model); analysis=gd_analyze_rules(model,rules);
    TEST_ASSERT(rules && analysis && !model->error_count);
    request.kind=GD_QUERY_STATE_CHART; request.case_index=request.effect_kind=-1; request.noun=find_record(model,"things/unit"); request.axis=find_record(model,"actions/State"); request.work_limit=GD_ACTOR_WORK;
    query=gd_query_build(model,rules,analysis,&request);
    TEST_ASSERT(query && query->state_chart && query->state_coverage && !query->state_coverage->limited && query->binding_count==2 && query->state_chart->scenario_count==4);
    for (i=0;i<query->state_chart->scenario_count;++i) {
        const GD_StateChartScenario *scenario=&query->state_chart->scenarios[i];
        TEST_ASSERT(scenario->binding>=0 && scenario->binding<2 && scenario->activation_count==1);
        for (j=0;j<scenario->write_count;++j) {
            int index=scenario->first_write+j; const GD_RuleWrite *write=&query->state_chart->writes[index];
            TEST_ASSERT(query->state_chart->write_activations[index]==0 && write->origin_case==0);
            if (write->actor==query->bindings[scenario->binding].participant && write->axis==request.axis) ++transitions;
        }
    }
    TEST_ASSERT(transitions==3 && query->state_coverage->work<=request.work_limit-query->work);
    printf("state chart owned storage: %llu bytes; scenarios %d; proposals %d\n",(unsigned long long)sizeof(*query->state_chart),query->state_chart->scenario_count,query->state_chart->write_count);
    gd_query_destroy(query);
    {
        GD_Allocator allocator=model->allocator; RouteFailure failure; int before=model->error_count;
        for (i=0;i<4;++i) {
            failure=(RouteFailure){allocator,i}; model->allocator=(GD_Allocator){&failure,route_allocate,route_release};
            TEST_ASSERT(!gd_query_build(model,rules,analysis,&request)); model->allocator=allocator;
            TEST_ASSERT(model->error_count>before); model->error_count=before;
        }
    }
    request.work_limit=0; query=gd_query_build(model,rules,analysis,&request);
    TEST_ASSERT(query && query->limited && !query->binding_count && !query->state_chart->scenario_count && query->state_coverage->values[0].no_retained_candidate==GD_RULE_TRUTH_UNKNOWN); gd_query_destroy(query);
    request.work_limit=GD_ACTOR_WORK; request.status_count=1; request.statuses[0]="absent"; query=gd_query_build(model,rules,analysis,&request);
    TEST_ASSERT(query && !query->limited && !query->state_coverage->limited && !query->state_chart->scenario_count); gd_query_destroy(query);
    gd_rule_analysis_destroy(analysis); gd_rules_destroy(rules); gd_destroy(model);
)
