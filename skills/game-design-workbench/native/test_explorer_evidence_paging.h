/* Bounded, source-read-only paging checks; no GUI window or live viewer. */
#ifndef TEST_EXPLORER_EVIDENCE_PAGING_H
#define TEST_EXPLORER_EVIDENCE_PAGING_H
#include <limits.h>
static int paging_integer(const char *reply,const char *name) {
    char key[64];const char *value;snprintf(key,sizeof(key),"\"%s\":",name);value=strstr(reply,key);
    return value ? (int)strtol(value+strlen(key),NULL,10):-1;
}
static int explorer_evidence_paging_checks(GD_ExplorerContext *c,GD_Layout *saved,const GD_Collection *collections,int count,int *fail_at,int *calls) {
    GD_Allocator allocator=c->project->allocator;char *reply=allocator.allocate(allocator.context,GD_VIEWER_REPLY_LIMIT);int result=0,checks=0,i,j,ordinal,allocations;
    GD_ExplorerPane *p;ExplorerViewer viewer={0};GD_ViewerRequest request={0};GD_ViewerWriter writer;char message[256],expected[80];
#define PAGE_CHECK(value) do{++checks;if(!(value)){fprintf(stderr,"Evidence paging failed: line %d: %s\n",__LINE__,#value);goto cleanup;}}while(0)
    PAGE_CHECK(reply);
    {int totals[]={0,1,16,17,63,64,65,128,129,INT_MAX},n,size,page;
        for(n=0;n<(int)(sizeof(totals)/sizeof(*totals));++n)for(size=16;size<=64;size*=4)for(page=-1;page<=130;++page){
            GD_EvidencePage range=gd_evidence_page(totals[n],size,page);
            PAGE_CHECK(range.first>=0 && range.end>=range.first && range.end<=range.total && range.end-range.first<=size);
            PAGE_CHECK(range.page>=0 && (range.pages ? range.page<range.pages && range.first<range.total:!range.first && !range.end));
        }
    }
    {int sizes[]={16,64},n;for(n=0;n<2;++n){GD_EvidencePage range=gd_evidence_page(INT_MAX,sizes[n],INT_MAX);PAGE_CHECK(range.end==INT_MAX && range.page==range.pages-1 && range.end-range.first<=sizes[n]);}}
    viewer.context=c;viewer.panes=c->panes;viewer.active=c->active_view;
    p=c->pane=&c->panes[GD_EXPLORER_NOUNS];*c->active_view=p->view;PAGE_CHECK(pane_prepare(c));
    {
        GD_ExplorerContext shell=*c;GD_ExplorerInspector inspector={.open=true,.viewport_width=1500};GD_Sidebar sidebar={0};
        GD_ExplorerPresentation current;GD_Camera saved_camera=p->camera;int saved_fit=p->fit;bool saved_override=p->focus_camera_override;
        shell.inspector=&inspector;shell.sidebar=&sidebar;
        PAGE_CHECK(gd_explorer_presentation_set(&shell,"camera_pose","125.5,-64,0.8",message,sizeof(message)));
        PAGE_CHECK(!gd_explorer_presentation_set(&shell,"camera_pose","1,2,nan",message,sizeof(message)) && p->camera.target.x==125.5f && p->camera.target.y==-64 && p->camera.zoom==0.8f && !p->fit);
        PAGE_CHECK(gd_explorer_presentation_set(&shell,"inspector_width","550",message,sizeof(message)) && inspector_pixels(&inspector)==550);
        PAGE_CHECK(!gd_explorer_presentation_set(&shell,"inspector_width","1245",message,sizeof(message)) && !gd_explorer_presentation_set(&shell,"inspector_width","309",message,sizeof(message)));
        PAGE_CHECK(gd_explorer_presentation_set(&shell,"inspector_open","false",message,sizeof(message)) && !inspector_pixels(&inspector));
        PAGE_CHECK(gd_explorer_presentation_set(&shell,"inspector_selection","true",message,sizeof(message)) && sidebar.open==(1u<<GD_SIDEBAR_SELECTION));
        gd_explorer_presentation_read(&shell,&current);
        PAGE_CHECK(!current.inspector_open && current.inspector_width==550 && !current.inspector_pixels && current.inspector_sections[GD_SIDEBAR_SELECTION]);
        p->camera=saved_camera;p->fit=saved_fit;p->focus_camera_override=saved_override;
    }
    strcpy(request.command,"evidence");strcpy(request.projection,"noun-interactions");strcpy(request.form,"noun-cases");request.limit=1;
    for(i=0;i<c->project->noun_grid->pair_count;++i){GD_Camera camera;GD_ExplorerSelection selection;GD_OverviewControls overview;
        p->selection=(GD_ExplorerSelection){i,-1,0,-1,-1};camera=p->camera;selection=p->selection;overview=p->overview;ordinal=0;
        for(j=0;j<c->project->rules->case_count;++j)if(gd_noun_grid_case(c->project->noun_grid,i,j)){
            request.offset=(unsigned)ordinal;allocations=*calls;writer=(GD_ViewerWriter){reply,0,GD_VIEWER_REPLY_LIMIT,0};viewer_dispatch(&viewer,&request,&writer);
            PAGE_CHECK(!writer.failed && *calls==allocations && paging_integer(reply,"total_rows")==c->project->noun_grid->pairs[i].count && paging_integer(reply,"returned_rows")==1);
            PAGE_CHECK(paging_integer(reply,"source_slot")==j && paging_integer(reply,"offset")==ordinal && !strstr(reply,"total_slots"));
            {char token[64];viewer_control_token(&viewer,c,GD_FORM_PRESENTATION,token,sizeof(token));viewer_case_key(p,9,j,token,expected,sizeof(expected));PAGE_CHECK(strstr(reply,expected));}++ordinal;
            if(ordinal<c->project->noun_grid->pairs[i].count)PAGE_CHECK(paging_integer(reply,"next_offset")==ordinal);else PAGE_CHECK(strstr(reply,"\"next_offset\":null"));
        }
        PAGE_CHECK(ordinal==c->project->noun_grid->pairs[i].count && !memcmp(&camera,&p->camera,sizeof(camera)) && !memcmp(&selection,&p->selection,sizeof(selection)) && !memcmp(&overview,&p->overview,sizeof(overview)));
        PAGE_CHECK(gd_explorer_presentation_maximum(c,"noun_case_page")==gd_evidence_page(ordinal,64,0).pages-1);
        request.offset=(unsigned)c->project->rules->case_count;writer=(GD_ViewerWriter){reply,0,GD_VIEWER_REPLY_LIMIT,0};viewer_dispatch(&viewer,&request,&writer);
        PAGE_CHECK(!writer.failed && paging_integer(reply,"returned_rows")==0 && strstr(reply,"\"rows\":[]") && strstr(reply,"\"next_offset\":null"));
    }
    {
        GD_ViewerRequest select=request;int pair=0,slot=-1,page;char token[64];
        for(i=1;i<c->project->noun_grid->pair_count;++i)if(c->project->noun_grid->pairs[i].count>c->project->noun_grid->pairs[pair].count)pair=i;
        p->selection.cell=pair;
        for(i=0;i<c->project->rules->case_count;++i)if(gd_noun_grid_case(c->project->noun_grid,pair,i))slot=i;
        PAGE_CHECK(slot>=0);page=(c->project->noun_grid->pairs[pair].count-1)/GD_EVIDENCE_PAGE_ROWS;
        strcpy(select.command,"select");strcpy(select.form,"noun.case");
        viewer_control_token(&viewer,c,GD_FORM_PRESENTATION,select.expect,sizeof(select.expect));
        viewer_case_key(p,9,slot,select.expect,select.id,sizeof(select.id));
        p->selection.cell=-1;writer=(GD_ViewerWriter){reply,0,GD_VIEWER_REPLY_LIMIT,0};viewer_dispatch(&viewer,&select,&writer);
        PAGE_CHECK(strstr(reply,"viewer.stale_controls") && !p->reveal_case);
        viewer_control_token(&viewer,c,GD_FORM_PRESENTATION,token,sizeof(token));strcpy(select.expect,token);
        writer=(GD_ViewerWriter){reply,0,GD_VIEWER_REPLY_LIMIT,0};viewer_dispatch(&viewer,&select,&writer);
        PAGE_CHECK(strstr(reply,"viewer.selection") && !p->reveal_case);
        p->selection.cell=pair;viewer_control_token(&viewer,c,GD_FORM_PRESENTATION,select.expect,sizeof(select.expect));
        writer=(GD_ViewerWriter){reply,0,GD_VIEWER_REPLY_LIMIT,0};viewer_dispatch(&viewer,&select,&writer);
        PAGE_CHECK(strstr(reply,"\"selected\":true") && p->noun_case_page==page && p->reveal_case==slot+1 && p->reveal_case_edge==pair && p->selection.cell==pair);
        {
            GD_ViewerRequest child=request,new_select=select;char participant_key[64];const GD_RuleCase *item=&c->project->rules->cases[slot];
            viewer_control_token(&viewer,c,GD_FORM_PRESENTATION,token,sizeof(token));viewer_case_key(p,9,slot,token,select.id,sizeof(select.id));
            strcpy(child.form,"case-index");child.offset=(unsigned)(c->project->noun_grid->pairs[pair].count-1);
            writer=(GD_ViewerWriter){reply,0,GD_VIEWER_REPLY_LIMIT,0};viewer_dispatch(&viewer,&child,&writer);
            PAGE_CHECK(strstr(reply,select.id) && !strstr(reply,"\"participants\":[") && paging_integer(reply,"participant_count")==item->participant_count);
            PAGE_CHECK(item->participant_count>0);strcpy(child.form,"case-participants");strcpy(child.reference,select.id);child.offset=0;child.limit=1;
            viewer_child_key(child.reference,token,0,participant_key,sizeof(participant_key));
            writer=(GD_ViewerWriter){reply,0,GD_VIEWER_REPLY_LIMIT,0};viewer_dispatch(&viewer,&child,&writer);
            PAGE_CHECK(strstr(reply,participant_key) && paging_integer(reply,"returned_rows")==1 && strstr(reply,"\"other_axes\":"));
            strcpy(new_select.form,"case.participant");strcpy(new_select.reference,child.reference);strcpy(new_select.expect,token);strcpy(new_select.id,participant_key);
            writer=(GD_ViewerWriter){reply,0,GD_VIEWER_REPLY_LIMIT,0};viewer_dispatch(&viewer,&new_select,&writer);
            PAGE_CHECK(strstr(reply,"\"selected\":true") && p->reveal_case==slot+1 && p->reveal_participant==item->first_participant+1);
        }
        p->reveal_case=p->reveal_participant=0;
    }
    /* Overview's visible source lists must not reuse another pair's cursor
       after a filter changes source-list order. Mark scratch, not authored data. */
    PAGE_CHECK(gd_explorer_presentation_set(c,"layout","free",message,sizeof(message)) && p->layout->edge_count>1);
    {int *first,*second,primary=p->noun_case_page;GD_OverviewControls overview=p->overview;
        memset(c->analysis->overview.selected_edges,0,sizeof(c->analysis->overview.selected_edges));c->analysis->overview.selected_edges[0]=c->analysis->overview.selected_edges[1]=1;
        first=noun_source_page(c,0);second=noun_source_page(c,1);PAGE_CHECK(first && second && first!=second && first!=&p->noun_case_page);*first=1;*second=2;
        c->analysis->overview.selected_edges[0]=0;PAGE_CHECK(noun_source_page(c,1)==first && !*first && p->noun_case_page==primary);
        ++p->overview.row;PAGE_CHECK(noun_source_page(c,1) && !*second);p->overview=overview;
        memset(c->analysis->overview.selected_edges,0,sizeof(c->analysis->overview.selected_edges));
    }
    /* Empty filtered data is success, not a fabricated source row. */
    p->selection.cell=-1;request.offset=0;writer=(GD_ViewerWriter){reply,0,GD_VIEWER_REPLY_LIMIT,0};viewer_dispatch(&viewer,&request,&writer);
    PAGE_CHECK(!writer.failed && paging_integer(reply,"total_rows")==0 && paging_integer(reply,"returned_rows")==0);
    /* Known evidence filters source slots too; matching offsets must not skip them. */
    p=c->pane=&c->panes[GD_EXPLORER_SKILLS];*c->active_view=p->view;PAGE_CHECK(pane_prepare(c));
    {const GD_Model *model=gd_explorer_model(c->project,p->view);int known[2],found=0;
        for(i=model->record_count-1;i>=0 && found<2;--i)if(model->records[i].role==GD_SKILL){known[found++]=i;p->known[i]=1;}
        PAGE_CHECK(found==2);strcpy(request.projection,"skills");strcpy(request.form,"known");
        for(i=0;i<2;++i){request.offset=(unsigned)i;writer=(GD_ViewerWriter){reply,0,GD_VIEWER_REPLY_LIMIT,0};viewer_dispatch(&viewer,&request,&writer);PAGE_CHECK(!writer.failed && paging_integer(reply,"total_rows")==2 && paging_integer(reply,"source_slot")==known[1-i]);}
        p->known[known[0]]=p->known[known[1]]=0;
    }
    {GD_NounGrid *grid=c->project->noun_grid;strcpy(request.form,"noun-cases");request.offset=0;c->project->noun_grid=NULL;
        writer=(GD_ViewerWriter){reply,0,GD_VIEWER_REPLY_LIMIT,0};viewer_dispatch(&viewer,&request,&writer);c->project->noun_grid=grid;
        PAGE_CHECK(!writer.failed && paging_integer(reply,"total_rows")==0 && strstr(reply,"\"producer_omissions\":null"));
    }
    /* Pane-owned rule pages outside Resources preserve exact row inspection. */
    p=c->pane=&c->panes[GD_EXPLORER_ACTORS];*c->active_view=p->view;PAGE_CHECK(pane_prepare(c));
    PAGE_CHECK(gd_explorer_control_set(c,GD_FORM_RULES,"bindings","true",message,sizeof(message)) && gd_explorer_action_run(c,GD_ACTION_RULES_RUN));
    {GD_ExplorerRuleState *state=p->rule_state;GD_ExplorerQueryState *query=p->query_state;GD_Camera camera=p->camera;GD_ExplorerSelection selection=p->selection;GD_OverviewControls overview=p->overview;
        GD_Query *bindings=state->bindings;uint64_t draft=c->project->draft ? viewer_control_hash(1,c->project->draft,sizeof(*c->project->draft)):0;
        uint64_t pending=viewer_form_hash(1,GD_FORM_RULES,&state->pending),applied=viewer_form_hash(1,GD_FORM_RULES,&state->applied),canvas=viewer_form_hash(1,GD_FORM_QUERY,&p->query_options);int maximum=gd_explorer_presentation_maximum(c,"rules_page");allocations=*calls;
        memcpy(saved,p->layout,sizeof(*saved));PAGE_CHECK(gd_explorer_presentation_editable(c,"rules_page") && gd_explorer_rules_row_count(state)>0);gd_explorer_rules_choose(state,0,-1);
        snprintf(expected,sizeof(expected),"%d",maximum);PAGE_CHECK(gd_explorer_presentation_set(c,"rules_page",expected,message,sizeof(message)) && state->page==maximum && state->row==0);
        snprintf(expected,sizeof(expected),"%d",maximum+1);PAGE_CHECK(!gd_explorer_presentation_set(c,"rules_page",expected,message,sizeof(message)) && state->page==maximum);
        PAGE_CHECK(state==p->rule_state && query==p->query_state && !memcmp(&camera,&p->camera,sizeof(camera)) && !memcmp(&selection,&p->selection,sizeof(selection)) && !memcmp(&overview,&p->overview,sizeof(overview)) && !memcmp(saved,p->layout,sizeof(*saved)));
        PAGE_CHECK(pending==viewer_form_hash(1,GD_FORM_RULES,&state->pending) && applied==viewer_form_hash(1,GD_FORM_RULES,&state->applied));
        PAGE_CHECK(*calls==allocations && bindings==state->bindings && canvas==viewer_form_hash(1,GD_FORM_QUERY,&p->query_options) && (!c->project->draft || draft==viewer_control_hash(1,c->project->draft,sizeof(*c->project->draft))));
        PAGE_CHECK(gd_explorer_presentation_editable(c,"rules_axis_page") && gd_explorer_presentation_set(c,"rules_axis_page","0",message,sizeof(message)));
    }
    PAGE_CHECK(gd_explorer_query_overview(c,GD_ACTOR_USAGE));
    strcpy(request.projection,"actors");strcpy(request.form,"actor-cases");request.offset=0;
    if(!p->layout->edge_count){writer=(GD_ViewerWriter){reply,0,GD_VIEWER_REPLY_LIMIT,0};viewer_dispatch(&viewer,&request,&writer);PAGE_CHECK(!writer.failed && paging_integer(reply,"total_rows")==0);}
    for(i=0;i<p->layout->edge_count;++i)if(p->query_state->scene->edges[i].kind!=GD_QUERY_EDGE_AXIS){int total=gd_explorer_query_case_count(c,i),page;
        p->selection=(GD_ExplorerSelection){i,-1,0,-1,-1};PAGE_CHECK(gd_explorer_presentation_editable(c,"actor_case_page"));
        request.offset=0;writer=(GD_ViewerWriter){reply,0,GD_VIEWER_REPLY_LIMIT,0};viewer_dispatch(&viewer,&request,&writer);PAGE_CHECK(!writer.failed && paging_integer(reply,"total_rows")==total && paging_integer(reply,"returned_rows")==!!total);
        if(total){
            GD_ViewerRequest select=request;
            for(j=0;j<c->project->rules->case_count && !gd_query_scene_overview_case(p->query_state->scene,i,j);++j){}
            PAGE_CHECK(paging_integer(reply,"source_slot")==j);
            strcpy(select.command,"select");strcpy(select.form,"actor.case");
            viewer_control_token(&viewer,c,GD_FORM_PRESENTATION,select.expect,sizeof(select.expect));viewer_case_key(p,10,j,select.expect,select.id,sizeof(select.id));
            PAGE_CHECK(strstr(reply,select.id));
            writer=(GD_ViewerWriter){reply,0,GD_VIEWER_REPLY_LIMIT,0};viewer_dispatch(&viewer,&select,&writer);
            if(!strstr(reply,"\"selected\":true"))fprintf(stderr,"Actor case selection failed: %s\n",reply);
            PAGE_CHECK(strstr(reply,"\"selected\":true") && p->reveal_case==j+1 && p->reveal_case_edge==i && p->selection.cell==i && !p->query_state->case_pages[i]);
            {
                GD_ViewerRequest participant=select;const GD_RuleCase *item=&c->project->rules->cases[j];
                if(item->participant_count){
                    strcpy(participant.form,"case.participant");strcpy(participant.reference,select.id);
                    viewer_child_key(participant.reference,participant.expect,0,participant.id,sizeof(participant.id));
                    p->query_state->case_participant_pages[j]=1;
                    writer=(GD_ViewerWriter){reply,0,GD_VIEWER_REPLY_LIMIT,0};viewer_dispatch(&viewer,&participant,&writer);
                    PAGE_CHECK(strstr(reply,"\"selected\":true") && p->reveal_participant==item->first_participant+1 && !p->query_state->case_participant_pages[j]);
                }
            }
            p->reveal_case=p->reveal_participant=0;
        }

        for(page=0;page<gd_evidence_page(total,64,0).pages;++page){snprintf(expected,sizeof(expected),"%d",page);PAGE_CHECK(gd_explorer_presentation_set(c,"actor_case_page",expected,message,sizeof(message)) && p->query_state->case_pages[i]==page);}
    }
    if(p->layout->edge_count){GD_QueryScene *scene=p->query_state->scene;GD_Camera camera=p->camera;GD_ExplorerSelection selection=p->selection;int changed;
        p->query_state->case_pages[0]=1;*fail_at=*calls+1;changed=gd_explorer_query_overview(c,GD_ACTOR_USAGE);*fail_at=0;
        PAGE_CHECK(!changed && p->query_state->scene==scene && p->query_state->case_pages[0]==1 && !memcmp(&camera,&p->camera,sizeof(camera)) && !memcmp(&selection,&p->selection,sizeof(selection)));
        PAGE_CHECK(gd_explorer_query_overview(c,GD_ACTOR_USAGE) && !p->query_state->case_pages[0]);
    }
    /* Synthetic cardinality exercises controls beyond64 without inventing
       authored contributor paths; restore count before any row serialization. */
    p=c->pane=&c->panes[GD_EXPLORER_EFFECTS];*c->active_view=p->view;PAGE_CHECK(pane_prepare(c) && p->query_state->selected);
    {GD_ExplorerQueryState *state=p->query_state;GD_EffectQuery *selected=state->selected;int retained=selected->count,page=state->contributor_page;GD_Camera camera=p->camera;GD_ExplorerSelection selection=p->selection;
        {int row,path[GD_MAX_RULE_EXPANSION_DEPTH];char token[64],path_key[64],step_key[64];
            for(row=0;row<retained;++row)if(gd_effect_contributor_path(selected,row,path,GD_MAX_RULE_EXPANSION_DEPTH)>0)break;
            PAGE_CHECK(row<retained);
            strcpy(request.command,"evidence");strcpy(request.projection,"effects");strcpy(request.form,"effect-paths");request.offset=(unsigned)row;request.limit=1;
            viewer_control_token(&viewer,c,GD_FORM_PRESENTATION,token,sizeof(token));viewer_child_key("effect-paths",token,row,path_key,sizeof(path_key));
            writer=(GD_ViewerWriter){reply,0,GD_VIEWER_REPLY_LIMIT,0};viewer_dispatch(&viewer,&request,&writer);PAGE_CHECK(strstr(reply,path_key) && paging_integer(reply,"steps")>0);
            strcpy(request.form,"effect-path-steps");strcpy(request.reference,path_key);request.offset=0;
            viewer_child_key(path_key,token,0,step_key,sizeof(step_key));writer=(GD_ViewerWriter){reply,0,GD_VIEWER_REPLY_LIMIT,0};viewer_dispatch(&viewer,&request,&writer);
            PAGE_CHECK(strstr(reply,step_key) && strstr(reply,"\"case\":"));
            strcpy(request.command,"select");strcpy(request.form,"effect.path-step");strcpy(request.id,step_key);strcpy(request.expect,token);
            writer=(GD_ViewerWriter){reply,0,GD_VIEWER_REPLY_LIMIT,0};viewer_dispatch(&viewer,&request,&writer);
            PAGE_CHECK(strstr(reply,"\"selected\":true") && state->reveal_contributor==row+1 && state->reveal_contributor_step==1 && state->selected==selected);
            state->contributor_page=page;state->reveal_contributor=state->reveal_contributor_step=0;
        }
        selected->count=129;
        {int maximum=gd_explorer_presentation_maximum(c,"contributor_page"),valid=gd_explorer_presentation_set(c,"contributor_page","2",message,sizeof(message));
            int changed=state->contributor_page==2,invalid=gd_explorer_presentation_set(c,"contributor_page","3",message,sizeof(message));selected->count=retained;state->contributor_page=page;
            PAGE_CHECK(maximum==2 && valid && changed && !invalid && state==p->query_state && selected==state->selected && !memcmp(&camera,&p->camera,sizeof(camera)) && !memcmp(&selection,&p->selection,sizeof(selection)));
        }
    }
    p=c->pane=&c->panes[GD_EXPLORER_ACTORS];*c->active_view=p->view;
    /* Explicitly restore test-only draft edits before testing clean refresh.
       These are this process's synthetic UX edits, never live viewer buffers. */
    if(c->project->draft){for(i=0;i<GD_MAX_DRAFT_EDITS && c->project->draft->history_count;++i)PAGE_CHECK(gd_explorer_action_run(c,GD_ACTION_UNDO));
        if(c->project->draft->redo_count)PAGE_CHECK(gd_explorer_action_run(c,GD_ACTION_DISCARD_REDO));
        PAGE_CHECK(!c->project->draft->modified_count && !c->project->draft->history_count && !c->project->draft->redo_count);
    }
    /* Focused reload regression: query panes must remap their rule owners too. */
    {int rows=gd_explorer_rules_row_count(p->rule_state);PAGE_CHECK(reload_project(c,c->panes,collections,count,message,sizeof(message)));
        p=&c->panes[GD_EXPLORER_ACTORS];PAGE_CHECK(c->pane==p && *c->active_view==GD_EXPLORER_ACTORS && p->rule_state && p->rule_state->bindings && gd_explorer_rules_row_count(p->rule_state)==rows && gd_explorer_form_equal(GD_FORM_RULES,&p->rule_state->pending,&p->rule_state->applied));
    }
    result=1;
cleanup:
    *fail_at=0;
    if(reply)allocator.release(allocator.context,reply);
    fprintf(stderr,"Evidence paging checks: %d; query edge-page scratch %llu bytes\n",checks,(unsigned long long)sizeof(((GD_ExplorerQueryState *)0)->case_pages));
#undef PAGE_CHECK
    return result;
}
#endif
