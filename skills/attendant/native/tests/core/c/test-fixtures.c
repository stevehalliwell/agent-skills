#include "test.h"

TEST(environment_utf8_empty_unset_and_capacity,
    char name[128], output[128] = "unchanged";
#if defined(_WIN32)
    unsigned long process = GetCurrentProcessId();
#else
    unsigned long process = (unsigned long)getpid();
#endif
    snprintf(name, sizeof(name), "PI_BASE_TEST_%lu_caf\xc3\xa9", process);
    arena *scratch = source_test_files.memory;
    usize mark = arena_pos(scratch), size = 17;
    TEST_ASSERT(environment_get(scratch, name, output, sizeof(output), &size) == environment_error_not_found);
    TEST_ASSERT(size == 17 && !strcmp(output, "unchanged"));
    const char *value = "caf\xc3\xa9 \xce\xa9 \xf0\x9f\x98\x80";
    TEST_ASSERT(environment_set(scratch, name, value) == environment_error_ok);
    TEST_ASSERT(environment_get(scratch, name, output, strlen(value) + 1, &size) == environment_error_ok);
    TEST_ASSERT(size == strlen(value) && !strcmp(output, value));
    strcpy(output, "unchanged"); size = 17;
    TEST_ASSERT(environment_get(scratch, name, output, strlen(value), &size) == environment_error_capacity);
    TEST_ASSERT(size == 17 && !strcmp(output, "unchanged"));
    TEST_ASSERT(environment_set(scratch, name, "") == environment_error_ok);
    TEST_ASSERT(environment_get(scratch, name, output, 1, &size) == environment_error_ok && size == 0 && !output[0]);
    size = 17;
    TEST_ASSERT(environment_get(scratch, name, NULL, 0, &size) == environment_error_capacity && size == 17);
    TEST_ASSERT(environment_set(scratch, name, NULL) == environment_error_ok);
    TEST_ASSERT(environment_set(scratch, name, NULL) == environment_error_ok); /* Idempotent unset. */
    TEST_ASSERT(environment_get(scratch, name, output, sizeof(output), &size) == environment_error_not_found && size == 17);
    TEST_ASSERT(environment_set(scratch, "", "value") == environment_error_invalid);
    TEST_ASSERT(environment_set(scratch, "bad=name", "value") == environment_error_invalid);
    TEST_ASSERT(environment_set(NULL, name, "value") == environment_error_invalid);
    TEST_ASSERT(environment_get(scratch, name, NULL, 1, &size) == environment_error_invalid);
    TEST_ASSERT(environment_get(scratch, name, output, sizeof(output), NULL) == environment_error_invalid);
#if defined(_WIN32)
    u8 small[128]; arena *limited = arena_from_buffer(small, sizeof(small));
    TEST_ASSERT(limited != NULL);
    char large[256]; memset(large, 'x', sizeof(large)); large[sizeof(large) - 1] = 0;
    TEST_ASSERT(environment_set(limited, name, large) == environment_error_capacity);
    TEST_ASSERT(environment_set(scratch, name, "\xff") == environment_error_invalid);
    TEST_ASSERT(environment_get(scratch, name, output, sizeof(output), &size) == environment_error_not_found);
#endif
    TEST_ASSERT(arena_pos(scratch) == mark);
)

TEST(file_suffix_streaming_binary_text_and_attributes,
    char path[NATIVE_PATH_SIZE];
    TEST_ASSERT_FILE_OK(file_tree_join(path, source_test_case_root, "suffix caf\xc3\xa9.bin"));
    u8 bytes[40000];
    for (usize i = 0; i < sizeof(bytes); ++i) bytes[i] = (u8)(i * 37);
    const char *suffix = "finish caf\xc3\xa9\r\n";
    memcpy(bytes + sizeof(bytes) - strlen(suffix), suffix, strlen(suffix));
    TEST_ASSERT_FILE_OK(file_create_bytes((cstring){path}, bytes, sizeof(bytes)));
    cstring paths[] = {{path}}; const char *texts[] = {suffix};
    const void *tails[] = {bytes + 15000}; usize sizes[] = {25000};
    int path_calls = 0, text_calls = 0, byte_calls = 0, size_calls = 0;
    TEST_ASSERT_FILE_ENDS_WITH_TEXT(paths[path_calls++], texts[text_calls++]);
    TEST_ASSERT(path_calls == 1 && text_calls == 1); path_calls = 0;
    TEST_ASSERT_FILE_ENDS_WITH_BYTES(paths[path_calls++], tails[byte_calls++], sizes[size_calls++]);
    TEST_ASSERT(path_calls == 1 && byte_calls == 1 && size_calls == 1);
    bytes[sizeof(bytes) - 1] ^= 1;
    int equal = 9;
    TEST_ASSERT_FILE_OK(file_ends_with_bytes((cstring){path}, bytes + 15000, 25000, &equal)); TEST_ASSERT(!equal);
    TEST_ASSERT_FILE_OK(file_ends_with_bytes((cstring){path}, NULL, 0, &equal)); TEST_ASSERT(equal);
    TEST_ASSERT(file_ends_with_bytes((cstring){path}, NULL, 1, &equal) == file_error_invalid);
    TEST_ASSERT(file_ends_with_text((cstring){path}, NULL, &equal) == file_error_invalid);
    TEST_ASSERT(file_ends_with_text((cstring){path}, "", NULL) == file_error_invalid);
    TEST_ASSERT(file_set_read_only((cstring){source_test_case_root}, 1) == file_error_invalid);
#if !defined(_WIN32)
    TEST_ASSERT(chmod(path, 0750) == 0);
    TEST_ASSERT_FILE_OK(file_set_read_only((cstring){path}, 1));
    struct stat status; TEST_ASSERT(lstat(path, &status) == 0 && (status.st_mode & 0777) == 0550);
    TEST_ASSERT_FILE_OK(file_set_read_only((cstring){path}, 0));
    TEST_ASSERT(lstat(path, &status) == 0 && (status.st_mode & 0777) == 0750);
#endif
    TEST_ASSERT_FILE_OK(file_remove((cstring){path}));
    TEST_ASSERT_FILE_OK(file_create_text((cstring){path}, "x"));
    TEST_ASSERT_FILE_OK(file_ends_with_text((cstring){path}, "longer", &equal)); TEST_ASSERT(!equal);
    TEST_ASSERT_FILE_ENDS_WITH_TEXT(((cstring){path}), "");
    TEST_ASSERT_FILE_OK(file_remove((cstring){path}));
    equal = 9;
    TEST_ASSERT(file_ends_with_text((cstring){path}, "", &equal) == file_error_not_found && equal == 9);
    TEST_ASSERT(file_set_read_only((cstring){path}, 1) == file_error_not_found);
)

TEST(test_directory_ownership_retention_reuse_and_limits,
    test_directory directory = {0};
    char root[NATIVE_PATH_SIZE], first[NATIVE_PATH_SIZE], second[NATIVE_PATH_SIZE];
    TEST_ASSERT_FILE_OK(file_tree_join(root, source_test_case_root, "owned"));
    TEST_ASSERT_FILE_OK(test_directory_finish(&directory, NULL, 0));
    TEST_ASSERT_FILE_OK(test_directory_create(&directory, (cstring){root}));
    TEST_ASSERT(directory.owned);
    TEST_ASSERT(test_directory_create(&directory, (cstring){root}) == file_error_invalid && directory.owned);
    TEST_ASSERT_FILE_OK(test_directory_next(&directory, "child caf\xc3\xa9", first));
    TEST_ASSERT_FILE_OK(test_directory_next(&directory, "child caf\xc3\xa9", second));
    TEST_ASSERT(strcmp(first, second) && strstr(first, "-001") && strstr(second, "-002"));
    TEST_ASSERT(!file_is_regular(first, 1)); /* Naming does not create files/directories. */
    TEST_ASSERT_FILE_OK(file_create_text((cstring){first}, "evidence"));
    usize sequence = directory.next_child;
    TEST_ASSERT(test_directory_next(&directory, "../escape", second) == file_error_invalid);
    TEST_ASSERT(test_directory_next(&directory, "..", second) == file_error_invalid);
    TEST_ASSERT(test_directory_next(&directory, "bad\\name", second) == file_error_invalid);
    TEST_ASSERT(directory.next_child == sequence);
    char label[NATIVE_PATH_SIZE]; memset(label, 'x', sizeof(label)); label[sizeof(label) - 1] = 0;
    TEST_ASSERT(test_directory_next(&directory, label, second) == file_error_capacity && directory.next_child == sequence);
    directory.next_child = SIZE_MAX;
    TEST_ASSERT(test_directory_next(&directory, "child", second) == file_error_capacity);
    directory.next_child = sequence;
    TEST_ASSERT_FILE_OK(test_directory_finish(&directory, &source_test_files, 1));
    TEST_ASSERT(!directory.owned);
    TEST_ASSERT_FILE_TEXT(((cstring){first}), "evidence");
    TEST_ASSERT_FILE_OK(test_directory_finish(&directory, &source_test_files, 0));
    TEST_ASSERT_FILE_TEXT(((cstring){first}), "evidence"); /* Finish cannot later delete retained evidence. */
    TEST_ASSERT(test_directory_create(&directory, (cstring){root}) == file_error_exists && !directory.owned);
    TEST_ASSERT_FILE_OK(test_directory_finish(&directory, &source_test_files, 0));
    TEST_ASSERT_FILE_TEXT(((cstring){first}), "evidence");
    TEST_ASSERT_FILE_OK(file_directory_remove_tree(&source_test_files, (cstring){root}));
    TEST_ASSERT_FILE_OK(test_directory_create(&directory, (cstring){root}));
    TEST_ASSERT(directory.next_child == 0);
    TEST_ASSERT_FILE_OK(test_directory_next(&directory, "file", first)); TEST_ASSERT_FILE_OK(file_create_text((cstring){first}, "one"));
    TEST_ASSERT_FILE_OK(test_directory_next(&directory, "file", second)); TEST_ASSERT_FILE_OK(file_create_text((cstring){second}, "two"));
    file_tree_workspace limited = source_test_files; limited.max_entries = 1;
    TEST_ASSERT(test_directory_finish(&directory, &limited, 0) == file_error_capacity && !directory.owned);
    TEST_ASSERT_FILE_TEXT(((cstring){first}), "one");
    TEST_ASSERT_FILE_OK(test_directory_finish(&directory, &source_test_files, 0));
    TEST_ASSERT_FILE_TEXT(((cstring){second}), "two");
    TEST_ASSERT_FILE_OK(file_directory_remove_tree(&source_test_files, (cstring){root}));
    TEST_ASSERT_FILE_OK(test_directory_create(&directory, (cstring){root}));
    TEST_ASSERT_FILE_OK(test_directory_finish(&directory, &source_test_files, 0));
    TEST_ASSERT(!file_is_directory(root, 1) && !directory.owned);
    TEST_ASSERT_FILE_OK(file_create_text((cstring){root}, "pre-existing file"));
    TEST_ASSERT(test_directory_create(&directory, (cstring){root}) == file_error_exists && !directory.owned);
    TEST_ASSERT_FILE_OK(test_directory_finish(&directory, &source_test_files, 0));
    TEST_ASSERT_FILE_TEXT(((cstring){root}), "pre-existing file");
    TEST_ASSERT(test_directory_next(&directory, "child", second) == file_error_invalid);
    TEST_ASSERT(test_directory_create(NULL, (cstring){root}) == file_error_invalid);
    TEST_ASSERT(test_directory_finish(NULL, &source_test_files, 0) == file_error_invalid);
)
