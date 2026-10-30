#include "gd_resource_graph.h"
#include <string.h>

static int charge(GD_ResourceGraph *graph, size_t work) {
    if (graph->limited) return 0;
    if (work > (size_t)(GD_RESOURCE_GRAPH_WORK-graph->work)) { graph->limited=1; return 0; }
    graph->work+=(int)work; return 1;
}
static const char *case_reference(const GD_ResourceGraph *graph, int c) {
    return graph->model->records[graph->rules->sources[graph->rules->cases[c].source].record].reference;
}
static void digest(unsigned long long *value, const char *text) {
    const unsigned char *p=(const unsigned char *)text;
    for (;*p;++p) { value[0]=(value[0]^*p)*1099511628211ULL; value[1]=(value[1]+*p)*14029467366897019727ULL; }
    value[0]=(value[0]^255)*1099511628211ULL; value[1]=(value[1]+257)*14029467366897019727ULL;
}
static int add_case(GD_ResourceGraph *graph, int c, int parent, int call, int ordinal) {
    GD_ResourceNode *node; int i; char number[24];
    const char *reference=case_reference(graph,c), *name=graph->rules->cases[c].name;
    if (!charge(graph,strlen(reference)+strlen(name)+1) || graph->node_count==GD_RESOURCE_GRAPH_NODES) { graph->limited=1; return -1; }
    node=&graph->nodes[graph->node_count];
    node->kind=GD_RESOURCE_CASE; node->case_index=c; node->participant=node->axis=node->pool=node->record=-1;
    node->parent=parent; node->call=call; node->ordinal=ordinal; node->instance=parent>=0;
    node->depth=parent>=0 ? graph->nodes[parent].depth+1:0;
    node->identity[0]=14695981039346656037ULL; node->identity[1]=7809847782465536322ULL;
    if (parent>=0) {
        const char *actor=graph->rules->participants[graph->calls->calls[call].caller].name;
        node->identity[0]=graph->nodes[parent].identity[0]; node->identity[1]=graph->nodes[parent].identity[1];
        if (!charge(graph,strlen(actor)+24)) return -1;
        digest(node->identity,actor); snprintf(number,sizeof(number),"%d",ordinal); digest(node->identity,number);
    }
    digest(node->identity,reference); digest(node->identity,name);
    /* Digests shorten opaque IDs, never establish equality. Fail closed on a
       duplicate digest rather than admitting ambiguous selector identities. */
    if (parent>=0) for (i=0;i<graph->node_count;++i) {
        const GD_ResourceNode *other=&graph->nodes[i];
        if (!charge(graph,1)) return -1;
        if (other->kind==GD_RESOURCE_CASE && other->instance && other->identity[0]==node->identity[0] && other->identity[1]==node->identity[1]) {
            ++graph->composition_unexamined; ++graph->identity_collisions; graph->limited=1; return -1;
        }
    }
    if (parent>=0) ++graph->instance_count;
    return graph->node_count++;
}
static int role_node(GD_ResourceGraph *graph, const char *reference, int record, GD_RuleResolution resolution) {
    GD_ResourceNode *node; int i;
    for (i=0;i<graph->node_count;++i) {
        const GD_ResourceNode *existing=&graph->nodes[i];
        if (!charge(graph,strlen(reference)+1)) return -1;
        if (existing->kind!=GD_RESOURCE_ROLE) continue;
        if (record>=0 && existing->record==record) return i;
        if (record<0 && existing->record<0 && existing->resolution==resolution && !strcmp(reference,existing->pool_reference)) return i;
    }
    if (!charge(graph,1) || graph->node_count==GD_RESOURCE_GRAPH_NODES) { graph->limited=1; return -1; }
    node=&graph->nodes[graph->node_count]; node->kind=GD_RESOURCE_ROLE; node->case_index=node->participant=node->axis=node->pool=-1;
    node->record=record; node->pool_reference=reference; node->resolution=resolution;
    return graph->node_count++;
}
int gd_resource_instance_actor(const GD_ResourceGraph *graph, int activation, int participant) {
    int depth=0;
    if (!graph || activation<0 || activation>=graph->node_count || graph->nodes[activation].kind!=GD_RESOURCE_CASE) return -1;
    while (graph->nodes[activation].instance) {
        const GD_ResourceNode *node=&graph->nodes[activation];
        const GD_RuleCall *call; int low,high,found=-1;
        if (!graph->calls || ++depth>=GD_MAX_RULE_EXPANSION_DEPTH || node->call<0 || node->call>=graph->calls->call_count || node->parent<0 || node->parent>=activation) return -1;
        {
            const GD_RuleCase *item=&graph->rules->cases[node->case_index];
            if (participant<item->first_participant || participant>=item->first_participant+item->participant_count) return -1;
        }
        call=&graph->calls->calls[node->call];
        /* Compiler orders bindings by selected case and participant index.
           A call may select many cases; binding_count is not capped at 64. */
        low=call->first_binding; high=low+call->binding_count;
        if (low<0 || call->binding_count<0 || high>graph->calls->binding_count) return -1;
        while (low<high) {
            int middle=low+(high-low)/2;
            if (graph->calls->bindings[middle].callee<participant) low=middle+1; else high=middle;
        }
        if (low<call->first_binding+call->binding_count && graph->calls->bindings[low].callee==participant) {
            if (low+1<call->first_binding+call->binding_count && graph->calls->bindings[low+1].callee==participant) return -1;
            found=graph->calls->bindings[low].caller;
        }
        if (found<0) return -1;
        participant=found; activation=node->parent;
    }
    {
        const GD_RuleCase *item=&graph->rules->cases[graph->nodes[activation].case_index];
        if (participant<item->first_participant || participant>=item->first_participant+item->participant_count) return -1;
    }
    return participant;
}
static int mapped_actor(GD_ResourceGraph *graph, int activation, int participant) {
    if (!charge(graph,(size_t)(graph->nodes[activation].depth+1)*GD_MAX_RULE_CALL_BINDINGS)) return -1;
    return gd_resource_instance_actor(graph,activation,participant);
}
static int root_node(const GD_ResourceGraph *graph, int activation) {
    int depth=0;
    while (graph->nodes[activation].instance && depth++<GD_MAX_RULE_EXPANSION_DEPTH) activation=graph->nodes[activation].parent;
    return activation;
}
/* Pool declarations follow explicit actor substitutions only in this placement.
   Never change the root participant or promote a descendant binding into its
   unrelated declarations. Distinct explicit bindings are conflicts, not winners. */
static int binding(GD_ResourceGraph *graph, int activation, int identity_actor, int axis, int *uncertain, int *conflict) {
    int current=activation,depth=0,selected=-1;
    if (!graph->rules->pool_count) return -1;
    do {
        const GD_RuleCase *item=&graph->rules->cases[graph->nodes[current].case_index]; int p,i;
        for (p=item->first_participant;p<item->first_participant+item->participant_count;++p) {
            const GD_RuleParticipant *actor=&graph->rules->participants[p];
            if (!charge(graph,1)) return -1;
            if (!actor->pool_count || mapped_actor(graph,current,p)!=identity_actor) continue;
            for (i=actor->first_pool;i<actor->first_pool+actor->pool_count;++i) {
                const GD_RulePool *pool=&graph->rules->pools[i];
                if (!charge(graph,strlen(pool->reference)+1)) return -1;
                if (pool->target.resolution!=GD_RULE_RESOLVED) { *uncertain=1; continue; }
                if (pool->target.axis_record!=axis) continue;
                if (pool->identity_resolution!=GD_RULE_RESOLVED && pool->identity_resolution!=GD_RULE_MISSING && pool->identity_resolution!=GD_RULE_UNLOADED) { *uncertain=1; continue; }
                if (selected>=0 && strcmp(graph->rules->pools[selected].reference,pool->reference)) *conflict=1;
                else selected=i;
            }
        }
        if (!graph->nodes[current].instance) break;
        current=graph->nodes[current].parent;
    } while (++depth<GD_MAX_RULE_EXPANSION_DEPTH && !graph->limited);
    return selected;
}
static int participant_case(const GD_Rules *rules, int p) {
    int c;
    for (c=0;c<rules->case_count;++c) if (p>=rules->cases[c].first_participant && p<rules->cases[c].first_participant+rules->cases[c].participant_count) return c;
    return -1;
}
static int facet(GD_ResourceGraph *graph, int activation, int identity_actor, GD_RuleTarget target) {
    const char *reference=NULL; int i,pool=-1,uncertain=0,conflict=0,role=-1,root=root_node(graph,activation);
    int axis=target.resolution==GD_RULE_RESOLVED ? target.axis_record:-1,placement=0;
    GD_ResourceNode *node;
    if (axis>=0) pool=binding(graph,activation,identity_actor,axis,&uncertain,&conflict);
    if (graph->limited) return -1;
    if (conflict && !graph->nodes[activation].binding_conflict) { graph->nodes[activation].binding_conflict=1; ++graph->binding_conflicts; }
    if (uncertain || conflict) pool=-1;
    if (pool>=0) reference=graph->rules->pools[pool].reference;
    if (axis<0 || uncertain || conflict) placement=graph->nodes[activation].instance ? activation+1:0;
    if (axis>=0) for (i=0;i<graph->node_count;++i) {
        const GD_ResourceNode *existing=&graph->nodes[i];
        if (!charge(graph,reference ? strlen(reference)+1:1)) return -1;
        if (existing->kind!=GD_RESOURCE_FACET || existing->axis!=axis || existing->identity_unresolved!=(uncertain || conflict)) continue;
        if (reference && existing->pool_reference && !strcmp(reference,existing->pool_reference)) return i;
        if (!reference && !existing->pool_reference && existing->participant==identity_actor && existing->placement==placement) return i;
    }
    if (pool>=0) {
        const GD_RulePool *item=&graph->rules->pools[pool];
        if (graph->relation_count>=GD_RESOURCE_GRAPH_RELATIONS-1) { graph->limited=1; return -1; }
        role=role_node(graph,reference,item->resource_record,item->resource_resolution); if (role<0) return -1;
    }
    if (!charge(graph,1) || graph->node_count==GD_RESOURCE_GRAPH_NODES) { graph->limited=1; return -1; }
    i=graph->node_count++; node=&graph->nodes[i]; node->kind=GD_RESOURCE_FACET; node->case_index=graph->nodes[root].case_index;
    node->participant=identity_actor; node->axis=axis; node->pool=pool; node->record=-1; node->pool_reference=reference;
    node->placement=placement; node->identity_unresolved=uncertain || conflict;
    if (axis<0 || uncertain || conflict || (pool>=0 && graph->rules->pools[pool].resource_resolution!=GD_RULE_RESOLVED)) ++graph->unresolved_facets;
    if (role>=0) {
        GD_ResourceRelation *edge=&graph->relations[graph->relation_count++];
        const GD_RulePool *item=&graph->rules->pools[pool]; int owner;
        if (!charge(graph,graph->rules->participant_count)) return -1;
        for (owner=0;owner<graph->rules->participant_count;++owner)
            if (pool>=graph->rules->participants[owner].first_pool && pool<graph->rules->participants[owner].first_pool+graph->rules->participants[owner].pool_count) break;
        if (owner==graph->rules->participant_count) { graph->limited=1; return -1; }
        edge->kind=GD_RESOURCE_ROLE_BINDING; edge->facet=i; edge->activation=role; edge->participant=owner;
        edge->declaration=pool; edge->line=item->line; edge->mapped_participant=identity_actor;
        if (!charge(graph,graph->rules->case_count)) return -1;
        edge->source_case=participant_case(graph->rules,owner);
    }
    return i;
}
static void relation(GD_ResourceGraph *graph, int c, int activation, int participant, GD_RuleTarget target,
                     GD_ResourceRelationKind kind, int declaration, int line) {
    GD_ResourceRelation *item; int target_node,identity_actor;
    if (graph->limited) return;
    if (graph->relation_count==GD_RESOURCE_GRAPH_RELATIONS) { graph->limited=1; return; }
    identity_actor=mapped_actor(graph,activation,participant);
    if (identity_actor<0) { graph->nodes[activation].frontier=1; ++graph->composition_unexamined; return; }
    target_node=facet(graph,activation,identity_actor,target); if (target_node<0) return;
    item=&graph->relations[graph->relation_count++]; item->kind=kind; item->facet=target_node; item->activation=activation;
    item->participant=participant; item->declaration=declaration; item->line=line; item->source_case=c; item->mapped_participant=identity_actor;
    if (kind==GD_RESOURCE_QUANTITY_WRITE) {
        int quantity=graph->rules->assignments[declaration].quantity;
        int tokens=quantity>=0 && quantity<graph->rules->quantity_count ? graph->rules->quantities[quantity].token_count:0;
        if (!charge(graph,(size_t)(tokens+1)*GD_RULE_NUMERIC_TOKEN_WORK)) return;
        item->delta=gd_rule_quantity_delta(graph->rules,quantity,&item->delta_recognized);
    }
}
static void declarations(GD_ResourceGraph *graph, int c, int activation, int *seen, int *generation) {
    const GD_Rules *rules=graph->rules; const GD_RuleCase *item=&rules->cases[c]; int p,i,k;
    int unknown=item->unknown_conditions || item->unknown_effects;
    graph->nodes[activation].first_relation=graph->relation_count;
    if (unknown) ++graph->unknown_case_coverage;
    for (p=item->first_participant;p<item->first_participant+item->participant_count && !graph->limited;++p) {
        const GD_RuleParticipant *actor=&rules->participants[p];
        if (actor->causes && !graph->calls_bound) ++graph->composition_unexamined;
        if ((actor->preserve && !actor->preservation_count) || (actor->incomplete_effect_fields & (1u<<5))) ++graph->preservation_unexamined;
        if (actor->incomplete_effect_fields && !unknown) { unknown=1; ++graph->unknown_case_coverage; }
        for (k=0;k<2 && !graph->limited;++k) {
            const GD_RuleCondition *condition=k ? &actor->skip_condition:&actor->required_condition;
            for (i=condition->first_token;i<condition->first_token+condition->token_count && !graph->limited;++i) {
                const GD_RuleConditionToken *token=&rules->condition_tokens[i];
                if (!charge(graph,1)) break;
                if (token->operation==GD_RULE_OP_STATE || token->operation==GD_RULE_OP_COMPARE)
                    relation(graph,c,activation,p,token->target,k ? GD_RESOURCE_EXCLUSION:GD_RESOURCE_REQUIREMENT,i,actor->lines[k ? 2:1]);
            }
        }
        for (i=actor->first_state_effect;i<actor->first_state_effect+actor->state_effect_count && !graph->limited;++i)
            relation(graph,c,activation,p,rules->state_effects[i].target,GD_RESOURCE_STATE_WRITE,i,rules->state_effects[i].line);
        for (i=actor->first_assignment;i<actor->first_assignment+actor->assignment_count && !graph->limited;++i)
            relation(graph,c,activation,p,rules->assignments[i].target,GD_RESOURCE_QUANTITY_WRITE,i,rules->assignments[i].line);
        for (i=actor->first_preservation;i<actor->first_preservation+actor->preservation_count && !graph->limited;++i) {
            const GD_RulePreservation *promise=&rules->preservations[i];
            if (!charge(graph,1)) break;
            if (promise->kind==GD_PRESERVE_AXIS) relation(graph,c,activation,p,promise->target,GD_RESOURCE_PRESERVATION,i,promise->line);
            else {
                int edge,owner=actor->noun_record;
                if (actor->noun_resolution!=GD_RULE_RESOLVED || owner<0) { ++graph->preservation_unexamined; continue; }
                if (!rules->actor_axes_complete[owner]) ++graph->preservation_unexamined;
                ++*generation;
                for (edge=rules->actor_first[owner];edge>=0 && !graph->limited;edge=rules->next_axis[edge]) {
                    int axis=graph->model->edges[edge].target,before=graph->relation_count;
                    if (!charge(graph,1)) break;
                    if (rules->axis_resolution[edge]!=GD_RULE_RESOLVED || axis<0 || !rules->axes[axis].valid) { ++graph->preservation_unexamined; continue; }
                    if (seen[axis]==*generation) continue; seen[axis]=*generation;
                    relation(graph,c,activation,p,(GD_RuleTarget){GD_RULE_RESOLVED,axis,NULL},GD_RESOURCE_PRESERVATION,i,promise->line);
                    if (graph->relation_count>before) graph->relations[graph->relation_count-1].conditional=promise->kind==GD_PRESERVE_OTHER;
                }
            }
        }
    }
    graph->nodes[activation].relation_count=graph->relation_count-graph->nodes[activation].first_relation;
}
GD_ResourceGraph *gd_resource_graph_build(GD_Model *model, const GD_Rules *rules) {
    GD_ResourceGraph *graph; int c,i,generation=0,seen[GD_MAX_RECORDS]={0};
    if (!model || !rules || rules->model!=model || !model->allocator.allocate || !model->allocator.release) return NULL;
    graph=model->allocator.allocate(model->allocator.context,sizeof(*graph));
    if (!graph) { gd_error(model,"resources.memory","Cannot allocate bounded resource graph",NULL,0); return NULL; }
    memset(graph,0,sizeof(*graph)); graph->allocator=model->allocator; graph->model=model; graph->rules=rules;
    for (i=0;i<model->record_count;++i) if (model->records[i].role==GD_RESOURCE && role_node(graph,model->records[i].reference,i,GD_RULE_RESOLVED)<0) {
        graph->unexamined_roles=model->record_count-i; graph->unexamined_cases=rules->case_count; return graph;
    }
    for (c=0;c<rules->case_count;++c) {
        int activation=add_case(graph,c,-1,-1,0);
        if (activation>=0) declarations(graph,c,activation,seen,&generation);
        if (graph->limited) { graph->unexamined_cases=rules->case_count-c; break; }
    }
    return graph;
}
/* Re-express ancestor enabling/debit inputs in this concrete placement context.
   They are not callee effects. Other ancestor declarations remain side bundles,
   addressed through parent IDs. This preserves invocation-scoped pool evidence
   without globally retargeting the caller's own declarations. */
static void ancestor_inputs(GD_ResourceGraph *graph, int activation, const int *case_nodes) {
    int parent=graph->nodes[activation].parent,depth=0;
    while (parent>=0 && depth++<GD_MAX_RULE_EXPANSION_DEPTH && !graph->limited) {
        int source=case_nodes[graph->nodes[parent].case_index],i;
        const GD_ResourceNode *node=&graph->nodes[source];
        for (i=node->first_relation;i<node->first_relation+node->relation_count && !graph->limited;++i) {
            GD_ResourceRelation copy=graph->relations[i]; int actor,other;
            if (!charge(graph,1)) break;
            if (copy.kind!=GD_RESOURCE_REQUIREMENT && copy.kind!=GD_RESOURCE_EXCLUSION && !(copy.kind==GD_RESOURCE_QUANTITY_WRITE && copy.delta_recognized && copy.delta.known && copy.delta.checks_complete && copy.delta.number<0)) continue;
            actor=mapped_actor(graph,parent,copy.participant); if (actor<0) continue;
            other=facet(graph,activation,actor,(GD_RuleTarget){graph->nodes[copy.facet].axis>=0 ? GD_RULE_RESOLVED:GD_RULE_MISSING,graph->nodes[copy.facet].axis,NULL});
            if (other<0) break;
            if (graph->relation_count==GD_RESOURCE_GRAPH_RELATIONS) { graph->limited=1; break; }
            copy.activation=activation; copy.facet=other; copy.contextual_input=1; copy.mapped_participant=actor;
            graph->relations[graph->relation_count++]=copy;
        }
        parent=graph->nodes[parent].instance ? graph->nodes[parent].parent:-1;
    }
}
int gd_resource_graph_bind_calls(GD_ResourceGraph *graph, const GD_RuleAnalysis *analysis) {
    int case_nodes[GD_MAX_RULE_CASES],seen[GD_MAX_RECORDS]={0},generation=0,i,p,c,activation;
    if (!graph || !analysis || graph->calls_bound || graph->limited || graph->unexamined_cases || analysis->model!=graph->model || analysis->rules!=graph->rules) return 0;
    for (i=0;i<GD_MAX_RULE_CASES;++i) case_nodes[i]=-1;
    for (i=0;i<graph->node_count;++i) if (graph->nodes[i].kind==GD_RESOURCE_CASE) case_nodes[graph->nodes[i].case_index]=i;
    graph->calls=analysis; graph->calls_bound=1; graph->composition_unexamined=0;
    /* Appended placements form a bounded breadth-first queue. Parents precede
       children; no recursive evaluator, simulated inventory or state matching. */
    for (activation=0;activation<graph->node_count && !graph->limited;++activation) {
        const GD_RuleCase *item;
        if (!charge(graph,1)) break;
        if (graph->nodes[activation].kind!=GD_RESOURCE_CASE) continue;
        item=&graph->rules->cases[graph->nodes[activation].case_index];
        for (p=item->first_participant;p<item->first_participant+item->participant_count && !graph->limited;++p) {
            int ordinal=-1,probe;
            if (!charge(graph,1)) break;
            for (probe=analysis->call_first[p];probe>=0;probe=analysis->calls[probe].next) { if (!charge(graph,1)) break; ++ordinal; }
            if (graph->rules->participants[p].causes && !analysis->calls_complete[p]) { ++graph->composition_unexamined; graph->nodes[activation].frontier=1; }
            for (i=analysis->call_first[p];i>=0 && !graph->limited;i=analysis->calls[i].next,--ordinal) {
                const GD_RuleCall *call=&analysis->calls[i];
                if (!charge(graph,1)) break;
                if (call->resolution!=GD_RULE_RESOLVED || !call->case_count) { ++graph->composition_unexamined; graph->nodes[activation].frontier=1; continue; }
                for (c=call->first_case;c<call->first_case+call->case_count && !graph->limited;++c) {
                    int ancestor=activation,cycle=0,depth=0,child; GD_ResourceRelation *edge;
                    if (!charge(graph,GD_MAX_RULE_EXPANSION_DEPTH)) break;
                    do {
                        if (graph->nodes[ancestor].case_index==c) { cycle=1; break; }
                        if (!graph->nodes[ancestor].instance) break;
                        ancestor=graph->nodes[ancestor].parent;
                    } while (++depth<GD_MAX_RULE_EXPANSION_DEPTH);
                    if (c<0 || c>=graph->rules->case_count || case_nodes[c]<0) { ++graph->composition_unexamined; graph->nodes[activation].frontier=1; continue; }
                    child=case_nodes[c];
                    if (cycle || graph->nodes[activation].depth+1>=GD_MAX_RULE_EXPANSION_DEPTH) {
                        ++graph->composition_unexamined; graph->nodes[activation].frontier=1;
                        if (cycle) ++graph->cycle_frontiers; else ++graph->depth_frontiers;
                    } else {
                        int primary;
                        child=add_case(graph,c,activation,i,ordinal); if (child<0) break;
                        primary=graph->rules->cases[c].primary_participant;
                        primary=primary>=0 ? mapped_actor(graph,child,primary):-1;
                        snprintf(graph->nodes[child].instance_label,sizeof(graph->nodes[child].instance_label),"%.65s [%.48s]",graph->rules->cases[c].name,primary>=0 ? graph->rules->participants[primary].name:"instance");
                        {
                            const GD_RuleCase *callee=&graph->rules->cases[c]; int member,pool;
                            for (member=callee->first_participant;member<callee->first_participant+callee->participant_count && !graph->limited;++member) {
                                const GD_RuleParticipant *actor=&graph->rules->participants[member];
                                for (pool=actor->first_pool;pool<actor->first_pool+actor->pool_count && !graph->limited;++pool) {
                                    int identity_actor=mapped_actor(graph,child,member),uncertain=0,conflict=0;
                                    if (identity_actor>=0 && graph->rules->pools[pool].target.resolution==GD_RULE_RESOLVED)
                                        (void)binding(graph,child,identity_actor,graph->rules->pools[pool].target.axis_record,&uncertain,&conflict);
                                    if (conflict && !graph->nodes[child].binding_conflict) { graph->nodes[child].binding_conflict=1; ++graph->binding_conflicts; }
                                }
                            }
                        }
                        declarations(graph,c,child,seen,&generation); ancestor_inputs(graph,child,case_nodes);
                    }
                    if (graph->limited || !charge(graph,1) || graph->relation_count==GD_RESOURCE_GRAPH_RELATIONS) { graph->limited=1; break; }
                    edge=&graph->relations[graph->relation_count++]; edge->kind=GD_RESOURCE_INVOCATION; edge->activation=activation; edge->facet=child;
                    edge->participant=call->caller; edge->declaration=i; edge->line=call->line; edge->source_case=call->caller_case;
                    edge->expanded=graph->nodes[child].instance && !cycle && graph->nodes[child].parent==activation;
                    edge->mapped_participant=mapped_actor(graph,activation,call->caller);
                }
            }
        }
    }
    if (graph->limited) ++graph->composition_unexamined;
    return !graph->limited;
}
void gd_resource_graph_destroy(GD_ResourceGraph *graph) { if (graph) graph->allocator.release(graph->allocator.context,graph); }
