/* Attendant subprocess protocol/fixtures only. Generic supervision is test.h. */
#include "tests/core/c/support.h"
#include "tests/support/integration-json.h"

static u8 integration_file_memory[4194304], integration_command_memory[262144];
static char integration_output[1048577], integration_errors[1048577];
static char integration_root[NATIVE_PATH_SIZE], integration_cli_path[NATIVE_PATH_SIZE], integration_probe[NATIVE_PATH_SIZE], integration_compiler[16];
static unsigned integration_groups;
static test_command_workspace integration_commands;
static test_command_result integration_result;
static test_directory integration_installation;

static int integration_suite_setup(void) {
    if (source_test_suite_setup()) return 1;
    source_test_files = (file_tree_workspace){arena_from_buffer(integration_file_memory, sizeof(integration_file_memory)), 32, 8192};
    integration_commands = (test_command_workspace){arena_from_buffer(integration_command_memory, sizeof(integration_command_memory)),
        integration_output, integration_errors, sizeof(integration_output), sizeof(integration_errors)};
    usize size;
    TEST_ASSERT(environment_get(source_test_files.memory, "ATTENDANT_TEST_COMPILER", integration_compiler, sizeof(integration_compiler), &size) == environment_error_ok);
    TEST_ASSERT(!strcmp(integration_compiler, "cjit") || !strcmp(integration_compiler, "zig"));
    TEST_ASSERT_FILE_OK(file_tree_path((cstring){"."}, integration_root, 0));
#if defined(_WIN32)
    const char *suffix = ".exe";
#else
    const char *suffix = "";
#endif
    char relative[NATIVE_PATH_SIZE];
    snprintf(relative, sizeof(relative), "build/attendant-%s%s", integration_compiler, suffix);
    TEST_ASSERT_FILE_OK(file_tree_join(integration_cli_path, integration_root, relative));
    snprintf(relative, sizeof(relative), "build/api-probe-%s%s", integration_compiler, suffix);
    TEST_ASSERT_FILE_OK(file_tree_join(integration_probe, integration_root, relative));
    TEST_ASSERT(file_is_regular(integration_cli_path, 0) && file_is_regular(integration_probe, 0));
    return 0;
}
static int integration_suite_teardown(int failed) {
    fprintf(stderr, "integration-groups:%u\n", integration_groups);
    int missing = !failed && integration_groups != 23;
    int result = source_test_suite_teardown(failed || missing);
    return result || missing;
}
static int integration_case_teardown(const test_case *test, int failed) {
    arena_reset(source_test_files.memory);
    file_error installation = test_directory_finish(&integration_installation, &source_test_files, failed);
    int result = source_test_case_teardown(test, failed || installation != file_error_ok);
    return result || installation != file_error_ok;
}
#undef TEST_SUITE_SETUP
#undef TEST_SUITE_TEARDOWN
#undef TEST_TEARDOWN
#define TEST_SUITE_SETUP integration_suite_setup
#define TEST_SUITE_TEARDOWN integration_suite_teardown
#define TEST_TEARDOWN integration_case_teardown
static void integration_check(const char *name) { ++integration_groups; fprintf(stderr, "check:%s\n", name); }

static int integration_execute(const test_command *command, unsigned long expected) {
    test_command_error error = test_command_run(&integration_commands, command, &integration_result);
    char stem[NATIVE_PATH_SIZE], path[NATIVE_PATH_SIZE];
    TEST_ASSERT_FILE_OK(test_directory_next(&source_test_case_directory, "command", stem));
    TEST_ASSERT(snprintf(path, sizeof(path), "%s.stdout", stem) < (int)sizeof(path));
    TEST_ASSERT_FILE_OK(file_create_bytes((cstring){path}, integration_result.output.data, integration_result.output.len));
    TEST_ASSERT(snprintf(path, sizeof(path), "%s.stderr", stem) < (int)sizeof(path));
    TEST_ASSERT_FILE_OK(file_create_bytes((cstring){path}, integration_result.error_output.data, integration_result.error_output.len));
    if (error != test_command_ok || !integration_result.exited || integration_result.exit_code != expected) {
        fprintf(stderr, "Command %s: %s, OS %lu, exit %lu, signal %d\n%.*s\n", command->arguments[0], test_command_error_message(error),
            integration_result.native_error, integration_result.exit_code, integration_result.signal_number,
            (int)(integration_result.error_output.len > 4096 ? 4096 : integration_result.error_output.len), integration_result.error_output.data);
    }
    TEST_ASSERT_COMMAND_OK(error); TEST_ASSERT_COMMAND_EXIT(&integration_result, expected); return 0;
}
static int integration_cli(const char *project, const char *const *arguments, usize count, unsigned long expected, const char *input) {
    TEST_ASSERT(count + 3 <= PROCESS_MAX_ARGUMENTS);
    const char *all[PROCESS_MAX_ARGUMENTS]; all[0] = integration_cli_path;
    for (usize i = 0; i < count; ++i) all[i + 1] = arguments[i];
    all[count + 1] = "-p"; all[count + 2] = project;
    test_command_environment environment = {"ATTENDANT_FEEDBACK_FAULT", ""};
    test_command command = {.arguments = all, .argument_count = count + 3, .environment = &environment, .environment_count = 1,
        .input = input, .input_size = input ? strlen(input) : 0, .timeout_ms = 30000};
    TEST_ASSERT(integration_execute(&command, expected) == 0);
    return integration_json_parse(0, integration_result.output.data, integration_result.output.len);
}
#define INTEGRATION_CLI(project, expected, ...) \
    do { const char *arguments[] = {__VA_ARGS__}; \
        TEST_ASSERT(integration_cli((project), arguments, sizeof(arguments) / sizeof(arguments[0]), (expected), NULL) == 0); } while (0)
#define INTEGRATION_STDIN(project, input, ...) \
    do { const char *arguments[] = {__VA_ARGS__}; \
        TEST_ASSERT(integration_cli((project), arguments, sizeof(arguments) / sizeof(arguments[0]), 0, (input)) == 0); } while (0)

/* Explicit one-level copy/filter. Bounded shared tree copy handles each subtree.
   Attendant cache/build exclusions stay here, not in base.h/test.h. */
static int integration_copy_entries(const char *source, const char *destination, const char *skip) {
    char checked[NATIVE_PATH_SIZE]; TEST_ASSERT_FILE_OK(file_tree_path((cstring){(char *)source}, checked, 0));
    file_directory directory; TEST_ASSERT(file_directory_open(&directory, source));
    int state = 0, ok = 1; usize visited = 0;
    while ((state = file_directory_read(&directory)) > 0) {
        if (++visited > 8192) { ok = 0; break; }
        const char *name = directory.name;
        if (!strcmp(name, ".") || !strcmp(name, "..") || (skip && !strcmp(name, skip))) continue;
        char from[NATIVE_PATH_SIZE], to[NATIVE_PATH_SIZE];
        if (file_tree_join(from, source, name) != file_error_ok || file_tree_join(to, destination, name) != file_error_ok || file_path_is_link(from)) { ok = 0; break; }
        if (file_is_directory(from, 0)) ok = file_directory_copy(&source_test_files, (cstring){from}, (cstring){to}) == file_error_ok;
        else ok = file_is_regular(from, 0) && file_copy(from, to, 1);
        if (!ok) break;
    }
    int closed = file_directory_close(&directory);
    TEST_ASSERT(ok && closed && state >= 0); return 0;
}
static int integration_missing(const char *path) {
    file_entry_kind kind; return file_native_kind(path, &kind) == file_error_not_found;
}
static int integration_project(char project[NATIVE_PATH_SIZE]) {
    TEST_ASSERT_FILE_OK(file_tree_join(project, source_test_case_root, "project café Ω"));
    TEST_ASSERT_FILE_OK(file_directory_copy(&source_test_files, (cstring){"tests/fixtures/project"}, (cstring){project})); return 0;
}
static int integration_snapshot(const char *project, int cache, char path[NATIVE_PATH_SIZE]) {
    TEST_ASSERT_FILE_OK(test_directory_next(&source_test_case_directory, "snapshot", path));
    TEST_ASSERT(file_directory_create(path, 1)); return integration_copy_entries(project, path, cache ? NULL : ".attendant");
}
static int integration_unchanged(const char *project, const char *baseline, int cache) {
    char current[NATIVE_PATH_SIZE]; TEST_ASSERT(integration_snapshot(project, cache, current) == 0);
    TEST_ASSERT_DIRECTORIES_EQUAL(&source_test_files, ((cstring){(char *)baseline}), ((cstring){current})); return 0;
}
static int integration_read(const char *path, char *text, usize capacity) {
    TEST_ASSERT(capacity > 0); usize size = 0;
    TEST_ASSERT_FILE_OK(file_read_into((cstring){(char *)path}, text, capacity - 1, &size));
    TEST_ASSERT(!memchr(text, 0, size)); text[size] = 0; return 0;
}
