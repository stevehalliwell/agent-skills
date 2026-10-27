#include "gd_resource_graph.h"
#include <string.h>

static int charge(GD_ResourceGraph *graph, size_t work) {
    if (graph->limited) return 0;
    if (work > (size_t)(GD_RESOURCE_GRAPH_WORK-graph->work)) { graph->limited=1; return 0; }
    graph->work+=(int)work; return 1;
}
static int add_case(GD_ResourceGraph *graph, int case_index) {
    GD_ResourceNode *node;
    if (!charge(graph,1) || graph->node_count==GD_RESOURCE_GRAPH_NODES) { graph->limited=1; return -1; }
    node=&graph->nodes[graph->node_count];
    node->kind=GD_RESOURCE_CASE; node->case_index=case_index; node->participant=node->axis=node->pool=node->record=-1;
    return graph->node_count++;
}
static int role_node(GD_ResourceGraph *graph, const char *reference, int record, GD_RuleResolution resolution) {
    GD_ResourceNode *node; int i;
    for (i=0;i<graph->node_count;++i) {
        const GD_ResourceNode *existing=&graph->nodes[i];
        if (!charge(graph,strlen(reference)+1)) return -1;
        if (existing->kind!=GD_RESOURCE_ROLE) continue;
        if (record>=0 && existing->record==record) return i;
        if (record<0 && existing->record<0 && existing->resolution==resolution &&
            !strcmp(reference,existing->pool_reference)) return i;
    }
    if (!charge(graph,1) || graph->node_count==GD_RESOURCE_GRAPH_NODES) { graph->limited=1; return -1; }
    node=&graph->nodes[graph->node_count]; node->kind=GD_RESOURCE_ROLE; node->case_index=node->participant=node->axis=node->pool=-1;
    node->record=record; node->pool_reference=reference; node->resolution=resolution;
    return graph->node_count++;
}
static int facet(GD_ResourceGraph *graph, int case_index, int participant, GD_RuleTarget target) {
    const GD_RuleParticipant *actor=&graph->rules->participants[participant];
    const char *reference=NULL; int i, pool=-1, uncertain_pool=0, role=-1, axis=target.resolution==GD_RULE_RESOLVED ? target.axis_record:-1;
    GD_ResourceNode *node;
    if (axis>=0) for (i=actor->first_pool;i<actor->first_pool+actor->pool_count;++i) {
        const GD_RulePool *item=&graph->rules->pools[i];
        if (!charge(graph,1)) return -1;
        if (item->target.resolution!=GD_RULE_RESOLVED) { uncertain_pool=1; continue; }
        if (item->target.axis_record!=axis) continue;
        if (item->identity_resolution==GD_RULE_RESOLVED || item->identity_resolution==GD_RULE_MISSING || item->identity_resolution==GD_RULE_UNLOADED) {
            reference=item->reference; pool=i;
        } else { reference=NULL; pool=-1; uncertain_pool=1; break; }
    }
    /* An unresolved pool axis could name this facet. Do not select a shared
       identity while that competing binding remains unexamined. */
    if (uncertain_pool) { reference=NULL; pool=-1; }
    /* An unresolved axis cannot safely merge by spelling or participant. */
    if (axis>=0) for (i=0;i<graph->node_count;++i) {
        const GD_ResourceNode *existing=&graph->nodes[i];
        if (!charge(graph,reference ? strlen(reference)+1:1)) return -1;
        if (existing->kind!=GD_RESOURCE_FACET || existing->axis!=axis) continue;
        if (reference && existing->pool_reference && !strcmp(reference,existing->pool_reference)) return i;
        if (!reference && !existing->pool_reference && existing->participant==participant) return i;
    }
    if (pool>=0) {
        const GD_RulePool *binding=&graph->rules->pools[pool];
        if (graph->relation_count>=GD_RESOURCE_GRAPH_RELATIONS-1) { graph->limited=1; return -1; }
        role=role_node(graph,reference,binding->resource_record,binding->resource_resolution);
        if (role<0) return -1;
    }
    if (!charge(graph,1) || graph->node_count==GD_RESOURCE_GRAPH_NODES) { graph->limited=1; return -1; }
    i=graph->node_count++; node=&graph->nodes[i]; node->kind=GD_RESOURCE_FACET; node->case_index=case_index;
    node->participant=participant; node->axis=axis; node->pool=pool; node->record=-1; node->pool_reference=reference;
    if (axis<0 || uncertain_pool || (pool>=0 && graph->rules->pools[pool].resource_resolution!=GD_RULE_RESOLVED)) ++graph->unresolved_facets;
    if (role>=0) {
        GD_ResourceRelation *binding=&graph->relations[graph->relation_count++];
        binding->kind=GD_RESOURCE_ROLE_BINDING; binding->facet=i; binding->activation=role;
        binding->participant=participant; binding->declaration=pool; binding->line=graph->rules->pools[pool].line;
    }
    return i;
}
static void relation(GD_ResourceGraph *graph, int case_index, int activation, int participant,
                     GD_RuleTarget target, GD_ResourceRelationKind kind, int declaration, int line) {
    GD_ResourceRelation *item;
    int target_node;
    if (graph->limited) return;
    if (graph->relation_count==GD_RESOURCE_GRAPH_RELATIONS) { graph->limited=1; return; }
    target_node=facet(graph,case_index,participant,target);
    if (target_node<0) return;
    item=&graph->relations[graph->relation_count++];
    item->kind=kind; item->facet=target_node; item->activation=activation;
    item->participant=participant; item->declaration=declaration; item->line=line;
    if (kind==GD_RESOURCE_QUANTITY_WRITE) {
        int quantity=graph->rules->assignments[declaration].quantity;
        int tokens=quantity>=0 && quantity<graph->rules->quantity_count ? graph->rules->quantities[quantity].token_count:0;
        if (!charge(graph,(size_t)(tokens+1)*GD_RULE_NUMERIC_TOKEN_WORK)) return;
        item->delta=gd_rule_quantity_delta(graph->rules,quantity,&item->delta_recognized);
    }
}
GD_ResourceGraph *gd_resource_graph_build(GD_Model *model, const GD_Rules *rules) {
    GD_ResourceGraph *graph;
    int c,p,i,k, generation=0, seen[GD_MAX_RECORDS]={0};
    if (!model || !rules || rules->model!=model || !model->allocator.allocate || !model->allocator.release) return NULL;
    graph=model->allocator.allocate(model->allocator.context,sizeof(*graph));
    if (!graph) { gd_error(model,"resources.memory","Cannot allocate bounded resource graph",NULL,0); return NULL; }
    memset(graph,0,sizeof(*graph)); graph->allocator=model->allocator; graph->model=model; graph->rules=rules;
    for (i=0;i<model->record_count;++i) if (model->records[i].role==GD_RESOURCE) {
        if (role_node(graph,model->records[i].reference,i,GD_RULE_RESOLVED)<0) {
            graph->unexamined_roles=model->record_count-i; graph->unexamined_cases=rules->case_count; return graph;
        }
    }
    for (c=0;c<rules->case_count;++c) {
        const GD_RuleCase *item=&rules->cases[c]; int activation=add_case(graph,c);
        if (activation<0) { graph->unexamined_cases=rules->case_count-c; break; }
        if (item->unknown_conditions || item->unknown_effects) ++graph->unknown_case_coverage;
        for (p=item->first_participant;p<item->first_participant+item->participant_count && !graph->limited;++p) {
            const GD_RuleParticipant *actor=&rules->participants[p];
            if (actor->causes) ++graph->composition_unexamined;
            if (actor->preserve && !actor->preservation_count) ++graph->preservation_unexamined;
            for (k=0;k<2 && !graph->limited;++k) {
                const GD_RuleCondition *condition=k ? &actor->skip_condition:&actor->required_condition;
                for (i=condition->first_token;i<condition->first_token+condition->token_count && !graph->limited;++i) {
                    const GD_RuleConditionToken *token=&rules->condition_tokens[i];
                    if (!charge(graph,1)) break;
                    if (token->operation!=GD_RULE_OP_STATE && token->operation!=GD_RULE_OP_COMPARE) continue;
                    relation(graph,c,activation,p,token->target,k ? GD_RESOURCE_EXCLUSION:GD_RESOURCE_REQUIREMENT,i,actor->lines[k ? 2:1]);
                }
            }
            for (i=actor->first_state_effect;i<actor->first_state_effect+actor->state_effect_count && !graph->limited;++i) {
                const GD_RuleStateEffect *effect=&rules->state_effects[i];
                relation(graph,c,activation,p,effect->target,GD_RESOURCE_STATE_WRITE,i,effect->line);
            }
            for (i=actor->first_assignment;i<actor->first_assignment+actor->assignment_count && !graph->limited;++i) {
                const GD_RuleAssignment *assignment=&rules->assignments[i];
                relation(graph,c,activation,p,assignment->target,GD_RESOURCE_QUANTITY_WRITE,i,assignment->line);
            }
            for (i=actor->first_preservation;i<actor->first_preservation+actor->preservation_count && !graph->limited;++i) {
                const GD_RulePreservation *promise=&rules->preservations[i];
                if (!charge(graph,1)) break;
                if (promise->kind==GD_PRESERVE_AXIS) {
                    relation(graph,c,activation,p,promise->target,GD_RESOURCE_PRESERVATION,i,promise->line);
                } else {
                    int edge, owner=actor->noun_record;
                    if (actor->noun_resolution!=GD_RULE_RESOLVED || owner<0) { ++graph->preservation_unexamined; continue; }
                    if (!rules->actor_axes_complete[owner]) ++graph->preservation_unexamined;
                    ++generation;
                    for (edge=rules->actor_first[owner];edge>=0 && !graph->limited;edge=rules->next_axis[edge]) {
                        int axis=model->edges[edge].target, before=graph->relation_count;
                        GD_RuleTarget target;
                        if (!charge(graph,1)) break;
                        if (rules->axis_resolution[edge]!=GD_RULE_RESOLVED || axis<0 || !rules->axes[axis].valid) {
                            ++graph->preservation_unexamined; continue;
                        }
                        if (seen[axis]==generation) continue;
                        seen[axis]=generation; target=(GD_RuleTarget){GD_RULE_RESOLVED,axis,NULL};
                        relation(graph,c,activation,p,target,GD_RESOURCE_PRESERVATION,i,promise->line);
                        if (graph->relation_count>before)
                            graph->relations[graph->relation_count-1].conditional=promise->kind==GD_PRESERVE_OTHER;
                    }
                }
            }
        }
        if (graph->limited) { graph->unexamined_cases=rules->case_count-c; break; }
    }
    return graph;
}
int gd_resource_graph_bind_calls(GD_ResourceGraph *graph, const GD_RuleAnalysis *analysis) {
    int case_nodes[GD_MAX_RULE_CASES], i,p,c;
    if (!graph || !analysis || graph->calls_bound || graph->limited || graph->unexamined_cases ||
        analysis->model!=graph->model || analysis->rules!=graph->rules) return 0;
    for (i=0;i<GD_MAX_RULE_CASES;++i) case_nodes[i]=-1;
    for (i=0;i<graph->node_count;++i) {
        const GD_ResourceNode *node=&graph->nodes[i];
        if (node->kind==GD_RESOURCE_CASE) case_nodes[node->case_index]=i;
    }
    graph->calls=analysis; graph->calls_bound=1; graph->composition_unexamined=0;
    for (p=0;p<graph->rules->participant_count;++p) {
        if (!charge(graph,1)) break;
        if (graph->rules->participants[p].causes && !analysis->calls_complete[p]) ++graph->composition_unexamined;
    }
    for (i=0;i<analysis->call_count && !graph->limited;++i) {
        const GD_RuleCall *call=&analysis->calls[i];
        if (!charge(graph,1)) break;
        if (call->resolution!=GD_RULE_RESOLVED || !call->case_count) { ++graph->composition_unexamined; continue; }
        for (c=call->first_case;c<call->first_case+call->case_count;++c) {
            GD_ResourceRelation *edge;
            if (!charge(graph,1) || graph->relation_count==GD_RESOURCE_GRAPH_RELATIONS) { graph->limited=1; break; }
            if (call->caller_case<0 || call->caller_case>=graph->rules->case_count || c<0 || c>=graph->rules->case_count ||
                case_nodes[call->caller_case]<0 || case_nodes[c]<0) { ++graph->composition_unexamined; continue; }
            edge=&graph->relations[graph->relation_count++];
            edge->kind=GD_RESOURCE_INVOCATION; edge->activation=case_nodes[call->caller_case]; edge->facet=case_nodes[c];
            edge->participant=call->caller; edge->declaration=i; edge->line=call->line;
        }
    }
    if (graph->limited) ++graph->composition_unexamined;
    return !graph->limited;
}
void gd_resource_graph_destroy(GD_ResourceGraph *graph) {
    if (graph) graph->allocator.release(graph->allocator.context,graph);
}
