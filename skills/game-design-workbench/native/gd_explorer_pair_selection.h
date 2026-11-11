/* Retained matrix selection is a node or typed endpoint pair, not a new edge.
   Drawing/framing only: never mutate layout, filters, source or evidence. */
#ifndef GD_EXPLORER_PAIR_SELECTION_H
#define GD_EXPLORER_PAIR_SELECTION_H
static int pair_selection_nodes(const GD_ExplorerContext *c,int nodes[2]) {
    const GD_ExplorerPane *p=c->pane;const GD_Model *model;int a=-1,b=-1,i,count=0;
    if((p->view!=GD_EXPLORER_MATRIX && p->view!=GD_EXPLORER_NOUNS) || !p->layout)return 0;
    model=gd_explorer_model(c->project,p->view);if(!model)return 0;
    if(p->selection.node>=0 && p->selection.node<p->layout->node_count){nodes[0]=p->selection.node;return 1;}
    if(p->selection.cell>=0){
        if(p->view==GD_EXPLORER_MATRIX && p->selection.cell<model->cell_count){const GD_Cell *cell=&model->cells[p->selection.cell];a=cell->noun;b=cell->verb;}
        if(p->view==GD_EXPLORER_NOUNS && p->selection.cell<c->project->noun_grid->pair_count){const GD_NounPair *pair=&c->project->noun_grid->pairs[p->selection.cell];a=pair->a;b=pair->b;}
    }else if(p->selection.linked<0){a=p->grid.noun;b=p->grid.verb;}
    if(a<0 || b<0 || a>=model->record_count || b>=model->record_count || model->records[a].role!=GD_NOUN || model->records[b].role!=(p->view==GD_EXPLORER_NOUNS ? GD_NOUN:GD_VERB))return 0;
    for(i=0;i<p->layout->node_count && count<2;++i)if(p->layout->nodes[i].key==a || p->layout->nodes[i].key==b)nodes[count++]=i;
    return count;
}
static int pair_selection_bounds(GD_ExplorerContext *c,Rectangle *bounds) {
    const GD_ExplorerPane *p=c->pane;const GD_Layout *shown=gd_explorer_display_layout(c);int nodes[2],count=pair_selection_nodes(c,nodes),i,found=0;
    for(i=0;i<count;++i){const GD_LayoutNode *node=&shown->nodes[nodes[i]];const GD_LayoutBounds *b=&p->layout->nodes[nodes[i]].footprint;
        grid_union(bounds,(Rectangle){node->x+b->left,node->y+b->top,b->right-b->left,b->bottom-b->top},&found);
    }
    if(found && p->selection.cell>=0)for(i=0;i<shown->edge_count;++i)if(shown->edges[i].key==p->selection.cell && (p->view!=GD_EXPLORER_MATRIX || shown->edges[i].active || p->overlay)){
        GD_LayoutRoute route;int j;gd_layout_route(shown,i,&route);for(j=0;j<route.count;++j)grid_union(bounds,(Rectangle){route.points[j].x,route.points[j].y,0,0},&found);break;
    }
    return found;
}
static void pair_selection_draw(GD_ExplorerContext *c) {
    GD_ExplorerPane *p=c->pane;const GD_Layout *shown=gd_explorer_display_layout(c);int nodes[2],count=pair_selection_nodes(c,nodes),i;
    if(!count || grid_view(p) || gd_explorer_similarity_active(c))return;
    BeginScissorMode((int)c->viewport.x,(int)c->viewport.y,(int)c->viewport.width,(int)c->viewport.height);BeginMode2D(p->camera);
    for(i=0;i<count;++i){const GD_LayoutNode *node=&shown->nodes[nodes[i]];const GD_LayoutBounds *b=&p->layout->nodes[nodes[i]].footprint;float pad=5/p->camera.zoom;
        DrawRectangleLinesEx((Rectangle){node->x+b->left-pad,node->y+b->top-pad,b->right-b->left+2*pad,b->bottom-b->top+2*pad},3/p->camera.zoom,gold);
    }
    EndMode2D();EndScissorMode();
    if(p->selection.node<0 && p->selection.cell<0)DrawText("Selected endpoints: no declared cell/pair; no link inferred.",(int)c->viewport.x+8,(int)c->viewport.y+8,14,gold);
    else if(p->view==GD_EXPLORER_MATRIX && p->selection.cell>=0 && p->selection.cell<gd_explorer_model(c->project,p->view)->cell_count && !p->overlay && !gd_explorer_model(c->project,p->view)->cells[p->selection.cell].active)DrawText("Selected endpoints retained; inactive link hidden by overlay setting.",(int)c->viewport.x+8,(int)c->viewport.y+8,14,gold);
}
#endif
