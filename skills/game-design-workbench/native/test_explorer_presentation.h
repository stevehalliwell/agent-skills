/* CPU-only production IPC investigation, with independently owned panes/cache. */
#ifndef TEST_EXPLORER_PRESENTATION_H
#define TEST_EXPLORER_PRESENTATION_H
#include <float.h>
#include "test_explorer_focus_selection.h"
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
    if(!assembled)return 0;strcpy(request.command,"result");strcpy(request.projection,viewer_projections[view]);snprintf(request.form,sizeof(request.form),"%s",form);snprintf(request.fields,sizeof(request.fields),"%s",fields ? fields:"");request.limit=64; /* KiB: inspect complete payload with fewer serializations. */
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
#include "test_layout_reference.h"
static int viewer_presentation_checks(ExplorerViewer *original,int *fail_at,int *calls,char *reply,int run_layout_research) {
    GD_Allocator a=original->context->project->allocator;GD_ExplorerContext c=*original->context,target;GD_ExplorerPane *panes=NULL;ExplorerViewer viewer={0};GD_ViewerRequest request={0};GD_ViewerWriter w;
    int checks=0,result=0,active=GD_EXPLORER_MATRIX,stale=0,v,before,i;char decoded[65537],token[64],key[GD_MAX_PATH];
    GD_GraphAnalysis *analysis=NULL;GD_SkillUnlocks *knowledge=NULL;
#define PRESENT_CHECK(value) do {++checks;if(!(value)){fprintf(stderr,"Presentation check failed: %s (line %d)\n",#value,__LINE__);goto cleanup;}}while(0)
#define PRESENT_SEND() do {w=(GD_ViewerWriter){reply,0,GD_VIEWER_REPLY_LIMIT,0};viewer_dispatch(&viewer,&request,&w);}while(0)
#define PRESENT_REQUEST(command_,view_) do {memset(&request,0,sizeof(request));strcpy(request.command,command_);strcpy(request.projection,viewer_projections[view_]);request.limit=64;target=c;target.pane=&panes[view_];}while(0)
#define PRESENT_STAGE(view_,field_,value_) do {PRESENT_REQUEST("configure",view_);strcpy(request.form,"presentation");strcpy(request.control,field_);snprintf(request.value,sizeof(request.value),"%s",value_);viewer_control_token(&viewer,&target,GD_FORM_PRESENTATION,request.expect,sizeof(request.expect));PRESENT_SEND();}while(0)
#define PRESENT_ACTION(view_,action_) do {PRESENT_REQUEST("action",view_);strcpy(request.action,action_);viewer_control_token(&viewer,&target,GD_FORM_PRESENTATION,request.expect,sizeof(request.expect));PRESENT_SEND();}while(0)
    {
        GD_CountRange counts={2,10};Color low=gd_count_color(counts,2),high=gd_count_color(counts,10),blank=gd_count_color(counts,0);
        PRESENT_CHECK(low.r==32 && low.g==48 && low.b==82 && high.r==24 && high.g==112 && high.b==96 && blank.r==29 && blank.g==38 && blank.b==51);
        counts=(GD_CountRange){7,7};low=gd_count_color(counts,7);
        PRESENT_CHECK(low.r==28 && low.g==80 && low.b==89 && low.a==255);
        counts=(GD_CountRange){0};low=gd_count_color(counts,0);PRESENT_CHECK(!memcmp(&low,&blank,sizeof(low)));
    }
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
    for(i=1;i<=3;i+=2){float scale=(float)i;GD_LayoutNode raw={.x=-10.4f,.y=20.6f},shown={.x=110.9f,.y=-4.6f},expected=raw;
        gd_layout_set_footprint(&raw,-1,-1,1,1);expected=raw;
        gd_graph_footprint_rect(&expected,(Rectangle){(int)(raw.x*scale)+36,(int)(raw.y*scale)-8,120,14},scale);
        gd_graph_footprint_rect_at(&raw,&shown,(Rectangle){(int)(shown.x*scale)+36,(int)(shown.y*scale)-8,120,14},scale);
        PRESENT_CHECK(!memcmp(&raw,&expected,sizeof(raw)));
    }
    panes=a.allocate(a.context,GD_EXPLORER_VIEWS*sizeof(*panes));analysis=a.allocate(a.context,sizeof(*analysis));knowledge=a.allocate(a.context,sizeof(*knowledge));PRESENT_CHECK(panes && analysis && knowledge);
    memset(panes,0,GD_EXPLORER_VIEWS*sizeof(*panes));memset(analysis,0,sizeof(*analysis));memset(knowledge,0,sizeof(*knowledge));
    for(v=0;v<GD_EXPLORER_VIEWS;++v)pane_defaults(&panes[v],(GD_ExplorerView)v);
    c.pane=&panes[active];c.analysis=analysis;c.query=knowledge;c.viewport=(Rectangle){0,0,1000,700};viewer.context=&c;viewer.panes=panes;viewer.active=&active;viewer.stale=&stale;viewer.reload_generation=19;
    for(v=0;v<GD_EXPLORER_VIEWS;++v)if(c.project->available&(1u<<v)) {
        PRESENT_REQUEST("controls",v);strcpy(request.form,"presentation");before=*calls;PRESENT_SEND();PRESENT_CHECK(!w.failed && strstr(reply,"\"presentation\"") && *calls==before && !panes[v].prepared);
        target=c;target.pane=&panes[v];PRESENT_CHECK(pane_prepare(&target));
        /* Comparative traces require research opt-in. Resource/noun gates
           below remain mandatory within deep mode; thresholds/steps unchanged. */
        if(run_layout_research && (v==GD_EXPLORER_MATRIX || v==GD_EXPLORER_RESOURCES || v==GD_EXPLORER_EFFECTS || v==GD_EXPLORER_NOUNS))PRESENT_CHECK(presentation_layout_reference(&target));
        if(free_graph(&panes[v]) && !grid_view(&panes[v])){
            GD_LayoutNode node=panes[v].layout->nodes[0];bool paused=panes[v].paused;GD_ExplorerSelection selection=panes[v].selection;
            PRESENT_STAGE(v,"workspace_bounds","true");PRESENT_CHECK(strstr(reply,"\"configured\":true") && panes[v].workspace_bounds && panes[v].layout->workspace.enabled==(panes[v].layout->node_count>0));
            PRESENT_CHECK(!memcmp(&node,&panes[v].layout->nodes[0],sizeof(node)) && paused==panes[v].paused && !memcmp(&selection,&panes[v].selection,sizeof(selection)));
            PRESENT_STAGE(v,"workspace_bounds","false");PRESENT_CHECK(strstr(reply,"\"configured\":true") && !panes[v].workspace_bounds && !panes[v].layout->workspace.enabled);
            PRESENT_STAGE(v,"visual_filter","false");PRESENT_CHECK(strstr(reply,"\"configured\":true") && !panes[v].visual_filter);
            PRESENT_STAGE(v,"visual_filter","true");PRESENT_CHECK(strstr(reply,"\"configured\":true") && panes[v].visual_filter);
            PRESENT_CHECK(!memcmp(&node,&panes[v].layout->nodes[0],sizeof(node)) && paused==panes[v].paused && !memcmp(&selection,&panes[v].selection,sizeof(selection)));
        }else{PRESENT_STAGE(v,"workspace_bounds","true");PRESENT_CHECK(strstr(reply,"viewer.control") && !panes[v].workspace_bounds);PRESENT_STAGE(v,"visual_filter","false");PRESENT_CHECK(strstr(reply,"viewer.control") && panes[v].visual_filter);}
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
        if(v==GD_EXPLORER_ACTORS && panes[v].query_state && panes[v].query_state->scene && panes[v].query_state->scene->overview) {
            GD_ExplorerQueryState *state=panes[v].query_state;GD_Query *query=state->query;GD_QueryScene *scene=state->scene;
            GD_ExplorerQueryOptions *saved=a.allocate(a.context,2*sizeof(*saved));int allocation;
            PRESENT_CHECK(saved!=NULL);saved[0]=panes[v].query_options;saved[1]=state->applied;
            PRESENT_CHECK(scene->overview && free_graph(&panes[v]) && layout_settles(&panes[v]));
            {GD_ExplorerPresentation values;gd_explorer_presentation_read(&target,&values);PRESENT_CHECK(values.layout==1 && !gd_explorer_presentation_editable(&target,"layout"));}
            PRESENT_CHECK(viewer_retained_smoke(&viewer,(GD_ExplorerView)v,"actor-overview","\"mode\":\"interactions\"",reply));
            snprintf(panes[v].query_options.contexts,sizeof(panes[v].query_options.contexts),"pending overview test");
            PRESENT_STAGE(v,"layout","native");PRESENT_CHECK(strstr(reply,"viewer.control") && state->scene==scene);
            for(allocation=1;allocation<=3;++allocation) {*fail_at=*calls+allocation;PRESENT_STAGE(v,"actor_overview","usage");*fail_at=-1;PRESENT_CHECK(strstr(reply,"viewer.control") && state->scene==scene && panes[v].actor_overview==GD_ACTOR_INTERACTIONS);}
            PRESENT_STAGE(v,"actor_overview","usage");PRESENT_CHECK(strstr(reply,"\"configured\":true") && state->scene!=scene && state->query==query && !free_graph(&panes[v]) && !layout_settles(&panes[v]));
            PRESENT_CHECK(!strcmp(panes[v].query_options.contexts,"pending overview test") && !memcmp(&saved[1],&state->applied,sizeof(saved[1])));
            PRESENT_CHECK(viewer_retained_smoke(&viewer,(GD_ExplorerView)v,"actor-overview","\"mode\":\"usage\"",reply));
            {
                GD_Layout *native=a.allocate(a.context,sizeof(*native));int node;GD_ExplorerSelection selection;
                PRESENT_CHECK(native);scene=state->scene;memcpy(native,&scene->layout,sizeof(*native));
                if(scene->layout.node_count)panes[v].selection.node=0;selection=panes[v].selection;
                PRESENT_CHECK(gd_explorer_presentation_editable(&target,"layout") && !gd_explorer_presentation_editable(&target,"initial_arrangement"));
                for(allocation=1;allocation<=1+(scene->layout.edge_count>0);++allocation){*fail_at=*calls+allocation;PRESENT_STAGE(v,"layout","free");*fail_at=-1;PRESENT_CHECK(strstr(reply,"viewer.control") && state->scene==scene && !scene->free_geometry && !memcmp(native,&scene->layout,sizeof(*native)));}
                PRESENT_STAGE(v,"layout","free");PRESENT_CHECK(strstr(reply,"\"configured\":true") && free_graph(&panes[v]) && layout_settles(&panes[v]) && state->scene==scene && state->query==query && !memcmp(&selection,&panes[v].selection,sizeof(selection)));
                PRESENT_STAGE(v,"initial_arrangement","neighborhood");PRESENT_CHECK(strstr(reply,"\"configured\":true") && panes[v].initial_arrangement==1);
                PRESENT_STAGE(v,"workspace_bounds","true");PRESENT_CHECK(strstr(reply,"\"configured\":true") && scene->layout.workspace.enabled==(scene->layout.node_count>0));
                PRESENT_STAGE(v,"visual_filter","false");PRESENT_CHECK(strstr(reply,"\"configured\":true") && !render_active(&target));
                PRESENT_STAGE(v,"visual_filter","true");PRESENT_CHECK(strstr(reply,"\"configured\":true") && render_active(&target));
                for(node=0;node<120;++node)gd_query_scene_step(scene,-1);
                PRESENT_CHECK(!strcmp(panes[v].query_options.contexts,"pending overview test") && !memcmp(&saved[1],&state->applied,sizeof(saved[1])) && viewer_retained_smoke(&viewer,(GD_ExplorerView)v,"actor-overview","\"mode\":\"usage\"",reply));
                PRESENT_STAGE(v,"layout","native");PRESENT_CHECK(strstr(reply,"\"configured\":true") && !free_graph(&panes[v]) && !layout_settles(&panes[v]) && !scene->free_geometry && !scene->layout.workspace.enabled && !render_active(&target) && !memcmp(&selection,&panes[v].selection,sizeof(selection)));
                for(node=0;node<scene->layout.node_count;++node)PRESENT_CHECK(scene->layout.nodes[node].x==native->nodes[node].x && scene->layout.nodes[node].y==native->nodes[node].y && scene->layout.nodes[node].vx==0 && scene->layout.nodes[node].vy==0);
                PRESENT_STAGE(v,"initial_arrangement","native");PRESENT_CHECK(strstr(reply,"viewer.control"));
                PRESENT_CHECK(state->scene==scene && state->query==query && !memcmp(&saved[1],&state->applied,sizeof(saved[1])));a.release(a.context,native);
            }
            PRESENT_STAGE(v,"actor_overview","interactions");PRESENT_CHECK(strstr(reply,"\"configured\":true") && free_graph(&panes[v]));
            panes[v].query_options=saved[0];a.release(a.context,saved);
        }
        if(v==GD_EXPLORER_NOUNS){
            const GD_NounGrid *grid=c.project->noun_grid;const GD_Model *model=grid->rules->model;
            PRESENT_CHECK(grid_view(&panes[v]) && !layout_settles(&panes[v]) && panes[v].layout->node_count==grid->noun_count && grid->pair_count>0);
            PRESENT_STAGE(v,"noun_row",model->records[grid->pairs[0].a].reference);PRESENT_CHECK(strstr(reply,"\"configured\":true"));
            PRESENT_STAGE(v,"noun_column",model->records[grid->pairs[0].b].reference);PRESENT_CHECK(strstr(reply,"\"configured\":true") && panes[v].selection.cell==0 && !panes[v].query_state);
            PRESENT_ACTION(v,"view.cell.fit");PRESENT_CHECK(strstr(reply,"\"executed\":true"));
            PRESENT_CHECK(viewer_result_reassemble(&viewer,(GD_ExplorerView)v,"noun-grid",NULL,reply) && strstr(reply,"distinct declared cases") && strstr(reply,"\"is_actor\":null"));
            PRESENT_REQUEST("evidence",v);strcpy(request.form,"noun-cases");PRESENT_SEND();PRESENT_CHECK(!w.failed && strstr(reply,"\"participants\":[") && strstr(reply,"\"requires\":"));
            {GD_Layout *last=panes[v].layout;int allocation;for(allocation=1;allocation<=2;++allocation){*fail_at=*calls+allocation;PRESENT_STAGE(v,"layout","free");*fail_at=-1;PRESENT_CHECK(strstr(reply,"viewer.control") && panes[v].layout==last && grid_view(&panes[v]) && panes[v].selection.cell==0);}}
            PRESENT_STAGE(v,"layout","free");PRESENT_CHECK(strstr(reply,"\"configured\":true") && free_graph(&panes[v]) && !grid_view(&panes[v]) && layout_settles(&panes[v]) && panes[v].selection.cell==0 && panes[v].layout->node_count==grid->noun_count && panes[v].layout->edge_count==grid->pair_count);
            {
                /* Both seeds: 4096 fixed steps; bound motion/drift over final 256.
                   Old Mario collapsed to 61-unit centers and drifted 34 units. */
                GD_Layout *probe=a.allocate(a.context,sizeof(*probe));GD_LayoutPoint *previous=a.allocate(a.context,GD_MAX_RECORDS*sizeof(*previous));int seed,valid=1;
                PRESENT_CHECK(probe && previous);
                for(seed=0;seed<2;++seed){int step,node;double square=0,maximum=0,first_x=0,first_y=0,last_x=0,last_y=0;float minimum=FLT_MAX;
                memcpy(probe,panes[v].layout,sizeof(*probe));if(seed)PRESENT_CHECK(gd_layout_seed_neighborhood(probe,a,1.4f));
                for(step=0;step<4096;++step){
                    if(step>=3840)for(node=0;node<probe->node_count;++node){previous[node]=(GD_LayoutPoint){probe->nodes[node].x,probe->nodes[node].y};if(step==3840){first_x+=previous[node].x;first_y+=previous[node].y;}}
                    gd_layout_step(probe,-1);
                    if(step>=3840)for(node=0;node<probe->node_count;++node){double dx=probe->nodes[node].x-previous[node].x,dy=probe->nodes[node].y-previous[node].y,movement=dx*dx+dy*dy;square+=movement;if(movement>maximum)maximum=movement;}
                }
                for(node=0;node<probe->node_count;++node){if(!isfinite(probe->nodes[node].x) || !isfinite(probe->nodes[node].y))valid=0;last_x+=probe->nodes[node].x;last_y+=probe->nodes[node].y;}
                for(node=0;node<probe->edge_count;++node){const GD_LayoutEdge *edge=&probe->edges[node];if(edge->a!=edge->b){const GD_LayoutNode *left=&probe->nodes[edge->a],*right=&probe->nodes[edge->b];minimum=fminf(minimum,hypotf(left->x-right->x,left->y-right->y));}}
                fprintf(stderr,"Noun force probe (%s): %d nodes / %d edges / min linked centers %.3f / tail RMS %.6f max %.6f / centroid drift %.6f / overlaps %lu / limited %d\n",seed ? "neighborhood":"native",probe->node_count,probe->edge_count,minimum,sqrt(square/(256*probe->node_count)),sqrt(maximum),hypot(last_x-first_x,last_y-first_y)/probe->node_count,presentation_footprint_overlaps(probe),probe->collision_limited);
                if(minimum<128 || square/(256*probe->node_count)>0.05*0.05 || maximum>0.2*0.2 || hypot(last_x-first_x,last_y-first_y)/probe->node_count>1 || presentation_footprint_overlaps(probe))valid=0;
                }
                a.release(a.context,previous);a.release(a.context,probe);PRESENT_CHECK(valid);
            }
            PRESENT_REQUEST("evidence",v);strcpy(request.form,"links");PRESENT_SEND();PRESENT_CHECK(!w.failed && strstr(reply,"\"case_count\":") && strstr(reply,"distinct declared cases") && presentation_token(reply,"selection_token",token,sizeof(token)) && presentation_token(reply,"key",key,sizeof(key)));
            PRESENT_REQUEST("select",v);strcpy(request.form,"link");strcpy(request.id,key);strcpy(request.expect,token);PRESENT_SEND();PRESENT_CHECK(strstr(reply,"\"selected\":true") && panes[v].selection.cell==0 && panes[v].grid.noun==grid->pairs[0].a && panes[v].grid.verb==grid->pairs[0].b);
            PRESENT_REQUEST("evidence",v);strcpy(request.form,"noun-cases");PRESENT_SEND();PRESENT_CHECK(!w.failed && strstr(reply,"\"participants\":["));
            PRESENT_STAGE(v,"layout","grid");PRESENT_CHECK(strstr(reply,"\"configured\":true") && grid_view(&panes[v]) && panes[v].selection.cell==0 && !layout_settles(&panes[v]));
            PRESENT_STAGE(v,"noun_row","__missing_noun__");PRESENT_CHECK(strstr(reply,"viewer.control") && panes[v].selection.cell==0);
            PRESENT_STAGE(v,"noun_case_page","4096");PRESENT_CHECK(strstr(reply,"viewer.control") && !panes[v].noun_case_page);
            PRESENT_ACTION(v,"view.rearrange");PRESENT_CHECK(strstr(reply,"\"executed\":true") && panes[v].selection.cell==0);
        }
        if(query_view((GD_ExplorerView)v) && panes[v].query_state && panes[v].query_state->scene && !panes[v].query_state->scene->overview) {
            GD_ExplorerQueryState *state=panes[v].query_state;GD_QueryScene *scene=state->scene;GD_Query *query=state->query;GD_EffectQuery *scope=state->selected;
            GD_ExplorerQueryOptions pending=panes[v].query_options,applied=state->applied;GD_ExplorerSelection selection=panes[v].selection;
            GD_Layout *native=a.allocate(a.context,sizeof(*native));int node,allocation,work=scene->work;
            PRESENT_CHECK(native!=NULL);memcpy(native,panes[v].layout,sizeof(*native));
            if(native->node_count) panes[v].selection.node=0;selection=panes[v].selection;
            snprintf(panes[v].query_options.contexts,sizeof(panes[v].query_options.contexts),"pending geometry-only test");
            PRESENT_CHECK(!free_graph(&panes[v]) && !layout_settles(&panes[v]));
            PRESENT_STAGE(v,"layout","free");PRESENT_CHECK(strstr(reply,"\"configured\":true") && free_graph(&panes[v]) && layout_settles(&panes[v]));
            for(node=0;node<native->node_count;++node) PRESENT_CHECK(scene->layout.nodes[node].x==native->nodes[node].x && scene->layout.nodes[node].y==native->nodes[node].y);
            PRESENT_STAGE(v,"workspace_bounds","true");PRESENT_CHECK(strstr(reply,"\"configured\":true") && panes[v].workspace_bounds && scene->layout.workspace.enabled==(scene->layout.node_count>0));
            PRESENT_STAGE(v,"initial_arrangement","neighborhood");PRESENT_CHECK(strstr(reply,"\"configured\":true") && panes[v].workspace_bounds);
            gd_query_scene_step(scene,-1);
            PRESENT_CHECK(state==panes[v].query_state && state->scene==scene && state->query==query && state->selected==scope && scene->work==work && !memcmp(&state->applied,&applied,sizeof(applied)));
            PRESENT_CHECK(!strcmp(panes[v].query_options.contexts,"pending geometry-only test") && !memcmp(&selection,&panes[v].selection,sizeof(selection)));
            PRESENT_STAGE(v,"layout","native");PRESENT_CHECK(strstr(reply,"\"configured\":true") && !free_graph(&panes[v]) && !layout_settles(&panes[v]) && !scene->free_geometry);
            for(node=0;node<native->node_count;++node) PRESENT_CHECK(scene->layout.nodes[node].x==native->nodes[node].x && scene->layout.nodes[node].y==native->nodes[node].y);
            panes[v].paused=true;
            for(allocation=1;allocation<=1+(native->edge_count>0)+(native->node_count>0);++allocation) {
                *fail_at=*calls+allocation;PRESENT_STAGE(v,"layout","free");*fail_at=-1;
                PRESENT_CHECK(strstr(reply,"viewer.control") && panes[v].paused && !free_graph(&panes[v]) && !scene->free_geometry && state->scene==scene && state->query==query && state->selected==scope);
            }
            panes[v].query_options=pending;panes[v].paused=false;
            PRESENT_STAGE(v,"layout","free");PRESENT_CHECK(strstr(reply,"\"configured\":true"));
            panes[v].query_options.weighted=!applied.weighted;
            target=c;target.pane=&panes[v];
            for(allocation=1;allocation<=2+(native->edge_count>0)+(native->node_count>0);++allocation) {
                int displayed;*fail_at=*calls+allocation;displayed=gd_explorer_query_display(&target);*fail_at=-1;
                PRESENT_CHECK(!displayed && state->scene==scene && state->query==query && state->selected==scope && free_graph(&panes[v]) && !memcmp(&state->applied,&applied,sizeof(applied)));
            }
            PRESENT_CHECK(gd_explorer_query_display(&target) && state->scene!=scene && state->query==query && state->selected==scope && free_graph(&panes[v]));
            if(v==GD_EXPLORER_EFFECTS && state->selected) {
                int goal=state->selected->selection.target;
                PRESENT_CHECK(gd_explorer_query_select_result(&target,goal) && state->query==query && free_graph(&panes[v]) && state->selected->selection.target==goal);
            }
            panes[v].query_options=state->applied;
            PRESENT_CHECK(gd_explorer_query_run(&target) && panes[v].query_state!=state && free_graph(&panes[v]) && panes[v].initial_arrangement==1);
            PRESENT_STAGE(v,"layout","native");PRESENT_CHECK(strstr(reply,"\"configured\":true") && !layout_settles(&panes[v]));
            panes[v].query_options=pending;panes[v].workspace_bounds=false;a.release(a.context,native);
        }
        if(v==GD_EXPLORER_NOUNS){PRESENT_STAGE(v,"filter","test source");PRESENT_CHECK(!strstr(reply,"\"configured\":true") && !*panes[v].filter && !gd_explorer_presentation_editable(&target,"paused"));}
        else {
            PRESENT_STAGE(v,"filter","test source");PRESENT_CHECK(strstr(reply,"\"configured\":true") && !strcmp(panes[v].filter,"test source") && active==GD_EXPLORER_MATRIX);
            PRESENT_SEND();PRESENT_CHECK(strstr(reply,"viewer.stale_controls")!=NULL);
            PRESENT_STAGE(v,"filter","");PRESENT_CHECK(strstr(reply,"\"configured\":true") && !*panes[v].filter);
        }
        PRESENT_ACTION(v,"view.fit");PRESENT_CHECK(strstr(reply,"\"executed\":true")!=NULL);
        PRESENT_REQUEST("evidence",v);strcpy(request.form,"links");before=*calls;PRESENT_SEND();PRESENT_CHECK(!w.failed && strstr(reply,"total_slots") && *calls==before);
        if(panes[v].layout && panes[v].layout->edge_count && !query_view((GD_ExplorerView)v)) {
            PRESENT_CHECK(presentation_token(reply,"selection_token",token,sizeof(token)) && presentation_token(reply,"key",key,sizeof(key)));
            PRESENT_REQUEST("select",v);strcpy(request.form,"link");strcpy(request.id,key);strcpy(request.expect,token);PRESENT_SEND();PRESENT_CHECK(strstr(reply,"\"selected\":true") && active==GD_EXPLORER_MATRIX);
            if(v==GD_EXPLORER_MDA){const GD_LayoutEdge *edge=&panes[v].layout->edges[0];PRESENT_CHECK(panes[v].mda.link==edge->key && panes[v].mda.record==gd_explorer_model(c.project,GD_EXPLORER_MDA)->edges[c.project->mda->links[edge->key].edge].source);}
            strcpy(request.expect,"stale");PRESENT_SEND();PRESENT_CHECK(strstr(reply,"viewer.stale_controls")!=NULL);
        }
        if(!query_view((GD_ExplorerView)v) && v!=GD_EXPLORER_NOUNS) {
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
    /* Stored selection alone did not establish visible continuity. Check the
       same endpoint mask and bounds consumed by drawing and camera code. */
    for(v=GD_EXPLORER_MATRIX;v<=GD_EXPLORER_NOUNS;++v)if(v==GD_EXPLORER_MATRIX || v==GD_EXPLORER_NOUNS){
        GD_ExplorerPane *p=&panes[v];const GD_Model *model=gd_explorer_model(c.project,(GD_ExplorerView)v);GD_Layout *raw=a.allocate(a.context,sizeof(*raw));
        GD_ExplorerSelection saved;int nodes[2],cell=0,noun,verb,mode,case_page=p->noun_case_page;
        PRESENT_CHECK(raw);PRESENT_STAGE(v,"layout","grid");PRESENT_CHECK(strstr(reply,"\"configured\":true"));
        if(v==GD_EXPLORER_MATRIX){for(cell=0;cell<model->cell_count && !model->cells[cell].pair_count;++cell){}PRESENT_CHECK(cell<model->cell_count);noun=model->cells[cell].noun;verb=model->cells[cell].verb;grid_choose(&target,p->grid.slot[noun],p->grid.slot[verb]);p->selection.interaction=model->cells[cell].pair_count-1;}
        else{noun=c.project->noun_grid->pairs[cell].a;verb=c.project->noun_grid->pairs[cell].b;noun_grid_choose(&target,noun,verb);}
        p->selection.linked=selected_record(&target);saved=p->selection;case_page=p->noun_case_page;
        for(mode=0;mode<4;++mode){const char *layout=mode==1 ? "grid":mode==2 && v==GD_EXPLORER_MATRIX ? "columns":"free";Rectangle bounds={0};
            PRESENT_STAGE(v,"layout",layout);PRESENT_CHECK(strstr(reply,"\"configured\":true") && !memcmp(&saved,&p->selection,sizeof(saved)) && p->noun_case_page==case_page && p->grid.noun==noun && p->grid.verb==verb);
            PRESENT_CHECK(pair_selection_nodes(&target,nodes)==(noun==verb ? 1:2));memcpy(raw,p->layout,sizeof(*raw));
            if(grid_view(p)){Rectangle area=v==GD_EXPLORER_NOUNS ? noun_grid_bounds(p,noun,verb):grid_bounds(&p->grid,noun,verb);grid_fit(&target,-1);PRESENT_CHECK(p->camera.zoom==1 && fabsf(p->camera.target.x-area.x-area.width/2)<0.01f && fabsf(p->camera.target.y-area.y-area.height/2)<0.01f);}
            else{PRESENT_CHECK(p->fit==GD_EXPLORER_FIT_SELECTION && pair_selection_bounds(&target,&bounds));fit_view(&target,GD_EXPLORER_FIT_SELECTION);PRESENT_CHECK(p->camera.zoom>0 && p->camera.zoom<=1.2f && bounds.width*p->camera.zoom<target.viewport.width && bounds.height*p->camera.zoom<target.viewport.height && fabsf(p->camera.target.x-bounds.x-bounds.width/2)<0.01f && fabsf(p->camera.target.y-bounds.y-bounds.height/2)<0.01f);PRESENT_ACTION(v,"view.cell.fit");PRESENT_CHECK(strstr(reply,"\"executed\":true") && p->fit==GD_EXPLORER_FIT_SELECTION);}
            PRESENT_ACTION(v,"view.selection.focus");PRESENT_CHECK(strstr(reply,"\"framing\":{") && p->focus_result.selected==(grid_view(p) ? 1:noun==verb ? 1:2) && p->camera.zoom>=1);
            PRESENT_CHECK(!memcmp(raw,p->layout,sizeof(*raw)) && !memcmp(&saved,&p->selection,sizeof(saved)));
        }
        /* Header/node selection maps by record key, never old layout offset. */
        p->selection=(GD_ExplorerSelection){-1,nodes[0],0,-1,-1};noun=p->layout->nodes[nodes[0]].key;
        PRESENT_STAGE(v,"layout","grid");PRESENT_CHECK(strstr(reply,"\"configured\":true") && p->layout->nodes[p->selection.node].key==noun);
        grid_fit(&target,-1);PRESENT_STAGE(v,"layout","free");PRESENT_CHECK(strstr(reply,"\"configured\":true") && p->layout->nodes[p->selection.node].key==noun && pair_selection_nodes(&target,nodes)==1);
        PRESENT_CHECK(!gd_explorer_action_available(&target,GD_ACTION_VIEW_CELL_FIT));a.release(a.context,raw);
    }
    {
        GD_ExplorerPane *p=&panes[GD_EXPLORER_MATRIX];const GD_Model *model=gd_explorer_model(c.project,GD_EXPLORER_MATRIX);int row,column,blank=0,nodes[2];bool overlay=p->overlay;
        PRESENT_STAGE(GD_EXPLORER_MATRIX,"layout","grid");
        for(row=0;row<p->grid.nouns && row<64 && !blank;++row)for(column=0;column<p->grid.verbs && column<64;++column)if(gd_matrix_grid_cell(model,p->grid.order[row],p->grid.order[p->grid.nouns+column])<0){grid_choose(&target,row,column);blank=1;break;}
        if(blank){int noun=p->grid.noun,verb=p->grid.verb;PRESENT_STAGE(GD_EXPLORER_MATRIX,"layout","free");PRESENT_CHECK(strstr(reply,"\"configured\":true") && p->selection.cell<0 && p->selection.node<0 && pair_selection_nodes(&target,nodes)==2 && p->grid.noun==noun && p->grid.verb==verb && p->overlay==overlay && p->layout->edge_count==model->cell_count);PRESENT_STAGE(GD_EXPLORER_MATRIX,"layout","grid");grid_fit(&target,-1);PRESENT_CHECK(p->selection.cell<0 && p->grid.noun==noun && p->grid.verb==verb);}
        for(i=0;i<model->cell_count;++i)if(!model->cells[i].active){int edge;grid_choose(&target,p->grid.slot[model->cells[i].noun],p->grid.slot[model->cells[i].verb]);p->overlay=false;PRESENT_STAGE(GD_EXPLORER_MATRIX,"layout","free");PRESENT_CHECK(strstr(reply,"\"configured\":true") && p->selection.cell==i && pair_selection_nodes(&target,nodes)==2 && !p->overlay);for(edge=0;edge<p->layout->edge_count && p->layout->edges[edge].key!=i;++edge){}PRESENT_CHECK(edge<p->layout->edge_count && !p->layout->edges[edge].active);PRESENT_ACTION(GD_EXPLORER_MATRIX,"view.selection.focus");PRESENT_CHECK(strstr(reply,"\"executed\":true") && p->focus_result.hidden_link && !p->overlay);p->overlay=overlay;break;}
    }
    {
        GD_ExplorerPane *source=&panes[GD_EXPLORER_MATRIX],*destination=&panes[GD_EXPLORER_NOUNS],*prior=c.pane;GD_ExplorerSelection first=source->selection,second=destination->selection;Camera2D camera=source->camera,other=destination->camera;
        const GD_Model *model=gd_explorer_model(c.project,GD_EXPLORER_MATRIX);const char *reference=model->records[source->grid.noun].reference;
        c.pane=source;enter_view(&c,panes,GD_EXPLORER_NOUNS,NULL);PRESENT_CHECK(!memcmp(&second,&destination->selection,sizeof(second)) && !memcmp(&other,&destination->camera,sizeof(other)));
        enter_view(&c,panes,GD_EXPLORER_MATRIX,NULL);PRESENT_CHECK(!memcmp(&first,&source->selection,sizeof(first)) && !memcmp(&camera,&source->camera,sizeof(camera)));
        enter_view(&c,panes,GD_EXPLORER_NOUNS,reference);matches(&c);PRESENT_CHECK(destination->visible_matches>0 && !strcmp(destination->find_reference,reference) && !memcmp(&second,&destination->selection,sizeof(second)) && !memcmp(&other,&destination->camera,sizeof(other)));
        enter_view(&c,panes,GD_EXPLORER_MATRIX,NULL);PRESENT_CHECK(!memcmp(&first,&source->selection,sizeof(first)));c.pane=prior;
    }
    {int focused=focus_selection_checks(&c,panes,calls,fail_at);PRESENT_CHECK(focused>30);checks+=focused;}
    {
        GD_ExplorerPane *p=&panes[GD_EXPLORER_MDA];int record=p->mda.record,link=p->mda.link;GD_ExplorerFocusResult previous=p->focus_result;
        p->mda.record=p->layout->nodes[0].key;p->mda.link=-1;target=c;target.pane=p;
        viewer_control_token(&viewer,&target,GD_FORM_PRESENTATION,token,sizeof(token));
        PRESENT_REQUEST("action",GD_EXPLORER_MDA);strcpy(request.action,"view.selection.focus");strcpy(request.expect,token);PRESENT_SEND();
        PRESENT_CHECK(strstr(reply,"\"executed\":true") && strstr(reply,"\"framing\":{") && strstr(reply,"\"context_nodes\":") && p->focus_result.status!=GD_FOCUS_NONE);
        viewer_control_token(&viewer,&target,GD_FORM_PRESENTATION,key,sizeof(key));PRESENT_CHECK(!strcmp(token,key));
        p->mda.record=record;p->mda.link=link;p->focus_result=previous;
    }
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
