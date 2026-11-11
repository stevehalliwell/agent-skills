/* Separate generated suites exercise failures and no-hook/empty defaults. */
#define TEST_GENERATOR
#include "test.h"
int main(void) {
    static u8 memory[TEST_GENERATE_BACKING_SIZE];
    const char *mode = getenv("TEST_H_MODE");
    const char *paths[] = {"tests/framework/lifecycle.h", NULL};
    if (mode && !strcmp(mode, "plain")) paths[0] = "tests/framework/plain.h";
    if (mode && !strcmp(mode, "empty")) paths[0] = "tests/framework/empty.h";
    return test_generate(stdout, arena_from_buffer(memory, sizeof(memory)), paths, 0);
}
