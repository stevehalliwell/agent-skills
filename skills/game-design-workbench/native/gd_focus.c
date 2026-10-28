/* Transient, explicit exploration over relationships already consumed by a
   domain loader. No schema inspection, reference repair, or impact inference. */
#include "gd_layout.h"
#include <string.h>
#include <math.h>

int gd_focus_expand(const GD_Model *model, GD_Focus *focus, int record) {
    int i;
    if (!model || !focus || record < 0 || record >= model->record_count || focus->root < 0) return 0;
    focus->visible[record] = focus->expanded[record] = 1;
    for (i = 0; i < model->edge_count; ++i) {
        const GD_Edge *edge = &model->edges[i];
        if (edge->source == record && edge->target >= 0) focus->visible[edge->target] = 1;
        if (edge->target == record) focus->visible[edge->source] = 1;
    }
    return 1;
}
int gd_focus_reset(const GD_Model *model, GD_Focus *focus, int root) {
    if (!model || !focus || root < -1 || root >= model->record_count) return 0;
    memset(focus, 0, sizeof(*focus)); focus->root = root;
    if (root >= 0) { focus->visible[root] = 1; return gd_focus_expand(model, focus, root); }
    return 1;
}
int gd_focus_layout(const GD_Model *model, GD_Focus *focus, GD_Layout *layout, GD_LayoutSort sort) {
    int map[GD_MAX_RECORDS], i;
    if (!model || !focus || !layout || focus->root < -1 || focus->root >= model->record_count) return 0;
    /* Explicitly expanded nodes also reveal newly edited declarations. */
    for (i = 0; i < model->edge_count; ++i) {
        const GD_Edge *edge = &model->edges[i];
        if (focus->expanded[edge->source] && edge->target >= 0) focus->visible[edge->target] = 1;
        if (edge->target >= 0 && focus->expanded[edge->target]) focus->visible[edge->source] = 1;
    }
    memset(layout, 0, sizeof(*layout)); layout->group_count = 1; focus->boundary_count = 0;
    for (i = 0; i < model->record_count; ++i) {
        map[i] = -1;
        if (focus->visible[i]) {
            GD_LayoutNode *node = &layout->nodes[layout->node_count];
            map[i] = layout->node_count++; node->key = i;
            node->label = model->records[i].name ? model->records[i].name:model->records[i].reference;
            node->side = i == focus->root;
        }
    }
    for (i = 0; i < model->edge_count; ++i) {
        const GD_Edge *edge = &model->edges[i];
        int source = map[edge->source], target = edge->target >= 0 ? map[edge->target] : -1;
        if (source >= 0 && target >= 0) {
            GD_LayoutEdge *drawn = &layout->edges[layout->edge_count++];
            drawn->a = source; drawn->b = target; drawn->key = i; drawn->active = 1; drawn->relation=edge->field;
            layout->nodes[source].degree++; layout->nodes[target].degree++;
        } else if (source >= 0 || target >= 0) ++focus->boundary_count;
    }
    if (!gd_layout_arrange(layout, GD_LAYOUT_FORCE, sort)) return 0;
    /* Stable ego presentation, not a route/prerequisite order. Center root;
       space the remaining revealed records by approximate label width. */
    if (layout->node_count) {
        const float full_turn=6.28318530718f;
        float spacing=120, radius, center;
        int placed=0, neighbors=layout->node_count-1;
        for (i=0;i<layout->node_count;++i) {
            float width=(float)strlen(layout->nodes[i].label)*10+40;
            int collection=model->records[layout->nodes[i].key].collection;
            if (collection>=0 && collection<model->collection_count) {
                float caption=(float)strlen(model->collections[collection].alias)*8+40;
                if (caption>width) width=caption;
            }
            if (width>spacing) spacing=width;
        }
        radius=fmaxf(180,neighbors*spacing/full_turn); center=radius+spacing;
        for (i=0;i<layout->node_count;++i) {
            GD_LayoutNode *node=&layout->nodes[layout->order[i]];
            float angle=neighbors ? full_turn*placed/neighbors:0;
            node->x=center; node->y=center;
            if (node->key!=focus->root) { node->x+=cosf(angle)*radius; node->y+=sinf(angle)*radius; ++placed; }
        }
        layout->width=layout->height=center*2;
        layout->groups[0].width=layout->groups[0].height=center*2;
    }
    return 1;
}
