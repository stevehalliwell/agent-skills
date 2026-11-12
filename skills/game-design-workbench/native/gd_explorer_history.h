/* Shell-owned, bounded typed-selection navigation. No retained query ownership. */
#ifndef GD_EXPLORER_HISTORY_H
#define GD_EXPLORER_HISTORY_H
enum { GD_SELECTION_HISTORY=128 };
enum { GD_HISTORY_NODE,GD_HISTORY_PAIR,GD_HISTORY_SOURCE,GD_HISTORY_EDGE,GD_HISTORY_EMPTY };
typedef struct {
    int view,kind,relation,similarity;
    char id[GD_MAX_PATH],peer[GD_MAX_PATH],detail[GD_MAX_PATH],linked[GD_MAX_PATH],node_kind[32];
} GD_HistoryEntry;
typedef struct GD_SelectionHistory {
    GD_HistoryEntry entries[GD_SELECTION_HISTORY];int count,cursor;uint64_t stamp;int stamped;
    char message[256];
} GD_SelectionHistory;
static uint64_t history_stamp(const GD_ExplorerContext *c){
    const GD_ExplorerPane *p=c->pane;GD_ExplorerSelection selection=p->selection;uint64_t hash=UINT64_C(14695981039346656037);int sim=gd_explorer_similarity_active(c),row=sim ? p->structure_state->similarity_row:-1;
    selection.held=-1;
#define HISTORY_HASH(value) hash=viewer_control_hash(hash,&(value),sizeof(value))
    HISTORY_HASH(c->project);HISTORY_HASH(c->project->revision);HISTORY_HASH(p->view);HISTORY_HASH(p->layout);HISTORY_HASH(selection);HISTORY_HASH(p->mda.record);HISTORY_HASH(p->mda.link);HISTORY_HASH(p->grid.noun);HISTORY_HASH(p->grid.verb);HISTORY_HASH(sim);HISTORY_HASH(row);
    if(p->query_state){HISTORY_HASH(p->query_state->query);HISTORY_HASH(p->query_state->scene);}
    if(p->structure_state){HISTORY_HASH(p->structure_state->result);HISTORY_HASH(p->structure_state->similarity_peer);}
#undef HISTORY_HASH
    return hash;
}
static int history_text(char *out,const char *text){size_t length=strlen(text);if(length>=GD_MAX_PATH)return 0;memcpy(out,text,length+1);return 1;}
static int history_capture(GD_ExplorerContext *c,GD_HistoryEntry *entry){
    GD_ExplorerPane *p=c->pane;GD_Model *model=gd_explorer_model(c->project,p->view);ViewerIdentities ids;int node=viewer_selected_node(c),a=-1,b=-1,edge=-1,i;
    memset(entry,0,sizeof(*entry));entry->view=p->view;entry->similarity=gd_explorer_similarity_active(c);viewer_identities(c,&ids);
    if(p->selection.linked>=0 && p->selection.linked<model->record_count && !history_text(entry->linked,model->records[p->selection.linked].reference))return 0;
    if(node>=0){
        entry->kind=GD_HISTORY_NODE;if(!viewer_canvas_id(c,node,&ids,entry->id,sizeof(entry->id)))return 0;
        snprintf(entry->node_kind,sizeof(entry->node_kind),"%s",viewer_canvas_kind(c,node));
        if(entry->similarity && p->structure_state->similarity_peer>=0 && !viewer_canvas_id(c,p->structure_state->similarity_peer,&ids,entry->peer,sizeof(entry->peer)))return 0;
        return 1;
    }
    if(p->view==GD_EXPLORER_MATRIX || p->view==GD_EXPLORER_NOUNS){
        a=p->grid.noun;b=p->grid.verb;
        if(p->view==GD_EXPLORER_MATRIX && p->selection.cell>=0 && p->selection.cell<model->cell_count){
            const GD_Cell *cell=&model->cells[p->selection.cell];a=cell->noun;b=cell->verb;
            if(p->selection.interaction>=0 && p->selection.interaction<cell->pair_count && !history_text(entry->detail,model->records[model->pairs[cell->first_pair+p->selection.interaction].interaction].reference))return 0;
        }
        if(a>=0 && b>=0 && a<model->record_count && b<model->record_count){entry->kind=GD_HISTORY_PAIR;return history_text(entry->id,model->records[a].reference) && history_text(entry->peer,model->records[b].reference);}
    }
    if(p->view==GD_EXPLORER_MDA){for(i=0;i<p->layout->edge_count;++i)if(p->layout->edges[i].key==p->mda.link && p->mda.link>=0)edge=i;}
    else if(p->selection.cell>=0){for(i=0;i<p->layout->edge_count;++i)if(p->layout->edges[i].key==p->selection.cell)edge=i;}
    if(edge>=0){const GD_LayoutEdge *link=&p->layout->edges[edge];entry->kind=GD_HISTORY_EDGE;entry->relation=link->relation;if(p->view==GD_EXPLORER_MDA && p->mda.record>=0 && p->mda.record<model->record_count && !history_text(entry->linked,model->records[p->mda.record].reference))return 0;return viewer_canvas_id(c,link->a,&ids,entry->id,sizeof(entry->id)) && viewer_canvas_id(c,link->b,&ids,entry->peer,sizeof(entry->peer));}
    i=selected_record(c);if(i<0 || i>=model->record_count){entry->kind=GD_HISTORY_EMPTY;return 1;}
    entry->kind=GD_HISTORY_SOURCE;return history_text(entry->id,model->records[i].reference);
}
static void history_observe(GD_ExplorerContext *c){
    GD_SelectionHistory *history=c->history;GD_HistoryEntry entry;uint64_t stamp;
    if(!history || !viewer_canvas_current(c))return;stamp=history_stamp(c);if(history->stamped && stamp==history->stamp)return;
    history->stamp=stamp;history->stamped=1;if(!history_capture(c,&entry)){snprintf(history->message,sizeof(history->message),"Cannot record exact selection within native identity capacity/work budget; no approximate entry stored.");return;}
    if(history->count && !memcmp(&history->entries[history->cursor],&entry,sizeof(entry)))return;
    if(history->count)history->count=history->cursor+1;
    if(history->count==GD_SELECTION_HISTORY){memmove(history->entries,history->entries+1,(GD_SELECTION_HISTORY-1)*sizeof(entry));--history->count;}
    history->entries[history->count++]=entry;history->cursor=history->count-1;*history->message=0;
}
int gd_explorer_history_available(const GD_ExplorerContext *c,int direction){
    return c && c->history && c->panes && c->active_view && c->pane->view==*c->active_view && c->pane->selection.held<0 && c->pane->mda.held<0 && !gd_explorer_rank_pending(c->project,c->panes) && c->history->count>0 && (direction<0 ? c->history->cursor>0:c->history->cursor+1<c->history->count);
}
/* Resolve even hidden nodes; unlike viewer_find_node, history may retain hidden
   selection. Identity ambiguity/absence never falls back to a source record. */
static int history_node(GD_ExplorerContext *c,const char *id){
    ViewerIdentities ids;char candidate[GD_MAX_PATH];int i,found=-1;viewer_identities(c,&ids);
    for(i=0;i<viewer_canvas_count(c);++i)if(viewer_canvas_id(c,i,&ids,candidate,sizeof(candidate)) && !strcmp(id,candidate)){if(found>=0)return -1;found=i;}
    return ids.limited ? -1:found;
}
int gd_explorer_history_move(GD_ExplorerContext *c,int direction){
    GD_SelectionHistory *history=c->history;GD_ExplorerContext target=*c;GD_HistoryEntry *entry;GD_ExplorerPane *p;GD_Model *model;int cursor,node=-1,peer=-1,linked=-1,a=-1,b=-1,cell=-1,interaction=0,edge=-1,i;
    if(!history || !c->panes || !c->active_view || c->pane->view!=*c->active_view)return 0;history_observe(c);
    if(!gd_explorer_history_available(c,direction))return 0;cursor=history->cursor+(direction<0 ? -1:1);entry=&history->entries[cursor];
    target.pane=p=&c->panes[entry->view];target.display_layout=NULL;
    if(!(c->project->available&(1u<<entry->view)) || !viewer_canvas_current(&target) || gd_explorer_similarity_active(&target)!=entry->similarity)goto unavailable;
    model=gd_explorer_model(c->project,p->view);
    if(*entry->linked){linked=gd_explorer_record(model,entry->linked);if(linked<0)goto unavailable;}
    if(entry->kind==GD_HISTORY_NODE){node=history_node(&target,entry->id);if(node<0 || strcmp(viewer_canvas_kind(&target,node),entry->node_kind))goto unavailable;if(entry->similarity && *entry->peer){peer=history_node(&target,entry->peer);if(peer<0)goto unavailable;}}
    else if(entry->kind==GD_HISTORY_SOURCE){linked=gd_explorer_record(model,entry->id);if(linked<0)goto unavailable;}
    else if(entry->kind==GD_HISTORY_PAIR){
        a=gd_explorer_record(model,entry->id);b=gd_explorer_record(model,entry->peer);if(a<0 || b<0)goto unavailable;
        if(p->view==GD_EXPLORER_NOUNS)cell=gd_noun_grid_pair(c->project->noun_grid,a,b);
        else{for(i=0;i<model->cell_count;++i)if(model->cells[i].noun==a && model->cells[i].verb==b){cell=i;break;}
            if(*entry->detail){int found=0;if(cell<0)goto unavailable;for(i=0;i<model->cells[cell].pair_count;++i)if(!strcmp(model->records[model->pairs[model->cells[cell].first_pair+i].interaction].reference,entry->detail)){interaction=i;++found;}if(found!=1)goto unavailable;}}
    }else if(entry->kind==GD_HISTORY_EDGE){
        a=history_node(&target,entry->id);b=history_node(&target,entry->peer);if(a<0 || b<0)goto unavailable;
        for(i=0;i<p->layout->edge_count;++i)if(p->layout->edges[i].a==a && p->layout->edges[i].b==b && p->layout->edges[i].relation==entry->relation){if(edge>=0)goto unavailable;edge=i;}if(edge<0)goto unavailable;
    }
    if(entry->similarity){p->structure_state->similarity_row=node;p->structure_state->similarity_peer=peer;}
    else if(p->view==GD_EXPLORER_MDA){p->mda.record=node>=0 ? p->layout->nodes[node].key:linked;p->mda.link=edge>=0 ? p->layout->edges[edge].key:-1;}
    else{p->selection=(GD_ExplorerSelection){edge>=0 ? p->layout->edges[edge].key:cell,node,interaction,linked,-1};if(entry->kind==GD_HISTORY_PAIR){p->grid.noun=a;p->grid.verb=b;}else if(entry->kind==GD_HISTORY_EMPTY){p->grid.noun=p->grid.verb=-1;}}
    if(entry->kind==GD_HISTORY_EMPTY)p->focus_result=(GD_ExplorerFocusResult){0};
    if(is_graph(p->view))p->inspected_record=selected_record(&target);
    c->pane=p;c->display_layout=NULL;*c->active_view=p->view;history->cursor=cursor;history->stamp=history_stamp(c);history->stamped=1;
    gd_explorer_focus_selection(c);
    if(node>=0 && !entry->similarity){p->find_hold_node=node;p->find_hold_layout=p->layout;}
    snprintf(history->message,sizeof(history->message),"%s selection in %s%s",direction<0 ? "Previous":"Next",gd_explorer_view_names[p->view],p->focus_result.status==GD_FOCUS_HIDDEN ? " (hidden by current display)":"");return 1;
unavailable:
    snprintf(history->message,sizeof(history->message),"History target unavailable in current %s canvas; no substitution or Run.",gd_explorer_view_names[entry->view]);snprintf(c->pane->query_message,sizeof(c->pane->query_message),"%s",history->message);return 0;
}
#endif
