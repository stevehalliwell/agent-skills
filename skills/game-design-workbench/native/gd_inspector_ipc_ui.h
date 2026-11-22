/* Inspector-only frame survey. Include after cimgui declarations. */
#ifndef GD_INSPECTOR_IPC_UI_H
#define GD_INSPECTOR_IPC_UI_H
#include <stdint.h>
#include "gd_viewer.h"
enum { GD_INSPECTOR_ITEMS=2048,GD_INSPECTOR_INDEX=4096,GD_INSPECTOR_LABEL=128,GD_INSPECTOR_MAX_SCROLL=10000000 };
typedef enum { GD_INSPECTOR_ROOT,GD_INSPECTOR_FOLD,GD_INSPECTOR_CHILD,GD_INSPECTOR_PAGE } GD_InspectorItemKind;
typedef struct {
    ImGuiID window,widget;
    int kind,section,depth,value,maximum,ambiguous;
    char label[GD_INSPECTOR_LABEL];
} GD_InspectorItem;
typedef struct { ImGuiID window,widget; int section,value,maximum,seen; char label[GD_INSPECTOR_LABEL]; } GD_InspectorPageChange;
typedef struct {
    int projection,count,limited,section,ready,page_count;
    GD_InspectorPageChange page_changes[GD_INSPECTOR_PATCH_ITEMS];
    uint64_t revision;
    char scope[64];
    GD_InspectorItem items[GD_INSPECTOR_ITEMS];
    int index[GD_INSPECTOR_INDEX]; /* Item index + 1, zero is empty. */
} GD_InspectorSurvey;
void gd_inspector_survey_begin(GD_InspectorSurvey *survey,int projection);
void gd_inspector_survey_root(int section,const char *label,int open);
void gd_inspector_survey_end(void);
int gd_inspector_pending_page(const char *label,int page,int maximum);
void gd_inspector_record_page(const char *label,int page,int maximum);
bool gd_inspector_tree(const char *label);
bool gd_inspector_tree_ex(const char *label,ImGuiTreeNodeFlags flags);
bool gd_inspector_header(const char *label,ImGuiTreeNodeFlags flags);
bool gd_inspector_child(const char *id,ImVec2_c size,ImGuiChildFlags child_flags,ImGuiWindowFlags flags);
void gd_inspector_end_child(void);
#ifndef GD_INSPECTOR_NO_HOOKS
#define igTreeNode_Str gd_inspector_tree
#define igTreeNodeEx_Str gd_inspector_tree_ex
#define igCollapsingHeader_TreeNodeFlags(...) gd_inspector_header(__VA_ARGS__)
#define igBeginChild_Str gd_inspector_child
#define igEndChild gd_inspector_end_child
#endif
#endif
