/* App-local named-question state/rollback checks. No OS input or source writes. */
#define STRUCTURE_CHECK(expression) do { if (!(expression)) { fprintf(stderr,"Structural UI check failed at line %d: %s\n",__LINE__,#expression); goto cleanup; } } while (0)
static int explorer_test_structure(GD_ExplorerContext *c, GD_ExplorerPane *panes, const GD_Collection *collections, int count, const char *output, int *fail_at, int *calls) {
    GD_Allocator allocator=c->project->allocator; GD_ExplorerProject *fresh=NULL; GD_ExplorerPane *candidate=NULL;
    GD_ExplorerContext replacement=*c; GD_ExplorerStructureForm initial; char from[GD_MAX_PATH],to[GD_MAX_PATH],path[GD_MAX_PATH],failure[512]; int v,i,ok=0;
    for (v=GD_EXPLORER_SKILLS;v<=GD_EXPLORER_RESOURCES;++v) {
        GD_ExplorerStructureState *state; int errors,stages=v==GD_EXPLORER_RESOURCES ? 6:5;
        c->pane=&panes[v]; STRUCTURE_CHECK(pane_prepare(c) && gd_explorer_structure_run(c)); state=c->pane->structure_state;
        STRUCTURE_CHECK(state && state->has_applied && state->result->project==c->project && state->result->view==(GD_ExplorerView)v && !gd_explorer_structure_pending(c->pane));
        errors=gd_explorer_model(c->project,(GD_ExplorerView)v)->error_count;
        snprintf(state->pending.from,sizeof(state->pending.from),"missing/selector");
        STRUCTURE_CHECK(!gd_explorer_structure_run(c) && c->pane->structure_state==state && gd_explorer_model(c->project,(GD_ExplorerView)v)->error_count==errors && gd_explorer_structure_pending(c->pane)); state->pending=state->applied;
        for (i=1;i<=stages;++i) {
            *fail_at=*calls+i; STRUCTURE_CHECK(!gd_explorer_structure_run(c)); *fail_at=0;
            STRUCTURE_CHECK(c->pane->structure_state==state && state->result && gd_explorer_model(c->project,(GD_ExplorerView)v)->error_count==errors);
        }
        for (i=1;i<=2;++i) { *fail_at=*calls+i; STRUCTURE_CHECK(!reset_view(c)); *fail_at=0; STRUCTURE_CHECK(c->pane->structure_state==state && state->result); }
    }
    c->pane=&panes[GD_EXPLORER_SKILLS]; initial=c->pane->structure_state->pending;
    {
        const GD_StructureResult *result=c->pane->structure_state->result; const GD_LayoutEdge *edge;
        STRUCTURE_CHECK(result->layout->edge_count>0); edge=&result->layout->edges[0];
        snprintf(from,sizeof(from),"%s",gd_explorer_model(c->project,GD_EXPLORER_SKILLS)->records[result->layout->nodes[edge->a].key].reference);
        snprintf(to,sizeof(to),"%s",gd_explorer_model(c->project,GD_EXPLORER_SKILLS)->records[result->layout->nodes[edge->b].key].reference);
    }
    for (i=0;i<GD_STRUCTURE_COUNT;++i) {
        GD_ExplorerStructureForm *form=&c->pane->structure_state->pending; *form=initial; form->kind=i;
        if (i==GD_STRUCTURE_RELATED || i==GD_STRUCTURE_REACHABILITY || i==GD_STRUCTURE_DOMINATORS || i==GD_STRUCTURE_BYPASS) { snprintf(form->from,sizeof(form->from),"%s",from); snprintf(form->to,sizeof(form->to),"%s",to); }
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
    for (v=GD_EXPLORER_SKILLS;v<=GD_EXPLORER_RESOURCES;++v) {
        c->pane=&panes[v]; STRUCTURE_CHECK(c->pane->structure_state && c->pane->structure_state->result && c->pane->structure_state->result->project==c->project);
        STRUCTURE_CHECK(reset_view(c) && !c->pane->structure_state);
    }
    fprintf(stderr,"Structural question input state: %llu bytes per activated pane; all checks passed\n",(unsigned long long)sizeof(GD_ExplorerStructureState));
    ok=1;
cleanup:
    *fail_at=0;
    if (candidate) { pane_release_layout(allocator,candidate); gd_diagnostics_clear(&candidate->diagnostics); allocator.release(allocator.context,candidate); }
    gd_explorer_destroy(fresh); return ok;
}
#undef STRUCTURE_CHECK
