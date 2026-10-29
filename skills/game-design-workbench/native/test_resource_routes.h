#include "gd_resource_routes.h"
TEST(resource_routes,
    GD_Model *model=model_create(); GD_Rules *rules; GD_ResourceGraph *graph;
    GD_Allocator allocator={storage,allocate,release}; GD_ResourceRoutes *query=gd_resource_routes_create(allocator);
    char id[GD_MAX_PATH],tiny[2]; int i;
    TEST_ASSERT(model && query); model->rule_mode=1; model->record_count=2;
    model->records[0].role=GD_RULE; model->records[0].name="bounds"; model->records[0].reference="rules/bounds";
    model->records[0].body=""; model->records[0].status="accepted";
    model->records[1].role=GD_AXIS; model->records[1].reference="axes/Quantity";
    rules=gd_read_rules(model); TEST_ASSERT(rules && !model->error_count);
    graph=allocate(storage,sizeof(*graph)); TEST_ASSERT(graph); memset(graph,0,sizeof(*graph));
    graph->allocator=allocator; graph->model=model; graph->rules=rules; graph->node_count=GD_RESOURCE_GRAPH_NODES;
    rules->case_count=rules->participant_count=GD_RESOURCE_GRAPH_NODES/2;
    /* Hand-built capacity projection: alternating guarded activations/facets. */
    for (i=0;i<rules->case_count;++i) {
        char *name=model->text+model->text_used; int length=snprintf(name,32,"case-%d",i);
        model->text_used+=(size_t)length+1;
        rules->cases[i].source=0; rules->cases[i].name=name; rules->cases[i].first_participant=i; rules->cases[i].participant_count=1;
        rules->participants[i].name="Wallet";
        graph->nodes[2*i]=(GD_ResourceNode){.kind=GD_RESOURCE_FACET,.case_index=i,.participant=i,.axis=1,.pool=-1,.record=-1};
        graph->nodes[2*i+1]=(GD_ResourceNode){.kind=GD_RESOURCE_CASE,.case_index=i,.participant=-1,.axis=-1,.pool=-1,.record=-1};
        graph->relations[2*i]=(GD_ResourceRelation){.kind=GD_RESOURCE_REQUIREMENT,.facet=2*i,.activation=2*i+1,.participant=i};
        graph->relations[2*i+1]=(GD_ResourceRelation){.kind=GD_RESOURCE_QUANTITY_WRITE,.facet=(2*i+2)%GD_RESOURCE_GRAPH_NODES,.activation=2*i+1,.participant=i,.delta_recognized=1,.delta={.number=1,.known=1,.checks_complete=1}};
    }
    graph->relation_count=GD_RESOURCE_GRAPH_NODES;
    TEST_ASSERT(gd_resource_query(query,graph,0,64,0,GD_RESOURCE_ROUTE_WORK) && query->witness_count==1 && !query->limited && !query->depth_limited);
    TEST_ASSERT(query->witnesses[0].count==GD_RESOURCE_ROUTE_STEPS && query->marked_nodes[64]);
    TEST_ASSERT(gd_resource_query(query,graph,0,66,0,GD_RESOURCE_ROUTE_WORK) && !query->witness_count && query->depth_limited);
    TEST_ASSERT(gd_resource_query(query,graph,0,64,0,0) && query->limited && !query->work && !query->witness_count);
    TEST_ASSERT(!gd_resource_query(query,graph,-2,0,0,GD_RESOURCE_ROUTE_WORK));
    TEST_ASSERT(gd_resource_node_id(graph,4095,-1,id,sizeof(id)) && gd_resource_select(query,graph,id)==4095);
    TEST_ASSERT(!gd_resource_node_id(graph,4095,-1,tiny,sizeof(tiny)) && !*tiny);
    for (i=GD_RESOURCE_GRAPH_NODES;i<GD_RESOURCE_GRAPH_RELATIONS;++i) graph->relations[i]=graph->relations[i%GD_RESOURCE_GRAPH_NODES];
    graph->relation_count=GD_RESOURCE_GRAPH_RELATIONS;
    TEST_ASSERT(gd_resource_query(query,graph,0,2,0,GD_RESOURCE_ROUTE_WORK) && query->arc_count==GD_RESOURCE_GRAPH_RELATIONS && query->witness_count==1);
    TEST_ASSERT(gd_resource_query(query,graph,0,64,0,GD_RESOURCE_ROUTE_WORK) && !query->limited && query->witness_count==1);
    TEST_ASSERT(query->witnesses[0].count==64 && query->work<400000);
    TEST_ASSERT(gd_resource_query(query,graph,-1,2,0,GD_RESOURCE_ROUTE_WORK) && (query->limited || query->depth_limited) && query->witness_count>0);
    {
        GD_ResourceGraph *fresh=allocate(storage,sizeof(*fresh)); char changed[GD_MAX_PATH];
        TEST_ASSERT(fresh); graph->node_count=3; graph->relation_count=1;
        graph->nodes[0].axis=-1; graph->relations[0].kind=GD_RESOURCE_REQUIREMENT; graph->relations[0].declaration=0;
        rules->condition_tokens[0].offset=0;
        graph->nodes[2]=(GD_ResourceNode){.kind=GD_RESOURCE_ROLE,.record=-1,.pool_reference="resources/pool",.resolution=GD_RULE_MISSING};
        memcpy(fresh,graph,sizeof(*fresh));
        TEST_ASSERT(gd_resource_node_remap(graph,0,fresh)==0);
        TEST_ASSERT(gd_resource_node_id(graph,0,0,id,sizeof(id)));
        fresh->relations[0].kind=GD_RESOURCE_STATE_WRITE; rules->participants[0].first_state_effect=0;
        TEST_ASSERT(gd_resource_node_id(fresh,0,0,changed,sizeof(changed)) && strcmp(id,changed));
        TEST_ASSERT(gd_resource_node_remap(graph,0,fresh)==-1); /* No requirement-to-effect retargeting. */
        TEST_ASSERT(gd_resource_node_id(graph,2,-1,id,sizeof(id)));
        fresh->nodes[2].record=1; fresh->nodes[2].resolution=GD_RULE_RESOLVED;
        TEST_ASSERT(gd_resource_node_id(fresh,2,-1,changed,sizeof(changed)) && !strcmp(id,changed));
        TEST_ASSERT(gd_resource_node_remap(graph,2,fresh)==2); /* Definition availability is not identity. */
        fresh->node_count=4; fresh->nodes[3]=fresh->nodes[2];
        TEST_ASSERT(gd_resource_node_remap(graph,2,fresh)==-1); /* Do not pick duplicate identity. */
        gd_resource_graph_destroy(fresh);
    }
    gd_resource_routes_destroy(query); gd_resource_graph_destroy(graph); gd_rules_destroy(rules); gd_destroy(model);
)
