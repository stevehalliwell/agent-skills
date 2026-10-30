/* Record diagnostics are shared by validate, projection and write preflight. */
static void json_diagnostics(context *c, jsonb *b, char *out, size_t capacity, record *filter) {
    int i;
    json_check(jsonb_array(b, out, capacity));
    for (i = 0; i < c->diagnostic_count; i++) {
        diagnostic *d = &c->diagnostics[i];
        if (filter && strcmp(filter->path, d->path)) continue;
        json_check(jsonb_object(b, out, capacity));
        json_property(b, out, capacity, "code", d->code);
        json_property(b, out, capacity, "path", d->path);
        json_property(b, out, capacity, "message", d->message);
        json_check(jsonb_object_pop(b, out, capacity));
    }
    json_check(jsonb_array_pop(b, out, capacity));
}
static const char *record_diagnostics_json(context *c, record *r) {
    jsonb b;
    jsonb_init(&b); json_diagnostics(c, &b, c->json_scratch, OUTPUT_CAPACITY, r);
    return copy_text(c, c->json_scratch, b.pos);
}
static int valid_uuid(value *v) {
    int i;
    if (!v || v->kind != VALUE_STRING || strlen(v->text) != 36 || v->text[14] != '7' || !strchr("89abAB", v->text[19])) return 0;
    for (i = 0; i < 36; i++) {
        if (i == 8 || i == 13 || i == 18 || i == 23) { if (v->text[i] != '-') return 0; }
        else if (!strchr("0123456789abcdefABCDEF", v->text[i])) return 0;
    }
    return 1;
}
static int record_string_array(value *v) {
    int i;
    if (!v || v->kind != VALUE_ARRAY) return 0;
    for (i = 0; i < v->count; i++) if (v->items[i].kind != VALUE_STRING) return 0;
    return 1;
}
static void record_problem(context *c, record *r, const char *code, const char *message) {
    add_diagnostic(c, r->owner, r->path, code, message);
    if (strcmp(code, "record-ref-missing")) r->blocked = 1;
}
static void validate_record(context *c, record *r) {
    value *v;
    FILETIME time;
    int i, j;
    if (r->front.problem) { record_problem(c, r, r->front.problem_code, r->front.problem); return; }
    if (!r->front.present) { r->blocked = 1; return; }
    v = lookup(&r->front, "name");
    if (v && v->kind == VALUE_STRING) r->validation_name = v->text;
    else {
        const char *base = strrchr(r->path, '\\'); base = base ? base + 1 : r->path;
        r->validation_name = copy_text(c, base, strlen(base) - 3);
    }
    v = lookup(&r->front, "id"); if (v && !valid_uuid(v)) record_problem(c, r, "record-id", "id must be UUIDv7.");
    v = lookup(&r->front, "name"); if (v && v->kind != VALUE_STRING) record_problem(c, r, "record-name", "name must be a string.");
    v = lookup(&r->front, "created_at"); if (v && (v->kind != VALUE_STRING || !parse_datetime(v->text, &time))) record_problem(c, r, "record-created-at", "created_at must be YYYY-MM-DDTHH:mm:ss.sssZ.");
    v = lookup(&r->front, "desc"); if (v && v->kind != VALUE_STRING) record_problem(c, r, "record-desc", "desc must be a string.");
    v = lookup(&r->front, "tags"); if (v && !record_string_array(v)) record_problem(c, r, "record-tags", "tags must be a string array.");
    v = lookup(&r->front, "hidden"); if (v && v->kind != VALUE_BOOL) record_problem(c, r, "record-hidden", "hidden must be boolean.");
    for (i = 0; i < r->owner->field_count; i++) {
        field *f = &r->owner->fields[i];
        v = lookup(&r->front, f->name);
        if (!v || v->kind == VALUE_NULL) {
            if (!f->optional && strcmp(f->kind, "default") && strcmp(f->kind, "enum") && strcmp(f->kind, "string-array") && strcmp(f->kind, "untyped"))
                record_problem(c, r, "record-required", format_text(c, "%s is required.", f->name));
            continue;
        }
        if (v->kind == VALUE_NUMBER && v->number_kind == NUMBER_U64) record_problem(c, r, "record-number-range", format_text(c, "%s exceeds SQLite signed 64-bit INTEGER range; use a string field for oversized identifiers.", f->name));
        if (!strcmp(f->kind, "default") && v->kind != f->fallback.kind) record_problem(c, r, "record-type", format_text(c, "%s must match its scalar default type.", f->name));
        if (!strcmp(f->kind, "enum")) {
            for (j = 0; j < f->declaration.count; j++) if (scalar_equal(*v, f->declaration.items[j])) break;
            if (j == f->declaration.count) record_problem(c, r, "record-enum", format_text(c, "%s must be one of its declared enum values.", f->name));
        }
        if (!strcmp(f->kind, "date") && (v->kind != VALUE_STRING || strlen(v->text) != 10 || !valid_date(v->text))) record_problem(c, r, "record-date", format_text(c, "%s must be YYYY-MM-DD.", f->name));
        if (!strcmp(f->kind, "datetime") && (v->kind != VALUE_STRING || !parse_datetime(v->text, &time))) record_problem(c, r, "record-datetime", format_text(c, "%s must be YYYY-MM-DDTHH:mm:ss.sssZ.", f->name));
        if (!strcmp(f->kind, "ref") && v->kind != VALUE_STRING) record_problem(c, r, "record-ref", format_text(c, "%s must be a collection/name string.", f->name));
        if (!strcmp(f->kind, "ref-array") && !record_string_array(v)) record_problem(c, r, "record-ref", format_text(c, "%s must be a collection/name string array.", f->name));
        if (!strcmp(f->kind, "string-array") && !record_string_array(v)) record_problem(c, r, "record-string-array", format_text(c, "%s must be a string array.", f->name));
    }
}
static void validate_reference(context *c, record *r, field *f, const char *text) {
    const char *slash = strchr(text, '/'), *name = slash ? slash + 1 : text;
    const char *table = slash ? copy_text(c, text, slash - text) : r->owner->name;
    int i;
    if (!*name || (slash ? slash == text : strcmp(f->name, "depends_on") != 0)) {
        record_problem(c, r, "record-ref", format_text(c, "%s must use collection/name.", f->name)); return;
    }
    for (i = 0; i < c->record_count; i++) {
        record *target = &c->records[i];
        if (target->front.problem || !target->front.present || strcmp(table, target->owner->name)) continue;
        if (target->validation_name && !strcmp(name, target->validation_name)) return;
    }
    record_problem(c, r, "record-ref-missing", format_text(c, "%s target %s was not found.", f->name, text));
}
static void validate_dataset(context *c) {
    int i, j, k;
    for (i = 0; i < c->record_count; i++) {
        record *r = &c->records[i];
        value *id;
        int duplicate_id = 0, duplicate_name = 0;
        if (r->front.problem || !r->front.present) continue;
        id = lookup(&r->front, "id");
        for (j = 0; j < i; j++) {
            record *prior = &c->records[j];
            value *other_id;
            if (prior->owner != r->owner || prior->front.problem || !prior->front.present) continue;
            other_id = lookup(&prior->front, "id");
            if (id && other_id && id->kind == VALUE_STRING && other_id->kind == VALUE_STRING && !strcmp(id->text, other_id->text)) duplicate_id = 1;
            if (r->validation_name && prior->validation_name && !strcmp(r->validation_name, prior->validation_name)) duplicate_name = 1;
        }
        if (duplicate_id) record_problem(c, r, "record-id-duplicate", format_text(c, "Duplicate id %s.", id->text));
        if (duplicate_name) record_problem(c, r, "record-name-duplicate", format_text(c, "Duplicate name %s.", r->validation_name));
        for (j = 0; j < r->owner->field_count; j++) {
            field *f = &r->owner->fields[j];
            value *v = lookup(&r->front, f->name);
            if (!v) continue;
            if (!strcmp(f->kind, "ref") && v->kind == VALUE_STRING) validate_reference(c, r, f, v->text);
            if (!strcmp(f->kind, "ref-array") && v->kind == VALUE_ARRAY) for (k = 0; k < v->count; k++)
                if (v->items[k].kind == VALUE_STRING) validate_reference(c, r, f, v->items[k].text);
        }
    }
}
static void projection_requirements(context *c) {
    int i, j;
    static const char *mandatory[] = {"id", "name", "created_at", "desc", "tags"};
    for (i = 0; i < c->record_count; i++) {
        record *r = &c->records[i];
        value *hidden = lookup(&r->front, "hidden");
        if (r->blocked || (hidden && hidden->kind == VALUE_BOOL && hidden->number.real)) continue;
        for (j = 0; j < 5; j++) if (!lookup(&r->front, mandatory[j])) {
            record_problem(c, r, "record-missing-mandatory", format_text(c, "Missing mandatory Attendant field: %s.", mandatory[j])); break;
        }
    }
}
