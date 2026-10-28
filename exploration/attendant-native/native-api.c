/* Recoverable, GUI-independent source operations over the existing C99 core.
 * One explicit setjmp boundary per public request; fail() cleans owned resources
 * before returning here. Never longjmp through GUI callbacks or C++ frames. */
#define ATTENDANT_EMBEDDED
#include "projection.c"
#include "native-api.h"

#define REQUEST_MEMORY (128 * 1024 * 1024)
struct attendant_session {
    context state;
    jmp_buf recovery;
    arena *memory;
};
static const char comment_schema[] =
    "---\nstatus: [pending, resolved]\ntarget_path: \"\"\ntarget_hash: \"\"\n"
    "range_start: 0\nrange_end: 0\nrange_unit: [unicode-codepoint]\nselected_text: \"\"\n---\n";
static const char comment_usage[] =
    "# Shared Markdown feedback\n\n"
    "- Query pending comments; read comment Markdown body and selected source.\n"
    "- Selection/comment gives contextual intent, not permission for unrelated edits.\n"
    "- Compare target_hash with exact saved UTF-8 bytes before using range.\n"
    "- range_start/range_end count Unicode code points, zero-based/end-exclusive; CR and LF each count.\n"
    "- Reading does not consume. Explicitly update status to resolved after handling.\n"
    "- Built-in source_path/source_hash identify this comment, not target document.\n";
static context *request_begin(attendant_session *session) {
    context *c = &session->state;
    arena_reset(session->memory);
    memset(c, 0, sizeof(*c));
    c->memory = session->memory; c->recovery = &session->recovery;
    c->error_code = "source-input";
    active_context = c;
    return c;
}
static attendant_result request_result(context *c, int ok, const char *path, const char *hash, const char *text) {
    attendant_result result = {0};
    result.ok = ok; result.path = ok ? path : c->error_path;
    result.hash = hash; result.text = text;
    result.code = ok ? NULL : c->error_code;
    result.message = ok ? NULL : c->error_message;
    result.written_count = c->written_count; result.written = c->written;
    c->recovery = NULL;
    return result;
}
attendant_session *attendant_open(void) {
    attendant_session *session = VirtualAlloc(NULL, sizeof(*session), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    void *backing;
    if (!session) return NULL;
    backing = VirtualAlloc(NULL, REQUEST_MEMORY, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!backing) { VirtualFree(session, 0, MEM_RELEASE); return NULL; }
    session->memory = arena_from_buffer(backing, REQUEST_MEMORY);
    return session;
}
void attendant_close(attendant_session *session) {
    if (!session) return;
    if (active_context == &session->state) active_context = NULL;
    VirtualFree(session->memory, 0, MEM_RELEASE);
    VirtualFree(session, 0, MEM_RELEASE);
}
static void input_text(context *c, const char *text) {
    if (!text || strlen(text) > TEXT_CAPACITY || !string_view_utf8_valid(string_view_from_cstr(text)))
        fail(c, "Expected NUL-terminated UTF-8 input up to 4 MiB");
}
static const char *regular_document(context *c, const char *path) {
    const char *absolute;
    DWORD attributes;
    input_text(c, path); absolute = full_path(c, ".", path); c->error_path = absolute;
    attributes = GetFileAttributesW(wide(c, absolute));
    if (attributes == INVALID_FILE_ATTRIBUTES || (attributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)))
        fail(c, "Document must be an existing regular file");
    return absolute;
}
attendant_result attendant_read_document(attendant_session *session, const char *path) {
    context *c = request_begin(session);
    const char *absolute, *text;
    if (setjmp(session->recovery)) return request_result(c, 0, NULL, NULL, NULL);
    absolute = regular_document(c, path); text = read_text(c, absolute, NULL);
    return request_result(c, 1, absolute, digest(c, text), text);
}
attendant_result attendant_save_document(attendant_session *session, const char *path, const char *expected_hash, const char *text) {
    context *c = request_begin(session);
    const char *absolute, *original, *hash, *staged;
    if (setjmp(session->recovery)) return request_result(c, 0, NULL, NULL, NULL);
    input_text(c, text); input_text(c, expected_hash);
    absolute = regular_document(c, path); original = read_text(c, absolute, NULL);
    c->error_code = "source-changed";
    if (strcmp(digest(c, original), expected_hash)) fail(c, "Document changed since it was loaded; reload or cancel save");
    hash = digest(c, text); c->error_code = "source-save";
    staged = stage_source(c, absolute, text); publish_source(c, staged, absolute, original);
    c->staged_count = 0;
    return request_result(c, 1, absolute, hash, NULL);
}
static value offset_value(uint64_t number) {
    value v = {0}; v.kind = VALUE_NUMBER; v.number_kind = NUMBER_I64;
    v.number.signed_integer = (long long)number; return v;
}
static void verify_selection(context *c, const attendant_feedback *input, const char *source) {
    size_t offset = 0, begin = 0, end = 0, length = strlen(source);
    uint64_t point = 0;
    c->error_code = "feedback-range";
    if (input->range_start >= input->range_end || input->range_end > TEXT_CAPACITY) fail(c, "Expected a non-empty bounded code-point range");
    while (offset < length) {
        if (point == input->range_start) begin = offset;
        if (point == input->range_end) break;
        offset += string_utf8_sequence_size((const u8 *)source + offset, length - offset); point++;
    }
    end = offset;
    if (point != input->range_end || strlen(input->selected_text) != end - begin || memcmp(source + begin, input->selected_text, end - begin))
        fail(c, "Selection text does not match saved source code-point range");
}
#ifdef ATTENDANT_TEST_FAULTS
static void feedback_fault(context *c, const char *point) {
    char fault[128];
    DWORD length = GetEnvironmentVariableA("ATTENDANT_FEEDBACK_FAULT", fault, sizeof(fault));
    if (length && length < sizeof(fault) && !strcmp(fault, point)) fail(c, "Injected feedback failure: %s", point);
}
static void feedback_external_edit(context *c, const char *target, const char *config) {
    char fault[128];
    DWORD length = GetEnvironmentVariableA("ATTENDANT_FEEDBACK_FAULT", fault, sizeof(fault)), written;
    const char *path, *edit = "\n# Concurrent external edit\n";
    HANDLE file;
    if (!length || length >= sizeof(fault)) return;
    path = !strcmp(fault, "target-change") ? target : !strcmp(fault, "config-change") ? config : NULL;
    if (!path) return;
    file = CreateFileW(wide(c, path), FILE_APPEND_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) fail(c, "Cannot inject external source edit");
    if (!WriteFile(file, edit, (DWORD)strlen(edit), &written, NULL) || written != strlen(edit)) { CloseHandle(file); fail(c, "Cannot inject external source bytes"); }
    CloseHandle(file);
}
#else
#define feedback_fault(c, point) ((void)0)
#define feedback_external_edit(c, target, config) ((void)0)
#endif
attendant_result attendant_send_feedback(attendant_session *session, const attendant_feedback *input) {
    context *c = request_begin(session);
    const char *root, *target, *source, *config_path, *config = NULL, *next_config = NULL;
    const char *directory, *name, *comment_path, *comment_text, *comment_stage, *config_stage = NULL;
    const char *seed_paths[3], *seed_stages[3], *seeds[3] = {comment_schema, "---\n---\n", comment_usage};
    collection *table = NULL;
    mapping expected, front = {0};
    FILETIME now;
    value empty = {0};
    int i, bootstrap;
    if (setjmp(session->recovery)) return request_result(c, 0, NULL, NULL, NULL);
    if (!input) fail(c, "Missing feedback input");
    input_text(c, input->project); input_text(c, input->target_hash);
    input_text(c, input->selected_text); input_text(c, input->comment);
    if (strspn(input->comment, " \t\r\n\v\f") == strlen(input->comment)) fail(c, "Comment must not be empty");
    root = full_path(c, ".", input->project); target = regular_document(c, input->target_path);
    source = read_text(c, target, NULL); c->error_code = "feedback-stale";
    if (strcmp(digest(c, source), input->target_hash)) fail(c, "Saved target hash changed; select text again");
    verify_selection(c, input, source);
    c->error_code = "feedback-conflict";
    config_path = full_path(c, root, ".pi/attendant.tables");
    {
        DWORD attributes = GetFileAttributesW(wide(c, config_path));
        if (attributes != INVALID_FILE_ATTRIBUTES) {
            if (attributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) fail(c, "Config must be a regular file");
            config = read_text(c, config_path, NULL); load_config(c, root);
            if (c->diagnostic_count) fail(c, "Cannot register feedback with invalid config/schema");
        } else if (GetLastError() != ERROR_FILE_NOT_FOUND && GetLastError() != ERROR_PATH_NOT_FOUND) fail(c, "Cannot inspect workspace config");
    }
    for (i = 0; i < c->collection_count; i++) if (!_stricmp(c->collections[i].name, "_meta_comments")) {
        if (strcmp(c->collections[i].name, "_meta_comments")) fail(c, "Feedback alias conflicts by case");
        table = &c->collections[i];
    }
    bootstrap = table == NULL;
    expected = read_mapping(c, comment_schema, NULL);
    if (bootstrap) {
        if (c->collection_count == MAX_COLLECTIONS) fail(c, "Collection capacity exceeded");
        directory = full_path(c, root, "records/_meta_comments");
        for (i = 0; i < c->collection_count; i++) if (!lstrcmpiW(wide(c, directory), wide(c, full_path(c, ".", c->collections[i].directory))))
            fail(c, "Feedback directory already belongs to another collection");
        require_empty_directory(c, directory);
        table = &c->collections[c->collection_count++];
        table->name = "_meta_comments"; table->directory = directory;
        table->schema_path = full_path(c, directory, ".schema.md"); table->schema_text = comment_schema;
        load_schema(c, table);
        {
            const char *line = "records/_meta_comments as _meta_comments\n";
            size_t a = config ? strlen(config) : 0, b = strlen(line);
            char *joined;
            if (a + b + 2 > TEXT_CAPACITY) fail(c, "Config exceeds source capacity");
            joined = allocate(c, a + b + 2);
            if (a) memcpy(joined, config, a);
            if (a && config[a - 1] != '\n') joined[a++] = '\n';
            memcpy(joined + a, line, b); next_config = joined;
        }
    } else {
        mapping actual = read_mapping(c, table->schema_text, NULL);
        if (actual.problem || !mapping_equal(&expected, &actual)) fail(c, "Existing _meta_comments schema is not managed schema v1");
        directory = full_path(c, ".", table->directory);
    }
    {
        const char *state = full_path(c, root, ".attendant");
        if (!_stricmp(directory, root) || !_stricmp(directory, state) || inside_project(state, directory))
            fail(c, "Feedback directory must not be workspace root or disposable state");
    }
    name = format_text(c, "comment-%s", uuid_v7(c)); comment_path = full_path(c, directory, format_text(c, "%s.md", name));
    empty.kind = VALUE_ARRAY;
    mapping_set(c, &front, "id", text_value(uuid_v7(c))); mapping_set(c, &front, "name", text_value(name));
    GetSystemTimeAsFileTime(&now); mapping_set(c, &front, "created_at", text_value(iso_time(c, now)));
    mapping_set(c, &front, "desc", text_value("")); mapping_set(c, &front, "tags", empty);
    mapping_set(c, &front, "status", text_value("pending")); mapping_set(c, &front, "target_path", text_value(target));
    mapping_set(c, &front, "target_hash", text_value(input->target_hash));
    mapping_set(c, &front, "range_start", offset_value(input->range_start)); mapping_set(c, &front, "range_end", offset_value(input->range_end));
    mapping_set(c, &front, "range_unit", text_value("unicode-codepoint")); mapping_set(c, &front, "selected_text", text_value(input->selected_text));
    comment_text = new_record_text(c, &front, input->comment);
    {
        mapping actual = read_mapping(c, comment_text, NULL);
        if (actual.problem || !mapping_equal(&front, &actual)) fail(c, "Generated feedback changed field meaning");
    }
    c->error_code = "feedback-write";
    make_directories(c, directory);
    if (bootstrap) {
        make_directories(c, full_path(c, root, ".pi"));
        for (i = 0; i < 3; i++) {
            const char *names[] = {".schema.md", ".template.md", ".usage.md"};
            seed_paths[i] = full_path(c, directory, names[i]); seed_stages[i] = stage_source(c, seed_paths[i], seeds[i]);
        }
        config_stage = stage_source(c, config_path, next_config);
    }
    comment_stage = stage_source(c, comment_path, comment_text);
    feedback_fault(c, "staged");
    feedback_external_edit(c, target, config_path);
    c->error_code = "feedback-stale"; c->error_path = target;
    if (strcmp(read_text(c, target, NULL), source)) fail(c, "Target changed before feedback publication");
    c->error_code = "feedback-write";
    if (bootstrap) {
        for (i = 0; i < 3; i++) { publish_source(c, seed_stages[i], seed_paths[i], NULL); c->rollback_paths[c->rollback_count++] = seed_paths[i]; }
        feedback_fault(c, "seeds-published");
    }
    publish_source(c, comment_stage, comment_path, NULL);
    if (bootstrap) {
        c->rollback_paths[c->rollback_count++] = comment_path;
        feedback_fault(c, "comment-published");
        publish_source(c, config_stage, config_path, config);
    }
    c->rollback_count = c->created_directory_count = c->staged_count = 0;
    return request_result(c, 1, comment_path, input->target_hash, NULL);
}
