/* Camera-only selection framing. Included by explorer owner after pair helpers.
   One bounded node mask, one-hop edges only; no allocation or solver writes. */
#ifndef GD_EXPLORER_FOCUS_SELECTION_H
#define GD_EXPLORER_FOCUS_SELECTION_H
static const char *focus_status_name(GD_ExplorerFocusStatus status) {
    static const char *names[]={"none","focused","partial","hidden","unrepresented","unplaced"};
    return status>=GD_FOCUS_NONE && status<=GD_FOCUS_UNPLACED ? names[status]:"none";
}
static float focus_fit(Rectangle viewport,Rectangle bounds) {
    return fminf(viewport.width/(bounds.width+60),viewport.height/(bounds.height+60));
}
void gd_explorer_focus_camera(Camera2D *camera,Rectangle viewport,Rectangle selected,Rectangle context,Vector2 primary,GD_ExplorerFocusResult *result) {
    float zoom=focus_fit(viewport,context);Vector2 target={context.x+context.width/2,context.y+context.height/2};
    if(zoom<1){zoom=focus_fit(viewport,selected);target=(Vector2){selected.x+selected.width/2,selected.y+selected.height/2};if(zoom<1){zoom=1;target=primary;}}
    camera->zoom=fminf(1.2f,zoom);camera->target=target;camera->offset=(Vector2){viewport.x+viewport.width/2,viewport.y+viewport.height/2};
    result->zoom=camera->zoom;
    result->clipped=(fabsf(target.x-context.x-context.width/2)+context.width/2)*camera->zoom>viewport.width/2 ||
        (fabsf(target.y-context.y-context.height/2)+context.height/2)*camera->zoom>viewport.height/2;
}
int gd_explorer_focus_available(const GD_ExplorerContext *c) {
    const GD_ExplorerPane *p=c->pane;
    if(!p->prepared || !p->layout)return 0;
    if(gd_explorer_similarity_active(c))return p->structure_state->similarity_row>=0;
    if(p->view==GD_EXPLORER_MDA)return p->mda.record>=0 || p->mda.link>=0;
    if(p->selection.node>=0 || p->selection.cell>=0 || p->selection.linked>=0)return 1;
    return (p->view==GD_EXPLORER_MATRIX || p->view==GD_EXPLORER_NOUNS) && p->grid.noun>=0 && p->grid.verb>=0;
}
static Rectangle focus_node_bounds(GD_ExplorerContext *c,const GD_Layout *shown,int index) {
    const GD_ExplorerPane *p=c->pane;const GD_LayoutNode *node=&shown->nodes[index];
    if(grid_view(p)){int key=p->layout->nodes[index].key;const GD_Model *model=gd_explorer_model(c->project,p->view);return p->view==GD_EXPLORER_NOUNS ? noun_grid_bounds(p,key,-1):grid_bounds(&p->grid,model->records[key].role==GD_NOUN ? key:-1,model->records[key].role==GD_VERB ? key:-1);}
    if(query_view(p->view))return (Rectangle){node->x-node->width/2,node->y-node->height/2,node->width,node->height};
    {
        const GD_LayoutBounds *b=&p->layout->nodes[index].footprint;float scale=p->view==GD_EXPLORER_MDA && p->layout->mode==GD_LAYOUT_FORCE ? 3:1;
        float half=p->view==GD_EXPLORER_MDA ? 121:110,left=fminf(-half,b->left*scale),right=fmaxf(half,b->right*scale),top=fminf(-48,b->top*scale),bottom=fmaxf(48,b->bottom*scale);
        return (Rectangle){node->x*scale+left,node->y*scale+top,right-left,bottom-top};
    }
}
static int focus_source_record(GD_ExplorerContext *c,int node) {
    GD_ExplorerPane *p=c->pane;int key=p->layout->nodes[node].key;
    if(query_view(p->view))return p->query_state && p->query_state->scene ? gd_query_scene_record(p->query_state->scene,node):-1;
    return p->view==GD_EXPLORER_RESOURCES ? gd_explorer_resource_record(c->project->resources,key):key;
}
static int focus_edge_shown(GD_ExplorerContext *c,int index) {
    const GD_LayoutEdge *edge=&c->pane->layout->edges[index];
    return visible(c,edge->a) && visible(c,edge->b) && (c->pane->view!=GD_EXPLORER_MATRIX || edge->active || c->pane->overlay);
}
int gd_explorer_focus_selection(GD_ExplorerContext *c) {
    GD_ExplorerPane *p=c->pane;const GD_Layout *shown;GD_ExplorerFocusResult result={0};
    unsigned char marked[GD_MAX_RECORDS]={0};Rectangle selected={0},context={0},primary_bounds={0};int found=0,primary=-1,edge=-1,i,count=0,nodes[2];
    if(!gd_explorer_focus_available(c) || !(c->viewport.width>0) || !(c->viewport.height>0) || !isfinite(c->viewport.width) || !isfinite(c->viewport.height))return 0;
    if(gd_explorer_similarity_active(c))return gd_explorer_similarity_focus(c);
    /* Borrow matching active pose only inside this operation. Never expose
       filtered positions through node IPC, reset sampler or borrow other pane. */
    shown=gd_explorer_display_layout(c);
    if(!c->display_layout && c->render_pose && render_active(c) && gd_render_pose_matches(c->render_pose,p->layout,render_identity(c)))shown=&c->render_pose->layout;
    result.zoom=p->camera.zoom;
    if(grid_view(p) && p->selection.node<0 && (p->selection.cell>=0 || p->selection.linked<0) && p->grid.noun>=0 && p->grid.verb>=0){
        selected=p->view==GD_EXPLORER_NOUNS ? noun_grid_bounds(p,p->grid.noun,p->grid.verb):grid_bounds(&p->grid,p->grid.noun,p->grid.verb);result.selected=1;
        gd_explorer_focus_camera(&p->camera,c->viewport,selected,selected,(Vector2){selected.x+selected.width/2,selected.y+selected.height/2},&result);goto framed;
    }
    if(p->view==GD_EXPLORER_MDA){
        if(p->mda.link>=0){for(i=0;i<p->layout->edge_count;++i)if(p->layout->edges[i].key==p->mda.link){edge=i;break;}}
        else for(i=0;i<p->layout->node_count;++i)if(p->layout->nodes[i].key==p->mda.record){marked[i]=1;primary=i;break;}
    }else if((count=pair_selection_nodes(c,nodes))>0){
        for(i=0;i<count;++i)marked[nodes[i]]=1;primary=nodes[0];
        for(i=0;i<count;++i)if(p->layout->nodes[nodes[i]].key==p->grid.noun)primary=nodes[i];
        if(!grid_view(p) && p->selection.cell>=0)for(i=0;i<p->layout->edge_count;++i)if(p->layout->edges[i].key==p->selection.cell){edge=i;break;}
    }else if(p->selection.node>=0 && p->selection.node<p->layout->node_count){marked[p->selection.node]=1;primary=p->selection.node;}
    else if(p->selection.cell>=0){for(i=0;i<p->layout->edge_count;++i)if(p->layout->edges[i].key==p->selection.cell){edge=i;break;}}
    else if(p->selection.linked>=0){
        for(i=0;i<p->layout->node_count;++i)if(focus_source_record(c,i)==p->selection.linked){marked[i]=1;if(primary<0)primary=i;}
        /* Matrix case sources are represented by declared cells/edges, not
           case nodes. Reuse exact retained pair membership, never Run/Find. */
        if(primary<0 && p->view==GD_EXPLORER_MATRIX){const GD_Model *model=gd_explorer_model(c->project,p->view);unsigned char records[GD_MAX_RECORDS]={0};
            for(i=0;i<model->cell_count;++i){const GD_Cell *cell=&model->cells[i];int j;for(j=0;j<cell->pair_count;++j)if(model->pairs[cell->first_pair+j].interaction==p->selection.linked)break;if(j==cell->pair_count)continue;
                if(grid_view(p)){Rectangle area=grid_bounds(&p->grid,cell->noun,cell->verb);if(!found)primary_bounds=area;grid_union(&selected,area,&found);++result.selected;}
                else{records[cell->noun]=records[cell->verb]=1;if(!cell->active && !p->overlay)result.hidden_link=true;}
            }
            if(grid_view(p) && found){gd_explorer_focus_camera(&p->camera,c->viewport,selected,selected,(Vector2){primary_bounds.x+primary_bounds.width/2,primary_bounds.y+primary_bounds.height/2},&result);goto framed;}
            for(i=0;i<p->layout->node_count;++i)if(records[p->layout->nodes[i].key]){marked[i]=1;if(primary<0)primary=i;}
        }
    }
    if(edge>=0){const GD_LayoutEdge *e=&p->layout->edges[edge];marked[e->a]=marked[e->b]=1;if(primary<0)primary=e->a;result.hidden_link=!focus_edge_shown(c,edge);}
    for(i=0;i<p->layout->node_count;++i)if(marked[i]){
        if(!visible(c,i)){++result.hidden;marked[i]=0;continue;}
        primary_bounds=focus_node_bounds(c,shown,i);grid_union(&selected,primary_bounds,&found);++result.selected;
    }
    if(!found){result.status=result.hidden ? GD_FOCUS_HIDDEN:GD_FOCUS_UNREPRESENTED;p->focus_result=result;return 1;}
    if(primary<0 || !marked[primary])for(i=0;i<p->layout->node_count;++i)if(marked[i]){primary=i;break;}
    primary_bounds=focus_node_bounds(c,shown,primary);context=selected;
    if(!grid_view(p))for(i=0;i<p->layout->edge_count;++i){
        const GD_LayoutEdge *e=&p->layout->edges[i];if(!(marked[e->a]&1) && !(marked[e->b]&1))continue;
        if(!focus_edge_shown(c,i))continue;
        marked[e->a]|=2;marked[e->b]|=2;
        /* Endpoint footprints plus padding cover MDA's quadratic bend. Its
           native/free renderer scales points, not the card text. */
        if(p->view!=GD_EXPLORER_MDA){GD_LayoutRoute route;int j;gd_layout_route(shown,i,&route);for(j=0;j<route.count;++j){Rectangle point={route.points[j].x,route.points[j].y,0,0};grid_union(&context,point,&found);if(i==edge)grid_union(&selected,point,&found);}}
    }
    for(i=0;i<p->layout->node_count;++i)if(marked[i]==2){grid_union(&context,focus_node_bounds(c,shown,i),&found);++result.context;}
    gd_explorer_focus_camera(&p->camera,c->viewport,selected,context,(Vector2){primary_bounds.x+primary_bounds.width/2,primary_bounds.y+primary_bounds.height/2},&result);
framed:
    result.status=result.hidden || result.hidden_link || result.clipped ? GD_FOCUS_PARTIAL:GD_FOCUS_FRAMED;p->focus_result=result;p->fit=0;p->focus_camera_override=true;return 1;
}
#endif
