#include "gd_effect_graph.h"
TEST(effect_result_concepts_and_targeted_links,
    GD_Model *model=model_create(); GD_Rules *rules; GD_RuleAnalysis *analysis; GD_ResourceGraph *source;
    GD_EffectGraph *graph,*tiny,*empty; GD_Allocator allocator; RouteFailure failure;
    unsigned char mask[GD_EFFECT_NODES]={0}; int counts[GD_EFFECT_NODES],i,j,life=-1,flag=-1,gain=-1,quantities=0,caller=-1,instance=-1;
    char first[GD_EFFECT_ID_CAPACITY],second[GD_EFFECT_ID_CAPACITY];
    TEST_ASSERT(model && setup("effect-concepts")); collections[1].role=GD_AXIS; collections[2].role=GD_RULE;
    TEST_ASSERT(write_record(1,"Life","---\nvalue_type: enum\nallowed_values: [alive, dead]\nclosed: true\n---\n"));
    TEST_ASSERT(write_record(1,"Flag","---\nvalue_type: enum\nallowed_values: [clear, dead]\nclosed: false\n---\n"));
    TEST_ASSERT(write_record(1,"Points","---\nvalue_type: integer\nunit: point\n---\n"));
    TEST_ASSERT(write_record(0,"unit","---\naxes: [actions/Life, actions/Flag, actions/Points]\n---\n"));
    TEST_ASSERT(write_record(2,"rules","---\nstatus: accepted\n---\n```gd-rule\n"
        "case shared:\n    target: A\n    context: ANY\n    A:\n        noun: things/unit\n        requires: ANY\n        becomes: Life.dead, Life.dead, Flag.dead\n        set:\n            Points: this + 1[point]\n    B:\n        noun: things/unit\n        requires: ANY\n        becomes: Life.dead\n        preserve: Points\n        set:\n            Points: this + 10[point]\n"
        "case caller:\n    target: One\n    context: ANY\n    One:\n        noun: things/unit\n        requires: ANY\n        causes: cases/rules#shared with A=One, B=Two\n    Two:\n        noun: things/unit\n        requires: ANY\n"
        "case absolute:\n    target: A\n    context: ANY\n    A:\n        noun: things/unit\n        requires: ANY\n        preserve: OTHER\n        set:\n            Points: 5[point]\n            Points: min(this + 1[point], 10[point])\n"
        "case unknown:\n    target: A\n    context: ANY\n    A:\n        noun: things/unit\n        requires: ANY\n        becomes: Flag.future\n```\n"));
    TEST_ASSERT(gd_load(model,collections,3)); rules=gd_read_rules(model); analysis=gd_analyze_rules(model,rules);
    TEST_ASSERT(rules && analysis && !model->error_count);
    source=gd_resource_graph_build(model,rules); TEST_ASSERT(source && gd_resource_graph_bind_calls(source,analysis) && source->instance_count==1);
    graph=gd_effect_graph_build(source,GD_EFFECT_WORK); TEST_ASSERT(graph && !graph->limited && graph->source_incomplete && graph->unknown_effects==1);
    for (i=0;i<graph->node_count;++i) {
        const GD_EffectNode *node=&graph->nodes[i];
        TEST_ASSERT(gd_effect_node_id(graph,i,first,sizeof(first)) && !gd_effect_node_id(graph,i,second,2));
        for (j=0;j<i;++j) { TEST_ASSERT(gd_effect_node_id(graph,j,second,sizeof(second))); TEST_ASSERT(strcmp(first,second)); }
        if (node->kind==GD_EFFECT_STATE) {
            TEST_ASSERT(!strcmp(node->value,"dead"));
            if (!strcmp(model->records[node->axis].name,"Life")) life=i; else if (!strcmp(model->records[node->axis].name,"Flag")) flag=i;
        }
        if (node->kind==GD_EFFECT_GAIN) { TEST_ASSERT(gain<0); gain=i; }
        if (node->kind==GD_EFFECT_QUANTITY) ++quantities;
        if (node->kind==GD_EFFECT_ACTIVATION) {
            if (source->nodes[node->activation].instance) instance=i;
            if (!strcmp(rules->cases[source->nodes[node->activation].case_index].name,"caller")) caller=i;
        }
    }
    TEST_ASSERT(life>=0 && flag>=0 && life!=flag && gain>=0 && quantities==2 && caller>=0 && instance>=0);
    gd_effect_incoming(graph,NULL,counts); TEST_ASSERT(counts[life]==6 && counts[flag]==2 && counts[gain]==4);
    mask[caller]=1; gd_effect_incoming(graph,mask,counts); TEST_ASSERT(!counts[life] && !counts[gain]); /* No flattened caller/result links. */
    mask[caller]=0; mask[instance]=1; gd_effect_incoming(graph,mask,counts); TEST_ASSERT(counts[life]==3 && counts[flag]==1 && counts[gain]==2);
    {
        int one=0,ten=0,calls=0,first_target=-1,second_target=-1;
        for (i=0;i<graph->link_count;++i) {
            const GD_EffectLink *link=&graph->links[i]; const GD_ResourceRelation *edge=&source->relations[link->relation];
            if (link->kind==GD_EFFECT_INVOCATION) { ++calls; TEST_ASSERT(link->from==caller && link->to==instance); }
            if (link->from==instance && link->to==gain) { one+=edge->delta.number==1; ten+=edge->delta.number==10; }
            if (link->from==instance && link->to==life) { if (first_target<0) first_target=link->participant; else if (link->participant!=first_target) second_target=link->participant; }
        }
        TEST_ASSERT(calls==1 && one==1 && ten==1 && first_target>=0 && second_target>=0);
        TEST_ASSERT(!strcmp(rules->participants[first_target].name,"One") && !strcmp(rules->participants[second_target].name,"Two"));
    }
    {
        GD_EffectGraph *fresh=gd_effect_graph_build(source,GD_EFFECT_WORK); GD_ResourceGraph *collision;
        int declaration=-1;
        TEST_ASSERT(fresh);
        for (i=0;i<graph->node_count;++i) TEST_ASSERT(gd_effect_node_remap(graph,i,fresh)==i);
        gd_effect_graph_destroy(fresh);
        collision=source->allocator.allocate(source->allocator.context,sizeof(*collision)); TEST_ASSERT(collision);
        memcpy(collision,source,sizeof(*collision)); ++collision->nodes[graph->nodes[instance].activation].ordinal;
        fresh=gd_effect_graph_build(collision,GD_EFFECT_WORK); TEST_ASSERT(fresh);
        TEST_ASSERT(gd_effect_node_remap(graph,instance,fresh)==-1 && gd_effect_node_remap(graph,life,fresh)>=0);
        for (i=0;i<graph->node_count;++i) if (graph->nodes[i].relation>=0 && source->relations[graph->nodes[i].relation].activation==graph->nodes[instance].activation) { declaration=i; break; }
        TEST_ASSERT(declaration>=0 && gd_effect_node_remap(graph,declaration,fresh)==-1);
        gd_effect_graph_destroy(fresh); gd_resource_graph_destroy(collision);
    }
    tiny=gd_effect_graph_build(source,1); TEST_ASSERT(tiny && tiny->limited && tiny->work<=1 && tiny->unexamined_source_nodes>0 && tiny->unexamined_relations==source->relation_count);
    gd_effect_graph_destroy(tiny);
    {
        GD_ResourceGraph *blank=source->allocator.allocate(source->allocator.context,sizeof(*blank)); TEST_ASSERT(blank);
        memset(blank,0,sizeof(*blank)); blank->allocator=source->allocator; blank->model=model; blank->rules=rules; blank->calls_bound=1;
        empty=gd_effect_graph_build(blank,GD_EFFECT_WORK); TEST_ASSERT(empty && !empty->node_count && !empty->link_count && !empty->limited);
        gd_effect_graph_destroy(empty); gd_resource_graph_destroy(blank);
    }
    allocator=source->allocator; failure=(RouteFailure){allocator,0}; source->allocator=(GD_Allocator){&failure,route_allocate,route_release};
    TEST_ASSERT(!gd_effect_graph_build(source,GD_EFFECT_WORK)); source->allocator=allocator;
    TEST_ASSERT(!gd_effect_graph_build(source,-1) && !gd_effect_graph_build(source,GD_EFFECT_WORK+1));
    gd_effect_graph_destroy(graph); gd_resource_graph_destroy(source); gd_rule_analysis_destroy(analysis); gd_rules_destroy(rules); gd_destroy(model);
)
TEST(effect_projection_capacity_and_source_limits,
    int mode;
    for (mode=0;mode<2;++mode) {
        GD_Model *model=model_create(); GD_Rules *rules; GD_RuleAnalysis *analysis; GD_ResourceGraph *source; GD_EffectGraph *graph;
        char axis[16],metadata[2048],values[1024]; char *body; size_t used=0,at=0; int i,j,counts[GD_EFFECT_NODES],effects=0;
        TEST_ASSERT(model && setup(mode ? "effect-link-capacity":"effect-node-capacity")); collections[1].role=GD_AXIS; collections[2].role=GD_RULE;
        body=model->allocator.allocate(model->allocator.context,131072); TEST_ASSERT(body);
        if (!mode) {
            at+=(size_t)snprintf(values+at,sizeof(values)-at,"---\nvalue_type: enum\nclosed: true\nallowed_values: [");
            for (i=0;i<64;++i) { at+=(size_t)snprintf(values+at,sizeof(values)-at,"%sv%d",i ? ", ":"",i); TEST_ASSERT(at<sizeof(values)); }
            TEST_ASSERT(snprintf(values+at,sizeof(values)-at,"]\n---\n")>0);
            at=0; at+=(size_t)snprintf(metadata+at,sizeof(metadata)-at,"---\naxes: [");
            for (i=0;i<64;++i) {
                snprintf(axis,sizeof(axis),"D%d",i); TEST_ASSERT(write_record(1,axis,values));
                at+=(size_t)snprintf(metadata+at,sizeof(metadata)-at,"%sactions/D%d",i ? ", ":"",i); TEST_ASSERT(at<sizeof(metadata));
            }
            TEST_ASSERT(snprintf(metadata+at,sizeof(metadata)-at,"]\n---\n")>0 && write_record(0,"unit",metadata));
            used+=(size_t)snprintf(body+used,131072-used,"---\nstatus: accepted\n---\n```gd-rule\ncase all:\n    target: P0\n    context: ANY\n");
            for (i=0;i<64;++i) {
                used+=(size_t)snprintf(body+used,131072-used,"    P%d:\n        noun: things/unit\n        requires: ANY\n        becomes: ",i); TEST_ASSERT(used<131072);
                for (j=0;j<64;++j) { used+=(size_t)snprintf(body+used,131072-used,"%sD%d.v%d",j ? ", ":"",i,j); TEST_ASSERT(used<131072); }
                body[used++]='\n';
            }
        } else {
            TEST_ASSERT(write_record(1,"Life","---\nvalue_type: enum\nallowed_values: [dead]\nclosed: true\n---\n"));
            TEST_ASSERT(write_record(0,"unit","---\naxes: [actions/Life]\n---\n"));
            used+=(size_t)snprintf(body+used,131072-used,"---\nstatus: accepted\n---\n```gd-rule\n");
            for (i=0;i<32;++i) {
                used+=(size_t)snprintf(body+used,131072-used,"case variant%d:\n    target: A\n    context: ANY\n    A:\n        noun: things/unit\n        requires: ANY\n        becomes: ",i); TEST_ASSERT(used<131072);
                for (j=0;j<256;++j) { used+=(size_t)snprintf(body+used,131072-used,"%sdead",j ? ", ":""); TEST_ASSERT(used<131072); }
                body[used++]='\n';
            }
            TEST_ASSERT(write_record(2,"caller","---\nstatus: accepted\n---\n```gd-rule\ncase caller:\n    target: A\n    context: ANY\n    A:\n        noun: things/unit\n        requires: ANY\n        causes: cases/template; cases/template; cases/template\n```\n"));
        }
        TEST_ASSERT(snprintf(body+used,131072-used,"```\n")>0 && write_record(2,"template",body));
        model->allocator.release(model->allocator.context,body);
        TEST_ASSERT(gd_load(model,collections,3)); rules=gd_read_rules(model); analysis=gd_analyze_rules(model,rules);
        TEST_ASSERT(rules && analysis && !model->error_count); source=gd_resource_graph_build(model,rules); TEST_ASSERT(source);
        if (mode) TEST_ASSERT(!gd_resource_graph_bind_calls(source,analysis) && source->limited && source->relation_count==GD_RESOURCE_GRAPH_RELATIONS);
        else TEST_ASSERT(gd_resource_graph_bind_calls(source,analysis) && !source->limited);
        graph=gd_effect_graph_build(source,GD_EFFECT_WORK); TEST_ASSERT(graph && graph->work<=GD_EFFECT_WORK);
        gd_effect_incoming(graph,NULL,counts);
        for (i=0;i<graph->node_count;++i) effects+=counts[i];
        if (mode) {
            TEST_ASSERT(!graph->limited && graph->source_incomplete && graph->link_count==GD_EFFECT_LINKS);
            TEST_ASSERT(effects>32000 && !graph->unexamined_relations);
        } else TEST_ASSERT(graph->limited && graph->node_count==GD_EFFECT_NODES && graph->link_count==4095 && graph->unexamined_relations==1 && effects==4095);
        gd_effect_graph_destroy(graph); gd_resource_graph_destroy(source); gd_rule_analysis_destroy(analysis); gd_rules_destroy(rules); gd_destroy(model);
    }
    printf("effect projection owned storage: %zu bytes; 4096-node/32768-link limits checked\n",sizeof(GD_EffectGraph));
)
