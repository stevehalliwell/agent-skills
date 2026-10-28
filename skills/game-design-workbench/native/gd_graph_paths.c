#include "gd_graph_analysis.h"
#include <assert.h>
#include <string.h>

static int target(const GD_GraphAnalysis *analysis, int arc) {
    const GD_LayoutEdge *edge=&analysis->edges[arc/2];
    return arc%2 ? edge->a:edge->b;
}
static int charge(GD_GraphPaths *paths) {
    if (paths->work==paths->limit) { paths->complete=0; return 0; }
    ++paths->work; return 1;
}
const GD_GraphPaths *gd_graph_paths(GD_GraphAnalysis *analysis, const unsigned char *roots, int undirected, int work_limit) {
    GD_GraphPaths *paths=&analysis->paths; int i,j,count=0,changed,virtual_start=analysis->node_count;
    assert(analysis->initialized && work_limit>=0 && work_limit<=GD_GRAPH_ANALYSIS_WORK); undirected=undirected!=0;
    if (paths->valid && paths->undirected==undirected && paths->limit==work_limit) {
        for (i=0;i<analysis->node_count;++i) if (paths->roots[i]!=(roots && roots[i] && analysis->shown[i])) break;
        if (i==analysis->node_count) return paths;
    }
    paths->valid=1; paths->complete=1; paths->undirected=undirected; paths->limit=work_limit;
    paths->root_count=paths->reachable_count=paths->work=0; paths->selection.node=-1; paths->bypass.valid=0;
    for (i=0;i<analysis->node_count;++i) {
        paths->roots[i]=(unsigned char)(roots && roots[i] && analysis->shown[i]); paths->root_count+=paths->roots[i];
        paths->out_head[i]=paths->in_head[i]=paths->dominator[i]=paths->parent_arc[i]=-1;
        paths->reached[i]=0; paths->dominated[i]=paths->incoming[i]=0;
    }
    if (!paths->root_count) return paths;
    /* Prepend backwards so witness traversal follows projected link order. */
    for (i=analysis->edge_count-1;i>=0;--i) {
        const GD_LayoutEdge *edge=&analysis->edges[i];
        if ((!edge->active && !analysis->include_inactive) || !analysis->shown[edge->a] || !analysis->shown[edge->b]) continue;
        for (j=0;j<(undirected ? 2:1);++j) {
            int arc=2*i+j,a=target(analysis,arc^1),b=target(analysis,arc);
            paths->out_next[arc]=paths->out_head[a]; paths->out_head[a]=arc;
            paths->in_next[arc]=paths->in_head[b]; paths->in_head[b]=arc;
        }
    }
    /* Iterative DFS forest is equivalent to one synthetic start connected to
       every chosen start. Real start nodes always have synthetic predecessor. */
    for (i=0;i<analysis->node_count;++i) if (paths->roots[i] && !paths->reached[i]) {
        int top=0; paths->stack[top++]=i; paths->reached[i]=1; paths->cursor[i]=paths->out_head[i];
        while (top) {
            int node=paths->stack[top-1],arc=paths->cursor[node],next;
            if (arc<0) { paths->number[node]=count; paths->order[count++]=node; --top; continue; }
            if (!charge(paths)) return paths;
            paths->cursor[node]=paths->out_next[arc]; next=target(analysis,arc);
            if (!paths->reached[next]) {
                paths->reached[next]=1; paths->parent_arc[next]=arc; paths->cursor[next]=paths->out_head[next];
                assert(top<GD_MAX_RECORDS); paths->stack[top++]=next;
            }
        }
    }
    paths->reachable_count=count; paths->number[virtual_start]=count; paths->dominator[virtual_start]=virtual_start;
    for (i=0;i<analysis->node_count;++i) if (paths->roots[i]) { paths->dominator[i]=virtual_start; paths->parent_arc[i]=-1; }
    /* Cooper/Harvey/Kennedy Figure 3: fixed point, not a single DAG pass.
       Postorder numbers increase toward a dominator. Undefined predecessors
       do not participate until processed; unreachable predecessors never do. */
    do {
        changed=0;
        for (i=count-1;i>=0;--i) {
            int node=paths->order[i],arc,parent=-1;
            if (!charge(paths)) return paths;
            if (paths->roots[node]) continue;
            for (arc=paths->in_head[node];arc>=0;arc=paths->in_next[arc]) {
                int predecessor=target(analysis,arc^1),left,right;
                if (!charge(paths)) return paths;
                if (paths->dominator[predecessor]<0) continue;
                if (parent<0) { parent=predecessor; continue; }
                left=parent; right=predecessor;
                while (left!=right) {
                    if (!charge(paths)) return paths;
                    if (paths->number[left]<paths->number[right]) left=paths->dominator[left];
                    else right=paths->dominator[right];
                    assert(left>=0 && right>=0);
                }
                parent=left;
            }
            assert(parent>=0);
            if (paths->dominator[node]!=parent) { paths->dominator[node]=parent; changed=1; }
        }
    } while (changed);
    /* Dominator parents have greater postorder numbers: aggregate subtree
       sizes once, excluding self, without quadratic ancestor walks. */
    for (i=0;i<count;++i) {
        int node=paths->order[i],parent=paths->dominator[node];
        assert(parent>=0 && paths->number[parent]>paths->number[node]);
        if (parent!=virtual_start) paths->dominated[parent]+=paths->dominated[node]+1;
    }
    memset(paths->stack,0,(size_t)analysis->node_count*sizeof(paths->stack[0]));
    for (i=0;i<analysis->node_count;++i) if (paths->reached[i]) {
        int arc;
        for (arc=paths->in_head[i];arc>=0;arc=paths->in_next[arc]) {
            int predecessor=target(analysis,arc^1);
            if (!charge(paths)) return paths;
            if (predecessor==i || !paths->reached[predecessor] || paths->stack[predecessor]==i+1) continue;
            paths->stack[predecessor]=i+1; ++paths->incoming[i];
        }
    }
    return paths;
}
const GD_PathSelection *gd_graph_path_selection(GD_GraphAnalysis *analysis, int selected) {
    GD_GraphPaths *paths=&analysis->paths; GD_PathSelection *selection=&paths->selection; int node;
    if (!paths->valid || !paths->complete || !paths->root_count || selected<0 || selected>=analysis->node_count || !paths->reached[selected]) return NULL;
    if (selection->node==selected) return selection;
    paths->bypass.valid=0;
    selection->node=selected; selection->ancestor_count=selection->retained=selection->length=selection->candidate_count=0;
    memset(selection->ancestors,0,(size_t)analysis->node_count);
    memset(selection->edges,0,(size_t)analysis->edge_count);
    for (node=paths->dominator[selected];node!=analysis->node_count;node=paths->dominator[node]) {
        assert(node>=0 && selection->ancestor_count<analysis->node_count);
        selection->ancestors[node]=1; ++selection->ancestor_count;
    }
    for (node=selected;paths->parent_arc[node]>=0;) {
        int arc=paths->parent_arc[node];
        assert(selection->length<analysis->node_count);
        selection->edges[arc/2]=1;
        if (selection->retained<64) selection->arcs[selection->retained++]=arc;
        ++selection->length; node=target(analysis,arc^1);
        if (!selection->ancestors[node]) selection->candidates[selection->candidate_count++]=node;
    }
    selection->candidate=selection->candidate_count ? selection->candidates[0]:-1;
    assert(paths->roots[node]); return selection;
}
const GD_PathBypass *gd_graph_path_bypass(GD_GraphAnalysis *analysis, int selected, int candidate, int work_limit) {
    GD_GraphPaths *paths=&analysis->paths; GD_PathBypass *bypass=&paths->bypass;
    const GD_PathSelection *selection=gd_graph_path_selection(analysis,selected);
    int i,node,front=0,back=0;
    assert(work_limit>=0 && work_limit<=GD_GRAPH_ANALYSIS_WORK);
    if (!selection) return NULL;
    if (candidate==-1) candidate=selection->candidate;
    for (i=0;i<selection->candidate_count;++i) if (selection->candidates[i]==candidate) break;
    if (i==selection->candidate_count) return NULL;
    if (bypass->valid && bypass->target==selected && bypass->candidate==candidate && bypass->limit==work_limit) return bypass;
    bypass->valid=bypass->complete=1; bypass->found=bypass->work=bypass->length=bypass->retained=0;
    bypass->target=selected; bypass->candidate=candidate; bypass->limit=work_limit; bypass->fork=bypass->rejoin=-1;
    memset(bypass->edges,0,(size_t)analysis->edge_count);
    /* Reuse completed-analysis scratch, never the original DFS parent arcs.
       This asks vertex avoidance, not the route engine's two-state skill query. */
    for (i=0;i<analysis->node_count;++i) paths->cursor[i]=-2;
    for (i=0;i<analysis->node_count;++i) if (paths->roots[i] && i!=candidate) {
        paths->cursor[i]=-1; paths->stack[back++]=i;
    }
    while (front<back && paths->cursor[selected]==-2) {
        int arc;
        if (bypass->work==work_limit) { bypass->complete=0; return bypass; } ++bypass->work;
        node=paths->stack[front++];
        for (arc=paths->out_head[node];arc>=0;arc=paths->out_next[arc]) {
            int next=target(analysis,arc);
            if (bypass->work==work_limit) { bypass->complete=0; return bypass; } ++bypass->work;
            if (next==candidate || paths->cursor[next]!=-2) continue;
            paths->cursor[next]=arc; assert(back<GD_MAX_RECORDS); paths->stack[back++]=next;
            if (next==selected) break;
        }
    }
    bypass->found=paths->cursor[selected]!=-2;
    /* A complete dominator result and nonmandatory support-path candidate
       guarantee a bypass. Failure here is broken reasoning, not no-route data. */
    assert(bypass->found);
    if (!bypass->found) return bypass;
    for (i=0;i<analysis->node_count;++i) paths->stack[i]=-1;
    for (node=selected;;) {
        int arc=paths->cursor[node]; paths->stack[node]=bypass->length;
        if (arc<0) { assert(paths->roots[node]); break; }
        assert(node!=candidate && bypass->length<analysis->node_count);
        bypass->edges[arc/2]=1;
        if (bypass->retained<64) bypass->arcs[bypass->retained++]=arc;
        ++bypass->length; node=target(analysis,arc^1);
    }
    /* Closest shared support prefix before C; first rejoin after C that also
       follows that prefix on bypass. Position check matters in cyclic graphs
       where common nodes can occur in opposite orders on the two paths. */
    for (node=candidate;paths->parent_arc[node]>=0;) {
        node=target(analysis,paths->parent_arc[node]^1);
        if (paths->stack[node]>=0) { bypass->fork=node; break; }
    }
    for (node=selected;node!=candidate;node=target(analysis,paths->parent_arc[node]^1)) {
        assert(paths->parent_arc[node]>=0);
        if (paths->stack[node]>=0 && (bypass->fork<0 || paths->stack[node]<paths->stack[bypass->fork])) bypass->rejoin=node;
    }
    assert(bypass->rejoin>=0); return bypass;
}
int gd_graph_path_start(GD_PathControls *controls, int key) {
    int low=0,high=controls->count;
    assert(high>=0 && high<=GD_MAX_RECORDS);
    while (low<high) { int middle=low+(high-low)/2; if (controls->keys[middle]<key) low=middle+1; else high=middle; }
    if (low<controls->count && controls->keys[low]==key) return 0;
    if (controls->count==GD_MAX_RECORDS) return -1;
    memmove(controls->keys+low+1,controls->keys+low,(size_t)(controls->count-low)*sizeof(controls->keys[0]));
    controls->keys[low]=key; ++controls->count; return 1;
}
const GD_GraphPaths *gd_graph_paths_prepare(GD_GraphAnalysis *analysis, const GD_PathControls *controls,
                                         const unsigned char *declared, int undirected) {
    unsigned char roots[GD_MAX_RECORDS]; int i;
    if (!controls->enabled) return NULL;
    for (i=0;i<analysis->node_count;++i) {
        int low=0,high=controls->count;
        while (low<high) { int middle=low+(high-low)/2; if (controls->keys[middle]<analysis->keys[i]) low=middle+1; else high=middle; }
        roots[i]=(unsigned char)(controls->declared ? declared && declared[i]:low<controls->count && controls->keys[low]==analysis->keys[i]);
    }
    return gd_graph_paths(analysis,roots,undirected,GD_GRAPH_ANALYSIS_WORK);
}
void gd_graph_paths_remap(GD_PathControls *controls, const GD_Model *old, const GD_Model *fresh) {
    GD_PathControls mapped={0}; int i,j;
    mapped.enabled=controls->enabled; mapped.declared=controls->declared; mapped.lost=controls->lost; mapped.compare=controls->compare;
    for (i=0;i<controls->count;++i) {
        int key=controls->keys[i],found=-1,matches=0,old_matches=0;
        if (key>=0 && key<old->record_count) for (j=0;j<old->record_count;++j)
            old_matches+=!strcmp(old->records[key].reference,old->records[j].reference);
        if (old_matches==1) for (j=0;j<fresh->record_count;++j)
            if (!strcmp(old->records[key].reference,fresh->records[j].reference)) { found=j; ++matches; }
        if (matches==1) gd_graph_path_start(&mapped,found); else ++mapped.lost;
    }
    *controls=mapped;
}
