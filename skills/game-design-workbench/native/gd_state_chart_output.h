/* Private serializer for retained local-binding hypotheses. No evaluation or allocation. */
static void chart_activation(FILE *out,const GD_Query *query,const GD_RuleActivation *activation,int index) {
    int step;
    fprintf(out,"{\"index\":%d,\"source\":",index); source_case(out,query,activation->case_index);
    member(out,"applicability",gd_rule_truth_name(activation->applicability));
    member(out,"local_applicability",gd_rule_truth_name(activation->local_applicability));
    fputs(",\"via\":[",out);
    for (step=0;step<activation->path_count && !ferror(out);++step) {
        if (step) fputc(',',out); fputs("{\"source\":",out); source_case(out,query,activation->path[step]);
        if (activation->calls[step]>=0) {
            const GD_RuleCall *call=&query->analysis->calls[activation->calls[step]];
            fprintf(out,",\"call_snapshot_index\":%d,\"call_line\":%d,\"call_text\":",activation->calls[step],call->line);
            gd_json_string_n(out,call->text,call->length);
        }
        fputc('}',out);
    }
    fputs("]}",out);
}
static int chart_axis_write(const GD_Query *query,const GD_StateChartScenario *scenario,const GD_RuleWrite *write) {
    return write->actor==query->bindings[scenario->binding].participant && (write->axis==query->request.axis || (query->state_chart->companion_axis>=0 && write->axis==query->state_chart->companion_axis)) &&
        (write->kind==GD_WRITE_STATE || write->kind==GD_WRITE_PRESERVE);
}
static void chart_scenarios(FILE *out,const GD_Query *query) {
    const GD_StateChart *chart=query->state_chart; int i,j;
    fputc('[',out);
    for (i=0;i<chart->scenario_count && !ferror(out);++i) {
        const GD_StateChartScenario *scenario=&chart->scenarios[i]; const GD_QueryBinding *binding=&query->bindings[scenario->binding];
        const char *from=query->state_coverage->values[scenario->value].value; int count=0;
        if (i) fputc(',',out);
        fprintf(out,"{\"index\":%d,\"binding_index\":%d,\"source\":",i,scenario->binding); source_case(out,query,binding->case_index);
        fputs(",\"actor\":",out); actor(out,query,binding->participant);
        member(out,"from",from); member(out,"companion_value",scenario->companion_value); fprintf(out,",\"value_index\":%d",scenario->value);
        member(out,"applicability",gd_rule_truth_name(scenario->applicability)); member(out,"context",gd_rule_truth_name(scenario->context)); member(out,"requirements",gd_rule_truth_name(scenario->requirements));
        fprintf(out,",\"exclusion_state\":%d,\"guards\":%d,\"checked_guards\":%d,\"constraints_complete\":%s,\"values_complete\":%s,\"limited\":%s,\"opaque_effects\":%d,\"cycles\":%d,\"conflicts\":%d,\"uncertain_pairs\":%d,\"transitions\":[",
            scenario->exclusion.state,scenario->exclusion.guard_count,scenario->exclusion.checked_count,scenario->constraints_complete ? "true":"false",scenario->values_complete ? "true":"false",scenario->limited ? "true":"false",scenario->opaque_effects,scenario->cycles,scenario->conflicts,scenario->uncertain_pairs);
        for (j=0;j<scenario->write_count && !ferror(out);++j) {
            const GD_RuleWrite *write=&chart->writes[scenario->first_write+j];
            if (!chart_axis_write(query,scenario,write)) continue;
            if (count++) fputc(',',out); fprintf(out,"{\"proposal_index\":%d,\"kind\":",j); gd_json_string(out,write->kind==GD_WRITE_PRESERVE ? "preserve":"state");
            member(out,"axis",query->model->records[write->axis].reference);
            member(out,"from",write->axis==query->request.axis ? from:scenario->companion_value);
            member(out,"to",write->kind==GD_WRITE_PRESERVE ? (write->axis==query->request.axis ? from:scenario->companion_value):write->state);
            fprintf(out,",\"conditional\":%s}",write->conditional ? "true":"false");
        }
        fputs("],\"axis_result\":",out); gd_json_string(out,count ? "retained_proposals":"unspecified");
        fputs(",\"proposals\":[",out);
        for (j=0;j<scenario->write_count && !ferror(out);++j) {
            int index=scenario->first_write+j; const GD_RuleWrite *write=&chart->writes[index];
            if (j) fputc(',',out); fprintf(out,"{\"index\":%d,\"activation\":%d,\"family\":",j,chart->write_activations[index]); gd_json_string(out,gd_effect_kind_name(chart->families[index]));
            fputs(",\"affected\":",out); actor(out,query,write->actor);
            member(out,"declared_unit",write->axis>=0 ? query->model->records[write->axis].axis.unit:NULL);
            fputs(",\"proposal\":",out); gd_write_rule_proposal(out,query->analysis,write); fputc('}',out);
        }
        fputs("],\"activations\":[",out);
        for (j=0;j<scenario->activation_count && !ferror(out);++j) {
            if (j) fputc(',',out); chart_activation(out,query,&chart->activations[scenario->first_activation+j],j);
        }
        fputs("]}",out);
    }
    fputc(']',out);
}
static void chart_text(FILE *out,const GD_Query *query) {
    const GD_StateChart *chart=query->state_chart; int i,j;
    fprintf(out,"Object %s | axis %s\n",query->model->records[query->request.noun].reference,query->model->records[query->request.axis].reference);
    if (chart->companion_axis>=0) fprintf(out,"Companion %s | state-value summaries aggregate existentially over declared companion values\n",query->model->records[chart->companion_axis].reference);
    for (i=0;i<chart->scenario_count;++i) {
        const GD_StateChartScenario *scenario=&chart->scenarios[i]; const GD_QueryBinding *binding=&query->bindings[scenario->binding];
        const GD_RuleCase *item=&query->rules->cases[binding->case_index]; int count=0;
        const char *from=query->state_coverage->values[scenario->value].value;
        fprintf(out,"%s#%s :: %s | from %s | applicability %s\n",query->model->records[query->rules->sources[item->source].record].reference,item->name,query->rules->participants[binding->participant].name,from,gd_rule_truth_name(scenario->applicability));
        if (scenario->companion_value) fprintf(out,"  Companion assumption %s=%s\n",query->model->records[chart->companion_axis].reference,scenario->companion_value);
        for (j=0;j<scenario->write_count;++j) {
            const GD_RuleWrite *write=&chart->writes[scenario->first_write+j];
            if (!chart_axis_write(query,scenario,write)) continue;
            ++count; fprintf(out,"  %s %s=%s | %s:%d%s\n",write->kind==GD_WRITE_PRESERVE ? "preserve":"propose",query->model->records[write->axis].reference,write->kind==GD_WRITE_PRESERVE ? (write->axis==query->request.axis ? from:scenario->companion_value):write->state,query->model->records[query->rules->sources[query->rules->cases[write->origin_case].source].record].source_path,write->line,write->conditional ? " | conditional":"");
        }
        if (!count) fputs("  No selected-axis proposal retained; preservation is not implied.\n",out);
        fprintf(out,"  %d total proposals; opaque effects %d; cycles %d; conflicts %d; limited %s\n",scenario->write_count,scenario->opaque_effects,scenario->cycles,scenario->conflicts,scenario->limited ? "yes":"no");
    }
    fputs("Independent local-binding hypotheses; optional companion values apply to that same binding, not same-noun peers. No combined-state reachability, inferred gameplay roles or final-outcome proof. Use JSON for full guards, mapped calls and side effects.\n",out);
}
