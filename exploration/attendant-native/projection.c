/* Native Windows exploration. Source contract and write boundaries: README.md.
   The process owns one bounded arena; libraries retain their own allocators.
   Reject unsupported inputs rather than advertise untested YAML compatibility. */
#include "include/base.h"
/* Minimal stable CNG ABI declarations: Windows SDK bcrypt.h uses annotations
   unsupported by CJIT's bundled headers. Link Windows bcrypt.dll directly. */
__declspec(dllimport) LPWSTR *WINAPI CommandLineToArgvW(LPCWSTR, int *);
typedef HANDLE BCRYPT_ALG_HANDLE;
__declspec(dllimport) LONG WINAPI BCryptOpenAlgorithmProvider(BCRYPT_ALG_HANDLE *, LPCWSTR, LPCWSTR, ULONG);
__declspec(dllimport) LONG WINAPI BCryptHash(BCRYPT_ALG_HANDLE, PUCHAR, ULONG, PUCHAR, ULONG, PUCHAR, ULONG);
__declspec(dllimport) LONG WINAPI BCryptCloseAlgorithmProvider(BCRYPT_ALG_HANDLE, ULONG);
#ifndef WC_ERR_INVALID_CHARS
#define WC_ERR_INVALID_CHARS 0x80
#endif
#include <math.h>
#include <stdarg.h>
#include <setjmp.h>
#include <yaml.h>
#include <sqlite3.h>
#define JSONB_STATIC
#include "vendor/json-build/json-build.h"

#define MAX_FIELDS 128
#define MAX_COLLECTIONS 32
#define MAX_RECORDS 4096
#define MAX_PAIRS 256
#define MAX_ITEMS 1024
#define TEXT_CAPACITY (4 * 1024 * 1024)
#define PATH_CAPACITY 4096
#define OUTPUT_CAPACITY (12 * 1024 * 1024)
#define MAX_BUILDER_STATEMENTS 2 /* Edge INSERT plus target SELECT; finalize in stack order. */

typedef struct value value;
struct value {
    enum { VALUE_NULL, VALUE_STRING, VALUE_NUMBER, VALUE_BOOL, VALUE_ARRAY } kind;
    enum { NUMBER_F64, NUMBER_I64, NUMBER_U64 } number_kind;
    const char *text;
    union { double real; long long signed_integer; unsigned long long unsigned_integer; } number;
    value *items;
    int count;
};
typedef struct pair {
    const char *key;
    value data;
    size_t value_start, value_end;
    int indentless_sequence;
} pair;
typedef struct mapping {
    pair *pairs;
    int count, present;
    const char *problem, *problem_code;
} mapping;
typedef struct field {
    const char *name, *kind, *reference;
    int optional;
    value declaration, fallback;
} field;
typedef struct collection {
    const char *name, *directory, *schema_path, *schema_text;
    field fields[MAX_FIELDS];
    int field_count, invalid;
} collection;
typedef struct record {
    collection *owner;
    const char *path, *text, *body, *hash, *front_json;
    mapping front, corrections;
    int blocked, projection_changed, cache_changed;
    const char *next_text, *validation_name;
    double mtime_ms;
    ULONGLONG created_on_disk;
    size_t size;
} record;
typedef struct diagnostic { collection *owner; const char *path, *code, *message; } diagnostic;
#define MAX_DIAGNOSTICS 32768

typedef struct context {
    arena *memory;
    sqlite3 *db;
    sqlite3_stmt *builder_statements[MAX_BUILDER_STATEMENTS];
    int builder_statement_count;
    const char *temporary, *config_hash;
    collection collections[MAX_COLLECTIONS];
    int collection_count;
    record records[MAX_RECORDS];
    int record_count, projected, allow_fts, fts_enabled, diagnostic_count, contract_diagnostic_count, auto_correct, browse_source;
    const char *applied_corrections;
    diagnostic *diagnostics;
    const char *written[MAX_RECORDS], *staged[MAX_RECORDS];
    int written_count, staged_count;
    const char *rollback_paths[MAX_RECORDS], *rollback_expected[MAX_RECORDS], *created_directories[128];
    int rollback_count, created_directory_count;
    const char *error_code, *error_path;
    /* Embedded request boundary only; CLI keeps its existing fatal contract. */
    jmp_buf *recovery;
    char error_message[6144];
    HANDLE reading_file, scanning_directory;
    yaml_parser_t *yaml_scanner, *yaml_parser;
    yaml_document_t *yaml_document;
    char json_scratch[OUTPUT_CAPACITY];
} context;

/* One process owns one command context, including cleanup on JSON capacity failure. */
static context *active_context;
static void fail(context *c, const char *format, ...);
static void json_check(jsonbcode status) {
    if (status < 0) {
        if (active_context) { active_context->error_code = "json-capacity"; fail(active_context, "JSON output capacity exhausted"); }
        fprintf(stderr, "JSON output capacity exhausted\n");
        puts("{\"error\":{\"code\":\"json-capacity\",\"message\":\"JSON output capacity exhausted\"}}");
        exit(1);
    }
}
/* Cleanup owns only the exact published bytes, not every future file at a path.
   Open without write/delete sharing, compare, then delete through same handle. */
static int rollback_source(const wchar_t *path, const char *expected) {
    typedef BOOL (WINAPI *dispose_file)(HANDLE, int, LPVOID, DWORD);
    dispose_file dispose = (dispose_file)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "SetFileInformationByHandle");
    HANDLE file;
    LARGE_INTEGER size;
    BY_HANDLE_FILE_INFORMATION info;
    char bytes[4096];
    size_t offset = 0, length;
    int ok = 1;
    struct { BOOL deleted; } disposition = {TRUE};
    if (!expected || !dispose) return 0;
    file = CreateFileW(path, GENERIC_READ | DELETE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT, NULL);
    if (file == INVALID_HANDLE_VALUE) return GetLastError() == ERROR_FILE_NOT_FOUND || GetLastError() == ERROR_PATH_NOT_FOUND;
    length = strlen(expected);
    if (!GetFileInformationByHandle(file, &info) || (info.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) ||
        !GetFileSizeEx(file, &size) || size.QuadPart != (LONGLONG)length) ok = 0;
    while (ok && offset < length) {
        DWORD count = 0, wanted = (DWORD)(length - offset > sizeof(bytes) ? sizeof(bytes) : length - offset);
        if (!ReadFile(file, bytes, wanted, &count, NULL) || count != wanted || memcmp(bytes, expected + offset, wanted)) ok = 0;
        offset += count;
    }
    if (ok) ok = dispose(file, 4 /* FileDispositionInfo */, &disposition, sizeof(disposition)) != 0;
    CloseHandle(file); return ok;
}
static void fail(context *c, const char *format, ...) {
    char message[2048], cleaned[6144], output[16384];
    size_t raw_length, offset = 0, cleaned_length = 0;
    int preserved_rollback = 0;
    jsonb b;
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args); message[sizeof(message) - 1] = 0;
    raw_length = strlen(message);
    while (offset < raw_length) {
        size_t width = string_utf8_sequence_size((const u8 *)message + offset, raw_length - offset);
        if (!width) { memcpy(cleaned + cleaned_length, "\xef\xbf\xbd", 3); cleaned_length += 3; offset++; }
        else { memcpy(cleaned + cleaned_length, message + offset, width); cleaned_length += width; offset += width; }
    }
    cleaned[cleaned_length] = 0;
    if (!c->recovery) fprintf(stderr, "%s\n", cleaned);
    if (c->reading_file) { CloseHandle(c->reading_file); c->reading_file = NULL; }
    if (c->scanning_directory) { FindClose(c->scanning_directory); c->scanning_directory = NULL; }
    if (c->yaml_document) { yaml_document_delete(c->yaml_document); c->yaml_document = NULL; }
    if (c->yaml_parser) { yaml_parser_delete(c->yaml_parser); c->yaml_parser = NULL; }
    if (c->yaml_scanner) { yaml_parser_delete(c->yaml_scanner); c->yaml_scanner = NULL; }
    if (c->db) {
        /* Only finalize our own statements, never FTS's internal statements. */
        while (c->builder_statement_count) sqlite3_finalize(c->builder_statements[--c->builder_statement_count]);
        sqlite3_close_v2(c->db); c->db = NULL;
    }
    while (c->staged_count) {
        wchar_t path[PATH_CAPACITY];
        const char *temporary = c->staged[--c->staged_count];
        if (temporary && MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, temporary, -1, path, PATH_CAPACITY)) DeleteFileW(path);
    }
    while (c->rollback_count) {
        wchar_t path[PATH_CAPACITY];
        const char *owned = c->rollback_paths[--c->rollback_count];
        if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, owned, -1, path, PATH_CAPACITY) && rollback_source(path, c->rollback_expected[c->rollback_count])) {
            int i;
            for (i = 0; i < c->written_count; i++) if (!strcmp(c->written[i], owned)) {
                memmove(&c->written[i], &c->written[i + 1], (c->written_count - i - 1) * sizeof(char *)); c->written_count--; break;
            }
        } else preserved_rollback = 1;
    }
    if (preserved_rollback) {
        const char *note = " Cleanup preserved changed or busy files; inspect written paths.";
        if (cleaned_length + strlen(note) < sizeof(cleaned)) { strcpy(cleaned + cleaned_length, note); cleaned_length += strlen(note); }
    }
    while (c->created_directory_count) {
        wchar_t path[PATH_CAPACITY];
        const char *owned = c->created_directories[--c->created_directory_count];
        if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, owned, -1, path, PATH_CAPACITY)) RemoveDirectoryW(path);
    }
    if (c->temporary) {
        const char *suffixes[] = {"", "-wal", "-shm", "-journal"};
        int i;
        for (i = 0; i < 4; i++) {
            wchar_t path[PATH_CAPACITY];
            char name[PATH_CAPACITY * 4];
            snprintf(name, sizeof(name), "%s%s", c->temporary, suffixes[i]);
            if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, name, -1, path, PATH_CAPACITY) &&
                !DeleteFileW(path) && GetLastError() != ERROR_FILE_NOT_FOUND)
                fprintf(stderr, "Cannot remove temporary projection: %s\n", name);
        }
    }
    if (c->recovery) {
        memcpy(c->error_message, cleaned, cleaned_length + 1);
        longjmp(*c->recovery, 1);
    }
    jsonb_init(&b);
    json_check(jsonb_object(&b, output, sizeof(output)));
    json_check(jsonb_key(&b, output, sizeof(output), "error", 5));
    json_check(jsonb_object(&b, output, sizeof(output)));
    json_check(jsonb_key(&b, output, sizeof(output), "code", 4));
    {
        const char *code = c->error_code ? c->error_code : "native-projection";
        json_check(jsonb_string(&b, output, sizeof(output), code, strlen(code)));
    }
    json_check(jsonb_key(&b, output, sizeof(output), "message", 7));
    json_check(jsonb_string(&b, output, sizeof(output), cleaned, cleaned_length));
    json_check(jsonb_object_pop(&b, output, sizeof(output)));
    json_check(jsonb_object_pop(&b, output, sizeof(output)));
    printf("%.*s", (int)b.pos - 1, output);
    if (c->error_path) {
        fputs(",\"failed\":", stdout);
        jsonb_init(&b); json_check(jsonb_string(&b, output, sizeof(output), c->error_path, strlen(c->error_path)));
        printf("%.*s", (int)b.pos, output);
    }
    if (c->written_count) {
        int i;
        fputs(",\"written\":[", stdout);
        for (i = 0; i < c->written_count; i++) {
            const char *path = c->written[i];
            jsonb_init(&b); json_check(jsonb_string(&b, output, sizeof(output), path, strlen(path)));
            printf("%s%.*s", i ? "," : "", (int)b.pos, output);
        }
        fputc(']', stdout);
    }
    puts("}");
    exit(1);
}
static void *allocate(context *c, size_t size) {
    arena_result result = arena_alloc(c->memory, size ? size : 1, 16);
    if (result.error) fail(c, "Native projection's 128 MiB workspace exhausted");
    return result.ptr;
}
static char *copy_text(context *c, const char *text, size_t length) {
    char *result = allocate(c, length + 1);
    memcpy(result, text, length);
    return result;
}
static const char *format_text(context *c, const char *format, ...) {
    char buffer[32768];
    int length;
    va_list args;
    va_start(args, format);
    length = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    if (length < 0 || length >= (int)sizeof(buffer)) fail(c, "Formatted text capacity exceeded");
    return copy_text(c, buffer, length);
}
static wchar_t *wide(context *c, const char *path) {
    int count = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path, -1, NULL, 0);
    wchar_t *result;
    if (!count) fail(c, "Invalid UTF-8 path");
    result = allocate(c, count * sizeof(wchar_t));
    if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path, -1, result, count)) fail(c, "Path conversion failed");
    return result;
}
static const char *utf8(context *c, const wchar_t *path) {
    int count = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, path, -1, NULL, 0, NULL, NULL);
    char *result;
    if (!count) fail(c, "Invalid Windows path");
    result = allocate(c, count);
    if (!WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, path, -1, result, count, NULL, NULL)) fail(c, "Path conversion failed");
    return result;
}
static int absolute_path(const char *path) {
    return path[0] == '/' || path[0] == '\\' ||
        (strlen(path) > 2 && path[1] == ':' && (path[2] == '/' || path[2] == '\\'));
}
static const char *full_path(context *c, const char *base, const char *path) {
    wchar_t output[PATH_CAPACITY];
    const char *input = path;
    DWORD length;
    if (!absolute_path(path))
        input = format_text(c, "%s\\%s", base, path);
    length = GetFullPathNameW(wide(c, input), PATH_CAPACITY, output, NULL);
    if (!length || length >= PATH_CAPACITY) fail(c, "Cannot resolve path: %s", input);
    while (length > 3 && (output[length - 1] == L'\\' || output[length - 1] == L'/')) output[--length] = 0;
    return utf8(c, output);
}
/* Per-file failures can be listed by workspace browser; allocation failures remain fatal. */
static const char *try_read_text(context *c, const char *path, size_t *length, const char **problem) {
    HANDLE file = CreateFileW(wide(c, path), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                              NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    LARGE_INTEGER size;
    DWORD count;
    char *text;
    if (file == INVALID_HANDLE_VALUE) { *problem = format_text(c, "Cannot read %s", path); return NULL; }
    if (!GetFileSizeEx(file, &size) || size.QuadPart < 0 || size.QuadPart > TEXT_CAPACITY) {
        CloseHandle(file); *problem = format_text(c, "Source exceeds 4 MiB capacity: %s", path); return NULL;
    }
    c->reading_file = file;
    text = allocate(c, (size_t)size.QuadPart + 1);
    if (!ReadFile(file, text, (DWORD)size.QuadPart, &count, NULL) || count != size.QuadPart) {
        CloseHandle(file); c->reading_file = NULL; *problem = format_text(c, "Read failed: %s", path); return NULL;
    }
    CloseHandle(file); c->reading_file = NULL;
    if (memchr(text, 0, count) || !string_view_utf8_valid(string_view_from_cstr(text))) { *problem = format_text(c, "Source must be NUL-free UTF-8: %s", path); return NULL; }
    if (length) *length = count;
    return text;
}
static const char *read_text(context *c, const char *path, size_t *length) {
    const char *problem = NULL, *text = try_read_text(c, path, length, &problem);
    if (!text) fail(c, "%s", problem);
    return text;
}
static const char *digest(context *c, const char *text) {
    BCRYPT_ALG_HANDLE algorithm;
    unsigned char bytes[32];
    char result[65];
    int i;
    if (BCryptOpenAlgorithmProvider(&algorithm, L"SHA256", NULL, 0) < 0) fail(c, "SHA-256 unavailable");
    if (BCryptHash(algorithm, NULL, 0, (PUCHAR)text, (ULONG)strlen(text), bytes, sizeof(bytes)) < 0) {
        BCryptCloseAlgorithmProvider(algorithm, 0); fail(c, "SHA-256 failed");
    }
    BCryptCloseAlgorithmProvider(algorithm, 0);
    for (i = 0; i < 32; i++) sprintf(result + i * 2, "%02x", bytes[i]);
    return copy_text(c, result, 64);
}
static char *trim(char *text) {
    char *end;
    for (;;) {
        if (*text == ' ' || *text == '\t' || *text == '\r' || *text == '\v' || *text == '\f') text++;
        else if (!strncmp(text, "\xef\xbb\xbf", 3)) text += 3; /* UTF-8 BOM, like JS trim. */
        else break;
    }
    end = text + strlen(text);
    while (end > text && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\r' || end[-1] == '\v' || end[-1] == '\f')) *--end = 0;
    return text;
}
static value *lookup(mapping *map, const char *key) {
    int i;
    for (i = 0; i < map->count; i++) if (!strcmp(map->pairs[i].key, key)) return &map->pairs[i].data;
    return NULL;
}
static void json_number(context *c, jsonb *b, char *output, size_t capacity, double number);
#include "numbers.h"
static int scalar_equal(value a, value b) {
    if (a.kind != b.kind) return 0;
    if (a.kind == VALUE_STRING) return !strcmp(a.text, b.text);
    if (a.kind == VALUE_NUMBER) return numbers_equal(a, b);
    if (a.kind == VALUE_BOOL) return a.number.real == b.number.real;
    return a.kind == VALUE_NULL;
}
static int value_equal(value a, value b) {
    int i;
    if (a.kind != VALUE_ARRAY || b.kind != VALUE_ARRAY) return scalar_equal(a, b);
    if (a.count != b.count) return 0;
    for (i = 0; i < a.count; i++) if (!scalar_equal(a.items[i], b.items[i])) return 0;
    return 1;
}
static int mapping_equal(mapping *a, mapping *b) {
    int i;
    if (a->count != b->count) return 0;
    for (i = 0; i < a->count; i++) {
        value *actual = lookup(b, a->pairs[i].key);
        if (!actual || !value_equal(a->pairs[i].data, *actual)) return 0;
    }
    return 1;
}
static void add_diagnostic(context *c, collection *owner, const char *path, const char *code, const char *message) {
    diagnostic *d;
    if (!c->diagnostics) c->diagnostics = allocate(c, MAX_DIAGNOSTICS * sizeof(diagnostic));
    if (c->diagnostic_count == MAX_DIAGNOSTICS) fail(c, "Diagnostic capacity exceeded");
    d = &c->diagnostics[c->diagnostic_count++];
    d->owner = owner; d->path = path; d->code = code; d->message = message;
}
#include "source-yaml.h"
/* Shortest decimal precision that round-trips through strtod, then ECMAScript's
   fixed/exponent range. This is not a general replacement for a proven dtoa;
   the differential fixture checks cover the numeric forms used in this slice. */
static void json_number(context *c, jsonb *b, char *output, size_t capacity, double number) {
    char shortest[64], token[128], digits[32];
    const char *p, *exponent;
    int precision, count = 0, decimal = 0, power, position, used = 0, i;
    if (number == 0) { json_check(jsonb_token(b, output, capacity, "0", 1)); return; }
    for (precision = 1; precision <= 17; precision++) {
        snprintf(shortest, sizeof(shortest), "%.*g", precision, number);
        if (strtod(shortest, NULL) == number) break;
    }
    p = shortest;
    if (*p == '-') { token[used++] = '-'; p++; }
    exponent = strchr(p, 'e');
    power = exponent ? atoi(exponent + 1) : 0;
    while (*p && *p != 'e') {
        if (*p == '.') decimal = count;
        else digits[count++] = *p;
        p++;
    }
    if (!strchr(shortest, '.')) decimal = count;
    position = decimal + power;
    while (count > 1 && digits[0] == '0') { memmove(digits, digits + 1, --count); position--; }
    while (count > 1 && digits[count - 1] == '0') count--;
    if (position > 0 && position <= 21) {
        for (i = 0; i < position || i < count; i++) {
            if (i == position) token[used++] = '.';
            token[used++] = i < count ? digits[i] : '0';
        }
    } else if (position <= 0 && position > -6) {
        token[used++] = '0'; token[used++] = '.';
        for (i = 0; i < -position; i++) token[used++] = '0';
        for (i = 0; i < count; i++) token[used++] = digits[i];
    } else {
        token[used++] = digits[0];
        if (count > 1) token[used++] = '.';
        for (i = 1; i < count; i++) token[used++] = digits[i];
        used += snprintf(token + used, sizeof(token) - used, "e%+d", position - 1);
    }
    if (used <= 0 || used >= (int)sizeof(token)) fail(c, "Number serialization failed");
    json_check(jsonb_token(b, output, capacity, token, used));
}
static void json_value_format(context *c, jsonb *b, char *output, size_t capacity, value v, number_format format) {
    int i;
    if (v.kind == VALUE_STRING) json_check(jsonb_string(b, output, capacity, v.text, strlen(v.text)));
    else if (v.kind == VALUE_NUMBER) json_numeric(c, b, output, capacity, v, format);
    else if (v.kind == VALUE_BOOL) json_check(jsonb_bool(b, output, capacity, (int)v.number.real));
    else if (v.kind == VALUE_NULL) json_check(jsonb_null(b, output, capacity));
    else {
        json_check(jsonb_array(b, output, capacity));
        for (i = 0; i < v.count; i++) json_value_format(c, b, output, capacity, v.items[i], format);
        json_check(jsonb_array_pop(b, output, capacity));
    }
}
static void json_value(context *c, jsonb *b, char *out, size_t capacity, value v) { json_value_format(c, b, out, capacity, v, JSON_NUMBERS); }
static void json_property(jsonb *b, char *out, size_t capacity, const char *key, const char *text) {
    json_check(jsonb_key(b, out, capacity, key, strlen(key)));
    json_check(jsonb_string(b, out, capacity, text, strlen(text)));
}
static const char *value_json(context *c, value v) {
    char *buffer = c->json_scratch;
    jsonb b;
    jsonb_init(&b);
    json_value(c, &b, buffer, TEXT_CAPACITY, v);
    return copy_text(c, buffer, b.pos);
}
static unsigned long long object_index(const char *key) {
    unsigned long long number = 0;
    int i;
    if (!*key || (key[0] == '0' && key[1]) || strlen(key) > 10) return 4294967295ULL;
    for (i = 0; key[i]; i++) {
        if (key[i] < '0' || key[i] > '9') return 4294967295ULL;
        number = number * 10 + key[i] - '0';
    }
    return number < 4294967295ULL ? number : 4294967295ULL;
}
static const char *mapping_json_format(context *c, mapping *map, number_format format) {
    char *buffer = c->json_scratch;
    jsonb b;
    int i, j, order[MAX_PAIRS];
    /* JS objects enumerate canonical integer-index keys before other keys.
       FTS indexes this JSON text, so ordering affects phrase matches. */
    for (i = 0; i < map->count; i++) {
        order[i] = i;
        for (j = i; j > 0 && object_index(map->pairs[order[j]].key) < object_index(map->pairs[order[j - 1]].key); j--) {
            int swap = order[j]; order[j] = order[j - 1]; order[j - 1] = swap;
        }
    }
    jsonb_init(&b);
    json_check(jsonb_object(&b, buffer, TEXT_CAPACITY));
    for (i = 0; i < map->count; i++) {
        pair *entry = &map->pairs[order[i]];
        json_check(jsonb_key(&b, buffer, TEXT_CAPACITY, entry->key, strlen(entry->key)));
        json_value_format(c, &b, buffer, TEXT_CAPACITY, entry->data, format);
    }
    json_check(jsonb_object_pop(&b, buffer, TEXT_CAPACITY));
    return copy_text(c, buffer, b.pos);
}
static const char *mapping_json(context *c, mapping *map) { return mapping_json_format(c, map, JSON_NUMBERS); }
static void sql(context *c, const char *statement) {
    char *error = NULL;
    if (sqlite3_exec(c->db, statement, NULL, NULL, &error) != SQLITE_OK) fail(c, "SQLite: %s", error ? error : sqlite3_errmsg(c->db));
}
static sqlite3_stmt *prepare(context *c, const char *statement) {
    sqlite3_stmt *result;
    if (c->builder_statement_count == MAX_BUILDER_STATEMENTS) fail(c, "Builder statement capacity exceeded");
    if (sqlite3_prepare_v2(c->db, statement, -1, &result, NULL) != SQLITE_OK) fail(c, "SQLite prepare: %s", sqlite3_errmsg(c->db));
    c->builder_statements[c->builder_statement_count++] = result;
    return result;
}
static int finalize_builder(context *c, sqlite3_stmt *statement) {
    if (!c->builder_statement_count || c->builder_statements[c->builder_statement_count - 1] != statement)
        fail(c, "Builder statement ownership mismatch");
    c->builder_statement_count--;
    return sqlite3_finalize(statement);
}
static void bind_text(context *c, sqlite3_stmt *s, int i, const char *text) {
    if (sqlite3_bind_text(s, i, text, -1, SQLITE_TRANSIENT) != SQLITE_OK) fail(c, "SQLite bind failed");
}
static void finish(context *c, sqlite3_stmt *s) {
    if (sqlite3_step(s) != SQLITE_DONE) fail(c, "SQLite step: %s", sqlite3_errmsg(c->db));
    if (finalize_builder(c, s) != SQLITE_OK) fail(c, "SQLite finalize failed");
}
static const char *identifier(context *c, const char *text) {
    char buffer[2048];
    size_t used = 0;
    buffer[used++] = '"';
    while (*text) {
        if (used + 3 >= sizeof(buffer)) fail(c, "SQL identifier too long");
        if (*text == '"') buffer[used++] = '"';
        buffer[used++] = *text++;
    }
    buffer[used++] = '"'; buffer[used] = 0;
    return copy_text(c, buffer, used);
}
static const char *internal(context *c, const char *prefix, collection *table) {
    return identifier(c, format_text(c, "__attendant_%s_%s", prefix, table->name));
}
static const char *builtins[] = {"id", "name", "created_at", "updated_at", "desc", "tags", "hidden", "source_path", "source_hash"};
static void schema_problem(context *c, collection *table, const char *code, const char *message) {
    table->invalid = 1;
    add_diagnostic(c, table, table->schema_path, code, message);
}
static void load_schema(context *c, collection *table) {
    mapping schema = read_mapping(c, table->schema_text, NULL);
    int i, j;
    if (schema.problem) { schema_problem(c, table, "schema-yaml", schema.problem); return; }
    if (schema.count > MAX_FIELDS) fail(c, "Schema exceeds field capacity");
    for (i = 0; i < schema.count; i++) {
        field f = {0};
        value v = schema.pairs[i].data;
        f.name = schema.pairs[i].key; f.declaration = v;
        for (j = 0; j < (int)(sizeof(builtins) / sizeof(builtins[0])); j++) if (!strcmp(f.name, builtins[j])) break;
        if (j != (int)(sizeof(builtins) / sizeof(builtins[0]))) { schema_problem(c, table, "schema-field-reserved", format_text(c, "%s is reserved for Attendant.", f.name)); continue; }
        if (v.kind == VALUE_NULL) { f.kind = "untyped"; f.optional = 1; }
        else if (v.kind == VALUE_ARRAY) {
            for (j = 0; j < v.count; j++) if (v.items[j].kind == VALUE_NULL) break;
            if (!v.count || j != v.count) { schema_problem(c, table, "schema-enum", "Enum must be a non-empty scalar array without null."); continue; }
            f.kind = "enum"; f.fallback = v.items[0];
        } else if (v.kind == VALUE_STRING) {
            const char *marker = v.text;
            if (*marker == '?') { f.optional = 1; marker++; }
            if (!strcmp(marker, "date") || !strcmp(marker, "datetime")) f.kind = marker;
            else if (!strcmp(marker, "string[]")) f.kind = "string-array";
            else if (!strncmp(marker, "ref:", 4) && marker[4]) { f.kind = "ref"; f.reference = marker + 4; }
            else if (!strncmp(marker, "ref[]:", 6) && marker[6]) { f.kind = "ref-array"; f.reference = marker + 6; }
            else if (!strcmp(marker, "ref") || !strcmp(marker, "ref[]") || !strncmp(marker, "ref:", 4) || !strncmp(marker, "ref[]:", 6)) {
                schema_problem(c, table, "schema-ref", "Reference marker needs a collection name."); continue;
            } else if (f.optional) { schema_problem(c, table, "schema-marker", "Unsupported optional schema marker."); continue; }
            else { f.kind = "default"; f.fallback = v; }
        } else { f.kind = "default"; f.fallback = v; }
        if ((f.fallback.kind == VALUE_NUMBER && f.fallback.number_kind == NUMBER_U64)) {
            schema_problem(c, table, "schema-number-range", "Numeric default exceeds SQLite signed 64-bit INTEGER range."); continue;
        }
        if (!strcmp(f.kind, "enum")) {
            for (j = 0; j < f.declaration.count; j++) if (f.declaration.items[j].kind == VALUE_NUMBER && f.declaration.items[j].number_kind == NUMBER_U64) break;
            if (j != f.declaration.count) { schema_problem(c, table, "schema-number-range", "Numeric enum value exceeds SQLite signed 64-bit INTEGER range."); continue; }
        }
        table->fields[table->field_count++] = f;
    }
}
static char *table_alias_separator(char *directory) {
    char *p;
    for (p = directory; *p; p++) if ((*p == ' ' || *p == '\t') && p[1] == 'a' && p[2] == 's' && (p[3] == ' ' || p[3] == '\t')) return p;
    return NULL;
}
static void load_config(context *c, const char *root) {
    const char *path = full_path(c, root, ".pi/attendant.tables");
    const char *source;
    char *config, *line;
    if (GetFileAttributesW(wide(c, path)) == INVALID_FILE_ATTRIBUTES) {
        add_diagnostic(c, NULL, path, "tables-missing", "Missing .pi/attendant.tables."); return;
    }
    source = read_text(c, path, NULL);
    config = copy_text(c, source, strlen(source)); line = config;
    c->config_hash = digest(c, source);
    while (line) {
        char *next = strchr(line, '\n');
        char *directory, *alias, *name;
        collection *table;
        int i;
        if (next) *next++ = 0;
        directory = trim(line);
        line = next;
        if (!*directory || *directory == '#') continue;
        if (c->collection_count == MAX_COLLECTIONS) fail(c, "Collection capacity exceeded");
        alias = table_alias_separator(directory);
        if (alias) { *alias = 0; name = trim(alias + 3); directory = trim(directory); }
        else {
            char *base = copy_text(c, directory, strlen(directory));
            size_t length = strlen(base);
            char *a, *b;
            while (length && (base[length - 1] == '/' || base[length - 1] == '\\')) base[--length] = 0;
            a = strrchr(base, '/'); b = strrchr(base, '\\');
            name = a && (!b || a > b) ? a + 1 : b ? b + 1 : base;
        }
        if (!*name || strchr(name, '/') || !strncmp(name, "__attendant_", 12)) {
            add_diagnostic(c, NULL, path, "collection-name", format_text(c, "Invalid collection name: %s", name)); continue;
        }
        for (i = 0; i < c->collection_count; i++) if (!strcmp(name, c->collections[i].name)) break;
        if (i != c->collection_count) { add_diagnostic(c, NULL, path, "collection-duplicate", format_text(c, "Collection name %s is repeated.", name)); continue; }
        table = &c->collections[c->collection_count++];
        table->name = name;
        /* loadContract preserves spelling of absolute configured paths. Its
           schema hash includes that spelling even when file paths normalize. */
        table->directory = absolute_path(directory) ? directory : full_path(c, root, directory);
        table->schema_path = full_path(c, table->directory, ".schema.md");
        if (GetFileAttributesW(wide(c, table->schema_path)) == INVALID_FILE_ATTRIBUTES) {
            schema_problem(c, table, "schema-missing", "Collection has no .schema.md.");
            table->schema_text = ""; continue;
        }
        table->schema_text = read_text(c, table->schema_path, NULL);
        load_schema(c, table);
    }
    {
        int i, j, k;
        for (i = 0; i < c->collection_count; i++) for (j = 0; j < c->collections[i].field_count; j++) {
            field *f = &c->collections[i].fields[j];
            if (!f->reference) continue;
            for (k = 0; k < c->collection_count; k++) if (!strcmp(f->reference, c->collections[k].name)) break;
            if (k == c->collection_count) schema_problem(c, &c->collections[i], "schema-ref", format_text(c, "%s references unknown collection %s.", f->name, f->reference));
        }
    }
}
static const char *iso_time(context *c, FILETIME filetime) {
    SYSTEMTIME time;
    if (!FileTimeToSystemTime(&filetime, &time)) fail(c, "Invalid file timestamp");
    return format_text(c, "%04u-%02u-%02uT%02u:%02u:%02u.%03uZ", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);
}
#include "datetime.h"
#include "records.h"
static int compare_paths(const void *a, const void *b) { return strcmp(*(const char *const *)a, *(const char *const *)b); }
static void load_records(context *c, collection *table) {
    WIN32_FIND_DATAW entry;
    HANDLE search = FindFirstFileW(wide(c, format_text(c, "%s\\*", table->directory)), &entry);
    const char *paths[MAX_RECORDS];
    int count = 0, i;
    if (search == INVALID_HANDLE_VALUE) { add_diagnostic(c, table, table->directory, "collection-read", "Collection directory cannot be read."); table->invalid = 1; return; }
    c->scanning_directory = search;
    do {
        const char *name = utf8(c, entry.cFileName);
        size_t length = strlen(name);
        if ((entry.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) || *name == '.' || length < 3 || strcmp(name + length - 3, ".md")) continue;
        if (count == MAX_RECORDS) fail(c, "Record capacity exceeded");
        paths[count++] = full_path(c, table->directory, name);
    } while (FindNextFileW(search, &entry));
    if (GetLastError() != ERROR_NO_MORE_FILES) { FindClose(search); c->scanning_directory = NULL; fail(c, "Directory enumeration failed"); }
    FindClose(search); c->scanning_directory = NULL;
    qsort(paths, count, sizeof(paths[0]), compare_paths);
    for (i = 0; i < count; i++) {
        WIN32_FILE_ATTRIBUTE_DATA attributes;
        ULARGE_INTEGER ticks;
        record *r;
        if (c->record_count == MAX_RECORDS) fail(c, "Total record capacity exceeded");
        r = &c->records[c->record_count++];
        r->owner = table; r->path = paths[i];
        if (c->browse_source) {
            const char *problem = NULL;
            r->text = try_read_text(c, r->path, &r->size, &problem);
            if (!r->text) { record_problem(c, r, "record-read", problem); continue; }
        } else r->text = read_text(c, r->path, &r->size);
        if (!GetFileAttributesExW(wide(c, r->path), GetFileExInfoStandard, &attributes)) {
            if (c->browse_source) { record_problem(c, r, "record-read", format_text(c, "Cannot stat %s", r->path)); continue; }
            fail(c, "Cannot stat %s", r->path);
        }
        ticks.LowPart = attributes.ftLastWriteTime.dwLowDateTime; ticks.HighPart = attributes.ftLastWriteTime.dwHighDateTime;
        ticks.QuadPart -= 116444736000000000ULL;
        /* Match Node/libuv's seconds + nanosecond fraction arithmetic. */
        r->mtime_ms = (double)(ticks.QuadPart / 10000000ULL) * 1000.0 + (double)(ticks.QuadPart % 10000000ULL) / 10000.0;
        ticks.LowPart = attributes.ftCreationTime.dwLowDateTime; ticks.HighPart = attributes.ftCreationTime.dwHighDateTime;
        r->created_on_disk = ticks.QuadPart;
        r->front = read_mapping(c, r->text, &r->body);
        if (!r->front.present && !r->front.problem) {
            r->front.problem_code = "record-frontmatter"; r->front.problem = "Record front matter must be a block mapping.";
        }
        validate_record(c, r);
        r->hash = digest(c, r->text);
        r->front_json = r->front.problem || !r->front.present ? "null" : mapping_json_format(c, &r->front, INDEX_NUMBERS);
    }
}
static void metadata(context *c, const char *key, const char *value) {
    sqlite3_stmt *s = prepare(c, "INSERT INTO __attendant_metadata VALUES (?,?)");
    bind_text(c, s, 1, key); bind_text(c, s, 2, value); finish(c, s);
}
static const char *field_type(field *f) {
    if (!strcmp(f->kind, "untyped")) return "";
    if (!strcmp(f->kind, "default") || !strcmp(f->kind, "enum"))
        return f->fallback.kind == VALUE_NUMBER ? "NUMERIC" : f->fallback.kind == VALUE_BOOL ? "INTEGER" : "TEXT";
    return "TEXT";
}
static void make_search_tables(context *c, collection *table) {
    sql(c, format_text(c, "CREATE VIRTUAL TABLE %s USING fts5(record_id UNINDEXED, content, tokenize = 'unicode61 remove_diacritics 2')", internal(c, "fts", table)));
    sql(c, format_text(c, "CREATE VIRTUAL TABLE %s USING fts5(record_id UNINDEXED, content, tokenize = 'trigram')", internal(c, "trigram", table)));
}
static void make_tables(context *c, collection *table) {
    char statement[32768];
    int i;
    size_t length;
    snprintf(statement, sizeof(statement), "CREATE TABLE %s (\"id\" TEXT PRIMARY KEY, \"name\" TEXT NOT NULL, \"created_at\" TEXT NOT NULL, \"updated_at\" TEXT NOT NULL, \"desc\" TEXT NOT NULL, \"tags\" TEXT NOT NULL, \"hidden\" INTEGER NOT NULL, \"source_path\" TEXT NOT NULL UNIQUE, \"source_hash\" TEXT NOT NULL", identifier(c, table->name));
    for (i = 0; i < table->field_count; i++) {
        const char *part = format_text(c, ", %s %s", identifier(c, table->fields[i].name), field_type(&table->fields[i]));
        length = strlen(statement);
        if (length + strlen(part) + 2 >= sizeof(statement)) fail(c, "Table SQL capacity exceeded");
        strcat(statement, part);
    }
    strcat(statement, ")"); sql(c, statement);
    if (c->fts_enabled) make_search_tables(c, table);
}
static void bind_value(context *c, sqlite3_stmt *s, int index, value *v, field *f) {
    if (!v && f && (!strcmp(f->kind, "default") || !strcmp(f->kind, "enum"))) v = &f->fallback;
    if (!v && f && !strcmp(f->kind, "string-array")) bind_text(c, s, index, "[]");
    else if (!v || v->kind == VALUE_NULL) sqlite3_bind_null(s, index);
    else if (v->kind == VALUE_ARRAY || (v->kind == VALUE_NUMBER && f && !strcmp(field_type(f), "TEXT")))
        bind_text(c, s, index, value_json(c, *v)); /* Avoid SQLite's lossy double-to-TEXT conversion for mixed enums. */
    else if (v->kind == VALUE_STRING) bind_text(c, s, index, f && !strcmp(f->kind, "datetime") ? validated_datetime(c, v->text) : v->text);
    else if (v->kind == VALUE_BOOL) sqlite3_bind_int(s, index, (int)v->number.real);
    else if (v->number_kind == NUMBER_I64) sqlite3_bind_int64(s, index, v->number.signed_integer);
    else if (v->number_kind == NUMBER_U64) fail(c, "Unsigned scalar exceeds SQLite INTEGER range");
    else sqlite3_bind_double(s, index, v->number.real);
}
static void index_record(context *c, record *r) {
    collection *table = r->owner;
    sqlite3_stmt *s;
    value *hidden = lookup(&r->front, "hidden");
    int i;
    if (r->blocked || (hidden && hidden->number.real)) return;
    for (i = 0; c->fts_enabled && i < 2; i++) {
        s = prepare(c, format_text(c, "INSERT INTO %s (record_id,content) VALUES (?,?)", internal(c, i ? "trigram" : "fts", table)));
        bind_text(c, s, 1, lookup(&r->front, "id")->text);
        {
            size_t a = strlen(r->front_json), b = strlen(r->body);
            char *content = allocate(c, a + b + 2);
            memcpy(content, r->front_json, a); content[a] = '\n'; memcpy(content + a + 1, r->body, b + 1);
            bind_text(c, s, 2, content);
        }
        finish(c, s);
    }
}
static void project_record(context *c, record *r) {
    collection *table = r->owner;
    sqlite3_stmt *s;
    value *hidden = lookup(&r->front, "hidden");
    char placeholders[1024] = "?";
    int i;
    s = prepare(c, "INSERT INTO __attendant_cache VALUES (?,?,?,?,?,?,?,?)");
    bind_text(c, s, 1, r->path); bind_text(c, s, 2, table->name);
    sqlite3_bind_double(s, 3, r->mtime_ms); sqlite3_bind_int64(s, 4, (sqlite3_int64)r->size);
    bind_text(c, s, 5, r->hash); bind_text(c, s, 6, r->front_json);
    bind_text(c, s, 7, record_diagnostics_json(c, r)); bind_text(c, s, 8, r->text); finish(c, s);
    if (r->blocked || (hidden && hidden->number.real)) return;
    for (i = 1; i < 9 + table->field_count; i++) strcat(placeholders, ",?");
    s = prepare(c, format_text(c, "INSERT INTO %s VALUES (%s)", identifier(c, table->name), placeholders));
    bind_value(c, s, 1, lookup(&r->front, "id"), NULL);
    bind_value(c, s, 2, lookup(&r->front, "name"), NULL);
    bind_text(c, s, 3, validated_datetime(c, lookup(&r->front, "created_at")->text));
    {
        ULARGE_INTEGER time;
        FILETIME ft;
        /* Node's Stats Date accessor rounds fractional milliseconds. */
        time.QuadPart = (ULONGLONG)floor(r->mtime_ms + 0.5) * 10000ULL + 116444736000000000ULL;
        ft.dwLowDateTime = time.LowPart; ft.dwHighDateTime = time.HighPart;
        bind_text(c, s, 4, iso_time(c, ft));
    }
    bind_value(c, s, 5, lookup(&r->front, "desc"), NULL);
    bind_value(c, s, 6, lookup(&r->front, "tags"), NULL);
    sqlite3_bind_int(s, 7, 0);
    bind_text(c, s, 8, r->path); bind_text(c, s, 9, r->hash);
    for (i = 0; i < table->field_count; i++) bind_value(c, s, 10 + i, lookup(&r->front, table->fields[i].name), &table->fields[i]);
    finish(c, s);
    index_record(c, r);
    c->projected++;
}
static void edge(context *c, record *r, field *f, value *v) {
    const char *slash = strchr(v->text, '/');
    const char *target_collection, *target_name;
    sqlite3_stmt *s, *find;
    int i;
    collection *declared = NULL;
    if (slash) { target_collection = copy_text(c, v->text, slash - v->text); target_name = slash + 1; }
    else if (!strcmp(f->name, "depends_on") && *v->text) { target_collection = r->owner->name; target_name = v->text; }
    else fail(c, "Invalid reference %s: %s", v->text, r->path);
    for (i = 0; i < c->collection_count; i++) if (!strcmp(c->collections[i].name, f->reference)) declared = &c->collections[i];
    if (!declared) fail(c, "Reference to undeclared collection: %s", f->reference);
    s = prepare(c, "INSERT INTO __attendant_edges VALUES (?,?,?,?,?,?,?)");
    bind_text(c, s, 1, r->owner->name); bind_text(c, s, 2, r->path);
    bind_text(c, s, 3, lookup(&r->front, "id")->text); bind_text(c, s, 4, f->name);
    bind_text(c, s, 5, target_collection); bind_text(c, s, 6, target_name);
    if (!strcmp(target_collection, declared->name) || (!strcmp(f->name, "depends_on") && !strcmp(target_collection, r->owner->name))) {
        const char *table = !strcmp(target_collection, declared->name) ? declared->name : r->owner->name;
        find = prepare(c, format_text(c, "SELECT source_path FROM %s WHERE name=?", identifier(c, table)));
        bind_text(c, find, 1, target_name);
        if (sqlite3_step(find) == SQLITE_ROW) bind_text(c, s, 7, (const char *)sqlite3_column_text(find, 0));
        if (finalize_builder(c, find) != SQLITE_OK) fail(c, "Cannot finish reference lookup");
    }
    finish(c, s);
}
static void make_edges(context *c) {
    int i, j, k;
    for (i = 0; i < c->record_count; i++) {
        record *r = &c->records[i];
        value *hidden = lookup(&r->front, "hidden");
        if (r->blocked || (hidden && hidden->number.real)) continue;
        for (j = 0; j < r->owner->field_count; j++) {
            field *f = &r->owner->fields[j];
            value *v = lookup(&r->front, f->name);
            if (!v || v->kind == VALUE_NULL) continue;
            if (!strcmp(f->kind, "ref")) edge(c, r, f, v);
            if (!strcmp(f->kind, "ref-array")) for (k = 0; k < v->count; k++) edge(c, r, f, &v->items[k]);
        }
    }
}
static const char *schema_digest(context *c, collection *table) {
    size_t a = strlen(table->name), b = strlen(table->directory), d = strlen(table->schema_text);
    char *joined = allocate(c, a + b + d + 5);
    /* Current runner hashes literal backslash-zero separators, not NUL. */
    memcpy(joined, table->name, a); memcpy(joined + a, "\\0", 2);
    memcpy(joined + a + 2, table->directory, b); memcpy(joined + a + b + 2, "\\0", 2);
    memcpy(joined + a + b + 4, table->schema_text, d + 1);
    return digest(c, joined);
}

static void load_source(context *c, const char *root) {
    int i;
    load_config(c, root);
    c->contract_diagnostic_count = c->diagnostic_count;
    for (i = 0; i < c->collection_count; i++) load_records(c, &c->collections[i]);
    validate_dataset(c);
}
static void reload_records(context *c) {
    int i;
    c->record_count = 0; c->diagnostic_count = c->contract_diagnostic_count;
    memset(c->records, 0, sizeof(c->records));
    for (i = 0; i < c->collection_count; i++) load_records(c, &c->collections[i]);
    validate_dataset(c);
}
#include "source-write.h"
#include "prepare.h"
#include "read.h"

#ifndef ATTENDANT_EMBEDDED
int main(int argc, char **argv) {
    static unsigned char backing[128 * 1024 * 1024];
    static context c;
    const char *root, *state;
    int i;
    active_context = &c;
    c.memory = arena_from_buffer(backing, sizeof(backing));
    {
        LPWSTR *arguments = CommandLineToArgvW(GetCommandLineW(), &argc);
        if (!arguments) fail(&c, "Cannot read Windows command line");
        argv = allocate(&c, (argc + 1) * sizeof(char *));
        for (i = 0; i < argc; i++) argv[i] = (char *)utf8(&c, arguments[i]);
        LocalFree(arguments);
    }
    if (argc < 2 || !strcmp(argv[1], "--help") || !strcmp(argv[1], "-h")) return native_help();
    if (!strcmp(argv[1], "doctor") || !strcmp(argv[1], "validate") || !strcmp(argv[1], "add-table") || !strcmp(argv[1], "create") || !strcmp(argv[1], "update") ||
        !strcmp(argv[1], "sync") || !strcmp(argv[1], "schema") || !strcmp(argv[1], "query") || !strcmp(argv[1], "search"))
        return native_read_main(&c, argc, argv);
    if (argc != 2) { c.error_code = "cli-command"; fail(&c, "Expected a native command or one fixture-project path"); }
    root = full_path(&c, ".", argv[1]);
    state = prepare_projection(&c, root, PROJECTION_REBUILD);
    print_sync(&c, state);
    return 0;
}
#endif
