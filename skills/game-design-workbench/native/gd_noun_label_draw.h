#ifndef GD_NOUN_LABEL_DRAW_H
#define GD_NOUN_LABEL_DRAW_H
#include "gd_noun_label.h"
#include "gd_scene_draw.h"
/* Use shared font metrics; decorate only the known noun label. */
static inline GD_Rect gd_noun_draw_text(const GD_Record *record,const char *text,GD_Point position,int size,GD_Color color){
    GD_Point extent=gd_scene_text_extent(text,(float)size);char prefix[GD_MAX_PATH];
    GD_Rect bounds={position.x,position.y,extent.x,extent.y};
    gd_scene_text_at(text,position,(float)size,color);
    if(gd_noun_label_prefix(record,text,prefix,sizeof(prefix))){
        const char *line=strrchr(prefix,'\n');
        GD_Point at={position.x+gd_scene_text_extent(line ? line+1:prefix,(float)size).x,position.y};
        for(const char *p=prefix;*p;++p)if(*p=='\n')at.y+=size;
        if(gd_noun_label_style(record)==GD_NOUN_LABEL_BOLD){at.x+=1;gd_scene_text_at(record->display_name,at,(float)size,color);bounds.width+=1;}
        else{float width=gd_scene_text_extent(record->display_name,(float)size).x;gd_scene_line((GD_Point){at.x,at.y+size+1},(GD_Point){at.x+width,at.y+size+1},1,color);bounds.height+=2;}
    }
    return bounds;
}
#endif
