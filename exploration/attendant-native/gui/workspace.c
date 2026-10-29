/* UI-independent document model. Explicit user actions own save/discard/send. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "workspace.h"
#include "../include/base.h"
#include <stdio.h>
#include <string.h>
#ifdef ATTENDANT_MODEL_TESTING
void gui_model_after_read(const char *path);
#endif

static int problem(gui_workspace *workspace, const char *message) {
    snprintf(workspace->error, sizeof(workspace->error), "%s", message ? message : "Operation failed"); return 0;
}
static int result_problem(gui_workspace *workspace, attendant_result result) {
    int i;
    snprintf(workspace->error, sizeof(workspace->error), "%s: %s%s%s", result.code ? result.code : "source", result.message ? result.message : "Operation failed", result.path ? "\nFailed path: " : "", result.path ? result.path : "");
    snprintf(workspace->failed_path, sizeof(workspace->failed_path), "%s", result.path ? result.path : "");
    workspace->surviving_count = 0; workspace->surviving_total = result.written_count;
    for (i = 0; i < result.written_count && i < GUI_RECOVERY_PATHS; i++) {
        snprintf(workspace->surviving_paths[i], GUI_PATH, "%s", result.written[i]); workspace->surviving_count++;
    }
    fprintf(stderr, "%s\n", workspace->error);
    if (result.path) fprintf(stderr, "Failed path: %s\n", result.path);
    for (i = 0; i < result.written_count; i++) fprintf(stderr, "Written: %s\n", result.written[i]);
    return 0;
}
static int canonical_path(gui_workspace *workspace, const char *path, char *output) {
    wchar_t input[4096], absolute[4096];
    char joined[GUI_PATH];
    const char *source = path;
    DWORD size;
    if (!path || !*path) return problem(workspace, "Expected document path");
    if (*workspace->root && !(path[0] == '/' || path[0] == '\\' || (strlen(path) > 2 && path[1] == ':'))) {
        int length = snprintf(joined, sizeof(joined), "%s\\%s", workspace->root, path);
        if (length < 0 || length >= (int)sizeof(joined)) return problem(workspace, "Document path too long");
        source = joined;
    }
    if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, source, -1, input, 4096)) return problem(workspace, "Invalid UTF-8 path");
    size = GetFullPathNameW(input, 4096, absolute, NULL);
    if (!size || size >= 4096 || !WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, absolute, -1, output, GUI_PATH, NULL, NULL)) return problem(workspace, "Cannot resolve document path");
    return 1;
}
static char *navigation_text(arena *memory, const char *text) {
    arena_result space;
    size_t length;
    if (!text) return NULL;
    length = strlen(text); space = arena_alloc(memory, length + 1, 1);
    if (space.error) return NULL;
    memcpy(space.ptr, text, length + 1); return space.ptr;
}
int gui_refresh(gui_workspace *workspace) {
    attendant_workspace source;
    attendant_result result = attendant_read_workspace(workspace->session, workspace->root, &source);
    gui_navigation *next;
    arena *memory;
    int i, j;
    if (!result.ok) return result_problem(workspace, result);
    next = VirtualAlloc(NULL, sizeof(*next) + 16 * 1024 * 1024, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!next) return problem(workspace, "Cannot allocate workspace navigation");
    memory = arena_from_buffer(next + 1, 16 * 1024 * 1024);
    next->memory = memory; next->count = source.document_count; next->diagnostics = source.diagnostic_count;
    for (i = 0; i < source.document_count; i++) {
        attendant_document_info *document = &next->documents[i], *original = &source.documents[i];
        arena_result references;
        *document = *original;
        document->path = navigation_text(memory, original->path); document->name = navigation_text(memory, original->name);
        document->collection = navigation_text(memory, original->collection);
        references = arena_alloc(memory, original->reference_count ? (size_t)original->reference_count * sizeof(attendant_reference) : 1, 8);
        if (!document->path || !document->name || !document->collection || references.error) goto capacity;
        document->references = references.ptr;
        for (j = 0; j < original->reference_count; j++) {
            attendant_reference *reference = &document->references[j], *old = &original->references[j];
            reference->field = navigation_text(memory, old->field); reference->value = navigation_text(memory, old->value);
            reference->target_path = navigation_text(memory, old->target_path);
            if (!reference->field || !reference->value || (old->target_path && !reference->target_path)) goto capacity;
        }
    }
    if (workspace->navigation) VirtualFree(workspace->navigation, 0, MEM_RELEASE);
    workspace->navigation = next; return 1;
capacity:
    VirtualFree(next, 0, MEM_RELEASE); return problem(workspace, "Workspace navigation exceeds 16 MiB capacity; previous view retained");
}
int gui_workspace_init(gui_workspace *workspace, const char *root) {
    memset(workspace, 0, sizeof(*workspace)); workspace->active = -1;
    workspace->session = attendant_open();
    if (!workspace->session) return problem(workspace, "Cannot allocate source session");
    if (strlen(root) >= sizeof(workspace->root)) return problem(workspace, "Workspace path too long");
    snprintf(workspace->root, sizeof(workspace->root), "%s", root);
    {
        char resolved[GUI_PATH];
        /* Resolve project from process working directory, not relative to itself. */
        workspace->root[0] = 0;
        if (!canonical_path(workspace, root, resolved)) return 0;
        snprintf(workspace->root, sizeof(workspace->root), "%s", resolved);
    }
    return gui_refresh(workspace);
}
void gui_workspace_destroy(gui_workspace *workspace) {
    int i;
    for (i = 0; i < GUI_TABS; i++) if (workspace->tabs[i].text) VirtualFree(workspace->tabs[i].text, 0, MEM_RELEASE);
    if (workspace->navigation) VirtualFree(workspace->navigation, 0, MEM_RELEASE);
    attendant_close(workspace->session); memset(workspace, 0, sizeof(*workspace));
}
static void document_metadata(gui_tab *tab, attendant_result result) {
    tab->modified.dwLowDateTime = (DWORD)result.write_time;
    tab->modified.dwHighDateTime = (DWORD)(result.write_time >> 32); tab->file_size = (DWORD)result.file_size;
}
int gui_open_document(gui_workspace *workspace, const char *path) {
    char absolute[GUI_PATH];
    attendant_result result;
    gui_tab *tab;
    int i, slot = -1;
    if (!canonical_path(workspace, path, absolute)) return 0;
    for (i = 0; i < GUI_TABS; i++) {
        if (workspace->tabs[i].used && !_stricmp(absolute, workspace->tabs[i].path)) { workspace->active = i; workspace->activation++; return 1; }
        if (!workspace->tabs[i].used && slot < 0) slot = i;
    }
    if (slot < 0) return problem(workspace, "16 document tabs already open; close one first");
    result = attendant_read_document(workspace->session, absolute);
    if (!result.ok) return result_problem(workspace, result);
#ifdef ATTENDANT_MODEL_TESTING
    gui_model_after_read(result.path);
#endif
    tab = &workspace->tabs[slot]; memset(tab, 0, sizeof(*tab));
    tab->text = VirtualAlloc(NULL, GUI_TEXT + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!tab->text) return problem(workspace, "Cannot allocate document text");
    snprintf(tab->path, sizeof(tab->path), "%s", result.path); snprintf(tab->hash, sizeof(tab->hash), "%s", result.hash);
    tab->length = strlen(result.text); memcpy(tab->text, result.text, tab->length + 1);
    tab->used = 1; tab->text_revision = ++workspace->text_revision; document_metadata(tab, result); workspace->active = slot; workspace->tab_count++; workspace->activation++; return 1;
}
int gui_save_document(gui_workspace *workspace, int index) {
    gui_tab *tab = &workspace->tabs[index];
    attendant_result result;
    result = attendant_save_document(workspace->session, tab->path, tab->hash, tab->text);
    if (!result.ok) return result_problem(workspace, result);
    snprintf(tab->hash, sizeof(tab->hash), "%s", result.hash); tab->dirty = tab->external_change = 0;
    document_metadata(tab, result); snprintf(workspace->notice, sizeof(workspace->notice), "Saved %s", tab->path); return 1;
}
int gui_reload_document(gui_workspace *workspace, int index, int discard_dirty) {
    gui_tab *tab = &workspace->tabs[index];
    attendant_result result;
    if (tab->dirty && !discard_dirty) return problem(workspace, "Dirty document requires explicit discard or save");
    result = attendant_read_document(workspace->session, tab->path);
    if (!result.ok) return result_problem(workspace, result);
#ifdef ATTENDANT_MODEL_TESTING
    gui_model_after_read(result.path);
#endif
    tab->length = strlen(result.text); memcpy(tab->text, result.text, tab->length + 1);
    snprintf(tab->hash, sizeof(tab->hash), "%s", result.hash); tab->dirty = tab->external_change = 0;
    tab->selection_start = tab->selection_end = tab->reveal_selection = tab->anchor_stale = 0;
    tab->generation++; tab->text_revision = ++workspace->text_revision; document_metadata(tab, result); return 1;
}
int gui_close_document(gui_workspace *workspace, int index, int discard_dirty) {
    gui_tab *tab = &workspace->tabs[index]; int i;
    if (!tab->used) return 1;
    if (tab->dirty && !discard_dirty) return problem(workspace, "Dirty document requires Save, Discard or Cancel");
    VirtualFree(tab->text, 0, MEM_RELEASE); memset(tab, 0, sizeof(*tab)); workspace->tab_count--;
    if (workspace->active == index) {
        workspace->active = -1;
        for (i = 0; i < GUI_TABS; i++) if (workspace->tabs[i].used) { workspace->active = i; break; }
        workspace->activation++;
    }
    return 1;
}
void gui_check_external(gui_workspace *workspace, int index) {
    gui_tab *tab = &workspace->tabs[index];
    wchar_t path[4096]; WIN32_FILE_ATTRIBUTE_DATA data;
    if (!tab->used || !MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, tab->path, -1, path, 4096)) return;
    if (!GetFileAttributesExW(path, GetFileExInfoStandard, &data)) { tab->external_change = 1; return; }
    if (CompareFileTime(&data.ftLastWriteTime, &tab->modified) || data.nFileSizeLow != tab->file_size) {
        tab->external_change = 1;
        if (!tab->dirty) gui_reload_document(workspace, index, 0);
    }
}
int gui_set_document_text(gui_workspace *workspace, int index, const char *text) {
    gui_tab *tab = &workspace->tabs[index]; size_t length = strlen(text);
    if (length > GUI_TEXT || !string_view_utf8_valid(string_view_from_cstr(text))) return problem(workspace, "Document must be valid UTF-8 up to 4 MiB");
    memcpy(tab->text, text, length + 1); tab->length = length; tab->dirty = 1; tab->generation++; tab->text_revision = ++workspace->text_revision; return 1;
}
int gui_send_comment(gui_workspace *workspace, int index, int start, int end, const char *comment) {
    gui_tab *tab = &workspace->tabs[index];
    attendant_feedback input = {0}; attendant_result result;
    size_t offset = 0; uint64_t point = 0;
    char *selected;
    if (tab->dirty) return problem(workspace, "Save & Send or Cancel required for dirty source");
    if (start > end) { int swap = start; start = end; end = swap; }
    if (start < 0 || end <= start || (size_t)end > tab->length) return problem(workspace, "Select nonempty source text first");
    selected = VirtualAlloc(NULL, end - start + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!selected) return problem(workspace, "Cannot allocate selected text");
    memcpy(selected, tab->text + start, end - start);
    input.project = workspace->root; input.target_path = tab->path; input.target_hash = tab->hash;
    input.selected_text = selected; input.comment = comment;
    while (offset < (size_t)end) {
        size_t width = string_utf8_sequence_size((const u8 *)tab->text + offset, tab->length - offset);
        if (!width || offset + width > (size_t)end) { VirtualFree(selected, 0, MEM_RELEASE); return problem(workspace, "Selection must use UTF-8 boundaries"); }
        if (offset == (size_t)start) input.range_start = point;
        else if (offset < (size_t)start && offset + width > (size_t)start) { VirtualFree(selected, 0, MEM_RELEASE); return problem(workspace, "Selection must use UTF-8 boundaries"); }
        offset += width; point++;
    }
    input.range_end = point; result = attendant_send_feedback(workspace->session, &input); VirtualFree(selected, 0, MEM_RELEASE);
    if (!result.ok) return result_problem(workspace, result);
    workspace->error[0] = workspace->failed_path[0] = 0; workspace->surviving_count = workspace->surviving_total = 0;
    snprintf(workspace->notice, sizeof(workspace->notice), "Comment queued: %s", result.path);
    gui_refresh(workspace); return 1;
}
int gui_open_comment_target(gui_workspace *workspace, int comment_index) {
    attendant_comment comment;
    attendant_result result = attendant_read_comment(workspace->session, workspace->root, workspace->tabs[comment_index].path, &comment);
    char target[GUI_PATH], hash[65], status[16], *selected;
    uint64_t range_start, range_end, point = 0;
    size_t offset = 0, begin = 0, length;
    gui_tab *tab;
    int ok = 0;
    if (!result.ok) return result_problem(workspace, result);
    snprintf(target, sizeof(target), "%s", comment.target_path); snprintf(hash, sizeof(hash), "%s", comment.target_hash);
    snprintf(status, sizeof(status), "%s", comment.status); range_start = comment.range_start; range_end = comment.range_end;
    length = strlen(comment.selected_text); selected = VirtualAlloc(NULL, length + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!selected) return problem(workspace, "Cannot allocate feedback anchor");
    memcpy(selected, comment.selected_text, length + 1);
    if (!gui_open_document(workspace, target)) goto finished;
    tab = &workspace->tabs[workspace->active];
    if (tab->dirty) { problem(workspace, "Target has unsaved edits; comment range was not applied"); goto finished; }
    if (!gui_reload_document(workspace, workspace->active, 0)) goto finished;
    if (strcmp(tab->hash, hash)) { tab->anchor_stale = 1; problem(workspace, "Comment anchor is stale: saved target hash differs; no selection applied"); goto finished; }
    while (offset < tab->length && point < range_end) {
        size_t width = string_utf8_sequence_size((const u8 *)tab->text + offset, tab->length - offset);
        if (point == range_start) begin = offset;
        if (!width) break;
        offset += width; point++;
    }
    if (point != range_end || offset - begin != length || memcmp(tab->text + begin, selected, length)) {
        tab->anchor_stale = 1; problem(workspace, "Comment text/range does not match saved target; no selection applied"); goto finished;
    }
    tab->selection_start = (int)begin; tab->selection_end = (int)offset; tab->reveal_selection = 1;
    snprintf(workspace->notice, sizeof(workspace->notice), "Opened %s comment target. Reading does not resolve feedback.", status); ok = 1;
finished:
    VirtualFree(selected, 0, MEM_RELEASE); return ok;
}
int gui_open_link(gui_workspace *workspace, const char *url) {
    int i;
    if (!strncmp(url, "attendant://", 12)) {
        const char *reference = url + 12;
        for (i = 0; workspace->navigation && i < workspace->navigation->count; i++) {
            attendant_document_info *document = &workspace->navigation->documents[i];
            size_t collection_length = strlen(document->collection);
            if (!strncmp(reference, document->collection, collection_length) && reference[collection_length] == '/' && !strcmp(reference + collection_length + 1, document->name))
                return gui_open_document(workspace, document->path);
        }
        return problem(workspace, "Reference target unresolved");
    }
    if (strstr(url, "://") || url[0] == '#') return problem(workspace, "External URL or unresolved anchor not opened");
    if (workspace->active >= 0) {
        char joined[GUI_PATH]; gui_tab *tab = &workspace->tabs[workspace->active];
        const char *end = strrchr(tab->path, '\\');
        int length = snprintf(joined, sizeof(joined), "%.*s\\%s", end ? (int)(end - tab->path) : 0, tab->path, url);
        if (length < 0 || length >= (int)sizeof(joined)) return problem(workspace, "Link path too long");
        return gui_open_document(workspace, joined);
    }
    return problem(workspace, "No active document for relative link");
}
size_t gui_body_offset(const char *text) {
    const char *line = text + (!strncmp(text, "\xef\xbb\xbf", 3) ? 3 : 0);
    if (strncmp(line, "---", 3) || (line[3] != '\r' && line[3] != '\n')) return 0;
    line += 3; if (*line == '\r') line++; if (*line == '\n') line++;
    while (*line) {
        const char *end = line + strcspn(line, "\r\n"), *next = end;
        if (*next == '\r') next++; if (*next == '\n') next++;
        if (end - line == 3 && !memcmp(line, "---", 3)) return (size_t)(next - text);
        line = next;
    }
    return 0;
}
