/* Test-only document-state acceptance. Requires generated ownership marker.
   CHECK always runs, including toolchains which define NDEBUG at -O3. */
#define WIN32_LEAN_AND_MEAN
#if defined(_WIN32)
#include <windows.h>
#endif
#include "../../src/core/platform.h"
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include "../../apps/gui/workspace.h"
#include "../../apps/gui/reveal.h"
#include "../../apps/gui/inspect.h"
#if defined(_WIN32)
__declspec(dllimport) LPWSTR *WINAPI CommandLineToArgvW(LPCWSTR, int *);
#endif
static int checks, edit_after_read;
void gui_model_after_read(const char *path) {
    const char *suffix = "\nExternal read/stat gap edit.\n";
    if (!edit_after_read) return;
    edit_after_read = 0;
#if defined(_WIN32)
    wchar_t wide[4096]; HANDLE file; DWORD count;
    if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path, -1, wide, 4096)) ExitProcess(2);
    file = CreateFileW(wide, FILE_APPEND_DATA, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) ExitProcess(2);
    if (!WriteFile(file, suffix, (DWORD)strlen(suffix), &count, NULL) || count != strlen(suffix)) { CloseHandle(file); ExitProcess(2); }
    CloseHandle(file);
#else
    int descriptor = open(path, O_WRONLY | O_APPEND | O_NOFOLLOW);
    if (descriptor < 0) exit(2);
    void *file = (void *)(uptr)(descriptor + 1);
    int ok = file_stream_transfer(file, (void *)suffix, strlen(suffix), 1);
    if (!file_stream_close(file)) ok = 0;
    if (!ok) exit(2);
#endif
}
#define CHECK(expression) do { checks++; if (!(expression)) { fprintf(stderr, "Model check failed at line %d: %s\n", __LINE__, #expression); return 1; } } while (0)
static int test_environment(const char *name, const char *value) {
#if defined(_WIN32)
    return SetEnvironmentVariableA(name, value) != 0;
#else
    return (value ? setenv(name, value, 1) : unsetenv(name)) == 0;
#endif
}
#if defined(__APPLE__)
#include "ipc-model-macos.h"
#endif
int main(int argc, char **argv) {
    int first, second, comment_index, i, references = 0;
#if defined(_WIN32)
    LPWSTR *arguments = CommandLineToArgvW(GetCommandLineW(), &argc);
#endif
    char project[GUI_PATH], marker[GUI_PATH], path[GUI_PATH], comment_path[GUI_PATH] = {0}, expected_hash[65], *edited, *external;
    gui_workspace *workspace;
    gui_tab *tab;
    attendant_result result;
    const char *selected = "caf\xc3\xa9 \xce\xa9 \xf0\x9f\x98\x80\r\n";
#if defined(_WIN32)
    CHECK(arguments && argc == 2);
    CHECK(WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, arguments[1], -1, project, GUI_PATH, NULL, NULL)); LocalFree(arguments);
#else
    CHECK(argc == 2 && gui_full_path(NULL, argv[1], project));
#endif
    snprintf(marker, sizeof(marker), "%s/.gui-test-owned", project);
    CHECK(source_stat(marker).kind == SOURCE_REGULAR);
    workspace = native_memory_allocate(sizeof(*workspace));
    edited = native_memory_allocate(GUI_TEXT + 1);
    external = native_memory_allocate(GUI_TEXT + 1);
    CHECK(workspace && edited && external && gui_workspace_init(workspace, project));
    {
        gui_inspection *empty = native_memory_allocate(sizeof(*empty));
        char json[1024]; CHECK(empty);
        CHECK(gui_inspection_snapshot(workspace, empty) && !empty->header.has_document && !empty->header.length);
        CHECK(gui_inspection_json(empty, json, sizeof(json)) && strstr(json, "\"document\":null"));
        arena_memory_release(empty, sizeof(*empty));
    }
    {
        gui_search request = {0}, pending = {0}; int ready = 0;
        request.version = 1; memcpy(request.project, workspace->root, strlen(workspace->root) + 1); strcpy(request.query, "plan");
        CHECK(gui_search_receive(workspace, &pending, &ready, 0, &request, sizeof(request)) == GUI_SEARCH_ACCEPTED && ready && !strcmp(pending.query, "plan"));
        CHECK(gui_search_receive(workspace, &pending, &ready, 0, &request, sizeof(request)) == GUI_SEARCH_BUSY);
        ready = 0; CHECK(gui_search_receive(workspace, &pending, &ready, 1, &request, sizeof(request)) == GUI_SEARCH_BUSY && !ready);
        request.view = 1; CHECK(!gui_search_receive(workspace, &pending, &ready, 0, &request, sizeof(request)));
        request.view = 3; CHECK(!gui_search_receive(workspace, &pending, &ready, 0, &request, sizeof(request)));
        request.view = 0; request.match_case = 2; CHECK(!gui_search_receive(workspace, &pending, &ready, 0, &request, sizeof(request)));
        request.match_case = 0; request.version = 2; CHECK(!gui_search_receive(workspace, &pending, &ready, 0, &request, sizeof(request)));
        request.version = 1; strcpy(request.query, "\xc0\xaf"); CHECK(!gui_search_receive(workspace, &pending, &ready, 0, &request, sizeof(request)));
        strcpy(request.query, "two\nlines"); CHECK(!gui_search_receive(workspace, &pending, &ready, 0, &request, sizeof(request)));
        memset(request.query, 'x', sizeof(request.query)); request.query[sizeof(request.query) - 1] = 0;
        CHECK(gui_search_receive(workspace, &pending, &ready, 0, &request, sizeof(request)) == GUI_SEARCH_ACCEPTED);
        ready = 0; request.query[sizeof(request.query) - 1] = 'x'; CHECK(!gui_search_receive(workspace, &pending, &ready, 0, &request, sizeof(request)));
        strcpy(request.query, "plan"); request.project[0] = '!'; CHECK(!gui_search_receive(workspace, &pending, &ready, 0, &request, sizeof(request)));
        memcpy(request.project, workspace->root, strlen(workspace->root) + 1); request.view = 1;
        CHECK(gui_full_path(workspace->root, "../outside.md", request.path));
        CHECK(!gui_search_receive(workspace, &pending, &ready, 0, &request, sizeof(request)));
        CHECK(gui_full_path(workspace->root, "records/tasks/first.md", request.path));
        memcpy(external + 1, &request, sizeof(request));
        CHECK(gui_search_receive(workspace, &pending, &ready, 0, external + 1, sizeof(request)) == GUI_SEARCH_ACCEPTED && ready);
        ready = 0; CHECK(!gui_search_receive(workspace, &pending, &ready, 0, external + 1, sizeof(request) - 1));
        request.view = 0; CHECK(!gui_search_receive(workspace, &pending, &ready, 0, &request, sizeof(request)));
        CHECK(!workspace->tab_count && workspace->active < 0);
    }
    CHECK(gui_open_document(workspace, "records/tasks/first.md")); first = workspace->active; tab = &workspace->tabs[first];
    for (i = 0; i < workspace->navigation->count; i++) if (!strcmp(workspace->navigation->documents[i].path, tab->path)) {
        int j; attendant_document_info *document = &workspace->navigation->documents[i];
        for (j = 0; j < document->reference_count; j++) {
            attendant_reference *reference = &document->references[j];
            if (!strcmp(reference->value, "second")) { CHECK(reference->target_path); references++; }
            if (!strcmp(reference->value, "missing")) { CHECK(!reference->target_path); references++; }
        }
    }
    CHECK(references == 2); CHECK(tab->text_revision > 0);
    CHECK(gui_open_link(workspace, "second.md")); second = workspace->active; CHECK(second != first && workspace->tab_count == 2);
    CHECK(gui_open_link(workspace, "attendant://tasks/first")); CHECK(workspace->active == first && workspace->tab_count == 2);
    snprintf(edited, GUI_TEXT + 1, "%s\r\nModel selection: %s", tab->text, selected);
    CHECK(gui_set_document_text(workspace, first, edited));
    tab->selection_start = (int)(strstr(tab->text, "Model selection: ") - tab->text) + 17;
    tab->selection_end = tab->selection_start + (int)strlen(selected);
    {
        gui_inspection *snapshot = native_memory_allocate(sizeof(*snapshot));
        char json[8192]; int start = tab->selection_start, end = tab->selection_end;
        CHECK(snapshot);
        CHECK(gui_inspection_snapshot(workspace, snapshot) && snapshot->header.has_document && snapshot->header.status == GUI_SELECTION_NONE);
        gui_selection_capture(workspace, first, GUI_SELECTION_SOURCE, GUI_SELECTION_EXACT, end, start);
        CHECK(gui_selection_current(workspace).start == start && gui_selection_current(workspace).end == end);
        CHECK(gui_inspection_snapshot(workspace, snapshot));
        CHECK(snapshot->header.dirty && snapshot->header.view == GUI_SELECTION_SOURCE && !strcmp(snapshot->text, selected));
        CHECK(snapshot->header.end - snapshot->header.start == 10 && snapshot->header.start < (uint32_t)start);
        CHECK(gui_inspection_json(snapshot, json, sizeof(json)) && strstr(json, "\"dirty\":true") && strstr(json, "unicode-codepoint"));
        CHECK(!gui_inspection_json(snapshot, json, 10));
#if defined(_WIN32)
        {
            WNDCLASSA description = {0}; gui_reveal request = {0}; HWND client;
            description.lpfnWndProc = DefWindowProcA; description.hInstance = GetModuleHandleA(NULL); description.lpszClassName = GUI_INSPECT_CLIENT_CLASS;
            CHECK(RegisterClassA(&description));
            client = CreateWindowExA(0, description.lpszClassName, "", 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, description.hInstance, NULL);
            CHECK(client != NULL); request.version = 1; snprintf(request.project, sizeof(request.project), "%s", workspace->root);
            CHECK(gui_inspection_receive(NULL, client, workspace, 1, snapshot, &request, sizeof(request)) == GUI_INSPECT_BUSY);
            CHECK(!gui_inspection_receive(NULL, client, workspace, 1, snapshot, &request, sizeof(request) - 1));
            CHECK(!gui_inspection_receive(NULL, NULL, workspace, 1, snapshot, &request, sizeof(request)));
            DestroyWindow(client);
        }
#endif
        {
            size_t bytes = offsetof(gui_inspection, text) + snapshot->header.length + snapshot->header.context_length + 2;
            gui_inspection_header *packet = (gui_inspection_header *)external;
            memcpy(external, snapshot, bytes); CHECK(gui_inspection_accept(snapshot, workspace->root, external, bytes));
            memcpy(external + 1, snapshot, bytes); CHECK(gui_inspection_accept(snapshot, workspace->root, external + 1, bytes));
            CHECK(!gui_inspection_accept(snapshot, workspace->root, external + 1, bytes - 1));
            memcpy(external, snapshot, bytes); packet->version = 3; CHECK(!gui_inspection_accept(snapshot, workspace->root, external, bytes));
            memcpy(external, snapshot, bytes); packet->end++; CHECK(!gui_inspection_accept(snapshot, workspace->root, external, bytes));
            memcpy(external, snapshot, bytes); packet->project[0] = '!'; CHECK(!gui_inspection_accept(snapshot, workspace->root, external, bytes));
            memcpy(external, snapshot, bytes); packet->view = 99; CHECK(!gui_inspection_accept(snapshot, workspace->root, external, bytes));
            memcpy(external, snapshot, bytes); external[offsetof(gui_inspection, text)] = (char)0xff; CHECK(!gui_inspection_accept(snapshot, workspace->root, external, bytes));
        }
        gui_selection_capture(workspace, first, GUI_SELECTION_PREVIEW, GUI_SELECTION_UNSUPPORTED, 0, 0);
        CHECK(gui_inspection_snapshot(workspace, snapshot) && snapshot->header.status == GUI_SELECTION_UNSUPPORTED && !snapshot->header.length && !*snapshot->text);
        CHECK(gui_inspection_json(snapshot, json, sizeof(json)) && strstr(json, "\"text\":null"));
        gui_selection_capture(workspace, first, GUI_SELECTION_SOURCE, GUI_SELECTION_EXACT, start + 4, end);
        CHECK(gui_selection_current(workspace).status == GUI_SELECTION_UNSUPPORTED);
        gui_selection_capture(workspace, first, GUI_SELECTION_SOURCE, GUI_SELECTION_EXACT, start, start);
        CHECK(gui_selection_current(workspace).status == GUI_SELECTION_NONE);
        gui_selection_capture(workspace, first, GUI_SELECTION_SOURCE, GUI_SELECTION_EXACT, start, end);
        CHECK(gui_open_document(workspace, workspace->tabs[second].path));
        CHECK(gui_selection_current(workspace).status == GUI_SELECTION_NONE);
        CHECK(gui_open_document(workspace, tab->path)); CHECK(gui_selection_current(workspace).status == GUI_SELECTION_NONE);
        gui_selection_capture(workspace, first, GUI_SELECTION_PREVIEW, GUI_SELECTION_EXACT, start, end);
        CHECK(gui_set_document_text(workspace, first, edited)); CHECK(gui_selection_current(workspace).status == GUI_SELECTION_NONE);
        arena_memory_release(snapshot, sizeof(*snapshot));
    }
    CHECK(!gui_close_document(workspace, first, 0));
    CHECK(!gui_send_comment(workspace, first, tab->selection_start, tab->selection_end, "Must not send dirty text."));
    CHECK(gui_refresh(workspace)); CHECK(workspace->active == first && tab->dirty && !strcmp(tab->text, edited));
    CHECK(gui_open_document(workspace, tab->path)); CHECK(workspace->tab_count == 2 && tab->dirty);
    { uint64_t revision = tab->text_revision; CHECK(gui_save_document(workspace, first)); CHECK(tab->text_revision == revision); }
    CHECK(!tab->dirty);
    result = attendant_read_document(workspace->session, tab->path); CHECK(result.ok && !strcmp(result.text, edited));
    CHECK(!gui_send_comment(workspace, first, -1, tab->selection_end, "Invalid range."));
    CHECK(!gui_send_comment(workspace, first, tab->selection_start + 4, tab->selection_end, "Inside UTF-8 sequence."));
    CHECK(!gui_send_comment(workspace, first, tab->selection_start, tab->selection_start + 11, "End inside supplementary sequence."));
    CHECK(test_environment("ATTENDANT_FEEDBACK_FAULT", "published-edit"));
    CHECK(!gui_send_comment(workspace, first, tab->selection_start, tab->selection_end, "Failed bootstrap must retain surviving paths."));
    CHECK(test_environment("ATTENDANT_FEEDBACK_FAULT", NULL));
#if defined(_WIN32)
    CHECK(workspace->surviving_count == 1 && workspace->surviving_total == 1);
#else
    CHECK(workspace->surviving_count == 4 && workspace->surviving_total == 4);
#endif
    snprintf(path, sizeof(path), "%s", workspace->surviving_paths[0]);
    CHECK(gui_refresh(workspace)); CHECK(!strcmp(path, workspace->surviving_paths[0]));
    result = attendant_read_document(workspace->session, path);
    CHECK(result.ok && strstr(result.text, "# Concurrent external edit"));
    for (i = 0; i < workspace->surviving_count; i++) CHECK(source_remove(workspace->surviving_paths[i], 0));
    { char *name = (char *)source_basename(path); CHECK(name > path); name[-1] = 0; CHECK(source_remove(path, 1)); }
    CHECK(gui_send_comment(workspace, first, tab->selection_end, tab->selection_start, "Review this Unicode selection.\r\nKeep body exact.\n"));
    CHECK(workspace->active == first && !tab->dirty);
    for (i = 0; i < workspace->navigation->count; i++) if (workspace->navigation->documents[i].pending) snprintf(comment_path, sizeof(comment_path), "%s", workspace->navigation->documents[i].path);
    CHECK(*comment_path);
    // Preserve absolute alternate-directory spelling while comparing canonical paths.
    snprintf(path, sizeof(path), "%s/.pi/attendant.tables", project);
    result = attendant_read_document(workspace->session, path); CHECK(result.ok);
    snprintf(expected_hash, sizeof(expected_hash), "%s", result.hash);
    {
        const char *registration = strstr(result.text, "records/_meta_comments as _meta_comments"); CHECK(registration);
        snprintf(external, GUI_TEXT + 1, "%.*s%s/records/_meta_comments/./ as _meta_comments\n", (int)(registration - result.text), result.text, project);
    }
    CHECK(attendant_save_document(workspace->session, path, expected_hash, external).ok);
    CHECK(gui_open_document(workspace, comment_path)); comment_index = workspace->active;
    CHECK(gui_open_comment_target(workspace, comment_index)); CHECK(workspace->active == first && tab->reveal_selection);
    CHECK(tab->selection_end - tab->selection_start == (int)strlen(selected) && !memcmp(tab->text + tab->selection_start, selected, strlen(selected)));
    { attendant_comment comment; result = attendant_read_comment(workspace->session, project, comment_path, &comment); CHECK(result.ok && !strcmp(comment.status, "pending")); }
    result = attendant_read_document(workspace->session, path); CHECK(result.ok && !strcmp(result.text, external));
    // Clean external edit reloads; dirty external edit is never overwritten.
    snprintf(external, GUI_TEXT + 1, "%s\nExternal clean edit.\n", edited);
    CHECK(attendant_save_document(workspace->session, tab->path, tab->hash, external).ok);
    gui_check_external(workspace, first); CHECK(!strcmp(tab->text, external) && !tab->dirty);
    CHECK(!gui_open_comment_target(workspace, comment_index)); CHECK(tab->anchor_stale && !tab->dirty && tab->selection_start == tab->selection_end);
    CHECK(gui_set_document_text(workspace, first, edited));
    CHECK(!gui_open_comment_target(workspace, comment_index)); CHECK(tab->dirty && !strcmp(tab->text, edited));
    snprintf(external, GUI_TEXT + 1, "%s\nExternal dirty conflict.\n", tab->text);
    CHECK(attendant_save_document(workspace->session, tab->path, tab->hash, external).ok);
    gui_check_external(workspace, first); CHECK(tab->external_change && tab->dirty && !strcmp(tab->text, edited));
    CHECK(!gui_save_document(workspace, first)); CHECK(!strcmp(workspace->failed_path, tab->path));
    CHECK(gui_refresh(workspace)); CHECK(!strcmp(workspace->failed_path, tab->path));
    CHECK(!gui_reload_document(workspace, first, 0));
    CHECK(gui_reload_document(workspace, first, 1)); CHECK(!tab->dirty && !strcmp(tab->text, external));
    CHECK(gui_set_document_text(workspace, first, edited)); CHECK(!gui_close_document(workspace, first, 0));
    CHECK(gui_close_document(workspace, first, 1)); CHECK(workspace->tab_count == 2 && workspace->active == second);
    snprintf(path, sizeof(path), "%s/missing.md", project);
    CHECK(!gui_open_document(workspace, path)); CHECK(workspace->tab_count == 2 && workspace->active == second);
    CHECK(!gui_open_document(workspace, "records/tasks/unreadable.md")); CHECK(workspace->active == second);
    CHECK(gui_refresh(workspace)); CHECK(workspace->active == second && workspace->navigation->diagnostics > 0);
    CHECK(gui_open_document(workspace, "README.md"));
    {
        int document = workspace->active; size_t length = workspace->tabs[document].length;
        uint64_t revision = workspace->tabs[document].text_revision;
        edit_after_read = 1; CHECK(gui_reload_document(workspace, document, 0)); CHECK(workspace->tabs[document].length == length);
        CHECK(workspace->tabs[document].text_revision > revision);
        gui_check_external(workspace, document); CHECK(workspace->tabs[document].length > length);
        length = workspace->tabs[document].length; revision = workspace->tabs[document].text_revision; CHECK(gui_close_document(workspace, document, 0));
        edit_after_read = 1; CHECK(gui_open_document(workspace, "README.md")); document = workspace->active;
        CHECK(workspace->tabs[document].length == length); CHECK(workspace->tabs[document].text_revision > revision); gui_check_external(workspace, document); CHECK(workspace->tabs[document].length > length);
    }
    CHECK(gui_body_offset("---\r\nname: a\r\n---\r\nBody") == 19);
    CHECK(gui_body_offset("\xef\xbb\xbf---\r\nname: a\r\n---\r\nBody") == 22);
    CHECK(gui_body_offset("---\rname: a\r---\rBody") == 16);
    CHECK(gui_body_offset("---\n---") == 7);
    CHECK(gui_body_offset("---\nnot closed") == 0);
    CHECK(gui_body_offset("--") == 0);
    CHECK(gui_body_offset("") == 0);
    {
        gui_reveal request = {0}, pending = {0};
        unsigned char packet[sizeof(request) + 1];
        gui_tab *current = &workspace->tabs[workspace->active];
        int ready = 0, active = workspace->active, count = workspace->tab_count, activation = workspace->activation;
        int start = current->selection_start, end = current->selection_end;
        uint64_t revision = current->text_revision;
        request.version = 1; request.line = 1;
        snprintf(request.project, sizeof(request.project), "%s", workspace->root);
        CHECK(gui_reveal_receive(workspace, &pending, &ready, GUI_REVEAL_QUERY, &request, sizeof(request)) == GUI_REVEAL_MATCH);
        CHECK(!ready);
        snprintf(request.path, sizeof(request.path), "%s", current->path); memcpy(request.hash, current->hash, 65);
        CHECK(gui_reveal_receive(workspace, &pending, &ready, GUI_REVEAL_QUERY, &request, sizeof(request)) == GUI_REVEAL_ALLOWED);
        CHECK(!ready);
        CHECK(gui_reveal_receive(workspace, &pending, &ready, GUI_REVEAL_SET, &request, sizeof(request)) == GUI_REVEAL_ACCEPTED);
        CHECK(ready && !memcmp(&request, &pending, sizeof(request)));
        request.line = 2;
        CHECK(gui_reveal_receive(workspace, &pending, &ready, GUI_REVEAL_SET, &request, sizeof(request)) == GUI_REVEAL_ACCEPTED);
        CHECK(pending.line == 2); request.line = 1;
        CHECK(workspace->active == active && workspace->tab_count == count && workspace->activation == activation);
        CHECK(current->selection_start == start && current->selection_end == end && current->text_revision == revision);
        CHECK(gui_reveal_matches(&request, current)); current->dirty = 1; CHECK(!gui_reveal_matches(&request, current)); current->dirty = 0;
        current->external_change = 1; CHECK(!gui_reveal_matches(&request, current)); current->external_change = 0;
        request.hash[0] = request.hash[0] == 'a' ? 'b' : 'a'; CHECK(!gui_reveal_matches(&request, current)); memcpy(request.hash, current->hash, 65);
        ready = 0;
        memcpy(packet + 1, &request, sizeof(request));
        CHECK(gui_reveal_receive(workspace, &pending, &ready, GUI_REVEAL_SET, packet + 1, sizeof(request)) == GUI_REVEAL_ACCEPTED);
        CHECK(ready); ready = 0;
        CHECK(!gui_reveal_receive(workspace, &pending, &ready, GUI_REVEAL_SET, &request, sizeof(request) - 1));
        CHECK(!gui_reveal_receive(workspace, &pending, &ready, 0, &request, sizeof(request)));
        CHECK(!gui_reveal_receive(workspace, &pending, &ready, GUI_REVEAL_SET, NULL, sizeof(request)));
        request.version = 2; CHECK(!gui_reveal_receive(workspace, &pending, &ready, GUI_REVEAL_SET, &request, sizeof(request))); request.version = 1;
        request.line = 0; CHECK(!gui_reveal_receive(workspace, &pending, &ready, GUI_REVEAL_SET, &request, sizeof(request)));
        request.line = GUI_TEXT + 2u; CHECK(!gui_reveal_receive(workspace, &pending, &ready, GUI_REVEAL_SET, &request, sizeof(request))); request.line = 1;
        request.hash[0] = 'G'; CHECK(!gui_reveal_receive(workspace, &pending, &ready, GUI_REVEAL_SET, &request, sizeof(request))); memcpy(request.hash, current->hash, 65);
        request.hash[1] = 0; CHECK(!gui_reveal_receive(workspace, &pending, &ready, GUI_REVEAL_SET, &request, sizeof(request))); memcpy(request.hash, current->hash, 65);
        memset(request.project, 'x', sizeof(request.project)); CHECK(!gui_reveal_receive(workspace, &pending, &ready, GUI_REVEAL_SET, &request, sizeof(request)));
        snprintf(request.project, sizeof(request.project), "%s", workspace->root);
        snprintf(request.path, sizeof(request.path), "%s\\bad-\xff.md", workspace->root); CHECK(!gui_reveal_receive(workspace, &pending, &ready, GUI_REVEAL_SET, &request, sizeof(request)));
        CHECK(gui_full_path(NULL, "../outside.md", request.path));
        CHECK(!gui_reveal_receive(workspace, &pending, &ready, GUI_REVEAL_SET, &request, sizeof(request)));
        {
            const char *original = workspace->navigation->documents[0].path;
            workspace->navigation->documents[0].path = request.path;
            CHECK(gui_reveal_receive(workspace, &pending, &ready, GUI_REVEAL_QUERY, &request, sizeof(request)) == GUI_REVEAL_ALLOWED);
            workspace->navigation->documents[0].path = original;
        }
        CHECK(!ready);
#if defined(_WIN32)
        CHECK(gui_reveal_test_discovery_limits());
#else
        snprintf(path, sizeof(path), "%s/reveal-link", project);
        CHECK(!symlink(current->path, path)); CHECK(!gui_reveal_ordinary_path(path)); CHECK(source_remove(path, 0));
        CHECK(gui_reveal_ordinary_path("/tmp"));
#endif
        CHECK(gui_reveal_ordinary_path(workspace->root)); CHECK(gui_reveal_ordinary_path(current->path));
        CHECK(!gui_reveal_ordinary_path("\\\\server\\share\\file.md"));
        CHECK(!gui_reveal_ordinary_path("\\\\?\\C:\\file.md"));
        CHECK(gui_reveal_has_line("", 1)); CHECK(!gui_reveal_has_line("", 2)); CHECK(!gui_reveal_has_line("x", 0));
        CHECK(gui_reveal_has_line("a\r\nb\n", 3)); CHECK(!gui_reveal_has_line("a\r\nb\n", 4)); CHECK(!gui_reveal_has_line("a\rb", 1));
        CHECK(gui_reveal_has_line("caf\xc3\xa9 \xf0\x9f\x98\x80\r\nx", 2));
    }
    {
        const char *context = "{\"sections\":[{\"text\":\"caf\xc3\xa9 section\",\"highlight_start\":0,\"highlight_end\":4,\"range_unit\":\"unicode-codepoint\"}]}";
        int document = workspace->active, note_index;
        gui_tab *current = &workspace->tabs[document];
        gui_inspection *snapshot = native_memory_allocate(sizeof(*snapshot));
        attendant_feedback input = {0}; attendant_comment note;
        char json[4096], note_path[GUI_PATH] = {0};
        CHECK(snapshot != NULL);
        CHECK(gui_selection_capture_preview(workspace, document, 0, 0, "caf\xc3\xa9", 5, context, strlen(context)));
        CHECK(gui_selection_current(workspace).status == GUI_SELECTION_CONTEXT);
        CHECK(gui_inspection_snapshot(workspace, snapshot) && snapshot->header.length == 5 && snapshot->header.context_length == strlen(context));
        CHECK(gui_inspection_json(snapshot, json, sizeof(json)) && strstr(json, "\"status\":\"context\"") && strstr(json, "rendered_context"));
        memcpy(external, snapshot, offsetof(gui_inspection, text) + 5 + strlen(context) + 2);
        CHECK(gui_inspection_accept(snapshot, workspace->root, external, offsetof(gui_inspection, text) + 5 + strlen(context) + 2));
        CHECK(gui_selection_capture_preview(workspace, document, 0, 8, "caf\xc3\xa9", 5, context, strlen(context)));
        CHECK(gui_selection_current(workspace).status == GUI_SELECTION_MAPPED);
        CHECK(gui_inspection_snapshot(workspace, snapshot) && snapshot->header.end == 8);
        memcpy(external, snapshot, offsetof(gui_inspection, text) + 5 + strlen(context) + 2);
        CHECK(gui_inspection_accept(snapshot, workspace->root, external, offsetof(gui_inspection, text) + 5 + strlen(context) + 2));
        gui_selection_capture(workspace, document, GUI_SELECTION_SOURCE, GUI_SELECTION_EXACT, 1, 3);
        CHECK(!workspace->preview_text && !workspace->preview_context);
        CHECK(!gui_selection_capture_preview(workspace, document, 0, 0, "bad", GUI_TEXT + 1u, context, strlen(context)));
        CHECK(gui_selection_current(workspace).status == GUI_SELECTION_UNSUPPORTED && gui_selection_current(workspace).view == GUI_SELECTION_PREVIEW);
        CHECK(gui_selection_capture_preview(workspace, document, 0, (int)current->length + 1, "caf\xc3\xa9", 5, context, strlen(context)));
        CHECK(gui_selection_current(workspace).status == GUI_SELECTION_CONTEXT && workspace->preview_context);
        input.project = workspace->root; input.target_path = current->path; input.target_hash = current->hash;
        input.selected_text = ""; input.comment = "Context without source anchor";
        CHECK(!attendant_send_feedback(workspace->session, &input).ok);
        input.mode = ATTENDANT_FEEDBACK_CONTEXT; input.range_end = 1;
        CHECK(!attendant_send_feedback(workspace->session, &input).ok);
        CHECK(gui_send_preview_comment(workspace, document, 0, 0, context, "Context fallback probe\r\nKeep human bytes."));
        for (i = 0; i < workspace->navigation->count; i++) if (workspace->navigation->documents[i].pending) {
            result = attendant_read_comment(workspace->session, workspace->root, workspace->navigation->documents[i].path, &note);
            if (result.ok && strstr(note.body, "Context fallback probe")) snprintf(note_path, sizeof(note_path), "%s", workspace->navigation->documents[i].path);
        }
        CHECK(*note_path);
        result = attendant_read_comment(workspace->session, workspace->root, note_path, &note);
        CHECK(result.ok && !note.range_start && !note.range_end && !*note.selected_text && !strcmp(note.status, "pending"));
        CHECK(!strncmp(note.body, "Context fallback probe\r\nKeep human bytes.", strlen("Context fallback probe\r\nKeep human bytes.")) && strstr(note.body, context));
        snprintf(edited, GUI_TEXT + 1, "%s", current->text);
        CHECK(gui_set_document_text(workspace, document, edited));
        current->selection_start = 1; current->selection_end = 3;
        CHECK(gui_open_document(workspace, note_path)); note_index = workspace->active;
        CHECK(gui_open_comment_target(workspace, note_index));
        CHECK(workspace->active == document && current->dirty && current->selection_start == 1 && current->selection_end == 3 && strstr(workspace->notice, "No source anchor"));
        arena_memory_release(snapshot, sizeof(*snapshot));
    }
#if defined(__APPLE__)
    CHECK(model_delayed_inspection(workspace));
#endif
    gui_workspace_destroy(workspace); arena_memory_release(workspace, sizeof(*workspace));
    arena_memory_release(edited, GUI_TEXT + 1); arena_memory_release(external, GUI_TEXT + 1);
    printf("{\"ok\":true,\"checks\":%d}\n", checks); return 0;
}
