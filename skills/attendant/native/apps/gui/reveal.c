/* Local, bounded, latest-location IPC. No source writes or automatic navigation. */
#define WIN32_LEAN_AND_MEAN
#include "reveal.h"
#include "../../src/core/platform.h"
#if defined(__APPLE__)
#include "ipc-macos.h"
#include <sys/mount.h>
#endif
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
#if defined(_WIN32)
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
#else
    char prefix[SOURCE_PATH_CAPACITY];
    if (!path || *path != '/' || strlen(path) >= sizeof(prefix)) return 0;
    snprintf(prefix, sizeof(prefix), "%s", path);
    for (size_t i = 1; ; i++) {
        if (prefix[i] && prefix[i] != '/') continue;
        char saved = prefix[i]; prefix[i] = 0;
        source_kind kind = source_stat(prefix).kind;
        if (kind == SOURCE_LINK) {
            /* Standard macOS aliases are OS-owned, not application links. */
            char resolved[SOURCE_PATH_CAPACITY];
            const char *expected = !strcmp(prefix, "/tmp") ? "/private/tmp" : !strcmp(prefix, "/var") ? "/private/var" : !strcmp(prefix, "/etc") ? "/private/etc" : NULL;
            if (!expected || !realpath(prefix, resolved) || strcmp(resolved, expected)) return 0;
        } else if (kind != SOURCE_DIRECTORY && (saved || (kind != SOURCE_REGULAR && kind != SOURCE_MISSING))) return 0;
        if (kind != SOURCE_MISSING) {
            struct statfs volume;
            if (statfs(prefix, &volume) || !(volume.f_flags & MNT_LOCAL)) return 0;
        }
        prefix[i] = saved;
        if (!saved) break;
    }
    return 1;
#endif
}
int gui_reveal_matches(const gui_reveal *hint, const gui_tab *tab) {
    return hint->version == 1 && tab->used && !tab->dirty && !tab->external_change &&
        !source_path_compare(hint->path, tab->path) && !strcmp(hint->hash, tab->hash);
}
static int path_allowed(const gui_workspace *workspace, const char *path) {
    size_t length = strlen(workspace->root);
    int i;
#if defined(_WIN32)
    if (!_strnicmp(workspace->root, path, length) &&
        (workspace->root[length - 1] == '\\' || path[length] == '\\')) return 1;
#else
    if (!strncmp(workspace->root, path, length) && (length == 1 || path[length] == '/')) return 1;
#endif
    if (workspace->navigation) for (i = 0; i < workspace->navigation->count; i++)
        if (!source_path_compare(path, workspace->navigation->documents[i].path)) return 1;
    for (i = 0; i < GUI_TABS; i++) if (workspace->tabs[i].used && !source_path_compare(path, workspace->tabs[i].path)) return 1;
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
    if (!gui_full_path(NULL, request->project, absolute) || source_path_compare(absolute, workspace->root)) return 0;
    if (operation == GUI_REVEAL_QUERY && !*request->path) return GUI_REVEAL_MATCH;
    length = strlen(request->path);
    if (length < 3 || source_case_compare(request->path + length - 3, ".md")) return 0;
    for (i = 0; i < length; i++) if ((unsigned char)request->path[i] < 32) return 0;
    if (!gui_full_path(NULL, request->path, absolute) || source_path_compare(absolute, request->path) ||
        !path_allowed(workspace, absolute)) return 0;
    if (operation == GUI_REVEAL_QUERY) return GUI_REVEAL_ALLOWED;
    if (!pending || !ready || !request->line || request->line > GUI_TEXT + 1u || strlen(request->hash) != 64) return 0;
    for (i = 0; i < 64; i++) if (!((request->hash[i] >= '0' && request->hash[i] <= '9') ||
        (request->hash[i] >= 'a' && request->hash[i] <= 'f'))) return 0;
    memcpy(pending, request, sizeof(*pending)); *ready = 1;
    return GUI_REVEAL_ACCEPTED;
}
uintptr_t gui_search_receive(const gui_workspace *workspace, gui_search *pending, int *ready,
    int busy, const void *data, size_t size) {
    gui_search request; char absolute[GUI_PATH];
    if (!workspace || !pending || !ready || !data || size != sizeof(request)) return 0;
    memcpy(&request, data, sizeof(request));
    if (request.version != 1 || request.view > 2 || request.match_case > 1 ||
        !memchr(request.project, 0, sizeof(request.project)) || !memchr(request.path, 0, sizeof(request.path)) ||
        !memchr(request.query, 0, sizeof(request.query)) || strpbrk(request.query, "\r\n") ||
        !string_view_utf8_valid(string_view_from_cstr(request.query))) return 0;
    if (!gui_full_path(NULL, request.project, absolute) || source_path_compare(absolute, workspace->root)) return 0;
    if (*request.path) {
        size_t length = strlen(request.path);
        if (!request.view || length < 3 || source_case_compare(request.path + length - 3, ".md") ||
            !gui_full_path(NULL, request.path, absolute) || source_path_compare(absolute, request.path) || !path_allowed(workspace, absolute)) return 0;
        for (size_t i = 0; i < length; ++i) if ((unsigned char)request.path[i] < 32) return 0;
    } else if (request.view && workspace->active < 0) return 0;
    if (busy || *ready) return GUI_SEARCH_BUSY;
    memcpy(pending, &request, sizeof(request)); *ready = 1; return GUI_SEARCH_ACCEPTED;
}
#if defined(_WIN32)
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
gui_endpoint gui_find_workspace(const char *project, int *status) {
    gui_reveal request;
    reveal_discovery discovery;
    const char *code = NULL, *message = NULL;
    memset(&request, 0, sizeof(request)); request.version = 1; *status = 1;
    if (!gui_full_path(NULL, project, request.project)) {
        *status = gui_reveal_error(1, "invalid-project", "Cannot resolve bounded UTF-8 project path"); return NULL;
    }
    memset(&discovery, 0, sizeof(discovery)); discovery.request = &request; discovery.started = GetTickCount();
    SetLastError(0);
    if (!EnumWindows(find_window, (LPARAM)&discovery) && !discovery.failed) {
        code = "gui-discovery-failed"; message = "Cannot enumerate running GUI workspaces";
    } else if (discovery.failed) {
        code = "gui-unavailable"; message = "GUI discovery exceeded bounds or encountered an unresponsive instance; no hint sent";
    } else if (discovery.matches > 1) {
        code = "gui-ambiguous"; message = "Multiple GUI instances use this project; keep one instance before sending a hint";
    } else if (!discovery.matches) {
        code = "gui-not-running"; message = "No running GUI for project; open workspace first";
    }
    if (code) { *status = gui_reveal_error(1, code, message); return NULL; }
    *status = 0; return discovery.window;
}
#else
static int deliver(gui_endpoint endpoint, uintptr_t operation, const gui_reveal *request, uint32_t timeout, uintptr_t *reply) {
    uint32_t result = 0;
    if (gui_ipc_call(endpoint, (uint32_t)operation, request, timeout, &result, NULL, NULL) != 1) return 0;
    *reply = result; return 1;
}
#endif
int gui_reveal_send(const char *project, const char *path, uint32_t line) {
    gui_reveal request;
    gui_endpoint window;
    attendant_session *session;
    attendant_result source;
    uintptr_t reply;
    int status;
    memset(&request, 0, sizeof(request)); request.version = 1; request.line = line;
    if (!gui_full_path(NULL, project, request.project)) return gui_reveal_error(1, "invalid-project", "Cannot resolve bounded UTF-8 project path");
    window = gui_find_workspace(request.project, &status); if (!window) return status;
    if (!gui_full_path(request.project, path, request.path)) return gui_reveal_error(1, "invalid-path", "Cannot resolve bounded UTF-8 Markdown path");
    if (!deliver(window, GUI_REVEAL_QUERY, &request, 1000, &reply) || reply != GUI_REVEAL_ALLOWED)
        return gui_reveal_error(1, "reveal-not-allowed", "Target must be Markdown inside project or a registered/open document; recipient may also have closed");
    if (!line || line > GUI_TEXT + 1u) return gui_reveal_error(2, "invalid-line", "Line must be between 1 and 4194305");
    if (!gui_reveal_ordinary_path(request.project) || !gui_reveal_ordinary_path(request.path))
        return gui_reveal_error(1, "unsupported-path", "Reveal requires accessible ordinary local paths without application-link ancestry; editor remains available");
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
        const char *override = getenv("ATTENDANT_GUI_TEST_REVEAL_HASH");
        if (override && *override) {
            if (strlen(override) != 64) return gui_reveal_error(2, "test-hash", "Test hash must have 64 lowercase hex digits");
            memcpy(request.hash, override, 65);
        }
    }
#endif
    if (!deliver(window, GUI_REVEAL_SET, &request, 1000, &reply))
        return gui_reveal_error(1, "reveal-delivery-unknown", "Delivery failed or timed out; hint may have arrived. Retrying latest hint is safe");
    if (reply != GUI_REVEAL_ACCEPTED) return gui_reveal_error(1, "reveal-rejected", "Running GUI rejected hint; workspace or permitted target may have changed");
    puts("{\"ok\":true,\"hint_updated\":true,\"persistent\":false,\"focus_changed\":false}");
    return 0;
}
int gui_search_send(const char *project, const char *path, const char *query, uint32_t view, int match_case) {
    gui_search request = {0}; int status; uint32_t reply = 0;
    size_t length = 0;
    while (query && length < GUI_SEARCH_QUERY && query[length]) ++length;
    if (!query || length == GUI_SEARCH_QUERY || strpbrk(query, "\r\n") || view > 2 ||
        !string_view_utf8_valid(string_view_from_cstr(query)) || (!view && path))
        return gui_reveal_error(2, "invalid-search", "Use one valid UTF-8 query of at most 1023 bytes; sidebar search cannot have --file");
    request.version = 1; request.view = view; request.match_case = match_case != 0;
    memcpy(request.query, query, length + 1);
    if (!gui_full_path(NULL, project, request.project) || (path && !gui_full_path(request.project, path, request.path)))
        return gui_reveal_error(1, "invalid-path", "Cannot resolve bounded search project/file path");
    if (!gui_reveal_ordinary_path(request.project) || (path && !gui_reveal_ordinary_path(request.path)))
        return gui_reveal_error(1, "unsupported-path", "Search requires ordinary local paths without application-link ancestry");
    gui_endpoint endpoint = gui_find_workspace(request.project, &status); if (!endpoint) return status;
#if defined(_WIN32)
    COPYDATASTRUCT copy; DWORD_PTR received = 0;
    copy.dwData = GUI_SEARCH_SET; copy.cbData = sizeof(request); copy.lpData = &request;
    int delivered = SendMessageTimeoutA(endpoint, WM_COPYDATA, 0, (LPARAM)&copy, SMTO_ABORTIFHUNG | SMTO_BLOCK, 1000, &received) != 0;
    reply = (uint32_t)received;
#else
    int delivered = gui_ipc_search_call(endpoint, &request, 1000, &reply) == 1;
#endif
    if (!delivered) return gui_reveal_error(1, "search-delivery-unknown", "Delivery failed or timed out; search may have arrived. Inspect GUI before retrying");
    if (reply == GUI_SEARCH_BUSY) return gui_reveal_error(1, "gui-busy", "GUI is rendering, displaying a modal dialog or already has a queued search; no search queued");
    if (reply != GUI_SEARCH_ACCEPTED) return gui_reveal_error(1, "search-refused", "GUI rejected search: missing active document, disallowed file, or unsupported command");
    puts("{\"ok\":true,\"delivered\":true,\"persistent\":false,\"os_focus_changed\":false}");
    return 0;
}
