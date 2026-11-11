/* Compile/run with only C99 + include path: no Attendant flags or libraries. */
#include "test.h"
int main(void) {
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
    return 0;
}
