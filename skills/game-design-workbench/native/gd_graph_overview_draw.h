#ifndef GD_GRAPH_OVERVIEW_DRAW_H
#define GD_GRAPH_OVERVIEW_DRAW_H
#include "gd_graph_analysis.h"
#include "gd_count_heatmap.h"
#include "raylib.h"
#include <math.h>

enum { GD_OVERVIEW_HEADER_HEIGHT=24 };
static inline Rectangle gd_graph_overview_bounds(Rectangle viewport) {
    float size=fminf(288,fminf(viewport.width-40,viewport.height-108-GD_OVERVIEW_HEADER_HEIGHT));
    if (size<64) return (Rectangle){0,0,0,0};
    return (Rectangle){viewport.x+viewport.width-size-32,viewport.y+12,size+16,size+82+GD_OVERVIEW_HEADER_HEIGHT};
}
static inline Rectangle gd_graph_overview_visible_bounds(const GD_OverviewControls *controls,Rectangle viewport) {
    Rectangle bounds=gd_graph_overview_bounds(viewport);float width;
    if(controls->enabled && bounds.width>0)return bounds;
    width=fminf(160,viewport.width-32);
    if(width<64 || viewport.height<GD_OVERVIEW_HEADER_HEIGHT+24)return (Rectangle){0};
    return (Rectangle){viewport.x+viewport.width-width-16,viewport.y+12,width,GD_OVERVIEW_HEADER_HEIGHT};
}
static inline int gd_graph_overview_hit(const GD_OverviewControls *controls, Rectangle viewport, Vector2 mouse) {
    Rectangle bounds=gd_graph_overview_visible_bounds(controls,viewport);
    return bounds.width>0 && CheckCollisionPointRec(mouse,bounds);
}
/* Same header geometry for drawing, input consumption and headless checks. */
static inline int gd_graph_overview_toggle(GD_OverviewControls *controls,Rectangle viewport,Vector2 mouse) {
    Rectangle header=gd_graph_overview_visible_bounds(controls,viewport);header.height=GD_OVERVIEW_HEADER_HEIGHT;
    if(!header.width || !CheckCollisionPointRec(mouse,header))return 0;
    controls->enabled=!controls->enabled;return 1;
}
/* Fixed-screen inset, not a second source projection or a camera-dependent
   subset. Pointer events are consumed here; callers suppress graph dragging. */
static inline void gd_graph_overview_draw(GD_GraphAnalysis *analysis, GD_OverviewControls *controls, Rectangle viewport, int allow_input) {
    const GD_GraphOverview *overview=&analysis->overview; Rectangle box,header;
    int row,column; GD_CountRange range={0}; float size,cell;
    Vector2 mouse=GetMousePosition(); Color text={225,233,241,255};
    if(allow_input && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))gd_graph_overview_toggle(controls,viewport,mouse);
    box=gd_graph_overview_visible_bounds(controls,viewport);if(!box.width)return;
    header=box;header.height=GD_OVERVIEW_HEADER_HEIGHT;
    DrawRectangleRec(header,allow_input && CheckCollisionPointRec(mouse,header) ? (Color){45,60,78,255}:(Color){29,38,51,250});
    DrawRectangleLinesEx(header,1,(Color){150,169,190,255});
    if(controls->row>=0 && controls->column>=0)DrawCircle((int)header.x+10,(int)header.y+12,3,(Color){255,217,125,255});
    if(header.width>=112)DrawText("Overview",(int)header.x+(controls->row>=0 && controls->column>=0 ? 20:8),(int)header.y+5,14,text);
    DrawText(controls->enabled ? "[-]":"[+]",(int)(header.x+header.width)-30,(int)header.y+5,14,text);
    if(!controls->enabled || !overview->valid || box.height<=GD_OVERVIEW_HEADER_HEIGHT)return;
    box.y+=GD_OVERVIEW_HEADER_HEIGHT;box.height-=GD_OVERVIEW_HEADER_HEIGHT;size=box.width-16;
    DrawRectangleRec(box,(Color){18,24,34,250}); DrawRectangleLinesEx(box,1,(Color){150,169,190,255});
    DrawText(TextFormat("%d bins / %d identities / %d link rows",overview->bins,overview->node_count,overview->link_count),(int)box.x+8,(int)box.y+6,12,text);
    DrawText(overview->undirected ? "Symmetric incidence; NOT flow":"Rows -> columns: shown arrows",(int)box.x+8,(int)box.y+22,12,text);
    for (row=0;row<overview->bins*overview->bins;++row) gd_count_range_add(&range,overview->counts[row]);
    gd_count_legend(range,box.x+8,box.y+38,size,"Link rows");
    if (!overview->bins) return;
    cell=size/overview->bins;
    for (row=0;row<overview->bins;++row) for (column=0;column<overview->bins;++column) {
        int count=overview->counts[row*overview->bins+column];
        DrawRectangleRec((Rectangle){box.x+8+column*cell,box.y+66+row*cell,cell-0.5f,cell-0.5f},gd_count_color(range,count));
    }
    if (allow_input && mouse.x>=box.x+8 && mouse.x<box.x+8+size && mouse.y>=box.y+66 && mouse.y<box.y+66+size) {
        column=(int)((mouse.x-box.x-8)/cell); row=(int)((mouse.y-box.y-66)/cell);
        DrawRectangleLinesEx((Rectangle){box.x+8+column*cell,box.y+66+row*cell,cell,cell},1,(Color){150,169,190,255});
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            controls->row=row; controls->column=column; gd_graph_overview_select(analysis,row,column);
        }
    }
    if (controls->row>=0 && controls->column>=0 && controls->row<overview->bins && controls->column<overview->bins)
        DrawRectangleLinesEx((Rectangle){box.x+8+controls->column*cell,box.y+66+controls->row*cell,cell,cell},2,(Color){255,217,125,255});
}
#endif
