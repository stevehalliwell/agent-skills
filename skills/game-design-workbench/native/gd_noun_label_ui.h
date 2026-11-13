#ifndef GD_NOUN_LABEL_UI_H
#define GD_NOUN_LABEL_UI_H
#include "gd_noun_label.h"
/* Include after cimgui. Decorations do not change widget IDs, text or layout. */
static inline void gd_noun_ui_decorate(const GD_Record *record,const char *text,ImVec2_c position){
    char prefix[GD_MAX_PATH];const char *line;ImVec2_c extent;ImU32 color;
    if(!igIsItemVisible() || !gd_noun_label_prefix(record,text,prefix,sizeof(prefix)))return;
    {size_t length=strlen(prefix);if(length+1>=sizeof(prefix))return;prefix[length]='M';prefix[length+1]=0;}
    /* Sentinel preserves native trailing advance before embedded label. */
    line=strrchr(prefix,'\n');extent=igCalcTextSize(line ? line+1:prefix,NULL,false,-1);position.x+=extent.x-igCalcTextSize("M",NULL,false,-1).x;
    if(line)position.y+=igCalcTextSize(prefix,NULL,false,-1).y-igGetFontSize();
    color=igGetColorU32_Col(ImGuiCol_Text,1);
    if(gd_noun_label_style(record)==GD_NOUN_LABEL_BOLD){position.x+=1;ImDrawList_AddText_Vec2(igGetWindowDrawList(),position,color,record->display_name,NULL);}
    else{float width=igCalcTextSize(record->display_name,NULL,false,-1).x;position.y+=igGetFontSize()+1;ImDrawList_AddLine(igGetWindowDrawList(),position,(ImVec2_c){position.x+width,position.y},color,1);}
}
static inline void gd_noun_ui_text(const GD_Record *record,const char *text){
    ImVec2_c position=igGetCursorScreenPos();igTextUnformatted(text,NULL);gd_noun_ui_decorate(record,text,position);
}
static inline int gd_noun_ui_small_button(const GD_Record *record,const char *text){
    int clicked=igSmallButton(text);ImVec2_c first=igGetItemRectMin(),last=igGetItemRectMax(),extent=igCalcTextSize(text,NULL,true,-1);
    gd_noun_ui_decorate(record,text,(ImVec2_c){first.x+(last.x-first.x-extent.x)/2,first.y+(last.y-first.y-extent.y)/2});return clicked;
}
#endif
