#include "gd_graph_analysis.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>

static int bit(const GD_GraphClosure *closure, int root, int node) {
    return (int)((closure->rows[root][node/64]>>(node%64))&1u);
}
int gd_graph_closure_contains(const GD_GraphClosure *closure, int root, int node) {
    return closure && closure->valid && root>=0 && node>=0 && root<closure->row_count && node<closure->row_count &&
        root!=node && bit(closure,root,node);
}
static int charge(GD_GraphClosure *closure) {
    if (closure->work==closure->limit) return 0;
    ++closure->work; return 1;
}
const GD_GraphClosure *gd_graph_closure(GD_GraphAnalysis *analysis, const unsigned char *eligible,
    const unsigned char *boundary, int relation, int reverse, int undirected, int work_limit) {
    GD_GraphClosure *closure=&analysis->foundations;
    int i,root,same,words=(analysis->node_count+63)/64;
    assert(analysis->initialized && relation>=0 && work_limit>=0 && work_limit<=GD_GRAPH_ANALYSIS_WORK);
    reverse=reverse!=0; undirected=undirected!=0;
    same=closure->valid && closure->relation==relation && closure->reverse==reverse &&
        closure->undirected==undirected && closure->limit==work_limit && closure->row_count==analysis->node_count;
    for (i=0;same && i<analysis->node_count;++i)
        if (closure->eligible[i]!=(analysis->shown[i] && (!eligible || eligible[i])) ||
            closure->boundary[i]!=(boundary && boundary[i]!=0)) same=0;
    if (same) return closure;
    closure->valid=1; closure->complete=1; closure->relation=relation; closure->reverse=reverse;
    closure->undirected=undirected; closure->work=0; closure->limit=work_limit; closure->row_count=analysis->node_count;
    closure->comparison.valid=0; closure->selected=-1; closure->peer_count=0;
    for (i=0;i<analysis->node_count;++i) {
        memset(closure->rows[i],0,(size_t)words*sizeof(uint64_t));
        closure->head[i]=-1; closure->direct[i]=closure->count[i]=closure->users[i]=0;
        closure->finished[i]=closure->cycle[i]=0;
        closure->eligible[i]=(unsigned char)(analysis->shown[i] && (!eligible || eligible[i]));
        closure->boundary[i]=(unsigned char)(boundary && boundary[i]!=0); closure->unknown[i]=closure->boundary[i];
    }
    /* Backward construction preserves projected declaration order in walks. */
    for (i=analysis->edge_count-1;i>=0;--i) {
        const GD_LayoutEdge *edge=&analysis->edges[i]; int side;
        if (edge->relation!=relation || (!edge->active && !analysis->include_inactive) ||
            !closure->eligible[edge->a] || !closure->eligible[edge->b]) continue;
        for (side=0;side<2;++side) if (undirected || side==reverse) {
            int owner=side ? edge->b:edge->a,arc=2*i+side;
            closure->next[arc]=closure->head[owner]; closure->head[owner]=arc;
        }
    }
    for (root=0;root<analysis->node_count;++root) {
        int read=0,count=1;
        if (!closure->eligible[root]) continue;
        closure->queue[0]=root;
        while (read<count) {
            int owner,arc;
            if (!charge(closure)) { closure->complete=0; return closure; }
            owner=closure->queue[read++];
            for (arc=closure->head[owner];arc>=0;arc=closure->next[arc]) {
                const GD_LayoutEdge *edge=&analysis->edges[arc/2]; int target=arc%2 ? edge->a:edge->b;
                if (!charge(closure)) { closure->complete=0; return closure; }
                if (bit(closure,root,target)) continue;
                closure->rows[root][target/64]|=UINT64_C(1)<<(target%64);
                if (owner==root) ++closure->direct[root];
                if (closure->boundary[target]) closure->unknown[root]=1;
                if (target==root) { closure->cycle[root]=(unsigned char)!undirected; continue; }
                ++closure->count[root]; ++closure->users[target];
                assert(count<analysis->node_count || count==1);
                closure->queue[count++]=target;
            }
        }
        closure->finished[root]=1;
    }
    return closure;
}
static int popcount(uint64_t value) {
    value-=(value>>1)&UINT64_C(0x5555555555555555);
    value=(value&UINT64_C(0x3333333333333333))+((value>>2)&UINT64_C(0x3333333333333333));
    value=(value+(value>>4))&UINT64_C(0x0f0f0f0f0f0f0f0f);
    return (int)((value*UINT64_C(0x0101010101010101))>>56);
}
static int shared_count(const GD_GraphClosure *closure, int a, int b) {
    int i,count=0;
    /* At most 4,096*64 word intersections for one selected root. */
    for (i=0;i<(closure->row_count+63)/64;++i) {
        uint64_t common=closure->rows[a][i]&closure->rows[b][i];
        if (a/64==i) common&=~(UINT64_C(1)<<(a%64));
        if (b/64==i) common&=~(UINT64_C(1)<<(b%64));
        count+=popcount(common);
    }
    return count;
}
const GD_GraphComparison *gd_graph_closure_compare(GD_GraphAnalysis *analysis, int a, int b) {
    GD_GraphClosure *closure=&analysis->foundations; GD_GraphComparison *comparison=&closure->comparison;
    int i;
    if (!closure->valid || a<0 || a>=analysis->node_count || !closure->eligible[a] || !closure->finished[a] ||
        b< -1 || b>=analysis->node_count || b==a || (b>=0 && (!closure->eligible[b] || !closure->finished[b]))) return NULL;
    if (closure->selected!=a) {
        closure->selected=a; closure->peer_count=0;
        for (i=0;i<analysis->node_count;++i) {
            closure->shared[i]=0;
            if (i==a || !closure->eligible[i] || !closure->finished[i]) continue;
            closure->shared[i]=shared_count(closure,a,i);
            if (closure->shared[i]) closure->peers[closure->peer_count++]=i;
        }
    }
    if (comparison->valid && comparison->a==a && comparison->b==b) return comparison;
    comparison->a=a; comparison->b=b; comparison->shared=comparison->a_only=comparison->b_only=comparison->other_role=0;
    for (i=0;i<analysis->node_count;++i) {
        int has_a=gd_graph_closure_contains(closure,a,i),has_b=gd_graph_closure_contains(closure,b,i);
        unsigned category=has_a && has_b ? GD_GRAPH_SHARED:has_a ? GD_GRAPH_A_ONLY:has_b ? GD_GRAPH_B_ONLY:0;
        comparison->nodes[i]=(unsigned char)category;
        comparison->shared+=category==GD_GRAPH_SHARED; comparison->a_only+=category==GD_GRAPH_A_ONLY; comparison->b_only+=category==GD_GRAPH_B_ONLY;
    }
    for (i=0;i<analysis->edge_count;++i) {
        const GD_LayoutEdge *edge=&analysis->edges[i]; int side;
        comparison->edges[i]=0;
        if (edge->relation!=closure->relation || (!edge->active && !analysis->include_inactive) ||
            !closure->eligible[edge->a] || !closure->eligible[edge->b]) continue;
        for (side=0;side<2;++side) if (closure->undirected || side==closure->reverse) {
            int owner=side ? edge->b:edge->a,has_a=owner==a || bit(closure,a,owner);
            int has_b=b>=0 && (owner==b || bit(closure,b,owner));
            comparison->edges[i]|=(unsigned char)(has_a && has_b ? GD_GRAPH_SHARED:has_a ? GD_GRAPH_A_ONLY:has_b ? GD_GRAPH_B_ONLY:0);
        }
    }
    comparison->valid=1; return comparison;
}
const GD_GraphComparison *gd_graph_closure_choose(GD_GraphAnalysis *analysis, int a, int *peer) {
    GD_GraphClosure *closure=&analysis->foundations;
    if (!closure->valid || a<0 || a>=analysis->node_count || !closure->eligible[a] || !closure->finished[a]) return NULL;
    if (closure->selected!=a && !gd_graph_closure_compare(analysis,a,-1)) return NULL;
    if (*peer<0 || *peer>=analysis->node_count || !closure->shared[*peer])
        *peer=closure->peer_count ? closure->peers[0]:-1;
    return gd_graph_closure_compare(analysis,a,*peer);
}
static int pair_order(const void *left, const void *right) {
    uint64_t a=*(const uint64_t *)left,b=*(const uint64_t *)right;
    return (a>b)-(a<b);
}
static int projected_pair(const GD_GraphClosure *closure, int owner, int target) {
    uint64_t key=(uint64_t)owner*GD_MAX_RECORDS+(unsigned)target;
    int low=0,high=closure->projected_count;
    while (low<high) {
        int middle=low+(high-low)/2;
        if (closure->projected[middle]==key) return 1;
        if (closure->projected[middle]<key) low=middle+1; else high=middle;
    }
    return 0;
}
const GD_GraphClosure *gd_graph_foundations(const GD_Model *model, GD_GraphAnalysis *analysis, int authored_direction) {
    unsigned char eligible[GD_MAX_RECORDS]={0},boundary[GD_MAX_RECORDS]={0};
    int map[GD_MAX_RECORDS],i,count=0;
    for (i=0;i<model->record_count;++i) map[i]=-1;
    for (i=0;i<analysis->node_count;++i) {
        int key=analysis->keys[i];
        if (key>=0 && key<model->record_count && model->records[key].role==GD_SKILL && analysis->shown[i]) {
            eligible[i]=1; map[key]=i; ++count;
        }
    }
    if (!count) return NULL;
    /* Sorted projected pairs avoid a node-by-source-edge scan every frame.
       Raw duplicates/source rows remain in analysis->edges and model->edges. */
    {
        GD_GraphClosure *closure=&analysis->foundations;
        if (!closure->projection_valid || closure->projected_direction!=authored_direction) {
            closure->projected_count=0; closure->projected_direction=authored_direction;
            for (i=0;i<analysis->edge_count;++i) {
                const GD_LayoutEdge *edge=&analysis->edges[i];
                int owner=authored_direction ? edge->a:edge->b,target=authored_direction ? edge->b:edge->a;
                if (edge->relation==GD_PREREQUISITES && eligible[owner] && eligible[target] && (edge->active || analysis->include_inactive))
                    closure->projected[closure->projected_count++]=(uint64_t)owner*GD_MAX_RECORDS+(unsigned)target;
            }
            qsort(closure->projected,(size_t)closure->projected_count,sizeof(uint64_t),pair_order);
            closure->projection_valid=1;
        }
        for (i=0;i<model->edge_count;++i) {
            const GD_Edge *edge=&model->edges[i]; int owner=map[edge->source];
            if (owner<0 || edge->field!=GD_PREREQUISITES) continue;
            if (edge->target<0 || model->records[edge->target].role!=GD_SKILL || map[edge->target]<0 ||
                !projected_pair(closure,owner,map[edge->target])) boundary[owner]=1;
        }
    }
    return gd_graph_closure(analysis,eligible,boundary,GD_PREREQUISITES,!authored_direction,0,GD_GRAPH_ANALYSIS_WORK);
}
