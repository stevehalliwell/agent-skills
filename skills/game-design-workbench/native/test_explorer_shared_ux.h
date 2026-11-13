/* Source-read-only shared-control checks. No GUI window or OS input. */
#ifndef TEST_EXPLORER_SHARED_UX_H
#define TEST_EXPLORER_SHARED_UX_H
int gd_explorer_shared_ux_checks(GD_Allocator allocator,const GD_Collection *collections,int count,int *fail_at,int *calls){
    GD_ExplorerProject *project=gd_explorer_load(allocator,collections,count);GD_ExplorerContext c={0};GD_ExplorerPane *panes=NULL;GD_SelectionHistory *history=NULL;GD_Layout *saved=NULL;
    ImGuiContext *imgui=NULL;
    int result=2,checks=0,active=0,v,i,visited=0,views[GD_EXPLORER_VIEWS],allocations;char id[GD_MAX_PATH],restored[GD_MAX_PATH];ViewerIdentities ids;
#define UX_CHECK(value) do{++checks;if(!(value)){fprintf(stderr,"Shared UX check failed: %s (line %d)\n",#value,__LINE__);goto cleanup;}}while(0)
    UX_CHECK(project && !gd_explorer_error(project));
    panes=allocator.allocate(allocator.context,sizeof(*panes)*GD_EXPLORER_VIEWS);if(panes)memset(panes,0,sizeof(*panes)*GD_EXPLORER_VIEWS);
    c.analysis=allocator.allocate(allocator.context,sizeof(*c.analysis));if(c.analysis)memset(c.analysis,0,sizeof(*c.analysis));
    history=allocator.allocate(allocator.context,sizeof(*history));if(history)memset(history,0,sizeof(*history));saved=allocator.allocate(allocator.context,sizeof(*saved));
    UX_CHECK(panes && c.analysis && history && saved);c.project=project;c.panes=panes;c.history=history;c.active_view=&active;c.viewport=(Rectangle){0,0,800,600};
    for(v=0;v<GD_EXPLORER_VIEWS;++v)pane_defaults(&panes[v],(GD_ExplorerView)v);
    UX_CHECK(gd_text_contains("Walker Ω","waLK") && gd_text_contains("Walker Ω","Ω") && !gd_text_contains("Walker Ω","ω") && !gd_text_contains("Walker","wlkr") && gd_text_contains("Walker",""));
    {GD_CountRange range={1,4};GD_CountLinkStyle previous={0},style;int values[3],value;
        UX_CHECK(gd_count_link_samples((GD_CountRange){0},values)==0);
        UX_CHECK(gd_count_link_samples((GD_CountRange){4,4},values)==1 && values[0]==4);
        UX_CHECK(gd_count_link_samples((GD_CountRange){1,2},values)==2 && values[0]==1 && values[1]==2);
        UX_CHECK(gd_count_link_samples(range,values)==3 && values[0]==1 && values[1]==2 && values[2]==4);
        style=gd_count_link_style(range,0);UX_CHECK(!style.width && !style.color.a);
        style=gd_count_link_style((GD_CountRange){0},1);UX_CHECK(!style.width && !style.color.a);
        style=gd_count_link_style((GD_CountRange){4,4},4);UX_CHECK(style.width==5 && style.color.r==72 && style.color.g==162 && style.color.b==192 && style.color.a==255);
        for(value=1;value<=4;++value){style=gd_count_link_style(range,value);UX_CHECK(fabsf(style.width-2*value)<0.001f && style.color.a==255 && style.color.g>previous.color.g);previous=style;}
        style=gd_count_link_style(range,1);UX_CHECK(style.color.r==64 && style.color.g==100 && style.color.b==176);
        style=gd_count_link_style(range,4);UX_CHECK(style.color.r==80 && style.color.g==224 && style.color.b==208);
        style=gd_count_link_style((GD_CountRange){1,2147483647},2147483647);UX_CHECK(style.width==8);
        UX_CHECK(gd_count_link_samples((GD_CountRange){1,2147483647},values)==3 && values[0]==1 && values[1]==1073741824 && values[2]==2147483647);
        /* Cell fills remain dark for white labels; link brightness is separate. */
        {Color fill=gd_count_color(range,4);UX_CHECK(fill.r==24 && fill.g==112 && fill.b==96);}
    }
    {GD_OverviewControls controls={1,-1,-1};Rectangle viewport={0,0,800,600},expanded=gd_graph_overview_visible_bounds(&controls,viewport),collapsed;Vector2 body={expanded.x+10,expanded.y+60};
        UX_CHECK(expanded.height>GD_OVERVIEW_HEADER_HEIGHT && gd_graph_overview_hit(&controls,viewport,body));
        UX_CHECK(!gd_graph_overview_toggle(&controls,viewport,body) && controls.enabled);
        UX_CHECK(gd_graph_overview_toggle(&controls,viewport,(Vector2){expanded.x+10,expanded.y+10}) && !controls.enabled);
        collapsed=gd_graph_overview_visible_bounds(&controls,viewport);
        UX_CHECK(collapsed.height==GD_OVERVIEW_HEADER_HEIGHT && collapsed.x+collapsed.width==expanded.x+expanded.width && collapsed.y==expanded.y && !gd_graph_overview_hit(&controls,viewport,body));
        UX_CHECK(gd_graph_overview_hit(&controls,viewport,(Vector2){collapsed.x+10,collapsed.y+10}));
        UX_CHECK(gd_graph_overview_toggle(&controls,viewport,(Vector2){collapsed.x+10,collapsed.y+10}) && controls.enabled);
        UX_CHECK(gd_graph_overview_visible_bounds(&controls,(Rectangle){0,0,800,120}).height==GD_OVERVIEW_HEADER_HEIGHT && controls.enabled);
        UX_CHECK(!gd_graph_overview_hit(&controls,(Rectangle){0,0,20,20},(Vector2){10,10}) && !gd_graph_overview_toggle(&controls,(Rectangle){0,0,20,20},(Vector2){10,10}) && controls.enabled);
    }
    for(v=0;v<GD_EXPLORER_VIEWS;++v)if(project->available&(1u<<v)){
        GD_ExplorerPane *p=&panes[v];GD_Query *query;GD_StructureResult *structure;GD_ExplorerQueryOptions pending;
        c.pane=p;active=v;UX_CHECK(pane_prepare(&c));
        UX_CHECK(p->workspace_bounds && !gd_explorer_presentation_editable(&c,"initial_arrangement"));
        if(free_graph(p) && !grid_view(p)){
            UX_CHECK(p->layout->workspace.enabled==(p->layout->node_count>0));
            if(!query_view(v)){
                int node;memcpy(saved,p->layout,sizeof(*saved));UX_CHECK(gd_layout_prepare_free(saved,allocator,pane_aspect(&c),1));
                if(v==GD_EXPLORER_RESOURCES){int step;for(step=0;step<120;++step)gd_layout_step(saved,-1);}
                for(node=0;node<p->layout->node_count;++node)UX_CHECK(saved->nodes[node].x==p->layout->nodes[node].x && saved->nodes[node].y==p->layout->nodes[node].y);
            }
        }else UX_CHECK(!p->layout->workspace.enabled);
        UX_CHECK(!p->island_separation && !p->layout->islands.enabled && p->overview.enabled);
        if(!free_graph(p) || grid_view(p)){
            UX_CHECK(!gd_explorer_presentation_editable(&c,"island_separation"));
            if(gd_explorer_presentation_editable(&c,"layout"))UX_CHECK(gd_explorer_presentation_set(&c,"layout","free",p->query_message,sizeof(p->query_message)));
        }
        if(free_graph(p) && !grid_view(p)){
            Camera2D camera=p->camera;GD_ExplorerSelection selection=p->selection;GD_ExplorerQueryOptions options=p->query_options;GD_ExplorerQueryState *state=p->query_state;int node;
            memcpy(saved,p->layout,sizeof(*saved));UX_CHECK(gd_explorer_presentation_editable(&c,"island_separation"));
            UX_CHECK(gd_explorer_presentation_set(&c,"island_separation","true",p->query_message,sizeof(p->query_message)) && p->island_separation && !p->paused && p->layout->islands.enabled);
            UX_CHECK(!memcmp(&camera,&p->camera,sizeof(camera)) && !memcmp(&selection,&p->selection,sizeof(selection)) && !memcmp(&options,&p->query_options,sizeof(options)) && state==p->query_state);
            for(node=0;node<p->layout->node_count;++node)UX_CHECK(!memcmp(&saved->nodes[node],&p->layout->nodes[node],sizeof(GD_LayoutNode)));
            UX_CHECK(gd_explorer_presentation_set(&c,"island_separation","false",p->query_message,sizeof(p->query_message)) && !p->island_separation && !p->layout->islands.enabled);
            UX_CHECK(gd_explorer_presentation_set(&c,"island_separation","true",p->query_message,sizeof(p->query_message)));
            UX_CHECK(reset_view(&c) && !p->island_separation && !p->layout->islands.enabled);
        }
        if(gd_explorer_presentation_editable(&c,"overview_enabled")){
            Camera2D camera=p->camera;GD_ExplorerSelection selection=p->selection;GD_ExplorerQueryOptions options=p->query_options;int fit=p->fit;
            memcpy(saved,p->layout,sizeof(*saved));
            UX_CHECK(gd_explorer_presentation_set(&c,"overview_enabled","false",p->query_message,sizeof(p->query_message)) && !p->overview.enabled);
            UX_CHECK(pane_prepare(&c) && !p->overview.enabled && panes[(v+1)%GD_EXPLORER_VIEWS].overview.enabled);
            UX_CHECK(!memcmp(saved,p->layout,sizeof(*saved)) && !memcmp(&camera,&p->camera,sizeof(camera)) && !memcmp(&selection,&p->selection,sizeof(selection)) && !memcmp(&options,&p->query_options,sizeof(options)) && p->fit==fit);
            if(viewer_canvas_count(&c))UX_CHECK(find_select(&c,panes,0) && !p->overview.enabled);
            if(v==GD_EXPLORER_MATRIX){
                UX_CHECK(gd_explorer_presentation_set(&c,"layout","grid",p->query_message,sizeof(p->query_message)) && !p->overview.enabled && !gd_explorer_presentation_editable(&c,"overview_enabled"));
                UX_CHECK(gd_explorer_presentation_set(&c,"layout","free",p->query_message,sizeof(p->query_message)) && !p->overview.enabled && gd_explorer_presentation_editable(&c,"overview_enabled"));
            }
            UX_CHECK(reset_view(&c) && p->overview.enabled);
        }
        if(!viewer_canvas_count(&c))continue;UX_CHECK(find_select(&c,panes,0));
        viewer_identities(&c,&ids);UX_CHECK(viewer_canvas_id(&c,viewer_selected_node(&c),&ids,id,sizeof(id)));
        if(p->view==GD_EXPLORER_SKILLS || p->view==GD_EXPLORER_ACTORS)UX_CHECK(gd_explorer_action_run(&c,GD_ACTION_STRUCTURE_RUN));
        if(query_view(p->view))snprintf(p->query_options.values,sizeof(p->query_options.values),"Unapplied reset input Ω");
        if(p->structure_state)p->structure_state->pending.work_limit=12345;
        query=p->query_state ? p->query_state->query:NULL;structure=p->structure_state ? p->structure_state->result:NULL;
        p->camera.target=(Vector2){12345,67890};snprintf(p->filter,sizeof(p->filter),"old canvas filter");snprintf(p->list_search,sizeof(p->list_search),"different list text");
        pending=p->query_options;memcpy(saved,p->layout,sizeof(*saved));
        UX_CHECK(gd_explorer_presentation_set(&c,"list_search","INDEPENDENT",p->query_message,sizeof(p->query_message)) && !strcmp(p->filter,"old canvas filter"));
        UX_CHECK(gd_explorer_action_run(&c,GD_ACTION_VIEW_RESET));
        UX_CHECK(!*p->filter && !strcmp(p->list_search,"INDEPENDENT") && !memcmp(&pending,&p->query_options,sizeof(pending)));
        UX_CHECK((!p->query_state || p->query_state->query==query) && (!p->structure_state || p->structure_state->result==structure));
        viewer_identities(&c,&ids);UX_CHECK(viewer_canvas_id(&c,viewer_selected_node(&c),&ids,restored,sizeof(restored)) && !strcmp(id,restored));
        UX_CHECK(!p->structure_state || p->structure_state->pending.work_limit==12345);
        UX_CHECK(gd_explorer_action_run(&c,GD_ACTION_INPUTS_RESET));
        UX_CHECK(p->query_options.ghosts==pending.ghosts && p->query_options.primary==pending.primary && p->query_options.compare==pending.compare && p->query_options.weighted==pending.weighted && p->query_options.include_excluded==pending.include_excluded);
        UX_CHECK((!p->query_state || p->query_state->query==query) && (!p->structure_state || p->structure_state->result==structure));
        *p->list_search=0;history_observe(&c);views[visited++]=v;UX_CHECK(history->count==visited);
    }
    UX_CHECK(visited>1);allocations=*calls;
    for(i=visited-2;i>=0;--i){UX_CHECK(gd_explorer_history_move(&c,-1) && active==views[i]);viewer_identities(&c,&ids);UX_CHECK(viewer_canvas_id(&c,viewer_selected_node(&c),&ids,id,sizeof(id)) && !strcmp(id,history->entries[history->cursor].id));}
    UX_CHECK(!gd_explorer_history_available(&c,-1));
    for(i=1;i<visited;++i)UX_CHECK(gd_explorer_history_move(&c,1) && active==views[i]);
    UX_CHECK(!gd_explorer_history_available(&c,1) && *calls==allocations);
    /* Missing target is atomic, never a source fallback or implicit Run. */
    {int cursor=history->cursor;GD_ExplorerSelection selection=c.pane->selection;Camera2D camera=c.pane->camera;char old[GD_MAX_PATH];
     snprintf(old,sizeof(old),"%s",history->entries[cursor-1].id);snprintf(history->entries[cursor-1].id,sizeof(old),"__missing_typed_target__");
     UX_CHECK(!gd_explorer_history_move(&c,-1) && cursor==history->cursor && !memcmp(&selection,&c.pane->selection,sizeof(selection)) && !memcmp(&camera,&c.pane->camera,sizeof(camera)));
     snprintf(history->entries[cursor-1].id,sizeof(old),"%s",old);}
    /* Hidden MDA selections restore identity but never reveal or move camera. */
    if(project->available&(1u<<GD_EXPLORER_MDA)){
        int slot;for(slot=0;slot<visited;++slot)if(views[slot]==GD_EXPLORER_MDA)break;
        if(slot+1<visited){Camera2D camera=panes[GD_EXPLORER_MDA].camera;history->cursor=slot+1;active=views[slot+1];c.pane=&panes[active];history->stamped=0;
            memset(panes[GD_EXPLORER_MDA].mda.roles,0,sizeof(panes[GD_EXPLORER_MDA].mda.roles));
            UX_CHECK(gd_explorer_history_move(&c,-1) && active==GD_EXPLORER_MDA && c.pane->focus_result.status==GD_FOCUS_HIDDEN && !memcmp(&camera,&c.pane->camera,sizeof(camera)));
            for(i=0;i<4;++i)c.pane->mda.roles[i]=true;
        }
    }
    /* Exact matrix pair/case round-trip, followed by branch truncation. */
    if(project->available&(1u<<GD_EXPLORER_MATRIX)){
        GD_Model *model=gd_explorer_model(project,GD_EXPLORER_MATRIX);const GD_Cell *cell=&model->cells[0];
        memset(history,0,sizeof(*history));active=GD_EXPLORER_MATRIX;c.pane=&panes[active];c.pane->selection=(GD_ExplorerSelection){0,-1,cell->pair_count-1,-1,-1};history_observe(&c);
        UX_CHECK(history->count==1 && history->entries[0].kind==GD_HISTORY_PAIR);
        UX_CHECK(find_select(&c,panes,0));history_observe(&c);UX_CHECK(history->count==2 && gd_explorer_history_move(&c,-1));
        UX_CHECK(c.pane->selection.cell==0 && c.pane->selection.interaction==cell->pair_count-1 && c.pane->grid.noun==cell->noun && c.pane->grid.verb==cell->verb);
        UX_CHECK(find_select(&c,panes,1));history_observe(&c);UX_CHECK(history->count==2 && !gd_explorer_history_available(&c,1));
        c.pane->selection=(GD_ExplorerSelection){-1,-1,0,-1,-1};c.pane->grid.noun=c.pane->grid.verb=-1;history_observe(&c);
        UX_CHECK(history->entries[history->cursor].kind==GD_HISTORY_EMPTY && gd_explorer_history_move(&c,-1) && viewer_selected_node(&c)==1 && gd_explorer_history_move(&c,1) && viewer_selected_node(&c)<0);
        for(i=0;i<GD_SELECTION_HISTORY+5;++i){UX_CHECK(find_select(&c,panes,i%2));history_observe(&c);}UX_CHECK(history->count==GD_SELECTION_HISTORY && history->cursor==GD_SELECTION_HISTORY-1);
        memcpy(saved,c.pane->layout,sizeof(*saved));{Camera2D camera=c.pane->camera;c.pane->overview.enabled=0;*fail_at=*calls+1;UX_CHECK(!reset_view(&c));*fail_at=0;UX_CHECK(!c.pane->overview.enabled && !memcmp(saved,c.pane->layout,sizeof(*saved)) && !memcmp(&camera,&c.pane->camera,sizeof(camera)));}
    }
    if(project->available&(1u<<GD_EXPLORER_RESOURCES)){
        GD_ExplorerPane *p=&panes[GD_EXPLORER_RESOURCES];const GD_GraphOverview *overview;Rectangle bounds={0};int found=0,cell,node,marked=0,extra=-1,previous_active=active;Camera2D camera,standalone;GD_ExplorerQueryOptions options;
        active=GD_EXPLORER_RESOURCES;c.pane=p;UX_CHECK(gd_explorer_action_run(&c,GD_ACTION_SELECTION_CLEAR));analyze(&c);
        p->overview.enabled=1;overview=gd_graph_overview(c.analysis,&p->overview,1);
        for(cell=0;cell<overview->bins*overview->bins && !overview->counts[cell];++cell){}
        UX_CHECK(cell<overview->bins*overview->bins);p->overview.row=cell/overview->bins;p->overview.column=cell%overview->bins;
        overview=gd_graph_overview(c.analysis,&p->overview,1);p->overview.enabled=0;
        UX_CHECK(evidence(&c,-1).overview==overview && overview->selected_count>0);
        for(node=0;node<p->layout->node_count;++node)if(visible(&c,node)){if(overview->selected_nodes[node]){grid_union(&bounds,focus_node_bounds(&c,p->layout,node),&found);++marked;}else if(extra<0)extra=node;}
        if(extra>=0){p->selection.node=extra;grid_union(&bounds,focus_node_bounds(&c,p->layout,extra),&found);++marked;}
        memcpy(saved,p->layout,sizeof(*saved));options=p->query_options;standalone=p->camera;
        UX_CHECK(gd_explorer_action_run(&c,GD_ACTION_VIEW_FIT) && p->focus_result.selected==marked && !p->overview.enabled);
        UX_CHECK(fabsf(p->camera.target.x-bounds.x-bounds.width/2)<0.01f && fabsf(p->camera.target.y-bounds.y-bounds.height/2)<0.01f);
        UX_CHECK(fabsf(p->camera.zoom-fmaxf(0.002f,fminf(1.2f,focus_fit(c.viewport,bounds))))<0.00001f && !memcmp(saved,p->layout,sizeof(*saved)));
        UX_CHECK(gd_graph_frame_selection(&standalone,p->layout,c.analysis,&p->overview,extra,-1,c.viewport,1) && fabsf(standalone.zoom-p->camera.zoom)<0.00001f && fabsf(standalone.target.x-p->camera.target.x)<0.01f && fabsf(standalone.target.y-p->camera.target.y)<0.01f);
        camera=p->camera;UX_CHECK(gd_explorer_action_run(&c,GD_ACTION_SELECTION_CLEAR));
        UX_CHECK(p->selection.node<0 && p->overview.row<0 && p->overview.column<0 && !p->overview.enabled && !memcmp(&camera,&p->camera,sizeof(camera)) && !memcmp(&options,&p->query_options,sizeof(options)) && !memcmp(saved,p->layout,sizeof(*saved)));
        analyze(&c);UX_CHECK(!evidence(&c,-1).overview && !gd_explorer_focus_available(&c));
        if(!p->distance_focus)p->distance_focus=gd_layout_focus_create(project->allocator);UX_CHECK(p->distance_focus!=NULL);
        p->distance_focus_enabled=true;distance_focus_update(&c,-1);p->distance_focus_enabled=false;
        UX_CHECK(!memcmp(&camera,&p->camera,sizeof(camera)) && !memcmp(saved,p->layout,sizeof(*saved)));
        bounds=(Rectangle){0};found=0;for(node=0;node<p->layout->node_count;++node)if(visible(&c,node))grid_union(&bounds,focus_node_bounds(&c,p->layout,node),&found);
        p->layout->workspace.enabled=1;p->layout->workspace.bounds=(GD_LayoutBounds){-100000000,-100000000,100000000,100000000};
        UX_CHECK(gd_explorer_action_run(&c,GD_ACTION_VIEW_FIT) && p->fit==1);fit_view(&c,0);p->fit=0;
        UX_CHECK(fabsf(p->camera.zoom-fmaxf(0.002f,fminf(1.2f,focus_fit(c.viewport,bounds))))<0.00001f && fabsf(p->camera.target.x-bounds.x-bounds.width/2)<0.01f);
        memcpy(p->layout,saved,sizeof(*saved));active=previous_active;c.pane=&panes[active];c.analysis->initialized=0;
    }
    if(project->draft){
        GD_Model *model=project->draft->model;int skill;GD_ExplorerQueryState *query=panes[GD_EXPLORER_ACTORS].query_state;
        for(skill=0;skill<model->record_count && model->records[skill].role!=GD_SKILL;++skill){}UX_CHECK(skill<model->record_count);
        UX_CHECK(gd_draft_rank(project->draft,skill,"987654321")>0 && gd_explorer_edited(project));
        UX_CHECK(gd_explorer_action_run(&c,GD_ACTION_UNDO) && project->draft->redo_count==1);
        UX_CHECK(gd_explorer_action_run(&c,GD_ACTION_VIEW_RESET) && project->draft->redo_count==1 && panes[GD_EXPLORER_ACTORS].query_state==query);
        UX_CHECK(gd_explorer_action_run(&c,GD_ACTION_REDO) && model->records[skill].rank==987654321 && !project->draft->redo_count);
        UX_CHECK(gd_explorer_action_run(&c,GD_ACTION_UNDO));
        {char message[512];UX_CHECK(refresh_sources(&c,panes,collections,count,message,sizeof(message))<0 && project->draft->redo_count==1);}
        UX_CHECK(gd_explorer_action_run(&c,GD_ACTION_DISCARD_REDO) && !project->draft->redo_count);
    }
    UX_CHECK(gd_explorer_action_reason(&c,GD_ACTION_VIEW_SELECTION_FOCUS)==NULL);
    c.pane->selection=(GD_ExplorerSelection){-1,-1,0,-1,-1};c.pane->grid.noun=c.pane->grid.verb=-1;
    UX_CHECK(!gd_explorer_action_available(&c,GD_ACTION_VIEW_SELECTION_FOCUS) && gd_explorer_action_reason(&c,GD_ACTION_VIEW_SELECTION_FOCUS)!=NULL);
    /* Reset all clears every pane but preserves draft bytes, including both
       history directions. Allocation failures retain the entire last-good set. */
    {
        GD_ExplorerFind finds[GD_EXPLORER_VIEWS]={0};GD_Sidebar sidebar={0},defaults;uint64_t draft_hash=0;GD_Draft *draft=project->draft;int original_root=-1;
        if(draft){GD_Model *model=draft->model;int skill;for(skill=0;skill<model->record_count && model->records[skill].role!=GD_SKILL;++skill){}
            UX_CHECK(skill<model->record_count && gd_draft_rank(draft,skill,"987654323")>0 && gd_draft_rank(draft,skill,"987654324")>0 && gd_draft_rank(draft,skill,"987654325")>0 && gd_draft_undo(draft)>0 && gd_explorer_edited(project));
            UX_CHECK(draft->modified_count>0 && draft->history_count>0 && draft->redo_count>0);draft_hash=viewer_control_hash(UINT64_C(14695981039346656037),draft,sizeof(*draft));
        }
        active=GD_EXPLORER_MDA;c.pane=&panes[active];c.finds=finds;c.sidebar=&sidebar;sidebar.open=255;finds[active].open=true;snprintf(finds[active].text,sizeof(finds[active].text),"old Find");
        if(project->mda){original_root=c.pane->layout->nodes[0].key;UX_CHECK(gd_mda_query(gd_explorer_model(project,GD_EXPLORER_MDA),project->mda,original_root,GD_MDA_FORWARD));}
        c.pane->mda.record=original_root;c.pane->mda.link=0;snprintf(c.pane->filter,sizeof(c.pane->filter),"old canvas");snprintf(c.pane->list_search,sizeof(c.pane->list_search),"old list");
        UX_CHECK(gd_explorer_action_available(&c,GD_ACTION_RESET_ALL));
        for(i=1;i<=2;++i){GD_Layout *layout=c.pane->layout;GD_ExplorerQueryState *query=panes[GD_EXPLORER_ACTORS].query_state;int history_count=history->count;
            *fail_at=*calls+i;UX_CHECK(!gd_explorer_action_run(&c,GD_ACTION_RESET_ALL));*fail_at=0;
            UX_CHECK(c.pane->layout==layout && panes[GD_EXPLORER_ACTORS].query_state==query && history->count==history_count && sidebar.open==255 && finds[active].open && c.pane->mda.record==original_root);
            UX_CHECK(!project->mda || project->mda->root==original_root);
        }
        c.pane->mda.held=0;UX_CHECK(!gd_explorer_action_available(&c,GD_ACTION_RESET_ALL));c.pane->mda.held=-1;
        UX_CHECK(gd_explorer_action_run(&c,GD_ACTION_RESET_ALL));gd_sidebar_defaults(&defaults);
        UX_CHECK(active==GD_EXPLORER_MDA && c.pane==&panes[active] && c.pane->prepared && c.pane->mda.record<0 && c.pane->mda.link<0 && !c.display_layout && !history->count && sidebar.open==defaults.open && !finds[active].open && !*finds[active].text);
        UX_CHECK(!project->mda || (project->mda->root<0 && project->mda->direction==GD_MDA_BOTH));
        for(v=0;v<GD_EXPLORER_VIEWS;++v){GD_ExplorerPane *p=&panes[v];
            UX_CHECK(p->overview.enabled && !*p->filter && !*p->list_search && p->selection.node<0 && p->selection.cell<0 && p->selection.linked<0 && p->mda.record<0 && p->mda.link<0);
            UX_CHECK(!p->structure_state && !p->rule_state && !p->query_state && !p->resource_routes && !p->distance_focus && p->workspace_bounds && !p->island_separation);
            UX_CHECK(p->camera.zoom==1 && !p->camera.target.x && !p->camera.target.y && !p->distance_focus_enabled && !p->resource_only_witness);
            if(v!=active)UX_CHECK(!p->prepared && !p->layout);
        }
        UX_CHECK(project->draft==draft && (!draft || draft_hash==viewer_control_hash(UINT64_C(14695981039346656037),draft,sizeof(*draft))));
        if(draft){UX_CHECK(gd_explorer_action_run(&c,GD_ACTION_UNDO));UX_CHECK(gd_explorer_action_run(&c,GD_ACTION_REDO));}
        {
            ExplorerViewer viewer={0};GD_ViewerRequest request={0};char reply[2048];GD_ViewerWriter writer={reply,0,sizeof(reply),0};GD_Layout *layout=c.pane->layout;
            imgui=igCreateContext(NULL);UX_CHECK(imgui!=NULL);igGetIO_Nil()->IniFilename=NULL;
            viewer.context=&c;viewer.panes=panes;viewer.active=&active;viewer.has_hint=viewer.hint_pending=1;snprintf(viewer.layer_token,sizeof(viewer.layer_token),"old layers");
            snprintf(request.command,sizeof(request.command),"action");snprintf(request.projection,sizeof(request.projection),"mda");snprintf(request.action,sizeof(request.action),"view.reset.all");
            viewer_dispatch(&viewer,&request,&writer);UX_CHECK(strstr(reply,"viewer.stale_controls") && c.pane->layout==layout);
            if(draft)draft_hash=viewer_control_hash(UINT64_C(14695981039346656037),draft,sizeof(*draft));
            viewer_control_token(&viewer,&c,GD_FORM_PRESENTATION,request.expect,sizeof(request.expect));writer=(GD_ViewerWriter){reply,0,sizeof(reply),0};viewer_dispatch(&viewer,&request,&writer);
            UX_CHECK(strstr(reply,"\"executed\":true") && strstr(reply,"\"source_draft_changed\":false") && !viewer.has_hint && !viewer.hint_pending && !*viewer.layer_token && c.pane->layout!=layout);
            UX_CHECK(!draft || draft_hash==viewer_control_hash(UINT64_C(14695981039346656037),draft,sizeof(*draft)));
            c.pane->mda.record=c.pane->layout->node_count ? c.pane->layout->nodes[0].key:-1;c.pane->overview.row=c.pane->overview.column=0;
            snprintf(request.action,sizeof(request.action),"view.selection.clear");viewer_control_token(&viewer,&c,GD_FORM_PRESENTATION,request.expect,sizeof(request.expect));writer=(GD_ViewerWriter){reply,0,sizeof(reply),0};viewer_dispatch(&viewer,&request,&writer);
            UX_CHECK(strstr(reply,"\"executed\":true") && c.pane->mda.record<0 && c.pane->overview.row<0 && c.pane->overview.column<0);
        }
        c.finds=NULL;c.sidebar=NULL;
    }
    result=0;
cleanup:
    if(imgui)igDestroyContext(imgui);
    *fail_at=0;if(saved)allocator.release(allocator.context,saved);if(history)allocator.release(allocator.context,history);
    if(panes){for(v=0;v<GD_EXPLORER_VIEWS;++v){gd_diagnostics_clear(&panes[v].diagnostics);gd_layout_focus_destroy(panes[v].distance_focus);gd_resource_routes_destroy(panes[v].resource_routes);pane_release_layout(allocator,&panes[v]);}allocator.release(allocator.context,panes);}
    if(c.analysis)allocator.release(allocator.context,c.analysis);gd_explorer_destroy(project);
    printf("{\"ok\":%s,\"shared_ux_checks\":%d,\"scope\":\"shared controls; no window, OS input or source writes\"}\n",result ? "false":"true",checks);
#undef UX_CHECK
    return result;
}
#endif
