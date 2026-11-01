#include "gd_similarity_output.h"
#include <string.h>

static int field(const char *fields,const char *name) {
    const char *start=fields; size_t length=strlen(name);
    if (!fields) return strcmp(name,"inputs") && strcmp(name,"source");
    while (*start) { const char *end=strchr(start,','); size_t count=end ? (size_t)(end-start):strlen(start); if (count==length && !strncmp(start,name,count)) return 1; if (!end) break; start=end+1; }
    return 0;
}
int gd_similarity_fields(int catalogue,const char *fields) {
    const char *start=fields;
    if (!fields || !*fields) return 0;
    while (*start) {
        const char *end=strchr(start,','); size_t size=end ? (size_t)(end-start):strlen(start);
        if (!((size==8 && !strncmp(start,"features",size)) || (size==6 && !strncmp(start,"source",size)) ||
              (!catalogue && ((size==6 && !strncmp(start,"points",size)) || (size==6 && !strncmp(start,"inputs",size)) || (size==8 && !strncmp(start,"evidence",size)))))) return 0;
        if (!end) return 1; start=end+1;
    }
    return 0;
}
static const char *kind_name(GD_SimilarityKind kind) { return kind==GD_SIMILARITY_NUMBER ? "number":kind==GD_SIMILARITY_NAME ? "named":"set"; }
static const char *missing_name(unsigned char missing) { return missing==GD_SIMILARITY_NO_VARIANT ? "no selected hypothesis context":"absent authored field"; }
static void descriptor(FILE *out,const GD_SimilarityFeatureInfo *info,double weight) {
    fputs("{\"id\":",out); gd_json_string(out,info->id); fputs(",\"type\":",out); gd_json_string(out,kind_name(info->feature.kind));
    fputs(",\"unit\":",out); gd_json_string(out,info->feature.unit); fprintf(out,",\"weight\":%.17g,\"property\":%d",weight,(int)info->property);
    if (info->property==GD_SIMILARITY_DIRECT_SET || info->property==GD_SIMILARITY_REACHED_SET) {
        fputs(",\"relation\":",out); gd_json_string(out,info->relation<0 ? "all selected context kinds":gd_graph_relation_name(info->relation));
        fprintf(out,",\"transitive\":%s,\"reverse_projection_endpoints\":%s",info->property==GD_SIMILARITY_REACHED_SET ? "true":"false",info->reverse ? "true":"false");
    }
}
static int write_features(FILE *out,const GD_StructureResult *context,int json,const char *fields,int retained);
static int write_result(FILE *out,const GD_SimilarityProjection *projection,int json,const char *fields,int first,int second,int retained);
int gd_write_similarity_features(FILE *out,const GD_StructureResult *context,int json,const char *fields) {return write_features(out,context,json,fields,0);}
int gd_write_similarity_features_retained(FILE *out,const GD_StructureResult *context,int json,const char *fields) {return write_features(out,context,json,fields,1);}
int gd_write_similarity_result(FILE *out,const GD_SimilarityProjection *projection,int json,const char *fields,int first,int second) {return write_result(out,projection,json,fields,first,second,0);}
int gd_write_similarity_result_retained(FILE *out,const GD_SimilarityProjection *projection,int json,const char *fields) {return write_result(out,projection,json,fields,-1,-1,1);}
static int source(FILE *out,const GD_StructureResult *context,int retained) {
    GD_StructureResult copy=*context;
    if(ferror(out))return 0;
    /* Bounded borrowed view, never destroyed. Avoid recursive frontend dispatch
       when a named similarity question owns this canonical structural context. */
    copy.options.question=gd_structure_name(context->query->request.kind);
    if (gd_structure_question_index(context->options.question)>=GD_STRUCTURE_COUNT) copy.options.from=copy.options.to=NULL; /* Pair inspection is not a context root. */
    fputs(",\"source_context\":",out); return retained ? gd_write_structure_retained(out,&copy,1,NULL):gd_write_structure_result(out,&copy,1,NULL);
}
static void begin(FILE *out,const GD_StructureResult *context,const char *question,int json) {
    const GD_StructureRequest *request=&context->query->request; int i;
    if (!json) fprintf(out,"%s [%s]: native typed similarity, not gameplay importance or outcome equivalence\n",question,context->options.projection);
    fputs("{\"schema_version\":1,\"ok\":true,\"data\":{\"question\":",out); gd_json_string(out,question);
    fputs(",\"projection\":",out); gd_json_string(out,context->options.projection);
    fprintf(out,",\"scope\":{\"context_nodes\":%d,\"context_links\":%d,\"incidence\":%s,\"entry_selection_erases_context\":false,\"source_incomplete\":%s,\"relation_kinds\":[",context->query->shown_nodes,context->query->shown_edges,request->incidence ? "true":"false",context->source_incomplete ? "true":"false");
    for (i=0;i<request->relation_count;++i) { if (i) fputc(',',out); gd_json_string(out,gd_graph_relation_name(request->relations[i])); }
    fputs("],\"statuses\":[",out);
    for (i=0;i<context->options.status_count;++i) { if (i) fputc(',',out); gd_json_string(out,context->options.statuses[i]); }
    fputs("],\"scope_root\":",out); gd_json_string(out,context->options.scope_root);
    fputs(",\"feature_direction\":\"canonical projection endpoints, not execution\"}",out);
    if (context->actor_scene && context->options.actor_query && context->options.actor_query->variants) {
        const GD_ActorQuery *query=context->options.actor_query->variants;
        const GD_Rules *rules=context->project->rules; const GD_Model *model=gd_explorer_model(context->project,context->view);
        fputs(",\"hypothesis_context\":{\"local_binding\":",out); gd_json_string(out,rules->participants[query->participant].name);
        fputs(",\"noun\":",out); gd_json_string(out,rules->participants[query->participant].noun);
        fputs(",\"axis\":",out); gd_json_string(out,model->records[query->axis].reference);
        fputs(",\"axis_id\":",out); gd_json_string(out,model->records[query->axis].id);
        fputs(",\"unit\":",out); gd_json_string(out,model->records[query->axis].axis.unit);
        fputs(",\"meaning\":\"selected local binding hypothesis, not inferred partner state\"}",out);
    }
}
static int write_features(FILE *out,const GD_StructureResult *context,int json,const char *fields,int retained) {
    GD_SimilarityFeatureInfo values[GD_SIMILARITY_FEATURES]; int count,i;
    if (!out || !context || !context->query || context->query->request.direction!=GD_STRUCTURE_FORWARD || context->query->request.lens!=GD_GRAPH_ANY || (fields && (!json || !gd_similarity_fields(1,fields)))) return 0;
    count=gd_similarity_feature_catalogue(context,values); if (!count) return 0;
    begin(out,context,"similarity-features",json);
    if (field(fields,"features")) {
        fputs(",\"features\":[",out);
        for (i=0;i<count;++i) { if (i) fputc(',',out); descriptor(out,&values[i],1); fputc('}',out); }
        fputc(']',out);
    }
    if (field(fields,"source") && !source(out,context,retained)) return 0;
    fputs("},\"coverage\":{\"available\":true,\"catalogue_complete\":true,\"values_checked\":false,\"fit_performed\":false},\"errors\":[],\"breadcrumbs\":[\"Select advertised feature IDs with explicit weights; unselected context nodes remain eligible set members.\",\"Retained projection counts/sets do not assert complete external declarations; inspect source_context when needed.\"]}\n",out);
    return !ferror(out);
}
static void point(FILE *out,const GD_SimilarityProjection *projection,int row) {
    if(ferror(out))return;
    const GD_Similarity *map=projection->map; const GD_StructureResult *context=projection->context;
    int feature,node=projection->nodes[row],record=gd_structure_result_record(context,context->layout->nodes[node].key),missing=0;
    const GD_Model *model=gd_explorer_model(context->project,context->view);
    fprintf(out,"{\"row\":%d,\"context_node\":%d,\"id\":",row,node); gd_json_string(out,projection->ids[row]);
    fputs(",\"linked_definition\":",out); gd_json_string(out,record>=0 ? model->records[record].reference:NULL);
    fputs(",\"source_path\":",out); gd_json_string(out,record>=0 ? model->records[record].source_path:NULL);
    fprintf(out,",\"placed\":%s,\"x\":",map && map->available && map->placed[row] ? "true":"false");
    if (map && map->available && map->placed[row]) fprintf(out,"%.17g,\"y\":%.17g",map->x[row],map->y[row]); else fputs("null,\"y\":null",out);
    fputs(",\"missing_features\":[",out);
    for (feature=0;feature<projection->feature_count && !ferror(out);++feature) {
        size_t offset=(size_t)row*projection->feature_count+feature;
        if (projection->cells[offset].present) continue;
        if (missing++) fputc(',',out);
        fputs("{\"feature\":",out); gd_json_string(out,projection->features[feature].name);
        fputs(",\"reason\":",out); gd_json_string(out,missing_name(projection->missing[offset])); fputc('}',out);
    }
    fputs("]}",out);
}
static int inputs(FILE *out,const GD_SimilarityProjection *projection) {
    const GD_StructureResult *context=projection->context; int row,feature,i;
    fputs(",\"inputs\":",out);
    if (!projection->table_complete) { fputs("null",out); return 1; }
    fputs("{\"context_nodes\":[",out);
    for (i=0;i<context->layout->node_count;++i) {
        char identity[GD_MAX_PATH]; int valid=gd_structure_result_node_id(context,context->layout->nodes[i].key,identity,sizeof(identity));
        if (i) fputc(',',out); fprintf(out,"{\"index\":%d,\"shown\":%s,\"id\":",i,context->query->analysis.shown[i] ? "true":"false"); gd_json_string(out,valid ? identity:NULL); fputc('}',out);
    }
    fputs("],\"rows\":[",out);
    for (row=0;row<projection->entry_count && !ferror(out);++row) {
        if (row) fputc(',',out); fputc('[',out);
        for (feature=0;feature<projection->feature_count && !ferror(out);++feature) {
            const GD_SimilarityCell *value=&projection->cells[(size_t)row*projection->feature_count+feature];
            if (feature) fputc(',',out);
            if (!value->present) fputs("null",out);
            else if (projection->features[feature].kind==GD_SIMILARITY_NUMBER) fprintf(out,"%.17g",value->number);
            else if (projection->features[feature].kind==GD_SIMILARITY_NAME) gd_json_string(out,value->name);
            else { int member; fputc('[',out); for (member=0;member<value->count && !ferror(out);++member) { if (member) fputc(',',out); fprintf(out,"%d",value->members[member]); } fputc(']',out); }
        }
        fputc(']',out);
    }
    fputs("]}",out); return !ferror(out);
}
static void evidence(FILE *out,const GD_SimilarityProjection *projection,int first,int second) {
    const GD_Similarity *map=projection->map; int i;
    fputs(",\"evidence\":{\"method\":",out);
    gd_json_string(out,map && map->input_complete ? map->complete_entries<=GD_SIMILARITY_LANDMARKS ? "native-classical-mds-v1":"native-landmark-mds-v1":NULL);
    fprintf(out,",\"landmark_limit\":%d,\"landmarks\":[",GD_SIMILARITY_LANDMARKS);
    if (map) for (i=0;i<map->landmark_count;++i) { if (i) fputc(',',out); gd_json_string(out,map->input.ids[map->landmarks[i]]); }
    fputs("],\"distortion\":",out);
    if (map) fprintf(out,"{\"distance_scale\":%.17g,\"normalized_squared_error\":%.17g,\"normalized_squared_distance\":%.17g,\"checked_pairs\":%d,\"complete\":%s,\"sample\":\"at most 8192 evenly spaced canonical-ID pair ordinals\",\"negative_eigenvalues\":%d,\"normalized_negative_eigenvalue_mass\":%.17g}",map->distance_scale,map->squared_error,map->squared_distance,map->distortion_pairs,map->distortion_complete ? "true":"false",map->negative_eigenvalues,map->negative_eigenvalue_mass);
    else fputs("null",out);
    fputs(",\"pair\":",out);
    if (first>=0 && second>=0) {
        int work=0,available=0; double components[GD_SIMILARITY_FEATURES],distance=0;
        if (map && projection->table_complete) available=gd_similarity_distance(map,first,second,components,&distance,&work,GD_SIMILARITY_WORK);
        fputs("{\"first\":",out); gd_json_string(out,projection->ids[first]); fputs(",\"second\":",out); gd_json_string(out,projection->ids[second]);
        fprintf(out,",\"available\":%s,\"distance\":",available ? "true":"false"); if (available) fprintf(out,"%.17g",distance); else fputs("null",out);
        fprintf(out,",\"independent_work\":%d,\"independent_work_limit\":%d,\"components\":[",work,GD_SIMILARITY_WORK);
        for (i=0;projection->table_complete && i<projection->feature_count;++i) {
            if (i) fputc(',',out); fputs("{\"feature\":",out); gd_json_string(out,projection->features[i].name);
            fprintf(out,",\"weight\":%.17g,\"unweighted_distance\":",projection->features[i].weight); if (available) fprintf(out,"%.17g",components[i]); else fputs("null",out); fputc('}',out);
        }
        fputs("]}",out);
    } else fputs("null",out);
    fputc('}',out);
}
static int write_result(FILE *out,const GD_SimilarityProjection *projection,int json,const char *fields,int first,int second,int retained) {
    const GD_Similarity *map; int feature,row;
    if(retained && (first>=0 || second>=0))return 0;
    if (!out || !projection || !projection->context || first<-1 || second<-1 || ((first<0)!=(second<0)) || first>=projection->entry_count || second>=projection->entry_count || (fields && (!json || !gd_similarity_fields(0,fields)))) return 0;
    map=projection->map; begin(out,projection->context,"similarity-map",json);
    if (field(fields,"features")) {
        fputs(",\"features\":[",out);
        for (feature=0;feature<projection->feature_count;++feature) {
            const GD_SimilarityFeatureInfo *info=&projection->catalogue[projection->selected_features[feature]];
            if (feature) fputc(',',out);
            if (!projection->features[feature].name) { fputs("null",out); continue; }
            descriptor(out,info,projection->features[feature].weight);
            if (info->feature.kind==GD_SIMILARITY_NUMBER) {
                fputs(",\"observed_minimum\":",out);
                if (map && map->input_complete && map->complete_entries) fprintf(out,"%.17g,\"observed_maximum\":%.17g",map->low[feature],map->high[feature]); else fputs("null,\"observed_maximum\":null",out);
            }
            fputc('}',out);
        }
        fputc(']',out);
    }
    if (field(fields,"points")) {
        fputs(",\"points\":",out);
        if (!projection->table_complete) fputs("null",out);
        else { fputc('[',out); for (row=0;row<projection->entry_count && !ferror(out);++row) { if (row) fputc(',',out); point(out,projection,row); } fputc(']',out); }
    }
    if (field(fields,"inputs") && !inputs(out,projection)) return 0;
    if (field(fields,"evidence")) evidence(out,projection,first,second);
    if (field(fields,"source") && !source(out,projection->context,retained)) return 0;
    fprintf(out,"},\"coverage\":{\"available\":%s,\"feature_table_complete\":%s,\"input_validated\":%s,\"limited\":%s,\"storage_limited\":%s,\"source_construction_limited\":%s,\"kernel_failure\":%d,\"construction_fit_work\":%d,\"construction_fit_work_limit\":%d,\"owned_bytes\":%llu,\"peak_owned_bytes\":%llu,\"owned_storage_limit\":%u,\"complete_entries\":",map && map->available ? "true":"false",projection->table_complete ? "true":"false",map && map->input_complete ? "true":"false",projection->limited ? "true":"false",projection->storage_limited ? "true":"false",projection->source_limited ? "true":"false",map ? (int)map->failure:0,projection->work,projection->work_limit,(unsigned long long)projection->owned_bytes,(unsigned long long)projection->peak_owned_bytes,GD_SIMILARITY_STORAGE);
    if (map && map->input_complete) fprintf(out,"%d,\"missing_entries\":%d",map->complete_entries,map->missing_entries); else fputs("null,\"missing_entries\":null",out);
    fputs("},\"errors\":[],\"breadcrumbs\":[\"Distances describe explicitly selected native features, never importance, balance, execution or whole-outcome equivalence.\",\"Missing selected values exclude entries from ranges, distances and fit; inspect unplaced points and missing_features. No values are imputed.\",\"Coordinates retain native distance units. Squared diagnostics use disclosed distance_scale; distortion_complete distinguishes exhaustive pair coverage.\",\"Use --fields inputs,source for exact typed cells and their native context; large sections are opt-in.\"]}\n",out);
    return !ferror(out);
}
