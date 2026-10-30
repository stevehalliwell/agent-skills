#include "gd_actor_query.h"
TEST(actor_variant_capacity_inputs_and_allocation,
    GD_Model *model=model_create(); GD_Rules *rules; GD_RuleAnalysis *analysis; GD_ActorQuery *query; GD_ActorSelection selection={0};
    GD_RuleScenario fixed={0}; GD_Allocator allocator; RouteFailure failure;
    char metadata[2048],body[8192]; size_t used=0,at=0; int i,root,axis;
    TEST_ASSERT(model && setup("actor-limits")); collections[1].role=GD_AXIS; collections[2].role=GD_RULE;
    at+=(size_t)snprintf(metadata+at,sizeof(metadata)-at,"---\nvalue_type: enum\nclosed: true\nallowed_values: [");
    for (i=0;i<64;++i) at+=(size_t)snprintf(metadata+at,sizeof(metadata)-at,"%sv%d",i ? ", ":"",i);
    TEST_ASSERT(at<sizeof(metadata) && snprintf(metadata+at,sizeof(metadata)-at,"]\n---\n")>0);
    TEST_ASSERT(write_record(1,"Form",metadata)); TEST_ASSERT(write_record(0,"unit","---\naxes: [actions/Form]\n---\n"));
    used+=(size_t)snprintf(body+used,sizeof(body)-used,"```gd-rule\ncase root:\n    target: A\n    context: ANY\n    A:\n        noun: things/unit\n        requires: ANY\n        causes: ");
    for (i=0;i<64;++i) used+=(size_t)snprintf(body+used,sizeof(body)-used,"%scases/rules#leaf with Victim=A",i ? "; ":"");
    TEST_ASSERT(used<sizeof(body) && snprintf(body+used,sizeof(body)-used,"\ncase leaf:\n    target: Victim\n    context: ANY\n    Victim:\n        noun: things/unit\n        requires: ANY\n```\n")>0);
    TEST_ASSERT(write_record(2,"rules",body)); TEST_ASSERT(gd_load(model,collections,3)); rules=gd_read_rules(model); analysis=gd_analyze_rules(model,rules);
    TEST_ASSERT(rules && analysis && !model->error_count); root=gd_rule_case_find(model,rules,"cases/rules#root"); axis=find_record(model,"actions/Form");
    TEST_ASSERT(root>=0 && axis>=0); selection.case_index=root; selection.participant=-1; selection.axis=axis;
    query=gd_actor_query_build(model,analysis,&selection,GD_ACTOR_WORK);
    TEST_ASSERT(query && query->limited && query->requested==64 && query->variant_count==63 && query->unexamined_variants==1);
    TEST_ASSERT(query->activation_count==4095 && !query->write_count && query->work<=GD_ACTOR_WORK);
    for (i=0;i<63;++i) TEST_ASSERT(query->variants[i].activation_count==65 && !query->variants[i].limited);
    for (i=2;i<65;++i) TEST_ASSERT(query->activations[i].calls[1]!=query->activations[i-1].calls[1]);
    printf("actor query owned storage: %llu bytes; 63/64 variants retained at activation capacity\n",(unsigned long long)sizeof(*query));
    gd_actor_query_destroy(query);
    allocator=model->allocator; failure=(RouteFailure){allocator,0}; model->allocator=(GD_Allocator){&failure,route_allocate,route_release};
    TEST_ASSERT(!gd_actor_query_build(model,analysis,&selection,GD_ACTOR_WORK)); model->allocator=allocator;
    TEST_ASSERT(!strcmp(model->errors[model->error_count-1].code,"actors.memory"));
    selection.value_count=2; selection.values[0]="v0"; selection.values[1]="v0";
    TEST_ASSERT(!gd_actor_query_build(model,analysis,&selection,GD_ACTOR_WORK) && !strcmp(model->errors[model->error_count-1].code,"actors.duplicate_value"));
    selection.value_count=1; selection.values[0]="outside";
    TEST_ASSERT(!gd_actor_query_build(model,analysis,&selection,GD_ACTOR_WORK));
    selection.values[0]="v1";
    TEST_ASSERT(gd_rule_assume(model,rules,root,"A:actions/Form=v0",&fixed)); selection.fixed=&fixed;
    TEST_ASSERT(!gd_actor_query_build(model,analysis,&selection,GD_ACTOR_WORK) && !strcmp(model->errors[model->error_count-1].code,"actors.fixed_axis"));
    selection.fixed=NULL; selection.participant=rules->cases[gd_rule_case_find(model,rules,"cases/rules#leaf")].first_participant;
    TEST_ASSERT(!gd_actor_query_build(model,analysis,&selection,GD_ACTOR_WORK) && !strcmp(model->errors[model->error_count-1].code,"actors.binding"));
    selection.participant=-1;
    TEST_ASSERT(!gd_actor_query_build(model,analysis,&selection,-1) && !gd_actor_query_build(model,analysis,&selection,GD_ACTOR_WORK+1));
    gd_rule_analysis_destroy(analysis); gd_rules_destroy(rules); gd_destroy(model);
)
