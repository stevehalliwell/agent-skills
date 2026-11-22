#include "gd.h"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_NO_EXPORT
#include "cimgui.h"
#define GD_INSPECTOR_NO_HOOKS
#include "gd_inspector_ipc_ui.h"
#include <math.h>
#include <string.h>

/* Explorer renders one inspector on the ImGui owner thread. Popups and other
   windows render outside the surveyed scope. Never retain borrowed labels. */
static GD_InspectorSurvey *active;
void gd_inspector_survey_begin(GD_InspectorSurvey *survey,int projection) {
    active=survey;
    if(!survey)return;
    survey->projection=projection;survey->count=survey->limited=survey->ready=0;survey->section=-1;
    memset(survey->index,0,sizeof(survey->index));
}
void gd_inspector_survey_end(void) {
    if(active){active->ready=1;active->page_count=0;}
    active=NULL;
}
static int inspector_bucket(int kind,ImGuiID window,ImGuiID widget) {
    uint32_t hash=(uint32_t)kind+(uint32_t)window*UINT32_C(0x9e3779b9)+(uint32_t)widget;
    hash=(hash^(hash>>16))*UINT32_C(0x85ebca6b);
    return (int)((hash^(hash>>13))&(GD_INSPECTOR_INDEX-1));
}
static int inspector_slot(int kind,ImGuiID window,ImGuiID widget) {
    int slot=inspector_bucket(kind,window,widget),n;
    for(n=0;n<GD_INSPECTOR_INDEX;++n,slot=(slot+1)&(GD_INSPECTOR_INDEX-1)){
        int row=active->index[slot]-1;
        if(row<0 || (active->items[row].kind==kind && active->items[row].window==window && active->items[row].widget==widget))return slot;
    }
    return -1;
}
static void inspector_item(int kind,ImGuiID window,ImGuiID widget,const char *label,int depth,int value) {
    int slot=inspector_slot(kind,window,widget),row;GD_InspectorItem *item;size_t n=0;
    if(slot<0){active->limited=1;return;}
    if(active->index[slot]){active->items[active->index[slot]-1].ambiguous=1;return;}
    if(active->count==GD_INSPECTOR_ITEMS){active->limited=1;return;}
    row=active->count++;active->index[slot]=row+1;item=&active->items[row];memset(item,0,sizeof(*item));
    item->kind=kind;item->window=window;item->widget=widget;item->section=active->section;item->depth=depth;item->value=value;
    while(n+1<sizeof(item->label) && label[n] && !(label[n]=='#' && label[n+1]=='#'))++n;
    while(n && !gd_valid_utf8((const unsigned char *)label,n))--n;
    memcpy(item->label,label,n);
}
void gd_inspector_survey_root(int section,const char *label,int open) {
    if(!active)return;
    active->section=section;inspector_item(GD_INSPECTOR_ROOT,0,(ImGuiID)section,label,0,open);
}
int gd_inspector_pending_page(const char *label,int page,int maximum) {
    ImGuiID window,widget;int i;
    if(!active || !active->page_count)return page;
    window=igGetCurrentWindowRead()->ID;widget=igGetID_Str("Page");
    for(i=0;i<active->page_count;++i){
        GD_InspectorPageChange *change=&active->page_changes[i];
        if(!change->seen && change->window==window && change->widget==widget && change->section==active->section &&
           change->maximum==maximum && !strncmp(label,change->label,sizeof(change->label))){
            change->seen=1;return change->value;
        }
    }
    return page;
}
void gd_inspector_record_page(const char *label,int page,int maximum) {
    ImGuiWindow *window;int slot;
    if(!active)return;
    window=igGetCurrentWindowRead();if(window->SkipItems)return;
    inspector_item(GD_INSPECTOR_PAGE,window->ID,igGetID_Str("Page"),label,window->DC.TreeDepth+1,page);
    slot=inspector_slot(GD_INSPECTOR_PAGE,window->ID,igGetID_Str("Page"));
    if(slot>=0 && active->index[slot])active->items[active->index[slot]-1].maximum=maximum;
}
static bool inspector_fold(const char *label,ImGuiTreeNodeFlags flags,int header) {
    ImGuiWindow *window=active ? igGetCurrentWindowRead():NULL;
    int depth=window ? window->DC.TreeDepth:0;
    ImGuiID id=window && !window->SkipItems ? igGetID_Str(label):0;
    bool open=header ? igCollapsingHeader_TreeNodeFlags(label,flags):igTreeNodeEx_Str(label,flags);
    if(id && igGetStateStorage()==&window->StateStorage)inspector_item(GD_INSPECTOR_FOLD,window->ID,id,label,depth+1,open);
    return open;
}
bool gd_inspector_tree(const char *label) { return inspector_fold(label,0,0); }
bool gd_inspector_tree_ex(const char *label,ImGuiTreeNodeFlags flags) { return inspector_fold(label,flags,0); }
bool gd_inspector_header(const char *label,ImGuiTreeNodeFlags flags) { return inspector_fold(label,flags,1); }
bool gd_inspector_child(const char *id,ImVec2_c size,ImGuiChildFlags child_flags,ImGuiWindowFlags flags) {
    int depth=active ? igGetCurrentWindowRead()->DC.TreeDepth:0;
    bool shown=igBeginChild_Str(id,size,child_flags,flags);
    if(active)inspector_item(GD_INSPECTOR_CHILD,igGetCurrentWindowRead()->ID,0,id,depth+1,0);
    return shown;
}
void gd_inspector_end_child(void) {
    if(active){
        int slot=inspector_slot(GD_INSPECTOR_CHILD,igGetCurrentWindowRead()->ID,0),row=slot<0 ? -1:active->index[slot]-1;
        if(row>=0){active->items[row].value=(int)fminf(GD_INSPECTOR_MAX_SCROLL,fmaxf(0,igGetScrollY()));active->items[row].maximum=(int)fminf(GD_INSPECTOR_MAX_SCROLL,fmaxf(0,igGetScrollMaxY()));}
    }
    igEndChild();
}
