/* Projection-only Windows exploration. Source is read-only. See README.md.
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

typedef struct value value;
struct value {
    enum { VALUE_NULL, VALUE_STRING, VALUE_NUMBER, VALUE_BOOL, VALUE_ARRAY } kind;
    const char *text;
    double number;
    value *items;
    int count;
};
typedef struct pair { const char *key; value data; } pair;
typedef struct mapping { pair *pairs; int count; } mapping;
typedef struct field {
    const char *name, *kind, *reference;
    int optional;
    value declaration, fallback;
} field;
typedef struct collection {
    const char *name, *directory, *schema_path, *schema_text;
    field fields[MAX_FIELDS];
    int field_count;
} collection;
typedef struct record {
    collection *owner;
    const char *path, *text, *body, *hash, *front_json;
    mapping front;
    double mtime_ms;
    size_t size;
} record;
typedef struct context {
    arena *memory;
    sqlite3 *db;
    const char *temporary;
    collection collections[MAX_COLLECTIONS];
    int collection_count;
    record records[MAX_RECORDS];
    int record_count, projected;
    char json_scratch[TEXT_CAPACITY];
} context;

static void json_check(jsonbcode status) {
    if (status < 0) { fprintf(stderr, "JSON output capacity exhausted\n"); exit(1); }
}
static void fail(context *c, const char *format, ...) {
    char message[2048], output[4096];
    jsonb b;
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    fprintf(stderr, "%s\n", message);
    if (c->db) { sqlite3_close_v2(c->db); c->db = NULL; }
    if (c->temporary) {
        wchar_t path[PATH_CAPACITY];
        if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, c->temporary, -1, path, PATH_CAPACITY)) DeleteFileW(path);
    }
    jsonb_init(&b);
    json_check(jsonb_object(&b, output, sizeof(output)));
    json_check(jsonb_key(&b, output, sizeof(output), "error", 5));
    json_check(jsonb_object(&b, output, sizeof(output)));
    json_check(jsonb_key(&b, output, sizeof(output), "code", 4));
    json_check(jsonb_string(&b, output, sizeof(output), "native-projection", 17));
    json_check(jsonb_key(&b, output, sizeof(output), "message", 7));
    json_check(jsonb_string(&b, output, sizeof(output), message, strlen(message)));
    json_check(jsonb_object_pop(&b, output, sizeof(output)));
    json_check(jsonb_object_pop(&b, output, sizeof(output)));
    printf("%.*s\n", (int)b.pos, output);
    exit(1);
}
static void *allocate(context *c, size_t size) {
    arena_result result = arena_alloc(c->memory, size, 16);
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
static const char *read_text(context *c, const char *path, size_t *length) {
    HANDLE file = CreateFileW(wide(c, path), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                              NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    LARGE_INTEGER size;
    DWORD count;
    char *text;
    if (file == INVALID_HANDLE_VALUE) fail(c, "Cannot read %s", path);
    if (!GetFileSizeEx(file, &size) || size.QuadPart < 0 || size.QuadPart > TEXT_CAPACITY) {
        CloseHandle(file); fail(c, "Source exceeds 4 MiB capacity: %s", path);
    }
    text = allocate(c, (size_t)size.QuadPart + 1);
    if (!ReadFile(file, text, (DWORD)size.QuadPart, &count, NULL) || count != size.QuadPart) {
        CloseHandle(file); fail(c, "Read failed: %s", path);
    }
    CloseHandle(file);
    if (memchr(text, 0, count) || !string_view_utf8_valid(string_view_from_cstr(text))) fail(c, "Source must be NUL-free UTF-8: %s", path);
    if (length) *length = count;
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
static int scalar_equal(value a, value b) {
    if (a.kind != b.kind) return 0;
    if (a.kind == VALUE_STRING) return !strcmp(a.text, b.text);
    if (a.kind == VALUE_NUMBER || a.kind == VALUE_BOOL) return a.number == b.number;
    return a.kind == VALUE_NULL;
}
static value scalar(context *c, yaml_node_t *node, const char *path) {
    value result = {0};
    char *end;
    const char *text = (const char *)node->data.scalar.value;
    if (node->data.scalar.length != strlen(text)) fail(c, "NUL in YAML scalar: %s", path);
    result.kind = VALUE_STRING;
    result.text = copy_text(c, text, strlen(text));
    if (node->data.scalar.style != YAML_PLAIN_SCALAR_STYLE) return result;
    if (!*text || !strcmp(text, "~") || !_stricmp(text, "null")) result.kind = VALUE_NULL;
    else if (!_stricmp(text, "true") || !_stricmp(text, "false")) {
        result.kind = VALUE_BOOL; result.number = !_stricmp(text, "true");
    } else if ((*text >= '0' && *text <= '9') || *text == '-' || *text == '+' || *text == '.') {
        /* C locale is the process default. This slice covers decimal numbers only. */
        double number;
        if (!_stricmp(text, ".nan") || !_stricmp(text, ".inf") || !_stricmp(text, "+.inf") || !_stricmp(text, "-.inf"))
            fail(c, "Nonfinite YAML number unsupported: %s", path);
        number = strtod(text, &end);
        if (end != text && !*end) {
            if (!isfinite(number)) fail(c, "Nonfinite YAML number unsupported: %s", path);
            result.kind = VALUE_NUMBER; result.number = number;
        } else if (!strncmp(text, "0x", 2) || !strncmp(text, "0o", 2) || !_stricmp(text, ".nan") || !_stricmp(text, ".inf"))
            fail(c, "Non-decimal YAML number unsupported in first slice: %s", path);
    }
    return result;
}
static mapping parse_mapping(context *c, const char *text, const char *path, const char **body) {
    const char *start, *line, *end = NULL;
    yaml_parser_t parser;
    yaml_document_t document;
    yaml_node_t *root;
    yaml_node_pair_t *entry;
    mapping result = {0};
    if (!strncmp(text, "---\r\n", 5)) start = text + 5;
    else if (!strncmp(text, "---\n", 4)) start = text + 4;
    else { fail(c, "Missing opening front matter delimiter: %s", path); return result; }
    for (line = start; *line; ) {
        const char *next = strchr(line, '\n');
        size_t length = next ? (size_t)(next - line) : strlen(line);
        if (length && line[length - 1] == '\r') length--;
        if (length == 3 && !memcmp(line, "---", 3)) {
            end = line;
            if (body) *body = next ? next + 1 : line + strlen(line);
            break;
        }
        if (!next) break;
        line = next + 1;
    }
    if (!end) fail(c, "Missing closing front matter delimiter: %s", path);
    if (!yaml_parser_initialize(&parser)) fail(c, "YAML parser allocation failed");
    yaml_parser_set_input_string(&parser, (const unsigned char *)start, end - start);
    if (!yaml_parser_load(&parser, &document)) {
        char problem[512];
        snprintf(problem, sizeof(problem), "%s at line %lu", parser.problem ? parser.problem : "parse failure", (unsigned long)parser.problem_mark.line + 2);
        yaml_parser_delete(&parser); fail(c, "YAML %s: %s", problem, path);
    }
    root = yaml_document_get_root_node(&document);
    if (!root || root->type != YAML_MAPPING_NODE) fail(c, "Front matter must be a mapping: %s", path);
    /* Reject aliases, anchors and explicit tags in this bounded first slice.
       LibYAML's document loader otherwise resolves aliases before subset checks. */
    {
        yaml_parser_t scanner;
        yaml_token_t token;
        int done = 0;
        if (!yaml_parser_initialize(&scanner)) fail(c, "YAML scanner allocation failed");
        yaml_parser_set_input_string(&scanner, (const unsigned char *)start, end - start);
        while (!done) {
            if (!yaml_parser_scan(&scanner, &token)) fail(c, "YAML scanning failed: %s", path);
            if (token.type == YAML_ALIAS_TOKEN || token.type == YAML_ANCHOR_TOKEN || token.type == YAML_TAG_TOKEN || token.type == YAML_DOCUMENT_START_TOKEN)
                fail(c, "Aliases, anchors, tags and extra documents unsupported: %s", path);
            done = token.type == YAML_STREAM_END_TOKEN;
            yaml_token_delete(&token);
        }
        yaml_parser_delete(&scanner);
    }
    result.pairs = allocate(c, MAX_PAIRS * sizeof(pair));
    for (entry = root->data.mapping.pairs.start; entry < root->data.mapping.pairs.top; entry++) {
        yaml_node_t *key = yaml_document_get_node(&document, entry->key);
        yaml_node_t *node = yaml_document_get_node(&document, entry->value);
        pair *p;
        value key_value;
        if (result.count == MAX_PAIRS) fail(c, "Too many front matter keys: %s", path);
        if (!key || key->type != YAML_SCALAR_NODE) fail(c, "Invalid YAML key: %s", path);
        key_value = scalar(c, key, path);
        if (key_value.kind != VALUE_STRING || !*key_value.text) fail(c, "YAML key must be nonempty string: %s", path);
        if (lookup(&result, key_value.text)) fail(c, "Duplicate YAML key %s: %s", key_value.text, path);
        p = &result.pairs[result.count++];
        p->key = key_value.text;
        if (node->type == YAML_SCALAR_NODE) p->data = scalar(c, node, path);
        else if (node->type == YAML_SEQUENCE_NODE) {
            yaml_node_item_t *item;
            p->data.kind = VALUE_ARRAY;
            p->data.count = (int)(node->data.sequence.items.top - node->data.sequence.items.start);
            if (p->data.count > MAX_ITEMS) fail(c, "YAML array capacity exceeded: %s", path);
            p->data.items = allocate(c, (p->data.count + 1) * sizeof(value));
            for (item = node->data.sequence.items.start; item < node->data.sequence.items.top; item++) {
                yaml_node_t *child = yaml_document_get_node(&document, *item);
                if (child->type != YAML_SCALAR_NODE) fail(c, "Nested YAML unsupported: %s", path);
                p->data.items[item - node->data.sequence.items.start] = scalar(c, child, path);
            }
        } else fail(c, "Nested YAML unsupported: %s", path);
    }
    yaml_document_delete(&document);
    yaml_parser_delete(&parser);
    return result;
}
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
static void json_value(context *c, jsonb *b, char *output, size_t capacity, value v) {
    int i;
    if (v.kind == VALUE_STRING) json_check(jsonb_string(b, output, capacity, v.text, strlen(v.text)));
    else if (v.kind == VALUE_NUMBER) json_number(c, b, output, capacity, v.number);
    else if (v.kind == VALUE_BOOL) json_check(jsonb_bool(b, output, capacity, (int)v.number));
    else if (v.kind == VALUE_NULL) json_check(jsonb_null(b, output, capacity));
    else {
        json_check(jsonb_array(b, output, capacity));
        for (i = 0; i < v.count; i++) json_value(c, b, output, capacity, v.items[i]);
        json_check(jsonb_array_pop(b, output, capacity));
    }
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
static const char *mapping_json(context *c, mapping *map) {
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
        json_value(c, &b, buffer, TEXT_CAPACITY, entry->data);
    }
    json_check(jsonb_object_pop(&b, buffer, TEXT_CAPACITY));
    return copy_text(c, buffer, b.pos);
}
static void sql(context *c, const char *statement) {
    char *error = NULL;
    if (sqlite3_exec(c->db, statement, NULL, NULL, &error) != SQLITE_OK) fail(c, "SQLite: %s", error ? error : sqlite3_errmsg(c->db));
}
static sqlite3_stmt *prepare(context *c, const char *statement) {
    sqlite3_stmt *result;
    if (sqlite3_prepare_v2(c->db, statement, -1, &result, NULL) != SQLITE_OK) fail(c, "SQLite prepare: %s", sqlite3_errmsg(c->db));
    return result;
}
static void bind_text(context *c, sqlite3_stmt *s, int i, const char *text) {
    if (sqlite3_bind_text(s, i, text, -1, SQLITE_TRANSIENT) != SQLITE_OK) fail(c, "SQLite bind failed");
}
static void finish(context *c, sqlite3_stmt *s) {
    if (sqlite3_step(s) != SQLITE_DONE) fail(c, "SQLite step: %s", sqlite3_errmsg(c->db));
    if (sqlite3_finalize(s) != SQLITE_OK) fail(c, "SQLite finalize failed");
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
static void load_schema(context *c, collection *table) {
    mapping schema = parse_mapping(c, table->schema_text, table->schema_path, NULL);
    int i, j;
    if (schema.count > MAX_FIELDS) fail(c, "Schema exceeds field capacity");
    for (i = 0; i < schema.count; i++) {
        field *f = &table->fields[table->field_count++];
        value v = schema.pairs[i].data;
        f->name = schema.pairs[i].key;
        for (j = 0; j < (int)(sizeof(builtins) / sizeof(builtins[0])); j++)
            if (!strcmp(f->name, builtins[j])) fail(c, "Reserved schema field: %s", f->name);
        f->declaration = v;
        if (v.kind == VALUE_NULL) { f->kind = "untyped"; f->optional = 1; }
        else if (v.kind == VALUE_ARRAY) {
            if (!v.count) fail(c, "Empty schema enum: %s", f->name);
            for (j = 0; j < v.count; j++) if (v.items[j].kind == VALUE_NULL) fail(c, "Null enum member: %s", f->name);
            f->kind = "enum"; f->fallback = v.items[0];
        } else if (v.kind == VALUE_STRING) {
            const char *marker = v.text;
            if (*marker == '?') { f->optional = 1; marker++; }
            if (!strcmp(marker, "date") || !strcmp(marker, "datetime")) f->kind = marker;
            else if (!strcmp(marker, "string[]")) f->kind = "string-array";
            else if (!strncmp(marker, "ref:", 4)) { f->kind = "ref"; f->reference = marker + 4; }
            else if (!strncmp(marker, "ref[]:", 6)) { f->kind = "ref-array"; f->reference = marker + 6; }
            else {
                if (f->optional) fail(c, "Unsupported optional schema marker: %s", marker);
                f->kind = "default"; f->fallback = v;
            }
        } else { f->kind = "default"; f->fallback = v; }
    }
}
static void load_config(context *c, const char *root) {
    const char *path = full_path(c, root, ".pi/attendant.tables");
    const char *source = read_text(c, path, NULL);
    char *config = copy_text(c, source, strlen(source));
    char *line = config;
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
        alias = NULL;
        {
            char *p;
            for (p = directory; *p; p++) {
                if ((*p == ' ' || *p == '\t') && p[1] == 'a' && p[2] == 's' && (p[3] == ' ' || p[3] == '\t')) { alias = p; break; }
            }
        }
        if (alias) { *alias = 0; name = trim(alias + 3); directory = trim(directory); }
        else {
            char *base = copy_text(c, directory, strlen(directory));
            size_t length = strlen(base);
            char *a, *b;
            while (length && (base[length - 1] == '/' || base[length - 1] == '\\')) base[--length] = 0;
            a = strrchr(base, '/'); b = strrchr(base, '\\');
            name = a && (!b || a > b) ? a + 1 : b ? b + 1 : base;
        }
        if (!*name || strchr(name, '/') || !strncmp(name, "__attendant_", 12)) fail(c, "Invalid collection name: %s", name);
        for (i = 0; i < c->collection_count; i++) if (!strcmp(name, c->collections[i].name)) fail(c, "Duplicate collection: %s", name);
        table = &c->collections[c->collection_count++];
        table->name = name;
        /* loadContract preserves spelling of absolute configured paths. Its
           schema hash includes that spelling even when file paths normalize. */
        table->directory = absolute_path(directory) ? directory : full_path(c, root, directory);
        table->schema_path = full_path(c, table->directory, ".schema.md");
        table->schema_text = read_text(c, table->schema_path, NULL);
        load_schema(c, table);
    }
}
static const char *iso_time(context *c, FILETIME filetime) {
    SYSTEMTIME time;
    if (!FileTimeToSystemTime(&filetime, &time)) fail(c, "Invalid file timestamp");
    return format_text(c, "%04u-%02u-%02uT%02u:%02u:%02u.%03uZ", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);
}
static const char *normalized_datetime(context *c, const char *text) {
    /* ponytail: normalized UTC date-times only for this read-only slice;
       support other runner-accepted RFC3339 forms before general parity. */
    size_t length = strlen(text);
    if (length == 20 && text[10] == 'T' && text[19] == 'Z') return format_text(c, "%.19s.000Z", text);
    if (length == 24 && text[10] == 'T' && text[19] == '.' && text[23] == 'Z') return text;
    fail(c, "First slice requires UTC timestamps with seconds and optional 3-digit milliseconds: %s", text);
    return NULL;
}
static value *required(context *c, record *r, const char *key, int kind) {
    value *v = lookup(&r->front, key);
    if (!v || v->kind != kind) fail(c, "Normalized record requires %s: %s", key, r->path);
    return v;
}
static void validate_record(context *c, record *r) {
    const char *id = required(c, r, "id", VALUE_STRING)->text;
    int i, j;
    required(c, r, "name", VALUE_STRING);
    normalized_datetime(c, required(c, r, "created_at", VALUE_STRING)->text);
    required(c, r, "desc", VALUE_STRING);
    {
        value tags = *required(c, r, "tags", VALUE_ARRAY);
        for (i = 0; i < tags.count; i++) if (tags.items[i].kind != VALUE_STRING) fail(c, "Invalid tags: %s", r->path);
    }
    if (strlen(id) != 36 || id[14] != '7' || !strchr("89abAB", id[19])) fail(c, "Expected UUIDv7: %s", r->path);
    for (i = 0; i < 36; i++) {
        if (i == 8 || i == 13 || i == 18 || i == 23) { if (id[i] != '-') fail(c, "Invalid UUID: %s", r->path); }
        else if (!strchr("0123456789abcdefABCDEF", id[i])) fail(c, "Invalid UUID: %s", r->path);
    }
    for (i = 0; i < r->owner->field_count; i++) {
        field *f = &r->owner->fields[i];
        value *v = lookup(&r->front, f->name);
        if (!v || v->kind == VALUE_NULL) {
            if (!v && (!strcmp(f->kind, "default") || !strcmp(f->kind, "enum") || !strcmp(f->kind, "string-array")))
                fail(c, "Fixture needs default normalization for %s: %s", f->name, r->path);
            if (!f->optional && strcmp(f->kind, "default") && strcmp(f->kind, "enum") && strcmp(f->kind, "string-array") && strcmp(f->kind, "untyped"))
                fail(c, "Missing required field %s: %s", f->name, r->path);
            continue;
        }
        if (!strcmp(f->kind, "default") && v->kind != f->fallback.kind) fail(c, "Incorrect scalar type %s: %s", f->name, r->path);
        if (!strcmp(f->kind, "enum")) {
            for (j = 0; j < f->declaration.count; j++) if (scalar_equal(*v, f->declaration.items[j])) break;
            if (j == f->declaration.count) fail(c, "Invalid enum %s: %s", f->name, r->path);
        }
        if (!strcmp(f->kind, "datetime")) {
            if (v->kind != VALUE_STRING) fail(c, "Expected datetime string: %s", r->path);
            normalized_datetime(c, v->text);
        }
        if (!strcmp(f->kind, "date") || !strcmp(f->kind, "ref")) {
            if (v->kind != VALUE_STRING) fail(c, "Expected string %s: %s", f->name, r->path);
        }
        if (!strcmp(f->kind, "ref-array") || !strcmp(f->kind, "string-array")) {
            if (v->kind != VALUE_ARRAY) fail(c, "Expected array %s: %s", f->name, r->path);
            for (j = 0; j < v->count; j++) if (v->items[j].kind != VALUE_STRING) fail(c, "Expected string item %s: %s", f->name, r->path);
        }
    }
    {
        value *hidden = lookup(&r->front, "hidden");
        if (hidden && hidden->kind != VALUE_BOOL) fail(c, "Invalid hidden field: %s", r->path);
    }
}
static void validate_dataset(context *c) {
    int i, j;
    for (i = 0; i < c->record_count; i++) {
        record *r = &c->records[i];
        for (j = 0; j < i; j++) {
            record *prior = &c->records[j];
            if (prior->owner != r->owner) continue;
            if (!strcmp(lookup(&r->front, "id")->text, lookup(&prior->front, "id")->text) ||
                !strcmp(lookup(&r->front, "name")->text, lookup(&prior->front, "name")->text))
                fail(c, "First slice requires normalized unique IDs and names: %s", r->path);
        }
    }
}
static int compare_paths(const void *a, const void *b) { return strcmp(*(const char *const *)a, *(const char *const *)b); }
static void load_records(context *c, collection *table) {
    WIN32_FIND_DATAW entry;
    HANDLE search = FindFirstFileW(wide(c, format_text(c, "%s\\*", table->directory)), &entry);
    const char *paths[MAX_RECORDS];
    int count = 0, i;
    if (search == INVALID_HANDLE_VALUE) fail(c, "Cannot scan collection: %s", table->directory);
    do {
        const char *name = utf8(c, entry.cFileName);
        size_t length = strlen(name);
        if ((entry.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) || *name == '.' || length < 3 || strcmp(name + length - 3, ".md")) continue;
        if (count == MAX_RECORDS) fail(c, "Record capacity exceeded");
        paths[count++] = full_path(c, table->directory, name);
    } while (FindNextFileW(search, &entry));
    if (GetLastError() != ERROR_NO_MORE_FILES) { FindClose(search); fail(c, "Directory enumeration failed"); }
    FindClose(search);
    qsort(paths, count, sizeof(paths[0]), compare_paths);
    for (i = 0; i < count; i++) {
        WIN32_FILE_ATTRIBUTE_DATA attributes;
        ULARGE_INTEGER ticks;
        record *r;
        if (c->record_count == MAX_RECORDS) fail(c, "Total record capacity exceeded");
        r = &c->records[c->record_count++];
        r->owner = table; r->path = paths[i];
        r->text = read_text(c, r->path, &r->size);
        if (!GetFileAttributesExW(wide(c, r->path), GetFileExInfoStandard, &attributes)) fail(c, "Cannot stat %s", r->path);
        ticks.LowPart = attributes.ftLastWriteTime.dwLowDateTime; ticks.HighPart = attributes.ftLastWriteTime.dwHighDateTime;
        ticks.QuadPart -= 116444736000000000ULL;
        /* Match Node/libuv's seconds + nanosecond fraction arithmetic. */
        r->mtime_ms = (double)(ticks.QuadPart / 10000000ULL) * 1000.0 + (double)(ticks.QuadPart % 10000000ULL) / 10000.0;
        r->front = parse_mapping(c, r->text, r->path, &r->body);
        validate_record(c, r);
        r->hash = digest(c, r->text);
        r->front_json = mapping_json(c, &r->front);
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
    sql(c, format_text(c, "CREATE VIRTUAL TABLE %s USING fts5(record_id UNINDEXED, content, tokenize = 'unicode61 remove_diacritics 2')", internal(c, "fts", table)));
    sql(c, format_text(c, "CREATE VIRTUAL TABLE %s USING fts5(record_id UNINDEXED, content, tokenize = 'trigram')", internal(c, "trigram", table)));
}
static void bind_value(context *c, sqlite3_stmt *s, int index, value *v, field *f) {
    if (!v || v->kind == VALUE_NULL) sqlite3_bind_null(s, index);
    else if (v->kind == VALUE_ARRAY) bind_text(c, s, index, value_json(c, *v));
    else if (v->kind == VALUE_STRING) bind_text(c, s, index, f && !strcmp(f->kind, "datetime") ? normalized_datetime(c, v->text) : v->text);
    else if (v->kind == VALUE_BOOL) sqlite3_bind_int(s, index, (int)v->number);
    else sqlite3_bind_double(s, index, v->number);
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
    bind_text(c, s, 7, "[]"); bind_text(c, s, 8, r->text); finish(c, s);
    if (hidden && hidden->number) return;
    for (i = 1; i < 9 + table->field_count; i++) strcat(placeholders, ",?");
    s = prepare(c, format_text(c, "INSERT INTO %s VALUES (%s)", identifier(c, table->name), placeholders));
    bind_value(c, s, 1, lookup(&r->front, "id"), NULL);
    bind_value(c, s, 2, lookup(&r->front, "name"), NULL);
    bind_text(c, s, 3, normalized_datetime(c, lookup(&r->front, "created_at")->text));
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
    for (i = 0; i < 2; i++) {
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
        sqlite3_finalize(find);
    }
    finish(c, s);
}
static void make_edges(context *c) {
    int i, j, k;
    for (i = 0; i < c->record_count; i++) {
        record *r = &c->records[i];
        value *hidden = lookup(&r->front, "hidden");
        if (hidden && hidden->number) continue;
        for (j = 0; j < r->owner->field_count; j++) {
            field *f = &r->owner->fields[j];
            value *v = lookup(&r->front, f->name);
            if (!v || v->kind == VALUE_NULL) continue;
            if (!strcmp(f->kind, "ref")) edge(c, r, f, v);
            if (!strcmp(f->kind, "ref-array")) for (k = 0; k < v->count; k++) edge(c, r, f, &v->items[k]);
        }
    }
}
int main(int argc, char **argv) {
    static unsigned char backing[128 * 1024 * 1024];
    static context c;
    const char *root, *directory, *target;
    int i;
    c.memory = arena_from_buffer(backing, sizeof(backing));
    {
        LPWSTR *arguments = CommandLineToArgvW(GetCommandLineW(), &argc);
        if (!arguments) fail(&c, "Cannot read Windows command line");
        argv = allocate(&c, (argc + 1) * sizeof(char *));
        for (i = 0; i < argc; i++) argv[i] = (char *)utf8(&c, arguments[i]);
        LocalFree(arguments);
    }
    if (argc == 2 && (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h"))) {
        puts("Usage: projection.exe <fixture-project>\nRead-only source; generates compatible .attendant/attendant.sqlite. Windows first-slice exploration only.");
        return 0;
    }
    if (argc != 2) fail(&c, "Usage: projection.exe <fixture-project>");
    root = full_path(&c, ".", argv[1]);
    load_config(&c, root);
    for (i = 0; i < c.collection_count; i++) load_records(&c, &c.collections[i]);
    validate_dataset(&c);
    directory = full_path(&c, root, ".attendant");
    if (!CreateDirectoryW(wide(&c, directory), NULL) && GetLastError() != ERROR_ALREADY_EXISTS) fail(&c, "Cannot create projection directory");
    target = full_path(&c, directory, "attendant.sqlite");
    {
        const char *temporary = full_path(&c, directory, format_text(&c, "native-%lu.sqlite", (unsigned long)GetCurrentProcessId()));
        if (GetFileAttributesW(wide(&c, temporary)) != INVALID_FILE_ATTRIBUTES) fail(&c, "Temporary database already exists");
        c.temporary = temporary;
    }
    if (sqlite3_open(c.temporary, &c.db) != SQLITE_OK) fail(&c, "Cannot create SQLite database");
    sql(&c, "PRAGMA journal_mode=DELETE; PRAGMA synchronous=FULL; BEGIN IMMEDIATE;");
    sql(&c, "CREATE TABLE __attendant_diagnostics (collection_name TEXT, source_path TEXT NOT NULL, code TEXT NOT NULL, message TEXT NOT NULL);");
    sql(&c, "CREATE TABLE __attendant_metadata (key TEXT PRIMARY KEY, value TEXT NOT NULL);");
    sql(&c, "CREATE TABLE __attendant_cache (source_path TEXT PRIMARY KEY, collection_name TEXT NOT NULL, mtime_ms REAL NOT NULL, size INTEGER NOT NULL, source_hash TEXT NOT NULL, front_matter TEXT NOT NULL, local_diagnostics TEXT NOT NULL, content TEXT NOT NULL);");
    sql(&c, "CREATE TABLE __attendant_edges (source_collection TEXT NOT NULL, source_path TEXT NOT NULL, source_id TEXT NOT NULL, field TEXT NOT NULL, target_collection TEXT NOT NULL, target_name TEXT NOT NULL, target_path TEXT);");
    metadata(&c, "layout_version", "2");
    metadata(&c, "config_hash", digest(&c, read_text(&c, full_path(&c, root, ".pi/attendant.tables"), NULL)));
    for (i = 0; i < c.collection_count; i++) {
        collection *table = &c.collections[i];
        const char *schema_material;
        size_t a = strlen(table->name), b = strlen(table->directory), d = strlen(table->schema_text);
        char *joined = allocate(&c, a + b + d + 5);
        /* Current runner hashes literal backslash-zero separators, not NUL. */
        memcpy(joined, table->name, a); memcpy(joined + a, "\\0", 2);
        memcpy(joined + a + 2, table->directory, b); memcpy(joined + a + b + 2, "\\0", 2);
        memcpy(joined + a + b + 4, table->schema_text, d + 1); schema_material = joined;
        metadata(&c, format_text(&c, "schema_hash:%s", table->name), digest(&c, schema_material));
        make_tables(&c, table);
    }
    for (i = 0; i < c.record_count; i++) project_record(&c, &c.records[i]);
    make_edges(&c);
    sql(&c, "COMMIT;");
    if (sqlite3_close(c.db) != SQLITE_OK) fail(&c, "Cannot close database");
    c.db = NULL;
    /* Sequential fixture use only. Never replace an active WAL database. */
    if (GetFileAttributesW(wide(&c, format_text(&c, "%s-wal", target))) != INVALID_FILE_ATTRIBUTES ||
        GetFileAttributesW(wide(&c, format_text(&c, "%s-shm", target))) != INVALID_FILE_ATTRIBUTES)
        fail(&c, "Existing WAL sidecars: stop consumers and remove disposable projection before rebuilding");
    if (!MoveFileExW(wide(&c, c.temporary), wide(&c, target), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) fail(&c, "Cannot publish database (is another process using it?)");
    c.temporary = NULL;
    printf("{\"collections\":%d,\"records\":%d,\"diagnostics\":[],\"state\":\"rebuilt\"}\n", c.collection_count, c.projected);
    return 0;
}
