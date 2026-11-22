#include "gd_explorer_ui.h"
#include "gd_explorer_controls.h"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_NO_EXPORT
#include "cimgui.h"
#include "gd_inspector_ipc_ui.h"
#include "gd_explorer_picker.h"
#include "gd_similarity_view.h"
#include <math.h>
#include <string.h>

static const GD_Color foreground={221,226,235,255},muted={151,165,185,255},first_color={100,180,255,255},second_color={255,191,90,255};
int gd_explorer_similarity_active(const GD_ExplorerContext *context) {
    const GD_ExplorerStructureState *state=context->pane->structure_state;
    return state && state->result && state->result->similarity && state->similarity_canvas;
}
int gd_explorer_similarity_available(const GD_ExplorerContext *context) {
    const GD_ExplorerStructureState *state=context->pane->structure_state;
    return state && state->result && state->result->project==context->project && state->result->similarity && state->result->similarity->table_complete &&
        (gd_explorer_family(context->pane->view)!=GD_EXPLORER_SKILL_DATA || state->revision==context->project->revision);
}
int gd_explorer_similarity_source_record(const GD_ExplorerContext *context) {
    const GD_ExplorerStructureState *state=context->pane->structure_state;const GD_SimilarityProjection *projection;int node;
    if(!gd_explorer_similarity_available(context))return -1;projection=state->result->similarity;
    if(state->similarity_row<0 || state->similarity_row>=projection->entry_count)return -1;node=projection->nodes[state->similarity_row];
    if(!projection->context || !projection->context->layout || node<0 || node>=projection->context->layout->node_count)return -1;
    return gd_structure_result_record(projection->context,projection->context->layout->nodes[node].key);
}
int gd_explorer_similarity_source(GD_ExplorerContext *context) {return gd_explorer_source_open(context,gd_explorer_similarity_source_record(context));}
int gd_explorer_similarity_return(GD_ExplorerContext *context) {
    if(!gd_explorer_similarity_available(context))return 0;context->pane->structure_state->similarity_canvas=true;return 1;
}
static int append(char *text,size_t capacity,const char *value) {
    size_t before=strlen(text),size=strlen(value);
    if (before+size+(before ? 1:0)>=capacity) return 0;
    if (before) text[before++]='\n'; memcpy(text+before,value,size+1); return 1;
}
int gd_explorer_similarity_prepare_view(GD_ExplorerStructureState *state) {
    const GD_SimilarityProjection *projection=state->result ? state->result->similarity:NULL;
    if(!projection || !projection->map || !projection->map->available)return 1;
    if(projection->entry_count!=projection->map->input.entry_count)return 0;
    if(state->similarity_view)return 1;
    GD_SimilarityView *view=state->allocator.allocate(state->allocator.context,sizeof(*view));
    if(!view)return 0;
    memset(view,0,sizeof(*view));
    if(!gd_similarity_view_build(view,projection->map)){state->allocator.release(state->allocator.context,view);return 0;}
    state->similarity_view=view;return 1;
}
int gd_explorer_similarity_select(GD_ExplorerContext *context,int row) {
    GD_ExplorerStructureState *state=context->pane->structure_state;
    const GD_Similarity *map;
    if (!gd_explorer_similarity_active(context) || !(map=state->result->similarity->map) || !map->available || row<0 || row>=map->input.entry_count || !map->placed[row]) return 0;
    gd_similarity_view_choose(state,row,0); state->similarity_camera.target=gd_similarity_view_point(map,row);
    state->similarity_camera.offset=(GD_Point){context->viewport.x+context->viewport.width/2,context->viewport.y+context->viewport.height/2};
    state->similarity_camera.zoom=1; state->similarity_camera_ready=true; state->similarity_fit=false; return 1;
}
static int similarity_focus(GD_ExplorerContext *context,int frame) {
    GD_ExplorerPane *pane=context->pane;GD_ExplorerStructureState *state=pane->structure_state;GD_ExplorerFocusResult result={0};
    const GD_Similarity *map;GD_Rect selected,bounds;GD_Point primary;int row,peer;
    if(!gd_explorer_similarity_active(context) || !(context->viewport.width>0) || !(context->viewport.height>0))return 0;
    map=state->result->similarity->map;row=state->similarity_row;peer=state->similarity_peer;result.zoom=state->similarity_camera.zoom;
    if(!map || !map->available || row<0 || row>=map->input.entry_count || !map->placed[row]){result.status=GD_FOCUS_UNPLACED;pane->focus_result=result;return 1;}
    primary=gd_similarity_view_point(map,row);selected=(GD_Rect){primary.x-16,primary.y-16,32,32};result.selected=1;
    if(peer>=0 && peer<map->input.entry_count && peer!=row){if(map->placed[peer]){GD_Point other=gd_similarity_view_point(map,peer);float left=fminf(selected.x,other.x-16),top=fminf(selected.y,other.y-16);selected.width=fmaxf(selected.x+selected.width,other.x+16)-left;selected.height=fmaxf(selected.y+selected.height,other.y+16)-top;selected.x=left;selected.y=top;++result.selected;}else ++result.hidden;}
    bounds=selected;
    if(frame){state->similarity_camera.target=(GD_Point){selected.x+selected.width/2,selected.y+selected.height/2};state->similarity_camera.offset=(GD_Point){context->viewport.x+context->viewport.width/2,context->viewport.y+context->viewport.height/2};state->similarity_camera.zoom=fmaxf(0.00001f,fminf(1.2f,fminf(context->viewport.width/(selected.width+60),context->viewport.height/(selected.height+60))));result.zoom=state->similarity_camera.zoom;}
    else gd_explorer_focus_camera(&state->similarity_camera,context->viewport,selected,bounds,primary,&result);
    result.status=result.clipped || result.hidden ? GD_FOCUS_PARTIAL:GD_FOCUS_FRAMED;pane->focus_result=result;
    state->similarity_camera_ready=true;state->similarity_fit=false;return 1;
}
int gd_explorer_similarity_focus(GD_ExplorerContext *context){return similarity_focus(context,0);}
int gd_explorer_similarity_frame(GD_ExplorerContext *context){return similarity_focus(context,1);}
static GD_Rect legend_bounds(const GD_ExplorerContext *context) {
    return (GD_Rect){context->viewport.x+6,context->viewport.y+5,fminf(454,context->viewport.width-12),66};
}
static GD_Rect similarity_footer(GD_Rect viewport) {
    return (GD_Rect){viewport.x+6,viewport.y+viewport.height-45,viewport.width-12,40};
}
static void fit(GD_ExplorerContext *context) {
    GD_ExplorerStructureState *state=context->pane->structure_state; const GD_Similarity *map=state->result->similarity->map;
    int row,found=0; float left=0,right=0,top=0,bottom=0;
    if (map && map->available) for (row=0;row<map->input.entry_count;++row) if (map->placed[row]) {
        GD_Point value=gd_similarity_view_point(map,row);
        if (!found || value.x<left) left=value.x; if (!found || value.x>right) right=value.x;
        if (!found || value.y<top) top=value.y; if (!found || value.y>bottom) bottom=value.y; found=1;
    }
    state->similarity_camera.target=(GD_Point){(left+right)/2,(top+bottom)/2};
    float inset=legend_bounds(context).height+12,available=fmaxf(40,context->viewport.height-inset-40);
    state->similarity_camera.zoom=fminf(fmaxf(40,context->viewport.width-48)/fmaxf(1,right-left),fmaxf(40,available-48)/fmaxf(1,bottom-top));
    if (state->similarity_camera.zoom>4) state->similarity_camera.zoom=4;
    if (state->similarity_camera.zoom<0.00001f) state->similarity_camera.zoom=0.00001f;
    state->similarity_camera.target.y-=(inset-40)/(2*state->similarity_camera.zoom);
    state->similarity_camera_ready=true; state->similarity_fit=false;
}
static const char overlap_popup[]="Similarity entries under pointer";
void gd_explorer_similarity_input(GD_ExplorerContext *context) {
    GD_ExplorerStructureState *state=context->pane->structure_state;const GD_SimilarityProjection *projection=state->result->similarity;const GD_Similarity *map=projection->map;
    GD_SimilarityView *view=state->similarity_view;GD_Camera *camera=&state->similarity_camera;GD_Point mouse=gd_mouse_position();
    if(view)view->hover_count=0;
    camera->offset=(GD_Point){context->viewport.x+context->viewport.width/2,context->viewport.y+context->viewport.height/2};
    if(!state->similarity_camera_ready || state->similarity_fit)fit(context);
    if(igGetIO_Nil()->WantCaptureMouse || !gd_rect_hit(mouse,context->viewport) || (view && view->pick_open && igIsPopupOpen_Str(overlap_popup,0)))return;
    {
        float wheel=gd_mouse_wheel();
        if(wheel){GD_Point before=gd_screen_to_world(mouse,*camera),after;camera->zoom*=powf(1.15f,wheel);if(camera->zoom<0.00001f)camera->zoom=0.00001f;if(camera->zoom>1000)camera->zoom=1000;after=gd_screen_to_world(mouse,*camera);camera->target.x+=before.x-after.x;camera->target.y+=before.y-after.y;}
    }
    if(igIsMouseDown_Nil(ImGuiMouseButton_Middle) || igIsMouseDown_Nil(ImGuiMouseButton_Right)){
        GD_Point delta=gd_mouse_delta();camera->target.x-=delta.x/camera->zoom;camera->target.y-=delta.y/camera->zoom;
    }
    if(!map || !map->available || !view || gd_rect_hit(mouse,legend_bounds(context)))return;
    if((state->similarity_row>=0 || state->similarity_peer>=0) && gd_rect_hit(mouse,similarity_footer(context->viewport)))return;
    view->hover_count=gd_similarity_view_hits(map,*camera,mouse,view->hover_rows,GD_SIMILARITY_HOVER_ROWS);
    if(!view->hover_count || !gd_mouse_pressed(ImGuiMouseButton_Left))return;
    int peer=igIsKeyDown_Nil(ImGuiKey_LeftShift) || igIsKeyDown_Nil(ImGuiKey_RightShift);
    if(view->hover_count==1){gd_similarity_view_choose(state,view->hover_rows[0],peer);return;}
    view->pick_count=gd_similarity_view_hits(map,*camera,mouse,view->pick_rows,GD_MAX_RECORDS);
    view->pick_page=0;view->pick_peer=peer;view->pick_open=1;
    /* ImGui owns this tag; compare only, never dereference after replacement. */
    ImGuiStorage_SetVoidPtr(igGetStateStorage(),igGetID_Str("Similarity popup owner"),state);igOpenPopup_Str(overlap_popup,0);
}
static void draw_name(GD_ExplorerContext *context,GD_SimilarityView *view,GD_Point point,const char *name,GD_Color color,int priority,int below) {
    GD_Rect viewport=context->viewport;
    if(!gd_rect_hit(point,viewport))return;
    if(view->label_count==GD_SIMILARITY_LABELS){++view->hidden_labels;return;}
    float width=fminf((float)gd_scene_text_width(name,15)+8,fminf(300,viewport.width-20));
    GD_Rect label={point.x+18,point.y+(below ? 8:-22),width,20};
    if(label.x+label.width>viewport.x+viewport.width)label.x=point.x-width-18;
    if(label.x<viewport.x)label.x=viewport.x;
    if(width<20){++view->hidden_labels;return;}
    if(!priority)for(int i=0;i<view->label_count;++i)if(gd_rect_intersects(label,view->labels[i])){++view->hidden_labels;return;}
    view->labels[view->label_count++]=label;gd_scene_rect(label,(GD_Color){24,29,38,240});
    GD_Rect clip=gd_rect_overlap(label,viewport);
    if(clip.width>0 && clip.height>0){gd_scene_clip((int)clip.x,(int)clip.y,(int)clip.width,(int)clip.height);gd_scene_text(name,(int)label.x+4,(int)label.y+2,15,color);gd_scene_unclip();}
}
void gd_explorer_similarity_draw(GD_ExplorerContext *context) {
    GD_ExplorerStructureState *state=context->pane->structure_state;const GD_SimilarityProjection *projection=state->result->similarity;const GD_Similarity *map=projection->map;GD_SimilarityView *view=state->similarity_view;
    GD_Rect viewport=context->viewport,legend=legend_bounds(context),footer=similarity_footer(viewport);
    int rows[2]={state->similarity_row,state->similarity_peer},show_footer=rows[0]>=0 || rows[1]>=0;
    gd_scene_clip((int)viewport.x,(int)viewport.y,(int)viewport.width,(int)viewport.height);
    if(!map || !map->available || !view){gd_scene_text("Similarity fit unavailable. Inspect limits/unplaced context in controls.",(int)viewport.x+15,(int)viewport.y+20,17,muted);gd_scene_unclip();return;}
    view->label_count=1;view->labels[0]=legend;view->hidden_labels=0;
    if(show_footer)view->labels[view->label_count++]=footer;
    for(int group=0;group<view->group_count;++group){
        const GD_SimilarityViewPoint *value=&view->groups[group];GD_Point screen=gd_world_to_screen(value->point,state->similarity_camera);
        if(!gd_rect_hit(screen,viewport))continue;
        gd_scene_circle(screen,value->count>1 ? 14:4,muted);
        if(value->count>1){const char *number=gd_scene_format("%d",value->count);gd_scene_text(number,(int)screen.x-gd_scene_text_width(number,10)/2,(int)screen.y-5,10,(GD_Color){20,25,34,255});}
    }
    for(int role=0;role<2;++role){
        int row=rows[role];GD_Color color=role ? second_color:first_color;
        if(row>=0 && row<projection->entry_count && map->placed[row]){
            GD_Point screen=gd_world_to_screen(gd_similarity_view_point(map,row),state->similarity_camera);
            if(role)gd_scene_outline((GD_Rect){screen.x-17,screen.y-17,34,34},2,color);
            else gd_scene_ring_pixels((int)screen.x,(int)screen.y,20,color);
            draw_name(context,view,screen,gd_similarity_view_name(projection,row),color,1,role);
        }
    }
    for(int group=0;group<view->group_count;++group){
        const GD_SimilarityViewPoint *value=&view->groups[group];int selected=0;
        for(int role=0;role<2;++role)if(rows[role]>=0 && rows[role]<projection->entry_count && map->placed[rows[role]]){GD_Point p=gd_similarity_view_point(map,rows[role]);if(p.x==value->point.x && p.y==value->point.y)selected=1;}
        if(!selected && value->count==1)draw_name(context,view,gd_world_to_screen(value->point,state->similarity_camera),gd_similarity_view_name(projection,value->row),foreground,0,0);
    }
    gd_scene_clip((int)legend.x,(int)legend.y,(int)legend.width,(int)legend.height);
    gd_scene_rect(legend,(GD_Color){24,29,38,245});int x=(int)legend.x+8,y=(int)legend.y+5;
    gd_scene_text(gd_scene_format("%d placed | %d missing | %d measurements",map->complete_entries,map->missing_entries,projection->feature_count),x,y,16,foreground);
    gd_scene_text("Approximate feature distance. Hover for measurements.",x,y+21,14,muted);
    gd_scene_text(gd_scene_format("%d crowded labels hidden. Hover or click to inspect.",view->hidden_labels),x,y+40,14,muted);
    gd_scene_unclip();
    if(show_footer){
        gd_scene_rect(footer,(GD_Color){24,29,38,245});
        for(int role=0;role<2;++role)if(rows[role]>=0 && rows[role]<projection->entry_count)
            gd_scene_text(gd_scene_format("%s: %s",role ? "Peer":"Selected",gd_similarity_view_name(projection,rows[role])),(int)footer.x+8,(int)footer.y+3+17*role,15,role ? second_color:first_color);
    }
    gd_scene_unclip();
}
/* Tooltips share an ImGui window. Measure this frame's text before Begin so
   previous legend/entry content cannot supply first-hover size or position. */
static void tooltip_lines(const char *const *lines,int count) {
    ImGuiStyle *style=igGetStyle();ImVec2_c display=igGetIO_Nil()->DisplaySize;
    float width=160,height=0;
    for(int line=0;line<count;++line){ImVec2_c size=igCalcTextSize(lines[line],NULL,false,0);width=fmaxf(width,size.x);}
    width=fmaxf(1,fminf(width,fminf(520,display.x-2*style->WindowPadding.x-32)));
    for(int line=0;line<count;++line){ImVec2_c size=igCalcTextSize(lines[line],NULL,false,width);height+=size.y+style->ItemSpacing.y;}
    height=fmaxf(0,height-style->ItemSpacing.y);
    igSetNextWindowContentSize((ImVec2_c){width,height});
    igSetNextWindowSize((ImVec2_c){width+2*style->WindowPadding.x,height+2*style->WindowPadding.y},ImGuiCond_Always);
    igSetNextWindowSizeConstraints((ImVec2_c){0,0},(ImVec2_c){fmaxf(1,display.x-16),fmaxf(1,display.y-16)},NULL,NULL);
    if(igBeginTooltip()){
        igPushTextWrapPos(igGetCursorPosX()+width);
        for(int line=0;line<count;++line)igTextUnformatted(lines[line],NULL);
        igPopTextWrapPos();igEndTooltip();
    }
}
/* Shell calls even outside similarity, so stale owner/pane popups close before
   accessing borrowed rows. All popup selection uses existing inspection path. */
void gd_explorer_similarity_popup(GD_ExplorerContext *context) {
    GD_ExplorerStructureState *state=context->pane->structure_state;GD_SimilarityView *view=state ? state->similarity_view:NULL;
    int active=gd_explorer_similarity_active(context);const GD_SimilarityProjection *projection=active ? state->result->similarity:NULL;
    const void *owner=ImGuiStorage_GetVoidPtr(igGetStateStorage(),igGetID_Str("Similarity popup owner"));
    if(igBeginPopup(overlap_popup,ImGuiWindowFlags_AlwaysAutoResize)){
        if(owner!=state || !active || !view || !view->pick_open || (igIsKeyPressed_Bool(ImGuiKey_Escape,false) && !igGetIO_Nil()->WantTextInput && !igIsAnyItemActive())){if(view)view->pick_open=0;igCloseCurrentPopup();}
        else {
            igText("Choose %s | %d entries under pointer",view->pick_peer ? "comparison peer":"inspected entry",view->pick_count);
            GD_EvidencePage page=gd_explorer_evidence_pager("Entries under pointer",&view->pick_page,view->pick_count,GD_EVIDENCE_PAGE_ROWS);
            igBeginChild_Str("Named overlap entries",(ImVec2_c){480,320},0,0);
            for(int item=page.first;item<page.end;++item){int row=view->pick_rows[item];igPushID_Int(row);
                if(igSmallButton(view->pick_peer ? "Use as peer":"Inspect")){gd_similarity_view_choose(state,row,view->pick_peer);view->pick_open=0;igCloseCurrentPopup();}
                igSameLine(0,-1);igTextUnformatted(gd_similarity_view_name(projection,row),NULL);
                igTextWrapped("%s",projection->ids[row]);igSeparator();igPopID();
            }
            igEndChild();if(igButton("Cancel",(ImVec2_c){0,0})){view->pick_open=0;igCloseCurrentPopup();}
        }
        igEndPopup();return;
    }
    if(view)view->pick_open=0;
    if(!active || !view || igGetIO_Nil()->WantCaptureMouse)return;
    if(gd_rect_hit(gd_mouse_position(),legend_bounds(context))){
        const char *lines[GD_SIMILARITY_HOVER_ROWS*4+3];char weights[GD_SIMILARITY_HOVER_ROWS][64],more[96];int count=0;
        lines[count++]="Applied measurements and weights (pending edits do not change this map)";
        for(int feature=0;feature<projection->feature_count && feature<GD_SIMILARITY_HOVER_ROWS;++feature){
            const GD_SimilarityFeature *value=&projection->features[feature];
            snprintf(weights[feature],sizeof(weights[feature]),"weight %.17g%s",value->weight,value->unit ? " | unit:":"");
            lines[count++]=value->name;lines[count++]=weights[feature];if(value->unit)lines[count++]=value->unit;lines[count++]=gd_similarity_view_measurement(projection,feature);
        }
        if(projection->feature_count>GD_SIMILARITY_HOVER_ROWS){snprintf(more,sizeof(more),"%d more applied measurements in selected-entry inspector",projection->feature_count-GD_SIMILARITY_HOVER_ROWS);lines[count++]=more;}
        lines[count++]="Native distance is weighted mean difference: numbers use observed complete-entry ranges, names use exact equality, sets use retained-member Jaccard distance. Missing values exclude entries. These measurements do not establish gameplay quality; coincident 2D positions do not prove identical feature values or equivalent gameplay.";
        tooltip_lines(lines,count);
    } else if(view->hover_count){
        const char *lines[GD_SIMILARITY_HOVER_ROWS*2+2];char header[64],more[96];int line=0;
        snprintf(header,sizeof(header),"%d %s under pointer",view->hover_count,view->hover_count==1 ? "entry":"entries");lines[line++]=header;
        int count=view->hover_count<GD_SIMILARITY_HOVER_ROWS ? view->hover_count:GD_SIMILARITY_HOVER_ROWS;
        for(int i=0;i<count;++i){int row=view->hover_rows[i];lines[line++]=gd_similarity_view_name(projection,row);lines[line++]=projection->ids[row];}
        if(view->hover_count>count){snprintf(more,sizeof(more),"%d more; click for paged named chooser",view->hover_count-count);lines[line++]=more;}
        else lines[line++]="Click to inspect; Shift-click to choose peer";
        tooltip_lines(lines,line);
    }
}
static void catalogue(GD_ExplorerContext *context) {
    GD_ExplorerStructureState *state=context->pane->structure_state; GD_SimilarityFeatureInfo values[GD_SIMILARITY_FEATURES];
    int i,count=gd_similarity_feature_catalogue(state->result,values);
    if (!igTreeNode_Str("Native feature catalogue (append to pending map)")) return;
    igTextWrapped("Capabilities depend on applied typed scope. Retained counts/sets are not proof of complete external source. Append IDs, edit literal weights, then Run.");
    GD_EvidencePage page=gd_explorer_evidence_pager("Feature catalogue",&state->similarity_feature_page,count,GD_EVIDENCE_PAGE_ROWS);
    igBeginChild_Str("Similarity feature catalogue",(ImVec2_c){0,190},0,0);
    for (i=page.first;i<page.end;++i) {
        igPushID_Int(i);
        if (igSmallButton(values[i].id)) {
            if (append(state->pending.features,sizeof(state->pending.features),values[i].id)) {
                state->pending.kind=GD_STRUCTURE_COUNT+1; state->pending.direction=GD_STRUCTURE_FORWARD; state->pending.lens=GD_GRAPH_ANY; *state->pending.starts=*state->pending.avoid=0;
            } else snprintf(context->pane->query_message,sizeof(context->pane->query_message),"Feature input exceeds 8192 bytes; edit list before appending");
        }
        igSameLine(0,-1); igText("%s%s%s",values[i].feature.kind==GD_SIMILARITY_NUMBER ? "number":values[i].feature.kind==GD_SIMILARITY_NAME ? "exact name":"target set",values[i].feature.unit ? " | ":"",values[i].feature.unit ? values[i].feature.unit:""); igPopID();
    }
    igEndChild(); igTreePop();
}
int gd_explorer_similarity_compare(GD_ExplorerContext *context) {
    GD_ExplorerStructureState *state=context->pane->structure_state;
    const GD_SimilarityProjection *projection=state && state->result ? state->result->similarity:NULL;
    if (!projection || state->similarity_row<0 || state->similarity_peer<0 || state->similarity_row>=projection->entry_count || state->similarity_peer>=projection->entry_count) return 0;
    if (state->comparison_first!=state->similarity_row || state->comparison_second!=state->similarity_peer) {
        state->comparison_first=state->similarity_row; state->comparison_second=state->similarity_peer; state->comparison_work=0;
        state->comparison_available=projection->map && gd_similarity_distance(projection->map,state->similarity_row,state->similarity_peer,state->comparison_components,&state->comparison_distance,&state->comparison_work,GD_SIMILARITY_WORK);
    }
    return state->comparison_available;
}
static void compare(GD_ExplorerContext *context) {
    GD_ExplorerStructureState *state=context->pane->structure_state; const GD_SimilarityProjection *projection=state->result->similarity; int i;
    if (state->similarity_row<0 || state->similarity_peer<0) return;
    gd_explorer_similarity_compare(context);
    igSeparator();igTextWrapped("Similarity pair: %s / %s",gd_similarity_view_name(projection,state->similarity_row),gd_similarity_view_name(projection,state->similarity_peer));
    igText("%d independent native distance work",state->comparison_work);
    if (!state->comparison_available) { igTextWrapped("Distance unavailable: missing selected cell, unfinished range validation or independent work ceiling. No value imputed."); return; }
    igText("Native weighted distance %.17g",state->comparison_distance);
    for (i=0;i<projection->feature_count;++i) igText("%s | unweighted %.17g | weight %.17g",projection->features[i].name,state->comparison_components[i],projection->features[i].weight);
}
static void inspect(GD_ExplorerContext *context) {
    GD_ExplorerStructureState *state=context->pane->structure_state; const GD_SimilarityProjection *projection=state->result->similarity; int row=state->similarity_row,feature;
    if (row<0 || row>=projection->entry_count) return;
    igSeparator();igTextWrapped("Selected: %s",gd_similarity_view_name(projection,row));igTextWrapped("%s",projection->ids[row]);
    if (projection->map && projection->map->available && projection->map->placed[row]) igText("Native coordinates %.17g, %.17g",projection->map->x[row],projection->map->y[row]);
    else igTextWrapped("Unplaced. Missing selected values exclude entry; unavailable fit withholds all coordinates. No '?' position is fabricated.");
    gd_explorer_action_button(context,GD_ACTION_SIMILARITY_SOURCE);
    igSameLine(0,-1); if (igSmallButton("Append exact ID to pending entries")) { if (!append(state->pending.entries,sizeof(state->pending.entries),projection->ids[row])) snprintf(context->pane->query_message,sizeof(context->pane->query_message),"Entry input exceeds 8192 bytes"); }
    for (feature=0;feature<projection->feature_count;++feature) {
        size_t offset=(size_t)row*projection->feature_count+feature; const GD_SimilarityCell *value=&projection->cells[offset]; const GD_SimilarityFeature *definition=&projection->features[feature];
        igPushID_Int(feature); igText("%s | weight %.17g%s%s",definition->name,definition->weight,definition->unit ? " | ":"",definition->unit ? definition->unit:"");
        if (!value->present) igText("Missing: %s",projection->missing[offset]==GD_SIMILARITY_NO_VARIANT ? "no selected hypothesis context":"absent authored field");
        else if (definition->kind==GD_SIMILARITY_NUMBER) { igText("Value %.17g | complete-row observed range",value->number); if (projection->map && projection->map->input_complete && projection->map->complete_entries) igText("%.17g .. %.17g",projection->map->low[feature],projection->map->high[feature]); else igTextUnformatted("Range unavailable",NULL); }
        else if (definition->kind==GD_SIMILARITY_NAME) igText("Exact value: %s",value->name);
        else {
            int reveal=state->reveal_similarity_feature==feature+1;
            if(reveal){igSetScrollHereY(0.1f);igSetNextItemOpen(true,ImGuiCond_Always);}
            if (igTreeNode_Str(gd_scene_format("%d exact context members (known empty differs missing)",value->count))) {
            int member;GD_EvidencePage page=gd_explorer_evidence_pager("Members",&state->similarity_member_pages[feature],value->count,GD_EVIDENCE_PAGE_ROWS);
            igBeginChild_Str("Feature members",(ImVec2_c){0,130},0,0);
            /* Exhaustive pagination; at most 64 widgets per expanded set. */
            for (member=page.first;member<page.end;++member) {
                int node=value->members[member]; igPushID_Int(member);
                if(reveal && state->reveal_similarity_member==member+1)igSetScrollFromPosY_Float(igGetCursorPosY(),0.1f);
                if (igSmallButton(gd_scene_format("#%d %s",node,projection->context->layout->nodes[node].label))) gd_explorer_structure_source_node(context,node);
                igPopID();
            }
            igEndChild(); igTreePop();
            }
        }
        igPopID();
    }
}
void gd_explorer_similarity_controls(GD_ExplorerContext *context) {
    GD_ExplorerStructureState *state=context->pane->structure_state; const GD_SimilarityProjection *projection=state->result->similarity; const GD_Similarity *map=projection ? projection->map:NULL; int row;GD_EvidencePage page;
    igSeparator(); igText("Applied: %s [%s]",state->result->options.question,state->result->options.projection);
    catalogue(context);
    if (!projection) { igTextWrapped("Discovery only; no distance, fit or source mutation performed. Select features for a map."); return; }
    igText("Current canvas: %s | %s",state->similarity_canvas ? "similarity map":"ordinary source",gd_explorer_view_names[context->pane->view]);
    if(!state->similarity_canvas){gd_explorer_action_button(context,GD_ACTION_SIMILARITY_RETURN);igTextWrapped("Return retains selected analytical row, comparison peer and map camera; no Run. Back remains scoped to current canvas mode.");}
    if(state->similarity_canvas){if(igSmallButton("Show ordinary canvas")){state->similarity_canvas=false;context->pane->fit=0;}igSameLine(0,-1);if(igSmallButton("Fit similarity map"))state->similarity_fit=true;}
    else if(igSmallButton("Frame source selection / all (F)"))gd_explorer_action_run(context,GD_ACTION_VIEW_FIT);
    igTextWrapped("Ordinary layout/camera stay separate. Native coordinates use selected-feature distance units; display uses one uniform scale. No force layout, inferred importance, outcome equivalence or '?' hub. Unplaced entries stay inspectable below.");
    igText("Work %d / %d | owned %llu / peak %llu bytes",projection->work,projection->work_limit,(unsigned long long)projection->owned_bytes,(unsigned long long)projection->peak_owned_bytes);
    igText("Table %s | source cap %s | storage cap %s | source incomplete %s",projection->table_complete ? "complete":"unfinished",projection->source_limited ? "hit":"no",projection->storage_limited ? "hit":"no",state->result->source_incomplete ? "yes":"no");
    if (map) {
        igText("Fit %s | failure %d | distance scale %.17g",map->available ? "available":"unavailable",(int)map->failure,map->distance_scale);
        if (map->input_complete) igText("Complete entries %d | missing excluded %d",map->complete_entries,map->missing_entries);
        igText("Distortion pairs %d | exhaustive %s | normalized SSE %.17g / source squares %.17g",map->distortion_pairs,map->distortion_complete ? "yes":"no",map->squared_error,map->squared_distance);
        if (map->squared_distance>0) igText("Sample normalized stress %.17g",sqrt(map->squared_error/map->squared_distance));
        igText("Negative eigenvalues %d | normalized negative mass %.17g",map->negative_eigenvalues,map->negative_eigenvalue_mass);
    }
    if (!projection->table_complete) { igTextWrapped("Feature validation/extraction unfinished. No input table or coordinates published; inspect native context/limits."); return; }
    igText("Comparison entries %d | full context %d shown nodes, %d rows",projection->entry_count,state->result->query->shown_nodes,state->result->query->shown_edges);
    page=gd_explorer_evidence_pager("Similarity entries",&state->similarity_page,projection->entry_count,GD_EVIDENCE_PAGE_ROWS);
    for (row=page.first;row<page.end;++row) {
        int placed=map && map->available && map->placed[row]; igPushID_Int(row);
        if (igSmallButton("Inspect")) gd_similarity_view_choose(state,row,0); igSameLine(0,-1);
        if (igSmallButton("Use as comparison peer")) gd_similarity_view_choose(state,row,1); igSameLine(0,-1);igTextUnformatted(gd_similarity_view_name(projection,row),NULL);
        igTextWrapped("%s | %s",placed ? "placed":"unplaced",projection->ids[row]);igPopID();
    }
    inspect(context); compare(context);
}
