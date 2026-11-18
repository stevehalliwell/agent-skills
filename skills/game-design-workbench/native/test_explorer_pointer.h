/* Source-read-only real canvas handlers with app-local ImGui events.
   No window, OS input or human-pointer/platform acceptance. */
#ifndef TEST_EXPLORER_POINTER_H
#define TEST_EXPLORER_POINTER_H
#include "gd_similarity_view.h"
int gd_resource_evidence_checks(const GD_ResourceGraph *graph,GD_Layout *layout,GD_GraphAnalysis *analysis,int *calls);
static void pointer_test_sample(int x,int y,int down) {
    ImGuiIO *io=igGetIO_Nil();
    ImGuiIO_AddMousePosEvent(io,(float)x,(float)y);
    ImGuiIO_AddMouseButtonEvent(io,ImGuiMouseButton_Left,down!=0);
    igNewFrame();io->WantCaptureMouse=false;
}
static void pointer_test_frame(void) {if(igGetCurrentContext()->WithinFrameScope)igEndFrame();}
int gd_explorer_pointer_checks(GD_Allocator allocator,const GD_Collection *collections,int count,int *calls) {
    GD_ExplorerProject *project=gd_explorer_load(allocator,collections,count);GD_ExplorerContext c={0};
    GD_ExplorerPane *panes=NULL;GD_Layout *before=NULL;GD_RenderPose *pose=NULL;
    ImGuiContext *imgui=NULL;int v,pass,checks=0,result=2,gestures=0,empty_actor_maps=0;
#define POINTER_CHECK(value) do{++checks;if(!(value)){fprintf(stderr,"Pointer check failed: %s (line %d, view %d, pass %d)\n",#value,__LINE__,v,pass);goto cleanup;}}while(0)
    v=pass=-1;POINTER_CHECK(project && !gd_explorer_error(project));
    panes=allocator.allocate(allocator.context,sizeof(*panes)*GD_EXPLORER_VIEWS);
    before=allocator.allocate(allocator.context,sizeof(*before));pose=allocator.allocate(allocator.context,sizeof(*pose));
    if(panes)memset(panes,0,sizeof(*panes)*GD_EXPLORER_VIEWS);
    c.analysis=allocator.allocate(allocator.context,sizeof(*c.analysis));
    POINTER_CHECK(panes && before && pose && c.analysis);memset(c.analysis,0,sizeof(*c.analysis));
    c.project=project;c.panes=panes;c.viewport=(GD_Rect){0,0,800,600};
    imgui=igCreateContext(NULL);POINTER_CHECK(imgui);
    ImGuiIO *input=igGetIO_Nil();input->IniFilename=NULL;input->LogFilename=NULL;
    input->DisplaySize=(ImVec2_c){800,600};input->DeltaTime=1.0f/60;
    input->BackendFlags|=ImGuiBackendFlags_RendererHasTextures;
    for(v=0;v<GD_EXPLORER_VIEWS;++v)pane_defaults(&panes[v],(GD_ExplorerView)v);
    for(pass=0;pass<3;++pass)for(v=0;v<GD_EXPLORER_VIEWS;++v)if(project->available&(1u<<v)) {
        GD_ExplorerPane *p=&panes[v];GD_QueryScene *scene;GD_EffectQuery *selected_result;
        GD_Camera camera;GD_ExplorerQueryOptions options;GD_Point point,mouse;int node,held,allocations,fit,focus_case=pass==2 && p->view!=GD_EXPLORER_NOUNS;
        if(p->view==GD_EXPLORER_STATES)continue; /* State-chart clicks use a separate non-drag handler. */
        c.pane=p;c.display_layout=NULL;c.render_pose=NULL;
        if(pass==1){p->layout_choice=p->view==GD_EXPLORER_NOUNS ? GD_NOUN_LAYOUT_FREE:p->view==GD_EXPLORER_MDA ? 1:GD_LAYOUT_FORCE;p->prepared=0;if(query_view(p->view))POINTER_CHECK(gd_explorer_query_display(&c));}
        if(pass==2 && (query_view(p->view) || grid_view(p)))continue;
        if(focus_case){
            POINTER_CHECK(distance_focus_toggle(&c,1));distance_focus_update(&c,analyze(&c));
            distance_focus_frame(&c,5);POINTER_CHECK(p->distance_focus->animating && p->distance_focus->transition_time==0);
            distance_focus_frame(&c,0.11f);POINTER_CHECK(p->distance_focus->transition_time==0.11f);
        } else {POINTER_CHECK(pane_prepare(&c));if(pass==2)POINTER_CHECK(pane_islands(&c,1));}
        if(p->view==GD_EXPLORER_ACTORS && !p->layout->node_count){++empty_actor_maps;continue;} /* No inferred actor classification. */
        if(grid_view(p))continue; /* Fixed headers are not draggable nodes. */
        POINTER_CHECK(p->layout->node_count>0);
        node=p->layout->node_count-1;
        if(query_view(p->view) && p->query_state->query->effects){
            for(;node>=0;--node)if(p->query_state->query->effects->nodes[p->query_state->scene->nodes[node].source].kind!=GD_EFFECT_ACTIVATION && gd_query_scene_hit_layout(p->query_state->scene,p->layout,p->layout->nodes[node].x,p->layout->nodes[node].y)==node)break;
            POINTER_CHECK(node>=0);
        }
        p->overview.enabled=false;p->fit=0;p->camera.zoom=pass==0 ? 2:pass==1 ? 0.5f:1;p->camera.offset=(GD_Point){400,300};
        if(pass==1){
            memset(pose,0,sizeof(*pose));c.render_pose=pose;render_update(&c,pose,1.0/60,1);
            /* Create raw/display lag through its actual geometry owner. */
            if(query_view(p->view)){float x=p->layout->nodes[node].x,y=p->layout->nodes[node].y;gd_layout_pickup_node(p->layout,node,x,y);gd_query_scene_drag(p->query_state->scene,node,x+10,y);}
            else gd_layout_move_node(p->layout,node,p->layout->nodes[node].x+10,p->layout->nodes[node].y);
            render_update(&c,pose,1.0/60,0);
        }
        point=position(&c,node);p->camera.target=point;
        mouse=gd_world_to_screen(point,p->camera);POINTER_CHECK(mouse.x==400 && mouse.y==300);
        pointer_test_frame();pointer_test_sample(400,300,0);pointer_test_frame();
        pointer_test_sample(400,300,1);
        POINTER_CHECK(gd_mouse_pressed(ImGuiMouseButton_Left) && gd_mouse_position().x==400 && gd_mouse_position().y==300);
        if(query_view(p->view))snprintf(p->query_options.values,sizeof(p->query_options.values),"Unapplied node click input");
        memcpy(before,p->layout,sizeof(*before));camera=p->camera;options=p->query_options;fit=p->fit;
        scene=p->query_state ? p->query_state->scene:NULL;selected_result=p->query_state ? p->query_state->selected:NULL;allocations=*calls;
        canvas_input(&c);held=p->view==GD_EXPLORER_MDA ? p->mda.held:p->selection.held;
        POINTER_CHECK(held==node && *calls==allocations && !memcmp(&camera,&p->camera,sizeof(camera)) && p->fit==fit);
        POINTER_CHECK(!memcmp(before->nodes,p->layout->nodes,sizeof(before->nodes)) && !memcmp(before->edges,p->layout->edges,sizeof(before->edges)));
        POINTER_CHECK(!memcmp(&options,&p->query_options,sizeof(options)) && (!p->query_state || (scene==p->query_state->scene && selected_result==p->query_state->selected)));
        if(focus_case){distance_focus_update(&c,analyze(&c));POINTER_CHECK(!p->distance_focus->animating && !memcmp(before->nodes,p->layout->nodes,sizeof(before->nodes)) && !memcmp(&camera,&p->camera,sizeof(camera)));}
        if(pass==1){double x=pose->layout.nodes[node].x,y=pose->layout.nodes[node].y;render_update(&c,pose,1.0/60,0);POINTER_CHECK(pose->layout.nodes[node].x==x && pose->layout.nodes[node].y==y);}
        pointer_test_frame();pointer_test_sample(402,301,1);POINTER_CHECK(!gd_mouse_pressed(ImGuiMouseButton_Left));
        canvas_input(&c);POINTER_CHECK(!p->layout->pickup_dragging && !memcmp(before->nodes,p->layout->nodes,sizeof(before->nodes)) && !memcmp(&camera,&p->camera,sizeof(camera)) && *calls==allocations);
        if(pass==1){
            float x=p->layout->nodes[node].x,y=p->layout->nodes[node].y,shown_x=pose->layout.nodes[node].x,shown_y=pose->layout.nodes[node].y;int step;
            for(step=0;step<12;++step){if(query_view(p->view))gd_query_scene_step(p->query_state->scene,held);else gd_layout_step(p->layout,held);render_update(&c,pose,1.0/60,0);}
            POINTER_CHECK(p->layout->nodes[node].x==x && p->layout->nodes[node].y==y);
            POINTER_CHECK(pose->layout.nodes[node].x==shown_x && pose->layout.nodes[node].y==shown_y);
            POINTER_CHECK(*calls==allocations);
            memcpy(before,p->layout,sizeof(*before)); /* Other unheld nodes may keep settling. */
        }
        pointer_test_frame();pointer_test_sample(402,301,0);canvas_input(&c);
        POINTER_CHECK((p->view==GD_EXPLORER_MDA ? p->mda.held:p->selection.held)<0 && !memcmp(before->nodes,p->layout->nodes,sizeof(before->nodes)) && !memcmp(&camera,&p->camera,sizeof(camera)));
        /* New gesture: threshold crossing moves from pickup offset, not node center. */
        pointer_test_frame();pointer_test_sample(400,300,1);canvas_input(&c);
        pointer_test_frame();pointer_test_sample(412,308,1);canvas_input(&c);
        POINTER_CHECK(p->layout->pickup_dragging && !memcmp(&camera,&p->camera,sizeof(camera)) && *calls==allocations);
        if(!query_view(p->view) || !(p->query_state->scene->overview && gd_query_scene_overview_mode(p->query_state->scene)==GD_ACTOR_USAGE && !p->query_state->scene->free_geometry))
            POINTER_CHECK(p->layout->nodes[node].x!=before->nodes[node].x || p->layout->nodes[node].y!=before->nodes[node].y);
        pointer_test_frame();pointer_test_sample(412,308,0);canvas_input(&c);
        POINTER_CHECK((p->view==GD_EXPLORER_MDA ? p->mda.held:p->selection.held)<0);
        ++gestures;c.display_layout=NULL;c.render_pose=NULL;
        if(focus_case){
            POINTER_CHECK(distance_focus_toggle(&c,0));distance_focus_frame(&c,5);POINTER_CHECK(p->distance_focus->transition_time==0);
            distance_focus_frame(&c,0.11f);p->camera.target=position(&c,node);camera=p->camera;memcpy(before,p->layout,sizeof(*before));
            pointer_test_frame();pointer_test_sample(400,300,1);canvas_input(&c);distance_focus_frame(&c,0.11f);
            POINTER_CHECK(p->distance_focus->animating && p->distance_focus->returning && p->distance_focus->transition_time==0.11f);
            POINTER_CHECK(!memcmp(before->nodes,p->layout->nodes,sizeof(before->nodes)) && !memcmp(&camera,&p->camera,sizeof(camera)));
            pointer_test_frame();pointer_test_sample(400,300,0);canvas_input(&c);distance_focus_frame(&c,0.11f);
            POINTER_CHECK(!p->distance_focus->animating && !p->distance_focus->returning);
        }
    }
    /* Content envelope uses displayed centers plus raw measured footprints,
       not solver container, filtered footprint copies or hidden cards. */
    {GD_LayoutBounds bounds,unchanged={1,2,3,4};unsigned char shown[GD_MAX_RECORDS]={0};int allocations=*calls;
     memset(before,0,sizeof(*before));before->node_count=2;before->workspace=(GD_LayoutWorkspace){{-500,-500,500,500},1};
     before->nodes[0]=(GD_LayoutNode){.x=10,.y=20,.footprint={-20,-5,50,7}};
     before->nodes[1]=(GD_LayoutNode){.x=-30,.y=50,.footprint={-10,-10,10,10}};
     memcpy(&pose->layout,before,sizeof(*before));pose->layout.nodes[0].x=1000;pose->layout.nodes[0].footprint=(GD_LayoutBounds){-1,-1,1,1};
     POINTER_CHECK(gd_graph_content_bounds(before,&pose->layout,NULL,24,&bounds));
     POINTER_CHECK(bounds.left==-64 && bounds.right==1074 && bounds.top==-9 && bounds.bottom==84);
     shown[0]=1;POINTER_CHECK(gd_graph_content_bounds(before,&pose->layout,shown,24,&bounds));
     POINTER_CHECK(bounds.left==956 && bounds.right==1074 && bounds.top==-9 && bounds.bottom==51);
     bounds=unchanged;shown[0]=0;POINTER_CHECK(!gd_graph_content_bounds(before,&pose->layout,shown,24,&bounds) && !memcmp(&bounds,&unchanged,sizeof(bounds)));
     pose->layout.nodes[0].x=NAN;POINTER_CHECK(!gd_graph_content_bounds(before,&pose->layout,NULL,24,&bounds) && !memcmp(&bounds,&unchanged,sizeof(bounds)));
     POINTER_CHECK(before->workspace.bounds.left==-500 && before->workspace.bounds.right==500 && before->nodes[0].x==10 && *calls==allocations);
    }
    /* Actual retained map: ambiguous presses open chooser, never pick a hidden
       last-painted row. Exercise ImGui popup lifecycle with app-local events. */
    c.pane=&panes[GD_EXPLORER_MDA];c.display_layout=NULL;c.render_pose=NULL;
    POINTER_CHECK(gd_explorer_action_run(&c,GD_ACTION_STRUCTURE_RUN));c.pane->structure_state->pending.kind=GD_STRUCTURE_COUNT+1;
    snprintf(c.pane->structure_state->pending.features,sizeof(c.pane->structure_state->pending.features),"retained-partners");
    POINTER_CHECK(gd_explorer_action_run(&c,GD_ACTION_STRUCTURE_RUN));
    {GD_ExplorerStructureState *state=c.pane->structure_state;GD_Similarity *map=state->result->similarity->map;GD_SimilarityView *view=state->similarity_view;
     POINTER_CHECK(view && map->available && map->complete_entries>1);
     GD_Camera ordinary=c.pane->camera;state->similarity_camera=(GD_Camera){{400,300},{0,0},0.00001f};state->similarity_camera_ready=true;state->similarity_fit=false;
     GD_Camera camera=state->similarity_camera;int row=state->similarity_row,peer=state->similarity_peer,allocations=*calls;
     uint64_t hash=viewer_control_hash(UINT64_C(1469598103934665603),map,sizeof(*map));
     ImGuiIO *io=igGetIO_Nil();io->DisplaySize=(ImVec2_c){800,600};io->DeltaTime=1.0f/60.0f;io->BackendFlags|=ImGuiBackendFlags_RendererHasTextures;
     pointer_test_frame();pointer_test_sample(400,300,1);canvas_input(&c);
     POINTER_CHECK(view->pick_open && view->pick_count==map->complete_entries && state->similarity_row==row && state->similarity_peer==peer);
     gd_explorer_similarity_draw(&c);gd_explorer_similarity_popup(&c);POINTER_CHECK(igIsPopupOpen_Str("Similarity entries under pointer",0));igEndFrame();
     ImGuiIO_AddKeyEvent(io,ImGuiKey_Escape,true);pointer_test_sample(400,300,0);gd_explorer_similarity_popup(&c);
     POINTER_CHECK(!view->pick_open && !igIsPopupOpen_Str("Similarity entries under pointer",0));igEndFrame();
     ImGuiIO_AddKeyEvent(io,ImGuiKey_Escape,false);ImGuiIO_AddKeyEvent(io,ImGuiKey_LeftShift,true);
     pointer_test_sample(400,300,1);POINTER_CHECK(igIsKeyDown_Nil(ImGuiKey_LeftShift));
     canvas_input(&c);POINTER_CHECK(view->pick_open && view->pick_peer && state->similarity_peer==peer);
     gd_explorer_similarity_popup(&c);igEndFrame();ImGuiIO_AddKeyEvent(io,ImGuiKey_LeftShift,false);pointer_test_sample(400,300,0);
     /* Owner tag mismatch closes before borrowed row lookup. */
     ImGuiStorage_SetVoidPtr(igGetStateStorage(),igGetID_Str("Similarity popup owner"),NULL);gd_explorer_similarity_popup(&c);
     POINTER_CHECK(!view->pick_open && !igIsPopupOpen_Str("Similarity entries under pointer",0));igEndFrame();
     /* Leaving canvas also closes popup without reusing old inspection. */
     igNewFrame();ImGuiStorage_SetVoidPtr(igGetStateStorage(),igGetID_Str("Similarity popup owner"),state);view->pick_open=1;igOpenPopup_Str("Similarity entries under pointer",0);
     state->similarity_canvas=false;gd_explorer_similarity_popup(&c);POINTER_CHECK(!igIsPopupOpen_Str("Similarity entries under pointer",0));igEndFrame();
     POINTER_CHECK(!memcmp(&ordinary,&c.pane->camera,sizeof(ordinary)) && !memcmp(&camera,&state->similarity_camera,sizeof(camera)) && state->similarity_row==row && state->similarity_peer==peer);
     POINTER_CHECK(hash==viewer_control_hash(UINT64_C(1469598103934665603),map,sizeof(*map)) && allocations==*calls);
    }
    if(project->resources)POINTER_CHECK(gd_resource_evidence_checks(project->resources,before,c.analysis,calls)>0);
    result=0;
cleanup:
    c.display_layout=NULL;c.render_pose=NULL;
    if(imgui){pointer_test_frame();igDestroyContext(imgui);}
    if(panes){for(v=0;v<GD_EXPLORER_VIEWS;++v){gd_diagnostics_clear(&panes[v].diagnostics);gd_layout_focus_destroy(panes[v].distance_focus);gd_resource_routes_destroy(panes[v].resource_routes);pane_release_layout(allocator,&panes[v]);}allocator.release(allocator.context,panes);}
    if(before)allocator.release(allocator.context,before);if(pose)allocator.release(allocator.context,pose);if(c.analysis)allocator.release(allocator.context,c.analysis);
    gd_explorer_destroy(project);
    if(!result)printf("{\"ok\":true,\"pointer_checks\":%d,\"gestures\":%d,\"empty_actor_maps\":%d,\"scope\":\"app-local ImGui events through real canvas handlers; no window, OS input or source writes; human-pointer acceptance separate\"}\n",checks,gestures,empty_actor_maps);
#undef POINTER_CHECK
    return result;
}
#endif
