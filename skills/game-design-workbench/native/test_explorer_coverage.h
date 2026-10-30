/* CJIT app-local checks; native coverage stays independent of variant canvas. */
#define COVERAGE_CHECK(expression) do { if (!(expression)) { fprintf(stderr,"Coverage UI check failed at line %d: %s\n",__LINE__,#expression); goto cleanup; } } while (0)
static int explorer_test_coverage(GD_ExplorerContext *c, GD_ExplorerPane *panes, const GD_Collection *collections, int count, const char *output, int *fail_at, int *calls) {
    GD_Allocator allocator=c->project->allocator; GD_ExplorerProject *fresh=NULL; GD_ExplorerPane *candidate=NULL; GD_ExplorerContext replacement=*c;
    GD_ExplorerPane *pane=&panes[GD_EXPLORER_ACTORS]; GD_ExplorerQueryState *state; GD_Query *query;
    GD_Model *model; int axis=-1,i,edge,before,steps,errors,ok=0; char path[GD_MAX_PATH],failure[512];
    c->pane=pane; COVERAGE_CHECK(reset_view(c)); model=gd_explorer_model(c->project,GD_EXPLORER_ACTORS);
    for (i=0;i<c->project->rules->participant_count && axis<0;++i) {
        const GD_RuleParticipant *participant=&c->project->rules->participants[i]; if (participant->noun_resolution!=GD_RULE_RESOLVED) continue;
        for (edge=c->project->rules->actor_first[participant->noun_record];edge>=0;edge=c->project->rules->next_axis[edge]) if (c->project->rules->axis_resolution[edge]==GD_RULE_RESOLVED) {
            int key=model->edges[edge].target; if (c->project->rules->axes[key].valid && c->project->rules->axes[key].type==GD_AXIS_ENUM && model->records[key].axis.allowed_count) { axis=key; break; }
        }
    }
    COVERAGE_CHECK(axis>=0); pane->query_options.coverage=true; pane->query_options.axis=axis; before=*calls;
    COVERAGE_CHECK(gd_explorer_query_run(c)); steps=*calls-before; state=pane->query_state; query=state->query; errors=model->error_count;
    COVERAGE_CHECK(query->request.kind==GD_QUERY_STATE_COVERAGE && query->state_coverage && query->binding_count>0 && query->request.case_index<0 && query->request.noun<0 && !query->request.participant && !query->variants);
    COVERAGE_CHECK(state->scene && !state->scene->layout.edge_count && state->scene->layout.node_count==query->state_coverage->value_count && state->applied.compare<0);
    for (i=1;i<=4;++i) { *fail_at=*calls+i; COVERAGE_CHECK(!gd_explorer_query_run(c)); *fail_at=0; COVERAGE_CHECK(pane->query_state==state && pane->layout==&state->scene->layout && model->error_count==errors); }
    *fail_at=*calls+steps; COVERAGE_CHECK(!gd_explorer_query_run(c)); *fail_at=0; COVERAGE_CHECK(pane->query_state==state && model->error_count==errors);
    for (i=1;i<=3;++i) { *fail_at=*calls+i; COVERAGE_CHECK(!reset_view(c)); *fail_at=0; COVERAGE_CHECK(pane->query_state==state); }
    snprintf(pane->query_options.values,sizeof(pane->query_options.values),"%s;%s",query->state_coverage->values[0].value,query->state_coverage->values[0].value);
    COVERAGE_CHECK(!gd_explorer_query_run(c) && pane->query_state==state && model->error_count==errors); pane->query_options=state->applied;
    COVERAGE_CHECK(gd_explorer_query_run(c)); state=pane->query_state; query=state->query;
    pane->selection.node=0; explorer_test_render(c); COVERAGE_CHECK(snprintf(path,sizeof(path),"%s/state-coverage.png",output)<(int)sizeof(path) && gd_save_screenshot(path));
    /* Optional exact filters preserve local identity and never infer A. */
    pane->query_options.root_case=query->bindings[0].case_index; pane->query_options.participant=query->bindings[0].participant;
    pane->query_options.noun=c->project->rules->participants[query->bindings[0].participant].noun_record;
    COVERAGE_CHECK(gd_explorer_query_run(c)); state=pane->query_state; query=state->query;
    COVERAGE_CHECK(query->binding_count==1 && query->bindings[0].participant==state->applied.participant && query->request.noun==state->applied.noun && query->request.participant);
    fresh=gd_explorer_load(allocator,collections,count); candidate=allocator.allocate(allocator.context,sizeof(*candidate)); if (candidate) memset(candidate,0,sizeof(*candidate)); COVERAGE_CHECK(fresh && candidate);
    pane_defaults(candidate,GD_EXPLORER_ACTORS); replacement.project=fresh; replacement.pane=candidate;
    {
        GD_Model *destination=gd_explorer_model(fresh,GD_EXPLORER_ACTORS); int mapped=gd_explorer_record(destination,model->records[axis].reference),noun=gd_explorer_record(destination,model->records[state->applied.noun].reference); const char *identity;
        COVERAGE_CHECK(mapped>=0 && noun>=0); identity=destination->records[mapped].id; destination->records[mapped].id="coverage-replacement-axis";
        COVERAGE_CHECK(!gd_explorer_query_remap(&replacement,c->project,pane) && !candidate->query_state && pane->query_state==state); destination->records[mapped].id=identity;
        destination->records[noun].id="coverage-replacement-object"; COVERAGE_CHECK(!gd_explorer_query_remap(&replacement,c->project,pane) && !candidate->query_state && pane->query_state==state);
    }
    pane_release_layout(allocator,candidate); allocator.release(allocator.context,candidate); candidate=NULL; gd_explorer_destroy(fresh); fresh=NULL;
    snprintf(pane->query_options.values,sizeof(pane->query_options.values),"%s",query->state_coverage->values[0].value);
    { GD_ExplorerProject *before_project=c->project; COVERAGE_CHECK(!reload_project(c,panes,collections,count,failure,sizeof(failure)) && c->project==before_project && pane->query_state==state); }
    pane->query_options=state->applied; COVERAGE_CHECK(reload_project(c,panes,collections,count,failure,sizeof(failure)));
    c->pane=pane; COVERAGE_CHECK(pane->query_state->query->state_coverage && pane->query_state->query->binding_count==1 && pane->selection.node<0);
    COVERAGE_CHECK(reset_view(c) && !pane->query_state->query && !pane->query_options.coverage && pane->query_options.noun<0); ok=1;
cleanup:
    *fail_at=0;
    if (candidate) { pane_release_layout(allocator,candidate); gd_diagnostics_clear(&candidate->diagnostics); allocator.release(allocator.context,candidate); }
    gd_explorer_destroy(fresh); return ok;
}
#undef COVERAGE_CHECK
