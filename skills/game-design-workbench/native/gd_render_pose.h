/* Presentation-only One Euro pose. Own this bounded scratch outside snapshots.
   No solver writes, allocation, integration, collision or spring changes.
   Algorithm: https://gery.casiez.net/1euro/ ; layout units, seconds, Hz. */
#ifndef GD_RENDER_POSE_H
#define GD_RENDER_POSE_H
#include <math.h>
#include <stddef.h>
#include <string.h>
#include "gd_layout.h"
typedef struct { double raw_x,raw_y,x,y,dx,dy; } GD_RenderSample;
typedef struct {
    GD_Layout layout;
    GD_RenderSample samples[GD_MAX_RECORDS];
    int anchors[GD_MAX_RECORDS];
    const GD_Layout *source;
    const void *identity;
    unsigned tick;
    int valid,held_anchor;
    double held_offset_x,held_offset_y;
} GD_RenderPose;
static double render_alpha(double cutoff,double seconds) {return 1/(1+1/(6.283185307179586*cutoff*seconds));}
static double render_axis(double value,double previous,double *filtered,double *derivative,double seconds) {
    double alpha=render_alpha(1,seconds),speed=(value-previous)/seconds;
    *derivative+=alpha*(speed-*derivative);
    alpha=render_alpha(1+0.005*fabs(*derivative),seconds);
    *filtered+=alpha*(value-*filtered);return *filtered;
}
static int gd_render_pose_matches(const GD_RenderPose *pose,const GD_Layout *source,const void *identity) {
    int i;if(!pose->valid || pose->source!=source || pose->identity!=identity || pose->tick>source->tick || pose->layout.node_count!=source->node_count || pose->layout.edge_count!=source->edge_count)return 0;
    for(i=0;i<source->node_count;++i)if(pose->layout.nodes[i].key!=source->nodes[i].key || pose->layout.nodes[i].label!=source->nodes[i].label)return 0;
    return 1;
}
static void render_copy(GD_Layout *out,const GD_Layout *source) {
    /* Copy live geometry/routing only; never copy unused capacity or collision scratch. */
    size_t nodes=(size_t)source->node_count,edges=(size_t)source->edge_count;
    memcpy(out->nodes,source->nodes,nodes*sizeof(out->nodes[0]));memcpy(out->edges,source->edges,edges*sizeof(out->edges[0]));
    memcpy(out->groups,source->groups,(size_t)source->group_count*sizeof(out->groups[0]));memcpy(out->lanes,source->lanes,edges*sizeof(out->lanes[0]));
    memcpy(out->edge_lane,source->edge_lane,edges*sizeof(out->edge_lane[0]));memcpy(out->route_lanes,source->route_lanes,nodes*sizeof(out->route_lanes[0]));memcpy(out->layer_span,source->layer_span,sizeof(out->layer_span));
    memcpy(&out->pickup_x,&source->pickup_x,offsetof(GD_Layout,order)-offsetof(GD_Layout,pickup_x));
    memcpy(out->order,source->order,nodes*sizeof(out->order[0]));memcpy(out->position,source->position,nodes*sizeof(out->position[0]));
    memcpy(&out->node_count,&source->node_count,offsetof(GD_Layout,collision_order)-offsetof(GD_Layout,node_count));
    out->collision_cursor=source->collision_cursor;out->collision_checks=source->collision_checks;out->collision_moves=source->collision_moves;out->collision_limited=source->collision_limited;
}
static int gd_render_pose_update(GD_RenderPose *pose,const GD_Layout *source,const void *identity,int held,double seconds,int reset) {
    int i,held_anchor=-1;
    if(!pose || !source || source==&pose->layout || source->node_count<0 || source->node_count>GD_MAX_RECORDS || source->edge_count<0 || source->edge_count>GD_MAX_PAIRS || source->group_count<0 || source->group_count>GD_MAX_RECORDS)return 0;
    for(i=0;i<source->node_count;++i)if(!isfinite(source->nodes[i].x) || !isfinite(source->nodes[i].y) || pose->anchors[i]<0 || pose->anchors[i]>=source->node_count || pose->anchors[pose->anchors[i]]!=pose->anchors[i])return 0;
    reset=reset || !gd_render_pose_matches(pose,source,identity) || !isfinite(seconds) || seconds<=0 || seconds>0.25;
    if(seconds<0.001)seconds=0.001;
    if(held>=0 && held<source->node_count)held_anchor=pose->anchors[held];
    render_copy(&pose->layout,source);
    for(i=0;i<source->node_count;++i)if(pose->anchors[i]==i){
        const GD_LayoutNode *node=&source->nodes[i];GD_RenderSample *sample=&pose->samples[i];
        if(reset)*sample=(GD_RenderSample){node->x,node->y,node->x,node->y,0,0};
        else if(i==held_anchor){
            /* Keep pickup's visible offset. Pointer deltas pass through without
               lag; neither a click nor smoothing teleports the solver node. */
            if(pose->held_anchor!=held_anchor){pose->held_offset_x=sample->x-node->x;pose->held_offset_y=sample->y-node->y;}
            sample->raw_x=node->x;sample->raw_y=node->y;sample->x=node->x+pose->held_offset_x;sample->y=node->y+pose->held_offset_y;sample->dx=sample->dy=0;
        }
        else {render_axis(node->x,sample->raw_x,&sample->x,&sample->dx,seconds);render_axis(node->y,sample->raw_y,&sample->y,&sample->dy,seconds);sample->raw_x=node->x;sample->raw_y=node->y;}
    }
    for(i=0;i<source->node_count;++i){int anchor=pose->anchors[i];const GD_RenderSample *sample=&pose->samples[anchor];
        pose->layout.nodes[i].x=(float)(sample->x+((double)source->nodes[i].x-source->nodes[anchor].x));
        pose->layout.nodes[i].y=(float)(sample->y+((double)source->nodes[i].y-source->nodes[anchor].y));
    }
    if(reset)pose->held_offset_x=pose->held_offset_y=0;
    pose->held_anchor=held_anchor;pose->source=source;pose->identity=identity;pose->tick=source->tick;pose->valid=1;return 1;
}
#endif
