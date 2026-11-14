/* Native source/projection platform boundary. UTF-8 paths, bounded caller
 * buffers, explicit missing/error distinction. No application policy.
 * Times retain 100 ns ticks since 1601 for existing API/cache compatibility. */
#ifndef ATTENDANT_CORE_PLATFORM_H
#define ATTENDANT_CORE_PLATFORM_H
#include "../include/base.h"
#if defined(_WIN32) && !defined(WC_ERR_INVALID_CHARS)
#define WC_ERR_INVALID_CHARS 0x80
#endif
#if defined(__APPLE__)
#include <CommonCrypto/CommonDigest.h>
#include <locale.h>
#include <wctype.h>
#include <xlocale.h>
#endif

typedef u64 source_time;
#define SOURCE_UNIX_EPOCH 116444736000000000ULL
#define SOURCE_PATH_CAPACITY 4096

/* Caller-owned zeroed OS-backed buffer on supported Windows/macOS targets.
 * Reserve alone does not commit Windows pages. Keep arena reservation semantics. */
static void *native_memory_allocate(size_t size) {
    void *memory = arena_memory_reserve(size);
    if (memory && !arena_memory_commit(memory, size)) {
        arena_memory_release(memory, size); return NULL;
    }
    return memory;
}

typedef enum source_kind {
    SOURCE_MISSING, SOURCE_REGULAR, SOURCE_DIRECTORY, SOURCE_LINK, SOURCE_OTHER, SOURCE_ERROR
} source_kind;
typedef struct source_file_info {
    source_kind kind;
    u64 size;
    source_time modified, created;
    double mtime_ms;
    int read_only;
} source_file_info;
#if defined(_WIN32)
static int source_wide(const char *path, wchar_t *output) {
    return path && MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path, -1, output, SOURCE_PATH_CAPACITY);
}
static source_time source_filetime(FILETIME time) {
    return ((u64)time.dwHighDateTime << 32) | time.dwLowDateTime;
}
#endif
static source_file_info source_stat(const char *path) {
    source_file_info result = {0};
#if defined(_WIN32)
    WIN32_FILE_ATTRIBUTE_DATA status;
    wchar_t wide[SOURCE_PATH_CAPACITY];
    if (!source_wide(path, wide)) { result.kind = SOURCE_ERROR; return result; }
    if (!GetFileAttributesExW(wide, GetFileExInfoStandard, &status)) {
        DWORD error = GetLastError();
        result.kind = error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND ? SOURCE_MISSING : SOURCE_ERROR;
        return result;
    }
    result.kind = status.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT ? SOURCE_LINK :
        status.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? SOURCE_DIRECTORY : SOURCE_REGULAR;
    result.size = ((u64)status.nFileSizeHigh << 32) | status.nFileSizeLow;
    result.modified = source_filetime(status.ftLastWriteTime);
    result.created = source_filetime(status.ftCreationTime);
    u64 ticks = result.modified - SOURCE_UNIX_EPOCH;
    result.mtime_ms = (double)(ticks / 10000000ULL) * 1000.0 + (double)(ticks % 10000000ULL) / 10000.0;
    result.read_only = (status.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0;
#elif defined(__APPLE__)
    struct stat status;
    if (lstat(path, &status)) {
        result.kind = errno == ENOENT || errno == ENOTDIR ? SOURCE_MISSING : SOURCE_ERROR;
        return result;
    }
    result.kind = S_ISLNK(status.st_mode) ? SOURCE_LINK : S_ISDIR(status.st_mode) ? SOURCE_DIRECTORY :
        S_ISREG(status.st_mode) ? SOURCE_REGULAR : SOURCE_OTHER;
    result.size = status.st_size < 0 ? 0 : (u64)status.st_size;
    result.modified = SOURCE_UNIX_EPOCH + (u64)status.st_mtimespec.tv_sec * 10000000ULL + (u64)status.st_mtimespec.tv_nsec / 100;
    result.created = SOURCE_UNIX_EPOCH + (u64)status.st_birthtimespec.tv_sec * 10000000ULL + (u64)status.st_birthtimespec.tv_nsec / 100;
    result.mtime_ms = (double)status.st_mtimespec.tv_sec * 1000.0 + (double)status.st_mtimespec.tv_nsec / 1000000.0;
    result.read_only = !(status.st_mode & 0222) || access(path, W_OK) != 0;
#else
#error Unsupported Attendant platform
#endif
    return result;
}
/* Unknown/inaccessible paths are not safe to treat as absent. */
static int source_exists(const char *path) { return source_stat(path).kind != SOURCE_MISSING; }
static int source_case_compare(const char *left, const char *right) {
#if defined(_WIN32)
    return _stricmp(left, right);
#else
    return strcasecmp(left, right);
#endif
}
static int source_case_compare_n(const char *left, const char *right, size_t length) {
#if defined(_WIN32)
    return _strnicmp(left, right, length);
#else
    return strncasecmp(left, right, length);
#endif
}
static int source_path_compare(const char *left, const char *right) {
#if defined(_WIN32)
    wchar_t a[SOURCE_PATH_CAPACITY], b[SOURCE_PATH_CAPACITY];
    if (source_wide(left, a) && source_wide(right, b)) return lstrcmpiW(a, b);
#endif
    return strcmp(left, right);
}
static int source_remove(const char *path, int directory) {
#if defined(_WIN32)
    wchar_t wide[SOURCE_PATH_CAPACITY];
    if (!source_wide(path, wide)) return 0;
    if (directory ? RemoveDirectoryW(wide) : DeleteFileW(wide)) return 1;
    return GetLastError() == ERROR_FILE_NOT_FOUND || GetLastError() == ERROR_PATH_NOT_FOUND;
#else
    return (directory ? rmdir(path) : unlink(path)) == 0 || errno == ENOENT;
#endif
}
static int source_mkdir(const char *path) {
#if defined(_WIN32)
    wchar_t wide[SOURCE_PATH_CAPACITY];
    return source_wide(path, wide) && CreateDirectoryW(wide, NULL);
#else
    return mkdir(path, 0755) == 0;
#endif
}
typedef enum source_publication { SOURCE_CREATE, SOURCE_REPLACE } source_publication;
static int source_publish(const char *temporary, const char *target, source_publication mode) {
#if defined(_WIN32)
    wchar_t from[SOURCE_PATH_CAPACITY], to[SOURCE_PATH_CAPACITY];
    return source_wide(temporary, from) && source_wide(target, to) &&
        MoveFileExW(from, to, MOVEFILE_WRITE_THROUGH | (mode == SOURCE_REPLACE ? MOVEFILE_REPLACE_EXISTING : 0));
#else
    return renamex_np(temporary, target, mode == SOURCE_CREATE ? RENAME_EXCL : 0) == 0;
#endif
}
static void *source_read_open(const char *path) {
#if defined(_WIN32)
    wchar_t wide[SOURCE_PATH_CAPACITY];
    if (!source_wide(path, wide)) return NULL;
    HANDLE file = CreateFileW(wide, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    return file == INVALID_HANDLE_VALUE ? NULL : file;
#else
    int descriptor = open(path, O_RDONLY | O_CLOEXEC);
    return descriptor < 0 ? NULL : (void *)(uptr)(descriptor + 1);
#endif
}
/* Doctor's immutable probe must exclude SQLite writers, not merely open a fd.
 * POSIX whole-file shared lock conflicts with SQLite's reserved/pending locks. */
static void *source_freeze_read(const char *path) {
#if defined(_WIN32)
    wchar_t wide[SOURCE_PATH_CAPACITY];
    if (!source_wide(path, wide)) return NULL;
    HANDLE file = CreateFileW(wide, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    return file == INVALID_HANDLE_VALUE ? NULL : file;
#else
    int descriptor = open(path, O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
    if (descriptor < 0) return NULL;
    struct flock lock = {0}; lock.l_type = F_RDLCK; lock.l_whence = SEEK_SET;
    if (fcntl(descriptor, F_SETLK, &lock)) { close(descriptor); return NULL; }
    return (void *)(uptr)(descriptor + 1);
#endif
}
static int source_directory_writable(const char *path) {
#if defined(_WIN32)
    wchar_t wide[SOURCE_PATH_CAPACITY];
    if (!source_wide(path, wide)) return 0;
    HANDLE directory = CreateFileW(wide, 0x0002 /* FILE_ADD_FILE */, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (directory == INVALID_HANDLE_VALUE) return 0;
    return CloseHandle(directory);
#else
    struct stat status;
    return !stat(path, &status) && S_ISDIR(status.st_mode) && !access(path, W_OK | X_OK);
#endif
}
static void *source_stage_open(const char *temporary, const char *target) {
#if defined(_WIN32)
    wchar_t wide[SOURCE_PATH_CAPACITY];
    (void)target;
    if (!source_wide(temporary, wide)) return NULL;
    HANDLE file = CreateFileW(wide, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    return file == INVALID_HANDLE_VALUE ? NULL : file;
#else
    int descriptor = open(temporary, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0600);
    if (descriptor < 0) return NULL;
    struct stat previous;
    /* Preserve ordinary file permissions on replacement; new files stay private. */
    if (!lstat(target, &previous) && S_ISREG(previous.st_mode) && fchmod(descriptor, previous.st_mode & 0777)) {
        close(descriptor); unlink(temporary); return NULL;
    }
    return (void *)(uptr)(descriptor + 1);
#endif
}
/* Lexical absolute paths, including not-yet-created leaves. Do not resolve away
 * links here: write/reveal validation must still be able to reject them. */
static int source_full_path(const char *input, char *output) {
#if defined(_WIN32)
    wchar_t wide[SOURCE_PATH_CAPACITY], absolute[SOURCE_PATH_CAPACITY];
    if (!source_wide(input, wide)) return 0;
    DWORD size = GetFullPathNameW(wide, SOURCE_PATH_CAPACITY, absolute, NULL);
    if (!size || size >= SOURCE_PATH_CAPACITY || !WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
        absolute, -1, output, SOURCE_PATH_CAPACITY, NULL, NULL)) return 0;
    size_t length = strlen(output);
    while (length > 3 && (output[length - 1] == '\\' || output[length - 1] == '/')) output[--length] = 0;
    return 1;
#else
    char joined[SOURCE_PATH_CAPACITY], cwd[SOURCE_PATH_CAPACITY];
    if (*input != '/') {
        if (!getcwd(cwd, sizeof(cwd))) return 0;
        int length = snprintf(joined, sizeof(joined), "%s/%s", cwd, input);
        if (length < 0 || length >= (int)sizeof(joined)) return 0;
    } else {
        if (strlen(input) >= sizeof(joined)) return 0;
        strcpy(joined, input);
    }
    size_t used = 1;
    output[0] = '/';
    const char *cursor = joined;
    while (*cursor) {
        while (*cursor == '/') cursor++;
        const char *begin = cursor;
        while (*cursor && *cursor != '/') cursor++;
        size_t length = (size_t)(cursor - begin);
        if (!length || (length == 1 && *begin == '.')) continue;
        if (length == 2 && begin[0] == '.' && begin[1] == '.') {
            while (used > 1 && output[used - 1] != '/') used--;
            if (used > 1) used--;
            continue;
        }
        if (used > 1) output[used++] = '/';
        if (used + length >= SOURCE_PATH_CAPACITY) return 0;
        memcpy(output + used, begin, length); used += length;
    }
    output[used] = 0; return 1;
#endif
}
static source_time source_now(void) {
#if defined(_WIN32)
    FILETIME now; GetSystemTimeAsFileTime(&now); return source_filetime(now);
#else
    struct timespec now;
    if (clock_gettime(CLOCK_REALTIME, &now)) return 0;
    return SOURCE_UNIX_EPOCH + (u64)now.tv_sec * 10000000ULL + (u64)now.tv_nsec / 100;
#endif
}
static unsigned long source_process_id(void) {
#if defined(_WIN32)
    return GetCurrentProcessId();
#else
    return (unsigned long)getpid();
#endif
}
static const char *source_basename(const char *path) {
    const char *last = strrchr(path, '/');
#if defined(_WIN32)
    const char *back = strrchr(path, '\\');
    if (back && (!last || back > last)) last = back;
#endif
    return last ? last + 1 : path;
}
#endif
