#include "gd_actor_inventory.h"
#include <assert.h>
#include <string.h>

const char *gd_actor_inventory_kind_name(GD_ActorInventoryKind kind) {
    static const char *const names[]={"case","local_binding","noun_definition","axis_definition"};
    return kind>=GD_ACTOR_CASE && kind<=GD_ACTOR_AXIS_DEFINITION ? names[kind]:NULL;
}
const char *gd_actor_inventory_link_name(GD_ActorInventoryLinkKind kind) {
    static const char *const names[]={"contains_binding","declared_noun","declared_axis","declared_context_axis","invokes","maps_binding"};
    return kind>=GD_ACTOR_CONTAINS_BINDING && kind<=GD_ACTOR_MAPS_BINDING ? names[kind]:NULL;
}
static int charge(GD_ActorInventory *inventory, int work) {
    assert(work>=0 && inventory->work<=inventory->work_limit);
    if (inventory->limited || work>inventory->work_limit-inventory->work) { inventory->limited=1; return 0; }
    inventory->work+=work; return 1;
}
static int node(GD_ActorInventory *inventory, GD_ActorInventoryKind kind, int record, int case_index, int participant) {
    int index=inventory->node_count;
    if (!charge(inventory,1) || index==GD_ACTOR_INVENTORY_NODES) { inventory->limited=1; return -1; }
    inventory->nodes[index]=(GD_ActorInventoryNode){kind,record,case_index,participant};
    ++inventory->node_count; return index;
}
static int link(GD_ActorInventory *inventory, GD_ActorInventoryLinkKind kind, int from, int to, int declaration, int binding) {
    assert(from>=0 && from<inventory->node_count && to>=0 && to<inventory->node_count);
    if (!charge(inventory,1) || inventory->link_count==GD_ACTOR_INVENTORY_LINKS) { inventory->limited=1; return 0; }
    inventory->links[inventory->link_count++]=(GD_ActorInventoryLink){kind,from,to,declaration,binding}; return 1;
}
GD_ActorInventory *gd_actor_inventory_build(GD_Allocator allocator, const GD_RuleAnalysis *source, int work_limit) {
    GD_ActorInventory *inventory; const GD_Model *model; const GD_Rules *rules; int i,p,source_indices[GD_MAX_RECORDS];
    if (!source || !(model=source->model) || !(rules=source->rules) || rules->model!=model ||
        !allocator.allocate || !allocator.release || work_limit<0 || work_limit>GD_ACTOR_INVENTORY_WORK) return NULL;
    assert(model->record_count>=0 && model->record_count<=GD_MAX_RECORDS && model->edge_count>=0 && model->edge_count<=GD_MAX_EDGES);
    assert(rules->case_count>=0 && rules->case_count<=GD_MAX_RULE_CASES && rules->participant_count>=0 && rules->participant_count<=GD_MAX_RULE_PARTICIPANTS);
    inventory=allocator.allocate(allocator.context,sizeof(*inventory)); if (!inventory) return NULL;
    memset(inventory,0,sizeof(*inventory)); inventory->allocator=allocator; inventory->source=source; inventory->work_limit=work_limit;
    for (i=0;i<GD_MAX_RECORDS;++i) { inventory->definition_nodes[i]=-1; source_indices[i]=-1; }
    for (i=0;i<GD_MAX_RULE_CASES;++i) inventory->case_nodes[i]=-1;
    for (i=0;i<GD_MAX_RULE_PARTICIPANTS;++i) inventory->participant_nodes[i]=-1;
    inventory->source_incomplete=model->error_count || model->errors_omitted || model->finding_count || model->findings_omitted ||
        rules->finding_count || rules->findings_omitted || !rules->bindings_complete || !rules->axis_definitions_complete || source->compile_limited;
    for (i=0;i<model->record_count;++i) {
        const GD_Record *record=&model->records[i];
        if (!charge(inventory,1)) break;
        if (record->role!=GD_NOUN && record->role!=GD_AXIS) continue;
        inventory->definition_nodes[i]=node(inventory,record->role==GD_NOUN ? GD_ACTOR_NOUN_DEFINITION:GD_ACTOR_AXIS_DEFINITION,i,-1,-1);
        if (inventory->definition_nodes[i]<0) break;
    }
    inventory->unexamined_records=model->record_count-i;
    for (i=0;i<rules->source_count && !inventory->limited;++i) {
        if (!charge(inventory,1)) break;
        source_indices[rules->sources[i].record]=i;
        if (rules->sources[i].state!=GD_RULE_PARSED) ++inventory->incomplete_sources;
    }
    inventory->unexamined_sources=rules->source_count-i;
    /* Source declarations without parsed cases remain explicit unknown scope;
       do not synthesize actors to make empty/invalid drafts look complete. */
    if (inventory->incomplete_sources) inventory->source_incomplete=1;
    for (i=0;i<rules->case_count && !inventory->limited;++i) {
        const GD_RuleCase *item=&rules->cases[i]; int record=rules->sources[item->source].record;
        assert(item->participant_count>=0 && item->first_participant>=0 && item->first_participant<=rules->participant_count-item->participant_count);
        if (!charge(inventory,1)) break;
        /* Keep case and all local binding identities together. Edges are a
           separately bounded retained prefix with explicit incompleteness. */
        if (1+item->participant_count>GD_ACTOR_INVENTORY_NODES-inventory->node_count ||
            1+item->participant_count>inventory->work_limit-inventory->work) { inventory->limited=1; break; }
        inventory->case_nodes[i]=node(inventory,GD_ACTOR_CASE,record,i,-1);
        for (p=item->first_participant;p<item->first_participant+item->participant_count;++p)
            inventory->participant_nodes[p]=node(inventory,GD_ACTOR_LOCAL_BINDING,record,i,p);
    }
    inventory->unexamined_cases=rules->case_count-i;
    for (i=0;i<rules->participant_count && !inventory->limited;++i) {
        const GD_RuleParticipant *participant=&rules->participants[i]; int binding=inventory->participant_nodes[i],definition;
        if (!charge(inventory,1)) break;
        if (binding<0) continue;
        { GD_RuleEffectSpan spans[2];
          if (gd_rule_effect_spans(rules,inventory->nodes[binding].case_index,i,GD_RULE_EFFECT_CALL,spans) && !source->calls_complete[i]) {
              ++inventory->incomplete_call_owners; inventory->source_incomplete=1;
          }
        }
        if (!link(inventory,GD_ACTOR_CONTAINS_BINDING,inventory->case_nodes[inventory->nodes[binding].case_index],binding,i,-1)) break;
        if (participant->noun_resolution!=GD_RULE_RESOLVED) { ++inventory->unresolved_nouns; inventory->source_incomplete=1; continue; }
        assert(participant->noun_record>=0 && participant->noun_record<model->record_count);
        definition=inventory->definition_nodes[participant->noun_record]; assert(definition>=0);
        if (!link(inventory,GD_ACTOR_DECLARED_NOUN,binding,definition,i,-1)) break;
    }
    inventory->unexamined_participants=rules->participant_count-i;
    for (i=0;i<model->edge_count && !inventory->limited;++i) {
        const GD_Edge *edge=&model->edges[i]; int from,to,rule_source=source_indices[edge->source];
        if (!charge(inventory,1)) break;
        if (edge->field!=GD_AXES && edge->field!=GD_CONTEXT_AXES) continue;
        if (model->records[edge->source].role!=GD_NOUN && (edge->field!=GD_CONTEXT_AXES || rule_source<0)) continue;
        if (rules->axis_resolution[i]!=GD_RULE_RESOLVED) { ++inventory->unresolved_axes; inventory->source_incomplete=1; continue; }
        to=inventory->definition_nodes[edge->target]; assert(to>=0);
        if (model->records[edge->source].role==GD_NOUN) {
            from=inventory->definition_nodes[edge->source]; assert(from>=0);
            if (!link(inventory,edge->field==GD_AXES ? GD_ACTOR_DECLARED_AXIS:GD_ACTOR_DECLARED_CONTEXT_AXIS,from,to,i,-1)) break;
        } else {
            const GD_RuleSource *declaration=&rules->sources[rule_source]; int c;
            for (c=declaration->first_case;c<declaration->first_case+declaration->case_count;++c)
                if (!link(inventory,GD_ACTOR_DECLARED_CONTEXT_AXIS,inventory->case_nodes[c],to,i,-1)) break;
            if (inventory->limited) break;
        }
    }
    inventory->unexamined_edges=model->edge_count-i;
    for (i=0;i<source->call_count && !inventory->limited;++i) {
        const GD_RuleCall *call=&source->calls[i]; int c,b;
        if (!charge(inventory,1)) break;
        if (call->resolution!=GD_RULE_RESOLVED) { ++inventory->unresolved_calls; inventory->source_incomplete=1; continue; }
        assert(call->caller_case>=0 && call->caller_case<rules->case_count && call->first_case>=0 && call->case_count<=rules->case_count-call->first_case);
        for (c=call->first_case;c<call->first_case+call->case_count;++c)
            if (!link(inventory,GD_ACTOR_INVOKES,inventory->case_nodes[call->caller_case],inventory->case_nodes[c],i,-1)) break;
        if (inventory->limited) break;
        for (b=call->first_binding;b<call->first_binding+call->binding_count;++b) {
            const GD_RuleBinding *map=&source->bindings[b];
            /* Charge conservative binary lookup ceiling; native compiled map
               remains authority, including whole-template maps larger than 64. */
            if (!charge(inventory,17)) break;
            if (gd_rule_call_participant(source,i,map->callee)!=map->caller) { inventory->source_incomplete=1; continue; }
            assert(map->caller>=0 && map->caller<rules->participant_count && map->callee>=0 && map->callee<rules->participant_count);
            if (!link(inventory,GD_ACTOR_MAPS_BINDING,inventory->participant_nodes[map->caller],inventory->participant_nodes[map->callee],i,b)) break;
        }
        if (inventory->limited) break;
    }
    inventory->unexamined_calls=source->call_count-i;
    return inventory;
}
void gd_actor_inventory_destroy(GD_ActorInventory *inventory) { if (inventory) inventory->allocator.release(inventory->allocator.context,inventory); }
int gd_actor_inventory_link_line(const GD_ActorInventory *inventory, int index) {
    const GD_ActorInventoryLink *edge; const GD_RuleAnalysis *source;
    if (!inventory || index<0 || index>=inventory->link_count) return 0;
    edge=&inventory->links[index]; source=inventory->source;
    if (edge->kind==GD_ACTOR_CONTAINS_BINDING) return source->rules->participants[edge->declaration].line;
    if (edge->kind==GD_ACTOR_DECLARED_NOUN) { const GD_RuleParticipant *binding=&source->rules->participants[edge->declaration]; return binding->lines[0]>0 ? binding->lines[0]:binding->line; }
    if (edge->kind==GD_ACTOR_DECLARED_AXIS || edge->kind==GD_ACTOR_DECLARED_CONTEXT_AXIS) return source->model->edges[edge->declaration].line;
    return source->calls[edge->declaration].line;
}
int gd_actor_inventory_node_id(const GD_ActorInventory *inventory, int index, char *text, size_t size) {
    const GD_ActorInventoryNode *item; const GD_Record *record; const char *kind,*id,*name="",*participant=""; int count;
    if (!text || !size) return 0; text[0]=0;
    if (!inventory || index<0 || index>=inventory->node_count) return 0;
    item=&inventory->nodes[index]; record=&inventory->source->model->records[item->record];
    kind=gd_actor_inventory_kind_name(item->kind); id=record->id ? record->id:"";
    if (item->case_index>=0) name=inventory->source->rules->cases[item->case_index].name;
    if (item->participant>=0) participant=inventory->source->rules->participants[item->participant].name;
    count=snprintf(text,size,"actor_inventory:%s:%llu:%s%llu:%s%llu:%s%llu:%s",kind,
        (unsigned long long)strlen(record->reference),record->reference,(unsigned long long)strlen(id),id,
        (unsigned long long)strlen(name),name,(unsigned long long)strlen(participant),participant);
    if (count<0 || (size_t)count>=size) { text[0]=0; return 0; }
    return 1;
}
