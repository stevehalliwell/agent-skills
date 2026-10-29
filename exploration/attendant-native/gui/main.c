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

typedef enum gui_action { GUI_ACTION_NONE, GUI_ACTION_CLOSE, GUI_ACTION_RELOAD, GUI_ACTION_EXIT } gui_action;
typedef struct gui_app {
    gui_workspace workspace;
    char open_path[GUI_PATH];
    int request_exit, shown_activation, bound_reached, exit_cancelled;
    gui_action pending_action;
    int pending_tab, pending_open, composer_active, composer_open, composer_tab, feedback_filter;
    int composer_start, composer_end, composer_from_preview;
    char comment[64 * 1024];
    DWORD last_poll;
    ImFont *source_font;
#ifdef ATTENDANT_GUI_TESTING
    int exercise, reference_clicks, edit_exercise, save_fail_exercise, cancel_clicks, sent_comments, target_clicks, undo_seen, saved_not_queued;
    int preview_exercise, preview_exact_seen, preview_dirty_send;
    ImVec2 preview_position, preview_comment_position;
    ImVec2 first_position, depends_position, editor_position, comment_position, send_position, cancel_position, composer_cancel_position, feedback_position, target_position;
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
static void recovery_details(gui_workspace *workspace) {
    int i;
    if (!workspace->surviving_total) return;
    show_text("Send failed; files remain. Inspect before retrying:");
    for (i = 0; i < workspace->surviving_count; i++) show_text(workspace->surviving_paths[i]);
    if (workspace->surviving_total > workspace->surviving_count) show_text("Additional written paths are logged to stderr.");
}
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
            if (!strcmp(collection, "_meta_comments")) {
                igRadioButton_IntPtr("All", &app->feedback_filter, 0); igSameLine(0, -1);
                igRadioButton_IntPtr("Pending", &app->feedback_filter, 1); igSameLine(0, -1);
                igRadioButton_IntPtr("Resolved", &app->feedback_filter, 2);
            }
            for (j = i; j < navigation->count; j++) {
                attendant_document_info *document = &navigation->documents[j];
                char label[2048]; bool selected;
                if (strcmp(collection, document->collection)) continue;
                if (!strcmp(collection, "_meta_comments") && ((app->feedback_filter == 1 && !document->pending) || (app->feedback_filter == 2 && !document->resolved))) continue;
                snprintf(label, sizeof(label), "%s%s%s###document-%d", document->pending ? "[pending] " : "", document->invalid ? "[invalid] " : "", document->name, j);
                selected = workspace->active >= 0 && !_stricmp(document->path, workspace->tabs[workspace->active].path);
                if (igSelectable_Bool(label, selected, 0, zero)) gui_open_document(workspace, document->path);
#ifdef ATTENDANT_GUI_TESTING
                if (!strcmp(document->collection, "_meta_comments") && document->pending) {
                    ImVec2 a = igGetItemRectMin(), b = igGetItemRectMax();
                    app->feedback_position.x = (a.x + b.x) * 0.5f; app->feedback_position.y = (a.y + b.y) * 0.5f;
                }
                if (!strcmp(document->collection, "tasks") && !strcmp(document->name, "first")) {
                    ImVec2 a = igGetItemRectMin(), b = igGetItemRectMax();
                    app->first_position.x = (a.x + b.x) * 0.5f; app->first_position.y = (a.y + b.y) * 0.5f;
                }
#endif
            }
        }
    }
}
static void reference_bar(gui_app *app, int index) {
    gui_tab *tab = &app->workspace.tabs[index];
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
        if (!strcmp(document->collection, "_meta_comments")) {
            if (igButton("Open target selection", zero)) {
                gui_open_comment_target(&app->workspace, index);
#ifdef ATTENDANT_GUI_TESTING
                app->target_clicks++;
#endif
            }
#ifdef ATTENDANT_GUI_TESTING
            { ImVec2 a = igGetItemRectMin(), b = igGetItemRectMax(); app->target_position.x = (a.x + b.x) * 0.5f; app->target_position.y = (a.y + b.y) * 0.5f; }
#endif
        }
        break;
    }
}
static int dirty_documents(gui_workspace *workspace) {
    int i;
    for (i = 0; i < GUI_TABS; i++) if (workspace->tabs[i].used && workspace->tabs[i].dirty) return 1;
    return 0;
}
static void pending_action(gui_app *app, gui_action action, int index) {
    app->pending_action = action; app->pending_tab = index; app->pending_open = 1;
}
#ifdef ATTENDANT_GUI_TESTING
static ImVec2 item_center(void) {
    ImVec2 a = igGetItemRectMin(), b = igGetItemRectMax(), center = {(a.x + b.x) * 0.5f, (a.y + b.y) * 0.5f}; return center;
}
#endif
static int source_selection(ImGuiInputTextCallbackData *data) {
    gui_tab *tab = data->UserData;
    if (tab->reveal_selection) {
        ImGuiInputTextCallbackData_SetSelection(data, tab->selection_start, tab->selection_end);
        data->CursorPos = tab->selection_end; tab->reveal_selection = 0;
    }
    tab->selection_start = data->SelectionStart; tab->selection_end = data->SelectionEnd;
    return 0;
}
static void document_panel(gui_app *app, int index) {
    gui_tab *tab = &app->workspace.tabs[index];
    ImVec2 remaining, editor, zero = {0, 0}; char editor_id[64];
    show_text(tab->path);
    if (tab->external_change) show_text("File changed or disappeared outside this editor.");
    if (tab->anchor_stale) show_text("Stored comment anchor is stale; no range was applied.");
    reference_bar(app, index);
    igBeginDisabled(!tab->dirty);
    if (igButton("Save", zero)) gui_save_document(&app->workspace, index);
    igEndDisabled(); igSameLine(0, -1);
    if (igButton("Reload", zero)) {
        if (tab->dirty) pending_action(app, GUI_ACTION_RELOAD, index); else gui_reload_document(&app->workspace, index, 0);
    }
    igSameLine(0, -1);
    igBeginDisabled(tab->selection_start == tab->selection_end);
    if (igButton("Comment on source selection", zero)) {
        app->composer_tab = index; app->composer_active = app->composer_open = 1; app->comment[0] = 0;
        app->composer_start = tab->selection_start; app->composer_end = tab->selection_end; app->composer_from_preview = 0;
        app->workspace.notice[0] = 0;
    }
#ifdef ATTENDANT_GUI_TESTING
    app->comment_position = item_center();
#endif
    igEndDisabled();
    if (!app->composer_active && app->pending_action == GUI_ACTION_NONE && igGetIO_Nil()->KeyCtrl && igIsKeyPressed_Bool(ImGuiKey_S, false) && tab->dirty)
        gui_save_document(&app->workspace, index);
    remaining = igGetContentRegionAvail();
    editor.x = remaining.x * 0.49f; editor.y = remaining.y;
    snprintf(editor_id, sizeof(editor_id), "##source-%d-%d", index, tab->generation);
    igPushFont(app->source_font, 16.0f);
    if (tab->reveal_selection) igSetKeyboardFocusHere(0);
    if (igInputTextMultiline(editor_id, tab->text, GUI_TEXT + 1, editor,
        ImGuiInputTextFlags_CallbackAlways | ImGuiInputTextFlags_AllowTabInput, source_selection, tab)) {
        tab->length = strlen(tab->text); tab->dirty = 1; tab->anchor_stale = 0; tab->text_revision = ++app->workspace.text_revision;
    }
#ifdef ATTENDANT_GUI_TESTING
    { ImVec2 position = igGetItemRectMin(); app->editor_position.x = position.x + 30; app->editor_position.y = position.y + 12; }
#endif
    igPopFont();
    igSameLine(0, -1);
    igBeginChild_Str("preview", zero, ImGuiChildFlags_Borders, 0);
    {
        crmd_source_selection selected = {0};
        size_t offset = gui_body_offset(tab->text), length = tab->length - offset;
        int exact, rendered;
        ImVec2 content = igGetContentRegionAvail(); content.x = 0; content.y = content.y > 54 ? content.y - 54 : 1;
        igBeginChild_Str("preview-scroll", content, 0, 0);
#ifdef ATTENDANT_GUI_TESTING
        app->preview_position = igGetCursorScreenPos(); app->preview_position.x += 2; app->preview_position.y += 10;
#endif
        rendered = crmd_render_raw(tab->path, tab->text_revision, tab->text + offset, length, &selected);
        if (!rendered) snprintf(app->workspace.error, sizeof(app->workspace.error), "%s", crmd_error());
        exact = rendered && selected.status == CRMD_SELECTION_EXACT && selected.begin < selected.end && selected.end <= length;
#ifdef ATTENDANT_GUI_TESTING
        if (exact) app->preview_exact_seen = 1;
#endif
        igEndChild();
        igBeginDisabled(!exact);
        if (igButton("Comment on preview selection", zero)) {
            app->composer_tab = index; app->composer_active = app->composer_open = 1; app->comment[0] = 0;
            app->composer_start = (int)(offset + selected.begin); app->composer_end = (int)(offset + selected.end); app->composer_from_preview = 1;
            app->workspace.notice[0] = 0;
        }
#ifdef ATTENDANT_GUI_TESTING
        app->preview_comment_position = item_center();
#endif
        igEndDisabled();
        show_text(selected.status == CRMD_SELECTION_UNSUPPORTED ? "Selection crosses generated/omitted content; use source." : "Only exact original-source spans can be sent.");
    }
    igEndChild();
}
static void draw_dialogs(gui_app *app) {
    gui_workspace *workspace = &app->workspace;
    ImVec2 zero = {0, 0}, selected_size = {660, 80}, comment_size = {660, 150};
    if (app->request_exit && !app->composer_active && app->pending_action == GUI_ACTION_NONE) {
        int i;
        for (i = 0; i < GUI_TABS; i++) if (workspace->tabs[i].used && workspace->tabs[i].dirty) { pending_action(app, GUI_ACTION_EXIT, i); break; }
    }
    if (app->pending_open) { igOpenPopup_Str("Unsaved source", 0); app->pending_open = 0; }
    if (igBeginPopupModal("Unsaved source", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        gui_action action = app->pending_action; int index = app->pending_tab;
        show_text(workspace->tabs[index].path); show_text("Source has unsaved edits. Choose Save, Discard or Cancel.");
        if (*workspace->error) show_text(workspace->error);
        if (igButton("Save", zero)) {
            if (gui_save_document(workspace, index) &&
                (action != GUI_ACTION_RELOAD || gui_reload_document(workspace, index, 0)) &&
                (action != GUI_ACTION_CLOSE || gui_close_document(workspace, index, 0))) {
                app->pending_action = GUI_ACTION_NONE; igCloseCurrentPopup();
            }
        }
        igSameLine(0, -1);
        if (igButton("Discard", zero)) {
            if (action == GUI_ACTION_RELOAD ? gui_reload_document(workspace, index, 1) : gui_close_document(workspace, index, 1)) {
                app->pending_action = GUI_ACTION_NONE; igCloseCurrentPopup();
            }
        }
        igSameLine(0, -1);
        if (igButton("Cancel", zero) || igIsKeyPressed_Bool(ImGuiKey_Escape, false)) {
            app->pending_action = GUI_ACTION_NONE; app->request_exit = 0;
            if (app->bound_reached) app->exit_cancelled = 1;
#ifdef ATTENDANT_GUI_TESTING
            app->cancel_clicks++;
#endif
            igCloseCurrentPopup();
        }
#ifdef ATTENDANT_GUI_TESTING
        app->cancel_position = item_center();
#endif
        igEndPopup();
    }
    if (app->composer_open) { igOpenPopup_Str("Comment on saved source", 0); app->composer_open = 0; }
    if (igBeginPopupModal("Comment on saved source", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        gui_tab *tab = &workspace->tabs[app->composer_tab];
        int start = app->composer_start, end = app->composer_end;
        if (start > end) { int swap = start; start = end; end = swap; }
        show_text(tab->path);
        if (app->composer_from_preview) show_text("Exact original-source selection from raw preview.");
        igBeginChild_Str("selected source", selected_size, ImGuiChildFlags_Borders, 0);
        if (start >= 0 && end >= start && (size_t)end <= tab->length) igTextUnformatted(tab->text + start, tab->text + end);
        igEndChild();
        if (igIsWindowAppearing()) igSetKeyboardFocusHere(0);
        igInputTextMultiline("##comment", app->comment, sizeof(app->comment), comment_size, 0, NULL, NULL);
        show_text("Context note only. Reading never resolves it or authorizes unrelated edits.");
        if (tab->dirty) show_text("Save & Send writes this source first, then queues comment.");
        if (*workspace->error) show_text(workspace->error);
        if (*workspace->notice) show_text(workspace->notice);
        recovery_details(workspace);
        igBeginDisabled(!*app->comment);
        if (igButton(tab->dirty ? "Save & Send" : "Send", zero)) {
            int saved = tab->dirty;
#ifdef ATTENDANT_GUI_TESTING
            if (app->preview_exercise && saved) app->preview_dirty_send = 1;
#endif
            if (!saved || gui_save_document(workspace, app->composer_tab)) {
                if (gui_send_comment(workspace, app->composer_tab, app->composer_start, app->composer_end, app->comment)) {
                    app->composer_active = 0; igCloseCurrentPopup();
#ifdef ATTENDANT_GUI_TESTING
                    app->sent_comments++;
#endif
                } else if (saved) {
                    snprintf(workspace->notice, sizeof(workspace->notice), "Saved, but comment was not queued. Source save was not rolled back.");
#ifdef ATTENDANT_GUI_TESTING
                    app->saved_not_queued = 1;
#endif
                }
            }
        }
#ifdef ATTENDANT_GUI_TESTING
        app->send_position = item_center();
#endif
        igEndDisabled(); igSameLine(0, -1);
        if (igButton("Cancel", zero) || igIsKeyPressed_Bool(ImGuiKey_Escape, false)) { app->composer_active = 0; igCloseCurrentPopup(); }
#ifdef ATTENDANT_GUI_TESTING
        app->composer_cancel_position = item_center();
#endif
        igEndPopup();
    }
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
        if (igButton("Dismiss error", zero)) { workspace->error[0] = workspace->failed_path[0] = 0; workspace->surviving_count = workspace->surviving_total = 0; }
    }
    recovery_details(workspace);
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
            if (!opened) {
                if (tab->dirty) pending_action(app, GUI_ACTION_CLOSE, i); else gui_close_document(workspace, i, 0);
            }
        }
        igEndTabBar();
    }
    if (!workspace->tab_count) show_text("Open a Markdown file. Source and rendered preview stay in the same tab.");
    igEndChild(); draw_dialogs(app); igEnd();
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
    if (app->preview_exercise) {
        ImGuiIO *io = igGetIO_Nil();
        if (frame >= 5 && frame <= 16) position = app->editor_position;
        else if (frame >= 20 && frame <= 28) position = app->preview_position;
        else if (frame >= 30 && frame <= 34) position = app->preview_comment_position;
        else if (frame >= 44 && frame <= 48) position = app->send_position;
        else position.x = position.y = -100;
        if (position.x >= 0) crmd_message(window, WM_MOUSEMOVE, 0, MAKELPARAM((int)position.x, (int)position.y));
        if (frame == 5 || frame == 20 || frame == 30 || frame == 44) ImGuiIO_AddMouseButtonEvent(io, 0, true);
        if (frame == 6 || frame == 21 || frame == 31 || frame == 45) ImGuiIO_AddMouseButtonEvent(io, 0, false);
        if (frame == 8 || frame == 24) ImGuiIO_AddKeyEvent(io, ImGuiMod_Ctrl, true);
        if (frame == 8 || frame == 9) ImGuiIO_AddKeyEvent(io, ImGuiKey_End, frame == 8);
        if (frame == 10 || frame == 26) ImGuiIO_AddKeyEvent(io, ImGuiMod_Ctrl, false);
        if (frame == 12) ImGuiIO_AddInputCharactersUTF8(io, "!");
        if (frame == 24 || frame == 25) ImGuiIO_AddKeyEvent(io, ImGuiKey_A, frame == 24);
        if (frame == 38) ImGuiIO_AddInputCharactersUTF8(io, "GUI preview context note\nSecond line.");
        return;
    }
    if (app->edit_exercise) {
        gui_tab *tab = &app->workspace.tabs[app->workspace.active];
        if (frame == 18) app->undo_seen = strstr(tab->text, " GUI edit") == NULL;
        if ((frame >= 5 && frame <= 7) || (frame >= 30 && frame <= 32)) position = app->editor_position;
        else if (frame >= 26 && frame <= 28) position = app->cancel_position;
        else if (frame >= 38 && frame <= 40) position = app->comment_position;
        else if (frame >= 50 && frame <= 52) position = app->send_position;
        else if (app->save_fail_exercise && frame >= 58 && frame <= 60) position = app->composer_cancel_position;
        else if (!app->save_fail_exercise && frame >= 55 && frame <= 57) position = app->feedback_position;
        else if (!app->save_fail_exercise && frame >= 59 && frame <= 61) position = app->target_position;
        else position.x = position.y = -100;
        if (position.x >= 0) crmd_message(window, WM_MOUSEMOVE, 0, MAKELPARAM((int)position.x, (int)position.y));
        if (frame == 5 || frame == 26 || frame == 30 || frame == 38 || frame == 50 || (app->save_fail_exercise && frame == 58) || (!app->save_fail_exercise && (frame == 55 || frame == 59))) ImGuiIO_AddMouseButtonEvent(igGetIO_Nil(), 0, true);
        if (frame == 6 || frame == 27 || frame == 31 || frame == 39 || frame == 51 || (app->save_fail_exercise && frame == 59) || (!app->save_fail_exercise && (frame == 56 || frame == 60))) ImGuiIO_AddMouseButtonEvent(igGetIO_Nil(), 0, false);
        if (frame == 8 || frame == 16 || frame == 19 || frame == 33) ImGuiIO_AddKeyEvent(igGetIO_Nil(), ImGuiMod_Ctrl, true);
        if (frame == 8) ImGuiIO_AddKeyEvent(igGetIO_Nil(), ImGuiKey_End, true);
        if (frame == 9) ImGuiIO_AddKeyEvent(igGetIO_Nil(), ImGuiKey_End, false);
        if (frame == 10 || frame == 17 || frame == 20 || frame == 35) ImGuiIO_AddKeyEvent(igGetIO_Nil(), ImGuiMod_Ctrl, false);
        if (frame == 12) ImGuiIO_AddInputCharactersUTF8(igGetIO_Nil(), " GUI edit");
        if (frame == 16 || frame == 17) ImGuiIO_AddKeyEvent(igGetIO_Nil(), ImGuiKey_Z, frame == 16);
        if (frame == 19 || frame == 20) ImGuiIO_AddKeyEvent(igGetIO_Nil(), ImGuiKey_Y, frame == 19);
        if (frame == 22) window_proc(window, WM_CLOSE, 0, 0);
        if (frame == 33 || frame == 34) ImGuiIO_AddKeyEvent(igGetIO_Nil(), ImGuiKey_A, frame == 33);
        if (frame == 44) ImGuiIO_AddInputCharactersUTF8(igGetIO_Nil(), "GUI context note\nSecond line.");
        return;
    }
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
    int exercise = 0, edit_exercise = 0, save_fail_exercise = 0, preview_exercise = 0;
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
        if (!strcmp(argv[i], "--exercise-preview")) { preview_exercise = 1; continue; }
        if (!strcmp(argv[i], "--exercise-edit")) { edit_exercise = 1; continue; }
        if (!strcmp(argv[i], "--exercise-save-fail")) { edit_exercise = save_fail_exercise = 1; continue; }
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
    application->exercise = exercise; application->edit_exercise = edit_exercise; application->save_fail_exercise = save_fail_exercise; application->preview_exercise = preview_exercise;
#endif
    if (!gui_workspace_init(&application->workspace, project)) { fprintf(stderr, "%s\n", application->workspace.error); goto cleanup; }
    if (initial) gui_open_document(&application->workspace, initial);
#ifdef ATTENDANT_GUI_TESTING
    if (edit_exercise || preview_exercise) {
        char marker[GUI_PATH]; wchar_t wide[4096];
        snprintf(marker, sizeof(marker), "%s/.gui-test-owned", application->workspace.root);
        if (application->workspace.active < 0 || !MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, marker, -1, wide, 4096) || GetFileAttributesW(wide) == INVALID_FILE_ATTRIBUTES) {
            fprintf(stderr, "Editing exercise requires generated owned fixture and initial document\n"); goto cleanup;
        }
    }
#endif
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
    while (running) {
        while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
            if (message.message == WM_QUIT) running = false;
            TranslateMessage(&message); DispatchMessageA(&message);
        }
        if (frame_limit && frame >= frame_limit && !application->exit_cancelled) { application->bound_reached = 1; application->request_exit = 1; }
        if (!running || (application->request_exit && !dirty_documents(&application->workspace) && !application->composer_active && application->pending_action == GUI_ACTION_NONE)) break;
        if (!application->composer_active && application->pending_action == GUI_ACTION_NONE && GetTickCount() - application->last_poll > 1000) {
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
    if (preview_exercise) {
        gui_workspace *workspace = &application->workspace;
        gui_tab *tab = workspace->active >= 0 ? &workspace->tabs[workspace->active] : NULL;
        if (!tab || application->sent_comments != 1 || !application->preview_exact_seen || !application->preview_dirty_send || !application->composer_from_preview ||
            application->composer_start != (int)gui_body_offset(tab->text) || (size_t)application->composer_end != tab->length ||
            tab->selection_start != tab->selection_end || dirty_documents(workspace)) {
            fprintf(stderr, "GUI preview source mapping/Save & Send changed selection or failed\n"); goto cleanup;
        }
    }
    if (edit_exercise && !save_fail_exercise) {
        gui_workspace *workspace = &application->workspace;
        gui_tab *tab = workspace->active >= 0 ? &workspace->tabs[workspace->active] : NULL;
        if (!tab || strcmp(file_name(tab->path), "README.md") || tab->selection_start != 0 ||
            (size_t)tab->selection_end != tab->length || tab->reveal_selection || tab->anchor_stale) {
            fprintf(stderr, "GUI target selection was not applied to exact saved source\n"); goto cleanup;
        }
    }
    if (edit_exercise && (application->cancel_clicks != 1 || application->sent_comments != (save_fail_exercise ? 0 : 1) ||
        (save_fail_exercise && !application->saved_not_queued) || (!save_fail_exercise && application->target_clicks != 1) || !application->undo_seen || dirty_documents(&application->workspace))) {
        fprintf(stderr, "GUI editing exercise failed: cancels=%d sent=%d dirty=%d\n", application->cancel_clicks, application->sent_comments, dirty_documents(&application->workspace)); goto cleanup;
    }
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
