/* Shared application control checks; no window, renderer, OS input or source writes. */
#ifndef TEST_EXPLORER_CONTROLS_H
#define TEST_EXPLORER_CONTROLS_H
static int explorer_control_checks(GD_ExplorerContext *context,char *reply,int *fail_at,int *calls,int *checks) {
    union { GD_ExplorerQueryOptions query; GD_ExplorerStructureForm structure; GD_ExplorerRuleForm rules; } a,b;
    GD_ExplorerPane *prior=context->pane; GD_ExplorerPane temporary; GD_ExplorerContext c=*context;
    int form,i,j,count,ok=0; GD_ViewerWriter writer;
#define CONTROL_CHECK(value) do { ++*checks; if (!(value)) { fprintf(stderr,"Shared control check failed: %s (line %d)\n",#value,__LINE__); goto cleanup; } } while (0)
    memset(&temporary,0,sizeof(temporary));
    for (form=0;form<GD_FORM_COUNT;++form) {
        const GD_ExplorerControl *fields=gd_explorer_form_fields((GD_ExplorerForm)form,&count); size_t size=gd_explorer_form_size((GD_ExplorerForm)form);
        CONTROL_CHECK(fields && count>0 && size<=sizeof(a)); gd_explorer_form_defaults((GD_ExplorerForm)form,&a);
        for (i=0;i<count;++i) {
            const GD_ExplorerControl *field=&fields[i]; char *value;
            CONTROL_CHECK(field->name && *field->name && field->label && *field->label && field->offset+field->capacity<=size);
            CONTROL_CHECK(gd_explorer_form_field((GD_ExplorerForm)form,field->name)==field);
            for (j=0;j<i;++j) CONTROL_CHECK(strcmp(field->name,fields[j].name));
            memcpy(&b,&a,size); value=(char *)&b+field->offset;
            if (field->type==GD_CONTROL_INT) { CONTROL_CHECK(field->capacity==sizeof(int)); ++*(int *)value; }
            else if (field->type==GD_CONTROL_BOOL) { CONTROL_CHECK(field->capacity==sizeof(bool)); *(bool *)value=!*(bool *)value; }
            else { CONTROL_CHECK(field->capacity>=2); strcpy(value,"x"); }
            if (form==GD_FORM_QUERY && (!strcmp(field->name,"literal_values") || !strcmp(field->name,"literal_assumptions"))) CONTROL_CHECK(gd_explorer_form_equal((GD_ExplorerForm)form,&a,&b));
            else CONTROL_CHECK(!gd_explorer_form_equal((GD_ExplorerForm)form,&a,&b));
            CONTROL_CHECK(gd_explorer_form_equal((GD_ExplorerForm)form,&a,&a));
        }
        CONTROL_CHECK(!gd_explorer_form_field((GD_ExplorerForm)form,"missing-control"));
        writer=(GD_ViewerWriter){reply,0,GD_VIEWER_REPLY_LIMIT,0}; viewer_form(&writer,(GD_ExplorerForm)form,&a); CONTROL_CHECK(!writer.failed && *reply=='{');
    }
    gd_explorer_form_defaults(GD_FORM_QUERY,&a.query); b.query=a.query;
    strcpy(a.query.values,"alive\ndead"); b.query=a.query; b.query.literal_values=!a.query.literal_values;
    CONTROL_CHECK(!gd_explorer_form_equal(GD_FORM_QUERY,&a.query,&b.query));
    *a.query.values=0; strcpy(a.query.facts,"fixed fact"); b.query=a.query; b.query.literal_assumptions=!a.query.literal_assumptions;
    CONTROL_CHECK(!gd_explorer_form_equal(GD_FORM_QUERY,&a.query,&b.query));
    gd_explorer_query_question(&a.query,1); CONTROL_CHECK(a.query.coverage && a.query.root_case<0 && a.query.participant<0 && a.query.axis<0 && a.query.noun<0 && a.query.compare<0 && !*a.query.facts && !*a.query.values);
    b.query=a.query; CONTROL_CHECK(!gd_explorer_query_root(&a.query,context->project->rules,-2) && !memcmp(&a.query,&b.query,sizeof(a.query)));
    {
        const GD_Rules *rules=context->project->rules; const GD_Model *model=rules->model; int root,axis;
        for (root=0;root<rules->case_count;++root) if (rules->cases[root].participant_count>0) break;
        for (axis=0;axis<model->record_count;++axis) if (model->records[axis].role==GD_AXIS) break;
        CONTROL_CHECK(root<rules->case_count && axis<model->record_count);
        gd_explorer_query_question(&a.query,0); strcpy(a.query.values,"old samples"); strcpy(a.query.facts,"old facts");
        CONTROL_CHECK(gd_explorer_query_root(&a.query,rules,root));
        CONTROL_CHECK(a.query.participant==rules->cases[root].primary_participant && a.query.axis<0 && !*a.query.values && !*a.query.facts);
        CONTROL_CHECK(gd_explorer_query_participant(&a.query,rules,rules->cases[root].first_participant));
        a.query.primary=1; a.query.compare=2; strcpy(a.query.values,"old samples");
        CONTROL_CHECK(gd_explorer_query_axis(&a.query,model,axis) && a.query.axis==axis && !*a.query.values && !a.query.primary && a.query.compare<0);
        b.query=a.query; CONTROL_CHECK(!gd_explorer_query_participant(&a.query,rules,-1) && !memcmp(&a.query,&b.query,sizeof(a.query)));
        CONTROL_CHECK(!gd_explorer_query_axis(&a.query,model,model->record_count) && !memcmp(&a.query,&b.query,sizeof(a.query)));
    }
    gd_explorer_form_defaults(GD_FORM_STRUCTURE,&a.structure); a.structure.actor_scope=1; strcpy(a.structure.starts,"start"); strcpy(a.structure.from,"from");
    CONTROL_CHECK(gd_explorer_structure_question(&a.structure,GD_STRUCTURE_COUNT+1) && !*a.structure.starts && !strcmp(a.structure.from,"from") && a.structure.work_limit==GD_SIMILARITY_WORK);
    CONTROL_CHECK(gd_explorer_structure_question(&a.structure,GD_STRUCTURE_DECLARATIONS) && a.structure.actor_scope==1 && !*a.structure.from && a.structure.work_limit==GD_GRAPH_ANALYSIS_WORK);
    b.structure=a.structure; CONTROL_CHECK(!gd_explorer_structure_question(&a.structure,GD_STRUCTURE_QUESTIONS) && !memcmp(&a.structure,&b.structure,sizeof(a.structure)));
    for (i=0;i<3;++i) { CONTROL_CHECK(gd_explorer_rule_mode(&a.rules,i)); CONTROL_CHECK(a.rules.bindings==(i==2) && a.rules.kind==(i==1 ? GD_NAMED_CONSISTENCY:GD_NAMED_OUTCOMES)); }
    b.rules=a.rules; CONTROL_CHECK(!gd_explorer_rule_mode(&a.rules,3) && !memcmp(&a.rules,&b.rules,sizeof(a.rules)));
    /* Opening inputs is explicit; reading and unsupported requests never allocate. */
    pane_defaults(&temporary,GD_EXPLORER_RESOURCES); c.pane=&temporary; i=*calls;
    CONTROL_CHECK(!gd_explorer_form_pending(&c,GD_FORM_STRUCTURE,0) && !gd_explorer_form_pending(&c,GD_FORM_RULES,0));
    CONTROL_CHECK(!gd_explorer_form_pending(&c,GD_FORM_QUERY,1) && *calls==i);
    *fail_at=*calls+1; CONTROL_CHECK(!gd_explorer_form_pending(&c,GD_FORM_STRUCTURE,1)); *fail_at=0;
    CONTROL_CHECK(!temporary.structure_state);
    CONTROL_CHECK(gd_explorer_form_pending(&c,GD_FORM_STRUCTURE,1)!=NULL);
    {
        GD_ExplorerStructureState *state=temporary.structure_state;
        *fail_at=*calls+1; CONTROL_CHECK(!gd_explorer_action_run(&c,GD_ACTION_STRUCTURE_CLEAR)); *fail_at=0;
        CONTROL_CHECK(temporary.structure_state==state);
    }
    CONTROL_CHECK(gd_explorer_action_run(&c,GD_ACTION_STRUCTURE_CLEAR));
    *fail_at=*calls+1; CONTROL_CHECK(!gd_explorer_form_pending(&c,GD_FORM_RULES,1)); *fail_at=0;
    CONTROL_CHECK(!temporary.rule_state);
    CONTROL_CHECK(gd_explorer_action_available(&c,GD_ACTION_RULES_RUN));
    *fail_at=*calls+1; CONTROL_CHECK(!gd_explorer_action_run(&c,GD_ACTION_RULES_RUN) && !temporary.rule_state); *fail_at=0;
    CONTROL_CHECK(gd_explorer_form_pending(&c,GD_FORM_RULES,1)!=NULL);
    CONTROL_CHECK(gd_explorer_rule_mode(&temporary.rule_state->pending,2));
    CONTROL_CHECK(gd_explorer_action_run(&c,GD_ACTION_RULES_RUN) && temporary.rule_state->bindings!=NULL);
    CONTROL_CHECK(gd_explorer_form_equal(GD_FORM_RULES,&temporary.rule_state->pending,&temporary.rule_state->applied));
    {
        GD_ExplorerRuleState *state=temporary.rule_state;
        *fail_at=*calls+1; CONTROL_CHECK(!gd_explorer_action_run(&c,GD_ACTION_RULES_RUN)); *fail_at=0;
        CONTROL_CHECK(temporary.rule_state==state);
    }
    CONTROL_CHECK(gd_explorer_action_run(&c,GD_ACTION_RULES_CLEAR) && !temporary.rule_state);
    CONTROL_CHECK(!gd_explorer_action_run(&c,GD_ACTION_QUERY_RUN));
    pane_defaults(&temporary,GD_EXPLORER_ACTORS);
    *fail_at=*calls+1; CONTROL_CHECK(!gd_explorer_action_run(&c,GD_ACTION_QUERY_RUN)); *fail_at=0;
    CONTROL_CHECK(!temporary.query_state);
    CONTROL_CHECK(gd_explorer_action_run(&c,GD_ACTION_QUERY_RUN) && temporary.query_state);
    CONTROL_CHECK(temporary.prepared && temporary.revision==1);
    {
        GD_QueryScene *scene=temporary.query_state->scene;
        *fail_at=*calls+1; CONTROL_CHECK(!gd_explorer_action_run(&c,GD_ACTION_QUERY_DISPLAY)); *fail_at=0;
        CONTROL_CHECK(temporary.query_state->scene==scene);
        CONTROL_CHECK(gd_explorer_action_run(&c,GD_ACTION_QUERY_DISPLAY));
        scene=temporary.query_state->scene; gd_explorer_query_question(&temporary.query_options,1);
        CONTROL_CHECK(!gd_explorer_action_run(&c,GD_ACTION_QUERY_RUN) && temporary.query_state->scene==scene);
        CONTROL_CHECK(temporary.query_options.coverage && !temporary.query_state->applied.coverage);
    }
    for (form=0;form<GD_ACTION_COUNT;++form) {
        const GD_ExplorerActionInfo *info=gd_explorer_action_info((GD_ExplorerAction)form);
        CONTROL_CHECK(info && *info->name && *info->label && info->views);
        for (j=0;j<form;++j) CONTROL_CHECK(strcmp(info->name,gd_explorer_action_info((GD_ExplorerAction)j)->name));
    }
    CONTROL_CHECK(!gd_explorer_action_info(GD_ACTION_COUNT)); ok=1;
cleanup:
    *fail_at=0; pane_release_layout(context->project->allocator,&temporary);
    context->pane=prior;
#undef CONTROL_CHECK
    return ok;
}
#endif
