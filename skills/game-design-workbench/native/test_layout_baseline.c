/* Headless deterministic geometry baseline, not a renderer or solver change.
   Run through scripts/measure-force-layout.ps1. Synthetic dimensions are named
   proxies; production text must be measured by renderer before collision work.
   Full node-pair metrics are offline only. Crossing counts are omitted above
   256 edges; never report a sampled count as an exact total. */
#include "gd_layout.h"
#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
    const char *name;
    int nodes, edges, groups, shape;
    float width, height;
} Fixture;
typedef struct {
    unsigned long long overlaps, crossings;
    float maximum_overlap;
    int crossings_available, finite;
} Metrics;
static GD_Layout layout;
static GD_LayoutPoint previous_positions[GD_MAX_RECORDS];
static const Fixture fixtures[]={
    {"interaction-components",48,48,6,0,160,64},
    /* MDA Free graph draws at 3x layout coordinates; conservative body/badge proxy. */
    {"mda-card-hub",64,63,1,1,70,32},
    {"resource-card-ring",550,550,1,0,180,80},
    {"query-card-hub",96,95,1,1,400,172},
    {"long-label-ring",64,64,1,0,640,80},
    {"coincident-cards",48,48,1,2,250,72},
    {"cross-group-cards",48,48,6,3,250,72},
    {"cycle-multiedges",32,96,1,4,220,120},
    {"dense-cards",64,256,1,0,250,72},
    {"capacity",GD_MAX_RECORDS,GD_MAX_PAIRS,1,0,250,72}
};
static double orientation(const GD_LayoutNode *a,const GD_LayoutNode *b,const GD_LayoutNode *c) {
    return ((double)b->x-a->x)*((double)c->y-a->y)-((double)b->y-a->y)*((double)c->x-a->x);
}
static int crosses(const GD_Layout *value,int first,int second) {
    const GD_LayoutEdge *a=&value->edges[first],*b=&value->edges[second];
    double p,q,r,s;
    if (!a->active || !b->active || a->a==a->b || b->a==b->b ||
        a->a==b->a || a->a==b->b || a->b==b->a || a->b==b->b) return 0;
    p=orientation(&value->nodes[a->a],&value->nodes[a->b],&value->nodes[b->a]);
    q=orientation(&value->nodes[a->a],&value->nodes[a->b],&value->nodes[b->b]);
    r=orientation(&value->nodes[b->a],&value->nodes[b->b],&value->nodes[a->a]);
    s=orientation(&value->nodes[b->a],&value->nodes[b->b],&value->nodes[a->b]);
    /* Strict interior centerline crossings; collinear overlap is not counted. */
    return ((p<0 && q>0)||(p>0 && q<0)) && ((r<0 && s>0)||(r>0 && s<0));
}
static Metrics measure(const GD_Layout *value) {
    Metrics result={0}; int i,j;
    result.finite=1; result.crossings_available=value->edge_count<=256;
    for (i=0;i<value->node_count;++i) {
        const GD_LayoutNode *a=&value->nodes[i];
        if (!isfinite(a->x)||!isfinite(a->y)||!isfinite(a->vx)||!isfinite(a->vy)) result.finite=0;
        for (j=i+1;j<value->node_count;++j) {
            const GD_LayoutNode *b=&value->nodes[j];
            float x=(a->width+b->width)/2-fabsf(a->x-b->x);
            float y=(a->height+b->height)/2-fabsf(a->y-b->y);
            if (x>0 && y>0) { ++result.overlaps; result.maximum_overlap=fmaxf(result.maximum_overlap,fminf(x,y)); }
        }
    }
    if (result.crossings_available) for (i=0;i<value->edge_count;++i) for (j=i+1;j<value->edge_count;++j)
        result.crossings+=crosses(value,i,j);
    return result;
}
static int prepare(const Fixture *fixture) {
    int i,per_group=fixture->nodes/fixture->groups;
    memset(&layout,0,sizeof(layout)); layout.node_count=fixture->nodes;
    layout.edge_count=fixture->edges; layout.group_count=fixture->groups;
    for (i=0;i<fixture->nodes;++i) {
        GD_LayoutNode *node=&layout.nodes[i];
        node->label="synthetic node"; node->key=i; node->group=i/per_group;
        node->width=fixture->width; node->height=fixture->height;
    }
    for (i=0;i<fixture->edges;++i) {
        int a,b;
        if (fixture->shape==1) { a=0; b=1+i%(fixture->nodes-1); }
        else if (fixture->shape==4) { a=i%fixture->nodes; b=i<fixture->nodes*2 ? (a+1)%fixture->nodes:a; }
        else { a=i%fixture->nodes; b=(a/per_group)*per_group+(a%per_group+1+i/fixture->nodes)%per_group; }
        layout.edges[i]=(GD_LayoutEdge){a,b,i,1,0};
        ++layout.nodes[a].degree; ++layout.nodes[b].degree;
    }
    if (!gd_layout_arrange(&layout,GD_LAYOUT_FORCE,GD_SORT_NAME)) return 0;
    for (i=0;i<fixture->nodes;++i) {
        if (fixture->shape==2) layout.nodes[i].x=layout.nodes[i].y=0;
        if (fixture->shape==3) { layout.nodes[i].x=(float)((i%per_group)*80); layout.nodes[i].y=0; }
    }
    return 1;
}
static uint64_t position_hash(void) {
    uint64_t hash=UINT64_C(14695981039346656037); int i,j;
    for (i=0;i<layout.node_count;++i) {
        const unsigned char *x=(const unsigned char *)&layout.nodes[i].x,*y=(const unsigned char *)&layout.nodes[i].y;
        for (j=0;j<(int)sizeof(float);++j) { hash=(hash^x[j])*UINT64_C(1099511628211); hash=(hash^y[j])*UINT64_C(1099511628211); }
    }
    return hash;
}
static int self_check(void) {
    Metrics result;
    memset(&layout,0,sizeof(layout)); layout.node_count=4; layout.edge_count=2;
    layout.nodes[0]=(GD_LayoutNode){.x=0,.y=0,.width=10,.height=10};
    layout.nodes[1]=(GD_LayoutNode){.x=100,.y=100,.width=10,.height=10};
    layout.nodes[2]=(GD_LayoutNode){.x=0,.y=100,.width=10,.height=10};
    layout.nodes[3]=(GD_LayoutNode){.x=100,.y=0,.width=10,.height=10};
    layout.edges[0]=(GD_LayoutEdge){0,1,0,1,0}; layout.edges[1]=(GD_LayoutEdge){2,3,1,1,0};
    result=measure(&layout); if (result.overlaps || result.crossings!=1 || !result.finite) return 0;
    layout.edges[1].active=0; if (measure(&layout).crossings) return 0;
    layout.edges[1].active=1; layout.edges[1].a=1; if (measure(&layout).crossings) return 0;
    layout.edges[1].a=0; layout.edges[1].b=0; if (measure(&layout).crossings) return 0;
    layout.nodes[3].x=5; layout.nodes[3].y=0;
    result=measure(&layout); if (result.overlaps!=1 || result.maximum_overlap!=5) return 0;
    layout.nodes[3].x=10; if (measure(&layout).overlaps) return 0;
    layout.edge_count=257; if (measure(&layout).crossings_available) return 0;
    return 1;
}
static void write_metrics(Metrics result) {
    printf("{\"overlap_pairs\":%llu,\"maximum_overlap\":%.6g,\"centerline_crossings\":",result.overlaps,(double)result.maximum_overlap);
    if (result.crossings_available) printf("%llu",result.crossings); else fputs("null",stdout);
    printf(",\"finite\":%s}",result.finite ? "true":"false");
}
int main(int argc,char **argv) {
    long steps; char *end; int i,j,ok=1;
    if (argc!=2) { fputs("Usage: test-layout-baseline.exe STEPS (1..600)\n",stderr); return 2; }
    errno=0; steps=strtol(argv[1],&end,10);
    if (errno || !*argv[1] || *end || steps<1 || steps>600) { fputs("Steps must be 1..600\n",stderr); return 2; }
    if (!self_check()) { fputs("Geometry metric self-check failed\n",stderr); return 2; }
    printf("{\"schema_version\":1,\"solver\":\"gd_layout_step\",\"steps\":%ld,\"synthetic\":true,\"layout_bytes\":%llu,\"node_bytes\":%llu,\"fixtures\":[",steps,(unsigned long long)sizeof(GD_Layout),(unsigned long long)sizeof(GD_LayoutNode));
    for (i=0;i<(int)(sizeof(fixtures)/sizeof(*fixtures));++i) {
        const Fixture *fixture=&fixtures[i]; Metrics before,after; uint64_t hash; clock_t start; double milliseconds,square=0,maximum=0; int deterministic,tail=0,limited=0,k; unsigned maximum_checks=0;
        if (!prepare(fixture)) { fputs("Fixture prepare failed\n",stderr); return 2; }
        before=measure(&layout); start=clock();
        for (j=0;j<steps;++j) gd_layout_step(&layout,-1);
        milliseconds=1000.0*(double)(clock()-start)/CLOCKS_PER_SEC;
        after=measure(&layout); hash=position_hash();
        if (!prepare(fixture)) return 2;
        for (j=0;j<steps;++j) {
            if (j>=steps-32) for (k=0;k<layout.node_count;++k) previous_positions[k]=(GD_LayoutPoint){layout.nodes[k].x,layout.nodes[k].y};
            gd_layout_step(&layout,-1);
            if (layout.collision_checks>maximum_checks) maximum_checks=layout.collision_checks;
            if (layout.collision_limited) ++limited;
            if (j>=steps-32) for (k=0;k<layout.node_count;++k) {
                double dx=(double)layout.nodes[k].x-previous_positions[k].x,dy=(double)layout.nodes[k].y-previous_positions[k].y,move=dx*dx+dy*dy;
                square+=move; if (move>maximum) maximum=move; ++tail;
            }
        }
        if (maximum_checks>GD_LAYOUT_COLLISION_WORK) ok=0;
        deterministic=hash==position_hash(); if (!deterministic || !after.finite) ok=0;
        printf("%s{\"name\":\"%s\",\"nodes\":%d,\"edges\":%d,\"groups\":%d,\"footprint\":[%.0f,%.0f],\"before\":",i ? ",":"",fixture->name,fixture->nodes,fixture->edges,fixture->groups,(double)fixture->width,(double)fixture->height);
        write_metrics(before); fputs(",\"after\":",stdout); write_metrics(after);
        printf(",\"step_total_ms\":%.3f,\"step_mean_ms\":%.6f,\"maximum_collision_checks\":%u,\"limited_steps\":%d,\"tail_displacement_rms\":%.6f,\"tail_displacement_max\":%.6f,\"deterministic\":%s}",milliseconds,milliseconds/steps,maximum_checks,limited,tail ? sqrt(square/tail):0,sqrt(maximum),deterministic ? "true":"false");
    }
    printf("],\"ok\":%s}\n",ok ? "true":"false");
    return ok ? 0:2;
}
