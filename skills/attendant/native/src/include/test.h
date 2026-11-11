#ifndef TEST_H
#define TEST_H

/* C99 assertions, optional lifecycle/fixture helpers and sequential runner
   generation. Derived from proto-framework test.h with local extensions. */
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
static int test_assert_file_ends_with_text(cstring path, const char *suffix, const char *source, int line) {
    int equal = 0;
    file_error error = file_ends_with_text(path, suffix, &equal);
    return test_assert_file_comparison(error, equal, path.data, "expected text suffix", source, line);
}
static int test_assert_file_ends_with_bytes(cstring path, const void *suffix, usize size, const char *source, int line) {
    int equal = 0;
    file_error error = file_ends_with_bytes(path, suffix, size, &equal);
    return test_assert_file_comparison(error, equal, path.data, "expected byte suffix", source, line);
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

#define TEST_ASSERT_FILE_ENDS_WITH_TEXT(path, suffix)                                                \
    do { if (test_assert_file_ends_with_text((path), (suffix), __FILE__, __LINE__)) return 1; } while (0)
#define TEST_ASSERT_FILE_ENDS_WITH_BYTES(path, suffix, size)                                         \
    do { if (test_assert_file_ends_with_bytes((path), (suffix), (size), __FILE__, __LINE__)) return 1; } while (0)

/* Optional fixture policy. Zero-initialize; do not copy ownership or change path
   while owned. No project paths, environment variables or automatic registry.
   Caller invokes finish from teardown, including after partial setup. */
typedef struct test_directory {
    char path[NATIVE_PATH_SIZE];
    usize next_child;
    int owned;
} test_directory;

static file_error test_directory_create(test_directory *directory, cstring path) {
    if (!directory || directory->owned) return file_error_invalid;
    char absolute[NATIVE_PATH_SIZE];
    file_error error = file_tree_path(path, absolute, 1);
    if (error != file_error_ok) return error;
    file_entry_kind kind;
    error = file_native_kind(absolute, &kind);
    if (error == file_error_ok) return file_error_exists;
    if (error != file_error_not_found) return error;
    if (!file_directory_create(absolute, 1)) return file_error_open_failed;
    memcpy(directory->path, absolute, strlen(absolute) + 1);
    directory->next_child = 0;
    directory->owned = 1;
    return file_error_ok;
}

/* Reserve numbered path, not directory. Caller creates/copies exclusively.
   Output must not overlap directory storage. No collision retry or adoption. */
static file_error test_directory_next(test_directory *directory, const char *label, char output[NATIVE_PATH_SIZE]) {
    if (!directory || !directory->owned || !output || !label || !label[0] ||
        !strcmp(label, ".") || !strcmp(label, "..") || strpbrk(label, "/\\:")) return file_error_invalid;
    if (directory->next_child == SIZE_MAX) return file_error_capacity;
    char leaf[NATIVE_PATH_SIZE];
    usize next = directory->next_child + 1;
    int count = snprintf(leaf, sizeof(leaf), "%s-%03zu", label, next);
    if (count < 0 || count >= (int)sizeof(leaf)) return file_error_capacity;
    file_error error = file_tree_join(output, directory->path, leaf);
    if (error == file_error_ok) directory->next_child = next;
    return error;
}

/* Finish relinquishes ownership, including retained/failed cleanup paths, so
   caller can start next case. Repeated finish never deletes retained evidence.
   Failed cleanup is reported and returned; it must fail the case/suite hook. */
static file_error test_directory_finish(test_directory *directory, file_tree_workspace *files, int failed) {
    if (!directory) return file_error_invalid;
    if (!directory->owned) return file_error_ok;
    file_error error = failed ? file_error_ok : file_directory_remove_tree(files, (cstring){directory->path});
    if (failed || error != file_error_ok) {
        fprintf(stderr, "Retained test directory: %s%s%s\n", directory->path,
                error == file_error_ok ? "" : ": ", error == file_error_ok ? "" : file_error_message(error));
    }
    directory->owned = 0;
    return error;
}

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

/* Test-owned commands, not a general application process API.
   No shell. Start borrows arguments/input/scratch until return; live state and
   captures stay at stable, exclusive addresses until completion. Do not copy live
   state. Caller must poll all active children; no background deadline watchdog.
   Capacities include NUL; byte lengths do not. No silent truncation.
   Parent environment/cwd/signal policy stays unchanged. Windows owns a job,
   waits direct child and verifies job active count zero; descendant process
   objects may signal later. POSIX owns a process group (detached descendants
   are outside contract).
   POSIX requires default SIGCHLD, no competing reaper, and modern libc's spawn
   chdir action. Caller coordinates concurrent environment/cwd/signal changes. */
typedef struct test_command_environment { const char *name, *value; } test_command_environment;
typedef struct test_command {
    const char *const *arguments;
    usize argument_count;
    const char *working_directory;
    const test_command_environment *environment;
    usize environment_count;
    const void *input;
    usize input_size;
    u32 timeout_ms;
} test_command;
typedef struct test_command_workspace {
    arena *scratch;
    char *output, *error_output;
    usize output_capacity, error_capacity;
} test_command_workspace;
typedef enum test_command_error {
    test_command_ok,
    test_command_invalid,
    test_command_launch_failed,
    test_command_io_failed,
    test_command_timeout,
    test_command_capacity,
    test_command_cleanup_failed,
    test_command_cancelled,
} test_command_error;
typedef struct test_command_result {
    test_command_error error;
    unsigned long native_error, exit_code;
    int exited, signal_number, cleanup_failed;
    string_view output, error_output;
} test_command_result;
/* Zero-initialize before first start. Completed state may be reused. Finish is
   idempotent; always call it on paths that leave a child active. A poll performs
   at most one 16 KiB read per stream, plus bounded terminal cleanup. */
typedef struct test_command_live {
    test_command_workspace workspace;
    test_command_result result;
    u64 start;
    u32 timeout_ms;
    int active, complete, observed, output_done, error_done;
#if defined(_WIN32)
    HANDLE process, job, output_read, error_read;
    int assigned;
#else
    pid_t process;
    int output_read, error_read;
#endif
} test_command_live;
enum { TEST_COMMAND_MAX_ENVIRONMENT = 4096, TEST_COMMAND_CLEANUP_MS = 1000 };

static const char *test_command_error_message(test_command_error error) {
    switch (error) {
    case test_command_ok: return "ok";
    case test_command_invalid: return "invalid command/workspace";
    case test_command_launch_failed: return "command launch failed";
    case test_command_io_failed: return "command I/O failed";
    case test_command_timeout: return "command deadline exceeded";
    case test_command_capacity: return "command/capture capacity exhausted";
    case test_command_cleanup_failed: return "command cleanup failed";
    case test_command_cancelled: return "command cancelled";
    }
    return "unknown command failure";
}
static void test_command_fail(test_command_result *result, test_command_error error, unsigned long native) {
    if (result->error == test_command_ok) { result->error = error; result->native_error = native; }
    if (error == test_command_cleanup_failed) result->cleanup_failed = 1;
}
static int test_command_clock(u64 *milliseconds) {
#if defined(_WIN32)
    LARGE_INTEGER counter, frequency;
    if (!QueryPerformanceCounter(&counter) || !QueryPerformanceFrequency(&frequency) || frequency.QuadPart <= 0) return 0;
    *milliseconds = (u64)(counter.QuadPart / frequency.QuadPart) * 1000 +
                    (u64)(counter.QuadPart % frequency.QuadPart) * 1000 / (u64)frequency.QuadPart;
#else
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now)) return 0;
    *milliseconds = (u64)now.tv_sec * 1000 + (u64)now.tv_nsec / 1000000;
#endif
    return 1;
}
static int test_command_deadline(test_command_result *result, u64 start, u32 timeout) {
    u64 now;
    if (!test_command_clock(&now)) test_command_fail(result, test_command_io_failed, 0);
    else if (now - start >= timeout) test_command_fail(result, test_command_timeout, 0);
    return result->error == test_command_ok;
}
static void test_command_sleep(unsigned milliseconds) {
#if defined(_WIN32)
    Sleep(milliseconds);
#else
    struct timespec delay = {milliseconds / 1000, (long)(milliseconds % 1000) * 1000000};
    (void)nanosleep(&delay, NULL);
#endif
}
static int test_command_absolute(const char *path) {
#if defined(_WIN32)
    return path[0] == '/' || path[0] == '\\' || (strlen(path) >= 3 && path[1] == ':');
#else
    return path[0] == '/';
#endif
}
static test_command_error test_command_path(const char *path, const char *cwd, char output[NATIVE_PATH_SIZE]) {
#if defined(_WIN32)
    const char *forms[] = {path, cwd};
    for (unsigned i = 0; i < 2; ++i) {
        const char *text = forms[i];
        if (strlen(text) >= 4 && (text[0] == '/' || text[0] == '\\') && (text[1] == '/' || text[1] == '\\') &&
            (text[2] == '?' || text[2] == '.') && (text[3] == '/' || text[3] == '\\')) return test_command_invalid;
    }
    if (path[0] && path[1] == ':' && path[2] != '/' && path[2] != '\\') return test_command_invalid;
    if ((path[0] == '/' || path[0] == '\\') && path[1] != '/' && path[1] != '\\') {
        usize root = 0;
        if (cwd[0] && cwd[1] == ':') root = 2;
        else if ((cwd[0] == '/' || cwd[0] == '\\') && (cwd[1] == '/' || cwd[1] == '\\')) {
            const char *server = strpbrk(cwd + 2, "/\\");
            if (server && server[1]) { const char *share = strpbrk(server + 1, "/\\"); root = share ? (usize)(share - cwd) : strlen(cwd); }
        }
        if (!root) return test_command_invalid;
        int count = snprintf(output, NATIVE_PATH_SIZE, "%.*s%s", (int)root, cwd, path);
        return count < 0 || count >= NATIVE_PATH_SIZE ? test_command_capacity : test_command_ok;
    }
#endif
    if (test_command_absolute(path)) {
        int count = snprintf(output, NATIVE_PATH_SIZE, "%s", path);
        return count < 0 || count >= NATIVE_PATH_SIZE ? test_command_capacity : test_command_ok;
    }
    return file_tree_join(output, cwd, path) == file_error_ok ? test_command_ok : test_command_capacity;
}
/* Bare executables use effective child PATH, not an implicit shell search.
   Relative PATH components and executable paths resolve against child cwd. */
static test_command_error test_command_executable(const char *argument, const char *cwd, const char *path,
                                                  char output[NATIVE_PATH_SIZE]) {
    if (strchr(argument, '/')
#if defined(_WIN32)
        || strchr(argument, '\\') || strchr(argument, ':')
#endif
    ) {
        return test_command_path(argument, cwd, output);
    }
#if defined(_WIN32)
    const char separator = ';';
    if (!path) path = "";
#else
    const char separator = ':';
    if (!path) path = "/bin:/usr/bin";
#endif
    const char *part = path;
    for (;;) {
        const char *end = strchr(part, separator);
        usize size = end ? (usize)(end - part) : strlen(part);
        char directory[NATIVE_PATH_SIZE], absolute[NATIVE_PATH_SIZE], candidate[NATIVE_PATH_SIZE];
#if defined(_WIN32)
        if (size >= 2 && part[0] == '"' && part[size - 1] == '"') { ++part; size -= 2; }
#endif
        if (size >= sizeof(directory)) return test_command_capacity;
        memcpy(directory, part, size); directory[size] = 0;
        const char *base = directory;
        if (!size) base = cwd;
        else {
            test_command_error error = test_command_path(directory, cwd, absolute);
            if (error != test_command_ok) return error;
            base = absolute;
        }
        if (file_tree_join(candidate, base, argument) != file_error_ok) return test_command_capacity;
#if defined(_WIN32)
        const char *extensions[] = {"", ".exe", ".com"};
        for (usize i = 0; i < (strchr(argument, '.') ? 1u : 3u); ++i) {
            int count = snprintf(output, NATIVE_PATH_SIZE, "%s%s", candidate, extensions[i]);
            if (count < 0 || count >= NATIVE_PATH_SIZE) return test_command_capacity;
            if (file_is_regular(output, 1)) return test_command_ok;
        }
#else
        if (file_is_regular(candidate, 1) && access(candidate, X_OK) == 0) {
            memcpy(output, candidate, strlen(candidate) + 1); return test_command_ok;
        }
#endif
        if (!end) break;
        part = end + 1;
    }
    return test_command_launch_failed;
}

#if defined(_WIN32)
static wchar_t *test_command_wide(arena *scratch, const char *text, test_command_result *result) {
    wchar_t *wide = NULL;
    environment_error error = environment_wide(scratch, text, &wide);
    if (error != environment_error_ok)
        test_command_fail(result, error == environment_error_capacity ? test_command_capacity : test_command_invalid, 0);
    return wide;
}
static int test_command_quote(const test_command *command, char *output, usize capacity, usize *length) {
    usize position = 0;
    for (usize i = 0; i < command->argument_count; ++i) {
        const char *text = command->arguments[i];
        if (position + 3 >= capacity) return 0;
        if (i) { if (output) output[position] = ' '; ++position; }
        if (output) output[position] = '"'; ++position;
        usize slashes = 0;
        for (;;) {
            char byte = *text++;
            if (byte == '\\') { if (++slashes >= capacity) return 0; continue; }
            usize repeat = byte == '"' || !byte ? slashes * 2 : slashes;
            if (position + repeat + 3 >= capacity) return 0;
            while (repeat--) { if (output) output[position] = '\\'; ++position; }
            slashes = 0;
            if (!byte) break;
            if (byte == '"') { if (output) output[position] = '\\'; ++position; }
            if (output) output[position] = byte; ++position;
        }
        if (output) output[position] = '"'; ++position;
    }
    if (output) output[position] = 0;
    *length = position;
    return 1;
}
typedef int (WINAPI *test_command_ordinal)(LPCWSTR, int, LPCWSTR, int, BOOL);
static int test_command_environment_name(const wchar_t *entry, const wchar_t *name, test_command_ordinal compare) {
    const wchar_t *equal = wcschr(entry + (entry[0] == '='), '=');
    if (!equal || (usize)(equal - entry) != wcslen(name)) return 0;
    return compare(entry, (int)(equal - entry), name, -1, TRUE) == CSTR_EQUAL;
}
static wchar_t *test_command_windows_environment(test_command_workspace *workspace, const test_command *command,
                                                 test_command_result *result, char **path) {
    wchar_t *inherited = GetEnvironmentStringsW();
    if (!inherited) { test_command_fail(result, test_command_io_failed, GetLastError()); return NULL; }
    usize count = 0;
    for (wchar_t *p = inherited; *p; p += wcslen(p) + 1) {
        if (++count > TEST_COMMAND_MAX_ENVIRONMENT) break;
    }
    if (count + command->environment_count > TEST_COMMAND_MAX_ENVIRONMENT) {
        FreeEnvironmentStringsW(inherited); test_command_fail(result, test_command_capacity, 0); return NULL;
    }
    arena_result allocation = arena_alloc(workspace->scratch,
        (count + command->environment_count + 1) * sizeof(wchar_t *), _Alignof(wchar_t *));
    wchar_t **entries = allocation.ptr;
    if (allocation.error != arena_error_ok) {
        FreeEnvironmentStringsW(inherited); test_command_fail(result, test_command_capacity, 0); return NULL;
    }
    test_command_ordinal compare = (test_command_ordinal)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "CompareStringOrdinal");
    if (!compare) { FreeEnvironmentStringsW(inherited); test_command_fail(result, test_command_launch_failed, 0); return NULL; }
    allocation = arena_alloc(workspace->scratch, (command->environment_count * 2 + 1) * sizeof(wchar_t *), _Alignof(wchar_t *));
    if (allocation.error != arena_error_ok) {
        FreeEnvironmentStringsW(inherited); test_command_fail(result, test_command_capacity, 0); return NULL;
    }
    wchar_t **names = allocation.ptr, **values = names + command->environment_count;
    for (usize i = 0; i < command->environment_count && result->error == test_command_ok; ++i) {
        names[i] = test_command_wide(workspace->scratch, command->environment[i].name, result);
        values[i] = command->environment[i].value ? test_command_wide(workspace->scratch, command->environment[i].value, result) : NULL;
        for (usize j = 0; names[i] && j < i; ++j)
            if (compare(names[i], -1, names[j], -1, TRUE) == CSTR_EQUAL) test_command_fail(result, test_command_invalid, 0);
    }
    usize kept = 0;
    for (wchar_t *p = inherited; *p && result->error == test_command_ok; p += wcslen(p) + 1) {
        int overridden = 0;
        for (usize i = 0; i < command->environment_count; ++i) {
            if (test_command_environment_name(p, names[i], compare)) { overridden = 1; break; }
        }
        if (!overridden) entries[kept++] = p;
    }
    for (usize i = 0; i < command->environment_count && result->error == test_command_ok; ++i) {
        const test_command_environment *entry = &command->environment[i];
        wchar_t *name = names[i], *value = values[i];
        if (result->error != test_command_ok) break;
        if (!entry->value) continue;
        usize units = wcslen(name) + wcslen(value) + 2;
        allocation = arena_alloc(workspace->scratch, units * sizeof(wchar_t), _Alignof(wchar_t));
        if (allocation.error != arena_error_ok) { test_command_fail(result, test_command_capacity, 0); break; }
        wchar_t *text = allocation.ptr;
        memcpy(text, name, wcslen(name) * sizeof(wchar_t));
        text[wcslen(name)] = '=';
        memcpy(text + wcslen(name) + 1, value, (wcslen(value) + 1) * sizeof(wchar_t));
        entries[kept++] = text;
    }
    /* Small, explicitly bounded environment; no library sort allocation. */
    for (usize i = 1; i < kept; ++i) {
        wchar_t *entry = entries[i]; usize j = i;
        while (j && compare(entries[j - 1], -1, entry, -1, TRUE) == CSTR_GREATER_THAN) { entries[j] = entries[j - 1]; --j; }
        entries[j] = entry;
    }
    usize units = 1;
    for (usize i = 0; i < kept; ++i) units += wcslen(entries[i]) + 1;
    if (!kept) ++units;
    allocation = arena_alloc(workspace->scratch, units * sizeof(wchar_t), _Alignof(wchar_t));
    wchar_t *block = allocation.ptr;
    if (allocation.error != arena_error_ok) test_command_fail(result, test_command_capacity, 0);
    if (result->error == test_command_ok) {
        usize offset = 0;
        for (usize i = 0; i < kept; ++i) {
            usize size = wcslen(entries[i]) + 1;
            memcpy(block + offset, entries[i], size * sizeof(wchar_t)); offset += size;
        }
        for (wchar_t *p = block; *p; p += wcslen(p) + 1) if (test_command_environment_name(p, L"PATH", compare)) {
            const wchar_t *value = wcschr(p, '=') + 1;
            int bytes = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value, -1, NULL, 0, NULL, NULL);
            if (!bytes) { test_command_fail(result, test_command_invalid, 0); break; }
            allocation = arena_alloc(workspace->scratch, (usize)bytes, 1);
            if (allocation.error != arena_error_ok) { test_command_fail(result, test_command_capacity, 0); break; }
            *path = allocation.ptr;
            if (!WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value, -1, *path, bytes, NULL, NULL))
                test_command_fail(result, test_command_invalid, GetLastError());
            break;
        }
    }
    if (!FreeEnvironmentStringsW(inherited)) test_command_fail(result, test_command_cleanup_failed, GetLastError());
    return result->error == test_command_ok ? block : NULL;
}
static int test_command_windows_read(HANDLE pipe, char *output, usize capacity, string_view *view,
                                     test_command_result *result, int *done) {
    DWORD available = 0;
    if (!PeekNamedPipe(pipe, NULL, 0, NULL, &available, NULL)) {
        if (GetLastError() == ERROR_BROKEN_PIPE) { *done = 1; return 0; }
        test_command_fail(result, test_command_io_failed, GetLastError()); return 0;
    }
    if (!available) return 0;
    usize room = capacity - 1 - view->len;
    DWORD wanted = (DWORD)(room < available ? room : available);
    if (wanted > 16384) wanted = 16384;
    DWORD read = 0;
    if (wanted && (!ReadFile(pipe, output + view->len, wanted, &read, NULL) || read != wanted)) {
        test_command_fail(result, test_command_io_failed, GetLastError()); return 0;
    }
    view->len += read; output[view->len] = 0;
    if ((usize)available > room) test_command_fail(result, test_command_capacity, 0);
    return read != 0;
}
static void test_command_windows_finish(test_command_live *live) {
    test_command_result *result = &live->result;
    if (live->process) {
        if (live->assigned) { if (!TerminateJobObject(live->job, 1)) test_command_fail(result, test_command_cleanup_failed, GetLastError()); }
        else if (!TerminateProcess(live->process, 1)) test_command_fail(result, test_command_cleanup_failed, GetLastError());
        if (WaitForSingleObject(live->process, TEST_COMMAND_CLEANUP_MS) != WAIT_OBJECT_0)
            test_command_fail(result, test_command_cleanup_failed, GetLastError());
        else if (!result->exited) {
            DWORD code;
            if (!GetExitCodeProcess(live->process, &code)) test_command_fail(result, test_command_cleanup_failed, GetLastError());
            else { result->exited = 1; result->exit_code = code; }
        }
        if (live->assigned) {
            int empty = 0;
            for (unsigned attempt = 0; attempt <= TEST_COMMAND_CLEANUP_MS; ++attempt) {
                JOBOBJECT_BASIC_ACCOUNTING_INFORMATION information = {0};
                if (!QueryInformationJobObject(live->job, JobObjectBasicAccountingInformation, &information, sizeof(information), NULL)) {
                    test_command_fail(result, test_command_cleanup_failed, GetLastError()); break;
                }
                if (!information.ActiveProcesses) { empty = 1; break; }
                test_command_sleep(1);
            }
            if (!empty) test_command_fail(result, test_command_cleanup_failed, WAIT_TIMEOUT);
        }
    }
    HANDLE owned[] = {live->process, live->job, live->output_read, live->error_read};
    for (usize i = 0; i < sizeof(owned) / sizeof(owned[0]); ++i)
        if (owned[i] && !CloseHandle(owned[i])) test_command_fail(result, test_command_cleanup_failed, GetLastError());
    live->process = live->job = live->output_read = live->error_read = NULL;
}
static void test_command_windows_start(test_command_live *live, const test_command *command) {
    test_command_workspace *workspace = &live->workspace;
    test_command_result *result = &live->result;
    u64 start = live->start;
    HANDLE input = INVALID_HANDLE_VALUE, output_read = NULL, output_write = NULL, error_read = NULL, error_write = NULL;
    HANDLE job = NULL; PROCESS_INFORMATION child = {0};
    SECURITY_ATTRIBUTES security = {sizeof(security), NULL, TRUE};
    typedef BOOL (WINAPI *test_attribute_initialize)(void *, DWORD, DWORD, SIZE_T *);
    typedef BOOL (WINAPI *test_attribute_update)(void *, DWORD, DWORD_PTR, void *, SIZE_T, void *, SIZE_T *);
    typedef VOID (WINAPI *test_attribute_delete)(void *);
    test_attribute_delete delete_attributes = NULL;
    struct { STARTUPINFOW startup; void *attributes; } startup = {0};
    int attributes_ready = 0, assigned = 0;
    wchar_t cwd_wide[NATIVE_PATH_SIZE], cwd_input[NATIVE_PATH_SIZE]; char cwd[NATIVE_PATH_SIZE], executable[NATIVE_PATH_SIZE];
    if (command->working_directory) {
        if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, command->working_directory, -1, cwd_input, NATIVE_PATH_SIZE)) {
            DWORD error = GetLastError();
            test_command_fail(result, error == ERROR_INSUFFICIENT_BUFFER ? test_command_capacity : test_command_invalid, error); goto cleanup;
        }
        DWORD size = GetFullPathNameW(cwd_input, NATIVE_PATH_SIZE, cwd_wide, NULL);
        if (!size || size >= NATIVE_PATH_SIZE) { test_command_fail(result, size ? test_command_capacity : test_command_launch_failed, GetLastError()); goto cleanup; }
    } else {
        DWORD count = GetCurrentDirectoryW(NATIVE_PATH_SIZE, cwd_wide);
        if (!count || count >= NATIVE_PATH_SIZE) {
            test_command_fail(result, count ? test_command_capacity : test_command_io_failed, GetLastError()); goto cleanup;
        }
    }
    if (!WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, cwd_wide, -1, cwd, sizeof(cwd), NULL, NULL)) {
        test_command_fail(result, test_command_capacity, GetLastError()); goto cleanup;
    }
    test_command_error cwd_error = test_command_path(cwd, cwd, executable);
    if (cwd_error != test_command_ok) { test_command_fail(result, cwd_error, 0); goto cleanup; }
    char *path = NULL;
    wchar_t *environment = test_command_windows_environment(workspace, command, result, &path);
    if (!environment) goto cleanup;
    test_command_error resolved = test_command_executable(command->arguments[0], cwd, path, executable);
    if (resolved != test_command_ok) { test_command_fail(result, resolved, ERROR_FILE_NOT_FOUND); goto cleanup; }
    wchar_t *application = test_command_wide(workspace->scratch, executable, result);
    usize length = 0;
    if (!test_command_quote(command, NULL, PROCESS_COMMAND_SIZE, &length)) {
        test_command_fail(result, test_command_capacity, 0); goto cleanup;
    }
    arena_result allocation = arena_alloc(workspace->scratch, length + 4, 1);
    if (allocation.error != arena_error_ok || !test_command_quote(command, allocation.ptr, length + 4, &length)) {
        test_command_fail(result, test_command_capacity, 0); goto cleanup;
    }
    wchar_t *line = test_command_wide(workspace->scratch, allocation.ptr, result);
    if (!application || !line) goto cleanup;
    if (command->input_size) {
        wchar_t temporary[NATIVE_PATH_SIZE], name[NATIVE_PATH_SIZE];
        DWORD count = GetTempPathW(NATIVE_PATH_SIZE, temporary);
        if (!count || count >= NATIVE_PATH_SIZE || !GetTempFileNameW(temporary, L"tst", 0, name)) {
            test_command_fail(result, test_command_io_failed, GetLastError()); goto cleanup;
        }
        input = CreateFileW(name, GENERIC_READ | GENERIC_WRITE | DELETE, FILE_SHARE_READ | FILE_SHARE_DELETE,
                            &security, OPEN_EXISTING, FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE, NULL);
        if (input == INVALID_HANDLE_VALUE) {
            DWORD native = GetLastError();
            test_command_fail(result, test_command_io_failed, native);
            if (!DeleteFileW(name)) test_command_fail(result, test_command_cleanup_failed, GetLastError());
            goto cleanup;
        }
        for (usize offset = 0; offset < command->input_size;) {
            DWORD count = command->input_size - offset > 16384 ? 16384 : (DWORD)(command->input_size - offset), written;
            if (!WriteFile(input, (const u8 *)command->input + offset, count, &written, NULL) || written != count) {
                test_command_fail(result, test_command_io_failed, GetLastError()); goto cleanup;
            }
            offset += count;
        }
        LARGE_INTEGER zero = {0};
        if (!SetFilePointerEx(input, zero, NULL, FILE_BEGIN)) { test_command_fail(result, test_command_io_failed, GetLastError()); goto cleanup; }
    } else input = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, &security, OPEN_EXISTING, 0, NULL);
    if (input == INVALID_HANDLE_VALUE || !CreatePipe(&output_read, &output_write, &security, 0) ||
        !CreatePipe(&error_read, &error_write, &security, 0) ||
        !SetHandleInformation(output_read, HANDLE_FLAG_INHERIT, 0) || !SetHandleInformation(error_read, HANDLE_FLAG_INHERIT, 0)) {
        test_command_fail(result, test_command_io_failed, GetLastError()); goto cleanup;
    }
    job = CreateJobObjectW(NULL, NULL);
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits = {0};
    limits.BasicLimitInformation.LimitFlags = 0x00002000; /* JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE */
    if (!job || !SetInformationJobObject(job, JobObjectExtendedLimitInformation, &limits, sizeof(limits))) {
        test_command_fail(result, test_command_launch_failed, GetLastError()); goto cleanup;
    }
    HMODULE kernel = GetModuleHandleW(L"kernel32.dll");
    test_attribute_initialize initialize = (test_attribute_initialize)GetProcAddress(kernel, "InitializeProcThreadAttributeList");
    test_attribute_update update = (test_attribute_update)GetProcAddress(kernel, "UpdateProcThreadAttribute");
    delete_attributes = (test_attribute_delete)GetProcAddress(kernel, "DeleteProcThreadAttributeList");
    SIZE_T bytes = 0;
    if (!initialize || !update || !delete_attributes) { test_command_fail(result, test_command_launch_failed, 0); goto cleanup; }
    initialize(NULL, 1, 0, &bytes);
    allocation = arena_alloc(workspace->scratch, bytes, _Alignof(void *));
    if (allocation.error != arena_error_ok) { test_command_fail(result, test_command_capacity, 0); goto cleanup; }
    startup.attributes = allocation.ptr;
    if (!initialize(startup.attributes, 1, 0, &bytes)) { test_command_fail(result, test_command_launch_failed, GetLastError()); goto cleanup; }
    attributes_ready = 1;
    HANDLE handles[] = {input, output_write, error_write};
    if (!update(startup.attributes, 0, 0x00020002, handles, sizeof(handles), NULL, NULL)) {
        test_command_fail(result, test_command_launch_failed, GetLastError()); goto cleanup;
    }
    startup.startup.cb = sizeof(startup);
    startup.startup.dwFlags = STARTF_USESTDHANDLES;
    startup.startup.hStdInput = input; startup.startup.hStdOutput = output_write; startup.startup.hStdError = error_write;
    if (!test_command_deadline(result, start, command->timeout_ms)) goto cleanup;
    if (!CreateProcessW(application, line, NULL, NULL, TRUE,
                        CREATE_SUSPENDED | CREATE_NO_WINDOW | CREATE_UNICODE_ENVIRONMENT | 0x00080000,
                        environment, cwd_wide, &startup.startup, &child)) {
        test_command_fail(result, test_command_launch_failed, GetLastError()); goto cleanup;
    }
    if (!AssignProcessToJobObject(job, child.hProcess)) { test_command_fail(result, test_command_launch_failed, GetLastError()); goto cleanup; }
    assigned = 1;
    if (ResumeThread(child.hThread) == (DWORD)-1) { test_command_fail(result, test_command_launch_failed, GetLastError()); goto cleanup; }
    if (!CloseHandle(input)) test_command_fail(result, test_command_cleanup_failed, GetLastError()); input = INVALID_HANDLE_VALUE;
    if (!CloseHandle(output_write)) test_command_fail(result, test_command_cleanup_failed, GetLastError()); output_write = NULL;
    if (!CloseHandle(error_write)) test_command_fail(result, test_command_cleanup_failed, GetLastError()); error_write = NULL;
cleanup:
    if (attributes_ready) delete_attributes(startup.attributes);
    HANDLE temporary[] = {child.hThread, output_write, error_write, input};
    for (usize i = 0; i < sizeof(temporary) / sizeof(temporary[0]); ++i)
        if (temporary[i] && temporary[i] != INVALID_HANDLE_VALUE && !CloseHandle(temporary[i])) test_command_fail(result, test_command_cleanup_failed, GetLastError());
    live->process = child.hProcess; live->job = job; live->assigned = assigned;
    live->output_read = output_read; live->error_read = error_read;
    if (result->error != test_command_ok) test_command_windows_finish(live);
}
static void test_command_windows_poll(test_command_live *live) {
    test_command_workspace *workspace = &live->workspace;
    test_command_result *result = &live->result;
    if (!live->observed) {
        DWORD wait = WaitForSingleObject(live->process, 0);
        if (wait == WAIT_OBJECT_0) {
            DWORD code;
            if (!GetExitCodeProcess(live->process, &code)) test_command_fail(result, test_command_io_failed, GetLastError());
            else { result->exited = 1; result->exit_code = code; live->observed = 1; }
            if (!TerminateJobObject(live->job, 1)) test_command_fail(result, test_command_cleanup_failed, GetLastError());
        } else if (wait != WAIT_TIMEOUT) test_command_fail(result, test_command_io_failed, GetLastError());
    }
    if (!live->output_done && result->error == test_command_ok)
        test_command_windows_read(live->output_read, workspace->output, workspace->output_capacity, &result->output, result, &live->output_done);
    if (!live->error_done && result->error == test_command_ok)
        test_command_windows_read(live->error_read, workspace->error_output, workspace->error_capacity, &result->error_output, result, &live->error_done);
}
#else
#include <signal.h>
/* Darwin provides this action; modern Linux libc exposes it with GNU features. */
#if defined(__linux__)
extern int posix_spawn_file_actions_addchdir_np(posix_spawn_file_actions_t *, const char *);
#endif
static int test_command_descriptor(int descriptor) {
    int copy = fcntl(descriptor, F_DUPFD_CLOEXEC, 3);
    int saved = errno;
    if (close(descriptor) && copy >= 0) { close(copy); return -1; }
    errno = saved;
    return copy;
}
static int test_command_posix_read(int descriptor, char *output, usize capacity, string_view *view,
                                    test_command_result *result, int *done) {
    usize room = capacity - 1 - view->len;
    char excess;
    ssize_t count = read(descriptor, room ? output + view->len : &excess, room > 16384 ? 16384 : room ? room : 1);
    if (count < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) test_command_fail(result, test_command_io_failed, errno);
        return 0;
    }
    if (!count) { *done = 1; return 0; }
    if (!room) test_command_fail(result, test_command_capacity, 0);
    else { view->len += (usize)count; output[view->len] = 0; }
    return 1;
}
static void test_command_posix_finish(test_command_live *live) {
    test_command_result *result = &live->result;
    pid_t child = live->process;
    if (child) {
        /* Keep child unreaped through cancellation: its group ID cannot be reused. */
        if (kill(-child, SIGKILL) && errno != ESRCH) test_command_fail(result, test_command_cleanup_failed, errno);
        if (kill(child, SIGKILL) && errno != ESRCH) test_command_fail(result, test_command_cleanup_failed, errno);
        int status = 0, reaped = 0;
        for (unsigned attempt = 0; attempt <= TEST_COMMAND_CLEANUP_MS; ++attempt) {
            pid_t waited = waitpid(child, &status, WNOHANG);
            if (waited == child) { reaped = 1; break; }
            if (waited < 0 && errno != EINTR) break;
            test_command_sleep(1);
        }
        if (!reaped) test_command_fail(result, test_command_cleanup_failed, errno);
        else if (WIFEXITED(status)) { result->exited = 1; result->exit_code = WEXITSTATUS(status); }
        else if (WIFSIGNALED(status)) result->signal_number = WTERMSIG(status);
    }
    if (live->output_read >= 0 && close(live->output_read)) test_command_fail(result, test_command_cleanup_failed, errno);
    if (live->error_read >= 0 && close(live->error_read)) test_command_fail(result, test_command_cleanup_failed, errno);
    live->process = 0; live->output_read = live->error_read = -1;
}
static void test_command_posix_start(test_command_live *live, const test_command *command) {
    test_command_workspace *workspace = &live->workspace;
    test_command_result *result = &live->result;
    u64 start = live->start;
    extern char **environ;
    int descriptors[5] = {-1, -1, -1, -1, -1}; /* stdin, stdout read/write, stderr read/write */
    int actions_ready = 0, attributes_ready = 0;
    pid_t child = 0;
    posix_spawn_file_actions_t actions; posix_spawnattr_t attributes;
    char cwd[NATIVE_PATH_SIZE], resolved[PATH_MAX], executable[NATIVE_PATH_SIZE];
    struct sigaction disposition;
    if (sigaction(SIGCHLD, NULL, &disposition) || disposition.sa_handler != SIG_DFL || (disposition.sa_flags & SA_NOCLDWAIT)) {
        test_command_fail(result, test_command_invalid, 0); goto cleanup;
    }
    if (!(command->working_directory ? realpath(command->working_directory, resolved) : getcwd(resolved, sizeof(resolved)))) {
        test_command_fail(result, test_command_launch_failed, errno); goto cleanup;
    }
    if (strlen(resolved) >= sizeof(cwd)) { test_command_fail(result, test_command_capacity, 0); goto cleanup; }
    memcpy(cwd, resolved, strlen(resolved) + 1);
    usize count = 0;
    while (count < TEST_COMMAND_MAX_ENVIRONMENT && environ[count]) ++count;
    if ((count == TEST_COMMAND_MAX_ENVIRONMENT && environ[count]) || count + command->environment_count > TEST_COMMAND_MAX_ENVIRONMENT) { test_command_fail(result, test_command_capacity, 0); goto cleanup; }
    arena_result allocation = arena_alloc(workspace->scratch, (count + command->environment_count + 1) * sizeof(char *), _Alignof(char *));
    if (allocation.error != arena_error_ok) { test_command_fail(result, test_command_capacity, 0); goto cleanup; }
    char **environment = allocation.ptr; usize kept = 0;
    for (usize i = 0; i < count; ++i) {
        int overridden = 0;
        for (usize j = 0; j < command->environment_count; ++j) {
            usize size = strlen(command->environment[j].name);
            if (!strncmp(environ[i], command->environment[j].name, size) && environ[i][size] == '=') { overridden = 1; break; }
        }
        if (!overridden) environment[kept++] = environ[i];
    }
    for (usize i = 0; i < command->environment_count; ++i) if (command->environment[i].value) {
        usize name = strlen(command->environment[i].name), value = strlen(command->environment[i].value);
        if (name > SIZE_MAX - value - 2) { test_command_fail(result, test_command_capacity, 0); goto cleanup; }
        allocation = arena_alloc(workspace->scratch, name + value + 2, 1);
        if (allocation.error != arena_error_ok) { test_command_fail(result, test_command_capacity, 0); goto cleanup; }
        char *text = allocation.ptr;
        memcpy(text, command->environment[i].name, name); text[name] = '=';
        memcpy(text + name + 1, command->environment[i].value, value + 1);
        environment[kept++] = text;
    }
    environment[kept] = NULL;
    const char *path = NULL;
    for (usize i = 0; i < kept; ++i) if (!strncmp(environment[i], "PATH=", 5)) path = environment[i] + 5;
    test_command_error error = test_command_executable(command->arguments[0], cwd, path, executable);
    if (error != test_command_ok) { test_command_fail(result, error, ENOENT); goto cleanup; }
    FILE *input = tmpfile();
    if (!input) { test_command_fail(result, test_command_io_failed, errno); goto cleanup; }
    int written = !command->input_size || fwrite(command->input, 1, command->input_size, input) == command->input_size;
    if (fflush(input) || fseek(input, 0, SEEK_SET)) written = 0;
    descriptors[0] = written ? fcntl(fileno(input), F_DUPFD_CLOEXEC, 3) : -1;
    int closed = fclose(input) == 0;
    if (!written || !closed || descriptors[0] < 0) { test_command_fail(result, test_command_io_failed, errno); goto cleanup; }
    for (int i = 1; i < 5; i += 2) {
        int pair[2];
        if (pipe(pair)) { test_command_fail(result, test_command_io_failed, errno); goto cleanup; }
        descriptors[i] = test_command_descriptor(pair[0]); descriptors[i + 1] = test_command_descriptor(pair[1]);
        if (descriptors[i] < 0 || descriptors[i + 1] < 0 || fcntl(descriptors[i], F_SETFL, O_NONBLOCK)) {
            test_command_fail(result, test_command_io_failed, errno); goto cleanup;
        }
    }
    int native = posix_spawn_file_actions_init(&actions);
    if (native) { test_command_fail(result, test_command_launch_failed, native); goto cleanup; }
    actions_ready = 1;
    native = posix_spawnattr_init(&attributes);
    if (native) { test_command_fail(result, test_command_launch_failed, native); goto cleanup; }
    attributes_ready = 1;
    sigset_t empty, defaults; sigemptyset(&empty); sigfillset(&defaults); sigdelset(&defaults, SIGKILL); sigdelset(&defaults, SIGSTOP);
    short flags = POSIX_SPAWN_SETPGROUP | POSIX_SPAWN_SETSIGMASK | POSIX_SPAWN_SETSIGDEF;
#if defined(__APPLE__)
    flags |= POSIX_SPAWN_CLOEXEC_DEFAULT;
#endif
    if (!(native = posix_spawnattr_setflags(&attributes, flags))) native = posix_spawnattr_setpgroup(&attributes, 0);
    if (!native) native = posix_spawnattr_setsigmask(&attributes, &empty);
    if (!native) native = posix_spawnattr_setsigdefault(&attributes, &defaults);
    if (!native) native = posix_spawn_file_actions_addchdir_np(&actions, cwd);
    if (!native) native = posix_spawn_file_actions_adddup2(&actions, descriptors[0], STDIN_FILENO);
    if (!native) native = posix_spawn_file_actions_adddup2(&actions, descriptors[2], STDOUT_FILENO);
    if (!native) native = posix_spawn_file_actions_adddup2(&actions, descriptors[4], STDERR_FILENO);
    for (int i = 0; i < 5 && !native; ++i) native = posix_spawn_file_actions_addclose(&actions, descriptors[i]);
    char *arguments[PROCESS_MAX_ARGUMENTS + 1];
    for (usize i = 0; i < command->argument_count; ++i) arguments[i] = (char *)command->arguments[i];
    arguments[command->argument_count] = NULL;
    if (!test_command_deadline(result, start, command->timeout_ms)) goto cleanup;
    if (!native) native = posix_spawn(&child, executable, &actions, &attributes, arguments, environment);
    if (native) { child = 0; test_command_fail(result, test_command_launch_failed, native); goto cleanup; }
    for (int i = 0; i < 5; i += 2) {
        if (close(descriptors[i])) test_command_fail(result, test_command_cleanup_failed, errno);
        descriptors[i] = -1;
    }
cleanup:
    if (attributes_ready) { int error = posix_spawnattr_destroy(&attributes); if (error) test_command_fail(result, test_command_cleanup_failed, error); }
    if (actions_ready) { int error = posix_spawn_file_actions_destroy(&actions); if (error) test_command_fail(result, test_command_cleanup_failed, error); }
    for (int i = 0; i < 5; i += 2) if (descriptors[i] >= 0 && close(descriptors[i])) test_command_fail(result, test_command_cleanup_failed, errno);
    live->process = child; live->output_read = descriptors[1]; live->error_read = descriptors[3];
    if (result->error != test_command_ok) test_command_posix_finish(live);
}
static void test_command_posix_poll(test_command_live *live) {
    test_command_workspace *workspace = &live->workspace;
    test_command_result *result = &live->result;
    if (!live->observed) {
        siginfo_t info = {0};
        if (waitid(P_PID, (id_t)live->process, &info, WEXITED | WNOHANG | WNOWAIT)) {
            if (errno == ECHILD) { test_command_fail(result, test_command_cleanup_failed, errno); live->process = 0; }
            else if (errno != EINTR) test_command_fail(result, test_command_io_failed, errno);
        } else if (info.si_pid) {
            live->observed = 1;
            if (kill(-live->process, SIGKILL) && errno != ESRCH) test_command_fail(result, test_command_cleanup_failed, errno);
        }
    }
    if (!live->output_done && result->error == test_command_ok)
        test_command_posix_read(live->output_read, workspace->output, workspace->output_capacity, &result->output, result, &live->output_done);
    if (!live->error_done && result->error == test_command_ok)
        test_command_posix_read(live->error_read, workspace->error_output, workspace->error_capacity, &result->error_output, result, &live->error_done);
}
#endif

static int test_command_overlap(const void *first, usize first_size, const void *second, usize second_size) {
    uptr a = (uptr)first, b = (uptr)second;
    return first_size && second_size && (a <= b ? b - a < first_size : a - b < second_size);
}
static test_command_error test_command_start(test_command_live *live, test_command_workspace *workspace,
                                              const test_command *command) {
    if (!live || live->active) return test_command_invalid;
    memset(live, 0, sizeof(*live)); live->complete = 1;
    test_command_result *result = &live->result; result->error = test_command_invalid;
    if (!workspace || !workspace->scratch || !workspace->output || !workspace->error_output ||
        !workspace->output_capacity || !workspace->error_capacity || !command || !command->arguments ||
        !command->argument_count || command->argument_count > PROCESS_MAX_ARGUMENTS || !command->timeout_ms ||
        (command->input_size && !command->input) || (command->working_directory && !command->working_directory[0]) ||
        (command->environment_count && !command->environment) ||
        command->environment_count > TEST_COMMAND_MAX_ENVIRONMENT) return result->error;
    if (test_command_overlap(workspace->output, workspace->output_capacity, workspace->error_output, workspace->error_capacity) ||
        test_command_overlap(command->input, command->input_size, workspace->output, workspace->output_capacity) ||
        test_command_overlap(command->input, command->input_size, workspace->error_output, workspace->error_capacity)) return result->error;
    for (usize i = 0; i < command->argument_count; ++i) if (!command->arguments[i] || (!i && !command->arguments[i][0])) return result->error;
    for (usize i = 0; i < command->environment_count; ++i) {
        const char *name = command->environment[i].name;
        if (!name || !name[0] || strchr(name, '=')) return result->error;
        for (usize j = 0; j < i; ++j) {
#if defined(_WIN32)
            if (!_stricmp(name, command->environment[j].name)) return result->error;
#else
            if (!strcmp(name, command->environment[j].name)) return result->error;
#endif
        }
    }
    workspace->output[0] = workspace->error_output[0] = 0;
    result->output = string_view_from_cstr(workspace->output);
    result->error_output = string_view_from_cstr(workspace->error_output);
    result->error = test_command_ok;
    arena_position_marker mark = arena_position_mark(workspace->scratch);
    live->workspace = *workspace; live->timeout_ms = command->timeout_ms;
    if (!test_command_clock(&live->start)) test_command_fail(result, test_command_io_failed, 0);
    else {
#if defined(_WIN32)
        test_command_windows_start(live, command);
#else
        test_command_posix_start(live, command);
#endif
    }
    live->active = result->error == test_command_ok;
    live->complete = !live->active;
    (void)arena_pop_to_marker(mark);
    return result->error;
}
static void test_command_release(test_command_live *live) {
#if defined(_WIN32)
    test_command_windows_finish(live);
#else
    test_command_posix_finish(live);
#endif
    live->active = 0; live->complete = 1;
}
static test_command_error test_command_poll(test_command_live *live) {
    if (!live || (!live->active && !live->complete)) return test_command_invalid;
    if (!live->active) return live->result.error;
    if (test_command_deadline(&live->result, live->start, live->timeout_ms)) {
#if defined(_WIN32)
        test_command_windows_poll(live);
#else
        test_command_posix_poll(live);
#endif
        (void)test_command_deadline(&live->result, live->start, live->timeout_ms);
    }
    if (live->result.error != test_command_ok || (live->observed && live->output_done && live->error_done))
        test_command_release(live);
    return live->result.error;
}
static test_command_error test_command_finish(test_command_live *live) {
    if (!live) return test_command_invalid;
    if (live->active) {
        (void)test_command_poll(live);
        if (live->active) {
            test_command_fail(&live->result, test_command_cancelled, 0);
            test_command_release(live);
        }
    }
    return live->result.error;
}
static test_command_error test_command_run(test_command_workspace *workspace, const test_command *command,
                                            test_command_result *result) {
    if (!result) return test_command_invalid;
    test_command_live live = {0};
    (void)test_command_start(&live, workspace, command);
    while (!live.complete) {
        usize output = live.result.output.len, errors = live.result.error_output.len;
        (void)test_command_poll(&live);
        if (!live.complete && output == live.result.output.len && errors == live.result.error_output.len) test_command_sleep(1);
    }
    *result = live.result;
    return result->error;
}
static int test_assert_command_operation(test_command_error error, const char *operation, const char *source, int line) {
    if (error == test_command_ok) return 0;
    fprintf(stderr, "%s:%d: %s: %s\n", source, line, operation, test_command_error_message(error)); return 1;
}
static int test_assert_command_exit(const test_command_result *result, unsigned long expected, const char *source, int line) {
    if (result && result->error == test_command_ok && result->exited && !result->signal_number && result->exit_code == expected) return 0;
    fprintf(stderr, "%s:%d: expected command exit %lu; actual %lu, signal %d, %s (OS %lu)\n", source, line, expected,
            result ? result->exit_code : 0, result ? result->signal_number : 0,
            result ? test_command_error_message(result->error) : "null result", result ? result->native_error : 0);
    return 1;
}
static int test_assert_command_output(const test_command_result *result, const void *expected, usize size,
                                       int error_output, const char *source, int line) {
    string_view actual = {0};
    if (result) actual = error_output ? result->error_output : result->output;
    if (result && result->error == test_command_ok && (expected || !size) && actual.len == size &&
        (!size || !memcmp(actual.data, expected, size))) return 0;
    fprintf(stderr, "%s:%d: command %s differs or failed\n", source, line, error_output ? "stderr" : "stdout"); return 1;
}
static int test_assert_command_text(const test_command_result *result, const char *expected, int error_output, const char *source, int line) {
    if (!expected) { fprintf(stderr, "%s:%d: null expected command text\n", source, line); return 1; }
    return test_assert_command_output(result, expected, strlen(expected), error_output, source, line);
}
#define TEST_ASSERT_COMMAND_OK(operation) \
    do { if (test_assert_command_operation((operation), #operation, __FILE__, __LINE__)) return 1; } while (0)
#define TEST_ASSERT_COMMAND_EXIT(result, expected) \
    do { if (test_assert_command_exit((result), (expected), __FILE__, __LINE__)) return 1; } while (0)
#define TEST_ASSERT_COMMAND_OUTPUT_TEXT(result, expected) \
    do { if (test_assert_command_text((result), (expected), 0, __FILE__, __LINE__)) return 1; } while (0)
#define TEST_ASSERT_COMMAND_ERROR_TEXT(result, expected) \
    do { if (test_assert_command_text((result), (expected), 1, __FILE__, __LINE__)) return 1; } while (0)
#define TEST_ASSERT_COMMAND_OUTPUT_BYTES(result, expected, size) \
    do { if (test_assert_command_output((result), (expected), (size), 0, __FILE__, __LINE__)) return 1; } while (0)
#define TEST_ASSERT_COMMAND_ERROR_BYTES(result, expected, size) \
    do { if (test_assert_command_output((result), (expected), (size), 1, __FILE__, __LINE__)) return 1; } while (0)

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
