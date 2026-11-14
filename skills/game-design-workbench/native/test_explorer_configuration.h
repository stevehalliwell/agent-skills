/* Production owner-dispatch loops on private panes; never touch human windows. */
#ifndef TEST_EXPLORER_CONFIGURATION_H
#define TEST_EXPLORER_CONFIGURATION_H
static int viewer_configuration_checks(ExplorerViewer *original,int *fail_at,int *calls,char *reply) {
    GD_Allocator a=original->context->project->allocator; GD_ExplorerPane *panes=NULL;
    GD_ExplorerContext c=*original->context,target; ExplorerViewer viewer={0}; GD_ViewerRequest request={0}; GD_ViewerWriter w;
    const GD_Rules *rules=c.project->rules; const GD_Model *model=rules->model;
    int active=GD_EXPLORER_ACTORS,stale=0,checks=0,result=0,v,root=-1,participant=-1,axis=-1,edge,i;
    char case_key[GD_MAX_PATH],samples[GD_MAX_RULE_LINE+1];
#define CONFIG_CHECK(value) do { ++checks; if (!(value)) { fprintf(stderr,"Viewer configuration check failed: %s (line %d)\n",#value,__LINE__); goto cleanup; } } while (0)
#define SEND() do { w=(GD_ViewerWriter){reply,0,GD_VIEWER_REPLY_LIMIT,0}; viewer_dispatch(&viewer,&request,&w); } while (0)
#define STAGE(view_,form_,field_,value_) do { memset(&request,0,sizeof(request)); strcpy(request.command,"configure"); strcpy(request.projection,viewer_projections[view_]); strcpy(request.form,gd_explorer_form_name(form_)); strcpy(request.control,field_); snprintf(request.value,sizeof(request.value),"%s",value_); target=c; target.pane=&panes[view_]; viewer_control_token(&viewer,&target,form_,request.expect,sizeof(request.expect)); SEND(); } while (0)
#define RUN(view_,action_,form_) do { memset(&request,0,sizeof(request)); strcpy(request.command,"action"); strcpy(request.projection,viewer_projections[view_]); strcpy(request.action,action_); target=c; target.pane=&panes[view_]; viewer_control_token(&viewer,&target,form_,request.expect,sizeof(request.expect)); SEND(); } while (0)
    panes=a.allocate(a.context,GD_EXPLORER_VIEWS*sizeof(*panes)); CONFIG_CHECK(panes!=NULL); memset(panes,0,GD_EXPLORER_VIEWS*sizeof(*panes));
    for (v=0;v<GD_EXPLORER_VIEWS;++v) pane_defaults(&panes[v],(GD_ExplorerView)v);
    c.pane=&panes[active]; viewer.context=&c; viewer.panes=panes; viewer.active=&active; viewer.stale=&stale; viewer.reload_generation=1; viewer.collections=original->collections; viewer.collection_count=original->collection_count;
    strcpy(request.command,"controls"); v=*calls; SEND();
    CONFIG_CHECK(!w.failed && strstr(reply,"\"value_encoding\"") && strstr(reply,"root_case"));
    CONFIG_CHECK(*calls==v && !panes[active].query_state && !panes[active].structure_state);
    STAGE(GD_EXPLORER_EFFECTS,GD_FORM_QUERY,"accepted","true"); CONFIG_CHECK(!panes[GD_EXPLORER_EFFECTS].query_state);
    {
        int before=*calls; memset(&request,0,sizeof(request)); strcpy(request.command,"refresh"); SEND();
        CONFIG_CHECK(strstr(reply,"viewer.refresh") && strstr(reply,"pending query inputs") && *calls==before && !panes[GD_EXPLORER_EFFECTS].query_state);
    }
    target=c; target.pane=&panes[GD_EXPLORER_EFFECTS]; CONFIG_CHECK(pane_prepare(&target));
    CONFIG_CHECK(panes[GD_EXPLORER_EFFECTS].query_options.accepted && !panes[GD_EXPLORER_EFFECTS].query_state->applied.accepted); /* Navigation preparer never applies staged inputs. */
    panes[GD_EXPLORER_EFFECTS].query_options=panes[GD_EXPLORER_EFFECTS].query_state->applied;
    STAGE(GD_EXPLORER_ACTORS,GD_FORM_QUERY,"coverage","true"); CONFIG_CHECK(strstr(reply,"\"configured\":true") && panes[active].query_options.coverage && !panes[active].query_state);
    SEND(); CONFIG_CHECK(strstr(reply,"viewer.stale_controls")!=NULL); /* Never replay an old write token. */
    igGetIO_Nil()->WantTextInput=true;
    STAGE(GD_EXPLORER_ACTORS,GD_FORM_QUERY,"coverage","false"); CONFIG_CHECK(strstr(reply,"viewer.busy")!=NULL); igGetIO_Nil()->WantTextInput=false;
    CONFIG_CHECK(panes[active].query_options.coverage && !panes[active].query_state);
    STAGE(GD_EXPLORER_ACTORS,GD_FORM_QUERY,"coverage","false"); CONFIG_CHECK(strstr(reply,"\"configured\":true")!=NULL);
    for (i=0;i<rules->case_count && root<0;++i) {
        const GD_RuleCase *item=&rules->cases[i]; int p;
        for (p=item->first_participant;p<item->first_participant+item->participant_count && root<0;++p) if (rules->participants[p].noun_resolution==GD_RULE_RESOLVED) {
            for (edge=rules->actor_first[rules->participants[p].noun_record];edge>=0;edge=rules->next_axis[edge]) if (rules->axis_resolution[edge]==GD_RULE_RESOLVED) {
                int candidate=model->edges[edge].target;
                if (rules->axes[candidate].type==GD_AXIS_ENUM && model->records[candidate].axis.allowed_count>=2) { root=i; participant=p; axis=candidate; break; }
            }
        }
    }
    CONFIG_CHECK(root>=0 && participant>=0 && axis>=0);
    CONFIG_CHECK(snprintf(case_key,sizeof(case_key),"%s#%s",model->records[rules->sources[rules->cases[root].source].record].reference,rules->cases[root].name)<(int)sizeof(case_key));
    STAGE(GD_EXPLORER_ACTORS,GD_FORM_QUERY,"root_case","0"); CONFIG_CHECK(strstr(reply,"viewer.control") && panes[active].query_options.root_case<0);
    STAGE(GD_EXPLORER_ACTORS,GD_FORM_QUERY,"root_case",case_key); CONFIG_CHECK(panes[active].query_options.root_case==root && !strstr(reply,"\"ok\":false"));
    STAGE(GD_EXPLORER_ACTORS,GD_FORM_QUERY,"participant",rules->participants[participant].name); CONFIG_CHECK(panes[active].query_options.participant==participant);
    STAGE(GD_EXPLORER_ACTORS,GD_FORM_QUERY,"axis",model->records[axis].reference); CONFIG_CHECK(panes[active].query_options.axis==axis);
    STAGE(GD_EXPLORER_ACTORS,GD_FORM_QUERY,"literal_values","true"); CONFIG_CHECK(panes[active].query_options.literal_values);
    {
        const char *first=model->records[axis].axis.allowed,*second=first+strlen(first)+1;
        CONFIG_CHECK(snprintf(samples,sizeof(samples),"%s\n%s",first,second)<(int)sizeof(samples));
        STAGE(GD_EXPLORER_ACTORS,GD_FORM_QUERY,"values",samples); CONFIG_CHECK(!strcmp(panes[active].query_options.values,samples) && !panes[active].query_state);
        RUN(GD_EXPLORER_ACTORS,"query.run",GD_FORM_QUERY);
        CONFIG_CHECK(strstr(reply,"\"executed\":true") && panes[active].query_state && panes[active].query_state->query->variants && panes[active].query_state->query->variants->variant_count==2);
        CONFIG_CHECK(!strcmp(panes[active].query_state->applied.values,samples));
        STAGE(GD_EXPLORER_ACTORS,GD_FORM_QUERY,"ghosts","stacked"); CONFIG_CHECK(panes[active].query_options.ghosts==GD_QUERY_STACKED_VARIANTS);
        STAGE(GD_EXPLORER_ACTORS,GD_FORM_QUERY,"compare",second); CONFIG_CHECK(panes[active].query_options.compare==1 && panes[active].query_state->applied.compare<0);
        RUN(GD_EXPLORER_ACTORS,"query.display",GD_FORM_QUERY); CONFIG_CHECK(strstr(reply,"\"executed\":true") && panes[active].query_state->applied.compare==1);
    }
    CONFIG_CHECK(viewer_retained_smoke(&viewer,GD_EXPLORER_ACTORS,"query","actor-states",reply));
    RUN(GD_EXPLORER_ACTORS,"view.compare",GD_FORM_PRESENTATION); CONFIG_CHECK(strstr(reply,"\"executed\":true") && panes[active].query_state->comparison_ready);
    CONFIG_CHECK(viewer_retained_smoke(&viewer,GD_EXPLORER_ACTORS,"comparison","\"family\":\"actor\"",reply));
    {
        GD_ExplorerQueryState *last=panes[active].query_state;
        STAGE(GD_EXPLORER_ACTORS,GD_FORM_QUERY,"axis",""); CONFIG_CHECK(panes[active].query_options.axis<0 && panes[active].query_state==last);
        RUN(GD_EXPLORER_ACTORS,"query.run",GD_FORM_QUERY); CONFIG_CHECK(strstr(reply,"viewer.action") && panes[active].query_state==last);
        STAGE(GD_EXPLORER_ACTORS,GD_FORM_QUERY,"axis",model->records[axis].reference); CONFIG_CHECK(panes[active].query_options.axis==axis);
        STAGE(GD_EXPLORER_ACTORS,GD_FORM_QUERY,"values",samples); CONFIG_CHECK(!strcmp(panes[active].query_options.values,samples));
    }
    STAGE(GD_EXPLORER_MDA,GD_FORM_STRUCTURE,"kind","similarity-features"); CONFIG_CHECK(panes[GD_EXPLORER_MDA].structure_state && !panes[GD_EXPLORER_MDA].structure_state->result && active==GD_EXPLORER_ACTORS);
    memset(&request,0,sizeof(request)); strcpy(request.command,"controls"); strcpy(request.projection,"mda"); strcpy(request.form,"structure"); strcpy(request.control,"features"); request.limit=64;
    v=*calls; SEND(); CONFIG_CHECK(strstr(reply,"\"canonical_choices_ready\":false") && strstr(reply,"\"choice_slots\":0")); CONFIG_CHECK(*calls==v);
    RUN(GD_EXPLORER_MDA,"structure.run",GD_FORM_STRUCTURE); CONFIG_CHECK(strstr(reply,"\"executed\":true") && panes[GD_EXPLORER_MDA].structure_state->result);
    memset(&request,0,sizeof(request)); strcpy(request.command,"controls"); strcpy(request.projection,"mda"); strcpy(request.form,"structure"); strcpy(request.control,"features"); request.limit=64;
    v=*calls; SEND(); CONFIG_CHECK(strstr(reply,"\"canonical_choices_ready\":true") && strstr(reply,"retained-partners")); CONFIG_CHECK(*calls==v);
    {
        char feature[GD_MAX_PATH]; int native; target=c; target.pane=&panes[GD_EXPLORER_MDA];
        CONFIG_CHECK(gd_explorer_control_option(&target,GD_FORM_STRUCTURE,gd_explorer_form_field(GD_FORM_STRUCTURE,"features"),&panes[GD_EXPLORER_MDA].structure_state->pending,0,&native,feature,sizeof(feature))==1);
        STAGE(GD_EXPLORER_MDA,GD_FORM_STRUCTURE,"kind","similarity-map"); CONFIG_CHECK(strstr(reply,"\"configured\":true")!=NULL);
        STAGE(GD_EXPLORER_MDA,GD_FORM_STRUCTURE,"features",feature); CONFIG_CHECK(strstr(reply,"\"configured\":true")!=NULL);
    }
    RUN(GD_EXPLORER_MDA,"structure.run",GD_FORM_STRUCTURE);
    CONFIG_CHECK(strstr(reply,"\"executed\":true") && panes[GD_EXPLORER_MDA].structure_state->result && panes[GD_EXPLORER_MDA].structure_state->result->similarity->map->available);
    CONFIG_CHECK(active==GD_EXPLORER_ACTORS && panes[active].query_state);
    CONFIG_CHECK(viewer_retained_smoke(&viewer,GD_EXPLORER_MDA,"structure","similarity",reply));
    {
        GD_Model *mda_model=gd_explorer_model(c.project,GD_EXPLORER_MDA);target=c;target.pane=&panes[GD_EXPLORER_MDA];CONFIG_CHECK(pane_prepare(&target));
        STAGE(GD_EXPLORER_MDA,GD_FORM_PRESENTATION,"similarity_canvas","true");CONFIG_CHECK(strstr(reply,"\"configured\":true") && panes[GD_EXPLORER_MDA].structure_state->similarity_canvas);
        STAGE(GD_EXPLORER_MDA,GD_FORM_RULES,"bindings","true");CONFIG_CHECK(strstr(reply,"\"configured\":true"));
        STAGE(GD_EXPLORER_MDA,GD_FORM_RULES,"case_name",case_key);CONFIG_CHECK(strstr(reply,"\"configured\":true"));
        RUN(GD_EXPLORER_MDA,"rules.run",GD_FORM_RULES);CONFIG_CHECK(strstr(reply,"\"executed\":true") && gd_explorer_rules_row_count(panes[GD_EXPLORER_MDA].rule_state)>0);
        {GD_ExplorerPane *p=&panes[GD_EXPLORER_MDA];GD_ExplorerStructureState *state=p->structure_state;Camera2D map_camera=state->similarity_camera,source_camera=p->camera;int record=p->mda.record,link=p->mda.link,before=*calls;
            memset(&request,0,sizeof(request));strcpy(request.command,"select");strcpy(request.projection,"mda");strcpy(request.form,"rule");target=c;target.pane=p;
            viewer_row_key(&target,"rule",0,request.id,sizeof(request.id));viewer_control_token(&viewer,&target,GD_FORM_PRESENTATION,request.expect,sizeof(request.expect));SEND();
            CONFIG_CHECK(strstr(reply,"\"selected\":true") && p->rule_state->row==0 && state->similarity_canvas && gd_explorer_similarity_active(&target));
            CONFIG_CHECK(*calls==before && active==GD_EXPLORER_ACTORS && p->mda.record==record && p->mda.link==link && !memcmp(&map_camera,&state->similarity_camera,sizeof(map_camera)) && !memcmp(&source_camera,&p->camera,sizeof(source_camera)));
        }
        {GD_ExplorerPane *p=&panes[GD_EXPLORER_MDA];GD_ExplorerStructureState *state=p->structure_state;const GD_SimilarityProjection *projection=state->result->similarity;int record,node,before,selected,peer;
            Camera2D source_camera=p->camera,map_camera=state->similarity_camera;const GD_StructureResult *retained=state->result;uint64_t pending,applied,rules_pending;
            STAGE(GD_EXPLORER_MDA,GD_FORM_PRESENTATION,"similarity_row",projection->ids[0]);CONFIG_CHECK(strstr(reply,"\"configured\":true"));
            STAGE(GD_EXPLORER_MDA,GD_FORM_PRESENTATION,"similarity_peer",projection->ids[projection->entry_count>1 ? 1:0]);CONFIG_CHECK(strstr(reply,"\"configured\":true"));
            node=projection->nodes[0];record=gd_structure_result_record(projection->context,projection->context->layout->nodes[node].key);CONFIG_CHECK(record>=0);
            selected=state->similarity_row;peer=state->similarity_peer;pending=viewer_form_hash(1,GD_FORM_STRUCTURE,&state->pending);applied=viewer_form_hash(1,GD_FORM_STRUCTURE,&state->applied);rules_pending=viewer_form_hash(1,GD_FORM_RULES,&p->rule_state->pending);before=*calls;
            RUN(GD_EXPLORER_MDA,"view.similarity.source",GD_FORM_PRESENTATION);CONFIG_CHECK(strstr(reply,"\"executed\":true") && !state->similarity_canvas && !gd_explorer_action_available(&target,GD_ACTION_VIEW_COMPARE) && p->mda.record==record && p->mda.link<0 && active==GD_EXPLORER_ACTORS && p->navigate<0 && !p->fit);
            CONFIG_CHECK(*calls==before && state->result==retained && selected==state->similarity_row && peer==state->similarity_peer && !memcmp(&source_camera,&p->camera,sizeof(source_camera)) && !memcmp(&map_camera,&state->similarity_camera,sizeof(map_camera)));
            RUN(GD_EXPLORER_MDA,"view.similarity.return",GD_FORM_PRESENTATION);CONFIG_CHECK(strstr(reply,"\"executed\":true") && state->similarity_canvas && gd_explorer_action_available(&target,GD_ACTION_VIEW_COMPARE) && selected==state->similarity_row && peer==state->similarity_peer && active==GD_EXPLORER_ACTORS);
            CONFIG_CHECK(*calls==before && pending==viewer_form_hash(1,GD_FORM_STRUCTURE,&state->pending) && applied==viewer_form_hash(1,GD_FORM_STRUCTURE,&state->applied) && rules_pending==viewer_form_hash(1,GD_FORM_RULES,&p->rule_state->pending) && !memcmp(&map_camera,&state->similarity_camera,sizeof(map_camera)));
            target=c;target.pane=p;CONFIG_CHECK(!gd_explorer_source_open(&target,-1) && state->similarity_canvas && p->mda.record==record && *calls==before);
            state->similarity_row=-1;CONFIG_CHECK(!gd_explorer_action_available(&target,GD_ACTION_SIMILARITY_SOURCE) && !gd_explorer_similarity_source(&target) && state->similarity_canvas);state->similarity_row=selected;
        }
        {
            GD_ExplorerContext resource_context=c;int first,count;resource_context.pane=&panes[GD_EXPLORER_RESOURCES];CONFIG_CHECK(pane_prepare(&resource_context));
            RUN(GD_EXPLORER_RESOURCES,"view.analyze",GD_FORM_PRESENTATION);CONFIG_CHECK(strstr(reply,"\"executed\":true")!=NULL);first=c.analysis->keys[0];count=c.analysis->node_count;
            target=c;target.pane=&panes[GD_EXPLORER_MDA];(void)analyze(&target);(void)evidence(&target,-1);viewer_record_layers(&viewer,&target);
            CONFIG_CHECK(c.analysis->keys[0]==first && c.analysis->node_count==count && viewer_layers_current(&viewer,&resource_context));
        }
        memset(&request,0,sizeof(request));strcpy(request.command,"select");strcpy(request.projection,"mda");strcpy(request.form,"source");strcpy(request.id,mda_model->records[mda_model->record_count-1].reference);viewer_control_token(&viewer,&target,GD_FORM_PRESENTATION,request.expect,sizeof(request.expect));SEND();
        CONFIG_CHECK(strstr(reply,"\"selected\":true") && !panes[GD_EXPLORER_MDA].structure_state->similarity_canvas && selected_record(&target)==mda_model->record_count-1 && active==GD_EXPLORER_ACTORS);
    }
    {
        GD_ExplorerPane *skill=&panes[GD_EXPLORER_SKILLS];GD_Model *skill_model=gd_explorer_model(c.project,GD_EXPLORER_SKILLS);int challenge=-1,before,valid,count;Camera2D camera;GD_LayoutNode nodes[2];
        target=c;target.pane=skill;CONFIG_CHECK(pane_prepare(&target) && !gd_explorer_similarity_return(&target));
        for(i=0;i<skill_model->record_count;++i)if(skill_model->records[i].role==GD_CHALLENGE){challenge=i;break;}CONFIG_CHECK(challenge>=0);
        camera=skill->camera;before=*calls;CONFIG_CHECK(gd_explorer_source_open(&target,challenge) && selected_record(&target)==challenge && skill->navigate<0 && active==GD_EXPLORER_ACTORS && *calls==before && !memcmp(&camera,&skill->camera,sizeof(camera)));
        /* Synthetic representation metadata only; restore before rendering. */
        count=skill->layout->node_count;memcpy(nodes,skill->layout->nodes,sizeof(nodes));skill->layout->node_count=0;
        valid=gd_explorer_source_open(&target,challenge) && skill->selection.node<0 && skill->selection.linked==challenge && strstr(skill->query_message,"no ordinary canvas representation");
        skill->layout->node_count=2;skill->layout->nodes[0].key=skill->layout->nodes[1].key=challenge;
        valid=valid && gd_explorer_source_open(&target,challenge) && skill->selection.node<0 && skill->selection.linked==challenge && strstr(skill->query_message,"multiple representations");
        skill->layout->node_count=count;memcpy(skill->layout->nodes,nodes,sizeof(nodes));CONFIG_CHECK(valid && *calls==before && active==GD_EXPLORER_ACTORS && !memcmp(&camera,&skill->camera,sizeof(camera)));
    }
    {
        GD_ExplorerPane *resource=&panes[GD_EXPLORER_RESOURCES]; GD_ResourceRoutes *last; int target_node=-1,witness; char key[GD_MAX_PATH]; unsigned char marked[GD_RESOURCE_GRAPH_NODES];
        CONFIG_CHECK(gd_resource_node_id(c.project->resources,0,-1,key,sizeof(key)));
        STAGE(GD_EXPLORER_RESOURCES,GD_FORM_RESOURCE,"to",key); CONFIG_CHECK(resource->resource_pending.to==0 && !resource->resource_routes);
        RUN(GD_EXPLORER_RESOURCES,"resource.run",GD_FORM_RESOURCE); CONFIG_CHECK(strstr(reply,"\"executed\":true") && resource->resource_routes && resource->resource_applied.to==0);
        for (i=0;i<resource->resource_routes->arc_count;++i) if (resource->resource_routes->arcs[i].kind==GD_RESOURCE_CREDIT || resource->resource_routes->arcs[i].kind==GD_RESOURCE_STATE_PROPOSAL) { target_node=resource->resource_routes->arcs[i].to; break; }
        CONFIG_CHECK(target_node>=0 && gd_resource_node_id(c.project->resources,target_node,-1,key,sizeof(key)));
        last=resource->resource_routes; witness=resource->resource_witness; resource->resource_only_witness=true;
        STAGE(GD_EXPLORER_RESOURCES,GD_FORM_RESOURCE,"to",key); CONFIG_CHECK(strstr(reply,"\"configured\":true") && resource->resource_routes==last && resource->resource_witness==witness && resource->resource_only_witness);
        RUN(GD_EXPLORER_RESOURCES,"resource.run",GD_FORM_RESOURCE); CONFIG_CHECK(strstr(reply,"\"executed\":true") && resource->resource_routes!=last && resource->resource_routes->witness_count>0 && resource->resource_applied.to==target_node);
        CONFIG_CHECK(viewer_retained_smoke(&viewer,GD_EXPLORER_RESOURCES,"resource",NULL,reply));
        last=resource->resource_routes; witness=resource->resource_witness; memcpy(marked,last->marked_nodes,sizeof(marked));
        STAGE(GD_EXPLORER_RESOURCES,GD_FORM_RESOURCE,"from",key); CONFIG_CHECK(resource->resource_pending.from==target_node && resource->resource_applied.from<0 && resource->resource_routes==last);
        STAGE(GD_EXPLORER_RESOURCES,GD_FORM_RESOURCE,"accepted","true"); CONFIG_CHECK(resource->resource_pending.accepted && !resource->resource_applied.accepted && resource->resource_routes==last);
        STAGE(GD_EXPLORER_RESOURCES,GD_FORM_RESOURCE,"work_limit","0"); CONFIG_CHECK(!resource->resource_pending.work_limit && resource->resource_applied.work_limit==GD_RESOURCE_ROUTE_WORK && resource->resource_routes==last);
        memset(&request,0,sizeof(request)); strcpy(request.command,"inspect"); strcpy(request.fields,"panes"); SEND();
        CONFIG_CHECK(strstr(reply,"\"resource_query\":") && strstr(reply,",\"accepted\":true,\"work_limit\":0}") && strstr(reply,",\"accepted\":false,\"work_limit\":16777216},\"pending_differs\":true}"));
        {
            ImGuiIO *io=igGetIO_Nil(); target=c; target.pane=resource; CONFIG_CHECK(pane_prepare(&target));
            io->DisplaySize=(ImVec2_c){1000,900}; io->DeltaTime=1.0f/60.0f; io->BackendFlags|=ImGuiBackendFlags_RendererHasTextures;
            igNewFrame(); igSetNextWindowSize((ImVec2_c){900,850},ImGuiCond_Always); igBegin("Resource staged controls",NULL,ImGuiWindowFlags_NoSavedSettings);
            gd_explorer_resource_controls(&target); igEnd(); igEndFrame();
            CONFIG_CHECK(resource->resource_routes==last && resource->resource_witness==witness && !memcmp(marked,last->marked_nodes,sizeof(marked)));
        }
        STAGE(GD_EXPLORER_RESOURCES,GD_FORM_RESOURCE,"to",""); CONFIG_CHECK(resource->resource_pending.to<0 && resource->resource_routes==last);
        RUN(GD_EXPLORER_RESOURCES,"resource.run",GD_FORM_RESOURCE); CONFIG_CHECK(strstr(reply,"viewer.action") && strstr(reply,"concrete target") && resource->resource_routes==last);
        CONFIG_CHECK(resource->resource_witness==witness && resource->resource_only_witness && !memcmp(marked,last->marked_nodes,sizeof(marked)));
        STAGE(GD_EXPLORER_RESOURCES,GD_FORM_RESOURCE,"to","0"); CONFIG_CHECK(strstr(reply,"viewer.control") && resource->resource_pending.to<0 && resource->resource_routes==last);
        {
            GD_ExplorerQueryOptions pending=panes[active].query_options; GD_ExplorerProject *project=c.project; int before=*calls;
            panes[active].query_options=panes[active].query_state->applied;
            memset(&request,0,sizeof(request)); strcpy(request.command,"refresh"); SEND();
            CONFIG_CHECK(strstr(reply,"viewer.refresh") && strstr(reply,"pending resource inputs") && c.project==project && *calls==before && resource->resource_routes==last);
            panes[active].query_options=pending;
        }
        resource->resource_pending=resource->resource_applied;
        *fail_at=*calls+1; RUN(GD_EXPLORER_RESOURCES,"resource.run",GD_FORM_RESOURCE); *fail_at=0;
        CONFIG_CHECK(strstr(reply,"viewer.action") && strstr(reply,"previous result retained") && resource->resource_routes==last && resource->resource_witness==witness && !memcmp(marked,last->marked_nodes,sizeof(marked)));
        RUN(GD_EXPLORER_RESOURCES,"resource.clear",GD_FORM_RESOURCE); CONFIG_CHECK(strstr(reply,"\"executed\":true") && !resource->resource_routes && resource->resource_pending.to<0 && resource->resource_pending.from<0 && resource->resource_witness<0 && resource->resource_only_witness);
        CONFIG_CHECK(active==GD_EXPLORER_ACTORS);
    }
    STAGE(GD_EXPLORER_RESOURCES,GD_FORM_RULES,"bindings","true"); CONFIG_CHECK(panes[GD_EXPLORER_RESOURCES].rule_state && panes[GD_EXPLORER_RESOURCES].rule_state->pending.bindings);
    STAGE(GD_EXPLORER_RESOURCES,GD_FORM_RULES,"object",model->records[rules->participants[participant].noun_record].reference); CONFIG_CHECK(strstr(reply,"\"configured\":true")!=NULL);
    STAGE(GD_EXPLORER_RESOURCES,GD_FORM_RULES,"case_name",case_key); CONFIG_CHECK(strstr(reply,"\"configured\":true")!=NULL);
    STAGE(GD_EXPLORER_RESOURCES,GD_FORM_RULES,"participant",rules->participants[participant].name); CONFIG_CHECK(strstr(reply,"\"configured\":true")!=NULL);
    STAGE(GD_EXPLORER_RESOURCES,GD_FORM_RULES,"axis",model->records[axis].reference); CONFIG_CHECK(strstr(reply,"\"configured\":true")!=NULL);
    RUN(GD_EXPLORER_RESOURCES,"rules.run",GD_FORM_RULES);
    CONFIG_CHECK(strstr(reply,"\"executed\":true") && panes[GD_EXPLORER_RESOURCES].rule_state->bindings && panes[GD_EXPLORER_RESOURCES].rule_state->bindings->binding_count>0);
    CONFIG_CHECK(viewer_retained_smoke(&viewer,GD_EXPLORER_RESOURCES,"bindings","bindings",reply));
    {
        char selection_token[64],row_key[64]; GD_ExplorerRuleState *state=panes[GD_EXPLORER_RESOURCES].rule_state;state->row=-1;state->axis_page=2;
        memset(&request,0,sizeof(request));strcpy(request.command,"evidence");strcpy(request.projection,"resources");strcpy(request.form,"rule-rows");request.limit=1;SEND();
        CONFIG_CHECK(!w.failed && presentation_token(reply,"selection_token",selection_token,sizeof(selection_token)) && presentation_token(reply,"key",row_key,sizeof(row_key)));
        memset(&request,0,sizeof(request));strcpy(request.command,"select");strcpy(request.projection,"resources");strcpy(request.form,"rule");strcpy(request.id,row_key);strcpy(request.expect,selection_token);SEND();
        CONFIG_CHECK(strstr(reply,"\"selected\":true") && state->row==0 && !state->axis_page && state->marked_cases[state->bindings->bindings[0].case_index]);
        SEND();CONFIG_CHECK(strstr(reply,"viewer.stale_controls")!=NULL);
    }
    {
        char other_case[GD_MAX_PATH]; int other=(root+1)%rules->case_count;
        const GD_ExplorerRuleForm *applied=&panes[GD_EXPLORER_RESOURCES].rule_state->applied;
        if (other!=root) {
            CONFIG_CHECK(snprintf(other_case,sizeof(other_case),"%s#%s",model->records[rules->sources[rules->cases[other].source].record].reference,rules->cases[other].name)<(int)sizeof(other_case));
            STAGE(GD_EXPLORER_RESOURCES,GD_FORM_RULES,"case_name",other_case);
            CONFIG_CHECK(strstr(reply,"\"configured\":true") && !*panes[GD_EXPLORER_RESOURCES].rule_state->pending.participant && !strcmp(applied->case_name,case_key));
        }
        STAGE(GD_EXPLORER_RESOURCES,GD_FORM_RULES,"kind",gd_named_name(GD_NAMED_CONSISTENCY));
        CONFIG_CHECK(!panes[GD_EXPLORER_RESOURCES].rule_state->pending.bindings && panes[GD_EXPLORER_RESOURCES].rule_state->pending.kind==GD_NAMED_CONSISTENCY && applied->bindings);
        STAGE(GD_EXPLORER_RESOURCES,GD_FORM_RULES,"work_limit","1"); CONFIG_CHECK(strstr(reply,"viewer.control") && panes[GD_EXPLORER_RESOURCES].rule_state->pending.work_limit==-1);
    }
    STAGE(GD_EXPLORER_SKILLS,GD_FORM_STRUCTURE,"work_limit","67108865"); CONFIG_CHECK(strstr(reply,"viewer.control") && !panes[GD_EXPLORER_SKILLS].structure_state);
    *fail_at=*calls+1; STAGE(GD_EXPLORER_SKILLS,GD_FORM_STRUCTURE,"kind",gd_structure_question_name(0)); *fail_at=0;
    CONFIG_CHECK(strstr(reply,"viewer.control") && !panes[GD_EXPLORER_SKILLS].structure_state);
    STAGE(GD_EXPLORER_ACTORS,GD_FORM_QUERY,"rank","999"); CONFIG_CHECK(strstr(reply,"viewer.control")!=NULL);
    STAGE(GD_EXPLORER_EFFECTS,GD_FORM_QUERY,"values","ignored"); CONFIG_CHECK(strstr(reply,"viewer.control") && !*panes[GD_EXPLORER_EFFECTS].query_options.values);
    STAGE(GD_EXPLORER_MDA,GD_FORM_STRUCTURE,"work_limit","12junk"); CONFIG_CHECK(strstr(reply,"viewer.control")!=NULL);
    memset(&request,0,sizeof(request)); strcpy(request.command,"configure"); strcpy(request.projection,"actors"); strcpy(request.form,"structure"); strcpy(request.control,"kind"); strcpy(request.value,gd_structure_question_name(0));
    viewer_control_token(&viewer,&c,GD_FORM_STRUCTURE,request.expect,sizeof(request.expect)); panes[active].query_state->applied.accepted=!panes[active].query_state->applied.accepted;
    SEND(); CONFIG_CHECK(strstr(reply,"viewer.stale_controls") && !panes[active].structure_state); panes[active].query_state->applied.accepted=!panes[active].query_state->applied.accepted;
    memset(&request,0,sizeof(request)); strcpy(request.command,"controls"); strcpy(request.form,"query"); strcpy(request.control,"axis"); request.limit=1; request.offset=(unsigned)axis+1;
    v=*calls; SEND(); CONFIG_CHECK(!w.failed && strstr(reply,model->records[axis].reference) && strstr(reply,"\"identity_available\":true")); CONFIG_CHECK(*calls==v);
    memset(&request,0,sizeof(request)); strcpy(request.command,"configure"); strcpy(request.projection,"actors"); strcpy(request.form,"query"); strcpy(request.control,"accepted"); strcpy(request.value,"true");
    viewer_control_token(&viewer,&c,GD_FORM_QUERY,request.expect,sizeof(request.expect)); ++viewer.reload_generation;
    SEND(); CONFIG_CHECK(strstr(reply,"viewer.stale_controls") && !panes[active].query_options.accepted); --viewer.reload_generation;
    panes[active].query_options.literal_values=!panes[active].query_options.literal_values;
    SEND(); CONFIG_CHECK(strstr(reply,"viewer.stale_controls") && !panes[active].query_options.accepted); panes[active].query_options.literal_values=!panes[active].query_options.literal_values;
    memset(&request,0,sizeof(request)); strcpy(request.command,"action"); strcpy(request.projection,"actors"); strcpy(request.action,"query.run"); strcpy(request.expect,"stale-token");
    SEND(); CONFIG_CHECK(strstr(reply,"viewer.stale_controls")!=NULL);
    STAGE(GD_EXPLORER_ACTORS,GD_FORM_STRUCTURE,"kind","similarity-features");CONFIG_CHECK(strstr(reply,"\"configured\":true")!=NULL);
    STAGE(GD_EXPLORER_ACTORS,GD_FORM_STRUCTURE,"actor_scope","applied-variants");CONFIG_CHECK(strstr(reply,"\"configured\":true")!=NULL);
    RUN(GD_EXPLORER_ACTORS,"structure.run",GD_FORM_STRUCTURE);CONFIG_CHECK(strstr(reply,"\"executed\":true")!=NULL);
    CONFIG_CHECK(viewer_result_reassemble(&viewer,GD_EXPLORER_ACTORS,"structure","source",reply));
    CONFIG_CHECK(strstr(reply,"\"actor_query_context\":") && strstr(reply,"\"question\":\"actor-states\"") && !strstr(reply,"\"comparisons\":"));
    memset(&request,0,sizeof(request)); strcpy(request.command,"controls"); strcpy(request.form,"query");
    v=*calls; SEND(); CONFIG_CHECK(!w.failed && strstr(reply,"\"control_token\"") && strstr(reply,"\"choices_closed\"")); CONFIG_CHECK(*calls==v);
    CONFIG_CHECK(!c.project->draft->modified_count && !c.project->draft->history_count);
    result=checks;
cleanup:
    *fail_at=0; igGetIO_Nil()->WantTextInput=false;
    if (panes) {
        for (v=0;v<GD_EXPLORER_VIEWS;++v) { gd_diagnostics_clear(&panes[v].diagnostics); gd_layout_focus_destroy(panes[v].distance_focus); gd_resource_routes_destroy(panes[v].resource_routes); pane_release_layout(a,&panes[v]); }
        a.release(a.context,panes);
    }
#undef CONFIG_CHECK
#undef STAGE
#undef RUN
#undef SEND
    return result;
}
#endif
