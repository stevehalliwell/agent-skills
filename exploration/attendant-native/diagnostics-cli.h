/* Doctor/validate never create a projection. Only validate's default mode writes
   reviewed mechanical corrections; --strict/--no-correct remain read-only. */
static void json_fields(context *c, jsonb *b, char *out, size_t capacity, collection *table) {
    int j;
    json_check(jsonb_array(b, out, capacity));
    for (j = 0; j < table->field_count; j++) {
        field *f = &table->fields[j];
        json_check(jsonb_object(b, out, capacity));
        json_property(b, out, capacity, "name", f->name); json_property(b, out, capacity, "kind", f->kind);
        json_check(jsonb_key(b, out, capacity, "optional", 8)); json_check(jsonb_bool(b, out, capacity, f->optional));
        if (!strcmp(f->kind, "enum")) { json_check(jsonb_key(b, out, capacity, "enumValues", 10)); json_value(c, b, out, capacity, f->declaration); }
        if (!strcmp(f->kind, "enum") || !strcmp(f->kind, "default")) { json_check(jsonb_key(b, out, capacity, "defaultValue", 12)); json_value(c, b, out, capacity, f->fallback); }
        if (f->reference) json_property(b, out, capacity, "referenceCollection", f->reference);
        json_check(jsonb_object_pop(b, out, capacity));
    }
    json_check(jsonb_array_pop(b, out, capacity));
}
static int native_validate(context *c, read_options *options) {
    jsonb b;
    char *out;
    const char *corrections;
    int i, count, correct = !options->strict && !options->no_correct;
    load_source(c, options->project);
    count = plan_corrections(c); corrections = corrections_json(c);
    if (correct && count) { apply_corrections(c); reload_records(c); }
    out = allocate(c, OUTPUT_CAPACITY); jsonb_init(&b); json_check(jsonb_object(&b, out, OUTPUT_CAPACITY));
    json_check(jsonb_key(&b, out, OUTPUT_CAPACITY, "collections", 11)); json_check(jsonb_array(&b, out, OUTPUT_CAPACITY));
    for (i = 0; i < c->collection_count; i++) {
        collection *table = &c->collections[i];
        json_check(jsonb_object(&b, out, OUTPUT_CAPACITY));
        json_property(&b, out, OUTPUT_CAPACITY, "name", table->name); json_property(&b, out, OUTPUT_CAPACITY, "directory", table->directory);
        json_property(&b, out, OUTPUT_CAPACITY, "schemaPath", table->schema_path);
        json_check(jsonb_key(&b, out, OUTPUT_CAPACITY, "fields", 6)); json_fields(c, &b, out, OUTPUT_CAPACITY, table);
        json_check(jsonb_object_pop(&b, out, OUTPUT_CAPACITY));
    }
    json_check(jsonb_array_pop(&b, out, OUTPUT_CAPACITY));
    json_check(jsonb_key(&b, out, OUTPUT_CAPACITY, "diagnostics", 11)); json_diagnostics(c, &b, out, OUTPUT_CAPACITY, NULL);
    json_check(jsonb_key(&b, out, OUTPUT_CAPACITY, "records", 7)); json_number(c, &b, out, OUTPUT_CAPACITY, c->record_count);
    json_check(jsonb_key(&b, out, OUTPUT_CAPACITY, "corrections", 11)); json_check(jsonb_token(&b, out, OUTPUT_CAPACITY, corrections, strlen(corrections)));
    json_check(jsonb_key(&b, out, OUTPUT_CAPACITY, "corrected", 9)); json_check(jsonb_bool(&b, out, OUTPUT_CAPACITY, correct));
    json_check(jsonb_key(&b, out, OUTPUT_CAPACITY, "strict", 6)); json_check(jsonb_bool(&b, out, OUTPUT_CAPACITY, options->strict));
    json_check(jsonb_object_pop(&b, out, OUTPUT_CAPACITY)); puts(out);
    return options->strict && (count || c->diagnostic_count) ? 1 : 0;
}
static void doctor_check(jsonb *b, char *out, const char *id, const char *status, const char *message, const char *details) {
    json_check(jsonb_object(b, out, OUTPUT_CAPACITY));
    json_property(b, out, OUTPUT_CAPACITY, "id", id); json_property(b, out, OUTPUT_CAPACITY, "status", status);
    json_property(b, out, OUTPUT_CAPACITY, "message", message);
    if (details) { json_check(jsonb_key(b, out, OUTPUT_CAPACITY, "details", 7)); json_check(jsonb_token(b, out, OUTPUT_CAPACITY, details, strlen(details))); }
    json_check(jsonb_object_pop(b, out, OUTPUT_CAPACITY));
}
static int native_doctor(context *c, read_options *options) {
    char *out = allocate(c, OUTPUT_CAPACITY), *result;
    const char *projection = "missing", *path = full_path(c, options->project, ".attendant/attendant.sqlite");
    jsonb b;
    sqlite3 *probe = NULL;
    HANDLE directory;
    int healthy = 1, sqlite_ok;
    load_source(c, options->project); projection_requirements(c);
    jsonb_init(&b); json_check(jsonb_array(&b, out, OUTPUT_CAPACITY));
    doctor_check(&b, out, "runtime-native", "ok", "Native Windows runtime available.", NULL);
    sqlite_ok = sqlite3_open(":memory:", &probe) == SQLITE_OK && sqlite3_exec(probe, "CREATE VIRTUAL TABLE probe USING fts5(content, tokenize='trigram')", NULL, NULL, NULL) == SQLITE_OK;
    sqlite3_close(probe);
    if (!sqlite_ok) healthy = 0;
    doctor_check(&b, out, "runtime-sqlite", sqlite_ok ? "ok" : "error", sqlite_ok ? "SQLite FTS5 with trigram tokenizer available." : "SQLite FTS5 trigram tokenizer is unavailable.", format_text(c, "{\"version\":\"%s\"}", sqlite3_libversion()));
    if (!c->config_hash || c->contract_diagnostic_count) healthy = 0;
    doctor_check(&b, out, "config", c->config_hash && !c->contract_diagnostic_count ? "ok" : "error", c->config_hash ? "Attendant config inspected." : "Missing .pi/attendant.tables.", format_text(c, "{\"collections\":%d}", c->collection_count));
    directory = CreateFileW(wide(c, options->project), 0x0002 /* FILE_ADD_FILE */, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (directory == INVALID_HANDLE_VALUE) healthy = 0; else CloseHandle(directory);
    doctor_check(&b, out, "state-write", directory != INVALID_HANDLE_VALUE ? "ok" : "error", directory != INVALID_HANDLE_VALUE ? "Project root is writable for generated state." : "Project root is not writable for generated state.", NULL);
    if (GetFileAttributesW(wide(c, path)) != INVALID_FILE_ATTRIBUTES) {
        const char *suffixes[] = {"-wal", "-shm", "-journal"};
        int i, sidecar = 0;
        for (i = 0; i < 3; i++) if (GetFileAttributesW(wide(c, format_text(c, "%s%s", path, suffixes[i]))) != INVALID_FILE_ATTRIBUTES) sidecar = 1;
        projection = "stale";
        /* Immutable URI prevents even WAL/SHM creation during health inspection. */
        if (!sidecar) {
            size_t length = strlen(path), used = 0, j;
            char *encoded = allocate(c, length * 3 + 1);
            for (j = 0; j < length; j++) {
                unsigned char byte = (unsigned char)path[j];
                if (byte == '\\') encoded[used++] = '/';
                else if (byte == '%' || byte == '?' || byte == '#') { snprintf(encoded + used, 4, "%%%02X", byte); used += 3; }
                else encoded[used++] = (char)byte;
            }
            if (sqlite3_open_v2(format_text(c, "file:%s?immutable=1", encoded), &c->db, SQLITE_OPEN_READONLY | SQLITE_OPEN_URI, NULL) == SQLITE_OK) {
                sqlite3_stmt *statement = NULL;
                int quick = sqlite3_prepare_v2(c->db, "PRAGMA quick_check", -1, &statement, NULL) == SQLITE_OK && sqlite3_step(statement) == SQLITE_ROW &&
                    sqlite3_column_type(statement, 0) == SQLITE_TEXT && !strcmp((const char *)sqlite3_column_text(statement, 0), "ok");
                sqlite3_finalize(statement);
                projection = !quick ? "corrupt" : c->config_hash && snapshot_matches(c) ? "fresh" : "stale";
            } else projection = "corrupt";
            sqlite3_close(c->db); c->db = NULL;
        }
    }
    if (strcmp(projection, "fresh")) healthy = 0;
    doctor_check(&b, out, "projection", !strcmp(projection, "fresh") ? "ok" : "error", format_text(c, "Projection is %s.", projection), NULL);
    doctor_check(&b, out, "diagnostics", c->diagnostic_count ? "warn" : "ok", format_text(c, "%d record/schema diagnostic(s).", c->diagnostic_count), format_text(c, "{\"count\":%d}", c->diagnostic_count));
    json_check(jsonb_array_pop(&b, out, OUTPUT_CAPACITY));
    result = allocate(c, OUTPUT_CAPACITY);
    {
        jsonb result_builder;
        jsonb_init(&result_builder); json_check(jsonb_object(&result_builder, result, OUTPUT_CAPACITY));
        json_check(jsonb_key(&result_builder, result, OUTPUT_CAPACITY, "ok", 2)); json_check(jsonb_bool(&result_builder, result, OUTPUT_CAPACITY, healthy));
        json_check(jsonb_key(&result_builder, result, OUTPUT_CAPACITY, "checks", 6)); json_check(jsonb_token(&result_builder, result, OUTPUT_CAPACITY, out, b.pos));
        json_check(jsonb_key(&result_builder, result, OUTPUT_CAPACITY, "diagnostics", 11)); json_diagnostics(c, &result_builder, result, OUTPUT_CAPACITY, NULL);
        json_property(&result_builder, result, OUTPUT_CAPACITY, "projection", projection);
        json_check(jsonb_object_pop(&result_builder, result, OUTPUT_CAPACITY)); puts(result);
    }
    return healthy ? 0 : 1;
}
