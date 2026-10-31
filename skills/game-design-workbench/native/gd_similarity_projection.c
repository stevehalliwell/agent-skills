#include "gd_similarity_projection.h"
#include <string.h>
#include <math.h>

int gd_similarity_feature_spec(const char *spec,char *id,size_t size,double *weight) {
    size_t length=0,name; const char *equal;
    if (!spec || !id || !size || !weight) return 0;
    while (length<GD_MAX_PATH && spec[length]) ++length;
    if (!length || length==GD_MAX_PATH) return 0;
    equal=strchr(spec,'='); name=equal ? (size_t)(equal-spec):length;
    if (!name || name>=128 || name>=size) return 0;
    *weight=1;
    if (equal && (!gd_control_number(equal+1,weight,0) || !isfinite(*weight) || *weight<0)) return 0;
    memcpy(id,spec,name); id[name]=0; return 1;
}
static void feature_add(GD_SimilarityFeatureInfo *values,int *count,const char *id,GD_SimilarityProperty property,GD_SimilarityKind kind,const char *unit,int relation,int reverse) {
    GD_SimilarityFeatureInfo *value=&values[(*count)++];
    memset(value,0,sizeof(*value)); snprintf(value->id,sizeof(value->id),"%s",id);
    value->feature=(GD_SimilarityFeature){value->id,unit,kind,1}; value->property=property; value->relation=relation; value->reverse=reverse;
}
int gd_similarity_feature_catalogue(const GD_StructureResult *context,GD_SimilarityFeatureInfo *values) {
    int count=0,relation,first=0,last=GD_FIELD_COUNT-1;
    if (!context || !context->project || !context->layout || !context->query || !values) return 0;
    feature_add(values,&count,"retained-partners",GD_SIMILARITY_PARTNERS,GD_SIMILARITY_NUMBER,"count",-1,0);
    feature_add(values,&count,"retained-incident-rows",GD_SIMILARITY_ROWS,GD_SIMILARITY_NUMBER,"count",-1,0);
    feature_add(values,&count,"retained-relation-kinds",GD_SIMILARITY_RELATIONS,GD_SIMILARITY_NUMBER,"count",-1,0);
    if (!context->query->request.incidence) {
        feature_add(values,&count,"retained-incoming-rows",GD_SIMILARITY_INCOMING,GD_SIMILARITY_NUMBER,"count",-1,0);
        feature_add(values,&count,"retained-outgoing-rows",GD_SIMILARITY_OUTGOING,GD_SIMILARITY_NUMBER,"count",-1,0);
    }
    if (context->view==GD_EXPLORER_SKILLS || context->view==GD_EXPLORER_RELATIONSHIPS)
        feature_add(values,&count,"source-authored-rank",GD_SIMILARITY_RANK,GD_SIMILARITY_NUMBER,NULL,-1,0);
    feature_add(values,&count,"source-status",GD_SIMILARITY_STATUS,GD_SIMILARITY_NAME,NULL,-1,0);
    feature_add(values,&count,"source-loaded-role",GD_SIMILARITY_ROLE,GD_SIMILARITY_NAME,NULL,-1,0);
    if (context->actor_scene && context->options.actor_query && context->options.actor_query->variants) {
        const GD_ActorQuery *query=context->options.actor_query->variants;
        const GD_Model *model=gd_explorer_model(context->project,context->view);
        int named=context->project->rules->axes[query->axis].type==GD_AXIS_ENUM;
        feature_add(values,&count,"selected-variant-value",GD_SIMILARITY_STATE,named ? GD_SIMILARITY_NAME:GD_SIMILARITY_NUMBER,model->records[query->axis].axis.unit,-1,0);
    }
    if (context->effects) { first=GD_RELATION_EFFECT_BASE; last=first+GD_EFFECT_CALL_FRONTIER; }
    else if (context->inventory) { first=GD_RELATION_ACTOR_BASE; last=first+GD_ACTOR_MAPS_BINDING; }
    else if (context->actor_scene) { first=GD_RELATION_QUERY_BASE; last=first+GD_QUERY_EDGE_FRONTIER; }
    else if (context->view==GD_EXPLORER_RESOURCES) { first=GD_RELATION_RESOURCE_BASE; last=first+GD_RESOURCE_INVOCATION; }
    else if (context->view==GD_EXPLORER_MATRIX) first=last=GD_RELATION_CASE_PAIR;
    else if (context->view==GD_EXPLORER_SKILLS) first=last=GD_PREREQUISITES;
    else if (context->view==GD_EXPLORER_CHALLENGES) first=last=GD_NEXT;
    for (relation=first-1;relation<=last;++relation) {
        int reverse,reach,kind=relation<first ? -1:relation;
        if (context->view==GD_EXPLORER_MDA && kind>=0 && kind!=GD_MECHANICS && kind!=GD_SYSTEMS && kind!=GD_DYNAMICS) continue;
        for (reach=0;reach<2;++reach) for (reverse=0;reverse<(context->query->request.incidence ? 1:2);++reverse) {
            char id[128];
            if (count==GD_SIMILARITY_FEATURES) return 0;
            snprintf(id,sizeof(id),"%s-%s:%s",reach ? "reached":"direct",context->query->request.incidence ? "adjacent":reverse ? "incoming":"outgoing",kind<0 ? "all-selected-kinds":gd_graph_relation_name(kind));
            feature_add(values,&count,id,reach ? GD_SIMILARITY_REACHED_SET:GD_SIMILARITY_DIRECT_SET,GD_SIMILARITY_SET,NULL,kind,reverse);
        }
    }
    return count;
}
static int charge(GD_SimilarityProjection *projection,size_t count) {
    if (count>(size_t)(projection->work_limit-projection->work)) { projection->limited=1; return 0; }
    projection->work+=(int)count; return 1;
}
static void *owned(GD_Model *diagnostic,GD_SimilarityProjection *projection,size_t size) {
    void *memory;
    if (!size) return NULL;
    if (size>GD_SIMILARITY_STORAGE-projection->owned_bytes) { projection->limited=projection->storage_limited=1; return NULL; }
    memory=projection->allocator.allocate(projection->allocator.context,size);
    if (!memory) { gd_error(diagnostic,"query.memory","Cannot allocate bounded similarity feature storage",NULL,0); return NULL; }
    projection->owned_bytes+=size;
    if (projection->owned_bytes>projection->peak_owned_bytes) projection->peak_owned_bytes=projection->owned_bytes;
    return memory;
}
static int identity(GD_SimilarityProjection *projection,int node,char *text,size_t size) {
    const GD_StructureResult *context=projection->context;
    if (context->actor_scene) {
        if (!gd_actor_scene_node_id(context->actor_scene,context->layout->nodes[node].key,text,size,&projection->work,projection->work_limit)) { projection->limited=1; return 0; }
        return 1;
    }
    if (!gd_structure_result_node_id(context,context->layout->nodes[node].key,text,size)) { projection->limited=1; return 0; }
    /* Existing opaque identity helpers retain their own fixed native bounds;
       this layer additionally charges every emitted identity byte. */
    return charge(projection,strlen(text)+1);
}
static int select_entries(GD_Model *diagnostic,GD_SimilarityProjection *projection,const GD_SimilaritySelection *selection,unsigned char *selected) {
    const GD_StructureResult *context=projection->context; int found[GD_STRUCTURE_SELECTORS]={0},i,j;
    size_t lengths[GD_STRUCTURE_SELECTORS]; char text[GD_MAX_PATH];
    memset(selected,0,GD_MAX_RECORDS);
    for (j=0;j<selection->entry_count;++j) {
        const char *id=selection->entries[j]; size_t length=0;
        if (id) while (length<GD_MAX_PATH && id[length]) ++length;
        if (!id || !length || length==GD_MAX_PATH) { gd_error(diagnostic,"query.similarity_entry","Entry must be an exact bounded native point identity",NULL,0); return 0; }
        if (!charge(projection,length+1)) return 0; lengths[j]=length;
    }
    for (i=0;i<context->layout->node_count;++i) if (context->query->analysis.shown[i]) {
        size_t length;
        if (!identity(projection,i,text,sizeof(text))) return 0; length=strlen(text);
        for (j=0;j<selection->entry_count;++j) {
            if (!charge(projection,length+lengths[j]+1)) return 0;
            if (length==lengths[j] && !strcmp(text,selection->entries[j])) { selected[i]=1; ++found[j]; }
        }
    }
    for (j=0;j<selection->entry_count;++j) if (found[j]!=1) { gd_error(diagnostic,"query.similarity_entry","Entry identity is missing, ambiguous or outside applied context",selection->entries[j],0); return 0; }
    return 1;
}
static int entries(GD_Model *diagnostic,GD_SimilarityProjection *projection,const unsigned char *selected) {
    const GD_StructureResult *context=projection->context; size_t bytes=0,position=0; int i;
    char text[GD_MAX_PATH];
    for (i=0;i<context->layout->node_count;++i) {
        if (selected && selected[i] && !context->query->analysis.shown[i]) { gd_error(diagnostic,"query.similarity_entry","Selected similarity entry is outside applied context",NULL,0); return 0; }
        if (!context->query->analysis.shown[i] || (selected && !selected[i])) continue;
        if (!identity(projection,i,text,sizeof(text))) return 0;
        projection->nodes[projection->entry_count++]=i; bytes+=strlen(text)+1;
    }
    projection->identity_storage=owned(diagnostic,projection,bytes);
    if (bytes && !projection->identity_storage) return 0;
    for (i=0;i<projection->entry_count;++i) {
        size_t size;
        if (!identity(projection,projection->nodes[i],text,sizeof(text))) return 0;
        size=strlen(text)+1; memcpy(projection->identity_storage+position,text,size);
        projection->ids[i]=projection->identity_storage+position; position+=size;
    }
    return 1;
}
static void scalar(GD_SimilarityProjection *projection,int row,int feature) {
    static const char *const roles[]={"nouns","verbs","interactions","related","skill-atoms","challenges","mechanics","systems","dynamics","aesthetics","control-vars","rules","axes","resources"};
    const GD_StructureResult *context=projection->context;
    const GD_SimilarityFeatureInfo *info=&projection->catalogue[projection->selected_features[feature]];
    int node=projection->nodes[row],key=context->layout->nodes[node].key,record=gd_structure_result_record(context,key);
    const GD_Model *model=gd_explorer_model(context->project,context->view); const GD_Record *source=record>=0 ? &model->records[record]:NULL;
    const GD_StructureNode *counts=&context->query->nodes[node];
    size_t offset=(size_t)row*projection->feature_count+feature; GD_SimilarityCell *value=&projection->cells[offset];
    value->present=1;
    switch (info->property) {
        case GD_SIMILARITY_PARTNERS:value->number=counts->partners;break;
        case GD_SIMILARITY_ROWS:value->number=counts->incident;break;
        case GD_SIMILARITY_RELATIONS:value->number=counts->kinds;break;
        case GD_SIMILARITY_INCOMING:value->number=counts->incoming;break;
        case GD_SIMILARITY_OUTGOING:value->number=counts->outgoing;break;
        case GD_SIMILARITY_RANK:if (source && source->has_rank) value->number=source->rank; else value->present=0;break;
        case GD_SIMILARITY_STATUS:value->name=source ? source->status:NULL; value->present=value->name!=NULL;break;
        case GD_SIMILARITY_ROLE:value->name=source && source->role>=GD_NOUN && source->role<=GD_RESOURCE ? roles[source->role]:NULL; value->present=value->name!=NULL;break;
        case GD_SIMILARITY_STATE: {
            const GD_ActorQuery *query=context->options.actor_query->variants; int variant=context->actor_scene->nodes[key].variant;
            if (variant<0 || variant>=query->variant_count) { value->present=0; projection->missing[offset]=GD_SIMILARITY_NO_VARIANT; }
            else if (info->feature.kind==GD_SIMILARITY_NAME) value->name=query->variants[variant].value;
            else value->present=gd_control_number(query->variants[variant].value,&value->number,context->project->rules->axes[query->axis].type==GD_AXIS_INTEGER);
            break;
        }
        default:value->present=0;break;
    }
    if (!value->present && !projection->missing[offset]) projection->missing[offset]=GD_SIMILARITY_ABSENT_FIELD;
}
static int members(GD_SimilarityProjection *projection,const GD_GraphClosure *closure,int row,int reach,int *output) {
    uint64_t bits[GD_CLOSURE_WORDS]={0}; const GD_GraphAnalysis *analysis=&projection->context->query->analysis;
    int owner=projection->nodes[row],word,count=0,words=(analysis->node_count+63)/64;
    if (reach) {
        memcpy(bits,closure->rows[owner],(size_t)words*sizeof(uint64_t));
        bits[owner/64]&=~(UINT64_C(1)<<(owner%64));
    } else {
        int arc;
        for (arc=closure->head[owner];arc>=0;arc=closure->next[arc]) {
            const GD_LayoutEdge *edge=&analysis->edges[arc/2]; int target=arc%2 ? edge->a:edge->b;
            if (!charge(projection,1)) return -1;
            bits[target/64]|=UINT64_C(1)<<(target%64);
        }
    }
    for (word=0;word<words;++word) {
        int bit;
        if (!charge(projection,1)) return -1;
        if (!bits[word]) continue;
        for (bit=0;bit<64;++bit) if ((bits[word]>>bit)&1u) {
            if (!charge(projection,1)) return -1;
            if (output) output[count]=word*64+bit;
            ++count;
        }
    }
    return count;
}
static int set_feature(GD_Model *diagnostic,GD_SimilarityProjection *projection,GD_GraphClosure *closure,int feature) {
    const GD_StructureResult *context=projection->context;
    const GD_SimilarityFeatureInfo *info=&projection->catalogue[projection->selected_features[feature]];
    int reach=info->property==GD_SIMILARITY_REACHED_SET,limit=0,i,total=0,position=0;
    const GD_GraphAnalysis *analysis=&context->query->analysis;
    if (!charge(projection,(size_t)analysis->node_count*((analysis->node_count+63)/64+1)+analysis->edge_count)) return 0;
    if (reach) { limit=projection->work_limit-projection->work; if (limit>GD_GRAPH_ANALYSIS_WORK) limit=GD_GRAPH_ANALYSIS_WORK; }
    gd_graph_closure_into(analysis,closure,NULL,NULL,info->relation,info->reverse,context->query->request.incidence,limit);
    if (!charge(projection,(size_t)closure->work)) return 0;
    if (reach && !closure->complete) { projection->limited=1; return 0; }
    for (i=0;i<projection->entry_count;++i) {
        int count=members(projection,closure,i,reach,NULL);
        if (count<0) return 0;
        projection->cells[(size_t)i*projection->feature_count+feature].count=count; total+=count;
    }
    projection->sets[feature]=owned(diagnostic,projection,(size_t)total*sizeof(int));
    if (total && !projection->sets[feature]) return 0;
    for (i=0;i<projection->entry_count;++i) {
        GD_SimilarityCell *value=&projection->cells[(size_t)i*projection->feature_count+feature];
        int *output=value->count ? projection->sets[feature]+position:NULL;
        if (members(projection,closure,i,reach,output)<0) return 0;
        value->members=output; value->present=1; position+=value->count;
    }
    return 1;
}
GD_SimilarityProjection *gd_similarity_projection_build(GD_Model *diagnostic,const GD_StructureResult *context,const GD_SimilaritySelection *selection) {
    GD_SimilarityProjection *projection; GD_GraphClosure *closure=NULL; int i,j,positive=0; size_t count;
    unsigned char chosen[GD_MAX_RECORDS]; const unsigned char *selected=selection ? selection->selected:NULL;
    if (!diagnostic || !context || !context->project || !context->query || !context->layout || !selection || !selection->features || selection->feature_count<1 || selection->feature_count>GD_SIMILARITY_FEATURES || selection->entry_count<0 || selection->entry_count>GD_STRUCTURE_SELECTORS || (selection->entry_count && (!selection->entries || selection->selected)) || selection->work_limit<0 || selection->work_limit>GD_SIMILARITY_WORK || context->query->request.direction!=GD_STRUCTURE_FORWARD || context->query->request.lens!=GD_GRAPH_ANY) {
        if (diagnostic) gd_error(diagnostic,"query.similarity_arguments","Similarity requires canonical forward context and explicit bounded advertised features",NULL,0); return NULL;
    }
    projection=context->project->allocator.allocate(context->project->allocator.context,sizeof(*projection));
    if (!projection) { gd_error(diagnostic,"query.memory","Cannot allocate similarity feature projection",NULL,0); return NULL; }
    memset(projection,0,sizeof(*projection)); projection->allocator=context->project->allocator; projection->context=context; projection->work_limit=selection->work_limit;
    projection->owned_bytes=projection->peak_owned_bytes=sizeof(*projection); projection->feature_count=selection->feature_count;
    projection->catalogue_count=gd_similarity_feature_catalogue(context,projection->catalogue);
    for (i=0;i<selection->feature_count;++i) {
        int found=-1; double weight=selection->weights ? selection->weights[i]:1; const char *id=selection->features[i];
        size_t length=0;
        if (id) while (length<sizeof(projection->catalogue[0].id) && id[length]) ++length;
        if (!charge(projection,length+1)) goto failure;
        if (id && length<sizeof(projection->catalogue[0].id)) for (j=0;j<projection->catalogue_count;++j) {
            if (!charge(projection,length+strlen(projection->catalogue[j].id)+1)) goto failure;
            if (!strcmp(id,projection->catalogue[j].id)) found=j;
        }
        if (found<0 || !isfinite(weight) || weight<0) { gd_error(diagnostic,"query.similarity_feature","Select advertised feature IDs and finite nonnegative weights",NULL,0); goto failure; }
        for (j=0;j<i;++j) if (projection->selected_features[j]==found) { gd_error(diagnostic,"query.similarity_feature","Duplicate selected similarity feature",NULL,0); goto failure; }
        projection->selected_features[i]=found; projection->features[i]=projection->catalogue[found].feature; projection->features[i].weight=weight;
        if (weight>0) positive=1;
    }
    if (!positive) { gd_error(diagnostic,"query.similarity_feature","At least one selected feature weight must be positive",NULL,0); goto failure; }
    if ((context->inventory && context->inventory->limited) || (context->effects && context->effects->limited) ||
        (context->actor_scene && (context->actor_scene->limited || context->options.actor_query->limited || context->options.actor_query->variants->limited)) ||
        ((context->view==GD_EXPLORER_RESOURCES || context->view==GD_EXPLORER_EFFECTS) && context->project->resources->limited)) {
        projection->limited=projection->source_limited=1; return projection;
    }
    if (selection->entry_count) { if (!select_entries(diagnostic,projection,selection,chosen)) goto failure; selected=chosen; }
    if (!entries(diagnostic,projection,selected)) goto failure;
    count=(size_t)projection->entry_count*projection->feature_count;
    projection->cells=owned(diagnostic,projection,count*sizeof(*projection->cells));
    if (count && !projection->cells) goto failure;
    projection->missing=owned(diagnostic,projection,count);
    if (count && !projection->missing) goto failure;
    if (count) { memset(projection->cells,0,count*sizeof(*projection->cells)); memset(projection->missing,0,count); }
    for (i=0;i<projection->feature_count;++i) {
        GD_SimilarityProperty property=projection->catalogue[projection->selected_features[i]].property;
        if (property==GD_SIMILARITY_DIRECT_SET || property==GD_SIMILARITY_REACHED_SET) {
            if (!closure) { closure=owned(diagnostic,projection,sizeof(*closure)); if (!closure) goto failure; memset(closure,0,sizeof(*closure)); }
            if (!set_feature(diagnostic,projection,closure,i)) goto failure;
        } else for (j=0;j<projection->entry_count;++j) { if (!charge(projection,1)) goto failure; scalar(projection,j,i); }
    }
    if (closure) { projection->allocator.release(projection->allocator.context,closure); closure=NULL; projection->owned_bytes-=sizeof(GD_GraphClosure); }
    projection->table_complete=1;
    if (gd_similarity_peak_owned_bytes()>GD_SIMILARITY_STORAGE-projection->owned_bytes) { projection->limited=projection->storage_limited=1; goto failure; }
    { GD_SimilarityInput input={projection->entry_count,projection->feature_count,projection->work_limit-projection->work,projection->ids,projection->features,projection->cells};
      size_t peak=projection->owned_bytes+gd_similarity_peak_owned_bytes();
      if (peak>projection->peak_owned_bytes) projection->peak_owned_bytes=peak;
      projection->map=gd_similarity_build(projection->allocator,&input);
      if (!projection->map) { gd_error(diagnostic,"query.similarity_input","Cannot allocate or validate native similarity input",NULL,0); goto failure; }
      projection->owned_bytes+=sizeof(GD_Similarity); projection->work+=projection->map->work; projection->limited=projection->map->limited;
    }
    return projection;
failure:
    if (closure) { projection->allocator.release(projection->allocator.context,closure); projection->owned_bytes-=sizeof(GD_GraphClosure); }
    if (projection->limited && !diagnostic->error_count) return projection;
    gd_similarity_projection_destroy(projection); return NULL;
}
void gd_similarity_projection_destroy(GD_SimilarityProjection *projection) {
    int i; if (!projection) return;
    gd_similarity_destroy(projection->map);
    for (i=0;i<projection->feature_count;++i) if (projection->sets[i]) projection->allocator.release(projection->allocator.context,projection->sets[i]);
    if (projection->cells) projection->allocator.release(projection->allocator.context,projection->cells);
    if (projection->missing) projection->allocator.release(projection->allocator.context,projection->missing);
    if (projection->identity_storage) projection->allocator.release(projection->allocator.context,projection->identity_storage);
    projection->allocator.release(projection->allocator.context,projection);
}
