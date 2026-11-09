/* CPU-only native-ID/catalogue/source checks. Called after ordinary panes prepare. */
#ifndef TEST_EXPLORER_VIEWER_NODES_H
#define TEST_EXPLORER_VIEWER_NODES_H
static int explorer_viewer_nodes_checks(ExplorerViewer *viewer, GD_ExplorerPane *before, char *reply, int *fail_at, int *calls, int *checks) {
    GD_ExplorerContext *c=viewer->context; GD_ExplorerPane *panes=viewer->panes;
    GD_ViewerRequest request={0}; GD_ViewerWriter w; ViewerIdentities ids;
    char identity[GD_MAX_PATH],selected[GD_MAX_PATH],old_actor[GD_MAX_PATH]="__no_classified_actor__";
    int v,node,i,ok=0;
#define NODE_CHECK(value) do { ++*checks; if (!(value)) { fprintf(stderr,"Viewer native-ID check failed: %s (line %d)\n",#value,__LINE__); goto cleanup; } } while (0)
#define REQUEST(command_,view_,id_) do { memset(&request,0,sizeof(request)); snprintf(request.command,sizeof(request.command),"%s",command_); snprintf(request.projection,sizeof(request.projection),"%s",view_); snprintf(request.id,sizeof(request.id),"%s",id_); w=(GD_ViewerWriter){reply,0,GD_VIEWER_REPLY_LIMIT,0}; viewer_dispatch(viewer,&request,&w); } while (0)
    for (v=0;v<GD_EXPLORER_VIEWS;++v) if (c->project->available&(1u<<v)) {
        GD_ExplorerContext target=*c; target.pane=&panes[v]; viewer_identities(&target,&ids); node=-1;
        for (i=0;i<viewer_canvas_count(&target);++i) if (viewer_canvas_visible(&target,i) && viewer_canvas_id(&target,i,&ids,identity,sizeof(identity))) { node=i; break; }
        if(v==GD_EXPLORER_ACTORS && !viewer_canvas_count(&target)) continue; /* No noun is implicitly an actor. */
        NODE_CHECK(node>=0);
        if (v==GD_EXPLORER_ACTORS) snprintf(old_actor,sizeof(old_actor),"%s",identity);
        memcpy(before,panes,sizeof(*panes)*GD_EXPLORER_VIEWS);
        if (c->project->draft) snprintf(c->project->draft->message,sizeof(c->project->draft->message),"Retained human diagnostic");
        REQUEST("navigate",viewer_projections[v],identity); NODE_CHECK(!w.failed && !strncmp(reply,"{\"ok\":true",10));
        NODE_CHECK(*viewer->active==v && viewer_selected_node(c)>=0);
        NODE_CHECK(c->pane->camera.offset.x==400 && c->pane->camera.offset.y==300 && c->pane->camera.zoom>0 && !c->pane->fit);
        if (!grid_view(c->pane)) {
            Vector2 center=position(c->pane,viewer_selected_node(c));
            NODE_CHECK(c->pane->camera.target.x==center.x && c->pane->camera.target.y==center.y);
        }
        viewer_identities(c,&ids); NODE_CHECK(viewer_canvas_id(c,viewer_selected_node(c),&ids,selected,sizeof(selected)) && !strcmp(identity,selected));
        for (i=0;i<GD_EXPLORER_VIEWS;++i) if (i!=v) NODE_CHECK(!memcmp(&before[i],&panes[i],sizeof(*panes)));
        if (c->project->draft) NODE_CHECK(!strcmp(c->project->draft->message,"Retained human diagnostic") && !c->project->draft->modified_count && !c->project->draft->history_count);
    }
    /* Read operations never allocate/prepare/apply, even with pending controls. */
    memcpy(before,panes,sizeof(*panes)*GD_EXPLORER_VIEWS); i=*calls;
    REQUEST("nodes","matrix",""); NODE_CHECK(!w.failed && strstr(reply,"\"total\":") && strstr(reply,"\"nodes\":[{"));
    NODE_CHECK(*calls==i && !memcmp(before,panes,sizeof(*panes)*GD_EXPLORER_VIEWS));
    request.offset=GD_MAX_RECORDS; request.limit=1; w=(GD_ViewerWriter){reply,0,GD_VIEWER_REPLY_LIMIT,0}; viewer_dispatch(viewer,&request,&w);
    NODE_CHECK(strstr(reply,"\"nodes\":[],\"next_offset\":null")!=NULL);
    REQUEST("navigate","matrix","__missing_native_node__"); NODE_CHECK(strstr(reply,"viewer.node_missing")!=NULL);
    NODE_CHECK(!memcmp(before,panes,sizeof(*panes)*GD_EXPLORER_VIEWS));
    {
        GD_ExplorerContext target=*c; GD_LayoutNode saved; unsigned revision;
        target.pane=&panes[GD_EXPLORER_MATRIX]; viewer_identities(&target,&ids); NODE_CHECK(viewer_canvas_id(&target,0,&ids,identity,sizeof(identity)));
        saved=target.pane->layout->nodes[1]; target.pane->layout->nodes[1]=target.pane->layout->nodes[0];
        REQUEST("navigate","matrix",identity); NODE_CHECK(strstr(reply,"viewer.ambiguous_id")!=NULL); target.pane->layout->nodes[1]=saved;
        revision=target.pane->revision; target.pane->revision=0;
        REQUEST("navigate","matrix",identity); NODE_CHECK(strstr(reply,"viewer.stale_canvas")!=NULL); target.pane->revision=revision;
        igGetIO_Nil()->WantTextInput=true; REQUEST("navigate","matrix",identity); NODE_CHECK(strstr(reply,"viewer.busy")!=NULL); igGetIO_Nil()->WantTextInput=false;
        REQUEST("hint","matrix",identity); NODE_CHECK(viewer->hint_pending && !strcmp(viewer->pending_hint.id,identity)); viewer_apply_hint(viewer);
        NODE_CHECK(viewer_hint_current(viewer)); ++viewer->reload_generation; NODE_CHECK(!viewer_hint_current(viewer)); --viewer->reload_generation;
        REQUEST("navigate","matrix",identity); NODE_CHECK(!strncmp(reply,"{\"ok\":true",10));
        memcpy(before,panes,sizeof(*panes)*GD_EXPLORER_VIEWS); i=*calls;
        w=(GD_ViewerWriter){reply,0,GD_VIEWER_REPLY_LIMIT,0}; viewer_inspect(viewer,&w,"selection,source");
        NODE_CHECK(!w.failed && strstr(reply,"\"body\":") && strstr(reply,"loaded Markdown body"));
        NODE_CHECK(*calls==i && !memcmp(before,panes,sizeof(*panes)*GD_EXPLORER_VIEWS));
        {
            GD_Model *model=gd_explorer_model(c->project,c->pane->view); int record=selected_record(c),failed;
            const char *saved_body=model->records[record].body; char *large=c->project->allocator.allocate(c->project->allocator.context,GD_VIEWER_REPLY_LIMIT);
            NODE_CHECK(large!=NULL); memset(large,'x',GD_VIEWER_REPLY_LIMIT-1); large[GD_VIEWER_REPLY_LIMIT-1]=0; model->records[record].body=large;
            w=(GD_ViewerWriter){reply,0,GD_VIEWER_REPLY_LIMIT,0}; viewer_inspect(viewer,&w,"source"); failed=w.failed;
            model->records[record].body=saved_body; c->project->allocator.release(c->project->allocator.context,large);
            NODE_CHECK(failed); /* Transport converts writer overflow to capacity error, never partial success. */
        }
    }
    {
        GD_ExplorerContext target=*c; int kind; bool shown;
        target.pane=&panes[GD_EXPLORER_RESOURCES]; viewer_identities(&target,&ids); NODE_CHECK(viewer_canvas_id(&target,0,&ids,identity,sizeof(identity)));
        kind=target.project->resources->nodes[target.pane->layout->nodes[0].key].kind; shown=target.pane->mda.roles[kind]; target.pane->mda.roles[kind]=false;
        memcpy(before,panes,sizeof(*panes)*GD_EXPLORER_VIEWS);
        REQUEST("navigate","resources",identity); NODE_CHECK(strstr(reply,"viewer.node_hidden")!=NULL);
        NODE_CHECK(!memcmp(before,panes,sizeof(*panes)*GD_EXPLORER_VIEWS)); target.pane->mda.roles[kind]=shown;
        for (kind=GD_RESOURCE_FACET;kind<=GD_RESOURCE_ROLE;++kind) {
            viewer_identities(&target,&ids); node=-1;
            for (i=0;i<viewer_canvas_count(&target);++i) if (target.project->resources->nodes[target.pane->layout->nodes[i].key].kind==kind && viewer_canvas_id(&target,i,&ids,identity,sizeof(identity))) { node=i; break; }
            NODE_CHECK(node>=0); REQUEST("navigate","resources",identity); NODE_CHECK(!strncmp(reply,"{\"ok\":true",10) && viewer_selected_node(c)==node);
        }
    }
    {
        GD_ExplorerContext target=*c; GD_ExplorerQueryState *state; GD_QueryScene *scene; GD_ExplorerSelection selection; int prior=*viewer->active,started,stages;
        target.pane=&panes[GD_EXPLORER_EFFECTS]; state=target.pane->query_state; viewer_identities(&target,&ids); node=-1;
        for (i=0;i<viewer_canvas_count(&target);++i) if (state->scene->nodes[i].kind==GD_QUERY_NODE_EFFECT && viewer_canvas_id(&target,i,&ids,identity,sizeof(identity))) { node=i; break; }
        NODE_CHECK(node>=0); scene=state->scene; selection=target.pane->selection;
        *fail_at=*calls+1; REQUEST("navigate","effects",identity); *fail_at=0;
        NODE_CHECK(strstr(reply,"viewer.select") && state->scene==scene && !memcmp(&selection,&target.pane->selection,sizeof(selection)) && *viewer->active==prior);
        snprintf(target.pane->query_options.facts,sizeof(target.pane->query_options.facts),"pending literal, not applied");
        started=*calls; REQUEST("navigate","effects",identity); stages=*calls-started;
        NODE_CHECK(!strncmp(reply,"{\"ok\":true",10) && stages>0 && stages<=32);
        scene=state->scene; selection=target.pane->selection;
        for (i=1;i<=stages;++i) {
            *fail_at=*calls+i; REQUEST("navigate","effects",identity); *fail_at=0;
            NODE_CHECK(strstr(reply,"viewer.select") && state->scene==scene && !memcmp(&selection,&target.pane->selection,sizeof(selection)) && *viewer->active==GD_EXPLORER_EFFECTS);
        }
        NODE_CHECK(!strcmp(target.pane->query_options.facts,"pending literal, not applied") && strcmp(state->applied.facts,target.pane->query_options.facts));
        viewer_identities(c,&ids); NODE_CHECK(viewer_canvas_id(c,viewer_selected_node(c),&ids,selected,sizeof(selected)) && !strcmp(identity,selected));
    }
    {
        const GD_Rules *rules=c->project->rules; GD_ExplorerPane *pane=&panes[GD_EXPLORER_ACTORS]; int root,edge,found=0;
        c->pane=pane; *viewer->active=GD_EXPLORER_ACTORS; pane->query_options=pane->query_state->applied;
        for (root=0;root<rules->case_count && !found;++root) {
            int actor=rules->cases[root].primary_participant;
            if (actor<0 || rules->participants[actor].noun_resolution!=GD_RULE_RESOLVED) continue;
            for (edge=rules->actor_first[rules->participants[actor].noun_record];edge>=0;edge=rules->next_axis[edge]) if (rules->axis_resolution[edge]==GD_RULE_RESOLVED) {
                int axis=rules->model->edges[edge].target;
                if (rules->axes[axis].valid && rules->axes[axis].type==GD_AXIS_ENUM && rules->model->records[axis].axis.allowed_count>1) {
                    pane->query_options.root_case=root; pane->query_options.participant=actor; pane->query_options.axis=axis; found=1; break;
                }
            }
        }
        NODE_CHECK(found && gd_explorer_query_run(c)); viewer_identities(c,&ids); node=-1;
        for (i=0;i<viewer_canvas_count(c);++i) if (pane->query_state->scene->nodes[i].kind==GD_QUERY_NODE_VARIANT && viewer_canvas_id(c,i,&ids,identity,sizeof(identity))) { node=i; break; }
        NODE_CHECK(node>=0); ids.work=GD_ACTOR_WORK;
        NODE_CHECK(!viewer_canvas_id(c,node,&ids,selected,sizeof(selected)) && ids.limited);
        snprintf(pane->query_options.values,sizeof(pane->query_options.values),"unapplied Ω");
        REQUEST("navigate","actors",identity); NODE_CHECK(!strncmp(reply,"{\"ok\":true",10) && viewer_selected_node(c)==node);
        NODE_CHECK(!strcmp(pane->query_options.values,"unapplied Ω") && strcmp(pane->query_state->applied.values,pane->query_options.values));
        REQUEST("navigate","actors",old_actor); NODE_CHECK(strstr(reply,"viewer.node_missing")!=NULL);
        pane->query_options=pane->query_state->applied; pane->query_options.coverage=true; NODE_CHECK(gd_explorer_query_run(c));
        viewer_identities(c,&ids); NODE_CHECK(viewer_canvas_count(c)>0);
        NODE_CHECK(!viewer_canvas_id(c,0,&ids,selected,sizeof(selected)) && !*selected);
        REQUEST("nodes","actors",""); NODE_CHECK(strstr(reply,"\"identity_available\":false")!=NULL);
        snprintf(pane->query_options.values,sizeof(pane->query_options.values),"unapplied Ω");
        {GD_ExplorerQueryState *last=pane->query_state;int retained=gd_explorer_query_record(pane);
         *fail_at=*calls+1;found=gd_explorer_query_overview(c,GD_ACTOR_USAGE);*fail_at=0;
         NODE_CHECK(!found && pane->query_state==last && !strcmp(pane->query_options.values,"unapplied Ω"));
         NODE_CHECK(gd_explorer_query_overview(c,GD_ACTOR_USAGE) && pane->query_state!=last && pane->query_state->scene->overview);
         NODE_CHECK(pane->query_state->query->request.kind==GD_QUERY_BINDINGS && !strcmp(pane->query_options.values,"unapplied Ω") && !*pane->query_state->applied.values);
         if(retained>=0) NODE_CHECK(gd_explorer_query_record(pane)==retained);}
    }
    {
        GD_ExplorerPane *pane=&panes[GD_EXPLORER_MDA]; GD_ExplorerStructureState *state; Camera2D camera=pane->camera;
        c->pane=pane; *viewer->active=GD_EXPLORER_MDA;
        NODE_CHECK(gd_explorer_action_run(c,GD_ACTION_STRUCTURE_RUN)); state=pane->structure_state; state->pending.kind=GD_STRUCTURE_COUNT+1;
        snprintf(state->pending.features,sizeof(state->pending.features),"retained-partners"); NODE_CHECK(gd_explorer_action_run(c,GD_ACTION_STRUCTURE_RUN)); state=pane->structure_state;
        NODE_CHECK(gd_explorer_similarity_active(c) && viewer_canvas_count(c)>0 && state->result->similarity->map->available);
        viewer_identities(c,&ids); NODE_CHECK(viewer_canvas_id(c,0,&ids,identity,sizeof(identity)));
        REQUEST("navigate","mda",identity); NODE_CHECK(!strncmp(reply,"{\"ok\":true",10) && state->similarity_row==0);
        NODE_CHECK(!memcmp(&camera,&pane->camera,sizeof(camera)) && state->similarity_camera_ready && !state->similarity_fit);
        state->result->similarity->map->placed[0]=0;
        REQUEST("navigate","mda",identity); NODE_CHECK(strstr(reply,"viewer.node_hidden")!=NULL); state->result->similarity->map->placed[0]=1;
        state->similarity_canvas=false;
    }
    c->pane=&panes[GD_EXPLORER_MATRIX]; *viewer->active=GD_EXPLORER_MATRIX; ok=1;
cleanup:
    *fail_at=0;
#undef NODE_CHECK
#undef REQUEST
    return ok;
}
#endif
