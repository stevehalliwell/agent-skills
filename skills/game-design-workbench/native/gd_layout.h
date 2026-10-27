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
} GD_LayoutNode;
typedef struct { int a, b, key, active; } GD_LayoutEdge;
typedef struct { float x, y, width, height; int first, count; } GD_LayoutGroup;
typedef struct {
    GD_LayoutNode nodes[GD_MAX_RECORDS];
    GD_LayoutEdge edges[GD_MAX_PAIRS];
    GD_LayoutGroup groups[GD_MAX_RECORDS];
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
