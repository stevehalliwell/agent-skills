/* Included after shared test helpers. Independent boolean-matrix oracle;
   fixture writes, if any, stay under native test's fresh build directory. */
static int test_graph_closure_oracle(void) {
    GD_Layout *layout=allocate(storage,sizeof(*layout)); GD_GraphAnalysis *analysis=allocate(storage,sizeof(*analysis));
    unsigned state=90123; int trial,mode,direction,a,b,i,j,k;
    TEST_ASSERT(layout && analysis); memset(analysis,0,sizeof(*analysis));
    for (trial=0;trial<64;++trial) {
        unsigned char shown[6],eligible[6],boundary[6];
        memset(layout,0,sizeof(*layout)); layout->node_count=6; layout->edge_count=24;
        for (i=0;i<6;++i) {
            state=state*1664525u+1013904223u;
            shown[i]=(unsigned char)(state%7!=0); eligible[i]=(unsigned char)(state%11!=0); boundary[i]=(unsigned char)(state%5==0);
            layout->nodes[i].key=i;
        }
        for (i=0;i<24;++i) {
            state=state*1664525u+1013904223u; a=(int)(state%6);
            state=state*1664525u+1013904223u; b=(int)(state%6);
            layout->edges[i]=(GD_LayoutEdge){a,b,i,state%4!=0,(int)(state%2)};
        }
        for (mode=0;mode<2;++mode) for (direction=0;direction<3;++direction) {
            unsigned char direct[6][6]={{0}},reach[6][6],unknown[6]={0}; int users[6]={0};
            const GD_GraphClosure *closure; int saved_work;
            gd_graph_analyze(layout,shown,mode,GD_GRAPH_ANY,analysis);
            for (i=0;i<24;++i) {
                const GD_LayoutEdge *edge=&layout->edges[i];
                if (edge->relation!=1 || (!edge->active && !mode) || !shown[edge->a] || !shown[edge->b] || !eligible[edge->a] || !eligible[edge->b]) continue;
                if (direction!=1) direct[edge->a][edge->b]=1;
                if (direction!=0) direct[edge->b][edge->a]=1;
            }
            memcpy(reach,direct,sizeof(reach));
            for (k=0;k<6;++k) for (i=0;i<6;++i) for (j=0;j<6;++j) reach[i][j]|=reach[i][k] && reach[k][j];
            closure=gd_graph_closure(analysis,eligible,boundary,1,direction==1,direction==2,GD_GRAPH_ANALYSIS_WORK);
            TEST_ASSERT(closure->complete);
            saved_work=analysis->work;
            for (a=0;a<6;++a) {
                int count=0,degree=0;
                unknown[a]=boundary[a];
                for (b=0;b<6;++b) {
                    TEST_ASSERT(gd_graph_closure_contains(closure,a,b)==(a!=b && reach[a][b]));
                    count+=a!=b && reach[a][b]; degree+=direct[a][b]; users[b]+=a!=b && reach[a][b];
                    if (reach[a][b] && boundary[b]) unknown[a]=1;
                }
                TEST_ASSERT(closure->count[a]==count && closure->direct[a]==degree && closure->unknown[a]==unknown[a]);
                TEST_ASSERT(closure->cycle[a]==(direction!=2 && reach[a][a]));
                TEST_ASSERT(closure->finished[a]==(shown[a] && eligible[a]));
                for (b=0;b<6;++b) if (a!=b) {
                    const GD_GraphComparison *comparison=gd_graph_closure_compare(analysis,a,b);
                    if (!shown[a] || !eligible[a] || !shown[b] || !eligible[b]) { TEST_ASSERT(!comparison); continue; }
                    TEST_ASSERT(comparison && comparison->valid);
                    for (i=0;i<6;++i) {
                        int has_a=i!=a && reach[a][i],has_b=i!=b && reach[b][i];
                        TEST_ASSERT(comparison->nodes[i]==(has_a && has_b ? GD_GRAPH_SHARED:has_a ? GD_GRAPH_A_ONLY:has_b ? GD_GRAPH_B_ONLY:0));
                    }
                    for (i=0;i<6;++i) {
                        int side;
                        for (side=0;side<2;++side) {
                            int origin=side ? b:a,arcs[3],length=gd_graph_closure_path(analysis,closure,side,i,arcs,3),node=i,step;
                            TEST_ASSERT((length>=0)==(i==origin || reach[origin][i]));
                            if (length<0) continue;
                            TEST_ASSERT(length<6);
                            for (step=0;step<length && step<3;++step) {
                                const GD_LayoutEdge *edge=&layout->edges[arcs[step]/2]; int reverse=arcs[step]%2;
                                TEST_ASSERT((reverse ? edge->a:edge->b)==node && edge->relation==1 && (edge->active || mode));
                                node=reverse ? edge->b:edge->a;
                                TEST_ASSERT(shown[node] && eligible[node] && (direction==2 || reverse==(direction==1)));
                            }
                            if (length<=3) TEST_ASSERT(node==origin);
                        }
                    }
                    for (i=0;i<24;++i) {
                        const GD_LayoutEdge *edge=&layout->edges[i]; int side,flags=0;
                        if (edge->relation==1 && (edge->active || mode) && shown[edge->a] && shown[edge->b] && eligible[edge->a] && eligible[edge->b])
                            for (side=0;side<2;++side) if (direction==2 || side==(direction==1)) {
                                int owner=side ? edge->b:edge->a,has_a=owner==a || reach[a][owner],has_b=owner==b || reach[b][owner];
                                flags|=has_a && has_b ? GD_GRAPH_SHARED:has_a ? GD_GRAPH_A_ONLY:has_b ? GD_GRAPH_B_ONLY:0;
                            }
                        TEST_ASSERT(comparison->edges[i]==flags);
                    }
                }
            }
            for (i=0;i<6;++i) TEST_ASSERT(closure->users[i]==users[i]);
            TEST_ASSERT(analysis->work==saved_work);
            { int work=closure->work; TEST_ASSERT(gd_graph_closure(analysis,eligible,boundary,1,direction==1,direction==2,GD_GRAPH_ANALYSIS_WORK)==closure && closure->work==work); }
        }
    }
    release(storage,analysis); release(storage,layout); TEST_PASS();
}
static int test_graph_closure_limits(void) {
    GD_Layout *layout=allocate(storage,sizeof(*layout)); GD_GraphAnalysis *analysis=allocate(storage,sizeof(*analysis));
    const GD_GraphClosure *closure; int i,peer=-1;
    TEST_ASSERT(layout && analysis); memset(layout,0,sizeof(*layout)); memset(analysis,0,sizeof(*analysis));
    layout->node_count=GD_MAX_RECORDS; layout->edge_count=GD_MAX_RECORDS-1;
    for (i=0;i<layout->node_count;++i) layout->nodes[i].key=i;
    for (i=0;i<layout->edge_count;++i) layout->edges[i]=(GD_LayoutEdge){i,i+1,i,1,GD_PREREQUISITES};
    gd_graph_analyze(layout,NULL,1,GD_GRAPH_ANY,analysis);
    closure=gd_graph_closure(analysis,NULL,NULL,GD_PREREQUISITES,0,0,0);
    TEST_ASSERT(!closure->complete && !closure->work && !gd_graph_closure_compare(analysis,0,-1));
    closure=gd_graph_closure(analysis,NULL,NULL,GD_PREREQUISITES,0,0,32);
    TEST_ASSERT(!closure->complete && closure->work==32 && !closure->finished[0]);
    closure=gd_graph_closure(analysis,NULL,NULL,GD_PREREQUISITES,0,0,GD_GRAPH_ANALYSIS_WORK);
    TEST_ASSERT(closure->complete && closure->work==GD_GRAPH_ANALYSIS_WORK && closure->count[0]==GD_MAX_RECORDS-1 && closure->users[GD_MAX_RECORDS-1]==GD_MAX_RECORDS-1);
    TEST_ASSERT(gd_graph_closure_choose(analysis,0,&peer) && peer==1 && closure->peer_count==GD_MAX_RECORDS-2);
    printf("closure cache: %lu bytes; 4096-chain work: %d; bitset table: %lu bytes\n",(unsigned long)sizeof(*closure),closure->work,(unsigned long)sizeof(closure->rows));
    layout->nodes[0].x=99; TEST_ASSERT(!gd_graph_analyze(layout,NULL,1,GD_GRAPH_ANY,analysis) && closure->valid);
    layout->edges[0].relation=GD_NEXT; TEST_ASSERT(gd_graph_analyze(layout,NULL,1,GD_GRAPH_ANY,analysis) && !closure->valid);
    closure=gd_graph_closure(analysis,NULL,NULL,GD_PREREQUISITES,0,0,GD_GRAPH_ANALYSIS_WORK);
    TEST_ASSERT(!closure->count[0] && closure->count[1]==GD_MAX_RECORDS-2);
    layout->node_count=3; layout->edge_count=2;
    layout->edges[0]=(GD_LayoutEdge){0,2,0,1,GD_PREREQUISITES}; layout->edges[1]=(GD_LayoutEdge){1,2,1,1,GD_PREREQUISITES};
    gd_graph_analyze(layout,NULL,1,GD_GRAPH_ANY,analysis);
    closure=gd_graph_closure(analysis,NULL,NULL,GD_PREREQUISITES,0,0,6);
    TEST_ASSERT(!closure->complete && closure->finished[0] && closure->finished[1] && !closure->finished[2]);
    TEST_ASSERT(gd_graph_closure_compare(analysis,0,1)->shared==1);
    TEST_ASSERT(!gd_graph_closure_compare(analysis,0,2) && !gd_graph_closure_compare(analysis,-1,1) && !gd_graph_closure_compare(analysis,0,0) && !gd_graph_closure_compare(analysis,0,-2));
    { unsigned char boundary[3]={0,0,1};
      closure=gd_graph_closure(analysis,NULL,boundary,GD_PREREQUISITES,0,0,GD_GRAPH_ANALYSIS_WORK);
      TEST_ASSERT(closure->complete && closure->unknown[0] && closure->unknown[1]); }
    layout->edge_count=GD_MAX_PAIRS;
    for (i=0;i<GD_MAX_PAIRS;++i) layout->edges[i]=(GD_LayoutEdge){0,1,i,1,GD_PREREQUISITES};
    gd_graph_analyze(layout,NULL,1,GD_GRAPH_ANY,analysis);
    closure=gd_graph_closure(analysis,NULL,NULL,GD_PREREQUISITES,0,0,GD_GRAPH_ANALYSIS_WORK);
    TEST_ASSERT(closure->complete && closure->direct[0]==1 && closure->count[0]==1 && closure->users[1]==1);
    layout->node_count=layout->edge_count=0; gd_graph_analyze(layout,NULL,1,GD_GRAPH_ANY,analysis);
    TEST_ASSERT(gd_graph_closure(analysis,NULL,NULL,GD_PREREQUISITES,0,0,0)->complete && !gd_graph_closure_choose(analysis,0,&peer));
    release(storage,analysis); release(storage,layout); TEST_PASS();
}
static int test_graph_foundations_adapter(void) {
    GD_Model *model=model_create(); GD_Layout *layout=allocate(storage,sizeof(*layout)); GD_GraphAnalysis *analysis=allocate(storage,sizeof(*analysis));
    const GD_GraphClosure *closure; const GD_GraphComparison *comparison; uint64_t saved[6][GD_CLOSURE_WORDS];
    int i,peer=-1; unsigned char shown[6]={1,1,1,1,1,1};
    TEST_ASSERT(model && layout && analysis); memset(layout,0,sizeof(*layout)); memset(analysis,0,sizeof(*analysis));
    model->record_count=7; model->edge_count=8; model->source_digest=123;
    for (i=0;i<7;++i) model->records[i].role=i==6 ? GD_NOUN:GD_SKILL;
    model->edges[0]=(GD_Edge){3,0,GD_PREREQUISITES,"atoms/stomp-walker",1};
    model->edges[1]=(GD_Edge){3,1,GD_PREREQUISITES,"atoms/land-on-target",2};
    model->edges[2]=(GD_Edge){4,0,GD_PREREQUISITES,"atoms/stomp-walker",3};
    model->edges[3]=(GD_Edge){4,2,GD_PREREQUISITES,"atoms/control-fall",4};
    model->edges[4]=(GD_Edge){2,5,GD_PREREQUISITES,"atoms/stop-at-edge",5};
    model->edges[5]=model->edges[1];
    model->edges[6]=(GD_Edge){5,-1,GD_PREREQUISITES,"atoms/unloaded",6};
    model->edges[7]=(GD_Edge){5,6,GD_PREREQUISITES,"nouns/not-a-skill",7};
    layout->node_count=6; layout->edge_count=5;
    for (i=0;i<6;++i) layout->nodes[i].key=i;
    for (i=0;i<5;++i) layout->edges[i]=(GD_LayoutEdge){model->edges[i].target,model->edges[i].source,i,1,GD_PREREQUISITES};
    gd_graph_analyze(layout,NULL,1,GD_GRAPH_ANY,analysis); closure=gd_graph_foundations(model,analysis,0);
    TEST_ASSERT(closure && closure->complete && closure->users[0]==2 && closure->direct[3]==2 && closure->direct[4]==2);
    TEST_ASSERT(!closure->unknown[3] && closure->unknown[4] && closure->unknown[2] && closure->unknown[5]);
    comparison=gd_graph_closure_compare(analysis,3,4);
    TEST_ASSERT(comparison->shared==1 && comparison->nodes[0]==GD_GRAPH_SHARED && comparison->nodes[1]==GD_GRAPH_A_ONLY && comparison->nodes[2]==GD_GRAPH_B_ONLY && comparison->nodes[5]==GD_GRAPH_B_ONLY);
    TEST_ASSERT(gd_graph_closure_choose(analysis,3,&peer)->b==4 && peer==4);
    memcpy(saved,closure->rows,sizeof(saved));
    for (i=0;i<5;++i) { int swap=layout->edges[i].a; layout->edges[i].a=layout->edges[i].b; layout->edges[i].b=swap; }
    gd_graph_analyze(layout,NULL,1,GD_GRAPH_ANY,analysis); closure=gd_graph_foundations(model,analysis,1);
    TEST_ASSERT(closure && !memcmp(saved,closure->rows,sizeof(saved)));
    shown[5]=0; gd_graph_analyze(layout,shown,1,GD_GRAPH_ANY,analysis); closure=gd_graph_foundations(model,analysis,1);
    TEST_ASSERT(closure->unknown[2] && closure->unknown[4] && !gd_graph_closure_contains(closure,4,5));
    shown[5]=1; layout->edge_count=4; gd_graph_analyze(layout,shown,1,GD_GRAPH_ANY,analysis); closure=gd_graph_foundations(model,analysis,1);
    TEST_ASSERT(closure->unknown[2] && closure->unknown[4] && !gd_graph_closure_contains(closure,4,5));
    layout->node_count=1; layout->edge_count=0; layout->nodes[0].key=6;
    gd_graph_analyze(layout,NULL,1,GD_GRAPH_ANY,analysis); TEST_ASSERT(!gd_graph_foundations(model,analysis,1));
    TEST_ASSERT(model->source_digest==123 && model->edge_count==8 && model->edges[6].target==-1 && model->edges[5].line==2);
    release(storage,analysis); release(storage,layout); gd_destroy(model); TEST_PASS();
}
