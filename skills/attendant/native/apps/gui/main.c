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
#include "reveal.h"
#include "inspect.h"
__declspec(dllimport) LPWSTR *WINAPI CommandLineToArgvW(LPCWSTR, int *);

typedef enum gui_action { GUI_ACTION_NONE, GUI_ACTION_CLOSE, GUI_ACTION_RELOAD, GUI_ACTION_EXIT } gui_action;
typedef struct gui_app {
    gui_workspace workspace;
    char open_path[GUI_PATH];
    gui_reveal hint, pending_hint;
    gui_inspection *inspection;
    int inspection_busy, render_failed;
    int pending_hint_ready, scroll_hint, marker_valid, hint_path_ok;
    uint64_t marker_revision, preview_interaction;
    char hint_notice[256];
    int request_exit, shown_activation, bound_reached, exit_cancelled;
    gui_action pending_action;
    int pending_tab, pending_open, composer_active, composer_open, composer_tab, feedback_filter;
    int composer_start, composer_end, composer_from_preview;
    char *composer_preview_text, *composer_preview_context;
    char comment[64 * 1024];
    DWORD last_poll;
    ImFont *source_font;
#ifdef ATTENDANT_GUI_TESTING
#include "../../tests/gui/app-state.inc"
#endif
} gui_app;
static gui_app *application;
static void link_clicked(const char *url, void *user) { gui_open_link(&((gui_app *)user)->workspace, url); }
static LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    if (message == WM_COPYDATA && application && lparam) {
        const COPYDATASTRUCT *copy = (const COPYDATASTRUCT *)lparam;
        if (copy->dwData == GUI_INSPECT_REQUEST) {
            uintptr_t result;
            if (!application->inspection) application->inspection = VirtualAlloc(NULL, sizeof(*application->inspection), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            result = gui_inspection_receive(window, (HWND)wparam, &application->workspace,
                application->inspection_busy, application->inspection, copy->lpData, copy->cbData);
#ifdef ATTENDANT_GUI_TESTING
            if (result == GUI_INSPECT_RESULT) application->inspection_requests++;
#endif
            return (LRESULT)result;
        }
        return (LRESULT)gui_reveal_receive(&application->workspace, &application->pending_hint,
            &application->pending_hint_ready, (uintptr_t)copy->dwData, copy->lpData, copy->cbData);
    }
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
#include "../../tests/gui/driver.h"
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
static void agent_hint_bar(gui_app *app) {
    ImVec2 size = {0, 30}, zero = {0, 0};
    igBeginChild_Str("agent-location", size, 0, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    if (!app->hint.version) show_text("Agent location hint: none");
    else {
        char label[GUI_PATH + 128];
        const char *state = app->hint_path_ok ? " [not opened]" : " [path unavailable]";
        int i;
        for (i = 0; i < GUI_TABS; i++) if (app->workspace.tabs[i].used && !_stricmp(app->hint.path, app->workspace.tabs[i].path)) {
            if (app->hint_path_ok) state = gui_reveal_matches(&app->hint, &app->workspace.tabs[i]) ? "" : " [stale/dirty]"; break;
        }
        if (igButton("Go to agent hint", zero)) {
            app->hint_notice[0] = 0; app->scroll_hint = 0;
#ifdef ATTENDANT_GUI_TESTING
            app->reveal_go++;
#endif
            app->hint_path_ok = gui_reveal_ordinary_path(app->workspace.root) && gui_reveal_ordinary_path(app->hint.path);
            if (!app->hint_path_ok) snprintf(app->hint_notice, sizeof(app->hint_notice), "Agent path unavailable/reparse; no document opened or location applied.");
            else if (gui_open_document(&app->workspace, app->hint.path)) {
                gui_tab *tab = &app->workspace.tabs[app->workspace.active];
                if (gui_reveal_matches(&app->hint, tab) && gui_reveal_has_line(tab->text, app->hint.line)) app->scroll_hint = 1;
                else snprintf(app->hint_notice, sizeof(app->hint_notice), "Agent hint is stale/dirty or line unavailable; no highlight/scroll applied.");
            }
        }
#ifdef ATTENDANT_GUI_TESTING
        app->hint_go_position = item_center();
#endif
        igSameLine(0, -1);
        if (igButton("Dismiss hint", zero)) { app->hint.version = 0; app->scroll_hint = 0; }
        igSameLine(0, -1);
        snprintf(label, sizeof(label), "Agent hint: %s:%lu%s", file_name(app->hint.path), (unsigned long)app->hint.line, state);
        show_text(*app->hint_notice ? app->hint_notice : label);
        if (igIsItemHovered(0)) igSetTooltip("%s", app->hint.path);
    }
    igEndChild();
}
static void agent_marker(gui_app *app, gui_tab *tab, ImGuiID widget) {
    crmd_text_view view;
    ImVec2 minimum, maximum, clip_minimum, clip_maximum;
    float y;
    int matching = app->hint.version && !_stricmp(app->hint.path, tab->path);
    if (!matching) return;
    if (!app->hint_path_ok || !gui_reveal_matches(&app->hint, tab)) { app->scroll_hint = 0; return; }
    if (app->marker_revision != tab->text_revision) {
        app->marker_revision = tab->text_revision;
        app->marker_valid = gui_reveal_has_line(tab->text, app->hint.line);
    }
    if (!app->marker_valid || !crmd_get_text_view(widget, &view, sizeof(view))) { app->scroll_hint = 0; return; }
    if (app->scroll_hint) {
        /* Explicit human Go only. Scroll window; never move caret/selection. */
        igSetScrollY_WindowPtr((ImGuiWindow *)view.window, (app->hint.line - 1) * view.line_height);
        app->scroll_hint = 0;
    }
    y = view.text_y + (app->hint.line - 1) * view.line_height;
    if (y + view.line_height <= view.clip_min_y || y >= view.clip_max_y) return;
    clip_minimum.x = view.clip_min_x; clip_minimum.y = view.clip_min_y;
    clip_maximum.x = view.clip_max_x; clip_maximum.y = view.clip_max_y;
    minimum.x = view.clip_min_x + 1; minimum.y = y;
    maximum.x = view.clip_max_x - 1; maximum.y = y + view.line_height;
    ImDrawList_PushClipRect((ImDrawList *)view.draw_list, clip_minimum, clip_maximum, true);
    ImDrawList_AddRect((ImDrawList *)view.draw_list, minimum, maximum, 0xff33adffu, 0, 1.0f, 0);
    maximum.x = minimum.x + 2;
    ImDrawList_AddRectFilled((ImDrawList *)view.draw_list, minimum, maximum, 0xff33adffu, 0, 0);
    ImDrawList_PopClipRect((ImDrawList *)view.draw_list);
#ifdef ATTENDANT_GUI_TESTING
    app->marker_draws++;
#endif
}
static void document_panel(gui_app *app, int index) {
    gui_tab *tab = &app->workspace.tabs[index];
    gui_selection selection;
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
    selection = gui_selection_current(&app->workspace);
    igBeginDisabled((selection.status != GUI_SELECTION_EXACT && selection.status != GUI_SELECTION_MAPPED && selection.status != GUI_SELECTION_CONTEXT) || selection.tab != index);
    if (igButton("Comment on selection", zero)) {
        int ready = 1;
        if (app->composer_preview_text) VirtualFree(app->composer_preview_text, 0, MEM_RELEASE);
        app->composer_preview_text = app->composer_preview_context = NULL;
        if (selection.view == GUI_SELECTION_PREVIEW) {
            size_t text_length = app->workspace.preview_text_length, context_length = app->workspace.preview_context_length;
            char *copy = VirtualAlloc(NULL, text_length + context_length + 2, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            ready = copy && app->workspace.preview_text && app->workspace.preview_context;
            if (ready) {
                memcpy(copy, app->workspace.preview_text, text_length + 1); memcpy(copy + text_length + 1, app->workspace.preview_context, context_length + 1);
                app->composer_preview_text = copy; app->composer_preview_context = copy + text_length + 1;
            } else { if (copy) VirtualFree(copy, 0, MEM_RELEASE); snprintf(app->workspace.error, sizeof(app->workspace.error), "Cannot freeze preview selection context"); }
        }
        if (ready) {
            app->composer_tab = index; app->composer_active = app->composer_open = 1; app->comment[0] = 0;
            app->composer_start = selection.start; app->composer_end = selection.end; app->composer_from_preview = selection.view == GUI_SELECTION_PREVIEW;
            app->workspace.notice[0] = 0;
        }
    }
#ifdef ATTENDANT_GUI_TESTING
    app->comment_position = item_center();
#endif
    igEndDisabled(); igSameLine(0, -1);
    show_text(selection.status == GUI_SELECTION_UNSUPPORTED ? "Selection capture unavailable; see error." : selection.status == GUI_SELECTION_NONE ? "Select text in source or preview." : selection.view == GUI_SELECTION_PREVIEW ? (selection.status == GUI_SELECTION_CONTEXT ? "Preview selection (context only)" : "Preview selection") : "Source selection");
    if (!app->composer_active && app->pending_action == GUI_ACTION_NONE && igGetIO_Nil()->KeyCtrl && igIsKeyPressed_Bool(ImGuiKey_S, false) && tab->dirty)
        gui_save_document(&app->workspace, index);
    remaining = igGetContentRegionAvail();
    editor.x = remaining.x * 0.49f; editor.y = remaining.y;
    /* Hash literal path bytes; string IDs interpret "###" specially. */
    igPushOverrideID(igImHashData(tab->path, strlen(tab->path), igGetID_Str("source-path")));
    snprintf(editor_id, sizeof(editor_id), "##source-%d-%d", index, tab->generation);
    igPushFont(app->source_font, 16.0f);
    if (tab->reveal_selection) igSetKeyboardFocusHere(0);
    if (igInputTextMultiline(editor_id, tab->text, GUI_TEXT + 1, editor,
        ImGuiInputTextFlags_CallbackAlways | ImGuiInputTextFlags_AllowTabInput, source_selection, tab)) {
        tab->length = strlen(tab->text); tab->dirty = 1; tab->anchor_stale = 0; tab->text_revision = ++app->workspace.text_revision;
    }
    if (igIsItemActive()) gui_selection_capture(&app->workspace, index, GUI_SELECTION_SOURCE, GUI_SELECTION_EXACT, tab->selection_start, tab->selection_end);
#ifdef ATTENDANT_GUI_TESTING
    { ImVec2 position = igGetItemRectMin(); app->editor_position.x = position.x + 30; app->editor_position.y = position.y + 12; }
#endif
    agent_marker(app, tab, igGetID_Str(editor_id));
#ifdef ATTENDANT_GUI_TESTING
    reveal_observe(app, tab, igGetID_Str(editor_id));
#endif
    igPopFont(); igPopID();
    igSameLine(0, -1);
    igBeginChild_Str("preview", zero, ImGuiChildFlags_Borders, 0);
    {
        crmd_preview_selection selected = {0};
        size_t offset = gui_body_offset(tab->text), length = tab->length - offset;
        int exact, rendered;
        uint64_t interaction;
        igBeginChild_Str("preview-scroll", zero, 0, 0);
#ifdef ATTENDANT_GUI_TESTING
        app->preview_position = igGetCursorScreenPos(); app->preview_position.x += 2; app->preview_position.y += 10;
#endif
#ifdef ATTENDANT_GUI_TESTING
        if (app->selection_exercise == 14 && app->test_frame == 22) rendered = crmd_check_render_failure();
        else
#endif
        rendered = crmd_render_preview(tab->path, tab->text_revision, tab->text + offset, length, &selected, sizeof(selected));
        if (crmd_render_failed()) { app->render_failed = 1; return; }
        if (!rendered) snprintf(app->workspace.error, sizeof(app->workspace.error), "%s", crmd_error());
        exact = rendered && selected.has_source_span && selected.begin < selected.end && selected.end <= length;
        interaction = crmd_selection_interaction();
        if (interaction != app->preview_interaction || gui_selection_current(&app->workspace).view == GUI_SELECTION_PREVIEW) {
            if (rendered) gui_selection_capture_preview(&app->workspace, index,
                exact ? (int)(offset + selected.begin) : 0, exact ? (int)(offset + selected.end) : 0,
                selected.text, selected.text_length, selected.context, selected.context_length);
            else gui_selection_capture(&app->workspace, index, GUI_SELECTION_PREVIEW, GUI_SELECTION_UNSUPPORTED, 0, 0);
        }
        app->preview_interaction = interaction;
#ifdef ATTENDANT_GUI_TESTING
        selection_observe(app, tab, offset, exact);
#endif
        igEndChild();
    }
    igEndChild();
}
static void draw_dialogs(gui_app *app) {
    gui_workspace *workspace = &app->workspace;
    ImVec2 zero = {0, 0}, selected_size = {660, 80}, comment_size = {660, 150};
    const ImVec4 comment_border = {0.55f, 0.58f, 0.65f, 1.0f};
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
        if (app->composer_from_preview) show_text(end > start ? "Preview selection; enclosing source span may include Markdown formatting." : "Preview selection; source position unavailable. Rendered sections/ranges will accompany note.");
        show_text(app->composer_from_preview ? "Selected preview:" : "Selected source:");
        igBeginChild_Str("selected source", selected_size, ImGuiChildFlags_Borders, 0);
        if (app->composer_from_preview && app->composer_preview_text) show_text(app->composer_preview_text);
        else if (start >= 0 && end >= start && (size_t)end <= tab->length) igTextUnformatted(tab->text + start, tab->text + end);
        igEndChild();
        if (app->composer_from_preview && app->composer_preview_context && igCollapsingHeader_TreeNodeFlags("Rendered sections and highlight ranges", 0)) {
            ImVec2 context_size = {660, 120};
            igBeginChild_Str("preview-context", context_size, ImGuiChildFlags_Borders, 0);
            igPushTextWrapPos(0); show_text(app->composer_preview_context); igPopTextWrapPos(); igEndChild();
        }
        show_text("Comment:");
        if (igIsWindowAppearing()) igSetKeyboardFocusHere(0);
        igPushStyleVar_Float(ImGuiStyleVar_FrameBorderSize, 1.0f);
        igPushStyleColor_Vec4(ImGuiCol_Border, comment_border);
        igInputTextMultiline("##comment", app->comment, sizeof(app->comment), comment_size, 0, NULL, NULL);
#ifdef ATTENDANT_GUI_TESTING
        { ImVec2 minimum = igGetItemRectMin(), maximum = igGetItemRectMax();
          app->comment_rect.x = minimum.x; app->comment_rect.y = minimum.y; app->comment_rect.z = maximum.x; app->comment_rect.w = maximum.y; }
#endif
        igPopStyleColor(1); igPopStyleVar(1);
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
                if (app->composer_from_preview ? gui_send_preview_comment(workspace, app->composer_tab, app->composer_start, app->composer_end, app->composer_preview_context, app->comment) :
                    gui_send_comment(workspace, app->composer_tab, app->composer_start, app->composer_end, app->comment)) {
                    app->composer_active = 0; igCloseCurrentPopup();
                    if (app->composer_preview_text) VirtualFree(app->composer_preview_text, 0, MEM_RELEASE);
                    app->composer_preview_text = app->composer_preview_context = NULL;
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
        if (igButton("Cancel", zero) || igIsKeyPressed_Bool(ImGuiKey_Escape, false)) {
            app->composer_active = 0; igCloseCurrentPopup();
            if (app->composer_preview_text) VirtualFree(app->composer_preview_text, 0, MEM_RELEASE);
            app->composer_preview_text = app->composer_preview_context = NULL;
        }
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
#ifdef ATTENDANT_GUI_TESTING
    app->issues_shown = 0;
#endif
    if (workspace->navigation && workspace->navigation->diagnostics) {
        char warning[160];
        snprintf(warning, sizeof(warning), "Attendant detected %d workspace issue(s). Some collections/records may be missing or invalid.", workspace->navigation->diagnostics);
        show_text(warning);
        show_text("Run native CLI: doctor -p <workspace> for details (read-only). Fix reported issues, then Refresh files.");
#ifdef ATTENDANT_GUI_TESTING
        app->issues_shown = workspace->navigation->diagnostics;
#endif
    }
    if (*workspace->error) {
        show_text(workspace->error); igSameLine(0, -1);
        if (igButton("Dismiss error", zero)) { workspace->error[0] = workspace->failed_path[0] = 0; workspace->surviving_count = workspace->surviving_total = 0; }
    }
    recovery_details(workspace);
    if (*workspace->notice) show_text(workspace->notice);
    agent_hint_bar(app);
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
                if (app->shown_activation == workspace->activation && (forced < 0 || forced == i)) {
                    if (workspace->active != i) gui_selection_clear(workspace);
                    workspace->active = i;
                }
                document_panel(app, i); if (app->render_failed) return; igEndTabItem();
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
static int usage(FILE *output, int code) {
    fprintf(output, "Usage: attendant-gui --project DIR --assets DIR [--file PATH] [--frames 1..6000 --shot FILE.ppm]\n       attendant-gui --project DIR --reveal FILE [--line N]\n       attendant-gui --project DIR --inspect\n");
    if (code) gui_reveal_error(code, "usage", "Expected GUI startup, --project DIR --inspect, or --project DIR --reveal FILE [--line N]; control cannot combine startup/test flags");
    return code;
}
int main(int argc, char **argv) {
    const char *project = NULL, *assets = NULL, *initial = NULL, *shot = NULL, *reveal = NULL;
    uint32_t reveal_line = 1;
    int line_supplied = 0, inspect = 0;
    char *owned[32] = {0};
    LPWSTR *arguments;
    int frame_limit = 0, frame = 0, code = 1, i, gui_ready = 0;
#ifdef ATTENDANT_GUI_TESTING
    int exercise = 0, edit_exercise = 0, save_fail_exercise = 0, preview_exercise = 0, reveal_exercise = 0, selection_exercise = 0;
#endif
    bool running = true;
    HWND window = NULL;
    WNDCLASSA description;
    ImGuiContext *context = NULL;
    MSG message;
    arguments = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!arguments) return gui_reveal_error(1, "argument-conversion", "Cannot decode Windows arguments");
    if (argc > 32) { LocalFree(arguments); return usage(stderr, 2); }
    for (i = 0; i < argc; i++) {
        int j, size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, arguments[i], -1, NULL, 0, NULL, NULL);
        if (size) owned[i] = HeapAlloc(GetProcessHeap(), 0, size);
        if (!size || !owned[i] || !WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, arguments[i], -1, owned[i], size, NULL, NULL)) {
            LocalFree(arguments);
            for (j = 0; j <= i; j++) if (owned[j]) HeapFree(GetProcessHeap(), 0, owned[j]);
            return gui_reveal_error(size ? 1 : 2, size ? "capacity" : "invalid-arguments", "Cannot allocate/decode valid Unicode arguments");
        }
    }
    LocalFree(arguments); argv = owned;
    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--help")) return usage(stdout, 0);
#ifdef ATTENDANT_GUI_TESTING
#include "../../tests/gui/options.inc"
#endif
        if (!strcmp(argv[i], "--project") && i + 1 < argc) project = argv[++i];
        else if (!strcmp(argv[i], "--assets") && i + 1 < argc) assets = argv[++i];
        else if (!strcmp(argv[i], "--file") && i + 1 < argc) initial = argv[++i];
        else if (!strcmp(argv[i], "--shot") && i + 1 < argc) shot = argv[++i];
        else if (!strcmp(argv[i], "--reveal") && i + 1 < argc) reveal = argv[++i];
        else if (!strcmp(argv[i], "--inspect")) inspect = 1;
        else if (!strcmp(argv[i], "--line") && i + 1 < argc) {
            char *end; const char *value = argv[++i]; unsigned long number = strtoul(value, &end, 10);
            if (*value < '0' || *value > '9' || *end || !number || number > GUI_TEXT + 1u) return usage(stderr, 2);
            reveal_line = (uint32_t)number; line_supplied = 1;
        }
        else if (!strcmp(argv[i], "--frames") && i + 1 < argc) {
            char *end; long value = strtol(argv[++i], &end, 10);
            if (*end || value < 1 || value > 6000) return usage(stderr, 2);
            frame_limit = (int)value;
        } else return usage(stderr, 2);
    }
    if (reveal || inspect) {
        if (!project || (reveal && inspect) || (inspect && line_supplied) || assets || initial || shot || frame_limit) return usage(stderr, 2);
#ifdef ATTENDANT_GUI_TESTING
        if (exercise || edit_exercise || preview_exercise || reveal_exercise || selection_exercise) return usage(stderr, 2);
#endif
        code = inspect ? gui_inspect(project) : gui_reveal_send(project, reveal, reveal_line);
        for (i = 0; i < argc; i++) HeapFree(GetProcessHeap(), 0, owned[i]);
        return code;
    }
    if (!project || !assets || line_supplied || (shot && !frame_limit)) return usage(stderr, 2);
    application = VirtualAlloc(NULL, sizeof(*application), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!application) return 3;
    application->shown_activation = -1;
#ifdef ATTENDANT_GUI_TESTING
#include "../../tests/gui/configure.inc"
#endif
    if (!gui_workspace_init(&application->workspace, project)) { fprintf(stderr, "%s\n", application->workspace.error); goto cleanup; }
    if (initial) gui_open_document(&application->workspace, initial);
#ifdef ATTENDANT_GUI_TESTING
#include "../../tests/gui/ownership.inc"
#endif
    memset(&description, 0, sizeof(description)); description.lpfnWndProc = window_proc;
    description.hInstance = GetModuleHandleA(NULL); description.lpszClassName = GUI_WINDOW_CLASS;
    description.hCursor = LoadCursorA(NULL, IDC_ARROW);
    if (!RegisterClassA(&description)) goto cleanup;
    context = igCreateContext(NULL); igGetIO_Nil()->IniFilename = NULL; igStyleColorsDark(NULL);
#ifdef ATTENDANT_GUI_TESTING
    if (selection_exercise) igGetIO_Nil()->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
#endif
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
#ifdef ATTENDANT_GUI_TESTING
        /* Owned mutable fixtures stop at bound; final checks reject incomplete work. */
        if ((reveal_exercise || edit_exercise || preview_exercise || selection_exercise) && frame_limit && frame >= frame_limit) break;
#endif
        if (frame_limit && frame >= frame_limit && !application->exit_cancelled) { application->bound_reached = 1; application->request_exit = 1; }
        if (!running || (application->request_exit && !dirty_documents(&application->workspace) && !application->composer_active && application->pending_action == GUI_ACTION_NONE)) break;
        if (!application->composer_active && application->pending_action == GUI_ACTION_NONE && GetTickCount() - application->last_poll > 1000) {
            if (application->workspace.active >= 0) gui_check_external(&application->workspace, application->workspace.active);
            application->last_poll = GetTickCount();
        }
#ifdef ATTENDANT_GUI_TESTING
        exercise_input(application, window, frame);
#endif
        application->inspection_busy = 1;
        if (!crmd_begin_frame()) goto cleanup;
        /* Later human navigation cancels an unrendered Go; incoming hints do not. */
        if (application->scroll_hint && (application->workspace.active < 0 ||
            (application->shown_activation == application->workspace.activation &&
             _stricmp(application->hint.path, application->workspace.tabs[application->workspace.active].path)))) application->scroll_hint = 0;
        /* Do not retarget a Go click/key or its queued scroll. Update after release. */
        if (application->pending_hint_ready && !application->scroll_hint && !igIsAnyMouseDown() && !igIsMouseReleased_Nil(0) &&
            !igIsKeyDown_Nil(ImGuiKey_Enter) && !igIsKeyReleased_Nil(ImGuiKey_Enter) &&
            !igIsKeyDown_Nil(ImGuiKey_Space) && !igIsKeyReleased_Nil(ImGuiKey_Space)) {
            memcpy(&application->hint, &application->pending_hint, sizeof(application->hint));
            application->pending_hint_ready = 0; application->marker_revision = 0;
            application->marker_valid = application->scroll_hint = 0; application->hint_notice[0] = 0;
            application->hint_path_ok = gui_reveal_ordinary_path(application->workspace.root) && gui_reveal_ordinary_path(application->hint.path);
        }
        draw_workspace(application);
        if (application->render_failed) {
            fprintf(stderr, "Render failure: frame abandoned without normal teardown. %s\n", crmd_error());
#ifndef ATTENDANT_GUI_TESTING
            MessageBoxA(window, "Markdown renderer failed. Workspace will close without saving unsaved buffers. No source changes were made by this failure.", "Attendant render failure", MB_OK | MB_ICONERROR);
#endif
            goto cleanup;
        }
#ifdef ATTENDANT_GUI_TESTING
        if (application->reveal_failed) goto cleanup;
#endif
        if (!crmd_end_frame()) goto cleanup;
        application->inspection_busy = 0;
        ++frame;
        if (shot && frame == frame_limit && !crmd_screenshot(shot)) goto cleanup;
        if (!crmd_present()) goto cleanup;
    }
#ifdef ATTENDANT_GUI_TESTING
#include "../../tests/gui/complete.inc"
#endif
    code = 0;
cleanup:
    if (gui_ready && !crmd_shutdown()) code = 1;
    if (crmd_live_images()) code = 1;
    if (window) DestroyWindow(window);
    if (context) igDestroyContext(context);
    if (!code) {
        printf("{\"ok\":true,\"frames\":%d,\"tabs\":%d,\"vertices\":%d", frame, application->workspace.tab_count, crmd_vertex_count());
#ifdef ATTENDANT_GUI_TESTING
        printf(",\"issues_shown\":%d", application->issues_shown);
#endif
        puts("}");
    } else fprintf(stderr, "GUI initialization, exercise, rendering or cleanup failed: %s\n", crmd_error());
    if (application->composer_preview_text) VirtualFree(application->composer_preview_text, 0, MEM_RELEASE);
    if (application->inspection) VirtualFree(application->inspection, 0, MEM_RELEASE);
    gui_workspace_destroy(&application->workspace); VirtualFree(application, 0, MEM_RELEASE); application = NULL;
    UnregisterClassA(GUI_WINDOW_CLASS, GetModuleHandleA(NULL));
    for (i = 0; i < argc; i++) HeapFree(GetProcessHeap(), 0, owned[i]);
    return code;
}
