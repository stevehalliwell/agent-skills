
TEST(rule_primary_partial_lists,
    int kind, bad_default, late;
    for (kind=0;kind<2;++kind) for (bad_default=0;bad_default<2;++bad_default) for (late=0;late<2;++late) {
        GD_Model *model=model_create(); GD_Rules *rules; GD_RuleAnalysis *analysis; GD_ResourceGraph *graph;
        GD_RuleAssumptions inputs={0}; char directory[80],text[2048],defaults[256],scoped[256]; int actor;
        const char *field=kind ? "preserve":"becomes";
        TEST_ASSERT(model && snprintf(directory,sizeof(directory),"primary-partial-%d-%d-%d",kind,bad_default,late)<(int)sizeof(directory) && setup(directory));
        collections[1].role=GD_AXIS; collections[2].role=GD_RULE;
        TEST_ASSERT(write_record(1,"Life","---\nvalue_type: enum\nallowed_values: [alive, dead]\nclosed: true\n---\n"));
        TEST_ASSERT(write_record(1,"Form","---\nvalue_type: enum\nallowed_values: [small, tall]\nclosed: true\n---\n"));
        TEST_ASSERT(write_record(0,"hero","---\naxes: [actions/Life, actions/Form]\n---\n"));
        TEST_ASSERT(snprintf(defaults,sizeof(defaults),"    %s: %s%s\n",field,kind ? "ALL":"alive",bad_default ? ",":"")<(int)sizeof(defaults));
        TEST_ASSERT(snprintf(scoped,sizeof(scoped),"        %s: %s%s\n%s",field,kind ? "Life":"dead",bad_default ? "":",",kind ? "":"        preserve: OTHER\n")<(int)sizeof(scoped));
        TEST_ASSERT(snprintf(text,sizeof(text),"---\nstatus: accepted\ncoverage: defined\n---\n```gd-rule\ncase root:\n    target: A\n    context: ANY\n%s    A:\n        noun: things/hero\n        requires: ANY\n        other_axes: ANY\n%s%s```\n",late ? "":defaults,scoped,late ? defaults:"")<(int)sizeof(text));
        TEST_ASSERT(write_record(2,"root",text) && gd_load(model,collections,3)); rules=gd_read_rules(model);
        TEST_ASSERT(rules && has_error(model,kind ? "rules.preserve_list":"rules.state_list") && rules->case_count==1);
        actor=rules->cases[0].primary_participant;
        TEST_ASSERT(rules->participants[actor].incomplete_effect_fields & (1u<<(kind+4)));
        TEST_ASSERT(kind ? rules->participants[actor].preservation_count==1:rules->participants[actor].state_effect_count==1);
        analysis=gd_analyze_rules(model,rules); TEST_ASSERT(analysis);
        gd_rule_evaluate(analysis,0,inputs,&analysis->scenario);
        TEST_ASSERT(!analysis->scenario.constraints_complete);
        if (!kind) {
            int i;
            TEST_ASSERT(!analysis->scenario.values_complete && analysis->scenario.opaque_effects);
            for (i=0;i<analysis->scenario.write_count;++i) if (analysis->scenario.writes[i].kind==GD_WRITE_PRESERVE)
                TEST_ASSERT(analysis->scenario.writes[i].conditional);
        }
        graph=gd_resource_graph_build(model,rules); TEST_ASSERT(graph && graph->unknown_case_coverage==1);
        if (kind) TEST_ASSERT(graph->preservation_unexamined==1);
        TEST_ASSERT(gd_resource_graph_bind_calls(graph,analysis));
        {
            GD_EffectGraph *effects=gd_effect_graph_build(graph,GD_EFFECT_WORK);
            TEST_ASSERT(effects && effects->source_incomplete && effects->link_count>0);
            gd_effect_graph_destroy(effects);
        }
        gd_resource_graph_destroy(graph); gd_rule_analysis_destroy(analysis); gd_rules_destroy(rules); gd_destroy(model);
    }
)
TEST(rule_primary_partial_limits,
    int kind, late;
    for (kind=0;kind<2;++kind) for (late=0;late<2;++late) {
        GD_Model *model=model_create(); GD_Rules *rules; GD_RuleAnalysis *analysis; GD_RuleAssumptions inputs={0};
        char directory[80],path[GD_MAX_PATH]; FILE *out; int c, n, root_case, actor;
        const char *field=kind ? "preserve":"becomes", *value=kind ? "Life":"alive";
        const char *defaults=kind ? "    preserve: ALL\n":"    becomes: dead\n";
        TEST_ASSERT(model && snprintf(directory,sizeof(directory),"primary-partial-limit-%d-%d",kind,late)<(int)sizeof(directory) && setup(directory));
        collections[1].role=GD_AXIS; collections[2].role=GD_RULE;
        TEST_ASSERT(write_record(1,"Life","---\nvalue_type: enum\nallowed_values: [alive, dead]\nclosed: true\n---\n"));
        TEST_ASSERT(write_record(0,"hero","---\naxes: [actions/Life]\n---\n"));
        TEST_ASSERT(snprintf(path,sizeof(path),"%s/root.md",folders[2])<(int)sizeof(path)); out=fopen(path,"wb"); TEST_ASSERT(out);
        fputs("```gd-rule\n",out);
        for (c=0;c<8;++c) {
            fprintf(out,"case fill-%d:\n    Actor:\n        noun: things/hero\n        %s: ",c,field);
            for (n=0;n<(c==7 ? 1023:1024);++n) { if (n) fputs(", ",out); fputs(value,out); }
            fputc('\n',out);
        }
        fprintf(out,"case root:\n    target: A\n    context: ANY\n%s    A:\n        noun: things/hero\n        requires: ANY\n        other_axes: ANY\n        %s: %s\n%s```\n",late ? "":defaults,field,value,late ? defaults:"");
        TEST_ASSERT(!fclose(out) && gd_load(model,collections,3)); rules=gd_read_rules(model);
        TEST_ASSERT(rules && has_error(model,kind ? "rules.preserve_limit":"rules.state_effect_limit"));
        root_case=gd_rule_case_find(model,rules,"cases/root#root"); TEST_ASSERT(root_case>=0); actor=rules->cases[root_case].primary_participant;
        TEST_ASSERT(rules->participants[actor].incomplete_effect_fields & (1u<<(kind+4)));
        TEST_ASSERT(kind ? rules->participants[actor].preservation_count==1:rules->participants[actor].state_effect_count==1);
        analysis=gd_analyze_rules(model,rules); TEST_ASSERT(analysis);
        gd_rule_evaluate(analysis,root_case,inputs,&analysis->scenario);
        TEST_ASSERT(!analysis->scenario.constraints_complete);
        if (!kind) TEST_ASSERT(!analysis->scenario.values_complete && analysis->scenario.opaque_effects);
        {
            GD_ResourceGraph *graph=gd_resource_graph_build(model,rules); GD_EffectGraph *effects;
            TEST_ASSERT(graph && gd_resource_graph_bind_calls(graph,analysis));
            effects=gd_effect_graph_build(graph,GD_EFFECT_WORK);
            TEST_ASSERT(effects && effects->source_incomplete && effects->link_count>0);
            gd_effect_graph_destroy(effects); gd_resource_graph_destroy(graph);
        }
        gd_rule_analysis_destroy(analysis); gd_rules_destroy(rules); gd_destroy(model);
    }
)
