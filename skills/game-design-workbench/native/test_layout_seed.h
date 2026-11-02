/* One-shot geometry tests, independent of any crossing optimizer. */
TEST(layout_neighborhood_seed,
    GD_Allocator allocator={storage,allocate,release};
    GD_Layout *layout=allocate(storage,sizeof(*layout)),*before=allocate(storage,sizeof(*before));
    GD_LayoutPoint positions[12]; int i,j;
    TEST_ASSERT(layout && before); memset(layout,0,sizeof(*layout));
    layout->node_count=12; layout->edge_count=14; layout->group_count=2;
    for(i=0;i<12;++i) {
        layout->nodes[i]=(GD_LayoutNode){.key=i,.label="node",.group=i%2,.layer=i};
        TEST_ASSERT(gd_layout_set_footprint(&layout->nodes[i],-20,-12,80,35));
    }
    for(i=0;i<6;++i) layout->edges[i]=(GD_LayoutEdge){0,i+1,i,1,GD_PREREQUISITES};
    layout->edges[6]=(GD_LayoutEdge){7,8,6,1,GD_NEXT}; layout->edges[7]=(GD_LayoutEdge){8,9,7,1,GD_NEXT};
    layout->edges[8]=(GD_LayoutEdge){0,0,8,1,GD_NEXT}; layout->edges[9]=(GD_LayoutEdge){6,7,9,0,GD_NEXT};
    layout->edges[10]=(GD_LayoutEdge){8,7,10,1,GD_NEXT};
    layout->edges[11]=(GD_LayoutEdge){9,7,11,1,GD_NEXT};layout->edges[12]=(GD_LayoutEdge){7,9,12,1,GD_NEXT};
    layout->edges[13]=(GD_LayoutEdge){1,2,13,1,GD_NEXT};
    TEST_ASSERT(gd_layout_arrange(layout,GD_LAYOUT_FORCE,GD_SORT_NAME)); memcpy(before,layout,sizeof(*before));
    TEST_ASSERT(gd_layout_seed_neighborhood(layout,allocator,2));
    TEST_ASSERT(!memcmp(layout->edges,before->edges,sizeof(layout->edges)) && !memcmp(layout->order,before->order,sizeof(layout->order)));
    TEST_ASSERT(!memcmp(layout->position,before->position,sizeof(layout->position)) && layout->group_count==2);
    for(i=0;i<12;++i) {
        GD_LayoutNode expected=before->nodes[i];
        expected.x=layout->nodes[i].x; expected.y=layout->nodes[i].y; expected.vx=expected.vy=0;
        TEST_ASSERT(!memcmp(&expected,&layout->nodes[i],sizeof(expected)));
        positions[i]=(GD_LayoutPoint){expected.x,expected.y};
        for(j=0;j<i;++j) TEST_ASSERT(!collision_overlaps(&layout->nodes[i],&layout->nodes[j]));
    }
    for(i=0;i<2;++i) TEST_ASSERT(layout->groups[i].first==before->groups[i].first && layout->groups[i].count==before->groups[i].count);
    for(i=0;i<12;++i) {layout->nodes[i].x=1000+i;layout->nodes[i].y=-1000-i;layout->nodes[i].vx=3;}
    TEST_ASSERT(gd_layout_seed_neighborhood(layout,allocator,2));
    for(i=0;i<12;++i) TEST_ASSERT(layout->nodes[i].x==positions[i].x && layout->nodes[i].y==positions[i].y && layout->nodes[i].vx==0);
    /* Distinct-neighbor tie picks 8, not parallel-edge-biased 7. */
    TEST_ASSERT(layout->nodes[7].x>layout->nodes[8].x && layout->nodes[9].y>layout->nodes[8].y);
    release(storage,before); release(storage,layout);
)
TEST(layout_neighborhood_aspect_capacity,
    GD_Allocator allocator={storage,allocate,release}; GD_Layout *layout=allocate(storage,sizeof(*layout)); int i;
    TEST_ASSERT(layout); memset(layout,0,sizeof(*layout)); layout->node_count=64; layout->group_count=1;
    for(i=0;i<64;++i) {layout->nodes[i]=(GD_LayoutNode){.key=i,.label="isolated"};TEST_ASSERT(gd_layout_set_footprint(&layout->nodes[i],-60,-25,60,25));}
    TEST_ASSERT(gd_layout_arrange(layout,GD_LAYOUT_FORCE,GD_SORT_NAME));
    TEST_ASSERT(gd_layout_seed_neighborhood(layout,allocator,4) && layout->width>layout->height);
    TEST_ASSERT(gd_layout_seed_neighborhood(layout,allocator,0.25f) && layout->width<layout->height && layout->group_count==1);
    memset(layout,0,sizeof(*layout));layout->node_count=GD_MAX_RECORDS;layout->edge_count=GD_MAX_PAIRS;layout->group_count=1;
    for(i=0;i<layout->node_count;++i) {layout->nodes[i]=(GD_LayoutNode){.key=i,.label="capacity"};TEST_ASSERT(gd_layout_set_footprint(&layout->nodes[i],-70,-25,70,25));}
    for(i=0;i<layout->edge_count;++i) layout->edges[i]=(GD_LayoutEdge){0,1+i%(GD_MAX_RECORDS-1),i,1,GD_NEXT};
    TEST_ASSERT(gd_layout_arrange(layout,GD_LAYOUT_FORCE,GD_SORT_NAME) && gd_layout_seed_neighborhood(layout,allocator,1.5f));
    for(i=0;i<layout->node_count;++i) TEST_ASSERT(isfinite(layout->nodes[i].x) && isfinite(layout->nodes[i].y) && layout->nodes[i].key==i);
    memset(layout,0,sizeof(*layout)); TEST_ASSERT(gd_layout_arrange(layout,GD_LAYOUT_FORCE,GD_SORT_NAME) && gd_layout_seed_neighborhood(layout,allocator,1));
    TEST_ASSERT(layout->width==0 && layout->height==0); release(storage,layout);
)
TEST(layout_neighborhood_failure,
    GD_Allocator allocator={storage,allocate,release},failing=allocator; RouteFailure failure={allocator,0};
    GD_Layout *layout=allocate(storage,sizeof(*layout)),*before=allocate(storage,sizeof(*before));
    TEST_ASSERT(layout && before); memset(layout,0,sizeof(*layout));layout->node_count=1;layout->group_count=1;layout->nodes[0].label="one";
    TEST_ASSERT(gd_layout_arrange(layout,GD_LAYOUT_FORCE,GD_SORT_NAME));memcpy(before,layout,sizeof(*before));
    failing.context=&failure;failing.allocate=route_allocate;failing.release=route_release;
    TEST_ASSERT(!gd_layout_seed_neighborhood(layout,failing,1) && !memcmp(layout,before,sizeof(*layout)));
    TEST_ASSERT(!gd_layout_seed_neighborhood(layout,allocator,NAN) && !memcmp(layout,before,sizeof(*layout)));
    TEST_ASSERT(!gd_layout_seed_neighborhood(layout,allocator,0) && !memcmp(layout,before,sizeof(*layout)));
    layout->edge_count=1;layout->edges[0].b=2;memcpy(before,layout,sizeof(*before));
    TEST_ASSERT(!gd_layout_seed_neighborhood(layout,allocator,1) && !memcmp(layout,before,sizeof(*layout)));
    layout->edge_count=0;layout->mode=GD_LAYOUT_DIRECTED;memcpy(before,layout,sizeof(*before));
    TEST_ASSERT(!gd_layout_seed_neighborhood(layout,allocator,1) && !memcmp(layout,before,sizeof(*layout)));
    release(storage,before);release(storage,layout);
)
