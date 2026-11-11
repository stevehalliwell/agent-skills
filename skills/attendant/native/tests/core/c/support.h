/* Attendant session/data/fault bindings only. test.h owns fixture-directory
   lifecycle; base.h owns ordinary filesystem/environment operations. */
#include "src/native-api.c"

static attendant_session *source_test_session;
static test_directory source_test_suite_directory, source_test_case_directory;
static char *source_test_case_root = source_test_case_directory.path;
static u8 source_test_file_memory[65536];
static file_tree_workspace source_test_files;
static const char source_test_original[] = "---\r\nname: file\r\n---\r\nHello caf\xc3\xa9 \xf0\x9f\x98\x80\r\nNext line.\r\n";
static const char source_test_saved[] = "---\r\nname: file\r\n---\r\nHello caf\xc3\xa9 \xf0\x9f\x98\x80\r\nNext line.\r\nSaved before comment.\n";
static const char source_test_selected[] = "caf\xc3\xa9 \xf0\x9f\x98\x80\r\nNext";
static const char source_test_comment[] = "Please check **this passage**.\r\n\r\nKeep the rest unchanged.\n";
static const char source_test_external[] = "\n# Concurrent external edit\n";
static const char source_test_context[] = "Context note: caf\xc3\xa9 \xf0\x9f\x98\x80.\r\n\nRendered context lives in note body.\n";
/* Bind shared scratch/error convention at existing Attendant fault call sites. */
static int source_test_environment(const char *name, const char *value) {
    return environment_set(source_test_files.memory, name, value) == environment_error_ok;
}
static int source_test_suite_setup(void) {
    char parent[SOURCE_PATH_CAPACITY], supplied[SOURCE_PATH_CAPACITY], path[NATIVE_PATH_SIZE];
    const char *directory = getenv("ATTENDANT_TEST_DIRECTORY");
    source_test_files = (file_tree_workspace){ arena_from_buffer(source_test_file_memory, sizeof(source_test_file_memory)), 12, 128 };
    if (!directory || !source_full_path("build/native-tests", parent) || !source_full_path(directory, supplied)) return 1;
    size_t prefix = strlen(parent);
    if (strncmp(parent, supplied, prefix) || (supplied[prefix] != '/' && supplied[prefix] != '\\') ||
        source_stat(supplied).kind != SOURCE_DIRECTORY) return 1;
    TEST_ASSERT_FILE_OK(file_tree_join(path, supplied, "fixtures"));
    TEST_ASSERT_FILE_OK(test_directory_create(&source_test_suite_directory, (cstring){path}));
    source_test_session = attendant_open();
    return source_test_session ? 0 : 1;
}
static int source_test_suite_teardown(int failed) {
    if (source_test_session) attendant_close(source_test_session);
    source_test_session = NULL;
    arena_reset(source_test_files.memory);
    TEST_ASSERT_FILE_OK(test_directory_finish(&source_test_suite_directory, &source_test_files, failed));
    return 0;
}
static int source_test_case_setup(const test_case *test) {
    (void)test;
    char path[NATIVE_PATH_SIZE];
    arena_reset(source_test_files.memory);
    TEST_ASSERT_FILE_OK(test_directory_next(&source_test_suite_directory, "case caf\xc3\xa9 \xce\xa9", path));
    TEST_ASSERT_FILE_OK(test_directory_create(&source_test_case_directory, (cstring){path}));
    return source_test_environment("ATTENDANT_FEEDBACK_FAULT", NULL) ? 0 : 1;
}
static int source_test_case_teardown(const test_case *test, int failed) {
    (void)test;
    arena_reset(source_test_files.memory);
    int cleared = source_test_environment("ATTENDANT_FEEDBACK_FAULT", NULL);
    TEST_ASSERT_FILE_OK(test_directory_finish(&source_test_case_directory, &source_test_files, failed || !cleared));
    return cleared ? 0 : 1;
}
#define TEST_SUITE_SETUP source_test_suite_setup
#define TEST_SUITE_TEARDOWN source_test_suite_teardown
#define TEST_SETUP source_test_case_setup
#define TEST_TEARDOWN source_test_case_teardown

typedef struct source_test_baseline { char path[SOURCE_PATH_CAPACITY]; } source_test_baseline;
static int source_test_baseline_create(source_test_baseline *baseline, const char *path) {
    /* This helper returns a predicate, unlike hook/assertion status functions. */
    if (test_assert_file_operation(test_directory_next(&source_test_case_directory, "baseline", baseline->path),
                                   "baseline path", __FILE__, __LINE__)) return 0;
    return !test_assert_file_operation(file_directory_copy(&source_test_files, (cstring){(char *)path}, (cstring){baseline->path}),
                                       path, __FILE__, __LINE__);
}
typedef struct source_test_fixture {
    char root[SOURCE_PATH_CAPACITY], target[SOURCE_PATH_CAPACITY], hash[65];
    attendant_feedback input;
} source_test_fixture;
static int source_test_fixture_open(source_test_fixture *fixture, const char *name, attendant_feedback_mode mode) {
    if (!source_test_session || !source_test_case_directory.owned || !source_test_environment("ATTENDANT_FEEDBACK_FAULT", NULL)) return 0;
    memset(fixture, 0, sizeof(*fixture));
    if (snprintf(fixture->root, sizeof(fixture->root), "%s/%s", source_test_case_root, name) >= (int)sizeof(fixture->root) ||
        !file_directory_create(fixture->root, 1) || snprintf(fixture->target, sizeof(fixture->target), "%s/document.md", fixture->root) >= (int)sizeof(fixture->target)) return 0;
    if (test_assert_file_operation(file_create_text((cstring){fixture->target}, source_test_saved), fixture->target, __FILE__, __LINE__)) return 0;
    attendant_result read = attendant_read_document(source_test_session, fixture->target);
    if (!read.ok) return 0;
    snprintf(fixture->hash, sizeof(fixture->hash), "%s", read.hash);
    fixture->input.project = fixture->root; fixture->input.target_path = fixture->target; fixture->input.target_hash = fixture->hash;
    fixture->input.mode = mode;
    fixture->input.range_start = mode == ATTENDANT_FEEDBACK_SOURCE ? strlen("---\r\nname: file\r\n---\r\nHello ") : 0;
    fixture->input.range_end = mode == ATTENDANT_FEEDBACK_SOURCE ? fixture->input.range_start + 12 : 0;
    fixture->input.selected_text = mode == ATTENDANT_FEEDBACK_SOURCE ? source_test_selected : "";
    fixture->input.comment = mode == ATTENDANT_FEEDBACK_SOURCE ? source_test_comment : source_test_context;
    return 1;
}
static int source_test_recovered(const source_test_fixture *fixture) {
    return attendant_read_document(source_test_session, fixture->target).ok;
}
