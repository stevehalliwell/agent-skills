/* Paging-only transitions: no window, source writes or implicit reader Run. */
#ifndef TEST_EXPLORER_EXTENDED_PAGING_H
#define TEST_EXPLORER_EXTENDED_PAGING_H
static int explorer_extended_paging_checks(GD_ExplorerContext *c,GD_Layout *saved,int *fail_at,int *calls) {
    GD_ExplorerPane *p;GD_Allocator allocator=c->project->allocator;int checks=0,i,j,result=0;char message[256],value[64];
#define EXT_PAGE_CHECK(test) do{++checks;if(!(test)){fprintf(stderr,"Extended paging failed at line %d: %s\n",__LINE__,#test);goto cleanup;}}while(0)
    /* Sparse native identities never become page-relative selectors. */
    for(i=0;i<3;++i){GD_EvidencePage page=gd_evidence_page(3,1,i);int ordinal=0,selected=-1;
        for(j=0;j<1000;++j)if(j==67 || j==153 || j==998)if(gd_evidence_page_take(&page,&ordinal))selected=j;
        EXT_PAGE_CHECK(ordinal==3 && selected==(i==0 ? 67:i==1 ? 153:998));
    }
    {GD_EvidencePage page=gd_evidence_page(INT_MAX,64,INT_MAX);int ordinal=INT_MAX-1;EXT_PAGE_CHECK(gd_evidence_page_take(&page,&ordinal) && ordinal==INT_MAX);}
    p=c->pane=&c->panes[GD_EXPLORER_ACTORS];*c->active_view=p->view;
    EXT_PAGE_CHECK(pane_prepare(c) && gd_explorer_structure_run(c));
    {GD_ExplorerStructureState *state=p->structure_state;GD_StructureResult *retained=state->result;Camera2D camera=p->camera;GD_ExplorerSelection selection=p->selection;
        int total=gd_explorer_structure_row_count(state),maximum=gd_explorer_presentation_maximum(c,"structure_page"),allocations=*calls,changed;
        uint64_t pending=viewer_form_hash(1,GD_FORM_STRUCTURE,&state->pending),applied=viewer_form_hash(1,GD_FORM_STRUCTURE,&state->applied);
        memcpy(saved,p->layout,sizeof(*saved));EXT_PAGE_CHECK(total==retained->query->shown_nodes && total>0 && maximum==(total-1)/64);
        fprintf(stderr,"Retained structural paging rows: %d\n",total);
        snprintf(value,sizeof(value),"%d",maximum);EXT_PAGE_CHECK(gd_explorer_presentation_set(c,"structure_page",value,message,sizeof(message)) && state->page==maximum);
        snprintf(value,sizeof(value),"%d",maximum+1);EXT_PAGE_CHECK(!gd_explorer_presentation_set(c,"structure_page",value,message,sizeof(message)) && state->page==maximum);
        EXT_PAGE_CHECK(*calls==allocations && p->structure_state==state && state->result==retained && !memcmp(saved,p->layout,sizeof(*saved)) && !memcmp(&camera,&p->camera,sizeof(camera)) && !memcmp(&selection,&p->selection,sizeof(selection)));
        EXT_PAGE_CHECK(pending==viewer_form_hash(1,GD_FORM_STRUCTURE,&state->pending) && applied==viewer_form_hash(1,GD_FORM_STRUCTURE,&state->applied));
        state->marked_node_page=2;state->similarity_feature_page=1;state->matrix_case_pages[0]=3;state->reach_member_pages[0]=4;state->support_pages[1][0]=5;
        *fail_at=*calls+1;changed=gd_explorer_structure_run(c);*fail_at=0;
        EXT_PAGE_CHECK(!changed && p->structure_state==state && state->marked_node_page==2 && state->similarity_feature_page==1 && state->matrix_case_pages[0]==3 && state->reach_member_pages[0]==4 && state->support_pages[1][0]==5);
        EXT_PAGE_CHECK(gd_explorer_structure_run(c));state=p->structure_state;
        EXT_PAGE_CHECK(!state->page && !state->marked_node_page && !state->similarity_feature_page && !state->matrix_case_pages[0] && !state->reach_member_pages[0] && !state->support_pages[1][0]);
        /* Synthetic metadata cardinality; no invented authored similarity table. */
        {GD_SimilarityProjection *synthetic=allocator.allocate(allocator.context,sizeof(*synthetic)),*prior=state->result->similarity;int maximum,valid,invalid,page;
            EXT_PAGE_CHECK(synthetic);memset(synthetic,0,sizeof(*synthetic));synthetic->entry_count=129;state->result->similarity=synthetic;allocations=*calls;
            maximum=gd_explorer_presentation_maximum(c,"similarity_page");valid=gd_explorer_presentation_set(c,"similarity_page","2",message,sizeof(message));page=state->similarity_page;
            invalid=gd_explorer_presentation_set(c,"similarity_page","3",message,sizeof(message));
            state->result->similarity=prior;state->similarity_page=state->page=0;allocator.release(allocator.context,synthetic);
            EXT_PAGE_CHECK(maximum==2 && valid && !invalid && page==2 && *calls==allocations && !memcmp(&camera,&p->camera,sizeof(camera)) && !memcmp(&selection,&p->selection,sizeof(selection)));
        }
        state->pending.kind=GD_STRUCTURE_DECLARATIONS;EXT_PAGE_CHECK(gd_explorer_structure_run(c));state=p->structure_state;
        total=state->result->gaps->rows-state->result->gaps->counts[GD_DECLARATION_RESOLVED];
        EXT_PAGE_CHECK(gd_explorer_structure_row_count(state)==total && gd_explorer_presentation_maximum(c,"structure_page")==(total ? (total-1)/64:0));
    }
    /* Synthetic source-group window exercises native slots above first64.
       Restore graph metadata before any renderer or source serialization. */
    p=c->pane=&c->panes[GD_EXPLORER_NOUNS];*c->active_view=p->view;
    EXT_PAGE_CHECK(pane_prepare(c) && gd_explorer_presentation_set(c,"layout","free",message,sizeof(message)));
    {GD_LayoutEdge edges[130];int count=p->layout->edge_count,valid;
        memcpy(edges,p->layout->edges,sizeof(edges));p->layout->edge_count=130;
        memset(c->analysis->overview.selected_edges,0,sizeof(c->analysis->overview.selected_edges));
        for(i=0;i<130;++i){p->layout->edges[i].key=i;c->analysis->overview.selected_edges[i]=1;}
        p->overview_source_page=1;valid=noun_source_page(c,70)==&p->noun_source_pages[6] && p->noun_source_pairs[6]==70;
        p->overview_source_page=2;valid=valid && noun_source_page(c,129)==&p->noun_source_pages[1] && p->noun_source_pairs[1]==129;
        p->layout->edge_count=count;memcpy(p->layout->edges,edges,sizeof(edges));p->overview_source_page=0;
        memset(p->noun_source_pages,0,sizeof(p->noun_source_pages));memset(p->noun_source_pairs,0,sizeof(p->noun_source_pairs));memset(c->analysis->overview.selected_edges,0,sizeof(c->analysis->overview.selected_edges));
        EXT_PAGE_CHECK(valid);
    }
    /* Full retained parent chain, not a fabricated execution trace. */
    {int *arcs=c->panes[GD_EXPLORER_ACTORS].structure_state->witness_arcs,length,work;const GD_GraphClosure *closure;GD_EvidencePage page;
        memset(saved,0,sizeof(*saved));saved->node_count=130;saved->edge_count=129;
        for(i=0;i<130;++i)saved->nodes[i].key=i;
        for(i=0;i<129;++i)saved->edges[i]=(GD_LayoutEdge){i,i+1,i,1,1};
        gd_graph_analyze(saved,NULL,0,GD_GRAPH_ANY,c->analysis);
        closure=gd_graph_closure(c->analysis,NULL,NULL,1,0,0,GD_GRAPH_ANALYSIS_WORK);
        EXT_PAGE_CHECK(closure && closure->complete && gd_graph_closure_compare(c->analysis,0,1));work=c->analysis->work;
        length=gd_graph_closure_path(c->analysis,closure,0,129,arcs,GD_MAX_RECORDS);EXT_PAGE_CHECK(length==129);page=gd_evidence_page(length,64,2);
        EXT_PAGE_CHECK(page.first==128 && page.end==129 && arcs[length-1]/2==0 && arcs[length-1-page.first]/2==128 && c->analysis->work==work);
        c->analysis->initialized=0;
    }
    result=1;
cleanup:
    *fail_at=0;fprintf(stderr,"Extended paging: %d checks; structure cursor arrays %llu bytes; participants %llu bytes per rule/query owner\n",checks,
        (unsigned long long)(sizeof(((GD_ExplorerStructureState *)0)->matrix_case_pages)+sizeof(((GD_ExplorerStructureState *)0)->reach_member_pages)+sizeof(((GD_ExplorerStructureState *)0)->support_pages)),
        (unsigned long long)sizeof(((GD_ExplorerRuleState *)0)->case_participant_pages));
#undef EXT_PAGE_CHECK
    return result;
}
#endif
