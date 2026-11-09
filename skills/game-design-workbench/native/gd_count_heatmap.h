#ifndef GD_COUNT_HEATMAP_H
#define GD_COUNT_HEATMAP_H
#include "gd_matrix_grid.h"
#include "raylib.h"

/* Shared linear positive-count scale. White count labels retain contrast.
   Full retained dataset determines range, never camera or selected cell. */
enum { GD_COUNT_GRID_HEADER=56 };
static inline Color gd_count_color(GD_CountRange range, int count) {
    float value=gd_count_fraction(range,count);
    if (count<=0 || !range.maximum) return (Color){29,38,51,255};
    return (Color){(unsigned char)(32-8*value),(unsigned char)(48+64*value),
        (unsigned char)(82+14*value),255};
}
/* Fixed-screen 28-pixel legend. Equal bounds use same solid midpoint as cells. */
static inline void gd_count_legend(GD_CountRange range, float x, float y, float width, const char *metric) {
    Color text={226,233,240,255};
    if (width<1) return;
    if (!range.maximum) {
        DrawText(TextFormat("%s: no populated cells",metric),(int)x,(int)y,12,text);
        return;
    }
    DrawText(TextFormat("%s min %d",metric,range.minimum),(int)x,(int)y,12,text);
    {
        const char *label=TextFormat("max %d",range.maximum);
        DrawText(label,(int)(x+width-MeasureText(label,12)),(int)y,12,text);
    }
    DrawRectangleGradientH((int)x,(int)y+16,(int)width,9,
        gd_count_color(range,range.minimum),gd_count_color(range,range.maximum));
}
#endif
