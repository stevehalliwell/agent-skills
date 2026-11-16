/* Source-read-only application tests. No window, OS input, or implicit Run. */
#ifndef TEST_EXPLORER_STATE_CHART_H
#define TEST_EXPLORER_STATE_CHART_H
int gd_explorer_state_chart_checks(GD_Allocator allocator,const GD_Collection *collections,int count,int *fail_at,int *calls) {
    GD_ExplorerContext context={0};GD_ExplorerPane *panes=NULL;GD_ExplorerProject *project=gd_explorer_load(allocator,collections,count);GD_ExplorerChartState *state;GD_Model *model;GD_Query *retained;int active=GD_EXPLORER_STATES,checks=0,result=2,i,noun,axis=-1,companion=-1;char message[256],failure[256];
#define CHART_CHECK(value) do{++checks;if(!(value)){fprintf(stderr,"State chart view check failed: %s (line %d)\n",#value,__LINE__);goto cleanup;}}while(0)
    CHART_CHECK(project && !gd_explorer_error(project) && (project->available&(1u<<GD_EXPLORER_STATES)));
    panes=allocator.allocate(allocator.context,GD_EXPLORER_VIEWS*sizeof(*panes));if(panes)memset(panes,0,GD_EXPLORER_VIEWS*sizeof(*panes));context.analysis=allocator.allocate(allocator.context,sizeof(*context.analysis));CHART_CHECK(panes && context.analysis);
    memset(panes,0,GD_EXPLORER_VIEWS*sizeof(*panes));memset(context.analysis,0,sizeof(*context.analysis));for(i=0;i<GD_EXPLORER_VIEWS;++i)pane_defaults(&panes[i],(GD_ExplorerView)i);
    context.history=allocator.allocate(allocator.context,sizeof(*context.history));CHART_CHECK(context.history);memset(context.history,0,sizeof(*context.history));
    context.project=project;context.panes=panes;context.pane=&panes[active];context.active_view=&active;context.viewport=(Rectangle){0,0,880,800};
    CHART_CHECK(pane_prepare(&context) && context.pane->chart_state && !context.pane->chart_state->query && !context.pane->layout->node_count);
    state=context.pane->chart_state;model=gd_explorer_model(project,GD_EXPLORER_STATES);noun=gd_query_record(model,"nouns/shellback",GD_NOUN);if(noun<0)noun=gd_query_record(model,"nouns/switch",GD_NOUN);CHART_CHECK(noun>=0);
    for(i=project->rules->actor_first[noun];i>=0;i=project->rules->next_axis[i])if(project->rules->axis_resolution[i]==GD_RULE_RESOLVED){int target=model->edges[i].target;if(project->rules->axes[target].valid && project->rules->axes[target].type==GD_AXIS_ENUM){if(axis<0)axis=target;else if(target!=axis)companion=target;}}
    CHART_CHECK(axis>=0);snprintf(state->pending.object,sizeof(state->pending.object),"%s",model->records[noun].reference);snprintf(state->pending.axis,sizeof(state->pending.axis),"%s",model->records[axis].reference);if(companion>=0)snprintf(state->pending.companion_axis,sizeof(state->pending.companion_axis),"%s",model->records[companion].reference);
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
    {int before=*calls;CHART_CHECK(gd_explorer_presentation_set(&context,"state_chart_mode","grid",message,sizeof(message)));CHART_CHECK(*calls==before && state->query==retained && state->mode==1 && state->scenario==0 && state->row==0);
        state->scenario=-1;state->column=-1;CHART_CHECK(gd_explorer_chart_cell_filtered(state,0,0) && gd_explorer_chart_cell_filtered(state,0,1) && !gd_explorer_chart_cell_filtered(state,1,0));
        CHART_CHECK(frame_selection(&context) && context.pane->focus_result.selected==(companion>=0 ? retained->state_chart->companion_count:1) && *calls==before && state->query==retained);
        state->scenario=0;state->column=companion>=0 ? 0:-1;CHART_CHECK(gd_explorer_presentation_set(&context,"state_chart_mode","statechart",message,sizeof(message)));CHART_CHECK(state->query==retained && state->mode==0);}
    CHART_CHECK(gd_explorer_control_set(&context,GD_FORM_CHART,"object","nouns/not-present",message,sizeof(message)));CHART_CHECK(state->query==retained && strcmp(state->pending.object,state->applied.object));
    CHART_CHECK(!gd_explorer_action_run(&context,GD_ACTION_CHART_RUN) && state->query==retained);
    CHART_CHECK(!reload_project(&context,panes,collections,count,failure,sizeof(failure)) && context.project==project && state->query==retained);
    state->pending=state->applied;
    for(i=1;i<=4;++i){*fail_at=*calls+i;CHART_CHECK(!gd_explorer_action_run(&context,GD_ACTION_CHART_RUN));*fail_at=0;CHART_CHECK(state->query==retained && state->row==0 && state->scenario==0);}
    CHART_CHECK(gd_explorer_action_run(&context,GD_ACTION_VIEW_RESET) && state->query==retained && state->scenario==0);
    {Camera2D camera=context.pane->camera;CHART_CHECK(gd_explorer_action_run(&context,GD_ACTION_SELECTION_CLEAR));CHART_CHECK(state->query==retained && state->row==0 && state->column==(companion>=0 ? 0:-1) && state->scenario==-1 && !memcmp(&camera,&context.pane->camera,sizeof(camera)));}
    CHART_CHECK(gd_explorer_action_run(&context,GD_ACTION_INPUTS_RESET) && state->query==retained && !*state->pending.object && *state->applied.object);
    state->row=0;state->column=companion>=0 ? 0:-1;state->scenario=0;state->pending=state->applied;CHART_CHECK(reload_project(&context,panes,collections,count,failure,sizeof(failure)));project=context.project;state=context.pane->chart_state;CHART_CHECK(state && state->query && state->query!=retained && state->row==0 && state->scenario>=0 && state->query->state_chart->scenario_count>0 && gd_explorer_form_equal(GD_FORM_CHART,&state->pending,&state->applied));
    CHART_CHECK(gd_explorer_action_run(&context,GD_ACTION_CHART_CLEAR) && !state->query && *state->pending.object);
    CHART_CHECK(gd_explorer_action_run(&context,GD_ACTION_CHART_RUN) && state->query);
    {GD_Draft *draft=project->draft;CHART_CHECK(gd_explorer_action_run(&context,GD_ACTION_RESET_ALL));CHART_CHECK(project->draft==draft && context.pane->chart_state && !context.pane->chart_state->query && !*context.pane->chart_state->pending.object);}
    printf("{\"ok\":true,\"checks\":%d,\"scope\":\"state-chart native application transitions; no window or OS input\"}\n",checks);result=0;
cleanup:
    *fail_at=0;if(panes){for(i=0;i<GD_EXPLORER_VIEWS;++i){pane_release_layout(allocator,&panes[i]);gd_layout_focus_destroy(panes[i].distance_focus);gd_resource_routes_destroy(panes[i].resource_routes);gd_diagnostics_clear(&panes[i].diagnostics);}allocator.release(allocator.context,panes);}if(context.analysis)allocator.release(allocator.context,context.analysis);if(context.history)allocator.release(allocator.context,context.history);gd_explorer_destroy(project);return result;
#undef CHART_CHECK
}
#endif
