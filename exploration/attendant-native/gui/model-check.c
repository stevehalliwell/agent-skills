/* Test-only document-state acceptance. Requires generated ownership marker.
   CHECK always runs, including toolchains which define NDEBUG at -O3. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include "workspace.h"
__declspec(dllimport) LPWSTR *WINAPI CommandLineToArgvW(LPCWSTR, int *);
static int checks, edit_after_read;
void gui_model_after_read(const char *path) {
    wchar_t wide[4096]; HANDLE file; DWORD count;
    const char *suffix = "\nExternal read/stat gap edit.\n";
    if (!edit_after_read) return;
    edit_after_read = 0;
    if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path, -1, wide, 4096)) ExitProcess(2);
    file = CreateFileW(wide, FILE_APPEND_DATA, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) ExitProcess(2);
    if (!WriteFile(file, suffix, (DWORD)strlen(suffix), &count, NULL) || count != strlen(suffix)) { CloseHandle(file); ExitProcess(2); }
    CloseHandle(file);
}
#define CHECK(expression) do { checks++; if (!(expression)) { fprintf(stderr, "Model check failed at line %d: %s\n", __LINE__, #expression); return 1; } } while (0)
int main(void) {
    int argc, first, second, comment_index, i, references = 0;
    LPWSTR *arguments = CommandLineToArgvW(GetCommandLineW(), &argc);
    char project[GUI_PATH], marker[GUI_PATH], path[GUI_PATH], comment_path[GUI_PATH] = {0}, expected_hash[65], *edited, *external;
    wchar_t wide[4096];
    gui_workspace *workspace;
    gui_tab *tab;
    attendant_result result;
    const char *selected = "caf\xc3\xa9 \xce\xa9 \xf0\x9f\x98\x80\r\n";
    CHECK(arguments && argc == 2);
    CHECK(WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, arguments[1], -1, project, GUI_PATH, NULL, NULL)); LocalFree(arguments);
    snprintf(marker, sizeof(marker), "%s/.gui-test-owned", project);
    CHECK(MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, marker, -1, wide, 4096)); CHECK(GetFileAttributesW(wide) != INVALID_FILE_ATTRIBUTES);
    workspace = VirtualAlloc(NULL, sizeof(*workspace), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    edited = VirtualAlloc(NULL, GUI_TEXT + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    external = VirtualAlloc(NULL, GUI_TEXT + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    CHECK(workspace && edited && external && gui_workspace_init(workspace, project));
    CHECK(gui_open_document(workspace, "records/tasks/first.md")); first = workspace->active; tab = &workspace->tabs[first];
    for (i = 0; i < workspace->navigation->count; i++) if (!strcmp(workspace->navigation->documents[i].path, tab->path)) {
        int j; attendant_document_info *document = &workspace->navigation->documents[i];
        for (j = 0; j < document->reference_count; j++) {
            attendant_reference *reference = &document->references[j];
            if (!strcmp(reference->value, "second")) { CHECK(reference->target_path); references++; }
            if (!strcmp(reference->value, "missing")) { CHECK(!reference->target_path); references++; }
        }
    }
    CHECK(references == 2);
    CHECK(gui_open_link(workspace, "second.md")); second = workspace->active; CHECK(second != first && workspace->tab_count == 2);
    CHECK(gui_open_link(workspace, "attendant://tasks/first")); CHECK(workspace->active == first && workspace->tab_count == 2);
    snprintf(edited, GUI_TEXT + 1, "%s\r\nModel selection: %s", tab->text, selected);
    CHECK(gui_set_document_text(workspace, first, edited));
    tab->selection_start = (int)(strstr(tab->text, "Model selection: ") - tab->text) + 17;
    tab->selection_end = tab->selection_start + (int)strlen(selected);
    CHECK(!gui_close_document(workspace, first, 0));
    CHECK(!gui_send_comment(workspace, first, "Must not send dirty text."));
    CHECK(gui_refresh(workspace)); CHECK(workspace->active == first && tab->dirty && !strcmp(tab->text, edited));
    CHECK(gui_open_document(workspace, tab->path)); CHECK(workspace->tab_count == 2 && tab->dirty);
    CHECK(gui_save_document(workspace, first)); CHECK(!tab->dirty);
    result = attendant_read_document(workspace->session, tab->path); CHECK(result.ok && !strcmp(result.text, edited));
    CHECK(SetEnvironmentVariableA("ATTENDANT_FEEDBACK_FAULT", "published-edit"));
    CHECK(!gui_send_comment(workspace, first, "Failed bootstrap must retain surviving paths."));
    CHECK(SetEnvironmentVariableA("ATTENDANT_FEEDBACK_FAULT", NULL));
    CHECK(workspace->surviving_count == 1 && workspace->surviving_total == 1);
    snprintf(path, sizeof(path), "%s", workspace->surviving_paths[0]);
    CHECK(gui_refresh(workspace)); CHECK(!strcmp(path, workspace->surviving_paths[0]));
    result = attendant_read_document(workspace->session, path);
    CHECK(result.ok && strstr(result.text, "# Concurrent external edit"));
    CHECK(MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path, -1, wide, 4096)); CHECK(DeleteFileW(wide));
    { wchar_t *separator = wcsrchr(wide, L'\\'); CHECK(separator); *separator = 0; CHECK(RemoveDirectoryW(wide)); }
    CHECK(gui_send_comment(workspace, first, "Review this Unicode selection.\r\nKeep body exact.\n"));
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
        edit_after_read = 1; CHECK(gui_reload_document(workspace, document, 0)); CHECK(workspace->tabs[document].length == length);
        gui_check_external(workspace, document); CHECK(workspace->tabs[document].length > length);
        length = workspace->tabs[document].length; CHECK(gui_close_document(workspace, document, 0));
        edit_after_read = 1; CHECK(gui_open_document(workspace, "README.md")); document = workspace->active;
        CHECK(workspace->tabs[document].length == length); gui_check_external(workspace, document); CHECK(workspace->tabs[document].length > length);
    }
    CHECK(gui_body_offset("---\r\nname: a\r\n---\r\nBody") == 19);
    gui_workspace_destroy(workspace); VirtualFree(workspace, 0, MEM_RELEASE);
    VirtualFree(edited, 0, MEM_RELEASE); VirtualFree(external, 0, MEM_RELEASE);
    printf("{\"ok\":true,\"checks\":%d}\n", checks); return 0;
}
