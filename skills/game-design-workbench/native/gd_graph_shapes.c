#include "gd_graph_analysis.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

static int feature_compare(const void *left, const void *right) {
    const GD_ShapeFeature *a=left,*b=right;
    if (a->owner!=b->owner) return a->owner<b->owner ? -1:1;
    if (a->relation!=b->relation) return a->relation<b->relation ? -1:1;
    if (a->endpoint!=b->endpoint) return a->endpoint<b->endpoint ? -1:1;
    return (a->color>b->color)-(a->color<b->color);
}
static int charge(GD_GraphShapes *shapes) {
    if (shapes->work==shapes->limit) { shapes->complete=0; return 0; }
    ++shapes->work; return 1;
}
static int signature_compare(GD_GraphShapes *shapes, int a, int b) {
    int i,count_a=shapes->begin[a+1]-shapes->begin[a],count_b=shapes->begin[b+1]-shapes->begin[b];
    if (!charge(shapes)) return 0;
    if (shapes->previous[a]!=shapes->previous[b]) return shapes->previous[a]<shapes->previous[b] ? -1:1;
    if (count_a!=count_b) return count_a<count_b ? -1:1;
    for (i=0;i<count_a;++i) {
        const GD_ShapeFeature *x=&shapes->features[shapes->begin[a]+i],*y=&shapes->features[shapes->begin[b]+i];
        if (!charge(shapes)) return 0;
        if (x->relation!=y->relation) return x->relation<y->relation ? -1:1;
        if (x->endpoint!=y->endpoint) return x->endpoint<y->endpoint ? -1:1;
        if (x->color!=y->color) return x->color<y->color ? -1:1;
    }
    return 0;
}
static int signature_sort(GD_GraphShapes *shapes, int count) {
    int width,first;
    for (width=1;width<count;width*=2) {
        for (first=0;first<count;first+=2*width) {
            int middle=first+width<count ? first+width:count,end=first+2*width<count ? first+2*width:count;
            int a=first,b=middle,out=first;
            while (a<middle && b<end) {
                int order=signature_compare(shapes,shapes->order[a],shapes->order[b]);
                if (!shapes->complete) return 0;
                shapes->merge[out++]=order<=0 ? shapes->order[a++]:shapes->order[b++];
            }
            while (a<middle) shapes->merge[out++]=shapes->order[a++];
            while (b<end) shapes->merge[out++]=shapes->order[b++];
        }
        memcpy(shapes->order,shapes->merge,(size_t)count*sizeof(int));
    }
    return 1;
}
const GD_GraphShapes *gd_graph_shapes(GD_GraphAnalysis *analysis, const int *roles, int work_limit) {
    GD_GraphShapes *shapes=&analysis->shapes; int i,round,count=0,same;
    assert(analysis->initialized && work_limit>=0 && work_limit<=GD_GRAPH_ANALYSIS_WORK);
    same=shapes->valid && shapes->limit==work_limit && shapes->node_count==analysis->node_count;
    for (i=0;same && i<analysis->node_count;++i) if (shapes->roles[i]!=(roles ? roles[i]:0)) same=0;
    if (same) return shapes;
    shapes->valid=shapes->complete=1; shapes->work=0; shapes->limit=work_limit; shapes->node_count=analysis->node_count; shapes->family_count=0;
    for (i=0;i<analysis->node_count;++i) {
        shapes->roles[i]=roles ? roles[i]:0; shapes->colors[i]=shapes->roles[i];
        if (analysis->shown[i]) shapes->order[count++]=i;
    }
    for (round=0;round<2;++round) {
        int families=0;
        memcpy(shapes->previous,shapes->colors,(size_t)analysis->node_count*sizeof(int));
        memset(shapes->begin,0,(size_t)(analysis->node_count+1)*sizeof(int));
        for (i=0;i<analysis->feature_count;++i) {
            const GD_GraphFeature *source=&analysis->features[i]; GD_ShapeFeature *feature=&shapes->features[i];
            if (!charge(shapes)) return shapes;
            *feature=(GD_ShapeFeature){source->owner,source->relation,source->endpoint,shapes->previous[source->neighbor]};
            ++shapes->begin[source->owner+1];
        }
        qsort(shapes->features,(size_t)analysis->feature_count,sizeof(shapes->features[0]),feature_compare);
        for (i=0;i<analysis->node_count;++i) shapes->begin[i+1]+=shapes->begin[i];
        if (!signature_sort(shapes,count)) return shapes;
        for (i=0;i<count;++i) {
            int different=1;
            if (!charge(shapes)) return shapes;
            if (i) different=signature_compare(shapes,shapes->order[i-1],shapes->order[i])!=0;
            if (!shapes->complete) return shapes;
            if (different) ++families;
            shapes->colors[shapes->order[i]]=families-1;
        }
        shapes->family_count=families;
    }
    memset(shapes->size,0,(size_t)shapes->family_count*sizeof(int));
    for (i=0;i<analysis->node_count;++i) {
        if (!analysis->shown[i]) { shapes->colors[i]=-1; continue; }
        if (!shapes->size[shapes->colors[i]]++) shapes->representative[shapes->colors[i]]=i;
    }
    return shapes;
}
const GD_GraphShapes *gd_graph_model_shapes(const GD_Model *model, GD_GraphAnalysis *analysis) {
    int roles[GD_MAX_RECORDS],i;
    for (i=0;i<analysis->node_count;++i) {
        int key=analysis->keys[i]; roles[i]=key>=0 && key<model->record_count ? (int)model->records[key].role:-1;
    }
    return gd_graph_shapes(analysis,roles,GD_GRAPH_ANALYSIS_WORK);
}
