/* Optional whole-component force above existing per-node solver. */
#ifndef GD_LAYOUT_ISLANDS_H
#define GD_LAYOUT_ISLANDS_H
#include <float.h>
enum { ISLAND_PAIR_WORK=GD_LAYOUT_ISLAND_WORK };
static const float island_padding=GD_LAYOUT_ISLAND_PADDING,island_clearance=GD_LAYOUT_ISLAND_CLEARANCE,island_step_limit=6;
static const float island_force_gain=0.1f,island_damping=0.8f;
typedef char LayoutIslandScratchBound[sizeof(GD_LayoutIslands)<=262144 ? 1:-1];
static int island_root(GD_LayoutIslands *s,int node){
    int root=node;
    while(s->parent[root]!=root)root=s->parent[root];
    while(node!=root){int next=s->parent[node];s->parent[node]=root;node=next;}
    return root;
}
static void islands_refresh(GD_Layout *layout){
    GD_LayoutIslands *s=&layout->islands;int i,changed=0;
    for(i=0;i<layout->node_count;++i){s->parent[i]=i;s->size[i]=1;}
    for(i=0;i<layout->edge_count;++i){
        const GD_LayoutEdge *e=&layout->edges[i];int a,b;
        if(!e->active || e->a==e->b)continue;
        a=island_root(s,e->a);b=island_root(s,e->b);if(a==b)continue;
        if(s->size[a]<s->size[b] || (s->size[a]==s->size[b] && a>b)){int swap=a;a=b;b=swap;}
        s->parent[b]=a;s->size[a]+=s->size[b];
    }
    for(i=0;i<layout->node_count;++i){int root=island_root(s,i);if(s->component[i]!=root)changed=1;s->component[i]=root;}
    if(changed){memset(s->velocity,0,(size_t)layout->node_count*sizeof(*s->velocity));memset(s->offset,0,(size_t)layout->node_count*sizeof(*s->offset));}
    s->count=0;
    for(i=0;i<layout->node_count;++i){s->bounds[i]=(GD_LayoutBounds){FLT_MAX,FLT_MAX,-FLT_MAX,-FLT_MAX};if(s->component[i]==i)++s->count;}
    for(i=0;i<layout->node_count;++i){
        const GD_LayoutNode *n=&layout->nodes[i];GD_LayoutBounds *b=&s->bounds[s->component[i]];
        b->left=fminf(b->left,n->x+n->footprint.left-island_padding);b->right=fmaxf(b->right,n->x+n->footprint.right+island_padding);
        b->top=fminf(b->top,n->y+n->footprint.top-island_padding);b->bottom=fmaxf(b->bottom,n->y+n->footprint.bottom+island_padding);
    }
}
int gd_layout_islands_set(GD_Layout *layout,int enabled){
    GD_LayoutIslands *s;int i;
    if(!layout || (enabled!=0 && enabled!=1) || layout->node_count<0 || layout->node_count>GD_MAX_RECORDS || layout->edge_count<0 || layout->edge_count>GD_MAX_PAIRS || layout->mode<GD_LAYOUT_FORCE || layout->mode>GD_LAYOUT_DIRECTED)return 0;
    if(enabled)for(i=0;i<layout->edge_count;++i)if(layout->edges[i].a<0 || layout->edges[i].a>=layout->node_count || layout->edges[i].b<0 || layout->edges[i].b>=layout->node_count)return 0;
    s=&layout->islands;
    if(s->preferred!=enabled || s->enabled!=(enabled && layout->mode==GD_LAYOUT_FORCE))memset(s,0,sizeof(*s));
    s->preferred=enabled;s->enabled=enabled && layout->mode==GD_LAYOUT_FORCE;
    if(s->enabled)islands_refresh(layout);return 1;
}
static void islands_step(GD_Layout *layout,int held){
    GD_LayoutIslands *s=&layout->islands;int i,j,roots=0,pinned;
    islands_refresh(layout);pinned=held>=0 && held<layout->node_count ? s->component[held]:-1;s->checks=s->overlaps=0;s->limited=0;
    if(s->count<2)return;
    memset(s->force,0,(size_t)layout->node_count*sizeof(*s->force));
    for(i=0;i<layout->node_count;++i)if(s->component[i]==i)layout->collision_order[roots++]=(GD_LayoutCollisionItem){s->bounds[i].left,s->bounds[i].right,i};
    collision_sort(layout->collision_order,roots);
    for(i=0;i<roots;++i)for(j=i+1;j<roots && layout->collision_order[j].minimum<layout->collision_order[i].maximum+island_clearance;++j){
        int a=layout->collision_order[i].node,b=layout->collision_order[j].node;const GD_LayoutBounds *x=&s->bounds[a],*y=&s->bounds[b];
        float left=x->right-y->left+island_clearance,right=y->right-x->left+island_clearance,up=x->bottom-y->top+island_clearance,down=y->bottom-x->top+island_clearance,dx=0,dy=0,wa=a==pinned ? 0:b==pinned ? 1:0.5f,wb=1-wa;
        if(s->checks++==ISLAND_PAIR_WORK){s->checks=ISLAND_PAIR_WORK;s->limited=1;goto apply;}
        if(left<=0 || right<=0 || up<=0 || down<=0)continue;++s->overlaps;
        /* Centre-line repulsion avoids orthogonal minimum-axis forces
           cancelling in crowded rows despite substantial box intersections. */
        {float cx=(x->left+x->right-y->left-y->right)*0.5f,cy=(x->top+x->bottom-y->top-y->bottom)*0.5f;
            float length=hypotf(cx,cy),depth=fminf(fminf(left,right),fminf(up,down));
            if(length<0.001f){cx=a<b ? -1:1;cy=0;length=1;}dx=cx/length*depth;dy=cy/length*depth;
        }
        s->force[a].x+=dx*island_force_gain*wa;s->force[a].y+=dy*island_force_gain*wa;
        s->force[b].x-=dx*island_force_gain*wb;s->force[b].y-=dy*island_force_gain*wb;
    }
apply:
    for(i=0;i<roots;++i){
        int root=layout->collision_order[i].node;GD_LayoutPoint *v=&s->velocity[root],*delta=&s->force[root];const GD_LayoutBounds *b=&s->bounds[root];
        v->x=(v->x+delta->x)*island_damping;v->y=(v->y+delta->y)*island_damping;
        {float speed=hypotf(v->x,v->y);if(speed>island_step_limit){v->x*=island_step_limit/speed;v->y*=island_step_limit/speed;}}*delta=*v;
        if(root==pinned){*v=*delta=(GD_LayoutPoint){0};continue;}
        if(layout->workspace.enabled){
            const GD_LayoutBounds *w=&layout->workspace.bounds;
            float lx=w->left-(b->left+island_padding),rx=w->right-(b->right-island_padding),ty=w->top-(b->top+island_padding),by=w->bottom-(b->bottom-island_padding);
            float dx=delta->x,dy=delta->y;
            /* Never move farther than budget merely to repair an oversized or
               already-outside island. Node containment remains existing logic. */
            if(lx>rx || ty>by){*delta=*v=(GD_LayoutPoint){0};s->limited=1;}
            else{
                if(dx>0)delta->x=fmaxf(0,fminf(dx,rx));else if(dx<0)delta->x=fminf(0,fmaxf(dx,lx));
                if(dy>0)delta->y=fmaxf(0,fminf(dy,by));else if(dy<0)delta->y=fminf(0,fmaxf(dy,ty));
                if(dx!=delta->x){v->x=0;s->limited=1;}if(dy!=delta->y){v->y=0;s->limited=1;}
            }
        }
        s->offset[root].x+=delta->x;s->offset[root].y+=delta->y;
    }
    for(i=0;i<layout->node_count;++i){const GD_LayoutPoint *delta=&s->force[s->component[i]];layout->nodes[i].x+=delta->x;layout->nodes[i].y+=delta->y;}
    for(i=0;i<roots;++i){int root=layout->collision_order[i].node;const GD_LayoutPoint *d=&s->force[root];s->bounds[root].left+=d->x;s->bounds[root].right+=d->x;s->bounds[root].top+=d->y;s->bounds[root].bottom+=d->y;}
}
#endif
