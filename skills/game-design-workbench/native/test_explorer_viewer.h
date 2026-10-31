/* CPU-only adapter checks: no InitWindow, BeginDrawing, OS input or writes. */
#ifndef TEST_EXPLORER_VIEWER_H
#define TEST_EXPLORER_VIEWER_H
#include "test_explorer_viewer_nodes.h"
int gd_explorer_viewer_checks(GD_Allocator a, const GD_Collection *collections, int count, int *fail_at, int *calls) {
    GD_ExplorerProject *project=gd_explorer_load(a,collections,count);
    GD_ExplorerPane *panes=NULL,*before=NULL; GD_GraphAnalysis *analysis=NULL; char *reply=NULL;
    GD_ExplorerContext c={0}; ExplorerViewer viewer={0}; GD_ViewerRequest request={0};
    GD_ViewerWriter w; int active=GD_EXPLORER_MATRIX,stale=0,result=2,v,skill=-1,checks=0;
    ImGuiContext *imgui=NULL;
#define CHECK(value) do { ++checks; if (!(value)) { fprintf(stderr,"Viewer adapter check failed: %s (line %d)\n",#value,__LINE__); goto cleanup; } } while (0)
#define DISPATCH(command_,projection_,reference_) do { memset(&request,0,sizeof(request)); snprintf(request.command,sizeof(request.command),"%s",command_); snprintf(request.projection,sizeof(request.projection),"%s",projection_); snprintf(request.reference,sizeof(request.reference),"%s",reference_); w=(GD_ViewerWriter){reply,0,GD_VIEWER_REPLY_LIMIT,0}; viewer_dispatch(&viewer,&request,&w); } while (0)
    CHECK(project && !gd_explorer_error(project) && (project->available&(1u<<active)));
    panes=a.allocate(a.context,sizeof(*panes)*GD_EXPLORER_VIEWS);
    if (panes) memset(panes,0,sizeof(*panes)*GD_EXPLORER_VIEWS);
    before=a.allocate(a.context,sizeof(*panes)*GD_EXPLORER_VIEWS);
    analysis=a.allocate(a.context,sizeof(*analysis)); reply=a.allocate(a.context,GD_VIEWER_REPLY_LIMIT);
    CHECK(panes && before && analysis && reply);
    memset(panes,0,sizeof(*panes)*GD_EXPLORER_VIEWS); memset(analysis,0,sizeof(*analysis));
    for (v=0;v<GD_EXPLORER_VIEWS;++v) pane_defaults(&panes[v],(GD_ExplorerView)v);
    imgui=igCreateContext(NULL); CHECK(imgui!=NULL); igGetIO_Nil()->IniFilename=NULL;
    c.project=project; c.pane=&panes[active]; c.analysis=analysis; c.viewport=(Rectangle){0,0,800,600};
    c.pane->layout_choice=GD_MATRIX_GRID_CHOICE; CHECK(pane_prepare(&c)); c.pane->fit=0;
    viewer=(ExplorerViewer){&c,panes,NULL,&active,&stale,1,{0},0};
    memcpy(before,panes,sizeof(*panes)*GD_EXPLORER_VIEWS);
    v=*calls; DISPATCH("nodes","effects",""); CHECK(!w.failed && strstr(reply,"\"prepared\":false") && strstr(reply,"\"nodes\":[]"));
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
        CHECK(project->draft->history_count==0 && project->draft->modified_count==0);
        CHECK(!strcmp(panes[GD_EXPLORER_CHALLENGES].editor.rank_text,"invalid pending rank"));
        DISPATCH("inspect","",""); CHECK(strstr(reply,"\"rank_input_pending\":true")!=NULL);
        panes[GD_EXPLORER_CHALLENGES].editor.record=-1;
        CHECK(gd_draft_rank(project->draft,skill,"999")>0); CHECK(gd_explorer_edited(project));
        DISPATCH("navigate","matrix",""); CHECK(!strncmp(reply,"{\"ok\":true",10));
        CHECK(project->draft->history_count==1 && project->draft->modified_count==1);
        CHECK(gd_draft_undo(project->draft)); CHECK(gd_explorer_edited(project));
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
    DISPATCH("inspect","",""); CHECK(!w.failed && !strstr(reply,"\"body\":"));
    /* Outer Node test parses complete serialized production payloads. */
    printf("{\"ok\":true,\"snapshot\":%s",reply);
    DISPATCH("nodes","matrix",""); request.limit=2; w=(GD_ViewerWriter){reply,0,GD_VIEWER_REPLY_LIMIT,0}; viewer_dispatch(&viewer,&request,&w);
    CHECK(!w.failed); printf(",\"node_page\":%s",reply);
    w=(GD_ViewerWriter){reply,0,GD_VIEWER_REPLY_LIMIT,0}; viewer_inspect(&viewer,&w,"selection,source"); CHECK(!w.failed);
    printf(",\"source_context\":%s,\"checks\":%d}\n",reply,checks); result=0;
cleanup:
    if (imgui) igDestroyContext(imgui);
    if (panes) for (v=0;v<GD_EXPLORER_VIEWS;++v) {
        gd_diagnostics_clear(&panes[v].diagnostics); gd_layout_focus_destroy(panes[v].distance_focus); gd_resource_routes_destroy(panes[v].resource_routes); pane_release_layout(a,&panes[v]);
    }
    if (panes) a.release(a.context,panes); if (before) a.release(a.context,before);
    if (analysis) a.release(a.context,analysis); if (reply) a.release(a.context,reply); gd_explorer_destroy(project);
#undef CHECK
#undef DISPATCH
    return result;
}
#endif
