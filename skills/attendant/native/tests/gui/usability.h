/* Owned-fixture interaction regression: real ImGui input, find focus/undo,
 * wrapped geometry, rendered provenance and collapsed-tree filtering. */
#define USABILITY_CHECK(condition) do { ++app->usability_checks; if (!(condition)) { fprintf(stderr, "Usability check failed at frame %d: %s\n", app->test_frame, #condition); app->reveal_failed = 1; return; } } while (0)
static char usability_clipboard_text[TEXT_FIND_QUERY + 1];
static const char *usability_clipboard_get(ImGuiContext *context) { (void)context; return usability_clipboard_text; }
static void usability_query(gui_app *app, int target, const char *query) {
    igClearActiveID(); memcpy(app->find[target].query, query, strlen(query) + 1); app->find[target].cached = 0; app->find[target].error = 0;
}
static void usability_input(gui_app *app, void *window, int frame) {
    ImGuiIO *io = igGetIO_Nil();
    if (app->reveal_failed) return;
    if (frame == 7) app->wrap_source = false;
    if (frame == 12 || frame == 13 || frame == 68 || frame == 69) {
        test_mouse(window, frame < 60 ? app->editor_position : app->preview_position);
        ImGuiIO_AddMouseButtonEvent(io, 0, frame == 12 || frame == 68);
    }
    if (frame == 16 || frame == 58) ImGuiIO_AddKeyEvent(io, test_shortcut_modifier(), true);
    if (frame == 24 || frame == 72) ImGuiIO_AddKeyEvent(io, ImGuiMod_Ctrl, true);
    if (frame == 16 || frame == 17) ImGuiIO_AddKeyEvent(io, ImGuiKey_A, frame == 16);
    if (frame == 24 || frame == 25 || frame == 72 || frame == 73) ImGuiIO_AddKeyEvent(io, ImGuiKey_F, frame == 24 || frame == 72);
    if (frame == 58 || frame == 59) ImGuiIO_AddKeyEvent(io, ImGuiKey_Z, frame == 58);
    if (frame == 18 || frame == 60) ImGuiIO_AddKeyEvent(io, test_shortcut_modifier(), false);
    if (frame == 26 || frame == 74) ImGuiIO_AddKeyEvent(io, ImGuiMod_Ctrl, false);
    if (frame == 19) ImGuiIO_AddInputCharactersUTF8(io, "Plan **the** plan\n\nplan other block\n");
    if (frame == 29) ImGuiIO_AddInputCharactersUTF8(io, "plan");
    if (frame == 77) ImGuiIO_AddInputCharactersUTF8(io, "the plan");
    if (frame == 38 || frame == 39 || frame == 44 || frame == 45) ImGuiIO_AddKeyEvent(io, ImGuiKey_Enter, frame == 38 || frame == 44);
    if (frame == 44 || frame == 46 || frame == 116 || frame == 118) ImGuiIO_AddKeyEvent(io, ImGuiMod_Shift, frame == 44 || frame == 116);
    if (frame == 50) app->wrap_source = true;
    if (frame == 52 || frame == 53 || frame == 134 || frame == 135) ImGuiIO_AddKeyEvent(io, ImGuiKey_Escape, frame == 52 || frame == 134);
    if (frame == 85) usability_query(app, GUI_FIND_PREVIEW, "hidden-target");
    if (frame == 91) usability_query(app, GUI_FIND_PREVIEW, "**the**");
    if (frame == 97) usability_query(app, GUI_FIND_PREVIEW, "plan");
    if (frame == 104 || frame == 105 || frame == 110 || frame == 111 || frame == 116 || frame == 117)
        ImGuiIO_AddKeyEvent(io, ImGuiKey_F3, frame == 104 || frame == 110 || frame == 116);
    if (frame == 126) { usability_query(app, GUI_FIND_SIDEBAR, "usability"); app->find[0].focus = 1; app->find_target = GUI_FIND_SIDEBAR; }
    if (frame == 141) { ImGuiIO_AddKeyEvent(io, ImGuiMod_Ctrl, true); ImGuiIO_AddKeyEvent(io, ImGuiKey_F, true); }
    if (frame == 142) ImGuiIO_AddKeyEvent(io, ImGuiKey_F, false);
    if (frame == 143) { app->find_match_case = true; ImGuiIO_AddKeyEvent(io, ImGuiMod_Ctrl, false); }
    if (frame == 144 || frame == 145) { test_mouse(window, app->sidebar_clear_position); ImGuiIO_AddMouseButtonEvent(io, 0, frame == 144); }
    if (frame == 149 || frame == 154) {
        app->divider_drag_origin = frame == 149 ? app->sidebar_divider_position : app->source_divider_position;
        test_mouse(window, app->divider_drag_origin); ImGuiIO_AddMouseButtonEvent(io, 0, true);
    }
    if (frame == 150 || frame == 155) {
        ImVec2 point = app->divider_drag_origin; point.x += frame == 150 ? 40 : -40; test_mouse(window, point);
    }
    if (frame == 151 || frame == 156) ImGuiIO_AddMouseButtonEvent(io, 0, false);
    if ((frame == 66 || frame == 152) && !gui_reload_document(&app->workspace, app->workspace.active, 1)) app->reveal_failed = 1;
    if (frame == 158) {
        app->usability_clipboard = igGetPlatformIO_Nil()->Platform_GetClipboardTextFn; app->usability_clipboard_saved = 1;
        igGetPlatformIO_Nil()->Platform_GetClipboardTextFn = usability_clipboard_get;
        memcpy(usability_clipboard_text, "invalid\nquery", 14); app->find[GUI_FIND_PREVIEW].focus = 1; app->find_target = GUI_FIND_PREVIEW;
    }
    if (frame == 166) { memset(usability_clipboard_text, 'x', TEXT_FIND_QUERY); usability_clipboard_text[TEXT_FIND_QUERY] = 0; }
    if (frame == 172) memcpy(usability_clipboard_text, "\xc0\xaf", 3);
    if (frame == 161 || frame == 167 || frame == 173) { ImGuiIO_AddKeyEvent(io, test_shortcut_modifier(), true); ImGuiIO_AddKeyEvent(io, ImGuiKey_V, true); }
    if (frame == 162 || frame == 168 || frame == 174) ImGuiIO_AddKeyEvent(io, ImGuiKey_V, false);
    if (frame == 163 || frame == 169 || frame == 175) ImGuiIO_AddKeyEvent(io, test_shortcut_modifier(), false);
    if (frame == 179) { igGetPlatformIO_Nil()->Platform_GetClipboardTextFn = app->usability_clipboard; app->usability_clipboard_saved = 0; }
    if (frame == 180) {
        if (!gui_set_document_text(&app->workspace, app->workspace.active, app->usability_original)) app->reveal_failed = 1;
        app->close_all = 1;
    }
    if (frame == 181) {
        gui_search request = {0}; request.version = request.view = 1;
        memcpy(request.project, app->workspace.root, strlen(app->workspace.root) + 1); memcpy(request.query, "plan", 5);
        USABILITY_CHECK(app->pending_action == GUI_ACTION_CLOSE && gui_search_receive(&app->workspace, &app->pending_search, &app->search_ready,
            app->pending_action != GUI_ACTION_NONE, &request, sizeof(request)) == GUI_SEARCH_BUSY && !app->search_ready);
    }
    if (frame == 183 || frame == 184) { test_mouse(window, app->cancel_position); ImGuiIO_AddMouseButtonEvent(io, 0, frame == 183); }
    if (frame == 186 && !gui_reload_document(&app->workspace, app->workspace.active, 1)) app->reveal_failed = 1;
    if (frame == 188) app->close_all = 1;
    if (frame == 194) { ImGuiIO_AddKeyEvent(io, ImGuiMod_Ctrl, true); ImGuiIO_AddKeyEvent(io, ImGuiKey_F, true); }
    if (frame == 195) ImGuiIO_AddKeyEvent(io, ImGuiKey_F, false);
    if (frame == 196) ImGuiIO_AddKeyEvent(io, ImGuiMod_Ctrl, false);
    if (frame == 198) {
        USABILITY_CHECK(app->workspace.active < 0 && app->find_target == GUI_FIND_SIDEBAR && app->find[0].open && igIsAnyItemActive());
        app->usability_complete = !app->usability_clipboard_saved;
    }
}
static void usability_observe(gui_app *app, gui_tab *tab, ImGuiID widget) {
    if (app->selection_exercise != 16 || app->reveal_failed) return;
    int frame = app->test_frame; gui_find *raw = &app->find[GUI_FIND_SOURCE], *preview = &app->find[GUI_FIND_PREVIEW];
    if (frame == 2) {
        app->usability_original_size = tab->length + 1;
        app->usability_original = native_memory_allocate(app->usability_original_size);
        USABILITY_CHECK(app->usability_original && app->wrap_source && !app->find[GUI_FIND_SIDEBAR].open);
        memcpy(app->usability_original, tab->text, app->usability_original_size);
    }
    if (frame == 3) {
        const char *line = strchr(tab->text, '\n'); float x, wrapped, unwrapped;
        USABILITY_CHECK(line && crmd_text_position(widget, tab->text, tab->length, (size_t)(line + 1 - tab->text), 1, &x, &wrapped) &&
            crmd_text_position(widget, tab->text, tab->length, (size_t)(line + 1 - tab->text), 0, &x, &unwrapped) && wrapped > unwrapped * 4);
        app->hint.version = 1; app->hint.line = 3;
        memcpy(app->hint.project, app->workspace.root, strlen(app->workspace.root) + 1);
        memcpy(app->hint.path, tab->path, strlen(tab->path) + 1); memcpy(app->hint.hash, tab->hash, 65);
        app->hint_path_ok = app->scroll_hint = 1; app->marker_revision = 0;
    }
    if (frame == 6) { crmd_text_view view; USABILITY_CHECK(crmd_get_text_view(widget, &view, sizeof(view)) && view.scroll_y > 32 && app->marker_draws > 0); }
    if (frame == 14) { ImGuiInputTextState *state = igGetInputTextState(widget); USABILITY_CHECK(state && !(state->Flags & ImGuiInputTextFlags_WordWrap)); }
    if (frame == 22) USABILITY_CHECK(tab->dirty && !strcmp(tab->text, "Plan **the** plan\n\nplan other block\n"));
    if (frame == 35) {
        USABILITY_CHECK(raw->count == 3 && raw->current == 1 && raw->begin == 0 && raw->end == 4);
        crmd_text_view view; USABILITY_CHECK(crmd_get_text_view(widget, &view, sizeof(view)) && view.selection_start == 0 && view.selection_end == 4 &&
            !strcmp(tab->text, "Plan **the** plan\n\nplan other block\n"));
    }
    if (frame == 42) USABILITY_CHECK(raw->current == 2 && raw->begin > 4);
    if (frame == 48) USABILITY_CHECK(raw->current == 1);
    if (frame == 51) { ImGuiInputTextState *state = igGetInputTextState(widget); USABILITY_CHECK(app->wrap_source && state && (state->Flags & ImGuiInputTextFlags_WordWrap)); }
    if (frame == 56) {
        USABILITY_CHECK(!raw->open && igGetActiveID() == widget);
        USABILITY_CHECK(!strcmp(tab->text, "Plan **the** plan\n\nplan other block\n"));
    }
    /* Pinned ImGui undoes one inserted character, not whole input batches. */
    if (frame == 64) USABILITY_CHECK(!strcmp(tab->text, "Plan **the** plan\n\nplan other block") && !strcmp(raw->query, "plan"));
    if (frame == 83) {
        USABILITY_CHECK(preview->count == 1 && preview->current == 1 && app->workspace.preview_text && !strcmp(app->workspace.preview_text, "the plan"));
        USABILITY_CHECK(!strcmp(tab->text, app->usability_original));
    }
    if (frame == 89 || frame == 95) USABILITY_CHECK(!preview->count && !preview->error);
    if (frame == 101) USABILITY_CHECK(preview->count == 2 && preview->current == 1);
    if (frame == 108) USABILITY_CHECK(preview->current == 2);
    if (frame == 114) USABILITY_CHECK(preview->current == 1);
    if (frame == 122) USABILITY_CHECK(preview->current == 2 && app->workspace.preview_text && !strcmp(app->workspace.preview_text, "Plan"));
    if (frame == 132) {
        gui_navigation *navigation = app->workspace.navigation; int visible = 0, collapsed = 0;
        for (int i = 0; i < navigation->count; ++i) visible += app->document_visible[i] != 0;
        for (int i = 0; i < navigation->tree_count; ++i) if (navigation->tree[i].document < 0 && !navigation->tree[i].expanded && app->tree_visible[i]) ++collapsed;
        USABILITY_CHECK(visible == 1 && collapsed == 0);
        USABILITY_CHECK(!strcmp(app->find[0].query, "usability") && !strcmp(raw->query, "plan") && !strcmp(preview->query, "plan"));
    }
    if (frame == 140) {
        int visible = 0; for (int i = 0; i < app->workspace.navigation->count; ++i) visible += app->document_visible[i] != 0;
        USABILITY_CHECK(!*app->find[0].query && !app->find[0].open && visible == app->workspace.navigation->count);
    }
    if (frame == 148) USABILITY_CHECK(preview->count == 1 && preview->current == 1 && app->find_match_case && !app->find[0].open);
    if (frame == 153) USABILITY_CHECK(app->sidebar_width > 290);
    if (frame == 157) USABILITY_CHECK(app->source_fraction < 0.48f);
    if (frame == 185) USABILITY_CHECK(!app->close_all && app->pending_action == GUI_ACTION_NONE && tab->used && tab->dirty && !strcmp(tab->text, app->usability_original));
    if (frame == 156) {
        USABILITY_CHECK(!tab->dirty && !strcmp(tab->text, app->usability_original));
    }
    if (frame == 164 || frame == 170 || frame == 178) USABILITY_CHECK(preview->error == 1 && !strcmp(preview->query, "plan") && !tab->dirty && !strcmp(tab->text, app->usability_original));
}
#undef USABILITY_CHECK
