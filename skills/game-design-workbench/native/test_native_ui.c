/* Default: headless C99/CJIT ABI check. --gpu additionally creates one owned,
   hidden, non-activating window for device/render/resize/shutdown checks.
   No OS input or source writes. Link matching gd-native-ui.dll beside exe. */
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_NO_EXPORT
#include "cimgui.h"
#include "gd_native_ui.h"
#include "gd_native_window.h"
#include "gd_native_canvas.h"
#include <math.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#ifndef WC_ERR_INVALID_CHARS
#define WC_ERR_INVALID_CHARS 0x00000080
#endif
#include <stddef.h>
#include <stdio.h>
#include <string.h>

int main(int argc,char **argv) {
    ImGuiContext *context=NULL;GD_NativeWindow host={0};GD_NativeCanvas canvas={0};
    GD_CanvasCamera camera={{12,-8},{24,36},2};
    wchar_t temporary[MAX_PATH]={0},png_path[512]={0};int temporary_created=0,png_attempted=0,png_verified=0;
    FILE *capture_file=NULL;
    int gpu=argc==2 && !strcmp(argv[1],"--gpu");
    if(argc!=1 && !gpu){fputs("Usage: test-native-ui [--gpu]\n",stderr);return 2;}
    const size_t layout[]={sizeof(ImGuiStoragePair),offsetof(ImGuiStoragePair,value.val_i),
        sizeof(ImGuiStyleMod),offsetof(ImGuiStyleMod,backup.BackupInt),
        sizeof(ImGuiInputEvent),offsetof(ImGuiInputEvent,event.MousePos)};
    size_t invalid[6];int checks=0,result=2;
#define CHECK(value) do { ++checks; if(!(value)) { fprintf(stderr,"Native UI ABI check failed: %s (line %d): %s; host: %s\n",#value,__LINE__,gd_native_ui_error(),host.error);goto cleanup; } } while(0)
    CHECK(!strcmp(igGetVersion(),"1.93.0"));
    CHECK(!strcmp(gd_native_ui_backend(),"Win32/D3D11"));
    CHECK(GetModuleHandleW(L"raylib.dll")==NULL);
    CHECK(gd_native_ui_shutdown());
    CHECK(!gd_native_ui_init(NULL));
    CHECK(*gd_native_ui_error());
    CHECK(gd_native_ui_begin_frame()==-1);
    CHECK(!gd_native_ui_end_frame(0,0,0,1));
    CHECK(!gd_native_ui_message(NULL,0,0,0));
    CHECK(!gd_native_ui_capture_png(NULL));
    CHECK(gd_native_ui_check_layout(layout,sizeof(layout)/sizeof(layout[0])));
    CHECK(!gd_native_ui_check_layout(NULL,6));
    CHECK(!gd_native_ui_check_layout(layout,5));
    memcpy(invalid,layout,sizeof(invalid));++invalid[3];
    CHECK(!gd_native_ui_check_layout(invalid,6));
    context=igCreateContext(NULL);CHECK(context!=NULL);
    CHECK(igGetCurrentContext()==context);
    CHECK(igDebugCheckVersionAndDataLayout("1.93.0",sizeof(ImGuiIO),sizeof(ImGuiStyle),sizeof(ImVec2_c),sizeof(ImVec4_c),sizeof(ImDrawVert),sizeof(ImDrawIdx)));
    igGetIO_Nil()->IniFilename=NULL;
    igGetStyle()->Alpha=0.75f;CHECK(igGetStyle()->Alpha==0.75f);
    igGetStyle()->WindowPadding=(ImVec2_c){11,13};CHECK(igGetStyle()->WindowPadding.x==11 && igGetStyle()->WindowPadding.y==13);
    CHECK(gd_native_ui_check_layout(layout,6));
    CHECK(!gd_native_ui_init(NULL) && igGetCurrentContext()==context);
    CHECK(gd_native_ui_shutdown() && igGetCurrentContext()==context);
    {ImVec2_c point={-17,53},screen=gd_canvas_to_screen(camera,point),roundtrip=gd_canvas_to_world(camera,screen);
     CHECK(screen.x==-34 && screen.y==158);CHECK(roundtrip.x==point.x && roundtrip.y==point.y);}
    CHECK(!gd_canvas_begin(&canvas,camera)); /* No active frame; no draw allocation. */
    if(gpu) {
        int frame;RECT client;double first;
        CHECK(!gd_native_window_open(&host,"Existing context",640,480,320,240,GD_WINDOW_HIDDEN));
        CHECK(igGetCurrentContext()==context && !host.handle);
        igDestroyContext(context);context=NULL;
        CHECK(!gd_native_window_open(&host,"Invalid dimensions",0,480,320,240,GD_WINDOW_HIDDEN));
        CHECK(!host.handle && !igGetCurrentContext());
        CHECK(gd_native_window_open(&host,"Owned C99 native host",640,480,320,240,GD_WINDOW_HIDDEN));
        CHECK(!IsWindowVisible((HWND)host.handle) && host.context==igGetCurrentContext());
        CHECK((igGetIO_Nil()->ConfigFlags&ImGuiConfigFlags_NoMouseCursorChange) && (igGetIO_Nil()->ConfigFlags&ImGuiConfigFlags_NavEnableKeyboard) && !igGetIO_Nil()->ConfigNavMoveSetMousePos);
        CHECK(!gd_native_window_open(&host,"Duplicate",640,480,320,240,GD_WINDOW_HIDDEN));
        CHECK(host.handle && host.context==igGetCurrentContext());
        CHECK(!gd_native_ui_init(host.handle)); /* Duplicate init retains live adapter. */
        first=gd_native_window_seconds(&host);CHECK(first>=0 && gd_native_window_seconds(&host)>=first);
        for(frame=0;frame<3;++frame) {
            CHECK(SetWindowPos((HWND)host.handle,NULL,0,0,640+frame*80,480+frame*40,SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOMOVE));
            if(frame==1)CHECK(PostMessageW((HWND)host.handle,WM_CLOSE,0,0)); /* Owned close request, no OS input. */
            CHECK(gd_native_window_begin(&host)==1);
            if(!frame) {
                CHECK(!gd_native_ui_capture_png(NULL));
                CHECK(igGetFont()->Sources.Size>0 && strstr(igGetFont()->Sources.Data[0]->Name,"ProggyForever")!=NULL);
            }
            CHECK(host.close_requested==(frame>=1));
            CHECK(GetClientRect((HWND)host.handle,&client));
            CHECK(igGetIO_Nil()->DisplaySize.x==client.right && igGetIO_Nil()->DisplaySize.y==client.bottom);
            CHECK(gd_canvas_begin(&canvas,camera));
            CHECK(!gd_canvas_begin(&canvas,camera)); /* Active canvas cannot lose clips. */
            CHECK(gd_canvas_clip(&canvas,(ImVec2_c){0,0},(ImVec2_c){640,480}));
            gd_canvas_rect(&canvas,(ImVec2_c){16,16},(ImVec2_c){200,140},0xff503020,8,0);
            gd_canvas_line(&canvas,(ImVec2_c){16,16},(ImVec2_c){200,140},0xff40a0ff,2);
            gd_canvas_circle(&canvas,(ImVec2_c){80,60},12,0xff40a0ff,2);
            gd_canvas_curve(&canvas,(ImVec2_c){20,80},(ImVec2_c){50,20},(ImVec2_c){90,140},(ImVec2_c){150,80},0xffc09060,2);
            {ImVec2_c extent=gd_canvas_measure("C99 / ImGui 1.93",16);CHECK(extent.x>0 && extent.y>0);}
            gd_canvas_text(&canvas,(ImVec2_c){20,20},"C99 / ImGui 1.93",16,0xffffffff);
            {ImVec2_c normal=gd_canvas_measure("C99 / ImGui 1.93",16);ImFontBaked *baked=igGetFont()->LastBaked;
             float zoom=canvas.camera.zoom;canvas.camera.zoom=0.01f;
             ImVec2_c distant=gd_canvas_measure("C99 / ImGui 1.93",16);
             CHECK(normal.x==distant.x && normal.y==distant.y);
             int first=canvas.draw->VtxBuffer.Size;
             gd_canvas_text(&canvas,(ImVec2_c){100,100},"C99 / ImGui 1.93",16,0xffffffff);
             CHECK(canvas.draw->VtxBuffer.Size>first && igGetFont()->LastBaked==baked);
             canvas.camera.zoom=zoom;}
            CHECK(!gd_canvas_end(&canvas)); /* Unbalanced clips remain recoverable. */
            CHECK(gd_canvas_unclip(&canvas) && !gd_canvas_unclip(&canvas));
            if(frame==0) {
                for(int index=0;index<GD_CANVAS_CLIP_LIMIT;++index)CHECK(gd_canvas_clip(&canvas,(ImVec2_c){0,0},(ImVec2_c){640,480}));
                CHECK(!gd_canvas_clip(&canvas,(ImVec2_c){0,0},(ImVec2_c){640,480}));
                for(int index=0;index<GD_CANVAS_CLIP_LIMIT;++index)CHECK(gd_canvas_unclip(&canvas));
            }
            CHECK(gd_canvas_end(&canvas) && !gd_canvas_end(&canvas));
            igSetNextWindowPos((ImVec2_c){432,16},ImGuiCond_Always,(ImVec2_c){0,0});
            igSetNextWindowSize((ImVec2_c){320,180},ImGuiCond_Always);
            igBegin("Native backend",NULL,ImGuiWindowFlags_NoSavedSettings);
            igTextUnformatted("C99 host and canvas / no raylib",NULL);igEnd();
            CHECK(gd_native_window_end(&host,0.08f,0.09f,0.12f,1));
            CHECK(igGetDrawData()->TotalVtxCount>0);
        }
        CHECK(!IsWindowVisible((HWND)host.handle) && GetModuleHandleW(L"raylib.dll")==NULL);
        {wchar_t directory[MAX_PATH];char utf8[2048];unsigned char header[24];
         DWORD length=GetEnvironmentVariableW(L"GD_NATIVE_UI_TEST_PNG",png_path,512);
         CHECK(length<512);
         if(!length) {
             length=GetTempPathW(MAX_PATH,directory);CHECK(length>0 && length<MAX_PATH);
             CHECK(GetTempFileNameW(directory,L"gdw",0,temporary));temporary_created=1;
             CHECK(_snwprintf(png_path,512,L"%ls-\x03a3.png",temporary)>0);
         }
         CHECK(GetFileAttributesW(png_path)==INVALID_FILE_ATTRIBUTES && GetLastError()==ERROR_FILE_NOT_FOUND);
         CHECK(WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,png_path,-1,utf8,sizeof(utf8),NULL,NULL)>0);
         png_attempted=1;CHECK(gd_native_ui_capture_png(utf8));
         capture_file=_wfopen(png_path,L"rb");CHECK(capture_file);
         CHECK(fread(header,1,sizeof(header),capture_file)==sizeof(header) && !memcmp(header,"\x89PNG\r\n\x1a\n",8));
         unsigned width=((unsigned)header[16]<<24)|((unsigned)header[17]<<16)|((unsigned)header[18]<<8)|header[19];
         unsigned height=((unsigned)header[20]<<24)|((unsigned)header[21]<<16)|((unsigned)header[22]<<8)|header[23];
         CHECK(width==(unsigned)client.right && height==(unsigned)client.bottom);
         int closed=fclose(capture_file);capture_file=NULL;CHECK(!closed);png_verified=1;
         CHECK(!gd_native_ui_capture_png(""));
         {char blocked[2100];snprintf(blocked,sizeof(blocked),"%s/cannot-create.png",utf8);CHECK(!gd_native_ui_capture_png(blocked));}
         CHECK(gd_native_window_begin(&host)==1 && gd_native_window_end(&host,0.08f,0.09f,0.12f,1));}
        CHECK(gd_native_window_close(&host) && !igGetCurrentContext() && !host.handle);
        CHECK(gd_native_window_close(&host));
        CHECK(gd_native_window_open(&host,"Reopened C99 host",640,480,320,240,GD_WINDOW_HIDDEN));
        CHECK(!host.close_requested && !host.failed && gd_native_window_begin(&host)==1);
        CHECK(!gd_native_window_end(&host,NAN,0,0,1));
        CHECK(host.failed && host.handle && host.context==igGetCurrentContext());
        CHECK(PostMessageW((HWND)host.handle,WM_CLOSE,0,0));
        CHECK(gd_native_window_begin(&host)==-1 && host.close_requested && host.handle && host.context==igGetCurrentContext());
        CHECK(gd_native_window_close(&host)); /* Failed graphics never owns model/draft or exit. */
    }
    result=0;
cleanup:
    gd_native_window_close(&host);gd_native_ui_shutdown();if(context)igDestroyContext(context);
    if(capture_file)fclose(capture_file);if(png_attempted && temporary_created)DeleteFileW(png_path);if(temporary_created)DeleteFileW(temporary);
    printf("{\"ok\":%s,\"checks\":%d,\"imgui_version\":\"1.93.0\",\"gpu\":%s,\"png_verified\":%s,\"scope\":\"%s; human rendered acceptance untested\"}\n",result ? "false":"true",checks,gpu ? "true":"false",png_verified ? "true":"false",gpu ? "owned hidden non-activating window; C99 host/canvas and native device/render/resize/shutdown/PNG":"C99/CJIT ABI and pre-init errors; no window or GPU");
#undef CHECK
    return result;
}
