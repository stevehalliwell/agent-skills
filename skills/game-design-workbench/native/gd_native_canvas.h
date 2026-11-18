#ifndef GD_NATIVE_CANVAS_H
#define GD_NATIVE_CANVAS_H
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_NO_EXPORT
#include "cimgui.h"

/* Screen/world drawing share ImGui's font, AA and clip implementation.
   Explicit camera is pan + uniform zoom, matching Workbench's current cameras.
   Zero-initialize canvas. Canvas/draw data are borrowed for current frame only.
   Clip rectangles are screen pixels, independent of camera. Clip nesting <=16.
   Text bakes at fixed 4x world size (graph's maximum zoom), at most 256 pixels.
   Camera changes neither world metrics nor font-cache keys. Text size <=64
   world units; larger sizes clamp consistently in measurement and drawing. */
enum { GD_CANVAS_CLIP_LIMIT=16 };
typedef struct { ImVec2_c target, offset; float zoom; } GD_CanvasCamera;
typedef struct {
    ImDrawList *draw;
    GD_CanvasCamera camera;
    int clip_depth;
} GD_NativeCanvas;
ImVec2_c gd_canvas_to_screen(GD_CanvasCamera camera,ImVec2_c point);
ImVec2_c gd_canvas_to_world(GD_CanvasCamera camera,ImVec2_c point);
int gd_canvas_begin(GD_NativeCanvas *canvas,GD_CanvasCamera camera);
int gd_canvas_clip(GD_NativeCanvas *canvas,ImVec2_c first,ImVec2_c last);
int gd_canvas_unclip(GD_NativeCanvas *canvas);
int gd_canvas_end(GD_NativeCanvas *canvas);
void gd_canvas_line(GD_NativeCanvas *canvas,ImVec2_c first,ImVec2_c last,ImU32 color,float width);
void gd_canvas_rect(GD_NativeCanvas *canvas,ImVec2_c first,ImVec2_c last,ImU32 color,float rounding,float outline_width);
void gd_canvas_circle(GD_NativeCanvas *canvas,ImVec2_c center,float radius,ImU32 color,float outline_width);
void gd_canvas_curve(GD_NativeCanvas *canvas,ImVec2_c first,ImVec2_c first_control,ImVec2_c last_control,ImVec2_c last,ImU32 color,float width);
ImVec2_c gd_canvas_measure(const char *text,float size);
void gd_canvas_text(GD_NativeCanvas *canvas,ImVec2_c position,const char *text,float size,ImU32 color);
#endif
