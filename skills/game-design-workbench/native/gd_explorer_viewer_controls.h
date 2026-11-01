/* Shared form metadata/values over IPC. No source writes or hidden evaluation. */
#ifndef GD_EXPLORER_VIEWER_CONTROLS_H
#define GD_EXPLORER_VIEWER_CONTROLS_H
#include "gd_diagnostics.h"
static int viewer_form_kind(const char *name) {
    int form; for (form=0;form<GD_FORM_COUNT;++form) if (!strcmp(name,gd_explorer_form_name((GD_ExplorerForm)form))) return form;
    return -1;
}
static uint64_t viewer_control_hash(uint64_t hash,const void *data,size_t size) {
    const unsigned char *bytes=data; size_t i;
    for (i=0;i<size;++i) { hash^=bytes[i]; hash*=UINT64_C(1099511628211); } return hash;
}
static uint64_t viewer_form_hash(uint64_t hash,GD_ExplorerForm form,const void *values) {
    int i,count,present=values!=NULL; const GD_ExplorerControl *fields=gd_explorer_form_fields(form,&count);
    hash=viewer_control_hash(hash,&present,sizeof(present));
    if (values) for (i=0;i<count;++i) {
        const char *value=(const char *)values+fields[i].offset;
        hash=viewer_control_hash(hash,value,fields[i].type==GD_CONTROL_TEXT ? strlen(value)+1:fields[i].capacity);
    }
    return hash;
}
/* Opaque stale-input guard, not authorization. Field bytes exclude padding and
   stale text after NUL. Session/epoch/pending/applied scope binds one form. */
static void viewer_control_token(ExplorerViewer *viewer,GD_ExplorerContext *c,GD_ExplorerForm form,char *token,size_t capacity) {
    const char *session=gd_viewer_session(viewer->server); uint64_t hash=UINT64_C(14695981039346656037);
    GD_ExplorerPresentation presentation;
    int selected=c->pane->query_state && c->pane->query_state->selected ? c->pane->query_state->selected->selection.target:-1;
    hash=viewer_control_hash(hash,session ? session:"",session ? strlen(session):0);
    hash=viewer_control_hash(hash,&viewer->reload_generation,sizeof(viewer->reload_generation));
    hash=viewer_control_hash(hash,&c->project->revision,sizeof(c->project->revision));
    hash=viewer_control_hash(hash,&c->pane->view,sizeof(c->pane->view)); hash=viewer_control_hash(hash,&form,sizeof(form));
    if (form==GD_FORM_PRESENTATION) {
        int dependent,record=c->pane->prepared ? selected_record(c):-1;
        gd_explorer_presentation_read(c,&presentation); hash=viewer_form_hash(hash,form,&presentation);
        hash=viewer_control_hash(hash,&record,sizeof(record)); hash=viewer_control_hash(hash,&c->pane->selection,sizeof(c->pane->selection));
        hash=viewer_control_hash(hash,&selected,sizeof(selected));
        hash=viewer_control_hash(hash,c->pane->known,sizeof(c->pane->known));
        hash=viewer_control_hash(hash,&c->pane->paths.count,sizeof(c->pane->paths.count));
        hash=viewer_control_hash(hash,c->pane->paths.keys,(size_t)c->pane->paths.count*sizeof(*c->pane->paths.keys));
        hash=viewer_control_hash(hash,c->pane->focus.visible,sizeof(c->pane->focus.visible));hash=viewer_control_hash(hash,c->pane->focus.expanded,sizeof(c->pane->focus.expanded));
        hash=viewer_control_hash(hash,&c->pane->diagnostics.capture_failed,sizeof(c->pane->diagnostics.capture_failed));
        { const GD_DiagnosticErrors *errors=c->pane->diagnostics.rejected;int count=errors ? errors->count:0,i;
          hash=viewer_control_hash(hash,&count,sizeof(count));
          if(errors){hash=viewer_control_hash(hash,&errors->omitted,sizeof(errors->omitted));hash=viewer_control_hash(hash,errors->truncated,(size_t)count);hash=viewer_control_hash(hash,errors->path_truncated,(size_t)count);
            for(i=0;i<count;++i){const char *texts[3]={errors->errors[i].code,errors->errors[i].message,errors->errors[i].path};int j;hash=viewer_control_hash(hash,&errors->errors[i].line,sizeof(int));for(j=0;j<3;++j)hash=viewer_control_hash(hash,texts[j] ? texts[j]:"",texts[j] ? strlen(texts[j])+1:1);}
          }
        }
        if(c->pane->rule_state) {hash=viewer_control_hash(hash,&c->pane->rule_state->row,sizeof(int));hash=viewer_control_hash(hash,&c->pane->rule_state->peer_write,sizeof(int));hash=viewer_control_hash(hash,&c->pane->rule_state->axis_page,sizeof(int));}
        for(dependent=0;dependent<GD_FORM_PRESENTATION;++dependent) hash=viewer_form_hash(hash,(GD_ExplorerForm)dependent,gd_explorer_form_applied(c,(GD_ExplorerForm)dependent));
    } else { hash=viewer_form_hash(hash,form,gd_explorer_form_pending(c,form,0)); hash=viewer_form_hash(hash,form,gd_explorer_form_applied(c,form)); }
    /* Actor canonical pickers/questions may consume last applied hypotheses. */
    if (form==GD_FORM_STRUCTURE && c->pane->view==GD_EXPLORER_ACTORS) hash=viewer_form_hash(hash,GD_FORM_QUERY,gd_explorer_form_applied(c,GD_FORM_QUERY));
    if (form==GD_FORM_QUERY) hash=viewer_control_hash(hash,&selected,sizeof(selected));
    snprintf(token,capacity,"%016llx",(unsigned long long)hash);
}
static void viewer_control_text(GD_ViewerWriter *w,GD_ExplorerContext *c,GD_ExplorerForm form,const GD_ExplorerControl *field,const void *values) {
    char text[GD_MAX_RULE_LINE+1]; int available=values ? gd_explorer_control_value(c,form,field,values,text,field->type==GD_CONTROL_TEXT ? sizeof(text):GD_MAX_PATH):0;
    gd_viewer_quote(w,available==1 ? text:NULL);
}
static void viewer_controls(ExplorerViewer *viewer,const GD_ViewerRequest *request,GD_ExplorerContext *c,GD_ViewerWriter *w,int configured) {
    union { GD_ExplorerQueryOptions query; GD_ExplorerStructureForm structure; GD_ExplorerRuleForm rules; GD_ExplorerResourceForm resource; GD_ExplorerPresentation presentation; } defaults;
    int form,written=0,want=*request->form ? viewer_form_kind(request->form):-1;
    if ((*request->form && want<0) || (want>=0 && (!gd_explorer_form_available(c,(GD_ExplorerForm)want) || (*request->control && !gd_explorer_form_field((GD_ExplorerForm)want,request->control))))) {
        gd_viewer_error(w,"viewer.control","Unknown/unavailable form or control; read viewer controls for this projection"); return;
    }
    gd_viewer_write(w,"{\"ok\":true,\"configured\":%s,\"application_applied\":%s,",configured ? "true":"false",configured && want==GD_FORM_PRESENTATION ? "true":"false"); viewer_member(w,"projection",viewer_projections[c->pane->view]);
    gd_viewer_write(w,",\"reload_generation\":\"%llu\",\"draft_revision\":%u,\"selector_key_limit_bytes\":%u,\"value_encoding\":\"CLI text: true/false, decimal integers, exact choice keys or literal text\",\"forms\":[",(unsigned long long)viewer->reload_generation,c->project->revision,(unsigned)(GD_MAX_PATH-1));
    for (form=0;form<GD_FORM_COUNT;++form) {
        GD_ExplorerForm kind=(GD_ExplorerForm)form; const void *pending,*applied; const GD_ExplorerControl *fields; int i,count,items=0; char token[64];
        if ((want>=0 && want!=form) || !gd_explorer_form_available(c,kind)) continue;
        pending=gd_explorer_form_pending(c,kind,0); applied=gd_explorer_form_applied(c,kind);
        if(kind==GD_FORM_PRESENTATION) { gd_explorer_presentation_read(c,&defaults.presentation); pending=applied=&defaults.presentation; }
        viewer_control_token(viewer,c,kind,token,sizeof(token));
        if (written++) gd_viewer_write(w,","); gd_viewer_write(w,"{"); viewer_member(w,"name",gd_explorer_form_name(kind)); gd_viewer_write(w,","); viewer_member(w,"control_token",token);
        gd_viewer_write(w,",\"initialized\":%s,\"pending\":",pending ? "true":"false");
        if (!pending) { gd_explorer_form_defaults(kind,&defaults); pending=&defaults; }
        viewer_form(w,kind,pending); gd_viewer_write(w,",\"applied\":"); viewer_form(w,kind,applied);
        if (kind==GD_FORM_PRESENTATION) gd_viewer_write(w,",\"apply_policy\":\"immediate; same native operation as GUI\"");
        if (kind==GD_FORM_RULES && !((const GD_ExplorerRuleForm *)pending)->bindings && ((const GD_ExplorerRuleForm *)pending)->kind==GD_NAMED_CONSISTENCY) gd_viewer_write(w,",\"fixed_work_limit\":%d",GD_RULE_ANALYSIS_WORK_LIMIT);
        if (kind==GD_FORM_STRUCTURE) { gd_viewer_write(w,",\"canonical_choices_ready\":%s,\"choice_discovery_question\":",gd_explorer_structure_choices(c) ? "true":"false"); gd_viewer_quote(w,gd_structure_question_name(GD_STRUCTURE_COUNT)); }
        gd_viewer_write(w,",\"fields\":["); fields=gd_explorer_form_fields(kind,&count);
        for (i=0;i<count;++i) {
            const GD_ExplorerControl *field=&fields[i]; int slots=gd_explorer_control_option_count(c,kind,field,pending),editable=gd_explorer_control_editable(c,kind,field,pending);
            if (*request->control && strcmp(request->control,field->name)) continue;
            if (items++) gd_viewer_write(w,","); gd_viewer_write(w,"{"); viewer_member(w,"name",field->name); gd_viewer_write(w,","); viewer_member(w,"label",field->label);
            gd_viewer_write(w,","); viewer_member(w,"type",field->type==GD_CONTROL_TEXT ? "text":field->type==GD_CONTROL_BOOL ? "boolean":slots>=0 ? "choice":"integer");
            gd_viewer_write(w,",\"editable\":%s,\"value\":",editable ? "true":"false"); viewer_control_text(w,c,kind,field,pending);
            gd_viewer_write(w,",\"applied_value\":"); viewer_control_text(w,c,kind,field,applied);
            gd_viewer_write(w,",\"choice_slots\":%d,\"choices_closed\":%s",slots,slots>=0 && field->type!=GD_CONTROL_TEXT ? "true":"false");
            if (field->type==GD_CONTROL_TEXT) gd_viewer_write(w,",\"capacity_bytes\":%u,\"validation\":\"native Run validates literal lists/selectors; choices are suggestions\"",(unsigned)(field->capacity-1));
            if (editable && field->type==GD_CONTROL_INT && slots<0) gd_viewer_write(w,",\"minimum\":0,\"maximum\":%d",kind==GD_FORM_PRESENTATION && (!strcmp(field->name,"overview_row") || !strcmp(field->name,"overview_column")) ? GD_OVERVIEW_BINS-1:gd_explorer_form_work_limit(kind,pending));
            if (!configured && *request->control && slots>=0) {
                unsigned slot,end=request->offset+request->limit; int options=0;
                if (end>(unsigned)slots) end=(unsigned)slots;
                gd_viewer_write(w,",\"offset\":%u,\"choices\":[",request->offset);
                for (slot=request->offset;slot<end;++slot) {
                    int native,available; char key[GD_MAX_PATH]; available=gd_explorer_control_option(c,kind,field,pending,(int)slot,&native,key,sizeof(key));
                    if (!available) continue;
                    if (options++) gd_viewer_write(w,","); gd_viewer_write(w,"{\"slot\":%u,\"key\":",slot); gd_viewer_quote(w,available==1 ? key:NULL); gd_viewer_write(w,",\"identity_available\":%s}",available==1 ? "true":"false");
                }
                gd_viewer_write(w,"],\"next_offset\":"); if (end<(unsigned)slots) gd_viewer_write(w,"%u",end); else gd_viewer_write(w,"null");
            }
            gd_viewer_write(w,"}");
        }
        gd_viewer_write(w,"]}");
    }
    gd_viewer_write(w,"],\"scope\":\"loaded/applied source scope; choice pages scan source slots, may be empty, are not frozen transactions\",\"source_draft_edits\":false}");
}
#endif
