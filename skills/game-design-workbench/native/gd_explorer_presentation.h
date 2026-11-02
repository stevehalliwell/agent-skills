/* Included by explorer owner: shared GUI/IPC presentation operations, no transport. */
#ifndef GD_EXPLORER_PRESENTATION_H
#define GD_EXPLORER_PRESENTATION_H
#include <errno.h>
#include <stdlib.h>
#define PV_ALL ((1u<<GD_EXPLORER_VIEWS)-1)
#define PV_GRAPH ((1u<<GD_EXPLORER_ACTORS)-1)
#define PV_MDA (1u<<GD_EXPLORER_MDA)
#define PV_RESOURCE (1u<<GD_EXPLORER_RESOURCES)
#define PV_SKILL (1u<<GD_EXPLORER_SKILLS)
/* Storage descriptor also drives snapshot reads and simple writes. -1 is a
   domain operation, not an offset into a pane. */
typedef struct { GD_ExplorerControl control; unsigned views; size_t source; int integer_bool; } PresentationField;
#define PF(name,label,type,choices,keys,views,member,integer_bool) {{#name,label,choices,type,offsetof(GD_ExplorerPresentation,name),sizeof(((GD_ExplorerPresentation *)0)->name),0,keys},views,offsetof(GD_ExplorerPane,member),integer_bool}
#define VF(name,label,type,views) {{#name,label,NULL,type,offsetof(GD_ExplorerPresentation,name),sizeof(((GD_ExplorerPresentation *)0)->name),0,NULL},views,(size_t)-1,0}
static const PresentationField presentation_fields[]={
 VF(layout,"Layout",GD_CONTROL_INT,PV_SKILL|(1u<<GD_EXPLORER_MATRIX)|PV_MDA),
 PF(initial_arrangement,"Initial arrangement",GD_CONTROL_INT,"Native\0Neighborhood\0","native\0neighborhood\0",(1u<<GD_EXPLORER_MATRIX)|PV_MDA|PV_RESOURCE,initial_arrangement,0),
 PF(sort,"Sort / initial order",GD_CONTROL_INT,"Name\0Degree\0","name\0degree\0",15,sort,0),
 PF(direction,"Layout direction",GD_CONTROL_INT,"Top to bottom\0Left to right\0","vertical\0horizontal\0",3,horizontal,0),
 PF(paused,"Pause settling",GD_CONTROL_BOOL,NULL,NULL,PV_ALL,paused,0),
 PF(overlay,"Overlay inactive matrix pairs",GD_CONTROL_BOOL,NULL,NULL,1u<<GD_EXPLORER_MATRIX,overlay,0),
 VF(distance_focus,"Focus by shown-link distance",GD_CONTROL_BOOL,PV_GRAPH),
 PF(focus_auto_fit,"Auto-fit focus changes",GD_CONTROL_BOOL,NULL,NULL,PV_GRAPH,focus_auto_fit,0),
 PF(shared,"Shared neighbors",GD_CONTROL_BOOL,NULL,NULL,PV_GRAPH,shared,0),
 PF(lens,"Neighbor lens",GD_CONTROL_INT,"Any\0Same role\0Changed role\0","any\0same-role\0changed-role\0",PV_GRAPH,lens,0),
 PF(branches,"Compare shared / unique branches",GD_CONTROL_BOOL,NULL,NULL,PV_GRAPH,branches,0),
 PF(peer,"Shared-neighbor peer",GD_CONTROL_INT,NULL,NULL,PV_GRAPH,peer,0),
 PF(foundations,"Prerequisite foundations",GD_CONTROL_BOOL,NULL,NULL,PV_SKILL|(1u<<GD_EXPLORER_RELATIONSHIPS),foundations,0),
 PF(foundation_peer,"Foundation peer",GD_CONTROL_INT,NULL,NULL,PV_SKILL|(1u<<GD_EXPLORER_RELATIONSHIPS),foundation_peer,0),
 PF(paths_enabled,"Mandatory paths",GD_CONTROL_BOOL,NULL,NULL,PV_GRAPH,paths.enabled,1),
 PF(paths_declared,"Use authored challenge entries",GD_CONTROL_BOOL,NULL,NULL,1u<<GD_EXPLORER_CHALLENGES,paths.declared,1),
 PF(paths_compare,"Compare support / bypass",GD_CONTROL_BOOL,NULL,NULL,PV_GRAPH,paths.compare,1),
 PF(reach_enabled,"Typed multi-hop",GD_CONTROL_BOOL,NULL,NULL,PV_GRAPH,reach.enabled,1),
 PF(reach_reverse,"Against shown arrows",GD_CONTROL_BOOL,NULL,NULL,PV_GRAPH,reach.reverse,1),
 PF(reach_relation,"Traverse one relation kind",GD_CONTROL_INT,NULL,NULL,PV_GRAPH,reach.relation,0),
 PF(reach_peer,"Reachability peer",GD_CONTROL_INT,NULL,NULL,PV_GRAPH,reach.peer,0),
 PF(reach_target,"Reachability witness target",GD_CONTROL_INT,NULL,NULL,PV_GRAPH,reach.target,0),
 PF(shapes,"Structural shape families",GD_CONTROL_BOOL,NULL,NULL,PV_GRAPH,shapes,0),
 PF(overview_enabled,"Adjacency / incidence overview",GD_CONTROL_BOOL,NULL,NULL,PV_GRAPH,overview.enabled,1),
 PF(overview_row,"Overview row bin",GD_CONTROL_INT,NULL,NULL,PV_GRAPH,overview.row,0),
 PF(overview_column,"Overview column bin",GD_CONTROL_INT,NULL,NULL,PV_GRAPH,overview.column,0),
 VF(focus_root,"Relationship root",GD_CONTROL_INT,1u<<GD_EXPLORER_RELATIONSHIPS),
 VF(mda_root,"MDA query root",GD_CONTROL_INT,PV_MDA),
 VF(mda_direction,"MDA traversal",GD_CONTROL_INT,PV_MDA),
 PF(only_scope,"Only MDA query nodes",GD_CONTROL_BOOL,NULL,NULL,PV_MDA,mda.only_scope,0),
 PF(only_path,"Only MDA witness",GD_CONTROL_BOOL,NULL,NULL,PV_MDA,mda.only_path,0),
 PF(role0,"Mechanics / Resource cases",GD_CONTROL_BOOL,NULL,NULL,PV_MDA|PV_RESOURCE,mda.roles[0],0),
 PF(role1,"Systems / Resource facets",GD_CONTROL_BOOL,NULL,NULL,PV_MDA|PV_RESOURCE,mda.roles[1],0),
 PF(role2,"Dynamics / Resource roles",GD_CONTROL_BOOL,NULL,NULL,PV_MDA|PV_RESOURCE,mda.roles[2],0),
 PF(role3,"Aesthetics",GD_CONTROL_BOOL,NULL,NULL,PV_MDA,mda.roles[3],0),
 PF(filter,"Browse / Resource filter",GD_CONTROL_TEXT,NULL,NULL,PV_ALL,filter,0),
 PF(mda_filter,"MDA reference filter",GD_CONTROL_TEXT,NULL,NULL,PV_MDA,mda.filter,0),
 PF(only_witness,"Only Resource witness",GD_CONTROL_BOOL,NULL,NULL,PV_RESOURCE,resource_only_witness,0),
 VF(resource_witness,"Resource witness",GD_CONTROL_INT,PV_RESOURCE),
 PF(route_witness,"Challenge witness",GD_CONTROL_INT,NULL,NULL,1u<<GD_EXPLORER_CHALLENGES,witness,0),
 VF(similarity_canvas,"Separate similarity canvas",GD_CONTROL_BOOL,PV_ALL),
 VF(similarity_row,"Similarity first entry",GD_CONTROL_INT,PV_ALL),
 VF(similarity_peer,"Similarity peer entry",GD_CONTROL_INT,PV_ALL),
 VF(known,"Selected skill is hypothetically known",GD_CONTROL_BOOL,PV_SKILL),
 VF(coverage_page,"Coverage page",GD_CONTROL_INT,1u<<GD_EXPLORER_ACTORS),
 VF(structure_page,"Structure page",GD_CONTROL_INT,PV_ALL),
 VF(rules_page,"Rule inspection page",GD_CONTROL_INT,PV_RESOURCE),
 VF(rules_axis_page,"Binding declared-axis page",GD_CONTROL_INT,PV_RESOURCE),
 VF(similarity_page,"Similarity entry page",GD_CONTROL_INT,PV_ALL),
 VF(bypass_candidate,"Shown support-path bypass candidate",GD_CONTROL_INT,PV_GRAPH)
};
#undef PF
#undef VF
/* Controls are contiguous only in their public snapshot, not in this table. */
const GD_ExplorerControl *gd_explorer_presentation_fields(int *count) {
    /* Copied once into static immutable-after-init metadata on GUI owner thread. */
    static GD_ExplorerControl fields[sizeof(presentation_fields)/sizeof(*presentation_fields)]; static int initialized;
    int i; *count=(int)(sizeof(presentation_fields)/sizeof(*presentation_fields));
    if (!initialized) { for(i=0;i<*count;++i) fields[i]=presentation_fields[i].control; initialized=1; }
    return fields;
}
static const PresentationField *presentation_field(const char *name) {
    int i; for(i=0;i<(int)(sizeof(presentation_fields)/sizeof(*presentation_fields));++i) if(!strcmp(name,presentation_fields[i].control.name)) return &presentation_fields[i]; return NULL;
}
static int presentation_cache(const GD_ExplorerContext *c) {
    const GD_Layout *layout=c->pane->layout;const GD_GraphAnalysis *a=c->analysis;int i;
    if(!layout || !a || !a->initialized || a->node_count!=layout->node_count || a->edge_count!=layout->edge_count || a->lens!=c->pane->lens || a->include_inactive!=(c->pane->view==GD_EXPLORER_MATRIX ? c->pane->overlay:1))return 0;
    for(i=0;i<a->node_count;++i)if(a->keys[i]!=layout->nodes[i].key || a->shown[i]!=(unsigned char)visible((GD_ExplorerContext *)c,i))return 0;
    for(i=0;i<a->edge_count;++i)if(a->edges[i].a!=layout->edges[i].a || a->edges[i].b!=layout->edges[i].b || a->edges[i].key!=layout->edges[i].key || a->edges[i].relation!=layout->edges[i].relation)return 0;
    return 1;
}
void gd_explorer_presentation_read(const GD_ExplorerContext *c,GD_ExplorerPresentation *out) {
    const GD_ExplorerPane *p=c->pane; int i,record; memset(out,0,sizeof(*out));
    for(i=0;i<(int)(sizeof(presentation_fields)/sizeof(*presentation_fields));++i) {
        const PresentationField *field=&presentation_fields[i]; char *value=(char *)out+field->control.offset; const char *source;
        if(field->source==(size_t)-1) continue; source=(const char *)p+field->source;
        if(field->integer_bool) *(bool *)value=*(const int *)source!=0; else memcpy(value,source,field->control.capacity);
    }
    out->layout=p->view==GD_EXPLORER_SKILLS ? p->rank:p->layout_choice;
    out->distance_focus=p->distance_focus_enabled; out->focus_root=p->focus.root;
    out->mda_root=c->project->mda ? c->project->mda->root:-1; out->mda_direction=c->project->mda ? c->project->mda->direction:0;
    out->resource_witness=p->resource_witness; out->similarity_row=out->similarity_peer=-1;
    if(p->structure_state) { out->similarity_canvas=p->structure_state->similarity_canvas; out->similarity_row=p->structure_state->similarity_row; out->similarity_peer=p->structure_state->similarity_peer; out->structure_page=p->structure_state->page; }
    if(p->rule_state) out->rules_page=p->rule_state->page;
    if(p->query_state) out->coverage_page=p->query_state->coverage_page;
    record=p->prepared ? selected_record((GD_ExplorerContext *)c):-1;
    out->known=record>=0 && record<GD_MAX_RECORDS && p->known[record];
    out->rules_axis_page=p->rule_state ? p->rule_state->axis_page:0;out->similarity_page=p->structure_state ? p->structure_state->similarity_page:0;
    out->bypass_candidate=presentation_cache(c) && c->analysis->paths.valid ? c->analysis->paths.selection.candidate:-1;
}
int gd_explorer_presentation_editable(const GD_ExplorerContext *c,const char *name) {
    const PresentationField *field=presentation_field(name); const GD_ExplorerPane *p=c->pane;
    if(!field || !(field->views&(1u<<p->view))) return 0;
    if(!strcmp(name,"filter") || !strcmp(name,"paused") || !strcmp(name,"focus_auto_fit")) return 1;
    if(!p->prepared) return 0;
    if(!strcmp(name,"initial_arrangement")) return p->layout && p->layout->mode==GD_LAYOUT_FORCE && !grid_view(p) && !gd_explorer_similarity_active(c);
    if(!strncmp(name,"similarity_",11)) return p->structure_state && p->structure_state->result && p->structure_state->result->similarity;
    if(!strcmp(name,"known")) { int record=selected_record((GD_ExplorerContext *)c); return record>=0 && gd_explorer_model(c->project,p->view)->records[record].role==GD_SKILL; }
    if(!strcmp(name,"resource_witness")) return p->resource_routes!=NULL;
    if(!strcmp(name,"coverage_page")) return p->query_state && p->query_state->query && p->query_state->query->state_coverage;
    if(!strcmp(name,"structure_page")) return p->structure_state!=NULL;
    if(!strcmp(name,"rules_page")) return p->rule_state!=NULL;
    if(!strcmp(name,"rules_axis_page"))return p->rule_state && p->rule_state->bindings && p->rule_state->row>=0;
    if(!strcmp(name,"bypass_candidate"))return p->paths.enabled && p->paths.compare && presentation_cache(c) && c->analysis->paths.valid && c->analysis->paths.complete && c->analysis->paths.selection.node==p->selection.node && c->analysis->paths.selection.candidate_count>0;
    if(!strcmp(name,"reach_reverse") && incidence(p->view)) return 0;
    if(grid_view(p) && (field->views==PV_GRAPH || !strcmp(name,"overlay"))) return 0;
    if(gd_explorer_similarity_active(c) && field->views==PV_GRAPH) return 0;
    return 1;
}
static int presentation_node_id(const GD_ExplorerContext *c,int node,char *key,size_t capacity) {
    const GD_ExplorerPane *p=c->pane; const char *id=NULL; int work=0;
    if(node<0) { if(!capacity) return 0; *key=0; return 1; }
    if(!p->layout || node>=p->layout->node_count) return 0;
    if(p->view==GD_EXPLORER_RESOURCES) return gd_resource_node_id(c->project->resources,p->layout->nodes[node].key,-1,key,capacity);
    if(query_view(p->view)) {
        if(p->query_state->scene->query->effects) return gd_effect_node_id(p->query_state->scene->query->effects,p->query_state->scene->nodes[node].source,key,capacity);
        return gd_actor_scene_node_id(p->query_state->scene,node,key,capacity,&work,GD_ACTOR_WORK);
    }
    id=gd_explorer_model(c->project,p->view)->records[p->layout->nodes[node].key].reference;
    if(strlen(id)>=capacity) return 0; memcpy(key,id,strlen(id)+1); return 1;
}
static int presentation_node_field(const char *name) { return !strcmp(name,"peer") || !strcmp(name,"foundation_peer") || !strcmp(name,"reach_peer") || !strcmp(name,"reach_target"); }
static const char *presentation_list(const GD_ExplorerContext *c,const GD_ExplorerControl *field) {
    if(!strcmp(field->name,"layout")) return c->pane->view==GD_EXPLORER_SKILLS ? "depth\0rank\0":c->pane->view==GD_EXPLORER_MATRIX ? "free\0columns\0grid\0":"bands\0free\0";
    if(!strcmp(field->name,"mda_direction")) return "both\0forward\0backward\0";
    return field->keys ? field->keys:field->choices;
}
static const char *presentation_item(const char *list,int index) { if(!list || index<0) return NULL; while(*list && index--) list+=strlen(list)+1; return *list ? list:NULL; }
static uint64_t presentation_hash(uint64_t hash,const void *bytes,size_t count) { const unsigned char *p=bytes; size_t i; for(i=0;i<count;++i) {hash^=p[i];hash*=UINT64_C(1099511628211);} return hash; }
/* Witness handles are exact retained-row fingerprints, scoped by control token;
   not durable native node IDs or exposed integer row selectors. Collisions reject. */
static int presentation_witness_key(const GD_ExplorerContext *c,int resource,int row,char *key,size_t capacity) {
    uint64_t hash=UINT64_C(14695981039346656037); int i;
    if(row<0) { if(!capacity) return 0; *key=0; return 1; }
    if(resource) {
        const GD_ResourceRoutes *routes=c->pane->resource_routes; const GD_ResourceWitness *w;
        if(!routes || row>=routes->witness_count) return 0; w=&routes->witnesses[row];
        hash=presentation_hash(hash,w,sizeof(*w));
    } else {
        const GD_Routes *routes=c->project->routes; const GD_RouteWitness *w;
        if(!routes || row>=routes->witness_count) return 0; w=&routes->witnesses[row];
        hash=presentation_hash(hash,&w->check,sizeof(w->check)); hash=presentation_hash(hash,&w->supported,sizeof(w->supported)); hash=presentation_hash(hash,&w->full_length,sizeof(w->full_length));
        for(i=0;i<w->length;++i) hash=presentation_hash(hash,&w->path[i],sizeof(w->path[i]));
    }
    i=snprintf(key,capacity,"witness:%016llx",(unsigned long long)hash); return i>=0 && (size_t)i<capacity;
}
int gd_explorer_presentation_options(const GD_ExplorerContext *c,const GD_ExplorerControl *field) {
    const char *name=field->name,*list=presentation_list(c,field); int count=0;
    if(field->type==GD_CONTROL_BOOL) return 2;
    if(list) { while(*list) {++count;list+=strlen(list)+1;} return count; }
    if(!strcmp(name,"bypass_candidate"))return presentation_cache(c) && c->analysis->paths.valid ? c->analysis->paths.selection.candidate_count:0;
    if(presentation_node_field(name)) return c->pane->layout ? c->pane->layout->node_count+1:1;
    if(!strcmp(name,"focus_root") || !strcmp(name,"mda_root")) return gd_explorer_model(c->project,c->pane->view)->record_count+1;
    if(!strcmp(name,"reach_relation")) return GD_STRUCTURE_KINDS+1;
    if(!strcmp(name,"similarity_row") || !strcmp(name,"similarity_peer")) return c->pane->structure_state && c->pane->structure_state->result && c->pane->structure_state->result->similarity ? c->pane->structure_state->result->similarity->entry_count+1:1;
    if(!strcmp(name,"resource_witness")) return c->pane->resource_routes ? c->pane->resource_routes->witness_count+1:1;
    if(!strcmp(name,"route_witness")) return c->project->routes ? c->project->routes->witness_count+1:1;
    return -1;
}
int gd_explorer_presentation_option(const GD_ExplorerContext *c,const GD_ExplorerControl *field,int ordinal,int *native,char *key,size_t capacity) {
    const char *name=field->name,*id=NULL,*list=presentation_list(c,field); int count=gd_explorer_presentation_options(c,field);
    if(ordinal<0 || ordinal>=count) return 0; *native=ordinal;
    if(!strcmp(name,"bypass_candidate")){*native=c->analysis->paths.selection.candidates[ordinal];return presentation_node_id(c,*native,key,capacity) ? 1:-1;}
    if(field->type==GD_CONTROL_BOOL) id=ordinal ? "true":"false";
    else if(list) id=presentation_item(list,ordinal);
    else {
        *native=ordinal-1;
        if(presentation_node_field(name)) return presentation_node_id(c,*native,key,capacity) ? 1:-1;
        if(!ordinal) id="";
        else if(!strcmp(name,"focus_root") || !strcmp(name,"mda_root")) {
            const GD_Record *record=&gd_explorer_model(c->project,c->pane->view)->records[*native];
            if(!strcmp(name,"mda_root") && !gd_mda_role(record->role)) return 0; id=record->reference;
        } else if(!strcmp(name,"reach_relation")) id=gd_graph_relation_name(*native);
        else if(!strcmp(name,"resource_witness") || !strcmp(name,"route_witness")) return presentation_witness_key(c,!strcmp(name,"resource_witness"),*native,key,capacity) ? 1:-1;
        else id=c->pane->structure_state->result->similarity->ids[*native];
    }
    if(!id) return 0; if(strlen(id)>=capacity) return -1; memcpy(key,id,strlen(id)+1); return 1;
}
int gd_explorer_presentation_value(const GD_ExplorerContext *c,const GD_ExplorerControl *field,const GD_ExplorerPresentation *values,char *out,size_t capacity) {
    const char *value=(const char *)values+field->offset; int native,ordinal,slots=gd_explorer_presentation_options(c,field);
    if(field->type==GD_CONTROL_TEXT) { if(strlen(value)>=capacity) return -1; memcpy(out,value,strlen(value)+1); return 1; }
    native=field->type==GD_CONTROL_BOOL ? *(const bool *)value:*(const int *)value;
    if(!strcmp(field->name,"bypass_candidate")){if(native<0)return 0;return presentation_node_id(c,native,out,capacity) ? 1:-1;}
    if(slots>=0) { ordinal=native+(field->type!=GD_CONTROL_BOOL && !presentation_list(c,field)); return gd_explorer_presentation_option(c,field,ordinal,&native,out,capacity); }
    ordinal=snprintf(out,capacity,"%d",native); return ordinal>=0 && (size_t)ordinal<capacity ? 1:-1;
}
static int presentation_same_topology(const GD_Layout *a,const GD_Layout *b) {
    int i;
    if(!a || a->node_count!=b->node_count || a->edge_count!=b->edge_count || a->group_count!=b->group_count || a->mode!=b->mode) return 0;
    for(i=0;i<a->node_count;++i) if(a->nodes[i].key!=b->nodes[i].key || a->nodes[i].group!=b->nodes[i].group || a->nodes[i].layer!=b->nodes[i].layer) return 0;
    return !memcmp(a->edges,b->edges,(size_t)a->edge_count*sizeof(*a->edges));
}
static int presentation_rearrange(GD_ExplorerContext *c,const GD_ExplorerPresentation *values,int geometry_only) {
    GD_ExplorerPane next=*c->pane,*p=c->pane; GD_ExplorerContext candidate=*c;
    if(query_view(p->view)) return 0;
    next.layout=NULL; next.distance_focus=NULL; next.structure_state=NULL; next.prepared=0;
    next.rank=p->view==GD_EXPLORER_SKILLS ? values->layout:p->rank;
    next.layout_choice=p->view==GD_EXPLORER_SKILLS ? p->layout_choice:values->layout;
    next.sort=values->sort; next.horizontal=values->direction;
    next.initial_arrangement=0; /* Prepare native topology once; seed after footprint remap. */
    if(is_graph(p->view)) next.selection=(GD_ExplorerSelection){-1,-1,0,selected_record(c),-1};
    candidate.pane=&next;
    if(!pane_prepare(&candidate)) { if(next.layout) c->project->allocator.release(c->project->allocator.context,next.layout); gd_layout_focus_destroy(next.distance_focus); return 0; }
    if(p->prepared && p->layout) {
        int i,node_key=p->selection.node>=0 && p->selection.node<p->layout->node_count ? p->layout->nodes[p->selection.node].key:-1;
        next.selection=p->selection;next.selection.held=-1;
        if(node_key>=0){next.selection.node=-1;for(i=0;i<next.layout->node_count;++i)if(next.layout->nodes[i].key==node_key)next.selection.node=i;}
        if(next.selection.cell>=0){int found=0;for(i=0;i<next.layout->edge_count;++i)if(next.layout->edges[i].key==next.selection.cell)found=1;if(!found)next.selection.cell=-1;}
        if(p->view==GD_EXPLORER_MATRIX){next.grid.noun=p->grid.noun;next.grid.verb=p->grid.verb;if(next.selection.cell>=0){const GD_Cell *cell=&gd_explorer_model(c->project,p->view)->cells[next.selection.cell];next.grid.noun=cell->noun;next.grid.verb=cell->verb;}}
    }
    if(geometry_only && !presentation_same_topology(p->layout,next.layout)) {
        c->project->allocator.release(c->project->allocator.context,next.layout);gd_layout_focus_destroy(next.distance_focus);return 0;
    }
    next.initial_arrangement=values->initial_arrangement;
    if(next.initial_arrangement && next.layout->mode==GD_LAYOUT_FORCE && p->layout && p->layout->mode==GD_LAYOUT_FORCE) {
        int i; for(i=0;i<next.layout->node_count && i<p->layout->node_count;++i)
            if(next.layout->nodes[i].key==p->layout->nodes[i].key) next.layout->nodes[i].footprint=p->layout->nodes[i].footprint;
    }
    if(!pane_seed(&candidate)) { c->project->allocator.release(c->project->allocator.context,next.layout); gd_layout_focus_destroy(next.distance_focus); return 0; }
    if(next.distance_focus) gd_layout_focus_capture(next.distance_focus,next.layout);
    if(p->layout) c->project->allocator.release(c->project->allocator.context,p->layout); gd_layout_focus_destroy(p->distance_focus);
    p->initial_arrangement=next.initial_arrangement;
    p->layout=next.layout; p->distance_focus=next.distance_focus; p->distance_focus_revision=UINT64_MAX;
    p->rank=next.rank; p->layout_choice=next.layout_choice; p->sort=next.sort; p->horizontal=next.horizontal; p->grid=next.grid; p->selection=next.selection; p->focus=next.focus;
    p->prepared=next.prepared; p->revision=next.revision; p->fit=grid_view(p) ? GD_EXPLORER_FIT_GRID_READABLE:1; if(!geometry_only) c->analysis->initialized=0; return 1;
}
int gd_explorer_presentation_set(GD_ExplorerContext *c,const char *name,const char *value,char *message,size_t capacity) {
    const PresentationField *entry=presentation_field(name); const GD_ExplorerControl *field=entry ? &entry->control:NULL; GD_ExplorerPresentation next; GD_ExplorerPane *p=c->pane;
    int slots,i,native=-1,found=0; char key[GD_MAX_PATH],*slot; const char *error="Presentation control unavailable; explicitly open projection first";
    if(!field || !gd_explorer_presentation_editable(c,name)) goto failed;
    gd_explorer_presentation_read(c,&next); slot=(char *)&next+field->offset; slots=gd_explorer_presentation_options(c,field);
    if(field->type==GD_CONTROL_TEXT) { error="Filter exceeds native capacity"; if(strlen(value)>=field->capacity) goto failed; memcpy(slot,value,strlen(value)+1); }
    else if(slots>=0) {
        error="Use exact discovered choice key; row/node indices are not selectors";
        for(i=0;i<slots;++i) { int candidate; if(gd_explorer_presentation_option(c,field,i,&candidate,key,sizeof(key))!=1 || strcmp(key,value)) continue; if(found && native!=candidate) {error="Choice key ambiguous in current result";goto failed;} found=1;native=candidate; }
        if(!found) goto failed; if(field->type==GD_CONTROL_BOOL) *(bool *)slot=native!=0; else *(int *)slot=native;
    } else {
        char *end; long number; error="Page/bin requires decimal integer in 0..4096";
        if(!*value || *value<'0' || *value>'9') goto failed; errno=0; number=strtol(value,&end,10); if(errno || *end || number>4096) goto failed;
        if((!strcmp(name,"overview_row") || !strcmp(name,"overview_column")) && number>=GD_OVERVIEW_BINS) goto failed;
        *(int *)slot=(int)number;
    }
    error="Cannot apply presentation candidate; last good state retained";
    if(!strcmp(name,"layout") || !strcmp(name,"sort") || !strcmp(name,"direction") || !strcmp(name,"initial_arrangement")) {
        if(!presentation_rearrange(c,&next,!strcmp(name,"initial_arrangement"))) goto failed;
        if(!strcmp(name,"initial_arrangement")) p->paused=false;
    }
    else if(!strcmp(name,"distance_focus")) { if(!distance_focus_toggle(c,next.distance_focus)) goto failed; }
    else if(!strcmp(name,"focus_root")) { if(next.focus_root<0) goto failed; { GD_Focus old=p->focus; p->focus=(GD_Focus){0}; gd_focus_reset(gd_explorer_model(c->project,p->view),&p->focus,next.focus_root); if(!presentation_rearrange(c,&next,0)) {p->focus=old;goto failed;} } }
    else if(!strcmp(name,"mda_root") || !strcmp(name,"mda_direction")) { if(!gd_mda_query(gd_explorer_model(c->project,p->view),c->project->mda,next.mda_root,(GD_MdaDirection)next.mda_direction)) goto failed; }
    else if(!strcmp(name,"resource_witness")) { p->resource_witness=next.resource_witness; gd_resource_route_mark(p->resource_routes,p->resource_witness); }
    else if(!strcmp(name,"similarity_canvas")) p->structure_state->similarity_canvas=next.similarity_canvas;
    else if(!strcmp(name,"similarity_row")) p->structure_state->similarity_row=next.similarity_row;
    else if(!strcmp(name,"similarity_peer")) p->structure_state->similarity_peer=next.similarity_peer;
    else if(!strcmp(name,"known")) p->known[selected_record(c)]=next.known;
    else if(!strcmp(name,"coverage_page")) p->query_state->coverage_page=next.coverage_page;
    else if(!strcmp(name,"structure_page")) p->structure_state->page=next.structure_page;
    else if(!strcmp(name,"rules_page")) p->rule_state->page=next.rules_page;
    else if(!strcmp(name,"rules_axis_page")) p->rule_state->axis_page=next.rules_axis_page;
    else if(!strcmp(name,"similarity_page")) p->structure_state->similarity_page=next.similarity_page;
    else if(!strcmp(name,"bypass_candidate")){c->analysis->paths.selection.candidate=next.bypass_candidate;if(!gd_graph_path_bypass(c->analysis,p->selection.node,next.bypass_candidate,GD_GRAPH_ANALYSIS_WORK))goto failed;}
    else if(entry->source!=(size_t)-1) { if(entry->integer_bool) *(int *)((char *)p+entry->source)=*(bool *)slot; else memcpy((char *)p+entry->source,slot,field->capacity); }
    else goto failed;
    *message=0; return 1;
failed: snprintf(message,capacity,"%s",error); return 0;
}
int gd_explorer_presentation_action(GD_ExplorerContext *c,const char *name) {
    GD_ExplorerPane *p=c->pane; GD_ExplorerPresentation values;
    if(!p->prepared) return 0;
    if(!strcmp(name,"view.reset")) {
        if(c->project->draft && (c->project->draft->modified_count || c->project->draft->history_count)) return 0;
        return reset_view(c);
    }
    if(!strcmp(name,"view.find.clear")) {*p->find_reference=0;matches(c);return 1;}
    if(!strcmp(name,"view.witness.fit")) {if(p->view!=GD_EXPLORER_RESOURCES || !p->resource_routes || !p->resource_routes->witness_count)return 0;p->fit=GD_EXPLORER_FIT_RESOURCE_WITNESS;return 1;}
    if(!strcmp(name,"view.cell.fit")) {if(p->view!=GD_EXPLORER_MATRIX || !grid_view(p) || p->grid.noun<0 || p->grid.verb<0)return 0;grid_frame_selection(c);return 1;}
    if(!strcmp(name,"view.fit")) { if(gd_explorer_similarity_active(c)) p->structure_state->similarity_fit=true; else p->fit=1; return 1; }
    if(!strcmp(name,"view.rearrange")) { gd_explorer_presentation_read(c,&values); return presentation_rearrange(c,&values,1); }
    if(!strcmp(name,"view.start.clear")) { p->paths.count=p->paths.lost=0; return 1; }
    if(!strcmp(name,"view.start.add")) { int node=analyze(c); return node>=0 && gd_graph_path_start(&p->paths,p->layout->nodes[node].key)>=0; }
    if(!strcmp(name,"view.expand")) { int record=selected_record(c); GD_Focus prior=p->focus; if(p->view!=GD_EXPLORER_RELATIONSHIPS || record<0) return 0; gd_focus_expand(gd_explorer_model(c->project,p->view),&p->focus,record); gd_explorer_presentation_read(c,&values); if(presentation_rearrange(c,&values,0)) return 1; p->focus=prior; return 0; }
    if(!strcmp(name,"view.analyze")) { int selected;if(query_view(p->view) || grid_view(p) || gd_explorer_similarity_active(c)) return 0;selected=analyze(c); if(c->pane->view==GD_EXPLORER_SKILLS && (!c->query || !gd_skill_unlocks(gd_explorer_model(c->project,c->pane->view),p->known,selected_record(c),c->query))) return 0; (void)evidence(c,selected); if(p->paths.enabled && selected>=0) gd_graph_path_selection(c->analysis,selected); if(p->shared && selected>=0 && p->peer>=0) gd_graph_compare(c->analysis,selected,p->peer); return 1; }
    if(!strcmp(name,"view.compare")) return !gd_explorer_similarity_active(c) && c->pane->view==GD_EXPLORER_ACTORS ? gd_explorer_query_compare(c):gd_explorer_similarity_compare(c);
    return 0;
}
#undef PV_ALL
#undef PV_GRAPH
#undef PV_MDA
#undef PV_RESOURCE
#undef PV_SKILL
#endif
