    if (reveal_exercise && (exercise || edit_exercise || preview_exercise || !frame_limit)) { code = usage(stderr, 2); goto cleanup; }
    application->reveal_exercise = reveal_exercise; application->selection_exercise = selection_exercise;
    if (selection_exercise && (exercise || edit_exercise || preview_exercise || reveal_exercise || !frame_limit)) { code = usage(stderr, 2); goto cleanup; }
    application->exercise = exercise; application->edit_exercise = edit_exercise; application->save_fail_exercise = save_fail_exercise; application->preview_exercise = preview_exercise;
