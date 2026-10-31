/* Included in structural UI TU only for owned-window checks. */
int gd_explorer_structure_picker_checks(GD_ExplorerContext *context) {
    GD_ExplorerPane *pane=context->pane; GD_ExplorerStructureState *before=pane->structure_state,*state=NULL,*restricted=NULL;
    const GD_StructureResult *choices=NULL; GD_Model *model=gd_explorer_model(context->project,pane->view); int errors=model->error_count,ok=0;
    char identity[GD_MAX_PATH],message[256],features[128]=" source-status=2 \nsource-loaded-role";
#define PICKER_CHECK(expression) do { if (!(expression)) { fprintf(stderr,"Picker regression failed at line %d: %s\n",__LINE__,#expression); goto cleanup; } } while (0)
    state=empty_state(context->project->allocator); PICKER_CHECK(state); pane->structure_state=state;
    choices=picker_context(context); PICKER_CHECK(choices && choices->layout && choices->query && !state->result && !state->has_applied && model->error_count==errors);
    PICKER_CHECK(picker_context(context)==choices);
    if (pane->view==GD_EXPLORER_MDA && choices->layout->node_count && model->record_count>1) {
        GD_Layout *layout=(GD_Layout *)choices->layout; int old=layout->nodes[0].key,new_key=model->record_count-1,clicked=pane->mda.record,identity_ok,selected;
        layout->nodes[0].key=new_key; pane->mda.record=new_key;
        identity_ok=node_id(choices,0,identity,sizeof(identity)) && !strcmp(identity,model->records[new_key].reference); selected=picker_selected_node(context,choices);
        layout->nodes[0].key=old; pane->mda.record=clicked;
        PICKER_CHECK(identity_ok && selected==0);
    }
    if (pane->view==GD_EXPLORER_RELATIONSHIPS && model->record_count) {
        GD_ExplorerStructureForm form; defaults(&form); snprintf(form.scope_root,sizeof(form.scope_root),"%s",model->records[0].reference);
        restricted=build_state(context->project,pane->view,&form,NULL,message,sizeof(message)); PICKER_CHECK(restricted); pane->structure_state=restricted;
        { const GD_StructureResult *all=picker_context(context); PICKER_CHECK(all && all!=restricted->result && !all->options.scope_root && all->layout->node_count==choices->layout->node_count); }
    }
    PICKER_CHECK(picker_feature_update(features,sizeof(features),"source-status",0.5,0) && !strcmp(features,"source-status=0.5\nsource-loaded-role"));
    PICKER_CHECK(picker_feature_update(features,sizeof(features),"source-status",0,1) && !strcmp(features,"source-loaded-role"));
    PICKER_CHECK(!picker_feature_update(features,sizeof(features),"source-loaded-role",-1,0) && !strcmp(features,"source-loaded-role"));
    PICKER_CHECK(!picker_feature_update(features,4,"source-loaded-role",1,0) && !strcmp(features,"source-loaded-role")); ok=1;
cleanup:
    gd_explorer_structure_destroy(restricted); gd_explorer_structure_destroy(state); pane->structure_state=before; return ok;
#undef PICKER_CHECK
}
