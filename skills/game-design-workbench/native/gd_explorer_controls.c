/* Shared form descriptions, input transitions and application actions. */
#include "gd_explorer_controls.h"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_NO_EXPORT
#include "cimgui.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#define KEYFIELD(type,name,label,kind,choices,height,keys) {#name,label,choices,kind,offsetof(type,name),sizeof(((type *)0)->name),height,keys}
#define FIELD(type,name,label,kind,choices,height) KEYFIELD(type,name,label,kind,choices,height,NULL)
#define Q(name,label,kind,choices,height) FIELD(GD_ExplorerQueryOptions,name,label,kind,choices,height)
static const GD_ExplorerControl query_fields[]={
    Q(root_case,"Root case",GD_CONTROL_INT,NULL,0),
    Q(participant,"Local participant",GD_CONTROL_INT,NULL,0),
    Q(axis,"Typed axis",GD_CONTROL_INT,NULL,0),
    Q(noun,"Object filter",GD_CONTROL_INT,NULL,0),
    KEYFIELD(GD_ExplorerQueryOptions,ghosts,"Variant display",GD_CONTROL_INT,"All variants\0Primary only\0Stacked ghosts\0",0,"all\0primary\0stacked\0"),
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
    KEYFIELD(GD_ExplorerStructureForm,actor_scope,"Analysis scope##structure",GD_CONTROL_INT,"Declaration inventory\0State-specific (applied variants)\0",0,"declarations\0applied-variants\0"),
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
static const GD_ExplorerControl resource_fields[]={
    FIELD(GD_ExplorerResourceForm,from,"FROM (blank: any upstream)",GD_CONTROL_INT,NULL,0),
    FIELD(GD_ExplorerResourceForm,to,"TO",GD_CONTROL_INT,NULL,0),
    FIELD(GD_ExplorerResourceForm,accepted,"Accepted sources only",GD_CONTROL_BOOL,NULL,0),
    FIELD(GD_ExplorerResourceForm,work_limit,"Route work ceiling 0..16777216",GD_CONTROL_INT,NULL,0)
};
#undef FIELD
#undef KEYFIELD
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
        case GD_FORM_PRESENTATION: return gd_explorer_presentation_fields(count);
        case GD_FORM_RESOURCE: *count=(int)(sizeof(resource_fields)/sizeof(*resource_fields)); return resource_fields;
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
        case GD_FORM_RESOURCE: return sizeof(GD_ExplorerResourceForm);
        case GD_FORM_PRESENTATION: return sizeof(GD_ExplorerPresentation);
        default: return 0;
    }
}
void gd_explorer_form_defaults(GD_ExplorerForm form,void *values) {
    size_t size=gd_explorer_form_size(form); assert(size && values); memset(values,0,size);
    if (form==GD_FORM_QUERY) {
        GD_ExplorerQueryOptions *options=values;
        options->root_case=options->participant=options->axis=options->noun=options->compare=-1; options->weighted=true;
    } else if (form==GD_FORM_STRUCTURE) ((GD_ExplorerStructureForm *)values)->work_limit=GD_GRAPH_ANALYSIS_WORK;
    else if (form==GD_FORM_RESOURCE) { GD_ExplorerResourceForm *p=values; p->from=p->to=-1; p->work_limit=GD_RESOURCE_ROUTE_WORK; }
    else if (form==GD_FORM_RULES) ((GD_ExplorerRuleForm *)values)->work_limit=GD_RULE_EVALUATION_WORK_LIMIT;
}
void *gd_explorer_form_pending(GD_ExplorerContext *context,GD_ExplorerForm form,int create) {
    GD_ExplorerPane *pane;
    if (!context || !context->project || !(pane=context->pane) || !(context->project->available&(1u<<pane->view))) return NULL;
    switch (form) {
        case GD_FORM_QUERY: return pane->view==GD_EXPLORER_ACTORS || pane->view==GD_EXPLORER_EFFECTS ? &pane->query_options:NULL;
        case GD_FORM_STRUCTURE:
            if (create && !gd_explorer_structure_inputs(context)) return NULL;
            return pane->structure_state ? &pane->structure_state->pending:NULL;
        case GD_FORM_RESOURCE: return pane->view==GD_EXPLORER_RESOURCES ? &pane->resource_pending:NULL;
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
void gd_explorer_query_inventory(GD_ExplorerQueryOptions *options) {
    options->coverage=false; options->root_case=options->participant=options->axis=options->noun=options->compare=-1;
    *options->values=*options->facts=*options->contexts=0;
}
int gd_explorer_query_root(GD_ExplorerQueryOptions *options,const GD_Rules *rules,int root) {
    if (!rules || root<-1 || root>=rules->case_count) return 0;
    if (root<0 && !options->coverage) { gd_explorer_query_inventory(options); return 1; }
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
int gd_explorer_query_axis_allowed(const GD_ExplorerQueryOptions *options,const GD_Rules *rules,int axis) {
    int edge;
    if (!rules || axis<-1 || axis>=rules->model->record_count || (axis>=0 && rules->model->records[axis].role!=GD_AXIS)) return 0;
    if (axis<0 || options->coverage) return 1;
    if (options->participant<0 || options->participant>=rules->participant_count || rules->participants[options->participant].noun_resolution!=GD_RULE_RESOLVED) return 0;
    for (edge=rules->actor_first[rules->participants[options->participant].noun_record];edge>=0;edge=rules->next_axis[edge])
        if (rules->axis_resolution[edge]==GD_RULE_RESOLVED && rules->model->edges[edge].target==axis) return 1;
    return 0;
}
int gd_explorer_query_axis(GD_ExplorerQueryOptions *options,const GD_Rules *rules,int axis) {
    if (!gd_explorer_query_axis_allowed(options,rules,axis)) return 0;
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
const char *gd_explorer_form_name(GD_ExplorerForm form) {
    static const char *const names[]={"query","structure","rules","resource","presentation"};
    return form>=0 && form<GD_FORM_COUNT ? names[form]:NULL;
}
int gd_explorer_form_available(const GD_ExplorerContext *c,GD_ExplorerForm form) {
    if (!c || !c->project || !c->pane || !(c->project->available&(1u<<c->pane->view))) return 0;
    return form==GD_FORM_PRESENTATION || (form==GD_FORM_STRUCTURE && c->pane->view!=GD_EXPLORER_NOUNS) || ((form==GD_FORM_RULES || form==GD_FORM_RESOURCE) && c->pane->view==GD_EXPLORER_RESOURCES) ||
        (form==GD_FORM_QUERY && (c->pane->view==GD_EXPLORER_ACTORS || c->pane->view==GD_EXPLORER_EFFECTS));
}
const void *gd_explorer_form_applied(const GD_ExplorerContext *c,GD_ExplorerForm form) {
    const GD_ExplorerPane *p=c->pane;
    if (form==GD_FORM_QUERY) return p->query_state ? &p->query_state->applied:NULL;
    if (form==GD_FORM_STRUCTURE) return p->structure_state && p->structure_state->has_applied ? &p->structure_state->applied:NULL;
    if (form==GD_FORM_RESOURCE) return p->resource_routes ? &p->resource_applied:NULL;
    if (form==GD_FORM_RULES) return p->rule_state && (p->rule_state->result || p->rule_state->bindings) ? &p->rule_state->applied:NULL;
    return NULL;
}
const GD_StructureResult *gd_explorer_structure_choices(const GD_ExplorerContext *c) {
    const GD_ExplorerStructureState *state=c->pane->structure_state; int scope;
    if (!state) return NULL; scope=state->pending.actor_scope;
    if (state->result && state->revision==c->project->revision && !state->result->options.scope_root && !state->result->gaps && (c->pane->view!=GD_EXPLORER_ACTORS || !!state->result->actor_scene==!!scope)) return state->result;
    if (state->picker_context && state->picker_revision==c->project->revision && state->picker_scope==scope) return state->picker_context;
    return NULL;
}
int gd_explorer_form_work_limit(GD_ExplorerForm form,const void *values) {
    if(form==GD_FORM_PRESENTATION)return 4096;
    if(form==GD_FORM_QUERY)return GD_EFFECT_WORK;
    if (form==GD_FORM_RESOURCE) return GD_RESOURCE_ROUTE_WORK;
    if (form==GD_FORM_STRUCTURE) return ((const GD_ExplorerStructureForm *)values)->kind>=GD_STRUCTURE_COUNT ? GD_SIMILARITY_WORK:GD_GRAPH_ANALYSIS_WORK;
    return ((const GD_ExplorerRuleForm *)values)->bindings ? GD_EFFECT_WORK:GD_RULE_EVALUATION_WORK_LIMIT;
}
int gd_explorer_control_editable(const GD_ExplorerContext *c,GD_ExplorerForm form,const GD_ExplorerControl *field,const void *values) {
    const char *name=field->name;
    if (form==GD_FORM_PRESENTATION) return gd_explorer_presentation_editable(c,name);
    if (!gd_explorer_form_available(c,form)) return 0;
    if (form==GD_FORM_QUERY) {
        const GD_ExplorerQueryOptions *q=values;
        if (c->pane->view==GD_EXPLORER_EFFECTS) return !strcmp(name,"accepted") || !strcmp(name,"weighted");
        if (!strcmp(name,"noun")) return q->coverage;
        if (!strcmp(name,"participant")) return q->root_case>=0;
        if (!strcmp(name,"axis")) return q->coverage || (q->participant>=0 && q->participant<c->project->rules->participant_count && c->project->rules->participants[q->participant].noun_resolution==GD_RULE_RESOLVED);
        if (!strcmp(name,"ghosts") || !strcmp(name,"primary") || !strcmp(name,"compare") || !strcmp(name,"include_excluded")) return c->pane->query_state && c->pane->query_state->query && c->pane->query_state->query->variants;
        if (!strcmp(name,"facts") || !strcmp(name,"contexts") || !strcmp(name,"literal_assumptions")) return !q->coverage && q->root_case>=0;
        if (!strcmp(name,"values") || !strcmp(name,"literal_values")) return q->axis>=0;
        if (!strcmp(name,"weighted")) return 0; /* Effect-only drawing control. */
    } else if (form==GD_FORM_RULES) {
        const GD_ExplorerRuleForm *r=values;
        if (!strcmp(name,"kind") || !strcmp(name,"bindings")) return 1;
        if (!r->bindings && r->kind==GD_NAMED_CONSISTENCY) return 0;
        if (!strcmp(name,"facts") || !strcmp(name,"contexts")) return !r->bindings;
        if (!strcmp(name,"object") || !strcmp(name,"participant") || !strcmp(name,"axis") || !strcmp(name,"statuses")) return r->bindings;
    } else if (form==GD_FORM_STRUCTURE) {
        const GD_ExplorerStructureForm *s=values;
        if (!strcmp(name,"actor_scope")) return c->pane->view==GD_EXPLORER_ACTORS;
        if (s->kind==GD_STRUCTURE_DECLARATIONS) return !strcmp(name,"kind");
    }
    return 1;
}
typedef enum { CHOICE_NONE,CHOICE_BOOL,CHOICE_LIST,CHOICE_CASE,CHOICE_LOCAL,CHOICE_AXIS,CHOICE_NOUN,CHOICE_PRIMARY,CHOICE_COMPARE,CHOICE_QUESTION,CHOICE_RULE,CHOICE_VALUES,CHOICE_STATUS,CHOICE_NODE,CHOICE_FEATURE,CHOICE_RELATION,CHOICE_RESOURCE } ControlChoice;
static ControlChoice control_choice(GD_ExplorerForm form,const GD_ExplorerControl *field) {
    const char *name=field->name;
    if (form==GD_FORM_RESOURCE && (!strcmp(name,"from") || !strcmp(name,"to"))) return CHOICE_RESOURCE;
    if (!strcmp(name,"statuses")) return CHOICE_STATUS;
    if (form==GD_FORM_STRUCTURE) {
        if (!strcmp(name,"from") || !strcmp(name,"to") || !strcmp(name,"avoid") || !strcmp(name,"scope_root") || !strcmp(name,"starts") || !strcmp(name,"entries")) return CHOICE_NODE;
        if (!strcmp(name,"features")) return CHOICE_FEATURE;
        if (!strcmp(name,"relations")) return CHOICE_RELATION;
    }
    if (field->type==GD_CONTROL_BOOL) return CHOICE_BOOL;
    if (field->choices) return CHOICE_LIST;
    if (form==GD_FORM_QUERY) {
        if (!strcmp(name,"root_case")) return CHOICE_CASE;
        if (!strcmp(name,"participant")) return CHOICE_LOCAL;
        if (!strcmp(name,"axis")) return CHOICE_AXIS;
        if (!strcmp(name,"noun")) return CHOICE_NOUN;
        if (!strcmp(name,"primary")) return CHOICE_PRIMARY;
        if (!strcmp(name,"compare")) return CHOICE_COMPARE;
        if (!strcmp(name,"values")) return CHOICE_VALUES;
    } else if (form==GD_FORM_RULES) {
        if (!strcmp(name,"kind")) return CHOICE_RULE;
        if (!strcmp(name,"case_name")) return CHOICE_CASE;
        if (!strcmp(name,"axis")) return CHOICE_AXIS;
        if (!strcmp(name,"object")) return CHOICE_NOUN;
        if (!strcmp(name,"participant")) return CHOICE_LOCAL;
    } else if (!strcmp(name,"kind")) return CHOICE_QUESTION;
    return CHOICE_NONE;
}
static const char *control_list_item(const char *list,int index) {
    while (*list && index>0) { list+=strlen(list)+1; --index; }
    return *list && !index ? list:NULL;
}
/* Source slot count bounds discovery work. Filtered slots return 0; callers page
   slots, not fabricated/deduplicated identities. Duplicate same-key rows stay explicit. */
int gd_explorer_control_option_count(const GD_ExplorerContext *c,GD_ExplorerForm form,const GD_ExplorerControl *field,const void *values) {
    ControlChoice kind=control_choice(form,field); const GD_Rules *rules=c->project->rules; int count=0;
    if (form==GD_FORM_PRESENTATION) return gd_explorer_presentation_options(c,field);
    if (kind==CHOICE_NONE) return -1;
    if (kind==CHOICE_BOOL || kind==CHOICE_RULE) return 2;
    if (kind==CHOICE_LIST) { const char *p=field->choices; while (*p) { ++count; p+=strlen(p)+1; } return count; }
    if (kind==CHOICE_QUESTION) return GD_STRUCTURE_QUESTIONS;
    if (kind==CHOICE_RESOURCE) return c->project->resources->node_count+1;
    if (kind==CHOICE_PRIMARY || kind==CHOICE_COMPARE) {
        if (c->pane->query_state && c->pane->query_state->query && c->pane->query_state->query->variants) count=c->pane->query_state->query->variants->variant_count;
        return count+(kind==CHOICE_COMPARE);
    }
    if (kind==CHOICE_STATUS) return gd_explorer_model(c->project,c->pane->view)->record_count;
    if (kind==CHOICE_NODE || kind==CHOICE_FEATURE || kind==CHOICE_RELATION) {
        const GD_StructureResult *context=gd_explorer_structure_choices(c);
        if (!context) return 0;
        if (kind==CHOICE_NODE) return context->layout->node_count;
        if (kind==CHOICE_RELATION) return GD_STRUCTURE_KINDS+1;
        { GD_SimilarityFeatureInfo features[GD_SIMILARITY_FEATURES]; return gd_similarity_feature_catalogue(context,features); }
    }
    if (!rules) return 0;
    if (kind==CHOICE_VALUES) { int axis=((const GD_ExplorerQueryOptions *)values)->axis; return axis>=0 && axis<rules->model->record_count && rules->axes[axis].type==GD_AXIS_ENUM ? rules->model->records[axis].axis.allowed_count:-1; }
    if (kind==CHOICE_CASE) return rules->case_count+1;
    if (kind==CHOICE_LOCAL) return rules->participant_count+1;
    return rules->model->record_count+1;
}
static int control_key(char *out,size_t capacity,const char *value) {
    size_t length=strlen(value); if (length>=capacity) return -1;
    memcpy(out,value,length+1); return 1;
}
/* Translate native value to exact external key. Never expose an array index. */
static int control_native_key(const GD_ExplorerContext *c,GD_ExplorerForm form,const GD_ExplorerControl *field,int native,char *key,size_t capacity) {
    const GD_Rules *rules=c->project->rules; ControlChoice kind=control_choice(form,field); const char *value=NULL;
    if (native<0 && (kind==CHOICE_CASE || kind==CHOICE_LOCAL || kind==CHOICE_AXIS || kind==CHOICE_NOUN || kind==CHOICE_COMPARE || kind==CHOICE_RESOURCE)) return control_key(key,capacity,"");
    if (native<0) return 0;
    if (kind==CHOICE_RESOURCE) return gd_resource_node_id(c->project->resources,native,-1,key,capacity) ? 1:-1;
    if (kind==CHOICE_BOOL) value=native ? "true":"false";
    else if (kind==CHOICE_LIST) value=control_list_item(field->keys ? field->keys:field->choices,native);
    else if (kind==CHOICE_QUESTION) value=gd_structure_question_name(native);
    else if (kind==CHOICE_RULE && native<2) value=gd_named_name((GD_NamedKind)native);
    else if (kind==CHOICE_PRIMARY || kind==CHOICE_COMPARE) {
        const GD_Query *query=c->pane->query_state ? c->pane->query_state->query:NULL;
        if (query && query->variants && native<query->variants->variant_count) value=query->variants->variants[native].value;
    } else if (rules) {
        if (kind==CHOICE_CASE && native<rules->case_count) {
            const GD_RuleCase *item=&rules->cases[native]; int size=snprintf(key,capacity,"%s#%s",rules->model->records[rules->sources[item->source].record].reference,item->name);
            return size>=0 && (size_t)size<capacity ? 1:-1;
        }
        if (kind==CHOICE_LOCAL && native<rules->participant_count) value=rules->participants[native].name;
        if ((kind==CHOICE_AXIS || kind==CHOICE_NOUN) && native<rules->model->record_count) value=rules->model->records[native].reference;
    }
    return value ? control_key(key,capacity,value):0;
}
int gd_explorer_control_option(const GD_ExplorerContext *c,GD_ExplorerForm form,const GD_ExplorerControl *field,const void *values,int ordinal,int *native,char *key,size_t capacity) {
    ControlChoice kind=control_choice(form,field); const GD_Rules *rules=c->project->rules;
    int unset=kind==CHOICE_CASE || kind==CHOICE_LOCAL || kind==CHOICE_AXIS || kind==CHOICE_NOUN || kind==CHOICE_COMPARE || kind==CHOICE_RESOURCE;
    if (form==GD_FORM_PRESENTATION) return gd_explorer_presentation_option(c,field,ordinal,native,key,capacity);
    if (ordinal<0 || ordinal>=gd_explorer_control_option_count(c,form,field,values)) return 0;
    *native=ordinal-unset;
    if (kind==CHOICE_STATUS) { const char *status=gd_explorer_model(c->project,c->pane->view)->records[ordinal].status; return status && *status ? control_key(key,capacity,status):0; }
    if (kind==CHOICE_NODE || kind==CHOICE_FEATURE || kind==CHOICE_RELATION) {
        const GD_StructureResult *context=gd_explorer_structure_choices(c);
        if (kind==CHOICE_NODE) return gd_structure_result_node_id(context,context->layout->nodes[ordinal].key,key,capacity) ? 1:-1;
        if (kind==CHOICE_FEATURE) { GD_SimilarityFeatureInfo features[GD_SIMILARITY_FEATURES]; gd_similarity_feature_catalogue(context,features); return control_key(key,capacity,features[ordinal].id); }
        if (!ordinal) return control_key(key,capacity,"all");
        { int i; for (i=0;i<context->layout->edge_count;++i) if (context->layout->edges[i].relation==ordinal-1) return control_key(key,capacity,gd_graph_relation_name(ordinal-1)); } return 0;
    }
    if (kind==CHOICE_VALUES) { const char *value=control_list_item(rules->model->records[((const GD_ExplorerQueryOptions *)values)->axis].axis.allowed,ordinal); return value ? control_key(key,capacity,value):0; }
    if (*native<0 && kind==CHOICE_LOCAL && form==GD_FORM_QUERY && !((const GD_ExplorerQueryOptions *)values)->coverage) return 0;
    if (*native>=0 && kind==CHOICE_LOCAL) {
        int root=form==GD_FORM_QUERY ? ((const GD_ExplorerQueryOptions *)values)->root_case:gd_rule_case_find(rules->model,rules,((const GD_ExplorerRuleForm *)values)->case_name);
        if (form==GD_FORM_QUERY && root<0) return 0;
        if (root>=0 && (*native<rules->cases[root].first_participant || *native>=rules->cases[root].first_participant+rules->cases[root].participant_count)) return 0;
    }
    if (*native>=0 && (kind==CHOICE_AXIS || kind==CHOICE_NOUN)) {
        if (rules->model->records[*native].role!=(kind==CHOICE_AXIS ? GD_AXIS:GD_NOUN)) return 0;
        if (kind==CHOICE_AXIS && form==GD_FORM_QUERY && !gd_explorer_query_axis_allowed(values,rules,*native)) return 0;
    }
    return control_native_key(c,form,field,*native,key,capacity);
}
int gd_explorer_control_value(const GD_ExplorerContext *c,GD_ExplorerForm form,const GD_ExplorerControl *field,const void *values,char *out,size_t capacity) {
    const char *value=(const char *)values+field->offset; int size;
    if (form==GD_FORM_PRESENTATION) return gd_explorer_presentation_value(c,field,values,out,capacity);
    if (field->type==GD_CONTROL_TEXT) return control_key(out,capacity,value);
    if (control_choice(form,field)!=CHOICE_NONE) return control_native_key(c,form,field,field->type==GD_CONTROL_BOOL ? *(const bool *)value:*(const int *)value,out,capacity);
    size=snprintf(out,capacity,"%d",*(const int *)value); return size>=0 && (size_t)size<capacity ? 1:-1;
}
void gd_explorer_rule_case_transition(GD_ExplorerRuleForm *form,int before,int after) {
    if (before==after) return;
    if (form->bindings) *form->participant=0; else *form->facts=*form->contexts=0;
}
int gd_explorer_control_set(GD_ExplorerContext *c,GD_ExplorerForm form,const char *name,const char *value,char *message,size_t capacity) {
    union { GD_ExplorerQueryOptions query; GD_ExplorerStructureForm structure; GD_ExplorerRuleForm rules; GD_ExplorerResourceForm resource; } next;
    const GD_ExplorerControl *field=gd_explorer_form_field(form,name); void *prior,*target; char *slot; int native=-1,count,i;
    const char *error="Control unknown, inactive or unavailable for this pane/question";
    if (form==GD_FORM_PRESENTATION) return gd_explorer_presentation_set(c,name,value,message,capacity);
    if (!field || !value || !gd_explorer_form_available(c,form)) goto failed;
    prior=gd_explorer_form_pending(c,form,0);
    if (prior) memcpy(&next,prior,gd_explorer_form_size(form)); else gd_explorer_form_defaults(form,&next);
    if (!gd_explorer_control_editable(c,form,field,&next)) goto failed;
    slot=(char *)&next+field->offset;
    if (field->type==GD_CONTROL_TEXT) {
        error="Value exceeds native field capacity"; if (strlen(value)>=field->capacity) goto failed;
        if (form==GD_FORM_RULES && !strcmp(name,"case_name")) gd_explorer_rule_case_transition(&next.rules,gd_rule_case_find(c->project->rules->model,c->project->rules,next.rules.case_name),gd_rule_case_find(c->project->rules->model,c->project->rules,value));
        memcpy(slot,value,strlen(value)+1);
    } else if ((count=gd_explorer_control_option_count(c,form,field,&next))>=0) {
        char key[GD_MAX_PATH]; int found=0;
        error="Use one exact choice key from viewer controls; native indices are not selectors";
        int begin=0,end=count; ControlChoice choice=control_choice(form,field);
        if (choice==CHOICE_AXIS || choice==CHOICE_NOUN) { int record=*value ? gd_explorer_record(c->project->rules->model,value):-1; begin=record+1; end=begin+1; if (*value && record<0) goto failed; }
        for (i=begin;i<end;++i) { int candidate; if (gd_explorer_control_option(c,form,field,&next,i,&candidate,key,sizeof(key))!=1 || strcmp(key,value)) continue;
            if (found && native!=candidate) { error="Choice key is ambiguous in current scope"; goto failed; } native=candidate; found=1;
        }
        if (!found) goto failed;
        if (form==GD_FORM_QUERY && !strcmp(name,"coverage")) { if (next.query.coverage!=native) gd_explorer_query_question(&next.query,native); }
        else if (form==GD_FORM_QUERY && !strcmp(name,"root_case")) gd_explorer_query_root(&next.query,c->project->rules,native);
        else if (form==GD_FORM_QUERY && !strcmp(name,"participant")) { if (!gd_explorer_query_participant(&next.query,c->project->rules,native)) goto failed; }
        else if (form==GD_FORM_QUERY && !strcmp(name,"axis")) gd_explorer_query_axis(&next.query,c->project->rules,native);
        else if (form==GD_FORM_STRUCTURE && !strcmp(name,"kind")) gd_explorer_structure_question(&next.structure,native);
        else if (form==GD_FORM_RULES && !strcmp(name,"kind")) { if (next.rules.bindings || next.rules.kind!=native) gd_explorer_rule_mode(&next.rules,native); }
        else if (form==GD_FORM_RULES && !strcmp(name,"bindings")) { if (next.rules.bindings!=native) gd_explorer_rule_mode(&next.rules,native ? 2:next.rules.kind); }
        else if (field->type==GD_CONTROL_BOOL) *(bool *)slot=native!=0;
        else *(int *)slot=native;
    } else {
        char *end; long number; int maximum=gd_explorer_form_work_limit(form,&next);
        error="Work limit needs decimal integer in native range";
        if (!*value || *value<'0' || *value>'9') goto failed;
        errno=0; number=strtol(value,&end,10); if (errno || *end || number<0 || number>maximum) goto failed;
        *(int *)slot=(int)number;
    }
    target=gd_explorer_form_pending(c,form,1); error="Cannot allocate pending inputs; prior state retained"; if (!target) goto failed;
    memcpy(target,&next,gd_explorer_form_size(form)); *message=0; return 1;
failed:
    snprintf(message,capacity,"%s",error); return 0;
}
static const GD_ExplorerActionInfo actions[]={
    {"query.run","Apply native query",(1u<<GD_EXPLORER_ACTORS)|(1u<<GD_EXPLORER_EFFECTS),GD_FORM_QUERY},
    {"query.display","Rearrange query drawing",(1u<<GD_EXPLORER_ACTORS)|(1u<<GD_EXPLORER_EFFECTS),GD_FORM_QUERY},
    {"structure.run","Run named question",(1u<<GD_EXPLORER_VIEWS)-1,GD_FORM_STRUCTURE},
    {"structure.clear","Clear question",(1u<<GD_EXPLORER_VIEWS)-1,GD_FORM_STRUCTURE},
    {"rules.run","Run native reader question",1u<<GD_EXPLORER_RESOURCES,GD_FORM_RULES},
    {"rules.clear","Clear rule/binding inspection",1u<<GD_EXPLORER_RESOURCES,GD_FORM_RULES},
    {"resource.run","Run conditional witnesses",1u<<GD_EXPLORER_RESOURCES,GD_FORM_RESOURCE},
    {"resource.clear","Clear Resource query/witnesses",1u<<GD_EXPLORER_RESOURCES,GD_FORM_RESOURCE},
    {"view.fit","Fit",(1u<<GD_EXPLORER_VIEWS)-1,GD_FORM_PRESENTATION},
    {"view.rearrange","Rearrange",(1u<<GD_EXPLORER_VIEWS)-1,GD_FORM_PRESENTATION},
    {"view.expand","Expand selected relationship",1u<<GD_EXPLORER_RELATIONSHIPS,GD_FORM_PRESENTATION},
    {"view.start.add","Add selected analysis start",(1u<<GD_EXPLORER_ACTORS)-1,GD_FORM_PRESENTATION},
    {"view.start.clear","Clear analysis starts",(1u<<GD_EXPLORER_ACTORS)-1,GD_FORM_PRESENTATION},
    {"view.analyze","Analyze shown layers",(1u<<GD_EXPLORER_ACTORS)-1,GD_FORM_PRESENTATION},
    {"view.compare","Compare retained entries",(1u<<GD_EXPLORER_VIEWS)-1,GD_FORM_PRESENTATION},
    {"view.reset","Reset view",(1u<<GD_EXPLORER_VIEWS)-1,GD_FORM_PRESENTATION},
    {"view.find.clear","Clear Find",(1u<<GD_EXPLORER_VIEWS)-1,GD_FORM_PRESENTATION},
    {"view.witness.fit","Frame Resource witness",1u<<GD_EXPLORER_RESOURCES,GD_FORM_PRESENTATION},
    {"view.cell.fit","Frame selected pair",(1u<<GD_EXPLORER_MATRIX)|(1u<<GD_EXPLORER_NOUNS),GD_FORM_PRESENTATION},
    {"view.selection.focus","Focus selection",(1u<<GD_EXPLORER_VIEWS)-1,GD_FORM_PRESENTATION}
};
const GD_ExplorerActionInfo *gd_explorer_action_info(GD_ExplorerAction action) {
    return action>=0 && action<GD_ACTION_COUNT ? &actions[action]:NULL;
}
int gd_explorer_action_available(const GD_ExplorerContext *context,GD_ExplorerAction action) {
    const GD_ExplorerActionInfo *info=gd_explorer_action_info(action);
    if (!info || !context || !context->project || !context->pane || !(info->views&(1u<<context->pane->view)) || !(context->project->available&(1u<<context->pane->view))) return 0;
    if (info->form==GD_FORM_PRESENTATION) {
        const GD_ExplorerPane *p=context->pane;
        if(!p->prepared)return 0;
        if(action==GD_ACTION_VIEW_SELECTION_FOCUS)return gd_explorer_focus_available(context);
        if(action==GD_ACTION_VIEW_RESET && context->project->draft && (context->project->draft->modified_count || context->project->draft->history_count))return 0;
        if(action==GD_ACTION_VIEW_REARRANGE && p->view==GD_EXPLORER_NOUNS)return p->prepared;
        if(action==GD_ACTION_VIEW_REARRANGE && p->view>=GD_EXPLORER_ACTORS) return p->query_state && p->query_state->scene && !(p->structure_state && p->structure_state->similarity_canvas);
        if(action==GD_ACTION_VIEW_WITNESS_FIT)return p->resource_routes && p->resource_routes->witness_count>0;
        if(action==GD_ACTION_VIEW_CELL_FIT)return !gd_explorer_similarity_active(context) && p->selection.node<0 && (p->selection.cell>=0 || (p->grid.noun>=0 && p->grid.verb>=0));
        if(action==GD_ACTION_VIEW_ANALYZE)return p->view<GD_EXPLORER_ACTORS && !(p->view==GD_EXPLORER_MATRIX && p->layout_choice==2) && !(p->structure_state && p->structure_state->similarity_canvas);
        if(action==GD_ACTION_VIEW_START_ADD)return p->paths.enabled && p->selection.node>=0;
        if(action==GD_ACTION_VIEW_EXPAND)return p->selection.node>=0;
        if(action==GD_ACTION_VIEW_COMPARE)return (p->view==GD_EXPLORER_ACTORS && p->query_state && p->query_state->query && p->query_state->query->variants && p->query_state->applied.primary>=0 && p->query_state->applied.compare>=0) || (p->structure_state && p->structure_state->result && p->structure_state->result->similarity && p->structure_state->similarity_row>=0 && p->structure_state->similarity_peer>=0);
        return 1;
    }
    if (action==GD_ACTION_QUERY_DISPLAY) return context->pane->query_state && context->pane->query_state->query;
    return 1;
}
int gd_explorer_action_run(GD_ExplorerContext *context,GD_ExplorerAction action) {
    if (!gd_explorer_action_available(context,action)) return 0;
    if (gd_explorer_action_info(action)->form==GD_FORM_PRESENTATION) return gd_explorer_presentation_action(context,gd_explorer_action_info(action)->name);
    switch (action) {
        case GD_ACTION_QUERY_RUN: return gd_explorer_query_run(context);
        case GD_ACTION_QUERY_DISPLAY: return gd_explorer_query_display(context);
        case GD_ACTION_STRUCTURE_RUN: return gd_explorer_structure_run(context);
        case GD_ACTION_STRUCTURE_CLEAR: return gd_explorer_structure_clear(context);
        case GD_ACTION_RULES_RUN: return gd_explorer_form_pending(context,GD_FORM_RULES,1) && gd_explorer_rules_run(context);
        case GD_ACTION_RESOURCE_RUN: return gd_explorer_resource_run(context);
        case GD_ACTION_RESOURCE_CLEAR:
            gd_resource_routes_destroy(context->pane->resource_routes); context->pane->resource_routes=NULL; context->pane->resource_witness=-1;
            gd_explorer_form_defaults(GD_FORM_RESOURCE,&context->pane->resource_pending); context->pane->resource_applied=context->pane->resource_pending;
            *context->pane->resource_message=0; context->analysis->initialized=0; return 1;
        case GD_ACTION_RULES_CLEAR: gd_explorer_rules_destroy(context->pane->rule_state); context->pane->rule_state=NULL; *context->pane->rule_message=0; return 1;
        default: return 0;
    }
}
int gd_explorer_action_button(GD_ExplorerContext *context,GD_ExplorerAction action) {
    const GD_ExplorerActionInfo *info=gd_explorer_action_info(action); assert(info);
    if (!igButton(info->label,(ImVec2_c){0,0})) return 0;
    gd_explorer_action_run(context,action); return 1;
}
