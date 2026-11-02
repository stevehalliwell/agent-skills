/* Geometry regression shared by real Actor/Effect scene fixtures. */
#include "gd_query_scene.h"
#include <stdlib.h>
static int query_scene_free_checks(GD_QueryScene *scene) {
    GD_Allocator allocator=scene->allocator;GD_QueryScene *before=malloc(sizeof(*before));
    GD_LayoutPoint *points=malloc(GD_MAX_RECORDS*sizeof(*points));int okay=0,i,step,primary=-1,ghost=-1;
#define FREE_CHECK(value) do {if(!(value)){fprintf(stderr,"Query Free check failed: %s (line %d)\n",#value,__LINE__);goto cleanup;}}while(0)
    FREE_CHECK(before && points);memcpy(before,scene,sizeof(*before));
    FREE_CHECK(!gd_query_scene_place(NULL,GD_LAYOUT_FORCE,0,1) && !gd_query_scene_place(scene,GD_LAYOUT_COLUMNS,0,1));
    FREE_CHECK(!gd_query_scene_place(scene,GD_LAYOUT_FORCE,2,1) && !gd_query_scene_place(scene,GD_LAYOUT_FORCE,1,0) && !gd_query_scene_place(scene,GD_LAYOUT_FORCE,1,NAN));
    {int count=scene->layout.node_count;scene->layout.node_count=GD_MAX_RECORDS+1;i=gd_query_scene_place(scene,GD_LAYOUT_FORCE,0,1);scene->layout.node_count=count;FREE_CHECK(!i);}
    if(scene->layout.edge_count) {int endpoint=scene->layout.edges[0].a;scene->layout.edges[0].a=-1;i=gd_query_scene_place(scene,GD_LAYOUT_FORCE,0,1);scene->layout.edges[0].a=endpoint;FREE_CHECK(!i);}
    for(i=0;i<scene->layout.node_count;++i) if(scene->nodes[i].kind==GD_QUERY_NODE_VARIANT) {if(scene->nodes[i].ghost) ghost=i;else primary=i;}
    FREE_CHECK(gd_query_scene_place(scene,GD_LAYOUT_FORCE,0,1.5f));
    for(i=0;i<scene->layout.node_count;++i) FREE_CHECK(scene->layout.nodes[i].x==before->layout.nodes[i].x && scene->layout.nodes[i].y==before->layout.nodes[i].y);
    FREE_CHECK(gd_query_scene_place(scene,GD_LAYOUT_FORCE,1,1.5f));
    for(i=0;i<scene->layout.node_count;++i) points[i]=(GD_LayoutPoint){scene->layout.nodes[i].x,scene->layout.nodes[i].y};
    FREE_CHECK(gd_query_scene_place(scene,GD_LAYOUT_FORCE,1,1.5f));
    for(i=0;i<scene->layout.node_count;++i) FREE_CHECK(scene->layout.nodes[i].x==points[i].x && scene->layout.nodes[i].y==points[i].y);
    {
        GD_QueryFree *old=scene->free_geometry;int stage,allocations=scene->layout.edge_count ? 3:2;
        for(stage=0;stage<allocations;++stage) {
            RouteFailure failure={allocator,stage};scene->allocator=(GD_Allocator){&failure,route_allocate,route_release};
            i=gd_query_scene_place(scene,GD_LAYOUT_FORCE,1,1.5f);scene->allocator=allocator;
            FREE_CHECK(!i && scene->free_geometry==old && scene->layout.mode==GD_LAYOUT_FORCE);
            for(i=0;i<scene->layout.node_count;++i) FREE_CHECK(scene->layout.nodes[i].x==points[i].x && scene->layout.nodes[i].y==points[i].y);
        }
    }
    for(step=0;step<160;++step) gd_query_scene_step(scene,-1);
    for(i=0;i<scene->layout.node_count;++i) FREE_CHECK(isfinite(scene->layout.nodes[i].x) && isfinite(scene->layout.nodes[i].y));
    if(scene->ghosts==GD_QUERY_STACKED_VARIANTS && primary>=0 && ghost>=0) {
        float dx=before->layout.nodes[ghost].x-before->layout.nodes[primary].x,dy=before->layout.nodes[ghost].y-before->layout.nodes[primary].y;
        float x=scene->layout.nodes[ghost].x,y=scene->layout.nodes[ghost].y;
        FREE_CHECK(fabsf(scene->layout.nodes[ghost].x-scene->layout.nodes[primary].x-dx)<0.001f && fabsf(scene->layout.nodes[ghost].y-scene->layout.nodes[primary].y-dy)<0.001f);
        FREE_CHECK(gd_query_scene_hit(scene,x,y+scene->layout.nodes[ghost].height/2-2)==ghost);
        gd_layout_pickup_node(&scene->layout,ghost,x,y);gd_query_scene_drag(scene,ghost,x+80,y+70);
        FREE_CHECK(fabsf(scene->layout.nodes[ghost].x-x-80)<0.001f && fabsf(scene->layout.nodes[ghost].y-y-70)<0.001f);
        FREE_CHECK(fabsf(scene->layout.nodes[ghost].x-scene->layout.nodes[primary].x-dx)<0.001f && fabsf(scene->layout.nodes[ghost].y-scene->layout.nodes[primary].y-dy)<0.001f);
        x=scene->layout.nodes[ghost].x;y=scene->layout.nodes[ghost].y;gd_query_scene_step(scene,ghost);
        FREE_CHECK(scene->layout.nodes[ghost].x==x && scene->layout.nodes[ghost].y==y);
    }
    FREE_CHECK(gd_query_scene_place(scene,GD_LAYOUT_DIRECTED,0,1));
    FREE_CHECK(scene->query==before->query && scene->effect_scope==before->effect_scope && scene->work==before->work && scene->limited==before->limited);
    FREE_CHECK(!memcmp(scene->nodes,before->nodes,sizeof(scene->nodes)) && !memcmp(scene->edges,before->edges,sizeof(scene->edges)) && !memcmp(scene->labels,before->labels,sizeof(scene->labels)));
    FREE_CHECK(!memcmp(scene->layout.edges,before->layout.edges,sizeof(scene->layout.edges)) && scene->layout.node_count==before->layout.node_count && scene->layout.edge_count==before->layout.edge_count);
    for(i=0;i<scene->layout.node_count;++i) FREE_CHECK(scene->layout.nodes[i].x==before->layout.nodes[i].x && scene->layout.nodes[i].y==before->layout.nodes[i].y && scene->layout.nodes[i].width==before->layout.nodes[i].width && scene->layout.nodes[i].height==before->layout.nodes[i].height);
    okay=1;
cleanup:
    scene->allocator=allocator;if(scene->free_geometry) gd_query_scene_place(scene,GD_LAYOUT_DIRECTED,0,1);
    free(points);free(before);return okay;
#undef FREE_CHECK
}
