#ifndef GD_COUNT_HEATMAP_H
#define GD_COUNT_HEATMAP_H
#include "gd_matrix_grid.h"
#include "gd_scene_draw.h"

/* Shared linear positive-count scale. White count labels retain contrast.
   Full retained dataset determines range, never camera or selected cell. */
enum { GD_COUNT_GRID_HEADER=56 };
static inline GD_Color gd_count_color(GD_CountRange range, int count) {
    float value=gd_count_fraction(range,count);
    if (count<=0 || !range.maximum) return (GD_Color){29,38,51,255};
    return (GD_Color){(unsigned char)(32-8*value),(unsigned char)(48+64*value),
        (unsigned char)(82+14*value),255};
}
/* Stroke palette is brighter than cell fills; share full-dataset fraction. */
enum { GD_COUNT_LINK_WIDTH_MIN=2,GD_COUNT_LINK_WIDTH_MAX=8 };
typedef struct { GD_Color color;float width; } GD_CountLinkStyle;
static inline GD_CountLinkStyle gd_count_link_style(GD_CountRange range,int count) {
    float value=gd_count_fraction(range,count);
    if(count<=0 || !range.maximum)return (GD_CountLinkStyle){{0,0,0,0},0};
    return (GD_CountLinkStyle){{(unsigned char)(64+16*value),(unsigned char)(100+124*value),(unsigned char)(176+32*value),255},GD_COUNT_LINK_WIDTH_MIN+(GD_COUNT_LINK_WIDTH_MAX-GD_COUNT_LINK_WIDTH_MIN)*value};
}
static inline int gd_count_link_samples(GD_CountRange range,int values[3]) {
    int count=1;if(!range.maximum)return 0;values[0]=range.minimum;
    if(range.minimum==range.maximum)return count;
    if(range.maximum-range.minimum>1)values[count++]=range.minimum+(range.maximum-range.minimum)/2;
    values[count++]=range.maximum;return count;
}
/* Fixed-screen 28-pixel line samples, not cell-fill swatches. */
static inline void gd_count_link_legend(GD_CountRange range,float x,float y,float width,const char *metric) {
    GD_Color text={226,233,240,255};int values[3],count=gd_count_link_samples(range,values),i;
    if(width<1)return;
    if(!count){gd_scene_text(gd_scene_format("%s: no populated links",metric),(int)x,(int)y,12,text);return;}
    gd_scene_text(count==1 ? gd_scene_format("%s: all links %d",metric,range.minimum):gd_scene_format("%s (full graph): %d..%d",metric,range.minimum,range.maximum),(int)x,(int)y,12,text);
    if(count==3 && width<288){values[1]=values[2];count=2;}
    for(i=0;i<count;++i){float start=x+i*width/count;GD_CountLinkStyle style=gd_count_link_style(range,values[i]);
        gd_scene_line((GD_Point){start,y+22},(GD_Point){start+28,y+22},style.width,style.color);
        gd_scene_text(gd_scene_format("%d",values[i]),(int)start+34,(int)y+16,12,text);
    }
}
/* Fixed-screen 28-pixel legend. Equal bounds use same solid midpoint as cells. */
static inline void gd_count_legend(GD_CountRange range, float x, float y, float width, const char *metric) {
    GD_Color text={226,233,240,255};
    if (width<1) return;
    if (!range.maximum) {
        gd_scene_text(gd_scene_format("%s: no populated cells",metric),(int)x,(int)y,12,text);
        return;
    }
    gd_scene_text(gd_scene_format("%s min %d",metric,range.minimum),(int)x,(int)y,12,text);
    {
        const char *label=gd_scene_format("max %d",range.maximum);
        gd_scene_text(label,(int)(x+width-gd_scene_text_width(label,12)),(int)y,12,text);
    }
    gd_scene_gradient((int)x,(int)y+16,(int)width,9,
        gd_count_color(range,range.minimum),gd_count_color(range,range.maximum));
}
#endif
