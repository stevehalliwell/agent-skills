/* Current shown node/entry/header Find. Viewer-owned text, no query application. */
#ifndef GD_EXPLORER_FIND_H
#define GD_EXPLORER_FIND_H
#include <assert.h>
enum { GD_FIND_TEXT=129,GD_FIND_PREVIEW=6 };
enum { GD_FIND_NONE,GD_FIND_OPEN,GD_FIND_FIT };
typedef struct {char text[GD_FIND_TEXT];bool open,focus;} GD_ExplorerFind;
typedef struct {int nodes[GD_MAX_RECORDS],count,current;} GD_FindMatches;
typedef struct {unsigned char text[GD_FIND_TEXT];int prefix[GD_FIND_TEXT],length;} GD_FindPattern;
static unsigned char find_lower(unsigned char value){return value>='A' && value<='Z' ? (unsigned char)(value+'a'-'A'):value;}
static void find_pattern(GD_FindPattern *pattern,const char *text){
    int i,j=0;memset(pattern,0,sizeof(*pattern));
    for(i=0;text[i] && i<GD_FIND_TEXT-1;++i)pattern->text[i]=find_lower((unsigned char)text[i]);pattern->length=i;
    for(i=1;i<pattern->length;++i){while(j && pattern->text[i]!=pattern->text[j])j=pattern->prefix[j-1];if(pattern->text[i]==pattern->text[j])++j;pattern->prefix[i]=j;}
}
/* Literal ASCII-case-insensitive substring, UTF-8 otherwise exact. Linear scan;
   picker subsequence matching would find unrelated non-contiguous labels. */
static int find_contains(const GD_FindPattern *pattern,const char *text){
    int j=0;if(!text || !pattern->length)return 0;
    while(*text){unsigned char value=find_lower((unsigned char)*text++);while(j && value!=pattern->text[j])j=pattern->prefix[j-1];if(value==pattern->text[j])++j;if(j==pattern->length)return 1;}return 0;
}
static const char *find_label(const GD_ExplorerContext *c,int node){
    if(gd_explorer_similarity_active(c))return c->pane->structure_state->result->similarity->ids[node];
    return query_view(c->pane->view) ? gd_explorer_query_node_label(c,node):c->pane->layout->nodes[node].label;
}
static void find_collect(GD_ExplorerContext *c,const char *text,GD_FindMatches *matches){
    GD_FindPattern pattern;GD_Model *model=gd_explorer_model(c->project,c->pane->view);int i,total=viewer_canvas_count(c),selected=viewer_selected_node(c);
    matches->count=0;matches->current=-1;if(!*text || !viewer_canvas_current(c))return;find_pattern(&pattern,text);
    assert(total>=0 && total<=GD_MAX_RECORDS);
    for(i=0;i<total;++i){int record=viewer_canvas_record(c,i);const GD_Record *source=record>=0 && record<model->record_count ? &model->records[record]:NULL;
        if(!find_contains(&pattern,find_label(c,i)) && !find_contains(&pattern,source ? source->display_name:NULL) && !find_contains(&pattern,source ? source->reference:NULL))continue;
        if(!viewer_canvas_visible(c,i))continue;
        if(i==selected)matches->current=matches->count;matches->nodes[matches->count++]=i;
    }
}
static int find_key(int pressed,int control,int modified,int captured){return !pressed ? GD_FIND_NONE:control ? GD_FIND_OPEN:modified || captured ? GD_FIND_NONE:GD_FIND_FIT;}
static int find_select(GD_ExplorerContext *c,GD_ExplorerPane *panes,int node){
    GD_ExplorerPane *p=c->pane;
    if(gd_explorer_rank_pending(c->project,panes) || !viewer_canvas_current(c) || !viewer_canvas_visible(c,node) || !(c->viewport.width>0) || !(c->viewport.height>0) || !isfinite(c->viewport.width) || !isfinite(c->viewport.height))return 0;
    if(gd_explorer_similarity_active(c)){if(!gd_explorer_similarity_select(c,node))return 0;}
    else if(p->view==GD_EXPLORER_MDA){p->mda.record=p->layout->nodes[node].key;p->mda.link=-1;}
    else{
        /* Unlike IPC effect-result navigation, this must not rebuild retained
           contributor context or geometry. Select only already-shown card. */
        p->selection=(GD_ExplorerSelection){-1,node,0,-1,-1};
        if(is_graph(p->view))p->inspected_record=gd_explorer_graph_record(c);
    }
    if(!gd_explorer_focus_selection(c))return 0;
    if(!gd_explorer_similarity_active(c)){p->find_hold_node=node;p->find_hold_layout=p->layout;}
    return 1;
}
static int find_navigate(GD_ExplorerContext *c,GD_ExplorerPane *panes,GD_FindMatches *matches,int direction){
    int index;if(!matches->count)return 0;
    index=!direction ? 0:matches->current<0 ? (direction>0 ? 0:matches->count-1):(matches->current+matches->count+direction)%matches->count;
    if(!find_select(c,panes,matches->nodes[index]))return 0;matches->current=index;return 1;
}
static void find_controls(GD_ExplorerContext *c,GD_ExplorerPane *panes,GD_ExplorerFind *state){
    GD_FindMatches matches;char previous[GD_FIND_TEXT];int entered,changed,i,active;bool blocked=gd_explorer_rank_pending(c->project,panes)!=0;
    if(!state->open || igGetCurrentWindow()->SkipItems)return;
    igPushID_Str("current-view-find");active=igGetActiveID()==igGetID_Str("Find (shown items)");
    if(igIsKeyPressed_Bool(ImGuiKey_Escape,false) && (active || (!igGetIO_Nil()->WantTextInput && !igIsPopupOpen_Str(NULL,ImGuiPopupFlags_AnyPopupId|ImGuiPopupFlags_AnyPopupLevel)))){
        if(active)igClearActiveID();state->open=state->focus=false;igPopID();return;
    }
    if(blocked)igTextWrapped("Finish or cancel pending rank edit before Find. No selection changed.");
    memcpy(previous,state->text,sizeof(previous));igBeginDisabled(blocked);
    if(state->focus && !blocked){igSetKeyboardFocusHere(0);state->focus=false;}
    entered=igInputText("Find (shown items)",state->text,sizeof(state->text),ImGuiInputTextFlags_EnterReturnsTrue|ImGuiInputTextFlags_AutoSelectAll,0,0);
    if(entered)igSetKeyboardFocusHere(-1);
    changed=strcmp(previous,state->text)!=0;find_collect(c,state->text,&matches);
    if(changed)find_navigate(c,panes,&matches,0);else if(entered)find_navigate(c,panes,&matches,igGetIO_Nil()->KeyShift ? -1:1);
    igTextWrapped("%d matches; names / references. Current shown canvas only.",matches.count);
    igBeginDisabled(!matches.count);
    if(igSmallButton("Previous"))find_navigate(c,panes,&matches,-1);igSameLine(0,-1);
    if(igSmallButton("Next"))find_navigate(c,panes,&matches,1);igEndDisabled();igEndDisabled();igSameLine(0,-1);
    if(igSmallButton("Close Find")){state->open=state->focus=false;if(active)igClearActiveID();}
    igBeginDisabled(blocked);igPushTextWrapPos(0);
    for(i=0;i<matches.count && i<GD_FIND_PREVIEW;++i){int node=matches.nodes[i],record=viewer_canvas_record(c,node);GD_Model *model=gd_explorer_model(c->project,c->pane->view);
        igPushID_Int(i);if(igSmallButton("Select"))find_select(c,panes,node);igSameLine(0,-1);igTextUnformatted(find_label(c,node),NULL);
        if(record>=0 && record<model->record_count)igTextDisabled("%s",model->records[record].reference);igPopID();}
    igPopTextWrapPos();
    if(matches.count>GD_FIND_PREVIEW)igText("First %d listed; Enter / Shift+Enter cycles all.",GD_FIND_PREVIEW);
    if(!gd_explorer_similarity_active(c) && c->pane->distance_focus_enabled && c->pane->find_hold_node>=0)igTextWrapped("Find does not retarget layout focus. Click canvas to resume selection follow.");
    igEndDisabled();igSeparator();igPopID();
}
#endif
