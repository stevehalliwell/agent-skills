/* Included by structural controls; native context is owned by existing state. */
static const GD_StructureResult *picker_context(GD_ExplorerContext *context) {
    GD_ExplorerStructureState *state=context->pane->structure_state; int scope=state->pending.actor_scope;
    GD_Model *model=gd_explorer_model(context->project,context->pane->view); GD_Error saved[GD_MAX_ERRORS];
    GD_StructureOptions options={0}; GD_StructureResult *fresh; int errors=model->error_count,omitted=model->errors_omitted;
    /* Status/relation filters mask analysis, not canonical rows. Only an
       explicit Relationships scope root narrows the retained layout. */
    if (state->result && state->revision==context->project->revision && !state->result->options.scope_root && !state->result->gaps && (context->pane->view!=GD_EXPLORER_ACTORS || !!state->result->actor_scene==!!scope)) return state->result;
    if (state->picker_context && state->picker_revision==context->project->revision && state->picker_scope==scope) return state->picker_context;
    options.question=gd_structure_name(GD_STRUCTURE_CONNECTIONS); options.projection=context->pane->view==GD_EXPLORER_ACTORS && scope ? "actors":gd_projection_name(context->pane->view); options.work_limit=GD_GRAPH_ANALYSIS_WORK;
    if (scope) options.actor_query=context->pane->query_state ? context->pane->query_state->query:NULL;
    memcpy(saved,model->errors,sizeof(saved)); fresh=gd_structure_result_build(model,context->project,&options);
    if (!fresh) snprintf(context->pane->query_message,sizeof(context->pane->query_message),"Selection context unavailable: %s",model->error_count>errors ? model->errors[errors].message:"cannot allocate bounded native context");
    memcpy(model->errors,saved,sizeof(saved)); model->error_count=errors; model->errors_omitted=omitted;
    if (!fresh) return NULL;
    gd_structure_result_destroy(state->picker_context); state->picker_context=fresh; state->picker_revision=context->project->revision; state->picker_scope=scope; return fresh;
}
static int picker_selected_node(const GD_ExplorerContext *context,const GD_StructureResult *result) {
    const GD_ExplorerPane *pane=context->pane; int selected=pane->selection.node,record=-1,i,key=-1;
    if (gd_explorer_similarity_active(context)) {
        const GD_ExplorerStructureState *state=pane->structure_state; const GD_SimilarityProjection *projection=state->result->similarity;
        if (state->similarity_row>=0 && state->similarity_row<projection->entry_count) {
            char selected_id[GD_MAX_PATH],candidate[GD_MAX_PATH]; int source=projection->nodes[state->similarity_row];
            if (node_id(state->result,source,selected_id,sizeof(selected_id))) for (i=0;i<result->layout->node_count;++i) if (node_id(result,i,candidate,sizeof(candidate)) && !strcmp(selected_id,candidate)) return i;
        }
        return -1;
    }
    if (pane->view==GD_EXPLORER_MDA) { for (i=0;i<result->layout->node_count;++i) if (result->layout->nodes[i].key==pane->mda.record) return i; return -1; }
    if (selected>=0 && selected<pane->layout->node_count) {
        if (pane->view==GD_EXPLORER_RESOURCES) key=pane->layout->nodes[selected].key;
        else if (pane->view==GD_EXPLORER_ACTORS || pane->view==GD_EXPLORER_EFFECTS) {
            const GD_ExplorerQueryState *query=pane->query_state; const GD_QuerySceneNode *node=query && query->scene ? &query->scene->nodes[selected]:NULL;
            if (!node) return -1;
            if (result->actor_scene) {
                for (i=0;i<result->actor_scene->layout.node_count;++i) { const GD_QuerySceneNode *candidate=&result->actor_scene->nodes[i]; if (candidate->kind==node->kind && candidate->source==node->source && candidate->variant==node->variant) return i; } return -1;
            }
            if (result->effects && query->query->effects) key=gd_effect_node_remap(query->query->effects,node->source,result->effects);
            else if (result->inventory && node->kind==GD_QUERY_NODE_OBJECT) key=result->inventory->definition_nodes[node->source];
            else record=gd_query_scene_record(query->scene,selected);
        } else if (pane->view==GD_EXPLORER_MDA) record=selected;
        else record=pane->layout->nodes[selected].key;
    } else if (pane->selection.linked>=0) record=pane->selection.linked;
    if (key>=0) for (i=0;i<result->layout->node_count;++i) if (result->layout->nodes[i].key==key) return i;
    if (record>=0) { int found=-1; for (i=0;i<result->layout->node_count;++i) if (gd_structure_result_record(result,result->layout->nodes[i].key)==record) { if (found>=0) return -1; found=i; } return found; }
    return -1;
}
static const char *picker_node_label(const GD_StructureResult *result,int node) {
    int record=gd_structure_result_record(result,result->layout->nodes[node].key); const GD_Model *model=gd_explorer_model(result->project,result->view);
    return TextFormat("%s | %s [#%d]",result->layout->nodes[node].label,record>=0 ? model->records[record].reference:"unresolved source",node);
}
static void picker_nodes(GD_ExplorerContext *context,const GD_StructureResult *result,const char *label,char *text,size_t capacity,int multiple) {
    char id[GD_MAX_PATH]; int i,selected=picker_selected_node(context,result); const char *preview=*text ? "Selection configured":"Any / not selected";
    if (!multiple && *text) for (i=0;i<result->layout->node_count;++i) if (node_id(result,i,id,sizeof(id)) && !strcmp(id,text)) { preview=picker_node_label(result,i); break; }
    if (gd_explorer_picker_begin(context,label,preview)) {
        if (gd_explorer_picker_item(context,multiple ? "Clear selection (default scope)":"Any / not selected",!*text,multiple)) *text=0;
        for (i=0;i<result->layout->node_count;++i) if (node_id(result,i,id,sizeof(id))) {
            if (!gd_explorer_picker_match(context,picker_node_label(result,i))) continue;
            igPushID_Int(i);
            if (gd_explorer_picker_item(context,picker_node_label(result,i),multiple ? gd_picker_contains(text,id):!strcmp(text,id),multiple)) {
                if (multiple) { if (!gd_picker_toggle_limit(text,capacity,id,GD_STRUCTURE_SELECTORS)) snprintf(context->pane->query_message,sizeof(context->pane->query_message),"Selection exceeds native item/text cap; previous inputs retained."); }
                else snprintf(text,capacity,"%s",id);
            }
            if (igIsItemHovered(0)) igSetTooltip("%s",id); igPopID();
        }
        gd_explorer_picker_end();
    }
    igPushID_Str(label); igBeginDisabled(selected<0);
    if (igSmallButton(multiple ? "Add clicked graph item":"Use clicked graph item") && selected>=0 && node_id(result,selected,id,sizeof(id))) {
        if (multiple) { if (!gd_picker_contains(text,id) && !gd_picker_toggle_limit(text,capacity,id,GD_STRUCTURE_SELECTORS)) snprintf(context->pane->query_message,sizeof(context->pane->query_message),"Selection exceeds native item/text cap; prior inputs retained."); } else snprintf(text,capacity,"%s",id);
    }
    igEndDisabled(); if (selected<0 && igIsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) igSetTooltip("Click a compatible graph item. Source-only/ambiguous matches require exact picker selection."); igPopID();
    if (multiple) { const char *cursor=text; int count=*cursor ? 1:0; while (*cursor) if (*cursor++=='\n') ++count; igText("%d / %d explicit items | %u / %u text bytes",count,GD_STRUCTURE_SELECTORS,(unsigned)strlen(text),(unsigned)(capacity-1)); }
}
static void picker_relations(GD_ExplorerContext *context,const GD_StructureResult *result,char *text,size_t capacity) {
    uint64_t kinds=0; int i; for (i=0;i<result->layout->edge_count;++i) { int kind=result->layout->edges[i].relation; if (kind>=0 && kind<GD_STRUCTURE_KINDS) kinds|=UINT64_C(1)<<kind; }
    if (!gd_explorer_picker_begin(context,"Relationship kinds",*text ? "Explicit selected kinds":"Default native kinds")) return;
    if (gd_explorer_picker_item(context,"Default native kinds",!*text,1)) *text=0;
    if (gd_explorer_picker_item(context,"All kinds (including continuation)",!strcmp(text,"all"),1)) snprintf(text,capacity,"all");
    for (i=0;i<GD_STRUCTURE_KINDS;++i) if (kinds&(UINT64_C(1)<<i)) { const char *name=gd_graph_relation_name(i); if (gd_explorer_picker_item(context,name,gd_picker_contains(text,name),1)) { if (!strcmp(text,"all")) *text=0; gd_picker_toggle(text,capacity,name); } }
    gd_explorer_picker_end();
}
static int picker_feature_update(char *list,size_t capacity,const char *id,double weight,int remove) {
    char rebuilt[GD_MAX_RULE_LINE+1]={0},line[GD_MAX_RULE_LINE+1],feature[128]; const char *cursor=list; size_t used=0; double old; int matched=0;
    if (!remove && (!isfinite(weight) || weight<0)) return 0;
    while (*cursor) { const char *end=strchr(cursor,'\n'); size_t length=end ? (size_t)(end-cursor):strlen(cursor); int size,match; char *start,*tail;
        if (length>=sizeof(line)) return 0; memcpy(line,cursor,length); line[length]=0;
        start=line; while (*start && isspace((unsigned char)*start)) ++start; tail=start+strlen(start); while (tail>start && isspace((unsigned char)tail[-1])) *--tail=0;
        match=gd_similarity_feature_spec(start,feature,sizeof(feature),&old) && !strcmp(feature,id); matched|=match;
        if (match && remove) { if (!end) break; cursor=end+1; continue; }
        if (match) size=snprintf(rebuilt+used,sizeof(rebuilt)-used,"%s%s=%.17g",used ? "\n":"",id,weight);
        else size=snprintf(rebuilt+used,sizeof(rebuilt)-used,"%s%.*s",used ? "\n":"",(int)length,cursor);
        if (size<0 || (size_t)size>=sizeof(rebuilt)-used) return 0; used+=(size_t)size; if (!end) break; cursor=end+1;
    }
    if (!matched || used>=capacity) return 0; memcpy(list,rebuilt,used+1); return 1;
}
static const char *picker_feature_label(const GD_SimilarityFeatureInfo *feature,int incidence) {
    static const char *const names[]={"Connected item count","Incident relation-row count","Relation-kind count","Incoming row count","Outgoing row count","Authored source rank","Source status","Loaded source role","Selected hypothetical state value"};
    if (feature->property<=GD_SIMILARITY_STATE) return names[feature->property];
    return TextFormat("%s %s via %s",feature->property==GD_SIMILARITY_DIRECT_SET ? "Direct items":"Reachable items",incidence ? "(adjacent)":feature->reverse ? "(incoming)":"(outgoing)",feature->relation<0 ? "all selected kinds":gd_graph_relation_name(feature->relation));
}
static const char *picker_question_label(int kind) {
    static const char *const names[]={"Connections and counts","Shared direct neighbours","Reachability / support paths","Mandatory support / dominators","Alternative support avoiding item","Cycles / boundaries / shapes","Adjacency overview","Declaration gaps (whole reader)","Shared reached sets","Discover comparison features","Similarity map (MDS)"};
    return kind>=0 && kind<=GD_STRUCTURE_COUNT+1 ? names[kind]:"Unsupported question";
}
static void picker_features(GD_ExplorerContext *context,const GD_StructureResult *result,char *text,size_t capacity) {
    GD_SimilarityFeatureInfo features[GD_SIMILARITY_FEATURES]; int count=gd_similarity_feature_catalogue(result,features),i;
    if (gd_explorer_picker_begin(context,"Comparison features",*text ? "Selected typed features":"Choose one or more features")) {
        for (i=0;i<count;++i) { char tokens[GD_MAX_RULE_LINE+1],id[128]; const char *specs[GD_SIMILARITY_FEATURES]; int j,items; double weight; int selected=0;
            memcpy(tokens,text,strlen(text)+1); items=lines(tokens,specs,GD_SIMILARITY_FEATURES);
            for (j=0;j<items;++j) if (gd_similarity_feature_spec(specs[j],id,sizeof(id),&weight) && !strcmp(id,features[i].id)) { selected=1; break; }
            if (gd_explorer_picker_item(context,TextFormat("%s | %s",picker_feature_label(&features[i],result->query->request.incidence),features[i].id),selected,1)) { if (selected) picker_feature_update(text,capacity,features[i].id,0,1); else gd_picker_toggle_limit(text,capacity,features[i].id,GD_SIMILARITY_FEATURES); }
        }
        gd_explorer_picker_end();
    }
    for (i=0;i<count;++i) { char tokens[GD_MAX_RULE_LINE+1],id[128]; const char *specs[GD_SIMILARITY_FEATURES]; int j,items; double weight;
        memcpy(tokens,text,strlen(text)+1); items=lines(tokens,specs,GD_SIMILARITY_FEATURES);
        for (j=0;j<items;++j) if (gd_similarity_feature_spec(specs[j],id,sizeof(id),&weight) && !strcmp(id,features[i].id)) break;
        if (j==items || items<0) continue; igPushID_Int(i); igTextWrapped("%s",picker_feature_label(&features[i],result->query->request.incidence)); igSetNextItemWidth(180);
        if (igInputDouble("Weight",&weight,0,0,"%.6g",0) && !picker_feature_update(text,capacity,features[i].id,weight,0)) snprintf(context->pane->query_message,sizeof(context->pane->query_message),"Weight must be finite and nonnegative; prior input retained.");
        igSameLine(0,8); if (igSmallButton("Remove")) picker_feature_update(text,capacity,features[i].id,0,1); igPopID();
    }
}
