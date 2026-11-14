#ifndef ATTENDANT_GUI_REVEAL_H
#define ATTENDANT_GUI_REVEAL_H
#include "workspace.h"
#define GUI_WINDOW_CLASS "AttendantMarkdownWorkspace"
#define GUI_REVEAL_QUERY 0x41545131u
#define GUI_REVEAL_SET 0x41545231u
#define GUI_REVEAL_MATCH 0x41544d31u
#define GUI_REVEAL_ALLOWED 0x41545031u
#define GUI_REVEAL_ACCEPTED 0x41544131u
/* Local IPC v1. Windows desktop / macOS same UID across login sessions.
 * No pointers, source bytes, edit authority or persistence.
 * Same fixed-width layout for CJIT/Zig. Latest hint wins; receiver owns copies. */
typedef struct gui_reveal {
    uint32_t version, line;
    char project[GUI_PATH], path[GUI_PATH], hash[65];
} gui_reveal;
/* Callback only validates/copies. Never enters source session or changes view. */
uintptr_t gui_reveal_receive(const gui_workspace *workspace, gui_reveal *pending,
    int *ready, uintptr_t operation, const void *data, size_t size);
int gui_reveal_error(int status, const char *code, const char *message);
#if defined(_WIN32)
typedef HWND gui_endpoint;
#else
typedef const char *gui_endpoint; /* Borrowed discovery path; single-threaded CLI. */
#endif
gui_endpoint gui_find_workspace(const char *project, int *status);
int gui_reveal_send(const char *project, const char *path, uint32_t line);
/* Separate command from passive reveal. Receipt means queued, not match count.
 * View: 0 sidebar, 1 source, 2 preview. No pointers, writes or OS focus change. */
#define GUI_SEARCH_SET 0x41544631u
#define GUI_SEARCH_ACCEPTED 0x41544632u
#define GUI_SEARCH_BUSY 0x41544633u
#define GUI_SEARCH_QUERY 1024
typedef struct gui_search {
    uint32_t version, view, match_case;
    char project[GUI_PATH], path[GUI_PATH], query[GUI_SEARCH_QUERY];
} gui_search;
uintptr_t gui_search_receive(const gui_workspace *workspace, gui_search *pending, int *ready,
    int busy, const void *data, size_t size);
int gui_search_send(const char *project, const char *path, const char *query, uint32_t view, int match_case);
/* Lines follow InputTextMultiline: LF/CRLF. Bare CR cannot be mapped safely. */
int gui_reveal_has_line(const char *text, uint32_t line);
/* Outside IPC callback. Ordinary local ancestry only; no application links.
 * macOS permits standard /tmp, /var and /etc system aliases.
 * Missing final file is left to source reader. Not an atomic filesystem sandbox. */
int gui_reveal_ordinary_path(const char *path);
#ifdef ATTENDANT_MODEL_TESTING
int gui_reveal_test_discovery_limits(void);
#endif
int gui_reveal_matches(const gui_reveal *hint, const gui_tab *tab);
#endif
