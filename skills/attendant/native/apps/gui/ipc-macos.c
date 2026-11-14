/* Same-UID local control over existing Unix message pipes (across login sessions). No threads,
 * source writes, focus changes, or IPC files inside managed workspaces. */
#include "ipc-macos.h"
#if !defined(__APPLE__)
#error macOS transport compiled on another platform
#endif
#define GUI_IPC_DEADLINE 2000u
#define GUI_IPC_LABEL "attendant-gui"
enum { GUI_IPC_CONNECT, GUI_IPC_READ, GUI_IPC_REPLY, GUI_IPC_SNAPSHOT };

int gui_ipc_start(gui_ipc_server *server) {
    char path[104];
    if (!pipe_local_name(path, sizeof(path), GUI_IPC_LABEL, process_id()) ||
        pipe_server_open(path, sizeof(server->packet), sizeof(gui_inspection), &server->channel) != pipe_error_ok) return 0;
    server->phase = GUI_IPC_CONNECT;
    if (pipe_begin(&server->channel, pipe_operation_connect, NULL, 0) != pipe_error_pending) {
        gui_ipc_stop(server); return 0;
    }
    return 1;
}
void gui_ipc_stop(gui_ipc_server *server) {
    pipe_close(&server->channel, 0);
    memset(server, 0, sizeof(*server));
}
static void next_client(gui_ipc_server *server) {
    if (server->channel.pending) {
        usize ignored;
        pipe_cancel(&server->channel); pipe_poll(&server->channel, &ignored);
    }
    pipe_server_disconnect(&server->channel);
    server->phase = GUI_IPC_CONNECT; server->snapshot_size = 0;
    if (pipe_begin(&server->channel, pipe_operation_connect, NULL, 0) != pipe_error_pending) gui_ipc_stop(server);
}
void gui_ipc_poll(gui_ipc_server *server, const gui_workspace *workspace, gui_reveal *pending, int *ready, gui_inspection *snapshot, gui_search *search, int *search_ready, int busy) {
    if (!server->channel.handle) return;
    /* Finite work per frame, including fragmented messages and large selections. */
    for (int step = 0; step < 32; step++) {
        usize transferred = 0;
        if (server->phase != GUI_IPC_CONNECT && system_milliseconds() - server->started >= GUI_IPC_DEADLINE) {
            next_client(server); return;
        }
        pipe_error error = pipe_poll(&server->channel, &transferred);
        if (error == pipe_error_pending) return;
        if (error != pipe_error_ok) { next_client(server); return; }
        if (server->phase == GUI_IPC_CONNECT) {
            server->started = system_milliseconds(); server->phase = GUI_IPC_READ;
            error = pipe_begin(&server->channel, pipe_operation_read, server->packet, sizeof(server->packet));
        } else if (server->phase == GUI_IPC_READ) {
            uint32_t operation;
            if (transferred < sizeof(operation)) { next_client(server); return; }
            memcpy(&operation, server->packet, sizeof(operation));
            server->snapshot_size = 0;
            if (operation == GUI_SEARCH_SET && transferred == sizeof(gui_ipc_search_request)) {
                server->reply = (uint32_t)gui_search_receive(workspace, search, search_ready, busy,
                    server->packet + offsetof(gui_ipc_search_request, search), sizeof(gui_search));
            } else {
                /* Legacy reveal/inspect frame layout remains byte-for-byte unchanged. */
                if (transferred != sizeof(server->request)) { next_client(server); return; }
                memcpy(&server->request, server->packet, sizeof(server->request));
                if (server->request.operation == GUI_INSPECT_REQUEST) {
                    server->reply = 0;
                    if (snapshot && gui_reveal_receive(workspace, NULL, NULL, GUI_REVEAL_QUERY, &server->request.hint, sizeof(gui_reveal)) == GUI_REVEAL_MATCH &&
                        gui_inspection_snapshot(workspace, snapshot)) {
                        server->reply = GUI_INSPECT_RESULT;
#ifdef ATTENDANT_GUI_TESTING
                        server->inspections++;
#endif
                        server->snapshot_size = offsetof(gui_inspection, text) + snapshot->header.length + snapshot->header.context_length + 2;
                    }
                } else server->reply = (uint32_t)gui_reveal_receive(workspace, pending, ready,
                    server->request.operation, &server->request.hint, sizeof(gui_reveal));
            }
            server->phase = GUI_IPC_REPLY;
            error = pipe_begin(&server->channel, pipe_operation_write, &server->reply, sizeof(server->reply));
        } else if (server->phase == GUI_IPC_REPLY && server->snapshot_size) {
            server->phase = GUI_IPC_SNAPSHOT;
            error = pipe_begin(&server->channel, pipe_operation_write, snapshot, server->snapshot_size);
        } else { next_client(server); return; }
        if (error != pipe_error_pending) { next_client(server); return; }
    }
}
static int exchange(pipe_channel *channel, pipe_operation operation, void *bytes, size_t size, uint32_t started, uint32_t timeout, size_t *transferred) {
    uint32_t elapsed = system_milliseconds() - started;
    if (elapsed >= timeout || pipe_begin(channel, operation, bytes, size) != pipe_error_pending) return 0;
    return pipe_wait(channel, timeout - elapsed, transferred) == pipe_error_ok;
}
int gui_ipc_call(const char *endpoint, uint32_t operation, const gui_reveal *hint,
    uint32_t timeout, uint32_t *reply, gui_inspection *snapshot, size_t *snapshot_size) {
    pipe_channel channel = {0};
    gui_ipc_request request;
    size_t transferred = 0;
    uint32_t started = system_milliseconds();
    pipe_error error = pipe_client_open(endpoint, timeout, &channel);
    if (error != pipe_error_ok) return error == pipe_error_unavailable ? -1 : 0;
    memset(&request, 0, sizeof(request)); request.operation = operation; memcpy(&request.hint, hint, sizeof(*hint));
    int ok = exchange(&channel, pipe_operation_write, &request, sizeof(request), started, timeout, &transferred) && transferred == sizeof(request) &&
        exchange(&channel, pipe_operation_read, reply, sizeof(*reply), started, timeout, &transferred) && transferred == sizeof(*reply);
    if (ok && *reply == GUI_INSPECT_RESULT) {
        ok = snapshot && snapshot_size && exchange(&channel, pipe_operation_read, snapshot, sizeof(*snapshot), started, timeout, snapshot_size);
    }
    pipe_close(&channel, 0); return ok ? 1 : 0;
}
int gui_ipc_search_call(const char *endpoint, const gui_search *search, uint32_t timeout, uint32_t *reply) {
    pipe_channel channel = {0}; gui_ipc_search_request request = {0}; size_t transferred = 0;
    uint32_t started = system_milliseconds();
    pipe_error error = pipe_client_open(endpoint, timeout, &channel);
    if (error != pipe_error_ok) return error == pipe_error_unavailable ? -1 : 0;
    request.operation = GUI_SEARCH_SET; memcpy(&request.search, search, sizeof(*search));
    int ok = exchange(&channel, pipe_operation_write, &request, sizeof(request), started, timeout, &transferred) && transferred == sizeof(request) &&
        exchange(&channel, pipe_operation_read, reply, sizeof(*reply), started, timeout, &transferred) && transferred == sizeof(*reply);
    pipe_close(&channel, 0); return ok ? 1 : 0;
}
gui_endpoint gui_find_workspace(const char *project, int *status) {
    static char found[104];
    gui_reveal request = {0};
    char pattern[104], directory[104];
    uint32_t started = system_milliseconds();
    int visited = 0, candidates = 0, matches = 0, failed = 0;
    *status = 1; request.version = 1;
    if (!gui_full_path(NULL, project, request.project)) {
        gui_reveal_error(1, "invalid-project", "Cannot resolve bounded UTF-8 project path"); return NULL;
    }
    if (!pipe_local_name(pattern, sizeof(pattern), GUI_IPC_LABEL, 1)) return NULL;
    snprintf(directory, sizeof(directory), "%s", pattern); *strrchr(directory, '/') = 0;
    if (!pipe_macos_directory(directory)) {
        gui_reveal_error(1, "gui-not-running", "No accessible running GUI for project; open workspace first"); return NULL;
    }
    DIR *scan = opendir(directory);
    if (!scan) { gui_reveal_error(1, "gui-discovery-failed", "Cannot enumerate local GUI endpoints"); return NULL; }
    struct dirent *entry;
    errno = 0;
    while ((entry = readdir(scan))) {
        if (++visited > 4096 || system_milliseconds() - started >= GUI_IPC_DEADLINE) { failed = 1; break; }
        const char *prefix = GUI_IPC_LABEL "-";
        size_t prefix_length = strlen(prefix);
        if (strncmp(entry->d_name, prefix, prefix_length)) continue;
        const char *pid = entry->d_name + prefix_length;
        if (!*pid || strspn(pid, "0123456789") != strlen(pid)) continue;
        if (++candidates > 16) { failed = 1; break; }
        char endpoint[104];
        int length = snprintf(endpoint, sizeof(endpoint), "%s/%s", directory, entry->d_name);
        if (length < 0 || length >= (int)sizeof(endpoint)) { failed = 1; break; }
        struct stat metadata;
        if (lstat(endpoint, &metadata) || !S_ISSOCK(metadata.st_mode) || metadata.st_uid != getuid() || (metadata.st_mode & 0777) != 0600) { failed = 1; break; }
        uint32_t elapsed = system_milliseconds() - started, reply = 0;
        if (elapsed >= GUI_IPC_DEADLINE) { failed = 1; break; }
        uint32_t timeout = GUI_IPC_DEADLINE - elapsed;
        if (timeout > 250) timeout = 250;
        int result = gui_ipc_call(endpoint, GUI_REVEAL_QUERY, &request, timeout, &reply, NULL, NULL);
        if (!result || (result > 0 && reply != 0 && reply != GUI_REVEAL_MATCH)) failed = 1;
        else if (result > 0 && reply == GUI_REVEAL_MATCH) { matches++; snprintf(found, sizeof(found), "%s", endpoint); }
        errno = 0;
    }
    if (errno) failed = 1;
    closedir(scan);
    if (failed) gui_reveal_error(1, "gui-unavailable", "GUI discovery exceeded bounds or encountered an unresponsive instance; no request sent");
    else if (matches > 1) gui_reveal_error(1, "gui-ambiguous", "Multiple GUI instances use this project; keep one instance before sending a request");
    else if (!matches) gui_reveal_error(1, "gui-not-running", "No running GUI for project; open workspace first");
    else { *status = 0; return found; }
    return NULL;
}
