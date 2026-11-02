#include "gd_effect_query.h"
#include "gd_query.h"
#include "gd_query_scene.h"
TEST(effect_contributor_scope_paths_and_counts,
    GD_Model *model=model_create(); GD_Rules *rules; GD_RuleAnalysis *analysis; GD_ResourceGraph *source;
    GD_EffectGraph *graph,*duplicate; GD_EffectQuery *query; GD_EffectSelection selection={-1,-1,-1,-1,-1,NULL,NULL};
    unsigned char mask[GD_EFFECT_NODES]; int i,life=-1,root=-1,root_node=-1,noun,paths[GD_MAX_RULE_EXPANSION_DEPTH],invocations=0;
    char id[GD_EFFECT_ID_CAPACITY]; GD_Allocator allocator; RouteFailure failure;
    TEST_ASSERT(model && setup("effect-query")); collections[1].role=GD_AXIS; collections[2].role=GD_RULE;
    TEST_ASSERT(write_record(1,"Life","---\nvalue_type: enum\nallowed_values: [alive, dead]\nclosed: true\n---\n"));
    TEST_ASSERT(write_record(0,"one","---\naxes: [actions/Life]\n---\n")); TEST_ASSERT(write_record(0,"two","---\naxes: [actions/Life]\n---\n"));
    TEST_ASSERT(write_record(2,"rules","---\nstatus: accepted\n---\n```gd-rule\ncase caller:\n    target: One\n    context: ANY\n    One:\n        noun: things/one\n        requires: ANY\n        causes: cases/rules#pair with A=One, B=Two\n    Two:\n        noun: things/two\n        requires: ANY\ncase pair:\n    target: A\n    context: ANY\n    A:\n        noun: things/one\n        requires: ANY\n        becomes: Life.dead\n    B:\n        noun: things/two\n        requires: ANY\n        becomes: Life.dead\n```\n"));
    TEST_ASSERT(gd_load(model,collections,3)); rules=gd_read_rules(model); analysis=gd_analyze_rules(model,rules);
    TEST_ASSERT(rules && analysis && !model->error_count); source=gd_resource_graph_build(model,rules);
    TEST_ASSERT(source && gd_resource_graph_bind_calls(source,analysis)); graph=gd_effect_graph_build(source,GD_EFFECT_WORK); TEST_ASSERT(graph);
    root=gd_rule_case_find(model,rules,"cases/rules#caller"); noun=find_record(model,"things/one"); TEST_ASSERT(root>=0 && noun>=0);
    for (i=0;i<graph->node_count;++i) {
        if (graph->nodes[i].kind==GD_EFFECT_STATE) life=i;
        if (graph->nodes[i].kind==GD_EFFECT_ACTIVATION && !source->nodes[graph->nodes[i].activation].instance && source->nodes[graph->nodes[i].activation].case_index==root) root_node=i;
    }
    TEST_ASSERT(life>=0 && root_node>=0 && gd_effect_node_id(graph,life,id,sizeof(id)));
    TEST_ASSERT(gd_effect_select(graph,id,GD_EFFECT_WORK)==life && gd_effect_select(graph,id,0)==-3 && gd_effect_select(graph,"missing",GD_EFFECT_WORK)==-1);
    selection.target=life; query=gd_effect_query_build(graph,selection,GD_EFFECT_WORK);
    TEST_ASSERT(query && !query->limited && query->count==4 && query->incoming[life]==4 && !query->unexamined_links);
    gd_effect_query_destroy(query);
    {
        GD_QueryRequest request={0}; GD_Query *surface; int selected;
        request.kind=GD_QUERY_CONTRIBUTORS; request.case_index=request.noun=request.axis=request.effect_kind=-1; request.target_id=id; request.work_limit=GD_EFFECT_WORK;
        surface=gd_query_build(model,rules,analysis,&request); TEST_ASSERT(surface && surface->contributors && surface->contributors->count==4);
        selected=gd_effect_select(surface->effects,id,GD_EFFECT_WORK); TEST_ASSERT(selected>=0 && surface->contributors->incoming[selected]==4); gd_query_destroy(surface);
        surface=gd_query_build(model,rules,analysis,&request); TEST_ASSERT(surface);
        {
            GD_QueryScene *plain=gd_query_scene_build(surface,GD_QUERY_ALL_VARIANTS,-1,0,0,GD_EFFECT_WORK),*weighted=gd_query_scene_build(surface,GD_QUERY_ALL_VARIANTS,-1,1,0,GD_EFFECT_WORK); int changed=0;
            TEST_ASSERT(plain && weighted && !plain->limited && plain->layout.node_count==weighted->layout.node_count && plain->layout.edge_count==weighted->layout.edge_count);
            for (i=0;i<plain->layout.node_count;++i) { TEST_ASSERT(plain->nodes[i].source==weighted->nodes[i].source && plain->nodes[i].incoming==weighted->nodes[i].incoming); changed+=weighted->layout.nodes[i].width>plain->layout.nodes[i].width; }
            TEST_ASSERT(changed==1 && plain->layout.edge_count==surface->effects->link_count);
            gd_query_scene_destroy(plain); gd_query_scene_destroy(weighted);
            {
                GD_EffectSelection subset=surface->contributors->selection; GD_EffectQuery *scope; GD_QueryScene *scene; int found=0;
                subset.root_case=root; subset.noun=noun; scope=gd_effect_query_build(surface->effects,subset,GD_EFFECT_WORK); TEST_ASSERT(scope && scope->count==1);
                scene=gd_query_scene_effect_context(surface,scope,1,GD_EFFECT_WORK); TEST_ASSERT(scene && !scene->limited && scene->layout.node_count==3 && scene->layout.edge_count==3);
                for (i=0;i<scene->layout.node_count;++i) if (scene->nodes[i].kind==GD_QUERY_NODE_EFFECT) { TEST_ASSERT(scene->nodes[i].incoming==2 && scope->incoming[selected]==1 && surface->contributors->incoming[selected]==4); ++found; }
                TEST_ASSERT(found==1 && !gd_query_scene_compare(scene,0,1) && query_scene_free_checks(scene)); gd_query_scene_destroy(scene);
                scene=gd_query_scene_effect_context(surface,scope,1,0); TEST_ASSERT(scene && scene->limited && !scene->layout.node_count && scene->unexamined_items_upper_bound>0); gd_query_scene_destroy(scene);
                allocator=surface->allocator; failure=(RouteFailure){allocator,0}; surface->allocator=(GD_Allocator){&failure,route_allocate,route_release}; TEST_ASSERT(!gd_query_scene_effect_context(surface,scope,1,GD_EFFECT_WORK)); surface->allocator=allocator;
                TEST_ASSERT(!gd_query_scene_effect_context(surface,NULL,1,GD_EFFECT_WORK));
                gd_effect_query_destroy(scope);
            }
        }
        gd_query_destroy(surface);
        allocator=model->allocator;
        for (selected=0;selected<4;++selected) {
            failure=(RouteFailure){allocator,selected}; model->allocator=(GD_Allocator){&failure,route_allocate,route_release};
            surface=gd_query_build(model,rules,analysis,&request); TEST_ASSERT(!surface && model->error_count>0);
            model->allocator=allocator; model->error_count=model->errors_omitted=0;
        }
    }
    selection.root_case=root; selection.noun=noun; query=gd_effect_query_build(graph,selection,GD_EFFECT_WORK);
    TEST_ASSERT(query && query->count==1 && query->incoming[life]==1 && query->excluded_target==2 && query->excluded_scope==1);
    TEST_ASSERT(gd_effect_contributor_path(query,0,paths,GD_MAX_RULE_EXPANSION_DEPTH)==2 && paths[0]==root_node);
    TEST_ASSERT(!gd_effect_contributor_path(query,0,paths,1) && !gd_effect_contributor_path(query,1,paths,GD_MAX_RULE_EXPANSION_DEPTH));
    for (i=0;i<graph->link_count;++i) if (query->highlighted[i] && graph->links[i].kind==GD_EFFECT_INVOCATION) ++invocations;
    TEST_ASSERT(invocations==1); gd_effect_query_destroy(query);
    memset(mask,1,sizeof(mask)); mask[root_node]=0; selection.activation_mask=mask; selection.root_case=-1; selection.noun=-1;
    query=gd_effect_query_build(graph,selection,GD_EFFECT_WORK); TEST_ASSERT(query && query->count==2 && query->incoming[life]==2 && query->excluded_scope==2); gd_effect_query_destroy(query);
    selection.root_case=root; query=gd_effect_query_build(graph,selection,GD_EFFECT_WORK); TEST_ASSERT(query && !query->count && !query->limited); gd_effect_query_destroy(query);
    query=gd_effect_query_build(graph,selection,0); TEST_ASSERT(query && query->limited && !query->count && query->work==0 && query->unexamined_links==graph->link_count); gd_effect_query_destroy(query);
    duplicate=graph->allocator.allocate(graph->allocator.context,sizeof(*duplicate)); TEST_ASSERT(duplicate); memcpy(duplicate,graph,sizeof(*duplicate)); duplicate->nodes[duplicate->node_count++]=graph->nodes[life];
    TEST_ASSERT(gd_effect_select(duplicate,id,GD_EFFECT_WORK)==-2); gd_effect_graph_destroy(duplicate);
    selection.target=root_node; TEST_ASSERT(!gd_effect_query_build(graph,selection,GD_EFFECT_WORK)); selection.target=life;
    allocator=graph->allocator; failure=(RouteFailure){allocator,0}; graph->allocator=(GD_Allocator){&failure,route_allocate,route_release};
    TEST_ASSERT(!gd_effect_query_build(graph,selection,GD_EFFECT_WORK)); graph->allocator=allocator;
    printf("query scene owned storage: %llu bytes; borrowed query/scope excluded\n",(unsigned long long)sizeof(GD_QueryScene));
    printf("contributor owned storage: %llu bytes; query adapter: %llu bytes; borrowed snapshots/inputs excluded\n",(unsigned long long)sizeof(GD_EffectQuery),(unsigned long long)sizeof(GD_Query));
    gd_effect_graph_destroy(graph); gd_resource_graph_destroy(source); gd_rule_analysis_destroy(analysis); gd_rules_destroy(rules); gd_destroy(model);
)
