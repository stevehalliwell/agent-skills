#ifndef ATTENDANT_GUI_IPC_MACOS_H
#define ATTENDANT_GUI_IPC_MACOS_H
/* macOS transport only. Reuse bounded, framed, same-user pipes from base.h.
 * One viewer per process. Server pumps only between frames on UI thread. */
#include "../../src/include/base.h"
#include "inspect.h"
typedef struct gui_ipc_request {
    uint32_t operation;
    gui_reveal hint;
} gui_ipc_request;
typedef struct gui_ipc_server {
    pipe_channel channel;
    gui_ipc_request request;
    uint32_t started, reply;
    size_t snapshot_size;
    int phase;
#ifdef ATTENDANT_GUI_TESTING
    uint32_t inspections;
#endif
} gui_ipc_server;
int gui_ipc_start(gui_ipc_server *server);
void gui_ipc_poll(gui_ipc_server *server, const gui_workspace *workspace, gui_reveal *pending, int *ready, gui_inspection *snapshot);
void gui_ipc_stop(gui_ipc_server *server);
/* 1 reply received, -1 endpoint absent/refused, 0 failure/timeout. Snapshot
 * storage is caller-owned; accept/validate before interpreting received bytes. */
int gui_ipc_call(const char *endpoint, uint32_t operation, const gui_reveal *hint,
    uint32_t timeout, uint32_t *reply, gui_inspection *snapshot, size_t *snapshot_size);
#endif
