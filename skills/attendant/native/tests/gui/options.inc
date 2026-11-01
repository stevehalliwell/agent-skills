        if (!strcmp(argv[i], "--exercise-selection") && i + 1 < argc) {
            const char *mode = argv[++i];
            selection_exercise = !strcmp(mode, "plain-menu") ? 1 : !strcmp(mode, "markup-menu") ? 2 : !strcmp(mode, "widget") ? 3 : !strcmp(mode, "drag") ? 4 : !strcmp(mode, "reverse") ? 5 : !strcmp(mode, "context") ? 6 : !strcmp(mode, "multi") ? 7 : !strcmp(mode, "partial") ? 8 : !strcmp(mode, "reverse-partial") ? 9 : !strcmp(mode, "table") ? 10 : !strcmp(mode, "wrap") ? 11 : !strcmp(mode, "scroll") ? 12 : !strcmp(mode, "overflow") ? 13 : !strcmp(mode, "render-failure") ? 14 : !strcmp(mode, "undo") ? 15 : 0;
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
