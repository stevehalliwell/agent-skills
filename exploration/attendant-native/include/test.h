#ifndef TEST_H
#define TEST_H

/* Tiny assertions and sequential runner generation. See docs/testing.md. */
#include "base.h"

#define TEST_ASSERT(expr)                                                                          \
    do {                                                                                           \
        if (!(expr)) {                                                                             \
            fprintf(stderr, "%s:%d: assertion failed: %s\n", __FILE__, __LINE__, #expr);           \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)
#define TEST_PASS() return 0
#define TEST_FAIL() return 1

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

/* The generator application owns fixed backing storage. Exhaustion is an error.
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
    static const char *test_generate_paths_[] = {__VA_ARGS__, NULL};                               \
    TEST_GENERATE_MAIN(test_generate_paths_, 1)
#define TEST_GENERATE_FILES(...)                                                                   \
    static const char *test_generate_paths_[] = {__VA_ARGS__, NULL};                               \
    TEST_GENERATE_MAIN(test_generate_paths_, 0)

typedef generator_declaration test_declaration;

static int test_generate(FILE *out, arena *storage, const char *const *paths, int directories) {
    test_declaration *tests;
    size_t test_count, i;
    if (generator_collect(storage, paths, directories, "TEST(", &tests, &test_count))
        return 1;
    fputs("#include \"base.h\"\n", out);
    fputs(TEST_GENERATED_PREAMBLE, out);
    fputs("#define TEST_ENABLE 1\n", out);
    for (i = 0; i < test_count; i++) {
        if (i && tests[i].source == tests[i - 1].source)
            continue;
        if (!generator_write_include(out, TEST_GENERATED_INCLUDE_PREFIX, tests[i].source))
            return 1;
    }
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
          "    return fwrite(row.value.data, 1, row.value.len, out) == row.value.len && "
          "fflush(out) == 0;\n"
          "}\n"
          "\nint main(int argc, char **argv) {\n"
          "    int tests_run = 0, tests_failed = 0;\n"
          "    int tsv = argv_has_flag(argc, argv, \"-tsv\");\n"
          "    int verbose = argv_has_flag(argc, argv, \"-verbose\");\n",
          out);
    fprintf(out,
            "    if (tsv) {\n        if (fprintf(stdout, "
            "\"kind\\tstatus\\tsource\\tname\\nsuite\\t%zu\\t\\t\\n\") < 0 || fflush(stdout) != 0) "
            "return 2;\n    } else {\n        printf(\"running %zu tests\\n\");\n    }\n",
            test_count,
            test_count);
    for (i = 0; i < test_count; i++) {
        fprintf(out,
                "    {\n        int status = test_%s();\n        tests_run++;\n        "
                "tests_failed += status != 0;\n        if (tsv && !test_write_tsv_result(stdout, ",
                tests[i].name);
        generator_write_literal(out, "", tests[i].source);
        fputs(", ", out);
        generator_write_literal(out, "", tests[i].name);
        fputs(", status == 0 ? \"ok\" : \"fail\")) return 2;\n        if (status != 0 || (!tsv && "
              "verbose)) {\n            fprintf(status != 0 ? stderr : stdout, \"%s: %s %s\\n\", ",
              out);
        generator_write_literal(out, "", tests[i].source);
        fputs(", status != 0 ? \"FAIL\" : \"ok\", ", out);
        generator_write_literal(out, "", tests[i].name);
        fputs(");\n        }\n    }\n", out);
    }
    fputs("    if (tsv) {\n"
          "        if (fprintf(stdout, \"complete\\t%d\\t%d\\t%d\\n\", tests_failed, tests_run, "
          "tests_run - tests_failed) < 0 || fflush(stdout) != 0) return 2;\n"
          "    } else {\n"
          "        printf(\"%d passed, %d failed\\n\", tests_run - tests_failed, tests_failed);\n"
          "    }\n"
          "    return tests_failed != 0;\n"
          "}\n",
          out);
    if (ferror(out) || fflush(out) != 0) {
        fprintf(stderr, "failed to write generated runner\n");
        return 1;
    }
    return test_count == 0 ? TEST_GENERATE_EMPTY : 0;
}
#endif
#endif
