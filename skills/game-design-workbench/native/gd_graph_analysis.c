#include "gd_graph_analysis.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

const char *const gd_graph_lens_names[GD_GRAPH_LENS_COUNT]={"Any incidence","Same incident role","Changed incident role"};
static int role_order(const GD_GraphFeature *a, const GD_GraphFeature *b) {
    if (a->relation!=b->relation) return a->relation<b->relation ? -1:1;
    return (a->endpoint>b->endpoint)-(a->endpoint<b->endpoint);
}
static int feature_order(const void *left, const void *right) {
    const GD_GraphFeature *a=left,*b=right; int role;
    if (a->neighbor!=b->neighbor) return a->neighbor<b->neighbor ? -1:1;
    if (a->owner!=b->owner) return a->owner<b->owner ? -1:1;
    role=role_order(a,b); if (role) return role;
    return (a->edge>b->edge)-(a->edge<b->edge);
}
static int same_projection(const GD_Layout *layout, const unsigned char *shown,
                           int include_inactive, GD_GraphLens lens, const GD_GraphAnalysis *analysis) {
    int i;
    if (!analysis->initialized || analysis->node_count!=layout->node_count || analysis->lens!=lens ||
        analysis->edge_count!=layout->edge_count || analysis->include_inactive!=include_inactive) return 0;
    for (i=0;i<layout->node_count;++i)
        if (analysis->keys[i]!=layout->nodes[i].key || analysis->shown[i]!=(shown ? shown[i]!=0:1)) return 0;
    for (i=0;i<layout->edge_count;++i) {
        const GD_LayoutEdge *a=&analysis->edges[i],*b=&layout->edges[i];
        if (a->a!=b->a || a->b!=b->b || a->key!=b->key || a->active!=b->active || a->relation!=b->relation) return 0;
    }
    return 1;
}
static int charge(GD_GraphAnalysis *analysis) {
    if (analysis->work==GD_GRAPH_ANALYSIS_WORK) return 0;
    ++analysis->work; return 1;
}
/* One owner group per shared identity. Alternatives retain distinct typed
   incident roles; select one witness, but charge all role-search iterations. */
static int witness(GD_GraphAnalysis *analysis, int *left, int *right) {
    int a=*left,b=*right,end_a=analysis->features[a].owner_end,end_b=analysis->features[b].owner_end;
    if (analysis->lens==GD_GRAPH_ANY) return 1;
    if (analysis->lens==GD_GRAPH_CHANGED_ROLE) {
        if (role_order(&analysis->features[a],&analysis->features[b])) return 1;
        if (a+1<end_a) { *left=a+1; return 1; }
        if (b+1<end_b) { *right=b+1; return 1; }
        return 0;
    }
    while (a<end_a && b<end_b) {
        int order;
        if (!charge(analysis)) return -1;
        order=role_order(&analysis->features[a],&analysis->features[b]);
        if (!order) { *left=a; *right=b; return 1; }
        if (order<0) ++a; else ++b;
    }
    return 0;
}
static int add_match(GD_GraphAnalysis *analysis, int left, int right) {
    int a=analysis->features[left].owner,b=analysis->features[right].owner;
    unsigned slot=((unsigned)a*4099u+(unsigned)b)*2654435761u & (GD_GRAPH_ANALYSIS_HASH-1);
    int probe;
    assert(a<b && a>=0 && b<analysis->node_count);
    for (probe=0;probe<GD_GRAPH_ANALYSIS_HASH;++probe) {
        int index;
        if (!charge(analysis)) return 0;
        index=analysis->slots[slot]-1;
        if (index<0) {
            GD_GraphMatch *pair;
            if (analysis->pair_count==GD_GRAPH_ANALYSIS_PAIRS) return 0;
            index=analysis->pair_count++; analysis->slots[slot]=index+1;
            pair=&analysis->pairs[index]; pair->a=a; pair->b=b; pair->shared=1;
            pair->first_a=left; pair->first_b=right;
            ++analysis->matched[a]; ++analysis->matched[b]; return 1;
        }
        if (analysis->pairs[index].a==a && analysis->pairs[index].b==b) {
            ++analysis->pairs[index].shared; return 1;
        }
        slot=(slot+1)&(GD_GRAPH_ANALYSIS_HASH-1);
    }
    return 0;
}
int gd_graph_analyze(const GD_Layout *layout, const unsigned char *shown,
                     int include_inactive, GD_GraphLens lens, GD_GraphAnalysis *analysis) {
    int i,kept,begin;
    assert(layout && analysis && layout->node_count>=0 && layout->node_count<=GD_MAX_RECORDS);
    assert(layout->edge_count>=0 && layout->edge_count<=GD_MAX_PAIRS);
    assert(lens>=GD_GRAPH_ANY && lens<GD_GRAPH_LENS_COUNT);
    include_inactive=include_inactive!=0;
    if (same_projection(layout,shown,include_inactive,lens,analysis)) return 0;
    analysis->initialized=1; analysis->include_inactive=include_inactive; analysis->lens=lens;
    analysis->node_count=layout->node_count; analysis->edge_count=layout->edge_count;
    analysis->shown_nodes=analysis->shown_edges=analysis->feature_count=analysis->pair_count=analysis->work=0;
    analysis->complete=1; analysis->comparison.valid=0; analysis->paths.valid=0; analysis->foundations.valid=analysis->foundations.projection_valid=analysis->reachability.valid=analysis->shapes.valid=analysis->overview.valid=0; memset(analysis->slots,0,sizeof(analysis->slots));
    for (i=0;i<layout->node_count;++i) {
        analysis->keys[i]=layout->nodes[i].key; analysis->shown[i]=(unsigned char)(shown ? shown[i]!=0:1);
        analysis->shown_nodes+=analysis->shown[i];
        analysis->matched[i]=analysis->neighbors[i]=analysis->incident[i]=0;
    }
    for (i=0;i<layout->edge_count;++i) {
        const GD_LayoutEdge *edge=&layout->edges[i]; GD_GraphFeature *feature;
        assert(edge->a>=0 && edge->a<layout->node_count && edge->b>=0 && edge->b<layout->node_count && edge->relation>=0);
        analysis->edges[i]=*edge;
        if ((!edge->active && !include_inactive) || !analysis->shown[edge->a] || !analysis->shown[edge->b]) continue;
        ++analysis->shown_edges; ++analysis->incident[edge->a];
        if (edge->a!=edge->b) ++analysis->incident[edge->b];
        feature=&analysis->features[analysis->feature_count++];
        *feature=(GD_GraphFeature){edge->a,edge->b,i,0,edge->relation,0};
        feature=&analysis->features[analysis->feature_count++];
        *feature=(GD_GraphFeature){edge->b,edge->a,i,1,edge->relation,0};
    }
    qsort(analysis->features,(size_t)analysis->feature_count,sizeof(analysis->features[0]),feature_order);
    kept=0;
    for (i=0;i<analysis->feature_count;++i) {
        GD_GraphFeature feature=analysis->features[i];
        if (kept && analysis->features[kept-1].owner==feature.owner && analysis->features[kept-1].neighbor==feature.neighbor &&
            !role_order(&analysis->features[kept-1],&feature)) continue;
        analysis->features[kept++]=feature;
    }
    analysis->feature_count=kept;
    for (begin=0;begin<kept;) {
        int end=begin+1;
        while (end<kept && analysis->features[end].neighbor==analysis->features[begin].neighbor && analysis->features[end].owner==analysis->features[begin].owner) ++end;
        analysis->features[begin].owner_end=end;
        if (analysis->features[begin].owner!=analysis->features[begin].neighbor) ++analysis->neighbors[analysis->features[begin].owner];
        begin=end;
    }
    for (begin=0;begin<kept;) {
        int end=begin+1,left,right;
        while (end<kept && analysis->features[end].neighbor==analysis->features[begin].neighbor) ++end;
        for (left=begin;left<end;left=analysis->features[left].owner_end)
            for (right=analysis->features[left].owner_end;right<end;right=analysis->features[right].owner_end) {
                int a=left,b=right,qualified;
                if (!charge(analysis)) { analysis->complete=0; return 1; }
                qualified=witness(analysis,&a,&b);
                if (qualified<0 || (qualified && !add_match(analysis,a,b))) { analysis->complete=0; return 1; }
            }
        begin=end;
    }
    return 1;
}
/* A deduplicated owner group is sorted by relation/endpoint. Unlike global
   discovery, checking one pair needs no quadratic join or truncated result. */
static int comparison_role(const GD_GraphAnalysis *analysis, int first, int relation, int endpoint) {
    int low=first,high=analysis->features[first].owner_end;
    const GD_GraphFeature *feature=&analysis->features[first];
    if (analysis->lens==GD_GRAPH_ANY) return 1;
    if (analysis->lens==GD_GRAPH_CHANGED_ROLE)
        return high>first+1 || feature->relation!=relation || feature->endpoint!=endpoint;
    while (low<high) {
        int middle=low+(high-low)/2,order;
        feature=&analysis->features[middle];
        order=feature->relation!=relation ? (feature->relation<relation ? -1:1):
            (feature->endpoint>endpoint)-(feature->endpoint<endpoint);
        if (!order) return 1;
        if (order<0) low=middle+1; else high=middle;
    }
    return 0;
}
const GD_GraphComparison *gd_graph_compare(GD_GraphAnalysis *analysis, int a, int b) {
    GD_GraphComparison *comparison=&analysis->comparison; int i;
    if (!analysis->initialized || a<0 || b<0 || a==b || a>=analysis->node_count || b>=analysis->node_count ||
        !analysis->shown[a] || !analysis->shown[b]) return NULL;
    if (comparison->valid && comparison->a==a && comparison->b==b) return comparison;
    comparison->a=a; comparison->b=b;
    comparison->shared=comparison->a_only=comparison->b_only=comparison->other_role=0;
    for (i=0;i<analysis->node_count;++i) {
        comparison->first_a[i]=comparison->first_b[i]=-1; comparison->nodes[i]=0;
    }
    for (i=0;i<analysis->feature_count;i=analysis->features[i].owner_end) {
        const GD_GraphFeature *feature=&analysis->features[i];
        assert(feature->owner_end>i && feature->owner_end<=analysis->feature_count);
        if (feature->owner==a) comparison->first_a[feature->neighbor]=i;
        if (feature->owner==b) comparison->first_b[feature->neighbor]=i;
    }
    for (i=0;i<analysis->node_count;++i) {
        int has_a=comparison->first_a[i]>=0,has_b=comparison->first_b[i]>=0;
        comparison->nodes[i]=(unsigned char)(has_a && has_b ? GD_GRAPH_OTHER_ROLE:
            has_a ? GD_GRAPH_A_ONLY:has_b ? GD_GRAPH_B_ONLY:0);
    }
    for (i=0;i<analysis->edge_count;++i) {
        const GD_LayoutEdge *edge=&analysis->edges[i]; int side;
        comparison->edges[i]=0;
        if ((!edge->active && !analysis->include_inactive) || !analysis->shown[edge->a] || !analysis->shown[edge->b]) continue;
        for (side=0;side<2;++side) {
            int owner=side ? edge->b:edge->a,neighbor=side ? edge->a:edge->b,other;
            unsigned char category;
            if (owner!=a && owner!=b) continue;
            other=owner==a ? comparison->first_b[neighbor]:comparison->first_a[neighbor];
            if (other<0) category=(unsigned char)(owner==a ? GD_GRAPH_A_ONLY:GD_GRAPH_B_ONLY);
            else if (comparison_role(analysis,other,edge->relation,side)) {
                category=GD_GRAPH_SHARED; comparison->nodes[neighbor]=GD_GRAPH_SHARED;
            } else category=GD_GRAPH_OTHER_ROLE;
            comparison->edges[i]|=category;
        }
    }
    for (i=0;i<analysis->node_count;++i) {
        comparison->shared+=comparison->nodes[i]==GD_GRAPH_SHARED;
        comparison->a_only+=comparison->nodes[i]==GD_GRAPH_A_ONLY;
        comparison->b_only+=comparison->nodes[i]==GD_GRAPH_B_ONLY;
        comparison->other_role+=comparison->nodes[i]==GD_GRAPH_OTHER_ROLE;
    }
    comparison->valid=1; return comparison;
}
int gd_graph_match(const GD_GraphAnalysis *analysis, int node, int peer) {
    int i;
    if (node<0 || peer<0 || node==peer || node>=analysis->node_count || peer>=analysis->node_count) return -1;
    if (node>peer) { int swap=node; node=peer; peer=swap; }
    for (i=0;i<analysis->pair_count;++i) if (analysis->pairs[i].a==node && analysis->pairs[i].b==peer) return i;
    return -1;
}
