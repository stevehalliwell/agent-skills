TEST(graph_overview_oracle,
    GD_Layout *layout=allocate(storage,sizeof(*layout)); GD_GraphAnalysis *analysis=allocate(storage,sizeof(*analysis));
    unsigned state=5137; int trial,mode,undirected,i,row,column;
    TEST_ASSERT(layout && analysis); memset(analysis,0,sizeof(*analysis));
    for (trial=0;trial<16;++trial) {
        unsigned char shown[70]; int ordinal[70],node_count=0;
        memset(layout,0,sizeof(*layout)); layout->node_count=70; layout->edge_count=210;
        for (i=0;i<70;++i) {
            state=state*1664525u+1013904223u; shown[i]=(unsigned char)(state%7!=0); ordinal[i]=shown[i] ? node_count++:-1; layout->nodes[i].key=i;
        }
        for (i=0;i<210;++i) {
            int a,b; state=state*1664525u+1013904223u; a=(int)(state%70);
            state=state*1664525u+1013904223u; b=(int)(state%70);
            layout->edges[i]=(GD_LayoutEdge){a,b,100+i,state%3!=0,(int)(state%GD_FIELD_COUNT)};
        }
        for (mode=0;mode<2;++mode) for (undirected=0;undirected<2;++undirected) {
            GD_OverviewControls controls={1,-1,-1}; const GD_GraphOverview *overview; int total=0,work;
            gd_graph_analyze(layout,shown,mode,GD_GRAPH_ANY,analysis); work=analysis->work;
            overview=gd_graph_overview(analysis,&controls,undirected);
            TEST_ASSERT(overview && overview->bins==GD_OVERVIEW_BINS && overview->node_count==node_count);
            for (i=0;i<210;++i) total+=shown[layout->edges[i].a] && shown[layout->edges[i].b] && (layout->edges[i].active || mode);
            TEST_ASSERT(overview->link_count==total);
            for (row=0;row<overview->bins;++row) {
                for (i=overview->start[row];i<overview->start[row+1];++i)
                    TEST_ASSERT(ordinal[overview->nodes[i]]==i && overview->node_bin[overview->nodes[i]]==row);
                for (column=0;column<overview->bins;++column) {
                    int count=0,first=-1; unsigned char nodes[70]={0};
                    gd_graph_overview_select(analysis,row,column);
                    for (i=0;i<210;++i) {
                        const GD_LayoutEdge *edge=&layout->edges[i]; int qualifies=0;
                        if (shown[edge->a] && shown[edge->b] && (edge->active || mode)) {
                            int a=ordinal[edge->a]*overview->bins/node_count,b=ordinal[edge->b]*overview->bins/node_count;
                            qualifies=(a==row && b==column) || (undirected && a==column && b==row);
                        }
                        TEST_ASSERT(overview->selected_edges[i]==qualifies);
                        if (qualifies) { if (!count) first=i; ++count; nodes[edge->a]=nodes[edge->b]=1; }
                    }
                    TEST_ASSERT(overview->counts[row*overview->bins+column]==count && overview->first[row*overview->bins+column]==first && overview->selected_count==count);
                    for (i=0;i<70;++i) TEST_ASSERT(overview->selected_nodes[i]==nodes[i]);
                }
            }
            TEST_ASSERT(analysis->work==work);
            layout->nodes[0].x+=10; TEST_ASSERT(!gd_graph_analyze(layout,shown,mode,GD_GRAPH_ANY,analysis));
            TEST_ASSERT(gd_graph_overview(analysis,&controls,undirected)==overview && overview->link_count==total);
        }
    }
    release(storage,analysis); release(storage,layout);
)
TEST(graph_overview_limits,
    GD_Layout *layout=allocate(storage,sizeof(*layout)); GD_GraphAnalysis *analysis=allocate(storage,sizeof(*analysis));
    GD_OverviewControls controls={0,-1,-1}; const GD_GraphOverview *overview; int i,total=0;
    TEST_ASSERT(layout && analysis); memset(layout,0,sizeof(*layout)); memset(analysis,0,sizeof(*analysis));
    layout->node_count=3; layout->edge_count=3;
    for (i=0;i<3;++i) layout->nodes[i].key=i;
    layout->edges[0]=(GD_LayoutEdge){0,1,17,1,GD_NEXT}; layout->edges[1]=(GD_LayoutEdge){0,1,18,1,GD_PREREQUISITES}; layout->edges[2]=(GD_LayoutEdge){0,0,19,1,GD_NEXT};
    gd_graph_analyze(layout,NULL,1,GD_GRAPH_ANY,analysis); TEST_ASSERT(!gd_graph_overview(analysis,&controls,0));
    controls.enabled=1; overview=gd_graph_overview(analysis,&controls,0);
    TEST_ASSERT(overview->bins==3 && overview->counts[1]==2 && !overview->counts[3] && overview->counts[0]==1 && overview->link_count==3);
    controls.row=0; controls.column=1; overview=gd_graph_overview(analysis,&controls,0);
    TEST_ASSERT(overview->selected_count==2 && overview->selected_edges[0] && overview->selected_edges[1] && !overview->selected_edges[2] && !overview->selected_nodes[2]);
    overview=gd_graph_overview(analysis,&controls,1); TEST_ASSERT(controls.row==-1 && controls.column==-1 && overview->counts[1]==2 && overview->counts[3]==2 && overview->counts[0]==1);
    for (i=0;i<9;++i) total+=overview->counts[i]; TEST_ASSERT(total==5 && overview->link_count==3);
    gd_graph_overview_select(analysis,-1,0); TEST_ASSERT(!overview->selected_count);
    gd_graph_overview_select(analysis,3,0); TEST_ASSERT(!overview->selected_count);
    layout->edges[0].relation=GD_MECHANICS; TEST_ASSERT(gd_graph_analyze(layout,NULL,1,GD_GRAPH_ANY,analysis) && !overview->valid);
    overview=gd_graph_overview(analysis,&controls,0); TEST_ASSERT(overview->link_count==3);
    layout->node_count=GD_MAX_RECORDS; layout->edge_count=GD_MAX_PAIRS;
    for (i=0;i<layout->node_count;++i) layout->nodes[i].key=i;
    for (i=0;i<layout->edge_count;++i) layout->edges[i]=(GD_LayoutEdge){0,1+i%(GD_MAX_RECORDS-1),i,1,i%7};
    gd_graph_analyze(layout,NULL,1,GD_GRAPH_ANY,analysis); overview=gd_graph_overview(analysis,&controls,0);
    TEST_ASSERT(!analysis->complete && overview->bins==32 && overview->node_count==GD_MAX_RECORDS && overview->link_count==GD_MAX_PAIRS);
    gd_graph_overview_select(analysis,0,0); TEST_ASSERT(overview->selected_count>0);
    for (i=0;i<layout->edge_count;++i) if (overview->selected_edges[i]) TEST_ASSERT(layout->edges[i].a!=layout->edges[i].b); /* Diagonal bin is not self-loop proof. */
    layout->node_count=layout->edge_count=0; gd_graph_analyze(layout,NULL,1,GD_GRAPH_ANY,analysis); overview=gd_graph_overview(analysis,&controls,0);
    TEST_ASSERT(overview && !overview->bins && !overview->link_count && !overview->selected_count && overview->start[0]==0);
    printf("graph view scratch: %lu bytes; overview: %lu bytes\n",(unsigned long)sizeof(*analysis),(unsigned long)sizeof(*overview));
    release(storage,analysis); release(storage,layout);
)
