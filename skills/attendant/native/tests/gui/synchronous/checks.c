TEST(production_render_abi_screenshot_without_workspace_writes,
    char project[NATIVE_PATH_SIZE], baseline[NATIVE_PATH_SIZE], shot[NATIVE_PATH_SIZE];
    TEST_ASSERT(gui_project(project) == 0 && gui_baseline(project, baseline) == 0);
    /* Existing screenshot writer needs report path, not Unicode fixture path. */
    TEST_ASSERT_FILE_OK(file_tree_join(shot, gui_evidence, "production.ppm"));
    file_entry_kind kind; TEST_ASSERT(file_native_kind(shot, &kind) == file_error_not_found);
    const char *extra[] = {"--shot", shot};
    TEST_ASSERT(gui_window(project, "records/tasks/first.md", 1, 0, extra, 2) == 0);
    int vertices; TEST_ASSERT(gui_positive_count("vertices", &vertices) && vertices > 1000 && integration_number_is("tabs", 1));
    TEST_ASSERT(gui_screenshot(shot) == 0);
    TEST_ASSERT_DIRECTORIES_EQUAL(&gui_files, ((cstring){project}), ((cstring){baseline}));
    ++gui_groups;
)

TEST(existing_c_document_selection_ipc_model,
    char project[NATIVE_PATH_SIZE], path[NATIVE_PATH_SIZE], original[16384], changed[16384];
    TEST_ASSERT(gui_project(project) == 0);
    TEST_ASSERT_FILE_OK(file_tree_join(path, project, "records/tasks/first.md"));
    TEST_ASSERT(gui_read(path, original, sizeof(original)) == 0);
    const char *pattern = "depends_on: [second]"; char *found = strstr(original, pattern); TEST_ASSERT(found);
    TEST_ASSERT(snprintf(changed, sizeof(changed), "%.*sdepends_on: [second, missing]%s", (int)(found - original), original, found + strlen(pattern)) < (int)sizeof(changed));
    TEST_ASSERT_FILE_OK(file_write_bytes((cstring){path}, changed, strlen(changed)));
    TEST_ASSERT_FILE_OK(file_tree_join(path, project, "records/tasks/unreadable.md"));
    const u8 invalid[] = {0xff, 0xfe, 0}; TEST_ASSERT_FILE_OK(file_create_bytes((cstring){path}, invalid, sizeof(invalid)));
    const char *arguments[] = {gui_model, project}; TEST_ASSERT(gui_execute(arguments, 2, 0, 60000) == 0);
    TEST_ASSERT(integration_json_parse(0, gui_result.output.data, gui_result.output.len) == 0);
    integration_json_node *ok = integration_json_get(0, "ok"); int count;
    TEST_ASSERT(ok && ok->type == PDJSON_TRUE && gui_positive_count("checks", &count));
    fprintf(stderr, "model-checks:%d\n", count);
    ++gui_groups;
)

TEST(terminal_render_failure_preserves_source,
    char project[NATIVE_PATH_SIZE], baseline[NATIVE_PATH_SIZE];
    TEST_ASSERT(gui_project(project) == 0 && gui_baseline(project, baseline) == 0);
    const char *extra[] = {"--exercise-selection", "render-failure"};
    TEST_ASSERT(gui_window(project, "README.md", 0, 1, extra, 2) == 0);
    TEST_ASSERT(strstr((const char *)gui_result.error_output.data, "Render failure: frame abandoned without normal teardown"));
    TEST_ASSERT_DIRECTORIES_EQUAL(&gui_files, ((cstring){project}), ((cstring){baseline}));
    ++gui_groups;
)

TEST(partial_discovery_diagnostics_preserve_source,
    char project[NATIVE_PATH_SIZE], baseline[NATIVE_PATH_SIZE], path[NATIVE_PATH_SIZE], tables[16384];
    TEST_ASSERT(gui_project(project) == 0);
    TEST_ASSERT_FILE_OK(file_tree_join(path, project, "records/unavailable"));
    TEST_ASSERT_FILE_OK(file_create_text((cstring){path}, "Not a directory.\n"));
    TEST_ASSERT_FILE_OK(file_tree_join(path, project, ".pi/attendant.tables"));
    TEST_ASSERT(gui_read(path, tables, sizeof(tables)) == 0);
    const char *addition = "\nrecords/unavailable as unavailable\n"; TEST_ASSERT(strlen(tables) + strlen(addition) < sizeof(tables)); strcat(tables, addition);
    TEST_ASSERT_FILE_OK(file_write_bytes((cstring){path}, tables, strlen(tables)));
    TEST_ASSERT(gui_baseline(project, baseline) == 0);
    TEST_ASSERT(gui_window(project, "records/tasks/first.md", 0, 0, NULL, 0) == 0);
    TEST_ASSERT(gui_positive_count("issues_shown", NULL) && integration_number_is("tabs", 1));
    TEST_ASSERT_DIRECTORIES_EQUAL(&gui_files, ((cstring){project}), ((cstring){baseline}));
    ++gui_groups;
)

TEST(reference_navigation_and_tab_reuse_without_writes,
    char project[NATIVE_PATH_SIZE], baseline[NATIVE_PATH_SIZE];
    TEST_ASSERT(gui_project(project) == 0 && gui_baseline(project, baseline) == 0);
    GUI_WINDOW(project, "records/tasks/first.md", "--exercise");
    TEST_ASSERT(integration_number_is("tabs", 2));
    TEST_ASSERT_DIRECTORIES_EQUAL(&gui_files, ((cstring){project}), ((cstring){baseline}));
    ++gui_groups;
)

TEST(literal_path_undo_isolation_and_redo,
    char project[NATIVE_PATH_SIZE], first[NATIVE_PATH_SIZE], second[NATIVE_PATH_SIZE];
    TEST_ASSERT(gui_project(project) == 0);
    TEST_ASSERT_FILE_OK(file_tree_join(first, project, "undo-a###same.md"));
    TEST_ASSERT_FILE_OK(file_tree_join(second, project, "undo-b###same.md"));
    TEST_ASSERT_FILE_OK(file_create_text((cstring){first}, "undo original"));
    TEST_ASSERT_FILE_OK(file_create_text((cstring){second}, "undo shared"));
    GUI_WINDOW(project, "undo-a###same.md", "--exercise-selection", "undo");
    TEST_ASSERT_FILE_TEXT(((cstring){first}), "undo shared"); TEST_ASSERT_FILE_TEXT(((cstring){second}), "undo shared");
    ++gui_groups;
)

TEST(real_edit_feedback_and_explicit_resolution,
    char project[NATIVE_PATH_SIZE], path[NATIVE_PATH_SIZE], expected[256], prior[32768];
    TEST_ASSERT(gui_project(project) == 0); GUI_WINDOW(project, "README.md", "--exercise-edit");
    TEST_ASSERT_FILE_OK(file_tree_join(path, project, "README.md"));
    TEST_ASSERT(snprintf(expected, sizeof(expected), "%s GUI edit", gui_original) < (int)sizeof(expected));
    TEST_ASSERT_FILE_TEXT(((cstring){path}), expected);
    TEST_ASSERT(gui_query(project) == 0 && integration_number_is("meta_comments.pending", 1));
    TEST_ASSERT(gui_result.output.len < sizeof(prior)); memcpy(prior, gui_result.output.data, gui_result.output.len + 1);
    TEST_ASSERT(gui_query(project) == 0 && integration_json_matches("rows", prior, "rows"));
    TEST_ASSERT(gui_resolve(project) == 0);
    ++gui_groups;
)

TEST(failed_queue_keeps_save_and_other_files,
    char project[NATIVE_PATH_SIZE], path[NATIVE_PATH_SIZE], baseline[NATIVE_PATH_SIZE], expected[256];
    TEST_ASSERT(gui_project(project) == 0);
    TEST_ASSERT_FILE_OK(file_tree_join(path, project, "records/_meta_comments")); TEST_ASSERT(file_directory_create(path, 1));
    TEST_ASSERT_FILE_OK(file_tree_join(path, project, "records/_meta_comments/user.md"));
    TEST_ASSERT_FILE_OK(file_create_text((cstring){path}, "User-owned file.\n"));
    TEST_ASSERT(gui_baseline(project, baseline) == 0);
    GUI_WINDOW(project, "README.md", "--exercise-save-fail");
    TEST_ASSERT(snprintf(expected, sizeof(expected), "%s GUI edit", gui_original) < (int)sizeof(expected));
    TEST_ASSERT_FILE_OK(file_tree_join(path, project, "README.md")); TEST_ASSERT_FILE_TEXT(((cstring){path}), expected);
    TEST_ASSERT_FILE_OK(file_tree_join(path, baseline, "README.md")); TEST_ASSERT_FILE_OK(file_write_bytes((cstring){path}, expected, strlen(expected)));
    TEST_ASSERT_DIRECTORIES_EQUAL(&gui_files, ((cstring){project}), ((cstring){baseline}));
    ++gui_groups;
)
