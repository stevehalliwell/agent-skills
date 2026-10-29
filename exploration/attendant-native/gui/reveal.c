/* Local, bounded, latest-location IPC. No source writes or automatic navigation. */
#define WIN32_LEAN_AND_MEAN
#include "reveal.h"
#include <stdio.h>
#include <string.h>
#define JSONB_API static inline
#include "../vendor/json-build/json-build.h"

int gui_reveal_error(int status, const char *code, const char *message) {
    char quoted[65536];
    jsonb builder;
    if (!message) message = "Operation failed";
    jsonb_init(&builder);
    if (jsonb_string(&builder, quoted, sizeof(quoted), message, strlen(message)) < 0)
        snprintf(quoted, sizeof(quoted), "\"Operation failed; diagnostic exceeded output capacity\"");
    printf("{\"ok\":false,\"code\":\"%s\",\"message\":%s}\n", code, quoted);
    fprintf(stderr, "%s: %.8192s\n", code, message);
    return status;
}
int gui_reveal_has_line(const char *text, uint32_t line) {
    uint32_t count = 1;
    const char *cursor;
    if (!text || !line || line > GUI_TEXT + 1u) return 0;
    for (cursor = text; *cursor; cursor++) {
        if (*cursor == '\r' && cursor[1] != '\n') return 0;
        if (*cursor == '\n') count++;
    }
    return line <= count;
}
int gui_reveal_ordinary_path(const char *path) {
    wchar_t wide[4096], drive[4];
    DWORD attributes;
    UINT kind;
    size_t i;
    if (!path || strlen(path) < 3 || !((path[0] >= 'A' && path[0] <= 'Z') || (path[0] >= 'a' && path[0] <= 'z')) ||
        path[1] != ':' || path[2] != '\\' || !MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path, -1, wide, 4096)) return 0;
    drive[0] = wide[0]; drive[1] = L':'; drive[2] = L'\\'; drive[3] = 0;
    kind = GetDriveTypeW(drive);
    if (kind != DRIVE_FIXED && kind != DRIVE_REMOVABLE && kind != DRIVE_RAMDISK) return 0;
    attributes = GetFileAttributesW(drive);
    if (attributes == INVALID_FILE_ATTRIBUTES || (attributes & FILE_ATTRIBUTE_REPARSE_POINT)) return 0;
    for (i = 3; wide[i]; i++) if (wide[i] == L'\\') {
        wide[i] = 0; attributes = GetFileAttributesW(wide); wide[i] = L'\\';
        if (attributes == INVALID_FILE_ATTRIBUTES || !(attributes & FILE_ATTRIBUTE_DIRECTORY) || (attributes & FILE_ATTRIBUTE_REPARSE_POINT)) return 0;
    }
    attributes = GetFileAttributesW(wide);
    if (attributes == INVALID_FILE_ATTRIBUTES) return GetLastError() == ERROR_FILE_NOT_FOUND;
    return !(attributes & FILE_ATTRIBUTE_REPARSE_POINT);
}
int gui_reveal_matches(const gui_reveal *hint, const gui_tab *tab) {
    return hint->version == 1 && tab->used && !tab->dirty && !tab->external_change &&
        !_stricmp(hint->path, tab->path) && !strcmp(hint->hash, tab->hash);
}
static int path_allowed(const gui_workspace *workspace, const char *path) {
    size_t length = strlen(workspace->root);
    int i;
    if (!_strnicmp(workspace->root, path, length) &&
        (workspace->root[length - 1] == '\\' || path[length] == '\\')) return 1;
    if (workspace->navigation) for (i = 0; i < workspace->navigation->count; i++)
        if (!_stricmp(path, workspace->navigation->documents[i].path)) return 1;
    for (i = 0; i < GUI_TABS; i++) if (workspace->tabs[i].used && !_stricmp(path, workspace->tabs[i].path)) return 1;
    return 0;
}
uintptr_t gui_reveal_receive(const gui_workspace *workspace, gui_reveal *pending,
    int *ready, uintptr_t operation, const void *data, size_t size) {
    gui_reveal incoming;
    const gui_reveal *request;
    char absolute[GUI_PATH];
    size_t length, i;
    if (!workspace || !*workspace->root || !data || size != sizeof(incoming) ||
        (operation != GUI_REVEAL_QUERY && operation != GUI_REVEAL_SET)) return 0;
    /* Snapshot before typed access: raw IPC input need not be aligned. */
    memcpy(&incoming, data, sizeof(incoming)); request = &incoming;
    if (!memchr(request->project, 0, GUI_PATH) || !memchr(request->path, 0, GUI_PATH) ||
        !memchr(request->hash, 0, 65) || request->version != 1) return 0;
    if (!gui_full_path(NULL, request->project, absolute) || _stricmp(absolute, workspace->root)) return 0;
    if (operation == GUI_REVEAL_QUERY && !*request->path) return GUI_REVEAL_MATCH;
    length = strlen(request->path);
    if (length < 3 || _stricmp(request->path + length - 3, ".md")) return 0;
    for (i = 0; i < length; i++) if ((unsigned char)request->path[i] < 32) return 0;
    if (!gui_full_path(NULL, request->path, absolute) || _stricmp(absolute, request->path) ||
        !path_allowed(workspace, absolute)) return 0;
    if (operation == GUI_REVEAL_QUERY) return GUI_REVEAL_ALLOWED;
    if (!pending || !ready || !request->line || request->line > GUI_TEXT + 1u || strlen(request->hash) != 64) return 0;
    for (i = 0; i < 64; i++) if (!((request->hash[i] >= '0' && request->hash[i] <= '9') ||
        (request->hash[i] >= 'a' && request->hash[i] <= 'f'))) return 0;
    memcpy(pending, request, sizeof(*pending)); *ready = 1;
    return GUI_REVEAL_ACCEPTED;
}
static int deliver(HWND window, uintptr_t operation, const gui_reveal *request, DWORD timeout, uintptr_t *reply) {
    COPYDATASTRUCT copy;
    DWORD_PTR result = 0;
    copy.dwData = (ULONG_PTR)operation; copy.cbData = (DWORD)sizeof(*request); copy.lpData = (void *)request;
    if (!SendMessageTimeoutA(window, WM_COPYDATA, 0, (LPARAM)&copy,
        SMTO_ABORTIFHUNG | SMTO_BLOCK, timeout, &result)) return 0;
    *reply = (uintptr_t)result; return 1;
}
typedef struct reveal_discovery {
    gui_reveal *request;
    HWND window;
    DWORD started;
    int visited, candidates, matches, failed;
} reveal_discovery;
static BOOL CALLBACK find_window(HWND window, LPARAM parameter) {
    reveal_discovery *discovery = (reveal_discovery *)parameter;
    char name[128];
    uintptr_t reply;
    DWORD elapsed, timeout;
    elapsed = GetTickCount() - discovery->started;
    if (++discovery->visited > 4096 || elapsed >= 2000) { discovery->failed = 1; return FALSE; }
    if (!GetClassNameA(window, name, sizeof(name)) || strcmp(name, GUI_WINDOW_CLASS)) return TRUE;
    if (++discovery->candidates > 16) { discovery->failed = 1; return FALSE; }
    timeout = 2000 - elapsed; if (timeout > 250) timeout = 250;
    if (!deliver(window, GUI_REVEAL_QUERY, discovery->request, timeout, &reply)) discovery->failed = 1;
    else if (reply == GUI_REVEAL_MATCH) { discovery->matches++; discovery->window = window; }
    return TRUE;
}
#ifdef ATTENDANT_MODEL_TESTING
int gui_reveal_test_discovery_limits(void) {
    reveal_discovery discovery;
    memset(&discovery, 0, sizeof(discovery)); discovery.started = GetTickCount() - 2000;
    if (find_window(NULL, (LPARAM)&discovery) || !discovery.failed) return 0;
    memset(&discovery, 0, sizeof(discovery)); discovery.started = GetTickCount(); discovery.visited = 4096;
    if (find_window(NULL, (LPARAM)&discovery) || !discovery.failed) return 0;
    memset(&discovery, 0, sizeof(discovery)); discovery.started = GetTickCount();
    return find_window(NULL, (LPARAM)&discovery) && !discovery.failed && discovery.visited == 1;
}
#endif
int gui_reveal_send(const char *project, const char *path, uint32_t line) {
    gui_reveal request;
    reveal_discovery discovery;
    attendant_session *session;
    attendant_result source;
    uintptr_t reply;
    int status;
    memset(&request, 0, sizeof(request)); request.version = 1; request.line = line;
    if (!gui_full_path(NULL, project, request.project)) return gui_reveal_error(1, "invalid-project", "Cannot resolve bounded UTF-8 project path");
    memset(&discovery, 0, sizeof(discovery)); discovery.request = &request; discovery.started = GetTickCount();
    SetLastError(0);
    if (!EnumWindows(find_window, (LPARAM)&discovery) && !discovery.failed)
        return gui_reveal_error(1, "gui-discovery-failed", "Cannot enumerate running GUI workspaces");
    if (discovery.failed) return gui_reveal_error(1, "gui-unavailable", "GUI discovery exceeded bounds or encountered an unresponsive instance; no hint sent");
    if (discovery.matches > 1) return gui_reveal_error(1, "gui-ambiguous", "Multiple GUI instances use this project; keep one instance before sending a hint");
    if (!discovery.matches) return gui_reveal_error(1, "gui-not-running", "No running GUI for project; open workspace first");
    if (!gui_full_path(request.project, path, request.path)) return gui_reveal_error(1, "invalid-path", "Cannot resolve bounded UTF-8 Markdown path");
    if (!deliver(discovery.window, GUI_REVEAL_QUERY, &request, 1000, &reply) || reply != GUI_REVEAL_ALLOWED)
        return gui_reveal_error(1, "reveal-not-allowed", "Target must be Markdown inside project or a registered/open document; recipient may also have closed");
    if (!line || line > GUI_TEXT + 1u) return gui_reveal_error(2, "invalid-line", "Line must be between 1 and 4194305");
    if (!gui_reveal_ordinary_path(request.project) || !gui_reveal_ordinary_path(request.path))
        return gui_reveal_error(1, "unsupported-path", "Reveal requires accessible ordinary local drive paths without reparse ancestry; editor remains available");
    session = attendant_open();
    if (!session) return gui_reveal_error(1, "capacity", "Cannot allocate source read session");
    source = attendant_read_document(session, request.path);
    if (!source.ok) {
        status = gui_reveal_error(1, "source-read-failed", source.message);
        attendant_close(session); return status;
    }
    if (!gui_reveal_has_line(source.text, line)) {
        attendant_close(session); return gui_reveal_error(1, "unmapped-line", "Line exceeds saved source or source contains unsupported bare-CR line endings");
    }
    memcpy(request.hash, source.hash, 65); attendant_close(session);
#ifdef ATTENDANT_GUI_TESTING
    /* Test-only stale-source sender; production contains no override. */
    {
        char override[65]; DWORD length = GetEnvironmentVariableA("ATTENDANT_GUI_TEST_REVEAL_HASH", override, sizeof(override));
        if (length) {
            if (length != 64) return gui_reveal_error(2, "test-hash", "Test hash must have 64 lowercase hex digits");
            memcpy(request.hash, override, 65);
        }
    }
#endif
    if (!deliver(discovery.window, GUI_REVEAL_SET, &request, 1000, &reply))
        return gui_reveal_error(1, "reveal-delivery-unknown", "Delivery failed or timed out; hint may have arrived. Retrying latest hint is safe");
    if (reply != GUI_REVEAL_ACCEPTED) return gui_reveal_error(1, "reveal-rejected", "Running GUI rejected hint; workspace or permitted target may have changed");
    puts("{\"ok\":true,\"hint_updated\":true,\"persistent\":false,\"focus_changed\":false}");
    return 0;
}
