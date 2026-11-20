#include "gd_actor_identity.h"
#include <string.h>

typedef struct { char *text; size_t size,used; int *work,limit,valid; const GD_Query *query; } Identity;
static int charge(Identity *id, size_t amount) {
    if (!id->valid || amount>(size_t)(id->limit-*id->work)) { id->valid=0; return 0; }
    *id->work+=(int)amount; return 1;
}
static void span(Identity *id, const char *text, size_t length) {
    char count[32]; int bytes;
    if (!id->valid || !charge(id,length+1)) return;
    bytes=snprintf(count,sizeof(count),"%llu:",(unsigned long long)length);
    if (bytes<0 || (size_t)bytes>=sizeof(count) || (size_t)bytes>=id->size-id->used || length>=id->size-id->used-(size_t)bytes) { id->valid=0; return; }
    memcpy(id->text+id->used,count,(size_t)bytes); id->used+=(size_t)bytes;
    if (length) memcpy(id->text+id->used,text,length);
    id->used+=length; id->text[id->used]=0;
}
static void text(Identity *id, const char *value) { span(id,value ? value:"",value ? strlen(value):0); }
static void number(Identity *id, int value) { char digits[32]; int count=snprintf(digits,sizeof(digits),"%d",value); if (count<0 || (size_t)count>=sizeof(digits)) id->valid=0; else span(id,digits,(size_t)count); }
static void record(Identity *id, int index) {
    const GD_Record *item;
    if (index<0 || index>=id->query->model->record_count) { id->valid=0; return; }
    item=&id->query->model->records[index]; text(id,item->reference); text(id,item->id); number(id,(int)item->role);
    if (item->role==GD_AXIS) { text(id,item->axis.type); text(id,item->axis.unit); }
}
static void rule_case(Identity *id, int index) {
    const GD_RuleCase *item;
    if (index<0 || index>=id->query->rules->case_count) { id->valid=0; return; }
    item=&id->query->rules->cases[index]; record(id,id->query->rules->sources[item->source].record); text(id,item->name);
}
static void participant(Identity *id, int index) {
    const GD_RuleParticipant *item;
    if (index<0 || index>=id->query->rules->participant_count) { id->valid=0; return; }
    item=&id->query->rules->participants[index]; text(id,item->name); text(id,item->noun); number(id,(int)item->noun_resolution);
    if (item->noun_resolution==GD_RULE_RESOLVED) record(id,item->noun_record);
}
static void activation(Identity *id, int index) {
    const GD_ActorQuery *source=id->query->variants; const GD_RuleActivation *item; int i;
    if (index<0 || index>=source->activation_count) { id->valid=0; return; }
    item=&source->activations[index];
    for (i=0;i<item->path_count && id->valid;++i) {
        int call=item->calls[i]; rule_case(id,item->path[i]);
        if (call>=0) {
            const GD_RuleCall *declaration=&id->query->analysis->calls[call]; int j,ordinal=0;
            rule_case(id,declaration->caller_case); participant(id,declaration->caller); number(id,declaration->line);
            span(id,declaration->text,declaration->length);
            /* Source-line call-site ordinal, not global compiled-call offset.
               Exact ancestor body checks guard ambiguous edits during reload. */
            for (j=0;j<call && id->valid;++j) {
                const GD_RuleCall *other=&id->query->analysis->calls[j];
                if (!charge(id,1)) break;
                if (other->caller_case==declaration->caller_case && other->caller==declaration->caller && other->line==declaration->line) ++ordinal;
            }
            number(id,ordinal);
        }
    }
}
int gd_actor_scene_node_id(const GD_QueryScene *scene, int index, char *output, size_t size, int *work, int work_limit) {
    const GD_QuerySceneNode *node; const GD_ActorQuery *source; Identity id; int i;
    if (!output || !size) return 0; output[0]=0;
    if (!scene || !scene->query || !(source=scene->query->variants) || index<0 || index>=scene->layout.node_count || !work || *work<0 || work_limit<*work || work_limit>GD_ACTOR_WORK) return 0;
    id=(Identity){output,size,0,work,work_limit,1,scene->query}; node=&scene->nodes[index];
    text(&id,"actor_query"); rule_case(&id,source->case_index); participant(&id,source->participant); record(&id,source->axis);
    text(&id,gd_query_scene_node_kind_name(node->kind));
    if (node->variant>=0) text(&id,source->variants[node->variant].value);
    if (node->kind==GD_QUERY_NODE_BINDING || node->kind==GD_QUERY_NODE_PARTNER) participant(&id,node->source);
    else if (node->kind==GD_QUERY_NODE_ACTIVATION) activation(&id,node->source);
    else if (node->kind==GD_QUERY_NODE_EFFECT) {
        const GD_ActorEffect *effect=&source->effects[node->source];
        text(&id,gd_effect_kind_name(effect->kind));
        if (effect->axis>=0) record(&id,effect->axis); else text(&id,"unresolved_axis");
        text(&id,effect->state);
        if (effect->activation>=0) {
            const GD_RuleWrite *write=NULL; const GD_RuleParticipant *owner; int ordinal=-1;
            activation(&id,effect->activation); rule_case(&id,effect->origin_case); participant(&id,effect->actor);
            number(&id,(int)effect->declaration_kind);
            for (i=0;i<source->write_count && id.valid;++i) {
                if (!charge(&id,1)) break;
                if (source->write_effects[i]==node->source) { write=&source->writes[i]; break; }
            }
            if (!write) id.valid=0;
            else {
                participant(&id,write->origin_participant); owner=&id.query->rules->participants[write->origin_participant];
                ordinal=effect->declaration-(write->kind==GD_WRITE_STATE ? owner->first_state_effect:write->kind==GD_WRITE_QUANTITY ? owner->first_assignment:owner->first_preservation);
                number(&id,ordinal); number(&id,write->line);
            }
        }
    } else if (node->kind!=GD_QUERY_NODE_VARIANT) id.valid=0;
    if (!id.valid) output[0]=0;
    return id.valid;
}
