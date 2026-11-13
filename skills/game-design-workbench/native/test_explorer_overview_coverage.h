/* Focused, source-read-only coverage. No window, pointer injection or solver run. */
#ifndef TEST_EXPLORER_OVERVIEW_COVERAGE_H
#define TEST_EXPLORER_OVERVIEW_COVERAGE_H
static int explorer_overview_coverage_checks(GD_ExplorerContext *c,GD_Layout *saved,int *fail_at,int *calls) {
    int scenario,checks=0;
#define OVERVIEW_CHECK(value) do{++checks;if(!(value)){fprintf(stderr,"Overview coverage failed: scenario %d line %d: %s\n",scenario,__LINE__,#value);return 0;}}while(0)
    for(scenario=0;scenario<8;++scenario) {
        int view=scenario==0 ? GD_EXPLORER_NOUNS:scenario<6 ? GD_EXPLORER_ACTORS:GD_EXPLORER_EFFECTS;
        GD_ExplorerPane *p=&c->panes[view];GD_ExplorerQueryOptions pending,applied={0};GD_Query *query;GD_QueryScene *scene;
        const GD_GraphOverview *overview;int i,cell,marked=0,allocations,total=0;Camera2D camera;ExplorerViewer viewer={0};GD_ViewerRequest request={0};
        c->pane=p;*c->active_view=view;c->analysis->initialized=0;
        OVERVIEW_CHECK(pane_prepare(c));
        if(scenario==0) {
            OVERVIEW_CHECK(!overview_available(c) && !gd_explorer_presentation_editable(c,"overview_enabled") && !gd_explorer_action_available(c,GD_ACTION_VIEW_ANALYZE));
            OVERVIEW_CHECK(gd_explorer_presentation_set(c,"layout","free",p->query_message,sizeof(p->query_message)));
        }
        if(scenario==2)OVERVIEW_CHECK(gd_explorer_query_overview(c,GD_ACTOR_USAGE));
        if(scenario==3 || scenario==5 || scenario==7)OVERVIEW_CHECK(gd_explorer_presentation_set(c,"layout","free",p->query_message,sizeof(p->query_message)));
        if(scenario==4) {
            const GD_Rules *rules=c->project->rules;int root,edge,found=0;
            p->query_options=p->query_state->applied;
            for(root=0;root<rules->case_count && !found;++root) {
                int actor=rules->cases[root].primary_participant;
                if(actor<0 || rules->participants[actor].noun_resolution!=GD_RULE_RESOLVED)continue;
                for(edge=rules->actor_first[rules->participants[actor].noun_record];edge>=0;edge=rules->next_axis[edge])if(rules->axis_resolution[edge]==GD_RULE_RESOLVED) {
                    int axis=rules->model->edges[edge].target;
                    if(rules->axes[axis].valid && rules->axes[axis].type==GD_AXIS_ENUM && rules->model->records[axis].axis.allowed_count>1) {
                        p->query_options.root_case=root;p->query_options.participant=actor;p->query_options.axis=axis;found=1;break;
                    }
                }
            }
            p->query_options.ghosts=GD_QUERY_STACKED_VARIANTS;
            OVERVIEW_CHECK(found && gd_explorer_query_run(c));
            OVERVIEW_CHECK(gd_explorer_presentation_set(c,"layout","native",p->query_message,sizeof(p->query_message)));
        }
        OVERVIEW_CHECK(overview_available(c) && gd_explorer_presentation_editable(c,"overview_enabled"));
        OVERVIEW_CHECK(!gd_explorer_presentation_editable(c,"shared") && !gd_explorer_presentation_editable(c,"paths_enabled") && !gd_explorer_presentation_editable(c,"shapes"));
        query=p->query_state ? p->query_state->query:NULL;scene=p->query_state ? p->query_state->scene:NULL;
        pending=p->query_options;if(p->query_state)applied=p->query_state->applied;
        snprintf(p->query_options.values,sizeof(p->query_options.values),"Pending overview input, never Run");
        memcpy(saved,p->layout,sizeof(*saved));camera=p->camera;allocations=*calls;
        OVERVIEW_CHECK(gd_explorer_action_available(c,GD_ACTION_VIEW_ANALYZE) && gd_explorer_action_run(c,GD_ACTION_VIEW_ANALYZE));
        overview=evidence(c,-1).overview;
        OVERVIEW_CHECK(overview && overview->valid && overview->undirected && overview->link_count==p->layout->edge_count && overview->node_count==p->layout->node_count);
        OVERVIEW_CHECK(c->analysis->overview_only && c->analysis->complete && !c->analysis->work && !c->analysis->pair_count && !c->analysis->feature_count && !c->analysis->paths.valid && !c->analysis->shapes.valid);
        OVERVIEW_CHECK(*calls==allocations && !memcmp(saved,p->layout,sizeof(*saved)) && !memcmp(&camera,&p->camera,sizeof(camera)));
        for(i=0;i<overview->bins*overview->bins;++i)total+=overview->counts[i];
        OVERVIEW_CHECK(total>=overview->link_count && total<=2*overview->link_count);
        if(view==GD_EXPLORER_NOUNS)OVERVIEW_CHECK(!strcmp(overview_metric(p),"Noun-pair links") && overview->link_count==c->project->noun_grid->pair_count);
        OVERVIEW_CHECK(!p->query_state || (p->query_state->query==query && p->query_state->scene==scene && !memcmp(&applied,&p->query_state->applied,sizeof(applied))));
        viewer.context=c;viewer.panes=c->panes;viewer.active=c->active_view;viewer_record_layers(&viewer,c);OVERVIEW_CHECK(viewer_layers_current(&viewer,c));
        strcpy(request.form,"layers");strcpy(request.fields,"overview");OVERVIEW_CHECK(viewer_result_fields(c,&request));
        {char reply[4096],decoded[1025];GD_ViewerWriter writer={reply,0,sizeof(reply),0};
            strcpy(request.command,"result");strcpy(request.projection,viewer_projections[view]);request.limit=1;viewer_dispatch(&viewer,&request,&writer);
            OVERVIEW_CHECK(!writer.failed && presentation_decode(reply,decoded,sizeof(decoded))>0 && strstr(decoded,overview_metric(p)) && strstr(decoded,"\"undirected\":true"));
            strcpy(request.fields,"shared");OVERVIEW_CHECK(!viewer_result_fields(c,&request));writer=(GD_ViewerWriter){reply,0,sizeof(reply),0};viewer_dispatch(&viewer,&request,&writer);
            OVERVIEW_CHECK(!writer.failed && strstr(reply,"viewer.result_fields"));
        }
        if(overview->link_count) {
        for(cell=0;cell<overview->bins*overview->bins && !overview->counts[cell];++cell){}
        OVERVIEW_CHECK(cell<overview->bins*overview->bins);
        p->overview.row=cell/overview->bins;p->overview.column=cell%overview->bins;overview=evidence(c,-1).overview;
        for(i=0;i<p->layout->node_count;++i)marked+=overview->selected_nodes[i]!=0;
        OVERVIEW_CHECK(marked>0 && overview->selected_count==overview->counts[cell]);
        p->selection=(GD_ExplorerSelection){-1,-1,0,-1,-1};p->grid.noun=p->grid.verb=-1;p->overview.enabled=0;
        OVERVIEW_CHECK(evidence(c,-1).overview==overview && gd_explorer_focus_available(c));
        OVERVIEW_CHECK(gd_explorer_action_run(c,GD_ACTION_VIEW_FIT) && p->focus_result.selected==marked && !p->overview.enabled);
        for(i=0;i<p->layout->node_count && !overview->selected_nodes[i];++i){}
        p->selection.node=i;OVERVIEW_CHECK(gd_graph_overview_node_bin(c->analysis,i)>=0 && overview->selected_count>0 && !p->overview.enabled);
        }else {
            OVERVIEW_CHECK(!total && !overview->selected_count && overview->node_count==p->layout->node_count);
            p->overview.enabled=0;
            if(p->layout->node_count){p->selection.node=0;OVERVIEW_CHECK(gd_graph_overview_node_bin(c->analysis,0)>=0);}
        }
        camera=p->camera;
        OVERVIEW_CHECK(gd_explorer_action_run(c,GD_ACTION_SELECTION_CLEAR) && !gd_explorer_focus_available(c) && !p->overview.enabled);
        OVERVIEW_CHECK(!memcmp(&camera,&p->camera,sizeof(camera)) && !memcmp(saved,p->layout,sizeof(*saved)) && !strcmp(p->query_options.values,"Pending overview input, never Run"));
        if(query_view(p->view)) {
            if(p->layout->edge_count) {
                int edge=p->layout->edge_count-1;const GD_LayoutEdge *item=&p->layout->edges[edge];
                p->selection=(GD_ExplorerSelection){edge,-1,0,-1,-1};
                OVERVIEW_CHECK(gd_explorer_action_run(c,GD_ACTION_VIEW_FIT) && p->focus_result.selected==(item->a==item->b ? 1:2));
                OVERVIEW_CHECK(gd_explorer_action_run(c,GD_ACTION_SELECTION_CLEAR));
            }
            p->query_options=pending;
            OVERVIEW_CHECK(gd_explorer_action_run(c,GD_ACTION_VIEW_ANALYZE));
            p->overview.row=p->overview.column=0;
            *fail_at=*calls+1;OVERVIEW_CHECK(!gd_explorer_query_display(c));*fail_at=0;
            OVERVIEW_CHECK(p->query_state->scene==scene && p->overview.row==0 && p->overview.column==0 && c->analysis->initialized);
            OVERVIEW_CHECK(gd_explorer_query_display(c) && p->overview.row<0 && p->overview.column<0 && !p->overview.enabled && !c->analysis->initialized);
        }else {
            p->query_options=pending;
            OVERVIEW_CHECK(gd_explorer_presentation_set(c,"layout","grid",p->query_message,sizeof(p->query_message)) && !overview_available(c) && !p->overview.enabled);
            OVERVIEW_CHECK(gd_explorer_presentation_set(c,"layout","free",p->query_message,sizeof(p->query_message)) && overview_available(c) && !p->overview.enabled);
        }
        p->overview.enabled=1;
    }
    fprintf(stderr,"Overview coverage: %d checks, 8 noun/Actor/Effect scenarios\n",checks);
#undef OVERVIEW_CHECK
    return 1;
}
#endif
