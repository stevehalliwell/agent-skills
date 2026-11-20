/* Dependency only. Cocoa window/context lifecycle stays in the C99 host. */
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <OpenGL/gl3.h>
#include <CoreGraphics/CoreGraphics.h>
#include <ImageIO/ImageIO.h>
#include <pthread.h>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include "gd_native_ui.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

static GLFWwindow *window_handle;
static ImGuiContext *owner_context;
static bool platform_ready,renderer_ready,frame_active,frame_failed,rendered;
static float last_clear[4];
static const char *error_text="";
static int fail(const char *message){error_text=message;return 0;}
static bool owner(){return pthread_main_np() && owner_context && owner_context==ImGui::GetCurrentContext() && window_handle==glfwGetCurrentContext();}
const char *gd_native_ui_backend(){return "GLFW/OpenGL3";}
const char *gd_native_ui_error(){return error_text;}
int gd_native_ui_check_layout(const size_t *layout,size_t count){
    const size_t expected[]={sizeof(ImGuiStoragePair),offsetof(ImGuiStoragePair,val_i),sizeof(ImGuiStyleMod),offsetof(ImGuiStyleMod,BackupInt),sizeof(ImGuiInputEvent),offsetof(ImGuiInputEvent,MousePos)};
    if(!layout || count!=sizeof(expected)/sizeof(expected[0]))return fail("Invalid C99 layout probe");
    for(size_t i=0;i<count;++i)if(layout[i]!=expected[i])return fail("C99 union layout mismatch");
    return 1;
}
int gd_native_ui_init(void *window){
    if(!pthread_main_np() || !window || owner_context || !ImGui::GetCurrentContext() || glfwGetCurrentContext()!=window)
        return fail("Expected owned current GLFW/OpenGL and ImGui contexts on main thread");
    window_handle=static_cast<GLFWwindow *>(window);owner_context=ImGui::GetCurrentContext();
    platform_ready=ImGui_ImplGlfw_InitForOpenGL(window_handle,true);
    if(platform_ready)renderer_ready=ImGui_ImplOpenGL3_Init("#version 150");
    if(!renderer_ready){gd_native_ui_shutdown();return fail("Cannot initialize GLFW/OpenGL3 ImGui backends");}
    error_text="";return 1;
}
int gd_native_ui_shutdown(){
    if(owner_context && !owner())return fail("OpenGL shutdown requires owning main thread/context");
    if(renderer_ready)ImGui_ImplOpenGL3_Shutdown();
    if(platform_ready)ImGui_ImplGlfw_Shutdown();
    renderer_ready=platform_ready=frame_active=frame_failed=rendered=false;
    window_handle=nullptr;owner_context=nullptr;return 1;
}
int gd_native_ui_begin_frame(){
    if(!renderer_ready || !owner() || frame_active || frame_failed){fail("OpenGL frame requires healthy owning contexts");return -1;}
    int width,height;glfwGetFramebufferSize(window_handle,&width,&height);
    if(width<1 || height<1 || glfwGetWindowAttrib(window_handle,GLFW_ICONIFIED))return 0;
    ImGui_ImplOpenGL3_NewFrame();ImGui_ImplGlfw_NewFrame();ImGui::NewFrame();frame_active=true;return 1;
}
int gd_native_ui_end_frame(float r,float g,float b,float a){
    if(!renderer_ready || !owner() || !frame_active || frame_failed)return fail("OpenGL rendering requires an active owned frame");
    if(!std::isfinite(r) || !std::isfinite(g) || !std::isfinite(b) || !std::isfinite(a)){frame_failed=true;return fail("Native clear color must be finite");}
    ImGui::Render();frame_active=false;
    glClearColor(r,g,b,a);glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    if(glGetError()!=GL_NO_ERROR){frame_failed=true;return fail("Native OpenGL frame failed; do not retry");}
    glfwSwapBuffers(window_handle);
    last_clear[0]=r;last_clear[1]=g;last_clear[2]=b;last_clear[3]=a;rendered=true;return 1;
}
static bool write_png(const char *path,const unsigned char *pixels,int width,int height){
    CFStringRef name=CFStringCreateWithCString(nullptr,path,kCFStringEncodingUTF8);
    if(!name)return false;
    CFURLRef url=CFURLCreateWithFileSystemPath(nullptr,name,kCFURLPOSIXPathStyle,false);
    CFRelease(name);if(!url)return false;
    size_t stride=(size_t)width*4;
    CGDataProviderRef data=CGDataProviderCreateWithData(nullptr,pixels,stride*height,nullptr);
    CGColorSpaceRef color=CGColorSpaceCreateDeviceRGB();
    CGImageRef image=data && color ? CGImageCreate(width,height,8,32,stride,color,kCGImageAlphaNoneSkipLast|kCGBitmapByteOrder32Big,data,nullptr,false,kCGRenderingIntentDefault):nullptr;
    CGImageDestinationRef destination=image ? CGImageDestinationCreateWithURL(url,CFSTR("public.png"),1,nullptr):nullptr;
    bool ok=false;
    if(destination){CGImageDestinationAddImage(destination,image,nullptr);ok=CGImageDestinationFinalize(destination);CFRelease(destination);}
    if(image)CGImageRelease(image);if(color)CGColorSpaceRelease(color);if(data)CGDataProviderRelease(data);
    CFRelease(url);return ok;
}
int gd_native_ui_capture_png(const char *path){
    if(!renderer_ready || !owner() || frame_active || frame_failed || !rendered)return fail("PNG capture requires a completed owned OpenGL frame");
    ImDrawData *draw=ImGui::GetDrawData();
    if(!draw || !path || !*path || std::strlen(path)>=4096)return fail("PNG capture requires draw data and bounded UTF-8 path");
    int width=(int)(draw->DisplaySize.x*draw->FramebufferScale.x),height=(int)(draw->DisplaySize.y*draw->FramebufferScale.y);
    if(width<1 || height<1 || width>16777216/height)return fail("Native PNG capture exceeds 16777216 pixel limit");
    size_t stride=(size_t)width*4;
    unsigned char *pixels=static_cast<unsigned char *>(std::malloc(stride*height));
    if(!pixels)return fail("Cannot allocate bounded OpenGL PNG readback");
    GLuint framebuffer=0,color=0;GLint draw_binding,read_binding,render_binding,pack_alignment;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING,&draw_binding);glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING,&read_binding);
    glGetIntegerv(GL_RENDERBUFFER_BINDING,&render_binding);glGetIntegerv(GL_PACK_ALIGNMENT,&pack_alignment);
    glGenFramebuffers(1,&framebuffer);glGenRenderbuffers(1,&color);
    bool ready=false;
    if(framebuffer && color){
        glBindFramebuffer(GL_FRAMEBUFFER,framebuffer);glBindRenderbuffer(GL_RENDERBUFFER,color);
        glRenderbufferStorage(GL_RENDERBUFFER,GL_RGBA8,width,height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_RENDERBUFFER,color);
        ready=glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE;
        if(ready){
            glClearColor(last_clear[0],last_clear[1],last_clear[2],last_clear[3]);glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(draw);
            glPixelStorei(GL_PACK_ALIGNMENT,1);glReadPixels(0,0,width,height,GL_RGBA,GL_UNSIGNED_BYTE,pixels);
        }
    }
    ready=glGetError()==GL_NO_ERROR && ready;
    glPixelStorei(GL_PACK_ALIGNMENT,pack_alignment);glBindRenderbuffer(GL_RENDERBUFFER,render_binding);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER,draw_binding);glBindFramebuffer(GL_READ_FRAMEBUFFER,read_binding);
    if(color)glDeleteRenderbuffers(1,&color);if(framebuffer)glDeleteFramebuffers(1,&framebuffer);
    if(ready)for(int y=0;y<height/2;++y){
        unsigned char *top=pixels+stride*y,*bottom=pixels+stride*(height-1-y);
        for(size_t x=0;x<stride;++x){unsigned char byte=top[x];top[x]=bottom[x];bottom[x]=byte;}
    }
    bool ok=ready && write_png(path,pixels,width,height);std::free(pixels);
    return ok ? 1:fail("Cannot read or encode native OpenGL PNG");
}
