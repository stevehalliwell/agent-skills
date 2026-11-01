/* CPU-only production IPC investigation, with independently owned panes/cache. */
#ifndef TEST_EXPLORER_PRESENTATION_H
#define TEST_EXPLORER_PRESENTATION_H
static int presentation_decode(const char *reply,char *out,size_t capacity) {
    const char *p=strstr(reply,"\"data\":\"");unsigned value=0;int bits=0;size_t count=0;
    if(!p)return -1;p+=8;
    for(;*p && *p!='"';++p){const char *alphabet="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/",*digit;if(*p=='=')break;digit=strchr(alphabet,*p);if(!digit)return -1;value=(value<<6)|(unsigned)(digit-alphabet);bits+=6;if(bits>=8){bits-=8;if(count+1>=capacity)return -1;out[count++]=(char)(value>>bits);}}
    out[count]=0;return (int)count;
}
static int presentation_token(const char *reply,const char *name,char *out,size_t capacity) {
    char key[80];const char *begin,*end;snprintf(key,sizeof(key),"\"%s\":\"",name);begin=strstr(reply,key);if(!begin)return 0;begin+=strlen(key);end=strchr(begin,'"');if(!end || (size_t)(end-begin)>=capacity)return 0;memcpy(out,begin,(size_t)(end-begin));out[end-begin]=0;return 1;
}
static int viewer_result_reassemble(ExplorerViewer *viewer,GD_ExplorerView view,const char *form,const char *fields,char *reply) {
    GD_Allocator a=viewer->context->project->allocator;char *assembled=a.allocate(a.context,GD_VIEWER_REPLY_LIMIT),chunk[65537];GD_ViewerRequest request={0};GD_ViewerWriter w;size_t used=0;int okay=0;
    if(!assembled)return 0;strcpy(request.command,"result");strcpy(request.projection,viewer_projections[view]);snprintf(request.form,sizeof(request.form),"%s",form);snprintf(request.fields,sizeof(request.fields),"%s",fields ? fields:"");request.limit=1;
    for(;;){int count;const char *next;unsigned long offset;char *end;
        w=(GD_ViewerWriter){reply,0,GD_VIEWER_REPLY_LIMIT,0};viewer_dispatch(viewer,&request,&w);if(w.failed || (count=presentation_decode(reply,chunk,sizeof(chunk)))<0 || used+(size_t)count>=GD_VIEWER_REPLY_LIMIT)goto done;
        memcpy(assembled+used,chunk,(size_t)count);used+=(size_t)count;
        if(!presentation_token(reply,"result_token",request.expect,sizeof(request.expect)))goto done;
        next=strstr(reply,"\"next_offset\":");if(!next)goto done;next+=14;if(!strncmp(next,"null",4))break;
        offset=strtoul(next,&end,10);if(end==next || offset!=used || offset<=request.offset)goto done;request.offset=(unsigned)offset;
    }
    assembled[used]=0;memcpy(reply,assembled,used+1);okay=1;
done:a.release(a.context,assembled);return okay;
}
static int viewer_retained_smoke(ExplorerViewer *viewer,GD_ExplorerView view,const char *form,const char *expected,char *reply) {
    GD_ViewerRequest request={0};GD_ViewerWriter w;char decoded[65537];
    strcpy(request.command,"result");strcpy(request.projection,viewer_projections[view]);snprintf(request.form,sizeof(request.form),"%s",form);request.limit=64;
    w=(GD_ViewerWriter){reply,0,GD_VIEWER_REPLY_LIMIT,0};viewer_dispatch(viewer,&request,&w);
    return !w.failed && presentation_decode(reply,decoded,sizeof(decoded))>0 && (!expected || strstr(decoded,expected));
}
static int viewer_presentation_checks(ExplorerViewer *original,int *fail_at,int *calls,char *reply) {
    GD_Allocator a=original->context->project->allocator;GD_ExplorerContext c=*original->context,target;GD_ExplorerPane *panes=NULL;ExplorerViewer viewer={0};GD_ViewerRequest request={0};GD_ViewerWriter w;
    int checks=0,result=0,active=GD_EXPLORER_MATRIX,stale=0,v,before,i;char decoded[65537],token[64],key[GD_MAX_PATH];
    GD_GraphAnalysis *analysis=NULL;GD_SkillUnlocks *knowledge=NULL;
#define PRESENT_CHECK(value) do {++checks;if(!(value)){fprintf(stderr,"Presentation check failed: %s (line %d)\n",#value,__LINE__);goto cleanup;}}while(0)
#define PRESENT_SEND() do {w=(GD_ViewerWriter){reply,0,GD_VIEWER_REPLY_LIMIT,0};viewer_dispatch(&viewer,&request,&w);}while(0)
#define PRESENT_REQUEST(command_,view_) do {memset(&request,0,sizeof(request));strcpy(request.command,command_);strcpy(request.projection,viewer_projections[view_]);request.limit=64;target=c;target.pane=&panes[view_];}while(0)
#define PRESENT_STAGE(view_,field_,value_) do {PRESENT_REQUEST("configure",view_);strcpy(request.form,"presentation");strcpy(request.control,field_);snprintf(request.value,sizeof(request.value),"%s",value_);viewer_control_token(&viewer,&target,GD_FORM_PRESENTATION,request.expect,sizeof(request.expect));PRESENT_SEND();}while(0)
#define PRESENT_ACTION(view_,action_) do {PRESENT_REQUEST("action",view_);strcpy(request.action,action_);viewer_control_token(&viewer,&target,GD_FORM_PRESENTATION,request.expect,sizeof(request.expect));PRESENT_SEND();}while(0)
    panes=a.allocate(a.context,GD_EXPLORER_VIEWS*sizeof(*panes));analysis=a.allocate(a.context,sizeof(*analysis));knowledge=a.allocate(a.context,sizeof(*knowledge));PRESENT_CHECK(panes && analysis && knowledge);
    memset(panes,0,GD_EXPLORER_VIEWS*sizeof(*panes));memset(analysis,0,sizeof(*analysis));memset(knowledge,0,sizeof(*knowledge));
    for(v=0;v<GD_EXPLORER_VIEWS;++v)pane_defaults(&panes[v],(GD_ExplorerView)v);
    c.pane=&panes[active];c.analysis=analysis;c.query=knowledge;c.viewport=(Rectangle){0,0,1000,700};viewer.context=&c;viewer.panes=panes;viewer.active=&active;viewer.stale=&stale;viewer.reload_generation=19;
    for(v=0;v<GD_EXPLORER_VIEWS;++v)if(c.project->available&(1u<<v)) {
        PRESENT_REQUEST("controls",v);strcpy(request.form,"presentation");before=*calls;PRESENT_SEND();PRESENT_CHECK(!w.failed && strstr(reply,"\"presentation\"") && *calls==before && !panes[v].prepared);
        target=c;target.pane=&panes[v];PRESENT_CHECK(pane_prepare(&target));
        PRESENT_STAGE(v,"filter","test source");PRESENT_CHECK(strstr(reply,"\"configured\":true") && !strcmp(panes[v].filter,"test source") && active==GD_EXPLORER_MATRIX);
        PRESENT_SEND();PRESENT_CHECK(strstr(reply,"viewer.stale_controls")!=NULL);
        PRESENT_STAGE(v,"filter","");PRESENT_CHECK(strstr(reply,"\"configured\":true") && !*panes[v].filter);
        PRESENT_ACTION(v,"view.fit");PRESENT_CHECK(strstr(reply,"\"executed\":true")!=NULL);
        PRESENT_REQUEST("evidence",v);strcpy(request.form,"links");before=*calls;PRESENT_SEND();PRESENT_CHECK(!w.failed && strstr(reply,"total_slots") && *calls==before);
        if(panes[v].layout && panes[v].layout->edge_count && !query_view((GD_ExplorerView)v)) {
            PRESENT_CHECK(presentation_token(reply,"selection_token",token,sizeof(token)) && presentation_token(reply,"key",key,sizeof(key)));
            PRESENT_REQUEST("select",v);strcpy(request.form,"link");strcpy(request.id,key);strcpy(request.expect,token);PRESENT_SEND();PRESENT_CHECK(strstr(reply,"\"selected\":true") && active==GD_EXPLORER_MATRIX);
            if(v==GD_EXPLORER_MDA){const GD_LayoutEdge *edge=&panes[v].layout->edges[0];PRESENT_CHECK(panes[v].mda.link==edge->key && panes[v].mda.record==gd_explorer_model(c.project,GD_EXPLORER_MDA)->edges[c.project->mda->links[edge->key].edge].source);}
            strcpy(request.expect,"stale");PRESENT_SEND();PRESENT_CHECK(strstr(reply,"viewer.stale_controls")!=NULL);
        }
        if(!query_view((GD_ExplorerView)v)) {
            PRESENT_STAGE(v,"shared","true");PRESENT_CHECK(strstr(reply,"\"configured\":true")!=NULL);
            PRESENT_STAGE(v,"paths_enabled","true");PRESENT_CHECK(strstr(reply,"\"configured\":true")!=NULL);
            PRESENT_STAGE(v,"shapes","true");PRESENT_CHECK(strstr(reply,"\"configured\":true")!=NULL);
            PRESENT_STAGE(v,"overview_enabled","true");PRESENT_CHECK(strstr(reply,"\"configured\":true")!=NULL);
            PRESENT_ACTION(v,"view.analyze");PRESENT_CHECK(strstr(reply,"\"executed\":true") && analysis->initialized);
            PRESENT_REQUEST("result",v);strcpy(request.form,"layers");strcpy(request.fields,"nodes,links,shared,paths,foundations,reachability,shapes,overview");request.limit=1;PRESENT_SEND();PRESENT_CHECK(!w.failed && strstr(reply,"\"encoding\":\"base64\"") && presentation_decode(reply,decoded,sizeof(decoded))>0 && strstr(decoded,"\"ok\":true"));
            PRESENT_CHECK(presentation_token(reply,"result_token",token,sizeof(token)));
            strcpy(request.expect,token);request.offset=1024;PRESENT_SEND();PRESENT_CHECK(!w.failed && strstr(reply,"\"offset\":1024") && !strstr(reply,"viewer.stale_result"));
            strcpy(request.fields,"nodes");PRESENT_SEND();PRESENT_CHECK(strstr(reply,"viewer.stale_result")!=NULL);
            PRESENT_STAGE(v,"filter","new scope");PRESENT_CHECK(strstr(reply,"\"configured\":true")!=NULL);
            PRESENT_REQUEST("result",v);strcpy(request.form,"layers");PRESENT_SEND();PRESENT_CHECK(strstr(reply,"viewer.stale_result")!=NULL);
            PRESENT_STAGE(v,"filter","");PRESENT_CHECK(strstr(reply,"\"configured\":true")!=NULL);
        }
        PRESENT_REQUEST("evidence",v);strcpy(request.form,"diagnostics");PRESENT_SEND();PRESENT_CHECK(!w.failed && strstr(reply,"producer_omissions"));
    }
    {
        GD_Model *model=gd_explorer_model(c.project,GD_EXPLORER_MATRIX);int cell=0,offset=model->cells[cell].pair_count-1;
        PRESENT_REQUEST("select",GD_EXPLORER_MATRIX);strcpy(request.form,"cell");strcpy(request.reference,model->records[model->cells[cell].noun].reference);strcpy(request.id,model->records[model->cells[cell].verb].reference);viewer_control_token(&viewer,&target,GD_FORM_PRESENTATION,request.expect,sizeof(request.expect));PRESENT_SEND();PRESENT_CHECK(strstr(reply,"\"selected\":true")!=NULL);
        PRESENT_REQUEST("select",GD_EXPLORER_MATRIX);strcpy(request.form,"case");strcpy(request.id,model->records[model->pairs[model->cells[cell].first_pair+offset].interaction].reference);viewer_control_token(&viewer,&target,GD_FORM_PRESENTATION,request.expect,sizeof(request.expect));PRESENT_SEND();PRESENT_CHECK(strstr(reply,"\"selected\":true") && panes[GD_EXPLORER_MATRIX].selection.interaction==offset);
        PRESENT_STAGE(GD_EXPLORER_MATRIX,"sort","degree");PRESENT_CHECK(strstr(reply,"\"configured\":true") && panes[GD_EXPLORER_MATRIX].selection.cell==cell && panes[GD_EXPLORER_MATRIX].selection.interaction==offset);
        PRESENT_ACTION(GD_EXPLORER_MATRIX,"view.rearrange");PRESENT_CHECK(strstr(reply,"\"executed\":true") && panes[GD_EXPLORER_MATRIX].selection.cell==cell && panes[GD_EXPLORER_MATRIX].selection.interaction==offset);
    }
    PRESENT_STAGE(GD_EXPLORER_MATRIX,"layout","grid");PRESENT_CHECK(strstr(reply,"\"configured\":true") && panes[GD_EXPLORER_MATRIX].layout_choice==2);
    {
        const GD_Model *model=gd_explorer_model(c.project,GD_EXPLORER_MATRIX);int noun=-1,verb=-1;
        for(i=0;i<model->record_count;++i){if(model->records[i].role==GD_NOUN && noun<0)noun=i;if(model->records[i].role==GD_VERB && verb<0)verb=i;}
        PRESENT_CHECK(noun>=0 && verb>=0);PRESENT_REQUEST("select",GD_EXPLORER_MATRIX);strcpy(request.form,"cell");strcpy(request.reference,model->records[noun].reference);strcpy(request.id,model->records[verb].reference);viewer_control_token(&viewer,&target,GD_FORM_PRESENTATION,request.expect,sizeof(request.expect));PRESENT_SEND();
        PRESENT_CHECK(strstr(reply,"\"selected\":true") && panes[GD_EXPLORER_MATRIX].grid.noun==noun && panes[GD_EXPLORER_MATRIX].grid.verb==verb);
        PRESENT_ACTION(GD_EXPLORER_MATRIX,"view.cell.fit");PRESENT_CHECK(strstr(reply,"\"executed\":true")!=NULL);
    }
    PRESENT_ACTION(GD_EXPLORER_RESOURCES,"view.analyze");PRESENT_CHECK(strstr(reply,"\"executed\":true")!=NULL);
    target=c;target.pane=&panes[GD_EXPLORER_ACTORS];viewer_record_layers(&viewer,&target);PRESENT_CHECK(viewer.layer_view==GD_EXPLORER_RESOURCES);
    PRESENT_REQUEST("result",GD_EXPLORER_ACTORS);strcpy(request.form,"layers");PRESENT_SEND();PRESENT_CHECK(strstr(reply,"viewer.stale_result")!=NULL);
    PRESENT_STAGE(GD_EXPLORER_MATRIX,"layout","free");PRESENT_CHECK(strstr(reply,"\"configured\":true") && panes[GD_EXPLORER_MATRIX].layout_choice==0);
    {
        GD_Layout *last=panes[GD_EXPLORER_MATRIX].layout;*fail_at=*calls+1;
        PRESENT_STAGE(GD_EXPLORER_MATRIX,"layout","columns");*fail_at=-1;PRESENT_CHECK(strstr(reply,"viewer.control") && panes[GD_EXPLORER_MATRIX].layout==last && panes[GD_EXPLORER_MATRIX].layout_choice==0);
    }
    {
        GD_ExplorerPane saved=panes[GD_EXPLORER_RESOURCES];GD_Layout *diamond=a.allocate(a.context,sizeof(*diamond));const GD_ExplorerControl *field=gd_explorer_form_field(GD_FORM_PRESENTATION,"bypass_candidate");int candidate,okay;
        PRESENT_CHECK(diamond!=NULL);memset(diamond,0,sizeof(*diamond));diamond->node_count=4;diamond->edge_count=4;
        for(i=0;i<4;++i){diamond->nodes[i].key=i;diamond->nodes[i].label="Private path node";diamond->edges[i].key=i;diamond->edges[i].active=1;}
        diamond->edges[0].a=0;diamond->edges[0].b=1;diamond->edges[1].a=0;diamond->edges[1].b=2;diamond->edges[2].a=1;diamond->edges[2].b=3;diamond->edges[3].a=2;diamond->edges[3].b=3;
        panes[GD_EXPLORER_RESOURCES].layout=diamond;panes[GD_EXPLORER_RESOURCES].selection=(GD_ExplorerSelection){-1,3,0,-1,-1};panes[GD_EXPLORER_RESOURCES].paths.count=1;panes[GD_EXPLORER_RESOURCES].paths.keys[0]=0;panes[GD_EXPLORER_RESOURCES].paths.compare=1;analysis->initialized=0;
        PRESENT_ACTION(GD_EXPLORER_RESOURCES,"view.analyze");okay=strstr(reply,"\"executed\":true") && analysis->paths.selection.candidate_count>0;
        target=c;target.pane=&panes[GD_EXPLORER_RESOURCES];okay=okay && gd_explorer_presentation_option(&target,field,0,&candidate,key,sizeof(key))==1;
        if(okay){PRESENT_STAGE(GD_EXPLORER_RESOURCES,"bypass_candidate",key);okay=strstr(reply,"\"configured\":true") && analysis->paths.selection.candidate==candidate && analysis->paths.bypass.valid;}
        panes[GD_EXPLORER_RESOURCES]=saved;a.release(a.context,diamond);analysis->initialized=0;PRESENT_CHECK(okay);
    }
    PRESENT_ACTION(GD_EXPLORER_SKILLS,"view.analyze");PRESENT_CHECK(strstr(reply,"\"executed\":true")!=NULL);
    PRESENT_REQUEST("result",GD_EXPLORER_SKILLS);strcpy(request.form,"skills");PRESENT_SEND();PRESENT_CHECK(!w.failed && presentation_decode(reply,decoded,sizeof(decoded))>0 && strstr(decoded,"\"exploration\":"));
    PRESENT_CHECK(presentation_token(reply,"result_token",token,sizeof(token)));
    panes[GD_EXPLORER_SKILLS].known[0]=!panes[GD_EXPLORER_SKILLS].known[0];strcpy(request.expect,token);PRESENT_SEND();PRESENT_CHECK(strstr(reply,"viewer.stale_result")!=NULL);panes[GD_EXPLORER_SKILLS].known[0]=!panes[GD_EXPLORER_SKILLS].known[0];
    target=c;target.pane=&panes[GD_EXPLORER_SKILLS];viewer_control_token(&viewer,&target,GD_FORM_PRESENTATION,token,sizeof(token));panes[GD_EXPLORER_SKILLS].paths.count=1;panes[GD_EXPLORER_SKILLS].paths.keys[0]=0;viewer_control_token(&viewer,&target,GD_FORM_PRESENTATION,key,sizeof(key));PRESENT_CHECK(strcmp(key,token));panes[GD_EXPLORER_SKILLS].paths.count=0;
    PRESENT_REQUEST("result",GD_EXPLORER_MDA);strcpy(request.form,"mda");PRESENT_SEND();PRESENT_CHECK(!w.failed && presentation_decode(reply,decoded,sizeof(decoded))>0);
    PRESENT_REQUEST("result",GD_EXPLORER_CHALLENGES);strcpy(request.form,"challenges");PRESENT_SEND();PRESENT_CHECK(!w.failed && presentation_decode(reply,decoded,sizeof(decoded))>0);
    {
        GD_Error error={"test.rejected","first message",NULL,1};target=c;target.pane=&panes[GD_EXPLORER_MATRIX];viewer_control_token(&viewer,&target,GD_FORM_PRESENTATION,token,sizeof(token));
        panes[GD_EXPLORER_MATRIX].diagnostics.rejected=gd_diagnostics_capture(a,&error,1,3);PRESENT_CHECK(panes[GD_EXPLORER_MATRIX].diagnostics.rejected!=NULL);viewer_control_token(&viewer,&target,GD_FORM_PRESENTATION,key,sizeof(key));PRESENT_CHECK(strcmp(key,token));
        PRESENT_REQUEST("evidence",GD_EXPLORER_MATRIX);strcpy(request.form,"diagnostics");PRESENT_SEND();PRESENT_CHECK(strstr(reply,"\"rejected_errors_omitted\":3") && strstr(reply,"\"capture_failed\":false"));
    }
    PRESENT_ACTION(GD_EXPLORER_MATRIX,"view.find.clear");PRESENT_CHECK(strstr(reply,"\"executed\":true")!=NULL);
    PRESENT_ACTION(GD_EXPLORER_RESOURCES,"view.reset");PRESENT_CHECK(strstr(reply,"\"executed\":true") && panes[GD_EXPLORER_RESOURCES].prepared && active==GD_EXPLORER_MATRIX);
    PRESENT_ACTION(GD_EXPLORER_MDA,"view.analyze");PRESENT_CHECK(strstr(reply,"\"executed\":true")!=NULL);
    PRESENT_CHECK(viewer_result_reassemble(&viewer,GD_EXPLORER_MDA,"layers",NULL,reply));
    result=checks;
cleanup:
    *fail_at=-1;
    if(panes){for(v=0;v<GD_EXPLORER_VIEWS;++v){pane_release_layout(a,&panes[v]);gd_layout_focus_destroy(panes[v].distance_focus);gd_resource_routes_destroy(panes[v].resource_routes);gd_diagnostics_clear(&panes[v].diagnostics);}a.release(a.context,panes);}
    if(analysis)a.release(a.context,analysis);if(knowledge)a.release(a.context,knowledge);
#undef PRESENT_CHECK
#undef PRESENT_SEND
#undef PRESENT_REQUEST
#undef PRESENT_STAGE
#undef PRESENT_ACTION
    return result;
}
#endif
