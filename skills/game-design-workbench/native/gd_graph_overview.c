#include "gd_graph_analysis.h"
#include <assert.h>
#include <string.h>

static int shown_edge(const GD_GraphAnalysis *analysis, int index) {
    const GD_LayoutEdge *edge=&analysis->edges[index];
    return analysis->shown[edge->a] && analysis->shown[edge->b] && (edge->active || analysis->include_inactive);
}
static void add_cell(GD_GraphOverview *overview, int row, int column, int edge) {
    int cell=row*overview->bins+column;
    if (!overview->counts[cell]) overview->first[cell]=edge;
    if (++overview->counts[cell]>overview->peak) overview->peak=overview->counts[cell];
}
void gd_graph_overview_select(GD_GraphAnalysis *analysis, int row, int column) {
    GD_GraphOverview *overview=&analysis->overview; int i;
    assert(overview->valid);
    if (overview->selected_row==row && overview->selected_column==column) return;
    overview->selected_row=row; overview->selected_column=column; overview->selected_count=0;
    memset(overview->selected_nodes,0,(size_t)analysis->node_count);
    memset(overview->selected_edges,0,(size_t)analysis->edge_count);
    if (row<0 || column<0 || row>=overview->bins || column>=overview->bins) return;
    for (i=0;i<analysis->edge_count;++i) if (shown_edge(analysis,i)) {
        const GD_LayoutEdge *edge=&analysis->edges[i]; int a=overview->node_bin[edge->a],b=overview->node_bin[edge->b];
        if ((a==row && b==column) || (overview->undirected && a==column && b==row)) {
            overview->selected_edges[i]=overview->selected_nodes[edge->a]=overview->selected_nodes[edge->b]=1;
            ++overview->selected_count;
        }
    }
}
const GD_GraphOverview *gd_graph_overview(GD_GraphAnalysis *analysis, GD_OverviewControls *controls, int undirected) {
    GD_GraphOverview *overview=&analysis->overview; int i,count=0;
    if (!controls->enabled) return NULL;
    assert(analysis->initialized); undirected=undirected!=0;
    if (!overview->valid || overview->undirected!=undirected) {
        overview->valid=1; overview->undirected=undirected; overview->node_count=analysis->shown_nodes;
        overview->bins=analysis->shown_nodes<GD_OVERVIEW_BINS ? analysis->shown_nodes:GD_OVERVIEW_BINS;
        overview->link_count=overview->peak=0; overview->selected_row=overview->selected_column=-2;
        controls->row=controls->column=-1;
        memset(overview->counts,0,sizeof(overview->counts));
        for (i=0;i<GD_OVERVIEW_BINS*GD_OVERVIEW_BINS;++i) overview->first[i]=-1;
        for (i=0;i<analysis->node_count;++i) {
            overview->node_bin[i]=-1;
            if (analysis->shown[i]) {
                overview->nodes[count]=i;
                overview->node_bin[i]=count*overview->bins/overview->node_count; ++count;
            }
        }
        for (i=0;i<=overview->bins;++i)
            overview->start[i]=overview->bins ? (i*count+overview->bins-1)/overview->bins:0;
        for (i=0;i<analysis->edge_count;++i) if (shown_edge(analysis,i)) {
            const GD_LayoutEdge *edge=&analysis->edges[i]; int a=overview->node_bin[edge->a],b=overview->node_bin[edge->b];
            add_cell(overview,a,b,i); if (undirected && a!=b) add_cell(overview,b,a,i);
            ++overview->link_count;
        }
    }
    gd_graph_overview_select(analysis,controls->row,controls->column);
    return overview;
}
