#include "gd_actor_query.h"
TEST(actor_invalid_quantities_and_distinct_preservation_declarations,
    GD_Model *model=model_create(); GD_Rules *rules; GD_RuleAnalysis *analysis; GD_ActorQuery *query;
    GD_ActorSelection selection={0}; int root,axis,i,quantities=0,preservations=0,unknown_states=0,unit_error=0,arithmetic_error=0;
    TEST_ASSERT(model && setup("actor-invalid-quantity")); collections[1].role=GD_AXIS; collections[2].role=GD_RULE;
    TEST_ASSERT(write_record(1,"Points","---\nvalue_type: integer\nunit: point\n---\n"));
    TEST_ASSERT(write_record(0,"unit","---\naxes: [actions/Points]\n---\n"));
    TEST_ASSERT(write_record(2,"rules","```gd-rule\ncase invalid:\n    target: A\n    context: ANY\n    A:\n        noun: things/unit\n        requires: ANY\n        becomes: Phantom.one, Phantom.two\n        preserve: Points, Points\n        set:\n            Points: 5[second]\n            Points: this / 0\n```\n"));
    TEST_ASSERT(gd_load(model,collections,3)); rules=gd_read_rules(model); analysis=gd_analyze_rules(model,rules);
    TEST_ASSERT(rules && analysis); root=gd_rule_case_find(model,rules,"cases/rules#invalid"); axis=find_record(model,"actions/Points"); TEST_ASSERT(root>=0 && axis>=0);
    selection.case_index=root; selection.participant=-1; selection.axis=axis; selection.value_count=1; selection.values[0]="0";
    query=gd_actor_query_build(model,analysis,&selection,GD_ACTOR_WORK);
    TEST_ASSERT(query && query->variant_count==1 && !query->limited && query->write_count==6 && query->effect_count==6);
    for (i=0;i<query->write_count;++i) {
        const GD_RuleWrite *write=&query->writes[i];
        if (write->kind==GD_WRITE_PRESERVE) ++preservations;
        else if (write->kind==GD_WRITE_STATE) { ++unknown_states; TEST_ASSERT(gd_actor_write_family(query,0,write)==GD_EFFECT_UNKNOWN); }
        else {
            ++quantities; unit_error+=write->value.state==GD_EXPRESSION_UNIT_ERROR; arithmetic_error+=write->value.state==GD_EXPRESSION_ARITHMETIC_ERROR;
            TEST_ASSERT(gd_actor_write_family(query,0,write)==GD_EFFECT_QUANTITY);
        }
    }
    TEST_ASSERT(quantities==2 && preservations==2 && unknown_states==2 && unit_error==1 && arithmetic_error==1);
    TEST_ASSERT(query->writes[0].declaration!=query->writes[1].declaration && query->writes[4].declaration!=query->writes[5].declaration && !query->variants[0].constraints_complete);
    gd_actor_query_destroy(query); gd_rule_analysis_destroy(analysis); gd_rules_destroy(rules); gd_destroy(model);
)
