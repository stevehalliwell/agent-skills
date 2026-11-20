TEST(graph_shapes_oracle,
    GD_Layout *layout=allocate(storage,sizeof(*layout)); GD_GraphAnalysis *analysis=allocate(storage,sizeof(*analysis));
    unsigned state=11229; int trial,mode,i,j,k,r,round;
    TEST_ASSERT(layout && analysis); memset(analysis,0,sizeof(*analysis));
    for (trial=0;trial<64;++trial) {
        unsigned char shown[5]; int roles[5];
        memset(layout,0,sizeof(*layout)); layout->node_count=5; layout->edge_count=18;
        for (i=0;i<5;++i) { state=state*1664525u+1013904223u; shown[i]=(unsigned char)(state%7!=0); roles[i]=(int)(state%3); layout->nodes[i].key=i; }
        for (i=0;i<18;++i) {
            state=state*1664525u+1013904223u; j=(int)(state%5);
            state=state*1664525u+1013904223u; k=(int)(state%5);
            layout->edges[i]=(GD_LayoutEdge){j,k,i,state%4!=0,(int)(state%3)};
            if (i && i%5==0) layout->edges[i]=layout->edges[i-1];
        }
        for (mode=0;mode<2;++mode) {
            unsigned char adjacent[5][5][3]={{{0}}}; int colors[5],tokens[5][30],counts[5],next[5],members[5]={0};
            const GD_GraphShapes *shapes; int work;
            gd_graph_analyze(layout,shown,mode,GD_GRAPH_ANY,analysis);
            for (i=0;i<18;++i) {
                const GD_LayoutEdge *edge=&layout->edges[i];
                if (shown[edge->a] && shown[edge->b] && (edge->active || mode)) adjacent[edge->a][edge->b][edge->relation]=1;
            }
            for (i=0;i<5;++i) colors[i]=roles[i];
            /* Independent pairwise interning, boolean canonical adjacency and
               insertion-sorted integer tokens, not kernel feature/comparator. */
            for (round=0;round<2;++round) {
                int families=0;
                memset(counts,0,sizeof(counts));
                for (i=0;i<5;++i) if (shown[i]) {
                    for (j=0;j<5;++j) for (r=0;r<3;++r) {
                        if (adjacent[i][j][r]) tokens[i][counts[i]++]=(2*r)*8+colors[j];
                        if (adjacent[j][i][r]) tokens[i][counts[i]++]=(2*r+1)*8+colors[j];
                    }
                    for (j=1;j<counts[i];++j) { int value=tokens[i][j],at=j; while (at && tokens[i][at-1]>value) { tokens[i][at]=tokens[i][at-1]; --at; } tokens[i][at]=value; }
                }
                for (i=0;i<5;++i) {
                    next[i]=-1; if (!shown[i]) continue;
                    for (j=0;j<i;++j) if (shown[j] && colors[i]==colors[j] && counts[i]==counts[j] && !memcmp(tokens[i],tokens[j],(size_t)counts[i]*sizeof(int))) break;
                    next[i]=j<i ? next[j]:families++;
                }
                memcpy(colors,next,sizeof(colors));
            }
            work=analysis->work; shapes=gd_graph_shapes(analysis,roles,GD_GRAPH_ANALYSIS_WORK);
            TEST_ASSERT(shapes->complete && analysis->work==work);
            for (i=0;i<5;++i) {
                if (!shown[i]) { TEST_ASSERT(shapes->colors[i]==-1); continue; }
                for (j=0;j<5;++j) if (shown[j]) { TEST_ASSERT((shapes->colors[i]==shapes->colors[j])==(colors[i]==colors[j])); members[i]+=colors[i]==colors[j]; }
                TEST_ASSERT(shapes->size[shapes->colors[i]]==members[i]);
            }
            work=shapes->work; layout->nodes[0].x+=1;
            TEST_ASSERT(!gd_graph_analyze(layout,shown,mode,GD_GRAPH_ANY,analysis) && gd_graph_shapes(analysis,roles,GD_GRAPH_ANALYSIS_WORK)->work==work);
        }
    }
    release(storage,analysis); release(storage,layout);
)
TEST(graph_shapes_limits,
    GD_Layout *layout=allocate(storage,sizeof(*layout)); GD_GraphAnalysis *analysis=allocate(storage,sizeof(*analysis));
    const GD_GraphShapes *shapes; int roles[GD_MAX_RECORDS]={0},i;
    TEST_ASSERT(layout && analysis); memset(layout,0,sizeof(*layout)); memset(analysis,0,sizeof(*analysis));
    layout->node_count=GD_MAX_RECORDS;
    for (i=0;i<GD_MAX_RECORDS;++i) layout->nodes[i].key=i;
    gd_graph_analyze(layout,NULL,1,GD_GRAPH_ANY,analysis);
    TEST_ASSERT(!gd_graph_shapes(analysis,roles,0)->complete && !gd_graph_shapes(analysis,roles,1)->complete && !gd_graph_shapes(analysis,roles,32)->complete);
    shapes=gd_graph_shapes(analysis,roles,GD_GRAPH_ANALYSIS_WORK);
    TEST_ASSERT(shapes->complete && shapes->family_count==1 && shapes->size[0]==GD_MAX_RECORDS);
    roles[0]=1; shapes=gd_graph_shapes(analysis,roles,GD_GRAPH_ANALYSIS_WORK);
    TEST_ASSERT(shapes->family_count==2 && shapes->size[shapes->colors[0]]==1 && shapes->size[shapes->colors[1]]==GD_MAX_RECORDS-1);
    layout->node_count=4; layout->edge_count=GD_MAX_PAIRS;
    for (i=0;i<GD_MAX_PAIRS;++i) layout->edges[i]=(GD_LayoutEdge){i%2 ? 0:2,i%2 ? 1:3,i,1,GD_NEXT};
    roles[0]=0; gd_graph_analyze(layout,NULL,1,GD_GRAPH_ANY,analysis); shapes=gd_graph_shapes(analysis,roles,GD_GRAPH_ANALYSIS_WORK);
    TEST_ASSERT(shapes->complete && shapes->colors[0]==shapes->colors[2] && shapes->colors[1]==shapes->colors[3] && shapes->colors[0]!=shapes->colors[1]);
    { int before[4],j;
      for (i=0;i<4;++i) before[i]=shapes->colors[i];
      for (i=0;i<layout->edge_count;++i) { layout->edges[i].a=3-layout->edges[i].a; layout->edges[i].b=3-layout->edges[i].b; }
      gd_graph_analyze(layout,NULL,1,GD_GRAPH_ANY,analysis); shapes=gd_graph_shapes(analysis,roles,GD_GRAPH_ANALYSIS_WORK);
      for (i=0;i<4;++i) for (j=0;j<4;++j) TEST_ASSERT((shapes->colors[i]==shapes->colors[j])==(before[3-i]==before[3-j])); }
    for (i=0;i<GD_MAX_PAIRS;++i) layout->edges[i]=(GD_LayoutEdge){0,1,i,1,i};
    gd_graph_analyze(layout,NULL,1,GD_GRAPH_ANY,analysis); shapes=gd_graph_shapes(analysis,roles,GD_GRAPH_ANALYSIS_WORK);
    TEST_ASSERT(shapes->complete && analysis->feature_count==2*GD_MAX_PAIRS && shapes->begin[4]==2*GD_MAX_PAIRS && shapes->family_count==3);
    layout->edge_count=3; layout->edges[0]=(GD_LayoutEdge){0,1,0,1,GD_NEXT}; layout->edges[1]=(GD_LayoutEdge){2,3,1,1,GD_NEXT}; layout->edges[2]=(GD_LayoutEdge){0,3,2,1,GD_NEXT};
    gd_graph_analyze(layout,NULL,1,GD_GRAPH_ANY,analysis); shapes=gd_graph_shapes(analysis,roles,GD_GRAPH_ANALYSIS_WORK);
    TEST_ASSERT(shapes->colors[0]!=shapes->colors[2]); /* Distinct equal-colored neighbors retain multiplicity. */
    layout->edges[2].a=3; layout->edges[2].b=0; gd_graph_analyze(layout,NULL,1,GD_GRAPH_ANY,analysis); shapes=gd_graph_shapes(analysis,roles,GD_GRAPH_ANALYSIS_WORK); TEST_ASSERT(shapes->complete);
    layout->edges[2].relation=GD_PREREQUISITES; TEST_ASSERT(gd_graph_analyze(layout,NULL,1,GD_GRAPH_ANY,analysis) && !shapes->valid);
    shapes=gd_graph_shapes(analysis,roles,GD_GRAPH_ANALYSIS_WORK); TEST_ASSERT(shapes->complete);
    layout->node_count=GD_MAX_RECORDS; layout->edge_count=GD_MAX_RECORDS-1;
    for (i=0;i<layout->edge_count;++i) layout->edges[i]=(GD_LayoutEdge){0,i+1,i,1,GD_NEXT};
    gd_graph_analyze(layout,NULL,1,GD_GRAPH_ANY,analysis); shapes=gd_graph_shapes(analysis,roles,GD_GRAPH_ANALYSIS_WORK);
    TEST_ASSERT(!analysis->complete && shapes->complete && shapes->family_count==2 && shapes->size[shapes->colors[1]]==GD_MAX_RECORDS-1);
    layout->node_count=layout->edge_count=0; gd_graph_analyze(layout,NULL,1,GD_GRAPH_ANY,analysis);
    TEST_ASSERT(gd_graph_shapes(analysis,NULL,0)->complete && !analysis->shapes.family_count);
    release(storage,analysis); release(storage,layout);
)
