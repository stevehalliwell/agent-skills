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
    int inspection_busy;
    int pending_hint_ready, scroll_hint, marker_valid, hint_path_ok;
    uint64_t marker_revision, preview_interaction;
    char hint_notice[256];
    int request_exit, shown_activation, bound_reached, exit_cancelled;
    gui_action pending_action;
    int pending_tab, pending_open, composer_active, composer_open, composer_tab, feedback_filter;
    int composer_start, composer_end, composer_from_preview;
    char comment[64 * 1024];
    DWORD last_poll;
    ImFont *source_font;
#ifdef ATTENDANT_GUI_TESTING
    int exercise, reference_clicks, edit_exercise, save_fail_exercise, cancel_clicks, sent_comments, target_clicks, undo_seen, saved_not_queued;
    int preview_exercise, preview_exact_seen, preview_shared_seen, preview_dirty_send, inspection_requests, inspection_checked;
    int selection_exercise, selection_ready, selection_source_seen, selection_action_checked;
    int reveal_exercise, test_frame, reveal_phase, reveal_frame, reveal_go, reveal_failed, reveal_checks, marker_draws, reparse_go_armed;
    int before_active, before_tabs, before_activation, before_start, before_end;
    uint64_t before_revision;
    ImGuiID before_widget;
    HWND before_focus, before_foreground;
    crmd_text_view before_view;
    ImVec2 before_editor, hint_go_position;
    ImVec2 preview_position;
    ImVec2 first_position, depends_position, editor_position, comment_position, send_position, cancel_position, composer_cancel_position, feedback_position, target_position;
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
#ifdef ATTENDANT_GUI_TESTING
static void reveal_observe(gui_app *app, gui_tab *tab, ImGuiID widget) {
    crmd_text_view view;
    ImVec2 editor = igGetItemRectMin();
#define REVEAL_CHECK(test) do { app->reveal_checks++; if (!(test)) { fprintf(stderr, "Reveal check failed: %s\n", #test); app->reveal_failed = 1; return; } } while (0)
    if (!app->reveal_exercise || app->test_frame < 16 || app->reveal_phase == 3) return;
    if (app->reveal_exercise == 4) {
        if (!app->reveal_phase) { app->reveal_phase = 1; puts("{\"event\":\"ready\"}"); fflush(stdout); }
        return;
    }
    REVEAL_CHECK(crmd_get_text_view(widget, &view, sizeof(view)));
    if (!app->reveal_phase) {
        REVEAL_CHECK(view.active_id == widget && view.caret >= 0);
        REVEAL_CHECK(tab->selection_start == 0 && (size_t)tab->selection_end == tab->length);
        REVEAL_CHECK(view.scroll_y > 100);
        app->before_view = view; app->before_editor = editor; app->before_widget = widget;
        app->before_active = app->workspace.active; app->before_tabs = app->workspace.tab_count; app->before_activation = app->workspace.activation;
        app->before_start = tab->selection_start; app->before_end = tab->selection_end; app->before_revision = tab->text_revision;
        app->before_focus = GetFocus(); app->before_foreground = GetForegroundWindow();
        app->reveal_phase = 1; puts("{\"event\":\"ready\"}"); fflush(stdout); return;
    }
    if (app->reveal_phase == 1 && app->inspection_requests && !app->inspection_checked) {
        REVEAL_CHECK(app->workspace.active == app->before_active && app->workspace.tab_count == app->before_tabs && app->workspace.activation == app->before_activation);
        REVEAL_CHECK(view.active_id == app->before_view.active_id && GetFocus() == app->before_focus && GetForegroundWindow() == app->before_foreground);
        REVEAL_CHECK(view.scroll_y == app->before_view.scroll_y && editor.x == app->before_editor.x && editor.y == app->before_editor.y);
        REVEAL_CHECK(view.caret == app->before_view.caret && view.selection_start == app->before_view.selection_start && view.selection_end == app->before_view.selection_end);
        REVEAL_CHECK(tab->selection_start == app->before_start && tab->selection_end == app->before_end && tab->text_revision == app->before_revision);
        app->inspection_checked = 1; puts("{\"event\":\"inspected\",\"documentSelectionScrollFocusPreserved\":true}"); fflush(stdout);
    }
    if (app->reveal_phase == 1 && app->hint.version) {
        REVEAL_CHECK(app->workspace.active == app->before_active && app->workspace.tab_count == app->before_tabs && app->workspace.activation == app->before_activation);
        REVEAL_CHECK(view.active_id == app->before_view.active_id && GetFocus() == app->before_focus && GetForegroundWindow() == app->before_foreground);
        REVEAL_CHECK(view.scroll_y == app->before_view.scroll_y && editor.x == app->before_editor.x && editor.y == app->before_editor.y);
        REVEAL_CHECK(view.caret == app->before_view.caret && view.selection_start == app->before_view.selection_start && view.selection_end == app->before_view.selection_end);
        REVEAL_CHECK(tab->selection_start == app->before_start && tab->selection_end == app->before_end && tab->text_revision == app->before_revision);
        REVEAL_CHECK((gui_reveal_matches(&app->hint, tab) != 0) == (app->reveal_exercise == 1));
        REVEAL_CHECK(!app->marker_draws && !app->scroll_hint);
        app->reveal_frame = app->test_frame; app->reveal_phase = 2;
        puts("{\"event\":\"received\",\"documentSelectionScrollFocusPreserved\":true}"); fflush(stdout);
    }
    if (app->reveal_phase == 2 && app->reveal_go && app->test_frame >= app->reveal_frame + 8) {
        gui_tab *original = &app->workspace.tabs[app->before_active];
        ImGuiInputTextState *state = igGetInputTextState(app->before_widget);
        REVEAL_CHECK(original->selection_start == app->before_start && original->selection_end == app->before_end && original->text_revision == app->before_revision);
        REVEAL_CHECK(state && ImGuiInputTextState_GetCursorPos(state) == app->before_view.caret &&
            ImGuiInputTextState_GetSelectionStart(state) == app->before_view.selection_start && ImGuiInputTextState_GetSelectionEnd(state) == app->before_view.selection_end);
        if (app->reveal_exercise == 5) {
            REVEAL_CHECK(app->workspace.active != app->before_active && app->workspace.tab_count == app->before_tabs + 1);
            REVEAL_CHECK(tab->selection_start == 0 && tab->selection_end == 0 && view.caret == -1);
            REVEAL_CHECK(view.scroll_y > 0 && app->marker_draws > 0);
        } else if (app->reveal_exercise == 1) REVEAL_CHECK(view.scroll_y < app->before_view.scroll_y && app->marker_draws > 0);
        else REVEAL_CHECK(view.scroll_y == app->before_view.scroll_y && !app->marker_draws);
        if (app->reveal_exercise == 6) REVEAL_CHECK(!app->hint_path_ok && app->workspace.active == app->before_active && app->workspace.tab_count == app->before_tabs);
        app->reveal_phase = 3;
        printf("{\"event\":\"complete\",\"checks\":%d,\"markerDraws\":%d,\"dirty\":%s,\"caretSelectionPreservedAfterGo\":true,\"sourceClip\":[%.3f,%.3f,%.3f,%.3f]}\n", app->reveal_checks, app->marker_draws, tab->dirty ? "true" : "false", view.clip_min_x, view.clip_min_y, view.clip_max_x, view.clip_max_y); fflush(stdout);
    }
#undef REVEAL_CHECK
}
#endif
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
    igBeginDisabled(selection.status != GUI_SELECTION_EXACT || selection.tab != index);
    if (igButton("Comment on selection", zero)) {
        app->composer_tab = index; app->composer_active = app->composer_open = 1; app->comment[0] = 0;
        app->composer_start = selection.start; app->composer_end = selection.end; app->composer_from_preview = selection.view == GUI_SELECTION_PREVIEW;
        app->workspace.notice[0] = 0;
    }
#ifdef ATTENDANT_GUI_TESTING
    app->comment_position = item_center();
#endif
    igEndDisabled(); igSameLine(0, -1);
    show_text(selection.status == GUI_SELECTION_UNSUPPORTED ? "Selection unsupported; use source." : selection.status != GUI_SELECTION_EXACT ? "Select text in source or preview." : selection.view == GUI_SELECTION_PREVIEW ? "Preview selection" : "Source selection");
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
    if (igIsItemActive()) gui_selection_capture(&app->workspace, index, GUI_SELECTION_SOURCE, GUI_SELECTION_EXACT, tab->selection_start, tab->selection_end);
#ifdef ATTENDANT_GUI_TESTING
    { ImVec2 position = igGetItemRectMin(); app->editor_position.x = position.x + 30; app->editor_position.y = position.y + 12; }
#endif
    agent_marker(app, tab, igGetID_Str(editor_id));
#ifdef ATTENDANT_GUI_TESTING
    reveal_observe(app, tab, igGetID_Str(editor_id));
#endif
    igPopFont();
    igSameLine(0, -1);
    igBeginChild_Str("preview", zero, ImGuiChildFlags_Borders, 0);
    {
        crmd_source_selection selected = {0};
        size_t offset = gui_body_offset(tab->text), length = tab->length - offset;
        int exact, rendered;
        uint64_t interaction;
        igBeginChild_Str("preview-scroll", zero, 0, 0);
#ifdef ATTENDANT_GUI_TESTING
        app->preview_position = igGetCursorScreenPos(); app->preview_position.x += 2; app->preview_position.y += 10;
#endif
        rendered = crmd_render_raw(tab->path, tab->text_revision, tab->text + offset, length, &selected);
        if (!rendered) snprintf(app->workspace.error, sizeof(app->workspace.error), "%s", crmd_error());
        exact = rendered && selected.status == CRMD_SELECTION_EXACT && selected.begin < selected.end && selected.end <= length;
        interaction = rendered ? crmd_selection_interaction() : app->preview_interaction;
        if (interaction != app->preview_interaction || gui_selection_current(&app->workspace).view == GUI_SELECTION_PREVIEW) {
            gui_selection_capture(&app->workspace, index, GUI_SELECTION_PREVIEW,
                exact ? GUI_SELECTION_EXACT : !rendered || selected.status == CRMD_SELECTION_UNSUPPORTED ? GUI_SELECTION_UNSUPPORTED : GUI_SELECTION_NONE,
                exact ? (int)(offset + selected.begin) : 0, exact ? (int)(offset + selected.end) : 0);
        }
        app->preview_interaction = interaction;
#ifdef ATTENDANT_GUI_TESTING
        if (app->selection_exercise && app->test_frame >= 34 && !app->selection_ready) {
            gui_selection current = gui_selection_current(&app->workspace);
            if (!app->selection_source_seen || current.view != GUI_SELECTION_PREVIEW || current.status != (app->selection_exercise == 1 ? GUI_SELECTION_EXACT : GUI_SELECTION_UNSUPPORTED)) {
                fprintf(stderr, "Preview context/widget did not replace older source selection: mode %d, view %d, status %d\n", app->selection_exercise, current.view, current.status);
                app->reveal_failed = 1;
            } else { app->selection_ready = 1; puts("{\"event\":\"selection-ready\"}"); fflush(stdout); }
        }
        if (exact) app->preview_exact_seen = 1;
        if (exact && gui_selection_current(&app->workspace).view == GUI_SELECTION_PREVIEW &&
            gui_selection_current(&app->workspace).start == (int)offset && (size_t)gui_selection_current(&app->workspace).end == tab->length) app->preview_shared_seen = 1;
#endif
        igEndChild();
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
    app->test_frame = frame;
    if (app->selection_exercise) {
        ImGuiIO *io = igGetIO_Nil();
        if (frame == 16) app->selection_source_seen = gui_selection_current(&app->workspace).view == GUI_SELECTION_SOURCE && gui_selection_current(&app->workspace).status == GUI_SELECTION_EXACT;
        position = frame < 18 ? app->editor_position : app->preview_position;
        if (frame >= 18) { position.x += 40; if (app->selection_exercise == 3) position.y += 55; }
        /* Focus neutral project heading without replacing latest document selection. */
        if (frame >= 14 && frame <= 17) { position.x = 12; position.y = 12; }
        if (frame >= 40 && frame <= 41) position = app->comment_position;
        if (frame >= 48 && frame <= 49) position = app->composer_cancel_position;
        crmd_message(window, WM_MOUSEMOVE, 0, MAKELPARAM((int)position.x, (int)position.y));
        if (frame == 5 || frame == 6 || frame == 14 || frame == 15) ImGuiIO_AddMouseButtonEvent(io, 0, frame == 5 || frame == 14);
        if (frame == 8 || frame == 10) ImGuiIO_AddKeyEvent(io, ImGuiMod_Ctrl, frame == 8);
        if (frame == 8 || frame == 9) ImGuiIO_AddKeyEvent(io, ImGuiKey_A, frame == 8);
        if (frame == 40 || frame == 41 || frame == 48 || frame == 49)
            ImGuiIO_AddMouseButtonEvent(io, 0, frame == 40 || frame == 48);
        if (frame == 46) {
            gui_tab *tab = &app->workspace.tabs[app->workspace.active];
            int expected = app->selection_exercise == 1;
            if (app->composer_active != expected || (expected && (!app->composer_from_preview || app->composer_start != 0 || (size_t)app->composer_end != tab->length || !igIsAnyItemActive()))) {
                fprintf(stderr, "Unified comment action did not follow latest preview ownership\n"); app->reveal_failed = 1;
            } else {
                app->selection_action_checked = 1;
                printf("{\"event\":\"selection-action\",\"enabled\":%s,\"olderSourceNotUsed\":true}\n", expected ? "true" : "false"); fflush(stdout);
            }
        }
        if (app->selection_exercise == 3) {
            if (frame == 20 || frame == 21) ImGuiIO_AddMouseButtonEvent(io, 0, frame == 20);
            if (frame == 24 || frame == 26) ImGuiIO_AddKeyEvent(io, ImGuiMod_Ctrl, frame == 24);
            if (frame == 24 || frame == 25) ImGuiIO_AddKeyEvent(io, ImGuiKey_A, frame == 24);
        } else {
            if (frame == 20 || frame == 21) ImGuiIO_AddMouseButtonEvent(io, 1, frame == 20);
            if (frame == 24 || frame == 25) ImGuiIO_AddKeyEvent(io, ImGuiKey_DownArrow, frame == 24);
            if (frame == 28 || frame == 29) ImGuiIO_AddKeyEvent(io, ImGuiKey_Enter, frame == 28);
        }
        return;
    }
    if (app->reveal_exercise) {
        ImGuiIO *io = igGetIO_Nil();
        if (app->reveal_exercise == 6 && app->reveal_phase == 2 && !app->reparse_go_armed) {
            if (gui_reveal_ordinary_path(app->hint.path)) return;
            app->reparse_go_armed = 1; app->reveal_frame = frame - 4;
        }
        if (frame >= 5 && frame <= 14) position = app->editor_position;
        else if (app->reveal_phase == 2 && frame >= app->reveal_frame + 4 && frame <= app->reveal_frame + 5) position = app->hint_go_position;
        else position.x = position.y = -100;
        if (position.x >= 0) crmd_message(window, WM_MOUSEMOVE, 0, MAKELPARAM((int)position.x, (int)position.y));
        if (frame == 5 || (app->reveal_phase == 2 && frame == app->reveal_frame + 4)) ImGuiIO_AddMouseButtonEvent(io, 0, true);
        if (frame == 6 || (app->reveal_phase == 2 && frame == app->reveal_frame + 5)) ImGuiIO_AddMouseButtonEvent(io, 0, false);
        if (frame == 7 || frame == 12) ImGuiIO_AddKeyEvent(io, ImGuiMod_Ctrl, true);
        if (frame == 7 || frame == 8) ImGuiIO_AddKeyEvent(io, ImGuiKey_End, frame == 7);
        if (frame == 9 || frame == 14) ImGuiIO_AddKeyEvent(io, ImGuiMod_Ctrl, false);
        if (frame == 10 && app->reveal_exercise == 2) ImGuiIO_AddInputCharactersUTF8(io, "!");
        if (frame == 12 || frame == 13) ImGuiIO_AddKeyEvent(io, ImGuiKey_A, frame == 12);
        return;
    }
    if (app->preview_exercise) {
        ImGuiIO *io = igGetIO_Nil();
        if (frame >= 5 && frame <= 16) position = app->editor_position;
        else if (frame >= 20 && frame <= 28) position = app->preview_position;
        else if (frame >= 30 && frame <= 34) position = app->comment_position;
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
        if (!strcmp(argv[i], "--exercise-selection") && i + 1 < argc) {
            const char *mode = argv[++i];
            selection_exercise = !strcmp(mode, "plain-menu") ? 1 : !strcmp(mode, "markup-menu") ? 2 : !strcmp(mode, "widget") ? 3 : 0;
            if (!selection_exercise) return usage(stderr, 2);
            continue;
        }
        if (!strcmp(argv[i], "--exercise-reveal") && i + 1 < argc) {
            const char *mode = argv[++i];
            reveal_exercise = !strcmp(mode, "clean") ? 1 : !strcmp(mode, "dirty") ? 2 : !strcmp(mode, "stale") ? 3 : !strcmp(mode, "idle") ? 4 : !strcmp(mode, "other") ? 5 : !strcmp(mode, "reparse") ? 6 : 0;
            if (!reveal_exercise) return usage(stderr, 2);
            continue;
        }
        if (!strcmp(argv[i], "--exercise")) { exercise = 1; continue; }
        if (!strcmp(argv[i], "--exercise-preview")) { preview_exercise = 1; continue; }
        if (!strcmp(argv[i], "--exercise-edit")) { edit_exercise = 1; continue; }
        if (!strcmp(argv[i], "--exercise-save-fail")) { edit_exercise = save_fail_exercise = 1; continue; }
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
    if (reveal_exercise && (exercise || edit_exercise || preview_exercise || !frame_limit)) { code = usage(stderr, 2); goto cleanup; }
    application->reveal_exercise = reveal_exercise; application->selection_exercise = selection_exercise;
    if (selection_exercise && (exercise || edit_exercise || preview_exercise || reveal_exercise || !frame_limit)) { code = usage(stderr, 2); goto cleanup; }
    application->exercise = exercise; application->edit_exercise = edit_exercise; application->save_fail_exercise = save_fail_exercise; application->preview_exercise = preview_exercise;
#endif
    if (!gui_workspace_init(&application->workspace, project)) { fprintf(stderr, "%s\n", application->workspace.error); goto cleanup; }
    if (initial) gui_open_document(&application->workspace, initial);
#ifdef ATTENDANT_GUI_TESTING
    if (edit_exercise || preview_exercise || reveal_exercise || selection_exercise) {
        char marker[GUI_PATH]; wchar_t wide[4096];
        snprintf(marker, sizeof(marker), "%s/.gui-test-owned", application->workspace.root);
        if (application->workspace.active < 0 || !MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, marker, -1, wide, 4096) || GetFileAttributesW(wide) == INVALID_FILE_ATTRIBUTES) {
            fprintf(stderr, "Editing exercise requires generated owned fixture and initial document\n"); goto cleanup;
        }
    }
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
    if (selection_exercise && (!application->selection_ready || !application->selection_action_checked)) { fprintf(stderr, "Selection exercise did not complete\n"); goto cleanup; }
    if (reveal_exercise && (reveal_exercise == 4 ? application->hint.version != 0 : application->reveal_phase != 3)) {
        fprintf(stderr, "Reveal exercise did not complete expected delivery/navigation checks\n"); goto cleanup;
    }
    if (preview_exercise) {
        gui_workspace *workspace = &application->workspace;
        gui_tab *tab = workspace->active >= 0 ? &workspace->tabs[workspace->active] : NULL;
        if (!tab || application->sent_comments != 1 || !application->preview_exact_seen || !application->preview_shared_seen || !application->preview_dirty_send || !application->composer_from_preview ||
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
    if (application->inspection) VirtualFree(application->inspection, 0, MEM_RELEASE);
    gui_workspace_destroy(&application->workspace); VirtualFree(application, 0, MEM_RELEASE); application = NULL;
    UnregisterClassA(GUI_WINDOW_CLASS, GetModuleHandleA(NULL));
    for (i = 0; i < argc; i++) HeapFree(GetProcessHeap(), 0, owned[i]);
    return code;
}
