#include "gd_effect_graph.h"
#include <assert.h>
#include <string.h>

const char *gd_effect_kind_name(GD_EffectKind kind) {
    static const char *const names[]={"activation","state","gain","loss","no_change","quantity_declaration","preservation","unresolved_declaration"};
    return kind>=GD_EFFECT_ACTIVATION && kind<=GD_EFFECT_UNKNOWN ? names[kind]:NULL;
}
static int charge(GD_EffectGraph *graph, size_t work) {
    if (graph->limited || work>(size_t)(graph->work_limit-graph->work)) { graph->limited=1; return 0; }
    graph->work+=(int)work; return 1;
}
static int add_node(GD_EffectGraph *graph, GD_EffectKind kind, int activation, int axis, int relation, const char *value) {
    GD_EffectNode *node;
    if (!charge(graph,1) || graph->node_count==GD_EFFECT_NODES) { graph->limited=1; return -1; }
    node=&graph->nodes[graph->node_count]; node->kind=kind; node->activation=activation;
    node->axis=axis; node->relation=relation; node->value=value;
    if (kind==GD_EFFECT_UNKNOWN) ++graph->unknown_effects;
    return graph->node_count++;
}
static int result_node(GD_EffectGraph *graph, int relation) {
    const GD_ResourceGraph *source=graph->source;
    const GD_ResourceRelation *edge=&source->relations[relation];
    GD_EffectKind kind=GD_EFFECT_UNKNOWN; const char *value=NULL;
    int axis=-1,i,shared=0;
    if (edge->kind==GD_RESOURCE_STATE_WRITE) {
        const GD_RuleTarget *target=&source->rules->state_effects[edge->declaration].target;
        axis=target->axis_record;
        if (target->resolution==GD_RULE_RESOLVED && axis>=0 && target->state) { kind=GD_EFFECT_STATE; value=target->state; shared=1; }
    } else if (edge->kind==GD_RESOURCE_QUANTITY_WRITE) {
        const GD_RuleTarget *target=&source->rules->assignments[edge->declaration].target;
        axis=target->axis_record;
        if (target->resolution==GD_RULE_RESOLVED && axis>=0) {
            kind=GD_EFFECT_QUANTITY;
            if (edge->delta_recognized && edge->delta.known && edge->delta.checks_complete) {
                kind=edge->delta.number>0 ? GD_EFFECT_GAIN:edge->delta.number<0 ? GD_EFFECT_LOSS:GD_EFFECT_NO_CHANGE; shared=1;
            }
        }
    } else {
        assert(edge->kind==GD_RESOURCE_PRESERVATION);
        axis=source->nodes[edge->facet].axis; kind=GD_EFFECT_PRESERVE;
    }
    if (shared) for (i=0;i<graph->node_count;++i) {
        const GD_EffectNode *node=&graph->nodes[i];
        if (!charge(graph,1+(value ? strlen(value):0))) return -1;
        if (node->kind==kind && node->axis==axis && (!value || (node->value && !strcmp(value,node->value)))) return i;
    }
    /* Unknown/absolute quantities and preservation keep declaration identity.
       A common state/change concept never joins resource supplies or actors. */
    return add_node(graph,kind,-1,axis,shared ? -1:relation,value);
}
GD_EffectGraph *gd_effect_graph_build(const GD_ResourceGraph *source, int work_limit) {
    GD_EffectGraph *graph; int i;
    if (!source || !source->allocator.allocate || !source->allocator.release || work_limit<0 || work_limit>GD_EFFECT_WORK) return NULL;
    assert(source->node_count>=0 && source->node_count<=GD_RESOURCE_GRAPH_NODES);
    assert(source->relation_count>=0 && source->relation_count<=GD_RESOURCE_GRAPH_RELATIONS);
    graph=source->allocator.allocate(source->allocator.context,sizeof(*graph)); if (!graph) return NULL;
    memset(graph,0,sizeof(*graph)); graph->allocator=source->allocator; graph->source=source; graph->work_limit=work_limit;
    graph->source_incomplete=gd_resource_graph_incomplete(source);
    for (i=0;i<source->node_count;++i) graph->activations[i]=-1;
    for (i=0;i<source->node_count;++i) {
        if (!charge(graph,1)) break;
        if (source->nodes[i].kind==GD_RESOURCE_CASE) {
            graph->activations[i]=add_node(graph,GD_EFFECT_ACTIVATION,i,-1,-1,NULL);
            if (graph->activations[i]<0) break;
        }
    }
    graph->unexamined_source_nodes=source->node_count-i;
    for (i=0;i<source->relation_count && !graph->limited;++i) {
        const GD_ResourceRelation *edge=&source->relations[i]; GD_EffectLinkKind kind; int from,to;
        if (!charge(graph,1)) break;
        if (edge->contextual_input || (edge->kind!=GD_RESOURCE_STATE_WRITE && edge->kind!=GD_RESOURCE_QUANTITY_WRITE && edge->kind!=GD_RESOURCE_PRESERVATION && edge->kind!=GD_RESOURCE_INVOCATION)) continue;
        if (graph->link_count==GD_EFFECT_LINKS) { graph->limited=1; break; }
        from=graph->activations[edge->activation]; assert(from>=0);
        if (edge->kind==GD_RESOURCE_INVOCATION) {
            to=graph->activations[edge->facet]; assert(to>=0);
            kind=edge->expanded ? GD_EFFECT_INVOCATION:GD_EFFECT_CALL_FRONTIER;
        } else {
            to=result_node(graph,i); if (to<0) break;
            kind=GD_EFFECT_DECLARATION;
        }
        graph->links[graph->link_count++]=(GD_EffectLink){kind,from,to,i,edge->mapped_participant};
    }
    graph->unexamined_relations=source->relation_count-i;
    return graph;
}
void gd_effect_graph_destroy(GD_EffectGraph *graph) { if (graph) graph->allocator.release(graph->allocator.context,graph); }
void gd_effect_incoming(const GD_EffectGraph *graph, const unsigned char *mask, int *restrict counts) {
    int i;
    assert(graph && counts);
    memset(counts,0,sizeof(*counts)*(size_t)graph->node_count);
    for (i=0;i<graph->link_count;++i) {
        const GD_EffectLink *link=&graph->links[i];
        assert(link->from>=0 && link->from<graph->node_count && link->to>=0 && link->to<graph->node_count);
        if (link->kind==GD_EFFECT_DECLARATION && (!mask || mask[link->from])) ++counts[link->to];
    }
}
int gd_effect_node_id(const GD_EffectGraph *graph, int index, char *text, size_t size) {
    const GD_EffectNode *node; const GD_ResourceGraph *source; const char *axis; int count;
    if (!graph || index<0 || index>=graph->node_count || !text || !size) return 0;
    node=&graph->nodes[index]; source=graph->source;
    if (node->kind==GD_EFFECT_ACTIVATION) return gd_resource_node_id(source,node->activation,-1,text,size);
    axis=node->axis>=0 ? source->model->records[node->axis].reference:"";
    if (node->relation>=0) {
        const GD_ResourceRelation *edge=&source->relations[node->relation];
        const GD_RuleParticipant *actor=&source->rules->participants[edge->participant];
        char activation[GD_EFFECT_ID_CAPACITY]; int position;
        if (!gd_resource_node_id(source,edge->activation,-1,activation,sizeof(activation))) return 0;
        position=edge->declaration-(edge->kind==GD_RESOURCE_STATE_WRITE ? actor->first_state_effect:edge->kind==GD_RESOURCE_QUANTITY_WRITE ? actor->first_assignment:actor->first_preservation);
        const char *field=edge->kind==GD_RESOURCE_STATE_WRITE ? "state":edge->kind==GD_RESOURCE_QUANTITY_WRITE ? "quantity":"preservation";
        count=snprintf(text,size,"effect:declaration:%zu:%s:%zu:%s:%s:%d:%zu:%s",strlen(activation),activation,strlen(actor->name),actor->name,field,position,strlen(axis),axis);
    } else {
        const char *value=node->value ? node->value:"";
        count=snprintf(text,size,"effect:%s:%zu:%s:%zu:%s",gd_effect_kind_name(node->kind),strlen(axis),axis,strlen(value),value);
    }
    return count>=0 && (size_t)count<size;
}
int gd_effect_node_remap(const GD_EffectGraph *old, int index, const GD_EffectGraph *fresh) {
    char before[GD_EFFECT_ID_CAPACITY],after[GD_EFFECT_ID_CAPACITY]; int i,found=-1,activation=-1;
    const GD_EffectNode *node;
    if (!old || !fresh || !gd_effect_node_id(old,index,before,sizeof(before))) return -1;
    node=&old->nodes[index];
    if (node->kind==GD_EFFECT_ACTIVATION) activation=gd_resource_node_remap(old->source,node->activation,fresh->source);
    else if (node->relation>=0) activation=gd_resource_node_remap(old->source,old->source->relations[node->relation].activation,fresh->source);
    if ((node->kind==GD_EFFECT_ACTIVATION || node->relation>=0) && activation<0) return -1;
    for (i=0;i<fresh->node_count;++i) {
        const GD_EffectNode *candidate=&fresh->nodes[i];
        if (candidate->kind!=node->kind) continue;
        if (node->kind==GD_EFFECT_ACTIVATION && candidate->activation!=activation) continue;
        if (node->relation>=0 && (candidate->relation<0 || fresh->source->relations[candidate->relation].activation!=activation)) continue;
        if (!gd_effect_node_id(fresh,i,after,sizeof(after)) || strcmp(before,after)) continue;
        if (found>=0) return -1;
        found=i;
    }
    return found;
}
