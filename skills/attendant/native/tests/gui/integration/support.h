/* GUI subprocess fixtures/protocols. Generic supervision remains in test.h. */
#include "src/native-api.c"
#include "tests/support/integration-json.h"

static u8 gui_file_memory[4194304], gui_command_memory[262144];
static char gui_output[1048577], gui_errors[1048577];
static file_tree_workspace gui_files;
static test_command_workspace gui_commands;
static test_command_result gui_result;
static test_command_live gui_live[2];
static char gui_live_output[2][1048577], gui_live_errors[2][1048577];
static test_directory gui_suite, gui_case;
static char gui_root[NATIVE_PATH_SIZE], gui_evidence[NATIVE_PATH_SIZE], gui_assets[NATIVE_PATH_SIZE];
static char gui_production[NATIVE_PATH_SIZE], gui_testing[NATIVE_PATH_SIZE], gui_other[NATIVE_PATH_SIZE], gui_model[NATIVE_PATH_SIZE], gui_cli[NATIVE_PATH_SIZE];
static char gui_compiler[16];
static unsigned gui_groups;
static const char gui_original[] = "# Workspace\r\n\r\nPlain Markdown café Ω 😀.\r\n";
static const char gui_sql[] = "SELECT name,status,target_hash,range_start,range_end,selected_text,source_path FROM _meta_comments";

static int gui_suite_setup(void) {
    gui_files = (file_tree_workspace){arena_from_buffer(gui_file_memory, sizeof(gui_file_memory)), 32, 8192};
    gui_commands = (test_command_workspace){arena_from_buffer(gui_command_memory, sizeof(gui_command_memory)), gui_output, gui_errors, sizeof(gui_output), sizeof(gui_errors)};
    usize size; char supplied[NATIVE_PATH_SIZE], parent[NATIVE_PATH_SIZE], relative[NATIVE_PATH_SIZE], variant[128] = {0};
    TEST_ASSERT_FILE_OK(file_tree_path((cstring){"."}, gui_root, 0));
    TEST_ASSERT_FILE_OK(file_tree_path((cstring){"build/gui-check"}, parent, 0));
    TEST_ASSERT(environment_get(gui_files.memory, "ATTENDANT_TEST_DIRECTORY", supplied, sizeof(supplied), &size) == environment_error_ok);
    TEST_ASSERT_FILE_OK(file_tree_path((cstring){supplied}, gui_evidence, 0));
    usize prefix = strlen(parent);
    TEST_ASSERT(!strncmp(parent, gui_evidence, prefix) && (gui_evidence[prefix] == '/' || gui_evidence[prefix] == '\\'));
    TEST_ASSERT(environment_get(gui_files.memory, "ATTENDANT_TEST_COMPILER", gui_compiler, sizeof(gui_compiler), &size) == environment_error_ok);
    TEST_ASSERT(!strcmp(gui_compiler, "cjit") || !strcmp(gui_compiler, "zig"));
    environment_error error = environment_get(gui_files.memory, "ATTENDANT_GUI_VARIANT", variant, sizeof(variant), &size);
    TEST_ASSERT(error == environment_error_ok || error == environment_error_not_found);
    for (usize i = 0; variant[i]; ++i) TEST_ASSERT((variant[i] >= 'a' && variant[i] <= 'z') || (variant[i] >= '0' && variant[i] <= '9') || (i && variant[i] == '-'));
#if defined(_WIN32)
    const char *suffix = ".exe";
#else
    const char *suffix = "";
#endif
    TEST_ASSERT(snprintf(relative, sizeof(relative), "build/gui%s%s/attendant-gui-%s%s", *variant ? "-" : "", variant, gui_compiler, suffix) < (int)sizeof(relative));
    TEST_ASSERT_FILE_OK(file_tree_join(gui_production, gui_root, relative));
    TEST_ASSERT(snprintf(relative, sizeof(relative), "build/gui%s%s/attendant-gui-%s-check%s", *variant ? "-" : "", variant, gui_compiler, suffix) < (int)sizeof(relative));
    TEST_ASSERT_FILE_OK(file_tree_join(gui_testing, gui_root, relative));
    TEST_ASSERT(snprintf(relative, sizeof(relative), "build/gui%s%s/attendant-gui-%s%s", *variant ? "-" : "", variant, !strcmp(gui_compiler, "cjit") ? "zig" : "cjit", suffix) < (int)sizeof(relative));
    TEST_ASSERT_FILE_OK(file_tree_join(gui_other, gui_root, relative));
    TEST_ASSERT(snprintf(relative, sizeof(relative), "build/gui%s%s/model-check-%s%s", *variant ? "-" : "", variant, gui_compiler, suffix) < (int)sizeof(relative));
    TEST_ASSERT_FILE_OK(file_tree_join(gui_model, gui_root, relative));
    TEST_ASSERT(snprintf(relative, sizeof(relative), "build/attendant-%s%s", gui_compiler, suffix) < (int)sizeof(relative));
    TEST_ASSERT_FILE_OK(file_tree_join(gui_cli, gui_root, relative));
    TEST_ASSERT_FILE_OK(file_tree_join(gui_assets, gui_root, "build/deps/richmd/assets"));
    TEST_ASSERT(file_is_regular(gui_production, 0) && file_is_regular(gui_testing, 0) && file_is_regular(gui_other, 0) && file_is_regular(gui_model, 0) && file_is_regular(gui_cli, 0));
    TEST_ASSERT(file_is_directory(gui_assets, 0));
    TEST_ASSERT_FILE_OK(file_tree_join(relative, gui_evidence, "fixtures"));
    TEST_ASSERT_FILE_OK(test_directory_create(&gui_suite, (cstring){relative}));
    return 0;
}
static int gui_suite_teardown(int failed) {
    int missing = !failed && gui_groups != 10;
    fprintf(stderr, "gui-integration-groups:%u\n", gui_groups);
    arena_reset(gui_files.memory);
    TEST_ASSERT_FILE_OK(test_directory_finish(&gui_suite, &gui_files, failed || missing));
    return missing;
}
static int gui_case_setup(const test_case *test) {
    (void)test; char path[NATIVE_PATH_SIZE]; arena_reset(gui_files.memory);
    memset(gui_live, 0, sizeof(gui_live));
    TEST_ASSERT_FILE_OK(test_directory_next(&gui_suite, "case café Ω", path));
    TEST_ASSERT_FILE_OK(test_directory_create(&gui_case, (cstring){path})); return 0;
}
static int gui_case_teardown(const test_case *test, int failed) {
    (void)test; int cleanup_failed = 0;
    /* Finish both before any assertion can return or any fixture can be removed. */
    for (unsigned i = 0; i < 2; ++i) {
        test_command_error error = test_command_finish(&gui_live[i]);
        if (gui_live[i].result.cleanup_failed || (!failed && error != test_command_ok)) cleanup_failed = 1;
    }
    for (unsigned i = 0; i < 2 && gui_case.owned; ++i) if (gui_live[i].complete) {
        char path[NATIVE_PATH_SIZE];
        int count = snprintf(path, sizeof(path), "%s/live-%u.stdout", gui_case.path, i);
        if (count < 0 || count >= (int)sizeof(path) || file_create_bytes((cstring){path}, gui_live[i].result.output.data, gui_live[i].result.output.len) != file_error_ok) cleanup_failed = 1;
        count = snprintf(path, sizeof(path), "%s/live-%u.stderr", gui_case.path, i);
        if (count < 0 || count >= (int)sizeof(path) || file_create_bytes((cstring){path}, gui_live[i].result.error_output.data, gui_live[i].result.error_output.len) != file_error_ok) cleanup_failed = 1;
    }
    arena_reset(gui_files.memory);
    TEST_ASSERT_FILE_OK(test_directory_finish(&gui_case, &gui_files, failed || cleanup_failed)); return cleanup_failed;
}
#define TEST_SUITE_SETUP gui_suite_setup
#define TEST_SUITE_TEARDOWN gui_suite_teardown
#define TEST_SETUP gui_case_setup
#define TEST_TEARDOWN gui_case_teardown

static int gui_positive_count(const char *path, int *count) {
    integration_json_node *node = integration_json_get(0, path); value number = {0};
    if (!node || node->type != PDJSON_NUMBER || parse_number(node->text, &number) != NUMBER_VALID ||
        number.number_kind != NUMBER_I64 || number.number.signed_integer <= 0 || number.number.signed_integer > INT_MAX) return 0;
    if (count) *count = (int)number.number.signed_integer;
    return 1;
}
static int gui_project(char path[NATIVE_PATH_SIZE]) {
    TEST_ASSERT_FILE_OK(file_tree_join(path, gui_case.path, "project café Ω"));
    TEST_ASSERT_FILE_OK(file_directory_copy(&gui_files, (cstring){"tests/fixtures/project"}, (cstring){path}));
    char target[NATIVE_PATH_SIZE];
    TEST_ASSERT_FILE_OK(file_tree_join(target, path, ".gui-test-owned"));
    TEST_ASSERT_FILE_OK(file_create_text((cstring){target}, "Generated GUI fixture. Safe for test-driver edits.\n"));
    TEST_ASSERT_FILE_OK(file_tree_join(target, path, "README.md"));
    TEST_ASSERT_FILE_OK(file_write_bytes((cstring){target}, gui_original, sizeof(gui_original) - 1)); return 0;
}
static int gui_baseline(const char *project, char path[NATIVE_PATH_SIZE]) {
    TEST_ASSERT_FILE_OK(test_directory_next(&gui_case, "baseline", path));
    TEST_ASSERT_FILE_OK(file_directory_copy(&gui_files, (cstring){(char *)project}, (cstring){path})); return 0;
}
static int gui_read(const char *path, char *text, usize capacity) {
    usize size; TEST_ASSERT(capacity > 0);
    TEST_ASSERT_FILE_OK(file_read_into((cstring){(char *)path}, text, capacity - 1, &size));
    TEST_ASSERT(!memchr(text, 0, size)); text[size] = 0; return 0;
}
static int gui_execute(const char *const *arguments, usize count, unsigned long expected, u32 timeout) {
#if defined(__APPLE__)
    test_command_environment environment = {"DYLD_LIBRARY_PATH", NULL};
#endif
    test_command command = {.arguments = arguments, .argument_count = count, .working_directory = gui_root, .timeout_ms = timeout};
#if defined(__APPLE__)
    command.environment = &environment; command.environment_count = 1;
#endif
    test_command_error error = test_command_run(&gui_commands, &command, &gui_result);
    char stem[NATIVE_PATH_SIZE], path[NATIVE_PATH_SIZE];
    TEST_ASSERT_FILE_OK(test_directory_next(&gui_case, "command", stem));
    TEST_ASSERT(snprintf(path, sizeof(path), "%s.stdout", stem) < (int)sizeof(path));
    TEST_ASSERT_FILE_OK(file_create_bytes((cstring){path}, gui_result.output.data, gui_result.output.len));
    TEST_ASSERT(snprintf(path, sizeof(path), "%s.stderr", stem) < (int)sizeof(path));
    TEST_ASSERT_FILE_OK(file_create_bytes((cstring){path}, gui_result.error_output.data, gui_result.error_output.len));
    if (error != test_command_ok || !gui_result.exited || gui_result.exit_code != expected)
        fprintf(stderr, "GUI command %s: %s, OS %lu, exit %lu, signal %d\n%.*s\n", arguments[0], test_command_error_message(error), gui_result.native_error,
            gui_result.exit_code, gui_result.signal_number, (int)(gui_result.error_output.len > 4096 ? 4096 : gui_result.error_output.len), gui_result.error_output.data);
    TEST_ASSERT_COMMAND_OK(error); TEST_ASSERT_COMMAND_EXIT(&gui_result, expected); return 0;
}
static int gui_window(const char *project, const char *file, int production, unsigned long expected, const char *const *extra, usize count) {
    TEST_ASSERT(count + 9 <= PROCESS_MAX_ARGUMENTS);
    const char *arguments[PROCESS_MAX_ARGUMENTS] = {production ? gui_production : gui_testing, "--project", project, "--assets", gui_assets, "--file", file, "--frames", "70"};
    for (usize i = 0; i < count; ++i) arguments[i + 9] = extra[i];
    TEST_ASSERT(gui_execute(arguments, count + 9, expected, expected ? 30000 : 60000) == 0);
    return expected ? 0 : integration_json_parse(0, gui_result.output.data, gui_result.output.len);
}
#define GUI_WINDOW(project, file, ...) do { const char *extra[] = {__VA_ARGS__}; \
    TEST_ASSERT(gui_window((project), (file), 0, 0, extra, sizeof(extra) / sizeof(extra[0])) == 0); } while (0)
static int gui_query(const char *project) {
    const char *arguments[] = {gui_cli, "query", "-p", project, "-s", gui_sql};
    TEST_ASSERT(gui_execute(arguments, 6, 0, 60000) == 0);
    return integration_json_parse(0, gui_result.output.data, gui_result.output.len);
}
static int gui_resolve(const char *project) {
    const char *name = integration_text("rows.0.name"), *source = integration_text("rows.0.source_path");
    TEST_ASSERT(name && source && !strpbrk(name, "\"\\\r\n"));
    char path[NATIVE_PATH_SIZE], parent[NATIVE_PATH_SIZE], before[16384], after[16384], input[1024];
    TEST_ASSERT_FILE_OK(file_tree_path((cstring){(char *)source}, path, 0));
    TEST_ASSERT_FILE_OK(file_tree_path((cstring){(char *)project}, parent, 0));
    usize prefix = strlen(parent);
    TEST_ASSERT(!strncmp(path, parent, prefix) && (path[prefix] == '/' || path[prefix] == '\\'));
    TEST_ASSERT(gui_read(path, before, sizeof(before)) == 0);
    char *body = strstr(before, "\n---\n"); TEST_ASSERT(body); body += 5;
    TEST_ASSERT(snprintf(input, sizeof(input), "[{\"name\":\"%s\",\"fields\":{\"status\":\"resolved\"}}]", name) < (int)sizeof(input));
    const char *arguments[] = {gui_cli, "update", "-p", project, "-c", "_meta_comments", "-i", input};
    TEST_ASSERT(gui_execute(arguments, 8, 0, 60000) == 0);
    TEST_ASSERT(integration_json_parse(0, gui_result.output.data, gui_result.output.len) == 0);
    TEST_ASSERT(gui_query(project) == 0 && integration_number_is("meta_comments.pending", 0));
    TEST_ASSERT(gui_read(path, after, sizeof(after)) == 0);
    char *updated = strstr(after, "\n---\n"); TEST_ASSERT(updated && !strcmp(body, updated + 5)); return 0;
}
/* Inspect only bounded P6 prefix/size; never allocate whole high-DPI framebuffer. */
static int gui_screenshot(const char *path) {
    void *stream = file_stream_open(path, 0); TEST_ASSERT(stream);
    u64 size = 0; char header[81] = {0};
    int ok = file_stream_size(stream, &size) && size > 0 && size <= 16384ull * 16384ull * 3ull + 80;
    usize prefix = size < 80 ? (usize)size : 80;
    if (ok) ok = file_stream_transfer(stream, header, prefix, 0);
    if (!file_stream_close(stream)) ok = 0;
    TEST_ASSERT(ok);
    unsigned long width = 0, height = 0; char expected[80];
    TEST_ASSERT(sscanf(header, "P6\n%5lu %5lu\n255\n", &width, &height) == 2 && width && height && width <= 16384 && height <= 16384);
    int length = snprintf(expected, sizeof(expected), "P6\n%lu %lu\n255\n", width, height);
    TEST_ASSERT(length > 0 && length <= (int)prefix && !memcmp(header, expected, (usize)length));
    TEST_ASSERT(size == (u64)length + (u64)width * height * 3); return 0;
}
