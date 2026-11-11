#include "test.h"

TEST(skill_template_workflows,
    char project[NATIVE_PATH_SIZE], templates[NATIVE_PATH_SIZE], path[NATIVE_PATH_SIZE];
    TEST_ASSERT_FILE_OK(file_tree_path((cstring){"../../init-project/templates/records"}, templates, 0));
    TEST_ASSERT_FILE_OK(file_tree_join(project, source_test_case_root, "skills café Ω")); TEST_ASSERT(file_directory_create(project, 1));
    TEST_ASSERT_FILE_OK(file_tree_join(path, project, ".pi")); TEST_ASSERT(file_directory_create(path, 1));
    TEST_ASSERT_FILE_OK(file_tree_join(path, project, ".pi/attendant.tables")); TEST_ASSERT_FILE_OK(file_create_text((cstring){path}, ""));
    TEST_ASSERT_FILE_OK(file_tree_join(path, project, ".gitignore")); TEST_ASSERT_FILE_OK(file_create_text((cstring){path}, ".attendant/\n"));
    const char *collections[] = {"tasks", "decisions"}, *files[] = {".schema.md", ".usage.md", ".template.md"};
    for (usize i = 0; i < 2; ++i) {
        char relative[128]; snprintf(relative, sizeof(relative), "records/%s", collections[i]);
        INTEGRATION_CLI(project, 0, "add-table", "-d", relative, "-a", collections[i]); TEST_ASSERT(integration_text_is("name", collections[i]));
        for (usize j = 0; j < 3; ++j) {
            char from[NATIVE_PATH_SIZE], checked[NATIVE_PATH_SIZE]; snprintf(relative, sizeof(relative), "records/%s/%s", collections[i], files[j]);
            TEST_ASSERT_FILE_OK(file_tree_join(path, project, relative)); TEST_ASSERT_FILE_TEXT(((cstring){path}), j ? "" : "---\n---\n");
            snprintf(relative, sizeof(relative), "%s/%s", collections[i], files[j]); TEST_ASSERT_FILE_OK(file_tree_join(from, templates, relative));
            TEST_ASSERT_FILE_OK(file_tree_path((cstring){from}, checked, 0)); TEST_ASSERT(file_copy(checked, path, 0));
        }
    }
    INTEGRATION_CLI(project, 0, "schema"); TEST_ASSERT(integration_count("collections") == 2);
    for (int i = 0; i < 2; ++i) {
        char field[64], relative[128], expected[16384], canonical[NATIVE_PATH_SIZE]; snprintf(field, sizeof(field), "collections.%d.name", i);
        const char *name = integration_text(field); TEST_ASSERT(name && (!strcmp(name, "tasks") || !strcmp(name, "decisions")));
        snprintf(relative, sizeof(relative), "%s/.usage.md", name); TEST_ASSERT_FILE_OK(file_tree_join(path, templates, relative)); TEST_ASSERT(integration_read(path, expected, sizeof(expected)) == 0);
        snprintf(field, sizeof(field), "collections.%d.usage", i); TEST_ASSERT(integration_text_is(field, expected));
        snprintf(relative, sizeof(relative), "records/%s/.usage.md", name); TEST_ASSERT_FILE_OK(file_tree_join(path, project, relative));
        snprintf(field, sizeof(field), "collections.%d.usagePath", i); const char *usage_path = integration_text(field); TEST_ASSERT(usage_path != NULL);
        TEST_ASSERT_FILE_OK(file_tree_path((cstring){(char *)usage_path}, canonical, 0)); TEST_ASSERT(!strcmp(canonical, path));
    }
    integration_check("empty setup and approved template authoring; native schema discovers current usage");
    char items[NATIVE_PATH_SIZE], input[NATIVE_PATH_SIZE + 2], source[NATIVE_PATH_SIZE], original[16384], prose[16384], saved[16384];
    TEST_ASSERT_FILE_OK(file_tree_join(items, project, "items.json"));
    TEST_ASSERT_FILE_OK(file_create_text((cstring){items}, "[{\"name\":\"native-capture\",\"fields\":{\"status\":\"needs-refinement\",\"desc\":\"Native workflow capture\"}}]"));
    snprintf(input, sizeof(input), "@%s", items); INTEGRATION_CLI(project, 0, "create", "-c", "tasks", "-i", input); TEST_ASSERT(integration_count("records") == 1);
    TEST_ASSERT_FILE_OK(file_tree_join(source, project, "records/tasks/native-capture.md")); TEST_ASSERT(integration_read(source, original, sizeof(original)) == 0);
    TEST_ASSERT_FILE_OK(file_tree_join(path, templates, "tasks/.template.md")); TEST_ASSERT(integration_read(path, saved, sizeof(saved)) == 0);
    TEST_ASSERT_FILE_ENDS_WITH_TEXT(((cstring){source}), saved);
    int count = snprintf(prose, sizeof(prose), "%s\nHuman-authored nativeworkflowuniquetoken detail.\n", original); TEST_ASSERT(count >= 4 && count < (int)sizeof(prose));
    TEST_ASSERT_FILE_OK(file_write_bytes((cstring){source}, prose, strlen(prose)));
    const char *body = strstr(prose + 4, "\n---"); TEST_ASSERT(body != NULL); body += 4;
    const char *statuses[] = {"todo", "doing", "review"};
    for (usize i = 0; i < 3; ++i) {
        char payload[256]; snprintf(payload, sizeof(payload), "[{\"name\":\"native-capture\",\"fields\":{\"status\":\"%s\"}}]", statuses[i]);
        INTEGRATION_CLI(project, 0, "update", "-c", "tasks", "-i", payload);
        TEST_ASSERT(integration_read(source, saved, sizeof(saved)) == 0 && strlen(saved) >= 4);
        const char *updated_body = strstr(saved + 4, "\n---"); TEST_ASSERT(updated_body && !strcmp(updated_body + 4, body));
    }
    INTEGRATION_CLI(project, 0, "query", "-s", "SELECT name,status,priority,created_at,source_path FROM tasks WHERE status=:status ORDER BY priority DESC,created_at,name", "-P", "{\"status\":\"review\"}");
    TEST_ASSERT(integration_count("rows") == 1 && integration_number_is("rows.0.priority", 0) && integration_text_is("rows.0.name", "native-capture") && integration_text("rows.0.created_at"));
    INTEGRATION_CLI(project, 0, "search", "-q", "nativeworkflowuniquetoken", "-c", "tasks"); TEST_ASSERT(integration_text_is("rows.0.name", "native-capture"));
    integration_check("capture, refinement/implementation status flow, ordering/search and body preservation");
    INTEGRATION_CLI(project, 0, "create", "-c", "decisions", "-i", "[{\"name\":\"native-decision\",\"fields\":{\"status\":\"accepted\",\"revisit_triggers\":[\"native-change\"]}}]");
    INTEGRATION_CLI(project, 0, "query", "-s", "SELECT 'decisions' AS collection,name,status FROM decisions WHERE revisit_triggers LIKE :trigger_pattern", "-P", "{\"trigger_pattern\":\"%\\\"native-change\\\"%\"}");
    TEST_ASSERT(integration_json_matches("rows", "[{\"collection\":\"decisions\",\"name\":\"native-decision\",\"status\":\"accepted\"}]", NULL));
    integration_check("tradeoff-review decision template and documented revisit-trigger query");
    char before[NATIVE_PATH_SIZE]; TEST_ASSERT(integration_snapshot(project, 1, before) == 0);
    INTEGRATION_CLI(project, 0, "doctor"); TEST_ASSERT(integration_json_get(0, "ok") && integration_json_get(0, "ok")->type == PDJSON_TRUE);
    INTEGRATION_CLI(project, 1, "migrate"); TEST_ASSERT(integration_unchanged(project, before, 1) == 0);
    INTEGRATION_CLI(project, 0, "validate", "--strict"); TEST_ASSERT(integration_count("diagnostics") == 0);
    integration_check("read-only health check, unsupported migration and strict validation");
)
