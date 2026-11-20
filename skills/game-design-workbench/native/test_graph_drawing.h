/* Drawing checks exercise geometry directly, without pointer automation. */
TEST(layout_settling,
    GD_Layout *layout=allocate(storage,sizeof(*layout)),*other=allocate(storage,sizeof(*other));
    double square=0; float maximum=0; int i,step;
    TEST_ASSERT(layout && other); memset(layout,0,sizeof(*layout));
    layout->node_count=64; layout->group_count=1; layout->edge_count=64;
    for (i=0;i<64;++i) {
        layout->nodes[i].label="node"; layout->nodes[i].key=i;
        layout->edges[i]=(GD_LayoutEdge){i,(i+1)%64,i,1,GD_NEXT};
    }
    TEST_ASSERT(gd_layout_arrange(layout,GD_LAYOUT_FORCE,GD_SORT_NAME));
    for (step=0;step<4096;++step) {
        gd_layout_step(layout,-1);
        if (step>=4096-120) for (i=0;i<64;++i) {
            float speed=sqrtf(layout->nodes[i].vx*layout->nodes[i].vx+layout->nodes[i].vy*layout->nodes[i].vy);
            square+=speed*speed; if (speed>maximum) maximum=speed;
        }
    }
    fprintf(stderr,"settling probe: 64-node cycle / 4096 steps / last 120: RMS %.6f, max %.6f world units/step\n",sqrt(square/(64*120)),maximum);
    /* Bound residual movement, not a promise of full convergence. Original
       rotating sampler measured RMS 0.342987 / maximum 0.982166 here. */
    TEST_ASSERT(sqrt(square/(64*120))<0.04 && maximum<0.1f);
    memcpy(other,layout,sizeof(*layout)); other->tick+=17;
    gd_layout_step(layout,-1); gd_layout_step(other,-1);
    for (i=0;i<64;++i) TEST_ASSERT(layout->nodes[i].x==other->nodes[i].x && layout->nodes[i].y==other->nodes[i].y);
    release(storage,other); release(storage,layout);
)
TEST(directed_drawing_order,
    static const char *labels[]={"a","b","c","d","e","f"};
    GD_Layout *layout=allocate(storage,sizeof(*layout)); GD_LayoutPoint saved[6];
    GD_LayoutEdge edges[3]; int horizontal,i,j,step;
    TEST_ASSERT(layout); memset(layout,0,sizeof(*layout));
    layout->node_count=6; layout->group_count=1; layout->edge_count=3;
    for (i=0;i<6;++i) { layout->nodes[i].label=labels[i]; layout->nodes[i].key=i; layout->nodes[i].layer=i/3; layout->nodes[i].width=i==1 ? 1000:220; layout->nodes[i].height=120; }
    for (i=0;i<3;++i) layout->edges[i]=(GD_LayoutEdge){i,5-i,i,1,GD_PREREQUISITES};
    memcpy(edges,layout->edges,sizeof(edges));
    for (horizontal=0;horizontal<2;++horizontal) {
        layout->horizontal=horizontal;
        TEST_ASSERT(gd_layout_arrange(layout,GD_LAYOUT_DIRECTED,GD_SORT_NAME));
        for (i=0;i<3;++i) for (j=i+1;j<3;++j) {
            float a=horizontal ? layout->nodes[i].y:layout->nodes[i].x,b=horizontal ? layout->nodes[j].y:layout->nodes[j].x;
            float c=horizontal ? layout->nodes[5-i].y:layout->nodes[5-i].x,d=horizontal ? layout->nodes[5-j].y:layout->nodes[5-j].x;
            TEST_ASSERT((a-b)*(c-d)>0); /* Three initial inversions removed. */
        }
        for (step=0;step<120;++step) {
            for (i=0;i<6;++i) for (j=i+1;j<6;++j) if (layout->nodes[i].layer==layout->nodes[j].layer) {
                float span=horizontal ? 120:(layout->nodes[i].width+layout->nodes[j].width)/2;
                float delta=horizontal ? layout->nodes[i].y-layout->nodes[j].y:layout->nodes[i].x-layout->nodes[j].x;
                TEST_ASSERT(fabsf(delta)>span+40);
            }
            gd_layout_step(layout,-1);
        }
        TEST_ASSERT(gd_layout_arrange(layout,GD_LAYOUT_DIRECTED,GD_SORT_NAME));
        for (i=0;i<6;++i) saved[i]=(GD_LayoutPoint){layout->nodes[i].x,layout->nodes[i].y};
        TEST_ASSERT(gd_layout_arrange(layout,GD_LAYOUT_DIRECTED,GD_SORT_NAME));
        for (i=0;i<6;++i) TEST_ASSERT(layout->nodes[i].x==saved[i].x && layout->nodes[i].y==saved[i].y && layout->nodes[i].key==i && layout->nodes[i].layer==i/3);
        TEST_ASSERT(!memcmp(edges,layout->edges,sizeof(edges)));
    }
    layout->nodes[0].width=-1; TEST_ASSERT(!gd_layout_arrange(layout,GD_LAYOUT_DIRECTED,GD_SORT_NAME));
    layout->nodes[0].width=NAN; TEST_ASSERT(!gd_layout_arrange(layout,GD_LAYOUT_DIRECTED,GD_SORT_NAME));
    release(storage,layout);
)
TEST(directed_drawing_routes,
    static const char *labels[]={"long source","other root","middle","sibling","target","unplaced"};
    static const int from[]={0,0,4,1,0,0,2,2},to[]={2,4,0,3,2,0,3,0};
    GD_Layout *layout=allocate(storage,sizeof(*layout)); GD_LayoutEdge saved[8]; int i,j,k,horizontal;
    TEST_ASSERT(layout); memset(layout,0,sizeof(*layout)); layout->node_count=6; layout->edge_count=8; layout->group_count=1;
    for (i=0;i<6;++i) { layout->nodes[i].label=labels[i]; layout->nodes[i].key=i; layout->nodes[i].layer=i==5 ? -1:i/2; layout->nodes[i].width=i==1 ? 1000:220; layout->nodes[i].height=120; }
    for (i=0;i<8;++i) layout->edges[i]=(GD_LayoutEdge){from[i],to[i],100+i,1,i%2 ? GD_PREREQUISITES:GD_NEXT};
    memcpy(saved,layout->edges,sizeof(saved));
    for (horizontal=0;horizontal<2;++horizontal) {
        layout->horizontal=horizontal; TEST_ASSERT(gd_layout_arrange(layout,GD_LAYOUT_DIRECTED,GD_SORT_NAME));
        TEST_ASSERT(layout->nodes[5].axis>layout->nodes[4].axis && layout->nodes[5].layer==-1);
        for (i=0;i<8;++i) {
            GD_LayoutRoute route; gd_layout_route(layout,i,&route);
            TEST_ASSERT(route.count>=3 && route.count<=8);
            for (j=1;j<route.count;++j) {
                GD_LayoutPoint a=route.points[j-1],b=route.points[j]; float x=(a.x+b.x)/2,y=(a.y+b.y)/2;
                TEST_ASSERT(isfinite(x) && isfinite(y) && gd_layout_route_distance(layout,i,x,y)<0.001f);
                for (k=0;k<6;++k) if (k!=from[i] && k!=to[i]) {
                    const GD_LayoutNode *node=&layout->nodes[k];
                    float left=node->x-(node->width-44)/2,right=node->x+(node->width-44)/2,top=node->y+22,bottom=node->y+40;
                    /* All segments are orthogonal in this directed fixture.
                       Test complete segment against reserved primary-label box. */
                    TEST_ASSERT(!(fminf(a.x,b.x)<right && fmaxf(a.x,b.x)>left && fminf(a.y,b.y)<bottom && fmaxf(a.y,b.y)>top));
                    TEST_ASSERT(gd_layout_route_distance(layout,i,node->x,node->y)>20);
                }
            }
        }
        { GD_LayoutRoute long_link,parallel,reverse,self; const GD_LayoutNode *a=&layout->nodes[0],*b=&layout->nodes[4];
          gd_layout_route(layout,1,&long_link); gd_layout_route(layout,4,&parallel); gd_layout_route(layout,7,&reverse); gd_layout_route(layout,5,&self);
          TEST_ASSERT(long_link.count==8 && parallel.count==8 && reverse.count==8 && self.count==5);
          TEST_ASSERT(gd_layout_route_distance(layout,1,(a->x+b->x)/2,(a->y+b->y)/2)>20);
          TEST_ASSERT(!memcmp(saved,layout->edges,sizeof(saved)));
          TEST_ASSERT(layout->lanes[layout->edge_lane[0]].count==3 && layout->lanes[layout->edge_lane[4]].ordinal==1 && layout->lanes[layout->edge_lane[7]].ordinal==2); }
    }
    TEST_ASSERT(gd_layout_arrange(layout,GD_LAYOUT_FORCE,GD_SORT_NAME));
    for (i=0;i<8;++i) { GD_LayoutRoute route; gd_layout_route(layout,i,&route); TEST_ASSERT(route.count==(from[i]==to[i] ? 5:3)); }
    layout->direct_routes=1;
    for (horizontal=0;horizontal<2;++horizontal) {
        layout->horizontal=horizontal; TEST_ASSERT(gd_layout_arrange(layout,GD_LAYOUT_DIRECTED,GD_SORT_NAME));
        for (i=0;i<8;++i) {
            GD_LayoutRoute route; gd_layout_route(layout,i,&route);
            TEST_ASSERT(route.count==(from[i]==to[i] ? 8:
                (horizontal ? layout->nodes[from[i]].x==layout->nodes[to[i]].x:layout->nodes[from[i]].y==layout->nodes[to[i]].y) ? 8:3));
            for (j=1;j<route.count;++j) {
                float x=(route.points[j-1].x+route.points[j].x)/2,y=(route.points[j-1].y+route.points[j].y)/2;
                TEST_ASSERT(isfinite(x) && isfinite(y) && gd_layout_route_distance(layout,i,x,y)<0.001f);
            }
        }
        TEST_ASSERT(!memcmp(saved,layout->edges,sizeof(saved)));
    }
    /* Long direct link bypasses an unrelated middle node in a populated band;
       clearance must move the node, not reintroduce an orthogonal corridor. */
    memset(layout,0,sizeof(*layout)); layout->direct_routes=1;
    layout->node_count=5; layout->edge_count=1; layout->group_count=1;
    for (i=0;i<5;++i) {
        layout->nodes[i].key=i; layout->nodes[i].label=labels[i];
        layout->nodes[i].layer=i==0 ? 0:i==4 ? 2:1;
    }
    layout->edges[0]=(GD_LayoutEdge){0,4,0,1,GD_PREREQUISITES};
    for (horizontal=0;horizontal<2;++horizontal) {
        layout->horizontal=horizontal; TEST_ASSERT(gd_layout_arrange(layout,GD_LAYOUT_DIRECTED,GD_SORT_NAME));
        for (i=1;i<4;++i) TEST_ASSERT(gd_layout_route_distance(layout,0,layout->nodes[i].x,layout->nodes[i].y)>20);
        TEST_ASSERT(layout->nodes[1].layer==1 && layout->nodes[2].layer==1 && layout->nodes[3].layer==1);
    }
    layout->node_count=layout->edge_count=layout->group_count=0; TEST_ASSERT(gd_layout_arrange(layout,GD_LAYOUT_DIRECTED,GD_SORT_NAME));
    release(storage,layout);
)
