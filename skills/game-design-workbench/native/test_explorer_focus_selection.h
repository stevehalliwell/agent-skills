/* Graphics-free checks of production camera operation, not pixel acceptance. */
#ifndef TEST_EXPLORER_FOCUS_SELECTION_H
#define TEST_EXPLORER_FOCUS_SELECTION_H
static int focus_selection_checks(GD_ExplorerContext *base,GD_ExplorerPane *panes,int *calls,int *fail_at) {
    GD_Allocator a=base->project->allocator;GD_ExplorerContext c=*base;GD_ExplorerPane *saved=a.allocate(a.context,sizeof(*saved)),*p=NULL;
    GD_Layout *probe=a.allocate(a.context,sizeof(*probe)),*before=a.allocate(a.context,sizeof(*before));GD_RenderPose *pose=a.allocate(a.context,sizeof(*pose));GD_LayoutFocus *distance=NULL;int result=0,checks=0,v,i,saving=0;
#define FOCUS_CHECK(value) do{++checks;if(!(value)){fprintf(stderr,"Focus selection check failed: %d: %s\n",__LINE__,#value);goto cleanup;}}while(0)
    FOCUS_CHECK(saved && probe && before && pose);
    for(v=0;v<GD_EXPLORER_VIEWS;++v)if(base->project->available&(1u<<v)){
        GD_ExplorerSelection selection;Camera2D camera;GD_ExplorerPresentation inputs,after;int allocation_count=*calls;
        p=&panes[v];memcpy(saved,p,sizeof(*saved));saving=1;c.pane=p;c.display_layout=NULL;
        p->selection=(GD_ExplorerSelection){-1,-1,0,-1,-1};p->mda.record=p->mda.link=-1;p->grid.noun=p->grid.verb=-1;p->overview.row=p->overview.column=-1;
        FOCUS_CHECK(!gd_explorer_focus_available(&c));camera=p->camera;FOCUS_CHECK(!gd_explorer_focus_selection(&c) && !memcmp(&camera,&p->camera,sizeof(camera)));
        if(p->layout->node_count){
            p->selection.node=0;if(v==GD_EXPLORER_MDA){p->mda.record=p->layout->nodes[0].key;for(i=0;i<4;++i)p->mda.roles[i]=true;p->mda.only_scope=p->mda.only_path=false;*p->mda.filter=0;}
            if(v==GD_EXPLORER_RESOURCES){p->resource_only_witness=false;for(i=0;i<4;++i)p->mda.roles[i]=true;*p->filter=0;}
            memcpy(before,p->layout,sizeof(*before));selection=p->selection;gd_explorer_presentation_read(&c,&inputs);
            FOCUS_CHECK(gd_explorer_focus_selection(&c) && p->focus_result.selected>0 && p->camera.zoom>=1 && p->camera.zoom<=1.2f);
            gd_explorer_presentation_read(&c,&after);FOCUS_CHECK(!memcmp(&inputs,&after,sizeof(inputs)) && !memcmp(before,p->layout,sizeof(*before)) && !memcmp(&selection,&p->selection,sizeof(selection)) && *calls==allocation_count);
            if(v!=GD_EXPLORER_MDA && focus_source_record(&c,0)>=0){p->selection=(GD_ExplorerSelection){-1,-1,0,focus_source_record(&c,0),-1};FOCUS_CHECK(gd_explorer_focus_selection(&c) && p->focus_result.selected>0 && !memcmp(before,p->layout,sizeof(*before)));}
            if(v==GD_EXPLORER_MATRIX){const GD_Model *model=gd_explorer_model(c.project,GD_EXPLORER_MATRIX);for(i=0;i<model->cell_count && !model->cells[i].pair_count;++i){}if(i<model->cell_count){p->selection=(GD_ExplorerSelection){-1,-1,0,model->pairs[model->cells[i].first_pair].interaction,-1};FOCUS_CHECK(gd_explorer_focus_selection(&c) && p->focus_result.selected>0 && !memcmp(before,p->layout,sizeof(*before)));}}
        }
        memcpy(p,saved,sizeof(*saved));saving=0;
    }
    {
        const GD_Model *model=gd_explorer_model(c.project,GD_EXPLORER_MATRIX);const GD_Cell *cell=&model->cells[0];p=&panes[GD_EXPLORER_MATRIX];memcpy(saved,p,sizeof(*saved));saving=1;c.pane=p;
        memset(probe,0,sizeof(*probe));probe->mode=GD_LAYOUT_FORCE;probe->node_count=2;probe->edge_count=1;probe->nodes[0].key=cell->noun;probe->nodes[1].key=cell->verb;probe->nodes[1].x=300;probe->edges[0].b=1;
        p->layout=probe;p->layout_choice=GD_LAYOUT_FORCE;p->overlay=false;p->selection=(GD_ExplorerSelection){0,-1,0,-1,-1};
        FOCUS_CHECK(gd_explorer_focus_selection(&c) && p->focus_result.selected==2 && p->focus_result.hidden_link && p->focus_result.status==GD_FOCUS_PARTIAL && !p->overlay && !probe->edges[0].active);
        memcpy(p,saved,sizeof(*saved));saving=0;
    }
    p=&panes[GD_EXPLORER_MDA];memcpy(saved,p,sizeof(*saved));saving=1;c.pane=p;c.display_layout=NULL;
    FOCUS_CHECK(p->layout->node_count>=4);memset(probe,0,sizeof(*probe));probe->node_count=4;probe->edge_count=3;probe->group_count=1;probe->mode=GD_LAYOUT_FORCE;
    for(i=0;i<4;++i){probe->nodes[i]=p->layout->nodes[i];probe->nodes[i].group=0;probe->nodes[i].x=probe->nodes[i].y=0;probe->nodes[i].footprint=(GD_LayoutBounds){-121.0f/3,-16,121.0f/3,16};p->mda.roles[i]=true;}
    probe->nodes[1].x=80;probe->nodes[2].x=probe->nodes[2].y=10000;probe->nodes[3].y=65;
    probe->edges[0]=(GD_LayoutEdge){0};probe->edges[0].a=0;probe->edges[0].b=1;probe->edges[0].key=0;probe->edges[0].active=1;
    probe->edges[1]=probe->edges[0];probe->edges[1].a=1;probe->edges[1].b=2;probe->edges[1].key=1;
    probe->edges[2]=probe->edges[0];probe->edges[2].b=3;probe->edges[2].key=2;
    p->layout=probe;p->mda.record=probe->nodes[0].key;p->mda.link=-1;p->mda.only_scope=p->mda.only_path=false;*p->mda.filter=0;
    memcpy(before,probe,sizeof(*before));
    FOCUS_CHECK(gd_explorer_focus_selection(&c) && p->focus_result.selected==1 && p->focus_result.context==2 && !p->focus_result.clipped && p->camera.zoom>=1);
    FOCUS_CHECK(fabsf(p->camera.target.x-120)<0.01f && fabsf(p->camera.target.y-97.5f)<0.01f && !memcmp(before,probe,sizeof(*before)));
    /* Former full-fit behavior shrinks even though selection is one card. */
    fit_view(&c,0);FOCUS_CHECK(p->camera.zoom<1);probe->nodes[1].y=2000;
    FOCUS_CHECK(gd_explorer_focus_selection(&c) && p->focus_result.clipped && p->camera.zoom>=1 && p->camera.target.x==0 && p->camera.target.y==0);
    {
        Camera2D camera=p->camera;for(i=0;i<4;++i)p->mda.roles[i]=false;
        FOCUS_CHECK(gd_explorer_focus_selection(&c) && p->focus_result.status==GD_FOCUS_HIDDEN && p->focus_result.hidden==1 && !memcmp(&camera,&p->camera,sizeof(camera)));
        p->mda.record=base->project->models[GD_EXPLORER_MDA_DATA]->record_count+1;
        FOCUS_CHECK(gd_explorer_focus_selection(&c) && p->focus_result.status==GD_FOCUS_UNREPRESENTED && !memcmp(&camera,&p->camera,sizeof(camera)));
    }
    for(i=0;i<4;++i)p->mda.roles[i]=true;p->mda.record=-1;p->mda.link=0;
    FOCUS_CHECK(gd_explorer_focus_selection(&c) && p->focus_result.selected==2 && p->focus_result.clipped && p->camera.target.x==0 && p->camera.target.y==0);
    /* Presentation pose can differ; neither pose is modified by framing. */
    p->mda.record=probe->nodes[0].key;p->mda.link=-1;probe->edge_count=0;memcpy(before,probe,sizeof(*before));before->nodes[0].x=500;before->nodes[0].y=-400;c.display_layout=before;
    {
        uint64_t raw=viewer_control_hash(UINT64_C(1469598103934665603),probe,sizeof(*probe)),shown=viewer_control_hash(UINT64_C(1469598103934665603),before,sizeof(*before));int allocation_count=*calls;
        *fail_at=*calls+1;FOCUS_CHECK(gd_explorer_focus_selection(&c) && *calls==allocation_count);*fail_at=-1;
        FOCUS_CHECK(p->camera.target.x==1500 && p->camera.target.y==-1200 && viewer_control_hash(UINT64_C(1469598103934665603),probe,sizeof(*probe))==raw && viewer_control_hash(UINT64_C(1469598103934665603),before,sizeof(*before))==shown);
    }
    c.display_layout=NULL;c.render_pose=pose;p->visual_filter=true;p->distance_focus_enabled=false;p->distance_focus=NULL;memset(pose,0,sizeof(*pose));
    FOCUS_CHECK(gd_render_pose_update(pose,probe,render_identity(&c),-1,1.0/60,1));pose->layout.nodes[0].x=500;pose->layout.nodes[0].y=-400;
    {
        uint64_t raw=viewer_control_hash(UINT64_C(1469598103934665603),probe,sizeof(*probe)),state=viewer_control_hash(UINT64_C(1469598103934665603),pose,sizeof(*pose));
        FOCUS_CHECK(gd_explorer_focus_selection(&c) && p->camera.target.x==1500 && p->camera.target.y==-1200 && viewer_control_hash(UINT64_C(1469598103934665603),pose,sizeof(*pose))==state && viewer_control_hash(UINT64_C(1469598103934665603),probe,sizeof(*probe))==raw);
        pose->identity=&panes[GD_EXPLORER_SKILLS];FOCUS_CHECK(gd_explorer_focus_selection(&c) && p->camera.target.x==0 && p->camera.target.y==0);
    }
    {
        Camera2D camera=p->camera;c.viewport.width=0;FOCUS_CHECK(!gd_explorer_focus_selection(&c) && !memcmp(&camera,&p->camera,sizeof(camera)));
    }
    c.viewport=base->viewport;c.display_layout=NULL;c.render_pose=NULL;distance=gd_layout_focus_create(a);FOCUS_CHECK(distance);
    p->distance_focus=distance;p->distance_focus_enabled=true;p->focus_auto_fit=true;p->distance_focus_revision=UINT64_MAX;gd_layout_focus_capture(distance,probe);
    {Camera2D camera;int selected=analyze(&c);FOCUS_CHECK(gd_explorer_focus_selection(&c) && p->focus_camera_override);camera=p->camera;distance_focus_update(&c,selected);FOCUS_CHECK(distance->active && !memcmp(&camera,&p->camera,sizeof(camera)));}
    memcpy(p,saved,sizeof(*saved));saving=0;result=checks;
cleanup:
    *fail_at=-1;if(saving)memcpy(p,saved,sizeof(*saved));base->analysis->initialized=0;if(distance)gd_layout_focus_destroy(distance);if(pose)a.release(a.context,pose);if(before)a.release(a.context,before);if(probe)a.release(a.context,probe);if(saved)a.release(a.context,saved);
#undef FOCUS_CHECK
    return result;
}
#endif
