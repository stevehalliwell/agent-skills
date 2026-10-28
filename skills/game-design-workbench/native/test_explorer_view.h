/* Included only by -ExplorerTest. Exercise the shell's actual state helpers and
   renderers in one owned window. Does not claim real-pointer/UI acceptance. */
#define EXPLORER_CHECK(expression) do { if (!(expression)) { fprintf(stderr,"Explorer check failed at line %d: %s\n",__LINE__,#expression); goto cleanup; } } while (0)
static void explorer_test_render(GD_ExplorerContext *c) {
    GD_ExplorerPane *p=c->pane; int selected;
    c->viewport=(Rectangle){14,112,872,783};
    BeginDrawing(); ClearBackground(background); rlImGuiBegin();
    DrawText(gd_explorer_view_names[p->view],20,18,26,foreground);
    igSetNextWindowPos((ImVec2_c){900,12},ImGuiCond_Always,(ImVec2_c){0,0});
    igSetNextWindowSize((ImVec2_c){588,916},ImGuiCond_Always);
    igBegin("Explorer test controls",NULL,ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoSavedSettings);
    choose_view(c->project,"Data / projection",p->view,1);
    igButton("RESET CURRENT VIEW TO DEFAULTS",(ImVec2_c){-1,34});
    match_details(c); layout_controls(c); selected=analyze(c);
    analysis_controls(c,selected); igEnd();
    matches(c); p->camera.offset=(Vector2){450,503};
    if (p->fit) { fit_view(c,p->fit==2); p->fit=0; }
    {
        GD_ExplorerEvidence e=evidence(c,selected);
        if (is_graph(p->view)) gd_explorer_graph_draw(c,&e);
        else if (p->view==GD_EXPLORER_MDA) gd_explorer_mda_draw(c,&e);
        else gd_explorer_resource_draw(c,&e);
    }
    draw_matches(c); gd_graph_overview_draw(c->analysis,&p->overview,c->viewport,0);
    rlImGuiEnd(); EndDrawing();
}
int gd_explorer_checks(GD_Allocator allocator, const GD_Collection *collections, int count,
                       const char *output, int *fail_at, int *calls) {
    GD_ExplorerProject *project=gd_explorer_load(allocator,collections,count);
    GD_ExplorerPane *panes=NULL,*saved=NULL; GD_ExplorerContext c={0};
    int v,result=2,window=0,invalid_created=0,probe_created=0,resource_key=-1;
    char failure[GD_MAX_PATH+512]="",path[GD_MAX_PATH]="",invalid[GD_MAX_PATH]="",probe_path[GD_MAX_PATH]="",resource_source[260]="";
    GD_Model *model; GD_ExplorerProject *last_good; FILE *file;
    c.project=project;
    EXPLORER_CHECK(project && !gd_explorer_error(project) && project->available==63);
    panes=allocator.allocate(allocator.context,GD_EXPLORER_VIEWS*sizeof(*panes));
    if (panes) memset(panes,0,GD_EXPLORER_VIEWS*sizeof(*panes));
    saved=allocator.allocate(allocator.context,sizeof(*saved));
    c.analysis=allocator.allocate(allocator.context,sizeof(*c.analysis));
    c.query=allocator.allocate(allocator.context,sizeof(*c.query));
    EXPLORER_CHECK(panes && saved && c.analysis && c.query); memset(c.analysis,0,sizeof(*c.analysis));
    EXPLORER_CHECK(open_window("Workbench explorer app-local checks")); window=1;
    TraceLog(LOG_WARNING,"Explorer structured stdout: stderr warning probe");
    for (v=0;v<GD_EXPLORER_VIEWS;++v) {
        pane_defaults(&panes[v],(GD_ExplorerView)v); c.pane=&panes[v]; EXPLORER_CHECK(pane_prepare(&c));
        if (v==GD_EXPLORER_MDA) panes[v].mda.record=panes[v].layout->node_count ? panes[v].layout->nodes[0].key:-1;
        else panes[v].selection.node=panes[v].layout->node_count ? 0:-1;
        if (is_graph((GD_ExplorerView)v)) panes[v].inspected_record=gd_explorer_graph_record(&c);
        panes[v].paused=true; panes[v].paths.enabled=1;
        if (panes[v].layout->node_count) gd_graph_path_start(&panes[v].paths,panes[v].layout->nodes[0].key);
        panes[v].shapes=panes[v].foundations=true; panes[v].reach.enabled=1; panes[v].overview.enabled=1;
        explorer_test_render(&c);
        EXPLORER_CHECK(snprintf(path,sizeof(path),"%s/layers-%d.png",output,v)<(int)sizeof(path) && gd_save_screenshot(path));
    }
    /* Cross-family relationship roots resolve qualified source, not offsets.
       Missing destination roles must not mutate its pane or switch source. */
    {
        GD_Model *from=gd_explorer_model(project,GD_EXPLORER_MATRIX);
        GD_Model *target=gd_explorer_model(project,GD_EXPLORER_RELATIONSHIPS);
        int source=-1,mapped=-1,i; unsigned available=project->available;
        for (i=0;i<from->record_count;++i) if (from->records[i].role==GD_NOUN) {
            int r=gd_explorer_record(target,from->records[i].reference);
            if (r>=0 && r!=i) { source=i; mapped=r; break; }
        }
        EXPLORER_CHECK(source>=0 && mapped>=0);
        c.pane=&panes[GD_EXPLORER_MATRIX]; *saved=panes[GD_EXPLORER_RELATIONSHIPS];
        project->available&=~(1u<<GD_EXPLORER_RELATIONSHIPS);
        EXPLORER_CHECK(!center_relationships(&c,panes,source) && c.pane==&panes[GD_EXPLORER_MATRIX]);
        EXPLORER_CHECK(!memcmp(saved,&panes[GD_EXPLORER_RELATIONSHIPS],sizeof(*saved)));
        project->available=available;
        EXPLORER_CHECK(center_relationships(&c,panes,source) && c.pane->focus.root==mapped && pane_prepare(&c));
        EXPLORER_CHECK(!strcmp(target->records[c.pane->focus.root].reference,from->records[source].reference));
    }
    /* Find/canvas/source picks share validation. Invalid rank input must keep
       editor and selection; valid input commits before selecting another skill. */
    {
        GD_ExplorerPane *p=&panes[GD_EXPLORER_SKILLS]; GD_ExplorerSelection choice;
        int first,second;
        c.pane=p; EXPLORER_CHECK(pane_prepare(&c) && p->layout->node_count>=2);
        model=gd_explorer_model(project,GD_EXPLORER_SKILLS);
        first=p->layout->nodes[0].key; second=p->layout->nodes[1].key;
        p->selection=(GD_ExplorerSelection){-1,0,0,-1,-1}; p->inspected_record=first;
        p->editor.record=first; snprintf(p->editor.rank_text,sizeof(p->editor.rank_text),"invalid-rank");
        snprintf(p->find_reference,sizeof(p->find_reference),"%s",model->records[second].reference); matches(&c);
        EXPLORER_CHECK(p->match_nodes[1]); choice=(GD_ExplorerSelection){-1,1,0,-1,-1};
        EXPLORER_CHECK(!gd_explorer_graph_select(&c,&choice) && p->selection.node==0 && p->editor.record==first);
        EXPLORER_CHECK(!strcmp(p->editor.rank_text,"invalid-rank") && !project->draft->modified_count);
        EXPLORER_CHECK(!change_view(&c,panes,GD_EXPLORER_MDA,NULL) && !reset_view(&c));
        snprintf(p->editor.rank_text,sizeof(p->editor.rank_text),"37");
        EXPLORER_CHECK(gd_explorer_graph_select(&c,&choice) && p->selection.node==1 && p->inspected_record==second);
        EXPLORER_CHECK(model->records[first].has_rank && model->records[first].rank==37);
        p->editor.record=-1;
        EXPLORER_CHECK(gd_draft_undo(project->draft) && gd_explorer_edited(project) && pane_prepare(&c));
    }
    /* Remember edge owner, not compactable edge offset, across an inactive
       pane's prerequisite edit. Clear stale edge and retain source inspection. */
    {
        int i,edge=-1,owner; const char *reference;
        GD_ExplorerPane *p=&panes[GD_EXPLORER_RELATIONSHIPS];
        for (i=0;i<model->edge_count;++i) if (model->edges[i].field==GD_PREREQUISITES) { edge=i; break; }
        EXPLORER_CHECK(edge>=0); owner=model->edges[edge].source; reference=model->edges[edge].reference;
        EXPLORER_CHECK(center_relationships(&c,panes,owner) && pane_prepare(&c));
        p->selection=(GD_ExplorerSelection){edge,-1,0,-1,-1}; p->inspected_record=owner;
        EXPLORER_CHECK(gd_explorer_graph_record(&c)==owner && change_view(&c,panes,GD_EXPLORER_SKILLS,NULL));
        EXPLORER_CHECK(gd_draft_prerequisite(project->draft,owner,reference,0)>0 && gd_explorer_edited(project));
        EXPLORER_CHECK(change_view(&c,panes,GD_EXPLORER_RELATIONSHIPS,NULL) && pane_prepare(&c));
        EXPLORER_CHECK(p->selection.cell==-1 && gd_explorer_graph_record(&c)==owner);
        /* Active Undo restores baseline edge order. Capture current inspector
           before applying queued Undo, never read stale offsets afterward. */
        edge=-1;
        for (i=0;i<model->edge_count;++i) if (model->edges[i].source!=project->draft->baseline->edges[i].source) { edge=i; break; }
        EXPLORER_CHECK(edge>=0); owner=model->edges[edge].source;
        EXPLORER_CHECK(center_relationships(&c,panes,owner) && pane_prepare(&c));
        p->selection=(GD_ExplorerSelection){edge,-1,0,-1,-1};
        BeginDrawing(); rlImGuiBegin(); igBegin("Deferred Undo inspector",NULL,0);
        gd_explorer_graph_details(&c); igEnd(); rlImGuiEnd(); EndDrawing();
        EXPLORER_CHECK(p->inspected_record==owner && undo_draft(&c,panes));
        EXPLORER_CHECK(p->inspected_record==owner && pane_prepare(&c));
        EXPLORER_CHECK(p->selection.cell==-1 && gd_explorer_graph_record(&c)==owner);
        EXPLORER_CHECK(reset_view(&c) && pane_prepare(&c));
    }
    /* Ordinary switches preserve independent canvas, settings and selection. */
    c.pane=&panes[GD_EXPLORER_MDA]; panes[GD_EXPLORER_MDA].camera.target=(Vector2){321,123};
    snprintf(panes[GD_EXPLORER_MDA].mda.filter,sizeof(panes[GD_EXPLORER_MDA].mda.filter),"mechanics/");
    *saved=panes[GD_EXPLORER_MDA];
    EXPLORER_CHECK(change_view(&c,panes,GD_EXPLORER_RESOURCES,NULL));
    EXPLORER_CHECK(change_view(&c,panes,GD_EXPLORER_MDA,NULL));
    EXPLORER_CHECK(!memcmp(saved,&panes[GD_EXPLORER_MDA],sizeof(*saved)));
    /* Source identity maps across typed readers; all case/facet matches remain
       highlighted without changing a destination name/kind filter or query. */
    {
        GD_Model *rules=gd_explorer_model(project,GD_EXPLORER_RESOURCES); int i,source_record=-1;
        const char *reference=NULL;
        for (i=0;i<project->resources->node_count;++i) if (project->resources->nodes[i].kind==GD_RESOURCE_CASE) {
            int r=gd_explorer_resource_record(project->resources,i);
            if (r>=0 && !strncmp(rules->records[r].reference,"mechanics/",10)) { source_record=r; break; }
        }
        EXPLORER_CHECK(source_record>=0); reference=rules->records[source_record].reference;
        EXPLORER_CHECK(change_view(&c,panes,GD_EXPLORER_RESOURCES,reference));
        panes[GD_EXPLORER_RESOURCES].filter[0]=0;
        panes[GD_EXPLORER_RESOURCES].mda.roles[0]=panes[GD_EXPLORER_RESOURCES].mda.roles[1]=panes[GD_EXPLORER_RESOURCES].mda.roles[2]=true;
        matches(&c); EXPLORER_CHECK(c.pane->visible_matches>0 && !c.pane->hidden_matches);
        {
            int total=c.pane->visible_matches;
            c.pane->mda.roles[GD_RESOURCE_CASE]=false; matches(&c);
            EXPLORER_CHECK(c.pane->hidden_matches>0 && c.pane->visible_matches+c.pane->hidden_matches==total);
        }
        snprintf(c.pane->filter,sizeof(c.pane->filter),"no-match-for-this-filter");
        c.pane->camera.target=(Vector2){19,29}; matches(&c);
        EXPLORER_CHECK(!c.pane->visible_matches && c.pane->hidden_matches>0);
        fit_view(&c,1); EXPLORER_CHECK(c.pane->camera.target.x==19 && c.pane->camera.target.y==29);
        EXPLORER_CHECK(!strcmp(c.pane->filter,"no-match-for-this-filter"));
    }
    /* Reset one view, with a modified skill draft and another remembered view. */
    model=gd_explorer_model(project,GD_EXPLORER_SKILLS);
    {
        int i,skill=-1;
        for (i=0;i<model->record_count;++i) if (model->records[i].role==GD_SKILL) { skill=i; break; }
        EXPLORER_CHECK(skill>=0 && gd_draft_rank(project->draft,skill,"37")>0 && gd_explorer_edited(project));
        *saved=panes[GD_EXPLORER_MDA];
        EXPLORER_CHECK(reset_view(&c) && !c.pane->filter[0] && !c.pane->find_reference[0] && c.pane->selection.node==-1);
        EXPLORER_CHECK(!memcmp(saved,&panes[GD_EXPLORER_MDA],sizeof(*saved)) && project->draft->modified_count==1);
        EXPLORER_CHECK(gd_draft_undo(project->draft) && gd_explorer_edited(project));
    }
    for (v=0;v<GD_EXPLORER_VIEWS;++v) { c.pane=&panes[v]; EXPLORER_CHECK(pane_prepare(&c)); }
    /* Allocation failure in replacement layout preparation must retain every
       old snapshot, draft and visited layout, not just a stale pointer. */
    c.pane=&panes[GD_EXPLORER_MDA]; last_good=c.project;
    {
        GD_ExplorerProject *probe; int before=*calls,needed;
        probe=gd_explorer_load(allocator,collections,count); EXPLORER_CHECK(probe && !gd_explorer_error(probe));
        needed=*calls-before; gd_explorer_destroy(probe);
        *fail_at=*calls+needed+2; /* candidate pane array succeeds; first new layout fails */
        EXPLORER_CHECK(!reload_project(&c,panes,collections,count,failure,sizeof(failure)) && c.project==last_good);
        *fail_at=0;
        for (v=0;v<GD_EXPLORER_VIEWS;++v) EXPLORER_CHECK(panes[v].layout && panes[v].prepared);
    }
    /* New case source shifts resource snapshot keys. Retain a chosen case/start
       by source identity, never by coincident record/native-node offsets. */
    {
        GD_ExplorerPane *p=&panes[GD_EXPLORER_RESOURCES]; int i,source_record=-1,noun=-1;
        GD_Model *rules=gd_explorer_model(project,GD_EXPLORER_RESOURCES);
        for (i=0;i<project->resources->node_count;++i) if (project->resources->nodes[i].kind==GD_RESOURCE_CASE) {
            int r=gd_explorer_resource_record(project->resources,i);
            if (r>=0 && !strncmp(rules->records[r].reference,"mechanics/",10)) { p->selection.node=i; source_record=r; break; }
        }
        resource_key=p->selection.node;
        EXPLORER_CHECK(source_record>=0); snprintf(resource_source,sizeof(resource_source),"%s",rules->records[source_record].reference);
        p->paths.count=0; gd_graph_path_start(&p->paths,p->selection.node);
        for (i=0;i<rules->record_count;++i) if (rules->records[i].role==GD_NOUN) { noun=i; break; }
        EXPLORER_CHECK(noun>=0 && snprintf(probe_path,sizeof(probe_path),"%s/a-explorer-probe.md",collections[5].directory)<(int)sizeof(probe_path));
        file=fopen(probe_path,"rb"); if (file) { fclose(file); EXPLORER_CHECK(0); }
        file=fopen(probe_path,"wb"); EXPLORER_CHECK(file);
        fprintf(file,"```gd-rule\ncase explorer-probe:\n    context: ANY\n    Probe:\n        noun: %s\n        requires: ANY\n        preserve: ALL\n```\n",rules->records[noun].reference);
        fclose(file); probe_created=1;
    }
    /* Rejected malformed candidate and restored reload, private fixture only. */
    EXPLORER_CHECK(snprintf(invalid,sizeof(invalid),"%s/zz-explorer-invalid.md",collections[0].directory)<(int)sizeof(invalid));
    file=fopen(invalid,"rb"); if (file) { fclose(file); EXPLORER_CHECK(0); }
    file=fopen(invalid,"wb"); EXPLORER_CHECK(file); fputs("---\nname: [unterminated\n---\n",file); fclose(file); invalid_created=1;
    EXPLORER_CHECK(!reload_project(&c,panes,collections,count,failure,sizeof(failure)) && c.project==last_good);
    EXPLORER_CHECK(remove(invalid)==0); invalid_created=0;
    EXPLORER_CHECK(reload_project(&c,panes,collections,count,failure,sizeof(failure)) && c.project!=last_good);
    project=c.project;
    {
        GD_ExplorerPane *p=&panes[GD_EXPLORER_RESOURCES];
        int record=gd_explorer_resource_record(project->resources,p->selection.node);
        EXPLORER_CHECK(record>=0 && !strcmp(project->resources->model->records[record].reference,resource_source));
        EXPLORER_CHECK(p->selection.node!=resource_key && p->paths.count==1 && p->paths.keys[0]==p->selection.node && !p->paths.lost);
    }
    EXPLORER_CHECK(remove(probe_path)==0); probe_created=0;
    EXPLORER_CHECK(reload_project(&c,panes,collections,count,failure,sizeof(failure)));
    project=c.project;
    for (v=0;v<GD_EXPLORER_VIEWS;++v) { c.pane=&panes[v]; EXPLORER_CHECK(pane_prepare(&c)); explorer_test_render(&c); }
    fprintf(stderr,"Explorer checks passed: six projections; independent switching; multi-match Find; hidden counts; reset isolation; draft preservation; qualified roots; stale-edge recovery and deferred active Undo; validated selection; stderr logging; reload rollback and restoration.\n");
    result=0;
cleanup:
    *fail_at=0;
    if (invalid_created) remove(invalid);
    if (probe_created) remove(probe_path);
    if (window) { rlImGuiShutdown(); CloseWindow(); }
    if (panes) {
        for (v=0;v<GD_EXPLORER_VIEWS;++v) { gd_diagnostics_clear(&panes[v].diagnostics); if (panes[v].layout) allocator.release(allocator.context,panes[v].layout); }
        allocator.release(allocator.context,panes);
    }
    if (saved) allocator.release(allocator.context,saved);
    if (c.analysis) allocator.release(allocator.context,c.analysis);
    if (c.query) allocator.release(allocator.context,c.query);
    gd_explorer_destroy(c.project);
    return result;
}
#undef EXPLORER_CHECK
