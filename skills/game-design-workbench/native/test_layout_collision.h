/* Rectangle projection tests are independent of renderer and edge crossings. */
static int collision_overlaps(const GD_LayoutNode *a,const GD_LayoutNode *b) {
    return a->x+a->footprint.right>b->x+b->footprint.left &&
        b->x+b->footprint.right>a->x+a->footprint.left &&
        a->y+a->footprint.bottom>b->y+b->footprint.top &&
        b->y+b->footprint.bottom>a->y+a->footprint.top;
}
TEST(layout_footprints,
    GD_LayoutNode node={0}; GD_LayoutBounds before;
    TEST_ASSERT(gd_layout_set_footprint(&node,-10,-5,130,60)); before=node.footprint;
    TEST_ASSERT(!gd_layout_set_footprint(&node,NAN,0,1,1));
    TEST_ASSERT(!gd_layout_set_footprint(&node,0,0,INFINITY,1));
    TEST_ASSERT(!gd_layout_set_footprint(&node,0,0,0,1));
    TEST_ASSERT(!gd_layout_set_footprint(&node,1,0,0,1));
    TEST_ASSERT(!gd_layout_set_footprint(&node,0,0,1048577,1));
    TEST_ASSERT(!memcmp(&node.footprint,&before,sizeof(before)));
)
TEST(layout_rectangle_separation,
    GD_Layout *layout=allocate(storage,sizeof(*layout)); int held,step;
    TEST_ASSERT(layout);
    for (held=0;held<2;++held) {
        memset(layout,0,sizeof(*layout)); layout->node_count=2; layout->group_count=2;
        layout->nodes[0]=(GD_LayoutNode){.key=111,.label="left",.group=0};
        layout->nodes[1]=(GD_LayoutNode){.key=222,.label="right",.group=1};
        TEST_ASSERT(gd_layout_set_footprint(&layout->nodes[0],-10,-5,130,60));
        TEST_ASSERT(gd_layout_set_footprint(&layout->nodes[1],-30,-20,20,30));
        TEST_ASSERT(gd_layout_arrange(layout,GD_LAYOUT_FORCE,GD_SORT_NAME));
        layout->nodes[0].x=layout->nodes[1].x=0; layout->nodes[0].y=layout->nodes[1].y=0;
        for (step=0;step<120;++step) {
            gd_layout_step(layout,held);
            if (step>80) TEST_ASSERT(!collision_overlaps(&layout->nodes[0],&layout->nodes[1]));
            TEST_ASSERT(layout->nodes[held].x==0 && layout->nodes[held].y==0);
            TEST_ASSERT(layout->collision_checks<=GD_LAYOUT_COLLISION_WORK);
        }
        TEST_ASSERT(layout->nodes[0].key==111 && layout->nodes[1].key==222);
        TEST_ASSERT(layout->nodes[0].group==0 && layout->nodes[1].group==1);
        /* Full containment must move by escape distance, not intersection width. */
        TEST_ASSERT(gd_layout_set_footprint(&layout->nodes[0],-200,-200,200,200));
        TEST_ASSERT(gd_layout_set_footprint(&layout->nodes[1],-5,-5,5,5));
        layout->nodes[0].x=layout->nodes[1].x=layout->nodes[0].y=layout->nodes[1].y=0;
        for (step=0;step<120;++step) gd_layout_step(layout,held);
        TEST_ASSERT(!collision_overlaps(&layout->nodes[0],&layout->nodes[1]));
    }
    release(storage,layout);
)
TEST(layout_collision_components,
    GD_Layout *layout=allocate(storage,sizeof(*layout)); GD_LayoutEdge edges[3]; int i;
    TEST_ASSERT(layout); memset(layout,0,sizeof(*layout)); layout->node_count=5; layout->edge_count=3;
    for (i=0;i<5;++i) layout->nodes[i]=(GD_LayoutNode){.key=i,.label="component",.width=250,.height=72};
    layout->edges[0]=(GD_LayoutEdge){0,1,70,1,0}; layout->edges[1]=(GD_LayoutEdge){2,3,71,1,0};
    layout->edges[2]=(GD_LayoutEdge){1,2,72,0,0}; memcpy(edges,layout->edges,sizeof(edges));
    TEST_ASSERT(gd_layout_components(layout) && layout->group_count==3);
    TEST_ASSERT(layout->nodes[0].group==layout->nodes[1].group && layout->nodes[2].group==layout->nodes[3].group);
    TEST_ASSERT(layout->nodes[0].group!=layout->nodes[2].group && layout->nodes[4].group!=layout->nodes[2].group);
    TEST_ASSERT(!memcmp(edges,layout->edges,sizeof(edges)));
    TEST_ASSERT(gd_layout_arrange(layout,GD_LAYOUT_FORCE,GD_SORT_NAME));
    TEST_ASSERT(layout->groups[layout->nodes[4].group].width>=282);
    layout->edges[2].b=99; TEST_ASSERT(!gd_layout_components(layout));
    release(storage,layout);
)
TEST(layout_collision_constraints,
    GD_Layout *layout=allocate(storage,sizeof(*layout)); int mode,i; float before;
    TEST_ASSERT(layout);
    for (mode=GD_LAYOUT_COLUMNS;mode<=GD_LAYOUT_DIRECTED;++mode) {
        memset(layout,0,sizeof(*layout)); layout->node_count=2; layout->group_count=1;
        for (i=0;i<2;++i) layout->nodes[i]=(GD_LayoutNode){.key=i,.label="constrained",.layer=i,.width=250,.height=72};
        TEST_ASSERT(gd_layout_arrange(layout,(GD_LayoutMode)mode,GD_SORT_NAME));
        before=layout->nodes[0].y; gd_layout_step(layout,-1);
        TEST_ASSERT(layout->collision_checks==0 && layout->nodes[0].y==before);
    }
    release(storage,layout);
)
TEST(layout_collision_budget,
    GD_Layout *layout=allocate(storage,sizeof(*layout)); unsigned cursor; int i;
    TEST_ASSERT(layout); memset(layout,0,sizeof(*layout));
    layout->node_count=GD_MAX_RECORDS; layout->group_count=1;
    for (i=0;i<layout->node_count;++i) layout->nodes[i]=(GD_LayoutNode){.key=i,.label="dense",.width=250,.height=72};
    TEST_ASSERT(gd_layout_arrange(layout,GD_LAYOUT_FORCE,GD_SORT_NAME));
    for (i=0;i<layout->node_count;++i) layout->nodes[i].x=layout->nodes[i].y=0;
    gd_layout_step(layout,-1); cursor=layout->collision_cursor;
    for (i=0;i<layout->node_count;++i) TEST_ASSERT(sqrtf(layout->nodes[i].x*layout->nodes[i].x+layout->nodes[i].y*layout->nodes[i].y)<=25.01f);
    TEST_ASSERT(layout->collision_limited && layout->collision_checks==GD_LAYOUT_COLLISION_WORK);
    for (i=1;i<layout->node_count;++i) {
        TEST_ASSERT(layout->collision_order[i-1].minimum<=layout->collision_order[i].minimum);
        if (layout->collision_order[i-1].minimum==layout->collision_order[i].minimum) TEST_ASSERT(layout->collision_order[i-1].node<layout->collision_order[i].node);
    }
    gd_layout_step(layout,-1);
    TEST_ASSERT(layout->collision_checks<=GD_LAYOUT_COLLISION_WORK && layout->collision_cursor!=cursor);
    for (i=0;i<layout->node_count;++i) TEST_ASSERT(isfinite(layout->nodes[i].x) && isfinite(layout->nodes[i].y));
    TEST_ASSERT(sizeof(layout->collision_order)+sizeof(layout->collision_remaining)<1024*1024);
    release(storage,layout);
)
