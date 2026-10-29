#ifndef ATTENDANT_WORKSPACE_H
#define ATTENDANT_WORKSPACE_H
#include "../native-api.h"
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
typedef struct gui_workspace {
    attendant_session *session;
    char root[GUI_PATH], error[8192], notice[8192];
    char failed_path[GUI_PATH], surviving_paths[GUI_RECOVERY_PATHS][GUI_PATH];
    int surviving_count, surviving_total; /* Owned last failure evidence; refresh never invalidates it. */
    gui_navigation *navigation;
    gui_tab tabs[GUI_TABS];
    int active, tab_count, activation;
    /* activation changes only after explicit human navigation, never refresh. */
} gui_workspace;
int gui_workspace_init(gui_workspace *workspace, const char *root);
void gui_workspace_destroy(gui_workspace *workspace);
int gui_refresh(gui_workspace *workspace);
int gui_open_document(gui_workspace *workspace, const char *path);
int gui_save_document(gui_workspace *workspace, int index);
int gui_reload_document(gui_workspace *workspace, int index, int discard_dirty);
int gui_close_document(gui_workspace *workspace, int index, int discard_dirty);
void gui_check_external(gui_workspace *workspace, int index);
int gui_set_document_text(gui_workspace *workspace, int index, const char *text);
int gui_send_comment(gui_workspace *workspace, int index, const char *comment);
int gui_open_link(gui_workspace *workspace, const char *url);
int gui_open_comment_target(gui_workspace *workspace, int comment_index);
size_t gui_body_offset(const char *text);
#endif
