#include "gd_effect_query.h"
#include <assert.h>
#include <string.h>

static int charge(GD_EffectQuery *query, size_t amount) {
    if (query->limited || amount>(size_t)(query->work_limit-query->work)) { query->limited=1; return 0; }
    query->work+=(int)amount; return 1;
}
static int reverse_path(const GD_EffectGraph *graph, int activation, int *path) {
    const GD_ResourceGraph *source=graph->source; int count=0;
    if (activation<0 || activation>=graph->node_count || graph->nodes[activation].kind!=GD_EFFECT_ACTIVATION) return 0;
    for (;;) {
        const GD_ResourceNode *node;
        if (count==GD_MAX_RULE_EXPANSION_DEPTH) return 0;
        path[count++]=activation; node=&source->nodes[graph->nodes[activation].activation];
        if (!node->instance) return count;
        if (node->parent<0 || node->parent>=source->node_count) return 0;
        activation=graph->activations[node->parent];
        if (activation<0 || activation>=graph->node_count) return 0;
    }
}
static int eligible_path(const GD_EffectQuery *query, const int *path, int count) {
    const GD_EffectSelection *selection=&query->selection; int j;
    if (!count || (selection->root_case>=0 && query->graph->source->nodes[query->graph->nodes[path[count-1]].activation].case_index!=selection->root_case)) return 0;
    if (selection->activation_mask) for (j=0;j<count;++j) if (!selection->activation_mask[path[j]]) return 0;
    return 1;
}
GD_EffectQuery *gd_effect_query_build(const GD_EffectGraph *graph, GD_EffectSelection selection, int work_limit) {
    GD_EffectQuery *query; const GD_ResourceGraph *source; int i; size_t value_length=0;
    if (!graph || work_limit<0 || work_limit>GD_EFFECT_WORK) return NULL;
    source=graph->source;
    if (selection.value) { while (value_length<=GD_MAX_RULE_LINE && selection.value[value_length]) ++value_length; if (!value_length || value_length>GD_MAX_RULE_LINE) return NULL; }
    if (selection.target<-1 || selection.target>=graph->node_count || (selection.target>=0 && graph->nodes[selection.target].kind==GD_EFFECT_ACTIVATION) ||
        selection.root_case<-1 || selection.root_case>=source->rules->case_count || selection.noun<-1 || selection.noun>=source->model->record_count ||
        (selection.noun>=0 && source->model->records[selection.noun].role!=GD_NOUN) || selection.axis<-1 || selection.axis>=source->model->record_count ||
        (selection.axis>=0 && source->model->records[selection.axis].role!=GD_AXIS) || selection.kind<-1 || selection.kind==GD_EFFECT_ACTIVATION || selection.kind>GD_EFFECT_UNKNOWN) return NULL;
    query=graph->allocator.allocate(graph->allocator.context,sizeof(*query)); if (!query) return NULL;
    memset(query,0,sizeof(*query)); query->allocator=graph->allocator; query->graph=graph; query->selection=selection; query->work_limit=work_limit;
    query->source_incomplete=graph->source_incomplete || graph->limited || graph->unexamined_source_nodes || graph->unexamined_relations;
    for (i=0;i<graph->node_count;++i) query->incoming_call[i]=-1;
    for (i=0;i<graph->link_count;++i) {
        const GD_EffectLink *link=&graph->links[i];
        if (!charge(query,1)) break;
        if (link->kind==GD_EFFECT_INVOCATION) {
            assert(query->incoming_call[link->to]<0); query->incoming_call[link->to]=i;
        }
    }
    if (query->limited) { query->unexamined_links=graph->link_count; return query; }
    for (i=0;i<graph->link_count;++i) {
        const GD_EffectLink *link=&graph->links[i]; int path[GD_MAX_RULE_EXPANSION_DEPTH],count,j;
        if (!charge(query,1)) break;
        if (link->kind!=GD_EFFECT_DECLARATION || (selection.target>=0 && link->to!=selection.target)) continue;
        {
            const GD_EffectNode *node=&graph->nodes[link->to];
            if (selection.axis>=0 && node->axis!=selection.axis) continue;
            if (selection.kind>=0 && (int)node->kind!=selection.kind) continue;
            if (selection.value) {
                if (!charge(query,value_length+1)) break;
                if (!node->value || strcmp(node->value,selection.value)) continue;
            }
        }
        if (selection.noun>=0) {
            const GD_RuleParticipant *participant;
            if (!charge(query,1)) break;
            if (link->participant<0) { ++query->unresolved_targets; continue; }
            participant=&source->rules->participants[link->participant];
            if (participant->noun_resolution!=GD_RULE_RESOLVED) { ++query->unresolved_targets; continue; }
            if (participant->noun_record!=selection.noun) { ++query->excluded_target; continue; }
        }
        if (!charge(query,GD_MAX_RULE_EXPANSION_DEPTH+1)) break;
        count=reverse_path(graph,link->from,path);
        if (!count) { query->source_incomplete=1; ++query->unresolved_targets; continue; }
        if (!eligible_path(query,path,count)) { ++query->excluded_scope; continue; }
        query->links[query->count++]=i; ++query->incoming[link->to]; query->highlighted[i]=1;
        for (j=0;j<count;++j) {
            int call=query->incoming_call[path[j]]; query->context[path[j]]=1;
            if (call>=0) query->highlighted[call]=1;
        }
    }
    query->unexamined_links=graph->link_count-i;
    if (query->unresolved_targets) query->source_incomplete=1;
    return query;
}
void gd_effect_query_destroy(GD_EffectQuery *query) { if (query) query->allocator.release(query->allocator.context,query); }
int gd_effect_activation_path(const GD_EffectQuery *query, int activation, int *nodes, int capacity) {
    int reverse[GD_MAX_RULE_EXPANSION_DEPTH],count,i;
    if (!query || !nodes) return 0;
    count=reverse_path(query->graph,activation,reverse);
    if (!eligible_path(query,reverse,count) || capacity<count) return 0;
    for (i=0;i<count;++i) nodes[i]=reverse[count-i-1];
    return count;
}
int gd_effect_contributor_path(const GD_EffectQuery *query, int row, int *nodes, int capacity) {
    if (!query || row<0 || row>=query->count) return 0;
    return gd_effect_activation_path(query,query->graph->links[query->links[row]].from,nodes,capacity);
}
int gd_effect_select(const GD_EffectGraph *graph, const char *id, int work_limit) {
    char candidate[GD_EFFECT_ID_CAPACITY]; size_t length=0; int i,found=-1,work=0;
    if (!graph || !id || work_limit<0 || work_limit>GD_EFFECT_WORK) return -3;
    while (length<GD_EFFECT_ID_CAPACITY && id[length]) ++length;
    if (!length || length==GD_EFFECT_ID_CAPACITY) return -3;
    for (i=0;i<graph->node_count;++i) {
        size_t cost=length+GD_EFFECT_ID_CAPACITY+1;
        if (cost>(size_t)(work_limit-work)) return -3;
        work+=(int)cost;
        if (gd_effect_node_id(graph,i,candidate,sizeof(candidate)) && !strcmp(candidate,id)) {
            if (found>=0) return -2; found=i;
        }
    }
    return found;
}
