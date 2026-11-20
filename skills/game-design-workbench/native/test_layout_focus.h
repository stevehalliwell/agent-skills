#include "gd_layout_focus.h"
TEST(layout_distance_focus,
    GD_Allocator allocator={storage,allocate,release};
    GD_Layout *layout=allocate(storage,sizeof(*layout));
    GD_LayoutFocus *focus=gd_layout_focus_create(allocator);
    GD_LayoutNode before[8]; GD_LayoutEdge edges[32]; GD_LayoutGroup group;
    GD_LayoutPoint after[8]; unsigned char shown[8];
    unsigned seed=191; int sample,i,j,k,horizontal;
    TEST_ASSERT(layout && focus);
    /* Independent Floyd-Warshall oracle, not production BFS/adjacency. */
    for (sample=0;sample<128;++sample) {
        int expected[8][8],roots[8],include=sample%2,directed=(sample/2)%2;
        memset(layout,0,sizeof(*layout)); layout->node_count=8; layout->group_count=1;
        for (i=0;i<8;++i) {
            layout->nodes[i].key=i; layout->nodes[i].label="focus node";
            seed=1664525u*seed+1013904223u; shown[i]=(seed%5)!=0;
            for (j=0;j<8;++j) expected[i][j]=i==j ? 0:999;
        }
        for (i=0;i<8;++i) for (j=i+1;j<8;++j) {
            seed=1664525u*seed+1013904223u;
            if (seed%3==0) {
                int active=(seed&8)!=0;
                layout->edges[layout->edge_count++]=(GD_LayoutEdge){(seed&4) ? i:j,(seed&4) ? j:i,i*8+j,active,GD_PREREQUISITES};
                if (shown[i] && shown[j] && (include || active)) expected[i][j]=expected[j][i]=1;
            }
        }
        for (i=0;i<8;++i) roots[i]=directed && shown[i];
        for (i=0;i<layout->edge_count;++i) {
            const GD_LayoutEdge *edge=&layout->edges[i];
            if (shown[edge->a] && shown[edge->b] && (include || edge->active)) roots[edge->b]=0;
        }
        for (k=0;k<8;++k) for (i=0;i<8;++i) for (j=0;j<8;++j)
            if (expected[i][k]+expected[k][j]<expected[i][j]) expected[i][j]=expected[i][k]+expected[k][j];
        TEST_ASSERT(gd_layout_arrange(layout,GD_LAYOUT_FORCE,GD_SORT_NAME));
        memcpy(before,layout->nodes,sizeof(before)); memcpy(edges,layout->edges,sizeof(edges)); group=layout->groups[0];
        gd_layout_focus_capture(focus,layout);
        TEST_ASSERT(gd_layout_focus_apply(focus,layout,shown,include,directed,0,3)==(shown[0] ? 2:0));
        for (i=0;i<8;++i) {
            TEST_ASSERT(focus->distance[i]==(!shown[i] ? -2:!shown[0] || expected[0][i]==999 ? -1:expected[0][i]));
            TEST_ASSERT(focus->fixed_roots[i]==roots[i]);
            if (roots[i]) TEST_ASSERT(layout->nodes[i].x==before[i].x && layout->nodes[i].y==before[i].y);
            after[i]=(GD_LayoutPoint){layout->nodes[i].x,layout->nodes[i].y};
        }
        TEST_ASSERT(gd_layout_focus_apply(focus,layout,shown,include,directed,0,3)==(shown[0] ? 1:0));
        for (i=0;i<8;++i) TEST_ASSERT(layout->nodes[i].x==after[i].x && layout->nodes[i].y==after[i].y);
        if (shown[0]) {
            layout->nodes[0].x+=1000; gd_layout_focus_reapply(focus,layout,3);
            TEST_ASSERT(layout->nodes[0].x==after[0].x);
        }
        TEST_ASSERT(!memcmp(edges,layout->edges,sizeof(edges)));
        gd_layout_focus_restore(focus,layout);
        TEST_ASSERT(!memcmp(before,layout->nodes,sizeof(before)) && !memcmp(&group,&layout->groups[0],sizeof(group)));
    }
    for (horizontal=0;horizontal<2;++horizontal) {
        memset(layout,0,sizeof(*layout)); layout->node_count=6; layout->group_count=1;
        layout->edge_count=4; layout->direct_routes=1; layout->horizontal=horizontal;
        for (i=0;i<6;++i) {
            layout->nodes[i].key=i; layout->nodes[i].label="focus band";
            layout->nodes[i].layer=i==0 ? 0:i==4 ? 2:1;
        }
        layout->edges[0]=(GD_LayoutEdge){0,4,0,1,GD_PREREQUISITES};
        layout->edges[1]=(GD_LayoutEdge){0,1,1,1,GD_PREREQUISITES};
        layout->edges[2]=(GD_LayoutEdge){2,3,2,1,GD_PREREQUISITES};
        layout->edges[3]=(GD_LayoutEdge){3,2,3,1,GD_PREREQUISITES};
        TEST_ASSERT(gd_layout_arrange(layout,GD_LAYOUT_DIRECTED,GD_SORT_NAME));
        memcpy(before,layout->nodes,6*sizeof(before[0])); gd_layout_focus_capture(focus,layout);
        TEST_ASSERT(gd_layout_focus_apply(focus,layout,NULL,1,1,0,1)==2);
        TEST_ASSERT(focus->fixed_roots[0] && focus->fixed_roots[5] && !focus->fixed_roots[2] && !focus->fixed_roots[3]);
        TEST_ASSERT(layout->nodes[5].x==before[5].x && layout->nodes[5].y==before[5].y);
        for (i=0;i<6;++i) TEST_ASSERT(horizontal ? layout->nodes[i].x==before[i].x:layout->nodes[i].y==before[i].y);
        /* Uniform translations can round at different float exponents. */
        TEST_ASSERT(fabsf((layout->nodes[2].x-layout->nodes[3].x)-(before[2].x-before[3].x))<0.001f);
        TEST_ASSERT(fabsf((layout->nodes[2].y-layout->nodes[3].y)-(before[2].y-before[3].y))<0.001f);
        TEST_ASSERT(gd_layout_route_distance(layout,0,layout->nodes[1].x,layout->nodes[1].y)>20);
        gd_layout_focus_restore(focus,layout); TEST_ASSERT(!memcmp(before,layout->nodes,6*sizeof(before[0])));
        TEST_ASSERT(gd_layout_focus_apply(focus,layout,NULL,1,1,0,0)==-1);
    }
    /* Column constraints are independent of role/depth bands. */
    for (i=0;i<6;++i) layout->nodes[i].side=i%2;
    TEST_ASSERT(gd_layout_arrange(layout,GD_LAYOUT_COLUMNS,GD_SORT_NAME));
    memcpy(before,layout->nodes,6*sizeof(before[0])); gd_layout_focus_capture(focus,layout);
    TEST_ASSERT(gd_layout_focus_apply(focus,layout,NULL,1,0,0,1)==2);
    for (i=0;i<6;++i) TEST_ASSERT(!focus->fixed_roots[i]);
    for (i=0;i<6;++i) TEST_ASSERT(layout->nodes[i].x==before[i].x);
    gd_layout_focus_restore(focus,layout);
    /* Same-band SCC fan: direct straight links would pass through nearer peers.
       Keep band coordinates; shared bowed geometry must clear node footprints. */
    for (horizontal=0;horizontal<2;++horizontal) {
        int e;
        memset(layout,0,sizeof(*layout)); layout->node_count=4; layout->group_count=1;
        layout->edge_count=7; layout->direct_routes=1; layout->horizontal=horizontal;
        for (i=0;i<4;++i) { layout->nodes[i].key=i; layout->nodes[i].label="same band"; }
        for (i=1;i<4;++i) {
            layout->edges[2*(i-1)]=(GD_LayoutEdge){0,i,2*(i-1),1,GD_PREREQUISITES};
            layout->edges[2*(i-1)+1]=(GD_LayoutEdge){i,0,2*(i-1)+1,1,GD_PREREQUISITES};
        }
        layout->edges[6]=(GD_LayoutEdge){0,0,6,1,GD_PREREQUISITES};
        TEST_ASSERT(gd_layout_arrange(layout,GD_LAYOUT_DIRECTED,GD_SORT_NAME));
        gd_layout_focus_capture(focus,layout);
        for (j=0;j<2;++j) {
            if (j) TEST_ASSERT(gd_layout_focus_apply(focus,layout,NULL,1,1,0,1)==2);
            for (e=0;e<7;++e) for (i=0;i<4;++i) if (i!=layout->edges[e].a && i!=layout->edges[e].b)
                TEST_ASSERT(gd_layout_route_distance(layout,e,layout->nodes[i].x,layout->nodes[i].y)>20);
        }
        gd_layout_focus_restore(focus,layout);
    }
    /* Long same-band bows must not grow into an occupied neighboring band. */
    for (horizontal=0;horizontal<2;++horizontal) {
        int e;
        memset(layout,0,sizeof(*layout)); layout->node_count=4; layout->group_count=1;
        layout->edge_count=32; layout->direct_routes=1; layout->horizontal=horizontal;
        for (i=0;i<4;++i) {
            layout->nodes[i].key=i; layout->nodes[i].label="bow bands"; layout->nodes[i].layer=i==2 ? 0:1;
        }
        for (e=0;e<32;++e) layout->edges[e]=(GD_LayoutEdge){e%2,1-e%2,e,1,GD_PREREQUISITES};
        TEST_ASSERT(gd_layout_arrange(layout,GD_LAYOUT_DIRECTED,GD_SORT_NAME));
        if (horizontal) { layout->nodes[0].y=-1320; layout->nodes[1].y=1320; layout->nodes[2].y=layout->nodes[3].y=0; }
        else { layout->nodes[0].x=-1320; layout->nodes[1].x=1320; layout->nodes[2].x=layout->nodes[3].x=0; }
        for (e=0;e<32;++e) for (i=2;i<4;++i)
            TEST_ASSERT(gd_layout_route_distance(layout,e,layout->nodes[i].x,layout->nodes[i].y)>20);
    }
    /* Root-box repair must not collapse separate distance rings. */
    memset(layout,0,sizeof(*layout)); layout->node_count=4; layout->group_count=1;
    layout->edge_count=2; layout->mode=GD_LAYOUT_FORCE;
    for (i=0;i<4;++i) {
        layout->nodes[i].key=i; layout->nodes[i].label="root repair";
        layout->nodes[i].width=220; layout->nodes[i].height=120;
        layout->nodes[i].x=i==3 ? 636:(float)i*200;
    }
    layout->edges[0]=(GD_LayoutEdge){0,1,0,1,GD_PREREQUISITES};
    layout->edges[1]=(GD_LayoutEdge){1,2,1,1,GD_PREREQUISITES};
    gd_layout_focus_capture(focus,layout);
    TEST_ASSERT(gd_layout_focus_apply(focus,layout,NULL,1,1,0,1)==2);
    for (j=0;j<2;++j) {
        if (j) gd_layout_focus_reapply(focus,layout,1);
        TEST_ASSERT(focus->fixed_roots[0] && focus->fixed_roots[3]);
        TEST_ASSERT(layout->nodes[0].x==0 && layout->nodes[0].y==0 && layout->nodes[3].x==636 && layout->nodes[3].y==0);
        TEST_ASSERT(layout->nodes[2].x-layout->nodes[1].x>244);
        TEST_ASSERT(layout->nodes[1].y==layout->nodes[2].y && layout->nodes[1].y>=144);
        TEST_ASSERT(gd_layout_route_distance(layout,0,636,0)>20 && gd_layout_route_distance(layout,1,636,0)>20);
    }
    /* Maximum display scope, not a game simulation or convergence claim. */
    {
        clock_t start; int e;
        memset(layout,0,sizeof(*layout)); layout->node_count=GD_MAX_RECORDS;
        layout->edge_count=GD_MAX_PAIRS; layout->group_count=1; layout->direct_routes=1;
        for (i=0;i<GD_MAX_RECORDS;++i) {
            layout->nodes[i].key=i; layout->nodes[i].label="focus capacity"; layout->nodes[i].layer=i%16;
        }
        for (e=0;e<GD_MAX_PAIRS;++e) {
            int a=e%GD_MAX_RECORDS,b=(a+1+(e/GD_MAX_RECORDS)*7)%GD_MAX_RECORDS;
            layout->edges[e]=(GD_LayoutEdge){a,b,e,1,GD_PREREQUISITES};
        }
        TEST_ASSERT(gd_layout_arrange(layout,GD_LAYOUT_DIRECTED,GD_SORT_NAME));
        gd_layout_focus_capture(focus,layout); start=clock();
        TEST_ASSERT(gd_layout_focus_apply(focus,layout,NULL,1,1,0,1)==2);
        for (i=0;i<GD_MAX_RECORDS;++i) TEST_ASSERT(isfinite(layout->nodes[i].x) && layout->nodes[i].y==focus->base[i].y);
        fprintf(stderr,"distance focus: %zu bytes owned scratch; 4096 nodes / 32768 links: %.1f ms prepare; ",sizeof(*focus),1000.0*(clock()-start)/CLOCKS_PER_SEC);
        start=clock(); for (i=0;i<120;++i) gd_layout_focus_reapply(focus,layout,1);
        fprintf(stderr,"%.1f ms / 120 cached poses\n",1000.0*(clock()-start)/CLOCKS_PER_SEC);
    }
    /* Directed free layout with many fixed roots; layout mode is not direction. */
    {
        clock_t start; int e,half=GD_MAX_RECORDS/2;
        for (e=0;e<GD_MAX_PAIRS;++e) {
            int a=e%half,b=half+(a+(e/half)*7)%half;
            layout->edges[e]=(GD_LayoutEdge){a,b,e,1,GD_PREREQUISITES};
        }
        TEST_ASSERT(gd_layout_arrange(layout,GD_LAYOUT_FORCE,GD_SORT_NAME));
        gd_layout_focus_capture(focus,layout); start=clock();
        TEST_ASSERT(gd_layout_focus_apply(focus,layout,NULL,1,1,half,1)==2);
        for (i=0;i<GD_MAX_RECORDS;++i) {
            TEST_ASSERT(focus->fixed_roots[i]==(i<half) && isfinite(layout->nodes[i].x) && isfinite(layout->nodes[i].y));
            if (i<half) TEST_ASSERT(layout->nodes[i].x==focus->base[i].x && layout->nodes[i].y==focus->base[i].y);
        }
        fprintf(stderr,"distance focus: 2048 fixed roots / directed free layout: %.1f ms prepare\n",1000.0*(clock()-start)/CLOCKS_PER_SEC);
    }
    gd_layout_focus_destroy(focus); release(storage,layout);
)
