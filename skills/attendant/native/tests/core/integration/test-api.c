#include "test.h"

/* Independent expected SHA-256 of source_test_saved, not API-produced hash. */
static const char integration_saved_hash[] = "f5cc8c3a761482fd528a9d3152cdf27b8e6c9be8cf0f6264b0ffff4bd1a6fc54";
static int integration_send(const char *project, const char *target, int contextual) {
    char start[32], end[32]; usize begin = strlen("---\r\nname: file\r\n---\r\nHello ");
    snprintf(start, sizeof(start), "%zu", contextual ? 0 : begin); snprintf(end, sizeof(end), "%zu", contextual ? 0 : begin + 12);
    const char *arguments[] = {integration_probe, contextual ? "send-context" : "send", project, target, integration_saved_hash, start, end,
        contextual ? "" : source_test_selected, contextual ? source_test_context : source_test_comment, "probe"};
    test_command_environment environment = {"ATTENDANT_FEEDBACK_FAULT", ""};
    test_command command = {.arguments = arguments, .argument_count = 10, .environment = &environment, .environment_count = 1, .timeout_ms = 30000};
    TEST_ASSERT(integration_execute(&command, 0) == 0);
    return integration_json_parse(0, integration_result.output.data, integration_result.output.len);
}
static int integration_feedback_query(const char *project, int contextual) {
    INTEGRATION_CLI(project, 0, "query", "-s", contextual ?
        "SELECT name,status,range_start,range_end,selected_text FROM _meta_comments" :
        "SELECT name,status,target_path,range_start,range_end,selected_text FROM _meta_comments ORDER BY name"); return 0;
}
static int integration_resolve(const char *project, const char *name) {
    char items[512]; TEST_ASSERT(name && !strpbrk(name, "\"\\\r\n"));
    int count = snprintf(items, sizeof(items), "[{\"name\":\"%s\",\"fields\":{\"status\":\"resolved\"}}]", name);
    TEST_ASSERT(count >= 0 && count < (int)sizeof(items));
    INTEGRATION_CLI(project, 0, "update", "-c", "_meta_comments", "-i", items); return 0;
}
static int integration_copy_text(char *output, usize capacity, const char *text) {
    TEST_ASSERT(text && strlen(text) < capacity); memcpy(output, text, strlen(text) + 1); return 0;
}
static int integration_feedback(int absolute) {
    char project[NATIVE_PATH_SIZE], target[NATIVE_PATH_SIZE], original[NATIVE_PATH_SIZE], alternate[NATIVE_PATH_SIZE], config[NATIVE_PATH_SIZE];
    TEST_ASSERT_FILE_OK(file_tree_join(project, source_test_case_root, "feedback café Ω")); TEST_ASSERT(file_directory_create(project, 1));
    TEST_ASSERT_FILE_OK(file_tree_join(target, project, "document.md")); TEST_ASSERT_FILE_OK(file_create_text((cstring){target}, source_test_saved));
    char leaves[2][256];
    for (int i = 0; i < 2; ++i) {
        TEST_ASSERT(integration_send(project, target, 0) == 0); const char *path = integration_text("path"); TEST_ASSERT(path != NULL);
        const char *leaf = path; for (const char *p = path; *p; ++p) if (*p == '/' || *p == '\\') leaf = p + 1;
        TEST_ASSERT(integration_copy_text(leaves[i], sizeof(leaves[i]), leaf) == 0);
    }
    TEST_ASSERT_FILE_OK(file_tree_join(original, project, "records/_meta_comments"));
    TEST_ASSERT_FILE_OK(file_tree_join(alternate, project, "feedback elsewhere"));
    TEST_ASSERT_FILE_OK(file_directory_copy(&source_test_files, (cstring){original}, (cstring){alternate}));
    TEST_ASSERT_FILE_OK(file_directory_remove_tree(&source_test_files, (cstring){original}));
    TEST_ASSERT_FILE_OK(file_tree_join(config, project, ".pi/attendant.tables"));
    char config_text[NATIVE_PATH_SIZE + 128]; int count = snprintf(config_text, sizeof(config_text), "# Human config comment\r\n%s as _meta_comments\r\n", absolute ? alternate : "feedback elsewhere");
    TEST_ASSERT(count >= 0 && count < (int)sizeof(config_text)); TEST_ASSERT_FILE_OK(file_write_bytes((cstring){config}, config_text, strlen(config_text)));
    TEST_ASSERT(integration_send(project, target, 0) == 0); TEST_ASSERT_FILE_TEXT(((cstring){config}), config_text);
    char before[NATIVE_PATH_SIZE], previous[16384]; TEST_ASSERT(integration_snapshot(project, 0, before) == 0);
    TEST_ASSERT(integration_feedback_query(project, 0) == 0); TEST_ASSERT(integration_number_is("returned", 3) && integration_number_is("meta_comments.pending", 3));
    TEST_ASSERT(integration_count("rows") == 3);
    for (int i = 0; i < 3; ++i) {
        char path[64]; snprintf(path, sizeof(path), "rows.%d.selected_text", i); TEST_ASSERT(integration_text_is(path, source_test_selected));
        snprintf(path, sizeof(path), "rows.%d.range_start", i); TEST_ASSERT(integration_number_is(path, strlen("---\r\nname: file\r\n---\r\nHello ")));
        snprintf(path, sizeof(path), "rows.%d.range_end", i); TEST_ASSERT(integration_number_is(path, strlen("---\r\nname: file\r\n---\r\nHello ") + 12));
        snprintf(path, sizeof(path), "rows.%d.target_path", i); const char *actual = integration_text(path); TEST_ASSERT(actual != NULL);
        char canonical[NATIVE_PATH_SIZE]; TEST_ASSERT_FILE_OK(file_tree_path((cstring){(char *)actual}, canonical, 0)); TEST_ASSERT(!strcmp(canonical, target));
    }
    TEST_ASSERT(integration_copy_text(previous, sizeof(previous), (const char *)integration_result.output.data) == 0);
    TEST_ASSERT(integration_feedback_query(project, 0) == 0 && integration_json_matches("rows", previous, "rows"));
    TEST_ASSERT(integration_unchanged(project, before, 0) == 0); integration_check("CLI reads API-created records/ranges without consuming or changing source");
    char names[2][256];
    TEST_ASSERT(integration_copy_text(names[0], sizeof(names[0]), integration_text("rows.0.name")) == 0);
    TEST_ASSERT(integration_copy_text(names[1], sizeof(names[1]), integration_text("rows.1.name")) == 0);
    for (int i = 0; i < 2; ++i) TEST_ASSERT(integration_resolve(project, names[i]) == 0);
    TEST_ASSERT(integration_feedback_query(project, 0) == 0 && integration_number_is("meta_comments.pending", 1));
    char suffix[1024]; snprintf(suffix, sizeof(suffix), "---\n%s", source_test_comment);
    for (int i = 0; i < 2; ++i) { char path[NATIVE_PATH_SIZE]; TEST_ASSERT_FILE_OK(file_tree_join(path, alternate, leaves[i])); TEST_ASSERT_FILE_ENDS_WITH_TEXT(((cstring){path}), suffix); }
    integration_check("explicit CLI resolution preserves human body and decreases pending count");
    char contextual[NATIVE_PATH_SIZE], context_target[NATIVE_PATH_SIZE], context_record[NATIVE_PATH_SIZE];
    TEST_ASSERT_FILE_OK(file_tree_join(contextual, source_test_case_root, "context café Ω")); TEST_ASSERT(file_directory_create(contextual, 1));
    TEST_ASSERT_FILE_OK(file_tree_join(context_target, contextual, "preview.md")); TEST_ASSERT_FILE_OK(file_create_text((cstring){context_target}, source_test_saved));
    TEST_ASSERT(integration_send(contextual, context_target, 1) == 0);
    const char *record = integration_text("path"); TEST_ASSERT(record != NULL);
    TEST_ASSERT_FILE_OK(file_tree_path((cstring){(char *)record}, context_record, 0));
    TEST_ASSERT(!strncmp(context_record, contextual, strlen(contextual)) && context_record[strlen(contextual)] == '/');
    TEST_ASSERT(integration_feedback_query(contextual, 1) == 0 && integration_number_is("meta_comments.pending", 1));
    TEST_ASSERT(integration_number_is("rows.0.range_start", 0) && integration_number_is("rows.0.range_end", 0) && integration_text_is("rows.0.selected_text", ""));
    TEST_ASSERT(integration_copy_text(previous, sizeof(previous), (const char *)integration_result.output.data) == 0);
    TEST_ASSERT(integration_feedback_query(contextual, 1) == 0 && integration_json_matches("rows", previous, "rows"));
    integration_check("CLI reads API-created absent anchor without consuming");
    TEST_ASSERT(integration_copy_text(names[0], sizeof(names[0]), integration_text("rows.0.name")) == 0);
    TEST_ASSERT(integration_resolve(contextual, names[0]) == 0);
    TEST_ASSERT(integration_feedback_query(contextual, 1) == 0 && integration_text_is("rows.0.status", "resolved") && integration_number_is("meta_comments.pending", 0));
    snprintf(suffix, sizeof(suffix), "---\n%s", source_test_context); TEST_ASSERT_FILE_ENDS_WITH_TEXT(((cstring){context_record}), suffix);
    integration_check("explicit CLI context resolution preserves body and absent anchor"); return 0;
}
TEST(api_cli_relative_feedback, TEST_ASSERT(integration_feedback(0) == 0);)
TEST(api_cli_absolute_feedback, TEST_ASSERT(integration_feedback(1) == 0);)
