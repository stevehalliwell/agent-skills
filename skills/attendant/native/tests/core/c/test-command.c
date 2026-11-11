#include "test.h"

static u8 command_test_memory[262144];
static char command_test_output[1048577], command_test_errors[1048577];
static int command_test_prepare(test_command_workspace *workspace, char helper[NATIVE_PATH_SIZE]) {
    usize size;
    if (environment_get(source_test_files.memory, "ATTENDANT_TEST_COMMAND_HELPER", helper, NATIVE_PATH_SIZE, &size) != environment_error_ok || !size) return 0;
    *workspace = (test_command_workspace){arena_from_buffer(command_test_memory, sizeof(command_test_memory)),
        command_test_output, command_test_errors, sizeof(command_test_output), sizeof(command_test_errors)};
    return workspace->scratch != NULL;
}

TEST(command_arguments_unicode_quotes_and_no_shell,
    test_command_workspace workspace; char helper[NATIVE_PATH_SIZE];
    TEST_ASSERT(command_test_prepare(&workspace, helper));
    const char *arguments[] = {helper, "args", "", "space here", "café Ω 😀", "a\"b", "trail\\", "slashes\\\\\"quote", "|&;$(echo BAD)", "line\nnext"};
    char expected[1024] = {0}; usize used = 0;
    for (usize i = 2; i < sizeof(arguments) / sizeof(arguments[0]); ++i) {
        int count = snprintf(expected + used, sizeof(expected) - used, "%zu:%s\n", strlen(arguments[i]), arguments[i]);
        TEST_ASSERT(count >= 0 && (usize)count < sizeof(expected) - used); used += count;
    }
    test_command command = {.arguments = arguments, .argument_count = sizeof(arguments) / sizeof(arguments[0]), .timeout_ms = 30000};
    test_command_result result;
    int calls = 0; const test_command *commands[] = {&command};
    TEST_ASSERT_COMMAND_OK(test_command_run(&workspace, commands[calls++], &result)); TEST_ASSERT(calls == 1);
    test_command_result *results[] = {&result}; calls = 0;
    TEST_ASSERT_COMMAND_EXIT(results[calls++], 0); TEST_ASSERT(calls == 1);
    const char *texts[] = {expected}; int text_calls = 0; calls = 0;
    TEST_ASSERT_COMMAND_OUTPUT_TEXT(results[calls++], texts[text_calls++]); TEST_ASSERT(calls == 1 && text_calls == 1);
    TEST_ASSERT_COMMAND_ERROR_TEXT(&result, "");
)

TEST(command_binary_stdin_eof_and_separate_capture,
    test_command_workspace workspace; char helper[NATIVE_PATH_SIZE];
    TEST_ASSERT(command_test_prepare(&workspace, helper));
    u8 input[200000]; for (usize i = 0; i < sizeof(input); ++i) input[i] = (u8)(i % 251);
    const char *arguments[] = {helper, "echo"};
    test_command command = {.arguments = arguments, .argument_count = 2, .input = input, .input_size = sizeof(input), .timeout_ms = 30000};
    test_command_result result;
    TEST_ASSERT(arena_alloc(workspace.scratch, 32, 1).error == arena_error_ok);
    usize mark = arena_pos(workspace.scratch);
    TEST_ASSERT_COMMAND_OK(test_command_run(&workspace, &command, &result)); TEST_ASSERT_COMMAND_EXIT(&result, 0);
    const void *bytes[] = {input}; usize sizes[] = {sizeof(input)}; test_command_result *results[] = {&result};
    int result_calls = 0, byte_calls = 0, size_calls = 0;
    TEST_ASSERT_COMMAND_OUTPUT_BYTES(results[result_calls++], bytes[byte_calls++], sizes[size_calls++]);
    TEST_ASSERT(result_calls == 1 && byte_calls == 1 && size_calls == 1);
    TEST_ASSERT_COMMAND_ERROR_TEXT(&result, "error café Ω\n");
    TEST_ASSERT(arena_pos(workspace.scratch) == mark);
    command.input = NULL; command.input_size = 0;
    TEST_ASSERT_COMMAND_OK(test_command_run(&workspace, &command, &result)); TEST_ASSERT_COMMAND_EXIT(&result, 0);
    TEST_ASSERT_COMMAND_OUTPUT_BYTES(&result, NULL, 0); TEST_ASSERT_COMMAND_ERROR_TEXT(&result, "error café Ω\n");
)

TEST(command_cwd_and_child_environment_do_not_mutate_parent,
    test_command_workspace workspace; char helper[NATIVE_PATH_SIZE], parent[NATIVE_PATH_SIZE];
    TEST_ASSERT(command_test_prepare(&workspace, helper));
    TEST_ASSERT_FILE_OK(file_tree_path((cstring){"."}, parent, 0));
    const char *arguments[] = {helper, "cwd"};
    test_command command = {.arguments = arguments, .argument_count = 2, .working_directory = source_test_case_root, .timeout_ms = 30000};
    test_command_result result;
    TEST_ASSERT_COMMAND_OK(test_command_run(&workspace, &command, &result)); TEST_ASSERT_COMMAND_EXIT(&result, 0);
    TEST_ASSERT(result.output.len && !memchr(result.output.data, 0, result.output.len));
    u64 actual_identity[3], expected_identity[3];
    TEST_ASSERT_FILE_OK(file_directory_identity((const char *)result.output.data, actual_identity));
    TEST_ASSERT_FILE_OK(file_directory_identity(source_test_case_root, expected_identity));
    TEST_ASSERT(!memcmp(actual_identity, expected_identity, sizeof(actual_identity)));
    char current[NATIVE_PATH_SIZE]; TEST_ASSERT_FILE_OK(file_tree_path((cstring){"."}, current, 0)); TEST_ASSERT(!strcmp(current, parent));
    char name[128];
#if defined(_WIN32)
    unsigned long process = GetCurrentProcessId();
#else
    unsigned long process = (unsigned long)getpid();
#endif
    snprintf(name, sizeof(name), "PI_COMMAND_TEST_%lu_café", process);
    char value[128]; usize size;
    TEST_ASSERT(environment_get(source_test_files.memory, name, value, sizeof(value), &size) == environment_error_not_found);
    const char *environment_arguments[] = {helper, "environment", name};
    test_command_environment override = {name, "child café Ω 😀"};
    command.arguments = environment_arguments; command.argument_count = 3; command.environment = &override; command.environment_count = 1;
    TEST_ASSERT_COMMAND_OK(test_command_run(&workspace, &command, &result)); TEST_ASSERT_COMMAND_EXIT(&result, 0);
    TEST_ASSERT_COMMAND_OUTPUT_TEXT(&result, override.value);
    TEST_ASSERT(environment_get(source_test_files.memory, name, value, sizeof(value), &size) == environment_error_not_found);
    override.value = "";
    TEST_ASSERT_COMMAND_OK(test_command_run(&workspace, &command, &result)); TEST_ASSERT_COMMAND_EXIT(&result, 0); TEST_ASSERT_COMMAND_OUTPUT_TEXT(&result, "");
    override.value = NULL;
    TEST_ASSERT_COMMAND_OK(test_command_run(&workspace, &command, &result)); TEST_ASSERT_COMMAND_EXIT(&result, 0); TEST_ASSERT_COMMAND_OUTPUT_TEXT(&result, "<absent>");
    char inherited[32768], unchanged[32768];
    TEST_ASSERT(environment_get(source_test_files.memory, "PATH", inherited, sizeof(inherited), &size) == environment_error_ok);
    environment_arguments[2] = "PATH"; override.name = "PATH"; command.environment_count = 0;
    TEST_ASSERT_COMMAND_OK(test_command_run(&workspace, &command, &result)); TEST_ASSERT_COMMAND_EXIT(&result, 0); TEST_ASSERT_COMMAND_OUTPUT_TEXT(&result, inherited);
    command.environment_count = 1; override.value = NULL;
    TEST_ASSERT_COMMAND_OK(test_command_run(&workspace, &command, &result)); TEST_ASSERT_COMMAND_EXIT(&result, 0); TEST_ASSERT_COMMAND_OUTPUT_TEXT(&result, "<absent>");
    TEST_ASSERT(environment_get(source_test_files.memory, "PATH", unchanged, sizeof(unchanged), &size) == environment_error_ok && !strcmp(inherited, unchanged));
    const char *path_arguments[] = {
#if defined(_WIN32)
        "command-child.exe",
#else
        "command-child",
#endif
        "exit", "0"};
    char directory[NATIVE_PATH_SIZE]; strcpy(directory, helper); char *slash = strrchr(directory, '/');
#if defined(_WIN32)
    char *backslash = strrchr(directory, '\\'); if (!slash || (backslash && backslash > slash)) slash = backslash;
#endif
    TEST_ASSERT(slash != NULL); *slash = 0;
    override.name = "PATH"; override.value = directory;
    command.arguments = path_arguments;
    TEST_ASSERT_COMMAND_OK(test_command_run(&workspace, &command, &result)); TEST_ASSERT_COMMAND_EXIT(&result, 0);
    command.working_directory = directory; override.value = ".";
    TEST_ASSERT_COMMAND_OK(test_command_run(&workspace, &command, &result)); TEST_ASSERT_COMMAND_EXIT(&result, 0);
#if defined(_WIN32)
    TEST_ASSERT(test_command_path("/tools/helper.exe", "D:/child", current) == test_command_ok && !strcmp(current, "D:/tools/helper.exe"));
    TEST_ASSERT(test_command_path("/tools/helper.exe", "//server/share/child", current) == test_command_ok && !strcmp(current, "//server/share/tools/helper.exe"));
    TEST_ASSERT(test_command_path("D:helper.exe", "D:/child", current) == test_command_invalid);
    TEST_ASSERT(test_command_path("/tools/helper.exe", "//?/UNC/server/share/child", current) == test_command_invalid);
    TEST_ASSERT(test_command_path("//?/D:/helper.exe", "D:/child", current) == test_command_invalid);
#endif
)

TEST(command_exact_capacity_and_both_overflow_paths,
    test_command_workspace workspace; char helper[NATIVE_PATH_SIZE];
    TEST_ASSERT(command_test_prepare(&workspace, helper));
    const char *arguments[] = {helper, "args", "abc"};
    test_command command = {.arguments = arguments, .argument_count = 3, .timeout_ms = 30000};
    test_command_result result;
    workspace.output_capacity = strlen("3:abc\n") + 1;
    TEST_ASSERT_COMMAND_OK(test_command_run(&workspace, &command, &result)); TEST_ASSERT_COMMAND_EXIT(&result, 0); TEST_ASSERT_COMMAND_OUTPUT_TEXT(&result, "3:abc\n");
    --workspace.output_capacity;
    TEST_ASSERT(test_command_run(&workspace, &command, &result) == test_command_capacity && !result.cleanup_failed);
    TEST_ASSERT(result.output.len == workspace.output_capacity - 1 && !workspace.output[result.output.len]);
    TEST_ASSERT(arena_pos(workspace.scratch) == 0);
    const char *echo[] = {helper, "echo"}; command.arguments = echo; command.argument_count = 2;
    workspace.output_capacity = sizeof(command_test_output); workspace.error_capacity = 1;
    TEST_ASSERT(test_command_run(&workspace, &command, &result) == test_command_capacity && !result.cleanup_failed);
    TEST_ASSERT(!result.error_output.len && !workspace.error_output[0]);
    const char *flood[] = {helper, "flood"}; command.arguments = flood;
    workspace.error_capacity = sizeof(command_test_errors);
    TEST_ASSERT_COMMAND_OK(test_command_run(&workspace, &command, &result)); TEST_ASSERT_COMMAND_EXIT(&result, 0);
    TEST_ASSERT(result.output.len == 1048576 && result.error_output.len == 1048576);
    workspace.output_capacity = 64;
    TEST_ASSERT(test_command_run(&workspace, &command, &result) == test_command_capacity && !result.cleanup_failed);
)

TEST(command_launch_errors_exit_signal_timeout_and_validation,
    test_command_workspace workspace; char helper[NATIVE_PATH_SIZE], missing[NATIVE_PATH_SIZE];
    TEST_ASSERT(command_test_prepare(&workspace, helper));
    TEST_ASSERT_FILE_OK(file_tree_join(missing, source_test_case_root, "missing.exe"));
    const char *arguments[] = {missing, "exit", "7"};
    test_command command = {.arguments = arguments, .argument_count = 3, .timeout_ms = 30000};
    test_command_result result;
    TEST_ASSERT(test_command_run(&workspace, &command, &result) == test_command_launch_failed && !result.exited && !result.cleanup_failed);
#if defined(_WIN32)
    TEST_ASSERT(result.native_error == ERROR_FILE_NOT_FOUND);
#else
    TEST_ASSERT(result.native_error == ENOENT);
#endif
    arguments[0] = helper;
    TEST_ASSERT_COMMAND_OK(test_command_run(&workspace, &command, &result)); TEST_ASSERT_COMMAND_EXIT(&result, 7);
    const char *crash[] = {helper, "crash"}; command.arguments = crash; command.argument_count = 2;
    TEST_ASSERT_COMMAND_OK(test_command_run(&workspace, &command, &result));
#if defined(_WIN32)
    TEST_ASSERT(result.exited && result.exit_code == 0xc0000005UL && !result.signal_number);
#else
    TEST_ASSERT(!result.exited && result.signal_number == SIGKILL);
#endif
    const char *sleep[] = {helper, "sleep"}; command.arguments = sleep; command.timeout_ms = 100;
    u64 start, end; TEST_ASSERT(test_command_clock(&start));
    TEST_ASSERT(test_command_run(&workspace, &command, &result) == test_command_timeout && !result.cleanup_failed);
    TEST_ASSERT(test_command_clock(&end) && end - start < 5000);
#if !defined(_WIN32)
    char group[32]; snprintf(group, sizeof(group), "%ld", (long)getpgrp());
    const char *moved[] = {helper, "join-group", group}; command.arguments = moved; command.argument_count = 3;
    TEST_ASSERT(test_command_run(&workspace, &command, &result) == test_command_timeout && !result.cleanup_failed);
    TEST_ASSERT(result.signal_number == SIGKILL);
#endif
    command.timeout_ms = 0; TEST_ASSERT(test_command_run(&workspace, &command, &result) == test_command_invalid);
    command.timeout_ms = 30000; command.working_directory = "";
    TEST_ASSERT(test_command_run(&workspace, &command, &result) == test_command_invalid);
    command.working_directory = missing;
    TEST_ASSERT(test_command_run(&workspace, &command, &result) == test_command_launch_failed);
    command.working_directory = NULL; command.input_size = 1;
    TEST_ASSERT(test_command_run(&workspace, &command, &result) == test_command_invalid);
    command.input_size = 0;
    char *separate = workspace.error_output; workspace.error_output = workspace.output;
    TEST_ASSERT(test_command_run(&workspace, &command, &result) == test_command_invalid); workspace.error_output = separate;
    command.input = workspace.output; command.input_size = 1;
    TEST_ASSERT(test_command_run(&workspace, &command, &result) == test_command_invalid); command.input = NULL; command.input_size = 0;
#if defined(_WIN32)
    const char *invalid_utf8[] = {helper, "args", "\xff"}; command.arguments = invalid_utf8; command.argument_count = 3;
    TEST_ASSERT(test_command_run(&workspace, &command, &result) == test_command_invalid);
    test_command_environment duplicate[] = {{"PI_TEST_É", "first"}, {"pi_test_é", "second"}};
    command.arguments = sleep; command.argument_count = 2; command.environment = duplicate; command.environment_count = 2;
    TEST_ASSERT(test_command_run(&workspace, &command, &result) == test_command_invalid);
#endif
    test_command_environment invalid = {"bad=name", "value"}; command.environment = &invalid; command.environment_count = 1;
    TEST_ASSERT(test_command_run(&workspace, &command, &result) == test_command_invalid);
    TEST_ASSERT(test_command_run(NULL, &command, &result) == test_command_invalid);
    u8 limited[128]; workspace.scratch = arena_from_buffer(limited, sizeof(limited));
    command.environment_count = 0;
    TEST_ASSERT(test_command_run(&workspace, &command, &result) == test_command_capacity);
    TEST_ASSERT(arena_pos(workspace.scratch) == 0);
)

TEST(command_owned_descendants_stop_after_root_exit,
    test_command_workspace workspace; char helper[NATIVE_PATH_SIZE], heartbeat[NATIVE_PATH_SIZE];
    TEST_ASSERT(command_test_prepare(&workspace, helper));
    for (int closed = 0; closed < 2; ++closed) {
        TEST_ASSERT_FILE_OK(file_tree_join(heartbeat, source_test_case_root, closed ? "closed café.bin" : "heartbeat café.bin"));
        const char *arguments[] = {helper, closed ? "spawn-heartbeat-closed" : "spawn-heartbeat", heartbeat};
        test_command command = {.arguments = arguments, .argument_count = 3, .timeout_ms = 30000};
        test_command_result result; u64 start, end; TEST_ASSERT(test_command_clock(&start));
        TEST_ASSERT_COMMAND_OK(test_command_run(&workspace, &command, &result)); TEST_ASSERT_COMMAND_EXIT(&result, 0);
        TEST_ASSERT_COMMAND_OUTPUT_TEXT(&result, "spawned\n"); TEST_ASSERT(test_command_clock(&end) && end - start < 5000);
#if defined(_WIN32)
        char identity[256]; usize size = 0;
        TEST_ASSERT_FILE_OK(file_read_into((cstring){heartbeat}, identity, sizeof(identity) - 1, &size)); identity[size] = 0;
        char *end_identity; unsigned long process = strtoul(identity, &end_identity, 10);
        TEST_ASSERT(process && *end_identity == '\n');
        HANDLE descendant = OpenProcess(SYNCHRONIZE, FALSE, (DWORD)process);
        int stopped = descendant ? WaitForSingleObject(descendant, 0) == WAIT_OBJECT_0 : GetLastError() == ERROR_INVALID_PARAMETER;
        int closed_handle = !descendant || CloseHandle(descendant);
        TEST_ASSERT(stopped && closed_handle);
#else
        test_command_sleep(50); /* POSIX group signals do not reap grandchildren. */
#endif
        file_stat_result first = file_stat((cstring){heartbeat}); TEST_ASSERT_FILE_OK(first.error);
        test_command_sleep(100); file_stat_result second = file_stat((cstring){heartbeat}); TEST_ASSERT_FILE_OK(second.error);
        TEST_ASSERT(first.size > 0 && first.size == second.size);
    }
)
