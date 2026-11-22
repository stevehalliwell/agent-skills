/* Rendered widgets exercise the same state storage and pager path as Explorer. */
static void inspector_test_draw(ExplorerViewer *viewer,int *page) {
    GD_ExplorerContext *c=viewer->context;
    igNewFrame();igSetNextWindowSize((ImVec2_c){560,480},ImGuiCond_Always);
    gd_inspector_survey_begin(viewer->inspector_survey,*viewer->active);
    if(c->inspector->open){
        igBegin("Inspector snapshot test",NULL,ImGuiWindowFlags_NoSavedSettings);
        if(sidebar_section(c->sidebar,GD_SIDEBAR_ANALYSIS)){
            if(igTreeNode_Str("Nested evidence")){
                if(igTreeNode_Str("Deeper evidence"))igTreePop();
                gd_explorer_evidence_pager("Evidence list",page,40,10);
                if(igBeginChild_Str("Evidence scroll",(ImVec2_c){250,100},0,0))for(int i=0;i<40;++i)igText("Row %d",i);
                igEndChild();igTreePop();
            }
        }
        igEnd();
    }
    gd_inspector_survey_end();viewer_inspector_seal(viewer);igEndFrame();
}
static int inspector_snapshot_checks(ExplorerViewer *viewer,GD_Allocator a,char *reply) {
    GD_ExplorerContext *c=viewer->context;GD_ExplorerInspector shell={.open=true,.viewport_width=1100};GD_Sidebar sidebar={0};
    GD_InspectorSurvey *s=a.allocate(a.context,sizeof(*s));ImGuiContext *prior=igGetCurrentContext(),*ctx=NULL;
    GD_ExplorerInspector *old_shell=c->inspector;GD_Sidebar *old_sidebar=c->sidebar;int page=0,ok=0;GD_ViewerRequest r={0};GD_ViewerWriter w;
    char nested[40],scroll[40],pager[40],token[64],stale[64];
#define INS_CHECK(x) do{if(!(x)){fprintf(stderr,"Inspector snapshot check failed: %s (%d)\n",#x,__LINE__);goto done;}}while(0)
#define INS_SEND() do{w=(GD_ViewerWriter){reply,0,GD_VIEWER_REPLY_LIMIT,0};viewer_dispatch(viewer,&r,&w);INS_CHECK(!w.failed);}while(0)
    INS_CHECK(s);memset(s,0,sizeof(*s));ctx=igCreateContext(NULL);INS_CHECK(ctx);
    {ImGuiIO *io=igGetIO_Nil();io->DisplaySize=(ImVec2_c){800,600};io->DeltaTime=1.0f/60.0f;io->IniFilename=NULL;io->BackendFlags|=ImGuiBackendFlags_RendererHasTextures;}
    viewer->inspector_survey=s;c->inspector=&shell;c->sidebar=&sidebar;sidebar.open=1u<<GD_SIDEBAR_ANALYSIS;
    inspector_test_draw(viewer,&page);
    INS_CHECK(s->count==2 && s->items[0].kind==GD_INSPECTOR_ROOT && s->items[1].kind==GD_INSPECTOR_FOLD && !s->items[1].value);
    viewer_inspector_key(&s->items[1],nested,sizeof(nested));
    INS_CHECK(viewer_inspector_token(viewer,token,sizeof(token)));strcpy(stale,token);
    strcpy(r.command,"inspector");r.limit=1;INS_SEND();INS_CHECK(strstr(reply,"\"next_offset\":1") && !strstr(reply,nested));
    strcpy(r.expect,token);r.offset=1;INS_SEND();INS_CHECK(strstr(reply,nested) && strstr(reply,"\"next_offset\":null"));
    strcpy(r.command,"inspector-patch");r.offset=r.limit=0;
    strcpy(r.value,"{\"width\":500,\"folds\":{\"not-rendered\":true}}");INS_SEND();INS_CHECK(strstr(reply,"viewer.inspector_patch") && !shell.preferred_width);
    snprintf(r.value,sizeof(r.value),"{\"width\":500,\"folds\":{\"%s\":true}}",nested);INS_SEND();INS_CHECK(strstr(reply,"\"applied\":2") && shell.preferred_width==500);
    inspector_test_draw(viewer,&page);inspector_test_draw(viewer,&page);inspector_test_draw(viewer,&page);
    INS_CHECK(s->count==5 && s->items[1].value && !s->items[2].value && s->items[3].kind==GD_INSPECTOR_PAGE && s->items[4].kind==GD_INSPECTOR_CHILD && s->items[4].maximum>=30);
    viewer_inspector_key(&s->items[3],pager,sizeof(pager));viewer_inspector_key(&s->items[4],scroll,sizeof(scroll));
    strcpy(r.expect,stale);strcpy(r.value,"{\"open\":false}");INS_SEND();INS_CHECK(strstr(reply,"viewer.stale_inspector") && shell.open);
    INS_CHECK(viewer_inspector_token(viewer,r.expect,sizeof(r.expect)));
    igGetIO_Nil()->WantTextInput=true;INS_SEND();INS_CHECK(strstr(reply,"viewer.busy") && shell.open);
    igGetIO_Nil()->WantTextInput=false;
    snprintf(r.value,sizeof(r.value),"{\"open\":false,\"scrolls\":{\"%s\":%d}}",scroll,s->items[4].maximum+1);
    INS_SEND();INS_CHECK(strstr(reply,"viewer.inspector_patch") && shell.open && s->ready);
    snprintf(r.value,sizeof(r.value),"{\"scrolls\":{\"%s\":30}}",scroll);INS_SEND();INS_CHECK(strstr(reply,"\"applied\":1"));
    inspector_test_draw(viewer,&page);INS_CHECK(s->items[4].value==30 && !page);
    INS_CHECK(viewer_inspector_token(viewer,r.expect,sizeof(r.expect)));
    snprintf(r.value,sizeof(r.value),"{\"pages\":{\"%s\":2}}",pager);INS_SEND();INS_CHECK(strstr(reply,"\"queued\":1") && !page);
    inspector_test_draw(viewer,&page);INS_CHECK(page==2 && s->items[3].value==2 && s->page_count==0);
    INS_CHECK(viewer_inspector_token(viewer,r.expect,sizeof(r.expect)));
    snprintf(r.value,sizeof(r.value),"{\"folds\":{\"%s\":false},\"open\":false}",nested);INS_SEND();INS_CHECK(strstr(reply,"\"applied\":2") && !shell.open);
    inspector_test_draw(viewer,&page);INS_CHECK(s->count==0 && viewer_inspector_token(viewer,r.expect,sizeof(r.expect)));
    strcpy(r.value,"{\"open\":true}");INS_SEND();INS_CHECK(strstr(reply,"\"applied\":1"));
    inspector_test_draw(viewer,&page);INS_CHECK(s->count==2 && !s->items[1].value && page==2);
    ok=1;
done:
    viewer->inspector_survey=NULL;c->inspector=old_shell;c->sidebar=old_sidebar;
    if(ctx)igDestroyContext(ctx);igSetCurrentContext(prior);if(s)a.release(a.context,s);return ok;
#undef INS_SEND
#undef INS_CHECK
}
