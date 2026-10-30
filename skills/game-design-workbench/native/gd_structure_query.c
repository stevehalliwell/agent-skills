#include "gd_structure_query.h"
#include <string.h>

static const char *const names[]={"direct-connections","related-structure","reachability","structural-dominators","support-bypass","role-shapes","adjacency"};
const char *gd_structure_name(GD_StructureKind kind) { return kind>=0 && kind<GD_STRUCTURE_COUNT ? names[kind]:NULL; }
int gd_structure_kind(const char *name) { int i; if (name) for (i=0;i<GD_STRUCTURE_COUNT;++i) if (!strcmp(name,names[i])) return i; return -1; }
static int relation_selected(const GD_StructureRequest *request, int relation) {
    int i; if (!request->relation_count) return 1;
    for (i=0;i<request->relation_count;++i) if (request->relations[i]==relation) return 1;
    return 0;
}
static int path_question(GD_StructureKind kind) { return kind==GD_STRUCTURE_REACHABILITY || kind==GD_STRUCTURE_DOMINATORS || kind==GD_STRUCTURE_BYPASS; }
static int selected(const GD_Layout *layout, const unsigned char *shown, int node) { return node>=0 && node<layout->node_count && (!shown || shown[node]); }
int gd_structure_request_valid(const GD_Layout *layout, const unsigned char *shown, const GD_StructureRequest *request) {
    int i;
    if (!layout || !request || layout->node_count<0 || layout->node_count>GD_MAX_RECORDS || layout->edge_count<0 || layout->edge_count>GD_MAX_PAIRS || request->kind<0 || request->kind>=GD_STRUCTURE_COUNT || request->direction<GD_STRUCTURE_FORWARD || request->direction>GD_STRUCTURE_BOTH || request->lens<GD_GRAPH_ANY || request->lens>=GD_GRAPH_LENS_COUNT || request->source<-1 || request->target<-1 || request->avoid<-1 || request->root_count<0 || request->root_count>GD_STRUCTURE_SELECTORS || request->relation_count<0 || request->relation_count>GD_STRUCTURE_KINDS || request->work_limit<0 || request->work_limit>GD_GRAPH_ANALYSIS_WORK) return 0;
    if (!path_question(request->kind) && (request->direction!=GD_STRUCTURE_FORWARD || request->root_count)) return 0;
    if (request->kind!=GD_STRUCTURE_RELATED && request->lens!=GD_GRAPH_ANY) return 0;
    if (request->kind!=GD_STRUCTURE_BYPASS && request->avoid>=0) return 0;
    if (!path_question(request->kind) && request->kind!=GD_STRUCTURE_RELATED && request->target>=0) return 0;
    if ((request->kind==GD_STRUCTURE_SHAPES || request->kind==GD_STRUCTURE_ADJACENCY) && request->source>=0) return 0;
    if ((request->source>=0 && !selected(layout,shown,request->source)) || (request->target>=0 && !selected(layout,shown,request->target)) || (request->avoid>=0 && !selected(layout,shown,request->avoid))) return 0;
    for (i=0;i<request->root_count;++i) if (!selected(layout,shown,request->roots[i])) return 0;
    for (i=0;i<request->relation_count;++i) if (request->relations[i]<0 || request->relations[i]>=GD_STRUCTURE_KINDS) return 0;
    for (i=0;i<layout->edge_count;++i) if (layout->edges[i].a<0 || layout->edges[i].a>=layout->node_count || layout->edges[i].b<0 || layout->edges[i].b>=layout->node_count || layout->edges[i].relation<0 || layout->edges[i].relation>=GD_STRUCTURE_KINDS) return 0;
    if (path_question(request->kind) && request->source<0 && !request->root_count) return 0;
    if (request->kind==GD_STRUCTURE_RELATED && ((request->source<0)!=(request->target<0) || (request->source==request->target && request->source>=0))) return 0;
    if (request->kind==GD_STRUCTURE_BYPASS && request->target<0) return 0;
    return 1;
}
static void summary(GD_StructureQuery *query) {
    int i; GD_GraphAnalysis *analysis=&query->analysis;
    query->shown_nodes=analysis->shown_nodes; query->shown_edges=analysis->shown_edges;
    for (i=0;i<analysis->node_count;++i) {
        query->nodes[i].partners=analysis->neighbors[i]; query->nodes[i].incident=analysis->incident[i];
        if (analysis->shown[i] && !analysis->incident[i]) ++query->isolated;
        if (query->request.incidence) query->nodes[i].incoming=query->nodes[i].outgoing=-1;
    }
    for (i=0;i<analysis->edge_count;++i) if (analysis->edges[i].active && analysis->shown[analysis->edges[i].a] && analysis->shown[analysis->edges[i].b]) {
        const GD_LayoutEdge *original=&query->layout->edges[i]; uint64_t kind=(uint64_t)1<<original->relation;
        if (!query->request.incidence) { ++query->nodes[original->a].outgoing; ++query->nodes[original->b].incoming; }
        query->nodes[original->a].relation_mask|=kind; query->nodes[original->b].relation_mask|=kind;
    }
    for (i=0;i<analysis->node_count;++i) { uint64_t kinds=query->nodes[i].relation_mask; while (kinds) { kinds&=kinds-1; ++query->nodes[i].kinds; } }
    query->construction_rows=analysis->node_count+analysis->edge_count+analysis->feature_count;
}
static void mark_arcs(GD_StructureQuery *query, const int *arcs, int count) {
    int i;
    for (i=0;i<count;++i) { const GD_LayoutEdge *edge=&query->analysis.edges[arcs[i]/2]; query->marked_edges[arcs[i]/2]=1; query->marked_nodes[edge->a]=query->marked_nodes[edge->b]=1; }
}
GD_StructureQuery *gd_structure_query_build(GD_Allocator allocator, const GD_Layout *layout, const unsigned char *shown, const int *roles, const GD_StructureRequest *request) {
    GD_StructureQuery *query; GD_Layout *scope; int i,discovery;
    if (!allocator.allocate || !allocator.release || !gd_structure_request_valid(layout,shown,request)) return NULL;
    query=allocator.allocate(allocator.context,sizeof(*query)); if (!query) return NULL;
    memset(query,0,sizeof(*query)); query->allocator=allocator; query->layout=layout; query->request=*request; query->request.edge_mask=NULL;
    scope=allocator.allocate(allocator.context,sizeof(*scope)); if (!scope) { gd_structure_query_destroy(query); return NULL; }
    memcpy(scope,layout,sizeof(*scope)); query->undirected=request->incidence || request->direction==GD_STRUCTURE_BOTH;
    for (i=0;i<scope->edge_count;++i) {
        GD_LayoutEdge *edge=&scope->edges[i]; edge->active=(!request->edge_mask || request->edge_mask[i]) && relation_selected(request,edge->relation) && (edge->active || request->include_inactive);
        if (request->direction==GD_STRUCTURE_BACKWARD) { int node=edge->a; edge->a=edge->b; edge->b=node; }
    }
    discovery=request->kind==GD_STRUCTURE_RELATED && request->source<0;
    gd_graph_analyze_bounded(scope,shown,0,request->lens,discovery ? request->work_limit:0,&query->analysis);
    allocator.release(allocator.context,scope); summary(query); query->complete=query->available=1;
    if (request->source>=0) query->roots[request->source]=1;
    for (i=0;i<request->root_count;++i) query->roots[request->roots[i]]=1;
    if (request->kind==GD_STRUCTURE_RELATED) {
        if (discovery) { query->complete=query->analysis.complete; query->work=query->analysis.work; }
        else { const GD_GraphComparison *comparison=gd_graph_compare(&query->analysis,request->source,request->target); if (comparison) { memcpy(query->marked_nodes,comparison->nodes,sizeof(query->marked_nodes)); memcpy(query->marked_edges,comparison->edges,sizeof(query->marked_edges)); } else query->available=0; }
    } else if (request->kind==GD_STRUCTURE_REACHABILITY) {
        const GD_GraphClosure *closure=gd_graph_closure_into(&query->analysis,&query->analysis.reachability,NULL,NULL,-1,0,query->undirected,request->work_limit);
        query->complete=closure->complete; query->work=closure->work;
        for (i=0;i<query->analysis.node_count;++i) if (query->roots[i]) { int target; for (target=0;target<query->analysis.node_count;++target) { ++query->selection_rows; if (gd_graph_closure_contains(closure,i,target)) query->marked_nodes[target]=1; } }
        if (request->target>=0 && request->source>=0 && gd_graph_closure_contains(closure,request->source,request->target)) {
            int arcs[64],count; gd_graph_closure_compare_into(&query->analysis,&query->analysis.reachability,request->source,-1);
            count=gd_graph_closure_path(&query->analysis,&query->analysis.reachability,0,request->target,arcs,64); if (count>0) mark_arcs(query,arcs,count<64 ? count:64);
        }
    } else if (request->kind==GD_STRUCTURE_DOMINATORS || request->kind==GD_STRUCTURE_BYPASS) {
        const GD_GraphPaths *paths=gd_graph_paths(&query->analysis,query->roots,query->undirected,request->work_limit);
        query->available=paths->valid; query->complete=paths->complete; query->work=paths->work;
        if (query->available && request->target>=0) {
            const GD_PathSelection *selection=gd_graph_path_selection(&query->analysis,request->target);
            if (selection) { memcpy(query->marked_nodes,selection->ancestors,sizeof(query->marked_nodes)); mark_arcs(query,selection->arcs,selection->retained); }
            if (request->kind==GD_STRUCTURE_BYPASS) { const GD_PathBypass *bypass=gd_graph_path_bypass(&query->analysis,request->target,request->avoid,request->work_limit-paths->work); if (bypass) { query->complete=query->complete && bypass->complete; query->work+=bypass->work; if (bypass->found) { for (i=0;i<query->analysis.edge_count;++i) if (bypass->edges[i]) query->marked_edges[i]=1; } } else { query->available=0; query->complete=0; } }
        }
    } else if (request->kind==GD_STRUCTURE_SHAPES) {
        const GD_GraphShapes *shapes=gd_graph_shapes(&query->analysis,roles,request->work_limit); query->available=shapes->valid; query->complete=shapes->complete; query->work=shapes->work;
    } else if (request->kind==GD_STRUCTURE_ADJACENCY) {
        GD_OverviewControls controls={0}; controls.enabled=1; gd_graph_overview(&query->analysis,&controls,query->undirected);
    } else if (request->source>=0) {
        query->marked_nodes[request->source]=1;
        for (i=0;i<query->analysis.edge_count;++i) if (query->analysis.edges[i].active && query->analysis.shown[query->analysis.edges[i].a] && query->analysis.shown[query->analysis.edges[i].b] && (query->analysis.edges[i].a==request->source || query->analysis.edges[i].b==request->source)) { const GD_LayoutEdge *edge=&query->analysis.edges[i]; query->marked_edges[i]=1; query->marked_nodes[edge->a]=query->marked_nodes[edge->b]=1; }
    }
    if (!query->available) query->complete=0;
    return query;
}
void gd_structure_query_destroy(GD_StructureQuery *query) { if (query) query->allocator.release(query->allocator.context,query); }
