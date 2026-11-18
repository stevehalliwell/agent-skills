/* Private deterministic drawing pass. Included after LayoutOrder definition.
   Four full edge sweeps + bounded sorts; no source or layer mutation. */
static int directed_compare(const void *left, const void *right) {
    const LayoutOrder *a=left,*b=right; double x,y;
    if (a->group!=b->group) return a->group<b->group ? -1:1;
    if (a->layer!=b->layer) return a->layer<b->layer ? -1:1;
    x=a->sum*b->count; y=b->sum*a->count;
    if (x!=y) return x<y ? -1:1;
    return (a->previous>b->previous)-(a->previous<b->previous);
}
static void directed_order(GD_Layout *layout, LayoutOrder *order) {
    int rank[GD_MAX_RECORDS],sum[GD_MAX_RECORDS],count[GD_MAX_RECORDS]; int pass,i,begin;
    for (pass=0;pass<4;++pass) {
        for (begin=0;begin<layout->node_count;) {
            int end=begin+1;
            while (end<layout->node_count && order[end].group==order[begin].group && order[end].layer==order[begin].layer) ++end;
            for (i=begin;i<end;++i) rank[order[i].node]=end-begin>1 ? (i-begin)*GD_MAX_RECORDS/(end-begin-1):GD_MAX_RECORDS/2;
            begin=end;
        }
        memset(sum,0,(size_t)layout->node_count*sizeof(int)); memset(count,0,(size_t)layout->node_count*sizeof(int));
        for (i=0;i<layout->edge_count;++i) {
            const GD_LayoutEdge *edge=&layout->edges[i]; int owner=edge->a,neighbor=edge->b,swap;
            const GD_LayoutNode *a=&layout->nodes[owner],*b=&layout->nodes[neighbor];
            if (!edge->active || a->group!=b->group || a->layer<0 || b->layer<0 || a->layer==b->layer) continue;
            /* Ordering is geometric, irrespective of authored arrow direction.
               Backward MDA basis declarations must keep their direction. */
            if ((a->layer<b->layer)==(pass%2==0)) { swap=owner; owner=neighbor; neighbor=swap; }
            sum[owner]+=rank[neighbor]; ++count[owner];
        }
        for (i=0;i<layout->node_count;++i) {
            int node=order[i].node;
            order[i].sum=count[node] ? sum[node]:rank[node]; order[i].count=count[node] ? count[node]:1; order[i].previous=i;
        }
        qsort(order,(size_t)layout->node_count,sizeof(*order),directed_compare);
    }
    for (i=0;i<layout->node_count;++i) { layout->order[i]=order[i].node; layout->position[order[i].node]=i; }
}
static float directed_span(const GD_Layout *layout,const GD_LayoutNode *node,int x_axis) {
    float span=x_axis ? gd_layout_node_width(node):node->height>0 ? node->height:120;
    if(!layout->align_ranks)return span;
    return fmaxf(span,2*(x_axis ? fmaxf(-node->footprint.left,node->footprint.right):fmaxf(-node->footprint.top,node->footprint.bottom)));
}
static float directed_cross(const GD_Layout *layout,const GD_LayoutNode *node) {
    return layout->horizontal ? node->y:node->x;
}
/* Keep within-band order. Other ranks never consume lateral room. */
static float rank_clamp_cross(const GD_Layout *layout,int index,float cross,float margin) {
    const GD_LayoutNode *node=&layout->nodes[index];const GD_LayoutGroup *group=&layout->groups[node->group];
    float start=layout->horizontal ? group->y:group->x,extent=layout->horizontal ? group->height:group->width;
    float half=directed_span(layout,node,!layout->horizontal)/2,lo=start+half+margin,hi=start+extent-half-margin;
    int i=layout->position[index],side;
    for(side=-1;side<=1;side+=2)if(i+side>=group->first && i+side<group->first+group->count){
        const GD_LayoutNode *peer=&layout->nodes[layout->order[i+side]];
        if(peer->layer==node->layer){
            float gap=half+directed_span(layout,peer,!layout->horizontal)/2+16;
            if(side<0)lo=fmaxf(lo,directed_cross(layout,peer)+gap);
            else hi=fminf(hi,directed_cross(layout,peer)-gap);
        }
    }
    return lo<=hi ? fminf(hi,fmaxf(lo,cross)):directed_cross(layout,node);
}
/* Eight edge/vertex sweeps. Favor cross-rank neighbor alignment without
   changing band membership, within-band order or footprint clearance. */
static void rank_align(GD_Layout *layout,LayoutOrder *scratch) {
    int pass,i;
    /* Order/position now live in the layout. Reuse sort scratch by node. */
    for(pass=0;pass<8;++pass){
        for(i=0;i<layout->node_count;++i){scratch[i].sum=0;scratch[i].count=0;}
        for(i=0;i<layout->edge_count;++i){
            const GD_LayoutEdge *edge=&layout->edges[i];const GD_LayoutNode *a=&layout->nodes[edge->a],*b=&layout->nodes[edge->b];
            if(!edge->active || a->group!=b->group || a->layer==b->layer || a->layer<0 || b->layer<0)continue;
            scratch[edge->a].sum+=b->anchor;++scratch[edge->a].count;
            scratch[edge->b].sum+=a->anchor;++scratch[edge->b].count;
        }
        for(i=0;i<layout->node_count;++i){
            int index=layout->order[pass%2 ? layout->node_count-i-1:i];GD_LayoutNode *node=&layout->nodes[index];
            if(!scratch[index].count)continue;
            node->anchor=rank_clamp_cross(layout,index,(float)(0.4*node->anchor+0.6*scratch[index].sum/scratch[index].count),16);
            if(layout->horizontal)node->y=node->anchor;else node->x=node->anchor;
        }
    }
}
/* Straight long links must not turn unrelated intermediate nodes into apparent
   incidences. Move within existing band order, never route through a side lane.
   Two passes, at most node-capacity squared edge checks; not an obstacle solver. */
static void directed_clearance(GD_Layout *layout) {
    unsigned work=0; int pass,i,e;
    for (pass=0;pass<2;++pass) for (i=0;i<layout->node_count;++i) {
        GD_LayoutNode *node=&layout->nodes[layout->order[i]];
        float cross=layout->horizontal ? node->y:node->x;
        float axis=layout->horizontal ? node->x:node->y;
        float span=directed_span(layout,node,!layout->horizontal);
        float minimum=cross-64,maximum=cross+64;
        int neighbor;
        if(layout->align_ranks){
            minimum=rank_clamp_cross(layout,layout->order[i],minimum,16);
            maximum=rank_clamp_cross(layout,layout->order[i],maximum,16);
        }
        for (neighbor=-1;neighbor<=1;neighbor+=2) if (i+neighbor>=0 && i+neighbor<layout->node_count) {
            const GD_LayoutNode *other=&layout->nodes[layout->order[i+neighbor]];
            if (other->group==node->group && other->layer==node->layer) {
                float other_cross=layout->horizontal ? other->y:other->x;
                float other_span=directed_span(layout,other,!layout->horizontal);
                if (neighbor<0) minimum=fmaxf(minimum,other_cross+(span+other_span)/2+16);
                else maximum=fminf(maximum,other_cross-(span+other_span)/2-16);
            }
        }
        for (e=0;e<layout->edge_count;++e) {
            const GD_LayoutEdge *edge=&layout->edges[e];
            const GD_LayoutNode *a=&layout->nodes[edge->a],*b=&layout->nodes[edge->b];
            float aa,ba,ac,bc,intercept,left,right;
            if (++work>(unsigned)GD_MAX_RECORDS*GD_MAX_RECORDS) return;
            if (edge->a==layout->order[i] || edge->b==layout->order[i] || a->group!=node->group || b->group!=node->group) continue;
            aa=layout->horizontal ? a->x:a->y; ba=layout->horizontal ? b->x:b->y;
            if (axis<=fminf(aa,ba) || axis>=fmaxf(aa,ba)) continue;
            ac=layout->horizontal ? a->y:a->x; bc=layout->horizontal ? b->y:b->x;
            intercept=ac+(bc-ac)*(axis-aa)/(ba-aa);
            if (fabsf(cross-intercept)>=40) continue;
            left=intercept-40; right=intercept+40;
            if (left>=minimum && left<=maximum && (right>maximum || cross-left<=right-cross)) cross=left;
            else if (right>=minimum && right<=maximum) cross=right;
            node->anchor=cross;
            if (layout->horizontal) node->y=cross; else node->x=cross;
        }
    }
}
static int directed_arrange(GD_Layout *layout, LayoutOrder *order, GD_LayoutSort sort) {
    float *axis_span=layout->layer_span,axis[GD_MAX_RECORDS+2];
    float band_width[GD_MAX_RECORDS+2],used[GD_MAX_RECORDS+2];
    int i,maximum=-1; float cross=0,total_axis=80,spacing=layout->direct_routes ? 96:48;
    if (!layout->node_count) {
        layout->width=layout->height=0; layout->mode=GD_LAYOUT_DIRECTED; layout->sort=sort; layout->tick=0; return 1;
    }
    memset(axis_span,0,sizeof(layout->layer_span));
    directed_order(layout,order);
    for (i=0;i<layout->node_count;++i) if (layout->nodes[i].layer>maximum) maximum=layout->nodes[i].layer;
    for (i=0;i<layout->node_count;++i) {
        GD_LayoutNode *node=&layout->nodes[i]; int slot=node->layer<0 ? maximum+2:node->layer;
        float span=directed_span(layout,node,layout->horizontal);
        if (span>axis_span[slot]) axis_span[slot]=span;
    }
    layout->directed_shelf=maximum+2;
    for (i=0;i<=maximum+2;++i) {
        float span=axis_span[i]>0 ? axis_span[i]:120;
        axis[i]=total_axis+span/2; total_axis+=span+80;
    }
    for (i=0;i<layout->group_count;++i) {
        GD_LayoutGroup *group=&layout->groups[i]; int j; float widest=0;
        memset(band_width,0,sizeof(band_width)); memset(used,0,sizeof(used));
        for (j=0;j<group->count;++j) {
            const GD_LayoutNode *node=&layout->nodes[layout->order[group->first+j]];
            int slot=node->layer<0 ? maximum+2:node->layer;
            band_width[slot]+=directed_span(layout,node,!layout->horizontal)+spacing;
        }
        for (j=0;j<=maximum+2;++j) if (band_width[j]>0) {
            band_width[j]-=spacing; if (band_width[j]>widest) widest=band_width[j];
        }
        group->x=cross; group->y=0; group->width=widest+96; group->height=total_axis;
        for (j=0;j<group->count;++j) {
            GD_LayoutNode *node=&layout->nodes[layout->order[group->first+j]];
            int slot=node->layer<0 ? maximum+2:node->layer;
            float span=directed_span(layout,node,!layout->horizontal);
            node->anchor=cross+48+(widest-band_width[slot])/2+used[slot]+span/2;
            used[slot]+=span+spacing; node->axis=axis[slot]; node->vx=node->vy=0;
            node->x=layout->horizontal ? node->axis:node->anchor; node->y=layout->horizontal ? node->anchor:node->axis;
        }
        cross+=group->width+96+layout->route_lanes[i]*12.0f;
        if (layout->horizontal) {
            float swap=group->x; group->x=group->y; group->y=swap;
            swap=group->width; group->width=group->height; group->height=swap;
        }
    }
    if (layout->align_ranks) rank_align(layout,order);
    if (layout->direct_routes) directed_clearance(layout);
    layout->width=layout->horizontal ? total_axis:fmaxf(0,cross-48);
    layout->height=layout->horizontal ? fmaxf(0,cross-48):total_axis;
    layout->mode=GD_LAYOUT_DIRECTED; layout->sort=sort; layout->tick=0; return 1;
}
