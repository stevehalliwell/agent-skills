#ifndef GD_GRAPH_OVERVIEW_DRAW_H
#define GD_GRAPH_OVERVIEW_DRAW_H
#include "gd_graph_analysis.h"
#include "gd_count_heatmap.h"
#include "gd_scene_draw.h"
#include <math.h>

enum {
    GD_OVERVIEW_HEADER_HEIGHT=24, GD_OVERVIEW_MARGIN=8, GD_OVERVIEW_BODY_HEIGHT=82,
    GD_OVERVIEW_MIN_SIZE=128, GD_OVERVIEW_DEFAULT_SIZE=208, GD_OVERVIEW_MAX_SIZE=512,
    GD_OVERVIEW_GRIP=14, GD_OVERVIEW_TOGGLE=30, GD_OVERVIEW_MOVE=1, GD_OVERVIEW_RESIZE=2, GD_OVERVIEW_TOGGLING=3
};
static inline GD_Rect gd_graph_overview_visible_bounds(const GD_OverviewControls *controls,GD_Rect viewport) {
    float w=viewport.width-2*GD_OVERVIEW_MARGIN,h=viewport.height-2*GD_OVERVIEW_MARGIN;
    float size=fminf(controls->size>0 ? controls->size:GD_OVERVIEW_DEFAULT_SIZE,
        fminf(w-16,h-GD_OVERVIEW_HEADER_HEIGHT-GD_OVERVIEW_BODY_HEIGHT));
    GD_Rect box={0};
    if(w<64 || h<GD_OVERVIEW_HEADER_HEIGHT)return box;
    box.width=fminf(160,w);box.height=GD_OVERVIEW_HEADER_HEIGHT;
    if(controls->enabled && size>=GD_OVERVIEW_MIN_SIZE){
        box.width=size+16;box.height+=size+GD_OVERVIEW_BODY_HEIGHT;
    }
    box.x=viewport.x+GD_OVERVIEW_MARGIN+(controls->placed ? fminf(fmaxf(0,controls->x),w-box.width):w-box.width);
    box.y=viewport.y+GD_OVERVIEW_MARGIN+(controls->placed ? fminf(fmaxf(0,controls->y),h-box.height):h-box.height);
    return box;
}
static inline int gd_graph_overview_hit(const GD_OverviewControls *controls, GD_Rect viewport, GD_Point mouse) {
    GD_Rect box=gd_graph_overview_visible_bounds(controls,viewport);
    return controls->gesture || (box.width>0 && gd_rect_hit(mouse,box));
}
static inline GD_Rect gd_graph_overview_toggle_bounds(GD_Rect box) {
    return (GD_Rect){box.x+box.width-GD_OVERVIEW_TOGGLE,box.y,GD_OVERVIEW_TOGGLE,GD_OVERVIEW_HEADER_HEIGHT};
}
static inline int gd_graph_overview_toggle(GD_OverviewControls *controls,GD_Rect viewport,GD_Point mouse) {
    GD_Rect box=gd_graph_overview_visible_bounds(controls,viewport);
    if(!box.width || !gd_rect_hit(mouse,gd_graph_overview_toggle_bounds(box)))return 0;
    controls->enabled=!controls->enabled;return 1;
}
/* Update before graph input so drag ownership survives leaving the inset. */
static inline void gd_graph_overview_input(GD_OverviewControls *controls,GD_Rect viewport,int allow_input) {
    GD_Rect box=gd_graph_overview_visible_bounds(controls,viewport);
    GD_Point mouse=gd_mouse_position();
    int down=igIsMouseDown_Nil(ImGuiMouseButton_Left);
    if(!down || !box.width)controls->gesture=0;
    if(!box.width || !igIsMousePosValid(NULL))return;
    if(!controls->gesture && allow_input && gd_rect_hit(mouse,box) && mouse.y<box.y+GD_OVERVIEW_HEADER_HEIGHT){
        int toggle=gd_rect_hit(mouse,gd_graph_overview_toggle_bounds(box));
        int resize=box.height>GD_OVERVIEW_HEADER_HEIGHT && gd_rect_hit(mouse,(GD_Rect){box.x,box.y,GD_OVERVIEW_GRIP,GD_OVERVIEW_GRIP});
        if(!toggle)igSetMouseCursor(resize ? ImGuiMouseCursor_ResizeNWSE:ImGuiMouseCursor_ResizeAll);
        if(gd_mouse_pressed(ImGuiMouseButton_Left)){
            if(toggle){gd_graph_overview_toggle(controls,viewport,mouse);controls->gesture=GD_OVERVIEW_TOGGLING;return;}
            controls->gesture=resize ? GD_OVERVIEW_RESIZE:GD_OVERVIEW_MOVE;
            controls->mouse_x=mouse.x;controls->mouse_y=mouse.y;
            controls->start_x=box.x;controls->start_y=box.y;controls->start_width=box.width;controls->start_height=box.height;
        }
    }
    if(controls->gesture && controls->gesture!=GD_OVERVIEW_TOGGLING){
        GD_Rect start={controls->start_x,controls->start_y,controls->start_width,controls->start_height};
        float dx=mouse.x-controls->mouse_x,dy=mouse.y-controls->mouse_y;
        float left=viewport.x+GD_OVERVIEW_MARGIN,top=viewport.y+GD_OVERVIEW_MARGIN;
        if(!controls->placed && dx*dx+dy*dy<=16)return;
        if(controls->gesture==GD_OVERVIEW_RESIZE){
            float max=fminf(GD_OVERVIEW_MAX_SIZE,fminf(start.x+start.width-left-16,
                start.y+start.height-top-GD_OVERVIEW_HEADER_HEIGHT-GD_OVERVIEW_BODY_HEIGHT));
            float size=fminf(max,fmaxf(GD_OVERVIEW_MIN_SIZE,start.width-16-(dx+dy)*0.5f));
            if(size<GD_OVERVIEW_MIN_SIZE)return;
            controls->size=size;dx=dy=start.width-16-size;
        }
        controls->placed=1;
        controls->x=fmaxf(0,start.x+dx-left);controls->y=fmaxf(0,start.y+dy-top);
    }
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
    box=gd_graph_overview_visible_bounds(controls,viewport);if(!box.width)return;
    header=box;header.height=GD_OVERVIEW_HEADER_HEIGHT;
    gd_scene_clip((int)box.x,(int)box.y,(int)box.width,(int)box.height);
    allow_input=allow_input && !controls->gesture;
    gd_scene_rect(header,allow_input && gd_rect_hit(mouse,header) ? (GD_Color){45,60,78,255}:(GD_Color){29,38,51,250});
    gd_scene_outline(header,1,(GD_Color){150,169,190,255});
    if(box.height>GD_OVERVIEW_HEADER_HEIGHT){
        gd_scene_line((GD_Point){header.x+3,header.y+10},(GD_Point){header.x+10,header.y+3},1,text);
        gd_scene_line((GD_Point){header.x+3,header.y+6},(GD_Point){header.x+6,header.y+3},1,text);
    }
    if(controls->row>=0 && controls->column>=0)gd_scene_circle_pixels((int)header.x+17,(int)header.y+12,3,(GD_Color){255,217,125,255});
    if(header.width>=112)gd_scene_text("Overview",(int)header.x+24,(int)header.y+5,14,text);
    gd_scene_text(controls->enabled ? "[-]":"[+]",(int)(header.x+header.width)-25,(int)header.y+5,14,text);
    if(!controls->enabled || !overview->valid || box.height<=GD_OVERVIEW_HEADER_HEIGHT){gd_scene_unclip();return;}
    box.y+=GD_OVERVIEW_HEADER_HEIGHT;box.height-=GD_OVERVIEW_HEADER_HEIGHT;size=box.width-16;
    gd_scene_rect(box,(GD_Color){18,24,34,250}); gd_scene_outline(box,1,(GD_Color){150,169,190,255});
    gd_scene_text(gd_scene_format("%d bins / %d identities / %d link rows",overview->bins,overview->node_count,overview->link_count),(int)box.x+8,(int)box.y+6,12,text);
    gd_scene_text(overview->undirected ? "Symmetric incidence; NOT flow":"Rows -> columns: shown arrows",(int)box.x+8,(int)box.y+22,12,text);
    for (row=0;row<overview->bins*overview->bins;++row) gd_count_range_add(&range,overview->counts[row]);
    gd_count_legend(range,box.x+8,box.y+38,size,metric);
    if (!overview->bins) {gd_scene_unclip();return;}
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
    gd_scene_unclip();
}
static inline void gd_graph_overview_draw(GD_GraphAnalysis *analysis, GD_OverviewControls *controls, GD_Rect viewport, int selected_node,int allow_input) {
    gd_graph_overview_draw_metric(analysis,controls,viewport,selected_node,allow_input,"Link rows");
}
#endif
