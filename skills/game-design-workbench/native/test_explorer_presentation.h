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
static unsigned long presentation_footprint_overlaps(const GD_Layout *layout) {
    unsigned long count=0;int i,j;
    for(i=0;i<layout->node_count;++i)for(j=i+1;j<layout->node_count;++j){
        const GD_LayoutNode *a=&layout->nodes[i],*b=&layout->nodes[j];
        if(a->x+a->footprint.right>b->x+b->footprint.left && b->x+b->footprint.right>a->x+a->footprint.left &&
           a->y+a->footprint.bottom>b->y+b->footprint.top && b->y+b->footprint.bottom>a->y+a->footprint.top)++count;
    }
    return count;
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
    {
        GD_LayoutNode node={.x=10,.y=20};
        PRESENT_CHECK(gd_layout_set_footprint(&node,-1,-1,1,1));
        gd_graph_footprint_rect(&node,(Rectangle){-75,28,210,64},3);
        PRESENT_CHECK(fabsf(node.footprint.left+35)<0.001f && fabsf(node.footprint.right-35)<0.001f);
        PRESENT_CHECK(fabsf(node.footprint.top+32.0f/3)<0.001f && fabsf(node.footprint.bottom-32.0f/3)<0.001f);
        gd_graph_footprint_rect(&node,(Rectangle){180,105,120,14},3);
        PRESENT_CHECK(fabsf(node.footprint.right-90)<0.001f && fabsf(node.footprint.bottom-59.0f/3)<0.001f);
        PRESENT_CHECK(fabsf(node.footprint.left+35)<0.001f && fabsf(node.footprint.top+32.0f/3)<0.001f);
    }
    panes=a.allocate(a.context,GD_EXPLORER_VIEWS*sizeof(*panes));analysis=a.allocate(a.context,sizeof(*analysis));knowledge=a.allocate(a.context,sizeof(*knowledge));PRESENT_CHECK(panes && analysis && knowledge);
    memset(panes,0,GD_EXPLORER_VIEWS*sizeof(*panes));memset(analysis,0,sizeof(*analysis));memset(knowledge,0,sizeof(*knowledge));
    for(v=0;v<GD_EXPLORER_VIEWS;++v)pane_defaults(&panes[v],(GD_ExplorerView)v);
    c.pane=&panes[active];c.analysis=analysis;c.query=knowledge;c.viewport=(Rectangle){0,0,1000,700};viewer.context=&c;viewer.panes=panes;viewer.active=&active;viewer.stale=&stale;viewer.reload_generation=19;
    for(v=0;v<GD_EXPLORER_VIEWS;++v)if(c.project->available&(1u<<v)) {
        PRESENT_REQUEST("controls",v);strcpy(request.form,"presentation");before=*calls;PRESENT_SEND();PRESENT_CHECK(!w.failed && strstr(reply,"\"presentation\"") && *calls==before && !panes[v].prepared);
        target=c;target.pane=&panes[v];PRESENT_CHECK(pane_prepare(&target));
        if(v==GD_EXPLORER_RESOURCES){
            GD_Layout *probe=a.allocate(a.context,sizeof(*probe));unsigned long initial,remaining;int step,node,valid=1;
            GD_LayoutPoint previous[GD_MAX_RECORDS];double square=0,maximum=0;
            PRESENT_CHECK(probe!=NULL);memcpy(probe,panes[v].layout,sizeof(*probe));initial=presentation_footprint_overlaps(probe);
            for(step=0;step<600;++step){
                if(step>=568)for(node=0;node<probe->node_count;++node)previous[node]=(GD_LayoutPoint){probe->nodes[node].x,probe->nodes[node].y};
                gd_layout_step(probe,-1);if(probe->collision_checks>GD_LAYOUT_COLLISION_WORK)valid=0;
                if(step>=568)for(node=0;node<probe->node_count;++node){double dx=(double)probe->nodes[node].x-previous[node].x,dy=(double)probe->nodes[node].y-previous[node].y,movement=dx*dx+dy*dy;square+=movement;if(movement>maximum)maximum=movement;}
            }
            for(step=0;step<probe->node_count;++step)if(!isfinite(probe->nodes[step].x)||!isfinite(probe->nodes[step].y))valid=0;
            remaining=presentation_footprint_overlaps(probe);if(initial || remaining)valid=0;
            fprintf(stderr,"Resource footprint probe: %d nodes / %d groups / %lu initial overlaps / %lu after 600 more steps / tail RMS %.6f max %.6f / limited %d\n",probe->node_count,probe->group_count,initial,remaining,probe->node_count ? sqrt(square/(32*probe->node_count)):0,sqrt(maximum),probe->collision_limited);
            a.release(a.context,probe);PRESENT_CHECK(valid);
        }
        if(v<=GD_EXPLORER_RESOURCES) {
            GD_Layout *native=a.allocate(a.context,sizeof(*native)); GD_LayoutPoint *seeded=a.allocate(a.context,GD_MAX_RECORDS*sizeof(*seeded));
            GD_GraphAnalysis *retained=a.allocate(a.context,sizeof(*retained));
            GD_ExplorerSelection selection; int node,seed_allocations,root=c.project->mda ? c.project->mda->root:-1;
            GD_Focus scope=panes[v].focus;
            PRESENT_CHECK(native && seeded && retained);
            if(v<=GD_EXPLORER_RELATIONSHIPS) {
                if(v==GD_EXPLORER_SKILLS) {PRESENT_STAGE(v,"layout","rank");PRESENT_CHECK(strstr(reply,"\"configured\":true") && panes[v].rank==1);}
                if(v==GD_EXPLORER_RELATIONSHIPS && panes[v].layout->node_count>1) {
                    for(node=0;node<panes[v].layout->node_count;++node) if(panes[v].layout->nodes[node].key!=panes[v].focus.root) break;
                    if(node<panes[v].layout->node_count) {panes[v].selection.node=node;PRESENT_ACTION(v,"view.expand");PRESENT_CHECK(strstr(reply,"\"executed\":true"));}
                }
                if(v==GD_EXPLORER_RELATIONSHIPS) {
                    /* Retained partial scope must not replay expansion on mode
                       switch, even when expanded records now have more neighbors. */
                    for(node=0;node<panes[v].layout->node_count;++node) {
                        int record=panes[v].layout->nodes[node].key;
                        if(record!=panes[v].focus.root && !panes[v].focus.expanded[record]) break;
                    }
                    if(node<panes[v].layout->node_count) {
                        int hidden=panes[v].layout->nodes[node].key;
                        scope=panes[v].focus;scope.visible[hidden]=0;panes[v].focus=scope;
                        memset(panes[v].focus.expanded,0,sizeof(panes[v].focus.expanded));target=c;target.pane=&panes[v];
                        PRESENT_CHECK(gd_explorer_graph_prepare(&target));
                        memcpy(panes[v].focus.expanded,scope.expanded,sizeof(scope.expanded));
                        for(node=0;node<panes[v].layout->node_count;++node) if(panes[v].layout->nodes[node].key==scope.root) panes[v].selection.node=node;
                        panes[v].inspected_record=scope.root;PRESENT_CHECK(!panes[v].focus.visible[hidden]);
                    }
                }
                scope=panes[v].focus;memcpy(native,panes[v].layout,sizeof(*native));selection=panes[v].selection;
                PRESENT_STAGE(v,"initial_arrangement","neighborhood");PRESENT_CHECK(strstr(reply,"viewer.control") && !panes[v].initial_arrangement);
                PRESENT_ACTION(v,"view.analyze");PRESENT_CHECK(strstr(reply,"\"executed\":true") && analysis->initialized);memcpy(retained,analysis,sizeof(*retained));
                PRESENT_STAGE(v,"layout","free");PRESENT_CHECK(strstr(reply,"\"configured\":true") && free_graph(&panes[v]) && layout_settles(&panes[v]) && !panes[v].paused && !memcmp(retained,analysis,sizeof(*retained)));
                PRESENT_CHECK(!memcmp(&scope,&panes[v].focus,sizeof(scope)) && !memcmp(&selection,&panes[v].selection,sizeof(selection)));
                for(node=0;node<native->node_count;++node) PRESENT_CHECK(native->nodes[node].x==panes[v].layout->nodes[node].x && native->nodes[node].y==panes[v].layout->nodes[node].y && native->nodes[node].layer==panes[v].layout->nodes[node].layer);
            }
            if(v==GD_EXPLORER_MDA) {PRESENT_STAGE(v,"initial_arrangement","neighborhood");PRESENT_CHECK(!panes[v].initial_arrangement && strstr(reply,"viewer.control"));PRESENT_STAGE(v,"layout","free");PRESENT_CHECK(strstr(reply,"\"configured\":true"));}
            memcpy(native,panes[v].layout,sizeof(*native));selection=panes[v].selection;
            if(v<=GD_EXPLORER_RELATIONSHIPS && native->node_count) {
                int movable=0;float x,y;unsigned tick=panes[v].layout->tick;
                for(node=0;node<native->node_count;++node) if(native->nodes[node].side || native->nodes[node].layer==0) {movable=node;break;}
                x=panes[v].layout->nodes[movable].x+31;y=panes[v].layout->nodes[movable].y+43;
                gd_layout_move_node(panes[v].layout,movable,x,y);PRESENT_CHECK(panes[v].layout->nodes[movable].x==x && panes[v].layout->nodes[movable].y==y);
                gd_layout_step(panes[v].layout,movable);PRESENT_CHECK(panes[v].layout->tick==tick+1 && panes[v].layout->nodes[movable].x==x && panes[v].layout->nodes[movable].y==y);
            }
            PRESENT_ACTION(v,"view.analyze");PRESENT_CHECK(strstr(reply,"\"executed\":true") && analysis->initialized);memcpy(retained,analysis,sizeof(*retained));
            seed_allocations=*calls;PRESENT_STAGE(v,"initial_arrangement","neighborhood");seed_allocations=*calls-seed_allocations;
            PRESENT_CHECK(strstr(reply,"\"configured\":true") && panes[v].initial_arrangement==1 && !panes[v].paused);
            PRESENT_CHECK(!memcmp(&selection,&panes[v].selection,sizeof(selection)) && (!c.project->mda || c.project->mda->root==root));
            PRESENT_CHECK(!memcmp(retained,analysis,sizeof(*retained)));
            PRESENT_CHECK(native->node_count==panes[v].layout->node_count && !memcmp(native->edges,panes[v].layout->edges,sizeof(native->edges)));
            for(node=0;node<native->node_count;++node) seeded[node]=(GD_LayoutPoint){panes[v].layout->nodes[node].x,panes[v].layout->nodes[node].y};
            if(v==GD_EXPLORER_MATRIX && native->node_count>1) panes[v].selection.node=1;
            PRESENT_ACTION(v,"view.rearrange");PRESENT_CHECK(strstr(reply,"\"executed\":true"));
            for(node=0;node<native->node_count;++node) PRESENT_CHECK(seeded[node].x==panes[v].layout->nodes[node].x && seeded[node].y==panes[v].layout->nodes[node].y);
            panes[v].selection=selection;
            PRESENT_ACTION(v,"view.analyze");PRESENT_CHECK(strstr(reply,"\"executed\":true") && analysis->initialized);memcpy(retained,analysis,sizeof(*retained));
            PRESENT_STAGE(v,"initial_arrangement","native");PRESENT_CHECK(strstr(reply,"\"configured\":true") && !panes[v].initial_arrangement && !panes[v].paused);
            PRESENT_CHECK(!memcmp(retained,analysis,sizeof(*retained)));
            for(node=0;node<native->node_count;++node) PRESENT_CHECK(native->nodes[node].x==panes[v].layout->nodes[node].x && native->nodes[node].y==panes[v].layout->nodes[node].y);
            {
                GD_Layout *last=panes[v].layout;int allocation;panes[v].paused=true;
                for(allocation=1;allocation<=seed_allocations;++allocation) {
                    *fail_at=*calls+allocation;PRESENT_STAGE(v,"initial_arrangement","neighborhood");*fail_at=-1;
                    PRESENT_CHECK(strstr(reply,"viewer.control") && panes[v].layout==last && !panes[v].initial_arrangement && panes[v].paused && !memcmp(retained,analysis,sizeof(*retained)));
                }
            }
            if(v==GD_EXPLORER_MATRIX) {
                c.viewport=(Rectangle){0};PRESENT_CHECK(gd_layout_seed_neighborhood(native,a,1));
                PRESENT_STAGE(v,"initial_arrangement","neighborhood");PRESENT_CHECK(strstr(reply,"\"configured\":true"));
                c.viewport=(Rectangle){0,0,1200,400};PRESENT_REQUEST("controls",v);strcpy(request.form,"presentation");PRESENT_SEND();PRESENT_CHECK(!w.failed);
                for(node=0;node<native->node_count;++node) PRESENT_CHECK(native->nodes[node].x==panes[v].layout->nodes[node].x && native->nodes[node].y==panes[v].layout->nodes[node].y);
                PRESENT_STAGE(v,"initial_arrangement","native");PRESENT_CHECK(strstr(reply,"\"configured\":true"));c.viewport=(Rectangle){0,0,1000,700};
            }
            if(v<=GD_EXPLORER_RELATIONSHIPS) {
                PRESENT_CHECK(!memcmp(&scope,&panes[v].focus,sizeof(scope)));
                if(v==GD_EXPLORER_SKILLS) {
                    PRESENT_CHECK(panes[v].rank==1);
                    PRESENT_STAGE(v,"layout","depth");PRESENT_CHECK(strstr(reply,"\"configured\":true") && !panes[v].rank && panes[v].layout->mode==GD_LAYOUT_DIRECTED);
                    PRESENT_STAGE(v,"layout","free");PRESENT_CHECK(strstr(reply,"\"configured\":true") && !panes[v].rank && free_graph(&panes[v]));
                }
                PRESENT_STAGE(v,"layout",v==GD_EXPLORER_SKILLS ? "depth":v==GD_EXPLORER_CHALLENGES ? "bands":"native");
                PRESENT_CHECK(strstr(reply,"\"configured\":true") && !free_graph(&panes[v]));
                if(v!=GD_EXPLORER_SKILLS) PRESENT_CHECK(!layout_settles(&panes[v]));
                if(v==GD_EXPLORER_RELATIONSHIPS) PRESENT_CHECK(!memcmp(&scope,&panes[v].focus,sizeof(scope)));
            }
            if(v==GD_EXPLORER_MDA) {PRESENT_STAGE(v,"layout","bands");PRESENT_CHECK(strstr(reply,"\"configured\":true"));}
            a.release(a.context,retained);a.release(a.context,seeded);a.release(a.context,native);
        }
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
        GD_ExplorerPane *saved=a.allocate(a.context,sizeof(*saved));GD_Layout *diamond=a.allocate(a.context,sizeof(*diamond));const GD_ExplorerControl *field=gd_explorer_form_field(GD_FORM_PRESENTATION,"bypass_candidate");int candidate,okay;
        PRESENT_CHECK(saved && diamond);memcpy(saved,&panes[GD_EXPLORER_RESOURCES],sizeof(*saved));memset(diamond,0,sizeof(*diamond));diamond->node_count=4;diamond->edge_count=4;
        for(i=0;i<4;++i){diamond->nodes[i].key=i;diamond->nodes[i].label="Private path node";diamond->edges[i].key=i;diamond->edges[i].active=1;}
        diamond->edges[0].a=0;diamond->edges[0].b=1;diamond->edges[1].a=0;diamond->edges[1].b=2;diamond->edges[2].a=1;diamond->edges[2].b=3;diamond->edges[3].a=2;diamond->edges[3].b=3;
        panes[GD_EXPLORER_RESOURCES].layout=diamond;panes[GD_EXPLORER_RESOURCES].selection=(GD_ExplorerSelection){-1,3,0,-1,-1};panes[GD_EXPLORER_RESOURCES].paths.count=1;panes[GD_EXPLORER_RESOURCES].paths.keys[0]=0;panes[GD_EXPLORER_RESOURCES].paths.compare=1;analysis->initialized=0;
        PRESENT_ACTION(GD_EXPLORER_RESOURCES,"view.analyze");okay=strstr(reply,"\"executed\":true") && analysis->paths.selection.candidate_count>0;
        target=c;target.pane=&panes[GD_EXPLORER_RESOURCES];okay=okay && gd_explorer_presentation_option(&target,field,0,&candidate,key,sizeof(key))==1;
        if(okay){PRESENT_STAGE(GD_EXPLORER_RESOURCES,"bypass_candidate",key);okay=strstr(reply,"\"configured\":true") && analysis->paths.selection.candidate==candidate && analysis->paths.bypass.valid;}
        memcpy(&panes[GD_EXPLORER_RESOURCES],saved,sizeof(*saved));a.release(a.context,saved);a.release(a.context,diamond);analysis->initialized=0;PRESENT_CHECK(okay);
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
