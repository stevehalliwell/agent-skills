/* Live protocol stays local: no readiness/JSON knowledge in test.h. */
static int gui_index(unsigned slot, const char *path, usize *index) {
    integration_json_node *node = integration_json_get(slot, path); value number = {0};
    if (!node || node->type != PDJSON_NUMBER || parse_number(node->text, &number) != NUMBER_VALID ||
        number.number_kind != NUMBER_I64 || number.number.signed_integer < 0 || number.number.signed_integer > INT_MAX) return 0;
    *index = (usize)number.number.signed_integer; return 1;
}
static int gui_slice(const char *text, usize start, usize end, const char **bytes, usize *size) {
    if (!text || start > end) return 0;
    usize length = strlen(text), offset = 0, index = 0, first = 0;
    while (index < end) {
        if (offset == length) return 0;
        if (index == start) first = offset;
        usize count = string_utf8_sequence_size((const u8 *)text + offset, length - offset);
        if (!count) return 0;
        offset += count; ++index;
    }
    if (start == end) first = offset;
    *bytes = text + first; *size = offset - first; return 1;
}
static int gui_event_ready(test_command_live *live, usize *offset, int *ready, const char *event) {
    while (*offset < live->result.output.len) {
        const char *line = (const char *)live->result.output.data + *offset;
        const char *end = memchr(line, '\n', live->result.output.len - *offset);
        if (!end) break; /* Wait for complete line, including fragmented UTF-8. */
        TEST_ASSERT(integration_json_parse(0, line, (usize)(end - line)) == 0);
        if (integration_text_is("event", event)) { TEST_ASSERT(!*ready); *ready = 1; }
        *offset += (usize)(end - line) + 1;
    }
    return 0;
}
static int gui_preview_preserved(const char *project, const char *baseline, int context) {
    file_tree before = {0}; arena_reset(gui_files.memory);
    TEST_ASSERT_FILE_OK(file_directory_list_tree(&gui_files, (cstring){(char *)baseline}, &before));
    for (usize i = 0; i < before.count; ++i) {
        file_tree_entry *entry = &before.entries[i]; char prior[NATIVE_PATH_SIZE], current[NATIVE_PATH_SIZE]; file_entry_kind kind;
        TEST_ASSERT_FILE_OK(file_tree_join(prior, baseline, entry->path));
        TEST_ASSERT_FILE_OK(file_tree_join(current, project, entry->path));
        TEST_ASSERT_FILE_OK(file_native_kind(current, &kind)); TEST_ASSERT(kind == entry->kind);
        if (kind != file_entry_regular || !strcmp(entry->path, ".pi/attendant.tables") || (context && !strcmp(entry->path, "README.md"))) continue;
        TEST_ASSERT_FILES_EQUAL(((cstring){prior}), ((cstring){current}));
    }
    arena_reset(gui_files.memory); return 0;
}
static int gui_preview(const char *mode, const char *text, const char *hash, int context_mode) {
    char project[NATIVE_PATH_SIZE], target[NATIVE_PATH_SIZE], baseline[NATIVE_PATH_SIZE];
    TEST_ASSERT(gui_project(project) == 0);
    TEST_ASSERT_FILE_OK(file_tree_join(target, project, "README.md"));
    TEST_ASSERT_FILE_OK(file_write_bytes((cstring){target}, text, strlen(text)));
    TEST_ASSERT(gui_baseline(project, baseline) == 0);
    const char *arguments[] = {gui_testing, "--project", project, "--assets", gui_assets, "--file", "README.md", "--frames", "240", "--exercise-selection", mode};
    test_command_workspace workspaces[2];
    for (unsigned i = 0; i < 2; ++i) workspaces[i] = (test_command_workspace){gui_commands.scratch, gui_live_output[i], gui_live_errors[i], sizeof(gui_live_output[i]), sizeof(gui_live_errors[i])};
    test_command command = {.arguments = arguments, .argument_count = 11, .working_directory = gui_root, .timeout_ms = 30000};
#if defined(__APPLE__)
    test_command_environment environment = {"DYLD_LIBRARY_PATH", NULL};
    command.environment = &environment; command.environment_count = 1;
#endif
    TEST_ASSERT_COMMAND_OK(test_command_start(&gui_live[0], &workspaces[0], &command));
    usize offset = 0; int ready = 0;
    while (!ready && !gui_live[0].complete) {
        TEST_ASSERT_COMMAND_OK(test_command_poll(&gui_live[0]));
        TEST_ASSERT(gui_event_ready(&gui_live[0], &offset, &ready, "selection-ready") == 0);
        if (!ready && !gui_live[0].complete) test_command_sleep(1);
    }
    TEST_ASSERT(ready && gui_live[0].active && !gui_live[0].observed);
    const char *inspection[] = {gui_other, "--project", project, "--inspect"};
    command.arguments = inspection; command.argument_count = 4; command.timeout_ms = 60000;
    TEST_ASSERT_COMMAND_OK(test_command_start(&gui_live[1], &workspaces[1], &command));
    while (!gui_live[1].complete) {
        TEST_ASSERT_COMMAND_OK(test_command_poll(&gui_live[0]));
        TEST_ASSERT(gui_live[0].active && !gui_live[0].observed);
        TEST_ASSERT_COMMAND_OK(test_command_poll(&gui_live[1]));
        if (!gui_live[1].complete) test_command_sleep(1);
    }
    TEST_ASSERT_COMMAND_EXIT(&gui_live[1].result, 0);
    TEST_ASSERT(integration_json_parse(0, gui_live[1].result.output.data, gui_live[1].result.output.len) == 0);
    TEST_ASSERT(integration_text_is("selection.view", "preview") && integration_text_is("selection.status", context_mode ? "context" : "mapped"));
    integration_json_node *dirty = integration_json_get(0, "document.dirty");
    TEST_ASSERT(dirty && dirty->type == (context_mode ? PDJSON_TRUE : PDJSON_FALSE));
    const char *rendered = integration_text("selection.rendered_context"), *selection = integration_text("selection.text");
    char owned_context[16384], selected[16384];
    TEST_ASSERT(rendered && selection && strlen(rendered) < sizeof(owned_context) && strlen(selection) < sizeof(selected));
    strcpy(owned_context, rendered); strcpy(selected, selection);
    if (context_mode) {
        integration_json_node *start = integration_json_get(0, "selection.range_start"), *end = integration_json_get(0, "selection.range_end");
        TEST_ASSERT(start && end && start->type == PDJSON_NULL && end->type == PDJSON_NULL);
    } else { usize end; TEST_ASSERT(gui_index(0, "selection.range_end", &end) && end > 0); }
    TEST_ASSERT(integration_json_parse(1, rendered, strlen(rendered)) == 0);
    integration_json_node *sections = integration_json_get(1, "sections");
    TEST_ASSERT(sections && sections->type == PDJSON_ARRAY && (context_mode || sections->count >= 3));
    char assembled[16384] = {0}; usize used = 0;
    for (int i = 0; i < sections->count; ++i) {
        char path[80]; usize start, end;
        snprintf(path, sizeof(path), "sections.%d.highlight_start", i); TEST_ASSERT(gui_index(1, path, &start));
        snprintf(path, sizeof(path), "sections.%d.highlight_end", i); TEST_ASSERT(gui_index(1, path, &end));
        snprintf(path, sizeof(path), "sections.%d.text", i); integration_json_node *node = integration_json_get(1, path);
        const char *bytes; usize size;
        TEST_ASSERT(node && node->type == PDJSON_STRING && gui_slice(node->text, start, end, &bytes, &size));
        TEST_ASSERT(used + size + (i != 0) < sizeof(assembled));
        if (i) assembled[used++] = '\n'; memcpy(assembled + used, bytes, size); used += size; assembled[used] = 0;
    }
    TEST_ASSERT(!strcmp(assembled, selected));
    while (!gui_live[0].complete) { TEST_ASSERT_COMMAND_OK(test_command_poll(&gui_live[0])); if (!gui_live[0].complete) test_command_sleep(1); }
    TEST_ASSERT_COMMAND_EXIT(&gui_live[0].result, 0);
    TEST_ASSERT(gui_preview_preserved(project, baseline, context_mode) == 0);
    char expected[16384]; TEST_ASSERT(snprintf(expected, sizeof(expected), "%s%s", text, context_mode ? "\n" : "") < (int)sizeof(expected));
    TEST_ASSERT_FILE_TEXT(((cstring){target}), expected);
    TEST_ASSERT(gui_query(project) == 0 && integration_number_is("meta_comments.pending", 1) && integration_count("rows") == 1);
    TEST_ASSERT(integration_text_is("rows.0.target_hash", hash));
    if (context_mode) TEST_ASSERT(integration_number_is("rows.0.range_start", 0) && integration_number_is("rows.0.range_end", 0) && integration_text_is("rows.0.selected_text", ""));
    else {
        usize start, end, size; const char *bytes;
        TEST_ASSERT(gui_index(0, "rows.0.range_start", &start) && gui_index(0, "rows.0.range_end", &end));
        TEST_ASSERT(gui_slice(text, start, end, &bytes, &size));
        const char *actual = integration_text("rows.0.selected_text"); TEST_ASSERT(actual && strlen(actual) == size && !memcmp(actual, bytes, size));
    }
    const char *source = integration_text("rows.0.source_path"); TEST_ASSERT(source);
    /* Resolve checks canonical project containment before update; read only owned source here too. */
    char source_path[NATIVE_PATH_SIZE], parent[NATIVE_PATH_SIZE], comment[32768];
    TEST_ASSERT_FILE_OK(file_tree_path((cstring){(char *)source}, source_path, 0));
    TEST_ASSERT_FILE_OK(file_tree_path((cstring){project}, parent, 0));
    usize prefix = strlen(parent); TEST_ASSERT(!strncmp(source_path, parent, prefix) && (source_path[prefix] == '/' || source_path[prefix] == '\\'));
    TEST_ASSERT(gui_read(source_path, comment, sizeof(comment)) == 0 && strstr(comment, owned_context));
    TEST_ASSERT(gui_resolve(project) == 0);
    ++gui_groups; return 0;
}
static int gui_wait_event(usize *offset, const char *event) {
    int ready = 0;
    while (!ready && !gui_live[0].complete) {
        TEST_ASSERT_COMMAND_OK(test_command_poll(&gui_live[0]));
        TEST_ASSERT(gui_event_ready(&gui_live[0], offset, &ready, event) == 0);
        if (!ready && !gui_live[0].complete) test_command_sleep(1);
    }
    TEST_ASSERT(ready && gui_live[0].active && !gui_live[0].observed); return 0;
}
static int gui_search_roundtrip(void) {
    char project[NATIVE_PATH_SIZE], baseline[NATIVE_PATH_SIZE], path[NATIVE_PATH_SIZE];
    TEST_ASSERT_FILE_OK(test_directory_next(&gui_case, "search-project", project));
    TEST_ASSERT_FILE_OK(file_directory_copy(&gui_files, (cstring){"tests/fixtures/project"}, (cstring){project}));
    TEST_ASSERT_FILE_OK(file_tree_join(path, project, ".gui-test-owned"));
    TEST_ASSERT_FILE_OK(file_create_text((cstring){path}, "Owned search fixture\n"));
    TEST_ASSERT_FILE_OK(file_tree_join(path, project, "README.md"));
    const char content[] = "Read **the** plan.\n\nSecond plan.\n";
    TEST_ASSERT_FILE_OK(file_write_bytes((cstring){path}, content, sizeof(content) - 1));
    TEST_ASSERT(gui_baseline(project, baseline) == 0);
    const char *arguments[] = {gui_testing, "--project", project, "--assets", gui_assets, "--file", "records/tasks/first.md", "--frames", "1200", "--exercise-selection", "search-server"};
    test_command_workspace buffers = {gui_commands.scratch, gui_live_output[0], gui_live_errors[0], sizeof(gui_live_output[0]), sizeof(gui_live_errors[0])};
    test_command command = {.arguments = arguments, .argument_count = 11, .working_directory = gui_root, .timeout_ms = 30000};
#if defined(__APPLE__)
    test_command_environment environment = {"DYLD_LIBRARY_PATH", NULL}; command.environment = &environment; command.environment_count = 1;
#endif
    TEST_ASSERT_COMMAND_OK(test_command_start(&gui_live[0], &buffers, &command));
    usize offset = 0; TEST_ASSERT(gui_wait_event(&offset, "search-ready") == 0);
    const char *views[] = {"source", "preview", "sidebar"}, *queries[] = {"**the**", "the plan", "readme"};
    for (int view = 0; view < 3; ++view) {
        const char *search[] = {gui_other, "--project", project, "--find", queries[view], "--view", views[view], "--file", "README.md"};
        TEST_ASSERT(gui_execute(search, view == 0 ? 9 : 7, 0, 60000) == 0 && integration_json_parse(0, gui_result.output.data, gui_result.output.len) == 0);
        integration_json_node *delivered = integration_json_get(0, "delivered"), *focus = integration_json_get(0, "os_focus_changed");
        TEST_ASSERT(delivered && delivered->type == PDJSON_TRUE && focus && focus->type == PDJSON_FALSE);
        TEST_ASSERT(gui_wait_event(&offset, "search-applied") == 0 && integration_number_is("serial", view + 1) && integration_number_is("view", view == 2 ? 0 : view + 1));
        if (view < 2) {
            TEST_ASSERT(integration_number_is("count", 1));
            const char *inspect[] = {gui_other, "--project", project, "--inspect"};
            TEST_ASSERT(gui_execute(inspect, 4, 0, 60000) == 0 && integration_json_parse(0, gui_result.output.data, gui_result.output.len) == 0);
            TEST_ASSERT(integration_text_is("selection.view", views[view]) && integration_text_is("selection.text", queries[view]));
            integration_json_node *dirty = integration_json_get(0, "document.dirty"); TEST_ASSERT(dirty && dirty->type == PDJSON_FALSE);
        }
    }
    for (int empty = 0; empty < 2; ++empty) {
        const char *zero[] = {gui_other, "--project", project, "--find", empty ? "" : "Plan", "--view", empty ? "preview" : "source", "--match-case"};
        TEST_ASSERT(gui_execute(zero, 8, 0, 60000) == 0 && integration_json_parse(0, gui_result.output.data, gui_result.output.len) == 0);
        TEST_ASSERT(gui_wait_event(&offset, "search-applied") == 0 && integration_number_is("serial", 4 + empty) && integration_number_is("count", 0));
    }
    const char *unchanged[] = {gui_other, "--project", project, "--inspect"};
    TEST_ASSERT(gui_execute(unchanged, 4, 0, 60000) == 0 && integration_json_parse(0, gui_result.output.data, gui_result.output.len) == 0 &&
        integration_text_is("selection.view", "preview") && integration_text_is("selection.text", "the plan"));
    const char *invalid[] = {gui_other, "--project", project, "--find", "bad\nquery", "--view", "source"};
    TEST_ASSERT(gui_execute(invalid, 7, 2, 60000) == 0 && integration_json_parse(0, gui_result.output.data, gui_result.output.len) == 0 && integration_text_is("code", "invalid-search"));
    const char *missing_view[] = {gui_other, "--project", project, "--find", "plan"};
    TEST_ASSERT(gui_execute(missing_view, 5, 2, 60000) == 0 && integration_json_parse(0, gui_result.output.data, gui_result.output.len) == 0 && integration_text_is("code", "usage"));
    while (!gui_live[0].complete) { TEST_ASSERT_COMMAND_OK(test_command_poll(&gui_live[0])); if (!gui_live[0].complete) test_command_sleep(1); }
    TEST_ASSERT_COMMAND_EXIT(&gui_live[0].result, 0);
    TEST_ASSERT_DIRECTORIES_EQUAL(&gui_files, ((cstring){project}), ((cstring){baseline}));
    const char *absent[] = {gui_other, "--project", project, "--find", "plan", "--view", "source"};
    TEST_ASSERT(gui_execute(absent, 7, 1, 60000) == 0 && integration_json_parse(0, gui_result.output.data, gui_result.output.len) == 0 && integration_text_is("code", "gui-not-running"));
    return 0;
}
TEST(preview_multi_cross_compiler_inspection_context_save_send_and_resolution,
    TEST_ASSERT(gui_preview("multi", "plain **bold**\n\nsecond *emphasis* and [link](https://example.invalid)\n\n```c\ncode\n```",
        "4d332c91c7b1340f167508157f461f163b2d8d4f59a5dc2562dd790c72437e43", 0) == 0);
    TEST_ASSERT(gui_search_roundtrip() == 0);
)
TEST(preview_context_cross_compiler_inspection_context_save_send_and_resolution,
    TEST_ASSERT(gui_preview("context", "plain  \nnext", "5c775689d1b5ecf557741bf872e2ad7cd1767e3675e922ed348ff759d7c125c3", 1) == 0);
)
