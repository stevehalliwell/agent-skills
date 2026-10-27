#include "gd_resource_graph.h"
#include "gd_layout.h"
#include "raylib.h"
#include <math.h>
#include <string.h>

static const Color background={18,24,34,255}, foreground={225,233,241,255}, muted={155,174,194,255};
static const Color role_color={193,155,228,255}, facet_color={108,171,230,255}, case_color={224,160,109,255};
static const Color selected_color={255,217,125,255};
static const char *node_label(const GD_ResourceGraph *graph, int index) {
    const GD_ResourceNode *node=&graph->nodes[index];
    if (node->kind==GD_RESOURCE_ROLE) return node->pool_reference;
    if (node->kind==GD_RESOURCE_CASE) return graph->rules->cases[node->case_index].name;
    if (node->pool_reference) return node->pool_reference;
    if (node->axis>=0) return graph->model->records[node->axis].reference;
    return "unresolved axis";
}
static const char *relation_name(GD_ResourceRelationKind kind) {
    switch (kind) {
        case GD_RESOURCE_REQUIREMENT: return "condition";
        case GD_RESOURCE_EXCLUSION: return "exclusion";
        case GD_RESOURCE_STATE_WRITE: return "state write";
        case GD_RESOURCE_QUANTITY_WRITE: return "quantity write";
        case GD_RESOURCE_ROLE_BINDING: return "role binding (not flow)";
        case GD_RESOURCE_PRESERVATION: return "preservation";
        case GD_RESOURCE_INVOCATION: return "conditional invocation";
        default: return "unknown";
    }
}
static int resource_layout(const GD_ResourceGraph *graph, GD_Layout *layout) {
    int i;
    memset(layout,0,sizeof(*layout));
    layout->node_count=graph->node_count; layout->edge_count=graph->relation_count; layout->group_count=graph->node_count ? 1:0;
    for (i=0;i<graph->node_count;++i) {
        layout->nodes[i].key=i; layout->nodes[i].label=node_label(graph,i);
        layout->nodes[i].group=0; layout->nodes[i].side=graph->nodes[i].kind==GD_RESOURCE_CASE ? 1:0;
    }
    for (i=0;i<graph->relation_count;++i) {
        layout->edges[i].a=graph->relations[i].facet; layout->edges[i].b=graph->relations[i].activation;
        layout->edges[i].key=i; layout->edges[i].active=1;
    }
    return gd_layout_arrange(layout,GD_LAYOUT_FORCE,GD_SORT_NAME);
}
static void fit(Camera2D *camera, const GD_Layout *layout, Rectangle viewport) {
    float left=0,right=1,top=0,bottom=1; int i;
    for (i=0;i<layout->node_count;++i) {
        float x=layout->nodes[i].x,y=layout->nodes[i].y;
        if (!i || x-110<left) left=x-110; if (!i || x+110>right) right=x+110;
        if (!i || y-45<top) top=y-45; if (!i || y+45>bottom) bottom=y+45;
    }
    camera->target=(Vector2){(left+right)/2,(top+bottom)/2};
    camera->offset=(Vector2){viewport.x+viewport.width/2,viewport.y+viewport.height/2};
    camera->zoom=fminf(viewport.width/(right-left+50),viewport.height/(bottom-top+50));
    if (camera->zoom>1.2f) camera->zoom=1.2f; if (camera->zoom<0.002f) camera->zoom=0.002f;
}
static void detail(const GD_ResourceGraph *graph, int selected, int x, int width) {
    const GD_ResourceNode *node; const GD_Record *source=NULL; int y=118;
    DrawText("Selection",x,y,20,foreground); y+=32;
    if (selected<0 || selected>=graph->node_count) { DrawText("Select a node",x,y,16,muted); return; }
    node=&graph->nodes[selected];
    BeginScissorMode(x,y,width,GetScreenHeight()-y-20);
    DrawText(node->kind==GD_RESOURCE_CASE ? "Rule case":node->kind==GD_RESOURCE_ROLE ? "Resource role":"Actor facet",x,y,17,selected_color); y+=27;
    DrawText(node_label(graph,selected),x,y,16,foreground); y+=25;
    if (node->kind==GD_RESOURCE_CASE) {
        const GD_RuleCase *item=&graph->rules->cases[node->case_index];
        source=&graph->model->records[graph->rules->sources[item->source].record];
        DrawText(TextFormat("case line: %d",item->line),x,y,15,muted); y+=23;
        DrawText(TextFormat("context: %.55s",item->context ? item->context:"unspecified"),x,y,15,muted); y+=23;
    } else if (node->kind==GD_RESOURCE_ROLE) {
        if (node->record>=0) source=&graph->model->records[node->record];
        DrawText(TextFormat("resolution: %s",gd_rule_resolution_name(node->resolution)),x,y,15,muted); y+=23;
    } else {
        const GD_RuleParticipant *actor=&graph->rules->participants[node->participant];
        source=&graph->model->records[graph->rules->sources[graph->rules->cases[node->case_index].source].record];
        DrawText(TextFormat("participant: %.40s",actor->name),x,y,15,muted); y+=23;
        DrawText(TextFormat("noun: %.48s",actor->noun ? actor->noun:"unspecified"),x,y,15,muted); y+=23;
        DrawText(TextFormat("axis: %.48s",node->axis>=0 ? graph->model->records[node->axis].reference:"unresolved"),x,y,15,muted); y+=23;
    }
    if (source) {
        DrawText(TextFormat("source: %.52s",source->reference),x,y,15,foreground); y+=23;
        DrawText(TextFormat("status: %.42s",source->status ? source->status:"unspecified"),x,y,15,muted); y+=23;
        DrawText(TextFormat("path: %.52s",source->source_path),x,y,14,muted); y+=29;
    }
    DrawText("Incident declarations:",x,y,16,foreground); y+=23;
    { int i,shown=0;
      for (i=0;i<graph->relation_count && y<GetScreenHeight()-30;++i) {
          const GD_ResourceRelation *edge=&graph->relations[i];
          if (edge->facet!=selected && edge->activation!=selected) continue;
          DrawText(TextFormat("%s | line %d",relation_name(edge->kind),edge->line),x,y,14,muted); y+=20; ++shown;
          if (edge->kind==GD_RESOURCE_QUANTITY_WRITE) {
              const GD_RuleAssignment *assignment=&graph->rules->assignments[edge->declaration];
              DrawText(TextFormat("  %.48s",assignment->expression),x,y,14,muted); y+=20;
              if (edge->delta_recognized && edge->delta.known) { DrawText(TextFormat("  signed delta: %g",edge->delta.number),x,y,14,muted); y+=20; }
          } else if (edge->kind==GD_RESOURCE_REQUIREMENT || edge->kind==GD_RESOURCE_EXCLUSION) {
              const GD_RuleParticipant *actor=&graph->rules->participants[edge->participant];
              const char *condition=edge->kind==GD_RESOURCE_REQUIREMENT ? actor->requires:actor->skip_if;
              DrawText(TextFormat("  %.48s",condition ? condition:"unspecified"),x,y,14,muted); y+=20;
          }
      }
      if (!shown) DrawText("None retained",x,y,14,muted);
    }
    EndScissorMode();
}
int gd_resource_view(GD_Model *model, GD_ResourceGraph *graph, int frames, const char *screenshot) {
    GD_Layout *layout=model->allocator.allocate(model->allocator.context,sizeof(*layout));
    Camera2D camera={0}; int frame=0, selected=-1,held=-1,stale=0,result=0,paused=1,i; double next_check=0;
    if (!layout || !resource_layout(graph,layout)) {
        if (layout) model->allocator.release(model->allocator.context,layout);
        gd_error(model,"resources.layout","Cannot allocate or arrange resource preview",NULL,0); return 2;
    }
    SetConfigFlags(FLAG_WINDOW_RESIZABLE); InitWindow(1440,900,"Resource graph preview"); SetTargetFPS(60);
    for (i=0;i<120;++i) gd_layout_step(layout,-1);
    fit(&camera,layout,(Rectangle){0,100,1060,755});
    while (!WindowShouldClose() && (!frames || frame<frames)) {
        int width=GetScreenWidth(),height=GetScreenHeight(),split=width>700 ? width-380:width*2/3,i;
        Rectangle viewport={0,100,(float)split,(float)(height-145)};
        Vector2 mouse=GetMousePosition();
        camera.offset=(Vector2){viewport.x+viewport.width/2,viewport.y+viewport.height/2};
        if (GetTime()>=next_check) { if (gd_sources_changed(model)) stale=1; next_check=GetTime()+1; }
        if (IsKeyPressed(KEY_F)) fit(&camera,layout,viewport);
        if (IsKeyPressed(KEY_SPACE)) paused=!paused;
        if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) held=-1;
        if (CheckCollisionPointRec(mouse,viewport)) {
            Vector2 world=GetScreenToWorld2D(mouse,camera);
            float wheel=GetMouseWheelMove();
            if (wheel) { Vector2 after; camera.zoom*=powf(1.15f,wheel); if(camera.zoom<0.002f)camera.zoom=0.002f; if(camera.zoom>4)camera.zoom=4; after=GetScreenToWorld2D(mouse,camera); camera.target.x+=world.x-after.x; camera.target.y+=world.y-after.y; }
            if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) || IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) { Vector2 delta=GetMouseDelta(); camera.target.x-=delta.x/camera.zoom; camera.target.y-=delta.y/camera.zoom; }
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) for (i=layout->node_count-1;i>=0;--i) {
                const GD_LayoutNode *n=&layout->nodes[i];
                if (CheckCollisionPointRec(world,(Rectangle){n->x-90,n->y-24,180,48})) { selected=n->key; held=i; break; }
            }
            if (held>=0 && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) gd_layout_move_node(layout,held,world.x,world.y);
        }
        if (!paused) gd_layout_step(layout,held);
        BeginDrawing(); ClearBackground(background);
        DrawText("Resource graph preview",18,14,26,foreground);
        DrawText("Incomplete: routes and composed outcomes are not implemented. Lines are declarations, not proof of flow.",18,49,16,selected_color);
        DrawText(TextFormat("%d nodes | %d relations | %d unresolved facets | %d unexamined calls%s",graph->node_count,graph->relation_count,graph->unresolved_facets,graph->composition_unexamined,graph->limited ? " | GRAPH LIMIT REACHED":""),18,74,15,muted);
        BeginScissorMode((int)viewport.x,(int)viewport.y,(int)viewport.width,(int)viewport.height); BeginMode2D(camera);
        for (i=0;i<layout->edge_count;++i) {
            const GD_LayoutEdge *edge=&layout->edges[i]; const GD_ResourceRelation *rel=&graph->relations[edge->key];
            const GD_LayoutNode *a=&layout->nodes[edge->a],*b=&layout->nodes[edge->b];
            Color color=rel->kind==GD_RESOURCE_ROLE_BINDING ? role_color : rel->kind==GD_RESOURCE_INVOCATION ? selected_color :
                rel->kind==GD_RESOURCE_EXCLUSION ? (Color){230,125,125,255} : rel->kind==GD_RESOURCE_REQUIREMENT ? (Color){81,194,178,255} :
                rel->kind==GD_RESOURCE_PRESERVATION ? (Color){180,201,119,255} : facet_color;
            DrawLineEx((Vector2){a->x,a->y},(Vector2){b->x,b->y},1.5f,color);
        }
        for (i=0;i<layout->node_count;++i) {
            const GD_LayoutNode *n=&layout->nodes[i]; const GD_ResourceNode *item=&graph->nodes[n->key];
            Color color=item->kind==GD_RESOURCE_CASE ? case_color:item->kind==GD_RESOURCE_ROLE ? role_color:facet_color;
            DrawRectangleRounded((Rectangle){n->x-90,n->y-24,180,48},0.15f,5,(Color){28,38,50,255});
            DrawRectangleRoundedLinesEx((Rectangle){n->x-90,n->y-24,180,48},0.15f,5,selected==n->key ? 3:1.5f,selected==n->key ? selected_color:color);
            DrawText(item->kind==GD_RESOURCE_CASE ? "case":item->kind==GD_RESOURCE_ROLE ? "resource role":"facet",(int)n->x-83,(int)n->y-19,11,color);
            DrawText(TextFormat("%.23s",node_label(graph,n->key)),(int)n->x-83,(int)n->y-3,14,foreground);
        }
        EndMode2D(); EndScissorMode();
        DrawRectangle(split,100,width-split,height-100,(Color){29,38,51,255}); detail(graph,selected,split+14,width-split-28);
        if (stale) DrawText("STALE: sources changed. Close and reopen preview.",18,height-38,15,selected_color);
        else DrawText("Drag nodes | Right drag: pan | Wheel: zoom | F: fit | Space: settle/pause",18,height-38,15,muted);
        DrawText("Edges: teal condition | red exclusion | blue write | green preservation | purple role | gold call",18,height-20,13,muted);
        EndDrawing(); ++frame;
        if (screenshot && frame==frames && !gd_save_screenshot(screenshot)) { gd_error(model,"view.screenshot","Resource preview screenshot failed",screenshot,0); result=2; }
    }
    CloseWindow(); model->allocator.release(model->allocator.context,layout); return result;
}
