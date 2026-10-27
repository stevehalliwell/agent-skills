#include "gd.h"
#include "gd_layout.h"
#include <assert.h>
#include <string.h>

/* Scratch exists only during analysis. Every queue contains each record/state
   at most once. No route enumeration or persistent transitive closure. */
typedef struct {
    int head[GD_MAX_RECORDS], next[GD_MAX_EDGES];
    int check_head[GD_MAX_RECORDS], check_next[GD_MAX_ROUTE_CHECKS];
    int queue[2*GD_MAX_RECORDS], parent[2*GD_MAX_RECORDS];
    int origin[GD_MAX_RECORDS], line[GD_MAX_RECORDS], field[GD_MAX_RECORDS];
    unsigned char needed[GD_MAX_RECORDS], direct[GD_MAX_RECORDS];
    unsigned char opportunity[GD_MAX_RECORDS];
    int introduction_line[GD_MAX_RECORDS];
} RouteScratch;

const char *gd_support_name(GD_Support support) {
    switch (support) {
        case GD_SUPPORT_ALL: return "all_routes";
        case GD_SUPPORT_SOME: return "some_routes";
        case GD_SUPPORT_ABSENT: return "absent";
        default: return "unknown";
    }
}
static void finding(GD_Model *model, int record, const char *code, const char *message,
                    const char *field, const char *reference, int line) {
    GD_Finding *item;
    if (model->finding_count == GD_MAX_ERRORS) { ++model->findings_omitted; return; }
    item = &model->findings[model->finding_count++];
    item->record = record; item->code = code; item->message = message;
    item->field = field; item->reference = reference; item->line = line;
}
static int target_role(const GD_Model *model, const GD_Edge *edge, GD_Role role) {
    return edge->target >= 0 && model->records[edge->target].role == role;
}
static void validate(GD_Model *model, GD_Routes *routes) {
    int i, first = -1;
    routes->routes_complete = routes->requirements_complete = 1;
    if (model->skill_incomplete) routes->requirements_complete = 0;
    for (i = 0; i < model->record_count; ++i) if (model->records[i].role == GD_CHALLENGE) {
        if (first < 0) first = i;
        ++routes->challenge_count;
        if (model->records[i].entry) ++routes->entry_count;
    }
    if (first >= 0 && !routes->entry_count) {
        routes->routes_complete = 0;
        finding(model,first,"scaffolding.entry_missing","No marked entry challenge; route coverage is unknown","entry",NULL,0);
    }
    for (i = 0; i < model->edge_count; ++i) {
        const GD_Edge *edge = &model->edges[i];
        if (model->records[edge->source].role != GD_CHALLENGE) continue;
        if (edge->field == GD_NEXT) {
            if (target_role(model,edge,GD_CHALLENGE)) continue;
            routes->routes_complete = 0;
            finding(model,edge->source,edge->target == -2 ? "challenge.next_ambiguous" : edge->target < 0 ? "challenge.next_missing" : "challenge.next_target",
                    "Next challenge is missing, ambiguous, or not a loaded challenge; route coverage is unknown","next",edge->reference,edge->line);
        } else if (edge->field >= GD_INTRODUCES && edge->field <= GD_TESTS_MASTERY && !target_role(model,edge,GD_SKILL)) {
            routes->requirements_complete = 0;
            finding(model,edge->source,edge->target == -2 ? "challenge.skill_ambiguous" : edge->target < 0 ? "challenge.skill_missing" : "challenge.skill_target",
                    "Skill role target is not a unique loaded skill; demand/support declarations are incomplete",gd_field_name(edge->field),edge->reference,edge->line);
        }
    }
}
static void build_checks(GD_Model *model, GD_Routes *routes, RouteScratch *scratch) {
    int c, e, i;
    for (c = 0; c < model->record_count; ++c) if (model->records[c].role == GD_CHALLENGE) {
        int first = 0, last = 0;
        memset(scratch->needed,0,sizeof(scratch->needed));
        memset(scratch->direct,0,sizeof(scratch->direct));
        for (e = scratch->head[c]; e >= 0; e = scratch->next[e]) {
            const GD_Edge *edge = &model->edges[e];
            if ((edge->field != GD_REQUIRES && edge->field != GD_TESTS_MASTERY) || !target_role(model,edge,GD_SKILL)) continue;
            i = edge->target;
            if (!scratch->needed[i]) {
                scratch->queue[last++] = i; scratch->needed[i] = 1;
                scratch->origin[i] = i; scratch->line[i] = edge->line; scratch->field[i] = edge->field;
            }
            scratch->direct[i] |= edge->field == GD_REQUIRES ? 1 : 2;
        }
        while (first < last) {
            int v = scratch->queue[first++];
            for (e = scratch->head[v]; e >= 0; e = scratch->next[e]) {
                const GD_Edge *edge = &model->edges[e];
                if (edge->field != GD_PREREQUISITES || !target_role(model,edge,GD_SKILL)) continue;
                i = edge->target;
                if (!scratch->needed[i]) {
                    assert(last < GD_MAX_RECORDS);
                    scratch->queue[last++] = i; scratch->needed[i] = 1;
                    scratch->origin[i] = scratch->origin[v]; scratch->line[i] = scratch->line[v]; scratch->field[i] = scratch->field[v];
                }
                scratch->needed[i] |= 2;
            }
        }
        for (i = 0; i < model->record_count; ++i) if (scratch->needed[i]) {
            GD_RouteCheck *check;
            if (routes->check_count == GD_MAX_ROUTE_CHECKS) {
                gd_error(model,"scaffolding.check_limit","32768 challenge/skill check limit exceeded; remaining demands were not analyzed",model->records[c].source_path,0);
                routes->requirements_complete = 0; return;
            }
            check = &routes->checks[routes->check_count++];
            check->challenge = c; check->skill = i; check->demanded_by = scratch->origin[i];
            check->demand_line = scratch->line[i]; check->demand_field = (GD_Field)scratch->field[i];
            check->direct_requires = !!(scratch->direct[i]&1); check->direct_mastery = !!(scratch->direct[i]&2);
            check->prerequisite = !!(scratch->needed[i]&2);
        }
    }
}
/* States describe opportunity strictly BEFORE arriving at a challenge. Entry
   states start unsupported; only departure adds the current opportunity. */
static void walk(const GD_Model *model, RouteScratch *scratch) {
    int i, first = 0, last = 0;
    for (i = 0; i < 2*GD_MAX_RECORDS; ++i) scratch->parent[i] = -2;
    for (i = 0; i < model->record_count; ++i) if (model->records[i].role == GD_CHALLENGE && model->records[i].entry) {
        scratch->queue[last++] = 2*i; scratch->parent[2*i] = -1;
    }
    while (first < last) {
        int state = scratch->queue[first++], v = state/2, e;
        int seen = (state&1) || scratch->opportunity[v];
        for (e = scratch->head[v]; e >= 0; e = scratch->next[e]) {
            const GD_Edge *edge = &model->edges[e];
            int next;
            if (edge->field != GD_NEXT || !target_role(model,edge,GD_CHALLENGE)) continue;
            next = 2*edge->target+seen;
            if (scratch->parent[next] != -2) continue;
            assert(last < 2*GD_MAX_RECORDS);
            scratch->parent[next] = state; scratch->queue[last++] = next;
        }
    }
}
static void witness(GD_Routes *routes, const RouteScratch *scratch, int check, int supported) {
    GD_RouteWitness *item;
    int state = 2*routes->checks[check].challenge+supported, i;
    if (routes->witness_count == GD_MAX_ROUTE_WITNESSES) { ++routes->witnesses_omitted; return; }
    item = &routes->witnesses[routes->witness_count++]; item->check = check; item->supported = supported;
    while (state >= 0 && item->full_length < 2*GD_MAX_RECORDS) {
        if (item->length < GD_MAX_ROUTE_PATH) item->path[item->length++] = state/2;
        ++item->full_length; state = scratch->parent[state];
    }
    assert(state == -1);
    for (i = 0; i < item->length/2; ++i) {
        int v = item->path[i]; item->path[i] = item->path[item->length-1-i]; item->path[item->length-1-i] = v;
    }
}
GD_Routes *gd_analyze_routes(GD_Model *model) {
    GD_Routes *routes = model->allocator.allocate(model->allocator.context,sizeof(*routes));
    RouteScratch *scratch;
    int i, e;
    if (!routes) { gd_error(model,"input.memory","Cannot allocate bounded route result",NULL,0); return NULL; }
    memset(routes,0,sizeof(*routes)); routes->allocator = model->allocator;
    if (model->error_count || model->errors_omitted) return routes;
    if (!model->route_mode || !model->skill_mode) {
        gd_error(model,"scaffolding.collections","Route checks require explicit challenges and skill-atoms roles",NULL,0); return routes;
    }
    scratch = model->allocator.allocate(model->allocator.context,sizeof(*scratch));
    if (!scratch) { gd_error(model,"input.memory","Cannot allocate bounded route scratch",NULL,0); return routes; }
    memset(scratch,0,sizeof(*scratch));
    for (i = 0; i < GD_MAX_RECORDS; ++i) scratch->head[i] = scratch->check_head[i] = -1;
    for (e = 0; e < model->edge_count; ++e) { i = model->edges[e].source; scratch->next[e] = scratch->head[i]; scratch->head[i] = e; }
    validate(model,routes);
    walk(model,scratch);
    for (i = 0; i < model->record_count; ++i) if (model->records[i].role == GD_CHALLENGE) {
        routes->reachable[i] = scratch->parent[2*i] != -2;
        if (routes->entry_count && !routes->reachable[i]) finding(model,i,"scaffolding.unreachable","No declared route from a marked entry; opportunity coverage at this challenge is unknown","entry",NULL,model->records[i].entry_line);
    }
    /* On check-limit error, still analyze retained checks as explicit partial data. */
    build_checks(model,routes,scratch);
    for (i = 0; i < routes->check_count; ++i) {
        int skill = routes->checks[i].skill;
        scratch->check_next[i] = scratch->check_head[skill]; scratch->check_head[skill] = i;
    }
    for (i = 0; i < model->record_count; ++i) if (scratch->check_head[i] >= 0) {
        int index;
        memset(scratch->opportunity,0,sizeof(scratch->opportunity));
        for (e = 0; e < model->record_count; ++e) scratch->introduction_line[e] = -1;
        for (e = 0; e < model->edge_count; ++e) {
            const GD_Edge *edge = &model->edges[e];
            if (edge->target != i || model->records[edge->source].role != GD_CHALLENGE) continue;
            if (edge->field == GD_INTRODUCES || edge->field == GD_PRACTISES) scratch->opportunity[edge->source] = 1;
            if (edge->field == GD_INTRODUCES) scratch->introduction_line[edge->source] = edge->line;
        }
        walk(model,scratch);
        for (index = scratch->check_head[i]; index >= 0; index = scratch->check_next[index]) {
            GD_RouteCheck *check = &routes->checks[index];
            int c = check->challenge;
            check->current_introduction = scratch->introduction_line[c] >= 0;
            check->supported_route = scratch->parent[2*c+1] != -2; check->unsupported_route = scratch->parent[2*c] != -2;
            if (routes->routes_complete && routes->reachable[c])
                check->coverage = check->supported_route ? check->unsupported_route ? GD_SUPPORT_SOME : GD_SUPPORT_ALL : GD_SUPPORT_ABSENT;
            if (check->current_introduction && (check->direct_requires || check->direct_mastery))
                finding(model,c,"scaffolding.single_shot","Challenge declares introduction and demand for the same skill; review same-challenge support, not presumed learning failure","introduces",model->records[i].reference,scratch->introduction_line[c]);
            if (check->coverage == GD_SUPPORT_SOME)
                finding(model,c,"scaffolding.support_some_routes","Earlier introduction/practice exists on some declared routes/visits, but not all",gd_field_name(check->demand_field),model->records[i].reference,check->demand_line);
            else if (check->coverage == GD_SUPPORT_ABSENT && !check->current_introduction)
                finding(model,c,"scaffolding.support_absent","No earlier declared introduction/practice on any route/visit; this does not establish inability or require earlier teaching",gd_field_name(check->demand_field),model->records[i].reference,check->demand_line);
            else if (check->coverage == GD_SUPPORT_UNKNOWN && routes->reachable[c])
                finding(model,c,"scaffolding.support_unknown","Incomplete route declarations prevent a full prior-opportunity claim; known witnesses remain available",gd_field_name(check->demand_field),model->records[i].reference,check->demand_line);
            if (check->unsupported_route) witness(routes,scratch,index,0);
            if (check->supported_route) witness(routes,scratch,index,1);
        }
    }
    model->allocator.release(model->allocator.context,scratch); return routes;
}
void gd_routes_destroy(GD_Routes *routes) {
    if (routes) routes->allocator.release(routes->allocator.context,routes);
}
int gd_route_layout(const GD_Model *model, GD_Layout *layout, int horizontal, GD_LayoutSort sort) {
    int nodes[GD_MAX_RECORDS], head[GD_MAX_RECORDS], next[GD_MAX_EDGES];
    int distance[GD_MAX_RECORDS], queue[GD_MAX_RECORDS], read=0, write=0, i;
    float spacing=2.5f;
    memset(layout,0,sizeof(*layout)); layout->horizontal=horizontal; layout->group_count=1;
    for (i=0;i<model->record_count;++i) { nodes[i]=head[i]=distance[i]=-1; }
    for (i=0;i<model->record_count;++i) if (model->records[i].role==GD_CHALLENGE) {
        GD_LayoutNode *node=&layout->nodes[layout->node_count]; nodes[i]=layout->node_count++;
        node->key=i; node->label=model->records[i].name; node->group=0; node->side=model->records[i].entry;
        if (model->records[i].entry) { distance[i]=0; queue[write++]=i; }
    }
    for (i=0;i<model->edge_count;++i) {
        const GD_Edge *edge=&model->edges[i]; GD_LayoutEdge *link;
        if (edge->field!=GD_NEXT || nodes[edge->source]<0 || edge->target<0 || nodes[edge->target]<0) continue;
        next[i]=head[edge->source]; head[edge->source]=i;
        link=&layout->edges[layout->edge_count++]; link->a=nodes[edge->source]; link->b=nodes[edge->target]; link->key=i; link->active=1;
        ++layout->nodes[link->a].degree; ++layout->nodes[link->b].degree;
    }
    while (read<write) {
        int source=queue[read++], edge;
        for (edge=head[source];edge>=0;edge=next[edge]) {
            int target=model->edges[edge].target;
            if (distance[target]<0) { distance[target]=distance[source]+1; queue[write++]=target; }
        }
    }
    for (i=0;i<layout->node_count;++i) {
        GD_LayoutNode *node=&layout->nodes[i]; node->layer=distance[node->key]; node->axis=(float)node->layer;
    }
    /* ponytail: shortest-entry-hop bands are display only; never flatten route
       cycles or infer a teaching order from rank or graph placement. */
    if (!gd_layout_arrange(layout,GD_LAYOUT_DIRECTED,sort)) return 0;
    for (i=0;i<layout->node_count;++i) {
        float required=(float)strlen(layout->nodes[i].label)*10.0f/80.0f+0.6f;
        if (required>spacing) spacing=required;
    }
    /* Reserve lateral room for ASCII technical names and reachability captions.
       Scale anchors too so later settling cannot collapse adjacent labels. */
    for (i=0;i<layout->node_count;++i) {
        GD_LayoutNode *node=&layout->nodes[i]; node->anchor*=spacing;
        if (horizontal) node->y*=spacing; else node->x*=spacing;
    }
    if (horizontal) { layout->height*=spacing; layout->groups[0].height*=spacing; }
    else { layout->width*=spacing; layout->groups[0].width*=spacing; }
    return 1;
}
