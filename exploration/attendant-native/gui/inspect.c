/* Live document inspection. No filesystem, source-session or ImGui calls. */
#define WIN32_LEAN_AND_MEAN
#include "inspect.h"
#include "../include/base.h"
#include <stdio.h>
#include <string.h>
#define JSONB_API static inline
#include "../vendor/json-build/json-build.h"

int gui_inspection_snapshot(const gui_workspace *workspace, gui_inspection *output) {
    gui_inspection_header *header = &output->header;
    gui_selection selection = gui_selection_current(workspace);
    const gui_tab *tab;
    size_t offset = 0;
    uint32_t point = 0;
    memset(header, 0, sizeof(*header)); output->text[0] = 0;
    header->version = 1; snprintf(header->project, sizeof(header->project), "%s", workspace->root);
    if (workspace->active < 0 || workspace->active >= GUI_TABS || !workspace->tabs[workspace->active].used) return 1;
    tab = &workspace->tabs[workspace->active];
    header->has_document = 1; header->dirty = tab->dirty != 0; header->revision = tab->text_revision;
    snprintf(header->path, sizeof(header->path), "%s", tab->path);
    header->view = selection.view; header->status = selection.status;
    if (selection.status != GUI_SELECTION_EXACT) return 1;
    if (selection.start < 0 || selection.end <= selection.start || (size_t)selection.end > tab->length) return 0;
    while (offset < (size_t)selection.end) {
        size_t width = string_utf8_sequence_size((const u8 *)tab->text + offset, tab->length - offset);
        if (!width || offset + width > (size_t)selection.end || (offset < (size_t)selection.start && offset + width > (size_t)selection.start)) return 0;
        if (offset == (size_t)selection.start) header->start = point;
        offset += width; point++;
    }
    header->end = point; header->length = (uint32_t)(selection.end - selection.start);
    memcpy(output->text, tab->text + selection.start, header->length); output->text[header->length] = 0;
    return 1;
}
int gui_inspection_json(const gui_inspection *snapshot, char *output, size_t capacity) {
    const gui_inspection_header *header = &snapshot->header;
    const char *view = header->view == GUI_SELECTION_SOURCE ? "source" : header->view == GUI_SELECTION_PREVIEW ? "preview" : "none";
    const char *status = header->status == GUI_SELECTION_EXACT ? "exact" : header->status == GUI_SELECTION_UNSUPPORTED ? "unsupported" : "none";
    char revision[32]; jsonb builder;
    jsonb_init(&builder);
#define EMIT(expression) do { if ((expression) < 0) return 0; } while (0)
#define KEY(value) EMIT(jsonb_key(&builder, output, capacity, value, sizeof(value) - 1))
    EMIT(jsonb_object(&builder, output, capacity)); KEY("ok"); EMIT(jsonb_bool(&builder, output, capacity, 1));
    KEY("document");
    if (!header->has_document) EMIT(jsonb_null(&builder, output, capacity));
    else {
        EMIT(jsonb_object(&builder, output, capacity)); KEY("path"); EMIT(jsonb_string(&builder, output, capacity, header->path, strlen(header->path)));
        KEY("dirty"); EMIT(jsonb_bool(&builder, output, capacity, header->dirty));
        snprintf(revision, sizeof(revision), "%llu", (unsigned long long)header->revision);
        KEY("buffer_revision"); EMIT(jsonb_string(&builder, output, capacity, revision, strlen(revision)));
        EMIT(jsonb_object_pop(&builder, output, capacity));
    }
    KEY("selection"); EMIT(jsonb_object(&builder, output, capacity));
    KEY("view"); EMIT(jsonb_string(&builder, output, capacity, view, strlen(view)));
    KEY("status"); EMIT(jsonb_string(&builder, output, capacity, status, strlen(status)));
    KEY("text");
    if (header->status == GUI_SELECTION_EXACT) EMIT(jsonb_string(&builder, output, capacity, snapshot->text, header->length));
    else EMIT(jsonb_null(&builder, output, capacity));
    KEY("range_start");
    if (header->status == GUI_SELECTION_EXACT) EMIT(jsonb_number(&builder, output, capacity, header->start));
    else EMIT(jsonb_null(&builder, output, capacity));
    KEY("range_end");
    if (header->status == GUI_SELECTION_EXACT) EMIT(jsonb_number(&builder, output, capacity, header->end));
    else EMIT(jsonb_null(&builder, output, capacity));
    KEY("range_unit"); EMIT(jsonb_string(&builder, output, capacity, "unicode-codepoint", 17));
    EMIT(jsonb_object_pop(&builder, output, capacity)); EMIT(jsonb_object_pop(&builder, output, capacity));
#undef KEY
#undef EMIT
    return 1;
}
uintptr_t gui_inspection_receive(HWND window, HWND client, const gui_workspace *workspace,
    int busy, gui_inspection *output, const void *data, size_t size) {
    COPYDATASTRUCT copy; DWORD_PTR result = 0; char name[128];
    if (!output || gui_reveal_receive(workspace, NULL, NULL, GUI_REVEAL_QUERY, data, size) != GUI_REVEAL_MATCH ||
        !GetClassNameA(client, name, sizeof(name)) || strcmp(name, GUI_INSPECT_CLIENT_CLASS)) return 0;
    if (busy) return GUI_INSPECT_BUSY;
    if (!gui_inspection_snapshot(workspace, output)) return 0;
    copy.dwData = GUI_INSPECT_RESULT; copy.cbData = (DWORD)(offsetof(gui_inspection, text) + output->header.length + 1); copy.lpData = output;
    /* Caller permits this bounded nested reply. Block further receiver reentry. */
    if (!SendMessageTimeoutA(client, WM_COPYDATA, (WPARAM)window, (LPARAM)&copy,
        SMTO_ABORTIFHUNG | SMTO_BLOCK, 250, &result) || result != GUI_INSPECT_RESULT) return 0;
    return GUI_INSPECT_RESULT;
}
int gui_inspection_accept(gui_inspection *output, const char *project, const void *data, size_t size) {
    gui_inspection_header header;
    const char *text;
    size_t offset = 0;
    uint32_t points = 0;
    if (!data || size < offsetof(gui_inspection, text) + 1 || size > sizeof(gui_inspection)) return 0;
    memcpy(&header, data, sizeof(header));
    if (header.version != 1 || header.has_document > 1 || header.dirty > 1 || header.view > GUI_SELECTION_PREVIEW || header.status > GUI_SELECTION_UNSUPPORTED ||
        header.length > GUI_TEXT || size != offsetof(gui_inspection, text) + header.length + 1 ||
        !memchr(header.project, 0, GUI_PATH) || !memchr(header.path, 0, GUI_PATH) || _stricmp(header.project, project) ||
        !string_view_utf8_valid(string_view_from_cstr(header.path)) ||
        header.start > header.end || header.end > GUI_TEXT) return 0;
    if (header.has_document && !*header.path) return 0;
    if (!header.has_document && (*header.path || header.dirty || header.view || header.status || header.revision)) return 0;
    if (header.status == GUI_SELECTION_EXACT) {
        if (!header.has_document || !header.view || !header.length || header.start == header.end) return 0;
    } else if (header.length || header.start || header.end) return 0;
    text = (const char *)data + offsetof(gui_inspection, text);
    if (text[header.length] || memchr(text, 0, header.length)) return 0;
    while (offset < header.length) {
        size_t width = string_utf8_sequence_size((const u8 *)text + offset, header.length - offset);
        if (!width) return 0;
        offset += width; points++;
    }
    if (header.status == GUI_SELECTION_EXACT && points != header.end - header.start) return 0;
    memcpy(output, data, size); return 1;
}
typedef struct inspection_client {
    HWND server;
    const char *project;
    gui_inspection *snapshot;
    int received;
} inspection_client;
static inspection_client *current_client;
static LRESULT CALLBACK inspect_window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    if (message == WM_COPYDATA && current_client && (HWND)wparam == current_client->server && lparam) {
        const COPYDATASTRUCT *copy = (const COPYDATASTRUCT *)lparam;
        if (copy->dwData != GUI_INSPECT_RESULT || !gui_inspection_accept(current_client->snapshot, current_client->project, copy->lpData, copy->cbData)) return 0;
        current_client->received = 1;
        return GUI_INSPECT_RESULT;
    }
    return DefWindowProcA(window, message, wparam, lparam);
}
int gui_inspect(const char *project) {
    gui_reveal request;
    inspection_client client;
    COPYDATASTRUCT copy;
    WNDCLASSA description;
    HWND window = NULL;
    DWORD started;
    DWORD_PTR reply = 0;
    int status = 1, attempt;
    char *json = NULL;
    const size_t capacity = 6u * GUI_TEXT + 6u * GUI_PATH + 4096u;
    memset(&request, 0, sizeof(request)); request.version = 1;
    memset(&client, 0, sizeof(client));
    if (!gui_full_path(NULL, project, request.project)) return gui_reveal_error(1, "invalid-project", "Cannot resolve bounded UTF-8 project path");
    client.server = gui_find_workspace(request.project, &status); if (!client.server) return status;
    client.project = request.project; client.snapshot = VirtualAlloc(NULL, sizeof(*client.snapshot), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!client.snapshot) return gui_reveal_error(1, "capacity", "Cannot allocate bounded inspection response");
    memset(&description, 0, sizeof(description)); description.lpfnWndProc = inspect_window_proc;
    description.hInstance = GetModuleHandleA(NULL); description.lpszClassName = GUI_INSPECT_CLIENT_CLASS;
    if (!RegisterClassA(&description)) goto unavailable;
    current_client = &client;
    window = CreateWindowExA(0, description.lpszClassName, "", 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, description.hInstance, NULL);
    if (!window) goto unavailable;
    copy.dwData = GUI_INSPECT_REQUEST; copy.cbData = sizeof(request); copy.lpData = &request;
    started = GetTickCount();
    for (attempt = 0; attempt < 16 && GetTickCount() - started < 2000; attempt++) {
        /* No SMTO_BLOCK here: source replies through our message-only window. */
        if (!SendMessageTimeoutA(client.server, WM_COPYDATA, (WPARAM)window, (LPARAM)&copy, SMTO_ABORTIFHUNG, 1000, &reply)) break;
        if (reply == GUI_INSPECT_RESULT && client.received) break;
        if (reply != GUI_INSPECT_BUSY) break;
        Sleep(10);
    }
    if (!client.received) goto unavailable;
    json = VirtualAlloc(NULL, capacity, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!json || !gui_inspection_json(client.snapshot, json, capacity)) {
        status = gui_reveal_error(1, "capacity", "Cannot encode bounded inspection result"); goto cleanup;
    }
    puts(json); status = 0; goto cleanup;
unavailable:
    status = gui_reveal_error(1, "inspection-unavailable", "Viewer did not provide stable inspection; retry with a responsive inspection-capable build at the same desktop/integrity level");
cleanup:
    if (window) DestroyWindow(window);
    current_client = NULL;
    if (json) VirtualFree(json, 0, MEM_RELEASE);
    VirtualFree(client.snapshot, 0, MEM_RELEASE); return status;
}
