#ifndef GD_EXPLORER_PICKER_H
#define GD_EXPLORER_PICKER_H
#include "gd_picker.h"
#include "gd_evidence_page.h"
#include "gd_noun_label_ui.h"
/* Page state belongs to caller's exact list; page changes inspect no row. */
static GD_EvidencePage gd_explorer_evidence_pager(const char *label,int *page,int total,int size) {
    GD_EvidencePage range=gd_evidence_page(total,size,*page);int human;
    *page=range.page;igPushID_Str(label);
    igBeginDisabled(!range.page);if(igSmallButton("Previous"))--*page;igEndDisabled();igSameLine(0,-1);
    igBeginDisabled(!range.pages || range.page+1>=range.pages);if(igSmallButton("Next"))++*page;igEndDisabled();
    range=gd_evidence_page(total,size,*page);human=range.page+1;
    if(range.pages>1 && igSliderInt("Page",&human,1,range.pages,"%d",0))range=gd_evidence_page(total,size,human-1);
    *page=range.page;
    igText("%s %d-%d of %d retained",label,total ? range.first+1:0,range.end,total);
    if(range.pages)igText("Page %d of %d",range.page+1,range.pages);else igTextUnformatted("No retained rows in this scope",NULL);
    igPopID();return range;
}
/* Include after cimgui declarations. One popup at a time per independent pane.
   Search changes presentation only; callers adopt exact native choices. */
static int gd_explorer_picker_begin(GD_ExplorerContext *context,const char *label,const char *preview) {
    igPushID_Str(label); igTextUnformatted(label,NULL); igSetNextItemWidth(-1);
    if (!igBeginCombo("##choice",preview && *preview ? preview:"Choose...",ImGuiComboFlags_HeightLarge)) { igPopID(); return 0; }
    if (igIsWindowAppearing()) { *context->pane->picker_search=0; igSetKeyboardFocusHere(0); }
    igSetNextItemWidth(-1); igInputTextWithHint("##search","Fuzzy search...",context->pane->picker_search,sizeof(context->pane->picker_search),0,NULL,NULL); igSeparator(); return 1;
}
static int gd_explorer_picker_match(const GD_ExplorerContext *context,const char *label) { return gd_picker_match(label,context->pane->picker_search); }
static void gd_explorer_picker_end(void) { igEndCombo(); igPopID(); }
static int gd_explorer_picker_item(const GD_ExplorerContext *context,const char *label,int selected,int keep_open) {
    if (!gd_explorer_picker_match(context,label)) return 0;
    return igSelectable_Bool(label,selected,keep_open ? ImGuiSelectableFlags_NoAutoClosePopups:0,(ImVec2_c){0,0});
}
static int gd_explorer_picker_record_item(const GD_ExplorerContext *context,const GD_Record *record,const char *label,int selected){
    ImVec2_c position;int clicked;
    if(!gd_explorer_picker_match(context,label))return 0;
    position=igGetCursorScreenPos();clicked=igSelectable_Bool(label,selected,0,(ImVec2_c){0,0});gd_noun_ui_decorate(record,label,position);return clicked;
}
static void gd_explorer_picker_statuses(GD_ExplorerContext *context,const GD_Model *model,char *text,size_t capacity) {
    const char *statuses[GD_MAX_RECORDS]; int count=0,i,j;
    for (i=0;i<model->record_count;++i) if (model->records[i].status && *model->records[i].status) { const char *status=model->records[i].status; for (j=0;j<count && strcmp(statuses[j],status);++j) {} if (j==count && count<GD_MAX_RECORDS) statuses[count++]=status; }
    if (!gd_explorer_picker_begin(context,"Source statuses",*text ? "Explicit selected statuses":"All statuses")) return;
    if (gd_explorer_picker_item(context,"All statuses",!*text,1)) *text=0;
    for (i=0;i<count;++i) if (gd_explorer_picker_item(context,statuses[i],gd_picker_contains(text,statuses[i]),1) && !gd_picker_toggle_limit(text,capacity,statuses[i],GD_STRUCTURE_SELECTORS)) snprintf(context->pane->query_message,sizeof(context->pane->query_message),"Status selection exceeds native item/text cap; previous inputs retained.");
    gd_explorer_picker_end();
}
#endif
