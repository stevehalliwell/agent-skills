#include "gd_layout.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define GD_REPULSION_SAMPLES 32

typedef struct { int node, group, layer, degree, key; const char *label; } LayoutOrder;
static int compare_order(const void *a, const void *b) {
    const LayoutOrder *left = a, *right = b;
    int order;
    if (left->group != right->group) return left->group < right->group ? -1 : 1;
    if (left->layer != right->layer) return left->layer < right->layer ? -1 : 1;
    if (left->degree != right->degree) return left->degree > right->degree ? -1 : 1;
    order = strcmp(left->label, right->label);
    return order ? order : (left->key > right->key) - (left->key < right->key);
}
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
            (mode == GD_LAYOUT_DIRECTED && (node->layer < -1 || node->layer >= GD_MAX_RECORDS))) return 0;
        order[i].node = i; order[i].group = node->group; order[i].key = node->key;
        order[i].layer = mode == GD_LAYOUT_DIRECTED ? (node->layer < 0 ? GD_MAX_RECORDS+1 : node->layer) : 0;
        order[i].degree = sort == GD_SORT_PARTNERS ? node->degree : 0; order[i].label = node->label;
        layout->groups[node->group].count++; counts[node->group][node->side]++;
    }
    for (i = 0; i < layout->edge_count; ++i)
        if (layout->edges[i].a < 0 || layout->edges[i].a >= layout->node_count ||
            layout->edges[i].b < 0 || layout->edges[i].b >= layout->node_count) return 0;
    qsort(order, layout->node_count, sizeof(order[0]), compare_order);
    for (i = 0; i < layout->node_count; ++i) {
        layout->order[i] = order[i].node;
        layout->position[order[i].node] = i;
        if (!i || order[i-1].group != order[i].group) layout->groups[order[i].group].first = i;
    }
    layout->width = layout->height = 0;
    if (mode == GD_LAYOUT_DIRECTED) {
        int maximum = -1, layer_counts[GD_MAX_RECORDS+2], placed[GD_MAX_RECORDS+2];
        int incoming[GD_MAX_RECORDS], next[GD_MAX_PAIRS];
        float x = 0;
        for (i = 0; i < layout->node_count; ++i) {
            incoming[i] = -1;
            if (layout->nodes[i].layer > maximum) maximum = layout->nodes[i].layer;
        }
        for (i = 0; i < layout->edge_count; ++i) {
            const GD_LayoutEdge *edge = &layout->edges[i];
            if (!edge->active || layout->nodes[edge->a].group != layout->nodes[edge->b].group ||
                layout->nodes[edge->a].layer < 0 || layout->nodes[edge->a].layer >= layout->nodes[edge->b].layer) continue;
            next[i] = incoming[edge->b]; incoming[edge->b] = i;
        }
        for (i = 0; i < layout->group_count; ++i) {
            GD_LayoutGroup *group = &layout->groups[i];
            int j, widest = 1, roots = 0, root_index = 0;
            memset(layer_counts, 0, sizeof(layer_counts)); memset(placed, 0, sizeof(placed));
            for (j = 0; j < group->count; ++j) {
                const GD_LayoutNode *node = &layout->nodes[layout->order[group->first+j]];
                int slot = node->layer < 0 ? maximum+2 : node->layer;
                if (++layer_counts[slot] > widest) widest = layer_counts[slot];
                if (incoming[layout->order[group->first+j]] < 0) ++roots;
            }
            if (roots > widest) widest = roots;
            group->x = x; group->y = 0; group->width = 80+widest*170.0f;
            group->height = 130*(maximum+3)+100.0f;
            for (j = 0; j < group->count; ++j) {
                GD_LayoutNode *node = &layout->nodes[layout->order[group->first+j]];
                int slot = node->layer < 0 ? maximum+2 : node->layer;
                int edge = incoming[layout->order[group->first+j]], parents = 0;
                float anchor = 0;
                /* Keep parallel root lanes through late bands. Without this,
                   unrelated intermediate nodes can sit on a long edge. */
                for (; edge >= 0; edge = next[edge]) { anchor += layout->nodes[layout->edges[edge].a].anchor; ++parents; }
                node->anchor = parents ? anchor/parents+(placed[slot]-(layer_counts[slot]-1)/2.0f)*80 :
                    x+group->width/2+(root_index++-(roots-1)/2.0f)*170;
                ++placed[slot]; node->axis = 65+slot*130.0f;
                node->x = node->anchor;
                node->y = node->axis; node->vx = node->vy = 0;
                if (layout->horizontal) { float swap = node->x; node->x = node->y; node->y = swap; }
            }
            x += group->width+24;
            if (group->height > layout->height) layout->height = group->height;
            if (layout->horizontal) {
                float swap = group->x; group->x = group->y; group->y = swap;
                swap = group->width; group->width = group->height; group->height = swap;
            }
        }
        layout->width = x > 0 ? x-24 : 0;
        if (layout->horizontal) { float swap = layout->width; layout->width = layout->height; layout->height = swap; }
        layout->mode = mode; layout->sort = sort; layout->tick = 0;
        return 1;
    }
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
            unsigned offset = 1 + ((layout->tick*GD_REPULSION_SAMPLES + (unsigned)sample) % (unsigned)(group->count-1));
            int other_index = layout->order[group->first+(local+(int)offset)%group->count];
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
    for (i = 0; i < layout->edge_count; ++i) {
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
