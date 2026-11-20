#include "gd_native_canvas.h"
#include <float.h>
#include <math.h>

enum { GD_CANVAS_FONT_PIXEL_LIMIT=256, GD_CANVAS_FONT_RASTER_DENSITY=4 };

ImVec2_c gd_canvas_to_screen(GD_CanvasCamera camera,ImVec2_c point) {
    return (ImVec2_c){(point.x-camera.target.x)*camera.zoom+camera.offset.x,(point.y-camera.target.y)*camera.zoom+camera.offset.y};
}
ImVec2_c gd_canvas_to_world(GD_CanvasCamera camera,ImVec2_c point) {
    return (ImVec2_c){(point.x-camera.offset.x)/camera.zoom+camera.target.x,(point.y-camera.offset.y)/camera.zoom+camera.target.y};
}
int gd_canvas_begin(GD_NativeCanvas *canvas,GD_CanvasCamera camera) {
    if(!canvas || canvas->draw || !igGetCurrentContext() || !igGetCurrentContext()->WithinFrameScope ||
        !isfinite(camera.zoom) || camera.zoom<=0 || !isfinite(camera.target.x) || !isfinite(camera.target.y) ||
        !isfinite(camera.offset.x) || !isfinite(camera.offset.y))return 0;
    canvas->draw=igGetBackgroundDrawList_Nil();canvas->camera=camera;canvas->clip_depth=0;return 1;
}
int gd_canvas_clip(GD_NativeCanvas *canvas,ImVec2_c first,ImVec2_c last) {
    if(!canvas || !canvas->draw || canvas->clip_depth>=GD_CANVAS_CLIP_LIMIT ||
        !isfinite(first.x) || !isfinite(first.y) || !isfinite(last.x) || !isfinite(last.y) || first.x>last.x || first.y>last.y)return 0;
    ImDrawList_PushClipRect(canvas->draw,first,last,true);++canvas->clip_depth;return 1;
}
int gd_canvas_unclip(GD_NativeCanvas *canvas) {
    if(!canvas || !canvas->draw || !canvas->clip_depth)return 0;
    ImDrawList_PopClipRect(canvas->draw);--canvas->clip_depth;return 1;
}
int gd_canvas_end(GD_NativeCanvas *canvas) {
    if(!canvas || !canvas->draw || canvas->clip_depth)return 0;
    canvas->draw=NULL;return 1;
}
void gd_canvas_line(GD_NativeCanvas *canvas,ImVec2_c first,ImVec2_c last,ImU32 color,float width) {
    ImDrawList_AddLine(canvas->draw,gd_canvas_to_screen(canvas->camera,first),gd_canvas_to_screen(canvas->camera,last),color,width*canvas->camera.zoom,0);
}
void gd_canvas_rect(GD_NativeCanvas *canvas,ImVec2_c first,ImVec2_c last,ImU32 color,float rounding,float outline_width) {
    first=gd_canvas_to_screen(canvas->camera,first);last=gd_canvas_to_screen(canvas->camera,last);
    if(outline_width>0)ImDrawList_AddRect(canvas->draw,first,last,color,rounding*canvas->camera.zoom,outline_width*canvas->camera.zoom,0);
    else ImDrawList_AddRectFilled(canvas->draw,first,last,color,rounding*canvas->camera.zoom,0);
}
void gd_canvas_circle(GD_NativeCanvas *canvas,ImVec2_c center,float radius,ImU32 color,float outline_width) {
    center=gd_canvas_to_screen(canvas->camera,center);radius*=canvas->camera.zoom;
    if(outline_width>0)ImDrawList_AddCircle(canvas->draw,center,radius,color,0,outline_width*canvas->camera.zoom,0);
    else ImDrawList_AddCircleFilled(canvas->draw,center,radius,color,0);
}
void gd_canvas_curve(GD_NativeCanvas *canvas,ImVec2_c first,ImVec2_c first_control,ImVec2_c last_control,ImVec2_c last,ImU32 color,float width) {
    ImDrawList_AddBezierCubic(canvas->draw,gd_canvas_to_screen(canvas->camera,first),gd_canvas_to_screen(canvas->camera,first_control),
        gd_canvas_to_screen(canvas->camera,last_control),gd_canvas_to_screen(canvas->camera,last),color,width*canvas->camera.zoom,0,0);
}
static float font_pixels(float size) {
    /* Bake at maximum supported graph zoom, not each current camera zoom.
       World metrics and cache keys must not change when camera changes. */
    float pixels=size*GD_CANVAS_FONT_RASTER_DENSITY;
    if(!isfinite(pixels) || pixels<GD_CANVAS_FONT_RASTER_DENSITY)return GD_CANVAS_FONT_RASTER_DENSITY;
    if(pixels>GD_CANVAS_FONT_PIXEL_LIMIT)return GD_CANVAS_FONT_PIXEL_LIMIT;
    return floorf(pixels+0.5f);
}
ImVec2_c gd_canvas_measure(const char *text,float size) {
    if(!text || !igGetCurrentContext())return (ImVec2_c){0,0};
    ImGuiIO *io=igGetIO_Nil();
    ImFont *font=io->FontDefault ? io->FontDefault:io->Fonts->Fonts.Size ? io->Fonts->Fonts.Data[0]:NULL;
    if(!font)return (ImVec2_c){0,0};
    ImVec2_c extent=ImFont_CalcTextSizeA(font,font_pixels(size),FLT_MAX,0,text,NULL,NULL);
    extent.x/=GD_CANVAS_FONT_RASTER_DENSITY;extent.y/=GD_CANVAS_FONT_RASTER_DENSITY;return extent;
}
void gd_canvas_text(GD_NativeCanvas *canvas,ImVec2_c position,const char *text,float size,ImU32 color) {
    ImDrawList *draw=canvas->draw;int first=draw->VtxBuffer.Size;
    ImVec2_c low=gd_canvas_to_world(canvas->camera,ImDrawList_GetClipRectMin(draw));
    ImVec2_c high=gd_canvas_to_world(canvas->camera,ImDrawList_GetClipRectMax(draw));
    if(!text || !*text || !(color & 0xff000000u))return;
    position.x*=GD_CANVAS_FONT_RASTER_DENSITY;position.y*=GD_CANVAS_FONT_RASTER_DENSITY;
    /* CPU glyph culling uses scaled-world clip; GPU commands retain original
       screen clip. No pixel snapping in world space: smooth poses stay smooth. */
    ImFont_RenderText(igGetFont(),draw,font_pixels(size),position,color,
        (ImVec4_c){low.x*GD_CANVAS_FONT_RASTER_DENSITY,low.y*GD_CANVAS_FONT_RASTER_DENSITY,
                   high.x*GD_CANVAS_FONT_RASTER_DENSITY,high.y*GD_CANVAS_FONT_RASTER_DENSITY},
        text,NULL,0,ImDrawTextFlags_CpuFineClip|ImDrawFlags_TextNoPixelSnap);
    for(int index=first;index<draw->VtxBuffer.Size;++index) {
        ImVec2_c point=draw->VtxBuffer.Data[index].pos;
        point.x/=GD_CANVAS_FONT_RASTER_DENSITY;point.y/=GD_CANVAS_FONT_RASTER_DENSITY;
        draw->VtxBuffer.Data[index].pos=gd_canvas_to_screen(canvas->camera,point);
    }
}
