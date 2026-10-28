#include "gd_explorer.h"
#include <assert.h>
#include <string.h>

const char *const gd_explorer_view_names[GD_EXPLORER_VIEWS]={"Skills","Challenges","Relationships","Interactions","MDA","Resources"};
GD_ExplorerFamily gd_explorer_family(GD_ExplorerView view) {
    assert(view>=0 && view<GD_EXPLORER_VIEWS);
    return view<=GD_EXPLORER_RELATIONSHIPS ? GD_EXPLORER_SKILL_DATA :
        view==GD_EXPLORER_MATRIX ? GD_EXPLORER_MATRIX_DATA : view==GD_EXPLORER_MDA ? GD_EXPLORER_MDA_DATA:GD_EXPLORER_RULE_DATA;
}
int gd_explorer_record(const GD_Model *model, const char *reference) {
    int i,found=-1;
    if (!model || !reference || !*reference) return -1;
    for (i=0;i<model->record_count;++i) if (!strcmp(model->records[i].reference,reference)) {
        if (found>=0) return -1;
        found=i;
    }
    return found;
}
GD_Model *gd_explorer_model(const GD_ExplorerProject *project, GD_ExplorerView view) {
    GD_ExplorerFamily family=gd_explorer_family(view);
    return family==GD_EXPLORER_SKILL_DATA && project->draft ? project->draft->model:project->models[family];
}
const GD_Error *gd_explorer_error(const GD_ExplorerProject *project) {
    int i;
    if (!project) return NULL;
    for (i=0;i<GD_EXPLORER_FAMILIES;++i) if (project->models[i] && project->models[i]->error_count) return &project->models[i]->errors[0];
    if (project->draft && project->draft->model->error_count) return &project->draft->model->errors[0];
    return NULL;
}
void gd_explorer_destroy(GD_ExplorerProject *project) {
    int i;
    if (!project) return;
    gd_resource_graph_destroy(project->resources); gd_rule_analysis_destroy(project->calls); gd_rules_destroy(project->rules);
    gd_mda_destroy(project->mda); gd_routes_destroy(project->routes); gd_draft_destroy(project->draft);
    for (i=0;i<GD_EXPLORER_FAMILIES;++i) gd_destroy(project->models[i]);
    project->allocator.release(project->allocator.context,project);
}
static GD_Role family_role(GD_ExplorerFamily family, GD_Role role) {
    if (family==GD_EXPLORER_SKILL_DATA) return role==GD_SKILL || role==GD_CHALLENGE ? role:GD_RELATED;
    if (family==GD_EXPLORER_MATRIX_DATA) return role==GD_NOUN || role==GD_VERB || role==GD_CASE ? role:GD_RELATED;
    if (family==GD_EXPLORER_MDA_DATA) return gd_mda_role(role) ? role:GD_RELATED;
    if (role==GD_MDA_MECHANIC || role==GD_CASE || role==GD_RULE) return GD_RULE;
    return role==GD_NOUN || role==GD_VERB || role==GD_CONTROL || role==GD_AXIS || role==GD_RESOURCE ? role:GD_RELATED;
}
int gd_explorer_edited(GD_ExplorerProject *project) {
    GD_Model *model;
    if (!project || !project->draft) return 0;
    model=project->draft->model;
    gd_routes_destroy(project->routes); project->routes=NULL;
    gd_recompute_skills(model);
    if (model->route_mode) project->routes=gd_analyze_routes(model);
    ++project->revision;
    return !model->error_count && (!model->route_mode || project->routes!=NULL);
}
GD_ExplorerProject *gd_explorer_load(GD_Allocator allocator, const GD_Collection *collections, int count) {
    GD_ExplorerProject *project;
    unsigned roles=0; int i,f;
    GD_Collection inputs[GD_MAX_COLLECTIONS];
    assert(allocator.allocate && allocator.release);
    if (!collections || count<1 || count>GD_MAX_COLLECTIONS) return NULL;
    project=allocator.allocate(allocator.context,sizeof(*project));
    if (!project) return NULL;
    memset(project,0,sizeof(*project)); project->allocator=allocator; project->revision=1;
    for (i=0;i<count;++i) {
        if (collections[i].role<GD_NOUN || collections[i].role>GD_RESOURCE) { gd_explorer_destroy(project); return NULL; }
        roles|=1u<<collections[i].role;
    }
    if (roles&(1u<<GD_SKILL)) {
        project->available|=(1u<<GD_EXPLORER_SKILLS)|(1u<<GD_EXPLORER_RELATIONSHIPS);
        if (roles&(1u<<GD_CHALLENGE)) project->available|=1u<<GD_EXPLORER_CHALLENGES;
    }
    if ((roles&7u)==7u) project->available|=1u<<GD_EXPLORER_MATRIX;
    { unsigned required=(1u<<GD_MDA_MECHANIC)|(1u<<GD_MDA_DYNAMIC)|(1u<<GD_MDA_AESTHETIC);
      if ((roles&required)==required) project->available|=1u<<GD_EXPLORER_MDA; }
    if (roles&((1u<<GD_CASE)|(1u<<GD_MDA_MECHANIC)|(1u<<GD_RULE))) project->available|=1u<<GD_EXPLORER_RESOURCES;
    for (f=0;f<GD_EXPLORER_FAMILIES;++f) {
        int wanted=0,v;
        for (v=0;v<GD_EXPLORER_VIEWS;++v) if ((project->available&(1u<<v)) && gd_explorer_family((GD_ExplorerView)v)==(GD_ExplorerFamily)f) wanted=1;
        if (!wanted) continue;
        project->models[f]=gd_create(allocator);
        if (!project->models[f]) { gd_explorer_destroy(project); return NULL; }
        for (i=0;i<count;++i) { inputs[i]=collections[i]; inputs[i].role=family_role((GD_ExplorerFamily)f,collections[i].role); }
        if (!gd_load(project->models[f],inputs,count)) return project;
        if (f==GD_EXPLORER_SKILL_DATA) {
            project->draft=gd_draft_create(project->models[f]);
            if (!project->draft || !gd_explorer_edited(project)) {
                gd_error(project->models[f],"explorer.draft","Cannot prepare shared skill draft/routes",NULL,0); return project;
            }
        } else if (f==GD_EXPLORER_MDA_DATA) {
            project->mda=gd_analyze_mda(project->models[f]);
            if (!project->mda) { gd_error(project->models[f],"explorer.mda","Cannot prepare MDA projection",NULL,0); return project; }
        } else if (f==GD_EXPLORER_RULE_DATA) {
            project->rules=gd_read_rules(project->models[f]);
            if (project->rules) project->calls=gd_analyze_rules(project->models[f],project->rules);
            if (project->calls) project->resources=gd_resource_graph_build(project->models[f],project->rules);
            if (!project->resources) {
                gd_error(project->models[f],"explorer.resources","Cannot prepare resource incidence projection",NULL,0); return project;
            }
            gd_resource_graph_bind_calls(project->resources,project->calls); /* Limited projections remain explicitly partial. */
        }
        if (gd_explorer_error(project)) return project;
    }
    for (f=0;f<GD_EXPLORER_FAMILIES;++f) if (project->models[f] && gd_sources_changed(project->models[f])) {
        gd_error(project->models[f],"input.changed","Sources changed across explorer snapshot preparation; retry explicitly",NULL,0); break;
    }
    return project;
}
int gd_explorer_resource_record(const GD_ResourceGraph *graph, int index) {
    const GD_ResourceNode *node;
    if (!graph || index<0 || index>=graph->node_count) return -1;
    node=&graph->nodes[index];
    if (node->record>=0) return node->record;
    if (node->kind==GD_RESOURCE_FACET && node->pool_reference)
        return node->pool>=0 ? graph->rules->pools[node->pool].resource_record:-1;
    if (node->case_index>=0) return graph->rules->sources[graph->rules->cases[node->case_index].source].record;
    return node->axis;
}
static const char *record_reference(const GD_ResourceGraph *graph, int record) {
    return record>=0 ? graph->model->records[record].reference:"";
}
static int same_resource_node(const GD_ResourceGraph *a, int ai, const GD_ResourceGraph *b, int bi) {
    const GD_ResourceNode *x=&a->nodes[ai],*y=&b->nodes[bi];
    if (x->kind!=y->kind) return 0;
    if (x->kind==GD_RESOURCE_FACET && (x->pool_reference || y->pool_reference))
        return x->pool_reference && y->pool_reference && !strcmp(x->pool_reference,y->pool_reference) &&
            !strcmp(record_reference(a,x->axis),record_reference(b,y->axis));
    if ((x->case_index>=0)!=(y->case_index>=0) || (x->participant>=0)!=(y->participant>=0)) return 0;
    if (strcmp(record_reference(a,x->record),record_reference(b,y->record)) ||
        strcmp(record_reference(a,x->axis),record_reference(b,y->axis)) ||
        strcmp(x->pool_reference ? x->pool_reference:"",y->pool_reference ? y->pool_reference:"")) return 0;
    if (x->case_index>=0) {
        const GD_RuleCase *xc=&a->rules->cases[x->case_index],*yc=&b->rules->cases[y->case_index];
        if (strcmp(xc->name,yc->name) || strcmp(record_reference(a,a->rules->sources[xc->source].record),
                                             record_reference(b,b->rules->sources[yc->source].record))) return 0;
    }
    if (x->participant>=0 && strcmp(a->rules->participants[x->participant].name,b->rules->participants[y->participant].name)) return 0;
    return 1;
}
int gd_explorer_resource_remap(const GD_ResourceGraph *old, const GD_ResourceGraph *fresh, int node) {
    int i,found=-1;
    if (!old || !fresh || node<0 || node>=old->node_count) return -1;
    for (i=0;i<fresh->node_count;++i) if (same_resource_node(old,node,fresh,i)) {
        if (found>=0) return -1;
        found=i;
    }
    return found;
}
