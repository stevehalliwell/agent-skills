/* Private window-test fixture setup, not public Reset view behavior. These
   scenarios need fresh analytical state; public reset preserves that state. */
#ifndef TEST_EXPLORER_FRESH_PANE_H
#define TEST_EXPLORER_FRESH_PANE_H
static int explorer_test_fresh_pane(GD_ExplorerContext *c){
    GD_ExplorerPane *p=c->pane,*candidate;GD_ExplorerContext target=*c;GD_Allocator a=c->project->allocator;
    if(is_graph(p->view) && gd_explorer_graph_commit(c)<0)return 0;
    candidate=a.allocate(a.context,sizeof(*candidate));if(!candidate)return 0;
    memset(candidate,0,sizeof(*candidate));pane_defaults(candidate,p->view);target.pane=candidate;
    if(!pane_prepare(&target)){pane_release_layout(a,candidate);gd_diagnostics_clear(&candidate->diagnostics);a.release(a.context,candidate);return 0;}
    if(p->view==GD_EXPLORER_MDA)gd_mda_query(gd_explorer_model(c->project,p->view),c->project->mda,-1,GD_MDA_BOTH);
    pane_release_layout(a,p);gd_diagnostics_clear(&p->diagnostics);gd_layout_focus_destroy(p->distance_focus);gd_resource_routes_destroy(p->resource_routes);
    *p=*candidate;a.release(a.context,candidate);c->analysis->initialized=0;return 1;
}
#endif
