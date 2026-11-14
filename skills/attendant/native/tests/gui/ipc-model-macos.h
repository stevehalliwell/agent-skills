/* Test-only maximum inspection and frozen-reply check, without rendering 8 MiB.
 * Forked same-user client delays reading while model replaces preview selection.
 * Parent owns endpoint and all allocations; finite waits reap child on failure. */
#include "../../apps/gui/ipc-macos.h"
#include <sys/wait.h>
#include <signal.h>
static int model_delayed_inspection(gui_workspace *workspace) {
    const size_t capacity = 2u * GUI_TEXT + 2u;
    char *input = native_memory_allocate(capacity);
    gui_inspection *snapshot = native_memory_allocate(sizeof(*snapshot));
    gui_ipc_server server = {0};
    int ok = 0, changed = 0, status = 0, reaped = 0;
    pid_t child = -1;
    if (!input || !snapshot) goto cleanup;
    memset(input, 'a', GUI_TEXT); input[GUI_TEXT] = 0;
    char *context = input + GUI_TEXT + 1;
    memset(context, 'c', GUI_TEXT);
    memcpy(context, "{\"text\":\"", 9); context[GUI_TEXT - 2] = '"'; context[GUI_TEXT - 1] = '}'; context[GUI_TEXT] = 0;
    if (!gui_selection_capture_preview(workspace, workspace->active, 0, 0, input, GUI_TEXT, context, GUI_TEXT) || !gui_ipc_start(&server)) goto cleanup;
    child = fork();
    if (child < 0) goto cleanup;
    if (!child) {
        close(server.channel.listener - 1); /* Do not unlink parent's endpoint. */
        pipe_channel connection = {0};
        gui_ipc_request request = {0};
        uint32_t reply = 0;
        size_t bytes = 0;
        request.operation = GUI_INSPECT_REQUEST; request.hint.version = 1;
        snprintf(request.hint.project, sizeof(request.hint.project), "%s", workspace->root);
        int received = pipe_client_open(server.channel.path, 1000, &connection) == pipe_error_ok &&
            pipe_begin(&connection, pipe_operation_write, &request, sizeof(request)) == pipe_error_pending &&
            pipe_wait(&connection, 1000, &bytes) == pipe_error_ok && bytes == sizeof(request) &&
            pipe_begin(&connection, pipe_operation_read, &reply, sizeof(reply)) == pipe_error_pending &&
            pipe_wait(&connection, 1000, &bytes) == pipe_error_ok && bytes == sizeof(reply) && reply == GUI_INSPECT_RESULT;
        if (!received) fprintf(stderr, "Delayed inspection: request/reply failed (reply %u, bytes %zu)\n", reply, bytes);
        system_sleep(100);
        if (received) {
            pipe_error begin = pipe_begin(&connection, pipe_operation_read, snapshot, sizeof(*snapshot));
            /* Server enforces its own 2000 ms connection deadline. Do not
               impose a shorter client deadline on maximum-size backpressure. */
            pipe_error waited = begin == pipe_error_pending ? pipe_wait(&connection, 2000, &bytes) : begin;
            received = waited == pipe_error_ok &&
                gui_inspection_accept(snapshot, workspace->root, snapshot, bytes) &&
                snapshot->header.length == GUI_TEXT && snapshot->header.context_length == GUI_TEXT &&
                !memcmp(snapshot->text, input, capacity);
            if (!received) fprintf(stderr, "Delayed inspection: snapshot failed (begin %d, wait %d, bytes %zu, used %zu/%u)\n", begin, waited, bytes, connection.used, connection.message_size);
        }
        pipe_close(&connection, 0); _exit(received ? 0 : 1);
    }
    uint32_t started = system_milliseconds();
    gui_reveal pending = {0}; int ready = 0;
    while (system_milliseconds() - started < 4000) {
        gui_ipc_poll(&server, workspace, &pending, &ready, snapshot, NULL, NULL, 0);
        if (server.snapshot_size && !changed) {
            memset(workspace->preview_text, 'b', GUI_TEXT);
            memset(workspace->preview_context, 'd', GUI_TEXT); changed = 1;
        }
        pid_t result = waitpid(child, &status, WNOHANG);
        if (result == child) { reaped = 1; break; }
        if (result < 0) break;
        system_sleep(1);
    }
    ok = reaped && changed && !ready && WIFEXITED(status) && WEXITSTATUS(status) == 0;
cleanup:
    if (child > 0 && !reaped) { kill(child, SIGTERM); waitpid(child, &status, 0); }
    gui_ipc_stop(&server); gui_selection_clear(workspace);
    if (snapshot) arena_memory_release(snapshot, sizeof(*snapshot));
    if (input) arena_memory_release(input, capacity);
    return ok;
}
