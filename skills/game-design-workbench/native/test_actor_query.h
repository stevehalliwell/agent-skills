#include "gd_actor_query.h"
#include "gd_query.h"
#include "gd_query_scene.h"
TEST(actor_finite_variants_and_checked_activation_evidence,
    GD_Model *model=model_create(); GD_Rules *rules; GD_RuleAnalysis *analysis; GD_ActorQuery *query,*numeric,*tiny;
    GD_ActorSelection selection={0}; GD_RuleScenario fixed={0}; GD_ActorComparison comparison;
    int root,actor,other,form,points,i,small=-1,big=-1;
    TEST_ASSERT(model && setup("actor-variants")); collections[1].role=GD_AXIS; collections[2].role=GD_RULE;
    TEST_ASSERT(write_record(1,"Form","---\nvalue_type: enum\nallowed_values: [small, big]\nclosed: true\n---\n"));
    TEST_ASSERT(write_record(1,"Life","---\nvalue_type: enum\nallowed_values: [alive, dead]\nclosed: true\n---\n"));
    TEST_ASSERT(write_record(1,"Points","---\nvalue_type: integer\nunit: point\nminimum: 0\nmaximum: 10\n---\n"));
    TEST_ASSERT(write_record(0,"unit","---\naxes: [actions/Form, actions/Life, actions/Points]\n---\n"));
    TEST_ASSERT(write_record(2,"rules","---\nstatus: accepted\n---\n```gd-rule\n"
        "case root:\n    target: A\n    context: ANY\n    A:\n        noun: things/unit\n        requires: ANY\n        other_axes: ANY\n        causes: cases/rules#small with Victim=A, Other=B; cases/rules#big with Victim=A, Other=B\n    B:\n        noun: things/unit\n        requires: ANY\n        other_axes: ANY\n"
        "case small:\n    target: Victim\n    context: ANY\n    Victim:\n        noun: things/unit\n        requires: Form.small\n        other_axes: ANY\n        becomes: Life.dead\n    Other:\n        noun: things/unit\n        requires: ANY\n        other_axes: ANY\n"
        "case big:\n    target: Victim\n    context: ANY\n    Victim:\n        noun: things/unit\n        requires: Form.big\n        other_axes: ANY\n        set:\n            Points: min(this + 1[point], 10[point])\n    Other:\n        noun: things/unit\n        requires: ANY\n        other_axes: ANY\n        becomes: Life.dead\n```\n"));
    TEST_ASSERT(gd_load(model,collections,3)); rules=gd_read_rules(model); analysis=gd_analyze_rules(model,rules);
    TEST_ASSERT(rules && analysis && !model->error_count);
    root=gd_rule_case_find(model,rules,"cases/rules#root"); TEST_ASSERT(root>=0);
    actor=rules->cases[root].primary_participant; other=actor+1;
    form=find_record(model,"actions/Form"); points=find_record(model,"actions/Points"); TEST_ASSERT(form>=0 && points>=0);
    TEST_ASSERT(gd_rule_assume(model,rules,root,"A:actions/Points=9",&fixed));
    selection.case_index=root; selection.participant=-1; selection.axis=form; selection.fixed=&fixed;
    query=gd_actor_query_build(model,analysis,&selection,GD_ACTOR_WORK);
    TEST_ASSERT(query && !query->limited && query->requested==2 && query->variant_count==2 && query->participant==actor && query->vocabulary_closed);
    for (i=0;i<2;++i) {
        GD_ActorVariant *variant=&query->variants[i]; int j,excluded=0,entered=0;
        TEST_ASSERT(variant->applicability==GD_RULE_TRUTH_TRUE && !variant->constraints_complete && variant->values_complete && !variant->conflicts);
        TEST_ASSERT(variant->activation_count==3 && variant->partners[other/64]&(1ULL<<(other%64)));
        for (j=variant->first_activation;j<variant->first_activation+variant->activation_count;++j) {
            GD_RuleActivation *activation=&query->activations[j];
            excluded+=activation->applicability==GD_RULE_TRUTH_FALSE; entered+=activation->applicability==GD_RULE_TRUTH_TRUE;
            if (activation->path_count==2) TEST_ASSERT(activation->calls[0]==-1 && gd_actor_trace_participant(analysis,activation,rules->cases[activation->case_index].primary_participant)==actor);
        }
        TEST_ASSERT(excluded==1 && entered==2);
        if (!strcmp(variant->value,"small")) small=i; else big=i;
    }
    TEST_ASSERT(small>=0 && big>=0 && query->variants[small].write_count==1 && query->variants[big].write_count==2);
    TEST_ASSERT(gd_actor_compare(query,small,big,&comparison) && comparison.cases.shared==1 && comparison.cases.first_only==1 && comparison.cases.second_only==1);
    TEST_ASSERT(comparison.partners.shared==1 && !comparison.partners.first_only && !comparison.partners.second_only);
    TEST_ASSERT(comparison.effects.shared==1 && !comparison.effects.first_only && comparison.effects.second_only==1); /* Life.dead concept is shared; affected actors remain distinct proposal targets. */
    TEST_ASSERT(!gd_actor_compare(query,-1,big,&comparison));
    {
        GD_QueryRequest request={0}; GD_Query *surface;
        request.kind=GD_QUERY_ACTOR_STATES; request.case_index=root; request.noun=request.effect_kind=-1; request.axis=form; request.fixed=&fixed; request.work_limit=GD_ACTOR_WORK;
        surface=gd_query_build(model,rules,analysis,&request);
        TEST_ASSERT(surface && surface->variants && surface->variants->variant_count==query->variant_count && surface->variants->write_count==query->write_count);
        for (i=0;i<query->write_count;++i) TEST_ASSERT(surface->variants->writes[i].actor==query->writes[i].actor && surface->variants->writes[i].origin_case==query->writes[i].origin_case && surface->variants->writes[i].declaration==query->writes[i].declaration);
        {
            GD_QueryScene *scene=gd_query_scene_build(surface,GD_QUERY_ALL_VARIANTS,0,0,0,GD_EFFECT_WORK); int nodes=0,links=0;
            TEST_ASSERT(scene && !scene->limited && scene->excluded_activations==2);
            for (i=0;i<scene->layout.node_count;++i) { nodes+=scene->nodes[i].kind==GD_QUERY_NODE_VARIANT; TEST_ASSERT(scene->nodes[i].incoming==0 && gd_query_scene_record(scene,i)>=0); }
            for (i=0;i<scene->layout.edge_count;++i) links+=scene->edges[i].kind==GD_QUERY_EDGE_EFFECT;
            TEST_ASSERT(nodes==2 && links==3);
            { int shared=0,first=0,second=0; GD_ActorComparison expected; TEST_ASSERT(gd_actor_compare(surface->variants,small,big,&expected) && gd_query_scene_compare(scene,small,big));
              for (i=0;i<surface->variants->effect_count;++i) { int mask=gd_actor_compare_member(surface->variants,small,big,GD_ACTOR_SET_EFFECTS,i); shared+=mask==3; first+=mask==1; second+=mask==2; }
              TEST_ASSERT(shared==expected.effects.shared && first==expected.effects.first_only && second==expected.effects.second_only);
              for (i=0;i<scene->layout.node_count;++i) if (scene->nodes[i].kind==GD_QUERY_NODE_EFFECT) TEST_ASSERT(scene->nodes[i].comparison==gd_actor_compare_member(surface->variants,small,big,GD_ACTOR_SET_EFFECTS,scene->nodes[i].source));
              TEST_ASSERT(!gd_query_scene_compare(scene,-1,big) && gd_actor_compare_member(surface->variants,small,big,GD_ACTOR_SET_EFFECTS,-1)==-1 && gd_actor_compare_member(surface->variants,small,big,(GD_ActorSet)9,0)==-1); }
            gd_query_scene_destroy(scene);
            scene=gd_query_scene_build(surface,GD_QUERY_PRIMARY_VARIANT,big,0,1,GD_EFFECT_WORK); nodes=links=0; TEST_ASSERT(scene && !scene->limited);
            for (i=0;i<scene->layout.node_count;++i) nodes+=scene->nodes[i].kind==GD_QUERY_NODE_VARIANT;
            for (i=0;i<scene->layout.edge_count;++i) links+=scene->edges[i].kind==GD_QUERY_EDGE_EFFECT;
            TEST_ASSERT(nodes==1 && links==2); gd_query_scene_destroy(scene);
            scene=gd_query_scene_build(surface,GD_QUERY_STACKED_VARIANTS,big,0,0,GD_EFFECT_WORK); nodes=0; TEST_ASSERT(scene);
            for (i=0;i<scene->layout.node_count;++i) nodes+=scene->nodes[i].ghost;
            TEST_ASSERT(nodes>0);
            { int primary=-1,ghost=-1; for (i=0;i<scene->layout.node_count;++i) if (scene->nodes[i].kind==GD_QUERY_NODE_VARIANT) { if (scene->nodes[i].ghost) ghost=i; else primary=i; }
              TEST_ASSERT(primary>=0 && ghost>=0 && gd_query_scene_hit(scene,scene->layout.nodes[primary].x,scene->layout.nodes[primary].y)==primary);
              TEST_ASSERT(gd_query_scene_hit(scene,scene->layout.nodes[ghost].x,scene->layout.nodes[ghost].y+scene->layout.nodes[ghost].height/2-2)==ghost && gd_query_scene_hit(NULL,0,0)==-1); }
            gd_query_scene_destroy(scene);
            scene=gd_query_scene_build(surface,GD_QUERY_STACKED_VARIANTS,0,0,0,GD_EFFECT_WORK); TEST_ASSERT(scene);
            for (i=0;i<scene->layout.node_count;++i) if (scene->nodes[i].kind==GD_QUERY_NODE_VARIANT && !scene->nodes[i].ghost) TEST_ASSERT(gd_query_scene_hit(scene,scene->layout.nodes[i].x,scene->layout.nodes[i].y)==i);
            gd_query_scene_destroy(scene);
            scene=gd_query_scene_build(surface,GD_QUERY_ALL_VARIANTS,0,0,0,0); TEST_ASSERT(scene && scene->limited && !scene->layout.node_count && scene->unexamined_items_upper_bound>0); gd_query_scene_destroy(scene);
            TEST_ASSERT(gd_actor_write_effect(surface->variants,-1,0)==-1 && gd_actor_write_effect(surface->variants,0,-1)==-1);
            for (i=0;i<surface->variants->write_count;++i) TEST_ASSERT(surface->variants->write_effects[i]>=0);
        }
        gd_query_destroy(surface);
        request.kind=GD_QUERY_BINDINGS; request.case_index=-1; request.noun=rules->participants[actor].noun_record; request.fixed=NULL;
        surface=gd_query_build(model,rules,analysis,&request); TEST_ASSERT(surface && surface->binding_count==6);
        {
            GD_QueryScene *scene=gd_query_scene_build(surface,GD_QUERY_ALL_VARIANTS,-1,0,0,GD_EFFECT_WORK); int nouns=0,bindings=0,j;
            TEST_ASSERT(scene && !scene->limited && !scene->layout.edge_count);
            for (j=0;j<model->record_count;++j) if (model->records[j].role==GD_NOUN) ++nouns;
            TEST_ASSERT(scene->layout.node_count==nouns);
            for (j=0;j<scene->layout.node_count;++j) { TEST_ASSERT(scene->nodes[j].kind==GD_QUERY_NODE_OBJECT && gd_query_scene_record(scene,j)==scene->nodes[j].source); bindings+=scene->nodes[j].incoming; }
            TEST_ASSERT(bindings==6); gd_query_scene_destroy(scene);
            scene=gd_query_scene_build(surface,GD_QUERY_ALL_VARIANTS,-1,0,0,0); TEST_ASSERT(scene && scene->limited && !scene->layout.node_count && scene->unexamined_items_upper_bound==model->record_count); gd_query_scene_destroy(scene);
        }
        gd_query_destroy(surface);
        request.participant="B"; surface=gd_query_build(model,rules,analysis,&request); TEST_ASSERT(surface && surface->binding_count==1 && surface->bindings[0].participant==other); gd_query_destroy(surface);
        request.work_limit=0; surface=gd_query_build(model,rules,analysis,&request); TEST_ASSERT(surface && surface->limited && !surface->binding_count && surface->unexamined_bindings==rules->participant_count); gd_query_destroy(surface);
        TEST_ASSERT(gd_query_fields(GD_QUERY_ACTOR_STATES,"variants,cases") && !gd_query_fields(GD_QUERY_ACTOR_STATES,"contributors"));
    }
    tiny=gd_actor_query_build(model,analysis,&selection,0); TEST_ASSERT(tiny && tiny->limited && tiny->work==0 && !tiny->variant_count && tiny->unexamined_variants==2); gd_actor_query_destroy(tiny);
    memset(&fixed,0,sizeof(fixed)); TEST_ASSERT(gd_rule_assume(model,rules,root,"A:actions/Form=big",&fixed));
    selection.axis=points; selection.value_count=2; selection.values[0]="9"; selection.values[1]="10";
    numeric=gd_actor_query_build(model,analysis,&selection,GD_ACTOR_WORK);
    TEST_ASSERT(numeric && !numeric->limited && numeric->variant_count==2 && numeric->write_count==4);
    for (i=0;i<2;++i) {
        GD_ActorVariant *variant=&numeric->variants[i]; int j,quantities=0;
        for (j=variant->first_write;j<variant->first_write+variant->write_count;++j) if (numeric->writes[j].kind==GD_WRITE_QUANTITY) {
            ++quantities; TEST_ASSERT(numeric->writes[j].value.known && numeric->writes[j].value.number==10);
            TEST_ASSERT(gd_actor_write_family(numeric,i,&numeric->writes[j])==(i ? GD_EFFECT_NO_CHANGE:GD_EFFECT_GAIN));
        }
        TEST_ASSERT(quantities==1);
    }
    TEST_ASSERT(gd_actor_compare(numeric,0,1,&comparison) && comparison.effects.shared==1 && comparison.effects.first_only==1 && comparison.effects.second_only==1);
    TEST_ASSERT(comparison.cases.shared==2 && !comparison.cases.first_only && !comparison.cases.second_only);
    gd_actor_query_destroy(numeric); gd_actor_query_destroy(query);
    memset(&fixed,0,sizeof(fixed)); TEST_ASSERT(gd_rule_assume(model,rules,root,"A:actions/Points=9",&fixed));
    selection.participant=other; selection.axis=form; selection.value_count=0;
    query=gd_actor_query_build(model,analysis,&selection,GD_ACTOR_WORK);
    TEST_ASSERT(query && query->participant==other && query->variant_count==2 && !query->limited);
    for (i=0;i<2;++i) {
        GD_ActorVariant *variant=&query->variants[i]; int j,unknown=0;
        TEST_ASSERT(variant->write_count==3 && !variant->constraints_complete);
        for (j=variant->first_activation;j<variant->first_activation+variant->activation_count;++j) unknown+=query->activations[j].applicability==GD_RULE_TRUTH_UNKNOWN;
        TEST_ASSERT(unknown==2); /* Same noun does not transfer B's Form fact to A. */
        for (j=variant->first_write;j<variant->first_write+variant->write_count;++j) TEST_ASSERT(query->writes[j].conditional);
    }
    TEST_ASSERT(gd_actor_compare(query,0,1,&comparison) && !comparison.complete && comparison.cases.shared==3 && comparison.effects.shared==2);
    gd_actor_query_destroy(query);
    selection.participant=-1; selection.axis=points; selection.value_count=0; TEST_ASSERT(!gd_actor_query_build(model,analysis,&selection,GD_ACTOR_WORK));
    TEST_ASSERT(model->error_count && !strcmp(model->errors[model->error_count-1].code,"actors.values"));
    gd_rule_analysis_destroy(analysis); gd_rules_destroy(rules); gd_destroy(model);
)
