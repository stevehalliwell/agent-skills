#include "gd_resource_routes.h"
#include <string.h>

static int charge(GD_ResourceRoutes *routes, int work) {
    if (routes->limited || work>routes->work_limit-routes->work) { routes->limited=1; return 0; }
    routes->work+=work; return 1;
}
GD_ResourceRoutes *gd_resource_routes_create(GD_Allocator allocator) {
    GD_ResourceRoutes *routes;
    if (!allocator.allocate || !allocator.release) return NULL;
    routes=allocator.allocate(allocator.context,sizeof(*routes));
    if (routes) { memset(routes,0,sizeof(*routes)); routes->allocator=allocator; routes->from=routes->to=-1; routes->work_limit=GD_RESOURCE_ROUTE_WORK; }
    return routes;
}
void gd_resource_routes_destroy(GD_ResourceRoutes *routes) { if (routes) routes->allocator.release(routes->allocator.context,routes); }
static void identities(GD_ResourceRoutes *routes, const GD_ResourceGraph *graph) {
    int i;
    routes->graph=graph;
    for (i=0;i<graph->node_count;++i) routes->first_relation[i]=-1;
    for (i=0;i<graph->relation_count;++i) if (routes->first_relation[graph->relations[i].facet]<0) routes->first_relation[graph->relations[i].facet]=i;
}
int gd_resource_select(GD_ResourceRoutes *routes, const GD_ResourceGraph *graph, const char *selector) {
    int i,found=-1; char id[GD_MAX_PATH];
    if (!routes || !graph || !selector) return -1;
    routes->work=routes->limited=routes->selector_matches=0; routes->work_limit=GD_RESOURCE_ROUTE_WORK;
    if (!charge(routes,graph->node_count+graph->relation_count)) return -3;
    identities(routes,graph);
    for (i=0;i<graph->node_count;++i) {
        const GD_ResourceNode *node=&graph->nodes[i]; const char *reference=NULL;
        if (!charge(routes,GD_MAX_PATH)) return -3;
        if (!gd_resource_node_id(graph,i,routes->first_relation[i],id,sizeof(id))) return -3;
        routes->work-=GD_MAX_PATH-(int)strlen(id);
        if (!strcmp(id,selector)) { routes->selector_matches=0; return i; }
        if (node->kind==GD_RESOURCE_ROLE) reference=node->pool_reference;
        else if (node->kind==GD_RESOURCE_CASE) reference=graph->model->records[graph->rules->sources[graph->rules->cases[node->case_index].source].record].reference;
        else reference=node->pool_reference;
        if (reference && !strcmp(reference,selector)) {
            if (routes->selector_matches<GD_RESOURCE_ROUTE_WITNESSES) routes->selector_candidates[routes->selector_matches]=i;
            ++routes->selector_matches; found=i;
        }
    }
    return routes->selector_matches>1 ? -2:found;
}
const char *gd_resource_arc_name(GD_ResourceArcKind kind) {
    static const char *const names[]={"enabling_guard_not_consumed","checked_debit","checked_credit_not_final_outcome","state_proposal_not_gain"};
    return names[kind];
}
static void arc(GD_ResourceRoutes *routes, int from, int to, int relation, GD_ResourceArcKind kind, int reverse) {
    GD_ResourceArc *item; int key=reverse ? to:from;
    if (routes->arc_count==GD_RESOURCE_GRAPH_RELATIONS) { routes->limited=1; return; }
    item=&routes->arcs[routes->arc_count]; *item=(GD_ResourceArc){from,to,relation,routes->head[key],kind};
    routes->head[key]=routes->arc_count++;
}
/* Parallel declarations retain full side provenance but not duplicate search
   branches. Prefer a checked debit over an enabling guard for the same pair. */
static int collapse_arcs(GD_ResourceRoutes *routes, int reverse) {
    int stamp[GD_RESOURCE_GRAPH_NODES]={0},pair[GD_RESOURCE_GRAPH_NODES],i;
    for (i=0;i<routes->graph->node_count;++i) {
        int link=routes->head[i],previous=-1;
        while (link>=0) {
            GD_ResourceArc *item=&routes->arcs[link]; int next=item->next,other=reverse ? item->from:item->to;
            if (!charge(routes,1)) return 0;
            if (stamp[other]==i+1) {
                GD_ResourceArc *kept=&routes->arcs[pair[other]];
                if (kept->kind==GD_RESOURCE_ENABLE && item->kind==GD_RESOURCE_DEBIT) { kept->kind=item->kind; kept->relation=item->relation; }
                if (previous<0) routes->head[i]=next; else routes->arcs[previous].next=next;
            } else { stamp[other]=i+1; pair[other]=link; previous=link; }
            link=next;
        }
    }
    return 1;
}
static void seeds(GD_ResourceRoutes *routes, int selected, unsigned char *mask) {
    const GD_ResourceGraph *graph=routes->graph; int i;
    memset(mask,0,(size_t)graph->node_count);
    if (graph->nodes[selected].kind!=GD_RESOURCE_ROLE) { mask[selected]=1; return; }
    /* Role selection expands explicit bindings only, never noun-type identity. */
    for (i=0;i<graph->relation_count;++i) if (graph->relations[i].kind==GD_RESOURCE_ROLE_BINDING && graph->relations[i].activation==selected)
        mask[graph->relations[i].facet]=1;
}
static int unresolved(const GD_ResourceGraph *graph, int node) {
    const GD_ResourceNode *item=&graph->nodes[node];
    if (item->kind==GD_RESOURCE_FACET) return item->axis<0 || (item->pool>=0 && graph->rules->pools[item->pool].resource_resolution!=GD_RULE_RESOLVED);
    if (item->kind==GD_RESOURCE_CASE) {
        const GD_RuleCase *rule=&graph->rules->cases[item->case_index]; int p;
        if (rule->unknown_conditions || rule->unknown_effects) return 1;
        for (p=rule->first_participant;p<rule->first_participant+rule->participant_count;++p) if (graph->rules->participants[p].causes) return 1;
    }
    return 0;
}
static void retain(GD_ResourceRoutes *routes, const int *nodes, const int *arcs, int depth, int reverse) {
    int i,j; GD_ResourceWitness *w;
    if (!depth) return;
    /* Parallel guard/debit rows do not create fake alternative case routes. */
    for (i=0;i<routes->witness_count;++i) {
        w=&routes->witnesses[i]; if (w->count!=depth) continue;
        for (j=0;j<=depth;++j) {
            if (!charge(routes,1)) return;
            if (w->nodes[j]!=nodes[reverse ? depth-j:j]) break;
        }
        if (j>depth) return;
    }
    if (routes->witness_count==GD_RESOURCE_ROUTE_WITNESSES) { routes->limited=1; return; }
    w=&routes->witnesses[routes->witness_count++]; memset(w,0,sizeof(*w)); w->count=depth;
    for (i=0;i<=depth;++i) {
        int node=nodes[reverse ? depth-i:i]; const GD_ResourceNode *item=&routes->graph->nodes[node];
        w->nodes[i]=node; w->unresolved|=item->kind==GD_RESOURCE_CASE ? routes->case_unresolved[item->case_index]:unresolved(routes->graph,node);
        if (i<depth) w->arcs[i]=arcs[reverse ? depth-1-i:i];
        if (item->kind==GD_RESOURCE_CASE) {
            const GD_Record *record=&routes->graph->model->records[routes->graph->rules->sources[routes->graph->rules->cases[item->case_index].source].record];
            w->proposed|=record->status && !strcmp(record->status,"proposed");
        }
    }
}
void gd_resource_route_mark(GD_ResourceRoutes *routes, int witness) {
    const GD_ResourceWitness *w; int i;
    memset(routes->marked_nodes,0,sizeof(routes->marked_nodes)); memset(routes->marked_relations,0,sizeof(routes->marked_relations)); memset(routes->side_relations,0,sizeof(routes->side_relations));
    if (witness<0 || witness>=routes->witness_count) return;
    w=&routes->witnesses[witness];
    for (i=0;i<=w->count;++i) routes->marked_nodes[w->nodes[i]]=1;
    for (i=0;i<w->count;++i) routes->marked_relations[routes->arcs[w->arcs[i]].relation]=1;
    for (i=0;i<=w->count;++i) if (routes->graph->nodes[w->nodes[i]].kind==GD_RESOURCE_CASE) {
        int edge;
        for (edge=routes->case_head[w->nodes[i]];edge>=0;edge=routes->case_next[edge]) {
            if (!routes->marked_relations[edge]) routes->side_relations[edge]=1;
            routes->marked_nodes[routes->graph->relations[edge].facet]=1;
        }
    }
    for (i=0;i<routes->graph->relation_count;++i) {
        const GD_ResourceRelation *edge=&routes->graph->relations[i];
        if (edge->kind==GD_RESOURCE_ROLE_BINDING && routes->marked_nodes[edge->facet]) {
            routes->side_relations[i]=1; routes->marked_nodes[edge->activation]=1;
        }
    }
}
int gd_resource_query(GD_ResourceRoutes *routes, const GD_ResourceGraph *graph, int from, int to, int accepted_only, int work_limit) {
    unsigned char starts[GD_RESOURCE_GRAPH_NODES],visited[GD_RESOURCE_GRAPH_NODES],interaction[GD_MAX_RECORDS]={0};
    int nodes[GD_RESOURCE_ROUTE_STEPS+1],path[GD_RESOURCE_ROUTE_STEPS],cursor[GD_RESOURCE_ROUTE_STEPS+1],entered[GD_RESOURCE_ROUTE_STEPS+1],cases[GD_RESOURCE_ROUTE_STEPS+1];
    GD_Allocator allocator; int i,c,reverse=from<0;
    if (!routes || !graph || from < -1 || from>=graph->node_count || to<0 || to>=graph->node_count || work_limit<0 || work_limit>GD_RESOURCE_ROUTE_WORK) return 0;
    allocator=routes->allocator; memset(routes,0,sizeof(*routes)); routes->allocator=allocator;
    routes->graph=graph; routes->from=from; routes->to=to; routes->accepted_only=accepted_only; routes->queried=1; routes->work_limit=work_limit;
    if (!charge(routes,4*graph->node_count+5*graph->relation_count+graph->model->edge_count+graph->rules->case_count+graph->rules->participant_count)) return 1;
    routes->unexpanded_calls=graph->composition_unexamined;
    identities(routes,graph);
    for (i=0;i<graph->node_count;++i) routes->head[i]=routes->case_head[i]=-1;
    for (i=graph->relation_count-1;i>=0;--i) {
        int owner=graph->relations[i].activation;
        routes->case_next[i]=routes->case_head[owner]; routes->case_head[owner]=i;
    }
    for (i=0;i<graph->model->edge_count;++i) if (graph->model->edges[i].field==GD_VERB_REF) interaction[graph->model->edges[i].source]=1;
    for (c=0;c<graph->rules->case_count;++c) {
        const GD_Record *record=&graph->model->records[graph->rules->sources[graph->rules->cases[c].source].record];
        int is_interaction=interaction[record-graph->model->records] || (record->collection>=0 && record->collection<graph->model->collection_count && graph->model->collections[record->collection].role==GD_CASE);
        routes->eligible[c]=(unsigned char)(record->status && (!strcmp(record->status,"accepted") || (!accepted_only && !strcmp(record->status,"proposed"))) &&
            (!is_interaction || (record->coverage && !strcmp(record->coverage,"defined"))));
        if (!routes->eligible[c]) ++routes->excluded_cases;
        {
            const GD_RuleCase *rule=&graph->rules->cases[c]; int p;
            routes->case_unresolved[c]=(unsigned char)(rule->unknown_conditions || rule->unknown_effects);
            for (p=rule->first_participant;p<rule->first_participant+rule->participant_count;++p)
                if (graph->rules->participants[p].causes) routes->case_unresolved[c]=1;
        }
    }
    /* Reverse insertion preserves source declaration order during traversal. */
    for (i=graph->relation_count-1;i>=0;--i) {
        const GD_ResourceRelation *edge=&graph->relations[i]; int cnode=edge->activation;
        if (!charge(routes,1)) return 1;
        if (graph->nodes[cnode].kind!=GD_RESOURCE_CASE || !routes->eligible[graph->nodes[cnode].case_index]) continue;
        if (graph->nodes[edge->facet].kind==GD_RESOURCE_FACET && unresolved(graph,edge->facet)) routes->case_unresolved[graph->nodes[cnode].case_index]=1;
        if (edge->kind==GD_RESOURCE_INVOCATION) { ++routes->unexpanded_calls; continue; }
        if (edge->kind==GD_RESOURCE_REQUIREMENT) arc(routes,edge->facet,cnode,i,GD_RESOURCE_ENABLE,reverse);
        else if (edge->kind==GD_RESOURCE_STATE_WRITE) arc(routes,cnode,edge->facet,i,GD_RESOURCE_STATE_PROPOSAL,reverse);
        else if (edge->kind==GD_RESOURCE_QUANTITY_WRITE) {
            if (!edge->delta_recognized || !edge->delta.known || !edge->delta.checks_complete) { ++routes->unknown_writes; routes->case_unresolved[graph->nodes[cnode].case_index]=1; continue; }
            if (edge->delta.number<0) arc(routes,edge->facet,cnode,i,GD_RESOURCE_DEBIT,reverse);
            else if (edge->delta.number>0) arc(routes,cnode,edge->facet,i,GD_RESOURCE_CREDIT,reverse);
        }
    }
    if (!collapse_arcs(routes,reverse)) return 1;
    seeds(routes,to,routes->goals); if (reverse) memcpy(starts,routes->goals,(size_t)graph->node_count); else seeds(routes,from,starts);
    for (i=0;i<graph->node_count;++i) { routes->from_bindings+=starts[i]!=0; routes->to_bindings+=routes->goals[i]!=0; }
    for (i=0;i<graph->node_count && !routes->limited;++i) if (starts[i]) {
        int depth=0;
        memset(visited,0,(size_t)graph->node_count); nodes[0]=i; cursor[0]=routes->head[i]; entered[0]=0; cases[0]=graph->nodes[i].kind==GD_RESOURCE_CASE; visited[i]=1;
        while (depth>=0 && !routes->limited) {
            int node=nodes[depth],link=cursor[depth],other;
            if (!entered[depth] && depth && ((reverse && graph->nodes[node].kind!=GD_RESOURCE_ROLE) || (!reverse && routes->goals[node]))) {
                retain(routes,nodes,path,depth,reverse);
                if (!reverse) cursor[depth]=-1;
            }
            entered[depth]=1; link=cursor[depth];
            if (link<0) { visited[node]=0; --depth; continue; }
            if (!charge(routes,1)) break;
            cursor[depth]=routes->arcs[link].next; other=reverse ? routes->arcs[link].from:routes->arcs[link].to;
            if (visited[other]) { ++routes->cycle_edges; continue; }
            if (depth==GD_RESOURCE_ROUTE_STEPS || cases[depth]+(graph->nodes[other].kind==GD_RESOURCE_CASE)>32) { routes->depth_limited=1; continue; }
            cases[depth+1]=cases[depth]+(graph->nodes[other].kind==GD_RESOURCE_CASE);
            path[depth]=link; nodes[++depth]=other; cursor[depth]=routes->head[other]; entered[depth]=0; visited[other]=1;
        }
    }
    gd_resource_route_mark(routes,0); return 1;
}
