/* Focused parser/P6 rejection checks. Run in fresh owned build directory;
   never starts GUI, opens project or changes reusable header policy. */
#include "test.h"
#include "tests/gui/synchronous/support.h"
int main(void) {
    const struct { const char *json; int accepted; } counts[] = {
        {"-1", 0}, {"0", 0}, {"0.5", 0}, {"1e99", 0}, {"2147483648", 0},
        {"18446744073709551615", 0}, {"\"2\"", 0}, {"true", 0}, {"null", 0},
        {"1", 1}, {"2147483647", 1}
    };
    for (usize i = 0; i < sizeof(counts) / sizeof(counts[0]); ++i) {
        char json[128]; int count = -9;
        int size = snprintf(json, sizeof(json), "{\"count\":%s}", counts[i].json);
        TEST_ASSERT(size > 0 && size < (int)sizeof(json));
        TEST_ASSERT(integration_json_parse(0, json, (usize)size) == 0);
        TEST_ASSERT(gui_positive_count("count", &count) == counts[i].accepted);
        TEST_ASSERT(counts[i].accepted ? count > 0 : count == -9);
    }
    TEST_ASSERT(integration_json_parse(0, "{}", 2) == 0 && !gui_positive_count("count", NULL));
    const char *bad[] = {"P3\n1 1\n255\nabc", "P6\n", "P6\n1 1\n255\nab", "P6\n1 1\n255\nabcd", "P6\n16385 1\n255\nabc"};
    TEST_ASSERT_FILE_OK(file_create_text((cstring){"image.ppm"}, ""));
    for (usize i = 0; i < sizeof(bad) / sizeof(bad[0]); ++i) {
        TEST_ASSERT_FILE_OK(file_write_bytes((cstring){"image.ppm"}, bad[i], strlen(bad[i])));
        TEST_ASSERT(gui_screenshot("image.ppm") == 1);
    }
    const char valid[] = "P6\n1 1\n255\nabc";
    TEST_ASSERT_FILE_OK(file_write_bytes((cstring){"image.ppm"}, valid, sizeof(valid) - 1));
    TEST_ASSERT(gui_screenshot("image.ppm") == 0);
    puts("GUI validators: 12 count cases and 6 P6 cases passed");
    return 0;
}
