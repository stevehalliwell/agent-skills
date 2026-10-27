#include "gd.h"
#include "gd_layout.h"
#include "gd_diagnostics_ui.h"
#include "raylib.h"
#include "rlImGui.h"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_NO_EXPORT
#include "cimgui.h"
#include <math.h>
#include <stdarg.h>
#include <string.h>

static const Color graph_background = {18, 24, 34, 255};
static const Color graph_text = {225, 233, 241, 255};
static const Color graph_muted = {150, 169, 190, 255};
static const Color graph_active = {65, 180, 165, 255};
static const Color graph_selected = {255, 217, 125, 255};

typedef struct { int cell, node, interaction, linked, held; } GraphSelection;
static int prepare_layout(const GD_Model *model, GD_Layout *layout, GD_LayoutMode mode, GD_LayoutSort sort, int rank_mode, int horizontal, int challenges, GD_Focus *focus) {
    int record_node[GD_MAX_RECORDS], i;
    if (focus) return gd_focus_layout(model,focus,layout,sort);
    if (challenges) return gd_route_layout(model,layout,horizontal,sort);
    if (model->skill_mode) return gd_skill_layout(model, layout, rank_mode, horizontal, sort);
    memset(layout, 0, sizeof(*layout)); layout->group_count = model->component_count;
    for (i = 0; i < model->record_count; ++i) {
        const GD_Record *record = &model->records[i];
        GD_LayoutNode *node;
        record_node[i] = -1;
        if (record->role != GD_NOUN && record->role != GD_VERB) continue;
        record_node[i] = layout->node_count;
        node = &layout->nodes[layout->node_count++];
        node->key = i; node->label = record->name; node->group = model->component_of[i];
        node->side = record->role == GD_VERB; node->degree = model->connectivity[i].partners;
    }
    for (i = 0; i < model->cell_count; ++i) {
        GD_LayoutEdge *edge = &layout->edges[layout->edge_count++];
        edge->a = record_node[model->cells[i].noun]; edge->b = record_node[model->cells[i].verb];
        edge->key = i; edge->active = model->cells[i].active;
    }
    return gd_layout_arrange(layout, mode, sort);
}
static Vector2 node_point(const GD_LayoutNode *node) { return (Vector2){node->x, node->y}; }
static void fit_camera(Camera2D *camera, const GD_Layout *layout, Rectangle viewport) {
    float min_x = 0, min_y = 0, max_x = 1, max_y = 1;
    int i;
    for (i = 0; i < layout->node_count; ++i) {
        const GD_LayoutNode *node = &layout->nodes[i];
        float label_width = (float)MeasureText(node->label, 18)/2+20;
        if (!i || node->x-label_width < min_x) min_x = node->x-label_width;
        if (!i || node->x+label_width > max_x) max_x = node->x+label_width;
        if (!i || node->y-65 < min_y) min_y = node->y-65;
        if (!i || node->y+50 > max_y) max_y = node->y+50;
    }
    camera->target = (Vector2){(min_x+max_x)/2, (min_y+max_y)/2};
    camera->offset = (Vector2){viewport.x+viewport.width/2, viewport.y+viewport.height/2};
    camera->zoom = fminf(viewport.width/(max_x-min_x+130), viewport.height/(max_y-min_y+70));
    if (camera->zoom > 1.2f) camera->zoom = 1.2f;
    if (camera->zoom < 0.002f) camera->zoom = 0.002f;
}
static float segment_distance(Vector2 p, Vector2 a, Vector2 b) {
    float dx = b.x-a.x, dy = b.y-a.y, square = dx*dx+dy*dy;
    float t = square > 0 ? ((p.x-a.x)*dx+(p.y-a.y)*dy)/square : 0;
    if (t < 0) t = 0; if (t > 1) t = 1;
    dx = p.x-(a.x+t*dx); dy = p.y-(a.y+t*dy);
    return sqrtf(dx*dx+dy*dy);
}
static void graph_input(GD_Layout *layout, GraphSelection *selection, Camera2D *camera,
                        Rectangle viewport, int other_pairs) {
    Vector2 mouse = GetMousePosition(), world;
    int i;
    camera->offset = (Vector2){viewport.x+viewport.width/2, viewport.y+viewport.height/2};
    if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) selection->held = -1;
    if (selection->held >= 0) {
        world = GetScreenToWorld2D(mouse, *camera);
        gd_layout_move_node(layout, selection->held, world.x, world.y);
    }
    if (!CheckCollisionPointRec(mouse, viewport) || igGetIO_Nil()->WantCaptureMouse) return;
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) || IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
        Vector2 delta = GetMouseDelta(); camera->target.x -= delta.x/camera->zoom; camera->target.y -= delta.y/camera->zoom;
    }
    world = GetScreenToWorld2D(mouse, *camera);
    if (GetMouseWheelMove()) {
        Vector2 after;
        camera->zoom *= powf(1.15f, GetMouseWheelMove());
        if (camera->zoom < 0.002f) camera->zoom = 0.002f;
        if (camera->zoom > 4) camera->zoom = 4;
        after = GetScreenToWorld2D(mouse, *camera);
        camera->target.x += world.x-after.x; camera->target.y += world.y-after.y;
    }
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return;
    world = GetScreenToWorld2D(mouse, *camera);
    for (i = layout->node_count-1; i >= 0; --i) {
        Vector2 point = node_point(&layout->nodes[i]);
        float dx = point.x-world.x, dy = point.y-world.y;
        if (dx*dx+dy*dy <= 22*22) {
            selection->node = selection->held = i; selection->cell = selection->linked = -1;
            selection->interaction = 0; return;
        }
    }
    for (i = 0; i < layout->edge_count; ++i) {
        const GD_LayoutEdge *edge = &layout->edges[i];
        if (!edge->active && !other_pairs) continue;
        Vector2 a = node_point(&layout->nodes[edge->a]), b = node_point(&layout->nodes[edge->b]);
        float distance = segment_distance(world, a, b);
        if (edge->a == edge->b) distance = fabsf(sqrtf((world.x-a.x-24)*(world.x-a.x-24)+(world.y-a.y+24)*(world.y-a.y+24))-24);
        if (distance <= 7/camera->zoom) {
            selection->cell = edge->key; selection->node = selection->linked = -1;
            selection->interaction = 0; return;
        }
    }
}
static int witness_node(const GD_Routes *routes, int witness, int record) {
    int i;
    if (!routes || witness<0 || witness>=routes->witness_count) return 0;
    for (i=0;i<routes->witnesses[witness].length;++i) if (routes->witnesses[witness].path[i]==record) return 1;
    return 0;
}
static int witness_edge(const GD_Routes *routes, int witness, int source, int target) {
    int i;
    if (!routes || witness<0 || witness>=routes->witness_count) return 0;
    for (i=1;i<routes->witnesses[witness].length;++i) if (routes->witnesses[witness].path[i-1]==source && routes->witnesses[witness].path[i]==target) return 1;
    return 0;
}
static void draw_graph(const GD_Model *model, const GD_Layout *layout, const GraphSelection *selection,
                       Camera2D camera, Rectangle viewport, int other_pairs, const GD_Draft *draft, int challenges, int focused, const GD_Routes *routes, int witness) {
    Rectangle bounds[GD_MAX_RECORDS];
    int i;
    memset(bounds, 0, sizeof(bounds));
    if (!layout->node_count) {
        DrawText(focused ? "No relationship root selected." : challenges ? "No challenge nodes in supplied folder." : model->skill_mode ? "No skill nodes in supplied folder." : "No noun/verb nodes in supplied folders.", (int)viewport.x+16, (int)viewport.y+24, 18, graph_muted);
        return;
    }
    BeginScissorMode((int)viewport.x, (int)viewport.y, (int)viewport.width, (int)viewport.height);
    BeginMode2D(camera);
    for (i = 0; i < layout->node_count; ++i) {
        const GD_LayoutNode *node = &layout->nodes[i];
        Rectangle *box = &bounds[node->group];
        float half = (float)MeasureText(node->label, 18)/2+22;
        float left = node->x-half, right = node->x+half, top = node->y-55, bottom = node->y+50;
        if (!box->width) *box = (Rectangle){left, top, right-left, bottom-top};
        else {
            float x = fminf(box->x, left), y = fminf(box->y, top);
            box->width = fmaxf(box->x+box->width, right)-x;
            box->height = fmaxf(box->y+box->height, bottom)-y; box->x = x; box->y = y;
        }
    }
    for (i = 0; !focused && !challenges && i < model->component_count; ++i) {
        const GD_Component *group = &model->components[i];
        const char *prefix = group->member_count == 1 && !group->edge_count ? "Isolated: " : "Group: ";
        if (model->skill_mode && group->member_count == 1 && model->skills[group->anchor].incomplete)
            prefix = model->skills[group->anchor].cycle ? "Cycle: " : "Unresolved: ";
        DrawRectangleRounded(bounds[i], 0.12f, 6, (Color){27, 36, 48, 255});
        DrawText(TextFormat("%s%s", prefix, model->records[group->anchor].name),
                 (int)bounds[i].x+10, (int)bounds[i].y+9, 14, graph_muted);
    }
    for (i = 0; i < layout->edge_count; ++i) {
        const GD_LayoutEdge *edge = &layout->edges[i];
        Vector2 a = node_point(&layout->nodes[edge->a]), b = node_point(&layout->nodes[edge->b]);
        Color color = selection->cell == edge->key ? graph_selected : edge->active ? graph_active : graph_muted;
        if (!edge->active && !other_pairs) continue;
        int conflict = !focused && !challenges && model->skill_mode && (model->skills[layout->nodes[edge->a].key].incomplete || model->skills[layout->nodes[edge->b].key].incomplete);
        if (challenges && witness_edge(routes,witness,layout->nodes[edge->a].key,layout->nodes[edge->b].key)) color=graph_selected;
        if (conflict && selection->cell != edge->key) color = (Color){238, 114, 120, 255};
        if ((focused || model->skill_mode) && edge->a == edge->b) DrawCircleLines((int)a.x+24, (int)a.y-24, 24, color);
        else if (edge->active && !conflict) DrawLineEx(a, b, 2.5f, color);
        else {
            int segment;
            for (segment = 0; segment < 16; segment += 2) {
                float start = segment/16.0f, end = (segment+1)/16.0f;
                DrawLineEx((Vector2){a.x+(b.x-a.x)*start, a.y+(b.y-a.y)*start},
                           (Vector2){a.x+(b.x-a.x)*end, a.y+(b.y-a.y)*end}, 1.5f, color);
            }
        }
        if (focused || model->skill_mode) {
            float dx = b.x-a.x, dy = b.y-a.y, length = sqrtf(dx*dx+dy*dy);
            if (length > 1) {
                Vector2 tip;
                dx /= length; dy /= length; tip = (Vector2){b.x-dx*20, b.y-dy*20};
                DrawLineEx(tip, (Vector2){tip.x-dx*13-dy*6, tip.y-dy*13+dx*6}, 3, color);
                DrawLineEx(tip, (Vector2){tip.x-dx*13+dy*6, tip.y-dy*13-dx*6}, 3, color);
            } else {
                DrawLineEx((Vector2){a.x+24,a.y}, (Vector2){a.x+34,a.y-9}, 3, color);
                DrawLineEx((Vector2){a.x+24,a.y}, (Vector2){a.x+34,a.y+7}, 3, color);
            }
            if (focused && selection->cell==edge->key) {
                const char *label=gd_field_name(model->edges[edge->key].field);
                int width=MeasureText(label,13), x=(int)((a.x+b.x-width)/2), y=(int)((a.y+b.y)/2)-8;
                DrawRectangle(x-3,y-2,width+6,18,graph_background); DrawText(label,x,y,13,color);
            }
        } else {
            const char *label = TextFormat("%d cases", model->cells[edge->key].pair_count);
            int width = MeasureText(label, 15), x = (int)((a.x+b.x-width)/2), y = (int)((a.y+b.y)/2)-9;
            DrawRectangle(x-4, y-2, width+8, 20, graph_background); DrawText(label, x, y, 15, color);
        }
    }
    for (i = 0; i < layout->node_count; ++i) {
        const GD_LayoutNode *node = &layout->nodes[i];
        Color color = selection->node == i ? graph_selected : node->side ? (Color){224, 160, 109, 255} : (Color){108, 171, 230, 255};
        if (challenges && witness_node(routes,witness,node->key)) color=graph_selected;
        if (node->side) DrawRectangle((int)node->x-12, (int)node->y-12, 24, 24, color);
        else DrawCircleV(node_point(node), 13, color);
        DrawText(node->label, (int)node->x-MeasureText(node->label, 18)/2, (int)node->y+22, 18, graph_text);
        if (draft && (draft->rank_changed[node->key] || draft->prerequisites_changed[node->key])) {
            DrawCircleLines((int)node->x, (int)node->y, 25, graph_selected);
            DrawText("*", (int)node->x+22, (int)node->y-28, 24, graph_selected);
        }
        if (focused) {
            int collection=model->records[node->key].collection;
            const char *alias=collection>=0 && collection<model->collection_count ? model->collections[collection].alias:"loaded record";
            const char *caption = node->side ? TextFormat("ROOT | %s",alias):alias;
            DrawText(caption,(int)node->x-MeasureText(caption,13)/2,(int)node->y+43,13,graph_muted);
        } else if (challenges) {
            const char *caption = model->records[node->key].entry ? "ENTRY" : routes && routes->reachable[node->key] ? "declared reachable" : "no declared entry path";
            DrawText(caption,(int)node->x-MeasureText(caption,13)/2,(int)node->y+43,13,graph_muted);
        } else if (model->skill_mode) {
            const GD_Record *record = &model->records[node->key]; const GD_SkillState *state = &model->skills[node->key];
            const char *caption = state->cycle ? "CYCLE" : state->incomplete ? "UNKNOWN ANCESTRY" :
                state->rank_conflict ? TextFormat("rank %lld -> %lld (conflict)", (long long)record->rank, (long long)state->display_rank) :
                record->has_rank ? TextFormat("rank %lld", (long long)record->rank) : "rank null";
            Color warning = state->incomplete ? (Color){238,114,120,255} : (Color){245,179,96,255};
            if (state->incomplete || state->rank_conflict) DrawCircleLines((int)node->x, (int)node->y, 18, warning);
            DrawText(caption, (int)node->x-MeasureText(caption, 13)/2, (int)node->y+43, 13, state->incomplete || state->rank_conflict ? warning : graph_muted);
        }
    }
    EndMode2D(); EndScissorMode();
}
typedef struct { int record, has_rank; int64_t rank; char rank_text[64], filter[129]; } SkillEditor;
static int has_prerequisite(const GD_Model *model, int record, const char *reference) {
    int i;
    for (i = 0; i < model->edge_count; ++i) if (model->edges[i].source == record && model->edges[i].field == GD_PREREQUISITES && !strcmp(model->edges[i].reference, reference)) return 1;
    return 0;
}
static int edit_skill(GD_Draft *draft, int record, SkillEditor *editor) {
    const GD_Record *item = &draft->model->records[record];
    int i, changed = 0, apply;
    if (editor->record != record || editor->has_rank != item->has_rank || (item->has_rank && editor->rank != item->rank)) {
        editor->record = record; editor->has_rank = item->has_rank; editor->rank = item->rank;
        if (item->has_rank) snprintf(editor->rank_text, sizeof(editor->rank_text), "%lld", (long long)item->rank);
        else snprintf(editor->rank_text, sizeof(editor->rank_text), "null");
    }
    igSeparator(); igTextUnformatted("Edit in-memory draft (no Markdown writes)", NULL);
    igTextUnformatted("Rank (Enter / Apply)", NULL); igSetNextItemWidth(-1);
    apply = igInputText("##rank", editor->rank_text, sizeof(editor->rank_text), ImGuiInputTextFlags_EnterReturnsTrue, NULL, NULL);
    if (igIsItemDeactivatedAfterEdit()) apply = 1;
    if (igButton("Apply rank to draft", (ImVec2_c){0,0})) apply = 1;
    igSameLine(0, -1);
    if (igButton("Clear rank to null", (ImVec2_c){0,0})) { snprintf(editor->rank_text, sizeof(editor->rank_text), "null"); apply = 1; }
    if (apply) changed = gd_draft_rank(draft, record, editor->rank_text) > 0;
    igTextUnformatted("Filter prerequisites", NULL); igSetNextItemWidth(-1);
    igInputText("##prerequisite-filter", editor->filter, sizeof(editor->filter), 0, NULL, NULL);
    igTextWrapped("Checked skills are prerequisites of this skill. Self/cycle links are allowed and flagged. Node dragging changes layout only.");
    igBeginChild_Str("Prerequisite choices", (ImVec2_c){0,125}, 0, 0);
    for (i = 0; i < draft->model->record_count; ++i) {
        const GD_Record *target = &draft->model->records[i];
        bool enabled;
        if (target->role != GD_SKILL || (*editor->filter && !strstr(target->reference, editor->filter))) continue;
        enabled = has_prerequisite(draft->model, record, target->reference) != 0;
        if (igCheckbox(target->reference, &enabled)) { if (gd_draft_prerequisite(draft, record, target->reference, enabled) > 0) changed = 1; break; }
    }
    igEndChild();
    for (i = 0; i < draft->model->edge_count; ++i) {
        const GD_Edge *edge = &draft->model->edges[i];
        if (edge->source != record || edge->field != GD_PREREQUISITES || (edge->target >= 0 && draft->model->records[edge->target].role == GD_SKILL)) continue;
        igText("Unresolved/wrong-role: %s", edge->reference); igPushID_Int(i);
        if (igButton("Remove unresolved prerequisite", (ImVec2_c){0,0})) {
            if (gd_draft_prerequisite(draft, record, edge->reference, 0) > 0) changed = 1;
            igPopID(); break;
        }
        igPopID();
    }
    return changed;
}
static int undo_edit(GD_Draft *draft, const GD_Layout *layout, GraphSelection *selection, SkillEditor *editor) {
    int record = draft->history_count ? draft->history[draft->history_count-1].record : -1, i;
    if (!gd_draft_undo(draft)) return 0;
    /* Links may have disappeared/reordered. Invalidate edge selection before details. */
    selection->cell = selection->linked = selection->held = -1;
    for (i = 0; i < layout->node_count; ++i) if (layout->nodes[i].key == record) selection->node = i;
    editor->record = -1; return 1;
}
static void route_details(const GD_Model *model, const GD_Routes *routes, int record, int *selected_witness, int *navigate) {
    int i,j;
    if (!routes) { igTextUnformatted("Route analysis unavailable",NULL); return; }
    if (model->records[record].role==GD_CHALLENGE) {
        igText("Entry: %s | Reachable from declared entry: %s",model->records[record].entry ? "yes":"no",routes->reachable[record] ? "yes":"no / unknown");
        for (i=0;i<routes->check_count;++i) if (routes->checks[i].challenge==record) {
            const GD_RouteCheck *check=&routes->checks[i]; igPushID_Int(i);
            igTextWrapped("%s: %s | %s via %s (%s)",model->records[check->skill].reference,gd_support_name(check->coverage),gd_field_name(check->demand_field),model->records[check->demanded_by].reference,check->prerequisite ? "prerequisite":"direct demand");
            if (igButton("Inspect skill prerequisites",(ImVec2_c){0,0})) *navigate=check->skill;
            for (j=0;j<routes->witness_count;++j) if (routes->witnesses[j].check==i) {
                igPushID_Int(j);
                if (igButton(routes->witnesses[j].supported ? "Supported route witness":"Unsupported route witness",(ImVec2_c){0,0})) *selected_witness=j;
                igPopID();
            }
            igPopID();
        }
    }
    if (*selected_witness>=0 && *selected_witness<routes->witness_count) {
        const GD_RouteWitness *path=&routes->witnesses[*selected_witness];
        igSeparator(); igText("%s witness: %d of %d trailing challenges",path->supported ? "Supported":"Unsupported",path->length,path->full_length);
        if (path->length<path->full_length) igTextUnformatted("Prefix truncated; gold highlights retained suffix only.",NULL);
        if (igButton("Clear route highlight",(ImVec2_c){0,0})) *selected_witness=-1;
        for (i=0;i<path->length;++i) { igPushID_Int(i); if (igButton(model->records[path->path[i]].reference,(ImVec2_c){0,0})) *navigate=path->path[i]; igPopID(); }
    }
    if (routes->witnesses_omitted) igText("%d additional witnesses omitted",routes->witnesses_omitted);
}
static int details(const GD_Model *model, const GD_Layout *layout, GraphSelection *selection, GD_Draft *draft, SkillEditor *editor, int challenges, int focused, const GD_Routes *routes, int *witness, int *navigate, int *focus_root, int *focus_expand) {
    int record_index = -1, i, changed = 0;
    if (selection->cell >= 0 && (challenges || focused)) {
        const GD_Edge *edge=&model->edges[selection->cell];
        igText("%s -> %s",model->records[edge->source].name,edge->reference);
        igText("Authored %s reference, source line %d. Not causal evidence.",gd_field_name(edge->field),edge->line);
        record_index=edge->source;
        if (igButton(focused ? "Inspect reference target" : "Inspect next challenge",(ImVec2_c){0,0}) && edge->target>=0) *navigate=edge->target;
    } else if (selection->cell >= 0 && model->skill_mode) {
        const GD_SkillLink *link = &model->skill_links[selection->cell];
        igText("%s -> %s", model->records[link->prerequisite].name, model->records[link->dependent].name);
        igTextUnformatted("Prerequisite -> dependent. All declared statuses included.", NULL);
        record_index = link->dependent;
        if (igButton("Inspect prerequisite", (ImVec2_c){0,0})) selection->linked = link->prerequisite;
    } else if (selection->cell >= 0) {
        const GD_Cell *cell = &model->cells[selection->cell];
        igText("%s x %s", model->records[cell->noun].name, model->records[cell->verb].name);
        igText("%d conditional cases | active pair: %s", cell->pair_count, cell->active ? "yes" : "no");
        if (selection->interaction >= cell->pair_count) selection->interaction = 0;
        if (igButton("Previous case", (ImVec2_c){0,0})) { selection->interaction = (selection->interaction+cell->pair_count-1)%cell->pair_count; selection->linked = -1; }
        igSameLine(0, -1);
        if (igButton("Next case", (ImVec2_c){0,0})) { selection->interaction = (selection->interaction+1)%cell->pair_count; selection->linked = -1; }
        record_index = model->pairs[cell->first_pair+selection->interaction].interaction;
        igText("Case %d/%d: %s", selection->interaction+1, cell->pair_count, model->records[record_index].name);
        igText("Coverage: %s", model->records[record_index].coverage ? model->records[record_index].coverage : "unset");
    } else if (selection->node >= 0) {
        record_index = layout->nodes[selection->node].key;
        igText("%s: %s", focused ? "Record" : challenges ? "Challenge" : model->skill_mode ? "Skill" : model->records[record_index].role == GD_NOUN ? "Noun" : "Verb", model->records[record_index].name);
        if (!focused && !challenges && model->skill_mode) igText("%d loaded prerequisites | %d dependents", model->skills[record_index].prerequisite_count, model->skills[record_index].dependent_count);
        else if (!focused && !challenges) igText("%d active partners | %d total cases", model->connectivity[record_index].partners, model->connectivity[record_index].cases);
    }
    if (record_index<0 && selection->linked>=0) record_index=selection->linked;
    if (record_index < 0) { igTextUnformatted("Select a node or edge to inspect its source.", NULL); return 0; }
    if (selection->linked >= 0) {
        if (igButton("Back to selection", (ImVec2_c){0,0})) selection->linked = -1;
        else record_index = selection->linked;
    }
    igSeparator();
    igTextUnformatted(model->records[record_index].reference, NULL);
    igText("Status: %s", model->records[record_index].status ? model->records[record_index].status : "unset");
    if (igButton(focused ? "Center relationships here" : "Explore relationships here",(ImVec2_c){0,0})) *focus_root=record_index;
    if (focused && igButton("Expand this record one hop (in + out)",(ImVec2_c){0,0})) *focus_expand=record_index;
    if (model->records[record_index].role == GD_SKILL) {
        const GD_Record *record = &model->records[record_index]; const GD_SkillState *state = &model->skills[record_index];
        igText("Authored rank: %s", record->has_rank ? TextFormat("%lld", (long long)record->rank) : "null");
        igText("Prerequisite depth: %s", state->depth >= 0 ? TextFormat("%d", state->depth) : "unknown");
        igText("Relative prerequisite layer: %d (not authored rank)", state->layout_layer);
        igText("Minimum rank: %s | Display rank: %s", state->has_minimum ? TextFormat("%lld", (long long)state->minimum_rank) : state->incomplete ? "unknown" : "none", state->has_display ? TextFormat("%lld", (long long)state->display_rank) : state->incomplete ? "unknown" : "unranked (null)");
        if (state->incomplete) igTextUnformatted(state->cycle ? "CONFLICT: prerequisite cycle." : "UNKNOWN: incomplete prerequisite ancestry.", NULL);
        else if (state->rank_conflict) igTextUnformatted("CONFLICT: authored rank precedes prerequisites. No source edit.", NULL);
        else igTextUnformatted("No declared rank conflict. Not evidence of learning.", NULL);
        if (draft) {
            igTextUnformatted(draft->rank_changed[record_index] || draft->prerequisites_changed[record_index] ? "MODIFIED IN MEMORY: differs from loaded disk baseline" : "Draft values match loaded disk baseline", NULL);
            if (!focused && !challenges) changed = edit_skill(draft, record_index, editor);
        }
    }
    if (!focused && model->route_mode) {
        if ((challenges && model->records[record_index].role==GD_SKILL) || (!challenges && model->records[record_index].role==GD_CHALLENGE)) {
            if (igButton(challenges ? "Show in Skills mode":"Show in Challenges mode",(ImVec2_c){0,0})) *navigate=record_index;
        }
        if (model->records[record_index].role==GD_SKILL) {
            igSeparator(); igTextUnformatted("Challenges declaring this skill (all statuses)",NULL);
            for (i=0;i<model->edge_count;++i) {
                const GD_Edge *edge=&model->edges[i];
                if (edge->target!=record_index || edge->field<GD_INTRODUCES || edge->field>GD_TESTS_MASTERY || model->records[edge->source].role!=GD_CHALLENGE) continue;
                igPushID_Int(i);
                if (igButton(TextFormat("%s: %s",gd_field_name(edge->field),model->records[edge->source].reference),(ImVec2_c){0,0})) *navigate=edge->source;
                igPopID();
            }
        }
        if (!challenges && routes && model->records[record_index].role==GD_SKILL) {
            for (i=0;i<routes->check_count;++i) {
                const GD_RouteCheck *check=&routes->checks[i];
                if (check->skill!=record_index || !check->prerequisite) continue;
                igPushID_Int(GD_MAX_EDGES+i);
                if (igButton(TextFormat("Prerequisite demand: %s via %s",model->records[check->challenge].reference,model->records[check->demanded_by].reference),(ImVec2_c){0,0})) *navigate=check->challenge;
                igPopID();
            }
        }
        if (challenges) route_details(model,routes,record_index,witness,navigate);
    }
    igPushTextWrapPos(0);
    igTextUnformatted(model->records[record_index].source_path, NULL);
    if (igButton("Copy source path", (ImVec2_c){0,0})) SetClipboardText(model->records[record_index].source_path);
    for (i = 0; i < model->edge_count; ++i) {
        const GD_Edge *edge = &model->edges[i];
        const char *label;
        if (edge->source != record_index) continue;
        igPushID_Int(i);
        label = TextFormat("%s: %s%s", gd_field_name(edge->field), edge->reference, edge->target < 0 ? " [not uniquely loaded]" : "");
        igTextUnformatted(label, NULL);
        if (igIsItemClicked(ImGuiMouseButton_Left) && edge->target >= 0) selection->linked = edge->target;
        igPopID();
    }
    if (focused) {
        igSeparator(); igTextUnformatted("Incoming consumed declarations (all loaded records)",NULL);
        for (i=0;i<model->edge_count;++i) {
            const GD_Edge *edge=&model->edges[i];
            if (edge->target!=record_index) continue;
            igTextWrapped("%s: %s (source line %d)",gd_field_name(edge->field),model->records[edge->source].reference,edge->line);
            if (igIsItemClicked(ImGuiMouseButton_Left)) selection->linked=edge->source;
        }
    }
    igSeparator(); igTextUnformatted(model->records[record_index].body, NULL); igPopTextWrapPos();
    return changed;
}
static void graph_log(int level, const char *text, va_list arguments) {
    (void)level; vfprintf(stderr, text, arguments); fputc('\n', stderr);
}
static int selected_record(const GD_Model *model, const GD_Layout *layout, const GraphSelection *selection, int challenges, int focused) {
    if (selection->linked>=0) return selection->linked;
    if (selection->node>=0) return layout->nodes[selection->node].key;
    if (selection->cell<0) return -1;
    if (challenges || focused) return model->edges[selection->cell].source;
    if (model->skill_mode) return model->skill_links[selection->cell].dependent;
    return model->pairs[model->cells[selection->cell].first_pair+selection->interaction].interaction;
}
static void select_record(const GD_Layout *layout, GraphSelection *selection, int record) {
    int i; *selection=(GraphSelection){-1,-1,0,-1,-1};
    for (i=0;i<layout->node_count;++i) if (layout->nodes[i].key==record) { selection->node=i; return; }
    if (record>=0) selection->linked=record;
}
static int focus_find(const GD_Model *model, const char *reference) {
    int i, found=-1;
    for (i=0;i<model->record_count;++i) if (!strcmp(model->records[i].reference,reference)) {
        if (found>=0) return -1;
        found=i;
    }
    return found;
}
int gd_graph_view(GD_Model *initial, int frames, const char *screenshot, GD_ViewLayout initial_layout, int other_pairs, int horizontal, const char *output_path, GD_ExplorerMode explorer_mode, const char *focus_from) {
    GD_Model *baseline = initial;
    GD_Draft *draft = initial->skill_mode ? gd_draft_create(initial) : NULL;
    GD_Model *model = draft ? draft->model : initial;
    GD_Routes *routes = draft && model->route_mode ? gd_analyze_routes(model) : NULL;
    int challenges = explorer_mode == GD_EXPLORE_CHALLENGES, focused = explorer_mode == GD_EXPLORE_RELATIONSHIPS;
    int witness = -1, remembered[3]={-1,-1,-1};
    GD_Focus focus;
    GD_DiagnosticBrowser diagnostics={0};
    int root=focus_from ? focus_find(model,focus_from):-1;
    char browse[129]="";
    SkillEditor editor = {-1,0,0,{0},{0}};
    char report_path[GD_MAX_PATH] = "";
    int finish_next = 0, close_seen = 0, timed_finish_seen = 0;
    GD_Layout *layout = initial->allocator.allocate(initial->allocator.context, sizeof(*layout));
    GraphSelection selection = {-1, -1, 0, -1, -1};
    Camera2D camera = {{0,0},{0,0},0,1};
    int frame = 0, result = 0, mode = initial_layout == GD_VIEW_COLUMNS ? GD_LAYOUT_COLUMNS : GD_LAYOUT_FORCE, sort = GD_SORT_NAME;
    int rank_mode = initial_layout == GD_VIEW_RANK;
    bool paused = false, overlay = other_pairs != 0;
    double next_check = 0;
    float accumulator = 0;
    int stale = 0, fit = 1;
    char failure[GD_MAX_PATH+512] = "";
    if (output_path) snprintf(report_path, sizeof(report_path), "%s", output_path);
    if (focus_from && root<0) {
        if (layout) initial->allocator.release(initial->allocator.context,layout);
        gd_routes_destroy(routes); gd_draft_destroy(draft);
        gd_error(initial,"view.root","--from must identify a unique loaded record",NULL,0); return 2;
    }
    if (focused && root<0 && model->record_count) { root=0; for (int i=0;i<model->record_count;++i) if (model->records[i].role==GD_SKILL) { root=i; break; } }
    gd_focus_reset(model,&focus,root);
    if ((initial->skill_mode && !draft) || (model->route_mode && (!routes || model->error_count || model->errors_omitted)) || !layout || !prepare_layout(model, layout, (GD_LayoutMode)mode, GD_SORT_NAME, rank_mode, horizontal, challenges, focused ? &focus:NULL)) {
        if (layout) initial->allocator.release(initial->allocator.context, layout);
        gd_routes_destroy(routes); gd_draft_destroy(draft);
        gd_error(initial, "view.memory", "Cannot allocate draft or prepare graph presentation", NULL, 0); return 2;
    }
    {
        int i;
        for (i = 0; i < model->cell_count; ++i) if (model->cells[i].active || other_pairs) { selection.cell = i; break; }
        if (selection.cell < 0 && layout->node_count) selection.node = 0;
    }
    if (focused) select_record(layout,&selection,focus.root);
    SetTraceLogCallback(graph_log); SetTraceLogLevel(LOG_WARNING); SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1440, 900, model->skill_mode ? "Game Design Workbench - skill and progression explorer" : "Game Design Workbench - matrix graph");
    if (!IsWindowReady()) { gd_routes_destroy(routes); gd_draft_destroy(draft); initial->allocator.release(initial->allocator.context, layout); gd_error(initial, "view.window", "Cannot initialize graph window", NULL, 0); return 2; }
    if (draft) SetExitKey(KEY_NULL);
    SetWindowMinSize(1000, 650); SetTargetFPS(60); rlImGuiSetup(true);
    igGetIO_Nil()->IniFilename = NULL; igGetIO_Nil()->LogFilename = NULL;
    igGetStyle()->FontScaleMain = 1.1f;
    for (;;) {
        int close_requested = WindowShouldClose();
        int finishing = finish_next || (close_requested && !close_seen) || (frames && frame >= frames && !timed_finish_seen);
        int width = GetScreenWidth(), height = GetScreenHeight(), split = width*60/100;
        Rectangle viewport = {12, 140, (float)split-24, (float)height-190};
        int refresh = 0, steps = 0, rearrange = 0, draft_changed = 0;
        int current_mode=focused ? GD_EXPLORE_RELATIONSHIPS:challenges;
        int requested_mode=current_mode, navigate=-1, focus_root=-1, focus_expand=-1;
        int selected_before=selected_record(model,layout,&selection,challenges,focused);
        close_seen = close_requested; finish_next = 0;
        if (finishing) {
            int report = 0;
            /* After failure, retry only on explicit finish/close, not every frame. */
            timed_finish_seen = 1;
            if (!draft) break;
            {
                int applied=editor.record>=0 ? gd_draft_rank(draft,editor.record,editor.rank_text):0;
                if (applied>0 && model->route_mode) { gd_routes_destroy(routes); gd_recompute_skills(model); routes=gd_analyze_routes(model); witness=-1; }
                if (applied<0) report=-1;
                else report=gd_draft_finish(draft,*report_path ? report_path:NULL);
            }
            if (report >= 0) { if (!result) result = report; break; }
            fprintf(stderr, "gd: %s\n", draft->message);
            if (frames && !draft->modified_count) { result = 2; break; }
            prepare_layout(model, layout, (GD_LayoutMode)mode, (GD_LayoutSort)sort, rank_mode, horizontal, challenges, focused ? &focus:NULL); fit = 1;
        }
        BeginDrawing(); ClearBackground(graph_background); rlImGuiBegin();
        if (GetTime() >= next_check) { if (gd_sources_changed(baseline)) stale = 1; next_check = GetTime()+1; }
        DrawText(focused ? "Focused relationships" : challenges ? "Challenge progression" : model->skill_mode ? rank_mode ? "Skill rank graph" : "Skill prerequisite graph" : "Matrix graph", 20, 18, 26, graph_text);
        DrawText(focused ? TextFormat("%d loaded records | %d drawn declarations | %d boundary declarations",layout->node_count,layout->edge_count,focus.boundary_count) : challenges ? TextFormat("%d challenges | %d drawn next links | All statuses | Gold: selected witness",layout->node_count,layout->edge_count) : model->skill_mode ? TextFormat("%d skills | %d prerequisite links | %d groups | All statuses", layout->node_count, model->skill_link_count, model->component_count) : TextFormat("%d groups (singletons included) | %d unplaced cases | Circle: noun | Square: verb", model->component_count, model->unplaced_cases), 20, 53, 16, graph_muted);
        DrawText(draft ? "Drag: layout | Right drag: pan | Wheel: zoom | F: fit | F5: reload | Ctrl+Z: undo" : "Drag node | Right drag: pan | Wheel: zoom | F: fit | F5: reload", 20, 82, 16, graph_muted);
        if (draft) DrawText(draft->modified_count ? TextFormat("MODIFIED IN MEMORY: %d records * | No viewer Markdown writes", draft->modified_count) : "Draft matches loaded disk baseline | No Markdown writes", 20, 110, 16, draft->modified_count ? graph_selected : graph_muted);
        igSetNextWindowPos((ImVec2_c){(float)split, 12}, ImGuiCond_Always, (ImVec2_c){0,0});
        igSetNextWindowSize((ImVec2_c){(float)(width-split-12), (float)(height-55)}, ImGuiCond_Always);
        igBegin("Graph controls and source", NULL, ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoSavedSettings);
        if (model->skill_mode) {
            if (model->route_mode) igCombo_Str("Explore",&requested_mode,"Skills\0Challenges\0Relationships\0",-1);
            else {
                int option=focused ? 1:0;
                if (igCombo_Str("Explore",&option,"Skills\0Relationships\0",-1)) requested_mode=option ? GD_EXPLORE_RELATIONSHIPS:GD_EXPLORE_SKILLS;
            }
            if (!focused) {
                if (!challenges) rearrange = igCombo_Str("Layout", &rank_mode, "Prerequisite depth\0Authored rank constraints\0", -1);
                if (igCombo_Str("Direction", &horizontal, "Top to bottom\0Left to right\0", -1)) rearrange = 1;
                igTextWrapped(challenges ? "Read-only routes. Bands show shortest entry-hop distance; cycles/backlinks remain. Not physical reachability or teaching order." : "Chain axis is locked during dragging and settling. Rank bands compress numeric gaps; same-rank prerequisites still precede dependents.");
            }
        } else if (focused) { if (igButton("Return to matrix graph",(ImVec2_c){0,0})) requested_mode=GD_EXPLORE_SKILLS; }
        else rearrange = igCombo_Str("Layout", &mode, "Force-directed\0Noun / verb columns\0", -1);
        if (focused) {
            igTextWrapped("Already-consumed declarations only; not every project relationship. Both incoming/outgoing links; authored arrow direction. No inferred impact, causality or learning.");
            if (focus.root>=0) igTextWrapped("Root: %s",model->records[focus.root].reference);
            if (focus.boundary_count) igTextWrapped("Boundary declarations remain outside visible scope or not uniquely loaded. Inspect source links; no placeholder identities invented.");
        }
        if (igCombo_Str("Sort / initial order", &sort, focused ? "Name\0Drawn declarations (descending)\0" : challenges ? "Name\0Drawn route links (descending)\0" : model->skill_mode ? "Name\0Declared links (descending)\0" : "Name\0Active partners (descending)\0", -1)) rearrange = 1;
        if (rearrange) {
            prepare_layout(model, layout, (GD_LayoutMode)mode, (GD_LayoutSort)sort, rank_mode, horizontal, challenges, focused ? &focus:NULL); selection.held = -1; accumulator = 0; fit = 1;
        }
        if (!focused && !challenges) { igCheckbox("Pause settling", &paused); igSameLine(0, -1); }
        else igTextUnformatted(focused ? "Stable neighborhood; drag for transient placement." : "Stable bands; drag for transient lateral placement.",NULL);
        if (igButton("Fit", (ImVec2_c){0,0})) fit = 1;
        igSameLine(0, -1);
        if (igButton("Rearrange", (ImVec2_c){0,0})) { prepare_layout(model, layout, (GD_LayoutMode)mode, (GD_LayoutSort)sort, rank_mode, horizontal, challenges, focused ? &focus:NULL); selection.held = -1; fit = 1; }
        if (!focused && !model->skill_mode) igCheckbox("Overlay other pairs (does not merge groups)", &overlay);
        refresh = igButton("Refresh sources", (ImVec2_c){0,0});
        if (stale) igTextUnformatted("STALE: disk sources changed or could not be checked.", NULL);
        if (*failure) igTextWrapped("%s", failure);
        if (draft) {
            igText("%d modified records | %d undo steps to disk sync point", draft->modified_count, draft->history_count);
            if (igButton("Undo last edit (Ctrl+Z)", (ImVec2_c){0,0})) draft_changed = undo_edit(draft, layout, &selection, &editor);
            igTextUnformatted("Report file (empty = stdout; new .json file only)", NULL); igSetNextItemWidth(-1);
            igInputText("##report-path", report_path, sizeof(report_path), 0, NULL, NULL);
            if (igButton("Finish and report changes", (ImVec2_c){0,0})) finish_next = 1;
            if (*draft->message) igTextWrapped("%s", draft->message);
        }
        if ((model->route_mode || focused) && igCollapsingHeader_TreeNodeFlags("Browse current mode",0)) {
            int i; igInputText("Filter names",browse,sizeof(browse),0,NULL,NULL);
            igBeginChild_Str("Mode records",(ImVec2_c){0,110},0,0);
            for (i=0;i<model->record_count;++i) if ((focused || model->records[i].role==(challenges ? GD_CHALLENGE:GD_SKILL)) && (!*browse || strstr(model->records[i].reference,browse))) {
                if (igButton(model->records[i].reference,(ImVec2_c){0,0})) navigate=i;
            }
            igEndChild();
        }
        if (model->route_mode) {
            igTextWrapped("Route opportunity analysis uses current skill draft; not evidence of learning. Challenges remain read-only.");
            if (routes) igText("%d checks | Route references %s | Requirements %s",routes->check_count,routes->routes_complete ? "complete":"incomplete",routes->requirements_complete ? "complete":"incomplete");
        }
        igSeparator();
        {
            int target=gd_diagnostics_panel(&diagnostics,model,model->findings,model->finding_count,model->findings_omitted,selected_before);
            if (target>=0) navigate=target;
        }
        igBeginChild_Str("Selected source", (ImVec2_c){0,0}, 0, 0);
        if (details(model, layout, &selection, draft, &editor, challenges, focused, routes, &witness, &navigate, &focus_root, &focus_expand)) draft_changed = 1;
        igEndChild(); igEnd();
        if (draft && !igGetIO_Nil()->WantTextInput && (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyPressed(KEY_Z)) {
            if (undo_edit(draft, layout, &selection, &editor)) draft_changed = 1;
        }
        if (focus_root>=0) { requested_mode=GD_EXPLORE_RELATIONSHIPS; navigate=focus_root; }
        if (model->skill_mode && (requested_mode!=current_mode || navigate>=0)) {
            int applied=editor.record>=0 ? gd_draft_rank(draft,editor.record,editor.rank_text):0;
            if (applied<0) { requested_mode=current_mode; navigate=focus_root=-1; }
            else if (applied>0) draft_changed=1;
            if (requested_mode!=GD_EXPLORE_RELATIONSHIPS && navigate>=0 && model->route_mode && (model->records[navigate].role==GD_SKILL || model->records[navigate].role==GD_CHALLENGE)) requested_mode=model->records[navigate].role==GD_CHALLENGE;
        }
        if (draft_changed || requested_mode!=current_mode || navigate>=0 || focus_expand>=0) {
            /* Prerequisite edits can remove/reorder selected edge. Recover via
               editor/node identity, never dereference the old edge index. */
            int selected=draft_changed ? (editor.record>=0 ? editor.record : selection.node>=0 ? layout->nodes[selection.node].key : selected_before) : selected_record(model,layout,&selection,challenges,focused);
            if (requested_mode!=current_mode) { remembered[current_mode]=selected; selected=remembered[requested_mode]; }
            if (navigate>=0) selected=navigate;
            focused=requested_mode==GD_EXPLORE_RELATIONSHIPS;
            challenges=requested_mode==GD_EXPLORE_CHALLENGES;
            if (focused && (focus_root>=0 || focus.root<0)) {
                int root=focus_root>=0 ? focus_root:selected>=0 ? selected:selected_before;
                gd_focus_reset(model,&focus,root); selected=root;
            }
            if (focused && focus_expand>=0) gd_focus_expand(model,&focus,focus_expand);
            if (draft_changed && model->route_mode) {
                gd_routes_destroy(routes); gd_recompute_skills(model); routes=gd_analyze_routes(model); witness=-1;
            }
            prepare_layout(model,layout,(GD_LayoutMode)mode,(GD_LayoutSort)sort,rank_mode,horizontal,challenges,focused ? &focus:NULL);
            select_record(layout,&selection,selected); editor.record=-1;
            accumulator=0; fit=1;
        }
        if (!igGetIO_Nil()->WantCaptureKeyboard && IsKeyPressed(KEY_F)) fit = 1;
        if (!igGetIO_Nil()->WantCaptureKeyboard && IsKeyPressed(KEY_F5)) refresh = 1;
        if (fit) { fit_camera(&camera, layout, viewport); fit = 0; }
        graph_input(layout, &selection, &camera, viewport, overlay);
        if (!focused && !challenges && !paused && layout->mode != GD_LAYOUT_COLUMNS) {
            accumulator += fminf(GetFrameTime(), 0.05f);
            while (accumulator >= 1.0f/60 && steps++ < 4) { gd_layout_step(layout, selection.held); accumulator -= 1.0f/60; }
        } else accumulator = 0;
        draw_graph(model, layout, &selection, camera, viewport, overlay, draft, challenges, focused, routes, focused ? -1:witness);
        DrawText(focused ? "Arrows: authored source -> target | Square: root | Click source, then expand | Scope is partial" : challenges ? "Arrows: authored next | Square: entry | Gold: witness | Prior opportunities, not learning verdicts" : model->skill_mode ? "Arrows: prerequisite -> dependent | Red: unknown/cyclic | Orange: rank conflict | Positions transient" : overlay ? "Solid: accepted + defined | Dashed: other pairs; clustering unchanged" : "Active pairs only; other cases remain available in edge details/overlay", 20, height-30, 16, graph_muted);
        rlImGuiEnd(); EndDrawing(); ++frame;
        if (screenshot && frame == frames && !gd_save_screenshot(screenshot)) { fputs("gd: graph screenshot export failed\n", stderr); result = 2; }
        if (refresh && draft && draft->modified_count) {
            snprintf(draft->message, sizeof(draft->message), "Refresh blocked: draft modified. Undo to disk sync point or finish/report first.");
            refresh = 0;
        }
        if (refresh) {
            GD_Model *fresh = gd_create(initial->allocator);
            GD_Draft *replacement = NULL;
            GD_Routes *replacement_routes = NULL;
            char reference[260] = "", noun[260] = "", verb[260] = "", root_reference[260]="";
            int fresh_root=-1;
            if (focus.root>=0) snprintf(root_reference,sizeof(root_reference),"%s",model->records[focus.root].reference);
            int selected=selected_record(model,layout,&selection,challenges,focused);
            if (selected>=0) snprintf(reference,sizeof(reference),"%s",model->records[selected].reference);
            if (!focused && !challenges && !model->skill_mode && selection.cell >= 0) {
                snprintf(noun, sizeof(noun), "%s", model->records[model->cells[selection.cell].noun].reference);
                snprintf(verb, sizeof(verb), "%s", model->records[model->cells[selection.cell].verb].reference);
            }
            int ready=fresh && gd_load(fresh, baseline->collections, baseline->collection_count) && (!draft || (replacement = gd_draft_create(fresh)) != NULL) &&
                (!fresh->route_mode || ((replacement_routes=gd_analyze_routes(replacement->model))!=NULL && !replacement->model->error_count && !replacement->model->errors_omitted));
            if (ready && focused && *root_reference) {
                fresh_root=focus_find(fresh,root_reference);
                if (fresh_root<0) {
                    gd_error(fresh,"view.root","Relationship root no longer uniquely loaded; leave Relationships or center another record before reload",NULL,0);
                    ready=0;
                }
            }
            if (ready) {
                GD_Model *old = baseline;
                int i;
                gd_routes_destroy(routes); routes=replacement_routes;
                gd_draft_destroy(draft); draft = replacement;
                baseline = fresh; model = draft ? draft->model : baseline;
                editor.record = -1;
                witness=-1; remembered[0]=remembered[1]=remembered[2]=-1;
                if (!focused && *root_reference) fresh_root=focus_find(model,root_reference);
                gd_focus_reset(model,&focus,fresh_root);
                prepare_layout(model, layout, (GD_LayoutMode)mode, (GD_LayoutSort)sort, rank_mode, horizontal, challenges, focused ? &focus:NULL);
                selection = (GraphSelection){-1,-1,0,-1,-1};
                select_record(layout,&selection,focus_find(model,reference));
                for (i = 0; i < model->cell_count; ++i) if (!strcmp(noun, model->records[model->cells[i].noun].reference) && !strcmp(verb, model->records[model->cells[i].verb].reference)) selection.cell = i;
                if (old != initial) gd_destroy(old);
                gd_diagnostics_clear(&diagnostics);
                stale = 0; failure[0] = 0; fit = 1; accumulator = 0;
            } else {
                const GD_Error *error=fresh && fresh->error_count ? &fresh->errors[0] : replacement && replacement->model->error_count ? &replacement->model->errors[0] : NULL;
                stale=1;
                if (error) snprintf(failure,sizeof(failure),"%s:%d: %s",error->path ? error->path:"(runtime)",error->line,error->message);
                else snprintf(failure,sizeof(failure),"Cannot prepare refresh snapshot");
                gd_diagnostics_rejected(&diagnostics,initial->allocator,fresh && fresh->error_count ? fresh:replacement ? replacement->model:fresh);
                gd_routes_destroy(replacement_routes); gd_draft_destroy(replacement); gd_destroy(fresh);
            }
        }
    }
    rlImGuiShutdown(); CloseWindow();
    gd_diagnostics_clear(&diagnostics);
    gd_routes_destroy(routes); gd_draft_destroy(draft);
    if (baseline != initial) gd_destroy(baseline);
    initial->allocator.release(initial->allocator.context, layout);
    return result;
}
