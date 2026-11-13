/* Noun-only grid presentation. Included by explorer owner before verb grid. */
static void grid_label(GD_ExplorerContext *c,Rectangle area,const char *label,const GD_Record *record);
static Rectangle noun_grid_bounds(const GD_ExplorerPane *p,int a,int b) {
    return (Rectangle){b<0 ? -GD_MATRIX_NOUN_WIDTH:p->grid.slot[b]*GD_MATRIX_COLUMN_WIDTH,
        a<0 ? -GD_MATRIX_HEADER_HEIGHT:p->grid.slot[a]*GD_MATRIX_ROW_HEIGHT,
        b<0 ? GD_MATRIX_NOUN_WIDTH:GD_MATRIX_COLUMN_WIDTH,a<0 ? GD_MATRIX_HEADER_HEIGHT:GD_MATRIX_ROW_HEIGHT};
}
static int noun_grid_prepare(GD_ExplorerContext *c) {
    GD_ExplorerPane *p=c->pane;const GD_NounGrid *grid=c->project->noun_grid;const GD_Model *model=grid->rules->model;int i,a=p->grid.noun,b=p->grid.verb;
    if(!gd_matrix_grid_prepare(&p->grid,model,GD_SORT_NAME))return 0;
    p->grid.verbs=p->grid.nouns;p->grid.noun=a>=0 && a<model->record_count && model->records[a].role==GD_NOUN ? a:-1;p->grid.verb=b>=0 && b<model->record_count && model->records[b].role==GD_NOUN ? b:-1;
    if(p->layout_choice==GD_NOUN_LAYOUT_FREE)return gd_noun_grid_layout(grid,p->layout);
    memset(p->layout,0,sizeof(*p->layout));
    for(i=0;i<p->grid.nouns;++i){int record=p->grid.order[i];GD_LayoutNode *node=&p->layout->nodes[p->layout->node_count++];node->key=record;node->label=model->records[record].display_name;node->width=GD_MATRIX_NOUN_WIDTH;node->height=GD_MATRIX_ROW_HEIGHT;node->x=-GD_MATRIX_NOUN_WIDTH/2;node->y=i*GD_MATRIX_ROW_HEIGHT+GD_MATRIX_ROW_HEIGHT/2;}
    p->layout->mode=GD_LAYOUT_COLUMNS;p->layout->group_count=p->layout->node_count ? 1:0;
    p->layout->width=GD_MATRIX_NOUN_WIDTH+p->grid.nouns*GD_MATRIX_COLUMN_WIDTH;p->layout->height=GD_MATRIX_HEADER_HEIGHT+p->grid.nouns*GD_MATRIX_ROW_HEIGHT;
    return 1;
}
static void noun_grid_choose(GD_ExplorerContext *c,int a,int b) {
    GD_ExplorerPane *p=c->pane;p->grid.noun=a;p->grid.verb=b;
    p->selection=(GD_ExplorerSelection){gd_noun_grid_pair(c->project->noun_grid,a,b),-1,0,-1,-1};p->noun_case_page=0;
}
static int noun_grid_record(GD_ExplorerContext *c) {
    GD_ExplorerPane *p=c->pane;
    if(p->selection.linked>=0)return p->selection.linked;
    if(p->selection.node>=0 && p->selection.node<p->layout->node_count)return p->layout->nodes[p->selection.node].key;
    if(p->selection.cell>=0 && p->selection.cell<c->project->noun_grid->pair_count){int case_index=c->project->noun_grid->pairs[p->selection.cell].first_case;return c->project->rules->sources[c->project->rules->cases[case_index].source].record;}
    return -1;
}
static void noun_grid_fit(GD_ExplorerContext *c,int matches) {
    GD_ExplorerPane *p=c->pane;Rectangle area={-GD_MATRIX_NOUN_WIDTH,-GD_MATRIX_HEADER_HEIGHT,p->layout->width,p->layout->height};int i;
    if(matches<0){p->camera.zoom=1;p->camera.target=(Vector2){c->viewport.width/2-GD_MATRIX_NOUN_WIDTH,c->viewport.height/2-GD_MATRIX_HEADER_HEIGHT};if(p->selection.node>=0 && p->selection.node<p->layout->node_count)area=noun_grid_bounds(p,p->layout->nodes[p->selection.node].key,-1);else if(p->grid.noun>=0 && p->grid.verb>=0)area=noun_grid_bounds(p,p->grid.noun,p->grid.verb);else return;p->camera.target=(Vector2){area.x+area.width/2,area.y+area.height/2};return;}
    if(matches){for(i=0;i<p->layout->node_count;++i)if(p->match_nodes[i]){area=noun_grid_bounds(p,p->layout->nodes[i].key,-1);break;}if(i==p->layout->node_count)return;}
    p->camera.target=(Vector2){area.x+area.width/2,area.y+area.height/2};p->camera.zoom=fmaxf(0.0001f,fminf(1.2f,fminf(c->viewport.width/(area.width+40),c->viewport.height/(area.height+40))));
}
static void noun_grid_frame(GD_ExplorerContext *c) {
    Rectangle area=noun_grid_bounds(c->pane,c->pane->grid.noun,c->pane->grid.verb);
    c->pane->camera.target=(Vector2){area.x+area.width/2,area.y+area.height/2};
}
static void noun_grid_input(GD_ExplorerContext *c) {
    GD_ExplorerPane *p=c->pane;Vector2 mouse=GetMousePosition(),world;int row,column;
    if(!igGetIO_Nil()->WantCaptureKeyboard && p->grid.nouns){int down=(IsKeyPressed(KEY_DOWN)||IsKeyPressedRepeat(KEY_DOWN))-(IsKeyPressed(KEY_UP)||IsKeyPressedRepeat(KEY_UP));int right=(IsKeyPressed(KEY_RIGHT)||IsKeyPressedRepeat(KEY_RIGHT))-(IsKeyPressed(KEY_LEFT)||IsKeyPressedRepeat(KEY_LEFT));if(down || right){row=p->grid.noun<0 ? 0:p->grid.slot[p->grid.noun]+down;column=p->grid.verb<0 ? 0:p->grid.slot[p->grid.verb]+right;row=row<0 ? 0:row>=p->grid.nouns ? p->grid.nouns-1:row;column=column<0 ? 0:column>=p->grid.nouns ? p->grid.nouns-1:column;noun_grid_choose(c,p->grid.order[row],p->grid.order[column]);noun_grid_frame(c);}}
    if(igGetIO_Nil()->WantCaptureMouse || !CheckCollisionPointRec(mouse,c->viewport))return;
    if(IsMouseButtonDown(MOUSE_BUTTON_RIGHT)||IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)){Vector2 delta=GetMouseDelta();p->camera.target.x-=delta.x/p->camera.zoom;p->camera.target.y-=delta.y/p->camera.zoom;}
    world=GetScreenToWorld2D(mouse,p->camera);
    if(GetMouseWheelMove()){Vector2 after;p->camera.zoom=fminf(4,fmaxf(0.0001f,p->camera.zoom*powf(1.15f,GetMouseWheelMove())));after=GetScreenToWorld2D(mouse,p->camera);p->camera.target.x+=world.x-after.x;p->camera.target.y+=world.y-after.y;}
    if(!IsMouseButtonPressed(MOUSE_BUTTON_LEFT))return;world=GetScreenToWorld2D(mouse,p->camera);
    if(gd_matrix_grid_hit(&p->grid,world.x,world.y,&row,&column)){noun_grid_choose(c,p->grid.order[row],p->grid.order[column]);return;}
    row=world.y>=0 ? (int)(world.y/GD_MATRIX_ROW_HEIGHT):-1;column=world.x>=0 ? (int)(world.x/GD_MATRIX_COLUMN_WIDTH):-1;
    if((world.x>=-GD_MATRIX_NOUN_WIDTH && world.x<0 && row>=0 && row<p->grid.nouns) || (world.y>=-GD_MATRIX_HEADER_HEIGHT && world.y<0 && column>=0 && column<p->grid.nouns)) {int record=p->grid.order[world.x<0 ? row:column],i;for(i=0;i<p->layout->node_count;++i)if(p->layout->nodes[i].key==record){p->selection=(GD_ExplorerSelection){-1,i,0,-1,-1};break;}}
}
static void noun_grid_draw(GD_ExplorerContext *c) {
    GD_ExplorerPane *p=c->pane;const GD_NounGrid *grid=c->project->noun_grid;const GD_Model *model=grid->rules->model;int i,side;GD_CountRange counts={0};
    for(i=0;i<grid->pair_count;++i)gd_count_range_add(&counts,grid->pairs[i].count);
    Vector2 first=GetScreenToWorld2D((Vector2){c->viewport.x,c->viewport.y},p->camera),last=GetScreenToWorld2D((Vector2){c->viewport.x+c->viewport.width,c->viewport.y+c->viewport.height},p->camera);
    GD_MatrixGridRange range=gd_matrix_grid_range(&p->grid,first.x,first.y,last.x,last.y);
    BeginScissorMode((int)c->viewport.x,(int)c->viewport.y,(int)c->viewport.width,(int)c->viewport.height);BeginMode2D(p->camera);
    DrawRectangleRec((Rectangle){0,0,p->grid.nouns*GD_MATRIX_COLUMN_WIDTH,p->grid.nouns*GD_MATRIX_ROW_HEIGHT},(Color){29,38,51,255});
    for(i=0;i<grid->pair_count;++i)for(side=0;side<(grid->pairs[i].a==grid->pairs[i].b ? 1:2);++side){const GD_NounPair *pair=&grid->pairs[i];int a=side ? pair->b:pair->a,b=side ? pair->a:pair->b,row=p->grid.slot[a],column=p->grid.slot[b];Rectangle area;if(row<range.first_row || row>=range.end_row || column<range.first_column || column>=range.end_column)continue;area=noun_grid_bounds(p,a,b);DrawRectangleRec(area,gd_count_color(counts,pair->count));grid_label(c,area,TextFormat("%d cases",pair->count),NULL);}
    if(GD_MATRIX_ROW_HEIGHT*p->camera.zoom>=6)for(i=range.first_row;i<=range.end_row;++i)DrawLineEx((Vector2){0,i*GD_MATRIX_ROW_HEIGHT},(Vector2){p->grid.nouns*GD_MATRIX_COLUMN_WIDTH,i*GD_MATRIX_ROW_HEIGHT},1/p->camera.zoom,(Color){65,78,95,255});
    if(GD_MATRIX_COLUMN_WIDTH*p->camera.zoom>=6)for(i=range.first_column;i<=range.end_column;++i)DrawLineEx((Vector2){i*GD_MATRIX_COLUMN_WIDTH,0},(Vector2){i*GD_MATRIX_COLUMN_WIDTH,p->grid.nouns*GD_MATRIX_ROW_HEIGHT},1/p->camera.zoom,(Color){65,78,95,255});
    for(i=0;i<p->grid.nouns;++i)for(side=0;side<2;++side){int record=p->grid.order[i];Rectangle area=noun_grid_bounds(p,side ? -1:record,side ? record:-1);if(area.x>last.x || area.y>last.y || area.x+area.width<first.x || area.y+area.height<first.y)continue;DrawRectangleRec(area,(Color){39,51,68,255});grid_label(c,area,model->records[record].display_name,&model->records[record]);if((p->selection.node>=0 && p->layout->nodes[p->selection.node].key==record) || p->match_nodes[i])DrawRectangleLinesEx(area,2/p->camera.zoom,p->match_nodes[i] ? gold:(Color){100,178,245,255});}
    if(p->grid.noun>=0 && p->grid.verb>=0 && p->selection.node<0)DrawRectangleLinesEx(noun_grid_bounds(p,p->grid.noun,p->grid.verb),3/p->camera.zoom,(Color){100,178,245,255});
    EndMode2D();EndScissorMode();DrawText(TextFormat("Distinct declared cases | All %d nouns | %s%s | Blank does not prove impossibility",grid->noun_count,grid->incomplete ? "Incomplete scope":"Loaded declarations",grid->limited ? " / limited":""),(int)c->viewport.x+6,(int)c->viewport.y-GD_COUNT_GRID_HEADER+5,14,muted);
    gd_count_legend(counts,c->viewport.x+6,c->viewport.y-GD_COUNT_GRID_HEADER+24,fminf(360,c->viewport.width-12),"Declared cases");
    if(!igGetIO_Nil()->WantCaptureMouse && CheckCollisionPointRec(GetMousePosition(),c->viewport)){int row,column;Vector2 world=GetScreenToWorld2D(GetMousePosition(),p->camera);if(gd_matrix_grid_hit(&p->grid,world.x,world.y,&row,&column)){int a=p->grid.order[row],b=p->grid.order[column],pair=gd_noun_grid_pair(grid,a,b);igSetTooltip("%s x %s\n%d distinct declared cases. Shared participation, not proven applicability.",model->records[a].display_name,model->records[b].display_name,pair<0 ? 0:grid->pairs[pair].count);}}
}
static void noun_graph_input(GD_ExplorerContext *c) {
    GD_ExplorerPane *p=c->pane;const GD_Layout *shown=gd_explorer_display_layout(c);Vector2 mouse=GetMousePosition(),world;int i;
    if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT))p->selection.held=-1;
    if(igGetIO_Nil()->WantCaptureMouse || !CheckCollisionPointRec(mouse,c->viewport))return;
    world=GetScreenToWorld2D(mouse,p->camera);
    if(IsMouseButtonDown(MOUSE_BUTTON_RIGHT)||IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)){Vector2 delta=GetMouseDelta();p->camera.target.x-=delta.x/p->camera.zoom;p->camera.target.y-=delta.y/p->camera.zoom;}
    if(GetMouseWheelMove()){Vector2 after;p->camera.zoom=fminf(4,fmaxf(0.0001f,p->camera.zoom*powf(1.15f,GetMouseWheelMove())));after=GetScreenToWorld2D(mouse,p->camera);p->camera.target.x+=world.x-after.x;p->camera.target.y+=world.y-after.y;}
    world=GetScreenToWorld2D(mouse,p->camera);
    if(p->selection.held>=0 && IsMouseButtonDown(MOUSE_BUTTON_LEFT)){gd_layout_drag_node(p->layout,p->selection.held,world.x,world.y);return;}
    if(!IsMouseButtonPressed(MOUSE_BUTTON_LEFT))return;
    for(i=shown->node_count-1;i>=0;--i){const GD_LayoutNode *node=&shown->nodes[i];if(CheckCollisionPointRec(world,(Rectangle){node->x-node->width/2,node->y-24,node->width,48})){p->selection=(GD_ExplorerSelection){-1,i,0,-1,i};gd_layout_pickup_node(p->layout,i,world.x,world.y);return;}}
    for(i=0;i<p->layout->edge_count;++i)if(gd_layout_route_distance(shown,i,world.x,world.y)<=7/p->camera.zoom){const GD_NounPair *pair=&c->project->noun_grid->pairs[p->layout->edges[i].key];noun_grid_choose(c,pair->a,pair->b);return;}
}
static void noun_graph_draw(GD_ExplorerContext *c) {
    GD_ExplorerPane *p=c->pane;GD_Layout *layout=gd_explorer_display_layout(c);const GD_NounGrid *grid=c->project->noun_grid;GD_CountRange counts={0};int i;
    for(i=0;i<grid->pair_count;++i)gd_count_range_add(&counts,grid->pairs[i].count);
    for(i=0;i<layout->node_count;++i){GD_LayoutNode *node=&layout->nodes[i],*raw=&p->layout->nodes[i];node->width=fmaxf(240,MeasureText(node->label,18)+20);raw->width=node->width;gd_layout_set_footprint(raw,-node->width/2,-24,node->width/2,24);node->footprint=raw->footprint;}
    BeginScissorMode((int)c->viewport.x,(int)c->viewport.y,(int)c->viewport.width,(int)c->viewport.height);BeginMode2D(p->camera);
    for(i=0;i<layout->edge_count;++i){const GD_LayoutEdge *edge=&layout->edges[i];const GD_NounPair *pair=&grid->pairs[edge->key];GD_LayoutRoute route;int j;GD_CountLinkStyle style=gd_count_link_style(counts,pair->count);float thickness=style.width/p->camera.zoom;gd_layout_route(layout,i,&route);
        for(j=1;j<route.count;++j){Vector2 a={route.points[j-1].x,route.points[j-1].y},b={route.points[j].x,route.points[j].y};if(p->selection.cell==edge->key)DrawLineEx(a,b,thickness+4/p->camera.zoom,gold);DrawLineEx(a,b,thickness,style.color);}
        if(route.count>1 && p->camera.zoom*14>=8){int middle=route.count/2;Vector2 point={(route.points[middle-1].x+route.points[middle].x)/2,(route.points[middle-1].y+route.points[middle].y)/2};const char *label=TextFormat("%d",pair->count);float width=MeasureText(label,14)+12;DrawRectangleRec((Rectangle){point.x-width/2,point.y-10,width,20},gd_count_color(counts,pair->count));DrawText(label,(int)(point.x-width/2+6),(int)point.y-7,14,foreground);}
    }
    for(i=0;i<layout->node_count;++i){const GD_LayoutNode *node=&layout->nodes[i];Rectangle area={node->x-node->width/2,node->y-24,node->width,48};DrawRectangleRounded(area,0.14f,4,(Color){29,38,51,255});DrawRectangleRoundedLinesEx(area,0.14f,4,(i==p->selection.node || p->match_nodes[i] ? 3:1)/p->camera.zoom,i==p->selection.node || p->match_nodes[i] ? gold:muted);if(p->camera.zoom*18>=8)gd_noun_draw_text(&grid->rules->model->records[node->key],node->label,(Vector2){area.x+10,node->y-9},18,1,foreground);}
    EndMode2D();EndScissorMode();
    DrawText(TextFormat("All %d nouns | Undirected shared cases, not flow | %s%s",grid->noun_count,grid->incomplete ? "Incomplete scope":"Loaded declarations",grid->limited ? " / limited":""),(int)c->viewport.x+6,(int)c->viewport.y-GD_COUNT_GRID_HEADER+5,14,muted);
    gd_count_link_legend(counts,c->viewport.x+6,c->viewport.y-GD_COUNT_GRID_HEADER+24,fminf(360,c->viewport.width-12),"Declared cases");
}
static void noun_grid_details_pair(GD_ExplorerContext *c,int pair,int inspect_selection) {
    GD_ExplorerPane *p=c->pane;const GD_NounGrid *grid=c->project->noun_grid;const GD_Rules *rules=grid->rules;const GD_Model *model=rules->model;int record=inspect_selection ? noun_grid_record(c):-1,i;
    if(pair>=grid->pair_count)return;
    igTextWrapped("All %d loaded nouns. Cells/links count distinct declared interaction cases, including conditional/excluded declarations, not proven applicability. Scope: %s%s.",grid->noun_count,grid->incomplete ? "incomplete":"loaded declarations",grid->limited ? " / limited":"");
    igTextWrapped("Symmetric shared participation, not direct modification. Self-pairs require two distinct explicit local bindings. No retained pair does not mean impossible interaction.");
    if(inspect_selection && p->grid.noun>=0 && p->grid.verb>=0 && p->selection.node<0){igTextWrapped("%s x %s",model->records[p->grid.noun].display_name,model->records[p->grid.verb].display_name);igText("%d declared cases",pair<0 ? 0:grid->pairs[pair].count);}
    if(pair>=0){const GD_NounPair *item=&grid->pairs[pair];igTextWrapped("%s x %s | %d declared cases",model->records[item->a].display_name,model->records[item->b].display_name,item->count);}
    if(pair>=0){int pages=(rules->case_count+63)/64,start;if(p->noun_case_page>=pages)p->noun_case_page=0;if(pages>1)igSliderInt("Case evidence page",&p->noun_case_page,0,pages-1,"%d",0);start=p->noun_case_page*64;igText("Case slots %d..%d of %d",start,start+64<rules->case_count ? start+64:rules->case_count,rules->case_count);
        for(i=start;i<start+64 && i<rules->case_count;++i)if(gd_noun_grid_case(grid,pair,i)){const GD_RuleCase *item=&rules->cases[i];const GD_Record *source=&model->records[rules->sources[item->source].record];int participant;igPushID_Int(i);if(igTreeNode_Str(TextFormat("%s#%s | line %d",source->reference,item->name,item->line))){igTextWrapped("Context: %s",item->context ? item->context:"unspecified");for(participant=item->first_participant;participant<item->first_participant+item->participant_count;++participant){const GD_RuleParticipant *binding=&rules->participants[participant];igTextWrapped("%s: %s | requires %s | skip_if %s",binding->name,binding->noun_resolution==GD_RULE_RESOLVED ? model->records[binding->noun_record].display_name:"unresolved noun",binding->requires ? binding->requires:"unspecified",binding->skip_if ? binding->skip_if:"unspecified");}if(igSmallButton("Inspect this source"))p->selection.linked=rules->sources[item->source].record;igTreePop();}igPopID();}}
    if(record>=0){const GD_Record *source=&model->records[record];gd_noun_ui_text(source,source->display_name);igTextWrapped("%s | %s",source->reference,source->source_path);if(igSmallButton("Copy source path"))SetClipboardText(source->source_path);if(igTreeNode_Str("Full authored source")){igPushTextWrapPos(0);igTextUnformatted(source->body,NULL);igPopTextWrapPos();igTreePop();}}
}
static void noun_grid_details(GD_ExplorerContext *c) { noun_grid_details_pair(c,c->pane->selection.cell,1); }
