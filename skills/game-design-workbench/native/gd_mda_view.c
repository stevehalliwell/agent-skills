#include "gd_layout.h"
#include "gd_graph_analysis_ui.h"
#include "gd_graph_analysis_draw.h"
#include "gd_diagnostics_ui.h"
#include "raylib.h"
#include "rlImGui.h"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_NO_EXPORT
#include "cimgui.h"
#include <math.h>
#include <stdarg.h>
#include <string.h>

static const Color background = {18,24,34,255}, foreground = {225,233,241,255}, muted = {150,169,190,255}, selected_color = {255,217,125,255};
typedef struct {
    int record, link, held;
    bool roles[4], only_scope, paused, only_path;
    char filter[129];
    unsigned char path_nodes[GD_MAX_RECORDS], path_links[GD_MAX_EDGES];
} MdaDisplay;
static void mark_path(const GD_Mda *mda, MdaDisplay *display) {
    int v = display->record, count = 0;
    memset(display->path_nodes,0,sizeof(display->path_nodes)); memset(display->path_links,0,sizeof(display->path_links));
    if (mda->root < 0 || v < 0 || !mda->selected[v]) return;
    /* Highlight the existing parent witness; do not compute another trace. */
    while (v >= 0 && count++ < GD_MAX_RECORDS) {
        display->path_nodes[v] = 1;
        if (mda->parent_link[v] >= 0) display->path_links[mda->parent_link[v]] = 1;
        v = mda->parent[v];
    }
}
static Color role_color(GD_Role role) {
    const Color colors[] = {{108,171,230,255},{224,160,109,255},{65,180,165,255},{193,155,228,255}};
    return gd_mda_role(role) ? colors[role-GD_MDA_MECHANIC] : muted;
}
static void mda_log(int level, const char *text, va_list arguments) {
    if (level >= LOG_WARNING) { fputs("gd view: ",stderr); vfprintf(stderr,text,arguments); fputc('\n',stderr); }
}
static int visible(const GD_Model *model, const GD_Mda *mda, const MdaDisplay *display, int record) {
    const GD_Record *item = &model->records[record];
    return gd_mda_role(item->role) && display->roles[item->role-GD_MDA_MECHANIC] &&
        (!display->only_scope || mda->selected[record]) && (!display->only_path || display->path_nodes[record]) &&
        (!display->filter[0] || strstr(item->reference,display->filter));
}
/* Shared solver uses point-sized nodes. Render free-graph distances at 3x
   so its 110-unit resting edges leave room for 210-unit source cards. */
static float spacing(const GD_Layout *layout) { return layout->mode == GD_LAYOUT_FORCE ? 3.0f:1.0f; }
static Vector2 point(const GD_Layout *layout, const GD_LayoutNode *node) { float scale = spacing(layout); return (Vector2){node->x*scale,node->y*scale}; }
static Vector2 control(Vector2 a, Vector2 b) {
    /* Direct dynamics/mechanics links must not appear to pass through a
       systems node sitting in the intervening role band. */
    return (Vector2){(a.x+b.x)/2,(a.y+b.y)/2-(fabsf(a.x-b.x) > 300 ? 110:30)};
}
static Vector2 curve(Vector2 a, Vector2 c, Vector2 b, float t) {
    float u = 1-t; return (Vector2){u*u*a.x+2*u*t*c.x+t*t*b.x,u*u*a.y+2*u*t*c.y+t*t*b.y};
}
static float segment_square(Vector2 p, Vector2 a, Vector2 b) {
    float x = b.x-a.x, y = b.y-a.y, square = x*x+y*y, t = square ? ((p.x-a.x)*x+(p.y-a.y)*y)/square:0;
    if (t < 0) t = 0; if (t > 1) t = 1;
    x = p.x-a.x-t*x; y = p.y-a.y-t*y; return x*x+y*y;
}
static void fit(Camera2D *camera, const GD_Model *model, const GD_Mda *mda, const GD_Layout *layout, const MdaDisplay *display, Rectangle viewport) {
    int i, found = 0; float left = 0, right = 1, top = 0, bottom = 1;
    for (i = 0; i < layout->node_count; ++i) if (visible(model,mda,display,layout->nodes[i].key)) {
        Vector2 position = point(layout,&layout->nodes[i]);
        if (!found || position.x-120 < left) left = position.x-120;
        if (!found || position.x+120 > right) right = position.x+120;
        if (!found || position.y-100 < top) top = position.y-100;
        if (!found || position.y+55 > bottom) bottom = position.y+55;
        found = 1;
    }
    camera->target = (Vector2){(left+right)/2,(top+bottom)/2};
    camera->zoom = fminf(viewport.width/(right-left+60),viewport.height/(bottom-top+70));
    if (camera->zoom > 1.2f) camera->zoom = 1.2f;
    if (camera->zoom < 0.002f) camera->zoom = 0.002f;
}
static void input(const GD_Model *model, const GD_Mda *mda, GD_Layout *layout, MdaDisplay *display, Camera2D *camera, Rectangle viewport) {
    Vector2 mouse = GetMousePosition(), world; int i;
    camera->offset = (Vector2){viewport.x+viewport.width/2,viewport.y+viewport.height/2};
    if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) display->held = -1;
    if (display->held >= 0) {
        world = GetScreenToWorld2D(mouse,*camera); gd_layout_move_node(layout,display->held,world.x/spacing(layout),world.y/spacing(layout));
    }
    if (!CheckCollisionPointRec(mouse,viewport) || igGetIO_Nil()->WantCaptureMouse) return;
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) || IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
        Vector2 delta = GetMouseDelta(); camera->target.x -= delta.x/camera->zoom; camera->target.y -= delta.y/camera->zoom;
    }
    world = GetScreenToWorld2D(mouse,*camera);
    if (GetMouseWheelMove()) {
        Vector2 after; camera->zoom *= powf(1.15f,GetMouseWheelMove());
        if (camera->zoom < 0.002f) camera->zoom = 0.002f; if (camera->zoom > 4) camera->zoom = 4;
        after = GetScreenToWorld2D(mouse,*camera); camera->target.x += world.x-after.x; camera->target.y += world.y-after.y;
    }
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return;
    world = GetScreenToWorld2D(mouse,*camera);
    for (i = layout->node_count-1; i >= 0; --i) {
        const GD_LayoutNode *node = &layout->nodes[i];
        Vector2 position = point(layout,node);
        if (visible(model,mda,display,node->key) && CheckCollisionPointRec(world,(Rectangle){position.x-105,position.y-32,210,64})) {
            display->record = node->key; display->held = i; display->link = -1; return;
        }
    }
    for (i = 0; i < layout->edge_count; ++i) {
        const GD_LayoutEdge *edge = &layout->edges[i]; int k;
        Vector2 a = point(layout,&layout->nodes[edge->a]), b = point(layout,&layout->nodes[edge->b]), c = control(a,b), previous = a;
        if (!visible(model,mda,display,layout->nodes[edge->a].key) || !visible(model,mda,display,layout->nodes[edge->b].key)) continue;
        for (k = 1; k <= 20; ++k) {
            Vector2 next = curve(a,c,b,k/20.0f);
            if (segment_square(world,previous,next) < 64/(camera->zoom*camera->zoom)) {
                display->link = edge->key; display->record = model->edges[mda->links[edge->key].edge].source; return;
            }
            previous = next;
        }
    }
}
static int analyze_display(const GD_Model *model, const GD_Mda *mda, const GD_Layout *layout, const MdaDisplay *display, GD_GraphAnalysis *analysis, GD_GraphLens lens, int *peer) {
    unsigned char shown[GD_MAX_RECORDS]; int i,selected=-1;
    for (i=0;i<layout->node_count;++i) {
        shown[i]=(unsigned char)visible(model,mda,display,layout->nodes[i].key);
        if (shown[i] && layout->nodes[i].key==display->record) selected=i;
    }
    if (gd_graph_analyze(layout,shown,1,lens,analysis)) *peer=-1;
    return selected;
}
static void projected_source(const GD_Model *model, const GD_Mda *mda, const GD_Layout *layout, int index) {
    const GD_Edge *edge=&model->edges[mda->links[layout->edges[index].key].edge];
    igTextWrapped("%s | %s -> %s | %s:%d",gd_field_name(edge->field),model->records[edge->source].reference,edge->reference,model->records[edge->source].source_path,edge->line);
}
static void draw(const GD_Model *model, const GD_Mda *mda, const GD_Layout *layout, const MdaDisplay *display, Camera2D camera, Rectangle viewport, GD_GraphAnalysis *analysis, int shared, int selected, int peer, int branches, const GD_GraphPaths *paths, const GD_PathBypass *bypass) {
    const Color shared_color={193,155,228,255};
    int pair=shared ? gd_graph_match(analysis,selected,peer):-1;
    const GD_GraphFeature *left=pair>=0 ? &analysis->features[analysis->pairs[pair].first_a]:NULL;
    const GD_GraphFeature *right=pair>=0 ? &analysis->features[analysis->pairs[pair].first_b]:NULL;
    const GD_GraphComparison *comparison=pair>=0 && branches ? gd_graph_compare(analysis,selected,peer):NULL;
    const GD_PathSelection *path_selection=paths ? gd_graph_path_selection(analysis,selected):NULL;
    int i, pass, drawn = 0;
    BeginScissorMode((int)viewport.x,(int)viewport.y,(int)viewport.width,(int)viewport.height); BeginMode2D(camera);
    /* Draw emphasis last so duplicate declarations cannot hide a witness. */
    for (pass = 0; pass < 2; ++pass) for (i = 0; i < layout->edge_count; ++i) {
        const GD_LayoutEdge *edge = &layout->edges[i];
        int emphasized = display->link == edge->key || display->path_links[edge->key];
        int source = layout->nodes[edge->a].key, target = layout->nodes[edge->b].key;
        Vector2 a = point(layout,&layout->nodes[edge->a]), b = point(layout,&layout->nodes[edge->b]), c = control(a,b), tip, before;
        Color color = display->link == edge->key || display->path_links[edge->key] ? selected_color : mda->selected[source] && mda->selected[target] ? (Color){104,182,171,255}:(Color){73,88,107,255};
        float dx, dy, length, t;
        if (emphasized != pass || !visible(model,mda,display,source) || !visible(model,mda,display,target)) continue;
        if (bypass && bypass->complete && bypass->found && bypass->edges[i]) DrawSplineSegmentBezierQuadratic(a,c,b,15,gd_bypass_color);
        if (path_selection && path_selection->edges[i]) DrawSplineSegmentBezierQuadratic(a,c,b,11,gd_path_color);
        if (comparison && comparison->edges[i]) DrawSplineSegmentBezierQuadratic(a,c,b,7,gd_graph_branch_color(comparison->edges[i]));
        else if (!comparison && pair>=0 && (i==left->edge || i==right->edge)) DrawSplineSegmentBezierQuadratic(a,c,b,7,shared_color);
        DrawSplineSegmentBezierQuadratic(a,c,b,display->link == edge->key ? 4:2.5f,color);
        /* Mid-curve marker points in authored source-to-reference direction. */
        t = 0.5f; tip = curve(a,c,b,t); before = curve(a,c,b,t-0.025f);
        dx = tip.x-before.x; dy = tip.y-before.y; length = sqrtf(dx*dx+dy*dy);
        if (length > 0) {
            dx /= length; dy /= length;
            DrawLineEx(tip,(Vector2){tip.x-dx*13-dy*6,tip.y-dy*13+dx*6},3,color);
            DrawLineEx(tip,(Vector2){tip.x-dx*13+dy*6,tip.y-dy*13-dx*6},3,color);
        }
        if (display->link == edge->key) DrawText(gd_field_name(model->edges[mda->links[edge->key].edge].field),(int)tip.x+5,(int)tip.y-22,14,selected_color);
    }
    for (i = 0; i < layout->node_count; ++i) {
        const GD_LayoutNode *node = &layout->nodes[i]; const GD_Record *record = &model->records[node->key];
        Color color = role_color(record->role); char label[32]; Vector2 position = point(layout,node);
        if (!visible(model,mda,display,node->key)) continue;
        ++drawn;
        if (!mda->selected[node->key]) color = Fade(color,0.40f);
        DrawRectangleRounded((Rectangle){position.x-105,position.y-32,210,64},0.17f,6,(Color){27,36,48,255});
        DrawRectangleRoundedLinesEx((Rectangle){position.x-105,position.y-32,210,64},0.17f,6,display->record == node->key ? 3:1.5f,display->path_nodes[node->key] || display->record == node->key ? selected_color:color);
        snprintf(label,sizeof(label),"%.24s%s",record->name,strlen(record->name) > 24 ? "...":"");
        DrawText(gd_mda_role_name(record->role),(int)position.x-96,(int)position.y-24,12,color);
        DrawText(label,(int)position.x-96,(int)position.y-5,16,mda->selected[node->key] ? foreground:muted);
        if (node->key == mda->root) DrawText("ROOT",(int)position.x+60,(int)position.y-24,12,selected_color);
        if (shared) {
            DrawText(TextFormat("S%d%s",analysis->matched[i],analysis->complete ? "":"+"),(int)position.x-96,(int)position.y+35,13,shared_color);
            if (comparison) {
                unsigned category=comparison->nodes[i];
                if (category) {
                    Color branch_color=gd_graph_branch_color(category);
                    DrawRectangle((int)position.x+100,(int)position.y-20,3,40,branch_color);
                    gd_graph_branch_badge(category,(int)position.x-40,(int)position.y+35,background);
                }
                if (i==comparison->a || i==comparison->b) DrawText(i==comparison->a ? "A":"B",(int)position.x+83,(int)position.y+35,13,foreground);
            } else if (pair>=0 && i==left->neighbor) DrawText("shared",(int)position.x+25,(int)position.y+35,13,shared_color);
        }
        if (paths) {
            gd_graph_path_badge(paths,path_selection,bypass,i,(int)position.x-96,(int)position.y+15,background);
            if (paths->roots[i]) gd_graph_path_start_marker((int)position.x+94,(int)position.y+17);
        }
    }
    EndMode2D();
    if (!drawn) DrawText("No visible MDA nodes. Check supplied folders/display filters.",(int)viewport.x+12,(int)viewport.y+20,16,muted);
    EndScissorMode();
}
static void source_details(const GD_Model *model, const GD_Mda *mda, MdaDisplay *display) {
    int i, source = display->record;
    if (display->link >= 0) {
        const GD_MdaLink *link = &mda->links[display->link]; const GD_Edge *edge = &model->edges[link->edge];
        igTextUnformatted("Selected authored link",NULL);
        igTextWrapped("%s\nfield: %s\nreference: %s\nresolution: %s | source line %d",model->records[edge->source].reference,gd_field_name(edge->field),edge->reference,gd_mda_resolution_name(link->resolution),edge->line);
        if (edge->target >= 0 && igSmallButton("Inspect referenced record")) { display->record = edge->target; display->link = -1; source = display->record; }
        igSeparator();
    }
    if (source < 0) { igTextWrapped("Select a node, link, record, or finding to inspect its source."); return; }
    igTextWrapped("%s | %s",model->records[source].reference,gd_mda_role_name(model->records[source].role));
    igTextWrapped("Status: %s | ID: %s",model->records[source].status ? model->records[source].status:"null",model->records[source].id ? model->records[source].id:"null");
    igTextWrapped("%s",model->records[source].description ? model->records[source].description:"");
    igTextWrapped("Source: %s",model->records[source].source_path);
    if (gd_mda_role(model->records[source].role)) {
        igTextWrapped("Direct basis references: %s | Aesthetic trace: %s",gd_mda_basis(model,mda,source),gd_mda_trace(mda,source));
        if (mda->root >= 0 && mda->selected[source]) {
            int nodes[GD_MAX_MDA_PATH], links[GD_MAX_MDA_PATH], full, count = gd_mda_path(mda,source,nodes,links,GD_MAX_MDA_PATH,&full);
            igText("Shortest query witness: %d nodes%s",full,full > count ? " (trailing suffix shown)":"");
            for (i = 0; i < count; ++i) {
                igPushID_Int(i);
                if (igSmallButton(model->records[nodes[i]].reference)) { display->record = nodes[i]; display->link = -1; }
                if (i+1 < count) {
                    const GD_Edge *edge = &model->edges[mda->links[links[i]].edge];
                    igText("%s (%s reference)",gd_field_name(edge->field),edge->source == nodes[i] ? "along authored":"reverse authored");
                }
                igPopID();
            }
        } else igTextWrapped(mda->root < 0 ? "Choose a query root to inspect a shortest witness.":"Selected record is outside current query.");
    }
    if (igCollapsingHeader_TreeNodeFlags("Authored references / source context",0)) {
        for (i = 0; i < model->edge_count; ++i) if (model->edges[i].source == source) {
            const GD_Edge *edge = &model->edges[i]; igPushID_Int(i);
            igTextWrapped("%s: %s (line %d)",gd_field_name(edge->field),edge->reference,edge->line);
            if (edge->target >= 0) { if (igSmallButton("Inspect target")) { display->record = edge->target; display->link = -1; } }
            else igTextUnformatted("No unique loaded target. Inspect scoped core findings; other references are context only.",NULL);
            igPopID();
        }
    }
    igSeparator(); igPushTextWrapPos(0); igTextUnformatted(model->records[source].body,NULL); igPopTextWrapPos();
}
int gd_mda_view(GD_Model *initial, GD_Mda *initial_mda, int frames, const char *screenshot, int bands) {
    GD_Model *model = initial; GD_Mda *mda = initial_mda;
    GD_DiagnosticBrowser diagnostics={0};
    GD_GraphAnalysis *analysis=NULL; bool shared=true,branches=false; int shared_peer=-1,shared_selected=-1;
    GD_GraphLens shared_lens=GD_GRAPH_ANY;
    GD_PathControls path_controls={0}; bool foundation_enabled=false; int foundation_peer=-1;
    GD_Layout *layout = initial->allocator.allocate(initial->allocator.context,sizeof(*layout));
    MdaDisplay display = {.record = -1,.link = -1,.held = -1,.roles = {true,true,true,true},.paused = true};
    Camera2D camera = {{0,0},{0,0},0,1};
    int frame = 0, result = 0, fit_next = 1, stale = 0, layout_choice = bands ? 0:1;
    double next_check = 0; char failure[GD_MAX_PATH+512] = "";
    if (!mda || !layout || !gd_mda_layout(model,mda,layout,bands)) {
        if (layout) initial->allocator.release(initial->allocator.context,layout);
        gd_error(initial,"view.memory","Cannot prepare bounded MDA presentation",NULL,0); return 2;
    }
    display.record = mda->root;
    if (mda->root >= 0 && mda->direction == GD_MDA_FORWARD && mda->nodes[mda->root].aesthetic >= 0 && mda->selected[mda->nodes[mda->root].aesthetic]) display.record = mda->nodes[mda->root].aesthetic;
    mark_path(mda,&display);
    if (display.record < 0) { int i; for (i = 0; i < model->record_count; ++i) if (model->records[i].role == GD_MDA_AESTHETIC) { display.record = i; break; } }
    SetTraceLogCallback(mda_log); SetTraceLogLevel(LOG_WARNING); SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1500,940,"Game Design Workbench - MDA explorer");
    if (!IsWindowReady()) { initial->allocator.release(initial->allocator.context,layout); gd_error(initial,"view.window","Cannot initialize MDA window",NULL,0); return 2; }
    analysis=initial->allocator.allocate(initial->allocator.context,sizeof(*analysis));
    if (!analysis) { CloseWindow(); initial->allocator.release(initial->allocator.context,layout); gd_error(initial,"view.memory","Cannot allocate graph analysis",NULL,0); return 2; }
    memset(analysis,0,sizeof(*analysis));
    SetWindowMinSize(1100,720); SetTargetFPS(60); rlImGuiSetup(true);
    igGetIO_Nil()->IniFilename = NULL; igGetIO_Nil()->LogFilename = NULL; igGetStyle()->FontScaleMain = 1.1f;
    while (!WindowShouldClose() && (!frames || frame < frames)) {
        int width = GetScreenWidth(), height = GetScreenHeight(), split = width-480, refresh = 0, i;
        Rectangle viewport = {16,153,(float)split-32,(float)height-200};
        int direction = mda->direction, systems_supplied = 0;
        for (i = 0; i < model->collection_count; ++i) if (model->collections[i].role == GD_MDA_SYSTEM) systems_supplied = 1;
        BeginDrawing(); ClearBackground(background); rlImGuiBegin();
        if (GetTime() >= next_check) { if (gd_sources_changed(model)) stale = 1; next_check = GetTime()+1; }
        DrawText("MDA declared-link explorer",20,18,27,foreground);
        DrawText(TextFormat("%d / %d query nodes | %d core links | All statuses | Read-only",mda->selected_count,mda->node_count,mda->link_count),20,56,16,muted);
        DrawText("Authored arrows do not prove cause. Systems are not dynamics.",20,82,16,muted);
        for (i = 0; i < 4; ++i) DrawText(gd_mda_role_name((GD_Role)(GD_MDA_MECHANIC+i)),20+i*135,116,18,role_color((GD_Role)(GD_MDA_MECHANIC+i)));
        igSetNextWindowPos((ImVec2_c){(float)split,12},ImGuiCond_Always,(ImVec2_c){0,0});
        igSetNextWindowSize((ImVec2_c){(float)(width-split-12),(float)(height-24)},ImGuiCond_Always);
        igBegin("MDA query and source",NULL,ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoSavedSettings);
        igTextWrapped("Root: %s",mda->root >= 0 ? model->records[mda->root].reference:"all supplied MDA nodes");
        if (!systems_supplied) igTextWrapped("Systems role not supplied (optional); its declared references may remain unloaded.");
        if (igCombo_Str("Traversal",&direction,"Both / connected associations\0Forward / toward aesthetics\0Backward / authored basis\0",-1)) gd_mda_query(model,mda,mda->root,(GD_MdaDirection)direction);
        if (display.record >= 0 && gd_mda_role(model->records[display.record].role)) {
            if (igButton("Use selected as query root",(ImVec2_c){0,0})) { gd_mda_query(model,mda,display.record,mda->direction); fit_next = 1; }
            igSameLine(0,-1);
        }
        if (igButton("Show all",(ImVec2_c){0,0})) { gd_mda_query(model,mda,-1,mda->direction); fit_next = 1; }
        if (igCombo_Str("Layout",&layout_choice,"Role bands\0Free graph\0",-1)) { gd_mda_layout(model,mda,layout,!layout_choice); fit_next = 1; display.held = -1; }
        igCheckbox("Pause settling",&display.paused); igSameLine(0,-1);
        if (igButton("Fit",(ImVec2_c){0,0})) fit_next = 1;
        igSameLine(0,-1); refresh = igButton("Refresh (F5)",(ImVec2_c){0,0});
        if (stale) igTextWrapped("STALE: disk changed or could not be checked. Loaded snapshot remains visible.");
        if (failure[0]) igTextWrapped("Reload rejected: %s",failure);
        if (igCheckbox("Only query nodes (display only)",&display.only_scope)) fit_next = 1;
        if (igCheckbox("Only selected witness path (display only)",&display.only_path)) fit_next = 1;
        igTextUnformatted("Gold: selection/witness; teal: other query links.",NULL);
        for (i = 0; i < 4; ++i) { if (i) igSameLine(0,-1); igCheckbox(gd_mda_role_name((GD_Role)(GD_MDA_MECHANIC+i)),&display.roles[i]); }
        igSetNextItemWidth(-1); igInputText("##MDA-reference-filter",display.filter,sizeof(display.filter),0,NULL,NULL);
        igTextUnformatted("Reference filter; presentation only. Clear to show all.",NULL);
        mark_path(mda,&display); shared_selected=analyze_display(model,mda,layout,&display,analysis,shared_lens,&shared_peer);
        { int pair=gd_graph_analysis_panel(layout,analysis,shared_selected,&shared,&shared_lens,&branches,&shared_peer);
          if (pair>=0) for (i=0;i<2;++i) {
              const GD_GraphMatch *match=&analysis->pairs[pair];
              int feature=i ? match->first_b:match->first_a;
              projected_source(model,mda,layout,analysis->features[feature].edge);
          } }
        if (gd_graph_paths_panel(layout,analysis,&path_controls,shared_selected,NULL,0)) {
            const GD_PathSelection *support=gd_graph_path_selection(analysis,shared_selected);
            igTextUnformatted("Support source declarations:",NULL);
            for (i=support->retained-1;i>=0;--i) projected_source(model,mda,layout,support->arcs[i]/2);
            if (path_controls.compare) {
                const GD_PathBypass *bypass=gd_graph_path_bypass(analysis,shared_selected,-1,GD_GRAPH_ANALYSIS_WORK);
                if (bypass && bypass->complete && bypass->found) {
                    igTextUnformatted("Bypass source declarations:",NULL);
                    for (i=bypass->retained-1;i>=0;--i) projected_source(model,mda,layout,bypass->arcs[i]/2);
                }
            }
        }
        gd_graph_foundations_panel(model,layout,analysis,shared_selected,1,&foundation_enabled,&foundation_peer);
        if (path_controls.enabled) igTextWrapped("Path starts are independent of query root/traversal; follow authored arrows, not inferred causality.");
        if (igCollapsingHeader_TreeNodeFlags("Browse supplied MDA records",0)) {
            igBeginChild_Str("MDA records",(ImVec2_c){0,120},0,0);
            for (i = 0; i < model->record_count; ++i) if (visible(model,mda,&display,i)) {
                igPushID_Int(i);
                if (igSelectable_Bool(model->records[i].reference,display.record == i,0,(ImVec2_c){0,0})) { display.record = i; display.link = -1; }
                igPopID();
            }
            igEndChild();
        }
        {
            int target=gd_diagnostics_panel(&diagnostics,model,mda->findings,mda->finding_count,mda->findings_omitted,display.record);
            if (target>=0) { display.record=target; display.link=-1; }
        }
        igText("%d scoped records with questions | %d findings",mda->gap_count,mda->finding_count);
        igSeparator(); igBeginChild_Str("MDA source",(ImVec2_c){0,0},0,0); source_details(model,mda,&display); igEndChild(); igEnd();
        if (!igGetIO_Nil()->WantCaptureKeyboard && IsKeyPressed(KEY_F5)) refresh = 1;
        if (!igGetIO_Nil()->WantCaptureKeyboard && IsKeyPressed(KEY_F)) fit_next = 1;
        mark_path(mda,&display);
        camera.offset = (Vector2){viewport.x+viewport.width/2,viewport.y+viewport.height/2};
        if (fit_next) { fit(&camera,model,mda,layout,&display,viewport); fit_next = 0; }
        input(model,mda,layout,&display,&camera,viewport); mark_path(mda,&display);
        if (!display.paused) gd_layout_step(layout,display.held);
        shared_selected=analyze_display(model,mda,layout,&display,analysis,shared_lens,&shared_peer);
        const GD_GraphPaths *paths=gd_graph_paths_prepare(analysis,&path_controls,NULL,0);
        const GD_PathBypass *bypass=paths && path_controls.compare ? gd_graph_path_bypass(analysis,shared_selected,-1,GD_GRAPH_ANALYSIS_WORK):NULL;
        draw(model,mda,layout,&display,camera,viewport,analysis,shared,shared_selected,shared_peer,branches,paths,bypass);
        if (paths) DrawText(TextFormat("D: dominated targets | J: incoming neighbors | Pink: support | Cyan: bypass | C/F/R: witness | starts %d",paths->root_count),20,155,12,gd_path_color);
        if (shared) DrawText(branches ? "Pair branches (one hop): selected A / peer B | S badges remain graph-wide | Shown graph only":analysis->complete ? TextFormat("S: %s | Thick purple: example links | Shown graph only",gd_graph_lens_names[shared_lens]):"PARTIAL shared-neighbor query | S counts are lower bounds | Shown graph only",20,138,13,(Color){193,155,228,255});
        DrawText("Drag: layout | Right drag: pan | Wheel: zoom | F: fit | F5: reload",20,height-40,14,muted);
        DrawText("Declared links/questions are not design or experience verdicts.",20,height-21,14,muted);
        rlImGuiEnd(); EndDrawing(); ++frame;
        if (screenshot && frame == frames && !gd_save_screenshot(screenshot)) { gd_error(initial,"view.screenshot","MDA screenshot export failed",screenshot,0); result = 2; }
        if (refresh) {
            GD_Model *fresh = gd_create(initial->allocator); GD_Mda *replacement = NULL; GD_Layout *fresh_layout = NULL;
            char root[260] = "", selection[260] = ""; int ready;
            if (mda->root >= 0) snprintf(root,sizeof(root),"%s",model->records[mda->root].reference);
            if (display.record >= 0) snprintf(selection,sizeof(selection),"%s",model->records[display.record].reference);
            ready = fresh && gd_load(fresh,model->collections,model->collection_count);
            if (ready) {
                replacement = gd_analyze_mda(fresh);
                ready = replacement && gd_mda_query(fresh,replacement,root[0] ? gd_mda_find(fresh,root):-1,mda->direction);
                if (replacement && !ready && !fresh->error_count) gd_error(fresh,"mda.root","Query root no longer exists; clear root before reload to accept removal",NULL,0);
            }
            if (ready) { fresh_layout = initial->allocator.allocate(initial->allocator.context,sizeof(*fresh_layout)); ready = fresh_layout && gd_mda_layout(fresh,replacement,fresh_layout,!layout_choice); }
            if (ready) {
                gd_graph_paths_remap(&path_controls,model,fresh);
                if (model != initial) { gd_mda_destroy(mda); gd_destroy(model); }
                initial->allocator.release(initial->allocator.context,layout); layout = fresh_layout; model = fresh; mda = replacement;
                display.record = display.link = display.held = -1;
                analysis->initialized=0; shared_peer=-1;
                for (i = 0; i < model->record_count; ++i) if (!strcmp(selection,model->records[i].reference)) display.record = i;
                gd_diagnostics_clear(&diagnostics);
                mark_path(mda,&display); stale = 0; failure[0] = 0; fit_next = 1;
            } else {
                if (fresh && fresh->error_count) {
                    const GD_Error *error = &fresh->errors[0];
                    snprintf(failure,sizeof(failure),"%s: %s | %s (line %d)",error->code,error->message,error->path ? error->path:"supplied sources",error->line);
                } else snprintf(failure,sizeof(failure),"Cannot allocate replacement snapshot/layout");
                stale = 1;
                if (fresh_layout) initial->allocator.release(initial->allocator.context,fresh_layout);
                gd_diagnostics_rejected(&diagnostics,initial->allocator,fresh);
                gd_mda_destroy(replacement); gd_destroy(fresh);
            }
        }
    }
    rlImGuiShutdown(); CloseWindow();
    gd_diagnostics_clear(&diagnostics);
    if (model != initial) { gd_mda_destroy(mda); gd_destroy(model); }
    initial->allocator.release(initial->allocator.context,analysis);
    initial->allocator.release(initial->allocator.context,layout); return result;
}
