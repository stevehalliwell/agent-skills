/* Owned popup state, never OS input. Search must not mutate native query/scope. */
#include "gd_explorer_loaded_pickers.h"
static int explorer_test_picker_window(GD_ExplorerContext *context,const char *output) {
    GD_ExplorerPane *pane=context->pane; GD_Query *query=pane->query_state ? pane->query_state->query:NULL;
    GD_ExplorerQueryOptions options=pane->query_options; GD_ExplorerSelection selection=pane->selection;
    Camera2D camera=pane->camera; char text[GD_MAX_PATH]="",path[GD_MAX_PATH],saved[129]; int frame,opened=0;
    memcpy(saved,pane->picker_search,sizeof(saved));
    for (frame=0;frame<2;++frame) {
        BeginDrawing(); ClearBackground(background); rlImGuiBegin();
        igSetNextWindowPos((ImVec2_c){40,40},ImGuiCond_Always,(ImVec2_c){0,0}); igSetNextWindowSize((ImVec2_c){780,720},ImGuiCond_Always);
        igBegin("Owned loaded-data picker check",NULL,ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoSavedSettings);
        igTextWrapped("Search is presentation only. Native identities, applied query, selected source and ordinary camera remain unchanged.");
        igPushID_Str("Object definition");
        { ImGuiID popup=igImHashStr("##ComboPopup",0,igGetID_Str("##choice")); igOpenPopupEx(popup,0); opened|=igIsPopupOpen_ID(popup,0); }
        igPopID();
        gd_explorer_picker_record(context,"Object definition",GD_NOUN,text,sizeof(text));
        snprintf(pane->picker_search,sizeof(pane->picker_search),"r");
        igEnd(); rlImGuiEnd(); EndDrawing();
    }
    memcpy(pane->picker_search,saved,sizeof(saved));
    if (!opened || *text || pane->query_state->query!=query || memcmp(&options,&pane->query_options,sizeof(options)) || memcmp(&selection,&pane->selection,sizeof(selection)) || memcmp(&camera,&pane->camera,sizeof(camera))) return 0;
    if (snprintf(path,sizeof(path),"%s/fuzzy-loaded-picker.png",output)>=(int)sizeof(path)) return 0;
    return gd_save_screenshot(path);
}
