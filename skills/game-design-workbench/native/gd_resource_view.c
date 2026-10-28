#include "gd_resource_graph.h"
#include "gd_layout.h"
#include "gd_graph_analysis.h"
#include "gd_graph_analysis_draw.h"
#include "gd_graph_overview_draw.h"
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
        layout->edges[i].key=i; layout->edges[i].active=1; layout->edges[i].relation=GD_RELATION_RESOURCE_BASE+graph->relations[i].kind;
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
static int shared_choice(const GD_GraphAnalysis *analysis, int selected, int *peer, int next) {
    int current=gd_graph_match(analysis,selected,*peer),i,start=current>=0 ? current+1:0;
    if (current>=0 && !next) return current;
    for (i=0;i<analysis->pair_count;++i) {
        int index=(start+i)%analysis->pair_count; const GD_GraphMatch *match=&analysis->pairs[index];
        if (match->a!=selected && match->b!=selected) continue;
        *peer=match->a==selected ? match->b:match->a; return index;
    }
    *peer=-1; return -1;
}
static const GD_Record *relation_source(const GD_ResourceGraph *graph, int index) {
    const GD_ResourceRelation *relation=&graph->relations[index];
    int c=graph->nodes[relation->activation].kind==GD_RESOURCE_CASE ? graph->nodes[relation->activation].case_index:-1,k;
    if (c<0 && relation->participant>=0) for (k=0;k<graph->rules->case_count;++k) {
        const GD_RuleCase *item=&graph->rules->cases[k];
        if (relation->participant>=item->first_participant && relation->participant<item->first_participant+item->participant_count) { c=k; break; }
    }
    return c>=0 ? &graph->model->records[graph->rules->sources[graph->rules->cases[c].source].record]:NULL;
}
static int path_sources(const GD_ResourceGraph *graph, const GD_Layout *layout, const int *arcs, int retained, int x, int y) {
    int i;
    for (i=(retained<4 ? retained:4)-1;i>=0;--i) {
        int index=layout->edges[arcs[i]/2].key; const GD_ResourceRelation *relation=&graph->relations[index];
        const GD_Record *source=relation_source(graph,index);
        DrawText(TextFormat("%s | line %d",relation_name(relation->kind),relation->line),x,y,13,muted); y+=18;
        if (source) { DrawText(TextFormat("%.43s",source->reference),x,y,13,foreground); y+=18; }
    }
    return y;
}
static void overview_details(const GD_ResourceGraph *graph, const GD_Layout *layout, GD_GraphAnalysis *analysis,
                             GD_OverviewControls *controls, int x) {
    const GD_GraphOverview *overview=gd_graph_overview(analysis,controls,1); int i,count=0,arcs[4],types[7]={0},y=390;
    DrawText(controls->enabled ? "[x] Incidence overview (click)":"[ ] Incidence overview (click)",x,114,16,foreground);
    if (!overview) return;
    DrawText("Row + (click)",x,146,14,foreground); DrawText("Column + (click)",x+160,146,14,foreground);
    DrawText("Next occupied cell (click)",x,178,14,foreground); DrawText("Clear cell (click)",x,210,14,foreground);
    DrawText("Symmetric incidence, NOT flow. Log brightness:",x,244,13,muted);
    DrawText("projected link rows, not quantities/strength.",x,264,13,muted);
    DrawText("Index bins, not clusters. Diagonal is not",x,284,13,muted);
    DrawText("necessarily a self-link. Gold: contributors.",x,304,13,muted);
    DrawText(TextFormat("%d identities / %d bins / %d input rows",overview->node_count,overview->bins,overview->link_count),x,330,13,muted);
    if (controls->row<0 || controls->column<0 || !overview->bins) return;
    DrawText(TextFormat("Cell %d,%d: %d rows | arrows: move cell",controls->row,controls->column,overview->selected_count),x,352,13,foreground);
    DrawText(TextFormat("Row ordinals %d..%d | column %d..%d",overview->start[controls->row],overview->start[controls->row+1]-1,overview->start[controls->column],overview->start[controls->column+1]-1),x,372,12,muted);
    for (i=0;i<analysis->edge_count;++i) if (overview->selected_edges[i]) {
        int kind=graph->relations[layout->edges[i].key].kind;
        if (kind>=0 && kind<7) ++types[kind];
        if (count<4) arcs[count++]=2*i;
    }
    for (i=0;i<7;++i) if (types[i]) { DrawText(TextFormat("%s: %d",relation_name((GD_ResourceRelationKind)i),types[i]),x,y,12,muted); y+=18; }
    DrawText("Up to four contributing source examples:",x,y+4,12,muted); path_sources(graph,layout,arcs,count,x,y+24);
}
static const GD_GraphShapes *resource_shapes(const GD_ResourceGraph *graph, GD_GraphAnalysis *analysis, int enabled) {
    int roles[GD_MAX_RECORDS],i;
    if (!enabled) return NULL;
    for (i=0;i<analysis->node_count;++i) roles[i]=graph->nodes[i].kind;
    return gd_graph_shapes(analysis,roles,GD_GRAPH_ANALYSIS_WORK);
}
static void shape_details(const GD_ResourceGraph *graph, const GD_GraphShapes *shapes, int enabled, int selected, int x) {
    int i,shown=0,y=290;
    DrawText(enabled ? "[x] Structural shapes (click)":"[ ] Structural shapes (click)",x,114,16,foreground);
    DrawText("Two exact typed endpoint-neighborhood rounds.",x,146,13,muted);
    DrawText("Next repeated family (click)",x,178,14,foreground);
    DrawText("Transient IDs; NOT identity or equivalence.",x,210,13,muted);
    DrawText("Shown incidences, NOT resource flow.",x,232,13,muted);
    if (!shapes) return;
    if (!shapes->complete) { DrawText("Unavailable: shape work limit.",x,256,14,muted); return; }
    DrawText(TextFormat("%d families | gold outline: selected family",shapes->family_count),x,256,13,muted);
    if (selected<0 || selected>=shapes->node_count) return;
    DrawText(TextFormat("shape %d / %d members",shapes->colors[selected]+1,shapes->size[shapes->colors[selected]]),x,y,14,foreground); y+=24;
    for (i=0;i<shapes->node_count && shown<12;++i) if (shapes->colors[i]==shapes->colors[selected]) {
        DrawText(TextFormat("%.45s",node_label(graph,i)),x,y,13,foreground); y+=18; ++shown;
    }
    if (shapes->size[shapes->colors[selected]]>shown) { DrawText(TextFormat("%d more members",shapes->size[shapes->colors[selected]]-shown),x,y,13,muted); y+=20; }
    DrawText("Missing data can change families; rarity is",x,y+12,13,muted);
    DrawText("not a design defect. Source roles stay intact.",x,y+32,13,muted);
}
static void next_reach_kind(const GD_GraphAnalysis *analysis, GD_ReachControls *controls) {
    int i,first=-1,next=-1;
    for (i=0;i<analysis->edge_count;++i) {
        const GD_LayoutEdge *edge=&analysis->edges[i]; int kind=edge->relation;
        if (!analysis->shown[edge->a] || !analysis->shown[edge->b] || (!edge->active && !analysis->include_inactive)) continue;
        if (first<0 || kind<first) first=kind;
        if (kind>controls->relation && (next<0 || kind<next)) next=kind;
    }
    controls->relation=next>=0 ? next:first;
}
static void reach_details(const GD_ResourceGraph *graph, const GD_Layout *layout, GD_GraphAnalysis *analysis,
                          GD_ReachControls *controls, int selected, int x) {
    const GD_GraphClosure *closure=gd_graph_reachability_prepare(analysis,controls,1);
    const GD_GraphComparison *comparison=closure ? gd_graph_closure_choose_into(analysis,&analysis->reachability,selected,&controls->peer):NULL;
    int i,j,y=350;
    DrawText(controls->enabled ? "[x] Typed multi-hop (click)":"[ ] Typed multi-hop (click)",x,114,16,foreground);
    if (!controls->enabled) return;
    DrawText(TextFormat("Kind: %.36s (click)",controls->relation>=0 ? gd_graph_relation_name(controls->relation):"none"),x,146,13,foreground);
    DrawText(TextFormat("B: %.24s | %d peers (click)",comparison && comparison->b>=0 ? node_label(graph,comparison->b):"none",closure ? closure->peer_count:0),x,178,13,foreground);
    DrawText("Same-kind incidence connections, NOT flow.",x,211,14,muted);
    DrawText("T: other origins reaching this identity.",x,233,14,muted);
    DrawText("Purple shared | blue A-only | amber B-only",x,255,13,muted);
    if (!closure) { DrawText("No shown relation kind.",x,278,14,muted); return; }
    if (!comparison) { DrawText("Select node with completed local traversal.",x,278,14,muted); return; }
    DrawText(TextFormat("A: %.22s | %d targets%s",node_label(graph,selected),closure->count[selected],closure->complete ? "":" | partial"),x,278,13,muted);
    DrawText("Witness target (click to cycle)",x,302,14,foreground);
    if (controls->target<0 || controls->target>=layout->node_count || !comparison->nodes[controls->target]) {
        controls->target=-1; for (i=0;i<layout->node_count;++i) if (comparison->nodes[i]) { controls->target=i; break; }
    }
    if (controls->target>=0) DrawText(TextFormat("%.45s",node_label(graph,controls->target)),x,324,13,foreground);
    for (j=0;j<2 && controls->target>=0;++j) {
        int arcs[64],length=gd_graph_closure_path(analysis,closure,j,controls->target,arcs,64),retained=length<64 ? length:64;
        if (length<0) continue;
        DrawText(TextFormat("%c: %d links; last %d shown",j ? 'B':'A',length,retained<4 ? retained:4),x,y,13,foreground); y+=20;
        y=path_sources(graph,layout,arcs,retained,x,y)+12;
    }
    DrawText("Shown links only; not complete source semantics.",x,y,12,muted);
}
static void path_details(const GD_ResourceGraph *graph, const GD_Layout *layout, GD_GraphAnalysis *analysis,
                         const GD_GraphPaths *paths, const GD_PathControls *controls, int selected, int x) {
    const GD_PathSelection *support; int y=controls->compare ? 280:248;
    DrawText(controls->enabled ? "[x] Mandatory connections (click)":"[ ] Mandatory connections (click)",x,114,14,foreground);
    if (!paths) { DrawText("Enable, then choose analysis starts.",x,148,14,muted); return; }
    DrawRectangleLines(x,144,320,25,muted); DrawText("Add selection as analysis start",x+5,150,14,foreground);
    DrawRectangleLines(x,176,240,25,muted); DrawText("Clear analysis starts",x+5,182,14,foreground);
    DrawText(controls->compare ? "[x] Compare support / bypass (click)":"[ ] Compare support / bypass (click)",x,212,14,foreground);
    if (controls->compare) DrawText("Next support-path candidate (click)",x,244,14,foreground);
    DrawText(TextFormat("Configured %d | shown starts %d",controls->count,paths->root_count),x,y,14,muted); y+=22;
    DrawText("Virtual start; inspection does not re-root.",x,y,13,muted); y+=20;
    DrawText("D: necessary connections, NOT resource flow.",x,y,13,muted); y+=20;
    DrawText("D-: unreachable | D?: unavailable",x,y,13,muted); y+=20;
    DrawText("Pink: support | cyan: bypass | !: mandatory",x,y,13,muted); y+=20;
    DrawText("White triangle: chosen analysis start",x,y,13,muted); y+=20;
    if (!paths->root_count) { DrawText("Unavailable: no shown analysis starts.",x,y,14,selected_color); return; }
    if (!paths->complete) { DrawText("Work limit: intermediate results NOT proofs.",x,y,13,selected_color); return; }
    DrawText(TextFormat("%d / %d shown nodes reachable",paths->reachable_count,analysis->shown_nodes),x,y,14,muted); y+=23;
    DrawText("Shown incidence only; no composed outcomes.",x,y,13,muted); y+=23;
    support=gd_graph_path_selection(analysis,selected);
    if (!support) { DrawText("Selection has no path from starts.",x,y,14,muted); return; }
    DrawText(TextFormat("Selected D%d | %d mandatory ancestors",paths->dominated[selected],support->ancestor_count),x,y,14,foreground); y+=23;
    DrawText(TextFormat("Support: %d links; last %d shown below",support->length,support->retained<4 ? support->retained:4),x,y,13,muted); y+=23;
    y=path_sources(graph,layout,support->arcs,support->retained,x,y);
    if (controls->compare) {
        const GD_PathBypass *bypass=gd_graph_path_bypass(analysis,selected,-1,GD_GRAPH_ANALYSIS_WORK);
        if (!bypass) { DrawText("No nonmandatory node on this support path.",x,y,13,muted); DrawText("Alternatives elsewhere are not ruled out.",x,y+18,13,muted); return; }
        if (!bypass->complete || !bypass->found) { DrawText("Bypass witness unavailable; no partial proof.",x,y,13,muted); return; }
        DrawText(TextFormat("C (avoided): %.28s",node_label(graph,bypass->candidate)),x,y,13,gd_bypass_color); y+=20;
        DrawText(TextFormat("F (fork): %.29s",bypass->fork>=0 ? node_label(graph,bypass->fork):"virtual root (separate starts)"),x,y,13,gd_bypass_color); y+=20;
        DrawText(TextFormat("R (rejoin): %.27s",node_label(graph,bypass->rejoin)),x,y,13,gd_bypass_color); y+=20;
        DrawText(TextFormat("Bypass: %d links; last %d shown",bypass->length,bypass->retained<4 ? bypass->retained:4),x,y,13,muted); y+=20;
        path_sources(graph,layout,bypass->arcs,bypass->retained,x,y);
    }
}
static void shared_details(const GD_ResourceGraph *graph, const GD_Layout *layout, const GD_GraphAnalysis *analysis, const GD_GraphComparison *comparison, int selected, int pair, int x) {
    int i,y=292;
    DrawText(TextFormat("Shown: %d nodes / %d links",analysis->shown_nodes,analysis->shown_edges),x,198,14,muted);
    if (selected<0) { DrawText("Select a node for shared neighbors",x,220,14,muted); return; }
    DrawText(TextFormat("%d%s related | %d other neighbors",analysis->matched[selected],analysis->complete ? "":"+ retained",analysis->neighbors[selected]),x,220,14,muted);
    DrawText("Next retained match",x+6,249,14,foreground);
    if (pair>=0) {
        const GD_GraphMatch *match=&analysis->pairs[pair];
        int other=match->a==selected ? match->b:match->a;
        DrawText(TextFormat("%.31s | %d%s shared",node_label(graph,other),match->shared,analysis->complete ? "":"+"),x,274,14,foreground);
        for (i=0;i<2;++i) {
            const GD_GraphFeature *feature=&analysis->features[i ? match->first_b:match->first_a];
            const GD_ResourceRelation *relation=&graph->relations[layout->edges[feature->edge].key];
            const GD_Record *record=relation_source(graph,layout->edges[feature->edge].key);
            DrawText(TextFormat("%s | line %d",relation_name(relation->kind),relation->line),x,y,13,muted); y+=18;
            if (record) {
                DrawText(TextFormat("%.43s",record->reference),x,y,13,muted); y+=18;
            }
        }
    } else DrawText(analysis->complete ? "No shared-neighbor match":"No retained match; result is partial",x,274,14,muted);
    DrawText(analysis->complete ? (comparison ? "S: global matches | pair branches: one hop":"S: matches | thick purple: example links"):
        comparison ? "PARTIAL discovery | exact one-hop branches":"PARTIAL: counts are lower bounds",x,372,13,muted);
    DrawText("Incident role is not resource flow.",x,390,13,muted);
    if (comparison) {
        DrawText(TextFormat("A: %.18s | B: %.18s",node_label(graph,comparison->a),node_label(graph,comparison->b)),x,412,13,foreground);
        DrawText(TextFormat("Shared %d | A only %d | B only %d",comparison->shared,comparison->a_only,comparison->b_only),x,430,13,muted);
        DrawText(TextFormat("Shared identity, other role: %d",comparison->other_role),x,448,13,muted);
        DrawText("Purple shared | blue A | amber B",x,466,13,muted);
        DrawText("Gray other role | white mixed links",x,484,13,muted);
    }
}
static void detail(const GD_ResourceGraph *graph, int selected, int x, int width, int y) {
    const GD_ResourceNode *node; const GD_Record *source=NULL;
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
    GD_GraphAnalysis *analysis=model->allocator.allocate(model->allocator.context,sizeof(*analysis));
    Camera2D camera={0}; int frame=0, selected=graph->node_count ? 0:-1,held=-1,stale=0,result=0,paused=1,i;
    int shared=1,branches=0,peer=-1,pair=-1; double next_check=0;
    GD_GraphLens shared_lens=GD_GRAPH_ANY;
    GD_PathControls path_controls={0}; int path_tab=0;
    GD_ReachControls reach_controls={0,-1,0,-1,-1}; int shape_enabled=0;
    GD_OverviewControls overview_controls={0,-1,-1};
    if (!layout || !analysis || !resource_layout(graph,layout)) {
        if (analysis) model->allocator.release(model->allocator.context,analysis);
        if (layout) model->allocator.release(model->allocator.context,layout);
        gd_error(model,"resources.layout","Cannot allocate or arrange resource preview",NULL,0); return 2;
    }
    memset(analysis,0,sizeof(*analysis)); gd_graph_analyze(layout,NULL,1,shared_lens,analysis);
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
        if (gd_graph_overview_hit(&overview_controls,viewport,mouse)) held=-1;
        if (CheckCollisionPointRec(mouse,viewport) && !gd_graph_overview_hit(&overview_controls,viewport,mouse)) {
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
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse,(Rectangle){(float)split+14,72,100,25})) path_tab=0;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse,(Rectangle){(float)split+124,72,70,25})) path_tab=1;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse,(Rectangle){(float)split+204,72,140,25})) path_tab=path_tab>=2 && path_tab<5 ? path_tab+1:2;
        if (path_tab==5) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse,(Rectangle){(float)split+14,110,340,25})) overview_controls.enabled=!overview_controls.enabled;
            if (overview_controls.enabled) {
                const GD_GraphOverview *overview=gd_graph_overview(analysis,&overview_controls,1);
                int row=overview_controls.row<0 ? 0:overview_controls.row,column=overview_controls.column<0 ? 0:overview_controls.column,changed=0;
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    if (overview->bins && CheckCollisionPointRec(mouse,(Rectangle){(float)split+14,140,150,25})) { row=(row+1)%overview->bins; changed=1; }
                    if (overview->bins && CheckCollisionPointRec(mouse,(Rectangle){(float)split+174,140,170,25})) { column=(column+1)%overview->bins; changed=1; }
                    if (overview->bins && CheckCollisionPointRec(mouse,(Rectangle){(float)split+14,172,340,25})) {
                        int cells=overview->bins*overview->bins,start=overview_controls.row<0 ? -1:row*overview->bins+column;
                        for (i=1;i<=cells;++i) { int next=(start+i)%cells; if (overview->counts[next]) { row=next/overview->bins; column=next%overview->bins; changed=1; break; } }
                    }
                    if (CheckCollisionPointRec(mouse,(Rectangle){(float)split+14,204,340,25})) { row=column=-1; changed=1; }
                }
                if (overview->bins && (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_DOWN))) {
                    column+=IsKeyPressed(KEY_RIGHT)-IsKeyPressed(KEY_LEFT); row+=IsKeyPressed(KEY_DOWN)-IsKeyPressed(KEY_UP);
                    if (column<0) column=0; if (row<0) row=0; if (column>=overview->bins) column=overview->bins-1; if (row>=overview->bins) row=overview->bins-1; changed=1;
                }
                if (changed) { overview_controls.row=row; overview_controls.column=column; gd_graph_overview_select(analysis,row,column); }
            }
        }
        if (path_tab==4 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (CheckCollisionPointRec(mouse,(Rectangle){(float)split+14,110,340,25})) shape_enabled=!shape_enabled;
            if (shape_enabled && CheckCollisionPointRec(mouse,(Rectangle){(float)split+14,172,340,25})) {
                const GD_GraphShapes *shapes=resource_shapes(graph,analysis,1); int family=selected>=0 ? shapes->colors[selected]:-1;
                if (shapes->complete) for (i=0;i<shapes->family_count;++i) {
                    int next=(family+1+i)%shapes->family_count;
                    if (shapes->size[next]>1) { selected=shapes->representative[next]; break; }
                }
            }
        }
        if (path_tab==3 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (CheckCollisionPointRec(mouse,(Rectangle){(float)split+14,110,340,25})) reach_controls.enabled=!reach_controls.enabled;
            if (reach_controls.enabled && CheckCollisionPointRec(mouse,(Rectangle){(float)split+14,140,340,25})) next_reach_kind(analysis,&reach_controls);
            if (reach_controls.enabled && CheckCollisionPointRec(mouse,(Rectangle){(float)split+14,172,340,25})) {
                const GD_GraphClosure *closure=gd_graph_reachability_prepare(analysis,&reach_controls,1); int i;
                if (closure && gd_graph_closure_choose_into(analysis,&analysis->reachability,selected,&reach_controls.peer) && closure->peer_count) {
                    for (i=0;i<closure->peer_count && closure->peers[i]!=reach_controls.peer;++i) {}
                    reach_controls.peer=closure->peers[(i+1)%closure->peer_count];
                }
            }
            if (reach_controls.enabled && CheckCollisionPointRec(mouse,(Rectangle){(float)split+14,296,340,25})) {
                const GD_GraphClosure *closure=gd_graph_reachability_prepare(analysis,&reach_controls,1);
                const GD_GraphComparison *comparison=closure ? gd_graph_closure_choose_into(analysis,&analysis->reachability,selected,&reach_controls.peer):NULL;
                if (comparison) for (i=0;i<layout->node_count;++i) {
                    int node=(reach_controls.target+1+i)%layout->node_count;
                    if (comparison->nodes[node]) { reach_controls.target=node; break; }
                }
            }
        }
        if (path_tab==1 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (CheckCollisionPointRec(mouse,(Rectangle){(float)split+14,110,340,25})) path_controls.enabled=!path_controls.enabled;
            if (path_controls.enabled && selected>=0 && CheckCollisionPointRec(mouse,(Rectangle){(float)split+14,144,320,25})) gd_graph_path_start(&path_controls,selected);
            if (path_controls.enabled && CheckCollisionPointRec(mouse,(Rectangle){(float)split+14,176,240,25})) path_controls.count=0;
            if (path_controls.enabled && CheckCollisionPointRec(mouse,(Rectangle){(float)split+14,208,340,25})) path_controls.compare=!path_controls.compare;
            if (path_controls.enabled && path_controls.compare && CheckCollisionPointRec(mouse,(Rectangle){(float)split+14,240,340,25})) {
                const GD_PathSelection *support=gd_graph_path_selection(analysis,selected); int i;
                if (support && support->candidate_count) {
                    for (i=0;i<support->candidate_count;++i) if (support->candidates[i]==support->candidate) break;
                    analysis->paths.selection.candidate=support->candidates[(i+1)%support->candidate_count];
                }
            }
        }
        if (!path_tab && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse,(Rectangle){(float)split+14,110,340,25})) shared=!shared;
        if (!path_tab && shared && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse,(Rectangle){(float)split+14,136,340,25})) shared_lens=(GD_GraphLens)((shared_lens+1)%GD_GRAPH_LENS_COUNT);
        if (!path_tab && shared && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse,(Rectangle){(float)split+14,162,340,25})) branches=!branches;
        if (gd_graph_analyze(layout,NULL,1,shared_lens,analysis)) peer=-1;
        if (!frame && analysis->pair_count) selected=analysis->pairs[0].a;
        pair=shared_choice(analysis,selected,&peer,!path_tab && shared && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse,(Rectangle){(float)split+14,242,190,25}));
        const GD_GraphComparison *comparison=shared && branches && pair>=0 ? gd_graph_compare(analysis,selected,peer):NULL;
        const GD_GraphPaths *paths=gd_graph_paths_prepare(analysis,&path_controls,NULL,1);
        const GD_PathSelection *path_selection=paths ? gd_graph_path_selection(analysis,selected):NULL;
        const GD_PathBypass *bypass=paths && path_controls.compare ? gd_graph_path_bypass(analysis,selected,-1,GD_GRAPH_ANALYSIS_WORK):NULL;
        const GD_GraphClosure *reachability=gd_graph_reachability_prepare(analysis,&reach_controls,1);
        const GD_GraphComparison *reach_pair=reachability ? gd_graph_closure_choose_into(analysis,&analysis->reachability,selected,&reach_controls.peer):NULL;
        const GD_GraphShapes *shapes=resource_shapes(graph,analysis,shape_enabled);
        const GD_GraphOverview *overview=gd_graph_overview(analysis,&overview_controls,1);
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
            if (overview && overview->selected_edges[i]) DrawLineEx((Vector2){a->x,a->y},(Vector2){b->x,b->y},21,selected_color);
            if (reach_pair && reach_pair->edges[i]) DrawLineEx((Vector2){a->x,a->y},(Vector2){b->x,b->y},17,gd_graph_branch_color(reach_pair->edges[i]));
            if (bypass && bypass->complete && bypass->found && bypass->edges[i]) DrawLineEx((Vector2){a->x,a->y},(Vector2){b->x,b->y},15,gd_bypass_color);
            if (path_selection && path_selection->edges[i]) DrawLineEx((Vector2){a->x,a->y},(Vector2){b->x,b->y},11,gd_path_color);
            if (comparison && comparison->edges[i]) DrawLineEx((Vector2){a->x,a->y},(Vector2){b->x,b->y},7,gd_graph_branch_color(comparison->edges[i]));
            else if (!comparison && shared && pair>=0 && (i==analysis->features[analysis->pairs[pair].first_a].edge || i==analysis->features[analysis->pairs[pair].first_b].edge))
                DrawLineEx((Vector2){a->x,a->y},(Vector2){b->x,b->y},7,role_color);
            DrawLineEx((Vector2){a->x,a->y},(Vector2){b->x,b->y},1.5f,color);
        }
        for (i=0;i<layout->node_count;++i) {
            const GD_LayoutNode *n=&layout->nodes[i]; const GD_ResourceNode *item=&graph->nodes[n->key];
            Color color=item->kind==GD_RESOURCE_CASE ? case_color:item->kind==GD_RESOURCE_ROLE ? role_color:facet_color;
            DrawRectangleRounded((Rectangle){n->x-90,n->y-24,180,48},0.15f,5,(Color){28,38,50,255});
            DrawRectangleRoundedLinesEx((Rectangle){n->x-90,n->y-24,180,48},0.15f,5,selected==n->key ? 3:1.5f,selected==n->key ? selected_color:color);
            DrawText(item->kind==GD_RESOURCE_CASE ? "case":item->kind==GD_RESOURCE_ROLE ? "resource role":"facet",(int)n->x-83,(int)n->y-19,11,color);
            DrawText(TextFormat("%.23s",node_label(graph,n->key)),(int)n->x-83,(int)n->y-3,14,foreground);
            if (shared) DrawText(TextFormat("S%d%s",analysis->matched[i],analysis->complete ? "":"+"),(int)n->x-83,(int)n->y+28,13,role_color);
            if (comparison) {
                unsigned category=comparison->nodes[i];
                if (category) {
                    Color branch_color=gd_graph_branch_color(category);
                    DrawRectangle((int)n->x+86,(int)n->y-18,2,36,branch_color);
                    gd_graph_branch_badge(category,(int)n->x-35,(int)n->y+28,background);
                }
                if (i==comparison->a || i==comparison->b) DrawText(i==comparison->a ? "A":"B",(int)n->x+70,(int)n->y+28,13,foreground);
            }
            if (overview && overview->selected_nodes[i]) DrawRectangleLinesEx((Rectangle){n->x-106,n->y-40,212,80},2,selected_color);
            if (shapes) {
                gd_graph_shape_badge(shapes,i,(int)n->x-83,(int)n->y+45,background);
                if (gd_graph_shape_peer(shapes,i,selected)) DrawRectangleLinesEx((Rectangle){n->x-102,n->y-36,204,72},2,(Color){220,205,148,255});
            }
            if (reachability) {
                gd_graph_reachability_badge(reachability,i,(int)n->x-83,(int)n->y-43,background);
                if (reach_pair && reach_pair->nodes[i]) DrawRectangleLinesEx((Rectangle){n->x-98,n->y-32,196,64},2,gd_graph_branch_color(reach_pair->nodes[i]));
            }
            if (paths) {
                gd_graph_path_badge(paths,path_selection,bypass,i,(int)n->x,(int)n->y-20,background);
                if (paths->roots[i]) gd_graph_path_start_marker((int)n->x+76,(int)n->y-15);
            }
        }
        EndMode2D(); EndScissorMode();
        gd_graph_overview_draw(analysis,&overview_controls,viewport,1);
        DrawRectangle(split,100,width-split,height-100,(Color){29,38,51,255});
        DrawText(path_tab==0 ? "[Context]":"Context",split+14,76,14,foreground);
        DrawText(path_tab==1 ? "[Paths]":"Paths",split+124,76,14,foreground);
        DrawText(path_tab==5 ? "[Overview]":path_tab==4 ? "[Shapes]": path_tab==3 ? "[Typed paths]":path_tab==2 ? "[Foundations]":"Layers (click)",split+204,76,12,foreground);
        if (path_tab==2) {
            DrawText("Prerequisite foundations: unavailable",split+14,114,15,foreground);
            DrawText("This projection has resource/rule facets,",split+14,145,14,muted);
            DrawText("not shown skill-prerequisite declarations.",split+14,165,14,muted);
            DrawText("No hidden skill graph is analyzed.",split+14,195,14,muted);
            DrawText("Resource incidences are not AND skills,",split+14,225,14,muted);
            DrawText("learning requirements or proof of flow.",split+14,245,14,muted);
        }
        else if (path_tab==5) overview_details(graph,layout,analysis,&overview_controls,split+14);
        else if (path_tab==4) shape_details(graph,shapes,shape_enabled,selected,split+14);
        else if (path_tab==3) reach_details(graph,layout,analysis,&reach_controls,selected,split+14);
        else if (path_tab==1) path_details(graph,layout,analysis,paths,&path_controls,selected,split+14);
        else {
        DrawText(shared ? "[x] Shared neighbors (click)":"[ ] Shared neighbors (click)",split+14,114,16,foreground);
        if (shared) {
            DrawText(TextFormat("Lens: %s (click)",gd_graph_lens_names[shared_lens]),split+14,140,14,foreground);
            DrawText(branches ? "[x] Compare branches (click)":"[ ] Compare branches (click)",split+14,166,14,foreground);
            DrawRectangleLines(split+14,242,190,25,muted); shared_details(graph,layout,analysis,comparison,selected,pair,split+14);
        }
        detail(graph,selected,split+14,width-split-28,shared ? (comparison ? 510:417):150);
        }
        if (stale) DrawText("STALE: sources changed. Close and reopen preview.",18,height-38,15,selected_color);
        else DrawText("Drag nodes | Right drag: pan | Wheel: zoom | F: fit | Space: settle/pause",18,height-38,15,muted);
        DrawText("Edges: teal condition | red exclusion | blue write | green preservation | purple role | gold call",18,height-20,13,muted);
        EndDrawing(); ++frame;
        if (screenshot && frame==frames && !gd_save_screenshot(screenshot)) { gd_error(model,"view.screenshot","Resource preview screenshot failed",screenshot,0); result=2; }
    }
    CloseWindow(); model->allocator.release(model->allocator.context,analysis); model->allocator.release(model->allocator.context,layout); return result;
}
