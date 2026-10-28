/* CLI commands share config, schema, arena, JSON and in-process preparation.
   Read preparation may correct source; incremental reconciliation is excluded. */
#include "vendor/libpdjson5/pdjson5.h"
#include <io.h>
#include <fcntl.h>

#define RESULT_LIMIT 10000
#define RESULT_BYTES (10 * 1024 * 1024)
#define COLUMN_LIMIT 1024

typedef struct read_options {
    const char *command, *project, *sql_text, *params, *query_text, *collections;
    const char *directory, *alias, *collection, *items;
    int limit, stdin_used, allow_fts, strict, no_correct;
    unsigned long progress_calls;
} read_options;
typedef struct result_rows {
    const char *columns[COLUMN_LIMIT];
    int indexes[COLUMN_LIMIT], column_count, count, truncated;
    const char *rows[RESULT_LIMIT];
    size_t bytes;
} result_rows;

static int native_help(void) {
    puts("{\"usage\":\"projection.exe <doctor|validate|add-table|create|update|sync|schema|query|search> [options]; projection.exe <fixture-project> builds projection\","
         "\"project\":\"--project, -p <path>; defaults to current directory\","
         "\"doctor\":\"read-only health checks; no Git checks; exits 1 when unhealthy\","
         "\"validate\":\"[--no-correct|--strict]; default applies mechanical corrections\","
         "\"add-table\":\"--directory, -d <path> [--alias, -a <name>]\","
         "\"create/update\":\"--collection, -c <name> --items, -i <JSON array>\","
         "\"query\":\"--sql, -s <sql> [--params, -P <JSON object>] [--limit, -l <integer>]\","
         "\"search\":\"--query, -q <text> [--collections, -c <comma-separated names>] [--limit, -l <integer>]\","
         "\"input\":\"@path reads UTF-8; - reads stdin once for text options\","
         "\"projection\":\"automatic reuse/rebuild; reads apply reported mechanical corrections; malformed records diagnosed/skipped\","
         "\"output\":\"one JSON value on stdout; errors also explained on stderr; exits 0/1\"}");
    return 0;
}
static const char *option_input(context *c, read_options *options, const char *text) {
    if (!strcmp(text, "-")) {
        size_t size;
        char *input;
        if (options->stdin_used++) { c->error_code = "cli-input"; fail(c, "stdin may be used for only one option"); }
        input = allocate(c, TEXT_CAPACITY + 1);
        _setmode(_fileno(stdin), _O_BINARY);
        size = fread(input, 1, TEXT_CAPACITY, stdin);
        if (ferror(stdin) || (size == TEXT_CAPACITY && fgetc(stdin) != EOF)) fail(c, "stdin read failed or exceeds 4 MiB");
        if (memchr(input, 0, size) || !string_view_utf8_valid(string_view_from_cstr(input))) fail(c, "stdin must be NUL-free UTF-8");
        return input;
    }
    if (*text == '@') {
        if (!text[1]) fail(c, "@file requires a path");
        return read_text(c, text + 1, NULL);
    }
    return text;
}
static read_options parse_read_options(context *c, int argc, char **argv) {
    read_options result = {0};
    int i, limit_seen = 0;
    result.command = argv[1]; result.limit = 100;
    c->error_code = "cli-option";
    for (i = 2; i < argc; i++) {
        const char *arg = argv[i], **destination = NULL;
        if (!strcmp(arg, "--help") || !strcmp(arg, "-h")) { native_help(); exit(0); }
        if (!strcmp(result.command, "validate") && (!strcmp(arg, "--strict") || !strcmp(arg, "--no-correct"))) {
            int *flag = !strcmp(arg, "--strict") ? &result.strict : &result.no_correct;
            if (*flag) fail(c, "Option may be supplied only once: %s", arg);
            *flag = 1; continue;
        }
        if (!strcmp(arg, "--project") || !strcmp(arg, "-p")) destination = &result.project;
        else if (!strcmp(result.command, "add-table") && (!strcmp(arg, "--directory") || !strcmp(arg, "-d"))) destination = &result.directory;
        else if (!strcmp(result.command, "add-table") && (!strcmp(arg, "--alias") || !strcmp(arg, "-a"))) destination = &result.alias;
        else if ((!strcmp(result.command, "create") || !strcmp(result.command, "update")) && (!strcmp(arg, "--collection") || !strcmp(arg, "-c"))) destination = &result.collection;
        else if ((!strcmp(result.command, "create") || !strcmp(result.command, "update")) && (!strcmp(arg, "--items") || !strcmp(arg, "-i"))) destination = &result.items;
        else if (!strcmp(result.command, "query") && (!strcmp(arg, "--sql") || !strcmp(arg, "-s"))) destination = &result.sql_text;
        else if (!strcmp(result.command, "query") && (!strcmp(arg, "--params") || !strcmp(arg, "-P"))) destination = &result.params;
        else if (!strcmp(result.command, "search") && (!strcmp(arg, "--query") || !strcmp(arg, "-q"))) destination = &result.query_text;
        else if (!strcmp(result.command, "search") && (!strcmp(arg, "--collections") || !strcmp(arg, "-c"))) destination = &result.collections;
        else if ((!strcmp(result.command, "query") || !strcmp(result.command, "search")) && (!strcmp(arg, "--limit") || !strcmp(arg, "-l"))) {
            const char *p;
            unsigned long limit = 0;
            if (limit_seen++) fail(c, "--limit may be supplied only once");
            if (++i == argc || !*argv[i]) fail(c, "--limit requires a non-negative integer");
            for (p = argv[i]; *p; p++) {
                if (*p < '0' || *p > '9') fail(c, "--limit requires a non-negative integer");
                if (limit < RESULT_LIMIT) limit = limit * 10 + *p - '0';
            }
            result.limit = limit > RESULT_LIMIT ? RESULT_LIMIT : (int)limit;
            continue;
        } else fail(c, "Unknown or unsupported option for %s: %s", result.command, arg);
        if (*destination) fail(c, "Option may be supplied only once: %s", arg);
        if (++i == argc || !*argv[i] || !strncmp(argv[i], "--", 2)) fail(c, "Option requires a value: %s", arg);
        *destination = argv[i];
    }
    c->error_code = "cli-input";
    if (!strcmp(result.command, "query") && !result.sql_text) fail(c, "--sql is required for query");
    if (!strcmp(result.command, "search") && !result.query_text) fail(c, "--query is required for search");
    result.project = full_path(c, ".", result.project ? result.project : ".");
    if (!strcmp(result.command, "add-table") && !result.directory) fail(c, "--directory is required for add-table");
    if ((!strcmp(result.command, "create") || !strcmp(result.command, "update")) && (!result.collection || !result.items)) fail(c, "--collection and --items are required");
    if (result.items) result.items = option_input(c, &result, result.items);
    if (result.sql_text) result.sql_text = option_input(c, &result, result.sql_text);
    if (result.params) result.params = option_input(c, &result, result.params);
    if (result.query_text) result.query_text = option_input(c, &result, result.query_text);
    if (result.collections) result.collections = option_input(c, &result, result.collections);
    return result;
}
static int read_authorizer(void *opaque, int action, const char *first, const char *second, const char *database, const char *source) {
    context *c = opaque;
    int i;
    (void)database; (void)source;
    if (action == SQLITE_READ) {
        if (!first) return SQLITE_DENY;
        if (!strcmp(first, "sqlite_schema") || !strcmp(first, "sqlite_master") || !strcmp(first, "__attendant_diagnostics") || !strcmp(first, "__attendant_edges")) return SQLITE_OK;
        for (i = 0; i < c->collection_count; i++) {
            const char *name = c->collections[i].name;
            if (!strcmp(first, name)) return SQLITE_OK;
            /* No allocation in authorizer: SQLite can call it for every column. */
            if (!strncmp(first, "__attendant_fts_", 16) && !strcmp(first + 16, name)) return SQLITE_OK;
            if (c->allow_fts && !strncmp(first, "__attendant_trigram_", 20) && !strcmp(first + 20, name)) return SQLITE_OK;
            if (c->allow_fts) {
                const char *prefixes[] = {"__attendant_fts_", "__attendant_trigram_"};
                const char *suffixes[] = {"_config", "_content", "_data", "_docsize", "_idx"};
                int p, s;
                for (p = 0; p < 2; p++) {
                    size_t prefix = strlen(prefixes[p]), length = strlen(name);
                    if (strncmp(first, prefixes[p], prefix) || strncmp(first + prefix, name, length)) continue;
                    for (s = 0; s < 5; s++) if (!strcmp(first + prefix + length, suffixes[s])) return SQLITE_OK;
                }
            }
        }
        return SQLITE_DENY;
    }
    if (action == SQLITE_FUNCTION) {
        const char *name = second ? second : first;
        if (name && (!_stricmp(name, "load_extension") || !_stricmp(name, "readfile") || !_stricmp(name, "writefile") || !_stricmp(name, "eval"))) return SQLITE_DENY;
        return SQLITE_OK;
    }
    if (action == SQLITE_SELECT || action == SQLITE_RECURSIVE || action == SQLITE_PRAGMA) return SQLITE_OK;
    return SQLITE_DENY;
}
static int query_progress(void *opaque) {
    read_options *options = opaque;
    return ++options->progress_calls > 10000; /* 10 million VM instructions. */
}
static void open_snapshot(context *c, read_options *options) {
    const char *path = full_path(c, options->project, ".attendant/attendant.sqlite");
    c->error_code = "projection-open";
    if (projection_open(c, path, SQLITE_OPEN_READONLY) != SQLITE_OK) fail(c, "Cannot open prepared projection");
    sql(c, "PRAGMA query_only=ON; PRAGMA foreign_keys=ON; PRAGMA busy_timeout=0;");
    sqlite3_limit(c->db, SQLITE_LIMIT_LENGTH, OUTPUT_CAPACITY);
    sqlite3_limit(c->db, SQLITE_LIMIT_SQL_LENGTH, TEXT_CAPACITY);
    sqlite3_limit(c->db, SQLITE_LIMIT_COLUMN, COLUMN_LIMIT);
    sqlite3_limit(c->db, SQLITE_LIMIT_EXPR_DEPTH, 100);
    c->allow_fts = options->allow_fts;
    sqlite3_set_authorizer(c->db, read_authorizer, c);
    sqlite3_progress_handler(c->db, 1000, query_progress, options);
}
static void one_read_statement(context *c, const char *text) {
    const char *p;
    char quote = 0;
    int semicolon = 0, content = 0;
    c->error_code = "sql-shape";
    for (p = text; *p; p++) {
        if (quote) {
            if (*p == quote && p[1] == quote) p++;
            else if (*p == quote) quote = 0;
            continue;
        }
        if (semicolon) {
            if (*p != ' ' && *p != '\t' && *p != '\n' && *p != '\r') fail(c, "SQL must contain one statement");
            continue;
        }
        if ((*p == '-' && p[1] == '-') || (*p == '/' && p[1] == '*')) fail(c, "SQL comments are not supported");
        if (*p == '\'' || *p == '"' || *p == '`') quote = *p;
        else if (*p == ';') semicolon++;
        if (*p != ' ' && *p != '\t' && *p != '\n' && *p != '\r' && *p != ';') content = 1;
    }
    if (!content || quote) fail(c, "SQL must contain one non-empty statement");
}
static sqlite3_stmt *read_statement(context *c, const char *text) {
    sqlite3_stmt *s = NULL;
    const char *tail;
    int status = sqlite3_prepare_v2(c->db, text, -1, &s, &tail);
    if (status != SQLITE_OK) {
        c->error_code = status == SQLITE_AUTH ? "sql-denied" : "sql-execution";
        fail(c, "Query prepare failed: %s", sqlite3_errmsg(c->db));
    }
    if (!s) fail(c, "SQL did not produce a statement");
    while (*tail == ' ' || *tail == '\t' || *tail == '\r' || *tail == '\n' || *tail == ';') tail++;
    if (*tail) { c->error_code = "sql-shape"; fail(c, "SQL must contain one statement"); }
    if (!sqlite3_stmt_readonly(s)) { c->error_code = "sql-denied"; fail(c, "Only read-only SQL is allowed"); }
    return s;
}
static void *json_allocate(size_t size, void *opaque) {
    context *c = opaque;
    size_t *storage = allocate(c, size + 16);
    *storage = size;
    return (unsigned char *)storage + 16;
}
static void *json_reallocate(void *old, size_t size, void *opaque) {
    void *next = json_allocate(size, opaque);
    if (old) {
        size_t prior = *(size_t *)((unsigned char *)old - 16);
        memcpy(next, old, prior < size ? prior : size);
    }
    return next;
}
static void json_release(void *memory, size_t size, void *opaque) { (void)memory; (void)size; (void)opaque; }
typedef struct query_parameter {
    const char *name, *text;
    size_t length;
    enum pdjson_type type;
    value number;
} query_parameter;
static void query_parameters(context *c, sqlite3_stmt *statement, const char *text) {
    pdjson_stream parser;
    pdjson_allocator allocator = {json_allocate, json_reallocate, json_release};
    query_parameter parameters[MAX_PAIRS] = {{0}};
    enum pdjson_type token;
    int count = 0, events = 0, order[MAX_PAIRS], i, j;
    c->error_code = "cli-json";
    pdjson_open_string(&parser, text ? text : "{}");
    pdjson_set_allocator(&parser, &allocator, c);
    pdjson_set_language(&parser, PDJSON_LANGUAGE_JSON);
    pdjson_set_streaming(&parser, false);
    if (pdjson_next(&parser) != PDJSON_OBJECT) fail(c, "--params must be a strict JSON object");
    while ((token = pdjson_next(&parser)) == PDJSON_NAME) {
        size_t length;
        const char *name = pdjson_get_name(&parser, &length);
        query_parameter *p;
        if (++events > MAX_PAIRS) fail(c, "Too many query parameters");
        if (strlen(name) + 1 != length) fail(c, "Parameter names must not contain NUL");
        for (j = 0; j < count; j++) if (!strcmp(parameters[j].name, name)) break;
        p = &parameters[j];
        if (j == count) { p->name = copy_text(c, name, length - 1); count++; }
        token = pdjson_peek(&parser);
        p->type = token == PDJSON_OBJECT || token == PDJSON_ARRAY ? pdjson_skip(&parser) : pdjson_next(&parser);
        if (p->type == PDJSON_STRING) {
            const char *value = pdjson_get_value(&parser, &length);
            p->text = copy_text(c, value, length - 1); p->length = length - 1;
        } else if (p->type == PDJSON_NUMBER) {
            if (parse_number(pdjson_get_value(&parser, NULL), &p->number) != NUMBER_VALID) fail(c, "Parameter number exceeds i64/u64 or finite f64 range");
        }
        else if (p->type == PDJSON_ERROR) fail(c, "Invalid JSON parameter value");
    }
    if (token != PDJSON_OBJECT_END || pdjson_next(&parser) != PDJSON_DONE) fail(c, "Invalid or trailing JSON in --params");
    pdjson_close(&parser);
    /* JSON.parse keeps first insertion order and last duplicate value; JS
       integer-index keys enumerate first. Binding must happen after collapse. */
    for (i = 0; i < count; i++) {
        order[i] = i;
        for (j = i; j > 0 && object_index(parameters[order[j]].name) < object_index(parameters[order[j - 1]].name); j--) {
            int swap = order[j]; order[j] = order[j - 1]; order[j - 1] = swap;
        }
    }
    c->error_code = "sql-params";
    for (i = 0; i < count; i++) {
        enum pdjson_type type = parameters[i].type;
        if (type == PDJSON_NUMBER && parameters[i].number.number_kind == NUMBER_U64) fail(c, "Parameter exceeds SQLite signed 64-bit INTEGER range");
        if (type != PDJSON_STRING && type != PDJSON_NUMBER && type != PDJSON_TRUE && type != PDJSON_FALSE && type != PDJSON_NULL) fail(c, "Parameters must be scalar JSON values");
    }
    if (!statement) return; /* Transport preflight before any source correction. */
    for (i = 0; i < count; i++) {
        query_parameter *p = &parameters[order[i]];
        int index = sqlite3_bind_parameter_index(statement, p->name), status;
        if (!index) {
            const char prefixes[] = ":@$?";
            for (j = 0; j < 4; j++) {
                int found = sqlite3_bind_parameter_index(statement, format_text(c, "%c%s", prefixes[j], p->name));
                if (index && found) fail(c, "Ambiguous bare parameter: %s", p->name);
                if (found) index = found;
            }
        }
        if (!index) fail(c, "Unknown query parameter: %s", p->name);
        if (p->type == PDJSON_STRING) status = sqlite3_bind_text(statement, index, p->text, (int)p->length, SQLITE_TRANSIENT);
        else if (p->type == PDJSON_NUMBER) status = p->number.number_kind == NUMBER_I64 ? sqlite3_bind_int64(statement, index, p->number.number.signed_integer) : sqlite3_bind_double(statement, index, p->number.number.real);
        else if (p->type == PDJSON_TRUE || p->type == PDJSON_FALSE) status = sqlite3_bind_int(statement, index, p->type == PDJSON_TRUE);
        else if (p->type == PDJSON_NULL) status = sqlite3_bind_null(statement, index);
        else fail(c, "Parameters must be scalar JSON values");
        if (status != SQLITE_OK) fail(c, "Parameter bind failed: %s", sqlite3_errmsg(c->db));
    }
}
static void result_columns(context *c, result_rows *rows, sqlite3_stmt *s) {
    int i, j, count = sqlite3_column_count(s);
    if (count > COLUMN_LIMIT) fail(c, "Result exceeds 1024 columns");
    for (i = 0; i < count; i++) {
        const char *name = sqlite3_column_name(s, i);
        for (j = 0; j < rows->column_count; j++) if (!strcmp(rows->columns[j], name)) break;
        if (j == rows->column_count) rows->columns[rows->column_count++] = copy_text(c, name, strlen(name));
        rows->indexes[j] = i; /* Last duplicate column value wins, like JS row objects. */
    }
    for (i = 1; i < rows->column_count; i++) {
        for (j = i; j > 0 && object_index(rows->columns[j]) < object_index(rows->columns[j - 1]); j--) {
            const char *name = rows->columns[j]; int index = rows->indexes[j];
            rows->columns[j] = rows->columns[j - 1]; rows->indexes[j] = rows->indexes[j - 1];
            rows->columns[j - 1] = name; rows->indexes[j - 1] = index;
        }
    }
}
/* Decode invalid SQLite TEXT like Node's UTF-8 conversion: one U+FFFD per
   maximal invalid subpart. Embedded NUL remains data. Borrow valid input. */
static string_view sql_text(context *c, sqlite3_stmt *s, int index) {
    string_view input = {sqlite3_column_text(s, index), (size_t)sqlite3_column_bytes(s, index)}, result;
    size_t i = 0, used = 0;
    unsigned char *out;
    if (string_view_utf8_valid(input)) return input;
    out = allocate(c, input.len * 3 + 1);
    while (i < input.len) {
        unsigned char first = input.data[i];
        int width = first < 128 ? 1 : first >= 0xc2 && first <= 0xdf ? 2 : first >= 0xe0 && first <= 0xef ? 3 : first >= 0xf0 && first <= 0xf4 ? 4 : 0;
        int j = 1, valid = width != 0;
        while (valid && j < width) {
            unsigned char next;
            if (i + j == input.len) { valid = 0; break; }
            next = input.data[i + j];
            if (next < 0x80 || next > 0xbf || (j == 1 && ((first == 0xe0 && next < 0xa0) || (first == 0xed && next > 0x9f) || (first == 0xf0 && next < 0x90) || (first == 0xf4 && next > 0x8f)))) { valid = 0; break; }
            j++;
        }
        if (valid) { memcpy(out + used, input.data + i, width); used += width; i += width; }
        else { memcpy(out + used, "\xef\xbf\xbd", 3); used += 3; i += j; }
    }
    result.data = out; result.len = used;
    return result;
}
static size_t quoted_size(string_view text) {
    size_t i, size = 2;
    for (i = 0; i < text.len; i++) {
        unsigned char ch = text.data[i];
        size += ch == '"' || ch == '\\' || ch == '\b' || ch == '\f' || ch == '\n' || ch == '\r' || ch == '\t' ? 2 : ch < 32 ? 6 : 1;
    }
    return size;
}
static int decimal_digits(unsigned int number) { int count = 1; while (number >= 10) { count++; number /= 10; } return count; }
static size_t sql_json_size(context *c, sqlite3_stmt *s, int index, string_view *text) {
    int type = sqlite3_column_type(s, index);
    if (type == SQLITE_NULL) return 4;
    if (type == SQLITE_INTEGER) {
        char token[32]; jsonb b;
        jsonb_init(&b); json_integer(&b, token, sizeof(token), sqlite3_column_int64(s, index)); return b.pos;
    }
    if (type == SQLITE_TEXT) { *text = sql_text(c, s, index); return quoted_size(*text); }
    if (type == SQLITE_BLOB) {
        const unsigned char *bytes = sqlite3_column_blob(s, index);
        int i, length = sqlite3_column_bytes(s, index);
        size_t size = 2;
        for (i = 0; i < length && size <= RESULT_BYTES; i++) size += decimal_digits(i) + 3 + decimal_digits(bytes[i]) + (i != 0);
        return size;
    }
    {
        double number = sqlite3_column_double(s, index);
        char token[128]; jsonb b;
        if (!isfinite(number)) return 4;
        jsonb_init(&b); json_real(c, &b, token, sizeof(token), number);
        return b.pos;
    }
}
static void sql_json_value(context *c, jsonb *b, char *out, size_t capacity, sqlite3_stmt *s, int index, string_view text) {
    int type = sqlite3_column_type(s, index);
    if (type == SQLITE_NULL) json_check(jsonb_null(b, out, capacity));
    else if (type == SQLITE_INTEGER) json_integer(b, out, capacity, sqlite3_column_int64(s, index));
    else if (type == SQLITE_TEXT) json_check(jsonb_string(b, out, capacity, (const char *)text.data, text.len));
    else if (type == SQLITE_BLOB) {
        int i, length = sqlite3_column_bytes(s, index);
        const unsigned char *bytes = sqlite3_column_blob(s, index);
        json_check(jsonb_object(b, out, capacity));
        for (i = 0; i < length; i++) {
            char key[32];
            snprintf(key, sizeof(key), "%d", i);
            json_check(jsonb_key(b, out, capacity, key, strlen(key)));
            json_number(c, b, out, capacity, bytes[i]);
        }
        json_check(jsonb_object_pop(b, out, capacity));
    } else {
        double number = sqlite3_column_double(s, index);
        if (isfinite(number)) json_real(c, b, out, capacity, number);
        else json_check(jsonb_null(b, out, capacity));
    }
}
static const char *serialize_row(context *c, result_rows *rows, sqlite3_stmt *s, const char *collection_name) {
    jsonb b;
    char *out = c->json_scratch;
    string_view texts[COLUMN_LIMIT] = {{0}};
    size_t size = 2;
    int i;
    if (collection_name) size += 13 + quoted_size(string_view_from_cstr(collection_name));
    for (i = 0; i < rows->column_count; i++) {
        size += quoted_size(string_view_from_cstr(rows->columns[i])) + 1 + (i != 0 || collection_name != NULL);
        size += sql_json_size(c, s, rows->indexes[i], &texts[i]);
        if (size + 2 > RESULT_BYTES) return NULL; /* Row cannot fit even alone. */
    }
    jsonb_init(&b);
    json_check(jsonb_object(&b, out, OUTPUT_CAPACITY));
    if (collection_name) {
        json_check(jsonb_key(&b, out, OUTPUT_CAPACITY, "collection", 10));
        json_check(jsonb_string(&b, out, OUTPUT_CAPACITY, collection_name, strlen(collection_name)));
    }
    for (i = 0; i < rows->column_count; i++) {
        json_check(jsonb_key(&b, out, OUTPUT_CAPACITY, rows->columns[i], strlen(rows->columns[i])));
        sql_json_value(c, &b, out, OUTPUT_CAPACITY, s, rows->indexes[i], texts[i]);
    }
    json_check(jsonb_object_pop(&b, out, OUTPUT_CAPACITY));
    return copy_text(c, out, b.pos);
}
static int next_read(context *c, sqlite3_stmt *s) {
    int status = sqlite3_step(s);
    if (status != SQLITE_ROW && status != SQLITE_DONE) {
        if (status == SQLITE_INTERRUPT) c->error_code = "sql-budget";
        else c->error_code = status == SQLITE_AUTH || status == SQLITE_READONLY ? "sql-denied" : "sql-execution";
        fail(c, "Query execution failed: %s", sqlite3_errmsg(c->db));
    }
    return status;
}
static result_rows *collect_query(context *c, sqlite3_stmt *s, int limit) {
    result_rows *rows = allocate(c, sizeof(result_rows));
    result_columns(c, rows, s);
    while (next_read(c, s) == SQLITE_ROW) {
        const char *row;
        size_t size;
        if (rows->count == limit) { rows->truncated = 1; break; }
        row = serialize_row(c, rows, s, NULL);
        if (!row) { rows->truncated = 1; break; }
        size = strlen(row);
        if (rows->bytes + size + rows->count + 2 > RESULT_BYTES) { rows->truncated = 1; break; }
        rows->rows[rows->count++] = row; rows->bytes += size;
    }
    return rows;
}
static const char *result_json(context *c, result_rows *rows) {
    char *out = allocate(c, OUTPUT_CAPACITY);
    jsonb b;
    int i;
    jsonb_init(&b);
    json_check(jsonb_object(&b, out, OUTPUT_CAPACITY));
    json_check(jsonb_key(&b, out, OUTPUT_CAPACITY, "columns", 7));
    json_check(jsonb_array(&b, out, OUTPUT_CAPACITY));
    for (i = 0; i < rows->column_count; i++) json_check(jsonb_string(&b, out, OUTPUT_CAPACITY, rows->columns[i], strlen(rows->columns[i])));
    json_check(jsonb_array_pop(&b, out, OUTPUT_CAPACITY));
    json_check(jsonb_key(&b, out, OUTPUT_CAPACITY, "rows", 4));
    json_check(jsonb_array(&b, out, OUTPUT_CAPACITY));
    for (i = 0; i < rows->count; i++) json_check(jsonb_token(&b, out, OUTPUT_CAPACITY, rows->rows[i], strlen(rows->rows[i])));
    json_check(jsonb_array_pop(&b, out, OUTPUT_CAPACITY));
    json_check(jsonb_key(&b, out, OUTPUT_CAPACITY, "returned", 8)); json_number(c, &b, out, OUTPUT_CAPACITY, rows->count);
    json_check(jsonb_key(&b, out, OUTPUT_CAPACITY, "truncated", 9)); json_check(jsonb_bool(&b, out, OUTPUT_CAPACITY, rows->truncated));
    json_check(jsonb_object_pop(&b, out, OUTPUT_CAPACITY));
    return out;
}
static const char *columns_json(context *c, const char *table) {
    sqlite3_stmt *s = read_statement(c, format_text(c, "PRAGMA table_info(%s)", identifier(c, table)));
    result_rows *rows = collect_query(c, s, RESULT_LIMIT);
    char *out = allocate(c, 256 * 1024);
    jsonb b;
    int i;
    sqlite3_finalize(s);
    jsonb_init(&b);
    json_check(jsonb_array(&b, out, 256 * 1024));
    for (i = 0; i < rows->count; i++) json_check(jsonb_token(&b, out, 256 * 1024, rows->rows[i], strlen(rows->rows[i])));
    json_check(jsonb_array_pop(&b, out, 256 * 1024));
    return out;
}
static const char *schema_json(context *c, read_options *options) {
    char *out = allocate(c, OUTPUT_CAPACITY);
    jsonb b;
    int i, j;
    jsonb_init(&b);
    json_check(jsonb_object(&b, out, OUTPUT_CAPACITY));
    json_check(jsonb_key(&b, out, OUTPUT_CAPACITY, "collections", 11));
    json_check(jsonb_array(&b, out, OUTPUT_CAPACITY));
    for (i = 0; i < c->collection_count; i++) {
        collection *table = &c->collections[i];
        const char *usage_path = full_path(c, table->directory, ".usage.md");
        const char *fts = format_text(c, "__attendant_fts_%s", table->name);
        DWORD attributes;
        json_check(jsonb_object(&b, out, OUTPUT_CAPACITY));
        json_property(&b, out, OUTPUT_CAPACITY, "name", table->name);
        json_property(&b, out, OUTPUT_CAPACITY, "directory", table->directory);
        json_property(&b, out, OUTPUT_CAPACITY, "schemaPath", table->schema_path);
        json_property(&b, out, OUTPUT_CAPACITY, "usagePath", usage_path);
        json_check(jsonb_key(&b, out, OUTPUT_CAPACITY, "usage", 5));
        attributes = GetFileAttributesW(wide(c, usage_path));
        if (attributes == INVALID_FILE_ATTRIBUTES && (GetLastError() == ERROR_FILE_NOT_FOUND || GetLastError() == ERROR_PATH_NOT_FOUND)) json_check(jsonb_null(&b, out, OUTPUT_CAPACITY));
        else {
            const char *usage = read_text(c, usage_path, NULL);
            json_check(jsonb_string(&b, out, OUTPUT_CAPACITY, usage, strlen(usage)));
        }
        json_property(&b, out, OUTPUT_CAPACITY, "table", table->name);
        json_check(jsonb_key(&b, out, OUTPUT_CAPACITY, "columns", 7));
        {
            const char *columns = columns_json(c, table->name);
            json_check(jsonb_token(&b, out, OUTPUT_CAPACITY, columns, strlen(columns)));
        }
        json_check(jsonb_key(&b, out, OUTPUT_CAPACITY, "fields", 6)); json_check(jsonb_array(&b, out, OUTPUT_CAPACITY));
        for (j = 0; j < table->field_count; j++) {
            field *f = &table->fields[j];
            json_check(jsonb_object(&b, out, OUTPUT_CAPACITY));
            json_property(&b, out, OUTPUT_CAPACITY, "name", f->name); json_property(&b, out, OUTPUT_CAPACITY, "kind", f->kind);
            json_check(jsonb_key(&b, out, OUTPUT_CAPACITY, "optional", 8)); json_check(jsonb_bool(&b, out, OUTPUT_CAPACITY, f->optional));
            if (!strcmp(f->kind, "enum")) { json_check(jsonb_key(&b, out, OUTPUT_CAPACITY, "enumValues", 10)); json_value(c, &b, out, OUTPUT_CAPACITY, f->declaration); }
            if (!strcmp(f->kind, "enum") || !strcmp(f->kind, "default")) { json_check(jsonb_key(&b, out, OUTPUT_CAPACITY, "defaultValue", 12)); json_value(c, &b, out, OUTPUT_CAPACITY, f->fallback); }
            if (f->reference) json_property(&b, out, OUTPUT_CAPACITY, "referenceCollection", f->reference);
            json_check(jsonb_object_pop(&b, out, OUTPUT_CAPACITY));
        }
        json_check(jsonb_array_pop(&b, out, OUTPUT_CAPACITY));
        json_check(jsonb_key(&b, out, OUTPUT_CAPACITY, "fts", 3)); json_check(jsonb_object(&b, out, OUTPUT_CAPACITY));
        json_property(&b, out, OUTPUT_CAPACITY, "table", fts);
        json_check(jsonb_key(&b, out, OUTPUT_CAPACITY, "columns", 7));
        {
            /* Describe supported search schema even before physical creation. */
            const char *columns = c->fts_enabled ? columns_json(c, fts) :
                "[{\"cid\":0,\"name\":\"record_id\",\"type\":\"\",\"notnull\":0,\"dflt_value\":null,\"pk\":0},"
                "{\"cid\":1,\"name\":\"content\",\"type\":\"\",\"notnull\":0,\"dflt_value\":null,\"pk\":0}]";
            json_check(jsonb_token(&b, out, OUTPUT_CAPACITY, columns, strlen(columns)));
        }
        json_check(jsonb_object_pop(&b, out, OUTPUT_CAPACITY)); json_check(jsonb_object_pop(&b, out, OUTPUT_CAPACITY));
    }
    json_check(jsonb_array_pop(&b, out, OUTPUT_CAPACITY));
    json_check(jsonb_key(&b, out, OUTPUT_CAPACITY, "systemTables", 12)); json_check(jsonb_array(&b, out, OUTPUT_CAPACITY));
    for (i = 0; i < 2; i++) {
        const char *name = i ? "__attendant_edges" : "__attendant_diagnostics", *columns;
        json_check(jsonb_object(&b, out, OUTPUT_CAPACITY)); json_property(&b, out, OUTPUT_CAPACITY, "name", name);
        json_check(jsonb_key(&b, out, OUTPUT_CAPACITY, "columns", 7)); columns = columns_json(c, name);
        json_check(jsonb_token(&b, out, OUTPUT_CAPACITY, columns, strlen(columns))); json_check(jsonb_object_pop(&b, out, OUTPUT_CAPACITY));
    }
    json_check(jsonb_array_pop(&b, out, OUTPUT_CAPACITY)); json_property(&b, out, OUTPUT_CAPACITY, "diagnosticsTable", "__attendant_diagnostics");
    json_check(jsonb_object_pop(&b, out, OUTPUT_CAPACITY));
    (void)options;
    return out;
}
/* Windows NLS supplies word categories without another Unicode dependency.
   Tested Latin/CJK terms match the runner; Unicode-version/category edge parity
   and supplementary-plane letters remain deferred. Emoji are not words. */
static int search_word(const unsigned char *p, int *width) {
    unsigned int point;
    WCHAR utf16[2]; WORD categories[2]; int count = 1, i;
    if (*p < 128) { *width = 1; return (*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || (*p >= '0' && *p <= '9'); }
    *width = (*p & 0xe0) == 0xc0 ? 2 : (*p & 0xf0) == 0xe0 ? 3 : 4;
    point = *p & (*width == 2 ? 31 : *width == 3 ? 15 : 7);
    for (i = 1; i < *width; i++) point = (point << 6) | (p[i] & 63);
    if (point > 0xffff) { point -= 0x10000; utf16[0] = (WCHAR)(0xd800 + (point >> 10)); utf16[1] = (WCHAR)(0xdc00 + (point & 1023)); count = 2; }
    else utf16[0] = (WCHAR)point;
    if (!GetStringTypeW(CT_CTYPE1, utf16, count, categories)) return 0;
    return (categories[0] & (C1_ALPHA | C1_DIGIT)) != 0;
}
static int partial_search(const char *text) {
    const unsigned char *p = (const unsigned char *)text;
    int terms = 0, width;
    while (*p) {
        char word[8]; int bytes = 0, length = 0;
        while (*p && !search_word(p, &width)) p += width;
        while (*p && search_word(p, &width)) {
            int i;
            for (i = 0; i < width; i++) { if (bytes < 7) word[bytes] = (char)p[i]; bytes++; }
            length++; p += width;
        }
        word[bytes < 7 ? bytes : 7] = 0;
        if (!bytes) break;
        if (!_stricmp(word, "AND") || !_stricmp(word, "OR") || !_stricmp(word, "NOT") || !_stricmp(word, "NEAR")) continue;
        if (length < 3) return 0;
        terms++;
    }
    return terms > 0;
}
typedef struct search_hit { const char *row, *collection, *id; double score; int partial; } search_hit;
/* Stable result order is required; reproducing Node's locale collation is not. */
static int compare_hits(const void *left, const void *right) {
    const search_hit *a = left, *b = right;
    int order;
    if (a->partial != b->partial) return a->partial - b->partial;
    if (a->score != b->score) return a->score < b->score ? -1 : 1;
    order = strcmp(a->collection, b->collection);
    return order ? order : strcmp(a->id, b->id);
}
static result_rows *collect_search(context *c, read_options *options) {
    search_hit *hits = allocate(c, MAX_RECORDS * sizeof(search_hit));
    result_rows *result = allocate(c, sizeof(result_rows));
    int count = 0, i, phase, partial = partial_search(options->query_text), selected[MAX_COLLECTIONS] = {0};
    const char *names[] = {"collection", "id", "source_path", "name", "score", "snippet"};
    c->error_code = "fts-query";
    if (!*trim(copy_text(c, options->query_text, strlen(options->query_text)))) fail(c, "Search query must not be empty");
    if (options->collections) {
        char *list = copy_text(c, options->collections, strlen(options->collections)), *name = list;
        while (name) {
            char *next = strchr(name, ',');
            int found = -1;
            if (next) *next++ = 0;
            if (*name) {
                for (i = 0; i < c->collection_count; i++) if (!strcmp(name, c->collections[i].name)) found = i;
                if (found < 0 || selected[found]) fail(c, "Unknown or duplicate search collection: %s", name);
                selected[found] = 1;
            }
            name = next;
        }
    } else for (i = 0; i < c->collection_count; i++) selected[i] = 1;
    for (phase = 0; phase <= partial; phase++) {
        for (i = 0; i < c->collection_count; i++) {
            collection *table = &c->collections[i];
            sqlite3_stmt *s;
            result_rows *columns;
            const char *fts;
            if (!selected[i]) continue;
            fts = internal(c, phase ? "trigram" : "fts", table);
            s = read_statement(c, format_text(c, "SELECT records.id, records.source_path, records.name, bm25(%s) AS score, snippet(%s, 1, '[', ']', '…', 32) AS snippet FROM %s JOIN %s AS records ON records.id=%s.record_id WHERE %s MATCH ?", fts, fts, fts, identifier(c, table->name), fts, fts));
            bind_text(c, s, 1, options->query_text);
            columns = allocate(c, sizeof(result_rows)); result_columns(c, columns, s);
            while (next_read(c, s) == SQLITE_ROW) {
                const char *id = (const char *)sqlite3_column_text(s, 0);
                int j, duplicate = 0;
                if (phase) for (j = 0; j < count; j++) if (!strcmp(hits[j].collection, table->name) && !strcmp(hits[j].id, id)) { duplicate = 1; break; }
                if (duplicate) continue;
                if (count == MAX_RECORDS) fail(c, "Search exceeds 4096 retained hits");
                hits[count].row = serialize_row(c, columns, s, table->name);
                hits[count].collection = table->name; hits[count].id = copy_text(c, id, strlen(id));
                hits[count].score = sqlite3_column_double(s, 3); hits[count].partial = phase;
                count++;
            }
            sqlite3_finalize(s);
        }
    }
    qsort(hits, count, sizeof(search_hit), compare_hits);
    for (i = 0; i < 6; i++) result->columns[result->column_count++] = names[i];
    for (i = 0; i < count; i++) {
        size_t size;
        if (!hits[i].row) { result->truncated = 1; break; }
        size = strlen(hits[i].row);
        if (result->count == options->limit || result->bytes + size + result->count + 2 > RESULT_BYTES) { result->truncated = 1; break; }
        result->rows[result->count++] = hits[i].row; result->bytes += size;
    }
    return result;
}
#include "diagnostics-cli.h"
#include "write-cli.h"

static int native_read_main(context *c, int argc, char **argv) {
    read_options options = parse_read_options(c, argc, argv);
    const char *output;
    if (!strcmp(options.command, "doctor")) return native_doctor(c, &options);
    if (!strcmp(options.command, "validate")) return native_validate(c, &options);
    if (!strcmp(options.command, "add-table")) return native_add_table(c, &options);
    if (!strcmp(options.command, "create") || !strcmp(options.command, "update")) return native_write_records(c, &options);
    options.allow_fts = strcmp(options.command, "query") != 0;
    c->auto_correct = strcmp(options.command, "sync") != 0;
    if (options.sql_text) one_read_statement(c, options.sql_text);
    if (options.params) query_parameters(c, NULL, options.params);
    c->error_code = "projection-prepare";
    {
        const char *state = prepare_projection(c, options.project, !strcmp(options.command, "search") ? PROJECTION_SEARCH : PROJECTION_REUSE);
        if (!strcmp(options.command, "sync")) { print_sync(c, state); return 0; }
    }
    open_snapshot(c, &options);
    c->error_code = "sql-execution";
    if (!strcmp(options.command, "schema")) output = schema_json(c, &options);
    else if (!strcmp(options.command, "search")) output = result_json(c, collect_search(c, &options));
    else {
        sqlite3_stmt *s = read_statement(c, options.sql_text);
        result_rows *rows;
        query_parameters(c, s, options.params);
        c->error_code = "sql-execution";
        rows = collect_query(c, s, options.limit);
        sqlite3_finalize(s);
        output = result_json(c, rows);
    }
    if (sqlite3_close(c->db) != SQLITE_OK) fail(c, "Cannot close read-only database");
    c->db = NULL;
    if (c->applied_corrections || c->diagnostic_count) {
        jsonb report;
        char *tail = c->json_scratch, *extended;
        size_t a = strlen(output);
        jsonb_init(&report); json_check(jsonb_object(&report, tail, OUTPUT_CAPACITY));
        if (c->applied_corrections) {
            json_check(jsonb_key(&report, tail, OUTPUT_CAPACITY, "corrections", 11));
            json_check(jsonb_token(&report, tail, OUTPUT_CAPACITY, c->applied_corrections, strlen(c->applied_corrections)));
        }
        if (c->diagnostic_count) { json_check(jsonb_key(&report, tail, OUTPUT_CAPACITY, "diagnostics", 11)); json_diagnostics(c, &report, tail, OUTPUT_CAPACITY, NULL); }
        json_check(jsonb_object_pop(&report, tail, OUTPUT_CAPACITY));
        if (a + report.pos > OUTPUT_CAPACITY) fail(c, "Result plus source report exceeds output capacity");
        extended = allocate(c, a + report.pos + 1); memcpy(extended, output, a - 1); extended[a - 1] = ',';
        memcpy(extended + a, tail + 1, report.pos - 1); output = extended;
    }
    puts(output);
    return 0;
}
