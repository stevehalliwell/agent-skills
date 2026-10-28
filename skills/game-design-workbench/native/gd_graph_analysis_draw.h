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
static inline void gd_graph_branch_badge(unsigned flags, int x, int y, Color background) {
    const char *label=gd_graph_branch_label(flags);
    DrawRectangle(x-2,y-1,MeasureText(label,13)+4,15,background);
    DrawText(label,x,y,13,gd_graph_branch_color(flags));
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
static inline void gd_graph_path_badge(const GD_GraphPaths *paths, const GD_PathSelection *selection, const GD_PathBypass *bypass,
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
    DrawRectangle(x-2,y-1,MeasureText(label,12)+4,14,background); DrawText(label,x,y,12,*witness ? gd_bypass_color:gd_path_color);
}
static inline void gd_graph_foundation_badge(const GD_GraphClosure *closure, int node, int x, int y, Color background) {
    char label[80],combination[32]="";
    if (closure->direct[node]>=2) snprintf(combination,sizeof(combination)," AND%d%s",closure->direct[node],closure->finished[node] ? "":"+");
    snprintf(label,sizeof(label),"F%d%s%s%s%s",closure->users[node],closure->complete ? "":"+",combination,
             closure->unknown[node] ? " ?":"",closure->cycle[node] ? " cycle":"");
    DrawRectangle(x-2,y-1,MeasureText(label,12)+4,14,background);
    DrawText(label,x,y,12,(Color){165,207,151,255});
}
static inline void gd_graph_path_start_marker(int x, int y) {
    DrawTriangleLines((Vector2){(float)x,(float)y-5},(Vector2){(float)x-5,(float)y+5},(Vector2){(float)x+5,(float)y+5},(Color){225,233,241,255});
}
#endif
