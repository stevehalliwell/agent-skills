#ifndef ATTENDANT_GUI_INSPECT_H
#define ATTENDANT_GUI_INSPECT_H
#include "reveal.h"
#define GUI_INSPECT_REQUEST 0x41544931u
#define GUI_INSPECT_RESULT 0x41544932u
#define GUI_INSPECT_BUSY 0x41544933u
#define GUI_INSPECT_CLIENT_CLASS "AttendantWorkspaceInspector"
/* Same-desktop read-only inspection, not authentication/edit authority.
 * Header and selected UTF-8 bytes only. No pointers. Snapshot uses live buffer. */
typedef struct gui_inspection_header {
    uint32_t version, has_document, dirty, view, status, start, end, length, context_length;
    uint64_t revision;
    char project[GUI_PATH], path[GUI_PATH];
} gui_inspection_header;
typedef struct gui_inspection {
    gui_inspection_header header;
    char text[2 * GUI_TEXT + 2]; /* Selected text NUL, then rendered context NUL. */
} gui_inspection;
int gui_inspection_snapshot(const gui_workspace *workspace, gui_inspection *output);
int gui_inspection_json(const gui_inspection *snapshot, char *output, size_t capacity);
int gui_inspection_accept(gui_inspection *output, const char *project, const void *data, size_t size);
uintptr_t gui_inspection_receive(HWND window, HWND client, const gui_workspace *workspace,
    int busy, gui_inspection *output, const void *data, size_t size);
int gui_inspect(const char *project);
#endif
