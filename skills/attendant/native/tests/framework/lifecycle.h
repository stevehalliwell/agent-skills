/* Isolated runner self-check. Intentional failures never enter main suite. */
#include "test.h"

static const char *framework_scenario;
static char framework_root[NATIVE_PATH_SIZE], framework_file[NATIVE_PATH_SIZE], framework_other[NATIVE_PATH_SIZE];
static char framework_left[NATIVE_PATH_SIZE], framework_right[NATIVE_PATH_SIZE];
static u8 framework_memory[16384];
static file_tree_workspace framework_files;
static int framework_owned;
static int framework_is(const char *name) { return !strcmp(framework_scenario, name); }
static int framework_suite_setup(void) {
    framework_scenario = getenv("TEST_H_SCENARIO");
    if (!framework_scenario) framework_scenario = "success";
    fprintf(stderr, "event:suite-setup\n");
    framework_files = (file_tree_workspace){ arena_from_buffer(framework_memory, sizeof(framework_memory)), 4, 32 };
    if (framework_is("suite-setup")) return 1;
    const char *directory = getenv("ATTENDANT_TEST_DIRECTORY");
    if (!directory) return 1;
    TEST_ASSERT_FILE_OK(file_tree_join(framework_root, directory, "fixture"));
    return 0;
}
static int framework_suite_teardown(int failed) {
    fprintf(stderr, "event:suite-teardown:%d\n", failed != 0);
    return framework_is("suite-teardown");
}
static int framework_setup(const test_case *test) {
    fprintf(stderr, "event:setup:%s\n", test->name);
    TEST_ASSERT(!strcmp(test->source, "tests/framework/lifecycle.h"));
    framework_owned = 0;
    if (strcmp(test->name, "first")) return 0;
    if (!file_directory_create(framework_root, 1)) return 1;
    framework_owned = 1;
    TEST_ASSERT_FILE_OK(file_tree_join(framework_file, framework_root, "actual"));
    TEST_ASSERT_FILE_OK(file_tree_join(framework_other, framework_root, "expected"));
    TEST_ASSERT_FILE_OK(file_create_text((cstring){framework_file}, "actual"));
    TEST_ASSERT_FILE_OK(file_create_text((cstring){framework_other}, "different"));
    TEST_ASSERT_FILE_OK(file_tree_join(framework_left, framework_root, "left"));
    TEST_ASSERT_FILE_OK(file_tree_join(framework_right, framework_root, "right"));
    TEST_ASSERT(file_directory_create(framework_left, 1) && file_directory_create(framework_right, 1));
    char path[NATIVE_PATH_SIZE];
    TEST_ASSERT_FILE_OK(file_tree_join(path, framework_left, "entry")); TEST_ASSERT_FILE_OK(file_create_text((cstring){path}, "one"));
    TEST_ASSERT_FILE_OK(file_tree_join(path, framework_right, "entry")); TEST_ASSERT_FILE_OK(file_create_text((cstring){path}, "two"));
    return framework_is("setup");
}
static int framework_teardown(const test_case *test, int failed) {
    fprintf(stderr, "event:teardown:%s:%d\n", test->name, failed != 0);
    if (strcmp(test->name, "first") || !framework_owned || failed) return 0;
    if (framework_is("teardown")) return 1;
    if (framework_is("cleanup")) framework_files.max_entries = 1;
    TEST_ASSERT_FILE_OK(file_directory_remove_tree(&framework_files, (cstring){framework_root}));
    return 0;
}
#define TEST_SUITE_SETUP framework_suite_setup
#define TEST_SUITE_TEARDOWN framework_suite_teardown
#define TEST_SETUP framework_setup
#define TEST_TEARDOWN framework_teardown

TEST(first,
    fprintf(stderr, "event:body:first\n");
    if (framework_is("assertion")) TEST_ASSERT(0);
    if (framework_is("file-ok")) TEST_ASSERT_FILE_OK(file_create_text((cstring){framework_file}, "no overwrite"));
    if (framework_is("file-text")) TEST_ASSERT_FILE_TEXT(((cstring){framework_file}), "wrong");
    if (framework_is("file-bytes")) TEST_ASSERT_FILE_BYTES(((cstring){framework_file}), "actual\0", 7);
    if (framework_is("files")) TEST_ASSERT_FILES_EQUAL(((cstring){framework_file}), ((cstring){framework_other}));
    if (framework_is("directories")) TEST_ASSERT_DIRECTORIES_EQUAL(&framework_files, ((cstring){framework_left}), ((cstring){framework_right}));
    if (framework_is("missing")) {
        char missing[NATIVE_PATH_SIZE];
        TEST_ASSERT_FILE_OK(file_tree_join(missing, framework_root, "absent"));
        TEST_ASSERT_FILE_TEXT(((cstring){missing}), "absent");
    }
    if (framework_is("capacity")) {
        framework_files.max_entries = 1;
        TEST_ASSERT_DIRECTORIES_EQUAL(&framework_files, ((cstring){framework_root}), ((cstring){framework_root}));
    }
)
TEST(middle,
    fprintf(stderr, "event:body:middle\n");
)
TEST(last,
    fprintf(stderr, "event:body:last\n");
)
