/* Current-pane tools. CPU-only, unchanged source files, no live viewer. */
#ifndef TEST_EXPLORER_RULE_CONTEXT_H
#define TEST_EXPLORER_RULE_CONTEXT_H
static int explorer_rule_context_checks(GD_ExplorerContext *c,GD_Layout *saved,int *fail_at,int *calls) {
    GD_Allocator allocator=c->project->allocator;GD_ExplorerRuleState *snapshot=NULL,*owner;GD_Rules *rules=c->project->rules;
    GD_ExplorerPane *p;const GD_Model *model=rules->model;int result=0,v,i,j,noun=-1,axis=-1,root=-1,checks=0,allocations;
    char key[GD_MAX_PATH],other[GD_MAX_PATH],message[256];uint64_t draft_hash=c->project->draft ? viewer_control_hash(1,c->project->draft,sizeof(*c->project->draft)):0;
#define CONTEXT_CHECK(value) do{++checks;if(!(value)){fprintf(stderr,"Rule context failed: line %d: %s\n",__LINE__,#value);goto cleanup;}}while(0)
    snapshot=allocator.allocate(allocator.context,sizeof(*snapshot));CONTEXT_CHECK(snapshot);
    for(i=0;i<rules->case_count && root<0;++i)for(j=rules->cases[i].first_participant;j<rules->cases[i].first_participant+rules->cases[i].participant_count;++j)
        if(rules->participants[j].noun_resolution==GD_RULE_RESOLVED){root=i;noun=rules->participants[j].noun_record;break;}
    for(i=0;i<model->record_count && axis<0;++i)if(model->records[i].role==GD_AXIS)axis=i;
    CONTEXT_CHECK(root>=0 && noun>=0 && axis>=0 && gd_explorer_reset_all(c));
    for(v=0;v<GD_EXPLORER_VIEWS;++v)if(c->project->available&(1u<<v)) {
        Camera2D camera;GD_ExplorerSelection selection;GD_ExplorerQueryOptions pending;GD_ExplorerQueryState *query;
        GD_ExplorerRuleState *states[GD_EXPLORER_VIEWS];int mapped;
        p=c->pane=&c->panes[v];*c->active_view=v;CONTEXT_CHECK(pane_prepare(c));
        mapped=gd_explorer_record(gd_explorer_model(c->project,v),model->records[noun].reference);CONTEXT_CHECK(mapped>=0);
        p->selection=(GD_ExplorerSelection){-1,-1,0,mapped,-1};if(v==GD_EXPLORER_MDA)p->mda.record=mapped;camera=p->camera;selection=p->selection;pending=p->query_options;query=p->query_state;
        memcpy(saved,p->layout,sizeof(*saved));for(i=0;i<GD_EXPLORER_VIEWS;++i)states[i]=c->panes[i].rule_state;
        allocations=*calls;CONTEXT_CHECK(gd_explorer_rule_context_option(c,0,0,key,sizeof(key),NULL)==1 && *calls==allocations);
        if(v==GD_EXPLORER_SKILLS){int succeeded;*fail_at=*calls+1;succeeded=gd_explorer_control_set(c,GD_FORM_RULES,"selection_context",key,message,sizeof(message));*fail_at=0;CONTEXT_CHECK(!succeeded && !p->rule_state);
            *fail_at=*calls+1;succeeded=gd_explorer_action_run(c,GD_ACTION_RULES_BINDINGS_OPEN);*fail_at=0;CONTEXT_CHECK(!succeeded && !p->rule_state);
        }
        CONTEXT_CHECK(gd_explorer_action_run(c,GD_ACTION_RULES_BINDINGS_OPEN));
        CONTEXT_CHECK(p->rule_state && !p->rule_state->bindings && !p->rule_state->result && !*p->rule_state->pending.object);
        allocations=*calls;CONTEXT_CHECK(gd_explorer_control_set(c,GD_FORM_RULES,"selection_context",key,message,sizeof(message)) && *calls==allocations);
        CONTEXT_CHECK(p->rule_state->pending.bindings && !strcmp(p->rule_state->pending.object,model->records[noun].reference) && !p->rule_state->bindings && !p->rule_state->result);
        CONTEXT_CHECK(!memcmp(saved,p->layout,sizeof(*saved)) && !memcmp(&camera,&p->camera,sizeof(camera)) && !memcmp(&selection,&p->selection,sizeof(selection)) && !memcmp(&pending,&p->query_options,sizeof(pending)) && query==p->query_state);
        for(i=0;i<GD_EXPLORER_VIEWS;++i)if(i!=v)CONTEXT_CHECK(states[i]==c->panes[i].rule_state);
    }
    p=c->pane=&c->panes[GD_EXPLORER_NOUNS];*c->active_view=p->view;
    CONTEXT_CHECK(gd_explorer_action_run(c,GD_ACTION_RULES_RUN) && p->rule_state->bindings && p->rule_state->bindings->binding_count>0);
    {Camera2D camera=p->camera;GD_ExplorerSelection selection=p->selection;
        gd_explorer_rules_choose(p->rule_state,0,-1);CONTEXT_CHECK(p->rule_state->row==0 && !memcmp(&camera,&p->camera,sizeof(camera)) && !memcmp(&selection,&p->selection,sizeof(selection)));
    }
    owner=p->rule_state;*snapshot=*owner;*fail_at=*calls+1;
    {int ran=gd_explorer_action_run(c,GD_ACTION_RULES_RUN);*fail_at=0;CONTEXT_CHECK(!ran && p->rule_state==owner && !memcmp(snapshot,owner,sizeof(*snapshot)));}
    CONTEXT_CHECK(gd_explorer_control_set(c,GD_FORM_RULES,"axis",model->records[axis].reference,message,sizeof(message)));
    p->selection=(GD_ExplorerSelection){-1,-1,0,rules->sources[rules->cases[root].source].record,-1};
    *snapshot=*owner;CONTEXT_CHECK(gd_explorer_action_run(c,GD_ACTION_RULES_OUTCOMES_OPEN));
    CONTEXT_CHECK(!memcmp(&snapshot->pending,&owner->pending,sizeof(owner->pending)) && snapshot->bindings==owner->bindings && snapshot->result==owner->result);
    CONTEXT_CHECK(gd_explorer_rule_context_option(c,1,root,key,sizeof(key),NULL)==1);
    *snapshot=*owner;CONTEXT_CHECK(!gd_explorer_control_set(c,GD_FORM_RULES,"selection_context",key,message,sizeof(message)) && strstr(message,"Pending") && !memcmp(snapshot,owner,sizeof(*snapshot)));
    CONTEXT_CHECK(gd_explorer_action_run(c,GD_ACTION_RULES_INPUTS_RESET) && owner->bindings==snapshot->bindings);
    CONTEXT_CHECK(gd_explorer_control_set(c,GD_FORM_RULES,"selection_context",key,message,sizeof(message)) && owner->bindings==snapshot->bindings && !*owner->pending.facts && !*owner->pending.contexts && !*owner->pending.participant && !owner->pending.bindings);
    CONTEXT_CHECK(gd_explorer_action_run(c,GD_ACTION_RULES_RUN) && p->rule_state->result && !p->rule_state->bindings && p->rule_state->result->request.kind==GD_NAMED_OUTCOMES);
    owner=p->rule_state;snprintf(owner->pending.case_name,sizeof(owner->pending.case_name),"missing/context#missing");*snapshot=*owner;
    CONTEXT_CHECK(!gd_explorer_action_run(c,GD_ACTION_RULES_RUN) && p->rule_state==owner && !memcmp(snapshot,owner,sizeof(*snapshot)));
    p->selection=(GD_ExplorerSelection){-1,0,0,-1,-1};
    CONTEXT_CHECK(gd_explorer_rule_context_option(c,0,0,key,sizeof(key),&snapshot->pending)==1 && !strcmp(snapshot->pending.object,model->records[p->layout->nodes[0].key].reference));
    /* Reject duplicate exact identities; do not stage the first matching case. */
    {int duplicate=(root+1)%rules->case_count,source=rules->cases[duplicate].source,offered;const char *name=rules->cases[duplicate].name;
        p->selection=(GD_ExplorerSelection){-1,-1,0,rules->sources[rules->cases[root].source].record,-1};
        rules->cases[duplicate].source=rules->cases[root].source;rules->cases[duplicate].name=rules->cases[root].name;
        offered=gd_explorer_rule_context_option(c,1,root,key,sizeof(key),NULL);
        rules->cases[duplicate].source=source;rules->cases[duplicate].name=name;CONTEXT_CHECK(offered==-1);
    }
    /* Pair contexts retain declared membership, never the first source/case. */
    for(i=0;i<c->project->noun_grid->pair_count;++i){int choices=0;
        p->selection=(GD_ExplorerSelection){i,-1,0,-1,-1};
        for(j=0;j<rules->case_count;++j){int offered=gd_explorer_rule_context_option(c,1,j,key,sizeof(key),NULL);CONTEXT_CHECK(offered==gd_noun_grid_case(c->project->noun_grid,i,j));choices+=offered;}
        CONTEXT_CHECK(choices==c->project->noun_grid->pairs[i].count);
    }
    /* Exact local identity comes from native binding metadata, not authored A. */
    p=c->pane=&c->panes[GD_EXPLORER_ACTORS];*c->active_view=p->view;
    {int found=0,edge;for(i=0;i<rules->case_count && !found;++i)for(j=rules->cases[i].first_participant;j<rules->cases[i].first_participant+rules->cases[i].participant_count && !found;++j){
        if(rules->participants[j].noun_resolution!=GD_RULE_RESOLVED)continue;
        for(edge=rules->actor_first[rules->participants[j].noun_record];edge>=0;edge=rules->next_axis[edge])if(rules->axis_resolution[edge]==GD_RULE_RESOLVED){int target=model->edges[edge].target;
            if(rules->axes[target].valid && rules->axes[target].type==GD_AXIS_ENUM && model->records[target].axis.allowed_count>1){p->query_options.root_case=i;p->query_options.participant=j;p->query_options.axis=target;found=1;break;}
        }
    }CONTEXT_CHECK(found && gd_explorer_query_run(c));}
    for(i=0;i<p->layout->node_count;++i){const GD_QuerySceneNode *node=&p->query_state->scene->nodes[i];
        if(node->kind!=GD_QUERY_NODE_BINDING)continue;
        p->selection=(GD_ExplorerSelection){-1,i,0,-1,-1};
        CONTEXT_CHECK(gd_explorer_rule_context_option(c,0,rule_context_participant_case(rules,node->source)+2,key,sizeof(key),&snapshot->pending)==1 && !strcmp(snapshot->pending.participant,rules->participants[node->source].name));
        CONTEXT_CHECK(!snapshot->pending.facts[0] && !snapshot->pending.contexts[0]);
        {int source_case=rule_context_participant_case(rules,node->source);const GD_RuleCase *item=&rules->cases[source_case];
            if(item->participant_count>1){int duplicate=item->first_participant+(node->source==item->first_participant);const char *name=rules->participants[duplicate].name;int offered;
                rules->participants[duplicate].name=rules->participants[node->source].name;offered=gd_explorer_rule_context_option(c,0,source_case+2,key,sizeof(key),NULL);rules->participants[duplicate].name=name;CONTEXT_CHECK(offered==-1);
            }
        }break;
    }
    CONTEXT_CHECK(i<p->layout->node_count);
    /* Native Actor and Effect metadata stay typed; overview is not a relation index. */
    CONTEXT_CHECK(gd_explorer_query_overview(c,GD_ACTOR_USAGE));
    for(i=0;i<p->layout->edge_count;++i){const GD_QueryScene *scene=p->query_state->scene;int choices=0;
        p->selection=(GD_ExplorerSelection){i,-1,0,-1,-1};
        for(j=0;j<rules->case_count;++j){int offered=gd_explorer_rule_context_option(c,1,j,key,sizeof(key),NULL);CONTEXT_CHECK(offered==gd_query_scene_overview_case(scene,i,j));choices+=offered;}
        if(scene->edges[i].kind==GD_QUERY_EDGE_AXIS)CONTEXT_CHECK(!choices && gd_explorer_rule_context_option(c,0,0,key,sizeof(key),NULL)==1 && gd_explorer_rule_context_option(c,0,1,other,sizeof(other),NULL)==1 && strcmp(key,other));
    }
    p=c->pane=&c->panes[GD_EXPLORER_EFFECTS];*c->active_view=p->view;
    for(i=0;i<p->layout->node_count;++i){const GD_QueryScene *scene=p->query_state->scene;const GD_EffectNode *node=&scene->query->effects->nodes[scene->nodes[i].source];
        if(node->kind!=GD_EFFECT_ACTIVATION)continue;
        p->selection=(GD_ExplorerSelection){-1,i,0,-1,-1};j=scene->query->resource->nodes[node->activation].case_index;
        CONTEXT_CHECK(gd_explorer_rule_context_option(c,1,j,key,sizeof(key),NULL)==1);break;
    }
    CONTEXT_CHECK(i<p->layout->node_count);
    for(i=0;i<p->layout->node_count;++i){const GD_QueryScene *scene=p->query_state->scene;const GD_EffectNode *node=&scene->query->effects->nodes[scene->nodes[i].source];
        if(scene->nodes[i].kind!=GD_QUERY_NODE_EFFECT || node->relation>=0)continue;
        {GD_ExplorerRuleContext discovered;GD_ExplorerContext batch=*c;unsigned char expected[GD_MAX_RULE_CASES]={0};int edge;
            p->selection=(GD_ExplorerSelection){-1,i,0,-1,-1};gd_explorer_rule_context_discover(c,&discovered);batch.rule_context=&discovered;
            for(edge=0;edge<scene->layout.edge_count;++edge)if(scene->layout.edges[edge].b==i){int source_case=rule_context_edge_case(scene,edge);if(source_case>=0)expected[source_case]=1;}
            for(j=0;j<rules->case_count;++j)CONTEXT_CHECK(gd_explorer_rule_context_option(&batch,1,j,key,sizeof(key),NULL)==expected[j]);
        }break;
    }
    CONTEXT_CHECK(i<p->layout->node_count);
    /* Actual guarded IPC discovery, stage, stale selection and retained byte-page output. */
    p=c->pane=&c->panes[GD_EXPLORER_MDA];*c->active_view=p->view;
    {ExplorerViewer viewer={0};GD_ViewerRequest request={0};char reply[16384],decoded[8193];GD_ViewerWriter writer;
        viewer.context=c;viewer.panes=c->panes;viewer.active=c->active_view;
        strcpy(request.command,"controls");strcpy(request.projection,"mda");strcpy(request.form,"rules");strcpy(request.control,"selection_context");request.limit=32;
        writer=(GD_ViewerWriter){reply,0,sizeof(reply),0};viewer_dispatch(&viewer,&request,&writer);
        CONTEXT_CHECK(!writer.failed && strstr(reply,"\"choices_closed\":true") && strstr(reply,"noun:"));
        CONTEXT_CHECK(gd_explorer_rule_context_option(c,0,0,key,sizeof(key),NULL)==1);
        viewer_control_token(&viewer,c,GD_FORM_RULES,request.expect,sizeof(request.expect));strcpy(request.command,"configure");strcpy(request.value,key);
        p->selection.linked=p->mda.record=-1;owner=p->rule_state;*snapshot=*owner;
        writer=(GD_ViewerWriter){reply,0,sizeof(reply),0};viewer_dispatch(&viewer,&request,&writer);
        CONTEXT_CHECK(strstr(reply,"viewer.stale_controls") && p->rule_state==owner && !memcmp(snapshot,owner,sizeof(*snapshot)));
        viewer_control_token(&viewer,c,GD_FORM_RULES,request.expect,sizeof(request.expect));writer=(GD_ViewerWriter){reply,0,sizeof(reply),0};viewer_dispatch(&viewer,&request,&writer);
        CONTEXT_CHECK(strstr(reply,"viewer.control") && !memcmp(snapshot,owner,sizeof(*snapshot)));
        p->mda.record=p->selection.linked=gd_explorer_record(gd_explorer_model(c->project,p->view),model->records[noun].reference);
        viewer_control_token(&viewer,c,GD_FORM_RULES,request.expect,sizeof(request.expect));writer=(GD_ViewerWriter){reply,0,sizeof(reply),0};viewer_dispatch(&viewer,&request,&writer);
        CONTEXT_CHECK(strstr(reply,"\"configured\":true") && p->rule_state==owner && !owner->bindings && !owner->result);
        strcpy(request.command,"action");strcpy(request.action,"rules.run");*request.form=*request.control=0;
        viewer_control_token(&viewer,c,GD_FORM_RULES,request.expect,sizeof(request.expect));writer=(GD_ViewerWriter){reply,0,sizeof(reply),0};viewer_dispatch(&viewer,&request,&writer);
        CONTEXT_CHECK(strstr(reply,"\"executed\":true") && p->rule_state->bindings);
        strcpy(request.command,"result");strcpy(request.form,"bindings");strcpy(request.fields,"bindings");request.limit=8;*request.expect=0;
        writer=(GD_ViewerWriter){reply,0,sizeof(reply),0};viewer_dispatch(&viewer,&request,&writer);
        CONTEXT_CHECK(!writer.failed && presentation_decode(reply,decoded,sizeof(decoded))>0 && strstr(decoded,model->records[noun].reference));
    }
    /* Reload candidates and Reset view own no other pane's rule state. */
    {GD_ExplorerPane replacement=*p;GD_ExplorerContext next=*c;int remapped,stable;replacement.rule_state=NULL;next.pane=&replacement;
        owner=p->rule_state;*snapshot=*owner;remapped=gd_explorer_rules_remap(&next,c->project,p);
        stable=remapped && replacement.rule_state && replacement.rule_state!=owner && replacement.rule_state->bindings!=owner->bindings && gd_explorer_form_equal(GD_FORM_RULES,&replacement.rule_state->applied,&owner->applied);
        gd_explorer_rules_destroy(replacement.rule_state);CONTEXT_CHECK(stable && p->rule_state==owner && !memcmp(snapshot,owner,sizeof(*snapshot)));
        snprintf(owner->pending.axis,sizeof(owner->pending.axis),"%s",model->records[axis].reference);*snapshot=*owner;replacement.rule_state=NULL;
        remapped=gd_explorer_rules_remap(&next,c->project,p);gd_explorer_rules_destroy(replacement.rule_state);
        CONTEXT_CHECK(!remapped && p->rule_state==owner && !memcmp(snapshot,owner,sizeof(*snapshot)));
        CONTEXT_CHECK(reset_view(c) && p->rule_state==owner && !memcmp(snapshot,owner,sizeof(*snapshot)));
    }
    {GD_Rules *loaded=c->project->rules;int available,slots;c->project->rules=NULL;available=gd_explorer_form_available(c,GD_FORM_RULES);slots=gd_explorer_rule_context_slots(c,0);c->project->rules=loaded;CONTEXT_CHECK(!available && !slots);}
    CONTEXT_CHECK(!c->project->draft || draft_hash==viewer_control_hash(1,c->project->draft,sizeof(*c->project->draft)));
    CONTEXT_CHECK(gd_explorer_reset_all(c));for(v=0;v<GD_EXPLORER_VIEWS;++v)CONTEXT_CHECK(!c->panes[v].rule_state);
    result=1;
cleanup:
    *fail_at=0;if(snapshot)allocator.release(allocator.context,snapshot);
    fprintf(stderr,"Rule context checks: %d; discovery %llu bytes; pane rule owner %llu bytes\n",checks,(unsigned long long)sizeof(GD_ExplorerRuleContext),(unsigned long long)sizeof(GD_ExplorerRuleState));
#undef CONTEXT_CHECK
    return result;
}
#endif
