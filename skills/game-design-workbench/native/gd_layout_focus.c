#include "gd_layout_focus.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

GD_LayoutFocus *gd_layout_focus_create(GD_Allocator allocator) {
    GD_LayoutFocus *focus;
    if (!allocator.allocate || !allocator.release) return NULL;
    focus=allocator.allocate(allocator.context,sizeof(*focus));
    if (focus) { memset(focus,0,sizeof(*focus)); focus->allocator=allocator; focus->anchor_node=-1; }
    return focus;
}
void gd_layout_focus_destroy(GD_LayoutFocus *focus) {
    if (focus) focus->allocator.release(focus->allocator.context,focus);
}
void gd_layout_focus_capture(GD_LayoutFocus *focus, const GD_Layout *layout) {
    int i;
    if (!focus || !layout) return;
    if (layout->node_count<0 || layout->node_count>GD_MAX_RECORDS || layout->group_count<0 || layout->group_count>GD_MAX_RECORDS) {
        focus->captured=focus->active=0; return;
    }
    focus->node_count=layout->node_count; focus->group_count=layout->group_count;
    focus->direct_routes=layout->direct_routes; focus->anchor_node=-1; focus->active=0;
    for (i=0;i<layout->node_count;++i) {
        const GD_LayoutNode *node=&layout->nodes[i];
        focus->base[i]=(GD_FocusPose){node->x,node->y,node->vx,node->vy,node->key};
    }
    memcpy(focus->groups,layout->groups,(size_t)layout->group_count*sizeof(layout->groups[0]));
    focus->captured=1;
}
void gd_layout_focus_restore(GD_LayoutFocus *focus, GD_Layout *layout) {
    int i;
    if (!focus || !focus->captured || focus->node_count!=layout->node_count || focus->group_count!=layout->group_count) return;
    for (i=0;i<layout->node_count;++i) if (focus->base[i].key!=layout->nodes[i].key) return;
    for (i=0;i<layout->node_count;++i) {
        GD_LayoutNode *node=&layout->nodes[i]; const GD_FocusPose *base=&focus->base[i];
        node->x=base->x; node->y=base->y; node->vx=base->vx; node->vy=base->vy;
    }
    memcpy(layout->groups,focus->groups,(size_t)layout->group_count*sizeof(layout->groups[0]));
    layout->direct_routes=focus->direct_routes;
}
static int focus_order(const void *left, const void *right) {
    const GD_FocusOrder *a=left,*b=right;
    if (a->band!=b->band) return a->band<b->band ? -1:1;
    if (a->distance!=b->distance) return a->distance<b->distance ? -1:1;
    if (a->angle!=b->angle) return a->angle<b->angle ? -1:1;
    if (a->key!=b->key) return (a->key>b->key)-(a->key<b->key);
    return (a->node>b->node)-(a->node<b->node);
}
static float cross_span(const GD_LayoutNode *node, int horizontal, float scale) {
    return (horizontal ? (node->height>0 ? node->height:120):gd_layout_node_width(node))/scale;
}
static void focus_bounds(GD_Layout *layout, float scale) {
    int i; unsigned char used[GD_MAX_RECORDS]={0};
    for (i=0;i<layout->node_count;++i) {
        const GD_LayoutNode *node=&layout->nodes[i]; GD_LayoutGroup *group=&layout->groups[node->group];
        float half=gd_layout_node_width(node)/(2*scale)+24/scale;
        float height=(node->height>0 ? node->height:120)/(2*scale)+24/scale;
        float left=node->x-half,right=node->x+half,top=node->y-height,bottom=node->y+height;
        if (!used[node->group]) { group->x=left; group->y=top; group->width=right-left; group->height=bottom-top; used[node->group]=1; }
        else {
            float x=fminf(group->x,left),y=fminf(group->y,top);
            group->width=fmaxf(group->x+group->width,right)-x;
            group->height=fmaxf(group->y+group->height,bottom)-y; group->x=x; group->y=y;
        }
    }
}
void gd_layout_focus_reapply(GD_LayoutFocus *focus, GD_Layout *layout, float scale) {
    int i;
    if (!focus || !focus->active || !layout || layout->node_count!=focus->node_count || layout->group_count!=focus->group_count || !isfinite(scale) || scale<=0) return;
    for (i=0;i<layout->node_count;++i) if (focus->base[i].key!=layout->nodes[i].key) return;
    gd_layout_focus_restore(focus,layout);
    for (i=0;i<layout->node_count;++i) {
        layout->nodes[i].x=focus->focused[i].x; layout->nodes[i].y=focus->focused[i].y;
    }
    focus_bounds(layout,scale);
}
static void focus_clearance(GD_LayoutFocus *focus, GD_Layout *layout, int count, float scale, int include_inactive) {
    int i,pass; unsigned work=0,limit=(unsigned)GD_MAX_RECORDS*GD_MAX_RECORDS;
    if (!layout->direct_routes || layout->mode!=GD_LAYOUT_DIRECTED) return;
    for (i=0;i<count;++i) {
        int node=focus->order[i].node;
        focus->order[i].distance=0;
        focus->order[i].angle=layout->horizontal ? layout->nodes[node].y:layout->nodes[node].x;
    }
    qsort(focus->order,(size_t)count,sizeof(focus->order[0]),focus_order);
    for (pass=0;pass<2;++pass) for (i=0;i<count;++i) {
        int node=focus->order[i].node,e; GD_LayoutNode *item=&layout->nodes[node];
        float cross=layout->horizontal ? item->y:item->x,axis=layout->horizontal ? item->x:item->y;
        float minimum=-1e30f,maximum=1e30f,half=cross_span(item,layout->horizontal,scale)/2;
        if (focus->fixed_roots[node]) continue;
        if (i && focus->order[i-1].band==focus->order[i].band) {
            const GD_LayoutNode *peer=&layout->nodes[focus->order[i-1].node];
            minimum=(layout->horizontal ? peer->y:peer->x)+cross_span(peer,layout->horizontal,scale)/2+half+16/scale;
        }
        if (i+1<count && focus->order[i+1].band==focus->order[i].band) {
            const GD_LayoutNode *peer=&layout->nodes[focus->order[i+1].node];
            maximum=(layout->horizontal ? peer->y:peer->x)-cross_span(peer,layout->horizontal,scale)/2-half-16/scale;
        }
        for (e=0;e<layout->edge_count;++e) {
            const GD_LayoutEdge *edge=&layout->edges[e]; const GD_LayoutNode *a,*b;
            float aa,ba,t,line,left,right;
            if (++work>limit) return;
            if ((!include_inactive && !edge->active) || edge->a==node || edge->b==node ||
                focus->distance[edge->a]<0 || focus->distance[edge->b]<0) continue;
            a=&layout->nodes[edge->a]; b=&layout->nodes[edge->b];
            aa=layout->horizontal ? a->x:a->y; ba=layout->horizontal ? b->x:b->y;
            if (aa==ba || axis<=fminf(aa,ba) || axis>=fmaxf(aa,ba)) continue;
            t=(axis-aa)/(ba-aa); line=layout->horizontal ? a->y+t*(b->y-a->y):a->x+t*(b->x-a->x);
            if (fabsf(cross-line)>=40/scale) continue;
            left=line-40/scale; right=line+40/scale;
            if (left>=minimum && left<=maximum && (right>maximum || fabsf(left-cross)<=fabsf(right-cross))) cross=left;
            else if (right>=minimum && right<=maximum) cross=right;
        }
        if (layout->horizontal) item->y=cross; else item->x=cross;
    }
}
int gd_layout_focus_apply(GD_LayoutFocus *focus, GD_Layout *layout,
    const unsigned char *shown, int include_inactive, int directed, int root, float scale) {
    int i,e,first=0,last=0,count=0,changed,horizontal,banded;
    float unit,root_x,root_y,root_cross,outer=0;
    if (!focus || !layout || layout->node_count<0 || layout->node_count>GD_MAX_RECORDS ||
        layout->edge_count<0 || layout->edge_count>GD_MAX_PAIRS || layout->group_count<0 ||
        layout->group_count>GD_MAX_RECORDS || !isfinite(scale) || scale<=0) return -1;
    unit=120/scale;
    for (i=0;i<layout->node_count;++i) {
        const GD_LayoutNode *node=&layout->nodes[i];
        if (!isfinite(node->x) || !isfinite(node->y) || node->group<0 || node->group>=layout->group_count) return -1;
    }
    for (e=0;e<layout->edge_count;++e) if (layout->edges[e].a<0 || layout->edges[e].a>=layout->node_count ||
        layout->edges[e].b<0 || layout->edges[e].b>=layout->node_count) return -1;
    if (!focus->captured || focus->node_count!=layout->node_count || focus->group_count!=layout->group_count) gd_layout_focus_capture(focus,layout);
    for (i=0;i<layout->node_count;++i) if (focus->base[i].key!=layout->nodes[i].key) { gd_layout_focus_capture(focus,layout); break; }
    gd_layout_focus_restore(focus,layout);
    changed=!focus->active || root!=focus->anchor_node; focus->active=0; focus->anchor_node=-1;
    for (i=0;i<layout->node_count;++i) {
        focus->head[i]=-1; focus->distance[i]=shown && !shown[i] ? -2:-1; focus->visited[i]=0;
        focus->fixed_roots[i]=(unsigned char)(directed && focus->distance[i]!=-2);
    }
    for (e=0;e<layout->edge_count;++e) {
        const GD_LayoutEdge *edge=&layout->edges[e];
        if ((!include_inactive && !edge->active) || focus->distance[edge->a]==-2 || focus->distance[edge->b]==-2) continue;
        focus->fixed_roots[edge->b]=0;
        focus->next[2*e]=focus->head[edge->a]; focus->head[edge->a]=2*e;
        focus->next[2*e+1]=focus->head[edge->b]; focus->head[edge->b]=2*e+1;
    }
    if (root<0 || root>=layout->node_count || focus->distance[root]==-2) return 0;
    focus->distance[root]=0; focus->visited[root]=1; focus->queue[last++]=root;
    while (first<last) {
        int node=focus->queue[first++],arc;
        for (arc=focus->head[node];arc>=0;arc=focus->next[arc]) {
            const GD_LayoutEdge *edge=&layout->edges[arc/2]; int other=arc%2 ? edge->a:edge->b;
            if (focus->visited[other]) continue;
            focus->visited[other]=1; focus->distance[other]=focus->distance[node]+1; focus->queue[last++]=other;
        }
    }
    banded=layout->mode!=GD_LAYOUT_FORCE;
    horizontal=layout->mode==GD_LAYOUT_COLUMNS || layout->horizontal;
    root_x=focus->base[root].x; root_y=focus->base[root].y; root_cross=horizontal ? root_y:root_x;
    for (i=0;i<layout->node_count;++i) if (focus->distance[i]>=0 || (banded && focus->fixed_roots[i])) {
        const GD_FocusPose *base=&focus->base[i]; GD_FocusOrder *item=&focus->order[count++];
        float span=hypotf(gd_layout_node_width(&layout->nodes[i]),layout->nodes[i].height>0 ? layout->nodes[i].height:120)/scale+32/scale;
        if (span>unit) unit=span;
        item->node=i; item->distance=focus->distance[i]>=0 ? focus->distance[i]:GD_MAX_RECORDS+1; item->key=layout->nodes[i].key;
        item->band=banded ? (horizontal ? base->x:base->y):0;
        item->cross=(horizontal ? base->y:base->x)-root_cross;
        item->angle=banded ? fabsf(item->cross):atan2f(base->y-root_y,base->x-root_x);
    }
    qsort(focus->order,(size_t)count,sizeof(focus->order[0]),focus_order);
    if (banded) for (first=0;first<count;) {
        int end=first+1,previous=-1,fixed=0; float left=0,right=0,floor=0;
        float gap=(layout->direct_routes ? 96:24)/scale;
        while (end<count && focus->order[end].band==focus->order[first].band) ++end;
        for (i=first;i<end;++i) if (focus->fixed_roots[focus->order[i].node]) {
            int node=focus->order[i].node;
            float edge=(horizontal ? focus->base[node].y:focus->base[node].x)+cross_span(&layout->nodes[node],horizontal,scale)/2-root_cross;
            if (!fixed || edge>right) right=edge;
            fixed=1;
        }
        if (fixed) { right+=gap; left=right; }
        for (i=first;i<end;++i) {
            GD_FocusOrder *item=&focus->order[i]; GD_LayoutNode *node=&layout->nodes[item->node];
            float half=cross_span(node,horizontal,scale)/2,offset;
            if (focus->fixed_roots[item->node]) continue;
            if (item->distance!=previous) { floor=fmaxf(left,right)+(previous<0 ? item->distance>0 ? (item->distance-1)*unit:0:(item->distance-previous)*unit); previous=item->distance; }
            if (fixed) { offset=fmaxf(floor,right+half); right=offset+half+gap; }
            else if (item->node==root || (i==first && item->distance==1)) { offset=0; left=right=half+gap; }
            else if (left<right || (left==right && item->cross<0)) {
                offset=-fmaxf(floor,left+half); left=-offset+half+gap;
            } else { offset=fmaxf(floor,right+half); right=offset+half+gap; }
            if (horizontal) node->y=root_cross+offset; else node->x=root_cross+offset;
        }
        first=end;
    } else {
        float radius=0;
        for (first=0;first<count;) {
            int end=first+1,distance=focus->order[first].distance,n;
            while (end<count && focus->order[end].distance==distance) ++end;
            n=end-first;
            if (distance) radius=fmaxf(radius+unit*1.5f,n*unit/6.28318530718f);
            for (i=first;i<end;++i) {
                GD_LayoutNode *node=&layout->nodes[focus->order[i].node];
                float angle=focus->order[first].angle+6.28318530718f*(i-first)/n;
                if (!focus->fixed_roots[focus->order[i].node]) {
                    node->x=root_x+cosf(angle)*radius; node->y=root_y+sinf(angle)*radius;
                }
            }
            first=end;
        }
    }
    if (!banded) {
        int roots=0,movable=0,j; float min_x=0,max_x=0,min_y=0,max_y=0,shift=0;
        /* Move unpinned primary geometry together: independent repairs can
           collapse distinct distance rings onto the same root boundary. */
        for (i=0;i<layout->node_count;++i) {
            const GD_LayoutNode *node=&layout->nodes[i];
            float half=gd_layout_node_width(node)/(2*scale),height=cross_span(node,1,scale)/2;
            if (focus->fixed_roots[i]) {
                GD_FocusOrder *item=&focus->order[roots++];
                *item=(GD_FocusOrder){i,0,node->key,0,node->y+height,0};
            } else if (focus->distance[i]>=0) {
                if (!movable || node->x-half<min_x) min_x=node->x-half;
                if (!movable || node->x+half>max_x) max_x=node->x+half;
                if (!movable || node->y-height<min_y) min_y=node->y-height;
                if (!movable || node->y+height>max_y) max_y=node->y+height;
                ++movable;
            }
        }
        qsort(focus->order,(size_t)roots,sizeof(focus->order[0]),focus_order);
        for (j=0;movable && j<roots;++j) {
            const GD_LayoutNode *pin=&layout->nodes[focus->order[j].node];
            float half=gd_layout_node_width(pin)/(2*scale)+24/scale,height=cross_span(pin,1,scale)/2+24/scale;
            if (max_x>pin->x-half && min_x<pin->x+half && max_y+shift>pin->y-height && min_y+shift<pin->y+height)
                shift=pin->y+height-min_y;
        }
        if (shift) for (i=0;i<layout->node_count;++i)
            if (focus->distance[i]>=0 && !focus->fixed_roots[i]) layout->nodes[i].y+=shift;
    } else focus_clearance(focus,layout,count,scale,include_inactive);
    for (i=0;i<layout->node_count;++i) if (focus->distance[i]>=0) {
        GD_LayoutNode *node=&layout->nodes[i]; float radius=hypotf(node->x-root_x,node->y-root_y)+unit;
        if (radius>outer) outer=radius;
    }
    count=0;
    for (i=0;i<layout->node_count;++i) if (focus->fixed_roots[i]) {
        GD_FocusOrder *item=&focus->order[count++];
        *item=(GD_FocusOrder){i,0,layout->nodes[i].key,0,
            (horizontal ? layout->nodes[i].y:layout->nodes[i].x)+cross_span(&layout->nodes[i],horizontal,scale)/2,0};
    }
    qsort(focus->order,(size_t)count,sizeof(focus->order[0]),focus_order);
    /* Fixed roots anchor their components. Move remaining disconnected
       unpinned portions rigidly; never translate a structural root. */
    for (i=0;i<layout->node_count;++i) if (focus->fixed_roots[i]) focus->visited[i]=1;
    for (i=0;i<layout->node_count;++i) if (focus->distance[i]==-1 && !focus->visited[i]) {
        float minimum=0,maximum=0,axis_min=0,axis_max=0,shift; int j;
        first=last=0; focus->queue[last++]=i; focus->visited[i]=1;
        while (first<last) {
            int node=focus->queue[first++],arc; float cross=horizontal ? layout->nodes[node].y:layout->nodes[node].x;
            float half=cross_span(&layout->nodes[node],horizontal,scale)/2;
            float axis=horizontal ? layout->nodes[node].x:layout->nodes[node].y;
            float along=cross_span(&layout->nodes[node],!horizontal,scale)/2;
            if (first==1 || axis-along<axis_min) axis_min=axis-along;
            if (first==1 || axis+along>axis_max) axis_max=axis+along;
            if (first==1 || cross-half<minimum) minimum=cross-half;
            if (first==1 || cross+half>maximum) maximum=cross+half;
            for (arc=focus->head[node];arc>=0;arc=focus->next[arc]) {
                const GD_LayoutEdge *edge=&layout->edges[arc/2]; int other=arc%2 ? edge->a:edge->b;
                if (focus->visited[other]) continue;
                focus->visited[other]=1; focus->queue[last++]=other;
            }
        }
        shift=root_cross+outer-minimum;
        for (j=0;j<count;++j) {
            const GD_LayoutNode *pin=&layout->nodes[focus->order[j].node];
            float cross=horizontal ? pin->y:pin->x,axis=horizontal ? pin->x:pin->y;
            float half=cross_span(pin,horizontal,scale)/2+24/scale,along=cross_span(pin,!horizontal,scale)/2+24/scale;
            if (axis_max>axis-along && axis_min<axis+along && maximum+shift>cross-half && minimum+shift<cross+half)
                shift=cross+half-minimum;
        }
        for (j=0;j<last;++j) {
            GD_LayoutNode *node=&layout->nodes[focus->queue[j]];
            if (horizontal) node->y+=shift; else node->x+=shift;
        }
        outer=fmaxf(outer,maximum+shift-root_cross+unit);
    }
    focus_bounds(layout,scale);
    for (i=0;i<layout->node_count;++i) focus->focused[i]=(GD_LayoutPoint){layout->nodes[i].x,layout->nodes[i].y};
    focus->anchor_node=root; focus->active=1;
    return changed ? 2:1;
}
