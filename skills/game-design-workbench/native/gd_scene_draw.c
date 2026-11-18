#include "gd_scene_draw.h"
#include "gd.h"
#include "gd_native_ui.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static GD_NativeCanvas scene;
static GD_NativeWindow *scene_owner;
static const GD_CanvasCamera screen={{0,0},{0,0},1};
static ImU32 pixel(GD_Color c){return (ImU32)c.r|((ImU32)c.g<<8)|((ImU32)c.b<<16)|((ImU32)c.a<<24);}
int gd_scene_attach(void){return gd_canvas_begin(&scene,screen);}
int gd_scene_detach(void){
    if(gd_canvas_end(&scene))return 1;
    while(scene.clip_depth>0)gd_canvas_unclip(&scene);
    scene=(GD_NativeCanvas){0};return 0;
}
int gd_scene_begin(GD_NativeWindow *window) {
    int ready=gd_native_window_begin(window);
    if(ready!=1)return ready;
    if(scene_owner || !gd_scene_attach()) {
        window->failed=1;snprintf(window->error,sizeof(window->error),"Cannot attach Workbench scene to owned ImGui frame.");return -1;
    }
    scene_owner=window;return 1;
}
int gd_scene_end(GD_NativeWindow *window,GD_Color c) {
    if(scene_owner!=window || !gd_scene_detach()) {
        window->failed=1;snprintf(window->error,sizeof(window->error),"Unbalanced Workbench scene frame or clip.");return 0;
    }
    scene_owner=NULL;return gd_native_window_end(window,c.r/255.0f,c.g/255.0f,c.b/255.0f,c.a/255.0f);
}
int gd_scene_close(GD_NativeWindow *window) {
    if(scene_owner==window){scene=(GD_NativeCanvas){0};scene_owner=NULL;}
    return gd_native_window_close(window);
}
void gd_scene_error(GD_Model *model,const char *code,const GD_NativeWindow *window) {
    /* The model reports after window storage has gone out of scope. */
    size_t bytes=strlen(window->error)+1;const char *message="Native window or frame failed";
    if(bytes>1 && bytes<=GD_TEXT_CAPACITY-model->text_used){
        char *at=model->text+model->text_used;memcpy(at,window->error,bytes);model->text_used+=bytes;message=at;
    }
    gd_error(model,code,message,NULL,0);
}
int gd_save_screenshot(const char *path){return gd_native_ui_capture_png(path);}
void gd_scene_set_camera(GD_Camera camera){scene.camera=gd_scene_camera(camera);}
void gd_scene_screen(void){scene.camera=screen;}
void gd_scene_clip(int x,int y,int w,int h){if(scene.draw)gd_canvas_clip(&scene,(GD_Point){x,y},(GD_Point){x+fmaxf(0,w),y+fmaxf(0,h)});}
void gd_scene_unclip(void){if(scene.draw)gd_canvas_unclip(&scene);}
const char *gd_scene_format(const char *format,...) {
    static char text[8][4096];static unsigned index;va_list args;
    char *at=text[index++%8];va_start(args,format);vsnprintf(at,sizeof(text[0]),format,args);va_end(args);return at;
}
GD_Point gd_scene_text_extent(const char *text,float size) {
    if(igGetCurrentContext()) {
        ImGuiIO *io=igGetIO_Nil();
        if(!io->Fonts->Fonts.Size)ImFontAtlas_AddFontDefaultVector(io->Fonts,NULL);
        return gd_canvas_measure(text,size);
    }
    /* CPU-only model/IPC checks without any UI context do not draw. Use a
       conservative width until a real font is available in the frontend. */
    return (GD_Point){text ? strlen(text)*size*0.625f:0,size};
}
int gd_scene_text_width(const char *text,int size){return (int)ceilf(gd_scene_text_extent(text,(float)size).x);}
void gd_scene_text_at(const char *text,GD_Point p,float size,GD_Color c){if(scene.draw)gd_canvas_text(&scene,p,text,size,pixel(c));}
void gd_scene_text(const char *text,int x,int y,int size,GD_Color c){gd_scene_text_at(text,(GD_Point){x,y},(float)size,c);}
static void box(GD_Rect r,float rounding,float width,GD_Color c){if(scene.draw)gd_canvas_rect(&scene,(GD_Point){r.x,r.y},(GD_Point){r.x+r.width,r.y+r.height},pixel(c),rounding,width);}
void gd_scene_rect(GD_Rect r,GD_Color c){box(r,0,0,c);}
void gd_scene_outline(GD_Rect r,float w,GD_Color c){if(w>0)box(r,0,w,c);}
void gd_scene_round_rect(GD_Rect r,float radius,GD_Color c){box(r,fminf(r.width,r.height)*radius/2,0,c);}
void gd_scene_round_outline(GD_Rect r,float radius,float w,GD_Color c){if(w>0)box(r,fminf(r.width,r.height)*radius/2,w,c);}
void gd_scene_rect_pixels(int x,int y,int w,int h,GD_Color c){gd_scene_rect((GD_Rect){x,y,w,h},c);}
void gd_scene_outline_pixels(int x,int y,int w,int h,GD_Color c){gd_scene_outline((GD_Rect){x,y,w,h},1,c);}
void gd_scene_gradient(int x,int y,int w,int h,GD_Color low,GD_Color high){
    if(scene.draw)ImDrawList_AddRectFilledMultiColor(scene.draw,gd_canvas_to_screen(scene.camera,(GD_Point){x,y}),gd_canvas_to_screen(scene.camera,(GD_Point){x+w,y+h}),pixel(low),pixel(high),pixel(high),pixel(low));
}
void gd_scene_line(GD_Point a,GD_Point b,float w,GD_Color c){if(scene.draw)gd_canvas_line(&scene,a,b,pixel(c),w);}
void gd_scene_line_pixels(int x,int y,int end_x,int end_y,GD_Color c){gd_scene_line((GD_Point){x,y},(GD_Point){end_x,end_y},1,c);}
void gd_scene_circle(GD_Point p,float radius,GD_Color c){if(scene.draw)gd_canvas_circle(&scene,p,radius,pixel(c),0);}
void gd_scene_circle_pixels(int x,int y,float radius,GD_Color c){gd_scene_circle((GD_Point){x,y},radius,c);}
void gd_scene_ring_pixels(int x,int y,float radius,GD_Color c){if(scene.draw)gd_canvas_circle(&scene,(GD_Point){x,y},radius,pixel(c),1);}
void gd_scene_triangle(GD_Point a,GD_Point b,GD_Point c,GD_Color color){
    if(!scene.draw)return;
    if((b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x)<0){GD_Point swap=b;b=c;c=swap;}
    ImDrawList_AddTriangleFilled(scene.draw,gd_canvas_to_screen(scene.camera,a),gd_canvas_to_screen(scene.camera,b),gd_canvas_to_screen(scene.camera,c),pixel(color));
}
void gd_scene_triangle_outline(GD_Point a,GD_Point b,GD_Point c,GD_Color color){
    if(scene.draw)ImDrawList_AddTriangle(scene.draw,gd_canvas_to_screen(scene.camera,a),gd_canvas_to_screen(scene.camera,b),gd_canvas_to_screen(scene.camera,c),pixel(color),scene.camera.zoom,0);
}
void gd_scene_quadratic(GD_Point a,GD_Point control,GD_Point b,float w,GD_Color c){
    if(scene.draw)gd_canvas_curve(&scene,a,(GD_Point){(a.x+2*control.x)/3,(a.y+2*control.y)/3},(GD_Point){(b.x+2*control.x)/3,(b.y+2*control.y)/3},b,pixel(c),w);
}
