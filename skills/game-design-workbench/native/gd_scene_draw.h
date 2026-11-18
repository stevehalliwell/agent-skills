#ifndef GD_SCENE_DRAW_H
#define GD_SCENE_DRAW_H
#include "gd_native_canvas.h"
#include "gd_native_window.h"
#include <math.h>

typedef ImVec2_c GD_Point;
typedef struct {float x,y,width,height;} GD_Rect;
typedef struct {unsigned char r,g,b,a;} GD_Color;
/* Workbench cameras translate and scale only. */
typedef struct {GD_Point offset,target;float zoom;} GD_Camera;
static inline GD_CanvasCamera gd_scene_camera(GD_Camera camera){return (GD_CanvasCamera){camera.target,camera.offset,camera.zoom};}
static inline GD_Point gd_world_to_screen(GD_Point p,GD_Camera c){return gd_canvas_to_screen(gd_scene_camera(c),p);}
static inline GD_Point gd_screen_to_world(GD_Point p,GD_Camera c){return gd_canvas_to_world(gd_scene_camera(c),p);}
static inline int gd_rect_hit(GD_Point p,GD_Rect r){return p.x>=r.x && p.x<=r.x+r.width && p.y>=r.y && p.y<=r.y+r.height;}
static inline GD_Rect gd_rect_overlap(GD_Rect a,GD_Rect b){
    float x=fmaxf(a.x,b.x),y=fmaxf(a.y,b.y);
    return (GD_Rect){x,y,fmaxf(0,fminf(a.x+a.width,b.x+b.width)-x),fmaxf(0,fminf(a.y+a.height,b.y+b.height)-y)};
}
static inline int gd_rect_intersects(GD_Rect a,GD_Rect b){GD_Rect r=gd_rect_overlap(a,b);return r.width>0 && r.height>0;}
static inline GD_Color gd_color_alpha(GD_Color color,float alpha){color.a=(unsigned char)(255*fminf(1,fmaxf(0,alpha)));return color;}
static inline int gd_key_pressed(ImGuiKey key){return igIsKeyPressed_Bool(key,false);}
static inline int gd_key_repeat(ImGuiKey key){return igIsKeyPressed_Bool(key,true);}
static inline int gd_mouse_pressed(ImGuiMouseButton button){return igIsMouseClicked_Bool(button,false);}
static inline float gd_mouse_wheel(void){ImGuiIO *io=igGetIO_Nil();return io->MouseWheel;}
static inline GD_Point gd_mouse_position(void){return igGetIO_Nil()->MousePos;}
static inline GD_Point gd_mouse_delta(void){return igGetIO_Nil()->MouseDelta;}
static inline float gd_frame_seconds(void){return igGetIO_Nil()->DeltaTime;}
static inline int gd_scene_width(void){return igGetCurrentContext() ? (int)igGetIO_Nil()->DisplaySize.x:0;}
static inline int gd_scene_height(void){return igGetCurrentContext() ? (int)igGetIO_Nil()->DisplaySize.y:0;}
/* One immediate-mode scene per active frontend frame. ImGui controls draw
   above the scene; CPU geometry checks may measure without drawing. */
/* Attach/detach borrowed ImGui frame. Native host wraps these for GPU use. */
int gd_scene_attach(void);
int gd_scene_detach(void);
int gd_scene_begin(GD_NativeWindow *window);
int gd_scene_end(GD_NativeWindow *window,GD_Color clear);
int gd_scene_close(GD_NativeWindow *window);
struct GD_Model;
void gd_scene_error(struct GD_Model *model,const char *code,const GD_NativeWindow *window);
void gd_scene_set_camera(GD_Camera camera);
void gd_scene_screen(void);
void gd_scene_clip(int x,int y,int width,int height);
void gd_scene_unclip(void);
const char *gd_scene_format(const char *format,...);
int gd_scene_text_width(const char *text,int size);
GD_Point gd_scene_text_extent(const char *text,float size);
void gd_scene_text_at(const char *text,GD_Point p,float size,GD_Color color);
void gd_scene_rect(GD_Rect area,GD_Color color);
void gd_scene_outline(GD_Rect area,float width,GD_Color color);
void gd_scene_round_rect(GD_Rect area,float roundness,GD_Color color);
void gd_scene_round_outline(GD_Rect area,float roundness,float width,GD_Color color);
void gd_scene_rect_pixels(int x,int y,int width,int height,GD_Color color);
void gd_scene_outline_pixels(int x,int y,int width,int height,GD_Color color);
void gd_scene_text(const char *text,int x,int y,int size,GD_Color color);
void gd_scene_gradient(int x,int y,int width,int height,GD_Color low,GD_Color high);
void gd_scene_line(GD_Point a,GD_Point b,float width,GD_Color color);
void gd_scene_line_pixels(int x,int y,int end_x,int end_y,GD_Color color);
void gd_scene_circle(GD_Point center,float radius,GD_Color color);
void gd_scene_circle_pixels(int x,int y,float radius,GD_Color color);
void gd_scene_ring_pixels(int x,int y,float radius,GD_Color color);
void gd_scene_triangle(GD_Point a,GD_Point b,GD_Point c,GD_Color color);
void gd_scene_triangle_outline(GD_Point a,GD_Point b,GD_Point c,GD_Color color);
void gd_scene_quadratic(GD_Point a,GD_Point control,GD_Point b,float width,GD_Color color);
#endif
