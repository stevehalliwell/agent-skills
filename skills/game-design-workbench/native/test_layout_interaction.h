TEST(layout_interaction,
    GD_Allocator allocator={storage,allocate,release};
    GD_Layout *layout=allocate(storage,sizeof(*layout));
    GD_LayoutFocus *focus=gd_layout_focus_create(allocator);
    GD_LayoutNode before; float midpoint[4]; int i;
    TEST_ASSERT(layout && focus); memset(layout,0,sizeof(*layout));
    layout->node_count=4; layout->group_count=1; layout->edge_count=2;
    for (i=0;i<4;++i) {
        layout->nodes[i].key=i; layout->nodes[i].label="interaction";
        layout->nodes[i].x=i*80.0f; layout->nodes[i].y=i*30.0f; layout->nodes[i].vx=3; layout->nodes[i].vy=5;
    }
    layout->edges[0]=(GD_LayoutEdge){0,1,0,1,0}; layout->edges[1]=(GD_LayoutEdge){1,2,1,1,0};
    layout->nodes[1].x=0.3f; layout->nodes[1].y=0.7f; before=layout->nodes[1];
    gd_layout_pickup_node(layout,1,0.2f,0.1f); gd_layout_drag_node(layout,1,0.2f,0.1f);
    TEST_ASSERT(!memcmp(&before,&layout->nodes[1],sizeof(before)));
    gd_layout_drag_node(layout,1,1.2f,2.1f);
    TEST_ASSERT(fabsf(layout->nodes[1].x-1.3f)<0.00001f && fabsf(layout->nodes[1].y-2.7f)<0.00001f);
    before=layout->nodes[1]; gd_layout_drag_node(layout,1,NAN,0);
    TEST_ASSERT(!memcmp(&before,&layout->nodes[1],sizeof(before)));
    gd_layout_focus_capture(focus,layout); gd_layout_focus_begin(focus,layout);
    TEST_ASSERT(gd_layout_focus_apply(focus,layout,NULL,1,1,1,1)==2);
    gd_layout_focus_advance(focus,layout,0,1);
    for (i=0;i<4;++i) TEST_ASSERT(layout->nodes[i].x==focus->base[i].x && layout->nodes[i].y==focus->base[i].y);
    gd_layout_focus_advance(focus,layout,0.11f,1);
    for (i=0;i<4;++i) {
        TEST_ASSERT(fabsf(layout->nodes[i].x-(focus->base[i].x+focus->focused[i].x)/2)<0.001f);
        if (focus->fixed_roots[i]) TEST_ASSERT(layout->nodes[i].x==focus->base[i].x && layout->nodes[i].y==focus->base[i].y);
        midpoint[i]=layout->nodes[i].x;
    }
    gd_layout_focus_begin(focus,layout); TEST_ASSERT(gd_layout_focus_apply(focus,layout,NULL,1,1,2,1)==2);
    gd_layout_focus_advance(focus,layout,0,1);
    for (i=0;i<4;++i) TEST_ASSERT(layout->nodes[i].x==midpoint[i]);
    gd_layout_focus_advance(focus,layout,NAN,1); TEST_ASSERT(focus->transition_time==0);
    gd_layout_focus_advance(focus,layout,-1,1); TEST_ASSERT(focus->transition_time==0);
    gd_layout_focus_advance(focus,layout,100,1); TEST_ASSERT(!focus->animating);
    for (i=0;i<4;++i) TEST_ASSERT(layout->nodes[i].x==focus->focused[i].x && layout->nodes[i].y==focus->focused[i].y);
    gd_layout_focus_begin(focus,layout); focus->active=0; focus->returning=1;
    gd_layout_focus_advance(focus,layout,0.22f,1); TEST_ASSERT(!focus->animating && !focus->returning);
    for (i=0;i<4;++i) TEST_ASSERT(layout->nodes[i].x==focus->base[i].x && layout->nodes[i].y==focus->base[i].y && layout->nodes[i].vx==focus->base[i].vx);
    /* Drag still obeys semantic bands. */
    layout->mode=GD_LAYOUT_DIRECTED; layout->nodes[1].axis=layout->nodes[1].y;
    gd_layout_pickup_node(layout,1,0,0); gd_layout_drag_node(layout,1,10,20);
    TEST_ASSERT(layout->nodes[1].y==layout->nodes[1].axis);
    gd_layout_focus_destroy(focus); release(storage,layout);
)
