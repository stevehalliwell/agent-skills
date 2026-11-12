/* CPU-only adapter checks: no InitWindow, BeginDrawing, OS input or source/user-settings writes. */
#ifndef TEST_EXPLORER_VIEWER_H
#define TEST_EXPLORER_VIEWER_H
#include "test_explorer_viewer_nodes.h"
#include "test_explorer_controls.h"
#include "test_explorer_viewer_feedback.h"
#include "test_sidebar.h"
#include "test_explorer_presentation.h"
#include "test_explorer_configuration.h"
int gd_explorer_viewer_checks(GD_Allocator a, const GD_Collection *collections, int count, int *fail_at, int *calls,int run_deep_checks,int run_layout_research) {
    GD_ExplorerProject *project=gd_explorer_load(a,collections,count);
    GD_ExplorerPane *panes=NULL,*before=NULL; GD_GraphAnalysis *analysis=NULL; char *reply=NULL,*configuration_json=NULL,*presentation_json=NULL;
    GD_ExplorerContext c={0}; ExplorerViewer viewer={0}; GD_ViewerRequest request={0};
    GD_ViewerWriter w; int active=GD_EXPLORER_MATRIX,stale=0,result=2,v,skill=-1,checks=0,control_checks=0,feedback_checks=0,sidebar_checks=0,configuration_checks=0,presentation_checks=0;
    ImGuiContext *imgui=NULL;
#define CHECK(value) do { ++checks; if (!(value)) { fprintf(stderr,"Viewer adapter check failed: %s (line %d)\n",#value,__LINE__); goto cleanup; } } while (0)
#define DISPATCH(command_,projection_,reference_) do { memset(&request,0,sizeof(request)); snprintf(request.command,sizeof(request.command),"%s",command_); snprintf(request.projection,sizeof(request.projection),"%s",projection_); snprintf(request.reference,sizeof(request.reference),"%s",reference_); w=(GD_ViewerWriter){reply,0,GD_VIEWER_REPLY_LIMIT,0}; viewer_dispatch(&viewer,&request,&w); } while (0)
#define ACTION(projection_,action_) do { memset(&request,0,sizeof(request)); strcpy(request.command,"action"); snprintf(request.projection,sizeof(request.projection),"%s",projection_); snprintf(request.action,sizeof(request.action),"%s",action_); w=(GD_ViewerWriter){reply,0,GD_VIEWER_REPLY_LIMIT,0}; viewer_dispatch(&viewer,&request,&w); } while (0)
    CHECK(project && !gd_explorer_error(project) && (project->available&(1u<<active)));
    CHECK(!run_layout_research || run_deep_checks);
    panes=a.allocate(a.context,sizeof(*panes)*GD_EXPLORER_VIEWS);
    if (panes) memset(panes,0,sizeof(*panes)*GD_EXPLORER_VIEWS);
    before=a.allocate(a.context,sizeof(*panes)*GD_EXPLORER_VIEWS);
    analysis=a.allocate(a.context,sizeof(*analysis)); reply=a.allocate(a.context,GD_VIEWER_REPLY_LIMIT); configuration_json=a.allocate(a.context,GD_VIEWER_REPLY_LIMIT);presentation_json=a.allocate(a.context,GD_VIEWER_REPLY_LIMIT);
    CHECK(panes && before && analysis && reply && configuration_json && presentation_json);
    memset(panes,0,sizeof(*panes)*GD_EXPLORER_VIEWS); memset(analysis,0,sizeof(*analysis));
    for (v=0;v<GD_EXPLORER_VIEWS;++v) pane_defaults(&panes[v],(GD_ExplorerView)v);
    imgui=igCreateContext(NULL); CHECK(imgui!=NULL); igGetIO_Nil()->IniFilename=NULL;
    c.project=project; c.pane=&panes[active]; c.analysis=analysis; c.viewport=(Rectangle){0,0,800,600};
    c.pane->layout_choice=GD_MATRIX_GRID_CHOICE; CHECK(pane_prepare(&c)); c.pane->fit=0;
    viewer=(ExplorerViewer){&c,panes,NULL,&active,&stale,1,{0},0}; viewer.collections=collections; viewer.collection_count=count;
    memcpy(before,panes,sizeof(*panes)*GD_EXPLORER_VIEWS);
    v=*calls; DISPATCH("nodes","effects",""); CHECK(!w.failed && strstr(reply,"\"prepared\":false") && strstr(reply,"\"nodes\":[]"));
    CHECK(*calls==v && !memcmp(before,panes,sizeof(*panes)*GD_EXPLORER_VIEWS));
    DISPATCH("actions","actors",""); CHECK(!w.failed && strstr(reply,"query.run") && strstr(reply,"query.display"));
    CHECK(*calls==v && !memcmp(before,panes,sizeof(*panes)*GD_EXPLORER_VIEWS));
    DISPATCH("inspect","",""); CHECK(!w.failed && !strncmp(reply,"{\"ok\":true",10));
    CHECK(!memcmp(before,panes,sizeof(*panes)*GD_EXPLORER_VIEWS));
    CHECK(strstr(reply,"\"selection\":{\"source\":null")!=NULL);
    {
        GD_Model *matrix=gd_explorer_model(project,GD_EXPLORER_MATRIX); const char *reference=matrix->records[0].reference;
        DISPATCH("hint","matrix",reference); CHECK(!viewer.has_hint && viewer.hint_pending);
        CHECK(!memcmp(before,panes,sizeof(*panes)*GD_EXPLORER_VIEWS));
        igGetIO_Nil()->WantTextInput=true; viewer_apply_hint(&viewer); CHECK(!viewer.has_hint && viewer.hint_pending);
        igGetIO_Nil()->WantTextInput=false; viewer_apply_hint(&viewer); CHECK(viewer.has_hint && !viewer.hint_pending && !strcmp(viewer.hint.reference,reference));
        igGetIO_Nil()->WantTextInput=true;
        DISPATCH("navigate","matrix",reference); CHECK(strstr(reply,"viewer.busy")!=NULL);
        DISPATCH("refresh","",""); CHECK(strstr(reply,"viewer.busy")!=NULL);
        CHECK(!memcmp(before,panes,sizeof(*panes)*GD_EXPLORER_VIEWS)); igGetIO_Nil()->WantTextInput=false;
        DISPATCH("navigate","matrix",reference); CHECK(!strncmp(reply,"{\"ok\":true",10));
        CHECK(!strcmp(c.pane->find_reference,reference)); CHECK(c.pane->selection.node==before[active].selection.node);
        CHECK(viewer.has_hint); /* Direct navigation never consumes human hint/selection. */
    }
    memcpy(before,panes,sizeof(*panes)*GD_EXPLORER_VIEWS);
    DISPATCH("navigate","matrix","not/loaded"); CHECK(strstr(reply,"viewer.reference")!=NULL);
    CHECK(!memcmp(before,panes,sizeof(*panes)*GD_EXPLORER_VIEWS));
    DISPATCH("navigate","invalid",""); CHECK(strstr(reply,"viewer.projection")!=NULL);
    CHECK(active==GD_EXPLORER_MATRIX);
    if (project->draft) {
        GD_Model *model=gd_explorer_model(project,GD_EXPLORER_SKILLS);
        for (v=0;v<model->record_count;++v) if (model->records[v].role==GD_SKILL) { skill=v; break; }
        CHECK(skill>=0);
        panes[GD_EXPLORER_CHALLENGES].editor.record=skill;
        snprintf(panes[GD_EXPLORER_CHALLENGES].editor.rank_text,64,"invalid pending rank");
        DISPATCH("navigate","skills",""); CHECK(strstr(reply,"viewer.busy")!=NULL);
        DISPATCH("refresh","",""); CHECK(strstr(reply,"viewer.busy")!=NULL);
        CHECK(project->draft->history_count==0 && project->draft->modified_count==0);
        CHECK(!strcmp(panes[GD_EXPLORER_CHALLENGES].editor.rank_text,"invalid pending rank"));
        DISPATCH("inspect","",""); CHECK(strstr(reply,"\"rank_input_pending\":true")!=NULL);
        panes[GD_EXPLORER_CHALLENGES].editor.record=-1;
        CHECK(gd_draft_rank(project->draft,skill,"999")>0); CHECK(gd_explorer_edited(project));
        DISPATCH("refresh","",""); CHECK(strstr(reply,"viewer.busy")!=NULL && c.project==project);
        DISPATCH("navigate","matrix",""); CHECK(!strncmp(reply,"{\"ok\":true",10));
        CHECK(project->draft->history_count==1 && project->draft->modified_count==1);
        CHECK(gd_draft_undo(project->draft)); CHECK(gd_explorer_edited(project));
        {
            char original[64]; const GD_Record *record=&model->records[skill];
            if (record->has_rank) snprintf(original,sizeof(original),"%lld",(long long)record->rank); else strcpy(original,"null");
            CHECK(gd_draft_rank(project->draft,skill,"999")>0);
            CHECK(gd_draft_rank(project->draft,skill,original)>0);
            CHECK(!project->draft->modified_count && project->draft->history_count==2);
            DISPATCH("refresh","",""); CHECK(strstr(reply,"viewer.busy") && project->draft->history_count==2);
            CHECK(gd_draft_undo(project->draft) && gd_draft_undo(project->draft)); CHECK(gd_explorer_edited(project));
        }
    }
    memcpy(before,panes,sizeof(*panes)*GD_EXPLORER_VIEWS);
    *fail_at=*calls+1;
    DISPATCH("navigate","actors","");
    *fail_at=0;
    CHECK(strstr(reply,"viewer.prepare")!=NULL && active==GD_EXPLORER_MATRIX);
    CHECK(!memcmp(&before[active],&panes[active],sizeof(*panes)));
    for (v=0;v<GD_EXPLORER_VIEWS;++v) if (project->available&(1u<<v)) {
        c.pane=&panes[v]; CHECK(pane_prepare(&c));
        if (c.pane->layout && c.pane->layout->node_count && (v==GD_EXPLORER_SKILLS || v==GD_EXPLORER_RESOURCES || v==GD_EXPLORER_EFFECTS)) c.pane->selection.node=0;
    }
    c.pane=&panes[active];
    snprintf(panes[GD_EXPLORER_ACTORS].query_options.values,sizeof(panes[GD_EXPLORER_ACTORS].query_options.values),"unapplied Ω");
    memcpy(before,panes,sizeof(*panes)*GD_EXPLORER_VIEWS);
    DISPATCH("inspect","",""); CHECK(!w.failed);
    CHECK(!memcmp(before,panes,sizeof(*panes)*GD_EXPLORER_VIEWS));
    CHECK(strstr(reply,"unapplied Ω")!=NULL);
    DISPATCH("navigate","skills",""); CHECK(!strncmp(reply,"{\"ok\":true",10));
    CHECK(!strcmp(panes[GD_EXPLORER_ACTORS].query_options.values,"unapplied Ω"));
    DISPATCH("navigate","matrix",""); CHECK(!strncmp(reply,"{\"ok\":true",10));
    CHECK(!strcmp(panes[GD_EXPLORER_ACTORS].query_options.values,"unapplied Ω"));
    w=(GD_ViewerWriter){reply,0,GD_VIEWER_REPLY_LIMIT,0}; viewer_inspect(&viewer,&w,"selection");
    CHECK(!w.failed && strstr(reply,"\"selection\":") && !strstr(reply,"\"panes\":") && !strstr(reply,"\"snapshots\":"));
    CHECK(explorer_viewer_nodes_checks(&viewer,before,reply,fail_at,calls,&checks));
    CHECK(explorer_control_checks(&c,reply,fail_at,calls,&control_checks));
    {
        GD_ExplorerContext resource=c; GD_ExplorerRuleForm *form; resource.pane=&panes[GD_EXPLORER_RESOURCES];
        form=gd_explorer_form_pending(&resource,GD_FORM_RULES,1); CHECK(form!=NULL);
        CHECK(gd_explorer_rule_mode(form,2)); CHECK(gd_explorer_action_run(&resource,GD_ACTION_RULES_RUN));
    }
    {
        GD_ExplorerQueryState *last=panes[GD_EXPLORER_ACTORS].query_state;
        igGetIO_Nil()->WantTextInput=true; ACTION("actors","query.display"); CHECK(strstr(reply,"viewer.busy")!=NULL); igGetIO_Nil()->WantTextInput=false;
        ACTION("matrix","query.run"); CHECK(strstr(reply,"viewer.action")!=NULL);
        ACTION("actors","not-an-action"); CHECK(strstr(reply,"viewer.action")!=NULL);
        ACTION("actors","query.display"); CHECK(!strncmp(reply,"{\"ok\":true",10) && active==GD_EXPLORER_MATRIX);
        CHECK(panes[GD_EXPLORER_ACTORS].query_state==last && !strcmp(panes[GD_EXPLORER_ACTORS].query_options.values,"unapplied Ω"));
        {
            GD_ExplorerQueryOptions pending=panes[GD_EXPLORER_ACTORS].query_options;
            panes[GD_EXPLORER_ACTORS].query_options.axis=-1;
            ACTION("actors","query.run"); CHECK(strstr(reply,"viewer.action") && panes[GD_EXPLORER_ACTORS].query_state==last);
            panes[GD_EXPLORER_ACTORS].query_options=last->applied;
            ACTION("actors","query.run"); CHECK(!strncmp(reply,"{\"ok\":true",10) && active==GD_EXPLORER_MATRIX && panes[GD_EXPLORER_ACTORS].prepared && panes[GD_EXPLORER_ACTORS].revision==1);
            panes[GD_EXPLORER_ACTORS].query_options=pending;
        }
        ACTION("effects","query.display"); CHECK(!strncmp(reply,"{\"ok\":true",10));
        ACTION("resources","rules.run"); CHECK(!strncmp(reply,"{\"ok\":true",10) && panes[GD_EXPLORER_RESOURCES].rule_state->bindings);
        CHECK(!project->draft->modified_count && !project->draft->history_count);
    }
    if(run_deep_checks){
        GD_ExplorerProject *last=c.project; uint64_t generation=viewer.reload_generation;char noun_a[260],noun_b[260];int noun_cases;
        {GD_ExplorerContext target=c;const GD_NounPair *pair=&c.project->noun_grid->pairs[0];target.pane=&panes[GD_EXPLORER_NOUNS];noun_grid_choose(&target,pair->a,pair->b);CHECK(gd_explorer_presentation_set(&target,"layout","free",target.pane->query_message,sizeof(target.pane->query_message)));noun_cases=pair->count;snprintf(noun_a,sizeof(noun_a),"%s",c.project->rules->model->records[pair->a].reference);snprintf(noun_b,sizeof(noun_b),"%s",c.project->rules->model->records[pair->b].reference);}
        GD_ExplorerQueryOptions actor_options=panes[GD_EXPLORER_ACTORS].query_options,effect_options=panes[GD_EXPLORER_EFFECTS].query_options;
        panes[GD_EXPLORER_ACTORS].query_options=panes[GD_EXPLORER_ACTORS].query_state->applied;
        panes[GD_EXPLORER_EFFECTS].query_options=panes[GD_EXPLORER_EFFECTS].query_state->applied;
        memcpy(before,panes,sizeof(*panes)*GD_EXPLORER_VIEWS);
        *fail_at=*calls+1; DISPATCH("refresh","",""); *fail_at=0;
        CHECK(strstr(reply,"viewer.refresh") && c.project==last && active==GD_EXPLORER_MATRIX && viewer.reload_generation==generation);
        CHECK(!memcmp(before,panes,sizeof(*panes)*GD_EXPLORER_VIEWS));
        panes[GD_EXPLORER_ACTORS].query_options=actor_options; panes[GD_EXPLORER_EFFECTS].query_options=effect_options;
        DISPATCH("refresh","",""); CHECK(strstr(reply,"viewer.refresh") && c.project==last); /* Pending query inputs block native remap. */
        panes[GD_EXPLORER_ACTORS].query_options=panes[GD_EXPLORER_ACTORS].query_state->applied;
        panes[GD_EXPLORER_EFFECTS].query_options=panes[GD_EXPLORER_EFFECTS].query_state->applied;
        /* IPC can own applied analysis before its base canvas is visited. */
        CHECK(panes[GD_EXPLORER_MDA].structure_state && panes[GD_EXPLORER_MDA].structure_state->has_applied && panes[GD_EXPLORER_MDA].layout);
        a.release(a.context,panes[GD_EXPLORER_MDA].layout); panes[GD_EXPLORER_MDA].layout=NULL; panes[GD_EXPLORER_MDA].prepared=0;
        DISPATCH("refresh","",""); project=c.project;
        CHECK(panes[GD_EXPLORER_MDA].layout && panes[GD_EXPLORER_MDA].structure_state && panes[GD_EXPLORER_MDA].structure_state->has_applied && panes[GD_EXPLORER_MDA].structure_state->result);
        CHECK(!strncmp(reply,"{\"ok\":true",10) && c.project!=last && active==GD_EXPLORER_MATRIX);
        CHECK(viewer.reload_generation==generation+1 && !stale && !viewer_hint_current(&viewer));
        CHECK(free_graph(&panes[GD_EXPLORER_NOUNS]) && panes[GD_EXPLORER_NOUNS].layout->edge_count==project->noun_grid->pair_count && panes[GD_EXPLORER_NOUNS].selection.cell>=0 && project->noun_grid->pairs[panes[GD_EXPLORER_NOUNS].selection.cell].count==noun_cases && !strcmp(project->rules->model->records[panes[GD_EXPLORER_NOUNS].grid.noun].reference,noun_a) && !strcmp(project->rules->model->records[panes[GD_EXPLORER_NOUNS].grid.verb].reference,noun_b));
        CHECK(!project->draft->modified_count && !project->draft->history_count);
        panes[GD_EXPLORER_ACTORS].query_options=actor_options; panes[GD_EXPLORER_EFFECTS].query_options=effect_options;
    }
    memcpy(before,panes,sizeof(*panes)*GD_EXPLORER_VIEWS);
    configuration_checks=viewer_configuration_checks(&viewer,fail_at,calls,configuration_json); CHECK(configuration_checks>0);
    if(run_deep_checks){presentation_checks=viewer_presentation_checks(&viewer,fail_at,calls,presentation_json,run_layout_research);CHECK(presentation_checks>0);}
    feedback_checks=viewer_feedback_checks(&viewer); CHECK(feedback_checks>0);
    sidebar_checks=viewer_sidebar_checks(); CHECK(sidebar_checks>0);
    CHECK(!memcmp(before,panes,sizeof(*panes)*GD_EXPLORER_VIEWS) && active==GD_EXPLORER_MATRIX && !project->draft->modified_count);
    DISPATCH("inspect","",""); CHECK(!w.failed && !strstr(reply,"\"body\":"));
    /* Outer Node test parses complete serialized production payloads. */
    printf("{\"ok\":true,\"deep_checks\":%s,\"layout_research\":%s,\"control_catalogue\":%s,\"snapshot\":%s,\"layer_result\":%s",run_deep_checks ? "true":"false",run_layout_research ? "true":"false",configuration_json,reply,run_deep_checks ? presentation_json:"null");
    DISPATCH("nodes","matrix",""); request.limit=2; w=(GD_ViewerWriter){reply,0,GD_VIEWER_REPLY_LIMIT,0}; viewer_dispatch(&viewer,&request,&w);
    CHECK(!w.failed); printf(",\"node_page\":%s",reply);
    w=(GD_ViewerWriter){reply,0,GD_VIEWER_REPLY_LIMIT,0}; viewer_inspect(&viewer,&w,"selection,source"); CHECK(!w.failed);
    printf(",\"source_context\":%s",reply);
    DISPATCH("controls","resources",""); CHECK(!w.failed); printf(",\"resource_controls\":%s",reply);
    DISPATCH("actions","resources",""); CHECK(!w.failed); printf(",\"resource_actions\":%s",reply);
    printf(",\"checks\":%d,\"control_checks\":%d,\"feedback_checks\":%d,\"sidebar_checks\":%d,\"configuration_checks\":%d,\"presentation_checks\":%d}\n",checks,control_checks,feedback_checks,sidebar_checks,configuration_checks,presentation_checks); result=0;
cleanup:
    if (imgui) igDestroyContext(imgui);
    if (panes) for (v=0;v<GD_EXPLORER_VIEWS;++v) {
        gd_diagnostics_clear(&panes[v].diagnostics); gd_layout_focus_destroy(panes[v].distance_focus); gd_resource_routes_destroy(panes[v].resource_routes); pane_release_layout(a,&panes[v]);
    }
    if (panes) a.release(a.context,panes); if (before) a.release(a.context,before);
    if (analysis) a.release(a.context,analysis); if (reply) a.release(a.context,reply); if (configuration_json) a.release(a.context,configuration_json);if(presentation_json)a.release(a.context,presentation_json); gd_explorer_destroy(project);
#undef CHECK
#undef ACTION
#undef DISPATCH
    return result;
}
#endif
