#ifndef TEST_H
#define TEST_H

/* Tiny assertions, optional lifecycle hooks and sequential runner generation.
   Local extension of proto-framework test.h; see native/vendor/README.md. */
#include "base.h"

#define TEST_ASSERT(expr)                                                                          \
    do {                                                                                           \
        if (!(expr)) {                                                                             \
            fprintf(stderr, "%s:%d: assertion failed: %s\n", __FILE__, __LINE__, #expr);             \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)
#define TEST_PASS() return 0
#define TEST_FAIL() return 1

/* Filesystem assertions always run, evaluate arguments once, and return from
   current test/hook on failure. Mutations remain explicit base.h calls. */
static int test_assert_file_operation(file_error error, const char *operation, const char *source, int line) {
    if (error == file_error_ok) return 0;
    fprintf(stderr, "%s:%d: %s: %s\n", source, line, operation, file_error_message(error));
    return 1;
}
static int test_assert_file_comparison(file_error error, int equal, const char *left, const char *right,
                                       const char *source, int line) {
    if (error == file_error_ok && equal) return 0;
    fprintf(stderr, "%s:%d: comparison %s vs %s: %s\n", source, line,
            left ? left : "(null)", right ? right : "(null)",
            error == file_error_ok ? "contents or layout differ" : file_error_message(error));
    return 1;
}
static int test_assert_file_text(cstring path, const char *expected, const char *source, int line) {
    int equal = 0;
    file_error error = file_equal_text(path, expected, &equal);
    return test_assert_file_comparison(error, equal, path.data, "expected text", source, line);
}
static int test_assert_file_bytes(cstring path, const void *expected, usize size, const char *source, int line) {
    int equal = 0;
    file_error error = file_equal_bytes(path, expected, size, &equal);
    return test_assert_file_comparison(error, equal, path.data, "expected bytes", source, line);
}
static int test_assert_files_equal(cstring left, cstring right, const char *source, int line) {
    int equal = 0;
    file_error error = file_equal(left, right, &equal);
    return test_assert_file_comparison(error, equal, left.data, right.data, source, line);
}
static int test_assert_directories_equal(file_tree_workspace *workspace, cstring left, cstring right,
                                        const char *source, int line) {
    int equal = 0;
    file_error error = file_directory_equal(workspace, left, right, &equal);
    return test_assert_file_comparison(error, equal, left.data, right.data, source, line);
}
#define TEST_ASSERT_FILE_OK(operation)                                                              \
    do { if (test_assert_file_operation((operation), #operation, __FILE__, __LINE__)) return 1; } while (0)
#define TEST_ASSERT_FILE_TEXT(path, expected)                                                       \
    do { if (test_assert_file_text((path), (expected), __FILE__, __LINE__)) return 1; } while (0)
#define TEST_ASSERT_FILE_BYTES(path, expected, size)                                                \
    do { if (test_assert_file_bytes((path), (expected), (size), __FILE__, __LINE__)) return 1; } while (0)
#define TEST_ASSERT_FILES_EQUAL(left, right)                                                        \
    do { if (test_assert_files_equal((left), (right), __FILE__, __LINE__)) return 1; } while (0)
#define TEST_ASSERT_DIRECTORIES_EQUAL(workspace, left, right)                                       \
    do { if (test_assert_directories_equal((workspace), (left), (right), __FILE__, __LINE__)) return 1; } while (0)

typedef struct test_case {
    const char *source, *name; /* Borrowed runner literals; valid through teardown. */
} test_case;

/* Hooks return zero on success. Failed setup skips body, never teardown.
   Failed teardown fails case. No recovery from crash/abort/forced termination.
   Fixture data stays typed and owned by caller; no hidden mutable context. */
static int test_run_case(const test_case *test, int (*body)(void),
                          int (*setup)(const test_case *), int (*teardown)(const test_case *, int)) {
    int status = setup ? setup(test) : 0;
    if (status) fprintf(stderr, "%s: %s: setup failed (%d); body skipped\n", test->source, test->name, status);
    else status = body();
    if (teardown) {
        int cleanup = teardown(test, status != 0);
        if (cleanup) fprintf(stderr, "%s: %s: teardown failed (%d)\n", test->source, test->name, cleanup);
        if (cleanup) status = 1;
    }
    return status != 0;
}

#if defined(TEST_ENABLE) && TEST_ENABLE
#define TEST(name, ...)                                                                            \
    static int test_##name(void) {                                                                 \
        __VA_ARGS__                                                                                \
        return 0;                                                                                  \
    }
#else
#define TEST(name, ...)
#endif

#ifdef TEST_GENERATOR
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#ifndef TEST_GENERATED_PREAMBLE
#define TEST_GENERATED_PREAMBLE ""
#endif
#ifndef TEST_GENERATED_INCLUDE_PREFIX
#define TEST_GENERATED_INCLUDE_PREFIX ""
#endif

/* Generator application owns fixed backing storage. Exhaustion is an error.
   Empty discovery emits a valid zero-test runner and returns TEST_GENERATE_EMPTY. */
enum {
    TEST_GENERATE_EMPTY = 3,
    TEST_GENERATE_MAX_FILES = GENERATOR_MAX_FILES,
    TEST_GENERATE_MAX_TESTS = GENERATOR_MAX_DECLARATIONS,
    TEST_GENERATE_LINE_SIZE = GENERATOR_LINE_SIZE,
    TEST_GENERATE_NAME_SIZE = GENERATOR_NAME_SIZE,
    TEST_GENERATE_BACKING_SIZE = GENERATOR_BACKING_SIZE
};

#define TEST_GENERATE_MAIN(paths, directories)                                                     \
    int main(void) {                                                                               \
        static u8 backing[TEST_GENERATE_BACKING_SIZE];                                             \
        return test_generate(stdout,                                                               \
                             arena_from_buffer(backing, sizeof(backing)),                          \
                             paths,                                                                \
                             directories);                                                         \
    }
#define TEST_GENERATE_DIR(root_dir) TEST_GENERATE_DIRS(root_dir)
#define TEST_GENERATE_DIRS(...)                                                                    \
    static const char *test_generate_paths_[] = {__VA_ARGS__, NULL};                                 \
    TEST_GENERATE_MAIN(test_generate_paths_, 1)
#define TEST_GENERATE_FILES(...)                                                                   \
    static const char *test_generate_paths_[] = {__VA_ARGS__, NULL};                                 \
    TEST_GENERATE_MAIN(test_generate_paths_, 0)

typedef generator_declaration test_declaration;

static int test_generate(FILE *out, arena *storage, const char *const *paths, int directories) {
    test_declaration *tests;
    size_t test_count, i;
    if (generator_collect(storage, paths, directories, "TEST(", &tests, &test_count)) return 1;
    /* Make assertion/types available to preamble hooks, with TEST already active. */
    fputs("#define TEST_ENABLE 1\n#include \"test.h\"\n", out);
    fputs(TEST_GENERATED_PREAMBLE, out);
    for (i = 0; i < test_count; i++) {
        if (i && tests[i].source == tests[i - 1].source) continue;
        if (!generator_write_include(out, TEST_GENERATED_INCLUDE_PREFIX, tests[i].source)) return 1;
    }
    /* Defaults follow includes so a suite preamble or test file can bind hooks. */
    fputs("\n#ifndef TEST_SUITE_SETUP\n#define TEST_SUITE_SETUP() 0\n#endif\n"
          "#ifndef TEST_SUITE_TEARDOWN\n#define TEST_SUITE_TEARDOWN(failed) 0\n#endif\n"
          "#ifndef TEST_SETUP\n#define TEST_SETUP NULL\n#endif\n"
          "#ifndef TEST_TEARDOWN\n#define TEST_TEARDOWN NULL\n#endif\n", out);
    fputs("\nstatic int test_write_tsv_result(FILE *out, const char *source, const char *name, "
          "const char *status) {\n"
          "    u8 buffer[8192];\n"
          "    arena *backing = arena_from_buffer(buffer, sizeof(buffer));\n"
          "    string_builder builder = { .arena = backing };\n"
          "    string_result row;\n"
          "    if (tsv_write_cstr_field(&builder, \"test\") != arena_error_ok ||\n"
          "        tsv_write_separator(&builder) != arena_error_ok ||\n"
          "        tsv_write_cstr_field(&builder, status) != arena_error_ok ||\n"
          "        tsv_write_separator(&builder) != arena_error_ok ||\n"
          "        tsv_write_cstr_field(&builder, source) != arena_error_ok ||\n"
          "        tsv_write_separator(&builder) != arena_error_ok ||\n"
          "        tsv_write_cstr_field(&builder, name) != arena_error_ok ||\n"
          "        tsv_write_record_end(&builder) != arena_error_ok) {\n"
          "        fprintf(stderr, \"test TSV row storage exhausted (8192 bytes)\\n\");\n"
          "        return 0;\n"
          "    }\n"
          "    row = string_from_string_builder(&builder);\n"
          "    if (row.error != arena_error_ok) {\n"
          "        fprintf(stderr, \"test TSV row storage exhausted (8192 bytes)\\n\");\n"
          "        return 0;\n"
          "    }\n"
          "    return fwrite(row.value.data, 1, row.value.len, out) == row.value.len && fflush(out) == 0;\n"
          "}\n"
          "\nint main(int argc, char **argv) {\n"
          "    int tests_run = 0, tests_failed = 0, infrastructure_failed = 0;\n"
          "    int tsv = argv_has_flag(argc, argv, \"-tsv\");\n"
          "    int verbose = argv_has_flag(argc, argv, \"-verbose\");\n", out);
    fprintf(out,
            "    if (tsv) {\n        if (fprintf(stdout, "
            "\"kind\\tstatus\\tsource\\tname\\nsuite\\t%zu\\t\\t\\n\") < 0 || fflush(stdout) != 0) "
            "return 2;\n    } else {\n        if (printf(\"running %zu tests\\n\") < 0 || fflush(stdout) != 0) return 2;\n    }\n", test_count, test_count);
    fputs("    if (TEST_SUITE_SETUP() != 0) {\n"
          "        fprintf(stderr, \"suite setup failed; bodies skipped\\n\");\n"
          "        infrastructure_failed = 1;\n"
          "        goto test_suite_cleanup;\n"
          "    }\n", out);
    for (i = 0; i < test_count; i++) {
        fputs("    {\n        const test_case info = { ", out);
        generator_write_literal(out, "", tests[i].source);
        fputs(", ", out);
        generator_write_literal(out, "", tests[i].name);
        fprintf(out, " };\n        int status = test_run_case(&info, test_%s, TEST_SETUP, TEST_TEARDOWN);\n"
                "        tests_run++;\n        tests_failed += status != 0;\n"
                "        if (tsv && !test_write_tsv_result(stdout, info.source, info.name, status == 0 ? \"ok\" : \"fail\")) {\n"
                "            infrastructure_failed = 1; goto test_suite_cleanup;\n        }\n"
                "        if (status != 0 || (!tsv && verbose)) {\n"
                "            fprintf(status != 0 ? stderr : stdout, \"%%s: %%s %%s\\n\", info.source, status != 0 ? \"FAIL\" : \"ok\", info.name);\n"
                "        }\n    }\n", tests[i].name);
    }
    fputs("test_suite_cleanup:\n"
          "    if (TEST_SUITE_TEARDOWN(infrastructure_failed || tests_failed != 0) != 0) {\n"
          "        fprintf(stderr, \"suite teardown failed\\n\");\n"
          "        infrastructure_failed = 1;\n"
          "    }\n"
          "    if (infrastructure_failed) return 2;\n"
          "    if (tsv) {\n"
          "        if (fprintf(stdout, \"complete\\t%d\\t%d\\t%d\\n\", tests_failed, tests_run, "
          "tests_run - tests_failed) < 0 || fflush(stdout) != 0) return 2;\n"
          "    } else {\n"
          "        if (printf(\"%d passed, %d failed\\n\", tests_run - tests_failed, tests_failed) < 0 || fflush(stdout) != 0) return 2;\n"
          "    }\n"
          "    if (ferror(stdout)) return 2;\n"
          "    return tests_failed != 0;\n"
          "}\n", out);
    if (ferror(out) || fflush(out) != 0) {
        fprintf(stderr, "failed to write generated runner\n");
        return 1;
    }
    return test_count == 0 ? TEST_GENERATE_EMPTY : 0;
}
#endif
#endif
