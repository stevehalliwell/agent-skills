/* Source edits own only requested value spans or appended fields. Never emit an
   existing document through LibYAML: comments, style and body stay source-owned. */
__declspec(dllimport) LONG WINAPI BCryptGenRandom(void *, PUCHAR, ULONG, ULONG);
/* Escape characters JSON permits literally but YAML folds or rejects. */
static int yaml_escape_width(const unsigned char *p) {
    int width = yaml_linebreak_width(p);
    if (width) return width;
    if (*p == 0x7f) return 1;
    if (p[0] == 0xc2 && p[1] >= 0x80 && p[1] <= 0x9f) return 2;
    if (p[0] == 0xef && p[1] == 0xbf && (p[2] == 0xbe || p[2] == 0xbf)) return 3;
    return 0;
}
static const char *yaml_value(context *c, value v) {
    jsonb b;
    const char *json;
    const unsigned char *p;
    jsonb_init(&b); json_value_format(c, &b, c->json_scratch, TEXT_CAPACITY, v, YAML_NUMBERS);
    json = copy_text(c, c->json_scratch, b.pos);
    p = (const unsigned char *)json;
    size_t extra = 0, used = 0;
    char *out;
    while (*p) {
        int width = yaml_escape_width(p);
        if (width) { extra += 6 - width; p += width; }
        else p++;
    }
    if (!extra) return json;
    out = allocate(c, strlen(json) + extra + 1); p = (const unsigned char *)json;
    while (*p) {
        int width = yaml_escape_width(p);
        if (width) {
            unsigned point = width == 1 ? p[0] : width == 2 ? ((p[0] & 31u) << 6) | (p[1] & 63u) : ((p[0] & 15u) << 12) | ((p[1] & 63u) << 6) | (p[2] & 63u);
            snprintf(out + used, 7, "\\u%04x", point); used += 6; p += width;
        } else out[used++] = (char)*p++;
    }
    return out;
}
static value text_value(const char *text) { value v = {0}; v.kind = VALUE_STRING; v.text = text; return v; }
static void mapping_set(context *c, mapping *map, const char *key, value data) {
    value *old = lookup(map, key);
    if (old) { *old = data; return; }
    if (!map->pairs) map->pairs = allocate(c, MAX_PAIRS * sizeof(pair));
    if (map->count == MAX_PAIRS) fail(c, "Mapping key capacity exceeded");
    map->pairs[map->count].key = key; map->pairs[map->count++].data = data; map->present = 1;
}
static const char *uuid_v7(context *c) {
    unsigned char bytes[16];
    FILETIME now;
    ULARGE_INTEGER ticks;
    ULONGLONG milliseconds;
    char text[37];
    if (BCryptGenRandom(NULL, bytes, sizeof(bytes), 2 /* BCRYPT_USE_SYSTEM_PREFERRED_RNG */) < 0) fail(c, "Windows secure random source unavailable");
    GetSystemTimeAsFileTime(&now); ticks.LowPart = now.dwLowDateTime; ticks.HighPart = now.dwHighDateTime;
    milliseconds = (ticks.QuadPart - 116444736000000000ULL) / 10000ULL;
    bytes[0] = (unsigned char)(milliseconds >> 40); bytes[1] = (unsigned char)(milliseconds >> 32);
    bytes[2] = (unsigned char)(milliseconds >> 24); bytes[3] = (unsigned char)(milliseconds >> 16);
    bytes[4] = (unsigned char)(milliseconds >> 8); bytes[5] = (unsigned char)milliseconds;
    bytes[6] = (bytes[6] & 15) | 0x70; bytes[8] = (bytes[8] & 63) | 0x80;
    snprintf(text, sizeof(text), "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
        bytes[0],bytes[1],bytes[2],bytes[3],bytes[4],bytes[5],bytes[6],bytes[7],bytes[8],bytes[9],bytes[10],bytes[11],bytes[12],bytes[13],bytes[14],bytes[15]);
    return copy_text(c, text, 36);
}
static const char *parent_path(context *c, const char *path) {
    const char *a = strrchr(path, '\\'), *b = strrchr(path, '/'), *end = a && (!b || a > b) ? a : b;
    if (!end) fail(c, "Expected absolute target path");
    return copy_text(c, path, end - path);
}
static const char *stage_source(context *c, const char *target, const char *text) {
    const char *temporary;
    HANDLE file;
    DWORD written;
    int ok;
    size_t length = strlen(text);
    c->error_path = target;
    temporary = format_text(c, "%s\\.attendant-%s.tmp", parent_path(c, target), uuid_v7(c));
    if (length > TEXT_CAPACITY) fail(c, "Generated source exceeds 4 MiB capacity");
    if (c->staged_count == MAX_RECORDS) fail(c, "Source staging capacity exceeded");
    file = CreateFileW(wide(c, temporary), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) fail(c, "Cannot stage source for %s", target);
    c->staged[c->staged_count++] = temporary;
    ok = WriteFile(file, text, (DWORD)length, &written, NULL) && written == length && FlushFileBuffers(file);
    if (!CloseHandle(file)) ok = 0;
    if (!ok) fail(c, "Cannot finish source staging for %s", target);
    c->error_path = NULL;
    return temporary;
}
static void publish_source(context *c, const char *temporary, const char *target, const char *original) {
    int i;
    DWORD attributes;
    c->error_path = target;
    attributes = GetFileAttributesW(wide(c, target));
    if (original) {
        if (attributes == INVALID_FILE_ATTRIBUTES || (attributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT))) fail(c, "Source target changed or is not a regular file: %s", target);
        if (strcmp(read_text(c, target, NULL), original)) fail(c, "Source changed since preflight: %s", target);
    } else if (attributes != INVALID_FILE_ATTRIBUTES || (GetLastError() != ERROR_FILE_NOT_FOUND && GetLastError() != ERROR_PATH_NOT_FOUND)) fail(c, "Target exists or cannot be checked: %s", target);
    if (c->written_count == MAX_RECORDS) fail(c, "Written-path reporting capacity exceeded");
    if (!MoveFileExW(wide(c, temporary), wide(c, target), MOVEFILE_WRITE_THROUGH | (original ? MOVEFILE_REPLACE_EXISTING : 0))) fail(c, "Cannot publish source: %s", target);
    for (i = 0; i < c->staged_count; i++) if (c->staged[i] == temporary) { c->staged[i] = NULL; break; }
    c->written[c->written_count++] = target;
    c->error_path = NULL;
}
typedef struct source_edit { size_t start, end; const char *replacement; } source_edit;
static int compare_edits(const void *left, const void *right) {
    const source_edit *a = left, *b = right;
    return a->start < b->start ? -1 : a->start > b->start;
}
static const char *patch_record(context *c, record *r, mapping *changes) {
    source_edit edits[MAX_PAIRS];
    const char *newline = !strncmp(r->text, "---\r\n", 5) ? "\r\n" : "\n";
    const char *closing = r->text + (!strncmp(r->text, "---\r\n", 5) ? 5 : 4);
    char *added, *output;
    size_t added_length = 0, added_capacity = 1, length = strlen(r->text), cursor = 0, used = 0;
    int count = 0, i, j;
    for (i = 0; i < changes->count; i++) if (!lookup(&r->front, changes->pairs[i].key)) {
        added_capacity += strlen(changes->pairs[i].key) * 6 + strlen(yaml_value(c, changes->pairs[i].data)) + 8;
        if (added_capacity > TEXT_CAPACITY) fail(c, "Appended fields exceed source capacity");
    }
    added = allocate(c, added_capacity);
    while (*closing) {
        if (!strncmp(closing, "---\n", 4) || !strncmp(closing, "---\r\n", 5) || !strcmp(closing, "---")) break;
        closing = strchr(closing, '\n'); if (!closing) fail(c, "Closing delimiter lost during edit preflight"); closing++;
    }
    if (!*closing || r->front.problem || !r->front.present) fail(c, "Cannot edit unsupported front matter: %s", r->path);
    for (i = 0; i < changes->count; i++) {
        pair *change = &changes->pairs[i], *old = NULL;
        const char *replacement = yaml_value(c, change->data);
        for (j = 0; j < r->front.count; j++) if (!strcmp(change->key, r->front.pairs[j].key)) { old = &r->front.pairs[j]; break; }
        if (old) {
            const char *prefix = old->indentless_sequence ? "  " : old->value_start && r->text[old->value_start - 1] == ':' ? " " : "";
            const char *suffix = old->value_end > old->value_start && r->text[old->value_end - 1] == '\n' ? newline : "";
            edits[count].start = old->value_start; edits[count].end = old->value_end;
            /* Replacement can be large; do not pass it through format_text's 32 KiB buffer. */
            {
                size_t a = strlen(prefix), b = strlen(replacement), d = strlen(suffix);
                char *joined = allocate(c, a + b + d + 1);
                memcpy(joined, prefix, a); memcpy(joined + a, replacement, b); memcpy(joined + a + b, suffix, d);
                edits[count++].replacement = joined;
            }
        } else {
            const char *key = yaml_value(c, text_value(change->key));
            size_t a = strlen(key), b = strlen(replacement), d = strlen(newline);
            if (added_length + a + b + d + 2 >= added_capacity) fail(c, "Appended fields exceed source capacity");
            memcpy(added + added_length, key, a); added_length += a;
            memcpy(added + added_length, ": ", 2); added_length += 2;
            memcpy(added + added_length, replacement, b); added_length += b;
            memcpy(added + added_length, newline, d); added_length += d;
        }
    }
    qsort(edits, count, sizeof(source_edit), compare_edits);
    for (i = 0; i < count; i++) {
        if (edits[i].start < cursor || edits[i].end < edits[i].start || edits[i].end > (size_t)(closing - r->text)) fail(c, "Unsafe YAML value span: %s", r->path);
        length = length - (edits[i].end - edits[i].start) + strlen(edits[i].replacement); cursor = edits[i].end;
    }
    if (length + added_length > TEXT_CAPACITY) fail(c, "Edited record exceeds source capacity");
    output = allocate(c, length + added_length + 1); cursor = 0;
    for (i = 0; i < count; i++) {
        size_t size = edits[i].start - cursor, replacement = strlen(edits[i].replacement);
        memcpy(output + used, r->text + cursor, size); used += size;
        memcpy(output + used, edits[i].replacement, replacement); used += replacement; cursor = edits[i].end;
    }
    memcpy(output + used, r->text + cursor, closing - r->text - cursor); used += closing - r->text - cursor;
    memcpy(output + used, added, added_length); used += added_length;
    memcpy(output + used, closing, strlen(closing));
    {
        mapping parsed = read_mapping(c, output, NULL);
        if (parsed.problem || !parsed.present) fail(c, "Edited YAML did not parse safely: %s", r->path);
        for (i = 0; i < changes->count; i++) {
            value *actual = lookup(&parsed, changes->pairs[i].key);
            if (!actual || !value_equal(changes->pairs[i].data, *actual)) fail(c, "Edited YAML changed field meaning: %s", r->path);
        }
    }
    return output;
}
static value *planned_value(record *r, const char *key) {
    value *v = lookup(&r->corrections, key);
    return v ? v : lookup(&r->front, key);
}
typedef struct repair_candidate { record *r; const char *name, *id; ULONGLONG created; } repair_candidate;
static int compare_candidates(const void *left, const void *right) {
    const repair_candidate *a = left, *b = right;
    if (a->created != b->created) return a->created < b->created ? -1 : 1;
    return strcmp(a->r->path, b->r->path);
}
static int compare_birth_candidates(const void *left, const void *right) {
    const repair_candidate *a = *(repair_candidate *const *)left, *b = *(repair_candidate *const *)right;
    if (a->r->created_on_disk != b->r->created_on_disk) return a->r->created_on_disk < b->r->created_on_disk ? -1 : 1;
    return strcmp(a->r->path, b->r->path);
}
static void plan_duplicates(context *c, repair_candidate *candidates, int count, int names) {
    int handled[MAX_RECORDS] = {0}, i, j, k;
    repair_candidate *group[MAX_RECORDS];
    for (i = 0; i < count; i++) {
        repair_candidate *a = &candidates[i];
        const char *key = names ? a->name : a->id;
        int members = 0, start, end, suffix = 2;
        if (handled[i] || !key) continue;
        for (j = i; j < count; j++) {
            const char *other = names ? candidates[j].name : candidates[j].id;
            if (a->r->owner == candidates[j].r->owner && other && !strcmp(key, other)) { group[members++] = &candidates[j]; handled[j] = 1; }
        }
        if (members < 2) continue;
        /* Source created_at first; within each tied cohort use birth time only
           if every member has one. Otherwise retain deterministic path order. */
        for (start = 0; start < members; start = end) {
            int all_births = 1;
            for (end = start; end < members && group[end]->created == group[start]->created; end++) if (!group[end]->r->created_on_disk) all_births = 0;
            if (all_births) qsort(group + start, end - start, sizeof(group[0]), compare_birth_candidates);
        }
        for (j = 1; j < members; j++) {
            if (!names) mapping_set(c, &group[j]->r->corrections, "id", text_value(uuid_v7(c)));
            else {
                const char *next;
                for (;;) {
                    next = format_text(c, "%s (%d)", key, suffix++);
                    for (k = 0; k < count; k++) {
                        value *used = planned_value(candidates[k].r, "name");
                        if (a->r->owner == candidates[k].r->owner && used && used->kind == VALUE_STRING && !strcmp(used->text, next)) break;
                    }
                    if (k == count) break;
                }
                mapping_set(c, &group[j]->r->corrections, "name", text_value(next));
            }
        }
    }
}
static int plan_corrections(context *c) {
    repair_candidate *candidates = allocate(c, c->record_count * sizeof(repair_candidate));
    FILETIME now;
    const char *materialized;
    int count = 0, corrected = 0, i, j;
    GetSystemTimeAsFileTime(&now); materialized = iso_time(c, now);
    for (i = 0; i < c->record_count; i++) {
        record *r = &c->records[i];
        const char *base;
        value empty_array = {0}, *v;
        repair_candidate *candidate;
        FILETIME created;
        ULARGE_INTEGER ticks;
        if (r->front.problem || !r->front.present || r->owner->invalid) continue;
        empty_array.kind = VALUE_ARRAY;
        if (!valid_uuid(lookup(&r->front, "id"))) mapping_set(c, &r->corrections, "id", text_value(uuid_v7(c)));
        base = strrchr(r->path, '\\'); base = base ? base + 1 : r->path;
        if (!lookup(&r->front, "name")) mapping_set(c, &r->corrections, "name", text_value(copy_text(c, base, strlen(base) - 3)));
        if (!lookup(&r->front, "created_at")) mapping_set(c, &r->corrections, "created_at", text_value(materialized));
        if (!lookup(&r->front, "desc")) mapping_set(c, &r->corrections, "desc", text_value(""));
        if (!lookup(&r->front, "tags")) mapping_set(c, &r->corrections, "tags", empty_array);
        for (j = 0; j < r->owner->field_count; j++) {
            field *f = &r->owner->fields[j];
            if (lookup(&r->front, f->name)) continue;
            if (!strcmp(f->kind, "default") || !strcmp(f->kind, "enum")) mapping_set(c, &r->corrections, f->name, f->fallback);
            else if (!strcmp(f->kind, "string-array")) mapping_set(c, &r->corrections, f->name, empty_array);
        }
        candidate = &candidates[count++]; candidate->r = r; candidate->created = ~(ULONGLONG)0;
        v = planned_value(r, "name"); candidate->name = v && v->kind == VALUE_STRING ? v->text : NULL;
        v = planned_value(r, "id"); candidate->id = v && v->kind == VALUE_STRING ? v->text : "";
        v = lookup(&r->front, "created_at");
        if (v && v->kind == VALUE_STRING && parse_datetime(v->text, &created)) {
            ticks.LowPart = created.dwLowDateTime; ticks.HighPart = created.dwHighDateTime; candidate->created = ticks.QuadPart;
        }
    }
    qsort(candidates, count, sizeof(repair_candidate), compare_candidates);
    plan_duplicates(c, candidates, count, 1);
    plan_duplicates(c, candidates, count, 0);
    for (i = 0; i < c->record_count; i++) if (c->records[i].corrections.count) {
        c->records[i].next_text = patch_record(c, &c->records[i], &c->records[i].corrections); corrected++;
    }
    return corrected;
}
static const char *corrections_json(context *c) {
    char *out = allocate(c, OUTPUT_CAPACITY);
    jsonb b;
    int i;
    jsonb_init(&b); json_check(jsonb_array(&b, out, OUTPUT_CAPACITY));
    for (i = 0; i < c->record_count; i++) {
        record *r = &c->records[i];
        const char *fields;
        if (!r->corrections.count) continue;
        fields = mapping_json(c, &r->corrections);
        json_check(jsonb_object(&b, out, OUTPUT_CAPACITY)); json_property(&b, out, OUTPUT_CAPACITY, "path", r->path);
        json_check(jsonb_key(&b, out, OUTPUT_CAPACITY, "fields", 6)); json_check(jsonb_token(&b, out, OUTPUT_CAPACITY, fields, strlen(fields)));
        json_check(jsonb_object_pop(&b, out, OUTPUT_CAPACITY));
    }
    json_check(jsonb_array_pop(&b, out, OUTPUT_CAPACITY)); return out;
}
static void apply_corrections(context *c) {
    const char *staged[MAX_RECORDS] = {0};
    int i;
    for (i = 0; i < c->record_count; i++) if (c->records[i].next_text) staged[i] = stage_source(c, c->records[i].path, c->records[i].next_text);
    for (i = 0; i < c->record_count; i++) if (staged[i]) publish_source(c, staged[i], c->records[i].path, c->records[i].text);
    c->staged_count = 0;
}
