/* Primary-target normalization checks. Fixtures stay in the native test directory. */
TEST(rule_primary_targets,
    GD_Model *model=model_create(); GD_Rules *rules; GD_RuleAnalysis *analysis;
    GD_RuleEffectSpan spans[2]; const GD_RuleCase *item; const GD_RuleParticipant *actor;
    int c, a, b, first, n; char path[GD_MAX_PATH]; FILE *out;
    TEST_ASSERT(model && setup("rule-primary-targets")); collections[1].role=GD_AXIS; collections[2].role=GD_RULE;
    TEST_ASSERT(write_record(1,"Life","---\nvalue_type: enum\nallowed_values: [alive, dead]\nclosed: true\n---\nLife."));
    TEST_ASSERT(write_record(1,"Form","---\nvalue_type: enum\nallowed_values: [small, tall]\nclosed: true\n---\nForm."));
    TEST_ASSERT(write_record(1,"Amount","---\nvalue_type: integer\nunit: count\n---\nAmount."));
    TEST_ASSERT(write_record(0,"hero","---\naxes: [actions/Life, actions/Form, actions/Amount]\n---\nActor."));
    TEST_ASSERT(write_record(2,"helper","```gd-rule\ncase noop:\n    Actor:\n        noun: things/hero\n        requires: ANY\n        preserve: OTHER\n```\n"));
    TEST_ASSERT(write_record(2,"root",
        "```gd-rule\ncase early:\n    target: A\n    context: ANY\n    becomes: dead\n    preserve: OTHER\n    causes: cases/helper#noop\n    set:\n        Amount: this + 1[count]\n"
        "    B:\n        noun: things/hero\n        requires: ANY\n        set:\n            Amount: this + 3[count]\n"
        "    A:\n        noun: things/hero\n        requires: ANY\n        becomes: tall\n        preserve: OTHER\n        causes: cases/helper#noop\n        set:\n            Amount: this + 2[count]\n"
        "case late:\n    target: A\n    B:\n        noun: things/hero\n        set:\n            Amount: this + 4[count]\n"
        "    A:\n        noun: things/hero\n        becomes: tall\n        set:\n            Amount: this + 5[count]\n"
        "    becomes: dead\n    preserve: OTHER\n    causes: cases/helper#noop\n    set:\n        Amount: this + 6[count]\n```\n"));
    TEST_ASSERT(gd_load(model,collections,3)); rules=gd_read_rules(model); TEST_ASSERT(rules && !model->error_count);
    c=gd_rule_case_find(model,rules,"cases/root#early"); TEST_ASSERT(c>=0); item=&rules->cases[c];
    b=item->first_participant; a=b+1; actor=&rules->participants[a];
    TEST_ASSERT(item->primary_participant==a && !strcmp(item->primary_target,"A"));
    TEST_ASSERT(!strcmp(rules->participants[b].name,"B") && !strcmp(actor->name,"A"));
    TEST_ASSERT(actor->state_effect_count==2 && actor->preservation_count==2 && actor->assignment_count==2);
    first=actor->first_state_effect;
    TEST_ASSERT(!strncmp(rules->state_effects[first].text,"dead",4) && !strncmp(rules->state_effects[first+1].text,"tall",4));
    TEST_ASSERT(rules->state_effects[first].line==item->default_lines[0] && rules->state_effects[first+1].line==actor->lines[4]);
    first=actor->first_assignment;
    TEST_ASSERT(rules->assignments[first].from_primary && !rules->assignments[first+1].from_primary);
    TEST_ASSERT(rules->assignments[first].participant==a && rules->assignments[first+1].participant==a);
    TEST_ASSERT(!strcmp(rules->assignments[first].expression,"this + 1[count]") && !strcmp(rules->assignments[first+1].expression,"this + 2[count]"));
    TEST_ASSERT(rules->participants[b].assignment_count==1 && !strcmp(rules->assignments[rules->participants[b].first_assignment].expression,"this + 3[count]"));
    n=gd_rule_effect_spans(rules,c,a,GD_RULE_EFFECT_CALL,spans);
    TEST_ASSERT(n==2 && spans[0].from_primary && !spans[1].from_primary && spans[0].line<spans[1].line);
    c=gd_rule_case_find(model,rules,"cases/root#late"); TEST_ASSERT(c>=0); item=&rules->cases[c]; a=item->primary_participant; actor=&rules->participants[a];
    TEST_ASSERT(a==item->first_participant+1 && actor->assignment_count==2 && actor->state_effect_count==2 && actor->preservation_count==1);
    TEST_ASSERT((actor->defaulted_fields & (1u<<5)) && (actor->defaulted_fields & (1u<<6)) && !(actor->defaulted_fields & (1u<<4)));
    n=gd_rule_effect_spans(rules,c,a,GD_RULE_EFFECT_STATE,spans);
    TEST_ASSERT(n==2 && !spans[0].from_primary && spans[1].from_primary && spans[0].line<spans[1].line);
    first=actor->first_assignment;
    TEST_ASSERT(!strcmp(rules->assignments[first].expression,"this + 5[count]") && !strcmp(rules->assignments[first+1].expression,"this + 6[count]"));
    TEST_ASSERT(!rules->assignments[first].from_primary && rules->assignments[first+1].from_primary);
    analysis=gd_analyze_rules(model,rules); TEST_ASSERT(analysis && !model->error_count && analysis->call_count==3);
    TEST_ASSERT(analysis->calls[2].caller==a && analysis->calls[2].line==item->default_lines[2]);
    TEST_ASSERT(analysis->calls[0].line<analysis->calls[1].line && analysis->calls[0].resolution==GD_RULE_RESOLVED && analysis->calls[2].resolution==GD_RULE_RESOLVED);
    TEST_ASSERT(snprintf(path,sizeof(path),"%s/primary-target-rules.json",root)<(int)sizeof(path));
    out=fopen(path,"wb"); TEST_ASSERT(out && gd_write_rules_result(out,model,rules,1,"cases")); TEST_ASSERT(!fclose(out));
    gd_rule_analysis_destroy(analysis); gd_rules_destroy(rules); gd_destroy(model);
)
TEST(rule_primary_equivalence,
    GD_Model *model=model_create(); GD_Rules *rules; GD_RuleAnalysis *analysis;
    GD_RuleAssumptions inputs={0}; int first, second, i;
    TEST_ASSERT(model && setup("rule-primary-equivalence")); collections[1].role=GD_AXIS; collections[2].role=GD_RULE;
    TEST_ASSERT(write_record(1,"Life","---\nvalue_type: enum\nallowed_values: [alive, dead]\nclosed: true\n---\nLife."));
    TEST_ASSERT(write_record(1,"Form","---\nvalue_type: enum\nallowed_values: [small, tall]\nclosed: true\n---\nForm."));
    TEST_ASSERT(write_record(1,"Amount","---\nvalue_type: integer\nunit: count\n---\nAmount."));
    TEST_ASSERT(write_record(0,"hero","---\naxes: [actions/Life, actions/Form, actions/Amount]\n---\nActor."));
    TEST_ASSERT(write_record(2,"root",
        "```gd-rule\ncase defaults:\n    target: A\n    context: ANY\n    becomes: dead\n    preserve: OTHER\n    set:\n        Amount: this + 1[count]\n"
        "    B:\n        noun: things/hero\n        requires: ANY\n        other_axes: ANY\n        preserve: ALL\n"
        "    A:\n        noun: things/hero\n        requires: ANY\n        other_axes: ANY\n        becomes: tall\n"
        "case scoped:\n    context: ANY\n    B:\n        noun: things/hero\n        requires: ANY\n        other_axes: ANY\n        preserve: ALL\n"
        "    A:\n        noun: things/hero\n        requires: ANY\n        other_axes: ANY\n        becomes: dead, tall\n        preserve: OTHER\n        set:\n            Amount: this + 1[count]\n```\n"));
    TEST_ASSERT(gd_load(model,collections,3)); rules=gd_read_rules(model); TEST_ASSERT(rules && !model->error_count);
    analysis=gd_analyze_rules(model,rules); TEST_ASSERT(analysis && !model->error_count);
    first=gd_rule_case_find(model,rules,"cases/root#defaults"); second=gd_rule_case_find(model,rules,"cases/root#scoped");
    TEST_ASSERT(first>=0 && second>=0 && !rules->cases[second].primary_target && rules->cases[second].primary_participant==-1);
    gd_rule_evaluate(analysis,first,inputs,&analysis->first); gd_rule_evaluate(analysis,second,inputs,&analysis->second);
    TEST_ASSERT(analysis->first.write_count==6 && analysis->second.write_count==6);
    TEST_ASSERT(analysis->first.applicability==analysis->second.applicability && analysis->first.conflict_count==analysis->second.conflict_count);
    TEST_ASSERT(analysis->first.constraints_complete==analysis->second.constraints_complete && analysis->first.values_complete==analysis->second.values_complete);
    for (i=0;i<analysis->first.write_count;++i) {
        const GD_RuleWrite *a=&analysis->first.writes[i], *b=&analysis->second.writes[i];
        TEST_ASSERT(a->kind==b->kind && a->axis==b->axis && a->conditional==b->conditional);
        TEST_ASSERT(a->actor-rules->cases[first].first_participant==b->actor-rules->cases[second].first_participant);
        TEST_ASSERT((!a->state && !b->state) || (a->state && b->state && !strcmp(a->state,b->state)));
        TEST_ASSERT(a->value.state==b->value.state && a->value.number==b->value.number);
    }
    gd_rule_analysis_destroy(analysis); gd_rules_destroy(rules); gd_destroy(model);
)
TEST(rule_primary_errors,
    const char *bad[]={
        "case bad:\n    target: Missing\n    A:\n        requires: ANY\n",
        "case bad:\n    target: A\n    target: B\n    A:\n        requires: ANY\n",
        "case bad:\n    target: things/hero\n    A:\n        requires: ANY\n",
        "case bad:\n    becomes: dead\n    A:\n        requires: ANY\n",
        "case bad:\n    target: A\n    becomes: dead\n    becomes: alive\n    A:\n        requires: ANY\n",
        "case bad:\n    target: A\n    set:\n        Amount: this + 1[count]\n    set:\n        Amount: this + 2[count]\n    A:\n        requires: ANY\n"
    };
    const char *codes[]={"rules.primary_target","rules.duplicate_field","rules.primary_target","rules.primary_required","rules.duplicate_field","rules.duplicate_field"};
    int i;
    for (i=0;i<(int)(sizeof(bad)/sizeof(bad[0]));++i) {
        GD_Model *model=model_create(); GD_Rules *rules; char directory[64],text[2048];
        TEST_ASSERT(model && snprintf(directory,sizeof(directory),"rule-primary-errors-%d",i)<(int)sizeof(directory) && setup(directory)); collections[2].role=GD_RULE;
        TEST_ASSERT(snprintf(text,sizeof(text),"```gd-rule\ncase earlier:\n    target: A\n    set:\n        Amount: this + 9[count]\n    A:\n        requires: ANY\n%s```\n",bad[i])<(int)sizeof(text));
        TEST_ASSERT(write_record(2,"bad",text));
        TEST_ASSERT(write_record(2,"good","```gd-rule\ncase good:\n    target: Z\n    set:\n        Amount: this + 7[count]\n    Z:\n        requires: ANY\n```\n"));
        TEST_ASSERT(gd_load(model,collections,3)); rules=gd_read_rules(model);
        TEST_ASSERT(rules && has_error(model,codes[i]) && rules->source_count==2 && rules->sources[0].state==GD_RULE_INVALID);
        TEST_ASSERT(rules->case_count==1 && rules->participant_count==1 && rules->assignment_count==1 && rules->cases[0].primary_participant==0);
        TEST_ASSERT(rules->assignments[0].participant==0 && rules->assignments[0].from_primary && !strcmp(rules->assignments[0].expression,"this + 7[count]"));
        gd_rules_destroy(rules); gd_destroy(model);
    }
)
TEST(rule_primary_allocation_failure,
    GD_Model *model=model_create(); GD_Rules *rules; RouteFailure failure; GD_Allocator allocator;
    TEST_ASSERT(model && setup("rule-primary-memory")); collections[2].role=GD_RULE;
    TEST_ASSERT(write_record(2,"root","```gd-rule\ncase root:\n    target: A\n    set:\n        Amount: this + 1[count]\n    A:\n        requires: ANY\n```\n"));
    TEST_ASSERT(gd_load(model,collections,3)); allocator=model->allocator;
    failure.parent=allocator; failure.remaining=1;
    model->allocator.context=&failure; model->allocator.allocate=route_allocate; model->allocator.release=route_release;
    TEST_ASSERT(!gd_read_rules(model) && has_error(model,"rules.memory")); model->allocator=allocator;
    model->error_count=0; rules=gd_read_rules(model);
    TEST_ASSERT(rules && !model->error_count && rules->assignment_count==1 && rules->participants[0].assignment_count==1);
    gd_rules_destroy(rules); gd_destroy(model);
)
