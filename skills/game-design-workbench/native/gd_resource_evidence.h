#ifndef GD_RESOURCE_EVIDENCE_H
#define GD_RESOURCE_EVIDENCE_H
#include "gd_evidence_page.h"
#include <assert.h>
/* Standalone resource presentation only. Four provenance rows per page; wrapped
   identities and per-tab scrolling do not resize native retained results. */
#define GD_RESOURCE_EVIDENCE_ROWS 4
typedef struct {
    int members,sources,bin[2],witness[2],support,bypass,incidents;
    int selected,row,column,target,relation,peer;
    float scroll[6];
    GD_Point mouse;
    int clicked;
} GD_ResourceEvidence;
static void resource_sidebar_input(GD_ResourceEvidence *evidence,GD_Rect panel,int tab,float bottom,GD_Point mouse,float wheel,int pressed) {
    assert(tab>=0 && tab<6);
    int inside=gd_rect_hit(mouse,panel);float maximum=fmaxf(0,bottom-panel.y-panel.height);
    if(inside)evidence->scroll[tab]-=wheel*42;
    evidence->scroll[tab]=fmaxf(0,fminf(evidence->scroll[tab],maximum));
    evidence->clicked=pressed && inside;if(inside)mouse.y+=evidence->scroll[tab];evidence->mouse=mouse;
}
static GD_EvidencePage resource_pager(const char *label,int *cursor,int total,int x,int *y,const GD_ResourceEvidence *evidence) {
    GD_EvidencePage page=gd_evidence_page(total,GD_RESOURCE_EVIDENCE_ROWS,*cursor);
    GD_Rect previous={(float)x,(float)*y,78,22},next={(float)x+86,(float)*y,62,22};
    if(evidence->clicked){
        if(page.page && gd_rect_hit(evidence->mouse,previous))--page.page;
        else if(page.page+1<page.pages && gd_rect_hit(evidence->mouse,next))++page.page;
    }
    page=gd_evidence_page(total,GD_RESOURCE_EVIDENCE_ROWS,page.page);*cursor=page.page;
    gd_scene_outline(previous,1,muted);gd_scene_outline(next,1,muted);
    gd_scene_text("Previous",x+4,*y+4,12,page.page ? foreground:muted);
    gd_scene_text("Next",x+90,*y+4,12,page.page+1<page.pages ? foreground:muted);
    gd_scene_text(gd_scene_format("%d / %d",page.pages ? page.page+1:0,page.pages),x+160,*y+4,12,muted);*y+=25;
    gd_scene_text(gd_scene_format("%s %d-%d of %d retained",label,total ? page.first+1:0,page.end,total),x,*y,12,muted);*y+=22;
    return page;
}
/* Wrap full UTF-8 text without cutting byte sequences or fixed-prefix identity
   truncation. Caller scrolls/clips panel; this returns full measured bottom. */
static int resource_text(const char *text,int x,int y,int size,GD_Color color) {
    char line[512];int used=0,width=gd_scene_width()-x-20,height=size+5;
    if(!text)text="unspecified";if(width<1)width=1;
    while(*text){
        unsigned int codepoint=0;int bytes=igImTextCharFromUtf8(&codepoint,text,NULL);
        if(bytes<=0 || bytes>4)break;
        if(codepoint=='\r'){text+=bytes;continue;}
        if(used+bytes>=(int)sizeof(line)){line[used]=0;gd_scene_text(line,x,y,size,color);y+=height;used=0;}
        memcpy(line+used,text,(size_t)bytes);line[used+bytes]=0;
        if(codepoint=='\n' || (used && gd_scene_text_width(line,size)>width)){
            line[used]=0;gd_scene_text(line,x,y,size,color);y+=height;used=0;
            if(codepoint=='\n'){text+=bytes;continue;}
        }
        memcpy(line+used,text,(size_t)bytes);used+=bytes;text+=bytes;
    }
    line[used]=0;gd_scene_text(line,x,y,size,color);return y+height;
}
#endif
