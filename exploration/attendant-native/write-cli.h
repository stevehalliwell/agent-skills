/* Source-writing CLI. Preflight and stage complete batches before publication.
   Replacements are atomic per file, not a cross-file transaction. */
static int safe_stem(const char *name) {
    const char *p;
    size_t length = strlen(name);
    if (!length || !((*name >= 'A' && *name <= 'Z') || (*name >= 'a' && *name <= 'z') || (*name >= '0' && *name <= '9'))) return 0;
    for (p = name; *p; p++) if (!( (*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z') || (*p >= '0' && *p <= '9') || *p == '_' || *p == '-')) return 0;
    if (!_stricmp(name, "CON") || !_stricmp(name, "PRN") || !_stricmp(name, "AUX") || !_stricmp(name, "NUL")) return 0;
    if (length == 4 && (!_strnicmp(name, "COM", 3) || !_strnicmp(name, "LPT", 3)) && name[3] >= '1' && name[3] <= '9') return 0;
    return 1;
}
static void make_directories(context *c, const char *path) {
    wchar_t *target = wide(c, path);
    int i, length = (int)wcslen(target);
    if (length < 3 || target[1] != L':') fail(c, "Directory creation currently requires a Windows drive path");
    for (i = 3; i <= length; i++) if (i == length || target[i] == L'\\' || target[i] == L'/') {
        wchar_t saved = target[i];
        DWORD attributes;
        target[i] = 0; attributes = GetFileAttributesW(target);
        if (attributes == INVALID_FILE_ATTRIBUTES) {
            if (c->created_directory_count == 128) fail(c, "Directory creation depth exceeds 128 components");
            const char *owned = utf8(c, target);
            if (!CreateDirectoryW(target, NULL)) fail(c, "Cannot create collection directory");
            c->created_directories[c->created_directory_count++] = owned;
        } else if (!(attributes & FILE_ATTRIBUTE_DIRECTORY) || (attributes & FILE_ATTRIBUTE_REPARSE_POINT)) fail(c, "Collection path contains a non-directory or reparse point");
        target[i] = saved;
    }
}
static int inside_project(const char *root, const char *path) {
    size_t length = strlen(root);
    return !_strnicmp(root, path, length) && (path[length] == '\\' || path[length] == '/');
}
static void require_empty_directory(context *c, const char *directory) {
    DWORD attributes = GetFileAttributesW(wide(c, directory));
    if (attributes != INVALID_FILE_ATTRIBUTES) {
        WIN32_FIND_DATAW entry;
        HANDLE scan;
        if (!(attributes & FILE_ATTRIBUTE_DIRECTORY) || (attributes & FILE_ATTRIBUTE_REPARSE_POINT)) fail(c, "Collection target must be a regular empty directory");
        scan = FindFirstFileW(wide(c, format_text(c, "%s\\*", directory)), &entry);
        if (scan == INVALID_HANDLE_VALUE) fail(c, "Cannot inspect collection directory");
        do {
            if (wcscmp(entry.cFileName, L".") && wcscmp(entry.cFileName, L"..")) { FindClose(scan); fail(c, "Collection target must not contain files"); }
        } while (FindNextFileW(scan, &entry));
        if (GetLastError() != ERROR_NO_MORE_FILES) { FindClose(scan); fail(c, "Cannot finish directory inspection"); }
        FindClose(scan);
    } else if (GetLastError() != ERROR_FILE_NOT_FOUND && GetLastError() != ERROR_PATH_NOT_FOUND) fail(c, "Cannot inspect collection target");
}
static int native_add_table(context *c, read_options *options) {
    const char *directory = full_path(c, options->project, options->directory);
    const char *base = strrchr(directory, '\\'), *name = options->alias ? trim(copy_text(c, options->alias, strlen(options->alias))) : base ? base + 1 : directory;
    const char *config_path = full_path(c, options->project, ".pi/attendant.tables"), *config, *line, *next_config;
    const char *paths[3], *texts[] = {"---\n---\n", "", ""}, *staged[3], *staged_config;
    char *out = allocate(c, 65536);
    jsonb b;
    size_t a, d;
    int i;
    c->error_code = "add-table-input";
    if (!*name || !safe_stem(name) || !strncmp(name, "__attendant_", 12)) fail(c, "Collection name must be a safe non-reserved filename stem");
    if (!_stricmp(directory, options->project) || (!absolute_path(options->directory) && !inside_project(options->project, directory))) fail(c, "Relative collection path must be inside the project and not its root");
    {
        const char *state = full_path(c, options->project, ".attendant");
        if (!_stricmp(directory, state) || inside_project(state, directory)) fail(c, "Source collections must not be inside disposable .attendant state");
    }
    load_config(c, options->project);
    if (c->collection_count >= MAX_COLLECTIONS) fail(c, "Collection capacity exceeded");
    if (!c->config_hash || c->diagnostic_count) fail(c, "Cannot add collection to invalid config/schema");
    for (i = 0; i < c->collection_count; i++) {
        if (!_stricmp(name, c->collections[i].name) || !lstrcmpiW(wide(c, directory), wide(c, full_path(c, ".", c->collections[i].directory)))) fail(c, "Collection name or directory is already configured");
    }
    require_empty_directory(c, directory);
    config = read_text(c, config_path, NULL);
    line = inside_project(options->project, directory) ? directory + strlen(options->project) + 1 : directory;
    {
        char *spelling = copy_text(c, line, strlen(line)), *p;
        if (inside_project(options->project, directory)) for (p = spelling; *p; p++) if (*p == '\\') *p = '/';
        if (*spelling == '#' || strchr(spelling, '\r') || strchr(spelling, '\n') || table_alias_separator(spelling) ||
            strcmp(trim(copy_text(c, spelling, strlen(spelling))), spelling)) fail(c, "Directory cannot round-trip through attendant.tables grammar");
        line = options->alias && *options->alias ? format_text(c, "%s as %s", spelling, name) : spelling;
    }
    a = strlen(config); d = strlen(line);
    if (a + d + 3 > TEXT_CAPACITY) fail(c, "Config exceeds source capacity");
    {
        char *joined = allocate(c, a + d + 3);
        memcpy(joined, config, a);
        if (a && config[a - 1] != '\n') joined[a++] = '\n';
        memcpy(joined + a, line, d); joined[a + d] = '\n'; next_config = joined;
    }
    paths[0] = full_path(c, directory, ".schema.md"); paths[1] = full_path(c, directory, ".template.md"); paths[2] = full_path(c, directory, ".usage.md");
    jsonb_init(&b); json_check(jsonb_object(&b, out, 65536));
    json_property(&b, out, 65536, "name", name); json_property(&b, out, 65536, "directory", directory);
    json_property(&b, out, 65536, "schemaPath", paths[0]); json_property(&b, out, 65536, "templatePath", paths[1]);
    json_property(&b, out, 65536, "usagePath", paths[2]); json_property(&b, out, 65536, "configPath", config_path);
    json_check(jsonb_object_pop(&b, out, 65536));
    c->error_code = "add-table-write";
    make_directories(c, directory);
    for (i = 0; i < 3; i++) staged[i] = stage_source(c, paths[i], texts[i]);
    staged_config = stage_source(c, config_path, next_config);
    if (strcmp(read_text(c, config_path, NULL), config)) fail(c, "Config changed since preflight");
    for (i = 0; i < 3; i++) { publish_source(c, staged[i], paths[i], NULL); c->rollback_paths[c->rollback_count++] = paths[i]; }
    publish_source(c, staged_config, config_path, config);
    c->rollback_count = 0; c->created_directory_count = 0; c->staged_count = 0;
    puts(out); return 0;
}
static value json_item_value(context *c, pdjson_stream *parser) {
    enum pdjson_type token = pdjson_next(parser);
    value v = {0};
    if (token == PDJSON_STRING) {
        size_t length;
        const char *text = pdjson_get_value(parser, &length);
        if (strlen(text) + 1 != length) fail(c, "Source field strings must not contain NUL");
        v = text_value(copy_text(c, text, length - 1));
    } else if (token == PDJSON_NUMBER) {
        if (parse_number(pdjson_get_value(parser, NULL), &v) != NUMBER_VALID) fail(c, "Source number exceeds i64/u64 or finite f64 range");
    } else if (token == PDJSON_TRUE || token == PDJSON_FALSE) { v.kind = VALUE_BOOL; v.number.real = token == PDJSON_TRUE; }
    else if (token == PDJSON_ARRAY) {
        v.kind = VALUE_ARRAY; v.items = allocate(c, MAX_ITEMS * sizeof(value));
        while (pdjson_peek(parser) != PDJSON_ARRAY_END) {
            enum pdjson_type next = pdjson_peek(parser);
            if (v.count == MAX_ITEMS || next == PDJSON_ARRAY || next == PDJSON_OBJECT || next == PDJSON_ERROR || next == PDJSON_DONE) fail(c, "Fields accept only bounded scalar arrays");
            v.items[v.count++] = json_item_value(c, parser);
        }
        pdjson_next(parser);
    } else if (token != PDJSON_NULL) fail(c, "Fields must be scalar, null or scalar arrays");
    return v;
}
typedef struct write_request { const char *name, *path; mapping fields; record *target; int fields_present; } write_request;
static int parse_items(context *c, const char *text, write_request *requests) {
    pdjson_stream parser;
    pdjson_allocator allocator = {json_allocate, json_reallocate, json_release};
    int count = 0, events = 0;
    c->error_code = "cli-json";
    pdjson_open_string(&parser, text); pdjson_set_allocator(&parser, &allocator, c);
    pdjson_set_language(&parser, PDJSON_LANGUAGE_JSON); pdjson_set_streaming(&parser, false);
    if (pdjson_next(&parser) != PDJSON_ARRAY) fail(c, "--items must be a non-empty JSON array");
    while (pdjson_peek(&parser) != PDJSON_ARRAY_END) {
        enum pdjson_type token;
        write_request *item;
        if (count == MAX_RECORDS || pdjson_next(&parser) != PDJSON_OBJECT) fail(c, "Each bounded batch item must be an object");
        item = &requests[count++];
        while ((token = pdjson_next(&parser)) == PDJSON_NAME) {
            size_t length;
            const char *name = pdjson_get_name(&parser, &length);
            if (++events > MAX_RECORDS * MAX_PAIRS || strlen(name) + 1 != length) fail(c, "JSON item keys exceed limits or contain NUL");
            if (!strcmp(name, "name")) {
                value v = json_item_value(c, &parser);
                item->name = v.kind == VALUE_STRING ? v.text : NULL;
            } else if (!strcmp(name, "fields")) {
                item->fields_present = 1; memset(&item->fields, 0, sizeof(item->fields));
                if (pdjson_next(&parser) != PDJSON_OBJECT) fail(c, "fields must be a JSON object");
                while ((token = pdjson_next(&parser)) == PDJSON_NAME) {
                    const char *key = pdjson_get_name(&parser, &length);
                    if (++events > MAX_RECORDS * MAX_PAIRS || strlen(key) + 1 != length) fail(c, "JSON field keys exceed limits or contain NUL");
                    key = copy_text(c, key, length - 1); mapping_set(c, &item->fields, key, json_item_value(c, &parser));
                }
                if (token != PDJSON_OBJECT_END) fail(c, "Invalid fields object");
            } else {
                token = pdjson_peek(&parser);
                if (token == PDJSON_OBJECT || token == PDJSON_ARRAY) pdjson_skip(&parser); else pdjson_next(&parser);
            }
        }
        if (token != PDJSON_OBJECT_END) fail(c, "Invalid item object");
    }
    pdjson_next(&parser);
    if (!count || pdjson_next(&parser) != PDJSON_DONE) fail(c, "--items must be non-empty strict JSON without trailing input");
    pdjson_close(&parser); return count;
}
static const char *template_body(context *c, collection *table) {
    const char *path = full_path(c, table->directory, ".template.md"), *text, *body = NULL, *p;
    DWORD attributes = GetFileAttributesW(wide(c, path));
    if (attributes == INVALID_FILE_ATTRIBUTES && (GetLastError() == ERROR_FILE_NOT_FOUND || GetLastError() == ERROR_PATH_NOT_FOUND)) return "";
    text = read_text(c, path, NULL);
    if (strncmp(text, "---\n", 4) && strncmp(text, "---\r\n", 5)) return text;
    p = strchr(text, '\n') + 1;
    for (; *p; ) {
        if (!strncmp(p, "---\n", 4)) { body = p + 4; break; }
        if (!strncmp(p, "---\r\n", 5)) { body = p + 5; break; }
        if (!strcmp(p, "---")) { body = p + 3; break; }
        p = strchr(p, '\n'); if (!p) break; p++;
    }
    if (!body) fail(c, "Template front matter is missing closing delimiter");
    return body;
}
static const char *new_record_text(context *c, mapping *front, const char *body) {
    char *out;
    const char *keys[MAX_PAIRS], *values[MAX_PAIRS];
    size_t used = 4, capacity = 8 + strlen(body);
    int i;
    for (i = 0; i < front->count; i++) {
        keys[i] = yaml_value(c, text_value(front->pairs[i].key)); values[i] = yaml_value(c, front->pairs[i].data);
        capacity += strlen(keys[i]) + strlen(values[i]) + 3;
        if (capacity > TEXT_CAPACITY) fail(c, "Generated record exceeds source capacity");
    }
    out = allocate(c, capacity + 1); memcpy(out, "---\n", 4);
    for (i = 0; i < front->count; i++) {
        const char *key = keys[i], *data = values[i];
        size_t a = strlen(key), b = strlen(data);
        memcpy(out + used, key, a); used += a; memcpy(out + used, ": ", 2); used += 2;
        memcpy(out + used, data, b); used += b; out[used++] = '\n';
    }
    if (used + 4 + strlen(body) > TEXT_CAPACITY) fail(c, "Generated record body exceeds source capacity");
    memcpy(out + used, "---\n", 4); used += 4; memcpy(out + used, body, strlen(body));
    return out;
}
static void preflight_writable_fields(context *c, collection *table, mapping *fields) {
    int i, j;
    for (i = 0; i < fields->count; i++) {
        const char *name = fields->pairs[i].key;
        if (!strcmp(name, "desc") || !strcmp(name, "tags")) continue;
        for (j = 0; j < table->field_count; j++) if (!strcmp(name, table->fields[j].name)) break;
        if (j == table->field_count) fail(c, "Field %s is immutable or not declared by %s", name, table->name);
    }
}
static int native_write_records(context *c, read_options *options) {
    write_request *requests = allocate(c, MAX_RECORDS * sizeof(write_request));
    collection *table = NULL;
    const char *body = NULL, *output, *corrections = NULL;
    const char **staged;
    int count = parse_items(c, options->items, requests), creating = !strcmp(options->command, "create"), original_count, i, j;
    c->error_code = creating ? "create-preflight" : "update-preflight";
    load_source(c, options->project);
    if (!c->config_hash || c->contract_diagnostic_count) fail(c, "Config/schema must be valid before source writes");
    for (i = 0; i < c->collection_count; i++) if (c->collections[i].invalid) fail(c, "Collection cannot be read before source writes: %s", c->collections[i].name);
    for (i = 0; i < c->collection_count; i++) if (!strcmp(options->collection, c->collections[i].name)) table = &c->collections[i];
    if (!table) fail(c, "Unknown collection: %s", options->collection);
    {
        const char *state = full_path(c, options->project, ".attendant"), *source = full_path(c, ".", table->directory);
        if (!_stricmp(source, state) || inside_project(state, source)) fail(c, "Source collections must not be inside disposable .attendant state");
    }
    original_count = c->record_count;
    if (creating && count > MAX_RECORDS - original_count) fail(c, "Record capacity exceeded");
    for (i = 0; i < count; i++) {
        write_request *item = &requests[i];
        if (!item->name || !safe_stem(item->name)) fail(c, "Record name must be a safe non-device filename stem");
        for (j = 0; j < i; j++) if (!_stricmp(item->name, requests[j].name)) fail(c, "Repeated target name in batch: %s", item->name);
        if (!creating && (!item->fields_present || !item->fields.count)) fail(c, "Update fields must be a non-empty JSON object");
        preflight_writable_fields(c, table, &item->fields);
        item->path = full_path(c, table->directory, format_text(c, "%s.md", item->name));
        for (j = 0; j < original_count; j++) if (!_stricmp(item->path, c->records[j].path)) break;
        if (creating) {
            if (GetFileAttributesW(wide(c, item->path)) != INVALID_FILE_ATTRIBUTES || (GetLastError() != ERROR_FILE_NOT_FOUND && GetLastError() != ERROR_PATH_NOT_FOUND)) fail(c, "Record target exists or cannot be checked: %s", item->path);
        } else {
            if (j == original_count) fail(c, "Record was not found: %s", item->name);
            item->target = &c->records[j]; item->path = item->target->path;
            if (item->target->front.problem || !item->target->front.present) fail(c, "Cannot update malformed or unsupported front matter: %s", item->path);
        }
    }
    plan_corrections(c);
    if (creating) body = template_body(c, table);
    for (i = 0; i < count; i++) {
        write_request *item = &requests[i];
        record *r;
        if (creating) {
            FILETIME now;
            value empty = {0}; empty.kind = VALUE_ARRAY;
            r = &c->records[c->record_count++]; item->target = r; r->path = item->path; r->owner = table;
            mapping_set(c, &r->front, "id", text_value(uuid_v7(c))); mapping_set(c, &r->front, "name", text_value(item->name));
            GetSystemTimeAsFileTime(&now); mapping_set(c, &r->front, "created_at", text_value(iso_time(c, now)));
            mapping_set(c, &r->front, "desc", text_value("")); mapping_set(c, &r->front, "tags", empty);
            for (j = 0; j < table->field_count; j++) {
                field *f = &table->fields[j];
                value *requested = lookup(&item->fields, f->name);
                if (requested) mapping_set(c, &r->front, f->name, *requested);
                else if (!strcmp(f->kind, "enum") || !strcmp(f->kind, "default")) mapping_set(c, &r->front, f->name, f->fallback);
                else if (!strcmp(f->kind, "string-array")) mapping_set(c, &r->front, f->name, empty);
                else if (!f->optional && strcmp(f->kind, "untyped")) fail(c, "Required create field missing: %s", f->name);
            }
            for (j = 0; j < item->fields.count; j++) {
                pair *p = &item->fields.pairs[j];
                if (p->data.kind == VALUE_NULL && (!strcmp(p->key, "desc") || !strcmp(p->key, "tags"))) continue;
                mapping_set(c, &r->front, p->key, p->data);
            }
            {
                mapping intended = r->front;
                r->text = new_record_text(c, &r->front, body); r->front = read_mapping(c, r->text, &r->body);
                if (r->front.problem || !r->front.present || !mapping_equal(&intended, &r->front)) fail(c, "Generated YAML changed requested field meaning: %s", r->path);
            }
        } else {
            mapping combined = {0};
            r = item->target;
            for (j = 0; j < r->corrections.count; j++) mapping_set(c, &combined, r->corrections.pairs[j].key, r->corrections.pairs[j].data);
            for (j = 0; j < item->fields.count; j++) {
                mapping_set(c, &combined, item->fields.pairs[j].key, item->fields.pairs[j].data);
                if (lookup(&r->corrections, item->fields.pairs[j].key)) mapping_set(c, &r->corrections, item->fields.pairs[j].key, item->fields.pairs[j].data);
            }
            r->next_text = patch_record(c, r, &combined);
        }
    }
    c->diagnostic_count = c->contract_diagnostic_count;
    for (i = 0; i < c->record_count; i++) {
        record *r = &c->records[i];
        if (r->next_text) r->front = read_mapping(c, r->next_text, NULL);
        r->blocked = 0; validate_record(c, r);
    }
    validate_dataset(c); projection_requirements(c);
    if (c->diagnostic_count) fail(c, "Batch or current source is invalid: %s", c->diagnostics[0].message);
    corrections = corrections_json(c);
    {
        jsonb b;
        char *out = allocate(c, OUTPUT_CAPACITY);
        jsonb_init(&b); json_check(jsonb_object(&b, out, OUTPUT_CAPACITY));
        json_check(jsonb_key(&b, out, OUTPUT_CAPACITY, "records", 7)); json_check(jsonb_array(&b, out, OUTPUT_CAPACITY));
        for (i = 0; i < count; i++) {
            const char *fields = mapping_json(c, creating ? &requests[i].target->front : &requests[i].fields);
            json_check(jsonb_object(&b, out, OUTPUT_CAPACITY)); json_property(&b, out, OUTPUT_CAPACITY, "path", requests[i].path);
            json_check(jsonb_key(&b, out, OUTPUT_CAPACITY, "fields", 6)); json_check(jsonb_token(&b, out, OUTPUT_CAPACITY, fields, strlen(fields)));
            json_check(jsonb_object_pop(&b, out, OUTPUT_CAPACITY));
        }
        json_check(jsonb_array_pop(&b, out, OUTPUT_CAPACITY));
        if (strcmp(corrections, "[]")) { json_check(jsonb_key(&b, out, OUTPUT_CAPACITY, "corrections", 11)); json_check(jsonb_token(&b, out, OUTPUT_CAPACITY, corrections, strlen(corrections))); }
        json_check(jsonb_object_pop(&b, out, OUTPUT_CAPACITY)); output = out;
    }
    staged = allocate(c, c->record_count * sizeof(char *));
    c->error_code = creating ? "create-write" : "update-write";
    for (i = 0; i < c->record_count; i++) {
        record *r = &c->records[i];
        if (r->next_text || i >= original_count) staged[i] = stage_source(c, r->path, r->next_text ? r->next_text : r->text);
    }
    for (i = 0; i < c->record_count; i++) if (staged[i]) {
        record *r = &c->records[i];
        publish_source(c, staged[i], r->path, i < original_count ? r->text : NULL);
        if (i >= original_count) c->rollback_paths[c->rollback_count++] = r->path;
    }
    c->rollback_count = 0; c->staged_count = 0;
    c->error_code = "projection-refresh";
    reload_records(c); refresh_projection(c, options->project, PROJECTION_REUSE);
    puts(output); return 0;
}
