#include "gd_actor_query.h"
TEST(actor_open_values_and_parallel_preservation_provenance,
    GD_Model *model=model_create(); GD_Rules *rules; GD_RuleAnalysis *analysis; GD_ActorQuery *query;
    GD_ActorSelection selection={0}; GD_ActorComparison comparison; int root,axis,i;
    TEST_ASSERT(model && setup("actor-provenance")); collections[1].role=GD_AXIS; collections[2].role=GD_RULE;
    TEST_ASSERT(write_record(1,"Flag","---\nvalue_type: enum\nallowed_values: [known]\nclosed: false\n---\n"));
    TEST_ASSERT(write_record(0,"unit","---\naxes: [actions/Flag]\n---\n"));
    TEST_ASSERT(write_record(2,"rules","```gd-rule\ncase root:\n    target: A\n    context: ANY\n    A:\n        noun: things/unit\n        requires: ANY\n        causes: cases/rules#keep with A=A; cases/rules#keep with A=A\ncase keep:\n    target: A\n    context: ANY\n    A:\n        noun: things/unit\n        requires: ANY\n        preserve: ALL\n```\n"));
    TEST_ASSERT(gd_load(model,collections,3)); rules=gd_read_rules(model); analysis=gd_analyze_rules(model,rules);
    TEST_ASSERT(rules && analysis && !model->error_count);
    root=gd_rule_case_find(model,rules,"cases/rules#root"); axis=find_record(model,"actions/Flag"); TEST_ASSERT(root>=0 && axis>=0);
    selection.case_index=root; selection.participant=-1; selection.axis=axis; selection.value_count=2;
    selection.values[0]="future"; selection.values[1]="novel";
    query=gd_actor_query_build(model,analysis,&selection,GD_ACTOR_WORK);
    TEST_ASSERT(query && !query->limited && query->variant_count==2 && !query->vocabulary_closed && query->write_count==4);
    for (i=0;i<2;++i) {
        GD_ActorVariant *variant=&query->variants[i]; int first=variant->first_write,one=query->write_activations[first],two=query->write_activations[first+1];
        TEST_ASSERT(variant->write_count==2 && variant->activation_count==3 && variant->constraints_complete && variant->values_complete);
        TEST_ASSERT(!strcmp(query->writes[first].state,selection.values[i]) && !strcmp(query->writes[first+1].state,selection.values[i]));
        TEST_ASSERT(one!=two && one>=variant->first_activation && two<variant->first_activation+variant->activation_count);
        TEST_ASSERT(query->activations[one].calls[1]!=query->activations[two].calls[1]);
    }
    TEST_ASSERT(gd_actor_compare(query,0,1,&comparison) && comparison.complete && comparison.effects.shared==2);
    gd_actor_query_destroy(query);
    selection.participant=-2;
    TEST_ASSERT(!gd_actor_query_build(model,analysis,&selection,GD_ACTOR_WORK) && !strcmp(model->errors[model->error_count-1].code,"actors.binding"));
    gd_rule_analysis_destroy(analysis); gd_rules_destroy(rules); gd_destroy(model);
)
