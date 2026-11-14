/* Compile/run with only C99 + include path: no Attendant flags or libraries. */
#include "base.h"
/* Deterministic scheduling seam: time crosses deadline inside one poll. */
static int standalone_clock_armed, standalone_clock_calls;
#if defined(_WIN32)
static BOOL WINAPI standalone_counter(LARGE_INTEGER *counter) {
    BOOL ok = QueryPerformanceCounter(counter);
    if (ok && standalone_clock_armed && ++standalone_clock_calls >= 2) {
        LARGE_INTEGER frequency;
        if (!QueryPerformanceFrequency(&frequency)) return FALSE;
        counter->QuadPart += frequency.QuadPart * 60;
    }
    return ok;
}
#define QueryPerformanceCounter standalone_counter
#else
static int standalone_clock(clockid_t clock, struct timespec *now) {
    int error = clock_gettime(clock, now);
    if (!error && standalone_clock_armed && ++standalone_clock_calls >= 2) now->tv_sec += 60;
    return error;
}
#define clock_gettime standalone_clock
#endif
#if defined(__APPLE__)
#include <signal.h>
#include <sys/sysctl.h>
#include <sys/proc.h>
static int standalone_group_mode;
static int standalone_kill(pid_t process, int signal_number) {
    if (!standalone_group_mode) return kill(process, signal_number);
    errno = standalone_group_mode == 5 ? ESRCH : EPERM;
    return -1;
}
static int standalone_sysctl(int *query, u_int count, void *output, size_t *size, void *input, size_t input_size) {
    if (!standalone_group_mode) return sysctl(query, count, output, size, input, input_size);
    if (standalone_group_mode == 3 || standalone_group_mode == 4) {
        errno = standalone_group_mode == 3 ? EACCES : ENOMEM;
        return -1;
    }
    struct kinfo_proc *member = output;
    memset(member, 0, sizeof(*member));
    member->kp_proc.p_stat = standalone_group_mode == 2 ? SZOMB : SRUN;
    if (standalone_group_mode == 6) member->kp_proc.p_flag = P_WEXIT;
    *size = sizeof(*member);
    return 0;
}
#define kill standalone_kill
#define sysctl standalone_sysctl
#endif
#include "test.h"
#if defined(__APPLE__)
#undef kill
#undef sysctl
#endif
int main(void) {
#if defined(__APPLE__)
    /* Darwin exiting-only EPERM is benign; live/unknown members must fail. */
    for (int mode = 1; mode <= 6; ++mode) {
        standalone_group_mode = mode;
        errno = 0;
        int error = test_command_posix_kill_group(123);
        TEST_ASSERT(mode == 2 || mode == 5 || mode == 6 ? error == 0 : error == -1 && errno == EPERM);
    }
    standalone_group_mode = 0;
#endif
    static u8 memory[262144]; char output[128], errors[128], helper[NATIVE_PATH_SIZE];
    test_command_workspace workspace = {arena_from_buffer(memory, sizeof(memory)), output, errors, sizeof(output), sizeof(errors)};
    usize size;
    TEST_ASSERT(environment_get(workspace.scratch, "TEST_COMMAND_HELPER", helper, sizeof(helper), &size) == environment_error_ok && size);
    const char *arguments[] = {helper, "echo"};
    const u8 input[] = {'a', 0, 'b'};
    test_command command = {.arguments = arguments, .argument_count = 2, .input = input, .input_size = sizeof(input), .timeout_ms = 30000};
    test_command_result result;
    TEST_ASSERT_COMMAND_OK(test_command_run(&workspace, &command, &result));
    TEST_ASSERT_COMMAND_EXIT(&result, 0);
    TEST_ASSERT_COMMAND_OUTPUT_BYTES(&result, input, sizeof(input));
    TEST_ASSERT_COMMAND_ERROR_TEXT(&result, "error café Ω\n");
    const char *sleep[] = {helper, "sleep"};
    command.arguments = sleep; command.input = NULL; command.input_size = 0;
    test_command_live live = {0};
    test_command_error started = test_command_start(&live, &workspace, &command);
    standalone_clock_calls = 0; standalone_clock_armed = 1;
    test_command_error polled = test_command_poll(&live);
    int complete = live.complete;
    standalone_clock_armed = 0;
    test_command_error finished = test_command_finish(&live);
    TEST_ASSERT_COMMAND_OK(started);
    TEST_ASSERT(polled == test_command_timeout && complete && finished == test_command_timeout && !live.result.cleanup_failed);
    return 0;
}
