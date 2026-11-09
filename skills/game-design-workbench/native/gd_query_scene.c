#include "gd_query_scene.h"
#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <string.h>
struct GD_ActorOverview { GD_ActorOverviewMode mode; int unclassified,incomplete; int counts[GD_MAX_PAIRS]; };
#include "gd_query_scene_free.h"

const char *gd_query_scene_node_kind_name(GD_QueryNodeKind kind) {
    static const char *const names[]={"selected_binding","state_hypothesis","checked_activation","effect_concept","partner_binding","coverage_value","object_definition","axis_definition"};
    return kind>=GD_QUERY_NODE_BINDING && kind<=GD_QUERY_NODE_AXIS ? names[kind]:NULL;
}
const char *gd_query_scene_edge_kind_name(GD_QueryEdgeKind kind) {
    static const char *const names[]={"hypothesis_association","invocation","effect_proposal","partner_association","call_frontier","shared_interaction","shared_case_usage","declared_axis_usage"};
    return kind>=GD_QUERY_EDGE_ASSUMPTION && kind<=GD_QUERY_EDGE_AXIS ? names[kind]:NULL;
}
static int charge(GD_QueryScene *scene, int amount) {
    if (scene->limited || amount>scene->work_limit-scene->work) { scene->limited=1; return 0; }
    scene->work+=amount; return 1;
}
static int add_node(GD_QueryScene *scene, GD_QueryNodeKind kind, int source, int variant, int layer, GD_RuleTruth truth, const char *format, ...) {
    GD_LayoutNode *node; int index,count; va_list arguments;
    if (!charge(scene,1)) return -1;
    if (scene->layout.node_count==GD_MAX_RECORDS) { scene->limited=scene->capacity_limited=1; return -1; }
    index=scene->layout.node_count++; node=&scene->layout.nodes[index];
    va_start(arguments,format); count=vsnprintf(scene->labels[index],GD_QUERY_SCENE_LABEL,format,arguments); va_end(arguments);
    if (count<0 || count>=GD_QUERY_SCENE_LABEL) {
        size_t end=GD_QUERY_SCENE_LABEL-4;
        if (count<0) end=0;
        else while (end && ((unsigned char)scene->labels[index][end]&0xc0)==0x80) --end;
        memcpy(scene->labels[index]+end,"...",4); ++scene->label_truncations;
    }
    node->key=index; node->label=scene->labels[index]; node->group=0; node->layer=layer; node->width=250; node->height=72;
    scene->nodes[index]=(GD_QuerySceneNode){kind,source,variant,0,scene->ghosts==GD_QUERY_STACKED_VARIANTS && variant>=0 && variant!=scene->primary,truth,0};
    return index;
}
static void add_edge(GD_QueryScene *scene, int a, int b, GD_QueryEdgeKind kind, int source, int variant) {
    int index;
    if (a<0 || b<0 || !charge(scene,1)) return;
    if (scene->layout.edge_count==GD_MAX_PAIRS) { scene->limited=scene->capacity_limited=1; return; }
    index=scene->layout.edge_count++;
    scene->layout.edges[index]=(GD_LayoutEdge){a,b,index,1,GD_RELATION_QUERY_BASE+(int)kind};
    scene->edges[index]=(GD_QuerySceneEdge){kind,source,variant};
}
#include "gd_query_scene_overview.h"
static void objects(GD_QueryScene *scene) {
    const GD_Query *query=scene->query; int counts[GD_MAX_RECORDS]={0},i;
    if (!charge(scene,query->model->record_count+query->binding_count)) return;
    for (i=0;i<query->binding_count;++i) {
        const GD_RuleParticipant *binding=&query->rules->participants[query->bindings[i].participant];
        if (binding->noun_resolution==GD_RULE_RESOLVED) ++counts[binding->noun_record];
    }
    for (i=0;i<query->model->record_count && !scene->limited;++i) if (query->model->records[i].role==GD_NOUN) {
        const GD_Record *record=&query->model->records[i]; int node;
        node=add_node(scene,GD_QUERY_NODE_OBJECT,i,-1,0,GD_RULE_TRUTH_UNKNOWN,"%s\n%d local bindings%s",record->display_name,counts[i],query->limited ? " (partial)":"");
        if (node>=0) scene->nodes[node].incoming=counts[i];
    }
}
static int case_record(const GD_Query *query, int index) { return query->rules->sources[query->rules->cases[index].source].record; }
static void effects(GD_QueryScene *scene) {
    const GD_Query *query=scene->query; const GD_EffectGraph *graph=query->effects;
    int nodes[GD_EFFECT_NODES],counts[GD_EFFECT_NODES],i,maximum=0,unfiltered=query->request.case_index<0 && query->request.noun<0 && query->request.axis<0 && query->request.effect_kind<0 && !query->request.target_id && !query->request.value_count && !query->request.status_count;
    const GD_EffectQuery *scope=scene->effect_scope;
    if (scope) { if (!charge(scene,graph->node_count+graph->link_count)) return; gd_effect_incoming(graph,scope->context,counts); }
    for (i=0;i<graph->node_count;++i) nodes[i]=-1;
    for (i=0;i<graph->node_count && !scene->limited;++i) {
        const GD_EffectNode *item=&graph->nodes[i];
        if (!charge(scene,GD_MAX_RULE_EXPANSION_DEPTH+1)) break;
        if (item->kind==GD_EFFECT_ACTIVATION) {
            const GD_ResourceNode *source=&query->resource->nodes[item->activation]; int path[GD_MAX_RULE_EXPANSION_DEPTH];
            const GD_RuleCase *declaration=&query->rules->cases[source->case_index]; const GD_Record *record=&query->model->records[case_record(query,source->case_index)];
            if ((scope && !scope->context[i]) || !gd_effect_activation_path(query->contributors,i,path,GD_MAX_RULE_EXPANSION_DEPTH)) continue;
            nodes[i]=add_node(scene,GD_QUERY_NODE_ACTIVATION,i,-1,source->depth,GD_RULE_TRUTH_UNKNOWN,"%s\n%s%s",declaration->name,record->reference,source->instance ? " [mapped]":"");
            if (source->depth>maximum) maximum=source->depth;
        }
    }
    for (i=0;i<graph->node_count && !scene->limited;++i) {
        const GD_EffectNode *item=&graph->nodes[i]; int index,count=scope ? counts[i]:query->contributors->incoming[i];
        if (!charge(scene,1)) break;
        if (item->kind==GD_EFFECT_ACTIVATION || ((scope || !unfiltered) && !count)) continue;
        index=add_node(scene,GD_QUERY_NODE_EFFECT,i,-1,maximum+1,GD_RULE_TRUTH_UNKNOWN,"%s\n%s%s%s",item->axis>=0 ? query->model->records[item->axis].reference:"[unresolved axis]",gd_effect_kind_name(item->kind),item->value ? ": ":"",item->value ? item->value:"");
        nodes[i]=index;
        if (index>=0) {
            scene->nodes[index].incoming=count;
            if (scene->weighted) { float weight=log2f((float)count+1); scene->layout.nodes[index].width+=fminf(150,24*weight); scene->layout.nodes[index].height+=fminf(100,10*weight); }
        }
    }
    for (i=0;i<graph->link_count && !scene->limited;++i) {
        const GD_EffectLink *item=&graph->links[i];
        if (!charge(scene,1)) break;
        if (item->kind==GD_EFFECT_DECLARATION && (scope ? !scope->context[item->from]:!query->contributors->highlighted[i])) continue;
        add_edge(scene,nodes[item->from],nodes[item->to],item->kind==GD_EFFECT_DECLARATION ? GD_QUERY_EDGE_EFFECT:item->kind==GD_EFFECT_INVOCATION ? GD_QUERY_EDGE_INVOCATION:GD_QUERY_EDGE_FRONTIER,i,-1);
    }
}
static int same_parent(const GD_RuleActivation *parent, const GD_RuleActivation *child) {
    int i;
    if (parent->path_count+1!=child->path_count) return 0;
    for (i=0;i<parent->path_count;++i) if (parent->path[i]!=child->path[i] || parent->calls[i]!=child->calls[i]) return 0;
    return 1;
}
static void actors(GD_QueryScene *scene) {
    const GD_Query *query=scene->query; const GD_ActorQuery *source=query->variants;
    int activations[GD_ACTOR_ACTIVATIONS],partners[GD_MAX_RULE_PARTICIPANTS],concepts[GD_ACTOR_WRITES],variants[GD_ACTOR_VARIANTS];
    int base,i,v,maximum=2; const GD_RuleParticipant *binding;
    if (!source) return;
    binding=&query->rules->participants[source->participant];
    for (i=0;i<GD_ACTOR_ACTIVATIONS;++i) activations[i]=-1;
    for (i=0;i<GD_MAX_RULE_PARTICIPANTS;++i) partners[i]=-1;
    for (i=0;i<GD_ACTOR_WRITES;++i) concepts[i]=-1;
    for (i=0;i<GD_ACTOR_VARIANTS;++i) variants[i]=-1;
    base=add_node(scene,GD_QUERY_NODE_BINDING,source->participant,-1,0,GD_RULE_TRUTH_UNKNOWN,"%s\n%s",binding->name,gd_rule_noun_label(query->rules,binding));
    for (v=0;v<source->variant_count && !scene->limited;++v) {
        const GD_ActorVariant *variant=&source->variants[v]; int parents[GD_MAX_RULE_EXPANSION_DEPTH];
        if (scene->ghosts==GD_QUERY_PRIMARY_VARIANT && v!=scene->primary) continue;
        for (i=0;i<GD_MAX_RULE_EXPANSION_DEPTH;++i) parents[i]=-1;
        variants[v]=add_node(scene,GD_QUERY_NODE_VARIANT,v,v,1,variant->applicability,"%s\n%s = %s",binding->name,query->model->records[source->axis].reference,variant->value);
        add_edge(scene,base,variants[v],GD_QUERY_EDGE_ASSUMPTION,v,v);
        for (i=variant->first_activation;i<variant->first_activation+variant->activation_count && !scene->limited;++i) {
            const GD_RuleActivation *entry=&source->activations[i]; int depth=entry->path_count-1,node,p,parent;
            const GD_RuleCase *declaration=&query->rules->cases[entry->case_index];
            if (!charge(scene,GD_MAX_RULE_EXPANSION_DEPTH+1)) break;
            if (!scene->include_excluded && entry->applicability==GD_RULE_TRUTH_FALSE) { ++scene->excluded_activations; continue; }
            if (depth<0 || depth>=GD_MAX_RULE_EXPANSION_DEPTH) { scene->limited=1; break; }
            node=add_node(scene,GD_QUERY_NODE_ACTIVATION,i,v,2+depth,entry->applicability,"%s\n%s",declaration->name,query->model->records[case_record(query,entry->case_index)].reference);
            activations[i]=node; if (2+depth>maximum) maximum=2+depth;
            parent=depth ? parents[depth-1]:-1;
            if (parent>=0 && same_parent(&source->activations[parent],entry)) add_edge(scene,activations[parent],node,GD_QUERY_EDGE_INVOCATION,i,v);
            else add_edge(scene,variants[v],node,GD_QUERY_EDGE_ASSUMPTION,i,v);
            parents[depth]=i;
            if (entry->applicability==GD_RULE_TRUTH_FALSE) continue;
            for (p=declaration->first_participant;p<declaration->first_participant+declaration->participant_count && !scene->limited;++p) {
                int mapped;
                if (!charge(scene,GD_MAX_RULE_EXPANSION_DEPTH*GD_MAX_RULE_CALL_BINDINGS)) break;
                mapped=gd_actor_trace_participant(query->analysis,entry,p);
                if (mapped<query->rules->cases[source->case_index].first_participant || mapped>=query->rules->cases[source->case_index].first_participant+query->rules->cases[source->case_index].participant_count || mapped==source->participant) continue;
                if (partners[mapped]<0) { const GD_RuleParticipant *peer=&query->rules->participants[mapped]; partners[mapped]=add_node(scene,GD_QUERY_NODE_PARTNER,mapped,-1,0,GD_RULE_TRUTH_UNKNOWN,"%s\n%s",peer->name,gd_rule_noun_label(query->rules,peer)); }
                add_edge(scene,node,partners[mapped],GD_QUERY_EDGE_PARTICIPANT,p,v);
            }
        }
        for (i=variant->first_write;i<variant->first_write+variant->write_count && !scene->limited;++i) {
            int effect,from=source->write_activations[i];
            if (!charge(scene,1)) break;
            /* Vocabulary already owns canonicalization and declaration placement.
               Match retained proposal with its native concept, never re-evaluate. */
            effect=gd_actor_write_effect(source,v,i);
            if (effect<0 || from<0 || from>=source->activation_count || activations[from]<0) continue;
            if (concepts[effect]<0) {
                const GD_ActorEffect *item=&source->effects[effect];
                concepts[effect]=add_node(scene,GD_QUERY_NODE_EFFECT,effect,-1,maximum+1,GD_RULE_TRUTH_UNKNOWN,"%s\n%s%s%s",item->axis>=0 ? query->model->records[item->axis].reference:"[unresolved axis]",gd_effect_kind_name(item->kind),item->state ? ": ":"",item->state ? item->state:"");
            }
            add_edge(scene,activations[from],concepts[effect],GD_QUERY_EDGE_EFFECT,i,v);
        }
    }
    for (i=0;i<scene->layout.node_count;++i) if (scene->nodes[i].kind==GD_QUERY_NODE_EFFECT) scene->layout.nodes[i].layer=maximum+1;
    if (scene->ghosts==GD_QUERY_STACKED_VARIANTS && scene->primary>=0 && variants[scene->primary]>=0) scene->layout.nodes[variants[scene->primary]].degree+=source->variant_count;
}
/* Coverage cards summarize existing independent binding/value hypotheses.
   No edges: an aggregate card is not a simultaneous assignment or invocation. */
static void coverage_cards(GD_QueryScene *scene) {
    const GD_StateCoverage *coverage=scene->query->state_coverage; int i;
    for (i=0;i<coverage->value_count && !scene->limited;++i) {
        const GD_StateCoverageValue *value=&coverage->values[i];
        int node=add_node(scene,GD_QUERY_NODE_COVERAGE,i,-1,0,GD_RULE_TRUTH_UNKNOWN,
            "Cases: definite %d | unknown %d\nExcluded %d | unfinished %d\nBindings: definite %d | unknown %d\nExcluded %d | unfinished %d\n%s",
            value->definite_cases,value->undetermined_cases,value->excluded_cases,value->unfinished_cases,
            value->definite_bindings,value->undetermined_bindings,value->excluded_bindings,value->unfinished_bindings,value->value);
        if (node>=0) { scene->layout.nodes[node].width=440; scene->layout.nodes[node].height=112; }
    }
}
static void wrap_context(GD_QueryScene *scene) {
    enum { BANDS=GD_MAX_RULE_EXPANSION_DEPTH+2 };
    int counts[BANDS]={0},i,band; float widths[BANDS]={0},heights[BANDS]={0},top=0;
    for (i=0;i<scene->layout.node_count;++i) { const GD_LayoutNode *node=&scene->layout.nodes[i]; int layer=node->layer; assert(layer>=0 && layer<BANDS); ++counts[layer]; widths[layer]=fmaxf(widths[layer],node->width); heights[layer]=fmaxf(heights[layer],node->height); }
    scene->layout.horizontal=0;
    for (band=0;band<BANDS;++band) if (counts[band]) {
        int ordinal=0,columns=counts[band]<4 ? counts[band]:4;
        for (i=0;i<scene->layout.node_count;++i) if (scene->layout.nodes[i].layer==band) {
            GD_LayoutNode *node=&scene->layout.nodes[i]; node->x=((ordinal%columns)-(columns-1)/2.0f)*(widths[band]+36); node->y=top+(ordinal/columns)*(heights[band]+44)+heights[band]/2;
            node->axis=node->y; node->anchor=node->x; node->vx=node->vy=0; ++ordinal;
        }
        top+=((counts[band]+columns-1)/columns)*(heights[band]+44)+110;
    }
    scene->layout.height=top;
}
static GD_QueryScene *build_scene(GD_Allocator allocator, const GD_Query *query, const GD_EffectQuery *scope, GD_QueryGhostMode ghosts, int primary, int weighted, int include_excluded, int work_limit) {
    GD_QueryScene *scene;
    if (!query || !allocator.allocate || !allocator.release || (scope && (!query->effects || scope->graph!=query->effects)) || (query->request.kind!=GD_QUERY_BINDINGS && query->request.kind!=GD_QUERY_ACTOR_STATES && query->request.kind!=GD_QUERY_CONTRIBUTORS && query->request.kind!=GD_QUERY_STATE_COVERAGE) || ghosts<GD_QUERY_ALL_VARIANTS || ghosts>GD_QUERY_STACKED_VARIANTS || work_limit<0 || work_limit>GD_EFFECT_WORK || (query->variants && (primary<-1 || primary>=query->variants->variant_count))) return NULL;
    scene=allocator.allocate(allocator.context,sizeof(*scene)); if (!scene) return NULL;
    memset(scene,0,sizeof(*scene)); scene->allocator=allocator; scene->query=query; scene->effect_scope=scope; scene->ghosts=ghosts; scene->primary=primary; scene->weighted=weighted!=0; scene->include_excluded=include_excluded!=0; scene->work_limit=work_limit;
    if (query->request.kind==GD_QUERY_BINDINGS) objects(scene); else if (query->state_coverage) coverage_cards(scene); else if (query->effects) effects(scene); else actors(scene);
    if (scene->limited) scene->unexamined_items_upper_bound=query->effects ? query->effects->node_count+query->effects->link_count:query->variants ? query->variants->activation_count+query->variants->write_count+query->rules->participant_count+query->variants->effect_count+query->variants->variant_count+1:query->state_coverage ? query->state_coverage->value_count:query->model->record_count;
    scene->layout.group_count=scene->layout.node_count ? 1:0; scene->layout.horizontal=1; scene->layout.direct_routes=1;
    if (!gd_layout_arrange(&scene->layout,GD_LAYOUT_DIRECTED,GD_SORT_NAME)) { gd_query_scene_destroy(scene); return NULL; }
    if (scope || query->state_coverage || query->request.kind==GD_QUERY_BINDINGS) {
        if (charge(scene,scene->layout.node_count*(GD_MAX_RULE_EXPANSION_DEPTH+3))) wrap_context(scene);
        else scene->unexamined_items_upper_bound=query->state_coverage ? query->state_coverage->value_count:query->effects ? query->effects->node_count+query->effects->link_count:query->model->record_count;
    }
    if (ghosts==GD_QUERY_STACKED_VARIANTS && query->variants && primary>=0) {
        int i,anchor=-1; float offset=0;
        for (i=0;i<scene->layout.node_count;++i) if (scene->nodes[i].kind==GD_QUERY_NODE_VARIANT && scene->nodes[i].variant==primary) anchor=i;
        if (anchor>=0) for (i=0;i<scene->layout.node_count;++i) if (scene->nodes[i].kind==GD_QUERY_NODE_VARIANT && i!=anchor) { offset+=26; scene->layout.nodes[i].x=scene->layout.nodes[anchor].x; scene->layout.nodes[i].y=scene->layout.nodes[anchor].y+offset; }
    }
    {
        int i;for(i=0;i<scene->layout.node_count;++i) scene->native_points[i]=(GD_LayoutPoint){scene->layout.nodes[i].x,scene->layout.nodes[i].y};
        scene->native_group=scene->layout.groups[0];scene->native_width=scene->layout.width;scene->native_height=scene->layout.height;
    }
    return scene;
}
GD_QueryScene *gd_query_scene_build(const GD_Query *query, GD_QueryGhostMode ghosts, int primary, int weighted, int include_excluded, int work_limit) { if (!query) return NULL; return build_scene(query->allocator,query,NULL,ghosts,primary,weighted,include_excluded,work_limit); }
GD_QueryScene *gd_query_scene_effect_context(const GD_Query *query, const GD_EffectQuery *scope, int weighted, int work_limit) { if (!query || !scope) return NULL; return build_scene(query->allocator,query,scope,GD_QUERY_ALL_VARIANTS,-1,weighted,0,work_limit); }
GD_QueryScene *gd_query_scene_actor_topology(GD_Allocator allocator, const GD_Query *query, int work_limit) { if (!query || !query->variants || query->request.kind!=GD_QUERY_ACTOR_STATES) return NULL; return build_scene(allocator,query,NULL,GD_QUERY_ALL_VARIANTS,-1,0,1,work_limit); }
int gd_query_scene_compare(GD_QueryScene *scene, int first, int second) {
    const GD_ActorQuery *query; int i;
    if (!scene || !(query=scene->query->variants) || first<0 || first>=query->variant_count || second<0 || second>=query->variant_count || !charge(scene,scene->layout.node_count)) return 0;
    for (i=0;i<scene->layout.node_count;++i) {
        GD_QuerySceneNode *node=&scene->nodes[i]; node->comparison=0;
        if (node->kind==GD_QUERY_NODE_BINDING) node->comparison=3;
        else if (node->kind==GD_QUERY_NODE_VARIANT) node->comparison=(node->variant==first) | ((node->variant==second)<<1);
        else if (node->kind==GD_QUERY_NODE_EFFECT) node->comparison=gd_actor_compare_member(query,first,second,GD_ACTOR_SET_EFFECTS,node->source);
        else if (node->kind==GD_QUERY_NODE_PARTNER) node->comparison=gd_actor_compare_member(query,first,second,GD_ACTOR_SET_PARTNERS,node->source);
        else if (node->kind==GD_QUERY_NODE_ACTIVATION && node->applicability!=GD_RULE_TRUTH_FALSE) node->comparison=gd_actor_compare_member(query,first,second,GD_ACTOR_SET_CASES,query->activations[node->source].case_index);
    }
    return 1;
}
int gd_query_scene_hit(const GD_QueryScene *scene, float x, float y) {
    int pass,i; if (!scene) return -1;
    for (pass=0;pass<2;++pass) for (i=scene->layout.node_count-1;i>=0;--i) {
        const GD_LayoutNode *node=&scene->layout.nodes[i];
        if (!!scene->nodes[i].ghost!=pass) continue;
        if (x>=node->x-node->width/2 && x<=node->x+node->width/2 && y>=node->y-node->height/2 && y<=node->y+node->height/2) return i;
    }
    return -1;
}
void gd_query_scene_destroy(GD_QueryScene *scene) { if (scene) {if(scene->overview) scene->allocator.release(scene->allocator.context,scene->overview);if(scene->free_geometry) scene->allocator.release(scene->allocator.context,scene->free_geometry);scene->allocator.release(scene->allocator.context,scene);} }
int gd_query_scene_record(const GD_QueryScene *scene, int node) {
    const GD_QuerySceneNode *item; const GD_Query *query;
    if (!scene || node<0 || node>=scene->layout.node_count) return -1;
    item=&scene->nodes[node]; query=scene->query;
    if (item->kind==GD_QUERY_NODE_OBJECT || item->kind==GD_QUERY_NODE_AXIS) return item->source;
    if (query->state_coverage) return query->request.axis;
    if (query->effects) {
        const GD_EffectNode *effect=&query->effects->nodes[item->source];
        if (effect->kind==GD_EFFECT_ACTIVATION) return case_record(query,query->resource->nodes[effect->activation].case_index);
        if (effect->relation>=0) return case_record(query,query->resource->relations[effect->relation].source_case);
        return effect->axis;
    }
    if (item->kind==GD_QUERY_NODE_ACTIVATION) return case_record(query,query->variants->activations[item->source].case_index);
    if (item->kind==GD_QUERY_NODE_EFFECT) { const GD_ActorEffect *effect=&query->variants->effects[item->source]; return effect->origin_case>=0 ? case_record(query,effect->origin_case):effect->axis; }
    return case_record(query,query->variants->case_index);
}
