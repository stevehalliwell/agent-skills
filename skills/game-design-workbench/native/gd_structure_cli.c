#include "gd_structure_cli.h"
#include "gd_resource_routes.h"
#include <string.h>

static void member(FILE *out, const char *name, const char *value) { fputc(',',out); gd_json_string(out,name); fputc(':',out); gd_json_string(out,value); }
static int field(const char *fields, const char *name) {
    const char *start=fields; size_t length=strlen(name);
    if (!fields) return 1;
    while (*start) { const char *end=strchr(start,','); size_t count=end ? (size_t)(end-start):strlen(start); if (count==length && !strncmp(start,name,count)) return 1; if (!end) break; start=end+1; }
    return 0;
}
int gd_structure_fields(const char *fields) {
    const char *start=fields;
    if (!fields || !*fields) return 0;
    while (*start) { const char *end=strchr(start,','); size_t count=end ? (size_t)(end-start):strlen(start); if (!((count==5 && !strncmp(start,"nodes",5)) || (count==5 && !strncmp(start,"links",5)) || (count==8 && !strncmp(start,"evidence",8)) || (count==12 && !strncmp(start,"declarations",12)) || (count==8 && !strncmp(start,"findings",8)))) return 0; if (!end) return 1; start=end+1; }
    return 0;
}
int gd_write_structure_catalogue(FILE *out, int json, int comma) {
    int kind;
    for (kind=0;kind<GD_STRUCTURE_COUNT;++kind) {
        int path=kind==GD_STRUCTURE_REACHABILITY || kind==GD_STRUCTURE_DOMINATORS || kind==GD_STRUCTURE_BYPASS;
        if (!json) { fprintf(out,"%s [skills|challenges|relationships|matrix|mda|resources|actor-inventory|actors]: operation-specific parameters in query --list --json; structural evidence only\n",gd_structure_name((GD_StructureKind)kind)); continue; }
        if (comma++) fputc(',',out);
        fputs("{\"name\":",out); gd_json_string(out,gd_structure_name((GD_StructureKind)kind));
        fputs(",\"projections\":[\"skills\",\"challenges\",\"relationships\",\"matrix\",\"mda\",\"resources\",\"actor-inventory\",\"actors\"],\"required_bindings_by_projection\":{\"skills\":{\"all_of\":[\"skill-atoms\"],\"any_of\":[]},\"challenges\":{\"all_of\":[\"skill-atoms\",\"challenges\"],\"any_of\":[]},\"relationships\":{\"all_of\":[\"skill-atoms\"],\"any_of\":[]},\"matrix\":{\"all_of\":[\"nouns\",\"verbs\",\"interactions\"],\"any_of\":[]},\"mda\":{\"all_of\":[\"mechanics\",\"dynamics\",\"aesthetics\"],\"any_of\":[]},\"resources\":{\"all_of\":[],\"any_of\":[\"interactions\",\"mechanics\"]},\"actor-inventory\":{\"all_of\":[\"nouns\",\"axes\"],\"any_of\":[\"interactions\",\"mechanics\"]},\"actors\":{\"all_of\":[\"nouns\",\"axes\"],\"any_of\":[\"interactions\",\"mechanics\"]}}",out);
        if (kind==GD_STRUCTURE_DECLARATIONS) {
            fputs(",\"required_parameters\":[],\"parameters\":[\"--work-limit (search budget only; diagnostic inventory is fixed bounded work)\"],\"repeatable_parameters\":[],\"fields\":[\"nodes\",\"links\",\"evidence\",\"declarations\",\"findings\"],\"ui_available\":true,\"ui_scope\":\"canonical projection loaded-source diagnostic context, not canvas/graph filters; native identity gaps and findings, no new validation rules\",\"ui_unavailable_parameters\":[\"repeatable input beyond 8192 combined bytes per list\",\"graph selectors/status/relation/scope filters (rejected, not ignored)\"],\"ipc_available\":false,\"work_limit_max\":16777216}",out); continue;
        }
        fputs(",\"required_parameters_by_projection\":{\"actors\":[\"--case\",\"--axis\"]},\"parameters_by_projection\":{\"actors\":[\"--object\",\"--participant (default authored A)\",\"--value (repeat; numeric requires explicit samples)\",\"--assume (repeat)\",\"--context (repeat)\",\"--not-context (repeat)\"]}",out);
        fputs(",\"required_parameters\":[",out); if (path) gd_json_string(out,"at least one --from or --start"); if (kind==GD_STRUCTURE_BYPASS) fputs(",\"--to\"",out); fputs("],\"parameters\":[\"--relation\",\"--scope-root (Relationships only)\",\"--status\",\"--include-inactive\",\"--work-limit\"",out);
        if (path || kind==GD_STRUCTURE_RELATED || kind==GD_STRUCTURE_CONNECTIONS) fputs(",\"--from\"",out);
        if (path || kind==GD_STRUCTURE_RELATED) fputs(",\"--to\"",out);
        if (path) fputs(",\"--start\",\"--direction forward|backward|both\"",out);
        if (kind==GD_STRUCTURE_RELATED) fputs(",\"--lens any|same-role|changed-role\"",out);
        if (kind==GD_STRUCTURE_BYPASS) fputs(",\"--avoid\"",out);
        fputs("],\"repeatable_parameters\":[\"--relation\",\"--status\"",out); if (path) fputs(",\"--start\"",out);
        fputs("],\"fields\":[\"nodes\",\"links\",\"evidence\",\"declarations\",\"findings\"],\"ui_available\":true,\"ui_scope\":\"Built-in structural questions in seven existing panes; Actors explicitly selects declaration inventory or applied finite actor-states topology, independent of ghosts/primary/compare/excluded drawing controls; canonical projection plus explicit filters; result tables and source navigation, not current canvas filters or a query overlay\",\"ui_parameter_encoding\":\"Repeated starts, relations and statuses: one trimmed literal per line\",\"ui_unavailable_parameters\":[\"repeatable input beyond 8192 combined bytes per list\",\"--fields JSON pruning (expand result tables instead)\"],\"ipc_available\":false,\"work_limit_max\":16777216,\"bypass_scope\":\"native nonmandatory support-path candidates only; other selections return unavailable, not no alternative\",\"relationship_path_default\":\"all consumed kinds except next; explicit --relation next or --relation all includes continuation\",\"maximum_resource_selectors\":67,\"native_resource_selector_limit_each\":16777216,\"native_actor_inventory_selector_limit_each\":16777216,\"fixed_construction\":\"4096 nodes, 32768 rows, at most 65536 typed incidences and bounded native sorts; separate from search budget\"}",out);
    }
    return !ferror(out);
}
static int status_matches(const GD_StructureOptions *options, const GD_Record *record) {
    int i; if (!options->status_count || !strcmp(options->statuses[0],"all")) return 1;
    for (i=0;i<options->status_count;++i) if (record && record->status && !strcmp(options->statuses[i],record->status)) return 1;
    return 0;
}
int gd_structure_result_record(const GD_StructureResult *result, int key) {
    if (!result) return -1;
    if (result->actor_scene) return gd_query_scene_record(result->actor_scene,key);
    if (result->inventory) return key>=0 && key<result->inventory->node_count ? result->inventory->nodes[key].record:-1;
    return gd_projection_record(result->project,result->view,key);
}
int gd_structure_result_edge_record(const GD_StructureResult *result, int key) {
    if (!result) return -1;
    if (result->actor_scene) return key>=0 && key<result->actor_scene->layout.edge_count ? gd_query_scene_record(result->actor_scene,result->actor_scene->layout.edges[key].a):-1;
    if (result->inventory) return key>=0 && key<result->inventory->link_count ? result->inventory->nodes[result->inventory->links[key].from].record:-1;
    return gd_projection_edge_record(result->project,result->view,key);
}
int gd_structure_result_node_id(const GD_StructureResult *result, int key, char *text, size_t size) {
    const GD_Model *model; int count;
    if (!text || !size) return 0; text[0]=0;
    if (!result) return 0;
    if (result->actor_scene) { int work=0; return gd_actor_scene_node_id(result->actor_scene,key,text,size,&work,GD_ACTOR_WORK); }
    if (result->inventory) return gd_actor_inventory_node_id(result->inventory,key,text,size);
    if (result->view==GD_EXPLORER_RESOURCES) return gd_resource_node_id(result->project->resources,key,-1,text,size);
    model=gd_explorer_model(result->project,result->view); if (!model || key<0 || key>=model->record_count) return 0;
    count=snprintf(text,size,"%s",model->records[key].reference);
    if (count<0 || (size_t)count>=size) { text[0]=0; return 0; } return 1;
}
static int selector(const GD_StructureResult *result, const GD_Layout *layout, GD_ResourceRoutes *resolver, const char *name, int *work) {
    const GD_ExplorerProject *project=result->project; GD_ExplorerView view=result->view; int i,record,found=-1;
    if (!name) return -1;
    if (result->inventory || result->actor_scene) {
        char identity[GD_MAX_RULE_LINE+1]; int local_work=0;
        for (i=0;i<layout->node_count;++i) {
            if (result->actor_scene) {
                if (!gd_actor_scene_node_id(result->actor_scene,layout->nodes[i].key,identity,sizeof(identity),&local_work,GD_RESOURCE_ROUTE_WORK)) { *work+=local_work; return -3; }
            } else {
                const GD_ActorInventoryNode *node=&result->inventory->nodes[layout->nodes[i].key]; const GD_Record *source=&result->inventory->source->model->records[node->record];
                const GD_Rules *rules=result->inventory->source->rules; size_t cost=80+strlen(source->reference)+(source->id ? strlen(source->id):0);
                if (node->case_index>=0) cost+=strlen(rules->cases[node->case_index].name);
                if (node->participant>=0) cost+=strlen(rules->participants[node->participant].name);
                if (cost>(size_t)(GD_RESOURCE_ROUTE_WORK-local_work)) { *work+=local_work; return -3; }
                local_work+=(int)cost;
                if (!gd_structure_result_node_id(result,layout->nodes[i].key,identity,sizeof(identity))) { *work+=local_work; return -3; }
            }
            if (!strcmp(name,identity)) { if (found>=0) { *work+=local_work; return -2; } found=i; }
        }
        *work+=local_work; return found;
    }
    if (view==GD_EXPLORER_RESOURCES) { int key=gd_resource_select(resolver,project->resources,name); *work+=resolver->work; if (key<0) return key; for (i=0;i<layout->node_count;++i) if (layout->nodes[i].key==key) return i; return -1; }
    *work+=gd_explorer_model(project,view)->record_count+layout->node_count;
    record=gd_explorer_record(gd_explorer_model(project,view),name); if (record<0) return -1;
    for (i=0;i<layout->node_count;++i) if (layout->nodes[i].key==record) { if (found>=0) return -2; found=i; }
    return found;
}
static void record_json(FILE *out, const GD_Record *record) {
    static const char *const roles[]={"nouns","verbs","interactions","related","skill-atoms","challenges","mechanics","systems","dynamics","aesthetics","control-vars","rules","axes","resources"};
    fputs("{\"reference\":",out); gd_json_string(out,record->reference); member(out,"record_id",record->id); member(out,"loaded_role",record->role>=GD_NOUN && record->role<=GD_RESOURCE ? roles[record->role]:NULL); member(out,"status",record->status); member(out,"path",record->source_path); fputc('}',out);
}
static const char *selector_error(int value) { return value==-2 ? "query.selector_ambiguous":value==-3 ? "query.selector_limit":"query.selector_missing"; }
static void integers(FILE *out, const unsigned char *values, int count) { int i,comma=0; fputc('[',out); for (i=0;i<count;++i) if (values[i]) { if (comma++) fputc(',',out); fprintf(out,"%d",i); } fputc(']',out); }
static void arcs(FILE *out, const int *values, int count, int reversed) { int i; fputc('[',out); for (i=0;i<count;++i) { if (i) fputc(',',out); fprintf(out,"{\"link\":%d,\"reverse_traversal\":%s}",values[i]/2,(values[i]%2) ^ reversed ? "true":"false"); } fputc(']',out); }
/* Reference identity only. Domain type errors remain native findings/errors. */
const char *gd_declaration_identity_name(GD_DeclarationIdentity identity) {
    static const char *const names[]={"resolved","missing","unloaded","ambiguous"}; return identity>=0 && identity<GD_DECLARATION_STATES ? names[identity]:NULL;
}
static GD_DeclarationIdentity identity_resolution(const GD_Model *model, const GD_Edge *edge) {
    int i; if (edge->target>=0) return GD_DECLARATION_RESOLVED; if (edge->target==-2) return GD_DECLARATION_AMBIGUOUS;
    for (i=0;i<model->collection_count;++i) { size_t length=strlen(model->collections[i].alias); if (!strncmp(edge->reference,model->collections[i].alias,length) && edge->reference[length]=='/') return GD_DECLARATION_MISSING; }
    return GD_DECLARATION_UNLOADED;
}
static void findings(FILE *out, const GD_Finding *values, int count) { int i; fputc('[',out); for (i=0;i<count;++i) { if (i) fputc(',',out); fputs("{\"code\":",out); gd_json_string(out,values[i].code); member(out,"message",values[i].message); member(out,"reference",values[i].reference); fprintf(out,",\"record\":%d,\"line\":%d}",values[i].record,values[i].line); } fputc(']',out); }
static void gap_inventory(FILE *out, const GD_StructureResult *result) {
    const GD_DeclarationGaps *gaps=result->gaps; const GD_Model *model=gd_explorer_model(result->project,result->view); int i,comma=0;
    fprintf(out,",\"declaration_gap_inventory\":{\"scope\":\"whole loaded reader context; consumed explicit references only, independent of graph/canvas filters\",\"missing_metadata_checked\":false,\"inventory_checked\":true,\"rows\":%d,\"resolved\":%d,\"missing\":%d,\"unloaded\":%d,\"ambiguous\":%d,\"gaps\":[",gaps->rows,gaps->counts[GD_DECLARATION_RESOLVED],gaps->counts[GD_DECLARATION_MISSING],gaps->counts[GD_DECLARATION_UNLOADED],gaps->counts[GD_DECLARATION_AMBIGUOUS]);
    for (i=0;i<gaps->rows;++i) if (gaps->identities[i]!=GD_DECLARATION_RESOLVED) {
        const GD_Edge *edge=&model->edges[i]; if (comma++) fputc(',',out);
        fprintf(out,"{\"declaration_index\":%d,\"source\":",i); record_json(out,&model->records[edge->source]); fprintf(out,",\"line\":%d,\"field\":",edge->line); gd_json_string(out,gd_field_name(edge->field)); member(out,"reference",edge->reference); member(out,"identity_resolution",gd_declaration_identity_name((GD_DeclarationIdentity)gaps->identities[i])); fputc('}',out);
    }
    fputs("]}",out);
}
static int projection_incomplete(const GD_ExplorerProject *project, GD_ExplorerView view) {
    const GD_Model *model=gd_explorer_model(project,view); int i;
    if ((view==GD_EXPLORER_RESOURCES && gd_resource_graph_incomplete(project->resources)) || model->skill_incomplete || model->findings_omitted || (view==GD_EXPLORER_MDA && (!project->mda->graph_complete || project->mda->findings_omitted)) || (view==GD_EXPLORER_RESOURCES && project->rules->findings_omitted)) return 1;
    for (i=0;i<model->edge_count;++i) if (model->edges[i].target<0) return 1;
    return 0;
}
int gd_write_structure_result(FILE *out, const GD_StructureResult *result, int json, const char *fields) {
    if (!out || !result || !result->project || !result->query || !result->layout || (fields && (!json || !gd_structure_fields(fields)))) return 0;
    const GD_ExplorerProject *project=result->project; GD_ExplorerView view=result->view; const GD_StructureQuery *query=result->query; const GD_StructureOptions *options=&result->options; int selector_work=result->selector_work;
    const GD_Model *model=gd_explorer_model(project,view); const GD_GraphAnalysis *analysis=&query->analysis; int i,j,comma=0,ids_complete=1,hints[GD_MAX_RECORDS];
    const GD_StructureRequest *request=&query->request; int source_incomplete=result->source_incomplete;
    if (!json) {
        if (result->gaps) fprintf(out,"Declaration inventory: %d explicit reference rows; resolved %d, missing %d, unloaded %d, ambiguous %d; native domain findings separate; no missing-metadata validator\n",result->gaps->rows,result->gaps->counts[GD_DECLARATION_RESOLVED],result->gaps->counts[GD_DECLARATION_MISSING],result->gaps->counts[GD_DECLARATION_UNLOADED],result->gaps->counts[GD_DECLARATION_AMBIGUOUS]);
        fprintf(out,"%s [%s]: %d shown nodes, %d shown rows, %d isolates; search %s; structural evidence, not gameplay frequency/feasibility\n",gd_structure_name(request->kind),options->projection,query->shown_nodes,query->shown_edges,query->isolated,query->complete ? "complete in scope":"PARTIAL");
        for (i=0;i<analysis->node_count;++i) if (analysis->shown[i]) { fprintf(out,"%d %s | ",i,query->layout->nodes[i].label); if (!request->incidence) fprintf(out,"incoming %d outgoing %d ",query->nodes[i].incoming,query->nodes[i].outgoing); fprintf(out,"partners %d incident rows %d kinds %d\n",query->nodes[i].partners,query->nodes[i].incident,query->nodes[i].kinds); }
        fputs("Native evidence/source metadata (JSON; --json emits only this object):\n",out);
    }
    for (i=0;i<GD_MAX_RECORDS;++i) hints[i]=-1;
    if (result->inventory || result->actor_scene) for (i=0;i<result->layout->node_count;++i) { char identity[GD_MAX_PATH]; if (!gd_structure_result_node_id(result,i,identity,sizeof(identity))) ids_complete=0; }
    if (view==GD_EXPLORER_RESOURCES) for (i=0;i<project->resources->relation_count;++i) if (hints[project->resources->relations[i].facet]<0) hints[project->resources->relations[i].facet]=i;
    fputs("{\"schema_version\":1,\"ok\":true,\"data\":{\"question\":",out); gd_json_string(out,gd_structure_name(request->kind)); member(out,"projection",options->projection);
    fputs(",\"selectors\":{\"from\":",out); gd_json_string(out,options->from); member(out,"to",options->to); member(out,"avoid",options->avoid); member(out,"scope_root",options->scope_root); fputs(",\"statuses\":[",out); for (i=0;i<options->status_count;++i) { if (i) fputc(',',out); gd_json_string(out,options->statuses[i]); } fputs("],\"starts\":[",out); for (i=0;i<options->root_count;++i) { if (i) fputc(',',out); gd_json_string(out,options->roots[i]); } fputs("]},\"records\":[",out); for (i=0;i<model->record_count;++i) { if (i) fputc(',',out); fprintf(out,"{\"index\":%d,\"declaration\":",i); record_json(out,&model->records[i]); fputc('}',out); } fputc(']',out);
    fprintf(out,",\"scope\":{\"nodes\":%d,\"links\":%d,\"isolates\":%d,\"incidence\":%s,\"walk_direction\":",query->shown_nodes,query->shown_edges,query->isolated,request->incidence ? "true":"false"); gd_json_string(out,request->incidence ? "undirected incidence":request->direction==GD_STRUCTURE_BACKWARD ? "backward":request->direction==GD_STRUCTURE_BOTH ? "both":"forward");
    fprintf(out,",\"source\":%d,\"target\":%d,\"avoid\":%d,\"roots\":",request->source,request->target,request->avoid); integers(out,query->roots,analysis->node_count); fputs(",\"relation_kinds\":[",out); for (i=0;i<request->relation_count;++i) { if (i) fputc(',',out); gd_json_string(out,gd_graph_relation_name(request->relations[i])); } fputs("]}",out);
    if (field(fields,"nodes")) {
        fputs(",\"nodes\":[",out);
        for (i=0;i<analysis->node_count;++i) {
            const GD_LayoutNode *node=&query->layout->nodes[i]; const GD_StructureNode *summary=&query->nodes[i]; int source=gd_structure_result_record(result,node->key); char id[GD_MAX_PATH];
            if (i) fputc(',',out); fprintf(out,"{\"index\":%d,\"id\":",i);
            if (result->inventory || result->actor_scene) { if (gd_structure_result_node_id(result,node->key,id,sizeof(id))) gd_json_string(out,id); else { fputs("null",out); ids_complete=0; } }
            else if (view==GD_EXPLORER_RESOURCES) { if (gd_resource_node_id(project->resources,node->key,hints[node->key],id,sizeof(id))) gd_json_string(out,id); else { fputs("null",out); ids_complete=0; } } else gd_json_string(out,model->records[node->key].reference);
            member(out,"label",node->label); fprintf(out,",\"shown\":%s,\"source\":",analysis->shown[i] ? "true":"false"); if (source>=0) record_json(out,&model->records[source]); else fputs("null",out);
            if (view==GD_EXPLORER_RESOURCES) { const GD_ResourceNode *resource=&project->resources->nodes[node->key]; fprintf(out,",\"case\":%d,\"parent_placement\":%d,\"instance\":%s,\"frontier\":%s",resource->kind==GD_RESOURCE_CASE ? resource->case_index:-1,resource->kind==GD_RESOURCE_CASE && resource->instance ? resource->parent:-1,resource->instance ? "true":"false",resource->frontier ? "true":"false"); }
            if (result->inventory) {
                const GD_ActorInventoryNode *item=&result->inventory->nodes[node->key]; member(out,"native_kind",gd_actor_inventory_kind_name(item->kind));
                fprintf(out,",\"case\":%d,\"local_participant\":%d",item->case_index,item->participant);
                if (item->participant>=0) { const GD_RuleParticipant *binding=&project->rules->participants[item->participant]; member(out,"local_name",binding->name); member(out,"noun_reference",binding->noun); member(out,"noun_resolution",gd_rule_resolution_name(binding->noun_resolution)); }
            }
            if (result->actor_scene) { const GD_QuerySceneNode *item=&result->actor_scene->nodes[node->key]; member(out,"native_kind",gd_query_scene_node_kind_name(item->kind)); member(out,"applicability",gd_rule_truth_name(item->applicability)); fprintf(out,",\"native_source\":%d,\"variant\":%d",item->source,item->variant); }
            fputs(",\"counts\":",out);
            if (!analysis->shown[i]) fputs("null",out); else { fputs("{\"incoming\":",out); if (request->incidence) fputs("null",out); else fprintf(out,"%d",summary->incoming); fputs(",\"outgoing\":",out); if (request->incidence) fputs("null",out); else fprintf(out,"%d",summary->outgoing); fprintf(out,",\"distinct_partners\":%d,\"incident_rows\":%d,\"incident_kinds\":%d}",summary->partners,summary->incident,summary->kinds); }
            fputc('}',out);
        } fputc(']',out);
    }
    if (field(fields,"links")) {
        fputs(",\"links\":[",out);
        for (i=0;i<analysis->edge_count;++i) {
            const GD_LayoutEdge *edge=&query->layout->edges[i]; int source=gd_structure_result_edge_record(result,edge->key);
            if (i) fputc(',',out); fprintf(out,"{\"index\":%d,\"source_key\":%d,\"from\":%d,\"to\":%d,\"projection_active\":%s,\"eligible\":%s,\"kind\":",i,edge->key,edge->a,edge->b,edge->active ? "true":"false",analysis->edges[i].active && analysis->shown[edge->a] && analysis->shown[edge->b] ? "true":"false"); gd_json_string(out,gd_graph_relation_name(edge->relation));
            fputs(",\"source\":",out); if (source>=0) record_json(out,&model->records[source]); else fputs("null",out);
            if (view==GD_EXPLORER_MATRIX) { const GD_Cell *cell=&model->cells[edge->key]; fprintf(out,",\"derived_case_pair\":true,\"loaded_case_count\":%d,\"source_cases\":[",cell->pair_count); for (j=0;j<cell->pair_count;++j) { if (j) fputc(',',out); record_json(out,&model->records[model->pairs[cell->first_pair+j].interaction]); } fputc(']',out); }
            if (view==GD_EXPLORER_RESOURCES) { const GD_ResourceRelation *relation=&project->resources->relations[edge->key]; fprintf(out,",\"source_case\":%d,\"target_participant\":%d,\"line\":%d,\"contextual_ancestor_input\":%s",relation->source_case,relation->participant,relation->line,relation->contextual_input ? "true":"false"); }
            if (result->inventory) {
                const GD_ActorInventoryLink *item=&result->inventory->links[edge->key]; int line=gd_actor_inventory_link_line(result->inventory,edge->key);
                fprintf(out,",\"native_declaration\":%d,\"compiled_binding\":%d,\"line\":%d",item->declaration,item->binding,line);
            }
            if (result->actor_scene) { const GD_QuerySceneEdge *item=&result->actor_scene->edges[edge->key]; fprintf(out,",\"native_source\":%d,\"variant\":%d",item->source,item->variant); }
            fputc('}',out);
        } fputc(']',out);
    }
    if (field(fields,"evidence")) {
        fputs(",\"evidence\":{\"marked_nodes\":",out); integers(out,query->marked_nodes,analysis->node_count); fputs(",\"marked_links\":",out); integers(out,query->marked_edges,analysis->edge_count);
        if (request->kind==GD_STRUCTURE_RELATED) { if (analysis->comparison.valid) { const GD_GraphComparison *comparison=&analysis->comparison; fprintf(out,",\"comparison\":{\"shared\":%d,\"first_only\":%d,\"second_only\":%d,\"other_role\":%d}",comparison->shared,comparison->a_only,comparison->b_only,comparison->other_role); } else { fputs(",\"pairs\":[",out); for (i=0;i<analysis->pair_count;++i) { if (i) fputc(',',out); fprintf(out,"{\"first\":%d,\"second\":%d,\"shared\":%d}",analysis->pairs[i].a,analysis->pairs[i].b,analysis->pairs[i].shared); } fputc(']',out); } }
        if (request->kind==GD_STRUCTURE_REACHABILITY) { const GD_GraphClosure *closure=&analysis->reachability; fputs(",\"rows\":[",out); for (i=0;i<analysis->node_count;++i) if (query->roots[i]) { if (comma++) fputc(',',out); fprintf(out,"{\"root\":%d,\"finished\":%s,\"cycle\":%s,\"targets\":[",i,closure->finished[i] ? "true":"false",!closure->finished[i] ? "null":closure->cycle[i] ? "true":"false"); { int count=0; for (j=0;j<analysis->node_count;++j) if (gd_graph_closure_contains(closure,i,j)) { if (count++) fputc(',',out); fprintf(out,"%d",j); } } fputs("]}",out); } fputc(']',out); if (request->target>=0 && request->source>=0) { int values[64],count=gd_graph_closure_path(analysis,closure,0,request->target,values,64); fprintf(out,",\"support_length\":%d,\"support_suffix\":",count); arcs(out,values,count<0 ? 0:count<64 ? count:64,request->direction==GD_STRUCTURE_BACKWARD); } }
        if (request->kind==GD_STRUCTURE_DOMINATORS || request->kind==GD_STRUCTURE_BYPASS) { const GD_GraphPaths *paths=&analysis->paths; const GD_PathBypass *bypass=&paths->bypass; fputs(",\"path_nodes\":[",out); for (i=0;i<analysis->node_count;++i) { if (i) fputc(',',out); fprintf(out,"{\"index\":%d,\"reachable\":%s,\"dominated_other_nodes\":",i,paths->reached[i] ? "true":paths->complete ? "false":"null"); if (paths->complete) fprintf(out,"%d",paths->dominated[i]); else fputs("null",out); fputs(",\"dominates_target\":",out); if (paths->complete && request->target>=0 && paths->selection.node==request->target) fputs(paths->selection.ancestors[i] ? "true":"false",out); else fputs("null",out); fputc('}',out); } fprintf(out,"],\"support_length\":%d,\"support_suffix\":",request->target>=0 && paths->selection.node==request->target ? paths->selection.length:-1); arcs(out,paths->selection.arcs,paths->selection.retained,request->direction==GD_STRUCTURE_BACKWARD); if (request->kind==GD_STRUCTURE_BYPASS) { if (!bypass->valid) { fputs(",\"bypass\":null,\"bypass_unavailable_reason\":",out); gd_json_string(out,paths->complete ? "Native bypass requires a nonmandatory selected support-path candidate":"Native start/path analysis incomplete"); } else { fprintf(out,",\"bypass\":{\"valid\":%s,\"complete\":%s,\"found\":%s,\"candidate\":%d,\"fork\":%d,\"rejoin\":%d,\"length\":%d,\"suffix\":",bypass->valid ? "true":"false",bypass->complete ? "true":"false",bypass->found ? "true":"false",bypass->candidate,bypass->fork,bypass->rejoin,bypass->length); arcs(out,bypass->arcs,bypass->retained,request->direction==GD_STRUCTURE_BACKWARD); fputc('}',out); } } }
        if (request->kind==GD_STRUCTURE_SHAPES) { fputs(",\"families\":[",out); for (i=0;i<analysis->node_count;++i) { if (i) fputc(',',out); if (analysis->shapes.valid && analysis->shapes.complete && analysis->shown[i]) fprintf(out,"%d",analysis->shapes.colors[i]); else fputs("null",out); } fputc(']',out); }
        if (request->kind==GD_STRUCTURE_ADJACENCY) { const GD_GraphOverview *overview=&analysis->overview; fprintf(out,",\"bins\":%d,\"input_link_rows\":%d,\"cells\":[",overview->bins,overview->link_count); for (i=0;i<overview->bins*overview->bins;++i) { if (i) fputc(',',out); fprintf(out,"%d",overview->counts[i]); } fputc(']',out); }
        fputc('}',out);
    }
    if (field(fields,"declarations")) {
        fputs(",\"declarations\":[",out);
        for (i=0;i<model->edge_count;++i) { const GD_Edge *edge=&model->edges[i]; if (i) fputc(',',out); fprintf(out,"{\"index\":%d,\"source_record\":%d,\"target_record\":%d,\"line\":%d,\"field\":",i,edge->source,edge->target,edge->line); gd_json_string(out,gd_field_name(edge->field)); member(out,"reference",edge->reference); member(out,"identity_resolution",gd_declaration_identity_name(identity_resolution(model,edge))); fputc('}',out); } fputc(']',out);
        if (view==GD_EXPLORER_RESOURCES || result->inventory || result->actor_scene) { fputs(",\"case_context\":[",out); for (i=0;i<project->rules->case_count;++i) { if (i) fputc(',',out); gd_write_rule_case(out,model,project->rules,i); } fputc(']',out); }
    }
    if (view==GD_EXPLORER_RESOURCES) { fputs(",\"native_resource_coverage\":",out); gd_write_resource_coverage(out,project->resources); }
    if (field(fields,"findings")) { if (result->gaps) gap_inventory(out,result); fputs(",\"findings\":",out); findings(out,model->findings,model->finding_count); if (view==GD_EXPLORER_MDA) { fputs(",\"mda_findings\":",out); findings(out,project->mda->findings,project->mda->finding_count); } if (view==GD_EXPLORER_RESOURCES || result->inventory || result->actor_scene) { fputs(",\"rule_findings\":",out); findings(out,project->rules->findings,project->rules->finding_count); } }
    if (result->inventory) {
        const GD_ActorInventory *inventory=result->inventory;
        fprintf(out,",\"actor_inventory_coverage\":{\"scope\":\"inventory-wide declarations; no finite hypotheses or applicability checks\",\"nodes\":%d,\"links\":%d,\"construction_limited\":%s,\"source_incomplete\":%s,\"construction_work\":%d,\"construction_limit\":%d,\"unexamined_records\":%d,\"unexamined_sources\":%d,\"unexamined_cases\":%d,\"unexamined_participants\":%d,\"unexamined_edges\":%d,\"unexamined_calls\":%d,\"unresolved_nouns\":%d,\"unresolved_axes\":%d,\"unresolved_calls\":%d,\"incomplete_sources\":%d,\"incomplete_call_owners\":%d}",inventory->node_count,inventory->link_count,inventory->limited ? "true":"false",inventory->source_incomplete ? "true":"false",inventory->work,inventory->work_limit,inventory->unexamined_records,inventory->unexamined_sources,inventory->unexamined_cases,inventory->unexamined_participants,inventory->unexamined_edges,inventory->unexamined_calls,inventory->unresolved_nouns,inventory->unresolved_axes,inventory->unresolved_calls,inventory->incomplete_sources,inventory->incomplete_call_owners);
    }
    if (result->actor_scene) {
        const GD_QueryScene *scene=result->actor_scene;
        fprintf(out,",\"actor_topology_coverage\":{\"scope\":\"selected finite actor query; all retained variants and excluded checks, independent of drawing\",\"construction_limited\":%s,\"construction_work\":%d,\"construction_limit\":%d,\"unexamined_items_upper_bound\":%d,\"label_truncations\":%d},\"actor_query_context\":",scene->limited ? "true":"false",scene->work,scene->work_limit,scene->unexamined_items_upper_bound,scene->label_truncations);
        if (!gd_write_query(out,model,options->actor_query,1,NULL)) return 0;
    }
    fprintf(out,"},\"coverage\":{\"complete\":%s,\"query_complete\":%s,\"available\":%s,\"identities_complete\":%s,\"native_selector_work\":%d,\"native_search_work\":%d,\"native_search_limit\":%d,\"fixed_construction_rows\":%d,\"fixed_selection_rows\":%d,\"source_incomplete\":%s,\"scope\":\"explicit loaded projection/filter; status matches represented source records, not ancestor applicability; never camera\",\"final_outcomes_proven\":false},\"errors\":[],\"breadcrumbs\":[\"Parallel rows differ from distinct partners; raw declaration findings retain unresolved scope. Marked nodes are a union of support and mandatory ancestors, not the dominator set.\",\"Paths are structural relationships, not AND necessity, teaching, flow, gameplay frequency or final outcomes.\",\"Support paths retain at most 64 trailing arcs in target-to-root order; reverse_traversal refers to original links, never execution.\"]}\n",query->complete && ids_complete && !model->findings_omitted && !source_incomplete ? "true":"false",query->complete ? "true":"false",query->available ? "true":"false",ids_complete ? "true":"false",selector_work,query->work,request->work_limit,query->construction_rows,query->selection_rows,source_incomplete ? "true":"false");
    return !ferror(out);
}
static int bounded_text(const char *text, int maximum) { int i=0; if (!text || !*text) return 0; while (i<=maximum && text[i]) ++i; return i<=maximum; }
static int options_valid(GD_Model *diagnostic, const GD_StructureOptions *options) {
    int kind=options ? gd_structure_kind(options->question):-1,i; const char *selectors[4];
    if (!options || gd_projection_find(options->projection)<0) gd_error(diagnostic,"query.projection_unavailable","Structural operation requires supported typed projection; inspect query --list",NULL,0);
    else if (options->root_count<0 || options->root_count>GD_STRUCTURE_SELECTORS || options->relation_count<0 || options->relation_count>GD_STRUCTURE_KINDS || options->status_count<0 || options->status_count>GD_STRUCTURE_SELECTORS || options->work_limit<0 || options->work_limit>GD_GRAPH_ANALYSIS_WORK || kind<0 || kind>=GD_STRUCTURE_COUNT || (options->avoid && kind!=GD_STRUCTURE_BYPASS)) gd_error(diagnostic,"query.arguments","Explicit bindings, supported fields and bounded operation-specific structural parameters required",NULL,0);
    if (!diagnostic->error_count && options->actor_query && strcmp(options->projection,"actors")) gd_error(diagnostic,"query.actor_scope","Finite actor query belongs only to actors projection",NULL,0);
    if (!diagnostic->error_count && kind==GD_STRUCTURE_DECLARATIONS && options->actor_query) gd_error(diagnostic,"query.actor_scope","Declaration gaps inspect whole reader context, not a finite actor query",NULL,0);
    if (!diagnostic->error_count && kind==GD_STRUCTURE_DECLARATIONS && (options->from || options->to || options->avoid || options->scope_root || options->root_count || options->relation_count || options->status_count || options->include_inactive || options->direction!=GD_STRUCTURE_FORWARD || options->lens!=GD_GRAPH_ANY)) gd_error(diagnostic,"query.arguments","Declaration gaps inspect whole loaded reader context; graph selectors/status/relation/scope filters are unavailable",NULL,0);
    if (diagnostic->error_count) return 0;
    selectors[0]=options->from; selectors[1]=options->to; selectors[2]=options->avoid; selectors[3]=options->scope_root;
    for (i=0;i<4;++i) if (selectors[i] && !bounded_text(selectors[i],GD_MAX_RULE_LINE)) gd_error(diagnostic,"query.arguments","Structural selectors require bounded nonempty identities",NULL,0);
    for (i=0;i<options->root_count;++i) if (!bounded_text(options->roots[i],GD_MAX_RULE_LINE)) gd_error(diagnostic,"query.arguments","Starts require bounded nonempty identities",NULL,0);
    for (i=0;i<options->relation_count;++i) if (!bounded_text(options->relations[i],128)) gd_error(diagnostic,"query.relation","Relations require bounded native names",NULL,0);
    for (i=0;i<options->status_count;++i) if (!bounded_text(options->statuses[i],128) || (options->status_count>1 && !strcmp(options->statuses[i],"all"))) gd_error(diagnostic,"query.status","Bounded status values required; all must stand alone",NULL,0);
    return !diagnostic->error_count;
}
void gd_structure_result_destroy(GD_StructureResult *result) {
    if (!result) return;
    if (result->gaps) result->allocator.release(result->allocator.context,result->gaps);
    gd_structure_query_destroy(result->query); if (result->layout && !result->actor_scene) result->allocator.release(result->allocator.context,result->layout);
    gd_query_scene_destroy(result->actor_scene); gd_actor_inventory_destroy(result->inventory);
    result->allocator.release(result->allocator.context,result);
}
GD_StructureResult *gd_structure_result_build(GD_Model *diagnostic, const GD_ExplorerProject *project, const GD_StructureOptions *options) {
    GD_StructureResult *result; GD_Allocator allocator; GD_Layout *layout=NULL; GD_ResourceRoutes *resolver=NULL; GD_StructureQuery *query=NULL;
    GD_StructureRequest request={0}; GD_Focus focus; unsigned char shown[GD_MAX_RECORDS],eligible[GD_MAX_PAIRS]; int roles[GD_MAX_RECORDS]; int i,j,view,selector_work=0;
    if (!diagnostic || !options_valid(diagnostic,options)) return NULL;
    view=gd_projection_find(options->projection);
    if (!project || !(project->available&(1u<<view)) || !gd_explorer_model(project,(GD_ExplorerView)view)) { gd_error(diagnostic,"query.projection_unavailable","Required explicit bindings for projection are unavailable",NULL,0); return NULL; }
    allocator=project->allocator; result=allocator.allocate(allocator.context,sizeof(*result));
    if (!result) { gd_error(diagnostic,"query.memory","Cannot allocate shared structural result",NULL,0); return NULL; }
    memset(result,0,sizeof(*result)); result->allocator=allocator; result->project=project; result->view=(GD_ExplorerView)view; result->options=*options;
    request.kind=(GD_StructureKind)gd_structure_kind(options->question); request.source=request.target=request.avoid=-1;
    if (!diagnostic->error_count) {
        GD_Model *model=gd_explorer_model(project,(GD_ExplorerView)view); const char *selectors[3]={options->from,options->to,options->avoid}; int *destinations[3]={&request.source,&request.target,&request.avoid};
        if (view==GD_EXPLORER_ACTORS && request.kind!=GD_STRUCTURE_DECLARATIONS && !strcmp(options->projection,"actors")) {
            const GD_Query *source=options->actor_query;
            if (!source || !source->variants || source->model!=model || source->rules!=project->rules || source->analysis!=project->calls) gd_error(diagnostic,"query.actor_scope","Apply a finite actor-states query on this snapshot first; coverage/inventory is not a state-specific topology",NULL,0);
            else { result->actor_scene=gd_query_scene_actor_topology(allocator,source,GD_ACTOR_WORK); if (!result->actor_scene) gd_error(diagnostic,"query.memory","Cannot allocate canonical finite actor topology",NULL,0); else layout=&result->actor_scene->layout; }
        } else layout=allocator.allocate(allocator.context,sizeof(*layout));
        result->layout=layout; if (view==GD_EXPLORER_RESOURCES) resolver=gd_resource_routes_create(allocator);
        if (!diagnostic->error_count && (!layout || (view==GD_EXPLORER_RESOURCES && !resolver))) gd_error(diagnostic,"query.memory","Cannot allocate native projection/selectors",NULL,0);
        if (!diagnostic->error_count && options->scope_root) { int root=gd_explorer_record(model,options->scope_root); if (view!=GD_EXPLORER_RELATIONSHIPS || root<0) gd_error(diagnostic,"query.scope","--scope-root requires one loaded Relationships source record",options->scope_root,0); else gd_focus_reset(model,&focus,root); }
        if (!diagnostic->error_count && view==GD_EXPLORER_ACTORS && !result->actor_scene) {
            result->inventory=gd_actor_inventory_build(allocator,project->calls,GD_ACTOR_INVENTORY_WORK);
            if (!result->inventory) gd_error(diagnostic,"query.memory","Cannot allocate bounded actor declaration inventory",NULL,0);
            else if (!gd_actor_inventory_layout(result->inventory,layout)) gd_error(diagnostic,"query.projection_unavailable","Cannot construct actor inventory topology",NULL,0);
        } else if (!diagnostic->error_count && !result->actor_scene && !gd_projection_layout(project,(GD_ExplorerView)view,options->scope_root ? &focus:NULL,layout)) gd_error(diagnostic,"query.projection_unavailable","Cannot construct requested native projection",NULL,0);
        if (!diagnostic->error_count) {
            for (i=0;i<layout->node_count;++i) { int source=gd_structure_result_record(result,layout->nodes[i].key); shown[i]=(unsigned char)(source<0 ? options->status_count==0:status_matches(options,&model->records[source])); roles[i]=result->actor_scene ? (int)result->actor_scene->nodes[layout->nodes[i].key].kind:result->inventory ? (int)result->inventory->nodes[layout->nodes[i].key].kind:view==GD_EXPLORER_RESOURCES ? (int)project->resources->nodes[layout->nodes[i].key].kind:source>=0 ? (int)model->records[source].role:-1; }
            for (i=0;i<layout->edge_count;++i) { int source=gd_structure_result_edge_record(result,layout->edges[i].key); eligible[i]=(unsigned char)(source<0 || status_matches(options,&model->records[source])); if (view==GD_EXPLORER_MATRIX && options->status_count) { const GD_Cell *cell=&model->cells[layout->edges[i].key]; eligible[i]=0; for (j=0;j<cell->pair_count;++j) if (status_matches(options,&model->records[model->pairs[cell->first_pair+j].interaction]) && (options->include_inactive || gd_active_case(&model->records[model->pairs[cell->first_pair+j].interaction]))) eligible[i]=1; } }
            request.edge_mask=eligible; request.direction=options->direction; request.lens=options->lens; request.incidence=view==GD_EXPLORER_MATRIX || view==GD_EXPLORER_RESOURCES; request.include_inactive=options->include_inactive; request.work_limit=options->work_limit;
            for (i=0;i<3;++i) if (selectors[i]) { *destinations[i]=selector(result,layout,resolver,selectors[i],&selector_work); if (*destinations[i]<0) gd_error(diagnostic,selector_error(*destinations[i]),"Structural selector missing, ambiguous or limited; use stable projected identity",selectors[i],0); }
            request.root_count=options->root_count;
            for (i=0;i<options->root_count;++i) { request.roots[i]=selector(result,layout,resolver,options->roots[i],&selector_work); if (request.roots[i]<0) gd_error(diagnostic,selector_error(request.roots[i]),"Start must identify one loaded projected node",options->roots[i],0); }
            request.relation_count=options->relation_count;
            for (i=0;i<options->relation_count;++i) { int relation,found=-1; for (relation=0;relation<GD_FIELD_COUNT;++relation) if (!strcmp(options->relations[i],gd_field_name((GD_Field)relation))) found=relation; if (!strcmp(options->relations[i],"derived case pair")) found=GD_RELATION_CASE_PAIR; for (relation=GD_RELATION_RESOURCE_BASE;relation<=GD_RELATION_RESOURCE_BASE+GD_RESOURCE_INVOCATION;++relation) if (!strcmp(options->relations[i],gd_graph_relation_name(relation))) found=relation; for (relation=GD_RELATION_ACTOR_BASE;relation<=GD_RELATION_ACTOR_BASE+GD_ACTOR_MAPS_BINDING;++relation) if (!strcmp(options->relations[i],gd_graph_relation_name(relation))) found=relation; for (relation=GD_RELATION_QUERY_BASE;relation<=GD_RELATION_QUERY_BASE+GD_QUERY_EDGE_FRONTIER;++relation) if (!strcmp(options->relations[i],gd_graph_relation_name(relation))) found=relation; if (found<0 && !(options->relation_count==1 && !strcmp(options->relations[i],"all"))) gd_error(diagnostic,"query.relation","Relation must be native field or projection relation name",options->relations[i],0); request.relations[i]=found; }
            if (options->relation_count==1 && !strcmp(options->relations[0],"all")) request.relation_count=0;
            if (!options->relation_count && view==GD_EXPLORER_RELATIONSHIPS && (request.kind==GD_STRUCTURE_REACHABILITY || request.kind==GD_STRUCTURE_DOMINATORS || request.kind==GD_STRUCTURE_BYPASS)) { request.relation_count=0; for (i=0;i<GD_FIELD_COUNT;++i) if (i!=GD_NEXT) request.relations[request.relation_count++]=i; }
            if (!diagnostic->error_count) { if (!gd_structure_request_valid(layout,shown,&request)) gd_error(diagnostic,"query.arguments","Invalid structural parameters/hidden selectors; inspect required starts/direction/bounds",NULL,0); else { query=gd_structure_query_build(allocator,layout,shown,roles,&request); if (!query) gd_error(diagnostic,"query.memory","Cannot allocate bounded structural query/scope",NULL,0); } }
        }
    }
    gd_resource_routes_destroy(resolver); result->query=query; result->selector_work=selector_work;
    if (diagnostic->error_count || !query) { gd_structure_result_destroy(result); return NULL; }
    if (request.kind==GD_STRUCTURE_DECLARATIONS) {
        const GD_Model *model=gd_explorer_model(project,(GD_ExplorerView)view);
        result->gaps=allocator.allocate(allocator.context,sizeof(*result->gaps));
        if (!result->gaps) { gd_error(diagnostic,"query.memory","Cannot allocate bounded declaration inventory",NULL,0); gd_structure_result_destroy(result); return NULL; }
        memset(result->gaps,0,sizeof(*result->gaps)); result->gaps->rows=model->edge_count;
        for (i=0;i<model->edge_count;++i) { GD_DeclarationIdentity identity=identity_resolution(model,&model->edges[i]); result->gaps->identities[i]=(unsigned char)identity; ++result->gaps->counts[identity]; }
    }
    result->source_incomplete=projection_incomplete(project,(GD_ExplorerView)view) || (result->inventory && (result->inventory->source_incomplete || result->inventory->limited)) || (result->actor_scene && (result->actor_scene->limited || options->actor_query->limited || options->actor_query->variants->limited || project->rules->finding_count || project->rules->findings_omitted)); return result;
}
int gd_structure_cli(GD_Allocator allocator, const GD_Collection *collections, int count, const GD_StructureOptions *options, int json, const char *fields, const char *initial_error) {
    GD_Model *diagnostic=gd_create(allocator); GD_ExplorerProject *project=NULL; GD_StructureResult *result=NULL; int i,status=2;
    if (!diagnostic) { fputs("gd: cannot allocate structural diagnostic snapshot\n",stderr); return 2; }
    if (initial_error) gd_error(diagnostic,"cli.arguments",initial_error,NULL,0);
    else if (!options || gd_projection_find(options->projection)<0) (void)options_valid(diagnostic,options);
    else if (!count || (fields && (!json || !gd_structure_fields(fields)))) gd_error(diagnostic,"query.arguments","Explicit bindings and supported fields required",NULL,0);
    else (void)options_valid(diagnostic,options);
    if (!diagnostic->error_count) {
        project=gd_explorer_load(allocator,collections,count);
        if (!project) gd_error(diagnostic,"query.memory","Cannot prepare bounded typed snapshots",NULL,0);
        else { const GD_Error *error=gd_explorer_error(project); if (error) gd_error(diagnostic,error->code,error->message,error->path,error->line); else result=gd_structure_result_build(diagnostic,project,options); }
    }
    if (!diagnostic->error_count && result) status=gd_write_structure_result(stdout,result,json,fields) && !fflush(stdout) ? 0:2;
    else { gd_write_result(stdout,diagnostic,0,json,NULL); for (i=0;i<diagnostic->error_count;++i) fprintf(stderr,"gd: %s: %s\n",diagnostic->errors[i].code,diagnostic->errors[i].message); }
    gd_structure_result_destroy(result); gd_explorer_destroy(project); gd_destroy(diagnostic); return status;
}
