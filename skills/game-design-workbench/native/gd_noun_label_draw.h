#ifndef GD_NOUN_LABEL_DRAW_H
#define GD_NOUN_LABEL_DRAW_H
#include "gd_noun_label.h"
#include "raylib.h"
/* Reuse default font: one-unit overstrike supplies weight without another
   font asset. Style only explicit noun label, not surrounding card prose. */
static inline Rectangle gd_noun_draw_text(const GD_Record *record,const char *text,Vector2 position,int size,float spacing,Color color){
    Font font=GetFontDefault();Vector2 extent=MeasureTextEx(font,text,(float)size,spacing);char prefix[GD_MAX_PATH];
    Rectangle bounds={position.x,position.y,extent.x,extent.y};
    DrawTextEx(font,text,position,(float)size,spacing,color);
    if(gd_noun_label_prefix(record,text,prefix,sizeof(prefix))){
        const char *line=strrchr(prefix,'\n');size_t length=strlen(prefix);Vector2 label=MeasureTextEx(font,record->display_name,(float)size,spacing);
        if(length+1>=sizeof(prefix))return bounds;
        /* Sentinel cancels omitted final-glyph spacing in text measurements. */
        prefix[length]='M';prefix[length+1]=0;
        Vector2 at={position.x+MeasureTextEx(font,line ? line+1:prefix,(float)size,spacing).x-MeasureTextEx(font,"M",(float)size,spacing).x,position.y+(line ? MeasureTextEx(font,prefix,(float)size,spacing).y-size:0)};
        if(gd_noun_label_style(record)==GD_NOUN_LABEL_BOLD){at.x+=1;DrawTextEx(font,record->display_name,at,(float)size,spacing,color);bounds.width+=1;}
        else{DrawLineEx((Vector2){at.x,at.y+size+1},(Vector2){at.x+label.x,at.y+size+1},1,color);bounds.height+=2;}
    }
    return bounds;
}
#endif
