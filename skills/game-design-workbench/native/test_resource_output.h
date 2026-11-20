TEST(resource_output_limits,
    GD_Model *model=model_create(); GD_Rules *rules; GD_ResourceGraph *graph;
    char path[GD_MAX_PATH],tail[4097]; FILE *out; int i; long size;
    TEST_ASSERT(model && setup("resource-output"));
    model->rule_mode=1; model->record_count=2;
    model->records[0].role=GD_RULE; model->records[0].reference="rules/capacity";
    model->records[0].name="capacity"; model->records[0].body=""; model->records[0].source_path="capacity.md";
    model->records[1].role=GD_RESOURCE; model->records[1].reference="resources/pool";
    model->records[1].name="pool"; model->records[1].body=""; model->records[1].source_path="pool.md";
    rules=gd_read_rules(model); TEST_ASSERT(rules && !model->error_count && rules->source_count==1);
    for (i=0;i<GD_MAX_RULE_CASES;++i) {
        char *name=model->text+model->text_used;
        int length=snprintf(name,32,"case-%d",i);
        model->text_used+=(size_t)length+1;
        rules->cases[i].name=name; rules->cases[i].source=0; rules->cases[i].context="ANY"; rules->cases[i].line=i+1;
    }
    rules->case_count=GD_MAX_RULE_CASES;
    graph=gd_resource_graph_build(model,rules);
    TEST_ASSERT(graph && graph->node_count==GD_RESOURCE_GRAPH_NODES && graph->limited && graph->unexamined_cases==1);
    TEST_ASSERT(snprintf(path,sizeof(path),"%s/resource-output.json",root)<(int)sizeof(path));
    out=fopen(path,"wb"); TEST_ASSERT(out && gd_write_resource_result(out,model,graph,1,NULL)); TEST_ASSERT(fclose(out)==0);
    out=fopen(path,"rb"); TEST_ASSERT(out && !fseek(out,0,SEEK_END)); size=ftell(out);
    TEST_ASSERT(size>4096 && !fseek(out,-4096,SEEK_END) && fread(tail,1,4096,out)==4096); tail[4096]=0;
    TEST_ASSERT(strstr(tail,"\"graph_limited\":true") && strstr(tail,"\"unexamined_cases\":1") && strstr(tail,"\"routes_available\":true"));
    TEST_ASSERT(fclose(out)==0);
    /* Report I/O failure must propagate, not report a successful write. */
    out=fopen(path,"rb"); TEST_ASSERT(out && !gd_write_resource_result(out,model,graph,1,"routes")); fclose(out);
    gd_resource_graph_destroy(graph); gd_rules_destroy(rules); gd_destroy(model);
)
