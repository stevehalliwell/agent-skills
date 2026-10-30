#ifndef ATTENDANT_WORKSPACE_H
#define ATTENDANT_WORKSPACE_H
#include "../../src/native-api.h"
#include <windows.h>
#ifndef WC_ERR_INVALID_CHARS
#define WC_ERR_INVALID_CHARS 0x80
#endif
#define GUI_TABS 16
#define GUI_TEXT (4 * 1024 * 1024)
#define GUI_PATH 16384
#define GUI_DOCUMENTS 4096
#define GUI_RECOVERY_PATHS 5 /* Maximum managed bootstrap publication set. */

typedef struct gui_tab {
    char path[GUI_PATH], hash[65];
    char *text;
    size_t length;
    int used, dirty, external_change, generation;
    uint64_t text_revision;
    int selection_start, selection_end; /* Editor UTF-8 byte offsets. */
    int reveal_selection, anchor_stale;
    FILETIME modified;
    DWORD file_size;
} gui_tab;
typedef struct gui_navigation {
    int count, diagnostics;
    attendant_document_info documents[GUI_DOCUMENTS];
    void *memory;
} gui_navigation;
typedef enum gui_selection_view { GUI_SELECTION_NO_VIEW, GUI_SELECTION_SOURCE, GUI_SELECTION_PREVIEW } gui_selection_view;
typedef enum gui_selection_status { GUI_SELECTION_NONE, GUI_SELECTION_EXACT, GUI_SELECTION_UNSUPPORTED, GUI_SELECTION_MAPPED, GUI_SELECTION_CONTEXT } gui_selection_status;
typedef struct gui_selection {
    gui_selection_view view;
    gui_selection_status status;
    int tab, generation, start, end; /* Full-buffer UTF-8 bytes; valid for EXACT/MAPPED. */
    uint64_t revision;
} gui_selection;
typedef struct gui_workspace {
    attendant_session *session;
    char root[GUI_PATH], error[8192], notice[8192];
    char failed_path[GUI_PATH], surviving_paths[GUI_RECOVERY_PATHS][GUI_PATH];
    int surviving_count, surviving_total; /* Owned last failure evidence; refresh never invalidates it. */
    gui_navigation *navigation;
    gui_tab tabs[GUI_TABS];
    gui_selection selection; /* Latest human document selection; never agent marker. */
    char *preview_text, *preview_context; /* One allocation, owned by preview_text. */
    size_t preview_text_length, preview_context_length;
    int active, tab_count, activation;
    uint64_t text_revision; /* Workspace-wide: tab reuse/reload/edit cannot reuse preview identity. */
    /* activation changes only after explicit human navigation, never refresh. */
} gui_workspace;
/* Pure path resolution, no source/session operation. root NULL: process directory. */
int gui_full_path(const char *root, const char *path, char output[GUI_PATH]);
int gui_workspace_init(gui_workspace *workspace, const char *root);
void gui_workspace_destroy(gui_workspace *workspace);
int gui_refresh(gui_workspace *workspace);
int gui_open_document(gui_workspace *workspace, const char *path);
int gui_save_document(gui_workspace *workspace, int index);
int gui_reload_document(gui_workspace *workspace, int index, int discard_dirty);
int gui_close_document(gui_workspace *workspace, int index, int discard_dirty);
void gui_check_external(gui_workspace *workspace, int index);
int gui_set_document_text(gui_workspace *workspace, int index, const char *text);
void gui_selection_clear(gui_workspace *workspace);
void gui_selection_capture(gui_workspace *workspace, int index, gui_selection_view view, gui_selection_status status, int start, int end);
gui_selection gui_selection_current(const gui_workspace *workspace);
int gui_selection_capture_preview(gui_workspace *workspace, int index, int start, int end, const char *text, size_t text_length, const char *context, size_t context_length);
int gui_send_comment(gui_workspace *workspace, int index, int start, int end, const char *comment);
int gui_send_preview_comment(gui_workspace *workspace, int index, int start, int end, const char *context, const char *comment);
int gui_open_link(gui_workspace *workspace, const char *url);
int gui_open_comment_target(gui_workspace *workspace, int comment_index);
size_t gui_body_offset(const char *text);
#endif
