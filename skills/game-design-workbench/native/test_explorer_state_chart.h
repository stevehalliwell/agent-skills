/* Source-read-only application tests. No window, OS input, or implicit Run. */
#ifndef TEST_EXPLORER_STATE_CHART_H
#define TEST_EXPLORER_STATE_CHART_H
static int chart_ipc_checks(GD_ExplorerContext *context,int noun,int axis,int companion,int *calls) {
    GD_Allocator allocator=context->project->allocator;const GD_Model *model=context->project->rules->model;
    GD_ExplorerContext owner=*context;GD_ExplorerPane *pane=context->pane;GD_ExplorerChartState *state;
    GD_Draft *draft=context->project->draft;GD_Query *retained;GD_Camera camera=pane->camera,cameras[2];
    GD_ViewerRequest *request=allocator.allocate(allocator.context,sizeof(*request));
    ExplorerViewer *viewer=allocator.allocate(allocator.context,sizeof(*viewer));
    char *reply=allocator.allocate(allocator.context,GD_VIEWER_REPLY_LIMIT);
    ImGuiContext *imgui=igCreateContext(NULL);GD_ViewerWriter writer;
    char old_token[64],cell_key[64],hypothesis_key[64];const char *key;int checks=0,result=0,before,active=GD_EXPLORER_MATRIX,stale=0,undo=draft->history_count,redo=draft->redo_count;
#define IPC_CHECK(value) do{++checks;if(!(value)){fprintf(stderr,"State chart IPC check failed: %s (line %d)\n",#value,__LINE__);goto done;}}while(0)
#define IPC_REQUEST(command_,form_) do{memset(request,0,sizeof(*request));strcpy(request->command,command_);strcpy(request->projection,"state-charts");strcpy(request->form,form_);request->limit=1;}while(0)
#define IPC_TOKEN(form_) viewer_control_token(viewer,context,form_,request->expect,sizeof(request->expect))
#define IPC_SEND() do{writer=(GD_ViewerWriter){reply,0,GD_VIEWER_REPLY_LIMIT,0};viewer_dispatch(viewer,request,&writer);IPC_CHECK(!writer.failed);}while(0)
#define IPC_ACTION(name_) do{IPC_REQUEST("action","");strcpy(request->action,name_);}while(0)
#define IPC_READ_KEY(out_) do{key=strstr(reply,"\"key\":\"chart-");IPC_CHECK(key && sscanf(key+7,"%63[^\"]",out_)==1);}while(0)
    IPC_CHECK(request && viewer && reply && imgui);igGetIO_Nil()->IniFilename=NULL;
    owner.pane=&context->panes[active];owner.active_view=&active;
    memset(viewer,0,sizeof(*viewer));viewer->context=&owner;viewer->panes=context->panes;viewer->active=&active;viewer->stale=&stale;
    before=*calls;
    IPC_REQUEST("controls","chart");strcpy(request->control,"object");request->offset=noun+1;IPC_SEND();
    IPC_CHECK(strstr(reply,model->records[noun].reference) && !pane->chart_state && !pane->prepared && *calls==before);
    IPC_REQUEST("controls","chart");strcpy(request->control,"axis");request->offset=axis+1;IPC_SEND();
    IPC_CHECK(strstr(reply,"\"choices\":[]") && !pane->chart_state && *calls==before);
    IPC_REQUEST("evidence","chart-hypotheses");IPC_SEND();IPC_CHECK(strstr(reply,"\"total_rows\":0") && *calls==before);
    IPC_ACTION("chart.run");IPC_SEND();IPC_CHECK(strstr(reply,"viewer.stale_controls") && !pane->chart_state);
    IPC_TOKEN(GD_FORM_CHART);strcpy(old_token,request->expect);
    IPC_REQUEST("configure","chart");strcpy(request->control,"object");strcpy(request->value,model->records[noun].reference);IPC_TOKEN(GD_FORM_CHART);IPC_SEND();
    IPC_CHECK(strstr(reply,"\"configured\":true") && pane->chart_state && !pane->prepared && !pane->chart_state->query);
    state=pane->chart_state;
    IPC_REQUEST("controls","chart");strcpy(request->control,"axis");request->offset=axis+1;IPC_SEND();IPC_CHECK(strstr(reply,model->records[axis].reference));
    IPC_REQUEST("configure","chart");strcpy(request->control,"axis");strcpy(request->value,model->records[axis].reference);IPC_TOKEN(GD_FORM_CHART);IPC_SEND();
    IPC_CHECK(strstr(reply,"\"configured\":true") && !state->query && !pane->prepared);
    IPC_REQUEST("controls","chart");strcpy(request->control,"companion_axis");IPC_SEND();IPC_CHECK(strstr(reply,"\"key\":\"\""));
    request->offset=axis+1;IPC_SEND();IPC_CHECK(strstr(reply,"\"choices\":[]"));
    if(companion>=0){
        request->offset=companion+1;IPC_SEND();IPC_CHECK(strstr(reply,model->records[companion].reference));
        IPC_REQUEST("configure","chart");strcpy(request->control,"companion_axis");strcpy(request->value,model->records[companion].reference);IPC_TOKEN(GD_FORM_CHART);IPC_SEND();
        IPC_CHECK(strstr(reply,"\"configured\":true") && !state->query);
    }
    IPC_ACTION("chart.run");strcpy(request->expect,old_token);IPC_SEND();IPC_CHECK(strstr(reply,"viewer.stale_controls") && !state->query && !pane->prepared);
    IPC_TOKEN(GD_FORM_CHART);IPC_SEND();IPC_CHECK(strstr(reply,"\"executed\":true") && state->query && !pane->prepared);
    retained=state->query;pane->fit=0;before=*calls;memcpy(cameras,state->cameras,sizeof(cameras));
    IPC_REQUEST("evidence","chart-cells");IPC_SEND();IPC_READ_KEY(cell_key);
    IPC_CHECK(strstr(reply,"\"next_offset\":1") && strstr(reply,model->records[axis].reference));
    IPC_REQUEST("evidence","chart-hypotheses");IPC_SEND();IPC_READ_KEY(hypothesis_key);
    IPC_CHECK(strstr(reply,"\"case\":") && strstr(reply,"\"source\":") && strstr(reply,"\"participant\":") && strstr(reply,"\"value\":"));
    IPC_TOKEN(GD_FORM_PRESENTATION);strcpy(old_token,request->expect);
    IPC_REQUEST("select","chart.cell");strcpy(request->id,"0");IPC_TOKEN(GD_FORM_PRESENTATION);IPC_SEND();IPC_CHECK(strstr(reply,"viewer.selection") && state->row==-1 && state->scenario==-1);
    strcpy(request->id,cell_key);IPC_SEND();IPC_CHECK(strstr(reply,"\"selected\":true") && state->row==0 && state->column==0 && state->scenario==-1);
    IPC_REQUEST("select","chart.hypothesis");strcpy(request->id,hypothesis_key);strcpy(request->expect,old_token);IPC_SEND();IPC_CHECK(strstr(reply,"viewer.stale_controls") && state->scenario==-1);
    IPC_TOKEN(GD_FORM_PRESENTATION);IPC_SEND();IPC_CHECK(strstr(reply,"viewer.selection") && state->scenario==-1);
    IPC_REQUEST("evidence","chart-hypotheses");IPC_SEND();IPC_READ_KEY(hypothesis_key);
    IPC_REQUEST("select","chart.hypothesis");strcpy(request->id,hypothesis_key);IPC_TOKEN(GD_FORM_PRESENTATION);IPC_SEND();
    IPC_CHECK(strstr(reply,"\"selected\":true") && state->scenario==0 && state->row==0 && state->column==0 && pane->selection.linked>=0);
    IPC_CHECK(*calls==before && state->query==retained && !pane->prepared && !pane->fit && !state->mode);
    IPC_CHECK(!memcmp(&camera,&pane->camera,sizeof(camera)) && !memcmp(cameras,state->cameras,sizeof(cameras)));
    IPC_ACTION("chart.clear");IPC_SEND();IPC_CHECK(strstr(reply,"viewer.stale_controls") && state->query==retained && state->scenario==0);
    IPC_TOKEN(GD_FORM_CHART);strcpy(old_token,request->expect);
    IPC_REQUEST("configure","chart");strcpy(request->control,"work_limit");strcpy(request->value,"0");IPC_TOKEN(GD_FORM_CHART);IPC_SEND();
    IPC_CHECK(strstr(reply,"\"configured\":true") && state->query==retained && state->pending.work_limit==0 && state->applied.work_limit>0);
    IPC_ACTION("chart.clear");strcpy(request->expect,old_token);IPC_SEND();
    IPC_CHECK(strstr(reply,"viewer.stale_controls") && state->query==retained && state->pending.work_limit==0 && state->applied.work_limit>0 && state->scenario==0);
    IPC_TOKEN(GD_FORM_CHART);IPC_SEND();IPC_CHECK(strstr(reply,"\"executed\":true") && !state->query && state->pending.work_limit==0 && state->scenario==-1);
    IPC_CHECK(active==GD_EXPLORER_MATRIX && !owner.pane->prepared && !draft->modified_count && draft->history_count==undo && draft->redo_count==redo);
    gd_explorer_form_defaults(GD_FORM_CHART,&state->pending);result=checks;
done:
    if(imgui)igDestroyContext(imgui);if(reply)allocator.release(allocator.context,reply);if(viewer)allocator.release(allocator.context,viewer);if(request)allocator.release(allocator.context,request);
    return result;
#undef IPC_CHECK
#undef IPC_REQUEST
#undef IPC_TOKEN
#undef IPC_SEND
#undef IPC_ACTION
#undef IPC_READ_KEY
}
int gd_explorer_state_chart_checks(GD_Allocator allocator,const GD_Collection *collections,int count,int *fail_at,int *calls) {
    GD_ExplorerContext context={0};GD_ExplorerPane *panes=NULL;GD_ExplorerProject *project=gd_explorer_load(allocator,collections,count);GD_ExplorerChartState *state;GD_Model *model;GD_Query *retained;int active=GD_EXPLORER_STATES,checks=0,result=2,i,noun,axis=-1,companion=-1;char message[256],failure[256];
#define CHART_CHECK(value) do{++checks;if(!(value)){fprintf(stderr,"State chart view check failed: %s (line %d)\n",#value,__LINE__);goto cleanup;}}while(0)
    CHART_CHECK(project && !gd_explorer_error(project) && (project->available&(1u<<GD_EXPLORER_STATES)));
    panes=allocator.allocate(allocator.context,GD_EXPLORER_VIEWS*sizeof(*panes));if(panes)memset(panes,0,GD_EXPLORER_VIEWS*sizeof(*panes));context.analysis=allocator.allocate(allocator.context,sizeof(*context.analysis));CHART_CHECK(panes && context.analysis);
    memset(panes,0,GD_EXPLORER_VIEWS*sizeof(*panes));memset(context.analysis,0,sizeof(*context.analysis));for(i=0;i<GD_EXPLORER_VIEWS;++i)pane_defaults(&panes[i],(GD_ExplorerView)i);
    context.history=allocator.allocate(allocator.context,sizeof(*context.history));CHART_CHECK(context.history);memset(context.history,0,sizeof(*context.history));
    context.project=project;context.panes=panes;context.pane=&panes[active];context.active_view=&active;context.viewport=(GD_Rect){0,0,880,800};
    model=gd_explorer_model(project,GD_EXPLORER_STATES);noun=gd_query_record(model,"nouns/shellback",GD_NOUN);if(noun<0)noun=gd_query_record(model,"nouns/switch",GD_NOUN);CHART_CHECK(noun>=0);
    for(i=project->rules->actor_first[noun];i>=0;i=project->rules->next_axis[i])if(project->rules->axis_resolution[i]==GD_RULE_RESOLVED){int target=model->edges[i].target;if(project->rules->axes[target].valid && project->rules->axes[target].type==GD_AXIS_ENUM){if(axis<0)axis=target;else if(target!=axis)companion=target;}}
    CHART_CHECK(axis>=0);
    {int ipc=chart_ipc_checks(&context,noun,axis,companion,calls);CHART_CHECK(ipc>0);checks+=ipc;}
    CHART_CHECK(pane_prepare(&context) && context.pane->chart_state && !context.pane->chart_state->query && !context.pane->layout->node_count);
    state=context.pane->chart_state;snprintf(state->pending.object,sizeof(state->pending.object),"%s",model->records[noun].reference);snprintf(state->pending.axis,sizeof(state->pending.axis),"%s",model->records[axis].reference);if(companion>=0)snprintf(state->pending.companion_axis,sizeof(state->pending.companion_axis),"%s",model->records[companion].reference);
    CHART_CHECK(gd_explorer_action_run(&context,GD_ACTION_CHART_RUN));retained=state->query;CHART_CHECK(retained && retained->state_chart && retained->state_chart->scenario_count>0 && state->node_count[0]>0);
    CHART_CHECK(gd_explorer_form_equal(GD_FORM_CHART,&state->pending,&state->applied));
    {int total=0,columns=companion>=0 ? retained->state_chart->companion_count:1;for(i=0;i<retained->state_coverage->value_count*columns;++i)total+=state->cells[i].retained;CHART_CHECK(total==retained->state_chart->scenario_count);}
    CHART_CHECK(state->edge_count<=retained->state_chart->write_count);
    history_observe(&context);state->row=0;state->column=companion>=0 ? 0:-1;state->scenario=0;history_observe(&context);
    CHART_CHECK(gd_explorer_action_run(&context,GD_ACTION_BACK) && state->row==-1 && state->scenario==-1);
    CHART_CHECK(gd_explorer_action_run(&context,GD_ACTION_FORWARD) && state->row==0 && state->scenario==0);
    {int before=*calls;CHART_CHECK(gd_explorer_action_available(&context,GD_ACTION_VIEW_SELECTION_FOCUS));
        CHART_CHECK(gd_explorer_action_run(&context,GD_ACTION_VIEW_SELECTION_FOCUS) && context.pane->focus_result.selected>0 && context.pane->camera.zoom>=1);
        CHART_CHECK(frame_selection(&context) && isfinite(context.pane->camera.target.x) && isfinite(context.pane->camera.target.y));
        CHART_CHECK(*calls==before && state->query==retained && state->row==0 && state->scenario==0 && !context.pane->layout->node_count);
    }
    {int before=*calls;GD_Camera chart=context.pane->camera;GD_Rect viewport=context.viewport;
        CHART_CHECK(gd_explorer_presentation_set(&context,"state_chart_mode","grid",message,sizeof(message)));CHART_CHECK(*calls==before && state->query==retained && state->mode==1 && state->scenario==0 && state->row==0);
        CHART_CHECK(context.pane->fit==GD_EXPLORER_FIT_GRID_ALL);
        count_grid_viewport(&context);apply_pending_fit(&context);
        GD_Camera whole=context.pane->camera;context.viewport=viewport;
        CHART_CHECK(whole.zoom>0 && state->query==retained && state->scenario==0 && *calls==before);
        state->scenario=-1;state->column=-1;CHART_CHECK(gd_explorer_chart_cell_filtered(state,0,0) && gd_explorer_chart_cell_filtered(state,0,1) && !gd_explorer_chart_cell_filtered(state,1,0));
        CHART_CHECK(frame_selection(&context) && context.pane->focus_result.selected==(companion>=0 ? retained->state_chart->companion_count:1) && *calls==before && state->query==retained);
        state->scenario=0;state->column=companion>=0 ? 0:-1;CHART_CHECK(gd_explorer_presentation_set(&context,"state_chart_mode","statechart",message,sizeof(message)));CHART_CHECK(state->query==retained && state->mode==0 && !memcmp(&chart,&context.pane->camera,sizeof(chart)));
        CHART_CHECK(gd_explorer_presentation_set(&context,"state_chart_mode","grid",message,sizeof(message)) && context.pane->fit==GD_EXPLORER_FIT_GRID_ALL);
        count_grid_viewport(&context);apply_pending_fit(&context);context.viewport=viewport;
        CHART_CHECK(!memcmp(&whole,&context.pane->camera,sizeof(whole)) && state->query==retained && state->scenario==0 && *calls==before);
        enter_view(&context,panes,GD_EXPLORER_NOUNS,NULL);
        CHART_CHECK(context.pane->fit==GD_EXPLORER_FIT_GRID_ALL);
        enter_view(&context,panes,GD_EXPLORER_STATES,NULL);
        CHART_CHECK(context.pane->fit==GD_EXPLORER_FIT_GRID_ALL && state->query==retained && state->scenario==0 && *calls==before);
        CHART_CHECK(gd_explorer_presentation_set(&context,"state_chart_mode","statechart",message,sizeof(message)));}
    CHART_CHECK(gd_explorer_control_set(&context,GD_FORM_CHART,"object","nouns/not-present",message,sizeof(message)));CHART_CHECK(state->query==retained && strcmp(state->pending.object,state->applied.object));
    CHART_CHECK(!gd_explorer_action_run(&context,GD_ACTION_CHART_RUN) && state->query==retained);
    CHART_CHECK(!reload_project(&context,panes,collections,count,failure,sizeof(failure)) && context.project==project && state->query==retained);
    state->pending=state->applied;
    for(i=1;i<=4;++i){*fail_at=*calls+i;CHART_CHECK(!gd_explorer_action_run(&context,GD_ACTION_CHART_RUN));*fail_at=0;CHART_CHECK(state->query==retained && state->row==0 && state->scenario==0);}
    CHART_CHECK(gd_explorer_action_run(&context,GD_ACTION_VIEW_RESET) && state->query==retained && state->scenario==0);
    {GD_Camera camera=context.pane->camera;CHART_CHECK(gd_explorer_action_run(&context,GD_ACTION_SELECTION_CLEAR));CHART_CHECK(state->query==retained && state->row==0 && state->column==(companion>=0 ? 0:-1) && state->scenario==-1 && !memcmp(&camera,&context.pane->camera,sizeof(camera)));}
    CHART_CHECK(gd_explorer_action_run(&context,GD_ACTION_INPUTS_RESET) && state->query==retained && !*state->pending.object && *state->applied.object);
    state->row=0;state->column=companion>=0 ? 0:-1;state->scenario=0;state->pending=state->applied;CHART_CHECK(reload_project(&context,panes,collections,count,failure,sizeof(failure)));project=context.project;state=context.pane->chart_state;CHART_CHECK(state && state->query && state->query!=retained && state->row==0 && state->scenario>=0 && state->query->state_chart->scenario_count>0 && gd_explorer_form_equal(GD_FORM_CHART,&state->pending,&state->applied));
    CHART_CHECK(gd_explorer_action_run(&context,GD_ACTION_CHART_CLEAR) && !state->query && *state->pending.object);
    CHART_CHECK(gd_explorer_action_run(&context,GD_ACTION_CHART_RUN) && state->query);
    {GD_Draft *draft=project->draft;CHART_CHECK(gd_explorer_action_run(&context,GD_ACTION_RESET_ALL));CHART_CHECK(project->draft==draft && context.pane->chart_state && !context.pane->chart_state->query && !*context.pane->chart_state->pending.object);}
    printf("{\"ok\":true,\"checks\":%d,\"scope\":\"state-chart native application transitions and IPC dispatch; no window or OS input\"}\n",checks);result=0;
cleanup:
    *fail_at=0;if(panes){for(i=0;i<GD_EXPLORER_VIEWS;++i){pane_release_layout(allocator,&panes[i]);gd_layout_focus_destroy(panes[i].distance_focus);gd_resource_routes_destroy(panes[i].resource_routes);gd_diagnostics_clear(&panes[i].diagnostics);}allocator.release(allocator.context,panes);}if(context.analysis)allocator.release(allocator.context,context.analysis);if(context.history)allocator.release(allocator.context,context.history);gd_explorer_destroy(project);return result;
#undef CHART_CHECK
}
#endif
