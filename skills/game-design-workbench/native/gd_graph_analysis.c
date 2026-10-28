#include "gd_graph_analysis.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

static int feature_order(const void *left, const void *right) {
    const GD_GraphFeature *a=left,*b=right;
    if (a->neighbor!=b->neighbor) return a->neighbor<b->neighbor ? -1:1;
    if (a->owner!=b->owner) return a->owner<b->owner ? -1:1;
    if (a->edge!=b->edge) return a->edge<b->edge ? -1:1;
    return (a->endpoint>b->endpoint)-(a->endpoint<b->endpoint);
}
static int same_projection(const GD_Layout *layout, const unsigned char *shown,
                           int include_inactive, const GD_GraphAnalysis *analysis) {
    int i;
    if (!analysis->initialized || analysis->node_count!=layout->node_count ||
        analysis->edge_count!=layout->edge_count || analysis->include_inactive!=include_inactive) return 0;
    for (i=0;i<layout->node_count;++i)
        if (analysis->keys[i]!=layout->nodes[i].key || analysis->shown[i]!=(shown ? shown[i]!=0:1)) return 0;
    for (i=0;i<layout->edge_count;++i) {
        const GD_LayoutEdge *a=&analysis->edges[i],*b=&layout->edges[i];
        if (a->a!=b->a || a->b!=b->b || a->key!=b->key || a->active!=b->active) return 0;
    }
    return 1;
}
static int add_match(GD_GraphAnalysis *analysis, int left, int right) {
    int a=analysis->features[left].owner,b=analysis->features[right].owner;
    unsigned slot=((unsigned)a*4099u+(unsigned)b)*2654435761u & (GD_GRAPH_ANALYSIS_HASH-1);
    int probe;
    assert(a<b && a>=0 && b<analysis->node_count);
    for (probe=0;probe<GD_GRAPH_ANALYSIS_HASH;++probe) {
        int index;
        if (analysis->work==GD_GRAPH_ANALYSIS_WORK) return 0;
        ++analysis->work;
        index=analysis->slots[slot]-1;
        if (index<0) {
            GD_GraphMatch *pair;
            if (analysis->pair_count==GD_GRAPH_ANALYSIS_PAIRS) return 0;
            index=analysis->pair_count++; analysis->slots[slot]=index+1;
            pair=&analysis->pairs[index]; pair->a=a; pair->b=b; pair->shared=1;
            pair->first_a=left; pair->first_b=right;
            ++analysis->matched[a]; ++analysis->matched[b];
            return 1;
        }
        if (analysis->pairs[index].a==a && analysis->pairs[index].b==b) {
            ++analysis->pairs[index].shared; return 1;
        }
        slot=(slot+1)&(GD_GRAPH_ANALYSIS_HASH-1);
    }
    return 0;
}
int gd_graph_analyze(const GD_Layout *layout, const unsigned char *shown,
                     int include_inactive, GD_GraphAnalysis *analysis) {
    int i,kept,begin;
    assert(layout && analysis && layout->node_count>=0 && layout->node_count<=GD_MAX_RECORDS);
    assert(layout->edge_count>=0 && layout->edge_count<=GD_MAX_PAIRS);
    include_inactive=include_inactive!=0;
    if (same_projection(layout,shown,include_inactive,analysis)) return 0;
    analysis->initialized=1; analysis->include_inactive=include_inactive;
    analysis->node_count=layout->node_count; analysis->edge_count=layout->edge_count;
    analysis->shown_nodes=analysis->shown_edges=analysis->feature_count=analysis->pair_count=analysis->work=0;
    analysis->complete=1; memset(analysis->slots,0,sizeof(analysis->slots));
    for (i=0;i<layout->node_count;++i) {
        analysis->keys[i]=layout->nodes[i].key; analysis->shown[i]=(unsigned char)(shown ? shown[i]!=0:1);
        analysis->shown_nodes+=analysis->shown[i];
        analysis->matched[i]=analysis->neighbors[i]=analysis->incident[i]=0;
    }
    for (i=0;i<layout->edge_count;++i) {
        const GD_LayoutEdge *edge=&layout->edges[i]; GD_GraphFeature *feature;
        assert(edge->a>=0 && edge->a<layout->node_count && edge->b>=0 && edge->b<layout->node_count);
        analysis->edges[i]=*edge;
        if ((!edge->active && !include_inactive) || !analysis->shown[edge->a] || !analysis->shown[edge->b]) continue;
        ++analysis->shown_edges; ++analysis->incident[edge->a];
        if (edge->a!=edge->b) ++analysis->incident[edge->b];
        /* Retain one example edge per owner/neighbor below. No invented path:
           endpoint identifies the exact projected row and its incident side. */
        feature=&analysis->features[analysis->feature_count++];
        feature->owner=edge->a; feature->neighbor=edge->b; feature->edge=i; feature->endpoint=0;
        feature=&analysis->features[analysis->feature_count++];
        feature->owner=edge->b; feature->neighbor=edge->a; feature->edge=i; feature->endpoint=1;
    }
    qsort(analysis->features,(size_t)analysis->feature_count,sizeof(analysis->features[0]),feature_order);
    kept=0;
    for (i=0;i<analysis->feature_count;++i) {
        GD_GraphFeature feature=analysis->features[i];
        if (kept && analysis->features[kept-1].owner==feature.owner && analysis->features[kept-1].neighbor==feature.neighbor) continue;
        analysis->features[kept++]=feature;
        if (feature.owner!=feature.neighbor) ++analysis->neighbors[feature.owner];
    }
    analysis->feature_count=kept;
    for (begin=0;begin<kept;) {
        int end=begin+1,left,right;
        while (end<kept && analysis->features[end].neighbor==analysis->features[begin].neighbor) ++end;
        for (left=begin;left<end;++left) for (right=left+1;right<end;++right) {
            if (!add_match(analysis,left,right)) { analysis->complete=0; return 1; }
        }
        begin=end;
    }
    return 1;
}
int gd_graph_match(const GD_GraphAnalysis *analysis, int node, int peer) {
    int i;
    if (node<0 || peer<0 || node==peer || node>=analysis->node_count || peer>=analysis->node_count) return -1;
    if (node>peer) { int swap=node; node=peer; peer=swap; }
    for (i=0;i<analysis->pair_count;++i) if (analysis->pairs[i].a==node && analysis->pairs[i].b==peer) return i;
    return -1;
}
