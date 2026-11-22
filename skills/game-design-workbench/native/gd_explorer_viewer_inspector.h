/* Active rendered inspector snapshot and atomic, presentation-only patch. */
#ifndef GD_EXPLORER_VIEWER_INSPECTOR_H
#define GD_EXPLORER_VIEWER_INSPECTOR_H
static void viewer_inspector_key(const GD_InspectorItem *item,char *key,size_t capacity) {
    if(item->kind==GD_INSPECTOR_ROOT)snprintf(key,capacity,"section-%u",(unsigned)item->widget);
    else snprintf(key,capacity,"%s-%08x-%08x",item->kind==GD_INSPECTOR_FOLD ? "fold":item->kind==GD_INSPECTOR_CHILD ? "scroll":"page",(unsigned)item->window,(unsigned)item->widget);
}
static void viewer_inspector_seal(ExplorerViewer *viewer) {
    GD_InspectorSurvey *s=viewer->inspector_survey;
    if(s && s->ready)viewer_control_token(viewer,viewer->context,GD_FORM_PRESENTATION,s->scope,sizeof(s->scope));
}
static int viewer_inspector_token(ExplorerViewer *viewer,char *token,size_t capacity) {
    GD_ExplorerContext *c=viewer->context;const GD_InspectorSurvey *s=viewer->inspector_survey;GD_ExplorerInspector *inspector=c->inspector;
    char scope[64];uint64_t hash=UINT64_C(14695981039346656037);
    if(!s || !s->ready || !inspector || !c->sidebar || s->projection!=(int)c->pane->view ||
       c->sidebar->reveal || c->sidebar->reveal_form)return 0;
    viewer_control_token(viewer,c,GD_FORM_PRESENTATION,scope,sizeof(scope));
    if(strcmp(scope,s->scope))return 0;
    hash=viewer_control_hash(hash,scope,strlen(scope));
    hash=viewer_control_hash(hash,&s->revision,sizeof(s->revision));
    hash=viewer_control_hash(hash,&s->limited,sizeof(s->limited));
    hash=viewer_control_hash(hash,s->items,(size_t)s->count*sizeof(*s->items));
    hash=viewer_control_hash(hash,&inspector->open,sizeof(inspector->open));
    hash=viewer_control_hash(hash,&inspector->viewport_width,sizeof(inspector->viewport_width));
    hash=viewer_control_hash(hash,&inspector->preferred_width,sizeof(inspector->preferred_width));
    hash=viewer_control_hash(hash,&inspector->scroll,sizeof(inspector->scroll));
    hash=viewer_control_hash(hash,&inspector->scroll_max,sizeof(inspector->scroll_max));
    hash=viewer_control_hash(hash,&inspector->scroll_pending,sizeof(inspector->scroll_pending));
    hash=viewer_control_hash(hash,&c->sidebar->open,sizeof(c->sidebar->open));
    snprintf(token,capacity,"%016llx",(unsigned long long)hash);return 1;
}
static void viewer_inspector_read(ExplorerViewer *viewer,const GD_ViewerRequest *request,GD_ViewerWriter *w,const char *token) {
    GD_ExplorerContext *c=viewer->context;const GD_ExplorerInspector *shell=c->inspector;const GD_InspectorSurvey *s=viewer->inspector_survey;
    int i,start=(int)request->offset,end=(int)(request->offset+request->limit);
    if(end>s->count)end=s->count;
    gd_viewer_write(w,"{\"ok\":true,\"protocol\":1,");viewer_member(w,"session",gd_viewer_session(viewer->server));gd_viewer_write(w,",");
    viewer_member(w,"projection",viewer_projections[s->projection]);gd_viewer_write(w,",");viewer_member(w,"inspector_token",token);
    gd_viewer_write(w,",\"open\":%s,\"width\":%d,\"effective_width\":%d,\"width_max\":%d,\"scroll\":%d,\"scroll_max\":%d,\"limited\":%s,\"total_items\":%d,\"offset\":%d,\"items\":[",
        shell->open ? "true":"false",(int)shell->preferred_width,inspector_pixels(shell),gd_explorer_presentation_maximum(c,"inspector_width"),shell->scroll,shell->scroll_max,s->limited ? "true":"false",s->count,start);
    for(i=start;i<end;++i){
        const GD_InspectorItem *item=&s->items[i];char key[GD_INSPECTOR_PATCH_KEY];viewer_inspector_key(item,key,sizeof(key));
        gd_viewer_write(w,i==start ? "{":",{");viewer_member(w,"key",key);gd_viewer_write(w,",");viewer_member(w,"label",item->label);
        gd_viewer_write(w,",");viewer_member(w,"section",item->section>=0 && item->section<GD_SIDEBAR_COUNT ? gd_sidebar_labels[item->section]:NULL);
        gd_viewer_write(w,",\"depth\":%d,\"editable\":%s,",item->depth,item->ambiguous ? "false":"true");
        if(item->kind==GD_INSPECTOR_CHILD || item->kind==GD_INSPECTOR_PAGE)gd_viewer_write(w,"\"kind\":\"%s\",\"value\":%d,\"max\":%d}",item->kind==GD_INSPECTOR_CHILD ? "scroll":"page",item->value,item->maximum);
        else gd_viewer_write(w,"\"kind\":\"fold\",\"open\":%s}",item->value ? "true":"false");
    }
    gd_viewer_write(w,"],\"next_offset\":");if(end<s->count)gd_viewer_write(w,"%d",end);else gd_viewer_write(w,"null");gd_viewer_write(w,"}");
}
static void viewer_inspector_patch(ExplorerViewer *viewer,const GD_ViewerRequest *request,GD_ViewerWriter *w) {
    GD_ExplorerContext *c=viewer->context;GD_ExplorerInspector *shell=c->inspector;GD_InspectorSurvey *s=viewer->inspector_survey;
    GD_InspectorPatch patch;int i,j;const GD_InspectorItem *targets[GD_INSPECTOR_PATCH_ITEMS]={0};ImGuiWindow *windows[GD_INSPECTOR_PATCH_ITEMS]={0};
    const char *error="Use a partial JSON object with open, width, scroll, folds, scrolls or pages; no duplicate fields or keys";
    if(!gd_viewer_inspector_patch_parse(request->value,&patch))goto invalid;
    for(i=0;i<patch.count;++i){
        const GD_InspectorPatchItem *op=&patch.items[i];int value=(int)op->value;
        if(op->kind==GD_INSPECTOR_PATCH_OPEN)continue;
        if(op->kind==GD_INSPECTOR_PATCH_WIDTH){
            error="Width must be 0 (automatic) or at least 310 pixels within this window";
            if(value && (value<310 || value>gd_explorer_presentation_maximum(c,"inspector_width")))goto invalid;
            continue;
        }
        if(op->kind==GD_INSPECTOR_PATCH_SCROLL){error="Inspector must be shown and scroll must be within its observed range";if(!shell->open || value>shell->scroll_max)goto invalid;continue;}
        error="Unknown, ambiguous or wrong-type widget key; reread inspector using the current scope";
        if(!shell->open)goto invalid;
        /* Page writes are consumed by the native pager on the next draw.
           Do not let the same request hide or replace a pager before that draw. */
        if(op->kind==GD_INSPECTOR_PATCH_PAGE && patch.count!=1){error="Change one page at a time, without other fields; paging may change which widgets are rendered";goto invalid;}
        for(j=0;j<s->count;++j){
            const GD_InspectorItem *item=&s->items[j];char key[GD_INSPECTOR_PATCH_KEY];
            if((item->kind==GD_INSPECTOR_CHILD)!=(op->kind==GD_INSPECTOR_PATCH_CHILD) || (item->kind==GD_INSPECTOR_PAGE)!=(op->kind==GD_INSPECTOR_PATCH_PAGE))continue;
            viewer_inspector_key(item,key,sizeof(key));if(strcmp(key,op->key))continue;
            if(item->ambiguous || targets[i])goto invalid;
            targets[i]=item;
        }
        if(!targets[i])goto invalid;
        if((targets[i]->kind==GD_INSPECTOR_CHILD || targets[i]->kind==GD_INSPECTOR_PAGE) && value>targets[i]->maximum){error="Value exceeds its observed range";goto invalid;}
        if(targets[i]->kind!=GD_INSPECTOR_ROOT && !(windows[i]=igFindWindowByID(targets[i]->window)))goto invalid;
    }
    /* Validation above does not mutate the shell, controls, or ImGui storage. */
    for(i=0;i<patch.count;++i){
        const GD_InspectorPatchItem *op=&patch.items[i];int value=(int)op->value;const GD_InspectorItem *item=targets[i];
        if(op->kind==GD_INSPECTOR_PATCH_OPEN)shell->open=value!=0;
        else if(op->kind==GD_INSPECTOR_PATCH_WIDTH)shell->preferred_width=(float)value;
        else if(op->kind==GD_INSPECTOR_PATCH_SCROLL){shell->scroll=value;shell->scroll_pending=true;c->sidebar->reveal=c->sidebar->reveal_form=0;}
        else if(item->kind==GD_INSPECTOR_CHILD)igSetScrollY_WindowPtr(windows[i],(float)value);
        else if(item->kind==GD_INSPECTOR_PAGE){
            GD_InspectorPageChange *page=&s->page_changes[0];
            *page=(GD_InspectorPageChange){.window=item->window,.widget=item->widget,.section=item->section,.value=value,.maximum=item->maximum};
            memcpy(page->label,item->label,sizeof(page->label));s->page_count=1;
        }
        else if(item->kind==GD_INSPECTOR_FOLD)ImGuiStorage_SetInt(&windows[i]->StateStorage,item->widget,value);
        else if(value)c->sidebar->open|=1u<<item->widget;
        else c->sidebar->open&=~(1u<<item->widget);
    }
    if(patch.count){++s->revision;s->ready=0;}
    gd_viewer_write(w,"{\"ok\":true,\"applied\":%d,\"queued\":%d,\"refresh_required\":%s,\"source_draft_changed\":false}",patch.count-s->page_count,s->page_count,patch.count ? "true":"false");return;
invalid:gd_viewer_error(w,"viewer.inspector_patch",error);
}
static void viewer_inspector_dispatch(ExplorerViewer *viewer,const GD_ViewerRequest *request,GD_ViewerWriter *w) {
    char token[64];int patch=!strcmp(request->command,"inspector-patch");
    if(patch && (viewer_interacting() || gd_explorer_rank_pending(viewer->context->project,viewer->panes))){gd_viewer_error(w,"viewer.busy","Inspector patch blocked by active input or pending rank; nothing applied");return;}
    if(!viewer_inspector_token(viewer,token,sizeof(token))){gd_viewer_error(w,"viewer.inspector_unavailable","The active inspector has no current frame snapshot; read again after it renders");return;}
    if((patch || *request->expect) && strcmp(token,request->expect)){gd_viewer_error(w,"viewer.stale_inspector","Inspector state changed; reread before patching or paging");return;}
    if(patch)viewer_inspector_patch(viewer,request,w);else viewer_inspector_read(viewer,request,w,token);
}
#endif
