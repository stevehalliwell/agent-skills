/* Focused source-read-only Find checks: no layout-motion diagnostics, window or OS input. */
#ifndef TEST_EXPLORER_FIND_H
#define TEST_EXPLORER_FIND_H
static void find_test_frame(GD_ExplorerContext *c,GD_ExplorerPane *panes,GD_ExplorerFind *state){
    igNewFrame();igSetNextWindowSize((ImVec2_c){780,580},ImGuiCond_Always);igSetNextWindowPos((ImVec2_c){0,0},ImGuiCond_Always,(ImVec2_c){0,0});
    igBegin("Find CPU frame",NULL,ImGuiWindowFlags_NoSavedSettings);find_controls(c,panes,state);igEnd();igEndFrame();
}
int gd_explorer_find_checks(GD_Allocator allocator,const GD_Collection *collections,int count,int *fail_at,int *calls){
    GD_ExplorerProject *project=gd_explorer_load(allocator,collections,count);GD_ExplorerContext c={0};
    GD_ExplorerPane *panes=NULL,*before=NULL;GD_Layout *layout_before=NULL;GD_FindMatches matches;GD_FindPattern pattern;
    GD_RenderPose *pose=NULL,*pose_before=NULL;ImGuiContext *imgui=NULL;
    int checks=0,result=2,v,i,node,allocations;char text[GD_FIND_TEXT];
#define FIND_CHECK(value) do{++checks;if(!(value)){fprintf(stderr,"Find check failed: %s (line %d)\n",#value,__LINE__);goto cleanup;}}while(0)
    FIND_CHECK(project && !gd_explorer_error(project));
    panes=allocator.allocate(allocator.context,sizeof(*panes)*GD_EXPLORER_VIEWS);before=allocator.allocate(allocator.context,sizeof(*before)*GD_EXPLORER_VIEWS);
    layout_before=allocator.allocate(allocator.context,sizeof(*layout_before));c.analysis=allocator.allocate(allocator.context,sizeof(*c.analysis));
    if(panes)memset(panes,0,sizeof(*panes)*GD_EXPLORER_VIEWS);if(c.analysis)memset(c.analysis,0,sizeof(*c.analysis));
    FIND_CHECK(panes && before && layout_before && c.analysis);c.project=project;c.viewport=(Rectangle){0,0,800,600};
    for(v=0;v<GD_EXPLORER_VIEWS;++v)pane_defaults(&panes[v],(GD_ExplorerView)v);
    FIND_CHECK(find_key(1,1,0,0)==GD_FIND_OPEN && find_key(1,1,0,1)==GD_FIND_OPEN && find_key(1,0,0,0)==GD_FIND_FIT && find_key(1,0,0,1)==GD_FIND_NONE && find_key(1,0,1,0)==GD_FIND_NONE && find_key(0,1,0,0)==GD_FIND_NONE);
    find_pattern(&pattern,"WaLkEr");FIND_CHECK(find_contains(&pattern,"nouns/walker") && !find_contains(&pattern,"w_a_l_k_e_r"));
    find_pattern(&pattern,"aaab");FIND_CHECK(find_contains(&pattern,"aaaaab") && !find_contains(&pattern,"aaac"));
    find_pattern(&pattern,"Ω");FIND_CHECK(find_contains(&pattern,"entry Ω") && !find_contains(&pattern,"entry ω"));
    for(v=0;v<GD_EXPLORER_VIEWS;++v)if(project->available&(1u<<v)){c.pane=&panes[v];FIND_CHECK(pane_prepare(&c));}
    for(v=0;v<GD_EXPLORER_VIEWS;++v)if(project->available&(1u<<v)){
        GD_ExplorerPane *p=&panes[v];c.pane=p;analyze(&c);
        if(query_view(p->view))snprintf(p->query_options.values,sizeof(p->query_options.values),"Unapplied Find input");
        memcpy(before,panes,sizeof(*before)*GD_EXPLORER_VIEWS);memcpy(layout_before,p->layout,sizeof(*layout_before));allocations=*calls;
        find_collect(&c,"",&matches);FIND_CHECK(!matches.count && !find_navigate(&c,panes,&matches,0));
        find_collect(&c,"__No Find match__",&matches);FIND_CHECK(!matches.count && !find_navigate(&c,panes,&matches,1));
        FIND_CHECK(*calls==allocations && !memcmp(before,panes,sizeof(*before)*GD_EXPLORER_VIEWS) && !memcmp(layout_before,p->layout,sizeof(*layout_before)));
        for(node=0;node<viewer_canvas_count(&c);++node)if(viewer_canvas_visible(&c,node))break;
        if(node==viewer_canvas_count(&c))continue;
        snprintf(text,sizeof(text),"%.100s",find_label(&c,node));find_collect(&c,text,&matches);FIND_CHECK(matches.count>0);
        p->camera.target=(Vector2){100000,100000};
        *fail_at=*calls+1;FIND_CHECK(find_navigate(&c,panes,&matches,0));*fail_at=0;
        FIND_CHECK(*calls==allocations && viewer_selected_node(&c)==matches.nodes[0]);
        FIND_CHECK(p->camera.zoom>=1 && p->camera.zoom<=1.2f && p->camera.target.x!=100000);
        FIND_CHECK(!memcmp(layout_before,p->layout,sizeof(*layout_before)) && p->query_state==before[v].query_state && !memcmp(&p->query_options,&before[v].query_options,sizeof(p->query_options)));
        for(i=0;i<GD_EXPLORER_VIEWS;++i)if(i!=v)FIND_CHECK(!memcmp(&before[i],&panes[i],sizeof(*panes)));
        find_collect(&c,text,&matches);FIND_CHECK(matches.current==0 && find_navigate(&c,panes,&matches,-1));
        FIND_CHECK(viewer_selected_node(&c)==matches.nodes[matches.count-1]);find_collect(&c,text,&matches);
        FIND_CHECK(find_navigate(&c,panes,&matches,1) && viewer_selected_node(&c)==matches.nodes[0]);
        *p=before[v];
    }
    /* Prepared Actor variants remain typed targets, not rewritten actor roots. */
    {const GD_Rules *rules=project->rules;int root,edge,found=0;c.pane=&panes[GD_EXPLORER_ACTORS];c.pane->query_options=c.pane->query_state->applied;
     for(root=0;root<rules->case_count && !found;++root){int actor=rules->cases[root].primary_participant;
      if(actor<0 || rules->participants[actor].noun_resolution!=GD_RULE_RESOLVED)continue;
      for(edge=rules->actor_first[rules->participants[actor].noun_record];edge>=0;edge=rules->next_axis[edge])if(rules->axis_resolution[edge]==GD_RULE_RESOLVED){int axis=rules->model->edges[edge].target;
       if(rules->axes[axis].valid && rules->axes[axis].type==GD_AXIS_ENUM && rules->model->records[axis].axis.allowed_count>1){c.pane->query_options.root_case=root;c.pane->query_options.participant=actor;c.pane->query_options.axis=axis;found=1;break;}}}
     c.pane->query_options.ghosts=GD_QUERY_STACKED_VARIANTS;FIND_CHECK(found && gd_explorer_query_run(&c));
     for(node=0;node<viewer_canvas_count(&c);++node)if(c.pane->query_state->scene->nodes[node].kind==GD_QUERY_NODE_VARIANT)break;FIND_CHECK(node<viewer_canvas_count(&c));
     snprintf(c.pane->query_options.values,sizeof(c.pane->query_options.values),"Unapplied Find variant input");
     memcpy(layout_before,c.pane->layout,sizeof(*layout_before));allocations=*calls;FIND_CHECK(find_select(&c,panes,node) && viewer_selected_node(&c)==node);
     FIND_CHECK(*calls==allocations && !memcmp(layout_before,c.pane->layout,sizeof(*layout_before)) && !strcmp(c.pane->query_options.values,"Unapplied Find variant input"));
     for(node=0;node<viewer_canvas_count(&c);++node)if(c.pane->query_state->scene->nodes[node].kind==GD_QUERY_NODE_VARIANT && c.pane->query_state->scene->nodes[node].ghost)break;FIND_CHECK(node<viewer_canvas_count(&c));
     snprintf(text,sizeof(text),"%s",find_label(&c,node));find_collect(&c,text,&matches);FIND_CHECK(matches.count && find_select(&c,panes,node) && !memcmp(layout_before,c.pane->layout,sizeof(*layout_before)));}
    /* Borrowed One Euro pose stays byte-identical, including sampling history. */
    c.pane=&panes[GD_EXPLORER_RESOURCES];analyze(&c);
    pose=allocator.allocate(allocator.context,sizeof(*pose));pose_before=allocator.allocate(allocator.context,sizeof(*pose_before));FIND_CHECK(pose && pose_before);
    memset(pose,0,sizeof(*pose));c.render_pose=pose;render_update(&c,pose,1.0/60,1);FIND_CHECK(gd_render_pose_matches(pose,c.pane->layout,render_identity(&c)));memcpy(pose_before,pose,sizeof(*pose));
    FIND_CHECK(find_select(&c,panes,0) && !memcmp(pose,pose_before,sizeof(*pose)));c.render_pose=NULL;
    /* Grid headers, not invented positive cells/edges. */
    for(i=0;i<2;++i){
        GD_ExplorerPane *p;v=i ? GD_EXPLORER_NOUNS:GD_EXPLORER_MATRIX;p=&panes[v];c.pane=p;p->layout_choice=v==GD_EXPLORER_MATRIX ? GD_MATRIX_GRID_CHOICE:GD_NOUN_LAYOUT_GRID;p->prepared=0;
        FIND_CHECK(pane_prepare(&c));analyze(&c);p->grid.noun=p->grid.order[0];p->grid.verb=p->grid.order[v==GD_EXPLORER_MATRIX ? p->grid.nouns:0];p->selection=(GD_ExplorerSelection){-1,-1,0,-1,-1};
        before[v]=*p;find_collect(&c,"__No named header__",&matches);FIND_CHECK(!find_navigate(&c,panes,&matches,0) && !memcmp(&before[v],p,sizeof(*p)));
        snprintf(text,sizeof(text),"%.100s",find_label(&c,0));find_collect(&c,text,&matches);
        memcpy(layout_before,p->layout,sizeof(*layout_before));FIND_CHECK(matches.count && find_navigate(&c,panes,&matches,0));
        FIND_CHECK(p->selection.node>=0 && p->selection.cell<0 && !memcmp(layout_before,p->layout,sizeof(*layout_before)));
    }
    c.pane=&panes[GD_EXPLORER_MDA];
    for(i=0;i<4;++i)c.pane->mda.roles[i]=false;
    find_collect(&c,"/",&matches);FIND_CHECK(matches.count==0);
    for(i=0;i<4;++i)c.pane->mda.roles[i]=true;
    /* ImGui-only typing/Enter/Escape; never posts input to native OS. */
    imgui=igCreateContext(NULL);FIND_CHECK(imgui!=NULL);igGetIO_Nil()->IniFilename=NULL;igGetIO_Nil()->LogFilename=NULL;
    igGetIO_Nil()->DisplaySize=(ImVec2_c){800,600};igGetIO_Nil()->DeltaTime=1.0f/60.0f;igGetIO_Nil()->BackendFlags|=ImGuiBackendFlags_RendererHasTextures;
    {GD_ExplorerFind state={0};Camera2D camera=c.pane->camera;int selected;
     state.open=state.focus=true;analyze(&c);find_test_frame(&c,panes,&state);find_test_frame(&c,panes,&state);
     FIND_CHECK(!memcmp(&camera,&c.pane->camera,sizeof(camera)));
     snprintf(text,sizeof(text),"/");ImGuiIO_AddInputCharactersUTF8(igGetIO_Nil(),text);find_test_frame(&c,panes,&state);
     FIND_CHECK(!strcmp(state.text,text));find_collect(&c,state.text,&matches);FIND_CHECK(matches.count>2 && viewer_selected_node(&c)==matches.nodes[0]);
     FIND_CHECK(find_navigate(&c,panes,&matches,1) && find_navigate(&c,panes,&matches,1) && viewer_selected_node(&c)==matches.nodes[2] && find_navigate(&c,panes,&matches,0));
     ImGuiIO_AddKeyEvent(igGetIO_Nil(),ImGuiKey_Enter,true);find_test_frame(&c,panes,&state);FIND_CHECK(viewer_selected_node(&c)==matches.nodes[1]);
     ImGuiIO_AddKeyEvent(igGetIO_Nil(),ImGuiKey_Enter,false);find_test_frame(&c,panes,&state);find_test_frame(&c,panes,&state);
     ImGuiIO_AddKeyEvent(igGetIO_Nil(),ImGuiKey_Enter,true);find_test_frame(&c,panes,&state);FIND_CHECK(viewer_selected_node(&c)==matches.nodes[2]);
     ImGuiIO_AddKeyEvent(igGetIO_Nil(),ImGuiKey_Enter,false);find_test_frame(&c,panes,&state);find_test_frame(&c,panes,&state);
     ImGuiIO_AddKeyEvent(igGetIO_Nil(),ImGuiMod_Shift,true);ImGuiIO_AddKeyEvent(igGetIO_Nil(),ImGuiKey_Enter,true);find_test_frame(&c,panes,&state);FIND_CHECK(viewer_selected_node(&c)==matches.nodes[1]);
     ImGuiIO_AddKeyEvent(igGetIO_Nil(),ImGuiMod_Shift,false);ImGuiIO_AddKeyEvent(igGetIO_Nil(),ImGuiKey_Enter,false);find_test_frame(&c,panes,&state);find_test_frame(&c,panes,&state);
     selected=viewer_selected_node(&c);camera=c.pane->camera;ImGuiIO_AddKeyEvent(igGetIO_Nil(),ImGuiKey_Escape,true);find_test_frame(&c,panes,&state);
     FIND_CHECK(!state.open && viewer_selected_node(&c)==selected && !memcmp(&camera,&c.pane->camera,sizeof(camera)));}
    igDestroyContext(imgui);imgui=NULL;
    /* Pending rank input blocks selection without commit/discard. */
    {GD_ExplorerSkillEditor saved=panes[GD_EXPLORER_SKILLS].editor;GD_Model *skills=gd_explorer_model(project,GD_EXPLORER_SKILLS);
     for(i=0;i<skills->record_count;++i)if(skills->records[i].role==GD_SKILL)break;FIND_CHECK(i<skills->record_count);
     panes[GD_EXPLORER_SKILLS].editor.record=i;snprintf(panes[GD_EXPLORER_SKILLS].editor.rank_text,sizeof(saved.rank_text),"unfinished rank");
     memcpy(before,panes,sizeof(*before)*GD_EXPLORER_VIEWS);FIND_CHECK(!find_select(&c,panes,0) && !memcmp(before,panes,sizeof(*before)*GD_EXPLORER_VIEWS));panes[GD_EXPLORER_SKILLS].editor=saved;}
    /* Find changes selection/camera, not selection-driven distance pose. */
    FIND_CHECK(distance_focus_toggle(&c,1));node=analyze(&c);distance_focus_update(&c,node);memcpy(layout_before,c.pane->layout,sizeof(*layout_before));
    FIND_CHECK(find_select(&c,panes,0));distance_focus_update(&c,analyze(&c));FIND_CHECK(!memcmp(layout_before,c.pane->layout,sizeof(*layout_before)));
    c.pane->find_hold_node=-1;distance_focus_update(&c,analyze(&c));FIND_CHECK(c.pane->distance_focus->anchor_node==0);FIND_CHECK(distance_focus_toggle(&c,0));
    /* Actual retained similarity map: no ordinary graph or coordinate mutation. */
    FIND_CHECK(gd_explorer_action_run(&c,GD_ACTION_STRUCTURE_RUN));c.pane->structure_state->pending.kind=GD_STRUCTURE_COUNT+1;
    snprintf(c.pane->structure_state->pending.features,sizeof(c.pane->structure_state->pending.features),"retained-partners");FIND_CHECK(gd_explorer_action_run(&c,GD_ACTION_STRUCTURE_RUN));
    FIND_CHECK(gd_explorer_similarity_active(&c));
    {GD_Similarity *map=c.pane->structure_state->result->similarity->map;Camera2D ordinary=c.pane->camera;int hold=c.pane->find_hold_node;const GD_Layout *held_layout=c.pane->find_hold_layout;uint64_t hash=viewer_control_hash(UINT64_C(1469598103934665603),map,sizeof(*map));
     snprintf(text,sizeof(text),"%.100s",find_label(&c,0));find_collect(&c,text,&matches);FIND_CHECK(matches.count && find_navigate(&c,panes,&matches,0));
     FIND_CHECK(c.pane->find_hold_node==hold && c.pane->find_hold_layout==held_layout);
     FIND_CHECK(!memcmp(&ordinary,&c.pane->camera,sizeof(ordinary)) && hash==viewer_control_hash(UINT64_C(1469598103934665603),map,sizeof(*map)));
     map->placed[0]=0;FIND_CHECK(!find_select(&c,panes,0));map->placed[0]=1;}
    FIND_CHECK(!project->draft || (!project->draft->modified_count && !project->draft->history_count));
    if(project->draft){GD_Model *skills=gd_explorer_model(project,GD_EXPLORER_SKILLS);int history,modified;
        for(i=0;i<skills->record_count;++i)if(skills->records[i].role==GD_SKILL)break;
        FIND_CHECK(i<skills->record_count && gd_draft_rank(project->draft,i,"987654321")>0);history=project->draft->history_count;modified=project->draft->modified_count;
        FIND_CHECK(find_select(&c,panes,0) && project->draft->history_count==history && project->draft->modified_count==modified);
    }
    result=0;
cleanup:
    *fail_at=0;if(imgui)igDestroyContext(imgui);if(pose)allocator.release(allocator.context,pose);if(pose_before)allocator.release(allocator.context,pose_before);
    if(panes){for(v=0;v<GD_EXPLORER_VIEWS;++v){gd_diagnostics_clear(&panes[v].diagnostics);gd_layout_focus_destroy(panes[v].distance_focus);gd_resource_routes_destroy(panes[v].resource_routes);pane_release_layout(allocator,&panes[v]);}allocator.release(allocator.context,panes);}
    if(before)allocator.release(allocator.context,before);if(layout_before)allocator.release(allocator.context,layout_before);if(c.analysis)allocator.release(allocator.context,c.analysis);gd_explorer_destroy(project);
    printf("{\"ok\":%s,\"find_checks\":%d,\"scope\":\"current shown canvas; no window, OS input or source writes\"}\n",result ? "false":"true",checks);
#undef FIND_CHECK
    return result;
}
#endif
