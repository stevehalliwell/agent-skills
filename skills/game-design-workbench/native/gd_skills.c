#include "gd_layout.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

static void finding(GD_Model *model, int record, const char *code, const char *message,
                    const char *field, const char *reference) {
    if (model->finding_count < GD_MAX_ERRORS) {
        GD_Finding *item = &model->findings[model->finding_count++];
        item->record = record; item->code = code; item->message = message;
        item->field = field; item->reference = reference; item->line = 0;
    } else model->findings_omitted++;
}
static int compare_links(const void *a, const void *b) {
    const GD_SkillLink *left = a, *right = b;
    if (left->prerequisite != right->prerequisite) return left->prerequisite-right->prerequisite;
    return left->dependent-right->dependent;
}
void gd_analyze_skills(GD_Model *model) {
    /* Bounded adjacency and iterative SCC/topological passes. No recursion,
       transitive-closure table, or inference from missing/empty prerequisites. */
    int out_head[GD_MAX_RECORDS], in_head[GD_MAX_RECORDS];
    int out_next[GD_MAX_EDGES], in_next[GD_MAX_EDGES];
    int visited[GD_MAX_RECORDS] = {0}, cursor[GD_MAX_RECORDS], stack[GD_MAX_RECORDS];
    int order[GD_MAX_RECORDS], component[GD_MAX_RECORDS], sizes[GD_MAX_RECORDS] = {0};
    int pending[GD_MAX_RECORDS], queue[GD_MAX_RECORDS], direct_missing[GD_MAX_RECORDS] = {0};
    int i, j, count = 0, components = 0, front = 0, back = 0, unique = 0;
    for (i = 0; i < model->record_count; ++i) {
        model->skills[i].depth = -1; out_head[i] = in_head[i] = component[i] = -1;
    }
    for (i = 0; i < model->edge_count; ++i) {
        const GD_Edge *edge = &model->edges[i];
        GD_SkillLink *link;
        if (edge->field != GD_PREREQUISITES || model->records[edge->source].role != GD_SKILL) continue;
        model->skills[edge->source].declared_prerequisites++;
        if (edge->target < 0 || model->records[edge->target].role != GD_SKILL) {
            direct_missing[edge->source] = model->skills[edge->source].incomplete = model->skill_incomplete = 1;
            finding(model, edge->source, edge->target == -2 ? "skill.prerequisite_ambiguous" : edge->target >= 0 ? "skill.prerequisite_target" : "skill.prerequisite_missing",
                    "Declared prerequisite is not a uniquely loaded skill; downstream placement is unknown", "prerequisites", edge->reference);
            continue;
        }
        link = &model->skill_links[model->skill_link_count++];
        link->prerequisite = edge->target; link->dependent = edge->source;
    }
    qsort(model->skill_links, model->skill_link_count, sizeof(model->skill_links[0]), compare_links);
    for (i = 0; i < model->skill_link_count; ++i) {
        GD_SkillLink link = model->skill_links[i];
        if (unique && model->skill_links[unique-1].prerequisite == link.prerequisite && model->skill_links[unique-1].dependent == link.dependent) continue;
        model->skill_links[unique++] = link;
    }
    model->skill_link_count = unique;
    for (i = 0; i < unique; ++i) {
        int a = model->skill_links[i].prerequisite, b = model->skill_links[i].dependent;
        out_next[i] = out_head[a]; out_head[a] = i;
        in_next[i] = in_head[b]; in_head[b] = i;
        model->skills[a].dependent_count++; model->skills[b].prerequisite_count++;
    }
    /* Kosaraju pass 1: finishing order on prerequisite -> dependent edges. */
    for (i = 0; i < model->record_count; ++i) if (model->records[i].role == GD_SKILL && !visited[i]) {
        int top = 0;
        stack[top++] = i; visited[i] = 1; cursor[i] = out_head[i];
        while (top) {
            int v = stack[top-1], edge = cursor[v];
            if (edge < 0) { order[count++] = v; --top; continue; }
            cursor[v] = out_next[edge]; j = model->skill_links[edge].dependent;
            if (!visited[j]) { visited[j] = 1; cursor[j] = out_head[j]; stack[top++] = j; }
        }
    }
    /* Pass 2: traverse reversed edges. Descendants of cycles are not cycles. */
    for (i = count-1; i >= 0; --i) if (component[order[i]] < 0) {
        int top = 0;
        stack[top++] = order[i]; component[order[i]] = components;
        while (top) {
            int v = stack[--top], edge;
            sizes[components]++;
            for (edge = in_head[v]; edge >= 0; edge = in_next[edge]) {
                j = model->skill_links[edge].prerequisite;
                if (component[j] < 0) { component[j] = components; stack[top++] = j; }
            }
        }
        ++components;
    }
    for (i = 0; i < model->record_count; ++i) if (model->records[i].role == GD_SKILL) {
        if (sizes[component[i]] > 1) model->skills[i].cycle = 1;
        pending[i] = model->skills[i].prerequisite_count;
        if (!pending[i]) queue[back++] = i;
    }
    for (i = 0; i < unique; ++i) if (model->skill_links[i].prerequisite == model->skill_links[i].dependent)
        model->skills[model->skill_links[i].prerequisite].cycle = 1;
    while (front < back) {
        int v = queue[front++], edge;
        GD_SkillState *state = &model->skills[v];
        const GD_Record *record = &model->records[v];
        if (!state->incomplete) {
            if (state->depth < 0) state->depth = 0;
            state->has_display = record->has_rank || state->has_minimum;
            state->display_rank = record->has_rank ? record->rank : state->minimum_rank;
            if (state->has_minimum && (!record->has_rank || state->minimum_rank > state->display_rank)) state->display_rank = state->minimum_rank;
            if (record->has_rank && state->has_minimum && record->rank < state->minimum_rank) {
                state->rank_conflict = 1;
                finding(model, v, "skill.rank_conflict", "Authored rank precedes prerequisite placement; source rank is unchanged", "rank", NULL);
            }
        } else { state->depth = -1; state->has_display = state->has_minimum = 0; }
        for (edge = out_head[v]; edge >= 0; edge = out_next[edge]) {
            int child = model->skill_links[edge].dependent;
            GD_SkillState *target = &model->skills[child];
            if (state->incomplete) target->incomplete = 1;
            else {
                if (target->depth < state->depth+1) target->depth = state->depth+1;
                if (state->has_display && (!target->has_minimum || state->display_rank > target->minimum_rank)) {
                    target->minimum_rank = state->display_rank; target->has_minimum = 1;
                }
            }
            if (!--pending[child]) queue[back++] = child;
        }
    }
    for (i = 0; i < model->record_count; ++i) if (model->records[i].role == GD_SKILL) {
        GD_SkillState *state = &model->skills[i];
        if (pending[i]) {
            state->incomplete = model->skill_incomplete = 1; state->depth = -1;
            state->has_display = state->has_minimum = 0;
        }
        if (state->cycle) finding(model, i, "skill.prerequisite_cycle", "Prerequisite cycle has no valid chain ordering; links remain visible", "prerequisites", NULL);
        else if (state->incomplete && state->declared_prerequisites && !pending[i]) {
            /* Direct missing-reference finding already identifies source; add
               downstream context only when no direct missing edge exists. */
            if (!direct_missing[i]) finding(model, i, "skill.placement_unknown", "Prerequisite ancestry is incomplete; derived placement is unknown", "prerequisites", NULL);
        } else if (pending[i]) finding(model, i, "skill.placement_unknown", "Node depends on a prerequisite cycle; it is not itself necessarily cyclic", "prerequisites", NULL);
    }
    /* Relative presentation layers on the SCC condensation. Missing ancestry
       does not erase known downstream ordering. Cycle members share a conflict
       band; every original link remains in skill_links, including cycle edges. */
    front = back = 0;
    for (i = 0; i < components; ++i) { out_head[i] = -1; pending[i] = order[i] = 0; }
    for (i = 0; i < unique; ++i) {
        int a = component[model->skill_links[i].prerequisite], b = component[model->skill_links[i].dependent];
        if (a == b) continue;
        ++pending[b]; out_next[i] = out_head[a]; out_head[a] = i;
    }
    for (i = 0; i < components; ++i) if (!pending[i]) queue[back++] = i;
    while (front < back) {
        int v = queue[front++], edge;
        for (edge = out_head[v]; edge >= 0; edge = out_next[edge]) {
            int child = component[model->skill_links[edge].dependent];
            if (order[child] < order[v]+1) order[child] = order[v]+1;
            if (!--pending[child]) queue[back++] = child;
        }
    }
    assert(back == components);
    for (i = 0; i < model->record_count; ++i) if (model->records[i].role == GD_SKILL) model->skills[i].layout_layer = order[component[i]];
}

typedef struct { int node, ranked, depth; int64_t rank; } SkillOrder;
static int compare_skill_order(const void *a, const void *b) {
    const SkillOrder *left = a, *right = b;
    if (left->ranked != right->ranked) return left->ranked-right->ranked;
    if (left->ranked && left->rank != right->rank) return left->rank < right->rank ? -1 : 1;
    if (left->depth != right->depth) return left->depth-right->depth;
    return left->node-right->node;
}
int gd_skill_layout(const GD_Model *model, GD_Layout *layout, int rank_mode, int horizontal, GD_LayoutSort sort) {
    int record_node[GD_MAX_RECORDS], i, count = 0, layer = -1;
    SkillOrder order[GD_MAX_RECORDS];
    memset(layout, 0, sizeof(*layout)); layout->horizontal = horizontal;
    layout->group_count = model->component_count;
    for (i = 0; i < model->record_count; ++i) {
        GD_LayoutNode *node;
        const GD_SkillState *state = &model->skills[i];
        record_node[i] = -1;
        if (model->records[i].role != GD_SKILL) continue;
        record_node[i] = layout->node_count;
        node = &layout->nodes[layout->node_count++];
        node->key = i; node->label = model->records[i].name; node->group = model->component_of[i];
        node->degree = state->prerequisite_count+state->dependent_count;
        node->layer = state->layout_layer;
        if (rank_mode) {
            order[count].node = record_node[i]; order[count].ranked = state->incomplete ? 2 : state->has_display;
            order[count].depth = state->layout_layer; order[count].rank = state->incomplete ? 0 : state->display_rank; ++count;
        }
    }
    if (rank_mode) {
        qsort(order, count, sizeof(order[0]), compare_skill_order);
        for (i = 0; i < count; ++i) {
            if (!i || order[i].ranked != order[i-1].ranked ||
                (order[i].ranked && order[i].rank != order[i-1].rank) || order[i].depth != order[i-1].depth) ++layer;
            layout->nodes[order[i].node].layer = layer;
        }
    }
    for (i = 0; i < model->skill_link_count; ++i) {
        GD_LayoutEdge *edge = &layout->edges[layout->edge_count++];
        edge->a = record_node[model->skill_links[i].prerequisite];
        edge->b = record_node[model->skill_links[i].dependent]; edge->key = i; edge->active = 1;
    }
    return gd_layout_arrange(layout, GD_LAYOUT_DIRECTED, sort);
}
