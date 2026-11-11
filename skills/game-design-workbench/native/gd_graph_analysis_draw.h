#ifndef GD_GRAPH_ANALYSIS_DRAW_H
#define GD_GRAPH_ANALYSIS_DRAW_H
#include "gd_graph_analysis.h"
#include "raylib.h"
#include <stdio.h>
#include <math.h>
/* Shared palette for all graph adapters; source colors/arrowheads stay above
   these underlays. Mixed link classes remain explicit rather than choosing
   one endpoint's classification. Node classes are never mixed. */
static inline Color gd_graph_branch_color(unsigned flags) {
    if (flags && (flags&(flags-1))) return (Color){225,233,241,255};
    switch (flags) {
        case GD_GRAPH_SHARED: return (Color){193,155,228,255};
        case GD_GRAPH_A_ONLY: return (Color){108,171,230,255};
        case GD_GRAPH_B_ONLY: return (Color){224,160,109,255};
        default: return (Color){155,174,194,255};
    }
}
static inline const char *gd_graph_branch_label(unsigned flags) {
    switch (flags) {
        case GD_GRAPH_SHARED: return "shared";
        case GD_GRAPH_A_ONLY: return "A only";
        case GD_GRAPH_B_ONLY: return "B only";
        case GD_GRAPH_OTHER_ROLE: return "other role";
        default: return "mixed";
    }
}
/* Renderer-measured union in layout coordinates, including asymmetric text.
   Scale is MDA's displayed-position scale, not camera zoom. */
static inline void gd_graph_footprint_rect(GD_LayoutNode *node,Rectangle rectangle,float scale) {
    GD_LayoutBounds *bounds=&node->footprint;
    float left=rectangle.x/scale-node->x,top=rectangle.y/scale-node->y;
    float right=left+rectangle.width/scale,bottom=top+rectangle.height/scale;
    gd_layout_set_footprint(node,fminf(bounds->left,left),fminf(bounds->top,top),fmaxf(bounds->right,right),fmaxf(bounds->bottom,bottom));
}
/* Measure at raw integer anchors, not filtered positions. Otherwise sub-pixel
   text rounding would feed presentation smoothing back into collision bounds. */
static inline void gd_graph_footprint_rect_at(GD_LayoutNode *raw,const GD_LayoutNode *shown,Rectangle rectangle,float scale) {
    rectangle.x+=(int)(raw->x*scale)-(int)(shown->x*scale);rectangle.y+=(int)(raw->y*scale)-(int)(shown->y*scale);
    gd_graph_footprint_rect(raw,rectangle,scale);
}
static inline void gd_graph_node_text_at(GD_LayoutNode *raw,const GD_LayoutNode *shown,const char *text,int x,int y,int size,Color color,float scale) {
    DrawText(text,x,y,size,color);
    gd_graph_footprint_rect_at(raw,shown,(Rectangle){(float)x,(float)y,(float)MeasureText(text,size),(float)size},scale);
}
static inline Rectangle gd_graph_badge(const char *label,int x,int y,int size,Color background,Color foreground) {
    Rectangle rectangle={(float)x-2,(float)y-1,(float)MeasureText(label,size)+4,(float)size+2};
    DrawRectangleRec(rectangle,background); DrawText(label,x,y,size,foreground); return rectangle;
}
static inline Rectangle gd_graph_branch_badge(unsigned flags, int x, int y, Color background) {
    return gd_graph_badge(gd_graph_branch_label(flags),x,y,13,background,gd_graph_branch_color(flags));
}
static inline void gd_graph_underlay_line(Vector2 a, Vector2 b, float width, Color color, int dashed) {
    int segment;
    if (!dashed) { DrawLineEx(a,b,width,color); return; }
    for (segment=0;segment<16;segment+=2) {
        float start=segment/16.0f,end=(segment+1)/16.0f;
        DrawLineEx((Vector2){a.x+(b.x-a.x)*start,a.y+(b.y-a.y)*start},
                   (Vector2){a.x+(b.x-a.x)*end,a.y+(b.y-a.y)*end},width,color);
    }
}
static inline void gd_graph_route_line(const GD_LayoutRoute *route, float width, Color color, int dashed) {
    int i;
    for (i=1;i<route->count;++i) gd_graph_underlay_line((Vector2){route->points[i-1].x,route->points[i-1].y},
        (Vector2){route->points[i].x,route->points[i].y},width,color,dashed);
}
static inline void gd_graph_route_arrow(const GD_LayoutRoute *route, Color color) {
    GD_LayoutPoint from=route->points[route->count-2],tip=route->points[route->count-1];
    float dx=tip.x-from.x,dy=tip.y-from.y,length=sqrtf(dx*dx+dy*dy);
    if (length<1) return;
    dx/=length; dy/=length;
    DrawLineEx((Vector2){tip.x,tip.y},(Vector2){tip.x-dx*13-dy*6,tip.y-dy*13+dx*6},3,color);
    DrawLineEx((Vector2){tip.x,tip.y},(Vector2){tip.x-dx*13+dy*6,tip.y-dy*13-dx*6},3,color);
}
static const Color gd_path_color={230,142,180,255};
static const Color gd_bypass_color={89,220,226,255};
static inline Rectangle gd_graph_path_badge(const GD_GraphPaths *paths, const GD_PathSelection *selection, const GD_PathBypass *bypass,
                                      int node, int x, int y, Color background) {
    char label[48];
    const char *ancestor=selection && selection->ancestors[node] ? " !":"";
    const char *witness="";
    if (bypass) {
        if (bypass->candidate==node) witness=" C";
        else if (bypass->complete && bypass->found) {
            if (bypass->fork==node) witness=" F";
            else if (bypass->rejoin==node) witness=" R";
        }
    }
    if (!paths->root_count || !paths->complete) snprintf(label,sizeof(label),"D?");
    else if (!paths->reached[node]) snprintf(label,sizeof(label),"D-");
    else if (!paths->undirected && paths->incoming[node]>=2)
        snprintf(label,sizeof(label),"D%d J%d%s%s",paths->dominated[node],paths->incoming[node],ancestor,witness);
    else snprintf(label,sizeof(label),"D%d%s%s",paths->dominated[node],ancestor,witness);
    return gd_graph_badge(label,x,y,12,background,*witness ? gd_bypass_color:gd_path_color);
}
static inline Rectangle gd_graph_shape_badge(const GD_GraphShapes *shapes, int node, int x, int y, Color background) {
    char label[48];
    if (!shapes->complete) snprintf(label,sizeof(label),"shape?");
    else snprintf(label,sizeof(label),"shape %d/%d",shapes->colors[node]+1,shapes->size[shapes->colors[node]]);
    return gd_graph_badge(label,x,y,12,background,(Color){220,205,148,255});
}
static inline int gd_graph_shape_peer(const GD_GraphShapes *shapes, int node, int selected) {
    return shapes && shapes->complete && selected>=0 && selected<shapes->node_count && shapes->colors[selected]>=0 && shapes->colors[node]==shapes->colors[selected];
}
static inline Rectangle gd_graph_reachability_badge(const GD_GraphClosure *closure, int node, int x, int y, Color background) {
    char label[32]; snprintf(label,sizeof(label),"T%d%s",closure->users[node],closure->complete ? "":"+");
    return gd_graph_badge(label,x,y,12,background,(Color){160,200,224,255});
}
static inline Rectangle gd_graph_foundation_badge(const GD_GraphClosure *closure, int node, int x, int y, Color background) {
    char label[80],combination[32]="";
    if (closure->direct[node]>=2) snprintf(combination,sizeof(combination)," AND%d%s",closure->direct[node],closure->finished[node] ? "":"+");
    snprintf(label,sizeof(label),"F%d%s%s%s%s",closure->users[node],closure->complete ? "":"+",combination,
             closure->unknown[node] ? " ?":"",closure->cycle[node] ? " cycle":"");
    return gd_graph_badge(label,x,y,12,background,(Color){165,207,151,255});
}
static inline void gd_graph_path_start_marker(int x, int y) {
    DrawTriangleLines((Vector2){(float)x,(float)y-5},(Vector2){(float)x-5,(float)y+5},(Vector2){(float)x+5,(float)y+5},(Color){225,233,241,255});
}
#endif
