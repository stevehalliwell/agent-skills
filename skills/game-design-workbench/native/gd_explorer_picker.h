#ifndef GD_EXPLORER_PICKER_H
#define GD_EXPLORER_PICKER_H
#include "gd_picker.h"
#include "gd_evidence_page_ui.h"
#include "gd_noun_label_ui.h"
/* Compact snapshot-local cursors; all native retained-list bounds fit 16 bits of pages. */
_Static_assert(GD_MAX_PAIRS/GD_EVIDENCE_PAGE_ROWS<65536,"Evidence page cursor capacity");
static GD_EvidencePage gd_explorer_evidence_cursor_pager(const char *label,unsigned short *cursor,int total) {
    int page=*cursor;GD_EvidencePage range;igPushID_Ptr(cursor);
    range=gd_explorer_evidence_pager(label,&page,total,GD_EVIDENCE_PAGE_ROWS);igPopID();
    assert(page>=0 && page<=65535);*cursor=(unsigned short)page;return range;
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
