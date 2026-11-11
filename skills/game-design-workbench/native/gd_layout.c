#include "gd_layout.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define GD_REPULSION_SAMPLES 32

typedef struct { int node, group, layer, degree, key; const char *label; int64_t sum; int count,previous; } LayoutOrder;
static int compare_order(const void *a, const void *b) {
    const LayoutOrder *left = a, *right = b;
    int order;
    if (left->group != right->group) return left->group < right->group ? -1 : 1;
    if (left->layer != right->layer) return left->layer < right->layer ? -1 : 1;
    if (left->degree != right->degree) return left->degree > right->degree ? -1 : 1;
    order = strcmp(left->label, right->label);
    return order ? order : (left->key > right->key) - (left->key < right->key);
}
#include "gd_layout_directed.h"
#include "gd_layout_collision.h"
#include "gd_layout_springs.h"
#include "gd_layout_workspace.h"
#include "gd_layout_seed.h"
int gd_matrix_layout(const GD_Model *model, GD_Layout *layout, GD_LayoutMode mode, GD_LayoutSort sort) {
    int record_node[GD_MAX_RECORDS],i;
    if (!model || !layout) return 0;
    memset(layout,0,sizeof(*layout)); layout->group_count=model->component_count;
    for (i=0;i<model->record_count;++i) {
        const GD_Record *record=&model->records[i]; GD_LayoutNode *node; record_node[i]=-1;
        if (record->role!=GD_NOUN && record->role!=GD_VERB) continue;
        record_node[i]=layout->node_count; node=&layout->nodes[layout->node_count++];
        node->key=i; node->label=record->display_name ? record->display_name:record->name; node->group=model->component_of[i]; node->side=record->role==GD_VERB; node->degree=model->connectivity[i].partners;
    }
    for (i=0;i<model->cell_count;++i) {
        GD_LayoutEdge *edge=&layout->edges[layout->edge_count++]; edge->a=record_node[model->cells[i].noun]; edge->b=record_node[model->cells[i].verb];
        edge->key=i; edge->active=model->cells[i].active; edge->relation=GD_RELATION_CASE_PAIR;
    }
    return gd_layout_arrange(layout,mode,sort);
}
static int component_root(int *parent,int node) {
    while (parent[node]!=node) { parent[node]=parent[parent[node]]; node=parent[node]; }
    return node;
}
int gd_layout_components(GD_Layout *layout) {
    int i,count=0;
    if (!layout || layout->node_count<0 || layout->node_count>GD_MAX_RECORDS || layout->edge_count<0 || layout->edge_count>GD_MAX_PAIRS) return 0;
    for (i=0;i<layout->edge_count;++i) if (layout->edges[i].a<0 || layout->edges[i].a>=layout->node_count || layout->edges[i].b<0 || layout->edges[i].b>=layout->node_count) return 0;
    for (i=0;i<layout->node_count;++i) { layout->order[i]=i; layout->position[i]=0; }
    for (i=0;i<layout->edge_count;++i) if (layout->edges[i].active) {
        int a=component_root(layout->order,layout->edges[i].a),b=component_root(layout->order,layout->edges[i].b);
        if (a==b) continue;
        if (layout->position[a]<layout->position[b]) layout->order[a]=b;
        else { layout->order[b]=a; if (layout->position[a]==layout->position[b]) ++layout->position[a]; }
    }
    for (i=0;i<layout->node_count;++i) layout->position[i]=-1;
    for (i=0;i<layout->node_count;++i) {
        int root=component_root(layout->order,i);
        if (layout->position[root]<0) layout->position[root]=count++;
        layout->nodes[i].group=layout->position[root];
    }
    layout->group_count=count; return 1;
}
int gd_layout_arrange(GD_Layout *layout, GD_LayoutMode mode, GD_LayoutSort sort) {
    LayoutOrder order[GD_MAX_RECORDS];
    int i, pass, counts[GD_MAX_RECORDS][2] = {{0}};
    float y = 0;
    if (!layout || layout->node_count < 0 || layout->node_count > GD_MAX_RECORDS ||
        layout->edge_count < 0 || layout->edge_count > GD_MAX_PAIRS ||
        layout->group_count < 0 || layout->group_count > GD_MAX_RECORDS ||
        !isfinite(layout->spring_gap) || layout->spring_gap<0 || layout->spring_gap>1048576 ||
        (mode != GD_LAYOUT_FORCE && mode != GD_LAYOUT_COLUMNS && mode != GD_LAYOUT_DIRECTED) ||
        (sort != GD_SORT_NAME && sort != GD_SORT_PARTNERS)) return 0;
    memset(&layout->workspace,0,sizeof(layout->workspace));
    memset(layout->groups, 0, sizeof(layout->groups));
    for (i = 0; i < layout->node_count; ++i) {
        GD_LayoutNode *node = &layout->nodes[i];
        if (!node->label || node->group < 0 || node->group >= layout->group_count ||
            node->side < 0 || node->side > 1 || node->degree < 0 ||
            !isfinite(node->width) || !isfinite(node->height) || node->width<0 || node->height<0 ||
            (mode == GD_LAYOUT_DIRECTED && (node->layer < -1 || node->layer >= GD_MAX_RECORDS))) return 0;
        if (node->footprint.left==0 && node->footprint.top==0 && node->footprint.right==0 && node->footprint.bottom==0) {
            float width=gd_layout_node_width(node),height=node->height>0 ? node->height:120;
            if (!gd_layout_set_footprint(node,-width/2,-height/2,width/2,height/2)) return 0;
        } else if (!gd_layout_set_footprint(node,node->footprint.left,node->footprint.top,node->footprint.right,node->footprint.bottom)) return 0;
        order[i].node = i; order[i].group = node->group; order[i].key = node->key;
        order[i].layer = mode == GD_LAYOUT_DIRECTED ? (node->layer < 0 ? GD_MAX_RECORDS+1 : node->layer) : 0;
        order[i].degree = sort == GD_SORT_PARTNERS ? node->degree : 0; order[i].label = node->label;
        layout->groups[node->group].count++; counts[node->group][node->side]++;
        layout->groups[node->group].width=fmaxf(layout->groups[node->group].width,node->footprint.right-node->footprint.left);
        layout->groups[node->group].height=fmaxf(layout->groups[node->group].height,node->footprint.bottom-node->footprint.top);
    }
    for (i = 0; i < layout->edge_count; ++i)
        if (layout->edges[i].a < 0 || layout->edges[i].a >= layout->node_count ||
            layout->edges[i].b < 0 || layout->edges[i].b >= layout->node_count) return 0;
    layout->collision_cursor=layout->collision_checks=layout->collision_moves=0; layout->collision_limited=0;
    gd_layout_route_prepare(layout);
    qsort(order, layout->node_count, sizeof(order[0]), compare_order);
    for (i = 0; i < layout->node_count; ++i) {
        layout->order[i] = order[i].node;
        layout->position[order[i].node] = i;
        if (!i || order[i-1].group != order[i].group) layout->groups[order[i].group].first = i;
    }
    layout->width = layout->height = 0;
    if (mode == GD_LAYOUT_DIRECTED) return directed_arrange(layout,order,sort);
    /* Linked groups first, singleton groups below. Every singleton retains its
       own component; sharing a visual shelf never merges topology. */
    for (pass = 0; pass < 2; ++pass) {
        int column = 0, columns = pass ? 4 : 2;
        float x = 0, row_height = 0;
        for (i = 0; i < layout->group_count; ++i) {
            GD_LayoutGroup *group = &layout->groups[i];
            int j, side_rows[2] = {0, 0};
            if (!group->count || ((group->count == 1) != pass)) continue;
            if (pass) {
                group->width = mode==GD_LAYOUT_FORCE ? fmaxf(170,group->width+fmaxf(32,(float)spring_clearance(layout))):170;
                group->height = mode==GD_LAYOUT_FORCE ? fmaxf(100,group->height+fmaxf(32,(float)spring_clearance(layout))):100;
            }
            else if (mode == GD_LAYOUT_COLUMNS) {
                int rows = counts[i][0] > counts[i][1] ? counts[i][0] : counts[i][1];
                group->width = 340; group->height = rows * 52.0f + 90;
            } else {
                group->width = 140 + sqrtf((float)group->count) * fmaxf(90,group->width+(float)spring_clearance(layout));
                group->height = 140 + sqrtf((float)group->count) * fmaxf(90,group->height+(float)spring_clearance(layout));
            }
            group->x = x; group->y = y;
            for (j = 0; j < group->count; ++j) {
                GD_LayoutNode *node = &layout->nodes[layout->order[group->first+j]];
                node->vx = node->vy = 0;
                if (pass) { node->x = x+(mode==GD_LAYOUT_FORCE ? group->width/2:85); node->y = y+(mode==GD_LAYOUT_FORCE ? group->height/2:60); }
                else if (mode == GD_LAYOUT_COLUMNS) {
                    node->x = x + (node->side ? 250 : 80);
                    node->y = y + 65 + side_rows[node->side]++ * 52;
                } else {
                    float angle = 6.28318530718f * j / group->count;
                    float radius = (group->width-90) * 0.35f;
                    node->x = x+group->width/2 + cosf(angle)*radius;
                    node->y = y+group->height/2 + sinf(angle)*(group->height-90)*0.35f;
                }
            }
            x += group->width + 24; ++column;
            if (group->height > row_height) row_height = group->height;
            if (x-24 > layout->width) layout->width = x-24;
            if (column == columns) { x = 0; y += row_height+24; row_height = 0; column = 0; }
        }
        if (column) y += row_height+24;
    }
    layout->height = y > 0 ? y-24 : 0;
    layout->mode = mode; layout->sort = sort; layout->tick = 0;
    return 1;
}
void gd_layout_step_weighted(GD_Layout *layout, int held_node,const int *counts) {
    int i;
    if (layout->mode != GD_LAYOUT_FORCE && layout->mode != GD_LAYOUT_DIRECTED) return;
    if(layout->mode==GD_LAYOUT_FORCE)spring_neighbors_prepare(layout);
    for (i = 0; i < layout->node_count; ++i) {
        GD_LayoutNode *node = &layout->nodes[i];
        GD_LayoutGroup *group = &layout->groups[node->group];
        int local, sample, samples = group->count-1;
        float target_x = layout->mode == GD_LAYOUT_DIRECTED ? (layout->horizontal ? node->axis : node->anchor) : group->x+group->width/2;
        float target_y = layout->mode == GD_LAYOUT_DIRECTED ? (layout->horizontal ? node->anchor : node->axis) : group->y+group->height/2;
        node->vx += (target_x-node->x)*0.0015f;
        node->vy += (target_y-node->y)*0.0015f;
        if (samples > GD_REPULSION_SAMPLES) samples = GD_REPULSION_SAMPLES;
        local = layout->position[i]-group->first;
        for (sample = 0; sample < samples; ++sample) {
            /* Fixed reciprocal partners avoid time-varying forces near rest.
               Large groups retain the same bounded approximation, not all pairs. */
            int forward=1+(sample/2)*((group->count-1)/2)/(GD_REPULSION_SAMPLES/2);
            int offset=group->count-1<=GD_REPULSION_SAMPLES ? sample+1:
                sample%2 ? group->count-forward:forward;
            int other_index = layout->order[group->first+(local+offset)%group->count];
            const GD_LayoutNode *other = &layout->nodes[other_index];
            float dx = node->x-other->x, dy = node->y-other->y;
            float square = dx*dx+dy*dy;
            if (square < 1) { dx = (i < other_index ? -1.0f : 1.0f); dy = 0.5f; square = dx*dx+dy*dy; }
            {
                float force = 6000.0f / (square+100) / sqrtf(square);
                node->vx += dx*force; node->vy += dy*force;
            }
        }
    }
    /* Directed placement already orders semantic bands. Generic edge springs
       would pull distinct label lanes back onto their shared parent. */
    for (i = 0; layout->mode!=GD_LAYOUT_DIRECTED && i < layout->edge_count; ++i) {
        const GD_LayoutEdge *edge = &layout->edges[i];
        GD_LayoutNode *a, *b;
        float dx, dy, distance, force;
        if (!edge->active) continue;
        a = &layout->nodes[edge->a]; b = &layout->nodes[edge->b];
        dx = b->x-a->x; dy = b->y-a->y; distance = sqrtf(dx*dx+dy*dy);
        if (distance < 1) continue;
        force = (distance-spring_target(layout,edge->a,edge->b,GD_LAYOUT_CLEARANCE,collision_spring_length(layout,a,b,dx,dy,distance)))*0.015f/distance;
        if (counts && counts[i]>1) force*=1+fminf(3,log2f((float)counts[i]));
        a->vx += dx*force; a->vy += dy*force; b->vx -= dx*force; b->vy -= dy*force;
    }
    for (i = 0; i < layout->node_count; ++i) {
        GD_LayoutNode *node = &layout->nodes[i];
        float speed;
        if (layout->mode == GD_LAYOUT_DIRECTED) {
            if (layout->horizontal) { node->x = node->axis; node->vx = 0; }
            else { node->y = node->axis; node->vy = 0; }
        }
        if (i == held_node) { node->vx = node->vy = 0; continue; }
        node->vx *= 0.85f; node->vy *= 0.85f;
        speed = sqrtf(node->vx*node->vx+node->vy*node->vy);
        if (speed > 5) { node->vx *= 5/speed; node->vy *= 5/speed; }
        node->x += node->vx; node->y += node->vy;
    }
    if (layout->mode==GD_LAYOUT_FORCE) {collision_separate(layout,held_node);workspace_contain(layout,held_node);}
    ++layout->tick;
}
void gd_layout_step(GD_Layout *layout,int held_node) { gd_layout_step_weighted(layout,held_node,NULL); }
void gd_layout_move_node(GD_Layout *layout, int node, float x, float y) {
    if (node < 0 || node >= layout->node_count) return;
    if (!isfinite(x) || !isfinite(y) || (layout->nodes[node].x==x && layout->nodes[node].y==y)) return;
    layout->nodes[node].x = layout->mode == GD_LAYOUT_DIRECTED && layout->horizontal ? layout->nodes[node].axis : x;
    layout->nodes[node].y = layout->mode == GD_LAYOUT_DIRECTED && !layout->horizontal ? layout->nodes[node].axis : y;
    layout->nodes[node].vx = layout->nodes[node].vy = 0;
}
void gd_layout_pickup_node(GD_Layout *layout, int node, float x, float y) {
    if (node<0 || node>=layout->node_count || !isfinite(x) || !isfinite(y)) return;
    layout->pickup_x=layout->nodes[node].x-x; layout->pickup_y=layout->nodes[node].y-y;
    layout->pickup_cursor_x=x; layout->pickup_cursor_y=y;
}
void gd_layout_drag_node(GD_Layout *layout, int node, float x, float y) {
    if (!isfinite(x) || !isfinite(y) || (x==layout->pickup_cursor_x && y==layout->pickup_cursor_y)) return;
    layout->pickup_cursor_x=x; layout->pickup_cursor_y=y;
    gd_layout_move_node(layout,node,x+layout->pickup_x,y+layout->pickup_y);
}
