/* Paged child evidence for the current noun/Actor link and Similarity entry. */
#ifndef GD_EXPLORER_VIEWER_NESTED_H
#define GD_EXPLORER_VIEWER_NESTED_H
static int viewer_link_case_kind(const GD_ExplorerContext *c) {
    return c->pane->view==GD_EXPLORER_NOUNS ? 9:10;
}
static int viewer_find_case(GD_ExplorerContext *c,const GD_ViewerRequest *request,int kind,const char *id,const char *token,int *ordinal) {
    char key[64];int slot,row=0,found=-1,total=viewer_case_slots(c,kind);
    for(slot=0;slot<total;++slot)if(viewer_evidence_match(c,request,kind,slot,NULL,NULL)){
        viewer_case_key(c->pane,kind,slot,token,key,sizeof(key));
        if(!strcmp(key,id)){if(found>=0)return -1;found=slot;*ordinal=row;}
        ++row;
    }
    return found;
}
static int viewer_reveal_case(GD_ExplorerContext *c,int kind,int slot,int ordinal,int participant,GD_ViewerWriter *w) {
    GD_ExplorerPane *p=c->pane;char value[16],message[128];
    snprintf(value,sizeof(value),"%d",ordinal/GD_EVIDENCE_PAGE_ROWS);
    if(!gd_explorer_presentation_set(c,kind==9 ? "noun_case_page":"actor_case_page",value,message,sizeof(message))){gd_viewer_error(w,"viewer.selection",message);return 0;}
    p->reveal_case=slot+1;p->reveal_case_edge=p->selection.cell;
    p->reveal_participant=participant>=0 ? participant+1:0;
    if(kind==10 && participant>=0)p->query_state->case_participant_pages[slot]=(unsigned short)((participant-c->project->rules->cases[slot].first_participant)/GD_EVIDENCE_PAGE_ROWS);
    return 1;
}
static void viewer_child_key(const char *parent,const char *token,int row,char *key,size_t capacity) {
    uint64_t hash=viewer_control_hash(UINT64_C(14695981039346656037),token,strlen(token));
    hash=viewer_control_hash(hash,parent,strlen(parent));hash=viewer_control_hash(hash,&row,sizeof(row));
    snprintf(key,capacity,"child:%016llx",(unsigned long long)hash);
}
static void viewer_child_begin(GD_ViewerWriter *w,const GD_ViewerRequest *request,const char *token,int total,int visible_page) {
    gd_viewer_write(w,"{\"ok\":true,");viewer_member(w,"form",request->form);gd_viewer_write(w,",");viewer_member(w,"selection_token",token);
    gd_viewer_write(w,",");viewer_member(w,"parent",*request->reference ? request->reference:NULL);
    gd_viewer_write(w,",\"total_rows\":%d,\"offset\":%u,\"visible_page\":",total,request->offset);
    if(visible_page<0)gd_viewer_write(w,"null");else gd_viewer_write(w,"%d",visible_page);
    gd_viewer_write(w,",\"rows\":[");
}
static void viewer_child_end(GD_ViewerWriter *w,int start,int end,int total) {
    gd_viewer_write(w,"],\"returned_rows\":%d,\"next_offset\":",end-start);
    if(end<total)gd_viewer_write(w,"%d",end);else gd_viewer_write(w,"null");
    gd_viewer_write(w,"}");
}
static const GD_SimilarityProjection *viewer_similarity(const GD_ExplorerContext *c) {
    const GD_ExplorerStructureState *s=c->pane->structure_state;
    const GD_SimilarityProjection *projection=gd_explorer_similarity_available(c) ? s->result->similarity:NULL;
    return projection && projection->table_complete && s->similarity_row>=0 && s->similarity_row<projection->entry_count ? projection:NULL;
}
static const char *viewer_similarity_feature(const GD_SimilarityProjection *p,int feature) {
    return p->catalogue[p->selected_features[feature]].id;
}
static const GD_SimilarityCell *viewer_similarity_cell(const GD_SimilarityProjection *p,int row,int feature) {
    return &p->cells[(size_t)row*p->feature_count+feature];
}
static int viewer_similarity_find(const GD_SimilarityProjection *p,const char *id) {
    int i;for(i=0;i<p->feature_count;++i)if(!strcmp(viewer_similarity_feature(p,i),id))return i;
    return -1;
}
static void viewer_case_participants(ExplorerViewer *viewer,const GD_ViewerRequest *request,GD_ExplorerContext *c,GD_ViewerWriter *w) {
    const GD_Rules *rules=c->project->rules;char token[64];int kind=viewer_link_case_kind(c),slot,ordinal,start=(int)request->offset,i,total,limit=(int)request->limit;
    const GD_RuleCase *item;
    viewer_control_token(viewer,c,GD_FORM_PRESENTATION,token,sizeof(token));
    slot=viewer_find_case(c,request,kind,request->reference,token,&ordinal);
    if(slot<0){gd_viewer_error(w,"viewer.evidence","Use --reference CASE_KEY from the current case-index or noun/Actor case page");return;}
    item=&rules->cases[slot];total=item->participant_count;
    viewer_child_begin(w,request,token,total,kind==10 ? c->pane->query_state->case_participant_pages[slot]:-1);
    /* Two full rows fit even with maximum-size escaped authored fields. */
    if(limit>2)limit=2;
    for(i=start;i<total && i-start<limit;++i){
        char key[64];const GD_RuleParticipant *binding=&rules->participants[item->first_participant+i];
        viewer_child_key(request->reference,token,i,key,sizeof(key));
        gd_viewer_write(w,i==start ? "{":",{");viewer_member(w,"key",key);
        gd_viewer_write(w,",");viewer_member(w,"local",binding->name);
        gd_viewer_write(w,",");viewer_member(w,"noun",binding->noun);
        gd_viewer_write(w,",");viewer_member(w,"label",gd_rule_noun_label(rules,binding));
        gd_viewer_write(w,",");viewer_member(w,"requires",binding->requires);
        gd_viewer_write(w,",");viewer_member(w,"skip_if",binding->skip_if);
        gd_viewer_write(w,",");viewer_member(w,"other_axes",binding->other_axes);
        gd_viewer_write(w,",");viewer_member(w,"becomes",binding->becomes);
        gd_viewer_write(w,",");viewer_member(w,"preserve",binding->preserve);
        gd_viewer_write(w,",");viewer_member(w,"causes",binding->causes);gd_viewer_write(w,"}");
    }
    viewer_child_end(w,start,i,total);
}
static void viewer_similarity_evidence(ExplorerViewer *viewer,const GD_ViewerRequest *request,GD_ExplorerContext *c,GD_ViewerWriter *w,int members) {
    const GD_SimilarityProjection *p=viewer_similarity(c);const GD_ExplorerStructureState *s=c->pane->structure_state;
    const GD_SimilarityCell *cell=NULL;char token[64];int feature=-1,start=(int)request->offset,i,total=0,limit=(int)request->limit;
    if(members){
        if(!p || (feature=viewer_similarity_find(p,request->reference))<0 || p->features[feature].kind!=GD_SIMILARITY_SET){gd_viewer_error(w,"viewer.evidence","Select a Similarity entry and use its exact set feature ID as --reference");return;}
        cell=viewer_similarity_cell(p,s->similarity_row,feature);total=cell->present ? cell->count:0;
        if(limit>12)limit=12; /* Member IDs and labels can be long and escaped. */
    }else if(p)total=p->feature_count;
    viewer_control_token(viewer,c,GD_FORM_PRESENTATION,token,sizeof(token));
    viewer_child_begin(w,request,token,total,feature>=0 ? gd_evidence_page(total,GD_EVIDENCE_PAGE_ROWS,s->similarity_member_pages[feature]).page:-1);
    for(i=start;i<total && i-start<limit;++i){
        gd_viewer_write(w,i==start ? "{":",{");
        if(members){
            char key[64],id[GD_MAX_PATH];int node=cell->members[i],native=p->context->layout->nodes[node].key;
            const GD_Model *model=gd_explorer_model(c->project,p->context->view);int record=gd_structure_result_record(p->context,native);
            viewer_child_key(request->reference,token,i,key,sizeof(key));viewer_member(w,"key",key);
            gd_viewer_write(w,",");viewer_member(w,"id",gd_structure_result_node_id(p->context,native,id,sizeof(id)) ? id:NULL);
            gd_viewer_write(w,",");viewer_member(w,"source",record>=0 ? model->records[record].reference:NULL);
            gd_viewer_write(w,",");viewer_member(w,"label",p->context->layout->nodes[node].label);
        }else{
            const GD_SimilarityFeature *definition=&p->features[i];const GD_SimilarityCell *value=viewer_similarity_cell(p,s->similarity_row,i);
            const GD_Similarity *map=p->map;
            viewer_member(w,"id",viewer_similarity_feature(p,i));gd_viewer_write(w,",");viewer_member(w,"label",definition->name);
            gd_viewer_write(w,",");viewer_member(w,"unit",definition->unit);
            gd_viewer_write(w,",\"kind\":");gd_viewer_quote(w,definition->kind==GD_SIMILARITY_SET ? "set":definition->kind==GD_SIMILARITY_NAME ? "name":"number");
            gd_viewer_write(w,",\"present\":%s,\"weight\":%.17g,\"value\":",value->present ? "true":"false",definition->weight);
            if(!value->present || definition->kind==GD_SIMILARITY_SET)gd_viewer_write(w,"null");
            else if(definition->kind==GD_SIMILARITY_NAME)gd_viewer_quote(w,value->name);else gd_viewer_write(w,"%.17g",value->number);
            if(!value->present){gd_viewer_write(w,",");viewer_member(w,"missing",p->missing[(size_t)s->similarity_row*p->feature_count+i]==GD_SIMILARITY_NO_VARIANT ? "no-selected-variant":"absent-field");}
            else if(definition->kind==GD_SIMILARITY_SET)gd_viewer_write(w,",\"member_count\":%d,\"visible_page\":%d",value->count,gd_evidence_page(value->count,GD_EVIDENCE_PAGE_ROWS,s->similarity_member_pages[i]).page);
            else if(definition->kind==GD_SIMILARITY_NUMBER && map && map->input_complete && map->complete_entries)gd_viewer_write(w,",\"observed_range\":[%.17g,%.17g]",map->low[i],map->high[i]);
        }
        gd_viewer_write(w,"}");
    }
    viewer_child_end(w,start,i,total);
}
static int viewer_child_row(const GD_ViewerRequest *request,const char *parent,int total) {
    char key[64];int i,found=-1;
    for(i=0;i<total;++i){viewer_child_key(parent,request->expect,i,key,sizeof(key));if(!strcmp(key,request->id)){if(found>=0)return -1;found=i;}}
    return found;
}
static int viewer_select_participant(GD_ExplorerContext *c,const GD_ViewerRequest *request,GD_ViewerWriter *w) {
    int ordinal,kind=viewer_link_case_kind(c),slot=viewer_find_case(c,request,kind,request->reference,request->expect,&ordinal),row;
    if(slot>=0){
        const GD_RuleCase *item=&c->project->rules->cases[slot];row=viewer_child_row(request,request->reference,item->participant_count);
        if(row>=0)return viewer_reveal_case(c,kind,slot,ordinal,item->first_participant+row,w);
    }
    gd_viewer_error(w,"viewer.selection","Use the parent case key and an exact participant key from case-participants");return 0;
}
static int viewer_select_similarity_member(GD_ExplorerContext *c,const GD_ViewerRequest *request,GD_ViewerWriter *w) {
    const GD_SimilarityProjection *p=viewer_similarity(c);GD_ExplorerStructureState *s=c->pane->structure_state;int feature,row;
    if(p && (feature=viewer_similarity_find(p,request->reference))>=0 && p->features[feature].kind==GD_SIMILARITY_SET){
        const GD_SimilarityCell *cell=viewer_similarity_cell(p,s->similarity_row,feature);
        if(cell->present && (row=viewer_child_row(request,request->reference,cell->count))>=0){
            if(!strcmp(request->form,"similarity.member-source")){
                int record=gd_structure_result_record(p->context,p->context->layout->nodes[cell->members[row]].key);
                if(!gd_explorer_source_open(c,record)){gd_viewer_error(w,"viewer.selection","No current loaded source for this Similarity member");return 0;}
            }else{s->reveal_similarity_feature=feature+1;s->reveal_similarity_member=row+1;}
            s->similarity_member_pages[feature]=row/GD_EVIDENCE_PAGE_ROWS;
            return 1;
        }
    }
    gd_viewer_error(w,"viewer.selection","Use the exact feature ID and member key from similarity-members");return 0;
}
static void viewer_chart_children(ExplorerViewer *viewer,const GD_ViewerRequest *request,GD_ExplorerContext *c,GD_ViewerWriter *w,int activations) {
    const GD_StateChartScenario *scenario=presentation_chart_scenario(c);const GD_ExplorerChartState *s=c->pane->chart_state;
    char token[64];int total=0,i,start=(int)request->offset,limit=(int)request->limit;
    if(c->pane->view!=GD_EXPLORER_STATES || *request->reference){gd_viewer_error(w,"viewer.evidence","Chart children require state-charts and use the selected hypothesis, not --reference");return;}
    if(scenario)total=activations ? scenario->activation_count:scenario->write_count;
    if(limit>8)limit=8;
    viewer_control_token(viewer,c,GD_FORM_PRESENTATION,token,sizeof(token));
    viewer_child_begin(w,request,token,total,scenario ? activations ? s->activation_page:s->proposal_page:-1);
    for(i=start;i<total && i-start<limit;++i){
        const GD_Query *query=s->query;const GD_StateChart *chart=query->state_chart;
        int index=(activations ? scenario->first_activation:scenario->first_write)+i;
        const GD_RuleActivation *a=activations ? &chart->activations[index]:NULL;
        const GD_RuleWrite *write=activations ? NULL:&chart->writes[index];
        const GD_RuleCase *origin=&query->rules->cases[a ? a->case_index:write->origin_case];
        char key[64];viewer_child_key(request->form,token,i,key,sizeof(key));gd_viewer_write(w,i==start ? "{":",{");
        viewer_member(w,"key",key);gd_viewer_write(w,",");viewer_member(w,"source",query->model->records[query->rules->sources[origin->source].record].reference);
        gd_viewer_write(w,",");viewer_member(w,"case",origin->name);
        if(a){
            gd_viewer_write(w,",");viewer_member(w,"applicability",gd_rule_truth_name(a->applicability));
            gd_viewer_write(w,",");viewer_member(w,"local_applicability",gd_rule_truth_name(a->local_applicability));
            gd_viewer_write(w,",\"path_length\":%d",a->path_count);
        }else{
            gd_viewer_write(w,",");viewer_member(w,"participant",write->actor>=0 ? query->rules->participants[write->actor].name:NULL);
            gd_viewer_write(w,",");viewer_member(w,"axis",write->axis>=0 ? query->model->records[write->axis].reference:NULL);
            gd_viewer_write(w,",");viewer_member(w,"family",gd_effect_kind_name(chart->families[index]));
            gd_viewer_write(w,",");viewer_member(w,"state",write->kind==GD_WRITE_STATE ? write->state:NULL);
            gd_viewer_write(w,",\"line\":%d,\"conditional\":%s,\"quantity\":",write->line,write->conditional ? "true":"false");
            if(write->kind==GD_WRITE_QUANTITY && write->value.known)gd_viewer_write(w,"%.17g",write->value.number);else gd_viewer_write(w,"null");
        }
        gd_viewer_write(w,"}");
    }
    viewer_child_end(w,start,i,total);
}
static int viewer_select_chart_child(GD_ExplorerContext *c,const GD_ViewerRequest *request,GD_ViewerWriter *w,int activations) {
    const GD_StateChartScenario *scenario=presentation_chart_scenario(c);GD_ExplorerChartState *s=c->pane->chart_state;int row;
    if(scenario && !*request->reference && (row=viewer_child_row(request,activations ? "chart-activations":"chart-proposals",activations ? scenario->activation_count:scenario->write_count))>=0){
        if(activations){s->activation_page=row/GD_EVIDENCE_PAGE_ROWS;s->reveal_activation=row+1;s->reveal_proposal=0;}
        else{s->proposal_page=row/GD_EVIDENCE_PAGE_ROWS;s->reveal_proposal=row+1;s->reveal_activation=0;}
        return 1;
    }
    gd_viewer_error(w,"viewer.selection","Select a hypothesis, then use its exact proposal or activation key");return 0;
}
static const GD_EffectQuery *viewer_effect_context(const GD_ExplorerContext *c) {
    const GD_ExplorerQueryState *s=c->pane->query_state;
    return c->pane->view==GD_EXPLORER_EFFECTS && s ? s->selected:NULL;
}
static void viewer_effect_paths(ExplorerViewer *viewer,const GD_ViewerRequest *request,GD_ExplorerContext *c,GD_ViewerWriter *w,int steps) {
    const GD_EffectQuery *effect=viewer_effect_context(c);const GD_Rules *rules=c->project->rules;
    char token[64],key[64];int path[GD_MAX_RULE_EXPANSION_DEPTH],total=effect ? effect->count:0,i,row=-1,start=(int)request->offset,limit=(int)request->limit;
    if(c->pane->view!=GD_EXPLORER_EFFECTS || (!steps && *request->reference)){gd_viewer_error(w,"viewer.evidence","Effect paths require the effects projection; only step pages take --reference PATH_KEY");return;}
    viewer_control_token(viewer,c,GD_FORM_PRESENTATION,token,sizeof(token));
    if(steps){
        for(i=0;i<total;++i){viewer_child_key("effect-paths",token,i,key,sizeof(key));if(!strcmp(key,request->reference)){if(row>=0){row=-1;break;}row=i;}}
        if(row<0){gd_viewer_error(w,"viewer.evidence","Use the path key from effect-paths for the current result");return;}
        total=gd_effect_contributor_path(effect,row,path,GD_MAX_RULE_EXPANSION_DEPTH);
    }
    if(limit>12)limit=12;
    viewer_child_begin(w,request,token,total,effect ? c->pane->query_state->contributor_page:-1);
    for(i=start;i<total && i-start<limit;++i){
        const GD_EffectLink *link=&effect->graph->links[effect->links[steps ? row:i]];
        const GD_EffectNode *node=&effect->graph->nodes[steps ? path[i]:link->from];
        const GD_ResourceNode *activation=&c->pane->query_state->query->resource->nodes[node->activation];
        const GD_RuleCase *item=&rules->cases[activation->case_index];
        viewer_child_key(steps ? request->reference:"effect-paths",token,i,key,sizeof(key));
        gd_viewer_write(w,i==start ? "{":",{");viewer_member(w,"key",key);
        gd_viewer_write(w,",");viewer_member(w,"source",rules->model->records[rules->sources[item->source].record].reference);
        gd_viewer_write(w,",");viewer_member(w,"case",item->name);
        gd_viewer_write(w,",\"line\":%d",item->line);
        if(!steps)gd_viewer_write(w,",\"steps\":%d",gd_effect_contributor_path(effect,i,path,GD_MAX_RULE_EXPANSION_DEPTH));
        gd_viewer_write(w,"}");
    }
    viewer_child_end(w,start,i,total);
}
static int viewer_select_effect_step(GD_ExplorerContext *c,const GD_ViewerRequest *request,GD_ViewerWriter *w) {
    const GD_EffectQuery *effect=viewer_effect_context(c);GD_ExplorerQueryState *s=c->pane->query_state;
    int i,row=-1,step,path[GD_MAX_RULE_EXPANSION_DEPTH];char key[64];
    for(i=0;effect && i<effect->count;++i){viewer_child_key("effect-paths",request->expect,i,key,sizeof(key));if(!strcmp(key,request->reference)){if(row>=0){row=-1;break;}row=i;}}
    if(row>=0 && (step=viewer_child_row(request,request->reference,gd_effect_contributor_path(effect,row,path,GD_MAX_RULE_EXPANSION_DEPTH)))>=0){
        s->contributor_page=row/GD_EVIDENCE_PAGE_ROWS;s->reveal_contributor=row+1;s->reveal_contributor_step=step+1;return 1;
    }
    gd_viewer_error(w,"viewer.selection","Use the parent path key and exact step key from effect-path-steps");return 0;
}
#endif
