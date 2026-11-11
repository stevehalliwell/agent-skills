/* Current-canvas native identity/discovery. No disk reads, preparation or query application. */
#ifndef GD_EXPLORER_VIEWER_NODES_H
#define GD_EXPLORER_VIEWER_NODES_H

typedef struct { int hints[GD_RESOURCE_GRAPH_NODES],work,limited; } ViewerIdentities;
static void viewer_identities(const GD_ExplorerContext *c, ViewerIdentities *ids) {
    int i; ids->work=ids->limited=0;
    if (c->pane->view!=GD_EXPLORER_RESOURCES || !c->project->resources) return;
    for (i=0;i<GD_RESOURCE_GRAPH_NODES;++i) ids->hints[i]=-1;
    for (i=0;i<c->project->resources->relation_count;++i) {
        int facet=c->project->resources->relations[i].facet;
        if (facet>=0 && facet<GD_RESOURCE_GRAPH_NODES && ids->hints[facet]<0) ids->hints[facet]=i;
    }
}
static int viewer_canvas_count(const GD_ExplorerContext *c) {
    if (!c->pane->prepared) return 0;
    if (gd_explorer_similarity_active(c)) {
        const GD_SimilarityProjection *projection=c->pane->structure_state->result->similarity;
        return projection->table_complete?projection->entry_count:0;
    }
    return c->pane->layout?c->pane->layout->node_count:0;
}
static int viewer_canvas_record(const GD_ExplorerContext *c, int node) {
    const GD_ExplorerPane *p=c->pane;
    if (node<0 || node>=viewer_canvas_count(c)) return -1;
    if (gd_explorer_similarity_active(c)) {
        const GD_ExplorerStructureState *state=p->structure_state;
        int entry=state->result->similarity->nodes[node];
        return gd_structure_result_record(state->result,state->result->layout->nodes[entry].key);
    }
    if (p->view==GD_EXPLORER_RESOURCES) return gd_explorer_resource_record(c->project->resources,p->layout->nodes[node].key);
    if (query_view(p->view)) return p->query_state && p->query_state->scene?gd_query_scene_record(p->query_state->scene,node):-1;
    return p->layout->nodes[node].key;
}
static int viewer_canvas_visible(GD_ExplorerContext *c, int node) {
    if (node<0 || node>=viewer_canvas_count(c)) return 0;
    if (gd_explorer_similarity_active(c)) {
        const GD_Similarity *map=c->pane->structure_state->result->similarity->map;
        return map && map->available && map->placed[node];
    }
    return visible(c,node);
}
static int viewer_canvas_id(const GD_ExplorerContext *c, int node, ViewerIdentities *ids, char *output, size_t capacity) {
    const GD_ExplorerPane *p=c->pane; const char *id=NULL; int record;
    if (!capacity) return 0; *output=0;
    if (node<0 || node>=viewer_canvas_count(c)) return 0;
    if (gd_explorer_similarity_active(c)) id=p->structure_state->result->similarity->ids[node];
    else if (p->view==GD_EXPLORER_RESOURCES) {
        int key=p->layout->nodes[node].key;
        return gd_resource_node_id(c->project->resources,key,ids->hints[key],output,capacity);
    } else if (query_view(p->view)) {
        const GD_QueryScene *scene=p->query_state?p->query_state->scene:NULL;
        if (!scene || !scene->query) return 0;
        if (scene->query->effects) return gd_effect_node_id(scene->query->effects,scene->nodes[node].source,output,capacity);
        if (!scene->overview && scene->nodes[node].kind!=GD_QUERY_NODE_OBJECT) {
            int okay=gd_actor_scene_node_id(scene,node,output,capacity,&ids->work,GD_ACTOR_WORK);
            if (!okay && ids->work>=GD_ACTOR_WORK) ids->limited=1;
            return okay;
        }
    }
    record=viewer_canvas_record(c,node);
    if (!id && record>=0) id=gd_explorer_model(c->project,p->view)->records[record].reference;
    if (!id || strlen(id)>=capacity) return 0;
    memcpy(output,id,strlen(id)+1); return 1;
}
static const char *viewer_canvas_kind(const GD_ExplorerContext *c, int node) {
    if (gd_explorer_similarity_active(c)) return "similarity-entry";
    if (query_view(c->pane->view) && c->pane->query_state && c->pane->query_state->scene)
        return gd_query_scene_node_kind_name(c->pane->query_state->scene->nodes[node].kind);
    if (c->pane->view==GD_EXPLORER_RESOURCES) {
        static const char *const names[]={"case","facet","role"};
        int kind=c->project->resources->nodes[c->pane->layout->nodes[node].key].kind;
        return kind>=0 && kind<3?names[kind]:"unavailable";
    }
    return "source-record";
}
static int viewer_selected_node(const GD_ExplorerContext *c) {
    const GD_ExplorerPane *p=c->pane; int i;
    if (gd_explorer_similarity_active(c)) return p->structure_state->similarity_row;
    if (p->view==GD_EXPLORER_MDA) {
        if (p->mda.link>=0) return -1;
        for (i=0;i<viewer_canvas_count(c);++i) if (p->layout->nodes[i].key==p->mda.record) return i;
        return -1;
    }
    return p->selection.node;
}
static void viewer_nodes(ExplorerViewer *viewer, const GD_ViewerRequest *request, GD_ExplorerContext *c, GD_ViewerWriter *w) {
    ViewerIdentities ids; GD_Model *model=gd_explorer_model(c->project,c->pane->view);
    int i,total=viewer_canvas_count(c),end=(int)request->offset+(int)(request->limit?request->limit:GD_VIEWER_NODE_PAGE);
    viewer_identities(c,&ids); if (end>total) end=total;
    gd_viewer_write(w,"{\"ok\":true,\"protocol\":1,"); viewer_member(w,"projection",viewer_projections[c->pane->view]);
    gd_viewer_write(w,","); viewer_member(w,"session",gd_viewer_session(viewer->server));
    gd_viewer_write(w,",\"reload_generation\":\"%llu\",\"draft_revision\":%u,\"stale\":%s,\"prepared\":%s,\"canvas\":",
        (unsigned long long)viewer->reload_generation,c->project->revision,*viewer->stale?"true":"false",c->pane->prepared?"true":"false");
    gd_viewer_quote(w,gd_explorer_similarity_active(c)?"similarity":"ordinary");
    gd_viewer_write(w,",\"available\":%s,\"pane_revision\":%u,\"total\":%d,\"offset\":%u,\"nodes\":[",c->pane->prepared && (gd_explorer_similarity_active(c)?c->pane->structure_state->result->similarity->table_complete:c->pane->layout!=NULL)?"true":"false",c->pane->revision,total,request->offset);
    for (i=(int)request->offset;i<end;++i) {
        char id[GD_MAX_PATH]; int record=viewer_canvas_record(c,i),available=viewer_canvas_id(c,i,&ids,id,sizeof(id));
        const char *label=gd_explorer_similarity_active(c)?c->pane->structure_state->result->similarity->ids[i]:c->pane->layout->nodes[i].label;
        if (i!=(int)request->offset) gd_viewer_write(w,",");
        gd_viewer_write(w,"{\"id\":"); gd_viewer_quote(w,available?id:NULL);
        gd_viewer_write(w,","); viewer_member(w,"label",label);
        gd_viewer_write(w,","); viewer_member(w,"kind",viewer_canvas_kind(c,i));
        gd_viewer_write(w,","); viewer_member(w,"source",record>=0?model->records[record].reference:NULL);
        if(record>=0 && model->records[record].is_noun)gd_viewer_write(w,",\"is_actor\":%s",!model->records[record].is_actor_line ? "null":model->records[record].is_actor ? "true":"false");
        gd_viewer_write(w,",\"visible\":%s,\"selected\":%s,\"snapshot_local\":%d,\"identity_available\":%s}",viewer_canvas_visible(c,i)?"true":"false",viewer_selected_node(c)==i?"true":"false",i,available?"true":"false");
    }
    if (ids.limited) { gd_viewer_error(w,"viewer.identity_limit","Native identity work limit reached; request narrower page"); return; }
    gd_viewer_write(w,"],\"next_offset\":");
    if (end<total) gd_viewer_write(w,"%d",end); else gd_viewer_write(w,"null");
    gd_viewer_write(w,",\"scope\":\"current prepared canvas only; no query execution, filter changes or absence proof\"}");
}
static int viewer_find_node(GD_ExplorerContext *c, const char *id, GD_ViewerWriter *w) {
    ViewerIdentities ids; int i,found=-1; char candidate[GD_MAX_PATH];
    viewer_identities(c,&ids);
    for (i=0;i<viewer_canvas_count(c);++i) if (viewer_canvas_id(c,i,&ids,candidate,sizeof(candidate)) && !strcmp(id,candidate)) {
        if (found>=0) { gd_viewer_error(w,"viewer.ambiguous_id","Native ID matches multiple current representations; no arbitrary selection"); return -1; }
        found=i;
    }
    if (ids.limited) { gd_viewer_error(w,"viewer.identity_limit","Native identity work limit reached; no navigation performed"); return -1; }
    if (found<0) gd_viewer_error(w,"viewer.node_missing","Exact native ID is absent from current prepared canvas; no source fallback or query application");
    else if (!viewer_canvas_visible(c,found)) { gd_viewer_error(w,"viewer.node_hidden","Exact node is hidden or unplaced; no filter/canvas changes performed"); found=-1; }
    return found;
}
static int viewer_canvas_current(const GD_ExplorerContext *c) {
    unsigned revision=gd_explorer_family(c->pane->view)==GD_EXPLORER_SKILL_DATA?c->project->revision:1;
    return c->pane->prepared && c->pane->revision==revision;
}
static int viewer_select_node(GD_ExplorerContext *c, int node, GD_ViewerWriter *w) {
    GD_ExplorerPane *p=c->pane; float width,height; Vector2 center;
    if (gd_explorer_similarity_active(c)) {
        if (gd_explorer_similarity_select(c,node)) return 1;
        gd_viewer_error(w,"viewer.node_hidden","Similarity entry is unavailable or unplaced"); return 0;
    }
    if (query_view(p->view) && p->query_state && p->query_state->scene && p->query_state->query->effects) {
        int source=p->query_state->scene->nodes[node].source;
        if (p->query_state->query->effects->nodes[source].kind!=GD_EFFECT_ACTIVATION) {
            if (!gd_explorer_query_select_result(c,source)) {
                gd_viewer_error(w,"viewer.select","Cannot prepare contributor context; previous selection/query retained"); return 0;
            }
            node=p->selection.node;
        } else p->selection=(GD_ExplorerSelection){-1,node,0,-1,-1};
    } else if (is_graph(p->view)) {
        /* Caller rejected pending rank input; never invoke implicit edit commit. */
        p->selection=(GD_ExplorerSelection){-1,node,0,-1,-1}; p->inspected_record=gd_explorer_graph_record(c);
    } else if (p->view==GD_EXPLORER_MDA) { p->mda.record=p->layout->nodes[node].key; p->mda.link=-1; }
    else p->selection=(GD_ExplorerSelection){-1,node,0,-1,-1};
    if (grid_view(p)) {
        GD_Model *model=gd_explorer_model(c->project,p->view); int record=p->layout->nodes[node].key;
        Rectangle area=grid_bounds(&p->grid,model->records[record].role==GD_NOUN?record:-1,model->records[record].role==GD_VERB?record:-1);
        center=(Vector2){area.x+area.width/2,area.y+area.height/2}; width=area.width; height=area.height;
    } else {
        center=position(c,node); width=fmaxf(250,gd_layout_node_width(&p->layout->nodes[node])); height=fmaxf(170,p->layout->nodes[node].height);
    }
    p->camera.target=center; p->camera.offset=(Vector2){c->viewport.x+c->viewport.width/2,c->viewport.y+c->viewport.height/2};
    p->camera.zoom=fmaxf(0.0001f,fminf(1.2f,fminf(c->viewport.width/(width+80),c->viewport.height/(height+80))));
    p->fit=0; return 1;
}
static void viewer_source(GD_ExplorerContext *c, GD_ViewerWriter *w) {
    GD_Model *model=gd_explorer_model(c->project,c->pane->view); int record=c->pane->prepared?selected_record(c):-1;
    if (!model || record<0 || record>=model->record_count) { gd_viewer_write(w,"null"); return; }
    gd_viewer_write(w,"{"); viewer_member(w,"reference",model->records[record].reference);
    gd_viewer_write(w,","); viewer_member(w,"label",model->records[record].display_name);
    if(model->records[record].is_noun)gd_viewer_write(w,",\"is_actor\":%s",!model->records[record].is_actor_line ? "null":model->records[record].is_actor ? "true":"false");
    gd_viewer_write(w,","); viewer_member(w,"path",model->records[record].source_path);
    gd_viewer_write(w,","); viewer_member(w,"status",model->records[record].status);
    gd_viewer_write(w,","); viewer_member(w,"body",model->records[record].body);
    gd_viewer_write(w,",\"body_line\":%d,\"body_scope\":\"loaded Markdown body, not full raw file; excludes unsaved frontmatter edits\",\"evidence_scope\":\"selected source only, not all contributing declarations\"}",model->records[record].body_line);
}
#endif
