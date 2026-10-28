#ifndef GD_GRAPH_OVERVIEW_DRAW_H
#define GD_GRAPH_OVERVIEW_DRAW_H
#include "gd_graph_analysis.h"
#include "raylib.h"
#include <math.h>

static inline Rectangle gd_graph_overview_bounds(Rectangle viewport) {
    float size=fminf(288,fminf(viewport.width-40,viewport.height-80));
    if (size<64) return (Rectangle){0,0,0,0};
    return (Rectangle){viewport.x+viewport.width-size-32,viewport.y+12,size+16,size+54};
}
static inline int gd_graph_overview_hit(const GD_OverviewControls *controls, Rectangle viewport, Vector2 mouse) {
    Rectangle bounds=gd_graph_overview_bounds(viewport);
    return controls->enabled && bounds.width>0 && CheckCollisionPointRec(mouse,bounds);
}
static inline int gd_graph_overview_level(int value) {
    int level=0; while (value) { ++level; value>>=1; } return level;
}
/* Fixed-screen inset, not a second source projection or a camera-dependent
   subset. Pointer events are consumed here; callers suppress graph dragging. */
static inline void gd_graph_overview_draw(GD_GraphAnalysis *analysis, GD_OverviewControls *controls, Rectangle viewport, int allow_input) {
    const GD_GraphOverview *overview=&analysis->overview; Rectangle box=gd_graph_overview_bounds(viewport);
    int row,column,maximum=gd_graph_overview_level(overview->peak); float size=box.width-16,cell;
    Vector2 mouse=GetMousePosition(); Color text={225,233,241,255};
    if (!controls->enabled || !overview->valid || !box.width) return;
    DrawRectangleRec(box,(Color){18,24,34,250}); DrawRectangleLinesEx(box,1,(Color){150,169,190,255});
    DrawText(TextFormat("%d bins / %d identities / %d link rows",overview->bins,overview->node_count,overview->link_count),(int)box.x+8,(int)box.y+6,12,text);
    DrawText(overview->undirected ? "Symmetric incidence; NOT flow":"Rows -> columns: shown arrows",(int)box.x+8,(int)box.y+22,12,text);
    if (!overview->bins) return;
    cell=size/overview->bins;
    for (row=0;row<overview->bins;++row) for (column=0;column<overview->bins;++column) {
        int count=overview->counts[row*overview->bins+column];
        int shade=count ? 40+215*gd_graph_overview_level(count)/(maximum ? maximum:1):28;
        DrawRectangleRec((Rectangle){box.x+8+column*cell,box.y+38+row*cell,cell-0.5f,cell-0.5f},(Color){(unsigned char)shade,(unsigned char)shade,(unsigned char)shade,255});
    }
    if (allow_input && mouse.x>=box.x+8 && mouse.x<box.x+8+size && mouse.y>=box.y+38 && mouse.y<box.y+38+size) {
        column=(int)((mouse.x-box.x-8)/cell); row=(int)((mouse.y-box.y-38)/cell);
        DrawRectangleLinesEx((Rectangle){box.x+8+column*cell,box.y+38+row*cell,cell,cell},1,(Color){150,169,190,255});
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            controls->row=row; controls->column=column; gd_graph_overview_select(analysis,row,column);
        }
    }
    if (controls->row>=0 && controls->column>=0 && controls->row<overview->bins && controls->column<overview->bins)
        DrawRectangleLinesEx((Rectangle){box.x+8+controls->column*cell,box.y+38+controls->row*cell,cell,cell},2,(Color){255,217,125,255});
}
#endif
