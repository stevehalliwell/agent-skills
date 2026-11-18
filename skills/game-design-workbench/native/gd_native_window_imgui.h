#ifndef GD_NATIVE_WINDOW_IMGUI_H
#define GD_NATIVE_WINDOW_IMGUI_H
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_NO_EXPORT
#include "cimgui.h"

/* Same context, font and ABI profile for both native window owners. */
static int window_imgui_create(GD_NativeWindow *window,GD_WindowVisibility visibility) {
    window->context=igCreateContext(NULL);
    if(!window->context || !igDebugCheckVersionAndDataLayout("1.93.0",sizeof(ImGuiIO),sizeof(ImGuiStyle),sizeof(ImVec2_c),sizeof(ImVec4_c),sizeof(ImDrawVert),sizeof(ImDrawIdx)))return 0;
    ImGuiIO *io=igGetIO_Nil();io->IniFilename=NULL;io->LogFilename=NULL;
    io->ConfigFlags|=ImGuiConfigFlags_NavEnableKeyboard;
    io->ConfigNavMoveSetMousePos=false;
    if(visibility==GD_WINDOW_HIDDEN)io->ConfigFlags|=ImGuiConfigFlags_NoMouseCursorChange;
    /* Explicit vector default: the 13px heuristic otherwise picks bitmap. */
    if(!ImFontAtlas_AddFontDefaultVector(io->Fonts,NULL))return 0;
    igStyleColorsDark(NULL);return 1;
}
#endif
