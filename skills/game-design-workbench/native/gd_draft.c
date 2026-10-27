/* Skill-only transient edits. Baseline text is borrowed, never serialized back.
   History refuses new edits at capacity so Ctrl+Z can always reach disk baseline. */
#if !defined(_WIN32) && !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif
#include "gd.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

GD_Draft *gd_draft_create(const GD_Model *baseline) {
    GD_Draft *draft;
    GD_Model *model;
    if (!baseline->skill_mode || baseline->error_count || baseline->errors_omitted) return NULL;
    draft = baseline->allocator.allocate(baseline->allocator.context, sizeof(*draft));
    if (!draft) return NULL;
    memset(draft, 0, sizeof(*draft)); draft->baseline = baseline;
    model = draft->model = gd_create(baseline->allocator);
    if (!model) { baseline->allocator.release(baseline->allocator.context, draft); return NULL; }
    /* No text/input buffer copy: all strings retain immutable baseline ownership. */
    model->collection_count = baseline->collection_count;
    memcpy(model->collections, baseline->collections, sizeof(model->collections));
    model->record_count = baseline->record_count;
    memcpy(model->records, baseline->records, baseline->record_count*sizeof(model->records[0]));
    model->edge_count = baseline->edge_count;
    memcpy(model->edges, baseline->edges, baseline->edge_count*sizeof(model->edges[0]));
    model->skill_mode = 1; model->route_mode = baseline->route_mode; model->source_digest = baseline->source_digest;
    gd_recompute_skills(model); return draft;
}
void gd_draft_destroy(GD_Draft *draft) {
    if (draft) {
        GD_Allocator allocator = draft->baseline->allocator;
        gd_destroy(draft->model); allocator.release(allocator.context, draft);
    }
}
static int compare_reference(const void *a, const void *b) { return strcmp(*(const char *const *)a, *(const char *const *)b); }
static int prerequisite_set(const GD_Model *model, int record, const char **references) {
    int i, count = 0, unique = 0;
    for (i = 0; i < model->edge_count; ++i)
        if (model->edges[i].source == record && model->edges[i].field == GD_PREREQUISITES)
            references[count++] = model->edges[i].reference;
    qsort(references, count, sizeof(*references), compare_reference);
    for (i = 0; i < count; ++i) if (!unique || strcmp(references[i], references[unique-1])) references[unique++] = references[i];
    return unique;
}
static void changed(GD_Draft *draft, int record) {
    const GD_Record *base = &draft->baseline->records[record], *edited = &draft->model->records[record];
    int was_modified = draft->rank_changed[record] || draft->prerequisites_changed[record];
    int a = prerequisite_set(draft->baseline, record, draft->scratch_a);
    int b = prerequisite_set(draft->model, record, draft->scratch_b), i, same = a == b;
    for (i = 0; same && i < a; ++i) same = !strcmp(draft->scratch_a[i], draft->scratch_b[i]);
    draft->rank_changed[record] = base->has_rank != edited->has_rank || (base->has_rank && base->rank != edited->rank);
    draft->prerequisites_changed[record] = !same;
    draft->modified_count += (draft->rank_changed[record] || draft->prerequisites_changed[record]) - was_modified;
    gd_recompute_skills(draft->model);
}
static int can_edit(GD_Draft *draft, int record) {
    draft->message[0] = 0;
    if (record < 0 || record >= draft->model->record_count || draft->model->records[record].role != GD_SKILL) {
        snprintf(draft->message, sizeof(draft->message), "Select a loaded skill to edit"); return 0;
    }
    return 1;
}
static int history_available(GD_Draft *draft) {
    if (draft->history_count < GD_MAX_DRAFT_EDITS) return 1;
    snprintf(draft->message, sizeof(draft->message), "4096 edit history limit reached; undo before editing. Disk sync point is retained."); return 0;
}
int gd_draft_rank(GD_Draft *draft, int record, const char *text) {
    uint64_t magnitude = 0, limit = UINT64_C(9007199254740991);
    const unsigned char *p = (const unsigned char *)text;
    int has_rank, negative = 0;
    int64_t value;
    GD_Record *edited;
    GD_DraftEdit *action;
    if (!can_edit(draft, record)) return -1;
    has_rank = strcmp(text, "null") != 0;
    if (has_rank) {
        if (*p == '-' || *p == '+') negative = *p++ == '-';
        if (!*p) goto invalid;
        for (; *p; ++p) {
            unsigned digit = *p - '0';
            if (digit > 9 || magnitude > (limit-digit)/10) goto invalid;
            magnitude = magnitude*10+digit;
        }
    }
    value = negative ? -(int64_t)magnitude : (int64_t)magnitude;
    edited = &draft->model->records[record];
    if (edited->has_rank == has_rank && (!has_rank || edited->rank == value)) return 0;
    if (!history_available(draft)) return -1;
    action = &draft->history[draft->history_count++]; memset(action, 0, sizeof(*action));
    action->record = record; action->has_rank = edited->has_rank; action->rank = edited->rank;
    edited->has_rank = has_rank; edited->rank = value; changed(draft, record); return 1;
invalid:
    snprintf(draft->message, sizeof(draft->message), "Rank requires null or a signed decimal integer in [-9007199254740991,9007199254740991]"); return -1;
}
static int reference_count(const GD_Model *model, int record, const char *reference) {
    int i, count = 0;
    for (i = 0; i < model->edge_count; ++i) {
        const GD_Edge *edge = &model->edges[i];
        if (edge->source == record && edge->field == GD_PREREQUISITES && !strcmp(edge->reference, reference)) ++count;
    }
    return count;
}
static void replace_connection(GD_Model *model, int record, const char *reference, int target, int count) {
    int i, retained = 0;
    for (i = 0; i < model->edge_count; ++i) {
        GD_Edge edge = model->edges[i];
        if (edge.source != record || edge.field != GD_PREREQUISITES || strcmp(edge.reference, reference)) model->edges[retained++] = edge;
    }
    assert(retained+count <= GD_MAX_EDGES);
    for (i = 0; i < count; ++i) model->edges[retained++] = (GD_Edge){record, target, GD_PREREQUISITES, reference, 0};
    model->edge_count = retained;
}
static int target_record(const GD_Model *model, const char *reference) {
    int i;
    for (i = 0; i < model->record_count; ++i) if (!strcmp(model->records[i].reference, reference)) return i;
    return -1;
}
int gd_draft_prerequisite(GD_Draft *draft, int record, const char *reference, int enabled) {
    GD_Model *model = draft->model;
    GD_DraftEdit *action;
    const char *stored = NULL;
    int count, target, i;
    if (!can_edit(draft, record)) return -1;
    count = reference_count(model, record, reference);
    if ((enabled && count) || (!enabled && !count)) return 0;
    target = target_record(model, reference);
    if (enabled && (target < 0 || model->records[target].role != GD_SKILL)) {
        snprintf(draft->message, sizeof(draft->message), "New prerequisite must be a loaded skill"); return -1;
    }
    if (enabled && model->edge_count == GD_MAX_EDGES) {
        snprintf(draft->message, sizeof(draft->message), "32768 relationship limit reached; no change made"); return -1;
    }
    if (!history_available(draft)) return -1;
    if (enabled) stored = model->records[target].reference;
    else for (i = 0; i < model->edge_count; ++i) {
        const GD_Edge *edge = &model->edges[i];
        if (edge->source == record && edge->field == GD_PREREQUISITES && !strcmp(edge->reference, reference)) { stored = edge->reference; break; }
    }
    assert(stored);
    action = &draft->history[draft->history_count++]; memset(action, 0, sizeof(*action));
    action->record = record; action->prerequisite_edit = 1; action->reference = stored; action->count = count;
    replace_connection(model, record, stored, target, enabled ? 1 : 0);
    changed(draft, record); return 1;
}
int gd_draft_undo(GD_Draft *draft) {
    GD_DraftEdit action;
    if (!draft->history_count) return 0;
    action = draft->history[--draft->history_count]; draft->message[0] = 0;
    if (action.prerequisite_edit) replace_connection(draft->model, action.record, action.reference, target_record(draft->model, action.reference), action.count);
    else { draft->model->records[action.record].has_rank = action.has_rank; draft->model->records[action.record].rank = action.rank; }
    /* Restore raw duplicate declarations and ordering exactly at disk sync point. */
    if (!draft->history_count) {
        draft->model->edge_count = draft->baseline->edge_count;
        memcpy(draft->model->edges, draft->baseline->edges, draft->baseline->edge_count*sizeof(draft->model->edges[0]));
    }
    changed(draft, action.record); return 1;
}
static FILE *create_report(const char *path) {
    int descriptor;
#ifdef _WIN32
    wchar_t wide[GD_MAX_PATH]; HANDLE handle;
    if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path, -1, wide, GD_MAX_PATH)) return NULL;
    handle = CreateFileW(wide, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if (handle == INVALID_HANDLE_VALUE) return NULL;
    if (GetFileType(handle) != FILE_TYPE_DISK) { CloseHandle(handle); return NULL; }
    descriptor = _open_osfhandle((intptr_t)handle, _O_BINARY|_O_WRONLY);
    if (descriptor < 0) { CloseHandle(handle); DeleteFileW(wide); return NULL; }
    { FILE *file = _fdopen(descriptor, "wb"); if (!file) { _close(descriptor); DeleteFileW(wide); } return file; }
#else
    descriptor = open(path, O_CREAT|O_EXCL|O_WRONLY, 0600);
    if (descriptor < 0) return NULL;
    { FILE *file = fdopen(descriptor, "wb"); if (!file) { close(descriptor); unlink(path); } return file; }
#endif
}
static void remove_report(const char *path) {
#ifdef _WIN32
    wchar_t wide[GD_MAX_PATH];
    if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path, -1, wide, GD_MAX_PATH)) DeleteFileW(wide);
#else
    unlink(path);
#endif
}
int gd_draft_finish(GD_Draft *draft, const char *output_path) {
    GD_Model *current;
    FILE *output = stdout;
    int written, status;
    draft->message[0] = 0;
    if (output_path && *output_path) {
        size_t length = strlen(output_path);
        if (length >= GD_MAX_PATH || length < 5 || strcmp(output_path+length-5, ".json")) {
            snprintf(draft->message, sizeof(draft->message), "Report path must end in .json and fit 4095 UTF-8 bytes"); return -1;
        }
    }
    current = gd_create(draft->baseline->allocator);
    if (!current) { snprintf(draft->message, sizeof(draft->message), "Cannot allocate current-disk snapshot; draft retained"); return -1; }
    gd_load(current, draft->baseline->collections, draft->baseline->collection_count);
    if (output_path && *output_path) {
        output = create_report(output_path);
        if (!output) {
            gd_destroy(current); snprintf(draft->message, sizeof(draft->message), "Cannot create report: existing file, invalid path, or access denied. Choose a new path; draft retained."); return -1;
        }
    }
    written = gd_write_draft_result(output, draft, current);
    if (fflush(output)) written = 0;
    if (output != stdout && fclose(output)) written = 0;
    status = current->error_count || current->errors_omitted ? 2 : 0;
    gd_destroy(current);
    if (!written) {
        if (output != stdout) remove_report(output_path);
        else clearerr(stdout);
        snprintf(draft->message, sizeof(draft->message), "Report output failed; draft retained. Choose a new report file path."); return -1;
    }
    return status;
}
