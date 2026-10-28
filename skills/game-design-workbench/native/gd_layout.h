#ifndef GD_LAYOUT_H
#define GD_LAYOUT_H
#include "gd.h"

/* Transient graph presentation. No record/status/prerequisite/rank semantics.
   Caller supplies topology, display labels, grouping, and two-column sides. */
typedef enum { GD_LAYOUT_FORCE, GD_LAYOUT_COLUMNS, GD_LAYOUT_DIRECTED } GD_LayoutMode;
typedef enum { GD_SORT_NAME, GD_SORT_PARTNERS } GD_LayoutSort;
typedef struct {
    const char *label;
    int key, group, side, degree, layer;
    float axis, anchor;
    float x, y, vx, vy;
    float width, height; /* Drawing bounds; zero uses documented text/card defaults. */
} GD_LayoutNode;
/* Opaque incident-role tags: authored GD_Field, derived case pair, or resource
   relation kind. Endpoint a/b is part of incident role, never inferred flow. */
enum { GD_RELATION_CASE_PAIR=GD_FIELD_COUNT, GD_RELATION_RESOURCE_BASE };
typedef struct { int a, b, key, active, relation; } GD_LayoutEdge;
typedef struct { float x, y, width, height; int first, count; } GD_LayoutGroup;
typedef struct { int a,b,edge,ordinal,count; } GD_LayoutLane;
typedef struct { float x,y; } GD_LayoutPoint;
typedef struct { GD_LayoutPoint points[8]; int count; } GD_LayoutRoute;
typedef struct {
    GD_LayoutNode nodes[GD_MAX_RECORDS];
    GD_LayoutEdge edges[GD_MAX_PAIRS];
    GD_LayoutGroup groups[GD_MAX_RECORDS];
    GD_LayoutLane lanes[GD_MAX_PAIRS];
    int edge_lane[GD_MAX_PAIRS], route_lanes[GD_MAX_RECORDS];
    float layer_span[GD_MAX_RECORDS+2];
    int directed_shelf;
    int order[GD_MAX_RECORDS], position[GD_MAX_RECORDS];
    int node_count, edge_count, group_count;
    GD_LayoutMode mode;
    GD_LayoutSort sort;
    unsigned tick;
    int horizontal;
    float width, height;
} GD_Layout;

/* Call after filling nodes/edges/counts; labels/topology must outlive layout.
   Rearrangement resets positions/velocities, not caller topology or identities. */
int gd_layout_arrange(GD_Layout *layout, GD_LayoutMode mode, GD_LayoutSort sort);
/* Called by arrange after topology validation. Per-pair lanes retain source
   index/direction; no allocation or authored-edge changes. */
void gd_layout_route_prepare(GD_Layout *layout);
float gd_layout_node_width(const GD_LayoutNode *node);
/* At most eight polyline points. Geometry follows live dragged positions.
   Directed long/back links use group-side corridors; not a complete obstacle
   solver. MDA retains its card/Bezier renderer. No reachability semantics. */
void gd_layout_route(const GD_Layout *layout, int edge, GD_LayoutRoute *route);
float gd_layout_route_distance(const GD_Layout *layout, int edge, float x, float y);
/* One fixed display step; at most 32 repulsion samples/node + all active edges.
   held_node is temporarily fixed by pointer drag, or -1. No wall-clock dependency. */
void gd_layout_step(GD_Layout *layout, int held_node);
/* Directed layers lock the ordering axis, including paused pointer dragging.
   layer=-1 denotes an explicitly unplaced shelf; caller owns layer semantics. */
void gd_layout_move_node(GD_Layout *layout, int node, float x, float y);
/* Skill-domain adapter in gd_skills.c. Rank bands compress numeric gaps and
   order same-rank prerequisites by depth. No source ranks are assigned. */
int gd_skill_layout(const GD_Model *model, GD_Layout *layout, int rank_mode, int horizontal, GD_LayoutSort sort);
/* Challenge-domain adapter: shortest entry-hop bands, unreachable shelf;
   preserves next arrows/cycles. Bands are not teaching or physical depth. */
int gd_route_layout(const GD_Model *model, GD_Layout *layout, int horizontal, GD_LayoutSort sort);
/* Focused presentation of already-consumed declarations, not a schema reader
   or impact analysis. Indices belong to one model snapshot; reset after reload.
   Expansion is one-hop, both directions. No status filtering or inferred links. */
typedef struct {
    unsigned char visible[GD_MAX_RECORDS], expanded[GD_MAX_RECORDS];
    int root, boundary_count;
} GD_Focus;
int gd_focus_reset(const GD_Model *model, GD_Focus *focus, int root);
int gd_focus_expand(const GD_Model *model, GD_Focus *focus, int record);
/* Rebuild after draft edits. Retains revealed isolates; borrows model labels.
   Boundary count includes raw links incident to visible nodes but not drawn. */
int gd_focus_layout(const GD_Model *model, GD_Focus *focus, GD_Layout *layout, GD_LayoutSort sort);
/* MDA role bands preserve authored edge direction; no rank/causal inference. */
int gd_mda_layout(const GD_Model *model, const GD_Mda *mda, GD_Layout *layout, int bands);
#endif
