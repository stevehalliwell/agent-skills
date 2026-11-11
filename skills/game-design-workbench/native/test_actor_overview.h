/* Explicit noun classification and same-case/record-level overview projection. */
static int overview_find_node(const GD_QueryScene *scene,const char *reference) {
    int i;for(i=0;i<scene->layout.node_count;++i) if(!strcmp(scene->query->model->records[scene->nodes[i].source].reference,reference)) return i;return -1;
}
static int overview_find_edge(const GD_QueryScene *scene,int a,int b) {
    int i;for(i=0;i<scene->layout.edge_count;++i) {const GD_LayoutEdge *edge=&scene->layout.edges[i];if((edge->a==a && edge->b==b) || (edge->a==b && edge->b==a)) return i;}return -1;
}
TEST(actor_overview_pairs_cloud_and_evidence,
    GD_Model *model=model_create();GD_Rules *rules;GD_RuleAnalysis *analysis;GD_Query *query;GD_QueryScene *map,*cloud;
    GD_QueryRequest request={0};int hero,walker,lives,unknown,isolated,edge,i,c,matching=0;
    TEST_ASSERT(model && setup("actor-overview"));collections[1].role=GD_AXIS;collections[2].role=GD_RULE;
    TEST_ASSERT(write_record(0,"hero","---\nis_actor: true\naxes: [actions/Life, actions/Life]\n---\nHero."));
    TEST_ASSERT(write_record(0,"walker","---\nis_actor: true\naxes: [actions/Life]\n---\nWalker."));
    TEST_ASSERT(write_record(0,"isolated","---\nis_actor: true\naxes: []\n---\nIsolated."));
    TEST_ASSERT(write_record(0,"lives","---\nis_actor: false\naxes: []\n---\nNot an actor."));
    TEST_ASSERT(write_record(0,"unknown","---\naxes: []\n---\nUnclassified."));
    TEST_ASSERT(write_record(1,"Life","---\nvalue_type: enum\nallowed_values: [alive, dead]\nclosed: true\n---\n"));
    TEST_ASSERT(write_record(2,"one","```gd-rule\ncase first:\n    target: A\n    context: ANY\n    A:\n        noun: things/hero\n        requires: ANY\n    B:\n        noun: things/walker\n        requires: ANY\n    C:\n        noun: things/lives\n        requires: ANY\n    D:\n        noun: things/unknown\n        requires: ANY\n    Copy:\n        noun: things/hero\n        requires: ANY\ncase repeat:\n    target: A\n    context: ANY\n    A:\n        noun: things/hero\n        requires: ANY\n    B:\n        noun: things/walker\n        requires: ANY\n```\n"));
    TEST_ASSERT(write_record(2,"two","```gd-rule\ncase second_record:\n    target: A\n    context: ANY\n    A:\n        noun: things/hero\n        requires: ANY\n    B:\n        noun: things/walker\n        requires: ANY\n```\n"));
    TEST_ASSERT(write_record(2,"separate","```gd-rule\ncase only_hero:\n    target: A\n    context: ANY\n    A:\n        noun: things/hero\n        requires: ANY\ncase only_isolated:\n    target: B\n    context: ANY\n    B:\n        noun: things/isolated\n        requires: ANY\n```\n"));
    TEST_ASSERT(gd_load(model,collections,3));model->collections[2].role=GD_CASE; /* Match explorer normalization/restoration. */
    rules=gd_read_rules(model);analysis=gd_analyze_rules(model,rules);
    TEST_ASSERT(rules && analysis && !model->error_count);
    request.kind=GD_QUERY_BINDINGS;request.noun=request.axis=request.case_index=request.effect_kind=-1;request.work_limit=GD_ACTOR_WORK;
    query=gd_query_build(model,rules,analysis,&request);TEST_ASSERT(query!=NULL);
    map=gd_query_scene_overview(query,GD_ACTOR_INTERACTIONS,GD_EFFECT_WORK);cloud=gd_query_scene_overview(query,GD_ACTOR_USAGE,GD_EFFECT_WORK);
    TEST_ASSERT(map && cloud && map->layout.node_count==3 && map->layout.edge_count==1 && !map->limited && gd_query_scene_overview_unclassified(map)==1);
    hero=overview_find_node(map,"things/hero");walker=overview_find_node(map,"things/walker");isolated=overview_find_node(map,"things/isolated");
    TEST_ASSERT(hero>=0 && walker>=0 && isolated>=0 && overview_find_node(map,"things/lives")==-1);
    edge=overview_find_edge(map,hero,walker);TEST_ASSERT(edge>=0 && gd_query_scene_overview_count(map,edge)==2);
    for(c=0;c<rules->case_count;++c) matching+=gd_query_scene_overview_case(map,edge,c);TEST_ASSERT(matching==3);
    TEST_ASSERT(overview_find_edge(map,hero,isolated)==-1);
    {GD_Layout *ordinary=model->allocator.allocate(model->allocator.context,sizeof(*ordinary)),*weighted=model->allocator.allocate(model->allocator.context,sizeof(*weighted));int counts[1]={8};
     TEST_ASSERT(ordinary && weighted);memset(ordinary,0,sizeof(*ordinary));ordinary->node_count=2;ordinary->edge_count=1;ordinary->group_count=1;
     ordinary->nodes[0]=(GD_LayoutNode){0};ordinary->nodes[0].label="A";ordinary->nodes[1].label="B";ordinary->nodes[1].key=1;
     ordinary->edges[0]=(GD_LayoutEdge){0,1,0,1,0};TEST_ASSERT(gd_layout_arrange(ordinary,GD_LAYOUT_FORCE,GD_SORT_NAME));
     ordinary->nodes[0].x=-220;ordinary->nodes[1].x=220;ordinary->nodes[0].y=ordinary->nodes[1].y=0;memcpy(weighted,ordinary,sizeof(*weighted));
     gd_layout_step(ordinary,-1);gd_layout_step_weighted(weighted,-1,counts);TEST_ASSERT(weighted->nodes[0].x>ordinary->nodes[0].x);
     model->allocator.release(model->allocator.context,weighted);model->allocator.release(model->allocator.context,ordinary);}
    TEST_ASSERT(gd_query_scene_place(map,GD_LAYOUT_DIRECTED,1,1.5f) && map->layout.mode==GD_LAYOUT_FORCE);
    for(i=0;i<600;++i) gd_query_scene_step(map,-1);
    for(i=0;i<map->layout.node_count;++i) TEST_ASSERT(isfinite(map->layout.nodes[i].x) && isfinite(map->layout.nodes[i].y));
    TEST_ASSERT(gd_query_scene_overview_count(map,edge)==2 && map->query==query);
    hero=overview_find_node(cloud,"things/hero");walker=overview_find_node(cloud,"things/walker");lives=overview_find_node(cloud,"things/lives");unknown=overview_find_node(cloud,"things/unknown");
    TEST_ASSERT(cloud->layout.node_count==6 && cloud->layout.edge_count==6 && lives>=0 && unknown>=0);
    TEST_ASSERT(cloud->nodes[lives].incoming==2 && cloud->nodes[unknown].incoming==2);
    TEST_ASSERT(cloud->layout.nodes[lives].width>cloud->layout.nodes[hero].width && cloud->layout.nodes[lives].width<=400 && cloud->layout.nodes[lives].height<=160);
    i=overview_find_node(cloud,"actions/Life");TEST_ASSERT(i>=0 && cloud->nodes[i].incoming==2 && cloud->nodes[i].kind==GD_QUERY_NODE_AXIS);
    TEST_ASSERT(overview_find_edge(cloud,hero,walker)==-1 && overview_find_edge(cloud,lives,unknown)==-1);
    TEST_ASSERT(gd_query_scene_overview_count(cloud,overview_find_edge(cloud,hero,lives))==1);
    {
        GD_Allocator allocator=cloud->allocator;GD_Layout *native=allocator.allocate(allocator.context,sizeof(*native)),*before=allocator.allocate(allocator.context,sizeof(*before));
        GD_QuerySceneNode metadata[6];GD_QuerySceneEdge edges[6];int counts[6],seed,stage,step;
        TEST_ASSERT(native && before);memcpy(native,&cloud->layout,sizeof(*native));memcpy(metadata,cloud->nodes,sizeof(metadata));memcpy(edges,cloud->edges,sizeof(edges));
        for(i=0;i<6;++i)counts[i]=gd_query_scene_overview_count(cloud,i);
        TEST_ASSERT(gd_query_scene_place(cloud,GD_LAYOUT_DIRECTED,0,1));gd_query_scene_step(cloud,-1);gd_query_scene_drag(cloud,hero,native->nodes[hero].x+100,native->nodes[hero].y+100);
        TEST_ASSERT(cloud->layout.nodes[hero].x==native->nodes[hero].x && cloud->layout.nodes[hero].y==native->nodes[hero].y);
        for(seed=0;seed<2;++seed){
            TEST_ASSERT(gd_query_scene_place(cloud,GD_LAYOUT_FORCE,seed,1.4f) && cloud->free_geometry && cloud->layout.mode==GD_LAYOUT_FORCE);
            TEST_ASSERT(gd_query_scene_workspace(cloud,1,1.4f));
            for(i=0;i<6;++i){float x=cloud->layout.nodes[i].x,y=cloud->layout.nodes[i].y;gd_layout_pickup_node(&cloud->layout,i,x,y);gd_query_scene_drag(cloud,i,x+50,y+20);TEST_ASSERT(fabsf(cloud->layout.nodes[i].x-x-50)<0.001f && fabsf(cloud->layout.nodes[i].y-y-20)<0.001f);}
            for(step=0;step<600;++step)gd_query_scene_step(cloud,-1);
            TEST_ASSERT(cloud->layout.workspace.enabled && cloud->layout.collision_checks<=GD_LAYOUT_COLLISION_WORK);
            for(i=0;i<6;++i){const GD_LayoutNode *node=&cloud->layout.nodes[i];TEST_ASSERT(isfinite(node->x) && isfinite(node->y) && node->x+node->footprint.left>=cloud->layout.workspace.bounds.left-0.01f && node->x+node->footprint.right<=cloud->layout.workspace.bounds.right+0.01f && node->y+node->footprint.top>=cloud->layout.workspace.bounds.top-0.01f && node->y+node->footprint.bottom<=cloud->layout.workspace.bounds.bottom+0.01f);}
            memcpy(before,&cloud->layout,sizeof(*before));
            for(stage=0;stage<3;++stage){RouteFailure failure={allocator,stage};GD_QueryFree *prior=cloud->free_geometry;cloud->allocator=(GD_Allocator){&failure,route_allocate,route_release};TEST_ASSERT(!gd_query_scene_place(cloud,GD_LAYOUT_FORCE,1,1.4f));cloud->allocator=allocator;TEST_ASSERT(cloud->free_geometry==prior && !memcmp(before,&cloud->layout,sizeof(*before)));}
            TEST_ASSERT(!gd_query_scene_place(cloud,GD_LAYOUT_FORCE,2,1) && !gd_query_scene_place(cloud,GD_LAYOUT_FORCE,0,NAN));
            TEST_ASSERT(gd_query_scene_place(cloud,GD_LAYOUT_DIRECTED,seed,1.4f) && !cloud->free_geometry && !cloud->layout.workspace.enabled);
            for(i=0;i<6;++i)TEST_ASSERT(cloud->layout.nodes[i].x==native->nodes[i].x && cloud->layout.nodes[i].y==native->nodes[i].y && cloud->layout.nodes[i].vx==0 && cloud->layout.nodes[i].vy==0 && gd_query_scene_overview_count(cloud,i)==counts[i]);
            TEST_ASSERT(!memcmp(metadata,cloud->nodes,sizeof(metadata)) && !memcmp(edges,cloud->edges,sizeof(edges)) && !memcmp(native->edges,cloud->layout.edges,6*sizeof(native->edges[0])) && cloud->query==query);
        }
        allocator.release(allocator.context,before);allocator.release(allocator.context,native);
    }
    {char path[GD_MAX_PATH];FILE *file;snprintf(path,sizeof(path),"%s/actor-overview.json",root);file=fopen(path,"wb");TEST_ASSERT(file && gd_query_scene_overview_write(file,cloud));TEST_ASSERT(!fclose(file));}
    TEST_ASSERT(!gd_sources_changed(model));
    gd_query_scene_destroy(cloud);gd_query_scene_destroy(map);gd_query_destroy(query);gd_rule_analysis_destroy(analysis);gd_rules_destroy(rules);gd_destroy(model);
)
TEST(actor_overview_limits_and_allocations,
    GD_Model *model=model_create();GD_Rules *rules;GD_RuleAnalysis *analysis;GD_Query *query;GD_QueryScene *scene;GD_QueryRequest request={0};int stage;
    TEST_ASSERT(model && setup("actor-overview-limits"));collections[1].role=GD_AXIS;collections[2].role=GD_RULE;
    TEST_ASSERT(write_record(0,"hero","---\nis_actor: true\naxes: []\n---\n"));
    TEST_ASSERT(write_record(0,"walker","---\nis_actor: true\naxes: []\n---\n"));
    TEST_ASSERT(write_record(2,"helper","```gd-rule\ncase helper:\n    target: A\n    context: ANY\n    A:\n        noun: things/hero\n        requires: ANY\n    B:\n        noun: things/walker\n        requires: ANY\n```\n"));
    TEST_ASSERT(gd_load(model,collections,3));rules=gd_read_rules(model);analysis=gd_analyze_rules(model,rules);TEST_ASSERT(rules && analysis && !model->error_count);
    request.kind=GD_QUERY_BINDINGS;request.noun=request.axis=request.case_index=request.effect_kind=-1;request.work_limit=GD_ACTOR_WORK;
    query=gd_query_build(model,rules,analysis,&request);TEST_ASSERT(query);
    scene=gd_query_scene_overview(query,GD_ACTOR_INTERACTIONS,GD_EFFECT_WORK);TEST_ASSERT(scene && scene->layout.node_count==2 && !scene->layout.edge_count);gd_query_scene_destroy(scene);
    scene=gd_query_scene_overview(query,GD_ACTOR_INTERACTIONS,0);TEST_ASSERT(scene && scene->limited && gd_query_scene_overview_incomplete(scene) && !scene->layout.node_count);gd_query_scene_destroy(scene);
    {GD_Allocator original=query->allocator;
     for(stage=0;stage<3;++stage) {RouteFailure failure={original,stage};query->allocator=(GD_Allocator){&failure,route_allocate,route_release};scene=gd_query_scene_overview(query,GD_ACTOR_USAGE,GD_EFFECT_WORK);query->allocator=original;TEST_ASSERT(!scene);}}
    TEST_ASSERT(!gd_query_scene_overview(query,(GD_ActorOverviewMode)3,GD_EFFECT_WORK));
    gd_query_destroy(query);gd_rule_analysis_destroy(analysis);gd_rules_destroy(rules);gd_destroy(model);
)
