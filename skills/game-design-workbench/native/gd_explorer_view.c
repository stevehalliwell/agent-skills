#include "gd_explorer_ui.h"
#include "gd_graph_analysis_draw.h"
#include "gd_graph_overview_draw.h"
#include "rlImGui.h"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_NO_EXPORT
#include "cimgui.h"
#include <math.h>
#include <stdarg.h>
#include <string.h>

static const Color background={18,24,34,255}, foreground={225,233,241,255}, muted={150,169,190,255}, gold={255,217,125,255};
static void explorer_log(int level, const char *text, va_list arguments) {
    (void)level; vfprintf(stderr,text,arguments); fputc('\n',stderr);
}
static int open_window(const char *title) {
    SetTraceLogCallback(explorer_log); SetTraceLogLevel(LOG_WARNING); SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1500,940,title);
    if (!IsWindowReady()) return 0;
    SetWindowMinSize(1000,700); SetExitKey(KEY_NULL); SetTargetFPS(60); rlImGuiSetup(true);
    igGetIO_Nil()->IniFilename=NULL; igGetIO_Nil()->LogFilename=NULL; igGetStyle()->FontScaleMain=1.1f;
    return 1;
}
static int is_graph(GD_ExplorerView view) { return view<=GD_EXPLORER_MATRIX; }
static int incidence(GD_ExplorerView view) { return view==GD_EXPLORER_MATRIX || view==GD_EXPLORER_RESOURCES; }
static const char *missing_roles(GD_ExplorerView view) {
    switch (view) {
        case GD_EXPLORER_SKILLS: case GD_EXPLORER_RELATIONSHIPS: return "Supply --skill-atoms ALIAS=FOLDER";
        case GD_EXPLORER_CHALLENGES: return "Supply --skill-atoms and --challenges ALIAS=FOLDER";
        case GD_EXPLORER_MATRIX: return "Supply --nouns, --verbs and --interactions ALIAS=FOLDER";
        case GD_EXPLORER_MDA: return "Supply --mechanics, --dynamics and --aesthetics ALIAS=FOLDER";
        default: return "Supply --interactions and/or --mechanics ALIAS=FOLDER";
    }
}
static void pane_defaults(GD_ExplorerPane *p, GD_ExplorerView view) {
    GD_Layout *layout=p->layout;
    gd_diagnostics_clear(&p->diagnostics);
    memset(p,0,sizeof(*p)); p->layout=layout; p->view=view;
    p->selection=(GD_ExplorerSelection){-1,-1,0,-1,-1}; p->editor.record=-1;
    p->mda=(GD_ExplorerMdaDisplay){.record=-1,.link=-1,.held=-1,.roles={true,true,true,true},.paused=true};
    p->camera.zoom=1; p->peer=p->foundation_peer=p->witness=-1;
    p->navigate=p->focus_root=p->focus_expand=p->inspected_record=-1;
    p->reach=(GD_ReachControls){0,-1,0,-1,-1}; p->overview=(GD_OverviewControls){0,-1,-1};
    p->shared=true; p->paused=view==GD_EXPLORER_MDA || view==GD_EXPLORER_RESOURCES;
    p->layout_choice=GD_LAYOUT_FORCE; p->fit=1; p->focus.root=-1;
    p->paths.declared=view==GD_EXPLORER_CHALLENGES;
}
static int pane_prepare(GD_ExplorerContext *c) {
    GD_ExplorerPane *p=c->pane; GD_Model *model=gd_explorer_model(c->project,p->view); int i,record=-1;
    unsigned revision=gd_explorer_family(p->view)==GD_EXPLORER_SKILL_DATA ? c->project->revision:1;
    if (p->prepared && p->revision==revision) return 1;
    if (!p->layout) {
        p->layout=c->project->allocator.allocate(c->project->allocator.context,sizeof(*p->layout));
        if (p->layout) memset(p->layout,0,sizeof(*p->layout));
    }
    if (!p->layout) return 0;
    if (is_graph(p->view)) {
        if (p->prepared && p->revision!=revision) {
            record=p->inspected_record;
            p->selection=(GD_ExplorerSelection){-1,-1,0,-1,-1};
        } else if (p->selection.linked>=0) record=p->selection.linked;
        else if (p->selection.node>=0 && p->selection.node<p->layout->node_count) record=p->layout->nodes[p->selection.node].key;
        else if (p->editor.record>=0) record=p->editor.record;
    }
    if (p->view==GD_EXPLORER_RELATIONSHIPS && p->focus.root<0) {
        for (i=0;i<model->record_count;++i) if (model->records[i].role==GD_SKILL) { record=i; break; }
        if (record<0 && model->record_count) record=0;
        gd_focus_reset(model,&p->focus,record);
    }
    if (!(is_graph(p->view) ? gd_explorer_graph_prepare(c):p->view==GD_EXPLORER_MDA ? gd_explorer_mda_prepare(c):gd_explorer_resource_prepare(c))) return 0;
    if (record>=0 && is_graph(p->view)) {
        p->selection=(GD_ExplorerSelection){-1,-1,0,-1,-1};
        for (i=0;i<p->layout->node_count;++i) if (p->layout->nodes[i].key==record) p->selection.node=i;
        if (p->selection.node<0) p->selection.linked=record;
    }
    if (is_graph(p->view)) p->inspected_record=gd_explorer_graph_record(c);
    p->prepared=1; p->revision=revision; return 1;
}
static int selected_record(GD_ExplorerContext *c) {
    GD_ExplorerPane *p=c->pane;
    if (is_graph(p->view)) return gd_explorer_graph_record(c);
    if (p->view==GD_EXPLORER_MDA) return p->mda.record;
    return p->selection.linked>=0 ? p->selection.linked:gd_explorer_resource_record(c->project->resources,p->selection.node);
}
static Vector2 position(const GD_ExplorerPane *p, int node) {
    const GD_LayoutNode *item=&p->layout->nodes[node];
    float scale=p->view==GD_EXPLORER_MDA && p->layout->mode==GD_LAYOUT_FORCE ? 3:1;
    return (Vector2){item->x*scale,item->y*scale};
}
static int visible(GD_ExplorerContext *c, int node) {
    GD_ExplorerPane *p=c->pane; int record=p->layout->nodes[node].key;
    if (p->view==GD_EXPLORER_MDA) return gd_explorer_mda_visible(c,record);
    if (p->view==GD_EXPLORER_RESOURCES) {
        const GD_ResourceNode *item=&c->project->resources->nodes[record];
        int source=gd_explorer_resource_record(c->project->resources,record);
        const GD_Model *model=gd_explorer_model(c->project,p->view);
        return p->mda.roles[item->kind] && (!*p->filter || strstr(p->layout->nodes[node].label,p->filter) ||
            (source>=0 && strstr(model->records[source].reference,p->filter)));
    }
    return 1;
}
static int analyze(GD_ExplorerContext *c) {
    GD_ExplorerPane *p=c->pane; unsigned char shown[GD_MAX_RECORDS]; int i,selected=-1;
    if (p->view==GD_EXPLORER_MDA) gd_explorer_mda_mark(c);
    for (i=0;i<p->layout->node_count;++i) {
        shown[i]=(unsigned char)visible(c,i);
        if (p->view==GD_EXPLORER_MDA ? p->layout->nodes[i].key==p->mda.record : i==p->selection.node) selected=i;
    }
    if (gd_graph_analyze(p->layout,shown,p->view==GD_EXPLORER_MATRIX ? p->overlay:1,p->lens,c->analysis)) p->peer=p->foundation_peer=-1;
    return selected>=0 && shown[selected] ? selected:-1;
}
static void matches(GD_ExplorerContext *c) {
    GD_ExplorerPane *p=c->pane; GD_Model *model=gd_explorer_model(c->project,p->view);
    int i,record=gd_explorer_record(model,p->find_reference),represented=0;
    if (p->view==GD_EXPLORER_MDA) gd_explorer_mda_mark(c);
    memset(p->match_nodes,0,sizeof(p->match_nodes)); memset(p->match_edges,0,sizeof(p->match_edges));
    p->visible_matches=p->hidden_matches=0;
    if (!*p->find_reference || record<0) return;
    for (i=0;i<p->layout->node_count;++i) {
        int source=p->view==GD_EXPLORER_RESOURCES ? gd_explorer_resource_record(c->project->resources,p->layout->nodes[i].key):p->layout->nodes[i].key;
        if (source!=record) continue;
        represented=1;
        if (visible(c,i)) { p->match_nodes[i]=1; ++p->visible_matches; } else ++p->hidden_matches;
    }
    if (p->view==GD_EXPLORER_MATRIX) for (i=0;i<p->layout->edge_count;++i) {
        const GD_LayoutEdge *edge=&p->layout->edges[i]; const GD_Cell *cell=&model->cells[edge->key]; int j,found=0;
        for (j=0;j<cell->pair_count;++j) if (model->pairs[cell->first_pair+j].interaction==record) found=1;
        if (!found) continue;
        represented=1;
        if (edge->active || p->overlay) {
            p->match_edges[i]=1; p->match_nodes[edge->a]=p->match_nodes[edge->b]=1; ++p->visible_matches;
        } else ++p->hidden_matches;
    }
    /* A loaded identity outside a focused neighborhood is hidden by scope, not
       absent. Other projection kinds need not represent this record at all. */
    if (!represented && p->view==GD_EXPLORER_RELATIONSHIPS) ++p->hidden_matches;
}
static void fit_view(GD_ExplorerContext *c, int only_matches) {
    GD_ExplorerPane *p=c->pane; int i,found=0; float left=0,right=1,top=0,bottom=1;
    for (i=0;i<p->layout->node_count;++i) if (visible(c,i) && (!only_matches || p->match_nodes[i])) {
        Vector2 point=position(p,i); float half=p->view==GD_EXPLORER_MDA ? 125:p->view==GD_EXPLORER_RESOURCES ? 110:
            fmaxf(110,(float)MeasureText(p->layout->nodes[i].label,18)/2+25);
        if (!found || point.x-half<left) left=point.x-half;
        if (!found || point.x+half>right) right=point.x+half;
        if (!found || point.y-90<top) top=point.y-90;
        if (!found || point.y+80>bottom) bottom=point.y+80;
        found=1;
    }
    if (!only_matches && p->view!=GD_EXPLORER_MDA && p->view!=GD_EXPLORER_RESOURCES) for (i=0;i<p->layout->edge_count;++i) {
        GD_LayoutRoute route; int j; gd_layout_route(p->layout,i,&route);
        for (j=0;j<route.count;++j) {
            left=fminf(left,route.points[j].x); right=fmaxf(right,route.points[j].x);
            top=fminf(top,route.points[j].y); bottom=fmaxf(bottom,route.points[j].y);
        }
    }
    if (!found) return;
    p->camera.target=(Vector2){(left+right)/2,(top+bottom)/2};
    p->camera.zoom=fminf(c->viewport.width/(right-left+60),c->viewport.height/(bottom-top+60));
    if (p->camera.zoom>1.2f) p->camera.zoom=1.2f; if (p->camera.zoom<0.002f) p->camera.zoom=0.002f;
}
static void draw_matches(GD_ExplorerContext *c) {
    GD_ExplorerPane *p=c->pane; int i;
    BeginScissorMode((int)c->viewport.x,(int)c->viewport.y,(int)c->viewport.width,(int)c->viewport.height); BeginMode2D(p->camera);
    for (i=0;i<p->layout->edge_count;++i) if (p->match_edges[i]) {
        GD_LayoutRoute route; gd_layout_route(p->layout,i,&route); gd_graph_route_line(&route,5,gold,0);
    }
    for (i=0;i<p->layout->node_count;++i) if (p->match_nodes[i]) {
        Vector2 point=position(p,i);
        if (is_graph(p->view)) DrawCircleLines((int)point.x,(int)point.y,47,gold);
        else {
            float half=p->view==GD_EXPLORER_MDA ? 130:115;
            DrawRectangleLinesEx((Rectangle){point.x-half,point.y-60,2*half,120},3,gold);
        }
    }
    EndMode2D(); EndScissorMode();
}
static void source(GD_ExplorerContext *c, int edge) {
    if (is_graph(c->pane->view)) gd_explorer_graph_source(c,edge);
    else if (c->pane->view==GD_EXPLORER_MDA) gd_explorer_mda_source(c,edge);
    else gd_explorer_resource_source(c,edge);
}
static const unsigned char *entries(GD_ExplorerContext *c, unsigned char *roots) {
    GD_ExplorerPane *p=c->pane; GD_Model *model=gd_explorer_model(c->project,p->view); int i;
    if (p->view!=GD_EXPLORER_CHALLENGES) return NULL;
    for (i=0;i<p->layout->node_count;++i) roots[i]=(unsigned char)model->records[p->layout->nodes[i].key].entry;
    return roots;
}
static GD_ExplorerEvidence evidence(GD_ExplorerContext *c, int selected) {
    GD_ExplorerPane *p=c->pane; GD_Model *model=gd_explorer_model(c->project,p->view);
    GD_ExplorerEvidence e={0}; unsigned char roots[GD_MAX_RECORDS];
    e.paths=gd_graph_paths_prepare(c->analysis,&p->paths,entries(c,roots),incidence(p->view));
    if (e.paths && p->paths.compare) e.bypass=gd_graph_path_bypass(c->analysis,selected,-1,GD_GRAPH_ANALYSIS_WORK);
    if (p->foundations) e.foundations=gd_graph_foundations(model,c->analysis,p->view==GD_EXPLORER_RELATIONSHIPS);
    if (e.foundations) e.foundation_pair=gd_graph_closure_choose(c->analysis,selected,&p->foundation_peer);
    e.reachability=gd_graph_reachability_prepare(c->analysis,&p->reach,incidence(p->view));
    if (e.reachability) e.reach_pair=gd_graph_closure_choose_into(c->analysis,&c->analysis->reachability,selected,&p->reach.peer);
    if (p->shapes) {
        if (p->view==GD_EXPLORER_RESOURCES) {
            int roles[GD_MAX_RECORDS],i;
            for (i=0;i<p->layout->node_count;++i) roles[i]=c->project->resources->nodes[p->layout->nodes[i].key].kind;
            e.shapes=gd_graph_shapes(c->analysis,roles,GD_GRAPH_ANALYSIS_WORK);
        } else e.shapes=gd_graph_model_shapes(model,c->analysis);
    }
    e.overview=gd_graph_overview(c->analysis,&p->overview,incidence(p->view));
    return e;
}
static void sources_for_marks(GD_ExplorerContext *c, const unsigned char *marked) {
    int i,shown=0,total=0;
    for (i=0;i<c->pane->layout->edge_count;++i) if (marked[i]) { if (shown++<64) source(c,i); ++total; }
    if (total>64) igText("%d further marked declarations; all remain highlighted",total-64);
}
static void analysis_controls(GD_ExplorerContext *c, int selected) {
    GD_ExplorerPane *p=c->pane; GD_Model *model=gd_explorer_model(c->project,p->view);
    unsigned char roots[GD_MAX_RECORDS]; int pair;
    pair=gd_graph_analysis_panel(p->layout,c->analysis,selected,&p->shared,&p->lens,&p->branches,&p->peer);
    if (pair>=0) { source(c,c->analysis->features[c->analysis->pairs[pair].first_a].edge); source(c,c->analysis->features[c->analysis->pairs[pair].first_b].edge); }
    if (gd_graph_paths_panel(p->layout,c->analysis,&p->paths,selected,entries(c,roots),incidence(p->view))) {
        const GD_PathSelection *support=gd_graph_path_selection(c->analysis,selected); int i;
        for (i=support->retained-1;i>=0;--i) source(c,support->arcs[i]/2);
        if (p->paths.compare) {
            const GD_PathBypass *bypass=gd_graph_path_bypass(c->analysis,selected,-1,GD_GRAPH_ANALYSIS_WORK);
            if (bypass && bypass->complete && bypass->found) for (i=bypass->retained-1;i>=0;--i) source(c,bypass->arcs[i]/2);
        }
    }
    if (gd_graph_foundations_panel(model,p->layout,c->analysis,selected,p->view==GD_EXPLORER_RELATIONSHIPS,&p->foundations,&p->foundation_peer))
        sources_for_marks(c,c->analysis->foundations.comparison.edges);
    if (gd_graph_reachability_panel(p->layout,c->analysis,selected,&p->reach,incidence(p->view))) sources_for_marks(c,c->analysis->reachability.comparison.edges);
    if (p->view==GD_EXPLORER_RESOURCES) {
        igCheckbox("Structural shapes (two exact rounds)",&p->shapes);
        if (p->shapes) {
            int roles[GD_MAX_RECORDS],i; const GD_GraphShapes *shapes;
            for (i=0;i<p->layout->node_count;++i) roles[i]=c->project->resources->nodes[i].kind;
            shapes=gd_graph_shapes(c->analysis,roles,GD_GRAPH_ANALYSIS_WORK);
            igTextWrapped("%d structural families | %s. Not equivalence or flow.",shapes->family_count,shapes->complete ? "complete":"unavailable: work limit");
            if (shapes->complete && igButton("Next repeated family",(ImVec2_c){0,0})) {
                int family=selected>=0 ? shapes->colors[selected]:-1;
                for (i=0;i<shapes->family_count;++i) {
                    int next=(family+1+i)%shapes->family_count;
                    if (shapes->size[next]>1) { p->selection.node=shapes->representative[next]; selected=p->selection.node; break; }
                }
            }
            if (selected>=0 && shapes->complete) {
                int family=shapes->colors[selected];
                igText("Family %d | %d members",family+1,shapes->size[family]);
                if (igTreeNode_Str("Family members")) {
                    for (i=0;i<shapes->node_count;++i) if (shapes->colors[i]==family) {
                        igPushID_Int(i); if (igSmallButton(TextFormat("#%d %s",i,p->layout->nodes[i].label))) p->selection.node=i; igPopID();
                    }
                    igTreePop();
                }
            }
        }
    } else {
        int node=gd_graph_shapes_panel(model,p->layout,c->analysis,selected,&p->shapes);
        if (node>=0) { if (p->view==GD_EXPLORER_MDA) { p->mda.record=p->layout->nodes[node].key; p->mda.link=-1; } else p->navigate=p->layout->nodes[node].key; }
    }
    if (gd_graph_overview_panel(p->view==GD_EXPLORER_RESOURCES ? NULL:model,p->layout,c->analysis,&p->overview,incidence(p->view))) sources_for_marks(c,c->analysis->overview.selected_edges);
    if (p->view==GD_EXPLORER_SKILLS) igTextWrapped("Paths are not AND prerequisite acquisition plans. Known skills are explicit hypothetical assumptions, not mastery.");
    if (p->view==GD_EXPLORER_RESOURCES) igTextWrapped("Resource incidence is not flow. Routes and composed outcomes remain incomplete.");
}
static void resource_input(GD_ExplorerContext *c) {
    GD_ExplorerPane *p=c->pane; Vector2 mouse=GetMousePosition(); int i;
    if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) p->selection.held=-1;
    if (gd_graph_overview_hit(&p->overview,c->viewport,mouse) || igGetIO_Nil()->WantCaptureMouse) { p->selection.held=-1; return; }
    if (!CheckCollisionPointRec(mouse,c->viewport)) return;
    {
        Vector2 world=GetScreenToWorld2D(mouse,p->camera); float wheel=GetMouseWheelMove();
        if (wheel) {
            Vector2 after; p->camera.zoom*=powf(1.15f,wheel);
            if (p->camera.zoom<0.002f) p->camera.zoom=0.002f; if (p->camera.zoom>4) p->camera.zoom=4;
            after=GetScreenToWorld2D(mouse,p->camera); p->camera.target.x+=world.x-after.x; p->camera.target.y+=world.y-after.y;
        }
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) || IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
            Vector2 delta=GetMouseDelta(); p->camera.target.x-=delta.x/p->camera.zoom; p->camera.target.y-=delta.y/p->camera.zoom;
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) for (i=p->layout->node_count-1;i>=0;--i) if (visible(c,i)) {
            Vector2 point=position(p,i);
            if (CheckCollisionPointRec(world,(Rectangle){point.x-90,point.y-24,180,48})) { p->selection.node=p->selection.held=i; p->selection.linked=-1; break; }
        }
        if (p->selection.held>=0 && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) gd_layout_move_node(p->layout,p->selection.held,world.x,world.y);
    }
}
static int choose_view(const GD_ExplorerProject *project, const char *label, int active, int include_current) {
    int v,result=-1;
    if (igBeginCombo(label,include_current ? gd_explorer_view_names[active]:"Choose view",0)) {
        for (v=0;v<GD_EXPLORER_VIEWS;++v) if (include_current || v!=active) {
            int available=(project->available&(1u<<v))!=0;
            if (igSelectable_Bool(gd_explorer_view_names[v],v==active,available ? 0:ImGuiSelectableFlags_Disabled,(ImVec2_c){0,0})) result=v;
            if (!available && igIsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) igSetTooltip("%s",missing_roles((GD_ExplorerView)v));
        }
        igEndCombo();
    }
    return result;
}
static void match_details(GD_ExplorerContext *c) {
    GD_ExplorerPane *p=c->pane; int i,listed=0;
    if (!*p->find_reference) return;
    matches(c);
    igTextWrapped("Find source-associated representations: %s",p->find_reference);
    igText("%d visible matches | %d hidden by filters/scope",p->visible_matches,p->hidden_matches);
    if (!p->visible_matches && !p->hidden_matches) igTextWrapped("No representation in this projection. This is not proof of source absence; relevant data may be unloaded.");
    {
        int record=gd_explorer_record(gd_explorer_model(c->project,p->view),p->find_reference);
        if (record>=0 && igSmallButton("Inspect matched source record")) {
            if (p->view==GD_EXPLORER_MDA) { p->mda.record=record; p->mda.link=-1; }
            else if (is_graph(p->view)) p->navigate=record;
            else p->selection.linked=record;
        }
    }
    if (p->view==GD_EXPLORER_RESOURCES && c->project->resources->limited) igTextWrapped("Resource projection is partial; counts cover retained representations only.");
    if (igSmallButton("Clear Find highlights")) { p->find_reference[0]=0; matches(c); }
    if (p->visible_matches && igTreeNode_Str("Inspect visible matches")) {
        GD_Model *model=gd_explorer_model(c->project,p->view);
        for (i=0;i<p->layout->node_count && listed<64;++i) if (p->match_nodes[i]) {
            if (p->view==GD_EXPLORER_MATRIX && strcmp(model->records[p->layout->nodes[i].key].reference,p->find_reference)) continue;
            igPushID_Int(i);
            if (igSmallButton(p->layout->nodes[i].label)) {
                if (p->view==GD_EXPLORER_MDA) { p->mda.record=p->layout->nodes[i].key; p->mda.link=-1; }
                else {
                    GD_ExplorerSelection choice={-1,i,0,-1,-1};
                    if (is_graph(p->view)) p->navigate=p->layout->nodes[i].key; else p->selection=choice;
                }
            }
            igPopID(); ++listed;
        }
        if (p->view==GD_EXPLORER_MATRIX) for (i=0;i<p->layout->edge_count && listed<64;++i) if (p->match_edges[i]) {
            int key=p->layout->edges[i].key; const GD_Cell *cell=&model->cells[key];
            igPushID_Int(GD_MAX_RECORDS+i);
            if (igSmallButton(TextFormat("%s x %s (case cell)",model->records[cell->noun].name,model->records[cell->verb].name))) {
                int j; GD_ExplorerSelection choice={key,-1,0,-1,-1};
                for (j=0;j<cell->pair_count;++j) if (!strcmp(model->records[model->pairs[cell->first_pair+j].interaction].reference,p->find_reference)) choice.interaction=j;
                gd_explorer_graph_select(c,&choice);
            }
            igPopID(); ++listed;
        }
        if (listed==64) igTextWrapped("List capped at 64 representations; graph highlights all visible matches.");
        igTreePop();
    }
}
static int layout_controls(GD_ExplorerContext *c) {
    GD_ExplorerPane *p=c->pane; GD_Model *model=gd_explorer_model(c->project,p->view); int rearrange=0;
    if (p->view==GD_EXPLORER_SKILLS) rearrange=igCombo_Str("Layout",&p->rank,"Prerequisite depth\0Authored rank constraints\0",-1);
    else if (p->view==GD_EXPLORER_MATRIX) rearrange=igCombo_Str("Layout",&p->layout_choice,"Free graph\0Noun / verb columns\0",-1);
    else if (p->view==GD_EXPLORER_MDA) rearrange=igCombo_Str("Layout",&p->layout_choice,"Role bands\0Free graph\0",-1);
    else igTextWrapped(p->view==GD_EXPLORER_CHALLENGES ? "Layout: declared route bands":p->view==GD_EXPLORER_RELATIONSHIPS ? "Layout: focused consumed declarations":"Layout: free incidence graph");
    if (p->view==GD_EXPLORER_SKILLS || p->view==GD_EXPLORER_CHALLENGES) if (igCombo_Str("Direction",&p->horizontal,"Top to bottom\0Left to right\0",-1)) rearrange=1;
    if (is_graph(p->view) && igCombo_Str("Sort / initial order",&p->sort,"Name\0Shown degree (descending)\0",-1)) rearrange=1;
    if (p->view==GD_EXPLORER_CHALLENGES || p->view==GD_EXPLORER_RELATIONSHIPS || p->layout->mode==GD_LAYOUT_COLUMNS)
        igTextWrapped("Stable layout; settling applies only to movable graph layouts.");
    else { igCheckbox("Pause settling",&p->paused); igSameLine(0,-1); }
    if (igButton("Fit (F)",(ImVec2_c){0,0})) p->fit=1;
    igSameLine(0,-1); if (igButton("Rearrange",(ImVec2_c){0,0})) rearrange=1;
    if (p->view==GD_EXPLORER_MATRIX) igCheckbox("Overlay other pairs (does not merge groups)",&p->overlay);
    if (p->view==GD_EXPLORER_MDA) {
        int direction=c->project->mda->direction,i;
        if (igCombo_Str("Traversal",&direction,"Both / associations\0Forward / toward aesthetics\0Backward / authored basis\0",-1))
            gd_mda_query(model,c->project->mda,c->project->mda->root,(GD_MdaDirection)direction);
        igTextWrapped("Query root: %s",c->project->mda->root>=0 ? model->records[c->project->mda->root].reference:"all supplied nodes");
        if (p->mda.record>=0 && gd_mda_role(model->records[p->mda.record].role) && igButton("Use selected as query root",(ImVec2_c){0,0}))
            gd_mda_query(model,c->project->mda,p->mda.record,c->project->mda->direction);
        if (igButton("Show all query nodes",(ImVec2_c){0,0})) gd_mda_query(model,c->project->mda,-1,c->project->mda->direction);
        igCheckbox("Only query nodes (display only)",&p->mda.only_scope);
        igCheckbox("Only selected witness (display only)",&p->mda.only_path);
        for (i=0;i<4;++i) igCheckbox(gd_mda_role_name((GD_Role)(GD_MDA_MECHANIC+i)),&p->mda.roles[i]);
        igInputText("Reference filter",p->mda.filter,sizeof(p->mda.filter),0,NULL,NULL);
    } else if (p->view==GD_EXPLORER_RESOURCES) {
        igCheckbox("Rule cases",&p->mda.roles[GD_RESOURCE_CASE]); igSameLine(0,-1);
        igCheckbox("Actor facets",&p->mda.roles[GD_RESOURCE_FACET]);
        igCheckbox("Resource roles",&p->mda.roles[GD_RESOURCE_ROLE]);
        igInputText("Name / source filter",p->filter,sizeof(p->filter),0,NULL,NULL);
    }
    return rearrange;
}
static int reload_project(GD_ExplorerContext *c, GD_ExplorerPane *panes, const GD_Collection *collections, int count,
                          char *failure, size_t failure_size) {
    GD_ExplorerProject *old=c->project,*fresh=gd_explorer_load(old->allocator,collections,count);
    GD_ExplorerPane *replacement=NULL; GD_ExplorerContext candidate=*c;
    const GD_Error *error=gd_explorer_error(fresh);
    int v,ready=fresh && fresh->available==old->available && !error;
    if (ready) {
        replacement=old->allocator.allocate(old->allocator.context,GD_EXPLORER_VIEWS*sizeof(*replacement));
        ready=replacement!=NULL;
    }
    if (replacement) {
        memcpy(replacement,panes,GD_EXPLORER_VIEWS*sizeof(*replacement));
        for (v=0;v<GD_EXPLORER_VIEWS;++v) { replacement[v].layout=NULL; memset(&replacement[v].diagnostics,0,sizeof(replacement[v].diagnostics)); }
        candidate.project=fresh;
        if (old->mda) {
            int root=-1;
            if (old->mda->root>=0) {
                GD_Model *model=gd_explorer_model(old,GD_EXPLORER_MDA);
                root=gd_explorer_record(gd_explorer_model(fresh,GD_EXPLORER_MDA),model->records[old->mda->root].reference);
                if (root<0) ready=0;
            }
            if (ready) ready=gd_mda_query(gd_explorer_model(fresh,GD_EXPLORER_MDA),fresh->mda,root,old->mda->direction);
        }
        for (v=0;v<GD_EXPLORER_VIEWS && ready;++v) if (panes[v].layout) {
            GD_ExplorerPane *p=&replacement[v]; const GD_ExplorerPane *prior=&panes[v];
            GD_Model *model=gd_explorer_model(old,(GD_ExplorerView)v),*next=gd_explorer_model(fresh,(GD_ExplorerView)v);
            GD_Focus focus={0}; int record,root=-1,j;
            c->pane=&panes[v]; record=selected_record(c);
            record=record>=0 ? gd_explorer_record(next,model->records[record].reference):-1;
            if (v==GD_EXPLORER_RELATIONSHIPS && prior->focus.root>=0) {
                root=gd_explorer_record(next,model->records[prior->focus.root].reference);
                if (root<0) { ready=0; break; }
            }
            gd_focus_reset(next,&focus,root);
            if (v==GD_EXPLORER_RELATIONSHIPS) for (j=0;j<model->record_count;++j) if (prior->focus.visible[j] || prior->focus.expanded[j]) {
                int mapped=gd_explorer_record(next,model->records[j].reference);
                if (mapped>=0) { focus.visible[mapped]=prior->focus.visible[j]; if (prior->focus.expanded[j]) gd_focus_expand(next,&focus,mapped); }
            }
            p->focus=focus;
            memset(p->known,0,sizeof(p->known));
            for (j=0;j<model->record_count;++j) if (prior->known[j]) {
                int mapped=gd_explorer_record(next,model->records[j].reference); if (mapped>=0) p->known[mapped]=1;
            }
            if (v==GD_EXPLORER_RESOURCES) {
                GD_PathControls mapped=p->paths; mapped.count=0;
                for (j=0;j<prior->paths.count;++j) {
                    int key=gd_explorer_resource_remap(old->resources,fresh->resources,prior->paths.keys[j]);
                    if (key>=0) gd_graph_path_start(&mapped,key); else ++mapped.lost;
                }
                p->paths=mapped;
            } else gd_graph_paths_remap(&p->paths,model,next);
            p->selection=(GD_ExplorerSelection){-1,-1,0,-1,-1};
            if (v==GD_EXPLORER_RESOURCES) {
                p->selection.node=gd_explorer_resource_remap(old->resources,fresh->resources,prior->selection.node);
                if (prior->selection.linked>=0) p->selection.linked=record;
            }
            else if (is_graph((GD_ExplorerView)v)) p->selection.linked=record;
            p->editor.record=-1; p->mda.record=record; p->mda.link=p->mda.held=-1;
            p->witness=p->peer=p->foundation_peer=p->reach.peer=p->reach.target=-1;
            p->prepared=0; p->revision=0;
            candidate.pane=p;
            if (!pane_prepare(&candidate)) ready=0;
        }
        /* Preparing every visited layout is part of candidate validation. Never
           release last-good snapshots before all replacement allocations pass. */
        for (v=0;v<GD_EXPLORER_FAMILIES && ready;++v) if (fresh->models[v] && gd_sources_changed(fresh->models[v])) ready=0;
    }
    if (ready) {
        for (v=0;v<GD_EXPLORER_VIEWS;++v) {
            gd_diagnostics_clear(&panes[v].diagnostics);
            if (panes[v].layout) old->allocator.release(old->allocator.context,panes[v].layout);
        }
        memcpy(panes,replacement,GD_EXPLORER_VIEWS*sizeof(*panes));
        old->allocator.release(old->allocator.context,replacement);
        c->project=fresh; c->analysis->initialized=0; gd_explorer_destroy(old); *failure=0;
        return 1;
    }
    snprintf(failure,failure_size,"Reload rejected: %s. Last good snapshot retained.",error ? error->message:
             "Cannot prepare replacement, sources changed, or a query/focus root disappeared; clear root before retry");
    if (replacement) {
        for (v=0;v<GD_EXPLORER_VIEWS;++v) if (replacement[v].layout) old->allocator.release(old->allocator.context,replacement[v].layout);
        old->allocator.release(old->allocator.context,replacement);
    }
    gd_explorer_destroy(fresh); return 0;
}
static int change_view(GD_ExplorerContext *c, GD_ExplorerPane *panes, int target, const char *find_reference) {
    GD_ExplorerPane *p=c->pane;
    if (target<0 || target>=GD_EXPLORER_VIEWS || !(c->project->available&(1u<<target))) return 0;
    if (is_graph(p->view) && gd_explorer_graph_commit(c)<0) return 0;
    p->selection.held=p->mda.held=-1; p->right_started=0;
    c->pane=&panes[target];
    if (find_reference) { snprintf(c->pane->find_reference,sizeof(c->pane->find_reference),"%s",find_reference); c->pane->fit=2; }
    c->analysis->initialized=0; return 1;
}
static int center_relationships(GD_ExplorerContext *c, GD_ExplorerPane *panes, int source) {
    GD_Model *from=gd_explorer_model(c->project,c->pane->view),*target;
    int root;
    if (!(c->project->available&(1u<<GD_EXPLORER_RELATIONSHIPS)) || source<0 || source>=from->record_count) return 0;
    target=gd_explorer_model(c->project,GD_EXPLORER_RELATIONSHIPS);
    root=gd_explorer_record(target,from->records[source].reference);
    if (root<0 || !change_view(c,panes,GD_EXPLORER_RELATIONSHIPS,NULL)) return 0;
    gd_focus_reset(target,&c->pane->focus,root);
    c->pane->selection=(GD_ExplorerSelection){-1,-1,0,root,-1};
    c->pane->inspected_record=root; c->pane->prepared=0; c->pane->fit=1;
    return 1;
}
static int reset_view(GD_ExplorerContext *c) {
    GD_ExplorerPane *p=c->pane; GD_ExplorerView view=p->view;
    if (is_graph(view) && gd_explorer_graph_commit(c)<0) return 0;
    if (view==GD_EXPLORER_MDA) gd_mda_query(gd_explorer_model(c->project,view),c->project->mda,-1,GD_MDA_BOTH);
    pane_defaults(p,view); c->analysis->initialized=0; return 1;
}
static int undo_draft(GD_ExplorerContext *c, GD_ExplorerPane *panes) {
    if (!c->project->draft || !gd_draft_undo(c->project->draft)) return 0;
    panes[GD_EXPLORER_SKILLS].editor.record=-1;
    gd_explorer_edited(c->project); return 1;
}
static int finish(GD_ExplorerContext *c, const char *output) {
    int applied=0;
    if (c->project->draft) {
        /* Caller supplies Skills context even when closing from another view. */
        applied=gd_explorer_graph_commit(c);
        if (applied<0) return -1;
        return gd_draft_finish(c->project->draft,output && *output ? output:NULL);
    }
    return 0;
}
static void copy_error(GD_Model *diagnostic, const GD_Error *error) {
    const char *parts[3]={error->code,error->message,error->path}; const char *owned[3]={0};
    size_t total=0; int i;
    for (i=0;i<3;++i) if (parts[i]) total+=strlen(parts[i])+1;
    if (total>GD_TEXT_CAPACITY-diagnostic->text_used) {
        gd_error(diagnostic,"explorer.load","Explorer load failed; diagnostic text capacity exhausted",NULL,0); return;
    }
    for (i=0;i<3;++i) if (parts[i]) {
        size_t length=strlen(parts[i])+1; char *target=diagnostic->text+diagnostic->text_used;
        memcpy(target,parts[i],length); diagnostic->text_used+=length; owned[i]=target;
    }
    gd_error(diagnostic,owned[0],owned[1],owned[2],error->line);
}
int gd_explorer_view(GD_Model *diagnostic, const GD_Collection *collections, int count,
                     int initial_view, int frames, const char *screenshot, const char *output_path) {
    GD_Allocator allocator=diagnostic->allocator;
    GD_ExplorerProject *project=gd_explorer_load(allocator,collections,count);
    GD_ExplorerPane *panes=NULL;
    GD_ExplorerContext c={0};
    int active=0,v,frame=0,result=0,stale=0,close_seen=0,timed_seen=0,close_next=0;
    double next_check=0; char failure[GD_MAX_PATH+512]="",report_path[GD_MAX_PATH]="";
    const GD_Error *error=gd_explorer_error(project);
    if (!project || !project->available || error || (initial_view>=0 && (initial_view>=GD_EXPLORER_VIEWS || !(project->available&(1u<<initial_view))))) {
        if (error) copy_error(diagnostic,error);
        else gd_error(diagnostic,project ? "explorer.unavailable":"explorer.memory",!project ? "Cannot allocate explorer project":
            initial_view>=0 ? "Requested initial projection is unavailable; supply its explicit collection roles":"No complete projection roles supplied",NULL,0);
        gd_explorer_destroy(project); return 2;
    }
    panes=allocator.allocate(allocator.context,GD_EXPLORER_VIEWS*sizeof(*panes));
    if (panes) memset(panes,0,GD_EXPLORER_VIEWS*sizeof(*panes));
    c.analysis=allocator.allocate(allocator.context,sizeof(*c.analysis));
    c.query=allocator.allocate(allocator.context,sizeof(*c.query));
    if (!panes || !c.analysis || !c.query) { gd_error(diagnostic,"explorer.memory","Cannot allocate bounded explorer view state",NULL,0); result=2; goto cleanup; }
    memset(c.analysis,0,sizeof(*c.analysis));
    for (v=0;v<GD_EXPLORER_VIEWS;++v) pane_defaults(&panes[v],(GD_ExplorerView)v);
    if (initial_view>=0) active=initial_view;
    else while (!(project->available&(1u<<active))) ++active;
    if (output_path) snprintf(report_path,sizeof(report_path),"%s",output_path);
    c.project=project; c.pane=&panes[active];
    if (!open_window("Game Design Workbench - Explorer")) { gd_error(diagnostic,"view.window","Cannot initialize explorer window",NULL,0); result=2; goto cleanup; }
    for (;;) {
        int closing=WindowShouldClose(),width=GetScreenWidth(),height=GetScreenHeight(),split=width*60/100;
        int requested=-1,find_view=-1,reset=0,refresh=0,selected,relationship_root=-1,undo=0;
        char find_reference[260]="";
        GD_ExplorerPane *p=&panes[active]; GD_Model *model=gd_explorer_model(project,p->view);
        c.pane=p; c.viewport=(Rectangle){14,112,(float)split-28,(float)height-157};
        if (!pane_prepare(&c)) { gd_error(diagnostic,"explorer.projection","Cannot prepare bounded projection layout",NULL,0); result=2; break; }
        if (close_next || (closing && !close_seen) || (frames && frame>=frames && !timed_seen)) {
            GD_ExplorerPane *saved=c.pane; int reported;
            c.pane=&panes[GD_EXPLORER_SKILLS]; reported=finish(&c,report_path); c.pane=saved;
            timed_seen=1; close_next=0;
            if (reported>=0) { result=reported; break; }
            snprintf(failure,sizeof(failure),"%s",project->draft ? project->draft->message:"Cannot finish explorer");
            if (frames && project->draft && !project->draft->modified_count) {
                GD_Error report_error={"draft.report",project->draft->message,output_path,0}; copy_error(diagnostic,&report_error); result=2; break;
            }
        }
        close_seen=closing;
        if (GetTime()>=next_check) {
            for (v=0;v<GD_EXPLORER_FAMILIES;++v) if (project->models[v] && gd_sources_changed(project->models[v])) stale=1;
            next_check=GetTime()+1;
        }
        BeginDrawing(); ClearBackground(background); rlImGuiBegin();
        DrawText("Game Design Workbench",20,14,26,foreground);
        DrawText(TextFormat("%s | %d projected identities | %d projected links | positions transient",gd_explorer_view_names[active],p->layout->node_count,p->layout->edge_count),20,49,16,muted);
        DrawText(project->draft && project->draft->modified_count ? TextFormat("DRAFT: %d records modified in memory | source files unchanged",project->draft->modified_count):"Source-backed projections | Filters define shown analysis | No source writes",20,77,15,project->draft && project->draft->modified_count ? gold:muted);
        igSetNextWindowPos((ImVec2_c){(float)split,12},ImGuiCond_Always,(ImVec2_c){0,0});
        igSetNextWindowSize((ImVec2_c){(float)(width-split-12),(float)(height-24)},ImGuiCond_Always);
        igBegin("Explorer controls and source",NULL,ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoSavedSettings);
        requested=choose_view(project,"Data / projection",active,1);
        if (igButton("RESET CURRENT VIEW TO DEFAULTS",(ImVec2_c){-1,34})) reset=1;
        igTextWrapped("Reset affects this view only. Draft/source data and other views stay intact.");
        selected=selected_record(&c);
        if (selected>=0) {
            find_view=choose_view(project,"Find selected in",active,0);
            if (find_view>=0) snprintf(find_reference,sizeof(find_reference),"%s",model->records[selected].reference);
        } else igTextWrapped(p->view==GD_EXPLORER_RESOURCES && p->selection.node>=0 ?
            "Selected derived item has no unique loaded source. Inspect incident declarations; Find is unavailable.":
            "Select source to enable Find selected in another view.");
        match_details(&c);
        if (layout_controls(&c)) { p->prepared=0; pane_prepare(&c); p->fit=1; }
        selected=analyze(&c);
        if (igCollapsingHeader_TreeNodeFlags("Analysis layers",0)) analysis_controls(&c,selected);
        refresh=igButton("Refresh sources (F5)",(ImVec2_c){0,0});
        if (stale) igTextWrapped("STALE: source changed or could not be checked. Last good snapshot retained.");
        if (*failure) igTextWrapped("%s",failure);
        if (project->draft) {
            igText("%d modified records | %d undo steps",project->draft->modified_count,project->draft->history_count);
            if (igButton("Undo last edit (Ctrl+Z)",(ImVec2_c){0,0})) undo=1;
            igTextUnformatted("Report path (empty: stdout; new .json file only)",NULL); igSetNextItemWidth(-1);
            igInputText("##explorer-report-path",report_path,sizeof(report_path),0,NULL,NULL);
            if (igButton("Finish and report changes",(ImVec2_c){0,0})) close_next=1;
            if (*project->draft->message) igTextWrapped("%s",project->draft->message);
        }
        if (igTreeNode_Str("Browse shown nodes")) {
            int i;
            if (is_graph(p->view)) igInputText("Browse filter (list only)",p->filter,sizeof(p->filter),0,NULL,NULL);
            igBeginChild_Str("Explorer records",(ImVec2_c){0,120},0,0);
            for (i=0;i<p->layout->node_count;++i) if (visible(&c,i)) {
                int record=p->view==GD_EXPLORER_RESOURCES ? gd_explorer_resource_record(project->resources,p->layout->nodes[i].key):p->layout->nodes[i].key;
                const char *reference=record>=0 ? model->records[record].reference:"no unique loaded source";
                if (is_graph(p->view) && *p->filter && !strstr(p->layout->nodes[i].label,p->filter) && !strstr(reference,p->filter)) continue;
                igPushID_Int(i);
                if (igSmallButton(p->view==GD_EXPLORER_RESOURCES ? TextFormat("#%d %s | %s",i,p->layout->nodes[i].label,reference):reference)) {
                    if (p->view==GD_EXPLORER_MDA) { p->mda.record=record; p->mda.link=-1; }
                    else if (p->view==GD_EXPLORER_RESOURCES) p->selection=(GD_ExplorerSelection){-1,i,0,-1,-1};
                    else p->navigate=record;
                }
                igPopID();
            }
            igEndChild(); igTreePop();
        }
        if (p->view==GD_EXPLORER_SKILLS && igCollapsingHeader_TreeNodeFlags("Skill opportunities / knowledge",0)) gd_explorer_graph_unlocks(&c);
        {
            const GD_Finding *findings=p->view==GD_EXPLORER_MDA ? project->mda->findings:project->rules && p->view==GD_EXPLORER_RESOURCES ? project->rules->findings:model->findings;
            int count_findings=p->view==GD_EXPLORER_MDA ? project->mda->finding_count:p->view==GD_EXPLORER_RESOURCES ? project->rules->finding_count:model->finding_count;
            int omitted=p->view==GD_EXPLORER_MDA ? project->mda->findings_omitted:p->view==GD_EXPLORER_RESOURCES ? project->rules->findings_omitted:model->findings_omitted;
            int target=gd_diagnostics_panel(&p->diagnostics,model,findings,count_findings,omitted,selected_record(&c));
            if (target>=0) {
                if (p->view==GD_EXPLORER_MDA) { p->mda.record=target; p->mda.link=-1; }
                else if (is_graph(p->view)) p->navigate=target;
                else { p->selection=(GD_ExplorerSelection){-1,-1,0,target,-1}; }
            }
        }
        igSeparator();
        if (is_graph(p->view)) { if (gd_explorer_graph_details(&c)) gd_explorer_edited(project); }
        else if (p->view==GD_EXPLORER_MDA) gd_explorer_mda_details(&c);
        else gd_explorer_resource_details(&c);
        igEnd();
        {
            Vector2 mouse=GetMousePosition();
            if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && CheckCollisionPointRec(mouse,c.viewport) && !igGetIO_Nil()->WantCaptureMouse) {
                p->right_origin=mouse; p->right_started=1;
            }
            if (p->right_started==1 && IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
                float dx=mouse.x-p->right_origin.x,dy=mouse.y-p->right_origin.y;
                if (dx*dx+dy*dy>16) p->right_started=2;
            }
            if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT) && p->right_started) {
                float dx=mouse.x-p->right_origin.x,dy=mouse.y-p->right_origin.y;
                if (p->right_started==1 && dx*dx+dy*dy<=16 && CheckCollisionPointRec(mouse,c.viewport)) igOpenPopup_Str("Find selected context",0);
                p->right_started=0;
            }
            if (igBeginPopup("Find selected context",0)) {
                int record=selected_record(&c);
                igTextUnformatted("Find selected in",NULL);
                if (record<0) igTextWrapped("Select a source-backed item first.");
                else for (v=0;v<GD_EXPLORER_VIEWS;++v) if (v!=active) {
                    int available=(project->available&(1u<<v))!=0;
                    if (igSelectable_Bool(gd_explorer_view_names[v],false,available ? 0:ImGuiSelectableFlags_Disabled,(ImVec2_c){0,0})) {
                        find_view=v; snprintf(find_reference,sizeof(find_reference),"%s",model->records[record].reference);
                    }
                    if (!available && igIsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) igSetTooltip("%s",missing_roles((GD_ExplorerView)v));
                }
                igEndPopup();
            }
        }
        if (!igGetIO_Nil()->WantCaptureKeyboard) {
            if (IsKeyPressed(KEY_F)) p->fit=1;
            if (IsKeyPressed(KEY_F5)) refresh=1;
            if (IsKeyPressed(KEY_SPACE) && p->layout->mode!=GD_LAYOUT_COLUMNS && active!=GD_EXPLORER_CHALLENGES && active!=GD_EXPLORER_RELATIONSHIPS) p->paused=!p->paused;
        }
        if (!igGetIO_Nil()->WantTextInput && (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyPressed(KEY_Z)) undo=1;
        /* Both Undo inputs run after inspector readers. Repair revision before
           another selection reader can reinterpret compacted edge offsets. */
        if (undo) undo_draft(&c,panes);
        if (p->navigate>=0 && is_graph(p->view) && gd_explorer_graph_commit(&c)<0) p->navigate=-1;
        if (p->navigate>=0) {
            int i;
            if ((active==GD_EXPLORER_SKILLS && model->records[p->navigate].role==GD_CHALLENGE) ||
                (active==GD_EXPLORER_CHALLENGES && model->records[p->navigate].role==GD_SKILL)) {
                find_view=active==GD_EXPLORER_SKILLS ? GD_EXPLORER_CHALLENGES:GD_EXPLORER_SKILLS;
                snprintf(find_reference,sizeof(find_reference),"%s",model->records[p->navigate].reference);
            }
            p->selection=(GD_ExplorerSelection){-1,-1,0,-1,-1};
            for (i=0;i<p->layout->node_count;++i) if (p->layout->nodes[i].key==p->navigate) p->selection.node=i;
            if (p->selection.node<0) p->selection.linked=p->navigate;
            if (is_graph(p->view)) { GD_ExplorerSelection choice=p->selection; gd_explorer_graph_select(&c,&choice); }
            p->navigate=-1;
        }
        if (p->focus_root>=0) { relationship_root=p->focus_root; p->focus_root=-1; }
        if (p->focus_expand>=0) { gd_focus_expand(model,&p->focus,p->focus_expand); p->focus_expand=-1; p->prepared=0; p->fit=1; }
        if (!pane_prepare(&c)) { gd_error(diagnostic,"explorer.projection","Cannot update bounded projection layout",NULL,0); result=2; rlImGuiEnd(); EndDrawing(); break; }
        selected=analyze(&c); matches(&c);
        p->camera.offset=(Vector2){c.viewport.x+c.viewport.width/2,c.viewport.y+c.viewport.height/2};
        if (p->fit) { fit_view(&c,p->fit==2); p->fit=0; }
        if (is_graph(p->view)) gd_explorer_graph_input(&c);
        else if (p->view==GD_EXPLORER_MDA) gd_explorer_mda_input(&c);
        else resource_input(&c);
        if (!p->paused && p->view!=GD_EXPLORER_CHALLENGES && p->view!=GD_EXPLORER_RELATIONSHIPS) {
            int held=p->view==GD_EXPLORER_MDA ? p->mda.held:p->selection.held;
            gd_layout_step(p->layout,held);
        }
        selected=analyze(&c);
        {
            GD_ExplorerEvidence e=evidence(&c,selected);
            if (is_graph(p->view)) gd_explorer_graph_draw(&c,&e);
            else if (p->view==GD_EXPLORER_MDA) gd_explorer_mda_draw(&c,&e);
            else gd_explorer_resource_draw(&c,&e);
        }
        draw_matches(&c); gd_graph_overview_draw(c.analysis,&p->overview,c.viewport,!igGetIO_Nil()->WantCaptureMouse);
        DrawText("Drag: layout | Right click: Find | Right/middle drag: pan | Wheel: zoom | F: fit | F5: refresh",20,height-36,14,muted);
        DrawText(incidence(p->view) ? "Shown incidence only; not flow, causality or equivalence.":"Authored declarations only; not learning, causality or physical reachability.",20,height-18,13,muted);
        rlImGuiEnd(); EndDrawing(); ++frame;
        if (screenshot && frame==frames && !gd_save_screenshot(screenshot)) { gd_error(diagnostic,"view.screenshot","Explorer screenshot export failed",screenshot,0); result=2; break; }
        if (reset || requested>=0 || find_view>=0 || relationship_root>=0) {
            int changed=reset ? reset_view(&c):relationship_root>=0 ? center_relationships(&c,panes,relationship_root):
                find_view>=0 ? change_view(&c,panes,find_view,find_reference):change_view(&c,panes,requested,NULL);
            if (!changed) snprintf(failure,sizeof(failure),"Cannot leave/reset view: %s",project->draft ? project->draft->message:"unavailable view");
            active=c.pane->view;
        }
        if (refresh) {
            c.pane=&panes[GD_EXPLORER_SKILLS];
            if (gd_explorer_graph_commit(&c)<0 || (project->draft && project->draft->modified_count))
                snprintf(failure,sizeof(failure),"Refresh blocked: draft modified or editor invalid. Undo to disk sync point or finish/report first.");
            else {
                stale=!reload_project(&c,panes,collections,count,failure,sizeof(failure));
                project=c.project;
            }
            c.pane=&panes[active];
        }
    }
    rlImGuiShutdown(); CloseWindow();
cleanup:
    if (panes) {
        for (v=0;v<GD_EXPLORER_VIEWS;++v) {
            gd_diagnostics_clear(&panes[v].diagnostics);
            if (panes[v].layout) allocator.release(allocator.context,panes[v].layout);
        }
        allocator.release(allocator.context,panes);
    }
    if (c.query) allocator.release(allocator.context,c.query);
    if (c.analysis) allocator.release(allocator.context,c.analysis);
    gd_explorer_destroy(project); return result;
}
#ifdef GD_EXPLORER_TEST
#include "test_explorer_view.h"
#endif
