/* Unified Interactions canvas. Included by gd_explorer_view.c only. */
static int grid_view(const GD_ExplorerPane *p) { return p->view==GD_EXPLORER_MATRIX && p->layout_choice==GD_MATRIX_GRID_CHOICE; }
static Rectangle grid_bounds(const GD_MatrixGrid *grid, int noun, int verb) {
    return (Rectangle){verb<0 ? -GD_MATRIX_NOUN_WIDTH:grid->slot[verb]*GD_MATRIX_COLUMN_WIDTH,
        noun<0 ? -GD_MATRIX_HEADER_HEIGHT:grid->slot[noun]*GD_MATRIX_ROW_HEIGHT,
        verb<0 ? GD_MATRIX_NOUN_WIDTH:GD_MATRIX_COLUMN_WIDTH,noun<0 ? GD_MATRIX_HEADER_HEIGHT:GD_MATRIX_ROW_HEIGHT};
}
static void grid_union(Rectangle *bounds, Rectangle area, int *found) {
    if (!*found) *bounds=area;
    else {
        float right=fmaxf(bounds->x+bounds->width,area.x+area.width),bottom=fmaxf(bounds->y+bounds->height,area.y+area.height);
        bounds->x=fminf(bounds->x,area.x); bounds->y=fminf(bounds->y,area.y);
        bounds->width=right-bounds->x; bounds->height=bottom-bounds->y;
    }
    *found=1;
}
static void grid_fit(GD_ExplorerContext *c, int only_matches) {
    GD_ExplorerPane *p=c->pane; const GD_Model *model=gd_explorer_model(c->project,p->view);
    Rectangle bounds={-GD_MATRIX_NOUN_WIDTH,-GD_MATRIX_HEADER_HEIGHT,
        p->grid.verbs*GD_MATRIX_COLUMN_WIDTH+GD_MATRIX_NOUN_WIDTH,p->grid.nouns*GD_MATRIX_ROW_HEIGHT+GD_MATRIX_HEADER_HEIGHT};
    int i,found=0,record=gd_explorer_record(model,p->find_reference);
    if (only_matches<0) {
        /* Start readable, not fitted to a potentially huge Cartesian extent.
           F still fits the full grid; camera never changes source scope. */
        p->camera.zoom=1;
        p->camera.target=(Vector2){c->viewport.width/2-GD_MATRIX_NOUN_WIDTH,c->viewport.height/2-GD_MATRIX_HEADER_HEIGHT};
        if (p->selection.cell>=0) {
            const GD_Cell *cell=&model->cells[p->selection.cell]; Rectangle area=grid_bounds(&p->grid,cell->noun,cell->verb);
            p->camera.target=(Vector2){area.x+area.width/2,area.y+area.height/2};
        }
        return;
    }
    if (only_matches) {
        for (i=0;i<p->layout->node_count;++i) if (p->match_nodes[i] && p->layout->nodes[i].key==record)
            grid_union(&bounds,grid_bounds(&p->grid,model->records[record].role==GD_NOUN ? record:-1,
                model->records[record].role==GD_VERB ? record:-1),&found);
        for (i=0;i<p->layout->edge_count;++i) if (p->match_edges[i]) {
            const GD_Cell *cell=&model->cells[p->layout->edges[i].key];
            grid_union(&bounds,grid_bounds(&p->grid,cell->noun,cell->verb),&found);
        }
        if (!found) return;
    }
    p->camera.target=(Vector2){bounds.x+bounds.width/2,bounds.y+bounds.height/2};
    p->camera.zoom=fminf(1.2f,fminf(c->viewport.width/(bounds.width+40),c->viewport.height/(bounds.height+40)));
    p->camera.zoom=fmaxf(0.0001f,p->camera.zoom);
}
static void grid_choose(GD_ExplorerContext *c, int row, int column) {
    GD_ExplorerPane *p=c->pane; GD_Model *model=gd_explorer_model(c->project,p->view);
    if (row<0 || row>=p->grid.nouns || column<0 || column>=p->grid.verbs) return;
    p->grid.noun=p->grid.order[row]; p->grid.verb=p->grid.order[p->grid.nouns+column];
    {
        GD_ExplorerSelection choice={gd_matrix_grid_cell(model,p->grid.noun,p->grid.verb),-1,0,-1,-1};
        gd_explorer_graph_select(c,&choice);
    }
}
static void grid_frame_selection(GD_ExplorerContext *c) {
    GD_ExplorerPane *p=c->pane; Rectangle area=grid_bounds(&p->grid,p->grid.noun,p->grid.verb);
    Vector2 first=GetWorldToScreen2D((Vector2){area.x,area.y},p->camera),last=GetWorldToScreen2D((Vector2){area.x+area.width,area.y+area.height},p->camera);
    float dx=first.x<c->viewport.x+8 ? first.x-c->viewport.x-8:last.x>c->viewport.x+c->viewport.width-8 ? last.x-c->viewport.x-c->viewport.width+8:0;
    float dy=first.y<c->viewport.y+8 ? first.y-c->viewport.y-8:last.y>c->viewport.y+c->viewport.height-8 ? last.y-c->viewport.y-c->viewport.height+8:0;
    p->camera.target.x+=dx/p->camera.zoom; p->camera.target.y+=dy/p->camera.zoom;
}
static int grid_key(int key) { return IsKeyPressed(key) || IsKeyPressedRepeat(key); }
static void grid_input(GD_ExplorerContext *c) {
    GD_ExplorerPane *p=c->pane; const GD_Model *model=gd_explorer_model(c->project,p->view);
    Vector2 mouse=GetMousePosition(),world; int row,column,i;
    if (p->selection.cell>=0) {
        p->grid.noun=model->cells[p->selection.cell].noun; p->grid.verb=model->cells[p->selection.cell].verb;
    }
    if (!igGetIO_Nil()->WantCaptureKeyboard) {
        int rows=grid_key(KEY_DOWN)-grid_key(KEY_UP),columns=grid_key(KEY_RIGHT)-grid_key(KEY_LEFT);
        if ((rows || columns) && gd_matrix_grid_move(&p->grid,rows,columns)) {
            grid_choose(c,p->grid.slot[p->grid.noun],p->grid.slot[p->grid.verb]); grid_frame_selection(c);
        }
        if (p->selection.cell>=0) {
            int cases=model->cells[p->selection.cell].pair_count,delta=grid_key(KEY_PAGE_DOWN)-grid_key(KEY_PAGE_UP);
            if (delta && cases>0) { p->selection.interaction=(p->selection.interaction+cases+delta)%cases; p->selection.linked=-1; }
        }
    }
    if (igGetIO_Nil()->WantCaptureMouse || !CheckCollisionPointRec(mouse,c->viewport)) return;
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) || IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
        Vector2 delta=GetMouseDelta(); p->camera.target.x-=delta.x/p->camera.zoom; p->camera.target.y-=delta.y/p->camera.zoom;
    }
    world=GetScreenToWorld2D(mouse,p->camera);
    if (GetMouseWheelMove()) {
        Vector2 after;
        p->camera.zoom=fminf(4,fmaxf(0.0001f,p->camera.zoom*powf(1.15f,GetMouseWheelMove())));
        after=GetScreenToWorld2D(mouse,p->camera); p->camera.target.x+=world.x-after.x; p->camera.target.y+=world.y-after.y;
    }
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return;
    world=GetScreenToWorld2D(mouse,p->camera);
    if (gd_matrix_grid_hit(&p->grid,world.x,world.y,&row,&column)) { grid_choose(c,row,column); return; }
    for (i=0;i<p->layout->node_count;++i) {
        int record=p->layout->nodes[i].key;
        Rectangle area=grid_bounds(&p->grid,model->records[record].role==GD_NOUN ? record:-1,model->records[record].role==GD_VERB ? record:-1);
        if (CheckCollisionPointRec(world,area)) {
            GD_ExplorerSelection choice={-1,i,0,-1,-1}; gd_explorer_graph_select(c,&choice); return;
        }
    }
}
static void grid_label(GD_ExplorerContext *c, Rectangle area, const char *label) {
    Vector2 first=GetWorldToScreen2D((Vector2){area.x+6,area.y+4},c->pane->camera);
    Vector2 last=GetWorldToScreen2D((Vector2){area.x+area.width-6,area.y+area.height-4},c->pane->camera);
    Rectangle clip=GetCollisionRec(c->viewport,(Rectangle){first.x,first.y,last.x-first.x,last.y-first.y});
    if (clip.width<1 || clip.height<1 || c->pane->camera.zoom*18<8) return;
    BeginScissorMode((int)ceilf(clip.x),(int)ceilf(clip.y),(int)floorf(clip.width),(int)floorf(clip.height));
    DrawTextEx(GetFontDefault(),label,(Vector2){area.x+8,area.y+(area.height-18)/2},18,1,foreground);
    EndScissorMode(); BeginScissorMode((int)c->viewport.x,(int)c->viewport.y,(int)c->viewport.width,(int)c->viewport.height);
}
static void grid_draw(GD_ExplorerContext *c) {
    GD_ExplorerPane *p=c->pane; const GD_Model *model=gd_explorer_model(c->project,p->view);
    Vector2 first=GetScreenToWorld2D((Vector2){c->viewport.x,c->viewport.y},p->camera);
    Vector2 last=GetScreenToWorld2D((Vector2){c->viewport.x+c->viewport.width,c->viewport.y+c->viewport.height},p->camera);
    GD_MatrixGridRange range=gd_matrix_grid_range(&p->grid,first.x,first.y,last.x,last.y);
    int i; float thickness=2/p->camera.zoom;
    BeginScissorMode((int)c->viewport.x,(int)c->viewport.y,(int)c->viewport.width,(int)c->viewport.height); BeginMode2D(p->camera);
    DrawRectangleRec((Rectangle){0,0,p->grid.verbs*GD_MATRIX_COLUMN_WIDTH,p->grid.nouns*GD_MATRIX_ROW_HEIGHT},(Color){29,38,51,255});
    /* Sparse cells only. Zoom-out never allocates or scans the Cartesian product.
       Subpixel cells merge pixels; they are not density or semantic clusters. */
    for (i=0;i<model->cell_count;++i) {
        const GD_Cell *cell=&model->cells[i]; int row=p->grid.slot[cell->noun],column=p->grid.slot[cell->verb]; Rectangle area;
        if (row<range.first_row || row>=range.end_row || column<range.first_column || column>=range.end_column) continue;
        area=grid_bounds(&p->grid,cell->noun,cell->verb);
        DrawRectangleRec((Rectangle){area.x,area.y,fmaxf(area.width,1/p->camera.zoom),fmaxf(area.height,1/p->camera.zoom)},cell->active ? (Color){36,85,85,255}:(Color){61,66,79,255});
        grid_label(c,area,TextFormat("%d cases%s",cell->pair_count,cell->active ? " *":""));
    }
    if (GD_MATRIX_ROW_HEIGHT*p->camera.zoom>=6) for (i=range.first_row;i<=range.end_row;++i)
        DrawLineEx((Vector2){0,i*GD_MATRIX_ROW_HEIGHT},(Vector2){p->grid.verbs*GD_MATRIX_COLUMN_WIDTH,i*GD_MATRIX_ROW_HEIGHT},1/p->camera.zoom,(Color){65,78,95,255});
    if (GD_MATRIX_COLUMN_WIDTH*p->camera.zoom>=6) for (i=range.first_column;i<=range.end_column;++i)
        DrawLineEx((Vector2){i*GD_MATRIX_COLUMN_WIDTH,0},(Vector2){i*GD_MATRIX_COLUMN_WIDTH,p->grid.nouns*GD_MATRIX_ROW_HEIGHT},1/p->camera.zoom,(Color){65,78,95,255});
    for (i=0;i<p->layout->node_count;++i) {
        int record=p->layout->nodes[i].key; Rectangle area=grid_bounds(&p->grid,model->records[record].role==GD_NOUN ? record:-1,model->records[record].role==GD_VERB ? record:-1);
        if (area.x>last.x || area.y>last.y || area.x+area.width<first.x || area.y+area.height<first.y) continue;
        DrawRectangleRec(area,(Color){39,51,68,255}); grid_label(c,area,model->records[record].name);
        if (p->selection.node==i || p->match_nodes[i]) DrawRectangleLinesEx(area,thickness,p->match_nodes[i] ? gold:(Color){100,178,245,255});
    }
    for (i=0;i<p->layout->edge_count;++i) if (p->match_edges[i]) {
        const GD_Cell *cell=&model->cells[p->layout->edges[i].key]; DrawRectangleLinesEx(grid_bounds(&p->grid,cell->noun,cell->verb),thickness,gold);
    }
    if (p->selection.node<0 && p->selection.linked<0 && p->grid.noun>=0 && p->grid.verb>=0)
        DrawRectangleLinesEx(grid_bounds(&p->grid,p->grid.noun,p->grid.verb),3/p->camera.zoom,(Color){100,178,245,255});
    EndMode2D(); EndScissorMode();
    DrawText("* accepted + defined case | Other cases visible | Blank is not a defect",(int)c->viewport.x+6,(int)c->viewport.y+5,14,muted);
    if (!p->grid.nouns || !p->grid.verbs) DrawText("No cell pairs: one or both loaded axes are empty.",(int)c->viewport.x+6,(int)c->viewport.y+24,14,gold);
    else if (p->camera.zoom*18<8) DrawText("Zoom in for labels. Subpixel cells merge pixels; not density counts.",(int)c->viewport.x+6,(int)c->viewport.y+24,14,gold);
    if (!igGetIO_Nil()->WantCaptureMouse && CheckCollisionPointRec(GetMousePosition(),c->viewport)) {
        int row,column; Vector2 world=GetScreenToWorld2D(GetMousePosition(),p->camera);
        if (gd_matrix_grid_hit(&p->grid,world.x,world.y,&row,&column)) {
            int noun=p->grid.order[row],verb=p->grid.order[p->grid.nouns+column],cell=gd_matrix_grid_cell(model,noun,verb);
            igSetTooltip("%s x %s\n%d declared cases. No rule inferred from blank cells.",model->records[noun].name,model->records[verb].name,cell<0 ? 0:model->cells[cell].pair_count);
        }
    }
}
static int grid_blank_details(GD_ExplorerContext *c) {
    GD_ExplorerPane *p=c->pane; const GD_Model *model=gd_explorer_model(c->project,p->view);
    if (p->selection.cell>=0 || p->selection.node>=0 || p->selection.linked>=0 || p->grid.noun<0 || p->grid.verb<0) return 0;
    igTextWrapped("%s x %s",model->records[p->grid.noun].name,model->records[p->grid.verb].name);
    igTextWrapped("No declared case in this cell. No rule, exclusion or defect inferred."); return 1;
}
