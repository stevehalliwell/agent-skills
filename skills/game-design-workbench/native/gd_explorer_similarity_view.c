#include "gd_explorer_ui.h"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_NO_EXPORT
#include "cimgui.h"
#include <math.h>
#include <string.h>

static const Color foreground={221,226,235,255},muted={151,165,185,255},first_color={100,180,255,255},second_color={255,191,90,255};
int gd_explorer_similarity_active(const GD_ExplorerContext *context) {
    const GD_ExplorerStructureState *state=context->pane->structure_state;
    return state && state->result && state->result->similarity && state->similarity_canvas;
}
static int append(char *text,size_t capacity,const char *value) {
    size_t before=strlen(text),size=strlen(value);
    if (before+size+(before ? 1:0)>=capacity) return 0;
    if (before) text[before++]='\n'; memcpy(text+before,value,size+1); return 1;
}
static void choose_row(GD_ExplorerStructureState *state,int row,int peer) {
    if (peer) state->similarity_peer=row; else state->similarity_row=row;
}
static Vector2 point(const GD_Similarity *map,int row) {
    double scale=map->distance_scale>0 ? map->distance_scale:1;
    /* One uniform display scale only. Never change native distance coordinates,
       move individual points or feed map positions into ordinary layout. */
    return (Vector2){(float)(400*(map->x[row]/scale)),(float)(400*(map->y[row]/scale))};
}
int gd_explorer_similarity_select(GD_ExplorerContext *context,int row) {
    GD_ExplorerStructureState *state=context->pane->structure_state;
    const GD_Similarity *map;
    if (!gd_explorer_similarity_active(context) || !(map=state->result->similarity->map) || !map->available || row<0 || row>=map->input.entry_count || !map->placed[row]) return 0;
    choose_row(state,row,0); state->similarity_camera.target=point(map,row);
    state->similarity_camera.offset=(Vector2){context->viewport.x+context->viewport.width/2,context->viewport.y+context->viewport.height/2};
    state->similarity_camera.zoom=1; state->similarity_camera_ready=true; state->similarity_fit=false; return 1;
}
int gd_explorer_similarity_focus(GD_ExplorerContext *context) {
    GD_ExplorerPane *pane=context->pane;GD_ExplorerStructureState *state=pane->structure_state;GD_ExplorerFocusResult result={0};
    const GD_Similarity *map;Rectangle selected,bounds;Vector2 primary;int row,peer;
    if(!gd_explorer_similarity_active(context) || !(context->viewport.width>0) || !(context->viewport.height>0))return 0;
    map=state->result->similarity->map;row=state->similarity_row;peer=state->similarity_peer;result.zoom=state->similarity_camera.zoom;
    if(!map || !map->available || row<0 || row>=map->input.entry_count || !map->placed[row]){result.status=GD_FOCUS_UNPLACED;pane->focus_result=result;return 1;}
    primary=point(map,row);selected=(Rectangle){primary.x-16,primary.y-16,32,32};result.selected=1;
    if(peer>=0 && peer<map->input.entry_count && peer!=row){if(map->placed[peer]){Vector2 other=point(map,peer);float left=fminf(selected.x,other.x-16),top=fminf(selected.y,other.y-16);selected.width=fmaxf(selected.x+selected.width,other.x+16)-left;selected.height=fmaxf(selected.y+selected.height,other.y+16)-top;selected.x=left;selected.y=top;++result.selected;}else ++result.hidden;}
    bounds=selected;gd_explorer_focus_camera(&state->similarity_camera,context->viewport,selected,bounds,primary,&result);
    result.status=result.clipped || result.hidden ? GD_FOCUS_PARTIAL:GD_FOCUS_FRAMED;pane->focus_result=result;
    state->similarity_camera_ready=true;state->similarity_fit=false;return 1;
}
static void fit(GD_ExplorerContext *context) {
    GD_ExplorerStructureState *state=context->pane->structure_state; const GD_Similarity *map=state->result->similarity->map;
    int row,found=0; float left=0,right=0,top=0,bottom=0;
    if (map && map->available) for (row=0;row<map->input.entry_count;++row) if (map->placed[row]) {
        Vector2 value=point(map,row);
        if (!found || value.x<left) left=value.x; if (!found || value.x>right) right=value.x;
        if (!found || value.y<top) top=value.y; if (!found || value.y>bottom) bottom=value.y; found=1;
    }
    state->similarity_camera.target=(Vector2){(left+right)/2,(top+bottom)/2};
    state->similarity_camera.zoom=fminf(context->viewport.width/(right-left+80),context->viewport.height/(bottom-top+80));
    if (state->similarity_camera.zoom>4) state->similarity_camera.zoom=4;
    if (state->similarity_camera.zoom<0.00001f) state->similarity_camera.zoom=0.00001f;
    state->similarity_camera_ready=true; state->similarity_fit=false;
}
void gd_explorer_similarity_input(GD_ExplorerContext *context) {
    GD_ExplorerStructureState *state=context->pane->structure_state; const GD_Similarity *map=state->result->similarity->map;
    Camera2D *camera=&state->similarity_camera; Vector2 mouse=GetMousePosition(); int row,best=-1; float nearest=100;
    camera->offset=(Vector2){context->viewport.x+context->viewport.width/2,context->viewport.y+context->viewport.height/2};
    if (!state->similarity_camera_ready || state->similarity_fit) fit(context);
    if (igGetIO_Nil()->WantCaptureMouse || !CheckCollisionPointRec(mouse,context->viewport)) return;
    {
        float wheel=GetMouseWheelMove();
        if (wheel) {
            Vector2 before=GetScreenToWorld2D(mouse,*camera),after;
            camera->zoom*=powf(1.15f,wheel); if (camera->zoom<0.00001f) camera->zoom=0.00001f; if (camera->zoom>1000) camera->zoom=1000;
            after=GetScreenToWorld2D(mouse,*camera); camera->target.x+=before.x-after.x; camera->target.y+=before.y-after.y;
        }
    }
    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        Vector2 delta=GetMouseDelta(); camera->target.x-=delta.x/camera->zoom; camera->target.y-=delta.y/camera->zoom;
    }
    if (!map || !map->available || !IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return;
    /* Later painted point wins exact-distance ties, including coincident rows. */
    for (row=map->input.entry_count-1;row>=0;--row) if (map->placed[row]) {
        Vector2 screen=GetWorldToScreen2D(point(map,row),*camera); float dx=screen.x-mouse.x,dy=screen.y-mouse.y,distance=dx*dx+dy*dy;
        if (distance<nearest) { nearest=distance; best=row; }
    }
    if (best>=0) choose_row(state,best,IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT));
}
void gd_explorer_similarity_draw(GD_ExplorerContext *context) {
    GD_ExplorerStructureState *state=context->pane->structure_state; const GD_SimilarityProjection *projection=state->result->similarity; const GD_Similarity *map=projection->map; int row;
    BeginScissorMode((int)context->viewport.x,(int)context->viewport.y,(int)context->viewport.width,(int)context->viewport.height);
    if (!map || !map->available) DrawText("Similarity fit unavailable. Inspect limits/unplaced context in controls.",(int)context->viewport.x+15,(int)context->viewport.y+20,17,muted);
    else {
        BeginMode2D(state->similarity_camera);
        for (row=0;row<projection->entry_count;++row) if (map->placed[row]) {
            Vector2 value=point(map,row); Color color=row==state->similarity_row ? first_color:row==state->similarity_peer ? second_color:muted;
            DrawCircleV(value,(row==state->similarity_row || row==state->similarity_peer ? 7:4)/state->similarity_camera.zoom,color);
        }
        EndMode2D();
        for (row=0;row<projection->entry_count;++row) if (map->placed[row] && (projection->entry_count<=32 || row==state->similarity_row || row==state->similarity_peer)) {
            Vector2 screen=GetWorldToScreen2D(point(map,row),state->similarity_camera); DrawText(TextFormat("#%d",row),(int)screen.x+8,(int)screen.y-8,15,foreground);
        }
        DrawText(TextFormat("%d complete entries placed | %d missing entries unplaced | native distance scale %.5g",map->complete_entries,map->missing_entries,map->distance_scale),(int)context->viewport.x+10,(int)context->viewport.y+8,15,muted);
    }
    EndScissorMode();
}
static void catalogue(GD_ExplorerContext *context) {
    GD_ExplorerStructureState *state=context->pane->structure_state; GD_SimilarityFeatureInfo values[GD_SIMILARITY_FEATURES];
    int i,count=gd_similarity_feature_catalogue(state->result,values);
    if (!igTreeNode_Str("Native feature catalogue (append to pending map)")) return;
    igTextWrapped("Capabilities depend on applied typed scope. Retained counts/sets are not proof of complete external source. Append IDs, edit literal weights, then Run.");
    igBeginChild_Str("Similarity feature catalogue",(ImVec2_c){0,190},0,0);
    for (i=0;i<count;++i) {
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
    igSeparator(); igText("Pair #%d / #%d | %d independent native work",state->similarity_row,state->similarity_peer,state->comparison_work);
    if (!state->comparison_available) { igTextWrapped("Distance unavailable: missing selected cell, unfinished range validation or independent work ceiling. No value imputed."); return; }
    igText("Native weighted distance %.17g",state->comparison_distance);
    for (i=0;i<projection->feature_count;++i) igText("%s | unweighted %.17g | weight %.17g",projection->features[i].name,state->comparison_components[i],projection->features[i].weight);
}
static void inspect(GD_ExplorerContext *context) {
    GD_ExplorerStructureState *state=context->pane->structure_state; const GD_SimilarityProjection *projection=state->result->similarity; int row=state->similarity_row,feature;
    if (row<0 || row>=projection->entry_count) return;
    igSeparator(); igText("Selected #%d | %s",row,projection->ids[row]);
    if (projection->map && projection->map->available && projection->map->placed[row]) igText("Native coordinates %.17g, %.17g",projection->map->x[row],projection->map->y[row]);
    else igTextWrapped("Unplaced. Missing selected values exclude entry; unavailable fit withholds all coordinates. No '?' position is fabricated.");
    if (igSmallButton("Open linked source definition")) gd_explorer_structure_source_node(context,projection->nodes[row]);
    igSameLine(0,-1); if (igSmallButton("Append exact ID to pending entries")) { if (!append(state->pending.entries,sizeof(state->pending.entries),projection->ids[row])) snprintf(context->pane->query_message,sizeof(context->pane->query_message),"Entry input exceeds 8192 bytes"); }
    for (feature=0;feature<projection->feature_count;++feature) {
        size_t offset=(size_t)row*projection->feature_count+feature; const GD_SimilarityCell *value=&projection->cells[offset]; const GD_SimilarityFeature *definition=&projection->features[feature];
        igPushID_Int(feature); igText("%s | weight %.17g%s%s",definition->name,definition->weight,definition->unit ? " | ":"",definition->unit ? definition->unit:"");
        if (!value->present) igText("Missing: %s",projection->missing[offset]==GD_SIMILARITY_NO_VARIANT ? "no selected hypothesis context":"absent authored field");
        else if (definition->kind==GD_SIMILARITY_NUMBER) { igText("Value %.17g | complete-row observed range",value->number); if (projection->map && projection->map->input_complete && projection->map->complete_entries) igText("%.17g .. %.17g",projection->map->low[feature],projection->map->high[feature]); else igTextUnformatted("Range unavailable",NULL); }
        else if (definition->kind==GD_SIMILARITY_NAME) igText("Exact value: %s",value->name);
        else if (igTreeNode_Str(TextFormat("%d exact context members (known empty differs missing)",value->count))) {
            int member,start,end,*page=&state->similarity_member_pages[feature];
            if (*page<0 || *page*64>=value->count) *page=0;
            if (igSmallButton("Previous members") && *page>0) --*page; igSameLine(0,-1);
            if (igSmallButton("Next members") && (*page+1)*64<value->count) ++*page;
            start=*page*64; end=start+64; if (end>value->count) end=value->count;
            igText("Members %d .. %d / %d",start,end,value->count); igBeginChild_Str("Feature members",(ImVec2_c){0,130},0,0);
            /* Exhaustive pagination; at most 64 widgets per expanded set. */
            for (member=start;member<end;++member) {
                int node=value->members[member]; igPushID_Int(member);
                if (igSmallButton(TextFormat("#%d %s",node,projection->context->layout->nodes[node].label))) gd_explorer_structure_source_node(context,node);
                igPopID();
            }
            igEndChild(); igTreePop();
        }
        igPopID();
    }
}
void gd_explorer_similarity_controls(GD_ExplorerContext *context) {
    GD_ExplorerStructureState *state=context->pane->structure_state; const GD_SimilarityProjection *projection=state->result->similarity; const GD_Similarity *map=projection ? projection->map:NULL; int row,start,end;
    igSeparator(); igText("Applied: %s [%s]",state->result->options.question,state->result->options.projection);
    catalogue(context);
    if (!projection) { igTextWrapped("Discovery only; no distance, fit or source mutation performed. Select features for a map."); return; }
    igCheckbox("Show separate similarity canvas",&state->similarity_canvas); igSameLine(0,-1); if (igSmallButton("Fit similarity")) state->similarity_fit=true;
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
    if (state->similarity_page<0) state->similarity_page=0;
    if (state->similarity_page*32>=projection->entry_count) state->similarity_page=projection->entry_count ? (projection->entry_count-1)/32:0;
    igText("Comparison entries %d | full context %d shown nodes, %d rows",projection->entry_count,state->result->query->shown_nodes,state->result->query->shown_edges);
    if (igSmallButton("Previous similarity entries") && state->similarity_page>0) --state->similarity_page; igSameLine(0,-1);
    if (igSmallButton("Next similarity entries") && (state->similarity_page+1)*32<projection->entry_count) ++state->similarity_page;
    start=state->similarity_page*32; end=start+32; if (end>projection->entry_count) end=projection->entry_count;
    for (row=start;row<end;++row) {
        int placed=map && map->available && map->placed[row]; igPushID_Int(row);
        if (igSmallButton(TextFormat("Inspect #%d",row))) choose_row(state,row,0); igSameLine(0,-1);
        if (igSmallButton("Compare")) choose_row(state,row,1); igSameLine(0,-1); igText("%s | %s",placed ? "placed":"unplaced",projection->ids[row]); igPopID();
    }
    inspect(context); compare(context);
}
