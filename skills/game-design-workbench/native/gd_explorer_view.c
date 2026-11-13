#include "gd_explorer_controls.h"
#include "gd_render_pose.h"
#include "gd_sidebar.h"
#include "gd_graph_analysis_draw.h"
#include "gd_graph_overview_draw.h"
#include "rlImGui.h"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_NO_EXPORT
#include "cimgui.h"
#include "gd_explorer_picker.h"
#include <math.h>
#include <stdarg.h>
#include <string.h>

static const Color background={18,24,34,255}, foreground={225,233,241,255}, muted={150,169,190,255}, gold={255,217,125,255};
static void explorer_log(int level, const char *text, va_list arguments) {
    (void)level; vfprintf(stderr,text,arguments); fputc('\n',stderr);
}
static int sidebar_section(GD_Sidebar *sidebar,GD_SidebarSection section) {
    unsigned bit=1u<<section;int before=(sidebar->open&bit)!=0,open;
    /* Skipped/minimized windows return false without changing their tree state. */
    if(igGetCurrentWindow()->SkipItems)return 0;
    igSetNextItemOpen(before,ImGuiCond_Always);
    open=igCollapsingHeader_TreeNodeFlags(gd_sidebar_labels[section],0);
    if(igIsItemToggledOpen() && open!=before){if(open)sidebar->open|=bit;else sidebar->open&=~bit;gd_sidebar_save(sidebar);}
    return open;
}
static int open_window(const char *title) {
    SetTraceLogCallback(explorer_log); SetTraceLogLevel(LOG_WARNING); SetConfigFlags(FLAG_WINDOW_RESIZABLE|FLAG_MSAA_4X_HINT);
    InitWindow(1500,940,title);
    if (!IsWindowReady()) return 0;
    SetWindowMinSize(1000,700); SetExitKey(KEY_NULL); SetTargetFPS(60); rlImGuiSetup(true);
    igGetIO_Nil()->IniFilename=NULL; igGetIO_Nil()->LogFilename=NULL; igGetStyle()->FontScaleMain=1.1f;
    return 1;
}
#include "gd_noun_grid_view.h"
#include "gd_matrix_grid_view.h"
static int is_graph(GD_ExplorerView view) { return view<=GD_EXPLORER_MATRIX; }
static int query_view(GD_ExplorerView view) { return view==GD_EXPLORER_ACTORS || view==GD_EXPLORER_EFFECTS; }
static void pane_release_layout(GD_Allocator allocator, GD_ExplorerPane *pane) {
    gd_explorer_structure_destroy(pane->structure_state); pane->structure_state=NULL;
    gd_explorer_rules_destroy(pane->rule_state); pane->rule_state=NULL;
    if (pane->query_state) gd_explorer_query_destroy(pane->query_state);
    else if (pane->layout) allocator.release(allocator.context,pane->layout);
    pane->query_state=NULL; pane->layout=NULL;
}
static int incidence(GD_ExplorerView view) { return view==GD_EXPLORER_MATRIX || view==GD_EXPLORER_RESOURCES; }
static int free_graph(const GD_ExplorerPane *p) {
    return p->layout && p->layout->mode==GD_LAYOUT_FORCE && (p->view!=GD_EXPLORER_RELATIONSHIPS || p->layout_choice==GD_LAYOUT_FORCE) &&
        (!query_view(p->view) || (p->query_state && p->query_state->scene && (p->layout_choice==GD_LAYOUT_FORCE || gd_query_scene_overview_mode(p->query_state->scene)==GD_ACTOR_INTERACTIONS)));
}
static int layout_settles(const GD_ExplorerPane *p) {
    return p->layout && !grid_view(p) && (!query_view(p->view) || free_graph(p)) && p->layout->mode!=GD_LAYOUT_COLUMNS &&
        ((p->view!=GD_EXPLORER_CHALLENGES && p->view!=GD_EXPLORER_RELATIONSHIPS) || free_graph(p));
}
static int render_active(const GD_ExplorerContext *c) {
    const GD_ExplorerPane *p=c->pane;return p->visual_filter && free_graph(p) && !grid_view(p) && !gd_explorer_similarity_active(c) && !p->distance_focus_enabled && !(p->distance_focus && p->distance_focus->animating);
}
static const void *render_identity(const GD_ExplorerContext *c) {return query_view(c->pane->view) ? (const void *)c->pane->query_state->scene->free_geometry:(const void *)c->pane;}
static void render_update(GD_ExplorerContext *c,GD_RenderPose *pose,double seconds,int reset) {
    GD_ExplorerPane *p=c->pane;int i,held=p->view==GD_EXPLORER_MDA ? p->mda.held:p->selection.held;
    for(i=0;i<p->layout->node_count;++i)pose->anchors[i]=query_view(p->view) ? gd_query_scene_anchor(p->query_state->scene,i):i;
    if(gd_render_pose_update(pose,p->layout,render_identity(c),held,seconds,reset))c->display_layout=&pose->layout;
    else {pose->valid=0;c->display_layout=NULL;snprintf(p->query_message,sizeof(p->query_message),"Cannot prepare visual filter; showing raw pose.");}
}
static const char *missing_roles(GD_ExplorerView view) {
    switch (view) {
        case GD_EXPLORER_SKILLS: case GD_EXPLORER_RELATIONSHIPS: return "Supply --skill-atoms ALIAS=FOLDER";
        case GD_EXPLORER_CHALLENGES: return "Supply --skill-atoms and --challenges ALIAS=FOLDER";
        case GD_EXPLORER_NOUNS: return "Supply --nouns and --interactions ALIAS=FOLDER";
        case GD_EXPLORER_MATRIX: return "Supply --nouns, --verbs and --interactions ALIAS=FOLDER";
        case GD_EXPLORER_MDA: return "Supply --mechanics, --dynamics and --aesthetics ALIAS=FOLDER";
        case GD_EXPLORER_ACTORS: return "Supply --nouns, --axes and --interactions and/or --mechanics ALIAS=FOLDER";
        default: return "Supply --interactions and/or --mechanics ALIAS=FOLDER";
    }
}
static void pane_defaults(GD_ExplorerPane *p, GD_ExplorerView view) {
    GD_Layout *layout=p->layout;
    gd_explorer_structure_destroy(p->structure_state); gd_explorer_rules_destroy(p->rule_state);
    if (p->query_state) { gd_explorer_query_destroy(p->query_state); layout=NULL; }
    gd_diagnostics_clear(&p->diagnostics);
    gd_layout_focus_destroy(p->distance_focus); gd_resource_routes_destroy(p->resource_routes);
    memset(p,0,sizeof(*p)); p->layout=layout; p->view=view;
    p->selection=(GD_ExplorerSelection){-1,-1,0,-1,-1}; p->editor.record=-1;
    gd_explorer_form_defaults(GD_FORM_QUERY,&p->query_options);
    p->grid.noun=p->grid.verb=-1; p->resource_witness=-1; gd_explorer_form_defaults(GD_FORM_RESOURCE,&p->resource_pending);
    p->mda=(GD_ExplorerMdaDisplay){.record=-1,.link=-1,.held=-1,.roles={true,true,true,true},.paused=true};
    p->find_hold_node=-1;p->camera.zoom=1; p->focus_auto_fit=true; p->distance_focus_revision=UINT64_MAX; p->peer=p->foundation_peer=p->witness=-1;
    p->navigate=p->focus_root=p->focus_expand=p->inspected_record=-1;
    p->reach=(GD_ReachControls){0,-1,0,-1,-1}; p->overview=(GD_OverviewControls){0,-1,-1};
    p->shared=true;p->visual_filter=true;p->workspace_bounds=true; p->paused=view==GD_EXPLORER_MDA || view==GD_EXPLORER_RESOURCES || view==GD_EXPLORER_EFFECTS;
    p->layout_choice=view==GD_EXPLORER_NOUNS ? GD_NOUN_LAYOUT_GRID:view<=GD_EXPLORER_RELATIONSHIPS || query_view(view) ? GD_LAYOUT_DIRECTED:GD_LAYOUT_FORCE; p->fit=view==GD_EXPLORER_NOUNS ? GD_EXPLORER_FIT_GRID_READABLE:1; p->focus.root=-1;
    p->paths.declared=view==GD_EXPLORER_CHALLENGES;
}
static float pane_aspect(const GD_ExplorerContext *c) {
    if(isfinite(c->viewport.width) && isfinite(c->viewport.height) && c->viewport.width>0 && c->viewport.height>0){float aspect=c->viewport.width/c->viewport.height;if(isfinite(aspect) && aspect>0)return aspect;}
    return 1;
}
static int pane_workspace(GD_ExplorerContext *c,int enabled) {
    GD_ExplorerPane *p=c->pane;
    if(query_view(p->view))return p->query_state && p->query_state->scene && gd_query_scene_workspace(p->query_state->scene,enabled,pane_aspect(c));
    return gd_layout_workspace(p->layout,enabled,pane_aspect(c));
}
static int pane_islands(GD_ExplorerContext *c,int enabled){
    GD_ExplorerPane *p=c->pane;
    if(query_view(p->view))return p->query_state && p->query_state->scene && gd_query_scene_islands(p->query_state->scene,enabled);
    return gd_layout_islands_set(p->layout,enabled);
}
static int pane_seed(GD_ExplorerContext *c) {
    GD_ExplorerPane *p=c->pane;
    if(p->defer_free_prepare)return 1;
    if(!gd_layout_prepare_free(p->layout,c->project->allocator,pane_aspect(c),p->workspace_bounds && free_graph(p) && !grid_view(p)))return 0;
    if(!gd_layout_islands_set(p->layout,p->island_separation && free_graph(p) && !grid_view(p)))return 0;
    if(p->view==GD_EXPLORER_RESOURCES){int i;for(i=0;i<120;++i)gd_layout_step(p->layout,-1);}
    return 1;
}
static int pane_prepare(GD_ExplorerContext *c) {
    GD_ExplorerPane *p=c->pane; GD_Model *model=gd_explorer_model(c->project,p->view); int i,record=-1;
    unsigned revision=gd_explorer_family(p->view)==GD_EXPLORER_SKILL_DATA ? c->project->revision:1;
    gd_explorer_structure_prepare(c);
    if (p->prepared && p->revision==revision) return 1;
    if (query_view(p->view)) { if (!gd_explorer_query_prepare(c)) return 0; p->find_hold_node=-1;p->prepared=1; p->revision=revision; return 1; }
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
    if (!(p->view==GD_EXPLORER_NOUNS ? noun_grid_prepare(c):is_graph(p->view) ? gd_explorer_graph_prepare(c):p->view==GD_EXPLORER_MDA ? gd_explorer_mda_prepare(c):gd_explorer_resource_prepare(c))) return 0;
    if (p->view==GD_EXPLORER_MATRIX && grid_view(p) && !gd_matrix_grid_prepare(&p->grid,model,(GD_LayoutSort)p->sort)) return 0;
    if (!pane_seed(c)) return 0;
    if (record>=0 && is_graph(p->view)) {
        p->selection=(GD_ExplorerSelection){-1,-1,0,-1,-1};
        for (i=0;i<p->layout->node_count;++i) if (p->layout->nodes[i].key==record) p->selection.node=i;
        if (p->selection.node<0) p->selection.linked=record;
    }
    if (is_graph(p->view)) p->inspected_record=gd_explorer_graph_record(c);
    if (p->distance_focus_enabled && !p->distance_focus) {
        p->distance_focus=gd_layout_focus_create(c->project->allocator);
        if (!p->distance_focus) return 0;
    }
    if (p->distance_focus) { gd_layout_focus_capture(p->distance_focus,p->layout); p->distance_focus_revision=UINT64_MAX; }
    p->find_hold_node=-1;p->prepared=1; p->revision=revision; return 1;
}
static int selected_record(GD_ExplorerContext *c) {
    GD_ExplorerPane *p=c->pane;
    if (gd_explorer_similarity_active(c)) {
        const GD_ExplorerStructureState *state=p->structure_state; const GD_SimilarityProjection *projection=state->result->similarity;
        if (projection->table_complete && state->similarity_row>=0 && state->similarity_row<projection->entry_count) return gd_structure_result_record(state->result,state->result->layout->nodes[projection->nodes[state->similarity_row]].key);
        return -1;
    }
    if(p->view==GD_EXPLORER_NOUNS)return noun_grid_record(c);
    if (query_view(p->view)) return gd_explorer_query_record(p);
    if (is_graph(p->view)) return gd_explorer_graph_record(c);
    if (p->view==GD_EXPLORER_MDA) return p->mda.record;
    return p->selection.linked>=0 ? p->selection.linked:gd_explorer_resource_record(c->project->resources,p->selection.node);
}
static Vector2 position(const GD_ExplorerContext *c, int node) {
    const GD_ExplorerPane *p=c->pane;const GD_LayoutNode *item=&gd_explorer_display_layout(c)->nodes[node];
    float scale=p->view==GD_EXPLORER_MDA && p->layout->mode==GD_LAYOUT_FORCE ? 3:1;
    return (Vector2){item->x*scale,item->y*scale};
}
static int visible(GD_ExplorerContext *c, int node) {
    GD_ExplorerPane *p=c->pane; int record=p->layout->nodes[node].key;
    if (query_view(p->view)) return 1;
    if (p->view==GD_EXPLORER_MDA) return gd_explorer_mda_visible(c,record);
    if (p->view==GD_EXPLORER_RESOURCES) {
        const GD_ResourceNode *item=&c->project->resources->nodes[record];
        if (p->resource_only_witness && p->resource_routes && p->resource_routes->witness_count && !p->resource_routes->marked_nodes[record]) return 0;
        int source=gd_explorer_resource_record(c->project->resources,record);
        const GD_Model *model=gd_explorer_model(c->project,p->view);
        return p->mda.roles[item->kind] && (!*p->filter || gd_text_contains(p->layout->nodes[node].label,p->filter) ||
            (source>=0 && gd_text_contains(model->records[source].reference,p->filter)));
    }
    return 1;
}
static int analyze(GD_ExplorerContext *c) {
    GD_ExplorerPane *p=c->pane; unsigned char shown[GD_MAX_RECORDS]; int i,selected=-1;
    if (p->view==GD_EXPLORER_NOUNS || grid_view(p) || query_view(p->view) || gd_explorer_similarity_active(c)) return p->selection.node; /* No generic layers over Grid or state/ghost associations. */
    if (p->view==GD_EXPLORER_MDA) gd_explorer_mda_mark(c);
    for (i=0;i<p->layout->node_count;++i) {
        shown[i]=(unsigned char)visible(c,i);
        if (p->view==GD_EXPLORER_MDA ? p->layout->nodes[i].key==p->mda.record : i==p->selection.node) selected=i;
    }
    if (gd_graph_analyze(p->layout,shown,p->view==GD_EXPLORER_MATRIX ? p->overlay:1,p->lens,c->analysis)) {
        p->peer=p->foundation_peer=-1; ++c->scope_revision;
    }
    return selected>=0 && shown[selected] ? selected:-1;
}
static void fit_view(GD_ExplorerContext *c, int only_matches);
static int distance_focus_toggle(GD_ExplorerContext *c, int enabled) {
    GD_ExplorerPane *p=c->pane;
    if (!!enabled==!!p->distance_focus_enabled) return 1;
    if (enabled) {
        if (!p->distance_focus) p->distance_focus=gd_layout_focus_create(c->project->allocator);
        if (!p->distance_focus) return 0;
        if (!(p->distance_focus->animating && p->distance_focus->returning)) gd_layout_focus_capture(p->distance_focus,p->layout);
        p->regular_camera=p->camera; p->distance_focus_revision=UINT64_MAX;
    } else {
        gd_layout_focus_begin(p->distance_focus,p->layout); p->distance_focus->returning=1;
        if (p->focus_auto_fit) p->camera=p->regular_camera;
        if (p->fit==3) p->fit=0;
        p->distance_focus->active=0;
    }
    p->find_hold_node=-1;p->distance_focus_enabled=enabled!=0; p->selection.held=p->mda.held=-1;
    return 1;
}
static int distance_focus_node(const GD_ExplorerPane *p, int selected) {
    return (p->view==GD_EXPLORER_MDA ? p->mda.link>=0:p->selection.linked>=0 || p->selection.cell>=0) ? -1:selected;
}
static void distance_focus_update(GD_ExplorerContext *c, int selected) {
    GD_ExplorerPane *p=c->pane; int result;
    if (grid_view(p) || query_view(p->view) || !p->distance_focus_enabled || !p->distance_focus) return;
    if(p->find_hold_node>=0){if(p->find_hold_node==selected && p->find_hold_layout==p->layout)return;p->find_hold_node=-1;}
    /* A source-only or edge inspection is not an implicit canonical node. */
    selected=distance_focus_node(p,selected);
    if (p->distance_focus->anchor_node==selected && p->distance_focus_revision==c->scope_revision) {
        if (p->distance_focus->animating) gd_layout_focus_advance(p->distance_focus,p->layout,0,p->view==GD_EXPLORER_MDA && p->layout->mode==GD_LAYOUT_FORCE ? 3:1);
        else gd_layout_focus_reapply(p->distance_focus,p->layout,p->view==GD_EXPLORER_MDA && p->layout->mode==GD_LAYOUT_FORCE ? 3:1);
        p->selection.held=p->mda.held=-1; return;
    }
    gd_layout_focus_begin(p->distance_focus,p->layout);
    result=gd_layout_focus_apply(p->distance_focus,p->layout,c->analysis->shown,
        p->view==GD_EXPLORER_MATRIX ? p->overlay:1,!incidence(p->view) && !free_graph(p),selected,
        p->view==GD_EXPLORER_MDA && p->layout->mode==GD_LAYOUT_FORCE ? 3:1);
    p->distance_focus_revision=c->scope_revision;
    if (result==2 && p->focus_auto_fit && !p->focus_camera_override && p->fit!=2) fit_view(c,2);
    p->distance_focus->returning=!p->distance_focus->active;
    gd_layout_focus_advance(p->distance_focus,p->layout,0,p->view==GD_EXPLORER_MDA && p->layout->mode==GD_LAYOUT_FORCE ? 3:1);
    p->selection.held=p->mda.held=-1;
}
static void matches(GD_ExplorerContext *c) {
    GD_ExplorerPane *p=c->pane; GD_Model *model=gd_explorer_model(c->project,p->view);
    int i,record=gd_explorer_record(model,p->find_reference),represented=0;
    if (p->view==GD_EXPLORER_MDA) gd_explorer_mda_mark(c);
    memset(p->match_nodes,0,sizeof(p->match_nodes)); memset(p->match_edges,0,sizeof(p->match_edges));
    p->visible_matches=p->hidden_matches=0;
    if (!*p->find_reference || record<0) return;
    for (i=0;i<p->layout->node_count;++i) {
        int source=query_view(p->view) ? gd_query_scene_record(p->query_state->scene,i):p->view==GD_EXPLORER_RESOURCES ? gd_explorer_resource_record(c->project->resources,p->layout->nodes[i].key):p->layout->nodes[i].key;
        if (source!=record) continue;
        represented=1;
        if (visible(c,i)) { p->match_nodes[i]=1; ++p->visible_matches; } else ++p->hidden_matches;
    }
    if (p->view==GD_EXPLORER_MATRIX) for (i=0;i<p->layout->edge_count;++i) {
        const GD_LayoutEdge *edge=&p->layout->edges[i]; const GD_Cell *cell=&model->cells[edge->key]; int j,found=0;
        for (j=0;j<cell->pair_count;++j) if (model->pairs[cell->first_pair+j].interaction==record) found=1;
        if (!found) continue;
        represented=1;
        if (grid_view(p) || edge->active || p->overlay) {
            p->match_edges[i]=1; p->match_nodes[edge->a]=p->match_nodes[edge->b]=1; ++p->visible_matches;
        } else ++p->hidden_matches;
    }
    /* A loaded identity outside a focused neighborhood is hidden by scope, not
       absent. Other projection kinds need not represent this record at all. */
    if (!represented && p->view==GD_EXPLORER_RELATIONSHIPS) ++p->hidden_matches;
}
#include "gd_explorer_pair_selection.h"
#include "gd_explorer_focus_selection.h"
static void fit_view(GD_ExplorerContext *c, int only_matches) {
    GD_ExplorerPane *p=c->pane; int i,found=0; float left=0,right=1,top=0,bottom=1;
    if (grid_view(p)) { grid_fit(c,only_matches); return; }
    if(only_matches==GD_EXPLORER_FIT_SELECTION){Rectangle bounds={0};if(!pair_selection_bounds(c,&bounds))return;p->camera.target=(Vector2){bounds.x+bounds.width/2,bounds.y+bounds.height/2};p->camera.zoom=fmaxf(0.002f,fminf(1.2f,fminf(c->viewport.width/(bounds.width+60),c->viewport.height/(bounds.height+60))));return;}
    if (only_matches==4 && query_view(p->view)) {
        const GD_ExplorerQueryState *state=p->query_state; fit_view(c,0);
        if (p->camera.zoom<0.8f) p->camera.zoom=0.8f;
        if (state && state->query && state->query->state_coverage && p->layout->node_count) { int anchor=p->selection.node>=0 ? p->selection.node:0; p->camera.target=(Vector2){p->layout->nodes[anchor].x,p->layout->nodes[anchor].y-c->viewport.height/(p->camera.zoom*5)}; }
        if (state && state->selected) for (i=0;i<p->layout->node_count;++i) if (state->scene->nodes[i].source==state->selected->selection.target) p->camera.target=(Vector2){p->layout->nodes[i].x,p->layout->nodes[i].y-c->viewport.height/(p->camera.zoom*5)};
        return;
    }
    for (i=0;i<p->layout->node_count;++i) if (visible(c,i) && (!only_matches || (only_matches==1 ? p->match_nodes[i]:only_matches==3 ? p->resource_routes && p->resource_routes->marked_nodes[i]:
        p->distance_focus && p->distance_focus->active && p->distance_focus->distance[i]>=0 && p->distance_focus->distance[i]<=1))) {
        Vector2 point=position(c,i); float half=p->view==GD_EXPLORER_MDA ? 125:p->view==GD_EXPLORER_RESOURCES ? 110:
            fmaxf(110,(float)MeasureText(p->layout->nodes[i].label,18)/2+25);
        if (query_view(p->view)) half=p->layout->nodes[i].width/2;
        if (!found || point.x-half<left) left=point.x-half;
        if (!found || point.x+half>right) right=point.x+half;
        if (!found || point.y-90<top) top=point.y-90;
        if (!found || point.y+80>bottom) bottom=point.y+80;
        found=1;
    }
    if (!only_matches && p->view!=GD_EXPLORER_MDA && p->view!=GD_EXPLORER_RESOURCES) for (i=0;i<p->layout->edge_count;++i) {
        GD_LayoutRoute route; int j; gd_layout_route(gd_explorer_display_layout(c),i,&route);
        for (j=0;j<route.count;++j) {
            left=fminf(left,route.points[j].x); right=fmaxf(right,route.points[j].x);
            top=fminf(top,route.points[j].y); bottom=fmaxf(bottom,route.points[j].y);
        }
    }
    if(!only_matches && p->layout->workspace.enabled){const GD_LayoutBounds *b=&p->layout->workspace.bounds;float scale=p->view==GD_EXPLORER_MDA ? 3:1;if(!found){left=b->left*scale;right=b->right*scale;top=b->top*scale;bottom=b->bottom*scale;found=1;}else{left=fminf(left,b->left*scale);right=fmaxf(right,b->right*scale);top=fminf(top,b->top*scale);bottom=fmaxf(bottom,b->bottom*scale);}}
    if (!found) return;
    p->camera.target=(Vector2){(left+right)/2,(top+bottom)/2};
    p->camera.zoom=fminf(c->viewport.width/(right-left+60),c->viewport.height/(bottom-top+60));
    if (p->camera.zoom>1.2f) p->camera.zoom=1.2f; if (p->camera.zoom<0.002f) p->camera.zoom=0.002f;
}
static void draw_matches(GD_ExplorerContext *c) {
    GD_ExplorerPane *p=c->pane; int i;
    if (grid_view(p)) return; /* Grid draws matching headers/cells, not graph routes. */
    BeginScissorMode((int)c->viewport.x,(int)c->viewport.y,(int)c->viewport.width,(int)c->viewport.height); BeginMode2D(p->camera);
    for (i=0;i<p->layout->edge_count;++i) if (p->match_edges[i]) {
        GD_LayoutRoute route; gd_layout_route(gd_explorer_display_layout(c),i,&route); gd_graph_route_line(&route,5,gold,0);
    }
    for (i=0;i<p->layout->node_count;++i) if (p->match_nodes[i]) {
        Vector2 point=position(c,i);
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
    else if (query_view(c->pane->view)) gd_explorer_query_source(c,edge);
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
    if (p->view==GD_EXPLORER_NOUNS || grid_view(p) || query_view(p->view) || gd_explorer_similarity_active(c)) return e;
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
            Vector2 point=position(c,i);
            if (CheckCollisionPointRec(world,(Rectangle){point.x-90,point.y-24,180,48})) { p->selection.node=p->selection.held=i; p->selection.linked=-1; gd_layout_pickup_node(p->layout,i,world.x,world.y); break; }
        }
        if (p->selection.held>=0 && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) gd_layout_drag_node(p->layout,p->selection.held,world.x,world.y);
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
            if (igSmallButton(TextFormat("%s x %s (case cell)",model->records[cell->noun].display_name,model->records[cell->verb].display_name))) {
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
static int presentation_widget(GD_ExplorerContext *c,const char *name,const char *label) {
    GD_ExplorerPresentation values; const GD_ExplorerControl *field=gd_explorer_form_field(GD_FORM_PRESENTATION,name); char key[GD_MAX_PATH]; int changed;
    gd_explorer_presentation_read(c,&values);
    if(!strcmp(name,"layout")) changed=igCombo_Str(label,&values.layout,c->pane->view==GD_EXPLORER_SKILLS ? "Prerequisite depth\0Authored rank constraints\0Free graph\0":c->pane->view==GD_EXPLORER_NOUNS ? "Grid\0Free graph\0":c->pane->view==GD_EXPLORER_MATRIX ? "Free graph\0Noun / verb columns\0Grid\0":query_view(c->pane->view) ? c->pane->query_state && gd_query_scene_overview_mode(c->pane->query_state->scene)==GD_ACTOR_USAGE ? "Fixed actor-ring cloud\0Free graph\0":"Native query layout\0Free graph\0":c->pane->view==GD_EXPLORER_RELATIONSHIPS ? "Stable neighborhood\0Free graph\0":c->pane->view==GD_EXPLORER_CHALLENGES ? "Declared route bands\0Free graph\0":"Role bands\0Free graph\0",-1);
    else changed=gd_explorer_form_widget(GD_FORM_PRESENTATION,&values,name,label);
    if(changed && gd_explorer_presentation_value(c,field,&values,key,sizeof(key))==1) gd_explorer_presentation_set(c,name,key,c->pane->query_message,sizeof(c->pane->query_message));
    return changed;
}
static int layout_controls(GD_ExplorerContext *c) {
    GD_ExplorerPane *p=c->pane; GD_Model *model=gd_explorer_model(c->project,p->view); int rearrange=0;
    if(p->view==GD_EXPLORER_NOUNS){int side,i;for(side=0;side<2;++side){int selected=side ? p->grid.verb:p->grid.noun;if(gd_explorer_picker_begin(c,side ? "Column noun":"Row noun",selected>=0 ? model->records[selected].display_name:NULL)){for(i=0;i<c->project->noun_grid->noun_count;++i){int record=c->project->noun_grid->nouns[i];if(gd_explorer_picker_item(c,model->records[record].display_name,record==selected,0))gd_explorer_presentation_set(c,side ? "noun_column":"noun_row",model->records[record].reference,p->query_message,sizeof(p->query_message));}gd_explorer_picker_end();}}}
    if(p->view==GD_EXPLORER_ACTORS) presentation_widget(c,"actor_overview","Actor overview");
    if (p->view==GD_EXPLORER_NOUNS || is_graph(p->view) || p->view==GD_EXPLORER_MDA || (query_view(p->view) && p->query_state && p->query_state->scene && gd_query_scene_overview_mode(p->query_state->scene)!=GD_ACTOR_INTERACTIONS)) presentation_widget(c,"layout","Layout");
    else if (query_view(p->view)) igTextWrapped(p->query_state && p->query_state->scene && p->query_state->scene->overview ? "Overview placement: weighted Free interaction map. Usage cloud also offers fixed-ring and Free layouts. Explicit query layouts remain separate.":"Layout: separate native query bands; incoming weighting never changes declarations. Query/ghost associations are not temporal paths.");
    else igTextWrapped(p->view==GD_EXPLORER_NOUNS ? "Layout: noun x noun case-count grid":"Layout: free incidence graph");
    if (free_graph(p) && !grid_view(p))igTextDisabled("Free placement: shared Neighborhood seed.");
    if (p->view==GD_EXPLORER_SKILLS || p->view==GD_EXPLORER_CHALLENGES) presentation_widget(c,"direction","Direction");
    if (is_graph(p->view)) presentation_widget(c,"sort","Sort / initial order");
    if(query_view(p->view) && p->query_state && p->query_state->scene) {
        igTextWrapped("Placement never changes declarations or query evidence. Query/ghost associations are not temporal paths.");
        if(free_graph(p) && p->query_state->scene->ghosts==GD_QUERY_STACKED_VARIANTS) igTextDisabled("Stacked variant cards move together.");
    }
    if (p->view==GD_EXPLORER_SKILLS && free_graph(p)) igTextWrapped("Node band labels retain %s; positions are not locked to bands.",p->rank ? "authored rank constraints":"prerequisite depth");
    if(free_graph(p) && !grid_view(p) && !gd_explorer_similarity_active(c)){
        bool islands=p->island_separation;
        if(igCheckbox("Separate connected islands",&islands))gd_explorer_presentation_set(c,"island_separation",islands ? "true":"false",p->query_message,sizeof(p->query_message));
        if(igIsItemHovered(ImGuiHoveredFlags_None))igSetTooltip("Bounds follow shown cards in retained active-edge components. Whole-island forces run while settling; no analysis Run or inferred links.");
        if(p->layout->islands.enabled){igTextDisabled("%d islands; %u overlap/clearance pairs before last step",p->layout->islands.count,p->layout->islands.overlaps);if(p->layout->islands.limited)igTextDisabled("Island separation limited by work or workspace.");}
    }
    if(free_graph(p) && !grid_view(p) && !gd_explorer_similarity_active(c)){bool workspace=p->workspace_bounds;if(igCheckbox("Workspace bounds",&workspace))gd_explorer_presentation_set(c,"workspace_bounds",workspace ? "true":"false",p->query_message,sizeof(p->query_message));if(p->workspace_bounds){igTextDisabled("Fixed bounds; F fits workspace. Drag outside; release returns inside.");igTextDisabled("Bounds can crowd edges; internal motion may remain.");}}
    if(free_graph(p) && !grid_view(p) && !gd_explorer_similarity_active(c)){bool filtered=p->visual_filter;if(igCheckbox("Smooth displayed motion",&filtered))gd_explorer_presentation_set(c,"visual_filter",filtered ? "true":"false",p->query_message,sizeof(p->query_message));if(filtered)igTextDisabled("Rendering only; solver positions and velocities remain raw.");}
    if (!layout_settles(p))
        igTextWrapped("Stable layout; settling applies only to movable graph layouts.");
    else { igCheckbox("Pause settling",&p->paused); igSameLine(0,-1); }
    if (igButton("Fit (F)",(ImVec2_c){0,0})) gd_explorer_presentation_action(c,"view.fit");
    if (!query_view(p->view) || (p->query_state && p->query_state->scene)) { igSameLine(0,-1); if (igButton("Rearrange",(ImVec2_c){0,0})) gd_explorer_presentation_action(c,"view.rearrange"); }
    if(p->view==GD_EXPLORER_MATRIX || p->view==GD_EXPLORER_NOUNS){igSameLine(0,-1);igBeginDisabled(!gd_explorer_action_available(c,GD_ACTION_VIEW_CELL_FIT));gd_explorer_action_button(c,GD_ACTION_VIEW_CELL_FIT);igEndDisabled();}
    if (p->layout->mode==GD_LAYOUT_FORCE && p->layout->collision_limited)
        igTextWrapped("Footprint separation reached bounded work/movement limit; overlaps may remain. Resume settling to continue. Camera does not establish clearance.");
    if (p->view==GD_EXPLORER_MATRIX && grid_view(p)) igTextWrapped("Grid shows every loaded noun, verb and declared cell, including inactive pairs. Wheel zooms cells/labels; right/middle drag pans. Arrows select (held repeat); PageUp/Down changes case. Blank is not a defect. Graph analysis/focus layers unavailable in Grid; settings retained for graph layouts.");
    return rearrange;
}
static void scope_controls(GD_ExplorerContext *c) {
    GD_ExplorerPane *p=c->pane;GD_Model *model=gd_explorer_model(c->project,p->view);
    if (p->view==GD_EXPLORER_MDA) {
        int direction=c->project->mda->direction;
        if (igCombo_Str("Traversal",&direction,"Both / associations\0Forward / toward aesthetics\0Backward / authored basis\0",-1)) {
            const char *keys[]={"both","forward","backward"}; gd_explorer_presentation_set(c,"mda_direction",keys[direction],p->query_message,sizeof(p->query_message));
        }
        igTextWrapped("Query root: %s",c->project->mda->root>=0 ? model->records[c->project->mda->root].reference:"all supplied nodes");
        if (p->mda.record>=0 && gd_mda_role(model->records[p->mda.record].role) && igButton("Use selected as query root",(ImVec2_c){0,0}))
            gd_explorer_presentation_set(c,"mda_root",model->records[p->mda.record].reference,p->query_message,sizeof(p->query_message));
        if (igButton("Show all query nodes",(ImVec2_c){0,0})) gd_explorer_presentation_set(c,"mda_root","",p->query_message,sizeof(p->query_message));
    }
}
static int has_display_filters(const GD_ExplorerPane *p) {return query_view(p->view) || p->view==GD_EXPLORER_MDA || p->view==GD_EXPLORER_RESOURCES || (p->view==GD_EXPLORER_MATRIX && !grid_view(p));}
static void display_filter_controls(GD_ExplorerContext *c) {
    GD_ExplorerPane *p=c->pane;
    igTextWrapped("Visibility does not Run analytical inputs. Graph analysis uses shown nodes/links; retained query results keep applied inputs.");
    if(query_view(p->view))gd_explorer_query_display_controls(c);
    if(p->view==GD_EXPLORER_MATRIX)igCheckbox("Overlay other pairs (does not merge groups)",&p->overlay);
    if(p->view==GD_EXPLORER_MDA){int i;
        igCheckbox("Only query nodes (display only)",&p->mda.only_scope);
        igCheckbox("Only selected witness (display only)",&p->mda.only_path);
        for (i=0;i<4;++i) igCheckbox(gd_mda_role_name((GD_Role)(GD_MDA_MECHANIC+i)),&p->mda.roles[i]);
        igInputText("Reference filter (canvas)",p->mda.filter,sizeof(p->mda.filter),0,NULL,NULL);
    } else if (p->view==GD_EXPLORER_RESOURCES) {
        igCheckbox("Rule cases",&p->mda.roles[GD_RESOURCE_CASE]); igSameLine(0,-1);
        igCheckbox("Actor facets",&p->mda.roles[GD_RESOURCE_FACET]);
        igCheckbox("Resource roles",&p->mda.roles[GD_RESOURCE_ROLE]);
        igInputText("Name / source filter (canvas)",p->filter,sizeof(p->filter),0,NULL,NULL);
    }
}
static int reset_view(GD_ExplorerContext *c);
static void grid_frame_selection(GD_ExplorerContext *c);
#include "gd_explorer_presentation.h"
static int reload_project(GD_ExplorerContext *c, GD_ExplorerPane *panes, const GD_Collection *collections, int count,
                          char *failure, size_t failure_size) {
    union { GD_ExplorerQueryOptions query; GD_ExplorerStructureForm structure; GD_ExplorerRuleForm rules; GD_ExplorerResourceForm resource; } defaults;
    GD_ExplorerContext current=*c; int pane,form;
    /* Cold forms can be configured before any canvas exists. Never skip their
       pending inputs, or consume them implicitly while rebuilding a snapshot. */
    for (pane=0;pane<GD_EXPLORER_VIEWS;++pane) for (form=0;form<GD_FORM_COUNT;++form) {
        const void *pending,*applied; current.pane=&panes[pane];
        pending=gd_explorer_form_pending(&current,(GD_ExplorerForm)form,0); if (!pending) continue;
        applied=gd_explorer_form_applied(&current,(GD_ExplorerForm)form);
        if (!applied) { gd_explorer_form_defaults((GD_ExplorerForm)form,&defaults); applied=&defaults; }
        if (!gd_explorer_form_equal((GD_ExplorerForm)form,pending,applied)) {
            snprintf(failure,failure_size,"Run pending %s inputs in %s before refresh; prior state retained.",gd_explorer_form_name((GD_ExplorerForm)form),gd_explorer_view_names[pane]); return 0;
        }
    }
    GD_ExplorerProject *old=c->project,*fresh=gd_explorer_load(old->allocator,collections,count);
    GD_ExplorerPane *replacement=NULL; GD_ExplorerContext candidate=*c;
    const GD_Error *error=gd_explorer_error(fresh); const char *question_failure=NULL;
    int v,ready=fresh && fresh->available==old->available && !error;
    if (ready) {
        replacement=old->allocator.allocate(old->allocator.context,GD_EXPLORER_VIEWS*sizeof(*replacement));
        ready=replacement!=NULL;
    }
    if (replacement) {
        memcpy(replacement,panes,GD_EXPLORER_VIEWS*sizeof(*replacement));
        for (v=0;v<GD_EXPLORER_VIEWS;++v) { replacement[v].layout=NULL; replacement[v].distance_focus=NULL; replacement[v].resource_routes=NULL; replacement[v].query_state=NULL; replacement[v].structure_state=NULL; replacement[v].rule_state=NULL; memset(&replacement[v].diagnostics,0,sizeof(replacement[v].diagnostics)); }
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
        for (v=0;v<GD_EXPLORER_VIEWS && ready;++v) if (panes[v].layout || panes[v].query_state || panes[v].structure_state || panes[v].rule_state || panes[v].resource_routes) {
            GD_ExplorerPane *p=&replacement[v]; const GD_ExplorerPane *prior=&panes[v];
            GD_Model *model=gd_explorer_model(old,(GD_ExplorerView)v),*next=gd_explorer_model(fresh,(GD_ExplorerView)v);
            GD_Focus focus={0}; int record,root=-1,j;
            if (query_view((GD_ExplorerView)v)) {
                candidate.pane=p; p->selection=(GD_ExplorerSelection){-1,-1,0,-1,-1}; p->prepared=0;
                if (!gd_explorer_query_remap(&candidate,old,prior) || !pane_prepare(&candidate)) ready=0;
                if (ready && !gd_explorer_structure_remap(&candidate,old,prior)) { ready=0; question_failure=p->query_message; }
                continue;
            }
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
            if (v==GD_EXPLORER_MATRIX || v==GD_EXPLORER_NOUNS) {
                p->grid.noun=prior->grid.noun>=0 ? gd_explorer_record(next,model->records[prior->grid.noun].reference):-1;
                p->grid.verb=prior->grid.verb>=0 ? gd_explorer_record(next,model->records[prior->grid.verb].reference):-1;
            }
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
                p->resource_pending.from=gd_explorer_resource_remap(old->resources,fresh->resources,prior->resource_pending.from);
                p->resource_pending.to=gd_explorer_resource_remap(old->resources,fresh->resources,prior->resource_pending.to);
                if ((prior->resource_pending.from>=0 && p->resource_pending.from<0) || (prior->resource_pending.to>=0 && p->resource_pending.to<0)) {
                    snprintf(p->resource_message,sizeof(p->resource_message),"Resource endpoint cannot remap; prior snapshot/inputs/witnesses retained."); ready=0; question_failure=p->resource_message;
                }
                p->resource_witness=-1;
            } else gd_graph_paths_remap(&p->paths,model,next);
            p->selection=(GD_ExplorerSelection){-1,-1,0,-1,-1};
            if (v==GD_EXPLORER_RESOURCES) {
                p->selection.node=gd_explorer_resource_remap(old->resources,fresh->resources,prior->selection.node);
                if (prior->selection.linked>=0) p->selection.linked=record;
            }
            else if (is_graph((GD_ExplorerView)v) || (v==GD_EXPLORER_NOUNS && (prior->selection.node>=0 || prior->selection.linked>=0))) p->selection.linked=record;
            if(v==GD_EXPLORER_NOUNS && prior->selection.node<0 && prior->selection.linked<0){p->selection.cell=gd_noun_grid_pair(fresh->noun_grid,p->grid.noun,p->grid.verb);p->noun_case_page=0;}
            if (v==GD_EXPLORER_MATRIX && grid_view(p) && prior->selection.node<0 && prior->selection.linked<0 && p->grid.noun>=0 && p->grid.verb>=0) {
                int cell=gd_matrix_grid_cell(next,p->grid.noun,p->grid.verb);
                p->selection.cell=cell;
                if (cell>=0) {
                    for (j=0;j<next->cells[cell].pair_count;++j)
                        if (next->pairs[next->cells[cell].first_pair+j].interaction==record) p->selection.interaction=j;
                    if (record<0 || next->pairs[next->cells[cell].first_pair+p->selection.interaction].interaction==record) p->selection.linked=-1;
                }
            }
            p->editor.record=-1; p->mda.record=record; p->mda.link=p->mda.held=-1;
            p->witness=p->peer=p->foundation_peer=p->reach.peer=p->reach.target=-1;
            p->prepared=0; p->revision=0;
            candidate.pane=p;
            if (!pane_prepare(&candidate)) ready=0;
            if(ready && v==GD_EXPLORER_NOUNS && prior->selection.node>=0 && record>=0){for(j=0;j<p->layout->node_count;++j)if(p->layout->nodes[j].key==record){p->selection.node=j;p->selection.linked=-1;break;}}
            if (ready && !gd_explorer_structure_remap(&candidate,old,prior)) { ready=0; question_failure=p->query_message; }
            if (ready && !gd_explorer_rules_remap(&candidate,old,prior)) { ready=0; question_failure=p->rule_message; }
            if (ready && v==GD_EXPLORER_RESOURCES && prior->resource_routes) {
                if (!gd_explorer_form_equal(GD_FORM_RESOURCE,&prior->resource_pending,&prior->resource_applied)) { snprintf(p->resource_message,sizeof(p->resource_message),"Run pending Resource inputs before refresh; prior witnesses retained."); ready=0; question_failure=p->resource_message; }
                else if (!gd_explorer_resource_run(&candidate)) { ready=0; question_failure=p->resource_message; }
            }
        }
        /* Preparing every visited layout is part of candidate validation. Never
           release last-good snapshots before all replacement allocations pass. */
        for (v=0;v<GD_EXPLORER_FAMILIES && ready;++v) if (fresh->models[v] && gd_sources_changed(fresh->models[v])) ready=0;
    }
    if (ready) {
        for (v=0;v<GD_EXPLORER_VIEWS;++v) {
            gd_diagnostics_clear(&panes[v].diagnostics);
            gd_layout_focus_destroy(panes[v].distance_focus); gd_resource_routes_destroy(panes[v].resource_routes);
            pane_release_layout(old->allocator,&panes[v]);
        }
        memcpy(panes,replacement,GD_EXPLORER_VIEWS*sizeof(*panes));
        old->allocator.release(old->allocator.context,replacement);
        c->project=fresh; c->analysis->initialized=0; gd_explorer_destroy(old); *failure=0;
        return 1;
    }
    snprintf(failure,failure_size,"Reload rejected: %s. Last good snapshot retained.",error ? error->message:question_failure ? question_failure:
             "Cannot prepare replacement, sources changed, or a query/focus root disappeared; clear root before retry");
    if (replacement) {
        for (v=0;v<GD_EXPLORER_VIEWS;++v) {
            gd_layout_focus_destroy(replacement[v].distance_focus); gd_resource_routes_destroy(replacement[v].resource_routes);
            pane_release_layout(old->allocator,&replacement[v]);
        }
        old->allocator.release(old->allocator.context,replacement);
    }
    gd_explorer_destroy(fresh); return 0;
}
/* 1 loaded, 0 reload failed, -1 blocked. Never commits or discards editor input. */
static int refresh_sources(GD_ExplorerContext *c, GD_ExplorerPane *panes, const GD_Collection *collections, int count, char *failure, size_t capacity) {
    int active=c->pane->view,updated;
    if (gd_explorer_rank_pending(c->project,panes) || (c->project->draft && (c->project->draft->modified_count || c->project->draft->history_count || c->project->draft->redo_count))) {
        snprintf(failure,capacity,"Refresh blocked: pending editor, draft changes or Undo/Redo history; explicitly discard Redo-only history before refresh. No user buffers discarded."); return -1;
    }
    updated=reload_project(c,panes,collections,count,failure,capacity); c->pane=&panes[active]; return updated;
}
/* Shared state transition after caller establishes its own edit guard. */
static void enter_view(GD_ExplorerContext *c, GD_ExplorerPane *panes, int target, const char *find_reference) {
    c->pane->selection.held=c->pane->mda.held=-1; c->pane->right_started=0;
    c->pane=&panes[target];
    if (find_reference) { snprintf(c->pane->find_reference,sizeof(c->pane->find_reference),"%s",find_reference); c->pane->fit=2; }
    c->analysis->initialized=0;
}
static int change_view(GD_ExplorerContext *c, GD_ExplorerPane *panes, int target, const char *find_reference) {
    if (target<0 || target>=GD_EXPLORER_VIEWS || !(c->project->available&(1u<<target))) return 0;
    if (is_graph(c->pane->view) && gd_explorer_graph_commit(c)<0) return 0;
    enter_view(c,panes,target,find_reference); return 1;
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
    GD_ExplorerPane *p=c->pane;GD_ExplorerPresentation values;bool bounds=p->workspace_bounds;
    gd_explorer_presentation_read(c,&values);values.sort=values.direction=0;values.workspace_bounds=true;values.island_separation=false;
    values.layout=p->view==GD_EXPLORER_NOUNS ? GD_NOUN_LAYOUT_GRID:p->view==GD_EXPLORER_MATRIX || p->view==GD_EXPLORER_MDA || p->view==GD_EXPLORER_RESOURCES ? GD_LAYOUT_FORCE:0;
    if(p->view==GD_EXPLORER_ACTORS && p->query_state && gd_query_scene_overview_mode(p->query_state->scene)==GD_ACTOR_INTERACTIONS)values.layout=1;
    p->workspace_bounds=true;
    if(!gd_explorer_similarity_active(c) && !presentation_rearrange(c,&values,0)){
        p->workspace_bounds=bounds;snprintf(p->query_message,sizeof(p->query_message),"Cannot reset presentation; last good view retained.");return 0;
    }
    p->island_separation=false;if(!query_view(p->view) || free_graph(p))pane_islands(c,0);
    distance_focus_toggle(c,0);pane_workspace(c,free_graph(p) && !grid_view(p) && !gd_explorer_similarity_active(c));p->visual_filter=true;p->overlay=false;
    p->mda.only_scope=p->mda.only_path=p->resource_only_witness=false;
    p->mda.roles[0]=p->mda.roles[1]=p->mda.roles[2]=p->mda.roles[3]=true;
    *p->filter=*p->mda.filter=0;p->focus_auto_fit=true;p->find_hold_node=-1;p->find_hold_layout=NULL;
    p->paused=p->view==GD_EXPLORER_MDA || p->view==GD_EXPLORER_RESOURCES || p->view==GD_EXPLORER_EFFECTS;
    p->camera=(Camera2D){.zoom=1};p->focus_result=(GD_ExplorerFocusResult){0};
    if(gd_explorer_similarity_active(c))p->structure_state->similarity_fit=true;else p->fit=grid_view(p) ? GD_EXPLORER_FIT_GRID_READABLE:1;
    c->analysis->initialized=0;return 1;
}
int gd_explorer_draft_history(GD_ExplorerContext *c,int redo) {
    if(!c->project->draft || c->pane->selection.held>=0 || c->pane->mda.held>=0 || (c->panes && gd_explorer_rank_pending(c->project,c->panes)))return 0;
    if(!(redo ? gd_draft_redo(c->project->draft):gd_draft_undo(c->project->draft)))return 0;
    if(c->panes){int v;for(v=0;v<=GD_EXPLORER_RELATIONSHIPS;++v)c->panes[v].editor.record=-1;}
    else c->pane->editor.record=-1;
    gd_explorer_edited(c->project);return 1;
}
static int undo_draft(GD_ExplorerContext *c,GD_ExplorerPane *panes){GD_ExplorerContext current=*c;current.panes=panes;return gd_explorer_draft_history(&current,0);}
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
#include "gd_explorer_viewer.h"
#include "gd_explorer_find.h"
#include "gd_explorer_history.h"
int gd_explorer_reset_all(GD_ExplorerContext *c){
    GD_Allocator a=c->project->allocator;GD_ExplorerPane *fresh;GD_ExplorerContext target=*c;int v,active;
    if(!gd_explorer_action_available(c,GD_ACTION_RESET_ALL))return 0;
    active=*c->active_view;fresh=a.allocate(a.context,GD_EXPLORER_VIEWS*sizeof(*fresh));
    if(!fresh)goto failed;
    memset(fresh,0,GD_EXPLORER_VIEWS*sizeof(*fresh));
    for(v=0;v<GD_EXPLORER_VIEWS;++v)pane_defaults(&fresh[v],(GD_ExplorerView)v);
    target.pane=&fresh[active];target.panes=fresh;target.display_layout=NULL;
    if(!pane_prepare(&target)){
        for(v=0;v<GD_EXPLORER_VIEWS;++v){pane_release_layout(a,&fresh[v]);gd_diagnostics_clear(&fresh[v].diagnostics);gd_layout_focus_destroy(fresh[v].distance_focus);gd_resource_routes_destroy(fresh[v].resource_routes);}
        a.release(a.context,fresh);goto failed;
    }
    /* Prepare replacement before releasing any last-good pane or result. */
    fresh[active].selection=(GD_ExplorerSelection){-1,-1,0,-1,-1};fresh[active].inspected_record=fresh[active].editor.record=-1;
    for(v=0;v<GD_EXPLORER_VIEWS;++v){
        pane_release_layout(a,&c->panes[v]);gd_diagnostics_clear(&c->panes[v].diagnostics);gd_layout_focus_destroy(c->panes[v].distance_focus);gd_resource_routes_destroy(c->panes[v].resource_routes);
        c->panes[v]=fresh[v];
    }
    a.release(a.context,fresh);c->pane=&c->panes[active];c->display_layout=NULL;
    if(c->project->mda)gd_mda_query(gd_explorer_model(c->project,GD_EXPLORER_MDA),c->project->mda,-1,GD_MDA_BOTH);
    if(c->analysis)c->analysis->initialized=0;
    if(c->query)memset(c->query,0,sizeof(*c->query));
    if(c->finds)memset(c->finds,0,GD_EXPLORER_VIEWS*sizeof(*c->finds));
    if(c->sidebar){GD_Sidebar defaults;gd_sidebar_defaults(&defaults);c->sidebar->open=defaults.open;gd_sidebar_save(c->sidebar);}
    if(c->history){memset(c->history,0,sizeof(*c->history));snprintf(c->history->message,sizeof(c->history->message),"All exploration reset. Draft edits and Undo/Redo preserved; source files unchanged.");}
    return 1;
failed:
    snprintf(c->pane->query_message,sizeof(c->pane->query_message),"Cannot prepare Reset all; all panes, results and drafts retained.");return 0;
}
int gd_explorer_view(GD_Model *diagnostic, const GD_Collection *collections, int count,
                     int initial_view, int matrix_layout, int frames, const char *screenshot, const char *output_path) {
    GD_Allocator allocator=diagnostic->allocator;
    GD_ExplorerProject *project=gd_explorer_load(allocator,collections,count);
    GD_ExplorerPane *panes=NULL;GD_RenderPose *render=NULL;GD_SelectionHistory *history=NULL;
    GD_ExplorerContext c={0};
    ExplorerViewer viewer={0}; char ipc_error[256]="";
    int active=0,v,frame=0,result=0,stale=0,close_seen=0,timed_seen=0,close_next=0;
    GD_Sidebar sidebar;GD_ExplorerFind finds[GD_EXPLORER_VIEWS]={0};
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
    render=allocator.allocate(allocator.context,sizeof(*render));if(render)memset(render,0,sizeof(*render));
    history=allocator.allocate(allocator.context,sizeof(*history));if(history)memset(history,0,sizeof(*history));
    if (!panes || !c.analysis || !c.query || !render || !history) { gd_error(diagnostic,"explorer.memory","Cannot allocate bounded explorer view state",NULL,0); result=2; goto cleanup; }
    memset(c.analysis,0,sizeof(*c.analysis));
    for (v=0;v<GD_EXPLORER_VIEWS;++v) pane_defaults(&panes[v],(GD_ExplorerView)v);
    if (matrix_layout>=0 && initial_view==GD_EXPLORER_NOUNS) {
        panes[GD_EXPLORER_NOUNS].layout_choice=matrix_layout==GD_LAYOUT_FORCE ? GD_NOUN_LAYOUT_FREE:GD_NOUN_LAYOUT_GRID;
        panes[GD_EXPLORER_NOUNS].fit=matrix_layout==GD_LAYOUT_FORCE ? 1:GD_EXPLORER_FIT_GRID_READABLE;
    } else if (matrix_layout>=0) {
        panes[GD_EXPLORER_MATRIX].layout_choice=matrix_layout;
        if (matrix_layout==GD_MATRIX_GRID_CHOICE) panes[GD_EXPLORER_MATRIX].fit=GD_EXPLORER_FIT_GRID_READABLE;
    }
    if (initial_view>=0) active=initial_view;
    else while (!(project->available&(1u<<active))) ++active;
    if (output_path) snprintf(report_path,sizeof(report_path),"%s",output_path);
    c.project=project; c.pane=&panes[active];c.panes=panes;c.active_view=&active;c.history=history;c.finds=finds;c.sidebar=&sidebar;
    if (!open_window("Game Design Workbench - Explorer")) { gd_error(diagnostic,"view.window","Cannot initialize explorer window",NULL,0); result=2; goto cleanup; }
    gd_sidebar_init(&sidebar);
    viewer.context=&c; viewer.panes=panes; viewer.active=&active; viewer.stale=&stale; viewer.reload_generation=1;
    viewer.collections=collections; viewer.collection_count=count;
    viewer.server=gd_viewer_server_open(allocator,ipc_error,sizeof(ipc_error));
    for (;;) {
        int closing=WindowShouldClose(),width=GetScreenWidth(),height=GetScreenHeight(),split=width*60/100;
        int requested=-1,find_view=-1,refresh=0,selected,relationship_root=-1,undo=0,redo=0,navigation=0,reset_all=0;
        char find_reference[260]="";int find_shortcut;
        GD_ExplorerPane *p=&panes[active]; GD_Model *model=gd_explorer_model(project,p->view);
        c.pane=p;c.render_pose=render; c.viewport=(Rectangle){14,112,(float)split-28,(float)height-157};
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
        if (gd_explorer_similarity_active(&c)) DrawText(TextFormat("%s | SIMILARITY | %d selected entries | separate native distance approximation",p->structure_state->result->options.projection,p->structure_state->result->similarity->entry_count),20,49,16,muted);
        else DrawText(TextFormat("%s | %d projected identities | %d projected links | positions transient",gd_explorer_view_names[active],p->layout->node_count,p->layout->edge_count),20,49,16,muted);
        DrawText(project->draft && project->draft->modified_count ? TextFormat("DRAFT: %d records modified in memory | source files unchanged",project->draft->modified_count):"Source-backed projections | Graph analysis uses shown graph | No source writes",20,77,15,project->draft && project->draft->modified_count ? gold:muted);
        igSetNextWindowPos((ImVec2_c){(float)split,12},ImGuiCond_Always,(ImVec2_c){0,0});
        igSetNextWindowSize((ImVec2_c){(float)(width-split-12),(float)(height-24)},ImGuiCond_Always);
        igBegin("Explorer controls and source",NULL,ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoSavedSettings);
        if (*ipc_error) igTextWrapped("%s",ipc_error);
        if (gd_viewer_server_error(viewer.server)) igTextWrapped("%s",gd_viewer_server_error(viewer.server));
        find_shortcut=find_key(IsKeyPressed(KEY_F),IsKeyDown(KEY_LEFT_CONTROL)||IsKeyDown(KEY_RIGHT_CONTROL),IsKeyDown(KEY_LEFT_SHIFT)||IsKeyDown(KEY_RIGHT_SHIFT)||IsKeyDown(KEY_LEFT_ALT)||IsKeyDown(KEY_RIGHT_ALT)||IsKeyDown(KEY_LEFT_SUPER)||IsKeyDown(KEY_RIGHT_SUPER),igGetIO_Nil()->WantCaptureKeyboard);
        if(find_shortcut==GD_FIND_OPEN){
            if(gd_explorer_rank_pending(project,panes) || IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT) || IsMouseButtonDown(MOUSE_BUTTON_MIDDLE) || igIsPopupOpen_Str(NULL,ImGuiPopupFlags_AnyPopupId|ImGuiPopupFlags_AnyPopupLevel))snprintf(failure,sizeof(failure),"Finish active rank edit, drag or dialog before Find; no input discarded.");
            else{finds[active].open=true;finds[active].focus=true;}
        }
        if(!igGetIO_Nil()->WantTextInput)history_observe(&c);
        if(gd_explorer_action_request(&c,GD_ACTION_BACK))navigation=-1;igSameLine(0,8);
        if(gd_explorer_action_request(&c,GD_ACTION_FORWARD))navigation=1;igSameLine(0,8);
        if(gd_explorer_action_request(&c,GD_ACTION_UNDO))undo=1;igSameLine(0,8);
        if(gd_explorer_action_request(&c,GD_ACTION_REDO))redo=1;igSameLine(0,12);
        if(gd_explorer_action_request(&c,GD_ACTION_RESET_ALL))reset_all=1;
        if(igIsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))igSetTooltip("Reset all panes: clear selections, analyses, inputs, filters, Find, cameras and navigation history. Restore default folds. Keep draft edits and Undo/Redo. No source writes.");
        if(*history->message)igTextWrapped("%s",history->message);
        requested=choose_view(project,"Data / projection",active,1);
        if(finds[active].open){analyze(&c);find_controls(&c,panes,&finds[active]);}
        if(stale)igTextWrapped("STALE: source changed or could not be checked. Last good snapshot retained.");
        if(*failure)igTextWrapped("%s",failure);
        if(*sidebar.message)igTextWrapped("%s",sidebar.message);
        if(query_view(p->view) && p->query_state && p->query_state->scene && p->query_state->scene->limited)igTextWrapped("DRAWING LIMITED: up to %d unexamined items. Native evidence remains separate; missing drawing is not absence.",p->query_state->scene->unexamined_items_upper_bound);
        selected=selected_record(&c);
        if(sidebar_section(&sidebar,GD_SIDEBAR_VIEW)){
            gd_explorer_action_button(&c,GD_ACTION_VIEW_RESET);
            igTextWrapped("Reset view: camera/layout/display only. Selection, analysis and drafts stay intact.");
            if (!gd_explorer_similarity_active(&c) && layout_controls(&c)) { p->prepared=0; pane_prepare(&c); p->fit=grid_view(p) ? GD_EXPLORER_FIT_GRID_READABLE:1; }
            if(gd_explorer_similarity_active(&c))gd_explorer_action_button(&c,GD_ACTION_VIEW_FIT);
        igBeginDisabled(!gd_explorer_action_available(&c,GD_ACTION_VIEW_SELECTION_FOCUS));gd_explorer_action_button(&c,GD_ACTION_VIEW_SELECTION_FOCUS);igEndDisabled();
        if(igIsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))igSetTooltip("Camera only; select a canvas item/source. Keep readable text; distant context may stay off-screen. No layout/filter/query change.");
        if(p->focus_result.status!=GD_FOCUS_NONE)igTextWrapped("Last focus: %s | %d selected, %d hidden, %d direct context | %s%s",focus_status_name(p->focus_result.status),p->focus_result.selected,p->focus_result.hidden,p->focus_result.context,p->focus_result.clipped ? "some context off-screen":"no framing clip",p->focus_result.hidden_link ? " | selected link remains hidden":"");
        if (p->view!=GD_EXPLORER_NOUNS && !gd_explorer_similarity_active(&c) && !grid_view(p) && !query_view(p->view)) {
            bool enabled=p->distance_focus_enabled;
            if (igCheckbox("Focus layout by shown-link distance",&enabled) && !distance_focus_toggle(&c,enabled))
                snprintf(failure,sizeof(failure),"Cannot allocate bounded distance-focus scratch; regular layout retained.");
            igCheckbox("Auto-fit camera on focus changes",&p->focus_auto_fit);
            if (p->distance_focus_enabled) igTextWrapped("Shown-link distance is layout only, not flow. Animated 220 ms transitions retain constrained bands/roots; Free graphs do not pin data roots. Click again to interrupt. Auto-fit off keeps camera unchanged on focus selection/off. Off returns to regular pose, then resumes drag/settling. F and Find still frame explicitly.");
        }
        }
        if(p->focus_result.status==GD_FOCUS_HIDDEN)igTextWrapped("Selection hidden by current filters/scope; camera unchanged. No data revealed.");
        if(p->focus_result.status==GD_FOCUS_UNREPRESENTED || p->focus_result.status==GD_FOCUS_UNPLACED)igTextWrapped("No placed selection representation here; source may still be loaded. No query or filter changed.");
        if(sidebar_section(&sidebar,GD_SIDEBAR_FIND)){
        if (selected>=0) {
            find_view=choose_view(project,"Find selected in",active,0);
            if (find_view>=0) snprintf(find_reference,sizeof(find_reference),"%s",model->records[selected].reference);
        } else igTextWrapped(p->view==GD_EXPLORER_RESOURCES && p->selection.node>=0 ?
            "Selected derived item has no unique loaded source. Inspect incident declarations; Find is unavailable.":
            "Select source to enable Find selected in another view.");
        match_details(&c);
        }
        if(!gd_explorer_similarity_active(&c) && has_display_filters(p) && sidebar_section(&sidebar,GD_SIDEBAR_FILTERS))display_filter_controls(&c);
        selected=analyze(&c); /* Required independently of sidebar visibility. */
        if(sidebar_section(&sidebar,GD_SIDEBAR_ANALYSIS)){
        gd_explorer_action_button(&c,GD_ACTION_INPUTS_RESET);
        if(!gd_explorer_similarity_active(&c))scope_controls(&c);
        if (p->view==GD_EXPLORER_RESOURCES) { gd_explorer_resource_controls(&c); gd_explorer_rules_controls(&c); }
        if (query_view(p->view)) gd_explorer_query_controls(&c);
        if (!gd_explorer_similarity_active(&c) && !grid_view(p) && p->distance_focus_enabled && distance_focus_node(p,selected)<0)
            igTextWrapped("No distance focus applied: select a visible node. Source-only or edge inspection has no focus anchor.");
        gd_explorer_structure_controls(&c);
        if (p->view!=GD_EXPLORER_NOUNS && !gd_explorer_similarity_active(&c) && !grid_view(p) && !query_view(p->view) && igCollapsingHeader_TreeNodeFlags("Analysis layers",0)) analysis_controls(&c,selected);
        if (p->view==GD_EXPLORER_SKILLS && igCollapsingHeader_TreeNodeFlags("Skill opportunities / knowledge",0)) { gd_explorer_graph_unlocks(&c); viewer_control_token(&viewer,&c,GD_FORM_PRESENTATION,viewer.knowledge_token,sizeof(viewer.knowledge_token)); }
        }
        if(!(sidebar.open&(1u<<GD_SIDEBAR_ANALYSIS))){if(*p->query_message)igTextWrapped("%s",p->query_message);if(*p->rule_message)igTextWrapped("%s",p->rule_message);}
        if(sidebar_section(&sidebar,GD_SIDEBAR_BROWSE)){
            int i;
            igInputText("Search shown-node list",p->list_search,sizeof(p->list_search),0,NULL,NULL);
            igBeginChild_Str("Explorer records",(ImVec2_c){0,120},0,0);
            for (i=0;i<viewer_canvas_count(&c);++i) if (viewer_canvas_visible(&c,i)) {
                int record=viewer_canvas_record(&c,i);const char *label=find_label(&c,i);
                const char *reference=record>=0 ? model->records[record].reference:"no unique loaded source";
                if (!gd_text_contains(label,p->list_search) && !gd_text_contains(reference,p->list_search) && !(record>=0 && gd_text_contains(model->records[record].display_name,p->list_search))) continue;
                igPushID_Int(i);
                if (igSmallButton(TextFormat("#%d %s | %s",i,label,reference))) {
                    if(gd_explorer_similarity_active(&c))gd_explorer_similarity_select(&c,i);
                    else if (p->view==GD_EXPLORER_MDA) { p->mda.record=record; p->mda.link=-1; }
                    else if (p->view==GD_EXPLORER_RESOURCES || p->view==GD_EXPLORER_NOUNS || query_view(p->view)) p->selection=(GD_ExplorerSelection){-1,i,0,-1,-1};
                    else p->navigate=record;
                }
                igPopID();
            }
            igEndChild();
        }
        if(sidebar_section(&sidebar,GD_SIDEBAR_SELECTION)){
            if(p->view==GD_EXPLORER_NOUNS) noun_grid_details(&c);
            else if (is_graph(p->view)) { if (!(grid_view(p) && grid_blank_details(&c)) && gd_explorer_graph_details(&c)) gd_explorer_edited(project); }
            else if (p->view==GD_EXPLORER_MDA) gd_explorer_mda_details(&c);
            else if (query_view(p->view)) gd_explorer_query_details(&c);
            else gd_explorer_resource_details(&c);
        }
        if(sidebar_section(&sidebar,GD_SIDEBAR_SOURCES)){
        if (viewer.server) igText("IPC session: %s",gd_viewer_session(viewer.server));
        igTextWrapped("UI fold preferences: %s",*sidebar.path ? sidebar.path:"session only");
        refresh=igButton("Refresh sources (F5)",(ImVec2_c){0,0});
        if (project->draft) {
            igText("%d modified records | %d undo / %d redo steps",project->draft->modified_count,project->draft->history_count,project->draft->redo_count);
            igTextUnformatted("Report path (empty: stdout; new .json file only)",NULL); igSetNextItemWidth(-1);
            igInputText("##explorer-report-path",report_path,sizeof(report_path),0,NULL,NULL);
            if (igButton("Finish and report changes",(ImVec2_c){0,0})) close_next=1;
            if (*project->draft->message) igTextWrapped("%s",project->draft->message);
        }
        }
        if(project->draft && !(sidebar.open&(1u<<GD_SIDEBAR_SOURCES)) && *project->draft->message)igTextWrapped("%s",project->draft->message);
        if(sidebar_section(&sidebar,GD_SIDEBAR_DIAGNOSTICS)){
            const GD_Finding *findings=p->view==GD_EXPLORER_MDA ? project->mda->findings:project->rules && (p->view==GD_EXPLORER_RESOURCES || query_view(p->view)) ? project->rules->findings:model->findings;
            int count_findings=p->view==GD_EXPLORER_MDA ? project->mda->finding_count:(p->view==GD_EXPLORER_RESOURCES || query_view(p->view)) ? project->rules->finding_count:model->finding_count;
            int omitted=p->view==GD_EXPLORER_MDA ? project->mda->findings_omitted:(p->view==GD_EXPLORER_RESOURCES || query_view(p->view)) ? project->rules->findings_omitted:model->findings_omitted;
            int target=gd_diagnostics_panel(&p->diagnostics,model,findings,count_findings,omitted,selected_record(&c));
            if (target>=0) {
                if (p->view==GD_EXPLORER_MDA) { p->mda.record=target; p->mda.link=-1; }
                else if (is_graph(p->view)) p->navigate=target;
                else { p->selection=(GD_ExplorerSelection){-1,-1,0,target,-1}; }
            }
        }
        igEnd();
        viewer_feedback(&viewer);
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
        if (!igGetIO_Nil()->WantCaptureKeyboard && !igIsAnyItemActive()) {
            if (find_shortcut==GD_FIND_FIT) { if (gd_explorer_similarity_active(&c)) p->structure_state->similarity_fit=true; else p->fit=1; }
            if (IsKeyPressed(KEY_F5)) refresh=1;
            if (!gd_explorer_similarity_active(&c) && IsKeyPressed(KEY_SPACE) && layout_settles(p)) p->paused=!p->paused;
        }
        if (!igGetIO_Nil()->WantTextInput && !igIsAnyItemActive() && (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))){
            if(IsKeyPressed(KEY_Z)){if(IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))redo=1;else undo=1;}
            if(IsKeyPressed(KEY_Y))redo=1;
        }
        if(refresh && project->draft && project->draft->redo_count && !project->draft->modified_count && !project->draft->history_count){refresh=0;igOpenPopup_Str("Discard Redo before refresh?",0);}
        if(igBeginPopupModal("Discard Redo before refresh?",NULL,ImGuiWindowFlags_AlwaysAutoResize)){
            igTextWrapped("Refreshing replaces the draft baseline. Discard available Redo steps? Source files stay unchanged.");
            if(igButton("Discard Redo and refresh",(ImVec2_c){0,0})){gd_explorer_action_run(&c,GD_ACTION_DISCARD_REDO);refresh=1;igCloseCurrentPopup();}
            igSameLine(0,8);if(igButton("Cancel",(ImVec2_c){0,0}))igCloseCurrentPopup();igEndPopup();
        }
        /* Both Undo inputs run after inspector readers. Repair revision before
           another selection reader can reinterpret compacted edge offsets. */
        if (undo) undo_draft(&c,panes);
        if (redo) gd_explorer_action_run(&c,GD_ACTION_REDO);
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
        selected=analyze(&c); matches(&c); if (!gd_explorer_similarity_active(&c)) distance_focus_update(&c,selected);
        /* Reserve fixed-screen status/legend band outside grid pan/zoom and hit area. */
        if ((grid_view(p) || p->view==GD_EXPLORER_NOUNS) && !gd_explorer_similarity_active(&c)) { c.viewport.y+=GD_COUNT_GRID_HEADER; c.viewport.height-=GD_COUNT_GRID_HEADER; }
        p->camera.offset=(Vector2){c.viewport.x+c.viewport.width/2,c.viewport.y+c.viewport.height/2};
        c.display_layout=NULL;if(p->fit)render->valid=0;
        if (!gd_explorer_similarity_active(&c) && p->fit) { fit_view(&c,p->fit==GD_EXPLORER_FIT_SELECTION ? GD_EXPLORER_FIT_SELECTION:p->fit==GD_EXPLORER_FIT_QUERY_READABLE ? 4:p->fit==GD_EXPLORER_FIT_GRID_READABLE ? -1:p->fit==GD_EXPLORER_FIT_RESOURCE_WITNESS ? 3:p->fit==2 ? 1:p->fit==3 ? 2:0); p->fit=0; }
        if(render_active(&c)){if(gd_render_pose_matches(render,p->layout,render_identity(&c)))c.display_layout=&render->layout;else render_update(&c,render,0,1);}else render->valid=0;
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !igGetIO_Nil()->WantCaptureMouse && CheckCollisionPointRec(GetMousePosition(),c.viewport))p->find_hold_node=-1;
        if (gd_explorer_similarity_active(&c)) gd_explorer_similarity_input(&c);
        else if (grid_view(p)) grid_input(&c);
        else if (p->view==GD_EXPLORER_NOUNS) noun_graph_input(&c);
        else if (is_graph(p->view)) gd_explorer_graph_input(&c);
        else if (p->view==GD_EXPLORER_MDA) gd_explorer_mda_input(&c);
        else if (query_view(p->view)) gd_explorer_query_input(&c);
        else resource_input(&c);
        c.display_layout=NULL;
        if (!p->distance_focus_enabled && p->distance_focus && p->distance_focus->animating &&
            (p->view==GD_EXPLORER_MDA ? p->mda.held:p->selection.held)>=0)
            p->distance_focus->animating=p->distance_focus->returning=0;
        if (!gd_explorer_similarity_active(&c) && layout_settles(p) && !p->distance_focus_enabled && !(p->distance_focus && p->distance_focus->animating) && !p->paused) {
            int held=p->view==GD_EXPLORER_MDA ? p->mda.held:p->selection.held;
            if(query_view(p->view)) gd_query_scene_step(p->query_state->scene,held);else gd_layout_step(p->layout,held);
        }
        selected=analyze(&c); if (!gd_explorer_similarity_active(&c)) distance_focus_update(&c,selected);
        if (!gd_explorer_similarity_active(&c) && !grid_view(p) && p->distance_focus && p->distance_focus->animating)
            gd_layout_focus_advance(p->distance_focus,p->layout,GetFrameTime(),p->view==GD_EXPLORER_MDA && p->layout->mode==GD_LAYOUT_FORCE ? 3:1);
        if (!gd_explorer_similarity_active(&c) && p->fit==3) { fit_view(&c,2); p->fit=0; }
        if(render_active(&c))render_update(&c,render,GetFrameTime(),0);
        {
            GD_ExplorerEvidence e=evidence(&c,selected); viewer_record_layers(&viewer,&c);
            if (gd_explorer_similarity_active(&c)) gd_explorer_similarity_draw(&c);
            else if (grid_view(p)) grid_draw(&c);
            else if (p->view==GD_EXPLORER_NOUNS) noun_graph_draw(&c);
            else if (is_graph(p->view)) gd_explorer_graph_draw(&c,&e);
            else if (p->view==GD_EXPLORER_MDA) gd_explorer_mda_draw(&c,&e);
            else if (query_view(p->view)) gd_explorer_query_draw(&c);
            else gd_explorer_resource_draw(&c,&e);
        }
        pair_selection_draw(&c);
        if(!gd_explorer_similarity_active(&c) && free_graph(p)){
            gd_graph_workspace_draw(p->layout,p->camera,c.viewport,p->view==GD_EXPLORER_MDA ? 3:1);
            gd_graph_islands_draw(p->layout,c.display_layout ? c.display_layout:p->layout,query_view(p->view) ? NULL:c.analysis->shown,p->camera,c.viewport,p->view==GD_EXPLORER_MDA ? 3:1);
        }
        if (!gd_explorer_similarity_active(&c)) { draw_matches(&c); if (p->view!=GD_EXPLORER_NOUNS && !grid_view(p) && !query_view(p->view)) gd_graph_overview_draw(c.analysis,&p->overview,c.viewport,!igGetIO_Nil()->WantCaptureMouse); }
        DrawText(gd_explorer_similarity_active(&c) ? "Similarity: Click inspect | Shift-click compare | Wheel zoom | Right/middle pan | F fit | F5 refresh":grid_view(p) ? "Grid: Arrows select | PageUp/Down cases | Wheel zoom | Right/middle drag pan | F fit | F5 refresh":p->distance_focus_enabled ? "Distance focus: click node to refocus | Off: restore pose/drag | Pan/zoom unchanged | F: fit all":"Drag: layout | Right click: Find | Right/middle drag: pan | Wheel: zoom | F: fit | F5: refresh",20,height-36,14,muted);
        DrawText(gd_explorer_similarity_active(&c) ? "Selected typed-feature distance approximation only; missing entries remain unplaced. No importance or execution proof.":incidence(p->view) ? "Shown incidence only; not flow, causality or equivalence.":"Authored declarations only; not learning, causality or physical reachability.",20,height-18,13,muted);
        c.display_layout=NULL;p->focus_camera_override=false;
        rlImGuiEnd(); EndDrawing(); ++frame;
        if (screenshot && frame==frames && !gd_save_screenshot(screenshot)) { gd_error(diagnostic,"view.screenshot","Explorer screenshot export failed",screenshot,0); result=2; break; }
        if(!igGetIO_Nil()->WantTextInput && !igIsAnyItemActive() && !igIsPopupOpen_Str(NULL,ImGuiPopupFlags_AnyPopupId|ImGuiPopupFlags_AnyPopupLevel)){
            if(IsMouseButtonPressed(MOUSE_BUTTON_BACK))navigation=-1;if(IsMouseButtonPressed(MOUSE_BUTTON_FORWARD))navigation=1;
        }
        if(navigation)gd_explorer_history_move(&c,navigation);
        if (requested>=0 || find_view>=0 || relationship_root>=0) {
            int changed=relationship_root>=0 ? center_relationships(&c,panes,relationship_root):
                find_view>=0 ? change_view(&c,panes,find_view,find_reference):change_view(&c,panes,requested,NULL);
            if (!changed) snprintf(failure,sizeof(failure),"Cannot leave/reset view: %s",project->draft ? project->draft->message:"unavailable view");
            active=c.pane->view;
        }
        if (refresh) {
            c.pane=&panes[GD_EXPLORER_SKILLS];
            if (gd_explorer_graph_commit(&c)<0)
                snprintf(failure,sizeof(failure),"Refresh blocked: editor invalid. User input retained.");
            else {
                int updated=refresh_sources(&c,panes,collections,count,failure,sizeof(failure));
                if (updated>=0) stale=!updated;
                if (updated>0) ++viewer.reload_generation;
                project=c.project;
            }
            c.pane=&panes[active];
        }
        if (viewer.has_hint && !strcmp(viewer.hint.command,"navigate") && !viewer_interacting()) {
            char reply[1024]; GD_ViewerWriter writer={reply,0,sizeof(reply),0};
            if (!viewer_hint_current(&viewer)) gd_viewer_error(&writer,"viewer.stale_hint","Exact-node hint predates source reload or draft change; request a fresh hint");
            else { viewer.human_go=1; viewer_dispatch(&viewer,&viewer.hint,&writer); viewer.human_go=0; }
            if (!strncmp(reply,"{\"ok\":true",10)) viewer.has_hint=0;
            else snprintf(failure,sizeof(failure),"Agent hint not opened: %s",reply);
            snprintf(viewer.hint.command,sizeof(viewer.hint.command),"hint");
        }
        if(reset_all && gd_explorer_action_run(&c,GD_ACTION_RESET_ALL)){render->valid=0;viewer.has_hint=viewer.hint_pending=0;*viewer.layer_token=*viewer.knowledge_token=0;*failure=0;}
        gd_viewer_server_poll(viewer.server,viewer_dispatch,&viewer);
        project=c.project; /* IPC refresh can replace snapshot owner after rendering. */
        viewer_apply_hint(&viewer);
        if(!igGetIO_Nil()->WantTextInput)history_observe(&c);
    }
    rlImGuiShutdown(); CloseWindow();
cleanup:
    gd_viewer_server_close(viewer.server);
    if (panes) {
        for (v=0;v<GD_EXPLORER_VIEWS;++v) {
            gd_diagnostics_clear(&panes[v].diagnostics);
            gd_layout_focus_destroy(panes[v].distance_focus); gd_resource_routes_destroy(panes[v].resource_routes);
            pane_release_layout(allocator,&panes[v]);
        }
        allocator.release(allocator.context,panes);
    }
    if(history)allocator.release(allocator.context,history);
    if(render)allocator.release(allocator.context,render);
    if (c.query) allocator.release(allocator.context,c.query);
    if (c.analysis) allocator.release(allocator.context,c.analysis);
    gd_explorer_destroy(project); return result;
}
#ifdef GD_EXPLORER_TEST
#include "test_explorer_viewer.h"
#include "test_explorer_find.h"
#include "test_explorer_shared_ux.h"
#include "test_explorer_view.h"
#endif
