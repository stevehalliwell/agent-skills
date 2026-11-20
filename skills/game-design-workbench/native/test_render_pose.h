#include "gd_render_pose.h"
TEST(render_pose,
    GD_Layout *raw=allocate(storage,sizeof(*raw)),*twin=allocate(storage,sizeof(*twin));GD_RenderPose *pose=allocate(storage,sizeof(*pose));int i;double square=0,previous=0;
    TEST_ASSERT(raw && twin && pose && sizeof(*pose)<4*1024*1024);memset(raw,0,sizeof(*raw));memset(pose,0,sizeof(*pose));
    raw->node_count=3;raw->edge_count=2;raw->group_count=1;for(i=0;i<3;++i){raw->nodes[i]=(GD_LayoutNode){.label="render",.key=i,.width=120,.height=48};pose->anchors[i]=i;}
    raw->edges[0]=(GD_LayoutEdge){0,1,0,1,0};raw->edges[1]=(GD_LayoutEdge){0,2,1,1,0};TEST_ASSERT(gd_layout_arrange(raw,GD_LAYOUT_FORCE,GD_SORT_NAME));memcpy(twin,raw,sizeof(*raw));
    for(i=0;i<600;++i){gd_layout_step(raw,-1);gd_layout_step(twin,-1);TEST_ASSERT(gd_render_pose_update(pose,raw,raw,-1,1.0/60,0));TEST_ASSERT(!memcmp(raw,twin,sizeof(*raw)));}
    raw->nodes[0].x=raw->nodes[0].y=0;pose->anchors[1]=0;raw->nodes[1].x=24;raw->nodes[1].y=11;
    TEST_ASSERT(gd_render_pose_update(pose,raw,raw,-1,1.0/60,1));
    for(i=1;i<=600;++i){double value=i%2 ? 1:-1;raw->nodes[0].x=(float)value;raw->nodes[1].x=(float)value+24;
        TEST_ASSERT(gd_render_pose_update(pose,raw,raw,-1,1.0/60,0));
        if(i>480){double delta=pose->layout.nodes[0].x-previous;square+=delta*delta;}
        previous=pose->layout.nodes[0].x;TEST_ASSERT(fabs(pose->layout.nodes[1].x-pose->layout.nodes[0].x-24)<0.0001 && fabs(pose->layout.nodes[1].y-pose->layout.nodes[0].y-11)<0.0001);
    }
    TEST_ASSERT(square<48); /* Raw alternating signal has squared motion 480. */
    {double shown=pose->layout.nodes[0].x;memcpy(twin,raw,sizeof(*raw));
        TEST_ASSERT(gd_render_pose_update(pose,raw,raw,1,1.0/60,0) && pose->layout.nodes[0].x==(float)shown && !memcmp(raw,twin,sizeof(*raw)));
        raw->nodes[0].x+=10;raw->nodes[1].x+=10;TEST_ASSERT(gd_render_pose_update(pose,raw,raw,1,1.0/60,0) && fabs(pose->layout.nodes[0].x-shown-10)<0.0001);
    }
    raw->nodes[0].x=0;raw->nodes[1].x=24;TEST_ASSERT(gd_render_pose_update(pose,raw,raw,-1,1.0/60,1));
    for(i=1;i<=600;++i){raw->nodes[0].x=(float)i*4;raw->nodes[1].x=raw->nodes[0].x+24;TEST_ASSERT(gd_render_pose_update(pose,raw,raw,-1,1.0/60,0));}
    TEST_ASSERT(raw->nodes[0].x-pose->layout.nodes[0].x>0 && raw->nodes[0].x-pose->layout.nodes[0].x<25);
    TEST_ASSERT(gd_render_pose_update(pose,raw,raw,-1,NAN,0) && pose->layout.nodes[0].x==raw->nodes[0].x);
    raw->nodes[0].key=123;raw->nodes[0].x+=50;TEST_ASSERT(gd_render_pose_update(pose,raw,raw,-1,1.0/60,0) && pose->layout.nodes[0].x==raw->nodes[0].x);
    raw->tick=0;TEST_ASSERT(!gd_render_pose_matches(pose,raw,raw));TEST_ASSERT(gd_render_pose_update(pose,raw,raw,-1,0.5,0));
    TEST_ASSERT(!gd_render_pose_update(pose,&pose->layout,raw,-1,1.0/60,0));
    pose->anchors[0]=-1;TEST_ASSERT(!gd_render_pose_update(pose,raw,raw,-1,1.0/60,0));pose->anchors[0]=0;
    printf("render pose: %lu bytes; alternating-signal squared motion %.6f versus raw 480; solver twin identical for 600 steps\n",(unsigned long)sizeof(*pose),square);
    release(storage,pose);release(storage,twin);release(storage,raw);
)
