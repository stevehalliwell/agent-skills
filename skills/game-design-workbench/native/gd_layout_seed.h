/* Private one-shot topology-near seed. No crossing objective, source changes,
   fixed roots or per-frame work. Compact BFS order occupies a rectangular spiral. */
#ifndef GD_LAYOUT_SEED_H
#define GD_LAYOUT_SEED_H
#include <float.h>
typedef struct { double left,top,right,bottom; } LayoutSeedBox;
typedef struct {
    int head[GD_MAX_RECORDS],degree[GD_MAX_RECORDS],mark[GD_MAX_RECORDS];
    int component[GD_MAX_RECORDS],queue[GD_MAX_RECORDS],seen[GD_MAX_RECORDS];
    int next[GD_MAX_PAIRS*2];
    GD_LayoutPoint point[GD_MAX_RECORDS];
    LayoutSeedBox box[GD_MAX_RECORDS];
} LayoutSeedWork;
typedef char LayoutSeedScratchBound[sizeof(LayoutSeedWork)<=1048576 ? 1:-1];
static int seed_other(const GD_Layout *layout,int arc) {
    return arc%2 ? layout->edges[arc/2].a:layout->edges[arc/2].b;
}
int gd_layout_seed_neighborhood(GD_Layout *layout,GD_Allocator allocator,float aspect) {
    LayoutSeedWork *work; int i,e,components=0,okay=0;
    double area=0,widest=0,target,x=32,y=32,row=0,width=0,height=0;
    if (!layout || !allocator.allocate || !allocator.release || !isfinite(aspect) || aspect<=0 ||
        layout->mode!=GD_LAYOUT_FORCE || layout->node_count<0 || layout->node_count>GD_MAX_RECORDS ||
        layout->edge_count<0 || layout->edge_count>GD_MAX_PAIRS || layout->group_count<0 || layout->group_count>GD_MAX_RECORDS) return 0;
    work=allocator.allocate(allocator.context,sizeof(*work)); if (!work) return 0;
    memset(work,0,sizeof(*work));
    for (i=0;i<layout->node_count;++i) {
        const GD_LayoutNode *node=&layout->nodes[i]; const GD_LayoutBounds *b=&node->footprint;
        int ordered=layout->order[i];
        if (node->group<0 || node->group>=layout->group_count || ordered<0 || ordered>=layout->node_count || work->mark[ordered] ||
            layout->position[ordered]!=i || !isfinite(b->left) || !isfinite(b->top) || !isfinite(b->right) || !isfinite(b->bottom) ||
            b->left>=b->right || b->top>=b->bottom || fabsf(b->left)>1048576 || fabsf(b->top)>1048576 || fabsf(b->right)>1048576 || fabsf(b->bottom)>1048576) goto done;
        work->mark[ordered]=1; work->head[i]=-1; work->component[i]=-1;
    }
    memset(work->mark,0,sizeof(work->mark));
    for (e=0;e<layout->edge_count;++e) {
        const GD_LayoutEdge *edge=&layout->edges[e];
        if (edge->a<0 || edge->a>=layout->node_count || edge->b<0 || edge->b>=layout->node_count) goto done;
        if (!edge->active || edge->a==edge->b) continue;
        work->next[2*e]=work->head[edge->a]; work->head[edge->a]=2*e;
        work->next[2*e+1]=work->head[edge->b]; work->head[edge->b]=2*e+1;
    }
    /* Count distinct active neighbors; parallel edges and loops do not bias hub. */
    for (i=0;i<layout->node_count;++i) for (e=work->head[i];e>=0;e=work->next[e]) {
        int other=seed_other(layout,e);
        if (work->mark[other]!=i+1) {work->mark[other]=i+1; ++work->degree[i];}
    }
    for (i=0;i<layout->node_count;++i) {
        int start=layout->order[i],first=0,last=1,hub=start,c=components;
        float cell_width=0,cell_height=0; int column=0,line=0,direction=0,length=1,moved=0,turns=0;
        LayoutSeedBox *box;
        if (work->component[start]>=0) continue;
        work->queue[0]=start; work->component[start]=c;
        while (first<last) {
            int node=work->queue[first++]; const GD_LayoutBounds *b=&layout->nodes[node].footprint;
            cell_width=fmaxf(cell_width,b->right-b->left+16); cell_height=fmaxf(cell_height,b->bottom-b->top+16);
            if (work->degree[node]>work->degree[hub] || (work->degree[node]==work->degree[hub] && layout->position[node]<layout->position[hub])) hub=node;
            for (e=work->head[node];e>=0;e=work->next[e]) {
                int other=seed_other(layout,e);
                if (work->component[other]>=0) continue;
                work->component[other]=c; work->queue[last++]=other;
            }
        }
        box=&work->box[components++]; *box=(LayoutSeedBox){DBL_MAX,DBL_MAX,-DBL_MAX,-DBL_MAX};
        first=0; last=1; work->queue[0]=hub; work->seen[hub]=1;
        while (first<last) {
            int node=work->queue[first++]; const GD_LayoutBounds *b=&layout->nodes[node].footprint;
            GD_LayoutPoint *point=&work->point[node];
            point->x=(float)((double)column*cell_width-((double)b->left+b->right)/2);
            point->y=(float)((double)line*cell_height-((double)b->top+b->bottom)/2);
            box->left=fmin(box->left,(double)point->x+b->left); box->top=fmin(box->top,(double)point->y+b->top);
            box->right=fmax(box->right,(double)point->x+b->right); box->bottom=fmax(box->bottom,(double)point->y+b->bottom);
            if (direction==0) ++column; else if (direction==1) ++line; else if (direction==2) --column; else --line;
            if (++moved==length) {moved=0; direction=(direction+1)%4; if (++turns%2==0) ++length;}
            for (e=work->head[node];e>=0;e=work->next[e]) {
                int other=seed_other(layout,e);
                if (work->seen[other]) continue;
                work->seen[other]=1; work->queue[last++]=other;
            }
        }
        area+=(box->right-box->left+32)*(box->bottom-box->top+32);
        widest=fmax(widest,box->right-box->left);
    }
    target=fmax(widest,sqrt(area*(double)aspect));
    /* Linear shelf packing targets aspect, not optimal bin packing.
       Large components/unequal heights may dominate; refine only on visual evidence. */
    for (i=0;i<components;++i) {
        LayoutSeedBox *box=&work->box[i]; double w=box->right-box->left,h=box->bottom-box->top;
        if (x>32 && x+w>target+32) {x=32; y+=row+32; row=0;}
        box->left=x-box->left; box->top=y-box->top; x+=w+32; row=fmax(row,h);
        width=fmax(width,x); height=fmax(height,y+h+32);
    }
    for (i=0;i<layout->node_count;++i) {
        GD_LayoutPoint *point=&work->point[i]; const LayoutSeedBox *box=&work->box[work->component[i]];
        point->x=(float)(point->x+box->left); point->y=(float)(point->y+box->top);
        if (!isfinite(point->x) || !isfinite(point->y)) goto done;
    }
    for (i=0;i<layout->group_count;++i) work->box[i]=(LayoutSeedBox){DBL_MAX,DBL_MAX,-DBL_MAX,-DBL_MAX};
    for (i=0;i<layout->node_count;++i) {
        const GD_LayoutNode *node=&layout->nodes[i]; const GD_LayoutPoint *point=&work->point[i]; LayoutSeedBox *box=&work->box[node->group];
        box->left=fmin(box->left,(double)point->x+node->footprint.left); box->top=fmin(box->top,(double)point->y+node->footprint.top);
        box->right=fmax(box->right,(double)point->x+node->footprint.right); box->bottom=fmax(box->bottom,(double)point->y+node->footprint.bottom);
    }
    /* Commit geometry only after complete validation/computation. */
    for (i=0;i<layout->node_count;++i) {
        layout->nodes[i].x=work->point[i].x; layout->nodes[i].y=work->point[i].y; layout->nodes[i].vx=layout->nodes[i].vy=0;
    }
    for (i=0;i<layout->group_count;++i) {
        GD_LayoutGroup *group=&layout->groups[i]; const LayoutSeedBox *box=&work->box[i];
        if (box->left==DBL_MAX) group->x=group->y=group->width=group->height=0;
        else {group->x=(float)(box->left-16); group->y=(float)(box->top-16); group->width=(float)(box->right-box->left+32); group->height=(float)(box->bottom-box->top+32);}
    }
    layout->width=(float)width; layout->height=(float)height; layout->tick=0;
    layout->collision_cursor=layout->collision_checks=layout->collision_moves=0; layout->collision_limited=0; okay=1;
done:
    allocator.release(allocator.context,work); return okay;
}
#endif
