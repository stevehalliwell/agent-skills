/* Viewer-owned UI preferences only. No analytical/project state. */
#ifndef GD_SIDEBAR_H
#define GD_SIDEBAR_H
#include "gd.h"
typedef enum {
    GD_SIDEBAR_VIEW,GD_SIDEBAR_FIND,GD_SIDEBAR_FILTERS,GD_SIDEBAR_ANALYSIS,
    GD_SIDEBAR_BROWSE,GD_SIDEBAR_SELECTION,GD_SIDEBAR_SOURCES,GD_SIDEBAR_DIAGNOSTICS,GD_SIDEBAR_COUNT
} GD_SidebarSection;
typedef struct { unsigned open;char path[GD_MAX_PATH],message[192]; } GD_Sidebar;
extern const char *const gd_sidebar_labels[GD_SIDEBAR_COUNT];
void gd_sidebar_defaults(GD_Sidebar *sidebar);
void gd_sidebar_init(GD_Sidebar *sidebar);
/* Explicit path in sidebar permits private test fixtures, never source records. */
int gd_sidebar_load(GD_Sidebar *sidebar);
int gd_sidebar_save(GD_Sidebar *sidebar);
#endif
