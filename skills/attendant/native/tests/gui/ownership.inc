    if (edit_exercise || preview_exercise || reveal_exercise || selection_exercise) {
        char marker[GUI_PATH];
        snprintf(marker, sizeof(marker), "%s/.gui-test-owned", application->workspace.root);
        if (application->workspace.active < 0 || source_stat(marker).kind != SOURCE_REGULAR) {
            fprintf(stderr, "Editing exercise requires generated owned fixture and initial document\n"); goto cleanup;
        }
    }
