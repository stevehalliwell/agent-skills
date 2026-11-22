#ifndef GD_EVIDENCE_PAGE_UI_H
#define GD_EVIDENCE_PAGE_UI_H
#include "gd_evidence_page.h"
#include "gd_inspector_ipc_ui.h"
/* Include after cimgui. Caller owns exact-list cursor; paging never inspects a row. */
static GD_EvidencePage gd_explorer_evidence_pager(const char *label,int *page,int total,int size) {
    GD_EvidencePage range=gd_evidence_page(total,size,*page);int human;
    igPushID_Str(label);
    if(range.pages>1)range=gd_evidence_page(total,size,gd_inspector_pending_page(label,range.page,range.pages-1));
    *page=range.page;
    igBeginDisabled(!range.page);if(igSmallButton("Previous"))--*page;igEndDisabled();igSameLine(0,-1);
    igBeginDisabled(!range.pages || range.page+1>=range.pages);if(igSmallButton("Next"))++*page;igEndDisabled();
    range=gd_evidence_page(total,size,*page);human=range.page+1;
    if(range.pages>1 && igSliderInt("Page",&human,1,range.pages,"%d",0))range=gd_evidence_page(total,size,human-1);
    *page=range.page;
    igText("%s %d-%d of %d retained",label,total ? range.first+1:0,range.end,total);
    if(range.pages)igText("Page %d of %d",range.page+1,range.pages);else igTextUnformatted("No retained rows in this scope",NULL);
    if(range.pages>1)gd_inspector_record_page(label,range.page,range.pages-1);
    igPopID();return range;
}
#endif
