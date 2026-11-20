/* C99 GLFW window/event owner. All Cocoa operations stay on main thread. */
#include "gd_native_window.h"
#include "gd_native_ui.h"
#include "gd_native_window_imgui.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

static GD_NativeWindow *active_window;
static int window_error(GD_NativeWindow *window,const char *message) {
    if(window)snprintf(window->error,sizeof(window->error),"%s",message);return 0;
}
static int current(const GD_NativeWindow *window) {
    return pthread_main_np() && window && window==active_window && window->context==igGetCurrentContext() && window->handle==glfwGetCurrentContext();
}
int gd_native_window_open(GD_NativeWindow *window,const char *title,int width,int height,int min_width,int min_height,GD_WindowVisibility visibility) {
    if(!window || !pthread_main_np() || active_window || igGetCurrentContext())return window_error(window,"Native GLFW window requires main thread and no active host/context.");
    if(!title || !*title || width<1 || height<1 || width>16384 || height>16384 || min_width<1 || min_height<1 || min_width>width || min_height>height ||
       (visibility!=GD_WINDOW_HIDDEN && visibility!=GD_WINDOW_VISIBLE))return window_error(window,"Invalid GLFW window dimensions or visibility.");
    memset(window,0,sizeof(*window));
    glfwInitHint(GLFW_COCOA_CHDIR_RESOURCES,GLFW_FALSE);
    glfwInitHint(GLFW_COCOA_MENUBAR,GLFW_FALSE);
    if(!glfwInit())return window_error(window,"Cannot initialize GLFW Cocoa.");
    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,2);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE,GLFW_FALSE);glfwWindowHint(GLFW_FOCUS_ON_SHOW,GLFW_FALSE);
    window->handle=glfwCreateWindow(width,height,title,NULL,NULL);
    if(!window->handle){glfwTerminate();return window_error(window,"Cannot create GLFW OpenGL 3.2 window.");}
    active_window=window;
    window->minimum_width=min_width;window->minimum_height=min_height;
    glfwSetWindowSizeLimits(window->handle,min_width,min_height,GLFW_DONT_CARE,GLFW_DONT_CARE);
    glfwMakeContextCurrent(window->handle);glfwSwapInterval(1);
    window->start_ticks=(int64_t)glfwGetTimerValue();window->ticks_per_second=(int64_t)glfwGetTimerFrequency();
    if(!window_imgui_create(window,visibility) || !gd_native_ui_init(window->handle)) {
        window_error(window,window->context && *gd_native_ui_error() ? gd_native_ui_error():"Cannot initialize GLFW ImGui context/font.");
        gd_native_window_close(window);return 0;
    }
    window->backend_ready=1;
    if(visibility==GD_WINDOW_VISIBLE)glfwShowWindow(window->handle);
    return 1;
}
int gd_native_window_poll(GD_NativeWindow *window) {
    if(!current(window))return window_error(window,"GLFW poll requires owning main thread/context.");
    glfwPollEvents();
    if(glfwWindowShouldClose(window->handle)){window->close_requested=1;glfwSetWindowShouldClose(window->handle,GLFW_FALSE);}
    return 1;
}
void gd_native_window_size(const GD_NativeWindow *window,int *width,int *height) {
    *width=*height=0;if(window && window->handle)glfwGetWindowSize(window->handle,width,height);
}
int gd_native_window_begin(GD_NativeWindow *window) {
    if(!gd_native_window_poll(window))return -1;
    if(window->failed){glfwWaitEventsTimeout(0.016);return -1;}
    if(window->frame_active){window_error(window,"Native frame already active.");return -1;}
    int ready=gd_native_ui_begin_frame();window->frame_active=ready==1;
    if(ready<0){window->failed=1;ImGuiIO_SetAppAcceptingEvents(igGetIO_Nil(),false);window_error(window,gd_native_ui_error());}
    if(!ready)glfwWaitEventsTimeout(0.016);
    return ready;
}
int gd_native_window_end(GD_NativeWindow *window,float red,float green,float blue,float alpha) {
    if(!current(window) || !window->frame_active || window->failed)return window_error(window,"No active owned GLFW frame.");
    if(!gd_native_ui_end_frame(red,green,blue,alpha)){window->failed=1;ImGuiIO_SetAppAcceptingEvents(igGetIO_Nil(),false);return window_error(window,gd_native_ui_error());}
    window->frame_active=0;return 1;
}
int gd_native_window_close(GD_NativeWindow *window) {
    if(!window)return 0;
    if(!window->handle)return 1;
    if(!current(window))return window_error(window,"GLFW close requires owning main thread/context.");
    if(window->backend_ready && !gd_native_ui_shutdown())return window_error(window,gd_native_ui_error());
    window->backend_ready=0;
    if(window->context)igDestroyContext(window->context);
    glfwDestroyWindow(window->handle);glfwTerminate();
    window->handle=window->context=NULL;window->frame_active=0;active_window=NULL;
    return 1;
}
double gd_native_window_seconds(const GD_NativeWindow *window) {
    if(!window || !window->handle || window->ticks_per_second<=0)return 0;
    return (double)(glfwGetTimerValue()-(uint64_t)window->start_ticks)/(double)window->ticks_per_second;
}
