/* Test-only GUI state observation and synthetic input. Included after host helpers. */
static ImVec2 item_center(void) {
    ImVec2 a = igGetItemRectMin(), b = igGetItemRectMax(), center = {(a.x + b.x) * 0.5f, (a.y + b.y) * 0.5f}; return center;
}

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

static void exercise_input(gui_app *app, HWND window, int frame) {
    ImVec2 position;
    app->test_frame = frame;
    if (app->selection_exercise == 15) {
        ImGuiIO *io = igGetIO_Nil(); gui_workspace *workspace = &app->workspace;
        if (frame == 14 || frame == 54) {
            int slot = workspace->active;
            if ((frame == 54 && !gui_reload_document(workspace, slot, 1)) || strcmp(workspace->tabs[slot].text, "undo shared") ||
                (frame == 14 && !gui_save_document(workspace, slot)) ||
                !gui_close_document(workspace, slot, 0) || !gui_open_document(workspace, frame == 14 ? "undo-b###same.md" : "undo-a###same.md") || workspace->active != slot) {
                fprintf(stderr, "Undo fixture save/close/reuse failed\n"); app->reveal_failed = 1;
            }
        }
        position = app->editor_position;
        crmd_message(window, WM_MOUSEMOVE, 0, MAKELPARAM((int)position.x, (int)position.y));
        if (frame == 5 || frame == 6 || frame == 18 || frame == 19 || frame == 58 || frame == 59)
            ImGuiIO_AddMouseButtonEvent(io, 0, frame == 5 || frame == 18 || frame == 58);
        if (frame == 8 || frame == 30) { ImGuiIO_AddKeyEvent(io, ImGuiMod_Ctrl, true); ImGuiIO_AddKeyEvent(io, ImGuiKey_A, true); }
        if (frame == 9 || frame == 31) ImGuiIO_AddKeyEvent(io, ImGuiKey_A, false);
        if (frame == 10 || frame == 32) ImGuiIO_AddKeyEvent(io, ImGuiMod_Ctrl, false);
        if (frame == 11 || frame == 34) ImGuiIO_AddInputCharactersUTF8(io, frame == 11 ? "undo shared" : "undo B edit");
        if (frame == 22 || frame == 38 || frame == 44 || frame == 50 || frame == 62) {
            ImGuiIO_AddKeyEvent(io, ImGuiMod_Ctrl, true); ImGuiIO_AddKeyEvent(io, frame == 44 ? ImGuiKey_Y : ImGuiKey_Z, true);
        }
        if (frame == 23 || frame == 39 || frame == 45 || frame == 51 || frame == 63) ImGuiIO_AddKeyEvent(io, frame == 45 ? ImGuiKey_Y : ImGuiKey_Z, false);
        if (frame == 24 || frame == 40 || frame == 46 || frame == 52 || frame == 64) ImGuiIO_AddKeyEvent(io, ImGuiMod_Ctrl, false);
        if (frame == 26 || frame == 36 || frame == 42 || frame == 48 || frame == 66) {
            const char *expected = frame == 36 || frame == 48 ? "undo B edit" : frame == 42 ? "undo B edi" : "undo shared";
            if (strcmp(workspace->tabs[workspace->active].text, expected)) {
                fprintf(stderr, "Undo file isolation/redo failed at frame %d: expected '%s', got '%.40s'\n", frame, expected, workspace->tabs[workspace->active].text); app->reveal_failed = 1;
            } else app->undo_checks++;
        }
        return;
    }
    if (app->selection_exercise) {
        ImGuiIO *io = igGetIO_Nil();
        if (app->selection_exercise == 12 && frame >= 22) frame = frame < 100 ? 22 : frame - 78;
        if (frame == 16) app->selection_source_seen = gui_selection_current(&app->workspace).view == GUI_SELECTION_SOURCE && gui_selection_current(&app->workspace).status == GUI_SELECTION_EXACT;
        position = frame < 18 ? app->editor_position : app->preview_position;
        if (frame >= 18) { position.x += 40; if (app->selection_exercise == 4 || app->selection_exercise == 5 || app->selection_exercise >= 7) {
            int reverse = app->selection_exercise == 5 || app->selection_exercise == 9;
            int partial = app->selection_exercise == 8 || app->selection_exercise == 9;
            int at_end = (frame >= 22) != reverse;
            position.x = app->preview_position.x + (at_end ? (partial ? 85 : 300) : (partial ? 15 : 0));
            if (app->selection_exercise == 7 || app->selection_exercise >= 10) position.y += at_end ? 180 : 0;
            if (app->selection_exercise == 12 && at_end) position.y = app->preview_clip_bottom + 30;
        } }
        /* Focus neutral project heading without replacing latest document selection. */
        if (frame >= 14 && frame <= 17) { position.x = 12; position.y = 12; }
        if (frame >= 40 && frame <= 41) position = app->comment_position;
        if (frame >= 54 && frame <= 55) position = app->send_position;
        crmd_message(window, WM_MOUSEMOVE, 0, MAKELPARAM((int)position.x, (int)position.y));
        if (frame == 5 || frame == 6 || frame == 14 || frame == 15) ImGuiIO_AddMouseButtonEvent(io, 0, frame == 5 || frame == 14);
        if (frame == 8 || frame == 10) ImGuiIO_AddKeyEvent(io, ImGuiMod_Ctrl, frame == 8);
        if (frame == 8 || frame == 9) ImGuiIO_AddKeyEvent(io, ImGuiKey_A, frame == 8);
        if (app->selection_exercise == 6) {
            if (frame == 11) ImGuiIO_AddInputCharactersUTF8(io, "plain  \nnext\n");
            if (frame == 12 || frame == 13) { ImGuiIO_AddKeyEvent(io, ImGuiMod_Ctrl, frame == 12); ImGuiIO_AddKeyEvent(io, ImGuiKey_A, frame == 12); }
        }
        if (frame == 40 || frame == 41) ImGuiIO_AddMouseButtonEvent(io, 0, frame == 40);
        if (app->selection_exercise != 1 && app->selection_exercise != 13 && frame == 50) ImGuiIO_AddInputCharactersUTF8(io, "Preview selection review note\nHuman body unchanged.");
        if (app->selection_exercise != 1 && app->selection_exercise != 13 && (frame == 54 || frame == 55)) ImGuiIO_AddMouseButtonEvent(io, 0, frame == 54);
        if (frame == 46) {
            int expected = app->selection_exercise != 13;
            if (app->composer_active != expected || (expected && (*app->comment || !app->composer_from_preview ||
                app->composer_start != app->selection_before_comment.start || app->composer_end != app->selection_before_comment.end || !igIsAnyItemActive()))) {
                fprintf(stderr, "Unified comment action did not follow latest preview ownership\n"); app->reveal_failed = 1;
            } else {
                app->selection_action_checked = 1;
                printf("{\"event\":\"selection-action\",\"enabled\":%s,\"olderSourceNotUsed\":true,\"commentRect\":[%.1f,%.1f,%.1f,%.1f]}\n", expected ? "true" : "false", app->comment_rect.x, app->comment_rect.y, app->comment_rect.z, app->comment_rect.w); fflush(stdout);
            }
        }
        if (app->selection_exercise == 4 || app->selection_exercise == 5 || app->selection_exercise >= 7) {
            if (frame == 20 || frame == 25) ImGuiIO_AddMouseButtonEvent(io, 0, frame == 20);
        } else if (app->selection_exercise == 3) {
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

static void selection_observe(gui_app *app, gui_tab *tab, size_t offset, int exact) {
        app->preview_scroll_y = igGetScrollY(); app->preview_clip_bottom = igGetWindowPos().y + igGetWindowHeight();
        if (app->selection_exercise && app->selection_exercise != 15 && app->test_frame >= (app->selection_exercise == 12 ? 112 : 34) && !app->selection_ready) {
            gui_selection current = gui_selection_current(&app->workspace);
            if (!app->selection_source_seen || current.view != GUI_SELECTION_PREVIEW || current.status != (app->selection_exercise == 6 ? GUI_SELECTION_CONTEXT : app->selection_exercise == 13 ? GUI_SELECTION_UNSUPPORTED : GUI_SELECTION_MAPPED)) {
                fprintf(stderr, "Preview context/widget did not replace older source selection: mode %d, view %d, status %d\n", app->selection_exercise, current.view, current.status);
                app->reveal_failed = 1;
            } else if (app->selection_exercise == 12 && app->preview_scroll_y <= 0) {
                fprintf(stderr, "Preview drag did not auto-scroll\n"); app->reveal_failed = 1;
            } else { app->selection_before_comment = current; app->selection_ready = 1; printf("{\"event\":\"selection-ready\",\"previewScrollY\":%.0f}\n", app->preview_scroll_y); fflush(stdout); }
        }
        if (exact) app->preview_exact_seen = 1;
        if (exact && gui_selection_current(&app->workspace).view == GUI_SELECTION_PREVIEW &&
            gui_selection_current(&app->workspace).start == (int)offset && (size_t)gui_selection_current(&app->workspace).end == tab->length) app->preview_shared_seen = 1;
}
