/* Shared optional workspace. Fixed after explicit preparation; no pinning,
   per-frame allocation, graph rewrite or rendered-position filtering. */
#ifndef GD_LAYOUT_WORKSPACE_H
#define GD_LAYOUT_WORKSPACE_H
int gd_layout_workspace(GD_Layout *layout,int enabled,float aspect) {
    GD_LayoutWorkspace candidate={0};double area=0,left=DBL_MAX,top=DBL_MAX,right=-DBL_MAX,bottom=-DBL_MAX,max_width=0,max_height=0,height,width,cx,cy;int i,springs=0;
    if(!layout || (enabled!=0 && enabled!=1))return 0;
    if(!enabled){layout->workspace=candidate;return 1;}
    if(layout->mode!=GD_LAYOUT_FORCE || !isfinite(aspect) || aspect<=0 || layout->node_count<0 || layout->node_count>GD_MAX_RECORDS || layout->edge_count<0 || layout->edge_count>GD_MAX_PAIRS)return 0;
    for(i=0;i<layout->node_count;++i){
        const GD_LayoutNode *node=&layout->nodes[i];const GD_LayoutBounds *b=&node->footprint;double w=(double)b->right-b->left,h=(double)b->bottom-b->top;
        if(!isfinite(node->x)||!isfinite(node->y)||!isfinite(w)||!isfinite(h)||!isfinite(b->left)||!isfinite(b->top)||w<=0||h<=0)return 0;
        area+=w*h;max_width=fmax(max_width,w);max_height=fmax(max_height,h);
        left=fmin(left,(double)node->x+b->left);top=fmin(top,(double)node->y+b->top);right=fmax(right,(double)node->x+b->right);bottom=fmax(bottom,(double)node->y+b->bottom);
    }
    for(i=0;i<layout->edge_count;++i){const GD_LayoutEdge *edge=&layout->edges[i];if(edge->a<0 || edge->a>=layout->node_count || edge->b<0 || edge->b>=layout->node_count)return 0;if(edge->active && edge->a!=edge->b)++springs;}
    if(!layout->node_count){layout->workspace=candidate;return 1;}
    /* Spring density, not distinct-neighbor degree: retained parallel source
       springs can add pressure. Cap base spare-area multiplier at eight. */
    area*=4+fmin(4,2.0*springs/layout->node_count);
    height=fmax(sqrt(area/aspect),fmax(fmax(max_height+16,(max_width+16)/aspect),fmax(bottom-top+16,(right-left+16)/aspect)));height*=1.2; /* Review trial: 20% more space on each axis. */width=height*aspect;cx=(left+right)/2;cy=(top+bottom)/2;
    candidate.bounds=(GD_LayoutBounds){(float)(cx-width/2),(float)(cy-height/2),(float)(cx+width/2),(float)(cy+height/2)};candidate.enabled=1;
    if(!isfinite(candidate.bounds.left)||!isfinite(candidate.bounds.top)||!isfinite(candidate.bounds.right)||!isfinite(candidate.bounds.bottom)||candidate.bounds.left>=candidate.bounds.right||candidate.bounds.top>=candidate.bounds.bottom)return 0;
    layout->workspace=candidate;return 1;
}
static void workspace_contain(GD_Layout *layout,int held) {
    const GD_LayoutBounds *b=&layout->workspace.bounds;int i;
    if(!layout->workspace.enabled || layout->mode!=GD_LAYOUT_FORCE)return;
    for(i=0;i<layout->node_count;++i){
        GD_LayoutNode *node=&layout->nodes[i];float left=b->left-node->footprint.left,right=b->right-node->footprint.right,top=b->top-node->footprint.top,bottom=b->bottom-node->footprint.bottom;
        if(i==held)continue; /* Pointer owns held pose; release returns it inside. */
        if(left>right){node->x=(left+right)/2;node->vx=0;}
        else if(node->x<left){node->x=left;if(node->vx<0)node->vx=0;}else if(node->x>right){node->x=right;if(node->vx>0)node->vx=0;}
        if(top>bottom){node->y=(top+bottom)/2;node->vy=0;}
        else if(node->y<top){node->y=top;if(node->vy<0)node->vy=0;}else if(node->y>bottom){node->y=bottom;if(node->vy>0)node->vy=0;}
    }
}
#endif
