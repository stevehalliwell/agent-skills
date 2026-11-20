/* CPU-only presentation checks. No window, OS input, reader Run or source writes. */
#ifndef TEST_EXPLORER_SIMILARITY_H
#define TEST_EXPLORER_SIMILARITY_H
#include "gd_similarity_view.h"
static int similarity_view_checks(GD_Allocator allocator,int *fail_at,int *calls) {
    GD_ExplorerStructureState *state=NULL;GD_StructureResult *result=NULL;GD_SimilarityProjection *projection=NULL;GD_Similarity *map=NULL;GD_Layout *layout=NULL;
    GD_ExplorerPane *pane=NULL;ImGuiContext *imgui=NULL;
    int checks=0,ok=0,allocations;GD_Camera camera={{400,300},{0,0},1};uint64_t hash;
#define SIMILARITY_CHECK(value) do{++checks;if(!(value)){fprintf(stderr,"Similarity view check failed: %s (line %d)\n",#value,__LINE__);goto cleanup;}}while(0)
    state=allocator.allocate(allocator.context,sizeof(*state));result=allocator.allocate(allocator.context,sizeof(*result));projection=allocator.allocate(allocator.context,sizeof(*projection));map=allocator.allocate(allocator.context,sizeof(*map));layout=allocator.allocate(allocator.context,sizeof(*layout));
    if(state)memset(state,0,sizeof(*state));
    SIMILARITY_CHECK(state && result && projection && map && layout);
    memset(result,0,sizeof(*result));memset(projection,0,sizeof(*projection));memset(map,0,sizeof(*map));memset(layout,0,sizeof(*layout));
    state->allocator=allocator;state->result=result;result->similarity=projection;result->layout=layout;projection->map=map;projection->context=result;
    projection->entry_count=map->input.entry_count=70;map->available=1;map->distance_scale=1;
    for(int row=0;row<70;++row){map->placed[row]=1;map->x[row]=row<65 ? 0:row-64;}
    layout->node_count=2;layout->nodes[0].label=layout->nodes[1].label="Shared display name";
    projection->nodes[0]=0;projection->nodes[1]=1;projection->nodes[2]=-1;
    projection->ids[0]="mechanics/first";projection->ids[1]="dynamics/first";projection->ids[2]="interaction/local-case";
    SIMILARITY_CHECK(!strcmp(gd_similarity_view_name(projection,0),gd_similarity_view_name(projection,1)) && strcmp(projection->ids[0],projection->ids[1]));
    SIMILARITY_CHECK(!strcmp(gd_similarity_view_name(projection,2),projection->ids[2]));
    projection->feature_count=projection->catalogue_count=map->input.feature_count=2;projection->selected_features[1]=1;
    projection->catalogue[0].property=GD_SIMILARITY_PARTNERS;projection->catalogue[1].property=GD_SIMILARITY_ROLE;
    SIMILARITY_CHECK(strstr(gd_similarity_view_measurement(projection,0),"partner") && strstr(gd_similarity_view_measurement(projection,1),"role"));
    projection->selected_features[1]=2;SIMILARITY_CHECK(strstr(gd_similarity_view_measurement(projection,1),"Unavailable"));projection->selected_features[1]=1;
    hash=viewer_control_hash(UINT64_C(1469598103934665603),map,sizeof(*map));
    *fail_at=*calls+1;SIMILARITY_CHECK(!gd_explorer_similarity_prepare_view(state) && !state->similarity_view);*fail_at=0;
    SIMILARITY_CHECK(gd_explorer_similarity_prepare_view(state));GD_SimilarityView *view=state->similarity_view;allocations=*calls;
    SIMILARITY_CHECK(view->group_count==6 && view->groups[0].count==65 && view->groups[0].row==0);
    SIMILARITY_CHECK(gd_similarity_view_hits(map,camera,(GD_Point){400,300},view->pick_rows,GD_MAX_RECORDS)==65);
    for(int row=0;row<65;++row)SIMILARITY_CHECK(view->pick_rows[row]==row);
    GD_EvidencePage page=gd_evidence_page(65,GD_EVIDENCE_PAGE_ROWS,1);SIMILARITY_CHECK(page.first==64 && page.end==65 && view->pick_rows[page.first]==64);
    state->similarity_camera=camera;gd_similarity_view_choose(state,view->pick_rows[page.first],0);gd_similarity_view_choose(state,1,1);
    SIMILARITY_CHECK(state->similarity_row==64 && state->similarity_peer==1 && !memcmp(&state->similarity_camera,&camera,sizeof(camera)));
    SIMILARITY_CHECK(gd_similarity_view_hits(map,camera,(GD_Point){416,300},NULL,0)==65);
    SIMILARITY_CHECK(!gd_similarity_view_hits(map,camera,(GD_Point){417,300},NULL,0));
    camera.zoom=0.5f;SIMILARITY_CHECK(gd_similarity_view_hits(map,camera,(GD_Point){400,300},view->hover_rows,GD_SIMILARITY_HOVER_ROWS)==65);
    camera.zoom=2;SIMILARITY_CHECK(gd_similarity_view_hits(map,camera,(GD_Point){400,300},NULL,0)==65);
    camera.zoom=NAN;SIMILARITY_CHECK(!gd_similarity_view_hits(map,camera,(GD_Point){400,300},NULL,0));camera.zoom=1;
    SIMILARITY_CHECK(hash==viewer_control_hash(UINT64_C(1469598103934665603),map,sizeof(*map)) && allocations==*calls);
    map->placed[0]=0;gd_similarity_view_choose(state,0,0);SIMILARITY_CHECK(state->similarity_row==0 && state->similarity_peer==1);
    SIMILARITY_CHECK(gd_similarity_view_build(view,map) && view->groups[0].count==64 && view->groups[0].row==1);
    SIMILARITY_CHECK(gd_similarity_view_hits(map,camera,(GD_Point){400,300},view->pick_rows,GD_MAX_RECORDS)==64 && view->pick_rows[0]==1);
    map->x[1]=NAN;int groups=view->group_count;SIMILARITY_CHECK(!gd_similarity_view_build(view,map) && view->group_count==groups);map->x[1]=0;
    map->input.entry_count=GD_MAX_RECORDS;for(int row=0;row<GD_MAX_RECORDS;++row){map->placed[row]=1;map->x[row]=map->y[row]=0;}
    SIMILARITY_CHECK(gd_similarity_view_build(view,map) && view->group_count==1 && view->groups[0].count==GD_MAX_RECORDS);
    SIMILARITY_CHECK(gd_similarity_view_hits(map,camera,(GD_Point){400,300},view->pick_rows,GD_MAX_RECORDS)==GD_MAX_RECORDS && view->pick_rows[GD_MAX_RECORDS-1]==GD_MAX_RECORDS-1);
    map->input.entry_count=0;SIMILARITY_CHECK(gd_similarity_view_build(view,map) && !view->group_count && !gd_similarity_view_hits(map,camera,(GD_Point){400,300},NULL,0));
    SIMILARITY_CHECK(allocations==*calls);
    /* ImGui pools tooltip windows. Reusing legend geometry must not give entry
       tooltip a one-frame size inherited from different content. */
    pane=allocator.allocate(allocator.context,sizeof(*pane));SIMILARITY_CHECK(pane);memset(pane,0,sizeof(*pane));pane->structure_state=state;
    state->similarity_canvas=true;projection->features[0].name="retained-partners";projection->features[1].name="source-loaded-role";projection->features[0].weight=projection->features[1].weight=1;projection->features[0].unit="count";
    view->hover_count=2;view->hover_rows[0]=0;view->hover_rows[1]=1;
    imgui=igCreateContext(NULL);SIMILARITY_CHECK(imgui);ImGuiIO *io=igGetIO_Nil();io->IniFilename=NULL;io->LogFilename=NULL;io->DisplaySize=(ImVec2_c){800,600};io->DeltaTime=1.0f/60;io->BackendFlags|=ImGuiBackendFlags_RendererHasTextures;
    GD_ExplorerContext context={0};context.pane=pane;context.viewport=(GD_Rect){-20,-20,800,600};
    for(int frame=0;frame<3;++frame){igNewFrame();io->WantCaptureMouse=false;gd_explorer_similarity_popup(&context);igEndFrame();}
    context.viewport=(GD_Rect){1000,1000,800,600};ImVec2_c sizes[3];
    for(int frame=0;frame<3;++frame){igNewFrame();io->WantCaptureMouse=false;gd_explorer_similarity_popup(&context);igBeginTooltip();sizes[frame]=igGetWindowSize();float content=igGetCursorPosY();igEndTooltip();igEndFrame();SIMILARITY_CHECK(sizes[frame].x>0 && sizes[frame].y>0 && content>30);}
    SIMILARITY_CHECK(sizes[0].x==sizes[2].x && sizes[0].y==sizes[2].y);
    /* Reverse transition, then shorter entry content. Use public tooltip query
       after submission: same-frame Begin preserves current geometry/content. */
    for(int phase=0;phase<2;++phase){
        context.viewport=phase ? (GD_Rect){1000,1000,800,600}:(GD_Rect){-20,-20,800,600};view->hover_count=phase ? 1:2;
        ImVec2_c positions[3];
        for(int frame=0;frame<3;++frame){igNewFrame();io->WantCaptureMouse=false;gd_explorer_similarity_popup(&context);igBeginTooltip();sizes[frame]=igGetWindowSize();positions[frame]=igGetWindowPos();igEndTooltip();igEndFrame();}
        SIMILARITY_CHECK(sizes[0].x==sizes[2].x && sizes[0].y==sizes[2].y);
        SIMILARITY_CHECK(positions[0].x==positions[2].x && positions[0].y==positions[2].y);
        SIMILARITY_CHECK(sizes[0].x<=io->DisplaySize.x && sizes[0].y<=io->DisplaySize.y);
    }
    ok=1;
cleanup:
    *fail_at=0;if(imgui)igDestroyContext(imgui);if(pane)allocator.release(allocator.context,pane);if(state && state->similarity_view)allocator.release(allocator.context,state->similarity_view);
    if(layout)allocator.release(allocator.context,layout);if(map)allocator.release(allocator.context,map);if(projection)allocator.release(allocator.context,projection);if(result)allocator.release(allocator.context,result);if(state)allocator.release(allocator.context,state);
#undef SIMILARITY_CHECK
    return ok ? checks:0;
}
#endif
