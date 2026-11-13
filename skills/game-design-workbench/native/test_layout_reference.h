/* Matched CPU diagnostics, not a second solver or visual acceptance test.
   Adapter footprints only; no GPU/font/annotation measurement. Each run owns
   its geometry and borrows immutable source/query evidence. JSON goes to stderr
   so production-adapter stdout retains its existing machine-readable contract. */
#ifndef TEST_LAYOUT_REFERENCE_H
#define TEST_LAYOUT_REFERENCE_H
#include "gd_projection.h"
#include <stdint.h>
enum { REFERENCE_WINDOW=256, REFERENCE_STEPS=4096 };
typedef struct {
    GD_LayoutPoint previous[GD_MAX_RECORDS],visual_previous[GD_MAX_RECORDS];
    int parent[GD_MAX_RECORDS],degree[GD_MAX_RECORDS];
    uint64_t pairs[GD_MAX_PAIRS];
    double square[REFERENCE_WINDOW],maximum[REFERENCE_WINDOW],visual_square[REFERENCE_WINDOW],visual_maximum[REFERENCE_WINDOW];
    double centre_x[REFERENCE_WINDOW],centre_y[REFERENCE_WINDOW];
    unsigned checks[REFERENCE_WINDOW];int limited[REFERENCE_WINDOW];
} LayoutReferenceWork;
static int reference_pair_order(const void *left,const void *right) {
    uint64_t a=*(const uint64_t *)left,b=*(const uint64_t *)right;return (a>b)-(a<b);
}
static int reference_degree_order(const void *left,const void *right) {
    int a=*(const int *)left,b=*(const int *)right;return (a>b)-(a<b);
}
static int reference_root(int *parent,int node) {
    while(parent[node]!=node){parent[node]=parent[parent[node]];node=parent[node];}return node;
}
static void reference_topology(const GD_Layout *layout,LayoutReferenceWork *work,const char *view,const char *seed,const char *profile,int startup) {
    int i,count=0,unique=0,components=layout->node_count,loops=0,parallel=0,run=0;
    double min_width=DBL_MAX,max_width=0,min_height=DBL_MAX,max_height=0;
    for(i=0;i<layout->node_count;++i){const GD_LayoutNode *node=&layout->nodes[i];double width=node->footprint.right-node->footprint.left,height=node->footprint.bottom-node->footprint.top;work->parent[i]=i;work->degree[i]=0;min_width=fmin(min_width,width);max_width=fmax(max_width,width);min_height=fmin(min_height,height);max_height=fmax(max_height,height);}
    for(i=0;i<layout->edge_count;++i){const GD_LayoutEdge *edge=&layout->edges[i];int a=edge->a,b=edge->b,first,last;if(!edge->active)continue;if(a==b){++loops;continue;}first=reference_root(work->parent,a);last=reference_root(work->parent,b);if(first!=last){work->parent[last]=first;--components;}if(a>b){int swap=a;a=b;b=swap;}work->pairs[count++]=((uint64_t)(unsigned)a<<32)|(unsigned)b;}
    qsort(work->pairs,(size_t)count,sizeof(work->pairs[0]),reference_pair_order);
    for(i=0;i<count;++i){if(!i || work->pairs[i]!=work->pairs[i-1]){++unique;++work->degree[(unsigned)(work->pairs[i]>>32)];++work->degree[(unsigned)work->pairs[i]];run=0;}if(++run>parallel)parallel=run;}
    qsort(work->degree,(size_t)layout->node_count,sizeof(work->degree[0]),reference_degree_order);
    fprintf(stderr,"layout-reference {\"kind\":\"topology\",\"view\":\"%s\",\"seed\":\"%s\",\"profile\":\"%s\",\"startup_steps\":%d,\"aspect\":1.4,\"footprints\":\"adapter-only\",\"world_scale\":1,\"nodes\":%d,\"edges\":%d,\"groups\":%d,\"components\":%d,\"active_nonself_springs\":%d,\"unique_neighbor_pairs\":%d,\"self_loops\":%d,\"max_parallel\":%d,\"degree_mean\":%.6f,\"degree_median\":%.1f,\"degree_p90\":%d,\"degree_max\":%d,\"width_min\":%.6f,\"width_max\":%.6f,\"height_min\":%.6f,\"height_max\":%.6f,\"workspace_enabled\":%s,\"workspace_left\":%.6f,\"workspace_top\":%.6f,\"workspace_right\":%.6f,\"workspace_bottom\":%.6f}\n",view,seed,profile,startup,layout->node_count,layout->edge_count,layout->group_count,components,count,unique,loops,parallel,layout->node_count ? 2.0*unique/layout->node_count:0,layout->node_count ? (work->degree[(layout->node_count-1)/2]+work->degree[layout->node_count/2])/2.0:0,layout->node_count ? work->degree[(layout->node_count*9+9)/10-1]:0,layout->node_count ? work->degree[layout->node_count-1]:0,layout->node_count ? min_width:0,max_width,layout->node_count ? min_height:0,max_height,layout->workspace.enabled ? "true":"false",layout->workspace.bounds.left,layout->workspace.bounds.top,layout->workspace.bounds.right,layout->workspace.bounds.bottom);
}
static void reference_sample(const GD_Layout *layout,const LayoutReferenceWork *work,const char *view,const char *seed,const char *profile,int startup,int step) {
    int i,window=step<REFERENCE_WINDOW ? step:REFERENCE_WINDOW,limited=0,outside=0,boundary=0;double overflow=0;
    double square=0,maximum=0,visual_square=0,visual_maximum=0,cx=0,cy=0,left=DBL_MAX,top=DBL_MAX,right=-DBL_MAX,bottom=-DBL_MAX,drift=0;unsigned checks=0;
    for(i=0;i<window;++i){square+=work->square[i];maximum=fmax(maximum,work->maximum[i]);visual_square+=work->visual_square[i];visual_maximum=fmax(visual_maximum,work->visual_maximum[i]);if(work->checks[i]>checks)checks=work->checks[i];limited+=work->limited[i];}
    for(i=0;i<layout->node_count;++i){const GD_LayoutNode *node=&layout->nodes[i];cx+=node->x;cy+=node->y;left=fmin(left,(double)node->x+node->footprint.left);top=fmin(top,(double)node->y+node->footprint.top);right=fmax(right,(double)node->x+node->footprint.right);bottom=fmax(bottom,(double)node->y+node->footprint.bottom);}
    if(layout->workspace.enabled)for(i=0;i<layout->node_count;++i){const GD_LayoutNode *node=&layout->nodes[i];const GD_LayoutBounds *b=&layout->workspace.bounds;double amount=fmax(0,fmax(fmax(b->left-node->x-node->footprint.left,node->x+node->footprint.right-b->right),fmax(b->top-node->y-node->footprint.top,node->y+node->footprint.bottom-b->bottom)));if(amount>0.01)++outside; /* Ignore sub-pixel float roundoff in contact count. */if(fmin(fmin(node->x+node->footprint.left-b->left,b->right-node->x-node->footprint.right),fmin(node->y+node->footprint.top-b->top,b->bottom-node->y-node->footprint.bottom))<=1)++boundary;overflow=fmax(overflow,amount);}
    if(layout->node_count && window){int oldest=step<REFERENCE_WINDOW ? 0:step%REFERENCE_WINDOW;drift=hypot(cx/layout->node_count-work->centre_x[oldest],cy/layout->node_count-work->centre_y[oldest]);}
    fprintf(stderr,"layout-reference {\"kind\":\"motion\",\"view\":\"%s\",\"seed\":\"%s\",\"profile\":\"%s\",\"startup_steps\":%d,\"step\":%d,\"window\":%d,\"rms\":%.6f,\"max_step\":%.6f,\"centroid_drift\":%.6f,\"overlaps\":%lu,\"limited_steps\":%d,\"max_collision_checks\":%u,\"extent_width\":%.6f,\"extent_height\":%.6f,\"outside_nodes\":%d,\"max_overflow\":%.6f,\"boundary_nodes\":%d,\"visual_rms\":%.6f,\"visual_max_step\":%.6f}\n",view,seed,profile,startup,step,window,window && layout->node_count ? sqrt(square/(window*layout->node_count)):0,sqrt(maximum),drift,presentation_footprint_overlaps(layout),limited,checks,layout->node_count ? right-left:0,layout->node_count ? bottom-top:0,outside,overflow,boundary,window && layout->node_count ? sqrt(visual_square/(window*layout->node_count)):0,sqrt(visual_maximum));
}
static uint64_t reference_bytes(const void *pointer,size_t size) {
    const unsigned char *bytes=pointer;uint64_t hash=UINT64_C(14695981039346656037);size_t i;
    for(i=0;i<size;++i)hash=(hash^bytes[i])*UINT64_C(1099511628211);return hash;
}
static int presentation_layout_reference(const GD_ExplorerContext *context) {
    const GD_ExplorerPane *pane=context->pane;GD_Allocator a=context->project->allocator;
    const GD_QueryScene *original=pane->view==GD_EXPLORER_EFFECTS ? pane->query_state->scene:NULL;
    LayoutReferenceWork *work=NULL;GD_Layout *ordinary=NULL;GD_QueryScene *scene=NULL;GD_RenderPose *visual=NULL;
    const char *view=viewer_projections[pane->view];uint64_t before=reference_bytes(pane->layout,sizeof(*pane->layout)),typed=original ? reference_bytes(original,sizeof(*original)):0;
    int variant,node,step,valid=0,base_variants=original ? 2:pane->view==GD_EXPLORER_RESOURCES ? 3:2,variants=base_variants+2;
    work=a.allocate(a.context,sizeof(*work));visual=a.allocate(a.context,sizeof(*visual));if(!work || !visual)goto cleanup;memset(visual,0,sizeof(*visual));
    for(variant=0;variant<variants;++variant){
        GD_Layout *layout;GD_LayoutWorkspace workspace;int bounded=variant>=base_variants,unweighted=original && variant%2;
        int neighborhood=original || (bounded ? variant==base_variants+1:variant==1),startup=!bounded && !original && variant==2 ? 120:0;
        const char *seed=neighborhood ? "neighborhood":"native",*profile=bounded ? unweighted ? "bounded-unweighted-effects":"bounded-workspace":unweighted ? "unweighted-effects":"adapter";
        memset(work,0,sizeof(*work));
        if(original){
            if(original->overview || original->free_geometry)goto cleanup;
            if(unweighted){
                /* Existing display option, not a compact-card implementation. */
                scene=original->effect_scope ? gd_query_scene_effect_context(original->query,original->effect_scope,0,original->work_limit):gd_query_scene_build(original->query,original->ghosts,original->primary,0,original->include_excluded,original->work_limit);
                if(!scene || scene->layout.node_count!=original->layout.node_count || scene->layout.edge_count!=original->layout.edge_count || memcmp(scene->nodes,original->nodes,sizeof(scene->nodes)) || memcmp(scene->edges,original->edges,sizeof(scene->edges)) || memcmp(scene->labels,original->labels,sizeof(scene->labels)))goto cleanup;
            }else{
                scene=a.allocate(a.context,sizeof(*scene));if(!scene)goto cleanup;memcpy(scene,original,sizeof(*scene));
                for(node=0;node<scene->layout.node_count;++node)scene->layout.nodes[node].label=scene->labels[node];
            }
            if(!gd_query_scene_place(scene,GD_LAYOUT_FORCE,bounded,1.4f))goto cleanup;
            layout=&scene->layout;
        }else{
            ordinary=a.allocate(a.context,sizeof(*ordinary));if(!ordinary)goto cleanup;
            if(pane->view==GD_EXPLORER_RESOURCES){if(!gd_resource_layout(context->project->resources,ordinary))goto cleanup;}
            else if(pane->view==GD_EXPLORER_MATRIX){if(!gd_matrix_layout(gd_explorer_model(context->project,pane->view),ordinary,GD_LAYOUT_FORCE,GD_SORT_NAME))goto cleanup;}
            else if(pane->view==GD_EXPLORER_NOUNS){if(!gd_noun_grid_layout(context->project->noun_grid,ordinary))goto cleanup;}
            else goto cleanup;
            layout=ordinary;
            if(neighborhood && !gd_layout_seed_neighborhood(layout,a,1.4f))goto cleanup;
            for(step=0;step<startup;++step)gd_layout_step(layout,-1);
        }
        if(bounded && !(scene ? gd_query_scene_workspace(scene,1,1.4f):gd_layout_workspace(layout,1,1.4f)))goto cleanup;
        workspace=layout->workspace;
        for(node=0;node<layout->node_count;++node)visual->anchors[node]=scene ? gd_query_scene_anchor(scene,node):node;
        if(!gd_render_pose_update(visual,layout,layout,-1,1.0/60,1))goto cleanup;
        reference_topology(layout,work,view,seed,profile,startup);reference_sample(layout,work,view,seed,profile,startup,0);
        for(step=1;step<=REFERENCE_STEPS;++step){
            int slot=(step-1)%REFERENCE_WINDOW;double cx=0,cy=0;work->square[slot]=work->maximum[slot]=work->visual_square[slot]=work->visual_maximum[slot]=0;
            for(node=0;node<layout->node_count;++node)work->visual_previous[node]=(GD_LayoutPoint){visual->layout.nodes[node].x,visual->layout.nodes[node].y};
            for(node=0;node<layout->node_count;++node){work->previous[node]=(GD_LayoutPoint){layout->nodes[node].x,layout->nodes[node].y};cx+=layout->nodes[node].x;cy+=layout->nodes[node].y;}
            work->centre_x[slot]=layout->node_count ? cx/layout->node_count:0;work->centre_y[slot]=layout->node_count ? cy/layout->node_count:0;
            if(scene)gd_query_scene_step(scene,-1);else gd_layout_step(layout,-1);
            if(layout->collision_checks>GD_LAYOUT_COLLISION_WORK || memcmp(&workspace,&layout->workspace,sizeof(workspace)))goto cleanup;
            work->checks[slot]=layout->collision_checks;work->limited[slot]=layout->collision_limited!=0;
            for(node=0;node<layout->node_count;++node){double dx=(double)layout->nodes[node].x-work->previous[node].x,dy=(double)layout->nodes[node].y-work->previous[node].y,movement=dx*dx+dy*dy;if(!isfinite(movement))goto cleanup;work->square[slot]+=movement;work->maximum[slot]=fmax(work->maximum[slot],movement);}
            if(!gd_render_pose_update(visual,layout,layout,-1,1.0/60,0))goto cleanup;
            for(node=0;node<layout->node_count;++node){double dx=(double)visual->layout.nodes[node].x-work->visual_previous[node].x,dy=(double)visual->layout.nodes[node].y-work->visual_previous[node].y,movement=dx*dx+dy*dy;if(!isfinite(movement))goto cleanup;work->visual_square[slot]+=movement;work->visual_maximum[slot]=fmax(work->visual_maximum[slot],movement);}
            if(step==120 || step==600 || step==REFERENCE_STEPS)reference_sample(layout,work,view,seed,profile,startup,step);
        }
        if(scene){gd_query_scene_destroy(scene);scene=NULL;}if(ordinary){a.release(a.context,ordinary);ordinary=NULL;}
    }
    valid=before==reference_bytes(pane->layout,sizeof(*pane->layout)) && (!original || typed==reference_bytes(original,sizeof(*original)));
cleanup:
    if(scene)gd_query_scene_destroy(scene);if(ordinary)a.release(a.context,ordinary);if(work)a.release(a.context,work);if(visual)a.release(a.context,visual);
    return valid;
}
#endif
