    if (selection_exercise == 15 && (application->undo_checks != 5 || dirty_documents(&application->workspace))) { fprintf(stderr, "Undo exercise did not complete\n"); goto cleanup; }
    if (selection_exercise && selection_exercise != 15 && (!application->selection_ready || !application->selection_action_checked || (selection_exercise != 1 && selection_exercise != 13 && application->sent_comments != 1))) { fprintf(stderr, "Selection exercise did not complete\n"); goto cleanup; }
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
    if (application->workspace.navigation && application->issues_shown != application->workspace.navigation->diagnostics) {
        fprintf(stderr, "Workspace issue warning missing or stale\n"); goto cleanup;
    }
    if (exercise && (application->workspace.tab_count != 2 || application->reference_clicks != 2)) {
        fprintf(stderr, "GUI reference exercise failed: tabs=%d clicks=%d\n", application->workspace.tab_count, application->reference_clicks); goto cleanup;
    }
