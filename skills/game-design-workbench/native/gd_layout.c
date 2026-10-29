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
int gd_layout_arrange(GD_Layout *layout, GD_LayoutMode mode, GD_LayoutSort sort) {
    LayoutOrder order[GD_MAX_RECORDS];
    int i, pass, counts[GD_MAX_RECORDS][2] = {{0}};
    float y = 0;
    if (!layout || layout->node_count < 0 || layout->node_count > GD_MAX_RECORDS ||
        layout->edge_count < 0 || layout->edge_count > GD_MAX_PAIRS ||
        layout->group_count < 0 || layout->group_count > GD_MAX_RECORDS ||
        (mode != GD_LAYOUT_FORCE && mode != GD_LAYOUT_COLUMNS && mode != GD_LAYOUT_DIRECTED) ||
        (sort != GD_SORT_NAME && sort != GD_SORT_PARTNERS)) return 0;
    memset(layout->groups, 0, sizeof(layout->groups));
    for (i = 0; i < layout->node_count; ++i) {
        GD_LayoutNode *node = &layout->nodes[i];
        if (!node->label || node->group < 0 || node->group >= layout->group_count ||
            node->side < 0 || node->side > 1 || node->degree < 0 ||
            !isfinite(node->width) || !isfinite(node->height) || node->width<0 || node->height<0 ||
            (mode == GD_LAYOUT_DIRECTED && (node->layer < -1 || node->layer >= GD_MAX_RECORDS))) return 0;
        order[i].node = i; order[i].group = node->group; order[i].key = node->key;
        order[i].layer = mode == GD_LAYOUT_DIRECTED ? (node->layer < 0 ? GD_MAX_RECORDS+1 : node->layer) : 0;
        order[i].degree = sort == GD_SORT_PARTNERS ? node->degree : 0; order[i].label = node->label;
        layout->groups[node->group].count++; counts[node->group][node->side]++;
    }
    for (i = 0; i < layout->edge_count; ++i)
        if (layout->edges[i].a < 0 || layout->edges[i].a >= layout->node_count ||
            layout->edges[i].b < 0 || layout->edges[i].b >= layout->node_count) return 0;
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
            if (pass) { group->width = 170; group->height = 100; }
            else if (mode == GD_LAYOUT_COLUMNS) {
                int rows = counts[i][0] > counts[i][1] ? counts[i][0] : counts[i][1];
                group->width = 340; group->height = rows * 52.0f + 90;
            } else {
                group->width = group->height = 140 + sqrtf((float)group->count) * 90;
            }
            group->x = x; group->y = y;
            for (j = 0; j < group->count; ++j) {
                GD_LayoutNode *node = &layout->nodes[layout->order[group->first+j]];
                node->vx = node->vy = 0;
                if (pass) { node->x = x+85; node->y = y+60; }
                else if (mode == GD_LAYOUT_COLUMNS) {
                    node->x = x + (node->side ? 250 : 80);
                    node->y = y + 65 + side_rows[node->side]++ * 52;
                } else {
                    float angle = 6.28318530718f * j / group->count;
                    float radius = (group->width-90) * 0.35f;
                    node->x = x+group->width/2 + cosf(angle)*radius;
                    node->y = y+group->height/2 + sinf(angle)*radius;
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
void gd_layout_step(GD_Layout *layout, int held_node) {
    int i;
    if (layout->mode != GD_LAYOUT_FORCE && layout->mode != GD_LAYOUT_DIRECTED) return;
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
        force = (distance-110)*0.015f/distance;
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
    ++layout->tick;
}
void gd_layout_move_node(GD_Layout *layout, int node, float x, float y) {
    if (node < 0 || node >= layout->node_count) return;
    layout->nodes[node].x = layout->mode == GD_LAYOUT_DIRECTED && layout->horizontal ? layout->nodes[node].axis : x;
    layout->nodes[node].y = layout->mode == GD_LAYOUT_DIRECTED && !layout->horizontal ? layout->nodes[node].axis : y;
    layout->nodes[node].vx = layout->nodes[node].vy = 0;
}
