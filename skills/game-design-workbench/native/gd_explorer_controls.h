#ifndef GD_EXPLORER_CONTROLS_H
#define GD_EXPLORER_CONTROLS_H
#include "gd_explorer_ui.h"
#include <stddef.h>

typedef enum { GD_FORM_QUERY, GD_FORM_STRUCTURE, GD_FORM_RULES, GD_FORM_RESOURCE, GD_FORM_PRESENTATION, GD_FORM_COUNT } GD_ExplorerForm;
typedef enum { GD_CONTROL_INT, GD_CONTROL_BOOL, GD_CONTROL_TEXT } GD_ExplorerControlType;
/* Internal storage description, not permission to write native indices over IPC.
   Selector resolution and domain validation remain application operations. */
typedef struct {
    const char *name,*label,*choices;
    GD_ExplorerControlType type;
    size_t offset,capacity;
    float height;
    const char *keys; /* Stable CLI keys when primitive combo labels differ. */
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

const char *gd_explorer_form_name(GD_ExplorerForm form);
int gd_explorer_form_available(const GD_ExplorerContext *context, GD_ExplorerForm form);
const void *gd_explorer_form_applied(const GD_ExplorerContext *context, GD_ExplorerForm form);
int gd_explorer_form_work_limit(GD_ExplorerForm form, const void *values);
/* Already-built canonical picker/result scope only; never allocates or evaluates. */
const GD_StructureResult *gd_explorer_structure_choices(const GD_ExplorerContext *context);
int gd_explorer_control_editable(const GD_ExplorerContext *context, GD_ExplorerForm form, const GD_ExplorerControl *field, const void *values);
/* -1 means no enumerated choices. Text choices are suggestions for native
   literals/selectors; scalar choices are closed. Page source slots, not accepted
   rows. Option returns 1 key / 0 filtered slot / -1 identity capacity failure.
   Reads never create inputs, prepare or evaluate. */
int gd_explorer_control_option_count(const GD_ExplorerContext *context, GD_ExplorerForm form, const GD_ExplorerControl *field, const void *values);
int gd_explorer_control_option(const GD_ExplorerContext *context, GD_ExplorerForm form, const GD_ExplorerControl *field, const void *values, int ordinal, int *native, char *key, size_t capacity);
int gd_explorer_control_value(const GD_ExplorerContext *context, GD_ExplorerForm form, const GD_ExplorerControl *field, const void *values, char *value, size_t capacity);
/* Stage one input with primitive/key/capacity validation and normal dependent
   resets, atomically. Native Run still validates literal lists/raw text selectors.
   No Run, display application, source/draft mutation or input ownership guard. */
int gd_explorer_control_set(GD_ExplorerContext *context, GD_ExplorerForm form, const char *name, const char *value, char *message, size_t capacity);
/* Presentation is immediate, matching GUI; snapshots never own application state. */
const GD_ExplorerControl *gd_explorer_presentation_fields(int *count);
void gd_explorer_presentation_read(const GD_ExplorerContext *context,GD_ExplorerPresentation *values);
int gd_explorer_presentation_maximum(const GD_ExplorerContext *context,const char *name);
int gd_explorer_presentation_editable(const GD_ExplorerContext *context,const char *name);
int gd_explorer_presentation_options(const GD_ExplorerContext *context,const GD_ExplorerControl *field);
int gd_explorer_presentation_option(const GD_ExplorerContext *context,const GD_ExplorerControl *field,int ordinal,int *native,char *key,size_t capacity);
int gd_explorer_presentation_value(const GD_ExplorerContext *context,const GD_ExplorerControl *field,const GD_ExplorerPresentation *values,char *out,size_t capacity);
int gd_explorer_presentation_set(GD_ExplorerContext *context,const char *name,const char *value,char *message,size_t capacity);
int gd_explorer_presentation_action(GD_ExplorerContext *context,const char *name);
void gd_explorer_rule_case_transition(GD_ExplorerRuleForm *form, int before, int after);

/* Same dependent-input transitions for GUI and application-control callers. */
void gd_explorer_query_question(GD_ExplorerQueryOptions *options, int coverage);
void gd_explorer_query_inventory(GD_ExplorerQueryOptions *options);
int gd_explorer_query_root(GD_ExplorerQueryOptions *options, const GD_Rules *rules, int root);
int gd_explorer_query_participant(GD_ExplorerQueryOptions *options, const GD_Rules *rules, int participant);
int gd_explorer_query_axis_allowed(const GD_ExplorerQueryOptions *options, const GD_Rules *rules, int axis);
int gd_explorer_query_axis(GD_ExplorerQueryOptions *options, const GD_Rules *rules, int axis);
int gd_explorer_structure_question(GD_ExplorerStructureForm *form, int question);
extern const char gd_explorer_rule_modes[];
int gd_explorer_rule_mode(GD_ExplorerRuleForm *form, int mode);

/* Application actions, shared by buttons and control callers; no transport or OS input. */
typedef enum { GD_ACTION_QUERY_RUN, GD_ACTION_QUERY_DISPLAY, GD_ACTION_STRUCTURE_RUN,
    GD_ACTION_STRUCTURE_CLEAR, GD_ACTION_RULES_RUN, GD_ACTION_RULES_CLEAR, GD_ACTION_RESOURCE_RUN, GD_ACTION_RESOURCE_CLEAR, GD_ACTION_VIEW_FIT, GD_ACTION_VIEW_REARRANGE, GD_ACTION_VIEW_EXPAND,
    GD_ACTION_VIEW_START_ADD, GD_ACTION_VIEW_START_CLEAR, GD_ACTION_VIEW_ANALYZE, GD_ACTION_VIEW_COMPARE, GD_ACTION_VIEW_RESET, GD_ACTION_VIEW_FIND_CLEAR, GD_ACTION_VIEW_WITNESS_FIT, GD_ACTION_VIEW_CELL_FIT, GD_ACTION_VIEW_SELECTION_FOCUS, GD_ACTION_INPUTS_RESET, GD_ACTION_BACK, GD_ACTION_FORWARD, GD_ACTION_UNDO, GD_ACTION_REDO, GD_ACTION_DISCARD_REDO, GD_ACTION_RESET_ALL, GD_ACTION_SELECTION_CLEAR, GD_ACTION_RULES_BINDINGS_OPEN, GD_ACTION_RULES_OUTCOMES_OPEN, GD_ACTION_RULES_INPUTS_RESET, GD_ACTION_COUNT } GD_ExplorerAction;
typedef struct { const char *name,*label; unsigned views; GD_ExplorerForm form; } GD_ExplorerActionInfo;
const GD_ExplorerActionInfo *gd_explorer_action_info(GD_ExplorerAction action);
int gd_explorer_action_available(const GD_ExplorerContext *context, GD_ExplorerAction action);
const char *gd_explorer_action_reason(const GD_ExplorerContext *context,GD_ExplorerAction action);
int gd_explorer_action_run(GD_ExplorerContext *context, GD_ExplorerAction action);
/* Returns button press, including a failed action. Result/message stays in application state. */
int gd_explorer_action_request(GD_ExplorerContext *context,GD_ExplorerAction action);
int gd_explorer_action_button(GD_ExplorerContext *context, GD_ExplorerAction action);
#endif
