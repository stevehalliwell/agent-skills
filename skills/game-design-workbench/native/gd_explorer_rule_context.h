/* Included by explorer owner after typed source selection. Discovery only;
   no evaluator, allocation, source write or selection/layout mutation. */
#ifndef GD_EXPLORER_RULE_CONTEXT_H
#define GD_EXPLORER_RULE_CONTEXT_H
typedef struct {
    int records[2],source,case_index,participant,noun_pair,matrix_cell,overview_edge,query_effect_node;
} RuleContext;
static int rule_context_record(const GD_ExplorerContext *c,int record) {
    const GD_Model *from=gd_explorer_model(c->project,c->pane->view),*to=c->project->rules->model;int mapped;
    if(!from || record<0 || record>=from->record_count)return -1;
    mapped=gd_explorer_record(to,from->records[record].reference);
    if(mapped<0 || strcmp(from->records[record].source_path,to->records[mapped].source_path) || from->records[record].is_noun!=to->records[mapped].is_noun || (!!from->records[record].id!=!!to->records[mapped].id) ||
        (from->records[record].id && strcmp(from->records[record].id,to->records[mapped].id)))return -1;
    return mapped;
}
static int rule_context_participant_case(const GD_Rules *rules,int participant) {
    int low=0,high=rules->case_count;
    while(low<high){int middle=low+(high-low)/2;const GD_RuleCase *item=&rules->cases[middle];if(item->first_participant+item->participant_count<=participant)low=middle+1;else high=middle;}
    return low<rules->case_count && participant>=rules->cases[low].first_participant ? low:-1;
}
static RuleContext rule_context_selection(const GD_ExplorerContext *c) {
    RuleContext result={{-1,-1},-1,-1,-1,-1,-1,-1,-1};const GD_ExplorerPane *p=c->pane;const GD_Rules *rules=c->project->rules;
    GD_ExplorerContext current=*c;int record=-1,node=p->selection.node;
    if(!p->prepared || !p->layout || gd_explorer_similarity_active(c))return result;
    if(p->view==GD_EXPLORER_MDA)record=rule_context_record(c,p->mda.record);
    else if(p->selection.linked>=0)record=rule_context_record(c,p->selection.linked);
    else if(p->view==GD_EXPLORER_NOUNS && node<0 && p->selection.cell>=0 && p->selection.cell<c->project->noun_grid->pair_count) {
        const GD_NounPair *pair=&c->project->noun_grid->pairs[p->selection.cell];result.noun_pair=p->selection.cell;result.records[0]=pair->a;result.records[1]=pair->b;return result;
    }else if(p->view==GD_EXPLORER_MATRIX && node<0 && p->selection.cell>=0) {
        const GD_Model *model=gd_explorer_model(c->project,p->view);
        if(p->selection.cell<model->cell_count){result.matrix_cell=p->selection.cell;result.records[0]=rule_context_record(c,model->cells[p->selection.cell].noun);}return result;
    }else if(p->view==GD_EXPLORER_RESOURCES && node>=0 && node<p->layout->node_count) {
        const GD_ResourceNode *item=&c->project->resources->nodes[p->layout->nodes[node].key];
        result.case_index=item->kind==GD_RESOURCE_CASE || item->kind==GD_RESOURCE_ROLE ? item->case_index:-1;
        result.participant=item->kind==GD_RESOURCE_ROLE ? item->participant:-1;
        record=item->kind==GD_RESOURCE_FACET ? item->axis:item->record;
    }else if(query_view(p->view) && p->query_state && p->query_state->scene) {
        const GD_QueryScene *scene=p->query_state->scene;int edge=p->selection.cell;
        if(scene->overview && edge>=0 && edge<scene->layout.edge_count) {
            if(scene->edges[edge].kind==GD_QUERY_EDGE_AXIS){const GD_Edge *item=&rules->model->edges[scene->edges[edge].source];result.records[0]=item->source;result.records[1]=item->target;}
            else result.overview_edge=edge;
            return result;
        }
        if(node>=0 && node<scene->layout.node_count) {
            const GD_QuerySceneNode *item=&scene->nodes[node];
            if(item->kind==GD_QUERY_NODE_BINDING || item->kind==GD_QUERY_NODE_PARTNER){result.participant=item->source;result.case_index=rule_context_participant_case(rules,item->source);}
            else if(scene->query->variants){
                if(item->kind==GD_QUERY_NODE_VARIANT)result.case_index=scene->query->variants->case_index;
                else if(item->kind==GD_QUERY_NODE_ACTIVATION)result.case_index=scene->query->variants->activations[item->source].case_index;
                else if(item->kind==GD_QUERY_NODE_EFFECT){result.case_index=scene->query->variants->effects[item->source].origin_case;if(result.case_index<0)result.query_effect_node=node;}
            }else if(scene->query->effects){const GD_EffectNode *effect=&scene->query->effects->nodes[item->source];if(effect->kind==GD_EFFECT_ACTIVATION)result.case_index=scene->query->resource->nodes[effect->activation].case_index;else if(effect->relation>=0)result.case_index=scene->query->resource->relations[effect->relation].source_case;else if(item->kind==GD_QUERY_NODE_EFFECT)result.query_effect_node=node;}
        }else if(edge>=0 && edge<scene->layout.edge_count) {
            const GD_QuerySceneEdge *item=&scene->edges[edge];
            if(scene->query->effects){const GD_EffectLink *link=&scene->query->effects->links[item->source];result.case_index=scene->query->resource->relations[link->relation].source_case;}
            else if(scene->query->variants){
                const GD_QuerySceneNode *target=&scene->nodes[scene->layout.edges[edge].b];
                if(item->kind==GD_QUERY_EDGE_EFFECT)result.case_index=scene->query->variants->writes[item->source].origin_case;
                else if(target->kind==GD_QUERY_NODE_ACTIVATION)result.case_index=scene->query->variants->activations[target->source].case_index;
                else if(target->kind==GD_QUERY_NODE_VARIANT)result.case_index=scene->query->variants->case_index;
            }
        }
        record=rule_context_record(c,selected_record(&current));
    }else record=rule_context_record(c,selected_record(&current));
    if(record>=0){GD_Role role=rules->model->records[record].role;if(role==GD_NOUN || role==GD_AXIS)result.records[0]=record;else result.source=record;}
    return result;
}
static int rule_context_edge_case(const GD_QueryScene *scene,int edge) {
    const GD_QuerySceneEdge *item=&scene->edges[edge];
    if(item->kind!=GD_QUERY_EDGE_EFFECT)return -1;
    if(scene->query->effects){int relation=scene->query->effects->links[item->source].relation;return relation>=0 ? scene->query->resource->relations[relation].source_case:-1;}
    return scene->query->variants ? scene->query->variants->writes[item->source].origin_case:-1;
}
static int rule_context_case_matches(const GD_ExplorerContext *c,const RuleContext *selection,int index) {
    const GD_Rules *rules=c->project->rules;const GD_ExplorerPane *p=c->pane;const GD_RuleCase *item=&rules->cases[index];
    if(selection->case_index>=0)return selection->case_index==index;
    if(selection->query_effect_node>=0){const GD_QueryScene *scene=p->query_state->scene;int edge;
        for(edge=0;edge<scene->layout.edge_count;++edge)if(scene->layout.edges[edge].b==selection->query_effect_node && rule_context_edge_case(scene,edge)==index)return 1;
        return 0;
    }
    if(selection->noun_pair>=0)return gd_noun_grid_case(c->project->noun_grid,selection->noun_pair,index);
    if(selection->overview_edge>=0)return gd_query_scene_overview_case(p->query_state->scene,selection->overview_edge,index);
    if(selection->matrix_cell>=0){const GD_Model *model=gd_explorer_model(c->project,p->view);const GD_Cell *cell=&model->cells[selection->matrix_cell];int i;
        for(i=0;i<cell->pair_count;++i)if(rule_context_record(c,model->pairs[cell->first_pair+i].interaction)==rules->sources[item->source].record)return 1;
        return 0;
    }
    return selection->source>=0 && rules->sources[item->source].record==selection->source;
}
void gd_explorer_rule_context_discover(const GD_ExplorerContext *c,GD_ExplorerRuleContext *out) {
    RuleContext selection;int i;memset(out,0,sizeof(*out));out->records[0]=out->records[1]=out->participant=-1;
    if(!gd_explorer_form_available(c,GD_FORM_RULES) || !c->pane->prepared)return;
    selection=rule_context_selection(c);memcpy(out->records,selection.records,sizeof(out->records));out->participant=selection.participant;
    if(selection.query_effect_node>=0){const GD_QueryScene *scene=c->pane->query_state->scene;
        for(i=0;i<scene->layout.edge_count;++i)if(scene->layout.edges[i].b==selection.query_effect_node){int index=rule_context_edge_case(scene,i);if(index>=0 && index<c->project->rules->case_count)out->cases[index]=1;}
    }else if(selection.matrix_cell>=0){unsigned char sources[GD_MAX_RECORDS]={0};const GD_Model *model=gd_explorer_model(c->project,c->pane->view);const GD_Cell *cell=&model->cells[selection.matrix_cell];
        for(i=0;i<cell->pair_count;++i){int record=rule_context_record(c,model->pairs[cell->first_pair+i].interaction);if(record>=0)sources[record]=1;}
        for(i=0;i<c->project->rules->case_count;++i)out->cases[i]=sources[c->project->rules->sources[c->project->rules->cases[i].source].record];
    }else for(i=0;i<c->project->rules->case_count;++i)out->cases[i]=(unsigned char)rule_context_case_matches(c,&selection,i);
}
int gd_explorer_rule_context_slots(const GD_ExplorerContext *c,int outcomes) {
    return gd_explorer_form_available(c,GD_FORM_RULES) && c->pane->prepared && !gd_explorer_similarity_active(c) ? c->project->rules->case_count+(outcomes ? 0:2):0;
}
int gd_explorer_rule_context_option(const GD_ExplorerContext *c,int outcomes,int slot,char *key,size_t capacity,GD_ExplorerRuleForm *candidate) {
    RuleContext selection;const GD_Rules *rules;char case_key[GD_MAX_PATH]={0};const char *local=NULL;int record=-1,size,index=slot-(outcomes ? 0:2);
    if(slot<0 || slot>=gd_explorer_rule_context_slots(c,outcomes))return 0;
    rules=c->project->rules;
    if(c->rule_context){memset(&selection,0,sizeof(selection));memcpy(selection.records,c->rule_context->records,sizeof(selection.records));selection.participant=c->rule_context->participant;}
    else selection=rule_context_selection(c);
    if(!outcomes && slot<2){GD_Role role;record=selection.records[slot];
        if(record<0 || record>=rules->model->record_count || (slot==1 && record==selection.records[0]))return 0;
        role=rules->model->records[record].role;if(role!=GD_NOUN && role!=GD_AXIS)return 0;
        size=snprintf(key,capacity,"%s:%s",role==GD_NOUN ? "noun":"axis",rules->model->records[record].reference);
    }else {
        const GD_RuleCase *item=&rules->cases[index];int i,matches=0;
        if(!(c->rule_context ? c->rule_context->cases[index]:rule_context_case_matches(c,&selection,index)))return 0;
        size=snprintf(case_key,sizeof(case_key),"%s#%s",rules->model->records[rules->sources[item->source].record].reference,item->name);
        if(size<0 || (size_t)size>=sizeof(case_key) || gd_rule_case_find(rules->model,rules,case_key)!=index)return -1;
        if(!outcomes && selection.participant>=0){int participant=selection.participant;
            if(participant<item->first_participant || participant>=item->first_participant+item->participant_count)return 0;
            local=rules->participants[participant].name;
            for(i=item->first_participant;i<item->first_participant+item->participant_count;++i)matches+=!strcmp(local,rules->participants[i].name);
            if(matches!=1)return -1;
        }
        size=snprintf(key,capacity,"%s:%s%s%s",outcomes ? "outcome":local ? "binding":"case",case_key,local ? "/":"",local ? local:"");
    }
    if(size<0 || (size_t)size>=capacity || size>=GD_MAX_PATH)return -1;
    if(candidate){
        gd_explorer_rule_mode(candidate,outcomes ? 0:2);
        if(record>=0)snprintf(rules->model->records[record].role==GD_NOUN ? candidate->object:candidate->axis,GD_MAX_PATH,"%s",rules->model->records[record].reference);
        memcpy(candidate->case_name,case_key,strlen(case_key)+1);
        if(local)snprintf(candidate->participant,sizeof(candidate->participant),"%s",local);
        memcpy(candidate->selection_context,key,(size_t)size+1);
    }
    return 1;
}
int gd_explorer_rule_context_stage(GD_ExplorerContext *c,const char *key,char *message,size_t capacity) {
    GD_ExplorerContext current=*c;GD_ExplorerRuleContext selection;
    GD_ExplorerRuleForm candidate,defaults;char option[GD_MAX_PATH];int outcomes=c->pane->rule_state ? c->pane->rule_state->context_outcomes:0,i,found=0;
    gd_explorer_rule_context_discover(c,&selection);current.rule_context=&selection;c=&current;
    gd_explorer_form_defaults(GD_FORM_RULES,&defaults);
    const char *error="Use an exact context key from current selection; no case or local identity is inferred";
    for(i=0;i<gd_explorer_rule_context_slots(c,outcomes);++i) {
        if(gd_explorer_rule_context_option(c,outcomes,i,option,sizeof(option),NULL)!=1 || strcmp(key,option))continue;
        if(found++){error="Selected context is ambiguous";goto failed;}
        if(gd_explorer_rule_context_option(c,outcomes,i,option,sizeof(option),&candidate)!=1)goto failed;
    }
    if(!found)goto failed;
    if(c->pane->rule_state && !gd_explorer_form_equal(GD_FORM_RULES,&c->pane->rule_state->pending,&c->pane->rule_state->applied) &&
        !gd_explorer_form_equal(GD_FORM_RULES,&c->pane->rule_state->pending,&defaults) && !gd_explorer_form_equal(GD_FORM_RULES,&c->pane->rule_state->pending,&candidate)) {
        error="Pending rule inputs retained. Run or explicitly reset rule inputs before staging another context";goto failed;
    }
    if(!gd_explorer_rules_inputs(c)){error="Cannot allocate context workspace; previous state retained";goto failed;}
    c->pane->rule_state->pending=candidate;c->pane->rule_state->context_open=1;c->pane->rule_state->context_outcomes=!candidate.bindings;
    *message=0;return 1;
failed:snprintf(message,capacity,"%s",error);return 0;
}
#endif
