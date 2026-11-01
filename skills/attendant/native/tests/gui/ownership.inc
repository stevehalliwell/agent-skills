    if (edit_exercise || preview_exercise || reveal_exercise || selection_exercise) {
        char marker[GUI_PATH]; wchar_t wide[4096];
        snprintf(marker, sizeof(marker), "%s/.gui-test-owned", application->workspace.root);
        if (application->workspace.active < 0 || !MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, marker, -1, wide, 4096) || GetFileAttributesW(wide) == INVALID_FILE_ATTRIBUTES) {
            fprintf(stderr, "Editing exercise requires generated owned fixture and initial document\n"); goto cleanup;
        }
    }
