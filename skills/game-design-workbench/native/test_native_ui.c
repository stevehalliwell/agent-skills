/* Default: headless C99/CJIT ABI check. --gpu additionally creates one owned,
   hidden, non-activating window for device/render/resize/shutdown checks.
   No OS input or source writes. Link matching gd-native-ui.dll beside exe. */
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_NO_EXPORT
#include "cimgui.h"
#include "gd_native_ui.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static LRESULT CALLBACK native_check_message(HWND window,UINT message,WPARAM wparam,LPARAM lparam) {
    intptr_t handled=gd_native_ui_message(window,message,wparam,lparam);
    return handled ? (LRESULT)handled:DefWindowProcW(window,message,wparam,lparam);
}
int main(int argc,char **argv) {
    ImGuiContext *context=NULL;HWND window=NULL;ATOM registered=0;HINSTANCE instance=GetModuleHandleW(NULL);
    const wchar_t *class_name=L"GameDesignWorkbenchNativeUiCheck";int gpu=argc==2 && !strcmp(argv[1],"--gpu");
    if(argc!=1 && !gpu){fputs("Usage: test-native-ui [--gpu]\n",stderr);return 2;}
    const size_t layout[]={sizeof(ImGuiStoragePair),offsetof(ImGuiStoragePair,value.val_i),
        sizeof(ImGuiStyleMod),offsetof(ImGuiStyleMod,backup.BackupInt),
        sizeof(ImGuiInputEvent),offsetof(ImGuiInputEvent,event.MousePos)};
    size_t invalid[6];int checks=0,result=2;
#define CHECK(value) do { ++checks; if(!(value)) { fprintf(stderr,"Native UI ABI check failed: %s (line %d)\n",#value,__LINE__);goto cleanup; } } while(0)
    CHECK(!strcmp(igGetVersion(),"1.93.0"));
    CHECK(!strcmp(gd_native_ui_backend(),"Win32/D3D11"));
    CHECK(GetModuleHandleW(L"raylib.dll")==NULL);
    CHECK(gd_native_ui_shutdown());
    CHECK(!gd_native_ui_init(NULL));
    CHECK(*gd_native_ui_error());
    CHECK(gd_native_ui_begin_frame()==-1);
    CHECK(!gd_native_ui_end_frame(0,0,0,1));
    CHECK(!gd_native_ui_message(NULL,0,0,0));
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
    if(gpu) {
        WNDCLASSW definition={0};int frame;MSG message;RECT client;
        definition.lpfnWndProc=native_check_message;definition.hInstance=instance;definition.lpszClassName=class_name;
        registered=RegisterClassW(&definition);CHECK(registered!=0);
        window=CreateWindowExW(WS_EX_TOOLWINDOW|WS_EX_NOACTIVATE,class_name,L"Owned native backend check",WS_OVERLAPPEDWINDOW,0,0,640,480,NULL,NULL,instance,NULL);
        CHECK(window!=NULL && !IsWindowVisible(window));
        CHECK(gd_native_ui_init(window));
        CHECK(!gd_native_ui_init(window)); /* Duplicate init must retain live adapter. */
        for(frame=0;frame<3;++frame) {
            CHECK(SetWindowPos(window,NULL,0,0,640+frame*80,480+frame*40,SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOMOVE));
            for(int message_count=0;message_count<256 && PeekMessageW(&message,NULL,0,0,PM_REMOVE);++message_count){TranslateMessage(&message);DispatchMessageW(&message);}
            CHECK(gd_native_ui_begin_frame()==1);
            CHECK(GetClientRect(window,&client));
            CHECK(igGetIO_Nil()->DisplaySize.x==client.right && igGetIO_Nil()->DisplaySize.y==client.bottom);
            igSetNextWindowPos((ImVec2_c){16,16},ImGuiCond_Always,(ImVec2_c){0,0});
            igSetNextWindowSize((ImVec2_c){320,180},ImGuiCond_Always);
            igBegin("Native backend",NULL,ImGuiWindowFlags_NoSavedSettings);
            igTextUnformatted("C99 application / ImGui 1.93 / no raylib",NULL);
            ImDrawList_AddLine(igGetWindowDrawList(),(ImVec2_c){32,100},(ImVec2_c){280,100},0xff40a0ff,2,0);
            igEnd();
            CHECK(gd_native_ui_end_frame(0.08f,0.09f,0.12f,1));
            CHECK(igGetDrawData()->TotalVtxCount>0);
        }
        CHECK(!IsWindowVisible(window) && GetModuleHandleW(L"raylib.dll")==NULL);
        CHECK(gd_native_ui_shutdown());
        CHECK(!igGetIO_Nil()->BackendRendererUserData && !igGetIO_Nil()->BackendPlatformUserData);
        CHECK(gd_native_ui_init(window) && gd_native_ui_begin_frame()==1);
        CHECK(gd_native_ui_end_frame(0.08f,0.09f,0.12f,1) && gd_native_ui_shutdown());
    }
    result=0;
cleanup:
    gd_native_ui_shutdown();if(context)igDestroyContext(context);
    if(window)DestroyWindow(window);if(registered)UnregisterClassW(class_name,instance);
    printf("{\"ok\":%s,\"checks\":%d,\"imgui_version\":\"1.93.0\",\"gpu\":%s,\"scope\":\"%s; human rendered acceptance untested\"}\n",result ? "false":"true",checks,gpu ? "true":"false",gpu ? "owned hidden non-activating window; native device/render/resize/shutdown":"C99/CJIT ABI and pre-init errors; no window or GPU");
#undef CHECK
    return result;
}
