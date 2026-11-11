#include "test.h"

static int integration_installation_create(void) {
    char temporary[NATIVE_PATH_SIZE], path[NATIVE_PATH_SIZE];
#if defined(_WIN32)
    wchar_t wide[NATIVE_PATH_SIZE]; DWORD count = GetTempPathW(NATIVE_PATH_SIZE, wide);
    TEST_ASSERT(count && count < NATIVE_PATH_SIZE);
    TEST_ASSERT(WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, wide, -1, temporary, sizeof(temporary), NULL, NULL));
    unsigned long process = GetCurrentProcessId();
#elif defined(__APPLE__)
    /* /var and /tmp are aliases on macOS; use physical standard temp root. */
    strcpy(temporary, "/private/tmp"); unsigned long process = (unsigned long)getpid();
#else
    strcpy(temporary, "/tmp"); unsigned long process = (unsigned long)getpid();
#endif
    u64 stamp; TEST_ASSERT(test_command_clock(&stamp));
    char label[128]; snprintf(label, sizeof(label), "attendant-native-%lu-%llu café", process, (unsigned long long)stamp);
    TEST_ASSERT_FILE_OK(file_tree_join(path, temporary, label));
    TEST_ASSERT_FILE_OK(test_directory_create(&integration_installation, (cstring){path}));
    fprintf(stderr, "temporary-installation:%s\n", integration_installation.path);
    TEST_ASSERT_FILE_OK(file_tree_join(path, source_test_case_root, "installation-path.txt"));
    TEST_ASSERT_FILE_OK(file_create_text((cstring){path}, integration_installation.path)); return 0;
}
TEST(clean_install_missing_build_recovery,
    TEST_ASSERT(integration_installation_create() == 0);
    char skill[NATIVE_PATH_SIZE], copied[NATIVE_PATH_SIZE], native[NATIVE_PATH_SIZE], project[NATIVE_PATH_SIZE];
    TEST_ASSERT_FILE_OK(file_tree_path((cstring){".."}, skill, 0));
    TEST_ASSERT_FILE_OK(file_tree_join(copied, integration_installation.path, "attendant")); TEST_ASSERT(file_directory_create(copied, 1));
    TEST_ASSERT(integration_copy_entries(skill, copied, "native") == 0);
    TEST_ASSERT_FILE_OK(file_tree_join(native, copied, "native")); TEST_ASSERT(file_directory_create(native, 1));
    TEST_ASSERT(integration_copy_entries(integration_root, native, "build") == 0);
    TEST_ASSERT(integration_project(project) == 0);
    const char *retired[] = {"scripts/attendant.mjs", "scripts/yaml.mjs", "test/attendant.test.js", "references/migration.md", "references/native-commands.md"};
    for (usize i = 0; i < sizeof(retired) / sizeof(retired[0]); ++i) {
        char path[NATIVE_PATH_SIZE]; TEST_ASSERT_FILE_OK(file_tree_join(path, copied, retired[i])); TEST_ASSERT(integration_missing(path));
    }
    char executable[NATIVE_PATH_SIZE], relative[128];
#if defined(_WIN32)
    snprintf(relative, sizeof(relative), "build/attendant-%s.exe", integration_compiler);
#else
    snprintf(relative, sizeof(relative), "build/attendant-%s", integration_compiler);
#endif
    TEST_ASSERT_FILE_OK(file_tree_join(executable, native, relative)); TEST_ASSERT(integration_missing(executable));
    const char *schema[] = {executable, "schema"};
    test_command command = {.arguments = schema, .argument_count = 2, .working_directory = project, .timeout_ms = 30000};
    TEST_ASSERT(test_command_run(&integration_commands, &command, &integration_result) == test_command_launch_failed);
#if defined(_WIN32)
    TEST_ASSERT(integration_result.native_error == ERROR_FILE_NOT_FOUND || integration_result.native_error == ERROR_PATH_NOT_FOUND);
#else
    TEST_ASSERT(integration_result.native_error == ENOENT);
#endif
    const char *build[] = {"bash", "build.sh", integration_compiler};
    test_command_environment clean_cache[] = {{"ZIG_GLOBAL_CACHE_DIR", NULL}, {"ZIG_LOCAL_CACHE_DIR", NULL}};
    command.arguments = build; command.argument_count = 3; command.working_directory = native; command.timeout_ms = 180000;
    command.environment = clean_cache; command.environment_count = 2;
    TEST_ASSERT(integration_execute(&command, 0) == 0);
    const char *help[] = {executable, "--help"}; command.arguments = help; command.argument_count = 2; command.working_directory = project; command.timeout_ms = 10000;
    command.environment = NULL; command.environment_count = 0;
    TEST_ASSERT(integration_execute(&command, 0) == 0 && integration_json_parse(0, integration_result.output.data, integration_result.output.len) == 0);
    const char *usage = integration_text("usage"); TEST_ASSERT(usage && strstr(usage, "doctor|validate|add-table|create|update|sync|schema|query|search"));
    command.arguments = schema; command.timeout_ms = 30000;
    TEST_ASSERT(integration_execute(&command, 0) == 0 && integration_json_parse(0, integration_result.output.data, integration_result.output.len) == 0);
    TEST_ASSERT(integration_count("collections") == 2 && integration_text_is("collections.0.name", "decisions") && integration_text_is("collections.1.name", "tasks"));
    char actual[NATIVE_PATH_SIZE]; TEST_ASSERT_FILE_OK(file_tree_join(actual, project, ".pi/attendant.tables"));
    TEST_ASSERT_FILES_EQUAL(((cstring){actual}), ((cstring){"tests/fixtures/project/.pi/attendant.tables"}));
    integration_check("missing executable, isolated build from skill copy, help and return to original workspace");
)
