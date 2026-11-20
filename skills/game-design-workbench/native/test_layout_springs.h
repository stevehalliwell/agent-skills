#include "gd_layout_springs.h"
TEST(layout_spring_spacing,
    GD_Layout *layout=allocate(storage,sizeof(*layout));int i;float base,grown;
    TEST_ASSERT(layout);memset(layout,0,sizeof(*layout));layout->node_count=4;layout->group_count=1;layout->edge_count=7;
    for(i=0;i<4;++i)layout->nodes[i]=(GD_LayoutNode){.key=i,.label="spring",.degree=999,.width=100,.height=40};
    layout->edges[0]=(GD_LayoutEdge){0,1,0,1,0};layout->edges[1]=(GD_LayoutEdge){1,0,1,1,0};layout->edges[2]=(GD_LayoutEdge){0,1,2,0,0};layout->edges[3]=(GD_LayoutEdge){0,2,3,1,0};layout->edges[4]=(GD_LayoutEdge){0,0,4,1,0};layout->edges[5]=(GD_LayoutEdge){0,3,5,0,0};layout->edges[6]=(GD_LayoutEdge){3,3,6,1,0};
    TEST_ASSERT(gd_layout_arrange(layout,GD_LAYOUT_FORCE,GD_SORT_NAME));spring_neighbors_prepare(layout);
    TEST_ASSERT(layout->spring_neighbors[0]==2 && layout->spring_neighbors[1]==1 && layout->spring_neighbors[2]==1 && layout->spring_neighbors[3]==0 && layout->nodes[0].degree==999);
    base=spring_target(layout,0,1,8,110);TEST_ASSERT(fabs(base-(0.5*hypot(100,40)*(sqrt(1.125)+1)+8))<0.0001);
    TEST_ASSERT(gd_layout_set_footprint(&layout->nodes[0],-100,-40,100,40));grown=spring_target(layout,0,1,8,110);TEST_ASSERT(grown>base && spring_target(layout,1,0,8,110)==grown);
    TEST_ASSERT(spring_target(layout,0,1,8,1000)==1000 && spring_target(layout,0,1,8,330)>=330);
    layout->edges[3].active=0;spring_neighbors_prepare(layout);TEST_ASSERT(layout->spring_neighbors[0]==1 && spring_target(layout,0,1,8,110)<grown);
    layout->edges[0].active=layout->edges[1].active=0;spring_neighbors_prepare(layout);TEST_ASSERT(layout->spring_neighbors[0]==0 && layout->spring_neighbors[1]==0);
    TEST_ASSERT(spring_radius(&layout->nodes[0],64)==spring_radius(&layout->nodes[0],4095) && spring_radius(&layout->nodes[0],64)>spring_radius(&layout->nodes[0],63));
    TEST_ASSERT(gd_layout_set_footprint(&layout->nodes[0],-1048576,-1048576,1048576,1048576));TEST_ASSERT(isfinite(spring_target(layout,0,1,1048576,110)));
    memset(layout,0,sizeof(*layout));layout->node_count=2;layout->edge_count=1;layout->group_count=2;
    for(i=0;i<2;++i)layout->nodes[i]=(GD_LayoutNode){.key=i,.label="vertical card",.group=i,.width=240,.height=48};layout->edges[0]=(GD_LayoutEdge){0,1,0,1,0};
    TEST_ASSERT(gd_layout_arrange(layout,GD_LAYOUT_FORCE,GD_SORT_NAME));
    for(i=0;i<2;++i){layout->nodes[i].x=0;layout->nodes[i].y=(float)i*200;layout->groups[i].x=0;layout->groups[i].y=(float)i*200;layout->groups[i].width=layout->groups[i].height=0;}
    gd_layout_step(layout,-1);TEST_ASSERT(layout->nodes[0].y<0 && layout->nodes[1].y>200 && layout->spring_neighbors[0]==1 && layout->spring_neighbors[1]==1);
    release(storage,layout);
)
