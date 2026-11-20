/* Concrete placements reuse compiled maps; no mutable game state or new evaluator. */
TEST(resource_instance_identity_and_limits,
    GD_Model *model=model_create(); GD_Rules *rules; GD_RuleAnalysis *analysis;
    GD_ResourceGraph *graph,*fresh; GD_Allocator allocator; RouteFailure failure; int i,instance=-1;
    char id[GD_MAX_PATH],other[GD_MAX_PATH];
    TEST_ASSERT(model && setup("resource-instance-limits")); collections[1].role=GD_AXIS; collections[2].role=GD_RULE;
    TEST_ASSERT(write_record(1,"Amount","---\nvalue_type: integer\nunit: coin\n---\n"));
    TEST_ASSERT(write_record(0,"bank","---\naxes: [actions/Amount]\n---\n"));
    TEST_ASSERT(write_record(2,"cycle","---\nstatus: accepted\n---\n```gd-rule\ncase first:\n    target: A\n    context: ANY\n    A:\n        noun: things/bank\n        requires: ANY\n        causes: cases/cycle#second with B=A\ncase second:\n    target: B\n    context: ANY\n    set:\n        Amount: this + 1[coin]\n    B:\n        noun: things/bank\n        requires: ANY\n        causes: cases/cycle#first with A=B\n```\n"));
    TEST_ASSERT(gd_load(model,collections,3)); rules=gd_read_rules(model); analysis=gd_analyze_rules(model,rules);
    TEST_ASSERT(rules && analysis && !model->error_count);
    graph=gd_resource_graph_build(model,rules); TEST_ASSERT(graph && gd_resource_graph_bind_calls(graph,analysis));
    TEST_ASSERT(graph->instance_count==2 && graph->cycle_frontiers==2 && graph->composition_unexamined==2 && !graph->limited);
    for (i=0;i<graph->node_count;++i) if (graph->nodes[i].instance) {
        const GD_RuleCase *item=&rules->cases[graph->nodes[i].case_index]; int p=gd_resource_instance_actor(graph,i,item->first_participant);
        TEST_ASSERT(p>=0 && p!=item->first_participant);
        TEST_ASSERT(gd_resource_instance_actor(graph,i,p)==-1);
        instance=i;
    }
    TEST_ASSERT(instance>=0 && gd_resource_node_id(graph,instance,-1,id,sizeof(id)));
    fresh=graph->allocator.allocate(graph->allocator.context,sizeof(*fresh)); TEST_ASSERT(fresh); memcpy(fresh,graph,sizeof(*fresh));
    TEST_ASSERT(gd_resource_node_remap(graph,instance,fresh)==instance);
    ++fresh->nodes[instance].ordinal; /* Simulated digest collision must not establish placement equality. */
    TEST_ASSERT(gd_resource_node_id(fresh,instance,-1,other,sizeof(other)) && !strcmp(id,other));
    TEST_ASSERT(gd_resource_node_remap(graph,instance,fresh)==-1);
    gd_resource_graph_destroy(fresh);
    fresh=gd_resource_graph_build(model,rules); TEST_ASSERT(fresh);
    for (i=0;i<graph->node_count;++i) if (graph->nodes[i].instance) break;
    TEST_ASSERT(i<graph->node_count);
    fresh->nodes[fresh->node_count++]=graph->nodes[i]; /* Inject prospective digest to exercise collision stop. */
    TEST_ASSERT(!gd_resource_graph_bind_calls(fresh,analysis) && fresh->limited && fresh->identity_collisions==1);
    gd_resource_graph_destroy(fresh); gd_resource_graph_destroy(graph);
    graph=gd_resource_graph_build(model,rules); TEST_ASSERT(graph); graph->work=GD_RESOURCE_GRAPH_WORK-1;
    TEST_ASSERT(!gd_resource_graph_bind_calls(graph,analysis) && graph->limited && graph->work<=GD_RESOURCE_GRAPH_WORK && graph->composition_unexamined>0);
    gd_resource_graph_destroy(graph);
    graph=gd_resource_graph_build(model,rules); TEST_ASSERT(graph);
    for (i=graph->node_count;i<GD_RESOURCE_GRAPH_NODES;++i) graph->nodes[i].kind=GD_RESOURCE_ROLE;
    graph->node_count=GD_RESOURCE_GRAPH_NODES;
    TEST_ASSERT(!gd_resource_graph_bind_calls(graph,analysis) && graph->limited && graph->node_count==GD_RESOURCE_GRAPH_NODES);
    gd_resource_graph_destroy(graph);
    allocator=model->allocator; failure=(RouteFailure){allocator,0};
    model->allocator=(GD_Allocator){&failure,route_allocate,route_release};
    TEST_ASSERT(!gd_resource_graph_build(model,rules) && has_error(model,"resources.memory"));
    model->allocator=allocator;
    gd_rule_analysis_destroy(analysis); gd_rules_destroy(rules); gd_destroy(model);
)
TEST(resource_instance_depth_frontiers,
    GD_Model *model=model_create(); GD_Rules *rules; GD_RuleAnalysis *analysis; GD_ResourceGraph *graph;
    int i; char text[12000]; size_t used=0;
    TEST_ASSERT(model && setup("resource-instance-depth")); collections[1].role=GD_AXIS; collections[2].role=GD_RULE;
    TEST_ASSERT(write_record(1,"Amount","---\nvalue_type: integer\nunit: coin\n---\n"));
    TEST_ASSERT(write_record(0,"bank","---\naxes: [actions/Amount]\n---\n"));
    used+=(size_t)snprintf(text+used,sizeof(text)-used,"---\nstatus: accepted\n---\n```gd-rule\n");
    for (i=0;i<GD_MAX_RULE_EXPANSION_DEPTH+2;++i) {
        used+=(size_t)snprintf(text+used,sizeof(text)-used,"case step%d:\n    target: Bank\n    context: ANY\n    Bank:\n        noun: things/bank\n        requires: ANY\n        set:\n            Amount: this + 1[coin]\n",i);
        if (i<GD_MAX_RULE_EXPANSION_DEPTH+1) used+=(size_t)snprintf(text+used,sizeof(text)-used,"        causes: cases/chain#step%d with Bank=Bank\n",i+1);
        TEST_ASSERT(used<sizeof(text));
    }
    TEST_ASSERT(used+5<sizeof(text)); strcpy(text+used,"```\n");
    TEST_ASSERT(write_record(2,"chain",text) && gd_load(model,collections,3)); rules=gd_read_rules(model); analysis=gd_analyze_rules(model,rules);
    TEST_ASSERT(rules && analysis && !model->error_count);
    graph=gd_resource_graph_build(model,rules); TEST_ASSERT(graph && gd_resource_graph_bind_calls(graph,analysis));
    TEST_ASSERT(!graph->limited && graph->depth_frontiers>0 && !graph->cycle_frontiers && graph->instance_count>GD_MAX_RULE_EXPANSION_DEPTH);
    for (i=0;i<graph->node_count;++i) if (graph->nodes[i].instance) TEST_ASSERT(graph->nodes[i].depth<GD_MAX_RULE_EXPANSION_DEPTH);
    gd_resource_graph_destroy(graph); gd_rule_analysis_destroy(analysis); gd_rules_destroy(rules); gd_destroy(model);
)
TEST(resource_instance_unresolved_origins,
    GD_Model *model=model_create(); GD_Rules *rules; GD_RuleAnalysis *analysis; GD_ResourceGraph *graph,*fresh;
    int i,j,unknown=0; char first[GD_MAX_PATH],second[GD_MAX_PATH];
    TEST_ASSERT(model && setup("resource-instance-origins")); collections[1].role=GD_AXIS; collections[2].role=GD_RULE;
    TEST_ASSERT(write_record(1,"Mood","---\nvalue_type: enum\nallowed_values: [calm]\nclosed: false\n---\n"));
    TEST_ASSERT(write_record(1,"Points","---\nvalue_type: integer\nunit: point\n---\n"));
    TEST_ASSERT(write_record(0,"bank","---\naxes: [actions/Mood, actions/Points]\n---\n"));
    TEST_ASSERT(write_record(2,"root","---\nstatus: accepted\n---\n```gd-rule\ncase run:\n    target: A\n    context: ANY\n    A:\n        noun: things/bank\n        requires: mystery_root\n        causes: cases/helper#run with A=A\n```\n"));
    TEST_ASSERT(write_record(2,"helper","---\nstatus: accepted\n---\n```gd-rule\ncase run:\n    target: A\n    context: ANY\n    A:\n        noun: things/bank\n        requires: mystery_helper\n        set:\n            Points: this + 1[point]\n```\n"));
    TEST_ASSERT(gd_load(model,collections,3)); rules=gd_read_rules(model); analysis=gd_analyze_rules(model,rules);
    TEST_ASSERT(rules && analysis && !model->error_count);
    graph=gd_resource_graph_build(model,rules); fresh=gd_resource_graph_build(model,rules);
    TEST_ASSERT(graph && fresh && gd_resource_graph_bind_calls(graph,analysis) && gd_resource_graph_bind_calls(fresh,analysis));
    for (i=0;i<graph->node_count;++i) {
        TEST_ASSERT(gd_resource_node_id(graph,i,-1,first,sizeof(first)) && gd_resource_node_remap(graph,i,fresh)==i);
        if (graph->nodes[i].kind==GD_RESOURCE_FACET && graph->nodes[i].placement && graph->nodes[i].axis<0) ++unknown;
        for (j=0;j<i;++j) { TEST_ASSERT(gd_resource_node_id(graph,j,-1,second,sizeof(second))); TEST_ASSERT(strcmp(first,second)); }
    }
    TEST_ASSERT(unknown==2);
    gd_resource_graph_destroy(fresh); gd_resource_graph_destroy(graph); gd_rule_analysis_destroy(analysis); gd_rules_destroy(rules); gd_destroy(model);
)
