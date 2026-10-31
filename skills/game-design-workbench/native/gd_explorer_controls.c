/* Shared form descriptions, input transitions and application actions. */
#include "gd_explorer_controls.h"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_NO_EXPORT
#include "cimgui.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

#define FIELD(type,name,label,kind,choices,height) {#name,label,choices,kind,offsetof(type,name),sizeof(((type *)0)->name),height}
#define Q(name,label,kind,choices,height) FIELD(GD_ExplorerQueryOptions,name,label,kind,choices,height)
static const GD_ExplorerControl query_fields[]={
    Q(root_case,"Root case",GD_CONTROL_INT,NULL,0),
    Q(participant,"Local participant",GD_CONTROL_INT,NULL,0),
    Q(axis,"Typed axis",GD_CONTROL_INT,NULL,0),
    Q(noun,"Object filter",GD_CONTROL_INT,NULL,0),
    Q(ghosts,"Variant display",GD_CONTROL_INT,"All variants\0Primary only\0Stacked ghosts\0",0),
    Q(primary,"Primary state",GD_CONTROL_INT,NULL,0),
    Q(compare,"Compare state",GD_CONTROL_INT,NULL,0),
    Q(coverage,"Finite state coverage across bindings",GD_CONTROL_BOOL,NULL,0),
    Q(accepted,"Accepted sources",GD_CONTROL_BOOL,NULL,0),
    Q(weighted,"Incoming-weighted convergence cards (not frequency)",GD_CONTROL_BOOL,NULL,0),
    Q(include_excluded,"Show checked excluded cases",GD_CONTROL_BOOL,NULL,0),
    Q(literal_values,"Exact newline literals (otherwise ; shorthand)",GD_CONTROL_BOOL,NULL,0),
    Q(literal_assumptions,"Literal assumptions",GD_CONTROL_BOOL,NULL,0),
    Q(values,"Finite values",GD_CONTROL_TEXT,NULL,55),
    Q(facts,"Fixed state assumptions",GD_CONTROL_TEXT,NULL,55),
    Q(contexts,"Fixed context assumptions",GD_CONTROL_TEXT,NULL,55)
};
#undef Q
#define S(name,label,kind,choices,height) FIELD(GD_ExplorerStructureForm,name,label,kind,choices,height)
static const GD_ExplorerControl structure_fields[]={
    S(kind,"Question",GD_CONTROL_INT,NULL,0),
    S(direction,"Direction",GD_CONTROL_INT,"forward\0backward\0both\0",0),
    S(lens,"Partner lens",GD_CONTROL_INT,"any\0same-role\0changed-role\0",0),
    S(work_limit,"Search work ceiling",GD_CONTROL_INT,NULL,0),
    S(actor_scope,"Analysis scope##structure",GD_CONTROL_INT,"Declaration inventory\0State-specific (applied variants)\0",0),
    S(include_inactive,"Include inactive structural rows",GD_CONTROL_BOOL,NULL,0),
    S(from,"From identity",GD_CONTROL_TEXT,NULL,0),
    S(to,"To identity",GD_CONTROL_TEXT,NULL,0),
    S(avoid,"Avoid identity",GD_CONTROL_TEXT,NULL,0),
    S(scope_root,"Scope root",GD_CONTROL_TEXT,NULL,0),
    S(starts,"Starts",GD_CONTROL_TEXT,NULL,55),
    S(relations,"Relation names",GD_CONTROL_TEXT,NULL,55),
    S(statuses,"Statuses",GD_CONTROL_TEXT,NULL,45),
    S(features,"Features ID[=WEIGHT]",GD_CONTROL_TEXT,NULL,75),
    S(entries,"Exact entry IDs",GD_CONTROL_TEXT,NULL,65)
};
#undef S
#define R(name,label,kind,choices,height) FIELD(GD_ExplorerRuleForm,name,label,kind,choices,height)
static const GD_ExplorerControl rule_fields[]={
    R(kind,"Native question",GD_CONTROL_INT,NULL,0),
    R(work_limit,"Work ceiling",GD_CONTROL_INT,NULL,0),
    R(bindings,"Object bindings",GD_CONTROL_BOOL,NULL,0),
    R(case_name,"Exact source#case",GD_CONTROL_TEXT,NULL,0),
    R(facts,"Fixed state assumptions",GD_CONTROL_TEXT,NULL,55),
    R(contexts,"Fixed context assumptions",GD_CONTROL_TEXT,NULL,55),
    R(object,"Object reference",GD_CONTROL_TEXT,NULL,0),
    R(participant,"Local participant name",GD_CONTROL_TEXT,NULL,0),
    R(axis,"Axis reference",GD_CONTROL_TEXT,NULL,0),
    R(statuses,"Root statuses",GD_CONTROL_TEXT,NULL,55)
};
#undef R
#undef FIELD
int gd_explorer_rank_pending(const GD_ExplorerProject *project,const GD_ExplorerPane *panes) {
    GD_Model *model=gd_explorer_model(project,GD_EXPLORER_SKILLS); char expected[64]; int v;
    if (!model) return 0;
    for (v=0;v<=GD_EXPLORER_RELATIONSHIPS;++v) {
        const GD_ExplorerSkillEditor *editor=&panes[v].editor;
        if (editor->record<0) continue;
        if (editor->record>=model->record_count || model->records[editor->record].role!=GD_SKILL) return 1;
        if (model->records[editor->record].has_rank) snprintf(expected,sizeof(expected),"%lld",(long long)model->records[editor->record].rank);
        else snprintf(expected,sizeof(expected),"null");
        if (strcmp(expected,editor->rank_text)) return 1;
    }
    return 0;
}
const GD_ExplorerControl *gd_explorer_form_fields(GD_ExplorerForm form,int *count) {
    assert(count); *count=0;
    switch (form) {
        case GD_FORM_QUERY: *count=(int)(sizeof(query_fields)/sizeof(*query_fields)); return query_fields;
        case GD_FORM_STRUCTURE: *count=(int)(sizeof(structure_fields)/sizeof(*structure_fields)); return structure_fields;
        case GD_FORM_RULES: *count=(int)(sizeof(rule_fields)/sizeof(*rule_fields)); return rule_fields;
        default: return NULL;
    }
}
const GD_ExplorerControl *gd_explorer_form_field(GD_ExplorerForm form,const char *name) {
    int i,count; const GD_ExplorerControl *fields=gd_explorer_form_fields(form,&count);
    if (!name) return NULL;
    for (i=0;i<count;++i) if (!strcmp(name,fields[i].name)) return &fields[i];
    return NULL;
}
size_t gd_explorer_form_size(GD_ExplorerForm form) {
    switch (form) {
        case GD_FORM_QUERY: return sizeof(GD_ExplorerQueryOptions);
        case GD_FORM_STRUCTURE: return sizeof(GD_ExplorerStructureForm);
        case GD_FORM_RULES: return sizeof(GD_ExplorerRuleForm);
        default: return 0;
    }
}
void gd_explorer_form_defaults(GD_ExplorerForm form,void *values) {
    size_t size=gd_explorer_form_size(form); assert(size && values); memset(values,0,size);
    if (form==GD_FORM_QUERY) {
        GD_ExplorerQueryOptions *options=values;
        options->root_case=options->participant=options->axis=options->noun=options->compare=-1; options->weighted=true;
    } else if (form==GD_FORM_STRUCTURE) ((GD_ExplorerStructureForm *)values)->work_limit=GD_GRAPH_ANALYSIS_WORK;
    else ((GD_ExplorerRuleForm *)values)->work_limit=GD_RULE_EVALUATION_WORK_LIMIT;
}
void *gd_explorer_form_pending(GD_ExplorerContext *context,GD_ExplorerForm form,int create) {
    GD_ExplorerPane *pane;
    if (!context || !context->project || !(pane=context->pane) || !(context->project->available&(1u<<pane->view))) return NULL;
    switch (form) {
        case GD_FORM_QUERY: return pane->view==GD_EXPLORER_ACTORS || pane->view==GD_EXPLORER_EFFECTS ? &pane->query_options:NULL;
        case GD_FORM_STRUCTURE:
            if (create && !gd_explorer_structure_inputs(context)) return NULL;
            return pane->structure_state ? &pane->structure_state->pending:NULL;
        case GD_FORM_RULES:
            if (pane->view!=GD_EXPLORER_RESOURCES) return NULL;
            if (create && !gd_explorer_rules_inputs(context)) return NULL;
            return pane->rule_state ? &pane->rule_state->pending:NULL;
        default: return NULL;
    }
}
int gd_explorer_form_equal(GD_ExplorerForm form,const void *a,const void *b) {
    int i,count; const GD_ExplorerControl *fields=gd_explorer_form_fields(form,&count);
    assert(a && b); if (!fields) return 0;
    for (i=0;i<count;++i) {
        const GD_ExplorerControl *field=&fields[i]; const char *x=(const char *)a+field->offset,*y=(const char *)b+field->offset;
        /* Empty literal lists have no parser mode. Preserve existing query semantics. */
        if (form==GD_FORM_QUERY) {
            const GD_ExplorerQueryOptions *options=a;
            if (field->offset==offsetof(GD_ExplorerQueryOptions,literal_values) && !*options->values) continue;
            if (field->offset==offsetof(GD_ExplorerQueryOptions,literal_assumptions) && !*options->facts && !*options->contexts) continue;
        }
        if (field->type==GD_CONTROL_BOOL ? *(const bool *)x!=*(const bool *)y:
            field->type==GD_CONTROL_INT ? *(const int *)x!=*(const int *)y:strcmp(x,y)!=0) return 0;
    }
    return 1;
}
int gd_explorer_form_widget(GD_ExplorerForm form,void *values,const char *name,const char *label) {
    const GD_ExplorerControl *field=gd_explorer_form_field(form,name); char *value;
    assert(field && values); if (!field || !values) return 0;
    value=(char *)values+field->offset; if (!label) label=field->label;
    if (field->type==GD_CONTROL_BOOL) return igCheckbox(label,(bool *)value);
    if (field->type==GD_CONTROL_INT) return field->choices ? igCombo_Str(label,(int *)value,field->choices,-1):igInputInt(label,(int *)value,0,0,0);
    return field->height ? igInputTextMultiline(label,value,field->capacity,(ImVec2_c){0,field->height},0,NULL,NULL):igInputText(label,value,field->capacity,0,NULL,NULL);
}
void gd_explorer_query_question(GD_ExplorerQueryOptions *options,int coverage) {
    options->coverage=coverage!=0; options->root_case=options->participant=options->axis=options->noun=options->compare=-1;
    options->ghosts=GD_QUERY_ALL_VARIANTS; options->primary=0; *options->values=*options->facts=*options->contexts=0;
}
int gd_explorer_query_root(GD_ExplorerQueryOptions *options,const GD_Rules *rules,int root) {
    if (!rules || root<-1 || root>=rules->case_count) return 0;
    options->root_case=root; options->participant=root<0 || options->coverage ? -1:rules->cases[root].primary_participant;
    if (root<0) return 1;
    if (!options->coverage) options->axis=-1;
    options->primary=0; options->compare=-1; *options->values=*options->facts=*options->contexts=0; return 1;
}
int gd_explorer_query_participant(GD_ExplorerQueryOptions *options,const GD_Rules *rules,int participant) {
    const GD_RuleCase *root;
    if (!rules || options->root_case<0 || options->root_case>=rules->case_count) return 0;
    root=&rules->cases[options->root_case];
    if (participant<-1 || (participant<0 && !options->coverage) || (participant>=0 && (participant<root->first_participant || participant>=root->first_participant+root->participant_count))) return 0;
    options->participant=participant; if (!options->coverage) options->axis=-1; return 1;
}
int gd_explorer_query_axis(GD_ExplorerQueryOptions *options,const GD_Model *model,int axis) {
    if (!model || axis<-1 || axis>=model->record_count || (axis>=0 && model->records[axis].role!=GD_AXIS)) return 0;
    options->axis=axis; options->primary=0; options->compare=-1; *options->values=0; return 1;
}
int gd_explorer_structure_question(GD_ExplorerStructureForm *form,int question) {
    if (question<0 || question>=GD_STRUCTURE_QUESTIONS) return 0;
    if (question==GD_STRUCTURE_DECLARATIONS) { int scope=form->actor_scope; gd_explorer_form_defaults(GD_FORM_STRUCTURE,form); form->actor_scope=scope; }
    if (question>=GD_STRUCTURE_COUNT) {
        form->direction=GD_STRUCTURE_FORWARD; form->lens=GD_GRAPH_ANY; *form->starts=*form->avoid=0;
        if (question==GD_STRUCTURE_COUNT) *form->from=*form->to=*form->features=*form->entries=0;
        if (form->work_limit==GD_GRAPH_ANALYSIS_WORK) form->work_limit=GD_SIMILARITY_WORK;
    } else *form->features=*form->entries=0;
    form->kind=question; return 1;
}
const char gd_explorer_rule_modes[]="Conditional outcomes\0Conditional consistency\0Object bindings\0";
int gd_explorer_rule_mode(GD_ExplorerRuleForm *form,int mode) {
    if (mode<0 || mode>2) return 0;
    gd_explorer_form_defaults(GD_FORM_RULES,form); form->bindings=mode==2; form->kind=mode==1 ? GD_NAMED_CONSISTENCY:GD_NAMED_OUTCOMES;
    form->work_limit=mode==2 ? GD_EFFECT_WORK:mode==1 ? -1:GD_RULE_EVALUATION_WORK_LIMIT; return 1;
}
static const GD_ExplorerActionInfo actions[]={
    {"query.run","Apply native query",(1u<<GD_EXPLORER_ACTORS)|(1u<<GD_EXPLORER_EFFECTS)},
    {"query.display","Rearrange query drawing",(1u<<GD_EXPLORER_ACTORS)|(1u<<GD_EXPLORER_EFFECTS)},
    {"structure.run","Run named question",(1u<<GD_EXPLORER_VIEWS)-1},
    {"structure.clear","Clear question",(1u<<GD_EXPLORER_VIEWS)-1},
    {"rules.run","Run native reader question",1u<<GD_EXPLORER_RESOURCES},
    {"rules.clear","Clear rule/binding inspection",1u<<GD_EXPLORER_RESOURCES}
};
const GD_ExplorerActionInfo *gd_explorer_action_info(GD_ExplorerAction action) {
    return action>=0 && action<GD_ACTION_COUNT ? &actions[action]:NULL;
}
int gd_explorer_action_available(const GD_ExplorerContext *context,GD_ExplorerAction action) {
    const GD_ExplorerActionInfo *info=gd_explorer_action_info(action);
    if (!info || !context || !context->project || !context->pane || !(info->views&(1u<<context->pane->view)) || !(context->project->available&(1u<<context->pane->view))) return 0;
    if (action==GD_ACTION_QUERY_DISPLAY) return context->pane->query_state && context->pane->query_state->query;
    return 1;
}
int gd_explorer_action_run(GD_ExplorerContext *context,GD_ExplorerAction action) {
    if (!gd_explorer_action_available(context,action)) return 0;
    switch (action) {
        case GD_ACTION_QUERY_RUN: return gd_explorer_query_run(context);
        case GD_ACTION_QUERY_DISPLAY: return gd_explorer_query_display(context);
        case GD_ACTION_STRUCTURE_RUN: return gd_explorer_structure_run(context);
        case GD_ACTION_STRUCTURE_CLEAR: return gd_explorer_structure_clear(context);
        case GD_ACTION_RULES_RUN: return gd_explorer_form_pending(context,GD_FORM_RULES,1) && gd_explorer_rules_run(context);
        case GD_ACTION_RULES_CLEAR: gd_explorer_rules_destroy(context->pane->rule_state); context->pane->rule_state=NULL; *context->pane->rule_message=0; return 1;
        default: return 0;
    }
}
int gd_explorer_action_button(GD_ExplorerContext *context,GD_ExplorerAction action) {
    const GD_ExplorerActionInfo *info=gd_explorer_action_info(action); assert(info);
    if (!igButton(info->label,(ImVec2_c){0,0})) return 0;
    gd_explorer_action_run(context,action); return 1;
}
