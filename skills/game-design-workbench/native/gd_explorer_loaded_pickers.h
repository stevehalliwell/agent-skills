#ifndef GD_EXPLORER_LOADED_PICKERS_H
#define GD_EXPLORER_LOADED_PICKERS_H
#include "gd_explorer_picker.h"
#include <math.h>
static void gd_explorer_picker_toggle(GD_ExplorerContext *context,char *text,size_t capacity,const char *value,int maximum) {
    if (!gd_picker_toggle_limit(text,capacity,value,maximum)) snprintf(context->pane->query_message,sizeof(context->pane->query_message),"Selection exceeds native item/text cap or contains a line break; previous inputs retained.");
}
static int gd_explorer_picker_case_index(const GD_Model *model,const GD_Rules *rules,const char *text) {
    int i; char id[GD_MAX_PATH]; if (!*text) return -1;
    for (i=0;i<rules->case_count;++i) { const GD_RuleCase *item=&rules->cases[i]; int size=snprintf(id,sizeof(id),"%s#%s",model->records[rules->sources[item->source].record].reference,item->name); if (size>=0 && size<(int)sizeof(id) && !strcmp(id,text)) return i; } return -1;
}
static const GD_ResourceNode *gd_explorer_picker_resource_node(const GD_ExplorerContext *context) {
    const GD_ExplorerPane *pane=context->pane; int node=pane->selection.node;
    if (pane->view==GD_EXPLORER_RESOURCES && node>=0 && node<pane->layout->node_count) { int key=pane->layout->nodes[node].key; if (key>=0 && key<context->project->resources->node_count) return &context->project->resources->nodes[key]; } return NULL;
}
static int gd_explorer_picker_case(GD_ExplorerContext *context,const char *label,char *text,size_t capacity,int allow_any) {
    const GD_Rules *rules=context->project->rules; const GD_Model *model=rules->model; int i,before=gd_explorer_picker_case_index(model,rules,text); char id[GD_MAX_PATH];
    if (gd_explorer_picker_begin(context,label,before>=0 ? TextFormat("%s | %s",rules->cases[before].name,model->records[rules->sources[rules->cases[before].source].record].name):allow_any ? "All source cases":"Choose source case")) {
        if (allow_any && gd_explorer_picker_item(context,"All source cases",!*text,0)) *text=0;
        for (i=0;i<rules->case_count;++i) { const GD_RuleCase *item=&rules->cases[i]; const GD_Record *source=&model->records[rules->sources[item->source].record]; int size=snprintf(id,sizeof(id),"%s#%s",source->reference,item->name); if (size<0 || (size_t)size>=capacity || size>=(int)sizeof(id)) continue;
            igPushID_Int(i); if (gd_explorer_picker_item(context,TextFormat("%s | %s",item->name,source->reference),i==before,0)) memcpy(text,id,(size_t)size+1); igPopID();
        } gd_explorer_picker_end();
    }
    { const GD_ResourceNode *node=gd_explorer_picker_resource_node(context); igPushID_Str(label); igBeginDisabled(!node || node->case_index<0);
        if (igSmallButton("Use clicked case declaration") && node && node->case_index>=0) { const GD_RuleCase *item=&rules->cases[node->case_index]; int size=snprintf(id,sizeof(id),"%s#%s",model->records[rules->sources[item->source].record].reference,item->name); if (size>=0 && size<(int)sizeof(id) && (size_t)size<capacity) memcpy(text,id,(size_t)size+1); else snprintf(context->pane->query_message,sizeof(context->pane->query_message),"Case identity exceeds native UI text cap; prior input retained."); }
        igEndDisabled(); igPopID();
    }
    return gd_explorer_picker_case_index(model,rules,text);
}
static void gd_explorer_picker_record(GD_ExplorerContext *context,const char *label,GD_Role role,char *text,size_t capacity) {
    const GD_Model *model=context->project->rules->model; int i;
    if (gd_explorer_picker_begin(context,label,*text ? text:"All loaded definitions")) {
        if (gd_explorer_picker_item(context,"All loaded definitions",!*text,0)) *text=0;
        for (i=0;i<model->record_count;++i) if (model->records[i].role==role && strlen(model->records[i].reference)<capacity && gd_explorer_picker_item(context,TextFormat("%s | %s",model->records[i].name,model->records[i].reference),!strcmp(text,model->records[i].reference),0)) snprintf(text,capacity,"%s",model->records[i].reference);
        gd_explorer_picker_end();
    }
    { const GD_ResourceNode *node=gd_explorer_picker_resource_node(context); int record=-1;
        if (node) { if (role==GD_AXIS) record=node->axis; else if (role==GD_NOUN && node->participant>=0 && context->project->rules->participants[node->participant].noun_resolution==GD_RULE_RESOLVED) record=context->project->rules->participants[node->participant].noun_record; else if (node->record>=0 && model->records[node->record].role==role) record=node->record; }
        igPushID_Str(label); igBeginDisabled(record<0); if (igSmallButton("Use clicked definition") && record>=0) { if (strlen(model->records[record].reference)<capacity) snprintf(text,capacity,"%s",model->records[record].reference); else snprintf(context->pane->query_message,sizeof(context->pane->query_message),"Definition identity exceeds native UI text cap; prior input retained."); } igEndDisabled(); igPopID();
    }
}
static int gd_explorer_picker_local(GD_ExplorerContext *context,const char *label,int root,int selected,int allow_any) {
    const GD_Rules *rules=context->project->rules; int i; if (root<0 || root>=rules->case_count) return -1;
    if (gd_explorer_picker_begin(context,label,selected>=0 ? rules->participants[selected].name:allow_any ? "All local bindings":"Choose local binding")) {
        const GD_RuleCase *item=&rules->cases[root]; if (allow_any && gd_explorer_picker_item(context,"All local bindings",selected<0,0)) selected=-1;
        for (i=item->first_participant;i<item->first_participant+item->participant_count;++i) { const GD_RuleParticipant *participant=&rules->participants[i]; igPushID_Int(i); if (gd_explorer_picker_item(context,TextFormat("%s | %s%s",participant->name,participant->noun,i==item->primary_participant ? " [authored A]":""),i==selected,0)) selected=i; igPopID(); }
        gd_explorer_picker_end();
    } return selected;
}
static void gd_explorer_picker_lines(GD_ExplorerContext *context,const char *label,char *text,size_t capacity) {
    const char *cursor=text; char value[GD_MAX_RULE_LINE+1]; int row=0; igPushID_Str(label);
    igTextUnformatted(label,NULL); if (*text && igSmallButton("Clear all")) *text=0;
    while (*cursor) { const char *end=strchr(cursor,'\n'); size_t length=end ? (size_t)(end-cursor):strlen(cursor); if (length>=sizeof(value)) break; memcpy(value,cursor,length); value[length]=0;
        igPushID_Int(row++); if (igSmallButton("Remove")) { gd_picker_toggle(text,capacity,value); igPopID(); break; } igSameLine(0,8); igTextWrapped("%s",value); igPopID(); if (!end) break; cursor=end+1;
    }
    igPopID();
}
static const char *gd_explorer_picker_enum_value(const GD_ValueDefinition *axis,int index) {
    const char *value=axis->allowed; int i; if (index<0 || index>=axis->allowed_count) return NULL; for (i=0;i<index;++i) value+=strlen(value)+1; return value;
}
static int gd_explorer_picker_enum(GD_ExplorerContext *context,const char *label,int axis,int selected) {
    const GD_Model *model=context->project->rules->model; const GD_ValueDefinition *definition=&model->records[axis].axis; const char *value=definition->allowed; int i;
    if (gd_explorer_picker_begin(context,label,gd_explorer_picker_enum_value(definition,selected))) { for (i=0;i<definition->allowed_count;++i,value+=strlen(value)+1) if (gd_explorer_picker_item(context,value,i==selected,0)) selected=i; gd_explorer_picker_end(); } return selected;
}
static void gd_explorer_picker_assumptions(GD_ExplorerContext *context,int root,char *facts,size_t facts_capacity,char *contexts,size_t contexts_capacity,bool *literal_mode) {
    const GD_Rules *rules=context->project->rules; const GD_Model *model=rules->model; GD_ExplorerAssumptionForm *form=&context->pane->assumption_form; int i;
    if (form->project!=context->project || form->root!=root) { memset(form,0,sizeof(*form)); form->project=context->project; form->root=root; form->participant=form->axis=form->left=form->right=form->context_axis=-1; }
    if (literal_mode && !*facts && !*contexts) *literal_mode=true;
    if (literal_mode && !*literal_mode) igTextWrapped("Legacy semicolon assumptions: clear both lists before using typed builders, or switch raw input mode explicitly.");
    igBeginDisabled(literal_mode && !*literal_mode);
    if (root>=0 && root<rules->case_count && igTreeNode_Str("Add typed state assumption")) {
        int before=form->participant; form->participant=gd_explorer_picker_local(context,"State binding",root,form->participant,0); if (before!=form->participant) form->axis=-1;
        if (form->participant>=0 && rules->participants[form->participant].noun_resolution==GD_RULE_RESOLVED && gd_explorer_picker_begin(context,"Declared state axis",form->axis>=0 ? model->records[form->axis].reference:NULL)) {
            for (i=rules->actor_first[rules->participants[form->participant].noun_record];i>=0;i=rules->next_axis[i]) if (rules->axis_resolution[i]==GD_RULE_RESOLVED) { int axis=model->edges[i].target; if (gd_explorer_picker_item(context,TextFormat("%s | %s",model->records[axis].name,model->records[axis].reference),axis==form->axis,0)) { form->axis=axis; form->value=0; } }
            gd_explorer_picker_end();
        }
        if (form->axis>=0) {
            const GD_Record *axis=&model->records[form->axis]; const char *value=NULL; char number[64],literal[GD_MAX_RULE_LINE+1]; int type=rules->axes[form->axis].type;
            if (type==GD_AXIS_ENUM) { form->value=gd_explorer_picker_enum(context,"State value",form->axis,form->value); value=gd_explorer_picker_enum_value(&axis->axis,form->value); }
            else if (type==GD_AXIS_INTEGER || type==GD_AXIS_NUMBER) { igInputDouble("Numeric state",&form->number,0,0,"%.17g",0); igTextWrapped("Declared type %s | unit %s | bounds %s .. %s",axis->axis.type,axis->axis.unit ? axis->axis.unit:"unspecified",axis->axis.minimum ? axis->axis.minimum:"unspecified",axis->axis.maximum ? axis->axis.maximum:"unspecified"); if (isfinite(form->number) && (type!=GD_AXIS_INTEGER || floor(form->number)==form->number)) { snprintf(number,sizeof(number),"%.17g",form->number); value=number; } }
            igBeginDisabled(!value); if (igButton("Add state assumption",(ImVec2_c){0,0}) && value) { int size=snprintf(literal,sizeof(literal),"%s:%s=%s",rules->participants[form->participant].name,axis->reference,value); if (size>0 && size<(int)sizeof(literal) && !gd_picker_contains(facts,literal)) gd_explorer_picker_toggle(context,facts,facts_capacity,literal,GD_MAX_RULE_CONDITION_ITEMS); } igEndDisabled();
        }
        igTreePop();
    }
    if (root>=0 && root<rules->case_count && igTreeNode_Str("Add typed context assumption")) {
        int owner=rules->sources[rules->cases[root].source].record;
        form->left=gd_explorer_picker_local(context,"Context left binding",root,form->left,0);
        if (gd_explorer_picker_begin(context,"Declared context axis",form->context_axis>=0 ? model->records[form->context_axis].reference:NULL)) {
            for (i=0;i<model->edge_count;++i) if (model->edges[i].source==owner && model->edges[i].field==GD_CONTEXT_AXES && model->edges[i].target>=0) { int axis=model->edges[i].target; if (rules->axes[axis].valid && rules->axes[axis].type==GD_AXIS_ENUM && gd_explorer_picker_item(context,model->records[axis].reference,axis==form->context_axis,0)) { form->context_axis=axis; form->context_value=0; } }
            gd_explorer_picker_end();
        }
        if (form->context_axis>=0) form->context_value=gd_explorer_picker_enum(context,"Context relation",form->context_axis,form->context_value);
        form->right=gd_explorer_picker_local(context,"Context right binding",root,form->right,0); igCheckbox("Context is false (explicit context-not)",&form->context_false);
        { const char *value=form->context_axis>=0 ? gd_explorer_picker_enum_value(&model->records[form->context_axis].axis,form->context_value):NULL; int valid=form->left>=0 && form->right>=0 && value;
            igBeginDisabled(!valid); if (igButton("Add context assumption",(ImVec2_c){0,0}) && valid) { char literal[GD_MAX_RULE_LINE+1]; int size=snprintf(literal,sizeof(literal),"%s%s %s.%s %s",form->context_false ? "!":"",rules->participants[form->left].name,model->records[form->context_axis].reference,value,rules->participants[form->right].name); if (size>0 && size<(int)sizeof(literal) && !gd_picker_contains(contexts,literal)) gd_explorer_picker_toggle(context,contexts,contexts_capacity,literal,GD_MAX_RULE_CONTEXT_FACTS); } igEndDisabled();
        } igTreePop();
    }
    igEndDisabled();
    gd_explorer_picker_lines(context,"Pending state assumptions",facts,facts_capacity); gd_explorer_picker_lines(context,"Pending context assumptions",contexts,contexts_capacity);
    if (igTreeNode_Str("Advanced: raw assumptions")) { if (literal_mode) igCheckbox("Exact newline literals (otherwise ; shorthand)",literal_mode); igInputTextMultiline("Facts: LOCAL:ALIAS/AXIS=VALUE",facts,facts_capacity,(ImVec2_c){0,70},0,NULL,NULL); igInputTextMultiline("Contexts: LOCAL ALIAS/AXIS.STATE LOCAL; ! means false",contexts,contexts_capacity,(ImVec2_c){0,70},0,NULL,NULL); igTreePop(); }
}
#endif
