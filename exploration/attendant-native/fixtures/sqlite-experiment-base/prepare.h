/* In-process projection lifecycle. Reads may apply reported mechanical corrections.
   Hash/parse/validate bounded source, then reuse, reconcile rows, or rebuild.
   SQLite transactions protect projection writes, not concurrent source writers. */
typedef enum projection_mode { PROJECTION_REUSE, PROJECTION_REBUILD } projection_mode;

static int snapshot_metadata_matches(context *c, const char *key, const char *expected) {
    sqlite3_stmt *s = NULL;
    int matches = 0;
    if (sqlite3_prepare_v2(c->db, "SELECT value FROM __attendant_metadata WHERE key=?", -1, &s, NULL) == SQLITE_OK &&
        sqlite3_bind_text(s, 1, key, -1, SQLITE_TRANSIENT) == SQLITE_OK && sqlite3_step(s) == SQLITE_ROW &&
        sqlite3_column_type(s, 0) == SQLITE_TEXT) {
        const char *actual = (const char *)sqlite3_column_text(s, 0);
        matches = actual && sqlite3_column_bytes(s, 0) == (int)strlen(expected) && !strcmp(actual, expected);
    }
    sqlite3_finalize(s);
    return matches;
}
static int snapshot_count_matches(context *c, const char *statement, int expected) {
    sqlite3_stmt *s = NULL;
    int matches = sqlite3_prepare_v2(c->db, statement, -1, &s, NULL) == SQLITE_OK &&
        sqlite3_step(s) == SQLITE_ROW && sqlite3_column_type(s, 0) == SQLITE_INTEGER &&
        sqlite3_column_int64(s, 0) == expected;
    sqlite3_finalize(s);
    return matches;
}
/* JSON text can collide for old floats and newly exact integers. SQLite's
   double-to-TEXT conversion can also lose precision in string-first enums.
   Check actual values; a producer marker would survive JS snapshot rewrites. */
static value *projection_number(record *r, field *f) {
    value *v = lookup(&r->front, f->name);
    if (!v && (!strcmp(f->kind, "default") || !strcmp(f->kind, "enum"))) v = &f->fallback;
    return v && v->kind == VALUE_NUMBER && (!strcmp(field_type(f), "TEXT") || (v->number_kind == NUMBER_I64 &&
        (v->number.signed_integer > 9007199254740991LL || v->number.signed_integer < -9007199254740991LL))) ? v : NULL;
}
static int snapshot_number_matches(sqlite3_stmt *s, int index, field *f, value *expected) {
    if (!strcmp(field_type(f), "TEXT")) {
        const char *text;
        int length;
        if (sqlite3_column_type(s, index) != SQLITE_TEXT) return 0;
        length = sqlite3_column_bytes(s, index); text = (const char *)sqlite3_column_text(s, index);
        if (!text || length >= 128 || strlen(text) != (size_t)length) return 0;
        if (expected->number_kind == NUMBER_I64) {
            char token[32];
            int size = snprintf(token, sizeof(token), "%lld", expected->number.signed_integer);
            return size == length && !memcmp(token, text, size);
        } else {
            value actual = {0};
            return parse_number(text, &actual) == NUMBER_VALID && numbers_equal(*expected, actual);
        }
    }
    return sqlite3_column_type(s, index) == SQLITE_INTEGER && sqlite3_column_int64(s, index) == expected->number.signed_integer;
}
static int snapshot_numbers_match(context *c, collection *table) {
    sqlite3_stmt *s = NULL;
    int i, j, matches = 1;
    for (i = 0; i < c->record_count && matches; i++) {
        record *r = &c->records[i];
        value *hidden = lookup(&r->front, "hidden");
        if (r->owner != table || r->blocked || r->projection_changed || (hidden && hidden->number.real)) continue;
        for (j = 0; j < table->field_count; j++) if (projection_number(r, &table->fields[j])) break;
        if (j == table->field_count) continue;
        if (!s && sqlite3_prepare_v2(c->db, format_text(c, "SELECT * FROM %s WHERE source_path=?", identifier(c, table->name)), -1, &s, NULL) != SQLITE_OK) { matches = 0; break; }
        if (sqlite3_bind_text(s, 1, r->path, -1, SQLITE_TRANSIENT) != SQLITE_OK || sqlite3_step(s) != SQLITE_ROW) { matches = 0; break; }
        for (j = 0; j < table->field_count; j++) {
            value *v = projection_number(r, &table->fields[j]);
            /* Layout 2 puts nine universal columns before declared fields. */
            if (v && !snapshot_number_matches(s, j + 9, &table->fields[j], v)) { matches = 0; break; }
        }
        if (sqlite3_reset(s) != SQLITE_OK || sqlite3_clear_bindings(s) != SQLITE_OK) { matches = 0; break; }
    }
    sqlite3_finalize(s);
    return matches;
}
static int snapshot_integrity(context *c) {
    sqlite3_stmt *s = NULL;
    int status, matches;
    if (sqlite3_prepare_v2(c->db, "PRAGMA quick_check", -1, &s, NULL) != SQLITE_OK) { sqlite3_finalize(s); return 0; }
    status = sqlite3_step(s);
    matches = status == SQLITE_ROW && sqlite3_column_type(s, 0) == SQLITE_TEXT &&
        !strcmp((const char *)sqlite3_column_text(s, 0), "ok") && sqlite3_step(s) == SQLITE_DONE;
    sqlite3_finalize(s);
    return matches;
}
/* Caller holds a checked, stable SQLite snapshot. */
static int snapshot_matches(context *c) {
    sqlite3_stmt *s = NULL;
    int i, matches;
    if (!snapshot_metadata_matches(c, "layout_version", "2") ||
        !snapshot_metadata_matches(c, "config_hash", c->config_hash)) return 0;
    for (i = 0; i < c->collection_count; i++) {
        collection *table = &c->collections[i];
        int j, visible = 0;
        if (!snapshot_metadata_matches(c, format_text(c, "schema_hash:%s", table->name), schema_digest(c, table))) return 0;
        for (j = 0; j < c->record_count; j++) {
            record *r = &c->records[j];
            value *hidden = lookup(&r->front, "hidden");
            if (r->owner == table && !r->blocked && !(hidden && hidden->number.real)) visible++;
        }
        if (!snapshot_count_matches(c, format_text(c, "SELECT count(*) FROM %s", identifier(c, table->name)), visible) || !snapshot_numbers_match(c, table)) return 0;
        if (!snapshot_count_matches(c, format_text(c, "SELECT count(*) FROM %s", internal(c, "fts", table)), visible) ||
            !snapshot_count_matches(c, format_text(c, "SELECT count(*) FROM %s", internal(c, "trigram", table)), visible)) return 0;
    }
    if (!snapshot_count_matches(c, "SELECT count(*) FROM (SELECT collection_name,source_path,code,message FROM __attendant_diagnostics)", c->diagnostic_count)) return 0;
    if (sqlite3_prepare_v2(c->db, "SELECT source_collection,source_path,source_id,field,target_collection,target_name,target_path FROM __attendant_edges LIMIT 0", -1, &s, NULL) != SQLITE_OK) {
        sqlite3_finalize(s); return 0;
    }
    sqlite3_finalize(s); s = NULL;
    if (sqlite3_prepare_v2(c->db, "SELECT collection_name,mtime_ms,size,source_hash,front_matter,local_diagnostics,content FROM __attendant_cache WHERE source_path=?", -1, &s, NULL) != SQLITE_OK) {
        sqlite3_finalize(s); return 0;
    }
    matches = 1;
    for (i = 0; i < c->record_count; i++) {
        record *r = &c->records[i];
        if (sqlite3_bind_text(s, 1, r->path, -1, SQLITE_TRANSIENT) != SQLITE_OK || sqlite3_step(s) != SQLITE_ROW ||
            sqlite3_column_type(s, 0) != SQLITE_TEXT || sqlite3_column_type(s, 3) != SQLITE_TEXT ||
            strcmp((const char *)sqlite3_column_text(s, 0), r->owner->name) ||
            sqlite3_column_double(s, 1) != r->mtime_ms || sqlite3_column_int64(s, 2) != (sqlite3_int64)r->size ||
            strcmp((const char *)sqlite3_column_text(s, 3), r->hash) ||
            sqlite3_column_type(s, 4) != SQLITE_TEXT || strcmp((const char *)sqlite3_column_text(s, 4), r->front_json)) { matches = 0; break; }
        if (sqlite3_reset(s) != SQLITE_OK || sqlite3_clear_bindings(s) != SQLITE_OK) { matches = 0; break; }
    }
    sqlite3_finalize(s);
    return matches && snapshot_count_matches(c, "SELECT count(*) FROM __attendant_cache", c->record_count);
}
#define PROJECTION_BUSY_MS 2000
/* Keep SQLite's default bounded-WAL policy. Last connection close checkpoints;
   active readers may retain WAL pages, so never delete published sidecars. */
static void projection_writer(context *c) {
    sqlite3_stmt *s;
    int wal;
    sqlite3_busy_timeout(c->db, PROJECTION_BUSY_MS);
    s = prepare(c, "PRAGMA journal_mode=WAL");
    wal = sqlite3_step(s) == SQLITE_ROW && sqlite3_column_type(s, 0) == SQLITE_TEXT &&
        !strcmp((const char *)sqlite3_column_text(s, 0), "wal");
    if (finalize_builder(c, s) != SQLITE_OK || !wal) fail(c, "Cannot enable SQLite WAL (database busy or read-only)");
    sql(c, "PRAGMA synchronous=NORMAL; PRAGMA wal_autocheckpoint=1000; BEGIN IMMEDIATE;");
}
typedef struct deleted_projection { const char *path; collection *owner; } deleted_projection;
typedef struct projection_delta {
    deleted_projection deleted[MAX_RECORDS];
    int deleted_count, projected_changes, cache_changes;
} projection_delta;
static int projection_visible(record *r) {
    value *hidden = lookup(&r->front, "hidden");
    return !r->blocked && !(hidden && hidden->number.real);
}
/* Plan from a stable DB snapshot. Replan under BEGIN IMMEDIATE before mutation.
   Do not decode typed source from the lossy layout-2 JSON cache. */
static int plan_projection_delta(context *c, projection_delta *delta) {
    sqlite3_stmt *s = NULL;
    int i, j, status, valid = 0;
    memset(delta, 0, sizeof(*delta));
    if (!snapshot_metadata_matches(c, "layout_version", "2") || !snapshot_metadata_matches(c, "config_hash", c->config_hash)) return 0;
    for (i = 0; i < c->collection_count; i++) {
        collection *table = &c->collections[i];
        int count;
        if (!snapshot_metadata_matches(c, format_text(c, "schema_hash:%s", table->name), schema_digest(c, table))) return 0;
        if (sqlite3_prepare_v2(c->db, format_text(c, "SELECT * FROM %s LIMIT 0", identifier(c, table->name)), -1, &s, NULL) != SQLITE_OK || sqlite3_column_count(s) != 9 + table->field_count) goto done;
        sqlite3_finalize(s); s = NULL;
        if (sqlite3_prepare_v2(c->db, format_text(c, "SELECT count(*) FROM %s", identifier(c, table->name)), -1, &s, NULL) != SQLITE_OK || sqlite3_step(s) != SQLITE_ROW) goto done;
        count = sqlite3_column_int(s, 0); sqlite3_finalize(s); s = NULL;
        if (count < 0 || count > MAX_RECORDS) goto done;
        if (sqlite3_prepare_v2(c->db, format_text(c, "SELECT count(*) FROM %s p LEFT JOIN __attendant_cache c ON p.source_path=c.source_path WHERE c.source_path IS NULL", identifier(c, table->name)), -1, &s, NULL) != SQLITE_OK || sqlite3_step(s) != SQLITE_ROW || sqlite3_column_int(s, 0) != 0) goto done;
        sqlite3_finalize(s); s = NULL;
        if (!snapshot_count_matches(c, format_text(c, "SELECT count(*) FROM %s", internal(c, "fts", table)), count) ||
            !snapshot_count_matches(c, format_text(c, "SELECT count(*) FROM %s", internal(c, "trigram", table)), count)) goto done;
    }
    if (sqlite3_prepare_v2(c->db, "SELECT source_path,collection_name,mtime_ms,size,source_hash,front_matter,local_diagnostics,content FROM __attendant_cache LIMIT 0", -1, &s, NULL) != SQLITE_OK) goto done;
    sqlite3_finalize(s); s = NULL;
    if (sqlite3_prepare_v2(c->db, "SELECT collection_name,source_path,code,message FROM __attendant_diagnostics LIMIT 0", -1, &s, NULL) != SQLITE_OK) goto done;
    sqlite3_finalize(s); s = NULL;
    if (sqlite3_prepare_v2(c->db, "SELECT source_collection,source_path,source_id,field,target_collection,target_name,target_path FROM __attendant_edges LIMIT 0", -1, &s, NULL) != SQLITE_OK) goto done;
    sqlite3_finalize(s); s = NULL;
    if (sqlite3_prepare_v2(c->db, "SELECT collection_name,source_path FROM __attendant_cache", -1, &s, NULL) != SQLITE_OK) goto done;
    for (i = 0; (status = sqlite3_step(s)) == SQLITE_ROW; i++) {
        const char *owner, *path;
        collection *table = NULL;
        if (i >= MAX_RECORDS || sqlite3_column_type(s, 0) != SQLITE_TEXT || sqlite3_column_type(s, 1) != SQLITE_TEXT) goto done;
        owner = (const char *)sqlite3_column_text(s, 0); path = (const char *)sqlite3_column_text(s, 1);
        for (j = 0; j < c->collection_count; j++) if (!strcmp(owner, c->collections[j].name)) table = &c->collections[j];
        if (!table) goto done;
        for (j = 0; j < c->record_count; j++) if (c->records[j].owner == table && !strcmp(path, c->records[j].path)) break;
        if (j == c->record_count) {
            deleted_projection *deleted = &delta->deleted[delta->deleted_count++];
            deleted->owner = table; deleted->path = copy_text(c, path, strlen(path));
        }
    }
    if (status != SQLITE_DONE) goto done;
    sqlite3_finalize(s); s = NULL;
    for (i = 0; i < c->record_count; i++) {
        record *r = &c->records[i];
        int same = 0, exists;
        r->cache_changed = r->projection_changed = 0;
        if (sqlite3_prepare_v2(c->db, "SELECT collection_name,mtime_ms,size,source_hash,front_matter,local_diagnostics FROM __attendant_cache WHERE source_path=?", -1, &s, NULL) != SQLITE_OK || sqlite3_bind_text(s, 1, r->path, -1, SQLITE_TRANSIENT) != SQLITE_OK) goto done;
        status = sqlite3_step(s);
        if (status == SQLITE_ROW) {
            if (sqlite3_column_type(s, 0) != SQLITE_TEXT || sqlite3_column_type(s, 3) != SQLITE_TEXT || sqlite3_column_type(s, 4) != SQLITE_TEXT || sqlite3_column_type(s, 5) != SQLITE_TEXT || strcmp((const char *)sqlite3_column_text(s, 0), r->owner->name)) goto done;
            same = !strcmp((const char *)sqlite3_column_text(s, 3), r->hash);
            /* A matching hash with different decoded values is an invalid old cache. */
            if (same && strcmp((const char *)sqlite3_column_text(s, 4), r->front_json)) goto done;
            r->projection_changed = !same || strcmp((const char *)sqlite3_column_text(s, 5), record_diagnostics_json(c, r));
            r->cache_changed = r->projection_changed || sqlite3_column_double(s, 1) != r->mtime_ms || sqlite3_column_int64(s, 2) != (sqlite3_int64)r->size;
        } else if (status == SQLITE_DONE) r->cache_changed = r->projection_changed = 1;
        else goto done;
        sqlite3_finalize(s); s = NULL;
        if (sqlite3_prepare_v2(c->db, format_text(c, "SELECT id FROM %s WHERE source_path=?", identifier(c, r->owner->name)), -1, &s, NULL) != SQLITE_OK || sqlite3_bind_text(s, 1, r->path, -1, SQLITE_TRANSIENT) != SQLITE_OK) goto done;
        status = sqlite3_step(s); exists = status == SQLITE_ROW;
        if (!exists && status != SQLITE_DONE) goto done;
        if (exists && sqlite3_column_type(s, 0) != SQLITE_TEXT) goto done;
        if (exists != projection_visible(r) || (exists && projection_visible(r) && strcmp((const char *)sqlite3_column_text(s, 0), lookup(&r->front, "id")->text))) r->projection_changed = r->cache_changed = 1;
        sqlite3_finalize(s); s = NULL;
        delta->projected_changes += !!r->projection_changed; delta->cache_changes += !!r->cache_changed;
    }
    for (i = 0; i < c->collection_count; i++) if (!snapshot_numbers_match(c, &c->collections[i])) goto done;
    if (!snapshot_count_matches(c, "SELECT count(*) FROM __attendant_diagnostics", c->diagnostic_count)) delta->projected_changes++;
    valid = 1;
done:
    sqlite3_finalize(s);
    return valid;
}
static void delete_projected_record(context *c, collection *table, const char *path) {
    sqlite3_stmt *s = prepare(c, format_text(c, "SELECT id FROM %s WHERE source_path=?", identifier(c, table->name)));
    const char *id = NULL;
    int status, i;
    bind_text(c, s, 1, path); status = sqlite3_step(s);
    if (status == SQLITE_ROW) id = copy_text(c, (const char *)sqlite3_column_text(s, 0), sqlite3_column_bytes(s, 0));
    else if (status != SQLITE_DONE) fail(c, "Cannot find old projected record");
    if (finalize_builder(c, s) != SQLITE_OK) fail(c, "Cannot finish old projection lookup");
    if (id) {
        for (i = 0; i < 2; i++) {
            s = prepare(c, format_text(c, "DELETE FROM %s WHERE record_id=?", internal(c, i ? "trigram" : "fts", table)));
            bind_text(c, s, 1, id); finish(c, s);
        }
        s = prepare(c, format_text(c, "DELETE FROM %s WHERE source_path=?", identifier(c, table->name)));
        bind_text(c, s, 1, path); finish(c, s);
    }
    s = prepare(c, "DELETE FROM __attendant_cache WHERE source_path=?"); bind_text(c, s, 1, path); finish(c, s);
}
static void projection_diagnostics(context *c) {
    int i;
    sql(c, "DELETE FROM __attendant_diagnostics");
    for (i = 0; i < c->diagnostic_count; i++) {
        diagnostic *d = &c->diagnostics[i];
        sqlite3_stmt *s = prepare(c, "INSERT INTO __attendant_diagnostics VALUES (?,?,?,?)");
        if (d->owner) bind_text(c, s, 1, d->owner->name);
        bind_text(c, s, 2, d->path); bind_text(c, s, 3, d->code); bind_text(c, s, 4, d->message); finish(c, s);
    }
}
static const char *apply_projection_delta(context *c, projection_delta *delta) {
    int i, changed = delta->projected_changes || delta->deleted_count;
    /* Delete the whole affected set first: IDs may move between records. */
    for (i = 0; i < delta->deleted_count; i++) delete_projected_record(c, delta->deleted[i].owner, delta->deleted[i].path);
    for (i = 0; i < c->record_count; i++) if (c->records[i].projection_changed) delete_projected_record(c, c->records[i].owner, c->records[i].path);
    for (i = 0; i < c->record_count; i++) {
        record *r = &c->records[i];
        if (r->projection_changed) project_record(c, r);
        else if (r->cache_changed) {
            sqlite3_stmt *s = prepare(c, "UPDATE __attendant_cache SET mtime_ms=?,size=? WHERE source_path=?");
            sqlite3_bind_double(s, 1, r->mtime_ms); sqlite3_bind_int64(s, 2, (sqlite3_int64)r->size); bind_text(c, s, 3, r->path); finish(c, s);
        }
    }
    if (changed) {
        projection_diagnostics(c);
        sql(c, "DELETE FROM __attendant_edges"); make_edges(c);
    }
    sql(c, "COMMIT");
    return changed ? "updated" : "reused";
}
static void rebuild_projection(context *c, const char *root) {
    const char *directory = full_path(c, root, ".attendant");
    const char *target = full_path(c, directory, "attendant.sqlite");
    int i;
    if (!CreateDirectoryW(wide(c, directory), NULL) && GetLastError() != ERROR_ALREADY_EXISTS) fail(c, "Cannot create projection directory");
    /* Replacement still requires no active consumers. Healthy WAL updates use
       transactions instead; never rename a main DB over live WAL sidecars. */
    if (GetFileAttributesW(wide(c, format_text(c, "%s-wal", target))) != INVALID_FILE_ATTRIBUTES ||
        GetFileAttributesW(wide(c, format_text(c, "%s-shm", target))) != INVALID_FILE_ATTRIBUTES) {
        int status;
        if (sqlite3_open_v2(target, &c->db, SQLITE_OPEN_READWRITE, NULL) != SQLITE_OK) fail(c, "Cannot recover existing WAL projection");
        sqlite3_busy_timeout(c->db, PROJECTION_BUSY_MS);
        status = sqlite3_exec(c->db, "PRAGMA schema_version", NULL, NULL, NULL);
        if (status == SQLITE_OK) status = sqlite3_wal_checkpoint_v2(c->db, NULL, SQLITE_CHECKPOINT_TRUNCATE, NULL, NULL);
        if (sqlite3_close(c->db) != SQLITE_OK) fail(c, "Cannot close WAL checkpoint");
        c->db = NULL;
        if (status != SQLITE_OK) fail(c, "Existing WAL sidecars: stop consumers before rebuilding projection");
    }
    /* Old rollback journals must not replay on a new DB. */
    if (GetFileAttributesW(wide(c, format_text(c, "%s-wal", target))) != INVALID_FILE_ATTRIBUTES ||
        GetFileAttributesW(wide(c, format_text(c, "%s-shm", target))) != INVALID_FILE_ATTRIBUTES)
        fail(c, "Existing WAL sidecars: stop consumers and remove disposable projection before rebuilding");
    if (GetFileAttributesW(wide(c, format_text(c, "%s-journal", target))) != INVALID_FILE_ATTRIBUTES)
        fail(c, "Existing rollback journal: recover with SQLite or remove disposable projection after stopping consumers");
    c->temporary = full_path(c, directory, format_text(c, "native-%lu.sqlite", (unsigned long)GetCurrentProcessId()));
    {
        const char *suffixes[] = {"", "-wal", "-shm", "-journal"};
        for (i = 0; i < 4; i++) if (GetFileAttributesW(wide(c, format_text(c, "%s%s", c->temporary, suffixes[i]))) != INVALID_FILE_ATTRIBUTES) {
            c->temporary = NULL; fail(c, "Temporary database already exists");
        }
    }
    if (sqlite3_open(c->temporary, &c->db) != SQLITE_OK) fail(c, "Cannot create SQLite database");
    projection_writer(c);
    sql(c, "CREATE TABLE __attendant_diagnostics (collection_name TEXT, source_path TEXT NOT NULL, code TEXT NOT NULL, message TEXT NOT NULL);");
    sql(c, "CREATE TABLE __attendant_metadata (key TEXT PRIMARY KEY, value TEXT NOT NULL);");
    sql(c, "CREATE TABLE __attendant_cache (source_path TEXT PRIMARY KEY, collection_name TEXT NOT NULL, mtime_ms REAL NOT NULL, size INTEGER NOT NULL, source_hash TEXT NOT NULL, front_matter TEXT NOT NULL, local_diagnostics TEXT NOT NULL, content TEXT NOT NULL);");
    sql(c, "CREATE TABLE __attendant_edges (source_collection TEXT NOT NULL, source_path TEXT NOT NULL, source_id TEXT NOT NULL, field TEXT NOT NULL, target_collection TEXT NOT NULL, target_name TEXT NOT NULL, target_path TEXT);");
    metadata(c, "layout_version", "2");
    metadata(c, "config_hash", c->config_hash);
    for (i = 0; i < c->collection_count; i++) {
        collection *table = &c->collections[i];
        metadata(c, format_text(c, "schema_hash:%s", table->name), schema_digest(c, table));
        make_tables(c, table);
    }
    for (i = 0; i < c->diagnostic_count; i++) {
        diagnostic *d = &c->diagnostics[i];
        sqlite3_stmt *s = prepare(c, "INSERT INTO __attendant_diagnostics VALUES (?,?,?,?)");
        if (d->owner) bind_text(c, s, 1, d->owner->name);
        bind_text(c, s, 2, d->path); bind_text(c, s, 3, d->code); bind_text(c, s, 4, d->message); finish(c, s);
    }
    c->projected = 0;
    for (i = 0; i < c->record_count; i++) project_record(c, &c->records[i]);
    make_edges(c);
    sql(c, "COMMIT; PRAGMA optimize;");
    if (sqlite3_wal_checkpoint_v2(c->db, NULL, SQLITE_CHECKPOINT_TRUNCATE, NULL, NULL) != SQLITE_OK) fail(c, "Cannot checkpoint temporary projection");
    if (sqlite3_close(c->db) != SQLITE_OK) fail(c, "Cannot close database");
    c->db = NULL;
    if (!MoveFileExW(wide(c, c->temporary), wide(c, target), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) fail(c, "Cannot publish database (is another process using it?)");
    c->temporary = NULL;
}
static const char *refresh_projection(context *c, const char *root, projection_mode mode) {
    const char *target = full_path(c, root, ".attendant/attendant.sqlite");
    int i, reuse = 0, incremental = 0;
    const char *state = "reused";
    projection_delta *delta = allocate(c, sizeof(*delta));
    projection_requirements(c);
    /* Do not let a read probe recover or publish over a hot rollback journal. */
    if (GetFileAttributesW(wide(c, format_text(c, "%s-journal", target))) != INVALID_FILE_ATTRIBUTES)
        fail(c, "Existing rollback journal: recover with SQLite or remove disposable projection after stopping consumers");
    if (mode == PROJECTION_REUSE) {
        if (sqlite3_open_v2(target, &c->db, SQLITE_OPEN_READONLY, NULL) == SQLITE_OK) {
            if (sqlite3_exec(c->db, "BEGIN", NULL, NULL, NULL) == SQLITE_OK && snapshot_integrity(c)) {
                reuse = snapshot_matches(c);
                if (!reuse) incremental = plan_projection_delta(c, delta);
            }
        }
        if (c->db && sqlite3_close(c->db) != SQLITE_OK) fail(c, "Cannot close projection freshness check");
        c->db = NULL;
    }
    if (!reuse && incremental) {
        if (sqlite3_open_v2(target, &c->db, SQLITE_OPEN_READWRITE, NULL) != SQLITE_OK) fail(c, "Cannot open projection writer");
        projection_writer(c);
        if (snapshot_integrity(c) && plan_projection_delta(c, delta)) { state = apply_projection_delta(c, delta); reuse = 1; }
        else sql(c, "ROLLBACK");
        if (sqlite3_close(c->db) != SQLITE_OK) fail(c, "Cannot close projection writer");
        c->db = NULL;
    }
    if (!reuse) { rebuild_projection(c, root); return "rebuilt"; }
    c->projected = 0;
    for (i = 0; i < c->record_count; i++) {
        value *hidden = lookup(&c->records[i].front, "hidden");
        if (!c->records[i].blocked && !(hidden && hidden->number.real)) c->projected++;
    }
    return state;
}
static const char *prepare_projection(context *c, const char *root, projection_mode mode) {
    int i;
    load_source(c, root);
    if (!c->config_hash || c->contract_diagnostic_count) fail(c, "Invalid config/schema: %s", c->diagnostics[0].message);
    for (i = 0; i < c->collection_count; i++) if (c->collections[i].invalid) fail(c, "Cannot prepare inaccessible or invalid collection: %s", c->collections[i].name);
    if (c->auto_correct && plan_corrections(c)) {
        c->applied_corrections = corrections_json(c);
        apply_corrections(c); reload_records(c);
    }
    return refresh_projection(c, root, mode);
}
static void print_sync(context *c, const char *state) {
    jsonb b;
    char *out = c->json_scratch;
    jsonb_init(&b); json_check(jsonb_object(&b, out, OUTPUT_CAPACITY));
    json_check(jsonb_key(&b, out, OUTPUT_CAPACITY, "collections", 11)); json_number(c, &b, out, OUTPUT_CAPACITY, c->collection_count);
    json_check(jsonb_key(&b, out, OUTPUT_CAPACITY, "records", 7)); json_number(c, &b, out, OUTPUT_CAPACITY, c->projected);
    json_check(jsonb_key(&b, out, OUTPUT_CAPACITY, "diagnostics", 11)); json_diagnostics(c, &b, out, OUTPUT_CAPACITY, NULL);
    json_property(&b, out, OUTPUT_CAPACITY, "state", state);
    json_check(jsonb_object_pop(&b, out, OUTPUT_CAPACITY)); printf("%.*s\n", (int)b.pos, out);
}
