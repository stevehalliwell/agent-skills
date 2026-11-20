/* Higher-level grouping is active-edge connectivity, not adapter groups. */
static int island_boxes_overlap(GD_LayoutBounds a,GD_LayoutBounds b){return a.left<b.right && b.left<a.right && a.top<b.bottom && b.top<a.bottom;}
static void island_fixture(GD_Layout *layout){
    int i;memset(layout,0,sizeof(*layout));layout->node_count=4;layout->group_count=1;layout->edge_count=5;
    for(i=0;i<4;++i){layout->nodes[i]=(GD_LayoutNode){.key=i,.label="island"};gd_layout_set_footprint(&layout->nodes[i],-10,-10,10,10);}
    layout->edges[0]=(GD_LayoutEdge){0,1,0,1,GD_NEXT};layout->edges[1]=(GD_LayoutEdge){2,3,1,1,GD_NEXT};
    layout->edges[2]=(GD_LayoutEdge){1,2,2,0,GD_NEXT};layout->edges[3]=(GD_LayoutEdge){1,0,3,1,GD_NEXT};layout->edges[4]=(GD_LayoutEdge){0,0,4,1,GD_NEXT};
    gd_layout_arrange(layout,GD_LAYOUT_FORCE,GD_SORT_NAME);
    layout->nodes[0].x=-100;layout->nodes[1].x=100;layout->nodes[0].y=layout->nodes[1].y=0;
    layout->nodes[2].y=-100;layout->nodes[3].y=100;layout->nodes[2].x=layout->nodes[3].x=0;
}
TEST(layout_island_force,
    GD_Layout *on=allocate(storage,sizeof(*on)),*off=allocate(storage,sizeof(*off));int i,root;
    TEST_ASSERT(on && off);island_fixture(off);memcpy(on,off,sizeof(*on));
    TEST_ASSERT(!off->islands.enabled && gd_layout_islands_set(on,1) && on->islands.count==2);
    TEST_ASSERT(!memcmp(on->nodes,off->nodes,sizeof(on->nodes)) && !memcmp(on->edges,off->edges,sizeof(on->edges)));
    TEST_ASSERT(on->islands.component[0]==on->islands.component[1] && on->islands.component[0]!=on->islands.component[2]);
    gd_layout_step(on,-1);gd_layout_step(off,-1);TEST_ASSERT(on->islands.overlaps==1);
    for(i=0;i<4;++i){root=on->islands.component[i];TEST_ASSERT(fabsf(on->nodes[i].x-off->nodes[i].x-on->islands.force[root].x)<0.0001f);TEST_ASSERT(fabsf(on->nodes[i].y-off->nodes[i].y-on->islands.force[root].y)<0.0001f);TEST_ASSERT(fabsf(on->islands.force[root].x)<=6 && fabsf(on->islands.force[root].y)<=6);}
    for(i=0;i<600;++i)gd_layout_step(on,-1);
    TEST_ASSERT(!island_boxes_overlap(on->islands.bounds[on->islands.component[0]],on->islands.bounds[on->islands.component[2]]));
    memcpy(off,on,sizeof(*off));TEST_ASSERT(gd_layout_islands_set(on,0) && !memcmp(on->nodes,off->nodes,sizeof(on->nodes)));
    TEST_ASSERT(!gd_layout_islands_set(on,2) && !on->islands.enabled);
    island_fixture(off);off->nodes[2].x+=1000;off->nodes[3].x+=1000;memcpy(on,off,sizeof(*on));gd_layout_islands_set(on,1);
    for(i=0;i<12;++i){gd_layout_step(on,-1);gd_layout_step(off,-1);TEST_ASSERT(on->islands.overlaps==0 && !memcmp(on->nodes,off->nodes,sizeof(on->nodes)));}
    island_fixture(off);memcpy(on,off,sizeof(*on));gd_layout_islands_set(on,1);gd_layout_step(on,0);gd_layout_step(off,0);
    TEST_ASSERT(on->nodes[0].x==off->nodes[0].x && on->nodes[1].x==off->nodes[1].x);
    TEST_ASSERT(on->islands.force[on->islands.component[0]].x==0);
    release(storage,off);release(storage,on);
)
TEST(layout_island_single_and_workspace,
    GD_Layout *on=allocate(storage,sizeof(*on)),*off=allocate(storage,sizeof(*off));int i;
    TEST_ASSERT(on && off);island_fixture(off);off->edges[2].active=1;memcpy(on,off,sizeof(*on));
    TEST_ASSERT(gd_layout_islands_set(on,1) && on->islands.count==1);
    for(i=0;i<120;++i){gd_layout_step(on,-1);gd_layout_step(off,-1);TEST_ASSERT(!memcmp(on->nodes,off->nodes,sizeof(on->nodes)));}
    island_fixture(on);gd_layout_islands_set(on,1);on->workspace=(GD_LayoutWorkspace){{-110,-110,110,110},1};
    for(i=0;i<120;++i){int n;gd_layout_step(on,-1);for(n=0;n<4;++n){const GD_LayoutNode *node=&on->nodes[n];TEST_ASSERT(node->x+node->footprint.left>=-110.001f && node->x+node->footprint.right<=110.001f && node->y+node->footprint.top>=-110.001f && node->y+node->footprint.bottom<=110.001f);}}
    TEST_ASSERT(gd_layout_arrange(on,GD_LAYOUT_DIRECTED,GD_SORT_NAME) && !on->islands.enabled && on->islands.preferred);
    release(storage,off);release(storage,on);
)
TEST(layout_island_budget,
    GD_Layout *layout=allocate(storage,sizeof(*layout));int i;
    TEST_ASSERT(layout);memset(layout,0,sizeof(*layout));layout->node_count=GD_MAX_RECORDS;layout->group_count=1;
    for(i=0;i<layout->node_count;++i){layout->nodes[i]=(GD_LayoutNode){.key=i,.label="isolated"};gd_layout_set_footprint(&layout->nodes[i],-100,-100,100,100);}
    TEST_ASSERT(gd_layout_arrange(layout,GD_LAYOUT_FORCE,GD_SORT_NAME));for(i=0;i<layout->node_count;++i)layout->nodes[i].x=layout->nodes[i].y=0;
    TEST_ASSERT(gd_layout_islands_set(layout,1) && layout->islands.count==GD_MAX_RECORDS);gd_layout_step(layout,-1);
    TEST_ASSERT(layout->islands.limited && layout->islands.checks==65536);
    for(i=0;i<layout->node_count;++i)TEST_ASSERT(isfinite(layout->nodes[i].x) && isfinite(layout->nodes[i].y));release(storage,layout);
)
