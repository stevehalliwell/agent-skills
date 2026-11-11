/* Attendant subprocess protocol/fixtures only. Generic supervision is test.h. */
#include "tests/core/c/support.h"
#include "vendor/libpdjson5/pdjson5.h"

static u8 integration_file_memory[4194304], integration_command_memory[262144];
static char integration_output[1048577], integration_errors[1048577];
static char integration_root[NATIVE_PATH_SIZE], integration_cli_path[NATIVE_PATH_SIZE], integration_probe[NATIVE_PATH_SIZE], integration_compiler[16];
static unsigned integration_groups;
static test_command_workspace integration_commands;
static test_command_result integration_result;
static test_directory integration_installation;

static int integration_suite_setup(void) {
    if (source_test_suite_setup()) return 1;
    source_test_files = (file_tree_workspace){arena_from_buffer(integration_file_memory, sizeof(integration_file_memory)), 32, 8192};
    integration_commands = (test_command_workspace){arena_from_buffer(integration_command_memory, sizeof(integration_command_memory)),
        integration_output, integration_errors, sizeof(integration_output), sizeof(integration_errors)};
    usize size;
    TEST_ASSERT(environment_get(source_test_files.memory, "ATTENDANT_TEST_COMPILER", integration_compiler, sizeof(integration_compiler), &size) == environment_error_ok);
    TEST_ASSERT(!strcmp(integration_compiler, "cjit") || !strcmp(integration_compiler, "zig"));
    TEST_ASSERT_FILE_OK(file_tree_path((cstring){"."}, integration_root, 0));
#if defined(_WIN32)
    const char *suffix = ".exe";
#else
    const char *suffix = "";
#endif
    char relative[NATIVE_PATH_SIZE];
    snprintf(relative, sizeof(relative), "build/attendant-%s%s", integration_compiler, suffix);
    TEST_ASSERT_FILE_OK(file_tree_join(integration_cli_path, integration_root, relative));
    snprintf(relative, sizeof(relative), "build/api-probe-%s%s", integration_compiler, suffix);
    TEST_ASSERT_FILE_OK(file_tree_join(integration_probe, integration_root, relative));
    TEST_ASSERT(file_is_regular(integration_cli_path, 0) && file_is_regular(integration_probe, 0));
    return 0;
}
static int integration_suite_teardown(int failed) {
    fprintf(stderr, "integration-groups:%u\n", integration_groups);
    int missing = !failed && integration_groups != 23;
    int result = source_test_suite_teardown(failed || missing);
    return result || missing;
}
static int integration_case_teardown(const test_case *test, int failed) {
    arena_reset(source_test_files.memory);
    file_error installation = test_directory_finish(&integration_installation, &source_test_files, failed);
    int result = source_test_case_teardown(test, failed || installation != file_error_ok);
    return result || installation != file_error_ok;
}
#undef TEST_SUITE_SETUP
#undef TEST_SUITE_TEARDOWN
#undef TEST_TEARDOWN
#define TEST_SUITE_SETUP integration_suite_setup
#define TEST_SUITE_TEARDOWN integration_suite_teardown
#define TEST_TEARDOWN integration_case_teardown
static void integration_check(const char *name) { ++integration_groups; fprintf(stderr, "check:%s\n", name); }

/* Strict bounded JSON tree for CLI responses; never put Attendant protocol in
   reusable headers. Two independent documents support semantic comparisons. */
typedef struct integration_json_node {
    enum pdjson_type type;
    const char *name, *text;
    int first, last, next, count;
} integration_json_node;
typedef struct integration_json_document {
    integration_json_node nodes[4096];
    int count;
    arena *memory;
} integration_json_document;
static u8 integration_json_memory[2][2097152];
static integration_json_document integration_documents[2];
static void *integration_json_allocate(size_t size, void *opaque) {
    if (size > SIZE_MAX - 16) return NULL;
    arena_result allocation = arena_alloc(opaque, size + 16, 16);
    if (allocation.error != arena_error_ok) return NULL;
    *(size_t *)allocation.ptr = size;
    return (u8 *)allocation.ptr + 16;
}
static void *integration_json_reallocate(void *old, size_t size, void *opaque) {
    void *next = integration_json_allocate(size, opaque);
    if (next && old) { size_t prior = *(size_t *)((u8 *)old - 16); memcpy(next, old, prior < size ? prior : size); }
    return next;
}
static void integration_json_release(void *memory, size_t size, void *opaque) { (void)memory; (void)size; (void)opaque; }
static const char *integration_json_copy(integration_json_document *document, const char *text, usize size) {
    if (!text || !size || text[size - 1] || memchr(text, 0, size - 1)) return NULL;
    arena_result allocation = arena_alloc(document->memory, size, 1);
    if (allocation.error != arena_error_ok) return NULL;
    memcpy(allocation.ptr, text, size); return allocation.ptr;
}
static int integration_json_value(integration_json_document *document, pdjson_stream *parser, unsigned depth, const char *name) {
    if (depth > 16 || document->count == 4096) return -1;
    int index = document->count++;
    integration_json_node *node = &document->nodes[index];
    *node = (integration_json_node){pdjson_next(parser), name, NULL, -1, -1, -1, 0};
    if (node->type == PDJSON_OBJECT || node->type == PDJSON_ARRAY) {
        enum pdjson_type ending = node->type == PDJSON_OBJECT ? PDJSON_OBJECT_END : PDJSON_ARRAY_END;
        while (pdjson_peek(parser) != ending) {
            const char *key = NULL;
            if (node->type == PDJSON_OBJECT) {
                if (pdjson_next(parser) != PDJSON_NAME) return -1;
                size_t size; const char *text = pdjson_get_name(parser, &size);
                key = integration_json_copy(document, text, size); if (!key) return -1;
                for (int prior = node->first; prior >= 0; prior = document->nodes[prior].next)
                    if (!strcmp(key, document->nodes[prior].name)) return -1;
            }
            int child = integration_json_value(document, parser, depth + 1, key);
            if (child < 0) return -1;
            if (node->last >= 0) document->nodes[node->last].next = child; else node->first = child;
            node->last = child; ++node->count;
        }
        if (pdjson_next(parser) != ending) return -1;
    } else if (node->type == PDJSON_STRING || node->type == PDJSON_NUMBER) {
        size_t size; const char *text = pdjson_get_value(parser, &size);
        node->text = integration_json_copy(document, text, size); if (!node->text) return -1;
    } else if (node->type != PDJSON_TRUE && node->type != PDJSON_FALSE && node->type != PDJSON_NULL) return -1;
    return index;
}
static int integration_json_parse(unsigned slot, const void *bytes, usize size) {
    if (slot >= 2 || (!bytes && size) || (size && memchr(bytes, 0, size)) || !string_view_utf8_valid(string_view_from_raw((u8 *)bytes, size))) return 1;
    integration_json_document *document = &integration_documents[slot];
    document->count = 0; document->memory = arena_from_buffer(integration_json_memory[slot], sizeof(integration_json_memory[slot]));
    pdjson_stream parser; pdjson_open_buffer(&parser, bytes, size);
    pdjson_allocator allocator = {integration_json_allocate, integration_json_reallocate, integration_json_release};
    pdjson_set_allocator(&parser, &allocator, document->memory);
    pdjson_set_language(&parser, PDJSON_LANGUAGE_JSON); pdjson_set_streaming(&parser, false);
    int index = integration_json_value(document, &parser, 0, NULL);
    int ok = index == 0 && pdjson_next(&parser) == PDJSON_DONE;
    if (!ok) { const char *problem = pdjson_get_error(&parser); fprintf(stderr, "Invalid/beyond-capacity command JSON: %s\n", problem ? problem : "tree/shape limit"); }
    pdjson_close(&parser); return !ok;
}
static integration_json_node *integration_json_get(unsigned slot, const char *path) {
    integration_json_document *document = &integration_documents[slot];
    if (!document->count) return NULL;
    integration_json_node *node = &document->nodes[0];
    while (path && *path) {
        const char *end = strchr(path, '.'); usize size = end ? (usize)(end - path) : strlen(path);
        int child = node->first;
        if (node->type == PDJSON_ARRAY) {
            usize index = 0;
            if (!size || size > 8) return NULL;
            for (usize i = 0; i < size; ++i) { if (path[i] < '0' || path[i] > '9') return NULL; index = index * 10 + (usize)(path[i] - '0'); }
            if (index >= (usize)node->count) return NULL;
            while (index--) child = document->nodes[child].next;
        } else if (node->type == PDJSON_OBJECT) {
            while (child >= 0) {
                const char *key = document->nodes[child].name;
                if (strlen(key) == size && !memcmp(key, path, size)) break;
                child = document->nodes[child].next;
            }
        } else return NULL;
        if (child < 0) return NULL;
        node = &document->nodes[child]; path = end ? end + 1 : NULL;
    }
    return node;
}
static const char *integration_text(const char *path) {
    integration_json_node *node = integration_json_get(0, path);
    return node && node->type == PDJSON_STRING ? node->text : NULL;
}
static int integration_text_is(const char *path, const char *text) {
    const char *actual = integration_text(path); return actual && text && !strcmp(actual, text);
}
static int integration_number_is(const char *path, long long integer) {
    integration_json_node *node = integration_json_get(0, path); value actual = {0}, expected = {0};
    expected.kind = VALUE_NUMBER; expected.number_kind = NUMBER_I64; expected.number.signed_integer = integer;
    return node && node->type == PDJSON_NUMBER && parse_number(node->text, &actual) == NUMBER_VALID && numbers_equal(actual, expected);
}
static int integration_count(const char *path) {
    integration_json_node *node = integration_json_get(0, path);
    return node && (node->type == PDJSON_ARRAY || node->type == PDJSON_OBJECT) ? node->count : -1;
}
static int integration_json_equal(integration_json_document *left, integration_json_node *a,
                                   integration_json_document *right, integration_json_node *b, unsigned depth) {
    if (!a || !b || depth > 16 || a->type != b->type) return 0;
    if (a->type == PDJSON_STRING) return !strcmp(a->text, b->text);
    if (a->type == PDJSON_NUMBER) {
        value first = {0}, second = {0};
        return parse_number(a->text, &first) == NUMBER_VALID && parse_number(b->text, &second) == NUMBER_VALID && numbers_equal(first, second);
    }
    if (a->count != b->count) return 0;
    int other = b->first;
    for (int child = a->first; child >= 0; child = left->nodes[child].next) {
        if (a->type == PDJSON_OBJECT) {
            other = b->first;
            while (other >= 0 && strcmp(left->nodes[child].name, right->nodes[other].name)) other = right->nodes[other].next;
        }
        if (other < 0 || !integration_json_equal(left, &left->nodes[child], right, &right->nodes[other], depth + 1)) return 0;
        other = right->nodes[other].next;
    }
    return 1;
}
static int integration_json_matches(const char *path, const char *expected, const char *expected_path) {
    if (integration_json_parse(1, expected, strlen(expected))) return 0;
    return integration_json_equal(&integration_documents[0], integration_json_get(0, path), &integration_documents[1], integration_json_get(1, expected_path), 0);
}

static int integration_execute(const test_command *command, unsigned long expected) {
    test_command_error error = test_command_run(&integration_commands, command, &integration_result);
    char stem[NATIVE_PATH_SIZE], path[NATIVE_PATH_SIZE];
    TEST_ASSERT_FILE_OK(test_directory_next(&source_test_case_directory, "command", stem));
    TEST_ASSERT(snprintf(path, sizeof(path), "%s.stdout", stem) < (int)sizeof(path));
    TEST_ASSERT_FILE_OK(file_create_bytes((cstring){path}, integration_result.output.data, integration_result.output.len));
    TEST_ASSERT(snprintf(path, sizeof(path), "%s.stderr", stem) < (int)sizeof(path));
    TEST_ASSERT_FILE_OK(file_create_bytes((cstring){path}, integration_result.error_output.data, integration_result.error_output.len));
    if (error != test_command_ok || !integration_result.exited || integration_result.exit_code != expected) {
        fprintf(stderr, "Command %s: %s, OS %lu, exit %lu, signal %d\n%.*s\n", command->arguments[0], test_command_error_message(error),
            integration_result.native_error, integration_result.exit_code, integration_result.signal_number,
            (int)(integration_result.error_output.len > 4096 ? 4096 : integration_result.error_output.len), integration_result.error_output.data);
    }
    TEST_ASSERT_COMMAND_OK(error); TEST_ASSERT_COMMAND_EXIT(&integration_result, expected); return 0;
}
static int integration_cli(const char *project, const char *const *arguments, usize count, unsigned long expected, const char *input) {
    TEST_ASSERT(count + 3 <= PROCESS_MAX_ARGUMENTS);
    const char *all[PROCESS_MAX_ARGUMENTS]; all[0] = integration_cli_path;
    for (usize i = 0; i < count; ++i) all[i + 1] = arguments[i];
    all[count + 1] = "-p"; all[count + 2] = project;
    test_command_environment environment = {"ATTENDANT_FEEDBACK_FAULT", ""};
    test_command command = {.arguments = all, .argument_count = count + 3, .environment = &environment, .environment_count = 1,
        .input = input, .input_size = input ? strlen(input) : 0, .timeout_ms = 30000};
    TEST_ASSERT(integration_execute(&command, expected) == 0);
    return integration_json_parse(0, integration_result.output.data, integration_result.output.len);
}
#define INTEGRATION_CLI(project, expected, ...) \
    do { const char *arguments[] = {__VA_ARGS__}; \
        TEST_ASSERT(integration_cli((project), arguments, sizeof(arguments) / sizeof(arguments[0]), (expected), NULL) == 0); } while (0)
#define INTEGRATION_STDIN(project, input, ...) \
    do { const char *arguments[] = {__VA_ARGS__}; \
        TEST_ASSERT(integration_cli((project), arguments, sizeof(arguments) / sizeof(arguments[0]), 0, (input)) == 0); } while (0)

/* Explicit one-level copy/filter. Bounded shared tree copy handles each subtree.
   Attendant cache/build exclusions stay here, not in base.h/test.h. */
static int integration_copy_entries(const char *source, const char *destination, const char *skip) {
    char checked[NATIVE_PATH_SIZE]; TEST_ASSERT_FILE_OK(file_tree_path((cstring){(char *)source}, checked, 0));
    file_directory directory; TEST_ASSERT(file_directory_open(&directory, source));
    int state = 0, ok = 1; usize visited = 0;
    while ((state = file_directory_read(&directory)) > 0) {
        if (++visited > 8192) { ok = 0; break; }
        const char *name = directory.name;
        if (!strcmp(name, ".") || !strcmp(name, "..") || (skip && !strcmp(name, skip))) continue;
        char from[NATIVE_PATH_SIZE], to[NATIVE_PATH_SIZE];
        if (file_tree_join(from, source, name) != file_error_ok || file_tree_join(to, destination, name) != file_error_ok || file_path_is_link(from)) { ok = 0; break; }
        if (file_is_directory(from, 0)) ok = file_directory_copy(&source_test_files, (cstring){from}, (cstring){to}) == file_error_ok;
        else ok = file_is_regular(from, 0) && file_copy(from, to, 1);
        if (!ok) break;
    }
    int closed = file_directory_close(&directory);
    TEST_ASSERT(ok && closed && state >= 0); return 0;
}
static int integration_missing(const char *path) {
    file_entry_kind kind; return file_native_kind(path, &kind) == file_error_not_found;
}
static int integration_project(char project[NATIVE_PATH_SIZE]) {
    TEST_ASSERT_FILE_OK(file_tree_join(project, source_test_case_root, "project café Ω"));
    TEST_ASSERT_FILE_OK(file_directory_copy(&source_test_files, (cstring){"tests/fixtures/project"}, (cstring){project})); return 0;
}
static int integration_snapshot(const char *project, int cache, char path[NATIVE_PATH_SIZE]) {
    TEST_ASSERT_FILE_OK(test_directory_next(&source_test_case_directory, "snapshot", path));
    TEST_ASSERT(file_directory_create(path, 1)); return integration_copy_entries(project, path, cache ? NULL : ".attendant");
}
static int integration_unchanged(const char *project, const char *baseline, int cache) {
    char current[NATIVE_PATH_SIZE]; TEST_ASSERT(integration_snapshot(project, cache, current) == 0);
    TEST_ASSERT_DIRECTORIES_EQUAL(&source_test_files, ((cstring){(char *)baseline}), ((cstring){current})); return 0;
}
static int integration_read(const char *path, char *text, usize capacity) {
    TEST_ASSERT(capacity > 0); usize size = 0;
    TEST_ASSERT_FILE_OK(file_read_into((cstring){(char *)path}, text, capacity - 1, &size));
    TEST_ASSERT(!memchr(text, 0, size)); text[size] = 0; return 0;
}
