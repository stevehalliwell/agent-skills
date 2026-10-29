#include "gd_layout.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

float gd_layout_node_width(const GD_LayoutNode *node) {
    int columns=0; const unsigned char *text=(const unsigned char *)node->label;
    if (node->width>0) return node->width;
    for (;*text;++text) if ((*text&0xc0)!=0x80) ++columns;
    return fmaxf(220.0f,columns*10.0f+44);
}
static int lane_order(const void *left, const void *right) {
    const GD_LayoutLane *a=left,*b=right;
    if (a->a!=b->a) return a->a<b->a ? -1:1;
    if (a->b!=b->b) return a->b<b->b ? -1:1;
    return (a->edge>b->edge)-(a->edge<b->edge);
}
void gd_layout_route_prepare(GD_Layout *layout) {
    int i,begin;
    memset(layout->route_lanes,0,(size_t)layout->group_count*sizeof(int));
    for (i=0;i<layout->edge_count;++i) {
        const GD_LayoutEdge *edge=&layout->edges[i]; GD_LayoutLane *lane=&layout->lanes[i];
        lane->a=edge->a<edge->b ? edge->a:edge->b; lane->b=edge->a>edge->b ? edge->a:edge->b; lane->edge=i;
    }
    qsort(layout->lanes,(size_t)layout->edge_count,sizeof(layout->lanes[0]),lane_order);
    for (begin=0;begin<layout->edge_count;) {
        int end=begin+1;
        while (end<layout->edge_count && layout->lanes[end].a==layout->lanes[begin].a && layout->lanes[end].b==layout->lanes[begin].b) ++end;
        for (i=begin;i<end;++i) {
            GD_LayoutLane *lane=&layout->lanes[i]; int a=layout->nodes[lane->a].group,b=layout->nodes[lane->b].group;
            lane->ordinal=i-begin; lane->count=end-begin; layout->edge_lane[lane->edge]=i;
            if (end-begin>layout->route_lanes[a]) layout->route_lanes[a]=end-begin;
            if (end-begin>layout->route_lanes[b]) layout->route_lanes[b]=end-begin;
        }
        begin=end;
    }
}
static GD_LayoutPoint oriented(float cross, float axis, int horizontal) {
    return horizontal ? (GD_LayoutPoint){axis,cross}:(GD_LayoutPoint){cross,axis};
}
void gd_layout_route(const GD_Layout *layout, int index, GD_LayoutRoute *route) {
    const GD_LayoutEdge *edge; const GD_LayoutNode *a,*b; const GD_LayoutLane *lane;
    float ordinal,width_a,height_a,width_b,height_b;
    assert(layout && route && index>=0 && index<layout->edge_count);
    edge=&layout->edges[index]; a=&layout->nodes[edge->a]; b=&layout->nodes[edge->b]; lane=&layout->lanes[layout->edge_lane[index]];
    ordinal=(float)lane->ordinal; width_a=gd_layout_node_width(a); height_a=a->height>0 ? a->height:120;
    width_b=gd_layout_node_width(b); height_b=b->height>0 ? b->height:120;
    if (edge->a==edge->b && layout->mode==GD_LAYOUT_DIRECTED && layout->direct_routes) {
        int slot=a->layer<0 ? layout->directed_shelf:a->layer,i;
        float extra=fminf(32,ordinal*12),band=layout->layer_span[slot]/2+24;
        float right=a->x+fminf(width_a/2+36+extra,layout->horizontal ? band:width_a/2+68);
        float top=a->y-fminf(height_a/2+36+extra,layout->horizontal ? height_a/2+68:band);
        GD_LayoutPoint start={a->x+20,a->y},end={a->x,a->y-20};
        route->count=8;
        for (i=0;i<8;++i) {
            float t=i/7.0f,u=1-t;
            route->points[i]=(GD_LayoutPoint){u*u*u*start.x+3*u*u*t*right+3*u*t*t*right+t*t*t*end.x,
                u*u*u*start.y+3*u*u*t*a->y+3*u*t*t*top+t*t*t*end.y};
        }
    } else if (edge->a==edge->b) {
        float right=a->x+width_a/2+32+ordinal*12,top=a->y-height_a/2-32-ordinal*12;
        route->count=5;
        route->points[0]=(GD_LayoutPoint){a->x+20,a->y}; route->points[1]=(GD_LayoutPoint){right,a->y};
        route->points[2]=(GD_LayoutPoint){right,top}; route->points[3]=(GD_LayoutPoint){a->x,top}; route->points[4]=(GD_LayoutPoint){a->x,a->y-20};
    } else if (layout->mode==GD_LAYOUT_DIRECTED && layout->direct_routes &&
        (layout->horizontal ? a->x==b->x:a->y==b->y)) {
        /* Rounded bow stays inside own band clearance. Length-dependent
           quadratic height could hit adjacent bands; straight chords could hit
           intervening same-band nodes. Ports/ramp/flat span avoid both cases. */
        int horizontal=layout->horizontal,slot=a->layer<0 ? layout->directed_shelf:a->layer;
        float ac=horizontal ? a->y:a->x,bc=horizontal ? b->y:b->x,axis=horizontal ? a->x:a->y;
        float direction=bc>=ac ? 1.0f:-1.0f,sign=edge->a<edge->b ? -1.0f:1.0f;
        float rise=fminf(72+fminf(24,ordinal*8),layout->layer_span[slot]/2+24);
        float corner=fminf(48,fabsf(bc-ac)/4),near=20+0.4f*(rise-20),far=20+0.85f*(rise-20);
        route->count=8;
        route->points[0]=oriented(ac,axis+sign*20,horizontal);
        route->points[1]=oriented(ac,axis+sign*near,horizontal);
        route->points[2]=oriented(ac+direction*corner/4,axis+sign*far,horizontal);
        route->points[3]=oriented(ac+direction*corner,axis+sign*rise,horizontal);
        route->points[4]=oriented(bc-direction*corner,axis+sign*rise,horizontal);
        route->points[5]=oriented(bc-direction*corner/4,axis+sign*far,horizontal);
        route->points[6]=oriented(bc,axis+sign*near,horizontal);
        route->points[7]=oriented(bc,axis+sign*20,horizontal);
    } else if (layout->mode!=GD_LAYOUT_DIRECTED || layout->direct_routes) {
        float dx=b->x-a->x,dy=b->y-a->y,length=sqrtf(dx*dx+dy*dy),offset=(ordinal-(lane->count-1)/2.0f)*22;
        float sign=edge->a<edge->b ? 1.0f:-1.0f;
        if (length<1) { dx=1; dy=0; length=1; }
        route->count=3;
        route->points[0]=(GD_LayoutPoint){a->x+20*dx/length,a->y+20*dy/length};
        route->points[1]=(GD_LayoutPoint){(a->x+b->x)/2-sign*dy/length*offset,(a->y+b->y)/2+sign*dx/length*offset};
        route->points[2]=(GD_LayoutPoint){b->x-20*dx/length,b->y-20*dy/length};
    } else {
        int horizontal=layout->horizontal; float sign=horizontal ? -1.0f:1.0f;
        float ac=horizontal ? a->y:a->x,bc=horizontal ? b->y:b->x,aa=horizontal ? a->x:a->y,ba=horizontal ? b->x:b->y;
        float across_a=horizontal ? height_a:width_a,across_b=horizontal ? height_b:width_b;
        int slot_a=a->layer<0 ? layout->directed_shelf:a->layer,slot_b=b->layer<0 ? layout->directed_shelf:b->layer;
        float along_a=layout->layer_span[slot_a],along_b=layout->layer_span[slot_b];
        float local_a=ac+sign*(across_a/2+16),local_b=bc+sign*(across_b/2+16);
        if (a->layer>=0 && b->layer==a->layer+1 && lane->count==1) {
            float middle=(aa+along_a/2+ba-along_b/2)/2;
            route->count=6;
            route->points[0]=oriented(ac+sign*20,aa,horizontal); route->points[1]=oriented(local_a,aa,horizontal);
            route->points[2]=oriented(local_a,middle,horizontal); route->points[3]=oriented(local_b,middle,horizontal);
            route->points[4]=oriented(local_b,ba,horizontal); route->points[5]=oriented(bc+sign*20,ba,horizontal);
        } else if (a->layer==b->layer) {
            float top=fminf(aa-along_a/2,ba-along_b/2)-24-ordinal*12;
            route->count=4;
            route->points[0]=oriented(ac,aa-20,horizontal); route->points[1]=oriented(ac,top,horizontal);
            route->points[2]=oriented(bc,top,horizontal); route->points[3]=oriented(bc,ba-20,horizontal);
        } else {
            const GD_LayoutGroup *group=&layout->groups[a->group],*other=&layout->groups[b->group];
            float outer=fmaxf(horizontal ? group->y+group->height:group->x+group->width,
                              horizontal ? other->y+other->height:other->x+other->width)+40+ordinal*12;
            float gap_a=aa+along_a/2+24,gap_b=ba-along_b/2-24;
            route->count=8;
            route->points[0]=oriented(ac+sign*20,aa,horizontal); route->points[1]=oriented(local_a,aa,horizontal);
            route->points[2]=oriented(local_a,gap_a,horizontal); route->points[3]=oriented(outer,gap_a,horizontal);
            route->points[4]=oriented(outer,gap_b,horizontal); route->points[5]=oriented(local_b,gap_b,horizontal);
            route->points[6]=oriented(local_b,ba,horizontal); route->points[7]=oriented(bc+sign*20,ba,horizontal);
        }
    }
}
float gd_layout_route_distance(const GD_Layout *layout, int edge, float x, float y) {
    GD_LayoutRoute route; float nearest=INFINITY; int i;
    gd_layout_route(layout,edge,&route);
    for (i=1;i<route.count;++i) {
        GD_LayoutPoint a=route.points[i-1],b=route.points[i]; float dx=b.x-a.x,dy=b.y-a.y,square=dx*dx+dy*dy;
        float t=square>0 ? ((x-a.x)*dx+(y-a.y)*dy)/square:0;
        t=fmaxf(0,fminf(1,t)); dx=x-a.x-t*dx; dy=y-a.y-t*dy;
        nearest=fminf(nearest,sqrtf(dx*dx+dy*dy));
    }
    return nearest;
}
