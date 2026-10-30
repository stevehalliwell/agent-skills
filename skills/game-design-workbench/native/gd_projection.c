#include "gd_projection.h"
#include <string.h>

static const char *const names[]={"skills","challenges","relationships","matrix","mda","resources","actor-inventory","effects"};
const char *gd_projection_name(GD_ExplorerView view) { return view>=GD_EXPLORER_SKILLS && view<=GD_EXPLORER_EFFECTS ? names[view]:NULL; }
int gd_projection_find(const char *name) { int i; if (name && !strcmp(name,"actors")) return GD_EXPLORER_ACTORS; if (name) for (i=GD_EXPLORER_SKILLS;i<=GD_EXPLORER_EFFECTS;++i) if (!strcmp(name,names[i])) return i; return -1; }
const char *gd_resource_label(const GD_ResourceGraph *graph, int index) {
    const GD_ResourceNode *node;
    if (!graph || index<0 || index>=graph->node_count) return NULL;
    node=&graph->nodes[index];
    if (node->kind==GD_RESOURCE_ROLE) return node->pool_reference;
    if (node->kind==GD_RESOURCE_CASE) return node->instance ? node->instance_label:graph->rules->cases[node->case_index].name;
    if (node->pool_reference) return node->pool_reference;
    if (node->axis>=0) return graph->model->records[node->axis].reference;
    return "unresolved axis";
}
int gd_resource_layout(const GD_ResourceGraph *graph, GD_Layout *layout) {
    int i;
    if (!graph || !layout) return 0;
    memset(layout,0,sizeof(*layout)); layout->node_count=graph->node_count; layout->edge_count=graph->relation_count; layout->group_count=graph->node_count ? 1:0;
    for (i=0;i<graph->node_count;++i) { layout->nodes[i].key=i; layout->nodes[i].label=gd_resource_label(graph,i); layout->nodes[i].group=0; layout->nodes[i].side=graph->nodes[i].kind==GD_RESOURCE_CASE ? 1:0; }
    for (i=0;i<graph->relation_count;++i) { const GD_ResourceRelation *source=&graph->relations[i]; layout->edges[i]=(GD_LayoutEdge){source->facet,source->activation,i,1,GD_RELATION_RESOURCE_BASE+source->kind}; }
    return gd_layout_arrange(layout,GD_LAYOUT_FORCE,GD_SORT_NAME);
}
int gd_actor_inventory_layout(const GD_ActorInventory *inventory, GD_Layout *layout) {
    const GD_Rules *rules; const GD_Model *model; int i;
    if (!inventory || !layout) return 0;
    rules=inventory->source->rules; model=inventory->source->model;
    memset(layout,0,sizeof(*layout)); layout->node_count=inventory->node_count; layout->edge_count=inventory->link_count; layout->group_count=4;
    for (i=0;i<inventory->node_count;++i) {
        const GD_ActorInventoryNode *node=&inventory->nodes[i]; GD_LayoutNode *target=&layout->nodes[i];
        target->key=i; target->group=(int)node->kind;
        target->label=node->kind==GD_ACTOR_CASE ? rules->cases[node->case_index].name:node->kind==GD_ACTOR_LOCAL_BINDING ? rules->participants[node->participant].name:model->records[node->record].reference;
    }
    for (i=0;i<inventory->link_count;++i) {
        const GD_ActorInventoryLink *edge=&inventory->links[i];
        layout->edges[i]=(GD_LayoutEdge){edge->from,edge->to,i,1,GD_RELATION_ACTOR_BASE+(int)edge->kind};
    }
    return 1;
}
int gd_effect_projection_layout(const GD_EffectGraph *graph, GD_Layout *layout) {
    const GD_ResourceGraph *source; int i;
    if (!graph || !layout) return 0;
    source=graph->source; memset(layout,0,sizeof(*layout)); layout->node_count=graph->node_count; layout->edge_count=graph->link_count; layout->group_count=graph->node_count ? 1:0;
    for (i=0;i<graph->node_count;++i) {
        const GD_EffectNode *node=&graph->nodes[i]; GD_LayoutNode *target=&layout->nodes[i];
        target->key=i; target->group=0; target->label=node->kind==GD_EFFECT_ACTIVATION ? gd_resource_label(source,node->activation):node->axis>=0 ? source->model->records[node->axis].reference:gd_effect_kind_name(node->kind);
    }
    for (i=0;i<graph->link_count;++i) { const GD_EffectLink *edge=&graph->links[i]; layout->edges[i]=(GD_LayoutEdge){edge->from,edge->to,i,1,GD_RELATION_EFFECT_BASE+(int)edge->kind}; }
    return 1;
}
int gd_projection_layout(const GD_ExplorerProject *project, GD_ExplorerView view, const GD_Focus *focus, GD_Layout *layout) {
    GD_Model *model; GD_Focus all; int i;
    if (!project || view<GD_EXPLORER_SKILLS || view>GD_EXPLORER_RESOURCES || !(project->available&(1u<<view)) || !layout) return 0;
    model=gd_explorer_model(project,view);
    switch (view) {
        case GD_EXPLORER_SKILLS:return gd_skill_layout(model,layout,0,0,GD_SORT_NAME);
        case GD_EXPLORER_CHALLENGES:return gd_route_layout(model,layout,0,GD_SORT_NAME);
        case GD_EXPLORER_RELATIONSHIPS:
            if (focus) { all=*focus; return gd_focus_layout(model,&all,layout,GD_SORT_NAME); }
            memset(&all,0,sizeof(all)); all.root=-1; for (i=0;i<model->record_count;++i) all.visible[i]=1;
            return gd_focus_layout(model,&all,layout,GD_SORT_NAME);
        case GD_EXPLORER_MATRIX:return gd_matrix_layout(model,layout,GD_LAYOUT_FORCE,GD_SORT_NAME);
        case GD_EXPLORER_MDA:return gd_mda_layout(model,project->mda,layout,1);
        case GD_EXPLORER_RESOURCES:return gd_resource_layout(project->resources,layout);
        default:return 0;
    }
}
int gd_projection_record(const GD_ExplorerProject *project, GD_ExplorerView view, int key) {
    if (!project || view<GD_EXPLORER_SKILLS || view>GD_EXPLORER_RESOURCES || key<0) return -1;
    if (view!=GD_EXPLORER_RESOURCES) { const GD_Model *model=gd_explorer_model(project,view); return model && key<model->record_count ? key:-1; }
    return gd_explorer_resource_record(project->resources,key);
}
int gd_projection_edge_record(const GD_ExplorerProject *project, GD_ExplorerView view, int key) {
    const GD_Model *model;
    if (!project || view<GD_EXPLORER_SKILLS || view>GD_EXPLORER_RESOURCES || key<0 || !(model=gd_explorer_model(project,view))) return -1;
    if (view==GD_EXPLORER_MATRIX) return -1;
    if (view==GD_EXPLORER_SKILLS) return key<model->skill_link_count ? model->skill_links[key].dependent:-1;
    if (view==GD_EXPLORER_MDA) { if (!project->mda || key>=project->mda->link_count) return -1; key=project->mda->links[key].edge; }
    if (view==GD_EXPLORER_RESOURCES) { const GD_ResourceRelation *relation; if (!project->resources || key>=project->resources->relation_count) return -1; relation=&project->resources->relations[key]; return relation->source_case>=0 ? project->rules->sources[project->rules->cases[relation->source_case].source].record:-1; }
    return key<model->edge_count ? model->edges[key].source:-1;
}
