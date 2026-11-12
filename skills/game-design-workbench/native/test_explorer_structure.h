/* App-local named-question state/rollback checks. No OS input or source writes. */
#define STRUCTURE_CHECK(expression) do { if (!(expression)) { fprintf(stderr,"Structural UI check failed at line %d: %s\n",__LINE__,#expression); goto cleanup; } } while (0)
static int explorer_test_structure(GD_ExplorerContext *c, GD_ExplorerPane *panes, const GD_Collection *collections, int count, const char *output, int *fail_at, int *calls) {
    GD_Allocator allocator=c->project->allocator; GD_ExplorerProject *fresh=NULL; GD_ExplorerPane *candidate=NULL;
    GD_ExplorerContext replacement=*c; GD_ExplorerStructureForm initial; char from[GD_MAX_PATH],to[GD_MAX_PATH],path[GD_MAX_PATH],failure[512]; int v,i,ok=0;
    for (v=GD_EXPLORER_SKILLS;v<=GD_EXPLORER_ACTORS;++v) {
        GD_ExplorerStructureState *state; int errors,stages=(v==GD_EXPLORER_RESOURCES || v==GD_EXPLORER_ACTORS) ? 6:5;
        c->pane=&panes[v]; STRUCTURE_CHECK(pane_prepare(c) && gd_explorer_structure_run(c)); state=c->pane->structure_state;
        STRUCTURE_CHECK(state && state->has_applied && state->result->project==c->project && state->result->view==(GD_ExplorerView)v && !gd_explorer_structure_pending(c->pane));
        errors=gd_explorer_model(c->project,(GD_ExplorerView)v)->error_count;
        snprintf(state->pending.from,sizeof(state->pending.from),"missing/selector");
        STRUCTURE_CHECK(!gd_explorer_structure_run(c) && c->pane->structure_state==state && gd_explorer_model(c->project,(GD_ExplorerView)v)->error_count==errors && gd_explorer_structure_pending(c->pane)); state->pending=state->applied;
        for (i=1;i<=stages;++i) {
            *fail_at=*calls+i; STRUCTURE_CHECK(!gd_explorer_structure_run(c)); *fail_at=0;
            STRUCTURE_CHECK(c->pane->structure_state==state && state->result && gd_explorer_model(c->project,(GD_ExplorerView)v)->error_count==errors);
        }
        for (i=1;i<=2;++i) { *fail_at=*calls+i; STRUCTURE_CHECK(!explorer_test_fresh_pane(c)); *fail_at=0; STRUCTURE_CHECK(c->pane->structure_state==state && state->result); }
        state->pending.kind=GD_STRUCTURE_DECLARATIONS; STRUCTURE_CHECK(gd_explorer_structure_run(c)); state=c->pane->structure_state;
        STRUCTURE_CHECK(state->result->gaps && state->result->gaps->rows==gd_explorer_model(c->project,(GD_ExplorerView)v)->edge_count);
        *fail_at=*calls+stages+1; STRUCTURE_CHECK(!gd_explorer_structure_run(c)); *fail_at=0; STRUCTURE_CHECK(c->pane->structure_state==state && state->result->gaps && gd_explorer_model(c->project,(GD_ExplorerView)v)->error_count==errors);
    }
    c->pane=&panes[GD_EXPLORER_ACTORS]; initial=c->pane->structure_state->pending; initial.kind=GD_STRUCTURE_CONNECTIONS;
    {
        const GD_StructureResult *result=c->pane->structure_state->result; const GD_LayoutEdge *edge;
        STRUCTURE_CHECK(result->inventory && result->layout->edge_count>0); edge=&result->layout->edges[0];
        STRUCTURE_CHECK(gd_structure_result_node_id(result,result->layout->nodes[edge->a].key,from,sizeof(from)) && gd_structure_result_node_id(result,result->layout->nodes[edge->b].key,to,sizeof(to)));
    }
    for (i=0;i<GD_STRUCTURE_COUNT;++i) {
        GD_ExplorerStructureForm *form=&c->pane->structure_state->pending; *form=initial; form->kind=i;
        if (i==GD_STRUCTURE_RELATED || i==GD_STRUCTURE_RELATED_PATHS || i==GD_STRUCTURE_REACHABILITY || i==GD_STRUCTURE_DOMINATORS || i==GD_STRUCTURE_BYPASS) { snprintf(form->from,sizeof(form->from),"%s",from); snprintf(form->to,sizeof(form->to),"%s",to); }
        if (i==GD_STRUCTURE_BYPASS) snprintf(form->avoid,sizeof(form->avoid),"%s",from);
        STRUCTURE_CHECK(gd_explorer_structure_run(c) && c->pane->structure_state->result->inventory && c->pane->structure_state->result->query->request.kind==(GD_StructureKind)i);
    }
    c->pane->structure_state->pending=initial; snprintf(c->pane->structure_state->pending.from,sizeof(c->pane->structure_state->pending.from),"%s",to); STRUCTURE_CHECK(gd_explorer_structure_run(c));
    explorer_test_render(c); STRUCTURE_CHECK(snprintf(path,sizeof(path),"%s/actor-inventory-question.png",output)<(int)sizeof(path) && gd_save_screenshot(path));
    c->pane=&panes[GD_EXPLORER_SKILLS]; initial=c->pane->structure_state->pending;
    STRUCTURE_CHECK(gd_explorer_structure_run(c));
    explorer_test_render(c); STRUCTURE_CHECK(snprintf(path,sizeof(path),"%s/declaration-gaps.png",output)<(int)sizeof(path) && gd_save_screenshot(path)); initial.kind=GD_STRUCTURE_CONNECTIONS;
    {
        const GD_StructureResult *result=c->pane->structure_state->result; const GD_LayoutEdge *edge;
        STRUCTURE_CHECK(result->layout->edge_count>0); edge=&result->layout->edges[0];
        snprintf(from,sizeof(from),"%s",gd_explorer_model(c->project,GD_EXPLORER_SKILLS)->records[result->layout->nodes[edge->a].key].reference);
        snprintf(to,sizeof(to),"%s",gd_explorer_model(c->project,GD_EXPLORER_SKILLS)->records[result->layout->nodes[edge->b].key].reference);
    }
    for (i=0;i<GD_STRUCTURE_COUNT;++i) {
        GD_ExplorerStructureForm *form=&c->pane->structure_state->pending; *form=initial; form->kind=i;
        if (i==GD_STRUCTURE_RELATED || i==GD_STRUCTURE_RELATED_PATHS || i==GD_STRUCTURE_REACHABILITY || i==GD_STRUCTURE_DOMINATORS || i==GD_STRUCTURE_BYPASS) { snprintf(form->from,sizeof(form->from),"%s",from); snprintf(form->to,sizeof(form->to),"%s",to); }
        if (i==GD_STRUCTURE_BYPASS) snprintf(form->avoid,sizeof(form->avoid),"%s",from);
        STRUCTURE_CHECK(gd_explorer_structure_run(c) && c->pane->structure_state->result->query->request.kind==(GD_StructureKind)i);
        if (i==GD_STRUCTURE_BYPASS) STRUCTURE_CHECK(!c->pane->structure_state->result->query->available);
    }
    c->pane->structure_state->pending=initial; snprintf(c->pane->structure_state->pending.from,sizeof(c->pane->structure_state->pending.from),"%s",from);
    STRUCTURE_CHECK(gd_explorer_structure_run(c));
    explorer_test_render(c); STRUCTURE_CHECK(snprintf(path,sizeof(path),"%s/structural-question.png",output)<(int)sizeof(path) && gd_save_screenshot(path));
    snprintf(c->pane->structure_state->pending.from,sizeof(c->pane->structure_state->pending.from),"%s",to); STRUCTURE_CHECK(gd_explorer_structure_run(c));
    explorer_test_render(c); STRUCTURE_CHECK(snprintf(path,sizeof(path),"%s/structural-question-alternate.png",output)<(int)sizeof(path) && gd_save_screenshot(path));
    snprintf(c->pane->structure_state->pending.from,sizeof(c->pane->structure_state->pending.from),"%s",from); STRUCTURE_CHECK(gd_explorer_structure_run(c));
    /* Same reference with replacement ID must not silently retarget selection. */
    fresh=gd_explorer_load(allocator,collections,count); candidate=allocator.allocate(allocator.context,sizeof(*candidate)); STRUCTURE_CHECK(fresh && candidate);
    memset(candidate,0,sizeof(*candidate)); pane_defaults(candidate,GD_EXPLORER_SKILLS); replacement.project=fresh; replacement.pane=candidate;
    { GD_Model *model=gd_explorer_model(fresh,GD_EXPLORER_SKILLS); int record=gd_explorer_record(model,from); STRUCTURE_CHECK(record>=0); model->records[record].id="replacement-identity-test"; }
    STRUCTURE_CHECK(!gd_explorer_structure_remap(&replacement,c->project,c->pane) && !candidate->structure_state && c->pane->structure_state->result);
    {
        const GD_StructureResult *prior=panes[GD_EXPLORER_ACTORS].structure_state->result; int source=prior->query->request.source;
        const GD_ActorInventoryNode *binding; const GD_RuleParticipant *participant; GD_Model *model=gd_explorer_model(fresh,GD_EXPLORER_ACTORS); int noun;
        STRUCTURE_CHECK(source>=0); binding=&prior->inventory->nodes[prior->layout->nodes[source].key]; STRUCTURE_CHECK(binding->participant>=0);
        participant=&c->project->rules->participants[binding->participant]; STRUCTURE_CHECK(participant->noun_resolution==GD_RULE_RESOLVED);
        noun=gd_explorer_record(model,c->project->rules->model->records[participant->noun_record].reference); STRUCTURE_CHECK(noun>=0); model->records[noun].id="replacement-noun-identity-test";
        pane_defaults(candidate,GD_EXPLORER_ACTORS);
        STRUCTURE_CHECK(!gd_explorer_structure_remap(&replacement,c->project,&panes[GD_EXPLORER_ACTORS]) && !candidate->structure_state && panes[GD_EXPLORER_ACTORS].structure_state->result==prior);
        model->records[noun].id=c->project->rules->model->records[participant->noun_record].id;
        /* Synthetic native lost-resolution marker: same raw local identity,
           but old resolved noun must not become an unknown replacement. */
        for (i=0;i<fresh->rules->participant_count;++i) if (fresh->rules->participants[i].noun_record==noun) fresh->rules->participants[i].noun_resolution=GD_RULE_MISSING;
        STRUCTURE_CHECK(!gd_explorer_structure_remap(&replacement,c->project,&panes[GD_EXPLORER_ACTORS]) && !candidate->structure_state && panes[GD_EXPLORER_ACTORS].structure_state->result==prior);
    }
    pane_release_layout(allocator,candidate); allocator.release(allocator.context,candidate); candidate=NULL; gd_explorer_destroy(fresh); fresh=NULL;
    {
        GD_ExplorerProject *before=c->project; GD_ExplorerStructureState *state=c->pane->structure_state;
        snprintf(state->pending.from,sizeof(state->pending.from),"missing/pending");
        STRUCTURE_CHECK(!reload_project(c,panes,collections,count,failure,sizeof(failure)) && c->project==before && c->pane->structure_state==state);
        snprintf(state->pending.from,sizeof(state->pending.from),"%s",from);
        /* Inactive pane results survive an actual draft clone/undo before
           reload. Remapping must not read released draft-label storage. */
        { int record=gd_explorer_record(c->project->draft->model,from); STRUCTURE_CHECK(record>=0 && gd_draft_rank(c->project->draft,record,"987654")>0 && gd_explorer_edited(c->project) && undo_draft(c,panes)); }
        STRUCTURE_CHECK(reload_project(c,panes,collections,count,failure,sizeof(failure)) && c->project!=before);
    }
    for (v=GD_EXPLORER_SKILLS;v<=GD_EXPLORER_ACTORS;++v) {
        c->pane=&panes[v]; STRUCTURE_CHECK(c->pane->structure_state && c->pane->structure_state->result && c->pane->structure_state->result->project==c->project);
        STRUCTURE_CHECK(explorer_test_fresh_pane(c) && !c->pane->structure_state);
    }
    /* Finite scope borrows applied query, not its primary/ghost drawing. */
    c->pane=&panes[GD_EXPLORER_ACTORS];
    {
        GD_ExplorerPane *pane=c->pane; GD_ExplorerQueryState *query; GD_ExplorerStructureState *state; int root,edge,found=0,nodes,links;
        for (root=0;root<c->project->rules->case_count && !found;++root) {
            int actor=c->project->rules->cases[root].primary_participant;
            if (actor<0 || c->project->rules->participants[actor].noun_resolution!=GD_RULE_RESOLVED) continue;
            for (edge=c->project->rules->actor_first[c->project->rules->participants[actor].noun_record];edge>=0;edge=c->project->rules->next_axis[edge]) if (c->project->rules->axis_resolution[edge]==GD_RULE_RESOLVED) {
                int axis=gd_explorer_model(c->project,GD_EXPLORER_ACTORS)->edges[edge].target;
                if (c->project->rules->axes[axis].valid && c->project->rules->axes[axis].type==GD_AXIS_ENUM && gd_explorer_model(c->project,GD_EXPLORER_ACTORS)->records[axis].axis.allowed_count>1) { pane->query_options.root_case=root; pane->query_options.participant=actor; pane->query_options.axis=axis; found=1; break; }
            }
        }
        STRUCTURE_CHECK(found && gd_explorer_query_run(c)); query=pane->query_state;
        STRUCTURE_CHECK(gd_explorer_structure_run(c)); pane->structure_state->pending.actor_scope=1;
        STRUCTURE_CHECK(gd_explorer_structure_run(c)); state=pane->structure_state;
        STRUCTURE_CHECK(state->result->actor_scene && state->result->options.actor_query==query->query && !state->result->inventory);
        nodes=state->result->layout->node_count; links=state->result->layout->edge_count;
        STRUCTURE_CHECK(gd_structure_result_node_id(state->result,0,from,sizeof(from)) && gd_structure_result_node_id(state->result,1,to,sizeof(to)));
        initial=state->pending;
        for (i=0;i<GD_STRUCTURE_COUNT;++i) {
            GD_ExplorerStructureForm *form=&pane->structure_state->pending; *form=initial; form->kind=i;
            if (i==GD_STRUCTURE_RELATED || i==GD_STRUCTURE_RELATED_PATHS || i==GD_STRUCTURE_REACHABILITY || i==GD_STRUCTURE_DOMINATORS || i==GD_STRUCTURE_BYPASS) { snprintf(form->from,sizeof(form->from),"%s",from); snprintf(form->to,sizeof(form->to),"%s",to); }
            if (i==GD_STRUCTURE_BYPASS) snprintf(form->avoid,sizeof(form->avoid),"%s",from);
            STRUCTURE_CHECK(gd_explorer_structure_run(c) && pane->structure_state->result->query->request.kind==(GD_StructureKind)i);
            STRUCTURE_CHECK(i==GD_STRUCTURE_DECLARATIONS ? pane->structure_state->result->gaps!=NULL:pane->structure_state->result->actor_scene!=NULL);
        }
        pane->structure_state->pending=initial; STRUCTURE_CHECK(gd_explorer_structure_run(c)); state=pane->structure_state;
        for (i=1;i<=5;++i) { *fail_at=*calls+i; STRUCTURE_CHECK(!gd_explorer_structure_run(c)); *fail_at=0; STRUCTURE_CHECK(pane->structure_state==state && pane->query_state==query); }
        for (i=1;i<=9;++i) { *fail_at=*calls+i; STRUCTURE_CHECK(!gd_explorer_query_run(c)); *fail_at=0; STRUCTURE_CHECK(pane->structure_state==state && pane->query_state==query && *pane->query_message); }
        pane->query_options.ghosts=GD_QUERY_PRIMARY_VARIANT; pane->query_options.compare=-1; pane->query_options.include_excluded=false;
        STRUCTURE_CHECK(gd_explorer_query_run(c)); state=pane->structure_state; query=pane->query_state;
        STRUCTURE_CHECK(state->result->options.actor_query==query->query && state->result->actor_scene->ghosts==GD_QUERY_ALL_VARIANTS && state->result->actor_scene->include_excluded && state->result->layout->node_count==nodes && state->result->layout->edge_count==links && pane->layout->node_count<nodes);
        /* Coverage cannot silently replace the query borrowed by finite topology. */
        pane->query_options.coverage=true; STRUCTURE_CHECK(!gd_explorer_query_run(c) && pane->query_state==query && pane->structure_state==state); pane->query_options.coverage=false;
        STRUCTURE_CHECK(gd_explorer_structure_run(c)); explorer_test_render(c); STRUCTURE_CHECK(snprintf(path,sizeof(path),"%s/actor-state-question.png",output)<(int)sizeof(path) && gd_save_screenshot(path));
        STRUCTURE_CHECK(reload_project(c,panes,collections,count,failure,sizeof(failure))); pane=&panes[GD_EXPLORER_ACTORS]; c->pane=pane; state=pane->structure_state;
        STRUCTURE_CHECK(state && state->result->actor_scene && state->result->options.actor_query==pane->query_state->query && state->result->layout->node_count==nodes && state->result->layout->edge_count==links);
        /* Exact selected activation must not retarget after ancestor body edits. */
        for (i=0;i<state->result->actor_scene->layout.node_count;++i) if (state->result->actor_scene->nodes[i].kind==GD_QUERY_NODE_ACTIVATION) break;
        STRUCTURE_CHECK(i<state->result->actor_scene->layout.node_count && gd_structure_result_node_id(state->result,i,state->pending.from,sizeof(state->pending.from)) && gd_explorer_structure_run(c));
        fresh=gd_explorer_load(allocator,collections,count); candidate=allocator.allocate(allocator.context,sizeof(*candidate)); STRUCTURE_CHECK(fresh && candidate); memset(candidate,0,sizeof(*candidate)); pane_defaults(candidate,GD_EXPLORER_ACTORS); replacement.project=fresh; replacement.pane=candidate;
        STRUCTURE_CHECK(gd_explorer_query_remap(&replacement,c->project,pane));
        { GD_Query *native=candidate->query_state->query; int record=native->rules->sources[native->rules->cases[native->variants->case_index].source].record; fresh->models[GD_EXPLORER_RULE_DATA]->records[record].body="synthetic changed body; not a source write"; }
        STRUCTURE_CHECK(!gd_explorer_structure_remap(&replacement,c->project,pane) && !candidate->structure_state && pane->structure_state->result);
        pane_release_layout(allocator,candidate); allocator.release(allocator.context,candidate); candidate=NULL; gd_explorer_destroy(fresh); fresh=NULL;
        STRUCTURE_CHECK(explorer_test_fresh_pane(c) && !c->pane->structure_state);
    }
    fprintf(stderr,"Structural question input state: %llu bytes per activated pane; all checks passed\n",(unsigned long long)sizeof(GD_ExplorerStructureState));
    ok=1;
cleanup:
    *fail_at=0;
    if (candidate) { pane_release_layout(allocator,candidate); gd_diagnostics_clear(&candidate->diagnostics); allocator.release(allocator.context,candidate); }
    gd_explorer_destroy(fresh); return ok;
}
#undef STRUCTURE_CHECK
