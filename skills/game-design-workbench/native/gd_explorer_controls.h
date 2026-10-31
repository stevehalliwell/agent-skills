#ifndef GD_EXPLORER_CONTROLS_H
#define GD_EXPLORER_CONTROLS_H
#include "gd_explorer_ui.h"
#include <stddef.h>

typedef enum { GD_FORM_QUERY, GD_FORM_STRUCTURE, GD_FORM_RULES, GD_FORM_COUNT } GD_ExplorerForm;
typedef enum { GD_CONTROL_INT, GD_CONTROL_BOOL, GD_CONTROL_TEXT } GD_ExplorerControlType;
/* Internal storage description, not permission to write native indices over IPC.
   Selector resolution and domain validation remain application operations. */
typedef struct {
    const char *name,*label,*choices;
    GD_ExplorerControlType type;
    size_t offset,capacity;
    float height;
} GD_ExplorerControl;
const GD_ExplorerControl *gd_explorer_form_fields(GD_ExplorerForm form, int *count);
const GD_ExplorerControl *gd_explorer_form_field(GD_ExplorerForm form, const char *name);
size_t gd_explorer_form_size(GD_ExplorerForm form);
int gd_explorer_rank_pending(const GD_ExplorerProject *project, const GD_ExplorerPane *panes);
void gd_explorer_form_defaults(GD_ExplorerForm form, void *values);
/* Read existing pending form; create only for explicit GUI/control requests. */
void *gd_explorer_form_pending(GD_ExplorerContext *context, GD_ExplorerForm form, int create);
int gd_explorer_form_equal(GD_ExplorerForm form, const void *a, const void *b);
/* Draw primitive editor from shared description. Native selector pickers stay domain-specific. */
int gd_explorer_form_widget(GD_ExplorerForm form, void *values, const char *name, const char *label);

/* Same dependent-input transitions for GUI and application-control callers. */
void gd_explorer_query_question(GD_ExplorerQueryOptions *options, int coverage);
int gd_explorer_query_root(GD_ExplorerQueryOptions *options, const GD_Rules *rules, int root);
int gd_explorer_query_participant(GD_ExplorerQueryOptions *options, const GD_Rules *rules, int participant);
int gd_explorer_query_axis(GD_ExplorerQueryOptions *options, const GD_Model *model, int axis);
int gd_explorer_structure_question(GD_ExplorerStructureForm *form, int question);
extern const char gd_explorer_rule_modes[];
int gd_explorer_rule_mode(GD_ExplorerRuleForm *form, int mode);

/* Application actions, shared by buttons and control callers; no transport or OS input. */
typedef enum { GD_ACTION_QUERY_RUN, GD_ACTION_QUERY_DISPLAY, GD_ACTION_STRUCTURE_RUN,
    GD_ACTION_STRUCTURE_CLEAR, GD_ACTION_RULES_RUN, GD_ACTION_RULES_CLEAR, GD_ACTION_COUNT } GD_ExplorerAction;
typedef struct { const char *name,*label; unsigned views; } GD_ExplorerActionInfo;
const GD_ExplorerActionInfo *gd_explorer_action_info(GD_ExplorerAction action);
int gd_explorer_action_available(const GD_ExplorerContext *context, GD_ExplorerAction action);
int gd_explorer_action_run(GD_ExplorerContext *context, GD_ExplorerAction action);
/* Returns button press, including a failed action. Result/message stays in application state. */
int gd_explorer_action_button(GD_ExplorerContext *context, GD_ExplorerAction action);
#endif
