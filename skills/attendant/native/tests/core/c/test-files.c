#include "test.h"

TEST(file_create_compare_unicode_binary_and_exclusivity,
    char path[NATIVE_PATH_SIZE], other[NATIVE_PATH_SIZE];
    TEST_ASSERT_FILE_OK(file_tree_join(path, source_test_case_root, "caf\xc3\xa9 \xf0\x9f\x98\x80.bin"));
    TEST_ASSERT_FILE_OK(file_tree_join(other, source_test_case_root, "empty"));
    const u8 bytes[] = {'a', 0, 'b', '\r', '\n', 0xff};
    TEST_ASSERT_FILE_OK(file_create_bytes((cstring){path}, bytes, sizeof(bytes)));
    TEST_ASSERT_FILE_BYTES(((cstring){path}), bytes, sizeof(bytes));
    file_stat_result info = file_stat((cstring){path});
    TEST_ASSERT(info.error == file_error_ok && info.size == sizeof(bytes) && info.modified_time > 0);
    int equal = 9;
    TEST_ASSERT_FILE_OK(file_equal_text((cstring){path}, "a", &equal)); TEST_ASSERT(!equal);
    TEST_ASSERT(file_create_text((cstring){path}, "must not truncate") == file_error_exists);
    TEST_ASSERT_FILE_BYTES(((cstring){path}), bytes, sizeof(bytes));
    TEST_ASSERT_FILE_OK(file_create_text((cstring){other}, ""));
    TEST_ASSERT_FILE_BYTES(((cstring){other}), NULL, 0);
    TEST_ASSERT_FILE_TEXT(((cstring){other}), "");
    TEST_ASSERT_FILE_OK(file_equal((cstring){path}, (cstring){other}, &equal)); TEST_ASSERT(!equal);
    TEST_ASSERT(file_create_text((cstring){other}, NULL) == file_error_invalid);
    TEST_ASSERT(file_create_bytes((cstring){other}, NULL, 1) == file_error_invalid);
    TEST_ASSERT(file_equal_bytes((cstring){path}, NULL, 1, &equal) == file_error_invalid);
    TEST_ASSERT(file_equal((cstring){path}, (cstring){other}, NULL) == file_error_invalid);
    TEST_ASSERT_FILE_OK(file_remove((cstring){other}));
    equal = 9;
    TEST_ASSERT(file_equal((cstring){path}, (cstring){other}, &equal) == file_error_not_found && equal == 9);
)

TEST(file_comparison_streams_and_utf8_existing_helpers,
    char first[NATIVE_PATH_SIZE], second[NATIVE_PATH_SIZE];
    TEST_ASSERT_FILE_OK(file_tree_join(first, source_test_case_root, "one \xce\xa9"));
    TEST_ASSERT_FILE_OK(file_tree_join(second, source_test_case_root, "two \xce\xa9"));
    u8 bytes[20000], output[20000];
    for (usize i = 0; i < sizeof(bytes); ++i) bytes[i] = (u8)(i * 37);
    TEST_ASSERT_FILE_OK(file_create_bytes((cstring){first}, bytes, sizeof(bytes)));
    TEST_ASSERT_FILE_OK(file_create_bytes((cstring){second}, bytes, sizeof(bytes)));
    TEST_ASSERT_FILES_EQUAL(((cstring){first}), ((cstring){second}));
    TEST_ASSERT_FILES_EQUAL(((cstring){first}), ((cstring){first}));
    usize read = 0;
    TEST_ASSERT_FILE_OK(file_read_into((cstring){first}, output, sizeof(output), &read));
    TEST_ASSERT(read == sizeof(bytes) && !memcmp(bytes, output, read));
    arena_position_marker mark = arena_position_mark(source_test_files.memory);
    file_result whole = file_read_all(source_test_files.memory, (cstring){first});
    TEST_ASSERT(whole.error == file_error_ok && whole.value.len == sizeof(bytes) && !memcmp(whole.value.data, bytes, sizeof(bytes)));
    TEST_ASSERT(arena_pop_to_marker(mark) == arena_error_ok);
    bytes[sizeof(bytes) - 1] ^= 1;
    TEST_ASSERT_FILE_OK(file_write_bytes((cstring){second}, bytes, sizeof(bytes)));
    int equal = 1;
    TEST_ASSERT_FILE_OK(file_equal((cstring){first}, (cstring){second}, &equal)); TEST_ASSERT(!equal);
    TEST_ASSERT_FILE_BYTES(((cstring){second}), bytes, sizeof(bytes));
    read = 7;
    TEST_ASSERT(file_read_into((cstring){first}, output, 1, &read) == file_error_capacity && read == 7);
)

TEST(directory_copy_compare_layout_and_safe_removal,
    char root[NATIVE_PATH_SIZE], copy[NATIVE_PATH_SIZE], path[NATIVE_PATH_SIZE], other[NATIVE_PATH_SIZE];
    TEST_ASSERT_FILE_OK(file_tree_join(root, source_test_case_root, "tree"));
    TEST_ASSERT_FILE_OK(file_tree_join(copy, source_test_case_root, "copy"));
    TEST_ASSERT(file_directory_create(root, 1));
    TEST_ASSERT_FILE_OK(file_tree_join(path, root, "empty")); TEST_ASSERT(file_directory_create(path, 1));
    TEST_ASSERT_FILE_OK(file_tree_join(path, root, "file")); TEST_ASSERT_FILE_OK(file_create_text((cstring){path}, "caf\xc3\xa9\r\n"));
    const u8 binary[] = {0, 255, 0, 1};
    TEST_ASSERT_FILE_OK(file_tree_join(path, root, "binary")); TEST_ASSERT_FILE_OK(file_create_bytes((cstring){path}, binary, sizeof(binary)));
    usize before = arena_pos(source_test_files.memory);
    TEST_ASSERT_FILE_OK(file_directory_copy(&source_test_files, (cstring){root}, (cstring){copy}));
    TEST_ASSERT_DIRECTORIES_EQUAL(&source_test_files, ((cstring){root}), ((cstring){copy}));
    TEST_ASSERT(arena_pos(source_test_files.memory) == before);
    TEST_ASSERT(file_directory_copy(&source_test_files, (cstring){root}, (cstring){copy}) == file_error_exists);
    TEST_ASSERT(file_create_text((cstring){root}, "must not replace directory") == file_error_exists);
    TEST_ASSERT_FILE_OK(file_tree_join(other, copy, "file"));
    TEST_ASSERT_FILE_OK(file_set_read_only((cstring){other}, 1));
    int equal = 0;
    file_error comparison = file_directory_equal(&source_test_files, (cstring){root}, (cstring){copy}, &equal);
    int restored = file_set_read_only((cstring){other}, 0) == file_error_ok;
    TEST_ASSERT(restored && comparison == file_error_ok && equal); /* Permissions ignored. */
    TEST_ASSERT_FILE_OK(file_write_bytes((cstring){other}, "different", 9));
    TEST_ASSERT_FILE_OK(file_directory_equal(&source_test_files, (cstring){root}, (cstring){copy}, &equal)); TEST_ASSERT(!equal);
    TEST_ASSERT_FILE_OK(file_write_bytes((cstring){other}, "caf\xc3\xa9\r\n", 7));
    TEST_ASSERT_FILE_OK(file_tree_join(other, copy, "empty"));
    TEST_ASSERT_FILE_OK(file_directory_remove_tree(&source_test_files, (cstring){other}));
    TEST_ASSERT_FILE_OK(file_create_text((cstring){other}, "")); /* Same name, different kind. */
    TEST_ASSERT_FILE_OK(file_directory_equal(&source_test_files, (cstring){root}, (cstring){copy}, &equal)); TEST_ASSERT(!equal);
    TEST_ASSERT_FILE_OK(file_remove((cstring){other})); /* Missing empty directory also differs. */
    TEST_ASSERT_FILE_OK(file_directory_equal(&source_test_files, (cstring){root}, (cstring){copy}, &equal)); TEST_ASSERT(!equal);
    TEST_ASSERT_FILE_OK(file_tree_join(other, copy, "EMPTY")); TEST_ASSERT(file_directory_create(other, 1));
    TEST_ASSERT_FILE_OK(file_directory_equal(&source_test_files, (cstring){root}, (cstring){copy}, &equal)); TEST_ASSERT(!equal); /* Exact name case. */
    TEST_ASSERT_FILE_OK(file_directory_remove_tree(&source_test_files, (cstring){copy}));
    TEST_ASSERT(!file_is_directory(copy, 1));
    TEST_ASSERT(file_directory_remove_tree(&source_test_files, (cstring){copy}) == file_error_not_found);
    TEST_ASSERT(file_remove((cstring){root}) == file_error_invalid);
    TEST_ASSERT(file_directory_remove_tree(&source_test_files, (cstring){"."}) == file_error_invalid);
    TEST_ASSERT(file_directory_remove_tree(&source_test_files, (cstring){".."}) == file_error_invalid);
    TEST_ASSERT(file_directory_remove_tree(&source_test_files, (cstring){"/"}) == file_error_invalid);
    TEST_ASSERT_FILE_OK(file_tree_join(other, root, "child"));
    TEST_ASSERT(file_directory_copy(&source_test_files, (cstring){root}, (cstring){other}) == file_error_invalid);
    TEST_ASSERT(!file_is_directory(other, 1));
)

typedef struct file_search_test_result {
    char paths[16][NATIVE_PATH_SIZE];
    file_entry_kind kinds[16];
    usize count, stop_after;
    unsigned issues[file_search_entry + 1];
} file_search_test_result;
static file_error file_search_test_visit(void *user, const file_tree_entry *entry) {
    file_search_test_result *result = user;
    if (result->count == 16 || strlen(entry->path) >= NATIVE_PATH_SIZE) return file_error_capacity;
    memcpy(result->paths[result->count], entry->path, strlen(entry->path) + 1);
    result->kinds[result->count++] = entry->kind;
    return result->stop_after && result->count == result->stop_after ? file_error_capacity : file_error_ok;
}
static void file_search_test_issue(void *user, const char *path, file_search_problem problem) {
    file_search_test_result *result = user;
    (void)path; ++result->issues[problem];
}
static file_error file_search_test_run(const char *root, file_search_options *options, file_search_test_result *result) {
    arena_position_marker mark = arena_position_mark(source_test_files.memory);
    file_error error = file_directory_search(source_test_files.memory, (cstring){(char *)root}, options, file_search_test_visit, result);
    (void)arena_pop_to_marker(mark); return error;
}
TEST(directory_search_globs_are_literal_utf8_and_case_optional,
    TEST_ASSERT(file_search_match("*.md", "README.MD", 0));
    TEST_ASSERT(!file_search_match("*.md", "README.MD", 1));
    TEST_ASSERT(!file_search_match("*.md", "README.md.bak", 0));
    TEST_ASSERT(file_search_match("caf?.md", "caf\xc3\xa9.md", 1));
    TEST_ASSERT(file_search_match("?.md", "\xf0\x9f\x98\x80.md", 1));
    TEST_ASSERT(file_search_match("*caf?.md", "x caf\xc3\xa9.md", 1));
    TEST_ASSERT(!file_search_match("caf\xc3\xa9.md", "CAF\xc3\x89.md", 0));
    TEST_ASSERT(file_search_match("a*b?c", "aaabbbc", 1));
    TEST_ASSERT(file_search_match("***", "", 1));
    TEST_ASSERT(file_search_match("", "", 1));
    TEST_ASSERT(!file_search_match("?", "", 1));
    TEST_ASSERT(!file_search_match("*.*", "README", 0));
    file_search_options options = {.pattern = "nested/*.md", .max_results = 128, .max_depth = 3};
    file_search_test_result result = {0};
    TEST_ASSERT(file_search_test_run(source_test_case_root, &options, &result) == file_error_invalid);
    options.pattern = "nested\\*.md";
    TEST_ASSERT(file_search_test_run(source_test_case_root, &options, &result) == file_error_invalid);
    options.pattern = "\xc0\xaf";
    TEST_ASSERT(file_search_test_run(source_test_case_root, &options, &result) == file_error_invalid);
    char oversized[NATIVE_PATH_SIZE + 1]; memset(oversized, '*', sizeof(oversized)); oversized[NATIVE_PATH_SIZE] = 0;
    options.pattern = oversized;
    TEST_ASSERT(file_search_test_run(source_test_case_root, &options, &result) == file_error_invalid);
)
TEST(directory_search_filters_files_not_recursion_and_prioritizes_root_matches,
    char root[NATIVE_PATH_SIZE], path[NATIVE_PATH_SIZE];
    TEST_ASSERT_FILE_OK(file_tree_join(root, source_test_case_root, "tree")); TEST_ASSERT(file_directory_create(root, 1));
    const char *directories[] = {"nested.md", "nested.md/deeper", ".hidden", "empty"};
    for (usize i = 0; i < sizeof(directories)/sizeof(directories[0]); ++i) {
        TEST_ASSERT_FILE_OK(file_tree_join(path, root, directories[i])); TEST_ASSERT(file_directory_create(path, 1));
    }
    const char *files[] = {"one.md", "two.MD", "skip.bin", "nested.md/guide.md", "nested.md/deeper/caf\xc3\xa9.md", "nested.md/deeper/literal.md.bak", ".secret.md", ".hidden/no.md", "\xf0\x9f\x98\x80.md"};
    for (usize i = 0; i < sizeof(files)/sizeof(files[0]); ++i) {
        TEST_ASSERT_FILE_OK(file_tree_join(path, root, files[i])); TEST_ASSERT_FILE_OK(file_create_text((cstring){path}, "unchanged"));
    }
    file_search_options options = {.pattern = "*.md", .max_results = 128, .max_depth = 3, .skip_dot_names = 1};
    file_search_test_result result = {0};
    TEST_ASSERT_FILE_OK(file_search_test_run(root, &options, &result)); TEST_ASSERT(result.count == 5);
    TEST_ASSERT(!strchr(result.paths[0], '/') && !strchr(result.paths[1], '/'));
    for (usize i = 0; i < result.count; ++i) TEST_ASSERT(result.kinds[i] == file_entry_regular);
    memset(&result, 0, sizeof(result)); options.match_case = 1;
    TEST_ASSERT_FILE_OK(file_search_test_run(root, &options, &result)); TEST_ASSERT(result.count == 4);
    memset(&result, 0, sizeof(result)); options.pattern = "caf?.md";
    TEST_ASSERT_FILE_OK(file_search_test_run(root, &options, &result));
    TEST_ASSERT(result.count == 1 && !strcmp(result.paths[0], "nested.md/deeper/caf\xc3\xa9.md"));
    memset(&result, 0, sizeof(result)); options.pattern = "?.md";
    TEST_ASSERT_FILE_OK(file_search_test_run(root, &options, &result));
    TEST_ASSERT(result.count == 1 && !strcmp(result.paths[0], "\xf0\x9f\x98\x80.md"));
    memset(&result, 0, sizeof(result)); options.pattern = "";
    TEST_ASSERT_FILE_OK(file_search_test_run(root, &options, &result)); TEST_ASSERT(!result.count);
    memset(&result, 0, sizeof(result)); options.pattern = NULL;
    TEST_ASSERT_FILE_OK(file_search_test_run(root, &options, &result)); TEST_ASSERT(result.count == 10);
    memset(&result, 0, sizeof(result)); options.pattern = "no-match*";
    TEST_ASSERT_FILE_OK(file_search_test_run(root, &options, &result)); TEST_ASSERT(!result.count);
    memset(&result, 0, sizeof(result)); options.root_only = 1; options.max_depth = 0; options.pattern = "*.md"; options.match_case = 0;
    TEST_ASSERT_FILE_OK(file_search_test_run(root, &options, &result)); TEST_ASSERT(result.count == 3);
    for (usize i = 0; i < result.count; ++i) TEST_ASSERT(!strchr(result.paths[i], '/'));
    memset(&result, 0, sizeof(result)); options.pattern = NULL;
    TEST_ASSERT_FILE_OK(file_search_test_run(root, &options, &result)); TEST_ASSERT(result.count == 6);
    for (usize i = 0; i < result.count; ++i) TEST_ASSERT(!strchr(result.paths[i], '/'));
    for (usize i = 0; i < sizeof(files)/sizeof(files[0]); ++i) {
        TEST_ASSERT_FILE_OK(file_tree_join(path, root, files[i])); TEST_ASSERT_FILE_TEXT(((cstring){path}), "unchanged");
    }
)
TEST(directory_search_limits_partial_results_and_early_stop_close_handles,
    char root[NATIVE_PATH_SIZE], path[NATIVE_PATH_SIZE];
    TEST_ASSERT_FILE_OK(file_tree_join(root, source_test_case_root, "tree")); TEST_ASSERT(file_directory_create(root, 1));
    TEST_ASSERT_FILE_OK(file_tree_join(path, root, "root.md")); TEST_ASSERT_FILE_OK(file_create_text((cstring){path}, "original"));
    TEST_ASSERT_FILE_OK(file_tree_join(path, root, "nested")); TEST_ASSERT(file_directory_create(path, 1));
    TEST_ASSERT_FILE_OK(file_tree_join(path, root, "nested/child.md")); TEST_ASSERT_FILE_OK(file_create_text((cstring){path}, "deep"));
    file_search_options options = {.pattern = "*.md", .max_results = 128, .max_depth = 0, .issue = file_search_test_issue};
    file_search_test_result result = {0};
    TEST_ASSERT(file_search_test_run(root, &options, &result) == file_error_capacity);
    TEST_ASSERT(result.count == 1 && !strcmp(result.paths[0], "root.md") && result.issues[file_search_depth_limit] == 1);
    memset(&result, 0, sizeof(result)); options.max_results = 1; options.max_depth = 3;
    TEST_ASSERT(file_search_test_run(root, &options, &result) == file_error_capacity);
    TEST_ASSERT(result.count == 1 && result.issues[file_search_result_limit] == 1);
    /* Exact match/directory budgets succeed; unrelated files do not count. */
    memset(&result, 0, sizeof(result)); options.max_results = 2; options.max_directories = 1;
    TEST_ASSERT_FILE_OK(file_search_test_run(root, &options, &result)); TEST_ASSERT(result.count == 2);
    memset(&result, 0, sizeof(result)); options.pattern = "no-match"; options.max_results = 1;
    TEST_ASSERT_FILE_OK(file_search_test_run(root, &options, &result)); TEST_ASSERT(!result.count);
#if defined(_WIN32)
    DWORD handles_before = 0, handles_after = 0;
    TEST_ASSERT(GetProcessHandleCount(GetCurrentProcess(), &handles_before));
#endif
    for (int iteration = 0; iteration < 32; ++iteration) {
        memset(&result, 0, sizeof(result)); result.stop_after = 1;
        options.pattern = "*.md"; options.max_results = 128;
        TEST_ASSERT(file_search_test_run(root, &options, &result) == file_error_capacity && result.count == 1);
    }
    /* Fail child-frame allocation after opening root, not only before search. */
    u8 limited[sizeof(file_search_frame) + 256];
    for (int iteration = 0; iteration < 32; ++iteration) {
        memset(&result, 0, sizeof(result));
        arena *memory = arena_from_buffer(limited, sizeof(limited));
        TEST_ASSERT(file_directory_search(memory, (cstring){root}, &options, file_search_test_visit, &result) == file_error_out_of_memory);
        TEST_ASSERT(result.count == 1 && !strcmp(result.paths[0], "root.md"));
    }
#if defined(_WIN32)
    TEST_ASSERT(GetProcessHandleCount(GetCurrentProcess(), &handles_after) && handles_before == handles_after);
#endif
    u8 tiny[128]; arena *memory = arena_from_buffer(tiny, sizeof(tiny));
    TEST_ASSERT(file_directory_search(memory, (cstring){root}, &options, file_search_test_visit, &result) == file_error_out_of_memory);
    TEST_ASSERT_FILE_TEXT(((cstring){path}), "deep");
)

TEST(directory_search_ignores_cache_files_but_bounds_matches_and_directories,
    char root[NATIVE_PATH_SIZE], path[NATIVE_PATH_SIZE];
    TEST_ASSERT_FILE_OK(file_tree_join(root, source_test_case_root, "tree")); TEST_ASSERT(file_directory_create(root, 1));
    TEST_ASSERT_FILE_OK(file_tree_join(path, root, "cache")); TEST_ASSERT(file_directory_create(path, 1));
    TEST_ASSERT_FILE_OK(file_tree_join(path, root, "docs")); TEST_ASSERT(file_directory_create(path, 1));
    TEST_ASSERT_FILE_OK(file_tree_join(path, root, "README.md")); TEST_ASSERT_FILE_OK(file_create_text((cstring){path}, "root"));
    TEST_ASSERT_FILE_OK(file_tree_join(path, root, "docs/last.md")); TEST_ASSERT_FILE_OK(file_create_text((cstring){path}, "last"));
    file_search_options options = {.pattern = "*.md", .max_results = 2, .max_directories = 2, .max_depth = 3, .issue = file_search_test_issue};
    file_search_test_result result = {0};
    arena_position_marker mark = arena_position_mark(source_test_files.memory);
    usize before = arena_pos(source_test_files.memory);
    TEST_ASSERT_FILE_OK(file_directory_search(source_test_files.memory, (cstring){root}, &options, file_search_test_visit, &result));
    usize scratch_size = arena_pos(source_test_files.memory) - before;
    TEST_ASSERT(result.count == 2 && arena_pop_to_marker(mark) == arena_error_ok);
    for (int i = 0; i < 64; ++i) {
        char relative[64]; snprintf(relative, sizeof(relative), "%snoise-%03d.bin", i % 2 ? "cache/" : "", i);
        TEST_ASSERT_FILE_OK(file_tree_join(path, root, relative)); TEST_ASSERT_FILE_OK(file_create_text((cstring){path}, "unrelated"));
    }
    memset(&result, 0, sizeof(result));
    TEST_ASSERT_FILE_OK(file_directory_search(source_test_files.memory, (cstring){root}, &options, file_search_test_visit, &result));
    TEST_ASSERT(result.count == 2 && arena_pos(source_test_files.memory) - before == scratch_size);
    TEST_ASSERT(arena_pop_to_marker(mark) == arena_error_ok);
#if defined(_WIN32)
    /* Prove *.md reaches Windows, rather than enumerating '*' then filtering. */
    file_directory directory; int native_matches = 0, valid = 1, next = 0;
    TEST_ASSERT(file_directory_open_search(&directory, root, "*.md", 0));
    while ((next = file_directory_read(&directory)) > 0) {
        if (directory.entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        ++native_matches;
        if (!file_search_match("*.md", directory.name, 0)) valid = 0;
    }
    int closed = file_directory_close(&directory);
    TEST_ASSERT(closed && !next && valid && native_matches == 1);
#endif
    memset(&result, 0, sizeof(result)); options.max_directories = 1;
    TEST_ASSERT(file_search_test_run(root, &options, &result) == file_error_capacity);
    TEST_ASSERT(result.count >= 1 && result.issues[file_search_directory_limit] == 1);
    memset(&result, 0, sizeof(result)); options.max_directories = 2; options.max_results = 1;
    TEST_ASSERT(file_search_test_run(root, &options, &result) == file_error_capacity);
    TEST_ASSERT(result.count == 1 && result.issues[file_search_result_limit] == 1);
    memset(&result, 0, sizeof(result)); options.pattern = "no-match*";
    TEST_ASSERT_FILE_OK(file_search_test_run(root, &options, &result)); TEST_ASSERT(!result.count);
    TEST_ASSERT_FILE_OK(file_tree_join(path, root, "docs/last.md")); TEST_ASSERT_FILE_TEXT(((cstring){path}), "last");
    TEST_ASSERT_FILE_OK(file_tree_join(path, root, "README.md")); TEST_ASSERT_FILE_TEXT(((cstring){path}), "root");
)

TEST(directory_limits_preflight_without_mutation,
    const file_tree_entry unsorted[] = {{"z", 0, 0}, {"b/nested", 0, 0}, {"a", 0, 0}, {"B", 0, 0}, {"a", 0, 0}};
    for (usize count = 0; count <= 5; ++count) {
        file_tree_entry entries[5]; memcpy(entries, unsorted, sizeof(entries));
        file_tree_sort(entries, count);
        for (usize i = 1; i < count; ++i) TEST_ASSERT(strcmp(entries[i - 1].path, entries[i].path) <= 0);
    }
    char root[NATIVE_PATH_SIZE], copy[NATIVE_PATH_SIZE], path[NATIVE_PATH_SIZE];
    TEST_ASSERT_FILE_OK(file_tree_join(root, source_test_case_root, "tree"));
    TEST_ASSERT_FILE_OK(file_tree_join(copy, source_test_case_root, "copy"));
    TEST_ASSERT(file_directory_create(root, 1));
    TEST_ASSERT_FILE_OK(file_directory_copy(&source_test_files, (cstring){root}, (cstring){copy}));
    TEST_ASSERT_DIRECTORIES_EQUAL(&source_test_files, ((cstring){root}), ((cstring){copy}));
    TEST_ASSERT_FILE_OK(file_directory_remove_tree(&source_test_files, (cstring){copy}));
    TEST_ASSERT_FILE_OK(file_tree_join(path, root, "nested")); TEST_ASSERT(file_directory_create(path, 1));
    TEST_ASSERT_FILE_OK(file_tree_join(path, root, "nested/file")); TEST_ASSERT_FILE_OK(file_create_text((cstring){path}, "keep"));
    file_tree_workspace limited = source_test_files; limited.max_depth = 2; limited.max_entries = 2;
    TEST_ASSERT_FILE_OK(file_directory_copy(&limited, (cstring){root}, (cstring){copy}));
    TEST_ASSERT_DIRECTORIES_EQUAL(&limited, ((cstring){root}), ((cstring){copy}));
    TEST_ASSERT_FILE_OK(file_directory_remove_tree(&limited, (cstring){copy}));
    limited.max_entries = 1;
    TEST_ASSERT(file_directory_copy(&limited, (cstring){root}, (cstring){copy}) == file_error_capacity);
    TEST_ASSERT(!file_is_directory(copy, 1));
    TEST_ASSERT(file_directory_remove_tree(&limited, (cstring){root}) == file_error_capacity);
    TEST_ASSERT_FILE_TEXT(((cstring){path}), "keep");
    int equal = 9;
    TEST_ASSERT(file_directory_equal(&limited, (cstring){root}, (cstring){root}, &equal) == file_error_capacity && equal == 9);
    limited.max_entries = 2; limited.max_depth = 1;
    TEST_ASSERT(file_directory_copy(&limited, (cstring){root}, (cstring){copy}) == file_error_capacity);
    TEST_ASSERT(!file_is_directory(copy, 1));
    u8 small[128]; limited = (file_tree_workspace){ arena_from_buffer(small, sizeof(small)), 2, 32 };
    TEST_ASSERT(file_directory_copy(&limited, (cstring){root}, (cstring){copy}) == file_error_out_of_memory);
    TEST_ASSERT(!file_is_directory(copy, 1));
    TEST_ASSERT(file_directory_equal(NULL, (cstring){root}, (cstring){root}, &equal) == file_error_invalid);
    TEST_ASSERT(file_directory_equal(&source_test_files, (cstring){copy}, (cstring){root}, &equal) == file_error_not_found && equal == 9);
    TEST_ASSERT_FILE_OK(file_tree_join(path, source_test_case_root, "absent/file"));
    TEST_ASSERT(file_create_text((cstring){path}, "no implicit parent") == file_error_not_found);
)

/* Create owned junction without elevation on Windows; symlink on POSIX.
   Explicit removal below is test-only: generic operations must refuse links. */
static int filesystem_test_link(const char *path, const char *target) {
#if defined(_WIN32)
    char absolute[NATIVE_PATH_SIZE];
    if (file_tree_path((cstring){(char *)target}, absolute, 0) != file_error_ok) return 0;
    for (char *p = absolute; *p; ++p) if (*p == '/') *p = '\\';
    struct {
        DWORD tag; WORD length, reserved;
        WORD substitute_offset, substitute_length, print_offset, print_length;
        WCHAR names[NATIVE_PATH_SIZE * 2];
    } data = {0};
    data.tag = IO_REPARSE_TAG_MOUNT_POINT;
    WCHAR wide[NATIVE_PATH_SIZE];
    int units = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, absolute, -1, wide, NATIVE_PATH_SIZE);
    if (!units || units + 4 > NATIVE_PATH_SIZE || !file_directory_create(path, 1)) return 0;
    memcpy(data.names, L"\\??\\", 4 * sizeof(WCHAR));
    memcpy(data.names + 4, wide, units * sizeof(WCHAR));
    data.substitute_length = (WORD)((units + 3) * sizeof(WCHAR));
    data.print_offset = (WORD)((units + 4) * sizeof(WCHAR));
    data.print_length = (WORD)((units - 1) * sizeof(WCHAR));
    memcpy((u8 *)data.names + data.print_offset, wide, units * sizeof(WCHAR));
    data.length = (WORD)(8 + data.print_offset + units * sizeof(WCHAR));
    if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path, -1, wide, NATIVE_PATH_SIZE)) return 0;
    HANDLE handle = CreateFileW(wide, GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
                                FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (handle == INVALID_HANDLE_VALUE) return 0;
    DWORD returned;
    /* FSCTL_SET_REPARSE_POINT from winioctl.h. */
    int ok = DeviceIoControl(handle, 0x000900a4, &data, data.length + 8, NULL, 0, &returned, NULL);
    return CloseHandle(handle) && ok;
#else
    return symlink(target, path) == 0;
#endif
}
static int filesystem_test_unlink(const char *path) {
#if defined(_WIN32)
    return file_native_remove(path, 1) == file_error_ok;
#else
    return unlink(path) == 0;
#endif
}

TEST(directory_search_skips_links_only_when_requested_and_inventory_stays_strict,
    char root[NATIVE_PATH_SIZE], outside[NATIVE_PATH_SIZE], path[NATIVE_PATH_SIZE], link[NATIVE_PATH_SIZE];
    TEST_ASSERT_FILE_OK(file_tree_join(root, source_test_case_root, "tree")); TEST_ASSERT(file_directory_create(root, 1));
    TEST_ASSERT_FILE_OK(file_tree_join(outside, source_test_case_root, "outside")); TEST_ASSERT(file_directory_create(outside, 1));
    TEST_ASSERT_FILE_OK(file_tree_join(path, outside, "outside.md")); TEST_ASSERT_FILE_OK(file_create_text((cstring){path}, "untouched"));
    TEST_ASSERT_FILE_OK(file_tree_join(path, root, "inside.md")); TEST_ASSERT_FILE_OK(file_create_text((cstring){path}, "inside"));
    TEST_ASSERT_FILE_OK(file_tree_join(link, root, "linked.md")); TEST_ASSERT(filesystem_test_link(link, outside));
    file_search_options options = {.pattern = "*.md", .max_results = 128, .max_depth = 3, .skip_unsupported = 1};
    file_search_test_result result = {0};
    file_error skipped = file_search_test_run(root, &options, &result);
    int skipped_ok = skipped == file_error_ok && result.count == 1 && !strcmp(result.paths[0], "inside.md");
    memset(&result, 0, sizeof(result)); options.skip_unsupported = 0; options.issue = file_search_test_issue;
    file_error refused = file_search_test_run(root, &options, &result);
    int refused_ok = refused == file_error_unsupported && result.count == 1 && result.issues[file_search_entry] == 1;
    file_tree tree = {0}; usize before = arena_pos(source_test_files.memory);
    file_error strict = file_directory_list_tree(&source_test_files, (cstring){root}, &tree);
    int strict_ok = strict == file_error_unsupported && !tree.entries && !tree.count && arena_pos(source_test_files.memory) == before;
    file_error ancestor = file_search_test_run(link, &options, &result);
    int removed = filesystem_test_unlink(link);
    TEST_ASSERT(removed && skipped_ok && refused_ok && strict_ok && ancestor == file_error_unsupported);
    TEST_ASSERT_FILE_OK(file_tree_join(path, outside, "outside.md")); TEST_ASSERT_FILE_TEXT(((cstring){path}), "untouched");
)

TEST(directory_links_refused_including_ancestors_and_broken_links,
    char root[NATIVE_PATH_SIZE], outside[NATIVE_PATH_SIZE], link[NATIVE_PATH_SIZE], copy[NATIVE_PATH_SIZE], path[NATIVE_PATH_SIZE], keep[NATIVE_PATH_SIZE];
    TEST_ASSERT_FILE_OK(file_tree_join(root, source_test_case_root, "tree"));
    TEST_ASSERT_FILE_OK(file_tree_join(outside, source_test_case_root, "outside"));
    TEST_ASSERT_FILE_OK(file_tree_join(copy, source_test_case_root, "copy"));
    TEST_ASSERT(file_directory_create(root, 1) && file_directory_create(outside, 1));
    TEST_ASSERT_FILE_OK(file_tree_join(keep, outside, "keep")); TEST_ASSERT_FILE_OK(file_create_text((cstring){keep}, "untouched"));
    TEST_ASSERT_FILE_OK(file_tree_join(link, root, "link")); TEST_ASSERT(filesystem_test_link(link, outside));
    TEST_ASSERT(file_directory_remove_tree(&source_test_files, (cstring){link}) == file_error_unsupported);
    test_directory linked = {0};
    TEST_ASSERT(test_directory_create(&linked, (cstring){link}) == file_error_unsupported && !linked.owned);
    int equal = 9;
    TEST_ASSERT(file_directory_equal(&source_test_files, (cstring){root}, (cstring){outside}, &equal) == file_error_unsupported && equal == 9);
    TEST_ASSERT(file_directory_copy(&source_test_files, (cstring){root}, (cstring){copy}) == file_error_unsupported);
    TEST_ASSERT(file_directory_remove_tree(&source_test_files, (cstring){root}) == file_error_unsupported);
    TEST_ASSERT(!file_is_directory(copy, 1));
    TEST_ASSERT_FILE_OK(file_tree_join(path, link, "keep"));
    TEST_ASSERT(file_equal_text((cstring){path}, "untouched", &equal) == file_error_unsupported);
    TEST_ASSERT(file_ends_with_text((cstring){path}, "touched", &equal) == file_error_unsupported);
    TEST_ASSERT(file_set_read_only((cstring){path}, 1) == file_error_unsupported);
    TEST_ASSERT_FILE_OK(file_tree_join(path, link, "new"));
    TEST_ASSERT(file_create_text((cstring){path}, "must not follow") == file_error_unsupported);
    TEST_ASSERT_FILE_TEXT(((cstring){keep}), "untouched");
    TEST_ASSERT_FILE_OK(file_directory_remove_tree(&source_test_files, (cstring){outside}));
    TEST_ASSERT(file_directory_remove_tree(&source_test_files, (cstring){root}) == file_error_unsupported);
    TEST_ASSERT(filesystem_test_unlink(link));
    TEST_ASSERT_FILE_OK(file_directory_remove_tree(&source_test_files, (cstring){root}));
)

TEST(file_assertions_evaluate_arguments_once,
    char path[NATIVE_PATH_SIZE];
    TEST_ASSERT_FILE_OK(file_tree_join(path, source_test_case_root, "once"));
    cstring paths[] = {{path}, {path}, {path}, {path}};
    const char *texts[] = {"value", "value"};
    const void *bytes[] = {"value"}; usize sizes[] = {5};
    file_tree_workspace *workspaces[] = {&source_test_files};
    int operations = 0, path_calls = 0, text_calls = 0, byte_calls = 0, size_calls = 0, workspace_calls = 0;
    TEST_ASSERT_FILE_OK((++operations, file_create_text((cstring){path}, "value")));
    TEST_ASSERT_FILE_TEXT(paths[path_calls++], texts[text_calls++]);
    TEST_ASSERT_FILE_BYTES(paths[path_calls++], bytes[byte_calls++], sizes[size_calls++]);
    /* Separate counters avoid unsequenced modifications in function arguments. */
    int left_calls = 0, right_calls = 0;
    TEST_ASSERT_FILES_EQUAL(paths[left_calls++], paths[right_calls++]);
    cstring roots[] = {{source_test_case_root}}; left_calls = right_calls = 0;
    TEST_ASSERT_DIRECTORIES_EQUAL(workspaces[workspace_calls++], roots[left_calls++], roots[right_calls++]);
    TEST_ASSERT(operations == 1 && path_calls == 2 && text_calls == 1 && byte_calls == 1 && size_calls == 1 &&
                workspace_calls == 1 && left_calls == 1 && right_calls == 1);
)

TEST(directory_platform_specific_safety,
#if defined(_WIN32)
    char alias[NATIVE_PATH_SIZE], child[NATIVE_PATH_SIZE];
    TEST_ASSERT(strlen(source_test_case_root) < sizeof(alias));
    strcpy(alias, source_test_case_root);
    char *name = strstr(alias, "caf\xc3\xa9"); TEST_ASSERT(name != NULL);
    memcpy(name, "CAF\xc3\x89", 5);
    TEST_ASSERT_FILE_OK(file_tree_join(child, alias, "child"));
    TEST_ASSERT(file_directory_copy(&source_test_files, (cstring){source_test_case_root}, (cstring){child}) == file_error_invalid);
    TEST_ASSERT(!file_is_directory(child, 1));
    TEST_ASSERT(file_directory_create(child, 1));
    char path[NATIVE_PATH_SIZE];
    TEST_ASSERT_FILE_OK(file_tree_join(path, child, "readonly"));
    TEST_ASSERT_FILE_OK(file_create_text((cstring){path}, "keep"));
    TEST_ASSERT_FILE_OK(file_set_read_only((cstring){path}, 1));
    file_error removal = file_directory_remove_tree(&source_test_files, (cstring){child});
    int restored = file_set_read_only((cstring){path}, 0) == file_error_ok;
    TEST_ASSERT(restored && removal == file_error_cleanup_failed);
    TEST_ASSERT_FILE_TEXT(((cstring){path}), "keep");
#else
    char path[NATIVE_PATH_SIZE];
    TEST_ASSERT_FILE_OK(file_tree_join(path, source_test_case_root, "fifo"));
    TEST_ASSERT(mkfifo(path, 0600) == 0);
    int equal = 9;
    TEST_ASSERT(file_directory_equal(&source_test_files, (cstring){source_test_case_root}, (cstring){source_test_case_root}, &equal) == file_error_unsupported && equal == 9);
    TEST_ASSERT(unlink(path) == 0);
#endif
)
