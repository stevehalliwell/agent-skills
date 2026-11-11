#include "test.h"

static char live_test_output[262145], live_test_errors[1024];
/* No assertion may return while local children are active. */
TEST(command_live_readiness_two_children_scratch_reuse_and_completion,
    test_command_workspace workspace; char helper[NATIVE_PATH_SIZE], gate[NATIVE_PATH_SIZE];
    TEST_ASSERT(command_test_prepare(&workspace, helper));
    TEST_ASSERT_FILE_OK(file_tree_join(gate, source_test_case_root, "release café"));
    test_command_workspace other = {workspace.scratch, live_test_output, live_test_errors, sizeof(live_test_output), sizeof(live_test_errors)};
    const char *arguments[] = {helper, "live", gate};
    test_command command = {.arguments = arguments, .argument_count = 3, .timeout_ms = 30000};
    test_command_live viewer = {0}, inspector = {0};
    usize mark = arena_pos(workspace.scratch);
    int ok = test_command_start(&viewer, &workspace, &command) == test_command_ok;
    ok = ok && arena_pos(workspace.scratch) == mark;
    int duplicate = test_command_start(&viewer, &workspace, &command) == test_command_invalid && viewer.active;
    arena_result reuse = arena_alloc(workspace.scratch, 131072, 1);
    if (reuse.error == arena_error_ok) memset(reuse.ptr, 0x7f, 131072); else ok = 0;
    while (ok && viewer.active && !strstr(workspace.output, "ready\n")) {
        ok = test_command_poll(&viewer) == test_command_ok;
        test_command_sleep(1);
    }
    int ready = ok && viewer.active && strstr(workspace.output, "ready\n") != NULL;
    u8 input[200000]; for (usize i = 0; i < sizeof(input); ++i) input[i] = (u8)(i % 251);
    const char *inspect[] = {helper, "echo"};
    test_command second = {.arguments = inspect, .argument_count = 2, .input = input, .input_size = sizeof(input), .timeout_ms = 30000};
    if (ready) ok = test_command_start(&inspector, &other, &second) == test_command_ok; else ok = 0;
    while (ok && inspector.active && viewer.active) {
        ok = test_command_poll(&viewer) == test_command_ok;
        if (test_command_poll(&inspector) != test_command_ok) ok = 0;
        test_command_sleep(1);
    }
    int concurrent = ok && viewer.active && inspector.complete && inspector.result.exited && inspector.result.exit_code == 0 &&
        inspector.result.output.len == sizeof(input) && !memcmp(inspector.result.output.data, input, sizeof(input));
    if (concurrent) ok = file_create_text((cstring){gate}, "release") == file_error_ok; else ok = 0;
    while (ok && viewer.active) { ok = test_command_poll(&viewer) == test_command_ok; test_command_sleep(1); }
    test_command_error first_finish = test_command_finish(&viewer), second_finish = test_command_finish(&inspector);
    TEST_ASSERT(ok && duplicate && ready && concurrent);
    TEST_ASSERT_COMMAND_OK(first_finish); TEST_ASSERT_COMMAND_OK(second_finish);
    TEST_ASSERT_COMMAND_EXIT(&viewer.result, 0); TEST_ASSERT_COMMAND_OUTPUT_TEXT(&viewer.result, "ready\ndone\n");
    TEST_ASSERT_COMMAND_ERROR_TEXT(&viewer.result, "live-errors\n");
    TEST_ASSERT_COMMAND_ERROR_TEXT(&inspector.result, "error café Ω\n");
    TEST_ASSERT_COMMAND_OK(test_command_poll(&viewer)); TEST_ASSERT_COMMAND_OK(test_command_finish(&viewer));
)

TEST(command_live_cancellation_deadline_validation_and_reuse,
    test_command_workspace workspace; char helper[NATIVE_PATH_SIZE];
    TEST_ASSERT(command_test_prepare(&workspace, helper));
    test_command_live live = {0};
    TEST_ASSERT_COMMAND_OK(test_command_finish(&live));
    TEST_ASSERT(test_command_poll(&live) == test_command_invalid);
    TEST_ASSERT(test_command_start(&live, NULL, NULL) == test_command_invalid && live.complete);
    TEST_ASSERT(test_command_finish(&live) == test_command_invalid);
    char missing[NATIVE_PATH_SIZE]; TEST_ASSERT_FILE_OK(file_tree_join(missing, source_test_case_root, "absent child"));
    const char *arguments[] = {missing, "sleep"};
    test_command command = {.arguments = arguments, .argument_count = 2, .timeout_ms = 30000};
    TEST_ASSERT(test_command_start(&live, &workspace, &command) == test_command_launch_failed && live.complete && !live.active && !live.result.cleanup_failed);
    TEST_ASSERT(test_command_finish(&live) == test_command_launch_failed && test_command_finish(&live) == test_command_launch_failed);
    arguments[0] = helper;
    test_command_error started = test_command_start(&live, &workspace, &command);
    test_command_error finished = test_command_finish(&live);
    TEST_ASSERT_COMMAND_OK(started); TEST_ASSERT(finished == test_command_cancelled && live.complete && !live.active && !live.result.cleanup_failed);
    TEST_ASSERT(test_command_finish(&live) == test_command_cancelled);
    command.timeout_ms = 50;
    started = test_command_start(&live, &workspace, &command);
    test_command_sleep(75); /* No background watchdog; next poll enforces original deadline. */
    test_command_error polled = test_command_poll(&live);
    finished = test_command_finish(&live);
    TEST_ASSERT_COMMAND_OK(started); TEST_ASSERT(polled == test_command_timeout && finished == test_command_timeout && live.complete && !live.result.cleanup_failed);
    const char *exit[] = {helper, "exit", "7"}; command.arguments = exit; command.argument_count = 3; command.timeout_ms = 30000;
    started = test_command_start(&live, &workspace, &command);
    while (live.active) { (void)test_command_poll(&live); test_command_sleep(1); }
    finished = test_command_finish(&live);
    TEST_ASSERT_COMMAND_OK(started); TEST_ASSERT_COMMAND_OK(finished); TEST_ASSERT_COMMAND_EXIT(&live.result, 7);
)

TEST(command_live_overflow_automatically_releases_owned_child,
    test_command_workspace workspace; char helper[NATIVE_PATH_SIZE];
    TEST_ASSERT(command_test_prepare(&workspace, helper));
    const char *arguments[] = {helper, "flood"};
    test_command command = {.arguments = arguments, .argument_count = 2, .timeout_ms = 30000};
    for (int stream = 0; stream < 2; ++stream) {
        workspace.output_capacity = stream ? sizeof(command_test_output) : 8;
        workspace.error_capacity = stream ? 8 : sizeof(command_test_errors);
        test_command_live live = {0};
        test_command_error started = test_command_start(&live, &workspace, &command);
        while (live.active) { (void)test_command_poll(&live); test_command_sleep(1); }
        test_command_error finished = test_command_finish(&live);
        TEST_ASSERT_COMMAND_OK(started); TEST_ASSERT(finished == test_command_capacity && live.complete && !live.result.cleanup_failed);
        string_view capture = stream ? live.result.error_output : live.result.output;
        TEST_ASSERT(capture.len == 7 && capture.data[capture.len] == 0);
    }
)

TEST(command_live_exit_cancel_and_timeout_stop_owned_descendants,
    test_command_workspace workspace; char helper[NATIVE_PATH_SIZE], heartbeat[NATIVE_PATH_SIZE];
    TEST_ASSERT(command_test_prepare(&workspace, helper));
    for (int mode = 0; mode < 6; ++mode) {
        const char *names[] = {"cancel-open", "cancel-closed", "timeout-open", "timeout-closed", "exit-open", "exit-closed"};
        TEST_ASSERT_FILE_OK(file_tree_join(heartbeat, source_test_case_root, names[mode]));
        const char *arguments[] = {helper, mode % 2 ? "spawn-heartbeat-closed" : "spawn-heartbeat", heartbeat, "hold"};
        test_command command = {.arguments = arguments, .argument_count = 4, .timeout_ms = mode == 2 || mode == 3 ? 1000 : 30000};
        test_command_live live = {0};
        u64 operation_start = 0, operation_end = 0; int timed = test_command_clock(&operation_start);
        test_command_error started = test_command_start(&live, &workspace, &command);
        while (live.active && !strstr(workspace.output, "spawned\n")) { (void)test_command_poll(&live); test_command_sleep(1); }
        int ready = live.active && strstr(workspace.output, "spawned\n") != NULL;
#if defined(_WIN32)
        char identity[128] = {0}; usize size = 0;
        int identified = ready && file_read_into((cstring){heartbeat}, identity, sizeof(identity) - 1, &size) == file_error_ok;
        identity[size] = 0; char *end; unsigned long process = strtoul(identity, &end, 10);
        HANDLE descendant = identified && process && *end == '\n' ? OpenProcess(SYNCHRONIZE | PROCESS_QUERY_INFORMATION, FALSE, (DWORD)process) : NULL;
        DWORD open_error = descendant ? 0 : GetLastError();
        BOOL member = FALSE;
        int membership = descendant && IsProcessInJob(descendant, live.job, &member) && member;
#endif
        int released = 1;
        if (mode >= 4 && ready) {
            char release[NATIVE_PATH_SIZE]; int count = snprintf(release, sizeof(release), "%s.release", heartbeat);
            released = count > 0 && count < (int)sizeof(release) && file_create_text((cstring){release}, "release") == file_error_ok;
        }
        if (mode >= 2 && released) while (live.active) { (void)test_command_poll(&live); test_command_sleep(1); }
        test_command_error finished = test_command_finish(&live);
#if defined(_WIN32)
        /* User-approved job-drain contract: separately wait known descendant. */
        DWORD waited = descendant ? WaitForSingleObject(descendant, TEST_COMMAND_CLEANUP_MS) : WAIT_FAILED;
        DWORD wait_error = waited == WAIT_FAILED ? GetLastError() : 0;
        int stopped = descendant && waited == WAIT_OBJECT_0;
        int closed = !descendant || CloseHandle(descendant);
        if (!stopped || !closed) fprintf(stderr, "live descendant mode %d: ready=%d identified=%d PID=%lu open=%lu wait=%lu error=%lu finish=%s cleanup=%d\n", mode, ready, identified, process, (unsigned long)open_error, (unsigned long)waited, (unsigned long)wait_error, test_command_error_message(finished), live.result.cleanup_failed);
        TEST_ASSERT(membership && stopped && closed);
#else
        test_command_sleep(50);
#endif
        timed = test_command_clock(&operation_end) && timed;
        TEST_ASSERT_COMMAND_OK(started); TEST_ASSERT(ready && released && !live.result.cleanup_failed && live.complete);
        TEST_ASSERT(timed && operation_end - operation_start < 5000);
        TEST_ASSERT(live.result.output.len == strlen("spawned\n") && !memcmp(live.result.output.data, "spawned\n", strlen("spawned\n")));
        TEST_ASSERT(finished == (mode < 2 ? test_command_cancelled : mode < 4 ? test_command_timeout : test_command_ok));
        if (mode >= 4) TEST_ASSERT_COMMAND_EXIT(&live.result, 0);
        file_stat_result first = file_stat((cstring){heartbeat}); TEST_ASSERT_FILE_OK(first.error);
        test_command_sleep(100); file_stat_result second = file_stat((cstring){heartbeat}); TEST_ASSERT_FILE_OK(second.error);
        TEST_ASSERT(first.size > 0 && first.size == second.size);
    }
)
