/* Owned-window native question checks. No OS input or source-record writes. */
#define NATIVE_QUESTION_CHECK(expression) do { if (!(expression)) { fprintf(stderr,"Native question UI check failed at line %d: %s\n",__LINE__,#expression); goto cleanup; } } while (0)
static int explorer_test_native_questions(GD_ExplorerContext *c,GD_ExplorerPane *panes,const GD_Collection *collections,int count,const char *output,int *fail_at,int *calls) {
    char path[GD_MAX_PATH],failure[512]; int v,i,ok=0; GD_ExplorerPane *pane;
    for (v=0;v<=GD_EXPLORER_VIEWS;++v) {
        GD_ExplorerStructureState *state; GD_LayoutNode ordinary; Camera2D camera;
        c->pane=&panes[v==GD_EXPLORER_VIEWS ? GD_EXPLORER_ACTORS:v]; NATIVE_QUESTION_CHECK(explorer_test_fresh_pane(c) && pane_prepare(c)); pane=c->pane;
        if (v==GD_EXPLORER_VIEWS) {
            const GD_Rules *rules=c->project->rules; int root,edge,found=0;
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
            NATIVE_QUESTION_CHECK(found && gd_explorer_query_run(c));
        }
        NATIVE_QUESTION_CHECK(gd_explorer_structure_run(c)); state=pane->structure_state; state->pending.kind=GD_STRUCTURE_COUNT+1; state->pending.actor_scope=v==GD_EXPLORER_VIEWS;
        snprintf(state->pending.features,sizeof(state->pending.features),"retained-partners");
        NATIVE_QUESTION_CHECK(gd_explorer_structure_run(c)); state=pane->structure_state;
        NATIVE_QUESTION_CHECK(gd_explorer_similarity_active(c) && state->result->similarity && state->result->similarity->table_complete && state->result->similarity->map);
        if (pane->layout->node_count) ordinary=pane->layout->nodes[0]; camera=pane->camera;
        state->similarity_row=0; state->similarity_peer=state->result->similarity->entry_count>1 ? 1:0;
        /* Explicit test-only camera avoids consuming actual pointer/key input. */
        state->similarity_camera=(Camera2D){.offset={450,503},.zoom=0.7f}; state->similarity_camera_ready=true; state->similarity_fit=false;
        for (i=0;i<3;++i) explorer_test_render(c);
        NATIVE_QUESTION_CHECK((!pane->layout->node_count || !memcmp(&ordinary,&pane->layout->nodes[0],sizeof(ordinary))) && camera.target.x==pane->camera.target.x && camera.target.y==pane->camera.target.y && camera.zoom==pane->camera.zoom);
        NATIVE_QUESTION_CHECK(snprintf(path,sizeof(path),"%s/similarity-%d.png",output,v)<(int)sizeof(path) && gd_save_screenshot(path));
        snprintf(state->pending.features,sizeof(state->pending.features),"missing-feature"); NATIVE_QUESTION_CHECK(!gd_explorer_structure_run(c) && pane->structure_state==state); state->pending=state->applied;
    }
    NATIVE_QUESTION_CHECK(reload_project(c,panes,collections,count,failure,sizeof(failure)));
    for (v=0;v<GD_EXPLORER_VIEWS;++v) { c->pane=&panes[v]; NATIVE_QUESTION_CHECK(gd_explorer_similarity_active(c) && explorer_test_fresh_pane(c) && !c->pane->structure_state); }
    c->pane=&panes[GD_EXPLORER_RESOURCES]; pane=c->pane; NATIVE_QUESTION_CHECK(pane_prepare(c)); explorer_test_render(c);
    NATIVE_QUESTION_CHECK(pane->rule_state); pane->rule_state->pending.kind=GD_NAMED_CONSISTENCY; pane->rule_state->pending.work_limit=-1;
    NATIVE_QUESTION_CHECK(gd_explorer_rules_run(c) && pane->rule_state->result && !pane->rule_state->bindings);
    for (i=0;i<3;++i) explorer_test_render(c);
    NATIVE_QUESTION_CHECK(snprintf(path,sizeof(path),"%s/rule-consistency.png",output)<(int)sizeof(path) && gd_save_screenshot(path));
    {
        const GD_Rules *rules=c->project->rules; const GD_RuleCase *item=&rules->cases[0]; GD_ExplorerRuleState *state=pane->rule_state;
        state->pending.kind=GD_NAMED_OUTCOMES; snprintf(state->pending.case_name,sizeof(state->pending.case_name),"%s#%s",rules->model->records[rules->sources[item->source].record].reference,item->name);
        NATIVE_QUESTION_CHECK(gd_explorer_rules_run(c) && pane->rule_state->result->root==0 && gd_explorer_rules_marked(c,0));
        for (i=0;i<3;++i) explorer_test_render(c);
        NATIVE_QUESTION_CHECK(snprintf(path,sizeof(path),"%s/rule-outcomes.png",output)<(int)sizeof(path) && gd_save_screenshot(path));
    }
    pane->rule_state->pending.bindings=true; *pane->rule_state->pending.case_name=0; pane->rule_state->pending.work_limit=GD_EFFECT_WORK;
    NATIVE_QUESTION_CHECK(gd_explorer_rules_run(c) && pane->rule_state->bindings && !pane->rule_state->result);
    pane->rule_state->row=0; for (i=0;i<3;++i) explorer_test_render(c);
    NATIVE_QUESTION_CHECK(snprintf(path,sizeof(path),"%s/object-bindings.png",output)<(int)sizeof(path) && gd_save_screenshot(path));
    {
        GD_ExplorerRuleState *state=pane->rule_state;
        snprintf(state->pending.object,sizeof(state->pending.object),"missing/object"); NATIVE_QUESTION_CHECK(!gd_explorer_rules_run(c) && pane->rule_state==state); state->pending=state->applied;
        *fail_at=*calls+1; NATIVE_QUESTION_CHECK(!gd_explorer_rules_run(c) && pane->rule_state==state); *fail_at=0;
    }
    NATIVE_QUESTION_CHECK(reload_project(c,panes,collections,count,failure,sizeof(failure))); c->pane=&panes[GD_EXPLORER_RESOURCES];
    NATIVE_QUESTION_CHECK(c->pane->rule_state && c->pane->rule_state->bindings && explorer_test_fresh_pane(c) && !c->pane->rule_state);
    fprintf(stderr,"Native question window checks: nine MDS scopes, rule outcomes/consistency, binding inventory, rollback/reload/reset passed; no OS input.\n"); ok=1;
cleanup:
    *fail_at=0; return ok;
}
#undef NATIVE_QUESTION_CHECK
