TEST(layout_workspace,
    GD_Layout *layout=allocate(storage,sizeof(*layout)),*control=allocate(storage,sizeof(*control));GD_LayoutNode nodes[3];GD_LayoutWorkspace before;int i;double area,width,height;
    TEST_ASSERT(layout && control);memset(layout,0,sizeof(*layout));layout->node_count=3;layout->edge_count=2;layout->group_count=1;
    for(i=0;i<3;++i)layout->nodes[i]=(GD_LayoutNode){.key=i,.label="workspace",.width=240,.height=72};
    layout->edges[0]=(GD_LayoutEdge){0,1,0,1,0};layout->edges[1]=(GD_LayoutEdge){0,2,1,1,0};
    TEST_ASSERT(gd_layout_arrange(layout,GD_LAYOUT_FORCE,GD_SORT_NAME) && !layout->workspace.enabled);
    memcpy(nodes,layout->nodes,sizeof(nodes));TEST_ASSERT(gd_layout_workspace(layout,1,1.4f));before=layout->workspace;
    TEST_ASSERT(!memcmp(nodes,layout->nodes,sizeof(nodes)) && before.enabled);
    width=(double)before.bounds.right-before.bounds.left;height=(double)before.bounds.bottom-before.bounds.top;area=3*240*72*(4+4.0/3)*1.44;
    TEST_ASSERT(fabs(width/height-1.4)<0.00001 && fabs(width*height-area)/area<0.00001);
    for(i=0;i<3;++i)TEST_ASSERT(nodes[i].x+nodes[i].footprint.left>before.bounds.left && nodes[i].x+nodes[i].footprint.right<before.bounds.right && nodes[i].y+nodes[i].footprint.top>before.bounds.top && nodes[i].y+nodes[i].footprint.bottom<before.bounds.bottom);
    layout->edge_count=10;for(i=0;i<10;++i)layout->edges[i]=(GD_LayoutEdge){0,1,i,1,0};TEST_ASSERT(gd_layout_workspace(layout,1,1.4f));
    area=((double)layout->workspace.bounds.right-layout->workspace.bounds.left)*((double)layout->workspace.bounds.bottom-layout->workspace.bounds.top);TEST_ASSERT(fabs(area-3*240*72*8*1.44)/(3*240*72*8*1.44)<0.00001);
    layout->edge_count=2;layout->edges[1]=(GD_LayoutEdge){0,2,1,1,0};TEST_ASSERT(gd_layout_workspace(layout,1,1.4f) && !memcmp(&before,&layout->workspace,sizeof(before)));
    {float invalid[]={0,-1,NAN,INFINITY};for(i=0;i<4;++i)TEST_ASSERT(!gd_layout_workspace(layout,1,invalid[i]) && !memcmp(&before,&layout->workspace,sizeof(before)));}
    TEST_ASSERT(!gd_layout_workspace(layout,2,1) && !memcmp(&before,&layout->workspace,sizeof(before)));
    layout->edges[0].a=-1;TEST_ASSERT(!gd_layout_workspace(layout,1,1) && !memcmp(&before,&layout->workspace,sizeof(before)));layout->edges[0].a=0;
    layout->nodes[0].footprint.left=NAN;TEST_ASSERT(!gd_layout_workspace(layout,1,1) && !memcmp(&before,&layout->workspace,sizeof(before)));layout->nodes[0]=nodes[0];
    layout->edge_count=0; /* Isolate boundary projection from springs. */
    layout->nodes[0].x=before.bounds.right+100;layout->nodes[0].vx=layout->nodes[0].vy=0;memcpy(control,layout,sizeof(*layout));TEST_ASSERT(gd_layout_workspace(control,0,0));
    gd_layout_step(layout,-1);gd_layout_step(control,-1);TEST_ASSERT(layout->nodes[0].x<=before.bounds.right-layout->nodes[0].footprint.right && layout->nodes[0].x<control->nodes[0].x);
    TEST_ASSERT(!memcmp(&before,&layout->workspace,sizeof(before)));
    layout->nodes[0].x=before.bounds.right+100;
    {float x=layout->nodes[0].x,y=layout->nodes[0].y;gd_layout_step(layout,0);TEST_ASSERT(layout->nodes[0].x==x && layout->nodes[0].y==y);}
    for(i=0;i<600;++i)gd_layout_step(layout,-1);
    TEST_ASSERT(!memcmp(&before,&layout->workspace,sizeof(before)) && isfinite(layout->nodes[0].x) && layout->nodes[0].x<=before.bounds.right-layout->nodes[0].footprint.right);
    layout->nodes[0].x=before.bounds.right-layout->nodes[0].footprint.right-0.01f;layout->nodes[0].vx=5;gd_layout_step(layout,-1);TEST_ASSERT(layout->nodes[0].vx==0);
    TEST_ASSERT(gd_layout_workspace(layout,0,NAN) && !layout->workspace.enabled);
    layout->mode=GD_LAYOUT_COLUMNS;TEST_ASSERT(!gd_layout_workspace(layout,1,1) && gd_layout_workspace(layout,0,1));
    memset(layout,0,sizeof(*layout));layout->mode=GD_LAYOUT_FORCE;TEST_ASSERT(gd_layout_workspace(layout,1,1) && !layout->workspace.enabled);
    release(storage,control);release(storage,layout);
)
