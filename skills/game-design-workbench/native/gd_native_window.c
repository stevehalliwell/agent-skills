#include "gd_native_window.h"
#include "gd_native_ui.h"
#include "gd_native_window_imgui.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>

enum { GD_WINDOW_MESSAGE_LIMIT=256, GD_WINDOW_DIMENSION_LIMIT=16384 };
static const wchar_t window_class[]=L"GameDesignWorkbenchNativeWindow";
/* One backend/context per process is also the graphics adapter's contract. */
static GD_NativeWindow *active_window;
static int window_error(GD_NativeWindow *window,const char *message) {
    if(window)snprintf(window->error,sizeof(window->error),"%s",message);
    return 0;
}
static void window_frame_failed(GD_NativeWindow *window,const char *message) {
    window->failed=1;
    /* A retained failed host must not keep routing other windows' mouse input
       into its now-disabled ImGui backend. Release only this host's capture. */
    if(GetCapture()==(HWND)window->handle)ReleaseCapture();
    window_error(window,message);
}
static int window_current(const GD_NativeWindow *window) {
    return window && active_window==window && window->thread==GetCurrentThreadId() &&
        window->context && igGetCurrentContext()==window->context;
}
static LRESULT CALLBACK window_message(HWND handle,UINT message,WPARAM wparam,LPARAM lparam) {
    GD_NativeWindow *window=(GD_NativeWindow *)GetWindowLongPtrW(handle,GWLP_USERDATA);
    if(message==WM_NCCREATE) {
        window=(GD_NativeWindow *)((CREATESTRUCTW *)lparam)->lpCreateParams;
        SetWindowLongPtrW(handle,GWLP_USERDATA,(LONG_PTR)window);
    }
    if(window) {
        if(message==WM_CLOSE){window->close_requested=1;return 0;}
        if(message==WM_GETMINMAXINFO) {
            RECT area={0,0,window->minimum_width,window->minimum_height};
            if(AdjustWindowRectEx(&area,WS_OVERLAPPEDWINDOW,FALSE,(DWORD)GetWindowLongPtrW(handle,GWL_EXSTYLE))) {
                MINMAXINFO *limits=(MINMAXINFO *)lparam;
                limits->ptMinTrackSize.x=area.right-area.left;limits->ptMinTrackSize.y=area.bottom-area.top;
            }
            return 0;
        }
        if(message==WM_ERASEBKGND)return 1;
        if(window->context && !window->failed) {
            intptr_t handled=gd_native_ui_message(handle,message,wparam,lparam);
            if(handled)return (LRESULT)handled;
        }
    }
    return DefWindowProcW(handle,message,wparam,lparam);
}
int gd_native_window_open(GD_NativeWindow *window,const char *title,int width,int height,
    int minimum_width,int minimum_height,GD_WindowVisibility visibility) {
    WNDCLASSW definition={0},existing={0};RECT area={0,0,width,height};wchar_t wide[1024];
    HINSTANCE instance=GetModuleHandleW(NULL);LARGE_INTEGER ticks,frequency;
    DWORD extended=visibility==GD_WINDOW_HIDDEN ? WS_EX_TOOLWINDOW|WS_EX_NOACTIVATE:0;
    if(!window)return 0;
    if(active_window)return window_error(window,"A native window is already open; existing host retained.");
    if(igGetCurrentContext())return window_error(window,"Close existing ImGui context before opening native host.");
    if(!title || width<1 || height<1 || width>GD_WINDOW_DIMENSION_LIMIT || height>GD_WINDOW_DIMENSION_LIMIT ||
        minimum_width<1 || minimum_height<1 || minimum_width>width || minimum_height>height ||
        (visibility!=GD_WINDOW_HIDDEN && visibility!=GD_WINDOW_VISIBLE))return window_error(window,"Invalid native window dimensions/visibility.");
    if(!MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,title,-1,wide,1024))return window_error(window,"Window title must be bounded valid UTF-8.");
    if(!QueryPerformanceFrequency(&frequency) || frequency.QuadPart<=0 || !QueryPerformanceCounter(&ticks))return window_error(window,"Cannot initialize monotonic window clock.");
    memset(window,0,sizeof(*window));window->minimum_width=minimum_width;window->minimum_height=minimum_height;
    window->thread=GetCurrentThreadId();window->start_ticks=ticks.QuadPart;window->ticks_per_second=frequency.QuadPart;
    definition.lpfnWndProc=window_message;definition.hInstance=instance;definition.lpszClassName=window_class;
    definition.hCursor=LoadCursor(NULL,IDC_ARROW);
    if(!GetClassInfoW(instance,window_class,&existing)) {
        if(!RegisterClassW(&definition))return window_error(window,"Cannot register native window class.");
    } else if(existing.lpfnWndProc!=window_message)return window_error(window,"Native window class belongs to another implementation.");
    if(!AdjustWindowRectEx(&area,WS_OVERLAPPEDWINDOW,FALSE,extended))return window_error(window,"Cannot calculate native client rectangle.");
    window->handle=CreateWindowExW(extended,window_class,wide,WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,
        area.right-area.left,area.bottom-area.top,NULL,NULL,instance,window);
    if(!window->handle)return window_error(window,"Cannot create native window.");
    active_window=window;
    if(!window_imgui_create(window,visibility)) {
        window_error(window,"Cannot create matching native ImGui context/font.");gd_native_window_close(window);return 0;
    }
    if(!gd_native_ui_init(window->handle)) {
        window_error(window,gd_native_ui_error());gd_native_window_close(window);return 0;
    }
    window->backend_ready=1;
    if(visibility==GD_WINDOW_VISIBLE)ShowWindow(window->handle,SW_SHOWNOACTIVATE);
    return 1;
}
int gd_native_window_poll(GD_NativeWindow *window) {
    MSG message;
    if(!window_current(window))return window_error(window,"Native poll requires owning thread/context.");
    for(int count=0;count<GD_WINDOW_MESSAGE_LIMIT && PeekMessageW(&message,(HWND)window->handle,0,0,PM_REMOVE);++count) {
        if(message.message==WM_QUIT)window->close_requested=1;
        else {TranslateMessage(&message);DispatchMessageW(&message);}
    }
    return 1;
}
void gd_native_window_size(const GD_NativeWindow *window,int *width,int *height) {
    RECT area={0};
    if(window && window->handle)GetClientRect((HWND)window->handle,&area);
    *width=area.right-area.left;*height=area.bottom-area.top;
}
int gd_native_window_begin(GD_NativeWindow *window) {
    int result;
    if(!gd_native_window_poll(window))return -1;
    /* Failed graphics must not strand OS close messages or grow ImGui input
       queues. Pump ownership/close above, but never retry failed GPU frame. */
    if(window->failed){MsgWaitForMultipleObjects(0,NULL,FALSE,16,QS_ALLINPUT);return -1;}
    if(window->frame_active){window_error(window,"Native frame already active.");return -1;}
    result=gd_native_ui_begin_frame();
    if(result<0)window_frame_failed(window,gd_native_ui_error());
    window->frame_active=result==1;
    if(!result)MsgWaitForMultipleObjects(0,NULL,FALSE,16,QS_ALLINPUT);
    return result;
}
int gd_native_window_end(GD_NativeWindow *window,float red,float green,float blue,float alpha) {
    if(!window_current(window) || !window->frame_active || window->failed)return window_error(window,"No active owned native frame.");
    if(!gd_native_ui_end_frame(red,green,blue,alpha)) {
        window_frame_failed(window,gd_native_ui_error());return 0;
    }
    window->frame_active=0;return 1;
}
int gd_native_window_close(GD_NativeWindow *window) {
    if(!window)return 0;
    if(!window->handle && !window->context)return 1;
    if(active_window!=window || window->thread!=GetCurrentThreadId())return window_error(window,"Native close requires owning window thread.");
    if(window->context) {
        if(igGetCurrentContext()!=window->context)return window_error(window,"Restore owning ImGui context before native close.");
        if(window->backend_ready && !gd_native_ui_shutdown())return window_error(window,gd_native_ui_error());
        window->backend_ready=0;
        igDestroyContext(window->context);window->context=NULL;
    }
    if(window->handle && !DestroyWindow((HWND)window->handle))return window_error(window,"Cannot destroy owned native window.");
    window->handle=NULL;window->frame_active=0;active_window=NULL;
    /* Class stays registered until process exit; repeated open reuses same proc. */
    return 1;
}
double gd_native_window_seconds(const GD_NativeWindow *window) {
    LARGE_INTEGER now;
    if(!window || window->ticks_per_second<=0 || !QueryPerformanceCounter(&now))return 0;
    return (double)(now.QuadPart-window->start_ticks)/(double)window->ticks_per_second;
}
