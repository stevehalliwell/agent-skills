/* Retained-map presentation only. Caller owns storage; no map writes or GPU calls. */
#ifndef GD_SIMILARITY_VIEW_H
#define GD_SIMILARITY_VIEW_H
#include "gd_explorer_ui.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

enum { GD_SIMILARITY_HIT_PIXELS=16,GD_SIMILARITY_HOVER_ROWS=8,GD_SIMILARITY_LABELS=64 };
typedef struct { Vector2 point; int row,count; } GD_SimilarityViewPoint;
typedef struct GD_SimilarityView {
    GD_SimilarityViewPoint groups[GD_MAX_RECORDS]; int group_count;
    int pick_rows[GD_MAX_RECORDS],pick_count,pick_page,pick_peer,pick_open;
    int hover_rows[GD_SIMILARITY_HOVER_ROWS],hover_count;
    Rectangle labels[GD_SIMILARITY_LABELS]; int label_count,hidden_labels;
} GD_SimilarityView;

static inline void gd_similarity_view_choose(GD_ExplorerStructureState *state,int row,int peer) {
    assert(state && state->result && state->result->similarity && row>=0 && row<state->result->similarity->entry_count);
    if(peer)state->similarity_peer=row;else state->similarity_row=row;
}
static inline Vector2 gd_similarity_view_point(const GD_Similarity *map,int row) {
    double scale=map->distance_scale>0 ? map->distance_scale:1;
    /* One uniform scale only; never move individual points. */
    return (Vector2){(float)(400*(map->x[row]/scale)),(float)(400*(map->y[row]/scale))};
}
static inline const char *gd_similarity_view_name(const GD_SimilarityProjection *projection,int row) {
    int node;
    if(!projection || row<0 || row>=projection->entry_count)return "[Unavailable entry]";
    node=projection->nodes[row];
    if(projection->context && projection->context->layout && node>=0 && node<projection->context->layout->node_count && projection->context->layout->nodes[node].label && *projection->context->layout->nodes[node].label)
        return projection->context->layout->nodes[node].label;
    return projection->ids[row] ? projection->ids[row]:"[Unavailable identity]";
}
static inline const char *gd_similarity_view_measurement(const GD_SimilarityProjection *projection,int feature) {
    static const char *const descriptions[]={"Retained partner count","Retained incident row count","Retained relation-kind count","Retained canonical incoming row count","Retained canonical outgoing row count","Authored rank (not inferred importance)","Loaded source status (exact value)","Loaded source record role (exact category)","Explicitly selected finite-variant value","Exact retained direct-member set","Exact retained reachable-member set"};
    if(!projection || feature<0 || feature>=projection->feature_count)return "[Unavailable measurement]";
    int index=projection->selected_features[feature];if(index<0 || index>=projection->catalogue_count)return "[Unavailable measurement]";
    int property=projection->catalogue[index].property;
    return property>=0 && (size_t)property<sizeof(descriptions)/sizeof(*descriptions) ? descriptions[property]:"[Unavailable measurement]";
}
static int gd_similarity_view_order(const void *left,const void *right) {
    const GD_SimilarityViewPoint *a=left,*b=right;
    if(a->point.x!=b->point.x)return a->point.x<b->point.x ? -1:1;
    if(a->point.y!=b->point.y)return a->point.y<b->point.y ? -1:1;
    return (a->row>b->row)-(a->row<b->row);
}
static inline int gd_similarity_view_build(GD_SimilarityView *view,const GD_Similarity *map) {
    int row,count=0;
    if(!view || !map || !map->available || map->input.entry_count<0 || map->input.entry_count>GD_MAX_RECORDS || !isfinite(map->distance_scale) || map->distance_scale<0)return 0;
    /* Validate before changing existing presentation. */
    for(row=0;row<map->input.entry_count;++row)if(map->placed[row]){
        Vector2 p=gd_similarity_view_point(map,row);if(!isfinite(p.x)||!isfinite(p.y))return 0;
    }
    for(row=0;row<map->input.entry_count;++row)if(map->placed[row])view->groups[count++]=(GD_SimilarityViewPoint){gd_similarity_view_point(map,row),row,1};
    qsort(view->groups,(size_t)count,sizeof(*view->groups),gd_similarity_view_order);
    view->group_count=0;
    for(row=0;row<count;++row){
        GD_SimilarityViewPoint value=view->groups[row];
        if(view->group_count && value.point.x==view->groups[view->group_count-1].point.x && value.point.y==view->groups[view->group_count-1].point.y)++view->groups[view->group_count-1].count;
        else view->groups[view->group_count++]=value;
    }
    view->pick_open=view->pick_count=view->pick_page=view->hover_count=0;
    return 1;
}
/* Counts every placed hit. Optional prefix storage is presentation only;
   chooser passes full capacity. Retained row order preserves exact identities. */
static inline int gd_similarity_view_hits(const GD_Similarity *map,Camera2D camera,Vector2 mouse,int *rows,int capacity) {
    int row,count=0;
    if(!map || !map->available || map->input.entry_count<0 || map->input.entry_count>GD_MAX_RECORDS || capacity<0 || capacity>GD_MAX_RECORDS || (capacity && !rows) || !isfinite(camera.zoom) || camera.zoom<=0 || !isfinite(camera.rotation) || !isfinite(camera.target.x) || !isfinite(camera.target.y) || !isfinite(camera.offset.x) || !isfinite(camera.offset.y) || !isfinite(mouse.x) || !isfinite(mouse.y))return 0;
    for(row=0;row<map->input.entry_count;++row)if(map->placed[row]){
        Vector2 screen=GetWorldToScreen2D(gd_similarity_view_point(map,row),camera);float x=screen.x-mouse.x,y=screen.y-mouse.y;
        if(x*x+y*y<=GD_SIMILARITY_HIT_PIXELS*GD_SIMILARITY_HIT_PIXELS){if(count<capacity)rows[count]=row;++count;}
    }
    return count;
}
#endif
