#ifndef GD_LAYOUT_FOCUS_H
#define GD_LAYOUT_FOCUS_H
#include "gd_layout.h"

typedef struct { float x,y,vx,vy; int key; } GD_FocusPose;
typedef struct { int node,distance,key; float band,angle,cross; } GD_FocusOrder;
/* Per-view owned scratch. Undirected shown-link distance is a placement aid,
   not flow, knowledge, similarity, or a replacement for semantic bands. */
typedef struct {
    GD_Allocator allocator;
    GD_FocusPose base[GD_MAX_RECORDS];
    GD_LayoutPoint focused[GD_MAX_RECORDS];
    GD_LayoutGroup groups[GD_MAX_RECORDS];
    GD_FocusOrder order[GD_MAX_RECORDS];
    int head[GD_MAX_RECORDS],next[2*GD_MAX_PAIRS],queue[GD_MAX_RECORDS],distance[GD_MAX_RECORDS];
    unsigned char visited[GD_MAX_RECORDS],fixed_roots[GD_MAX_RECORDS];
    int captured,node_count,group_count,direct_routes,anchor_node,active;
} GD_LayoutFocus;
GD_LayoutFocus *gd_layout_focus_create(GD_Allocator allocator);
void gd_layout_focus_destroy(GD_LayoutFocus *focus);
void gd_layout_focus_capture(GD_LayoutFocus *focus, const GD_Layout *layout);
void gd_layout_focus_restore(GD_LayoutFocus *focus, GD_Layout *layout);
/* Rebuild from saved pose, never from previous focused positions. Return -1 for
   invalid scope, 0 for absent/hidden focus anchor, 1 for same anchor, 2 for new.
   Directed projections fix zero-incoming-shown-arrow data roots; undirected
   projections have none. No synthetic roots for cycles or changes to starts.
   O(nodes + links + node sort) placement, two incidences per source edge.
   Direct-route clearance adds at most GD_MAX_RECORDS squared edge checks;
   not an exhaustive collision/crossing solver. Cached UI frames reapply pose.
   display_scale converts raw positions to renderer coordinates (MDA free: 3).
   Distances are -1 for disconnected and -2 for hidden nodes. */
void gd_layout_focus_reapply(GD_LayoutFocus *focus, GD_Layout *layout, float display_scale);
int gd_layout_focus_apply(GD_LayoutFocus *focus, GD_Layout *layout,
    const unsigned char *shown, int include_inactive, int directed, int root, float display_scale);
#endif
