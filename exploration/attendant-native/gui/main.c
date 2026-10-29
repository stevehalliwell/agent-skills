/* Windows C99 workspace host; C++ dependencies stay in Zig-built GUI DLL. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_NO_EXPORT
#include "cimgui.h"
#include "richmd_c.h"
#include "workspace.h"
__declspec(dllimport) LPWSTR *WINAPI CommandLineToArgvW(LPCWSTR, int *);

typedef struct gui_app {
    gui_workspace workspace;
    char open_path[GUI_PATH];
    int request_exit, shown_activation;
    DWORD last_poll;
    ImFont *source_font;
#ifdef ATTENDANT_GUI_TESTING
    int exercise, reference_clicks;
    ImVec2 first_position, depends_position;
#endif
} gui_app;
static gui_app *application;
static void link_clicked(const char *url, void *user) { gui_open_link(&((gui_app *)user)->workspace, url); }
static LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    if (crmd_message(window, message, wparam, lparam)) return 1;
    if (message == WM_CLOSE) { if (application) application->request_exit = 1; return 0; }
    if (message == WM_DESTROY) { PostQuitMessage(0); return 0; }
    return DefWindowProcA(window, message, wparam, lparam);
}
static void show_text(const char *text) { igTextUnformatted(text, NULL); }
static const char *file_name(const char *path) {
    const char *a = strrchr(path, '\\'), *b = strrchr(path, '/');
    return a && (!b || a > b) ? a + 1 : b ? b + 1 : path;
}
static void navigation_panel(gui_app *app) {
    gui_workspace *workspace = &app->workspace;
    gui_navigation *navigation = workspace->navigation;
    int i, j;
    ImVec2 zero = {0, 0};
    if (!navigation) return;
    for (i = 0; i < navigation->count; i++) {
        const char *collection = navigation->documents[i].collection;
        for (j = 0; j < i; j++) if (!strcmp(collection, navigation->documents[j].collection)) break;
        if (j != i) continue;
        if (igCollapsingHeader_TreeNodeFlags(collection, ImGuiTreeNodeFlags_DefaultOpen)) {
            for (j = i; j < navigation->count; j++) {
                attendant_document_info *document = &navigation->documents[j];
                char label[2048]; bool selected;
                if (strcmp(collection, document->collection)) continue;
                snprintf(label, sizeof(label), "%s%s%s###document-%d", document->pending ? "[pending] " : "", document->invalid ? "[invalid] " : "", document->name, j);
                selected = workspace->active >= 0 && !_stricmp(document->path, workspace->tabs[workspace->active].path);
                if (igSelectable_Bool(label, selected, 0, zero)) gui_open_document(workspace, document->path);
#ifdef ATTENDANT_GUI_TESTING
                if (!strcmp(document->collection, "tasks") && !strcmp(document->name, "first")) {
                    ImVec2 a = igGetItemRectMin(), b = igGetItemRectMax();
                    app->first_position.x = (a.x + b.x) * 0.5f; app->first_position.y = (a.y + b.y) * 0.5f;
                }
#endif
            }
        }
    }
}
static void reference_bar(gui_app *app, gui_tab *tab) {
    gui_navigation *navigation = app->workspace.navigation;
    int i, j;
    ImVec2 zero = {0, 0};
    if (!navigation) return;
    for (i = 0; i < navigation->count; i++) {
        attendant_document_info *document = &navigation->documents[i];
        if (_stricmp(document->path, tab->path)) continue;
        for (j = 0; j < document->reference_count; j++) {
            attendant_reference *reference = &document->references[j]; char label[2048];
            snprintf(label, sizeof(label), "%s: %s%s###reference-%d", reference->field, reference->value, reference->target_path ? "" : " [unresolved]", j);
            igBeginDisabled(reference->target_path == NULL);
            if (igButton(label, zero) && reference->target_path) {
                gui_open_document(&app->workspace, reference->target_path);
#ifdef ATTENDANT_GUI_TESTING
                app->reference_clicks++;
#endif
            }
#ifdef ATTENDANT_GUI_TESTING
            if (reference->target_path && !strcmp(reference->value, "second") && !strcmp(reference->field, "depends_on") && !strcmp(file_name(tab->path), "first.md")) {
                ImVec2 a = igGetItemRectMin(), b = igGetItemRectMax();
                app->depends_position.x = (a.x + b.x) * 0.5f; app->depends_position.y = (a.y + b.y) * 0.5f;
            }
#endif
            igEndDisabled();
        }
        break;
    }
}
static int source_selection(ImGuiInputTextCallbackData *data) {
    gui_tab *tab = data->UserData;
    tab->selection_start = data->SelectionStart; tab->selection_end = data->SelectionEnd;
    return 0;
}
static void document_panel(gui_app *app, int index) {
    gui_tab *tab = &app->workspace.tabs[index];
    ImVec2 remaining, editor, zero = {0, 0}; char editor_id[64];
    show_text(tab->path);
    if (tab->external_change) show_text("File changed or disappeared outside this editor.");
    reference_bar(app, tab);
    remaining = igGetContentRegionAvail();
    editor.x = remaining.x * 0.49f; editor.y = remaining.y;
    snprintf(editor_id, sizeof(editor_id), "##source-%d-%d", index, tab->generation);
    igPushFont(app->source_font, 16.0f);
    igInputTextMultiline(editor_id, tab->text, GUI_TEXT + 1, editor,
        ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_CallbackAlways | ImGuiInputTextFlags_AllowTabInput, source_selection, tab);
    igPopFont();
    igSameLine(0, -1);
    igBeginChild_Str("preview", zero, ImGuiChildFlags_Borders, 0);
    if (!crmd_render(tab->text + gui_body_offset(tab->text), strlen(tab->text + gui_body_offset(tab->text))))
        snprintf(app->workspace.error, sizeof(app->workspace.error), "%s", crmd_error());
    igEndChild();
}
static void draw_workspace(gui_app *app) {
    gui_workspace *workspace = &app->workspace;
    ImGuiViewport *viewport = igGetMainViewport();
    ImVec2 zero = {0, 0}, sidebar = {260, 0};
    int i, forced = app->shown_activation != workspace->activation ? workspace->active : -1;
    app->shown_activation = workspace->activation;
    igSetNextWindowPos(viewport->WorkPos, ImGuiCond_Always, zero);
    igSetNextWindowSize(viewport->WorkSize, ImGuiCond_Always);
    igBegin("Attendant shared Markdown", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
    show_text(workspace->root);
    if (igButton("Refresh files", zero)) gui_refresh(workspace);
    igSameLine(0, -1); igSetNextItemWidth(500);
    igInputText("##open-path", app->open_path, sizeof(app->open_path), 0, NULL, NULL);
    igSameLine(0, -1);
    if (igButton("Open Markdown", zero)) gui_open_document(workspace, app->open_path);
    if (*workspace->error) {
        show_text(workspace->error); igSameLine(0, -1);
        if (igButton("Dismiss error", zero)) workspace->error[0] = 0;
    }
    if (*workspace->notice) show_text(workspace->notice);
    igSeparator();
    igBeginChild_Str("navigation", sidebar, ImGuiChildFlags_Borders, 0);
    navigation_panel(app); igEndChild(); igSameLine(0, -1);
    igBeginChild_Str("documents", zero, 0, 0);
    if (igBeginTabBar("document-tabs", ImGuiTabBarFlags_Reorderable)) {
        for (i = 0; i < GUI_TABS; i++) if (workspace->tabs[i].used) {
            gui_tab *tab = &workspace->tabs[i]; char label[2048]; bool opened = true;
            ImGuiTabItemFlags flags = forced == i ? ImGuiTabItemFlags_SetSelected : 0;
            snprintf(label, sizeof(label), "%s%s###tab-%d", file_name(tab->path), tab->dirty ? " *" : "", i);
            if (igBeginTabItem(label, &opened, flags)) {
                /* Navigation may queue another tab while old tab is still drawn.
                   Do not overwrite that human focus request with old render state. */
                if (app->shown_activation == workspace->activation) workspace->active = i;
                document_panel(app, i); igEndTabItem();
            }
            if (!opened) gui_close_document(workspace, i, 0);
        }
        igEndTabBar();
    }
    if (!workspace->tab_count) show_text("Open a Markdown file. Source and rendered preview stay in the same tab.");
    igEndChild(); igEnd();
}
static int load_fonts(gui_app *app, const char *assets) {
    const char *names[] = {"fonts/Roboto/Roboto-Regular.ttf", "fonts/Inconsolata-Medium.ttf", "fonts/Roboto/Roboto-Bold.ttf", "fonts/Roboto/Roboto-RegularItalic.ttf", "fonts/Roboto/Roboto-BoldItalic.ttf"};
    char paths[5][GUI_PATH]; wchar_t wide[4096]; int i;
    ImFontConfig *merge;
    for (i = 0; i < 5; i++) {
        int length = snprintf(paths[i], sizeof(paths[i]), "%s/%s", assets, names[i]);
        if (length < 0 || length >= GUI_PATH || !MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, paths[i], -1, wide, 4096) || GetFileAttributesW(wide) == INVALID_FILE_ATTRIBUTES) return 0;
    }
    igGetIO_Nil()->FontDefault = ImFontAtlas_AddFontFromFileTTF(igGetIO_Nil()->Fonts, paths[0], 16.0f, NULL, NULL);
    app->source_font = ImFontAtlas_AddFontFromFileTTF(igGetIO_Nil()->Fonts, paths[1], 16.0f, NULL, NULL);
    merge = ImFontConfig_ImFontConfig(); merge->MergeMode = true;
    ImFontAtlas_AddFontFromFileTTF(igGetIO_Nil()->Fonts, paths[0], 16.0f, merge, NULL); ImFontConfig_destroy(merge);
    return app->source_font && igGetIO_Nil()->FontDefault;
}
#ifdef ATTENDANT_GUI_TESTING
static void exercise_input(gui_app *app, HWND window, int frame) {
    ImVec2 position;
    if (!app->exercise) return;
    if (frame >= 5 && frame <= 8) position = app->depends_position;
    else if (frame >= 12 && frame <= 15) position = app->first_position;
    else if (frame >= 20 && frame <= 23) position = app->depends_position;
    else return;
    crmd_message(window, WM_MOUSEMOVE, 0, MAKELPARAM((int)position.x, (int)position.y));
    if (frame == 5 || frame == 12 || frame == 20) ImGuiIO_AddMouseButtonEvent(igGetIO_Nil(), 0, true);
    if (frame == 6 || frame == 13 || frame == 21) ImGuiIO_AddMouseButtonEvent(igGetIO_Nil(), 0, false);
}
#endif
static int usage(FILE *output, int code) {
    fprintf(output, "Usage: attendant-gui --project DIR --assets DIR [--file PATH] [--frames 1..6000 --shot FILE.ppm]\n");
    return code;
}
int main(int argc, char **argv) {
    const char *project = NULL, *assets = NULL, *initial = NULL, *shot = NULL;
    char *owned[32] = {0};
    LPWSTR *arguments;
    int frame_limit = 0, frame = 0, code = 1, i, gui_ready = 0;
#ifdef ATTENDANT_GUI_TESTING
    int exercise = 0;
#endif
    bool running = true;
    HWND window = NULL;
    WNDCLASSA description;
    ImGuiContext *context = NULL;
    MSG message;
    arguments = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!arguments || argc > 32) return usage(stderr, 2);
    for (i = 0; i < argc; i++) {
        int size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, arguments[i], -1, NULL, 0, NULL, NULL);
        owned[i] = HeapAlloc(GetProcessHeap(), 0, size);
        if (!owned[i] || !WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, arguments[i], -1, owned[i], size, NULL, NULL)) return 3;
    }
    LocalFree(arguments); argv = owned;
    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--help")) return usage(stdout, 0);
#ifdef ATTENDANT_GUI_TESTING
        if (!strcmp(argv[i], "--exercise")) { exercise = 1; continue; }
#endif
        if (!strcmp(argv[i], "--project") && i + 1 < argc) project = argv[++i];
        else if (!strcmp(argv[i], "--assets") && i + 1 < argc) assets = argv[++i];
        else if (!strcmp(argv[i], "--file") && i + 1 < argc) initial = argv[++i];
        else if (!strcmp(argv[i], "--shot") && i + 1 < argc) shot = argv[++i];
        else if (!strcmp(argv[i], "--frames") && i + 1 < argc) {
            char *end; long value = strtol(argv[++i], &end, 10);
            if (*end || value < 1 || value > 6000) return usage(stderr, 2);
            frame_limit = (int)value;
        } else return usage(stderr, 2);
    }
    if (!project || !assets || (shot && !frame_limit)) return usage(stderr, 2);
    application = VirtualAlloc(NULL, sizeof(*application), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!application) return 3;
    application->shown_activation = -1;
#ifdef ATTENDANT_GUI_TESTING
    application->exercise = exercise;
#endif
    if (!gui_workspace_init(&application->workspace, project)) { fprintf(stderr, "%s\n", application->workspace.error); goto cleanup; }
    if (initial) gui_open_document(&application->workspace, initial);
    memset(&description, 0, sizeof(description)); description.lpfnWndProc = window_proc;
    description.hInstance = GetModuleHandleA(NULL); description.lpszClassName = "AttendantMarkdownWorkspace";
    description.hCursor = LoadCursorA(NULL, IDC_ARROW);
    if (!RegisterClassA(&description)) goto cleanup;
    context = igCreateContext(NULL); igGetIO_Nil()->IniFilename = NULL; igStyleColorsDark(NULL);
    if (!load_fonts(application, assets)) { fprintf(stderr, "Missing GUI font assets\n"); goto cleanup; }
    if (!igDebugCheckVersionAndDataLayout(igGetVersion(), sizeof(ImGuiIO), sizeof(ImGuiStyle), sizeof(ImVec2), sizeof(ImVec4), sizeof(ImDrawVert), sizeof(ImDrawIdx))) goto cleanup;
    {
        const size_t layout[] = {sizeof(ImGuiStoragePair), offsetof(ImGuiStoragePair, value), sizeof(ImGuiStyleMod), offsetof(ImGuiStyleMod, backup), sizeof(ImGuiInputEvent), offsetof(ImGuiInputEvent, event)};
        if (!crmd_check_cimgui_layout(layout, sizeof(layout) / sizeof(layout[0]))) goto cleanup;
    }
    window = CreateWindowExA(0, description.lpszClassName, "Attendant - shared Markdown workspace", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1440, 1000, NULL, NULL, description.hInstance, NULL);
    if (!window || !crmd_init(window, assets, link_clicked, application)) { fprintf(stderr, "%s\n", crmd_error()); goto cleanup; }
    gui_ready = 1; ShowWindow(window, SW_SHOWDEFAULT); UpdateWindow(window);
    while (running && (!frame_limit || frame < frame_limit)) {
        while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
            if (message.message == WM_QUIT) running = false;
            TranslateMessage(&message); DispatchMessageA(&message);
        }
        if (application->request_exit || !running) break;
        if (GetTickCount() - application->last_poll > 1000) {
            if (application->workspace.active >= 0) gui_check_external(&application->workspace, application->workspace.active);
            application->last_poll = GetTickCount();
        }
#ifdef ATTENDANT_GUI_TESTING
        exercise_input(application, window, frame);
#endif
        if (!crmd_begin_frame()) goto cleanup;
        draw_workspace(application);
        if (!crmd_end_frame()) goto cleanup;
        ++frame;
        if (shot && frame == frame_limit && !crmd_screenshot(shot)) goto cleanup;
        if (!crmd_present()) goto cleanup;
    }
#ifdef ATTENDANT_GUI_TESTING
    if (exercise && (application->workspace.tab_count != 2 || application->reference_clicks != 2)) {
        fprintf(stderr, "GUI reference exercise failed: tabs=%d clicks=%d\n", application->workspace.tab_count, application->reference_clicks); goto cleanup;
    }
#endif
    code = 0;
cleanup:
    if (gui_ready && !crmd_shutdown()) code = 1;
    if (crmd_live_images()) code = 1;
    if (window) DestroyWindow(window);
    if (context) igDestroyContext(context);
    if (!code) printf("{\"ok\":true,\"frames\":%d,\"tabs\":%d,\"vertices\":%d}\n", frame, application->workspace.tab_count, crmd_vertex_count());
    else fprintf(stderr, "GUI initialization, exercise, rendering or cleanup failed: %s\n", crmd_error());
    gui_workspace_destroy(&application->workspace); VirtualFree(application, 0, MEM_RELEASE); application = NULL;
    UnregisterClassA("AttendantMarkdownWorkspace", GetModuleHandleA(NULL));
    for (i = 0; i < argc; i++) HeapFree(GetProcessHeap(), 0, owned[i]);
    return code;
}
