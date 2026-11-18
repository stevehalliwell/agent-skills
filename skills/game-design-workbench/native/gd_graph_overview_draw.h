#ifndef GD_GRAPH_OVERVIEW_DRAW_H
#define GD_GRAPH_OVERVIEW_DRAW_H
#include "gd_graph_analysis.h"
#include "gd_count_heatmap.h"
#include "gd_scene_draw.h"
#include <math.h>

enum { GD_OVERVIEW_HEADER_HEIGHT=24 };
static inline GD_Rect gd_graph_overview_bounds(GD_Rect viewport) {
    float size=fminf(288,fminf(viewport.width-40,viewport.height-108-GD_OVERVIEW_HEADER_HEIGHT));
    if (size<64) return (GD_Rect){0,0,0,0};
    return (GD_Rect){viewport.x+viewport.width-size-32,viewport.y+12,size+16,size+82+GD_OVERVIEW_HEADER_HEIGHT};
}
static inline GD_Rect gd_graph_overview_visible_bounds(const GD_OverviewControls *controls,GD_Rect viewport) {
    GD_Rect bounds=gd_graph_overview_bounds(viewport);float width;
    if(controls->enabled && bounds.width>0)return bounds;
    width=fminf(160,viewport.width-32);
    if(width<64 || viewport.height<GD_OVERVIEW_HEADER_HEIGHT+24)return (GD_Rect){0};
    return (GD_Rect){viewport.x+viewport.width-width-16,viewport.y+12,width,GD_OVERVIEW_HEADER_HEIGHT};
}
static inline int gd_graph_overview_hit(const GD_OverviewControls *controls, GD_Rect viewport, GD_Point mouse) {
    GD_Rect bounds=gd_graph_overview_visible_bounds(controls,viewport);
    return bounds.width>0 && gd_rect_hit(mouse,bounds);
}
/* Same header geometry for drawing, input consumption and headless checks. */
static inline int gd_graph_overview_toggle(GD_OverviewControls *controls,GD_Rect viewport,GD_Point mouse) {
    GD_Rect header=gd_graph_overview_visible_bounds(controls,viewport);header.height=GD_OVERVIEW_HEADER_HEIGHT;
    if(!header.width || !gd_rect_hit(mouse,header))return 0;
    controls->enabled=!controls->enabled;return 1;
}
/* Projection indices are bounded here; bins may contain several identities. */
static inline int gd_graph_overview_node_bin(const GD_GraphAnalysis *analysis,int node){
    int bin;if(!analysis->initialized || !analysis->overview.valid || node<0 || node>=analysis->node_count || !analysis->shown[node])return -1;
    bin=analysis->overview.node_bin[node];return bin>=0 && bin<analysis->overview.bins ? bin:-1;
}
/* Fixed-screen inset, not a second source projection or a camera-dependent
   subset. Pointer events are consumed here; callers suppress graph dragging. */
static inline void gd_graph_overview_draw_metric(GD_GraphAnalysis *analysis, GD_OverviewControls *controls, GD_Rect viewport, int selected_node,int allow_input,const char *metric) {
    const GD_GraphOverview *overview=&analysis->overview; GD_Rect box,header;
    int row,column,node_bin; GD_CountRange range={0}; float size,cell;
    GD_Point mouse=gd_mouse_position(); GD_Color text={225,233,241,255};
    if(allow_input && gd_mouse_pressed(ImGuiMouseButton_Left))gd_graph_overview_toggle(controls,viewport,mouse);
    box=gd_graph_overview_visible_bounds(controls,viewport);if(!box.width)return;
    header=box;header.height=GD_OVERVIEW_HEADER_HEIGHT;
    gd_scene_rect(header,allow_input && gd_rect_hit(mouse,header) ? (GD_Color){45,60,78,255}:(GD_Color){29,38,51,250});
    gd_scene_outline(header,1,(GD_Color){150,169,190,255});
    if(controls->row>=0 && controls->column>=0)gd_scene_circle_pixels((int)header.x+10,(int)header.y+12,3,(GD_Color){255,217,125,255});
    if(header.width>=112)gd_scene_text("Overview",(int)header.x+(controls->row>=0 && controls->column>=0 ? 20:8),(int)header.y+5,14,text);
    gd_scene_text(controls->enabled ? "[-]":"[+]",(int)(header.x+header.width)-30,(int)header.y+5,14,text);
    if(!controls->enabled || !overview->valid || box.height<=GD_OVERVIEW_HEADER_HEIGHT)return;
    box.y+=GD_OVERVIEW_HEADER_HEIGHT;box.height-=GD_OVERVIEW_HEADER_HEIGHT;size=box.width-16;
    gd_scene_rect(box,(GD_Color){18,24,34,250}); gd_scene_outline(box,1,(GD_Color){150,169,190,255});
    gd_scene_text(gd_scene_format("%d bins / %d identities / %d link rows",overview->bins,overview->node_count,overview->link_count),(int)box.x+8,(int)box.y+6,12,text);
    gd_scene_text(overview->undirected ? "Symmetric incidence; NOT flow":"Rows -> columns: shown arrows",(int)box.x+8,(int)box.y+22,12,text);
    for (row=0;row<overview->bins*overview->bins;++row) gd_count_range_add(&range,overview->counts[row]);
    gd_count_legend(range,box.x+8,box.y+38,size,metric);
    if (!overview->bins) return;
    cell=size/overview->bins;
    for (row=0;row<overview->bins;++row) for (column=0;column<overview->bins;++column) {
        int count=overview->counts[row*overview->bins+column];
        gd_scene_rect((GD_Rect){box.x+8+column*cell,box.y+66+row*cell,cell-0.5f,cell-0.5f},gd_count_color(range,count));
    }
    node_bin=gd_graph_overview_node_bin(analysis,selected_node);
    if(node_bin>=0){
        GD_Color cue={100,178,245,255};const char *label=gd_scene_format("Node bin %d (%d identities)",node_bin,overview->start[node_bin+1]-overview->start[node_bin]);
        gd_scene_outline((GD_Rect){box.x+8,box.y+66+node_bin*cell,size,cell},1.5f,cue);
        gd_scene_outline((GD_Rect){box.x+8+node_bin*cell,box.y+66,cell,size},1.5f,cue);
        if(gd_scene_text_width(label,12)>size)label=gd_scene_format("Bin %d",node_bin);
        gd_scene_text(label,(int)box.x+8,(int)(box.y+68+size),12,cue);
    }
    if (allow_input && mouse.x>=box.x+8 && mouse.x<box.x+8+size && mouse.y>=box.y+66 && mouse.y<box.y+66+size) {
        column=(int)((mouse.x-box.x-8)/cell); row=(int)((mouse.y-box.y-66)/cell);
        gd_scene_outline((GD_Rect){box.x+8+column*cell,box.y+66+row*cell,cell,cell},1,(GD_Color){150,169,190,255});
        if (gd_mouse_pressed(ImGuiMouseButton_Left)) {
            controls->row=row; controls->column=column; gd_graph_overview_select(analysis,row,column);
        }
    }
    if (controls->row>=0 && controls->column>=0 && controls->row<overview->bins && controls->column<overview->bins)
        gd_scene_outline((GD_Rect){box.x+8+controls->column*cell,box.y+66+controls->row*cell,cell,cell},2,(GD_Color){255,217,125,255});
}
static inline void gd_graph_overview_draw(GD_GraphAnalysis *analysis, GD_OverviewControls *controls, GD_Rect viewport, int selected_node,int allow_input) {
    gd_graph_overview_draw_metric(analysis,controls,viewport,selected_node,allow_input,"Link rows");
}
#endif
