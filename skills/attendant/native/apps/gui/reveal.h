#ifndef ATTENDANT_GUI_REVEAL_H
#define ATTENDANT_GUI_REVEAL_H
#include "workspace.h"
#define GUI_WINDOW_CLASS "AttendantMarkdownWorkspace"
#define GUI_REVEAL_QUERY 0x41545131u
#define GUI_REVEAL_SET 0x41545231u
#define GUI_REVEAL_MATCH 0x41544d31u
#define GUI_REVEAL_ALLOWED 0x41545031u
#define GUI_REVEAL_ACCEPTED 0x41544131u
/* Local desktop IPC v1. No pointers, source bytes, edit authority or persistence.
 * Same fixed-width layout for CJIT/Zig. Latest hint wins; receiver owns copies. */
typedef struct gui_reveal {
    uint32_t version, line;
    char project[GUI_PATH], path[GUI_PATH], hash[65];
} gui_reveal;
/* Callback only validates/copies. Never enters source session or changes view. */
uintptr_t gui_reveal_receive(const gui_workspace *workspace, gui_reveal *pending,
    int *ready, uintptr_t operation, const void *data, size_t size);
int gui_reveal_error(int status, const char *code, const char *message);
HWND gui_find_workspace(const char *project, int *status);
int gui_reveal_send(const char *project, const char *path, uint32_t line);
/* Lines follow InputTextMultiline: LF/CRLF. Bare CR cannot be mapped safely. */
int gui_reveal_has_line(const char *text, uint32_t line);
/* Outside message callback. Ordinary local drive ancestry only; no reparse paths.
 * Missing final file is left to source reader. Not an atomic filesystem sandbox. */
int gui_reveal_ordinary_path(const char *path);
#ifdef ATTENDANT_MODEL_TESTING
int gui_reveal_test_discovery_limits(void);
#endif
int gui_reveal_matches(const gui_reveal *hint, const gui_tab *tab);
#endif
