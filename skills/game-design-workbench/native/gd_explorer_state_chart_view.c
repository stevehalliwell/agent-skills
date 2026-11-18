/* Read-only UML-style projection and joint-hypothesis grid. No simulation. */
#include "gd_explorer_controls.h"
#include "gd_count_heatmap.h"
#include "gd_graph_analysis_draw.h"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_NO_EXPORT
#include "cimgui.h"
#include "gd_explorer_picker.h"
#include <string.h>
#include <math.h>

enum { CHART_ARROWS_PER_PAGE=8,CHART_REGION_HEIGHT=420,CHART_NODE_STEP=198,
    CHART_GRID_LEFT=210,CHART_GRID_TOP=104,CHART_GRID_COLUMN=205,CHART_GRID_ROW=100 };
static const GD_Color chart_text={225,233,241,255},chart_muted={150,169,190,255},chart_gold={255,217,125,255},chart_blue={86,166,255,255};
static int chart_node(GD_ExplorerChartState *state,int region,const char *value,int add) {
    int i; for(i=0;i<state->node_count[region];++i)if(!strcmp(value,state->nodes[region][i]))return i;
    if(add && i<GD_ACTOR_WRITES+GD_ACTOR_VARIANTS){state->nodes[region][state->node_count[region]++]=value;return i;}
    return -1;
}
static void chart_cache(GD_ExplorerChartState *state) {
    const GD_Query *query=state->query;const GD_StateChart *chart=query->state_chart;int i,j,columns=query->request.companion_axis ? chart->companion_count:1;
    memset(state->cells,0,sizeof(state->cells));state->node_count[0]=state->node_count[1]=state->edge_count=0;
    for(i=0;i<query->state_coverage->value_count;++i)chart_node(state,0,query->state_coverage->values[i].value,1);
    for(i=0;i<chart->companion_count;++i)chart_node(state,1,chart->companion_values[i],1);
    for(i=0;i<query->state_coverage->value_count*columns;++i)state->cells[i].unfinished=query->binding_count;
    for(i=0;i<chart->scenario_count;++i){
        const GD_StateChartScenario *scenario=&chart->scenarios[i];int column=scenario->companion_value ? chart_node(state,1,scenario->companion_value,0):0;
        int cell=scenario->value*columns+column;
        if(column>=0 && cell<GD_ACTOR_VARIANTS*GD_ACTOR_VARIANTS){
            ++state->cells[cell].retained;
            if(!scenario->limited){--state->cells[cell].unfinished;if(scenario->applicability==GD_RULE_TRUTH_TRUE)++state->cells[cell].matching;else if(scenario->applicability==GD_RULE_TRUTH_UNKNOWN)++state->cells[cell].unknown;else ++state->cells[cell].excluded;}
        }
        for(j=0;j<scenario->write_count;++j){
            int index=scenario->first_write+j;const GD_RuleWrite *write=&chart->writes[index];int region=write->axis==query->request.axis ? 0:chart->companion_axis>=0 && write->axis==chart->companion_axis ? 1:-1;
            if(region<0 || write->actor!=query->bindings[scenario->binding].participant || (write->kind!=GD_WRITE_STATE && write->kind!=GD_WRITE_PRESERVE) || scenario->applicability==GD_RULE_TRUTH_FALSE)continue;
            if(write->kind==GD_WRITE_STATE)chart_node(state,region,write->state,1);
            state->edge_scenarios[state->edge_count]=i;state->edge_writes[state->edge_count++]=index;
        }
    }
}
int gd_explorer_chart_prepare(GD_ExplorerContext *context) {
    GD_ExplorerPane *pane=context->pane;GD_ExplorerChartState *state;
    if(pane->view!=GD_EXPLORER_STATES)return 0;if(pane->chart_state)return 1;
    state=context->project->allocator.allocate(context->project->allocator.context,sizeof(*state));if(!state)return 0;
    memset(state,0,sizeof(*state));state->allocator=context->project->allocator;state->row=state->column=state->scenario=-1;
    state->cameras[0].zoom=state->cameras[1].zoom=1;gd_explorer_form_defaults(GD_FORM_CHART,&state->pending);pane->chart_state=state;return 1;
}
void gd_explorer_chart_destroy(GD_ExplorerChartState *state) {
    if(!state)return;gd_query_destroy(state->query);state->allocator.release(state->allocator.context,state);
}
int gd_explorer_chart_run(GD_ExplorerContext *context) {
    GD_Model *model=gd_explorer_model(context->project,GD_EXPLORER_STATES);GD_ExplorerChartState *state;GD_Query *query=NULL;GD_QueryRequest request={0};int errors=model->error_count,omitted=model->errors_omitted;
    if(!gd_explorer_chart_prepare(context))return 0;state=context->pane->chart_state;
    request.kind=GD_QUERY_STATE_CHART;request.noun=gd_query_record(model,state->pending.object,GD_NOUN);request.axis=gd_query_record(model,state->pending.axis,GD_AXIS);request.effect_kind=request.case_index=-1;request.work_limit=state->pending.work_limit;
    if(request.noun<0 || request.axis<0){snprintf(context->pane->query_message,sizeof(context->pane->query_message),"Choose one exact noun and declared enum axis. Last applied result retained.");return 0;}
    if(*state->pending.case_name){request.case_index=gd_rule_case_find(model,context->project->rules,state->pending.case_name);if(request.case_index<0){snprintf(context->pane->query_message,sizeof(context->pane->query_message),"Case is missing or ambiguous. Last applied result retained.");return 0;}}
    if(*state->pending.participant)request.participant=state->pending.participant;
    if(*state->pending.companion_axis)request.companion_axis=state->pending.companion_axis;
    query=gd_query_build(model,context->project->rules,context->project->calls,&request);
    if(!query){snprintf(context->pane->query_message,sizeof(context->pane->query_message),"%s. Last applied result retained.",model->error_count>errors ? model->errors[errors].message:"Cannot allocate bounded state-chart result");model->error_count=errors;model->errors_omitted=omitted;return 0;}
    gd_query_destroy(state->query);state->applied=state->pending;state->query=query;
    query->request.participant=*state->applied.participant ? state->applied.participant:NULL;
    query->request.companion_axis=*state->applied.companion_axis ? state->applied.companion_axis:NULL;
    state->row=state->column=state->scenario=-1;state->page=state->arrow_page=state->proposal_page=state->activation_page=0;++state->generation;
    state->camera_ready[state->mode]=true;
    memset(state->participant_pages,0,sizeof(state->participant_pages));chart_cache(state);
    context->pane->selection=(GD_ExplorerSelection){-1,-1,0,-1,-1};context->pane->fit=1;
    snprintf(context->pane->query_message,sizeof(context->pane->query_message),"Applied native hypotheses. Conditional proposals, not runtime reachability; source/query incompleteness remains explicit.");
    model->error_count=errors;model->errors_omitted=omitted;return 1;
}
int gd_explorer_chart_remap(GD_ExplorerContext *context,const GD_ExplorerProject *old,const GD_ExplorerPane *prior) {
    GD_ExplorerChartState *state;if(!prior->chart_state)return 1;if(!gd_explorer_chart_prepare(context))return 0;state=context->pane->chart_state;
    state->pending=prior->chart_state->pending;state->mode=prior->chart_state->mode;
    if(prior->chart_state->query && !gd_explorer_chart_run(context))return 0;
    if(prior->chart_state->query){
        const GD_ExplorerChartState *previous=prior->chart_state;int i,found;
        if(previous->row>=0){found=-1;for(i=0;i<state->query->state_coverage->value_count;++i)if(!strcmp(previous->query->state_coverage->values[previous->row].value,state->query->state_coverage->values[i].value))found=i;if(found<0)goto missing;state->row=found;}
        if(previous->column>=0){found=state->query->request.companion_axis ? -1:0;for(i=0;i<state->query->state_chart->companion_count;++i)if(!strcmp(previous->query->state_chart->companion_values[previous->column],state->query->state_chart->companion_values[i]))found=i;if(found<0)goto missing;state->column=found;}
        if(previous->scenario>=0){
            const GD_StateChartScenario *selected=&previous->query->state_chart->scenarios[previous->scenario];const GD_QueryBinding *binding=&previous->query->bindings[selected->binding];const GD_RuleCase *root=&previous->query->rules->cases[binding->case_index];found=-1;
            for(i=0;i<state->query->state_chart->scenario_count;++i){const GD_StateChartScenario *candidate=&state->query->state_chart->scenarios[i];const GD_QueryBinding *local=&state->query->bindings[candidate->binding];const GD_RuleCase *case_=&state->query->rules->cases[local->case_index];
                if(strcmp(root->name,case_->name) || strcmp(previous->query->model->records[previous->query->rules->sources[root->source].record].reference,state->query->model->records[state->query->rules->sources[case_->source].record].reference) || strcmp(previous->query->rules->participants[binding->participant].name,state->query->rules->participants[local->participant].name) || strcmp(previous->query->state_coverage->values[selected->value].value,state->query->state_coverage->values[candidate->value].value))continue;
                if((selected->companion_value!=NULL)!=(candidate->companion_value!=NULL) || (selected->companion_value && strcmp(selected->companion_value,candidate->companion_value)))continue;found=i;break;
            }
            if(found<0)goto missing;state->scenario=found;
        }
    }
    if(prior->selection.linked>=0)context->pane->selection.linked=gd_explorer_record(gd_explorer_model(context->project,GD_EXPLORER_STATES),gd_explorer_model(old,GD_EXPLORER_STATES)->records[prior->selection.linked].reference);
    state->generation=prior->chart_state->generation+1;memcpy(state->cameras,prior->chart_state->cameras,sizeof(state->cameras));memcpy(state->camera_ready,prior->chart_state->camera_ready,sizeof(state->camera_ready));context->pane->camera=prior->camera;context->pane->fit=0;return 1;
missing:
    snprintf(context->pane->query_message,sizeof(context->pane->query_message),"Selected chart state or exact local hypothesis cannot remap; last good snapshot retained.");return 0;
}
static void chart_picker(GD_ExplorerContext *context,const char *label,char *text,GD_Role role,int optional) {
    const GD_Model *model=gd_explorer_model(context->project,GD_EXPLORER_STATES);GD_ExplorerChartForm *form=&context->pane->chart_state->pending;int i,noun=gd_query_record(model,form->object,GD_NOUN);
    if(!gd_explorer_picker_begin(context,label,*text ? text:optional ? "None":"Choose..."))return;
    if(optional && gd_explorer_picker_item(context,"None",!*text,0))*text=0;
    for(i=0;i<model->record_count;++i)if(model->records[i].role==role){
        if(role==GD_AXIS){int edge,allowed=0;if(!context->project->rules->axes[i].valid || context->project->rules->axes[i].type!=GD_AXIS_ENUM || noun<0)continue;
            for(edge=context->project->rules->actor_first[noun];edge>=0;edge=context->project->rules->next_axis[edge])if(context->project->rules->axis_resolution[edge]==GD_RULE_RESOLVED && model->edges[edge].target==i)allowed=1;
            if(!allowed)continue;
        }
        if(gd_explorer_picker_record_item(context,&model->records[i],model->records[i].reference,!strcmp(text,model->records[i].reference)))snprintf(text,GD_MAX_PATH,"%s",model->records[i].reference);
    }
    gd_explorer_picker_end();
}
int gd_explorer_chart_cell_filtered(const GD_ExplorerChartState *state,int row,int column){return (state->row>=0 || state->column>=0) && (state->row<0 || state->row==row) && (state->column<0 || state->column==column);}
static int chart_matches(const GD_ExplorerChartState *state,int scenario) {
    const GD_StateChartScenario *item=&state->query->state_chart->scenarios[scenario];
    return (state->row<0 || state->row==item->value) && (state->column<0 || !item->companion_value || !strcmp(state->query->state_chart->companion_values[state->column],item->companion_value));
}
void gd_explorer_chart_controls(GD_ExplorerContext *context) {
    GD_ExplorerChartState *state=context->pane->chart_state;int i,count=0;GD_EvidencePage page;
    chart_picker(context,"Noun",state->pending.object,GD_NOUN,0);chart_picker(context,"Axis",state->pending.axis,GD_AXIS,0);chart_picker(context,"Companion axis (optional)",state->pending.companion_axis,GD_AXIS,1);
    igTextUnformatted("Exact source#case (optional)",NULL);igSetNextItemWidth(-1);gd_explorer_form_widget(GD_FORM_CHART,&state->pending,"case_name","##chart-case");
    igTextUnformatted("Local participant (requires case)",NULL);igSetNextItemWidth(-1);gd_explorer_form_widget(GD_FORM_CHART,&state->pending,"participant","##chart-participant");
    igTextUnformatted("Native work ceiling",NULL);igSetNextItemWidth(-1);gd_explorer_form_widget(GD_FORM_CHART,&state->pending,"work_limit","##chart-work");
    gd_explorer_action_button(context,GD_ACTION_CHART_RUN);gd_explorer_action_button(context,GD_ACTION_CHART_CLEAR);
    if(*context->pane->query_message)igTextWrapped("%s",context->pane->query_message);
    if(!state->query){igTextWrapped("Select a noun and enum axis, then Run. No automatic evaluation when changing inputs or modes.");return;}
    igTextWrapped("Applied: %s | %s | %s",state->applied.object,state->applied.axis,*state->applied.companion_axis ? state->applied.companion_axis:"one axis; other facts unknown");
    igTextWrapped("%d retained hypotheses, %d proposals, %d activations. Source incomplete: %s. Query limited: %s. Storage limited: %s.",state->query->state_chart->scenario_count,state->query->state_chart->write_count,state->query->state_chart->activation_count,state->query->state_coverage->source_incomplete ? "yes":"no",state->query->limited || state->query->state_coverage->limited ? "yes":"no",state->query->state_chart->storage_limited ? "yes":"no");
    igTextWrapped("Native work %d / %d; %d unretained hypotheses on inventoried bindings; %d unexamined bindings (upper bound). Retention work limited: %s.",state->query->work+state->query->state_coverage->work,state->query->request.work_limit,state->query->binding_count*state->query->state_coverage->value_count*(state->query->request.companion_axis ? state->query->state_chart->companion_count:1)-state->query->state_chart->scenario_count,state->query->unexamined_bindings,state->query->state_chart->work_limited ? "yes":"no");
    if(igTreeNode_Str("Scope and legend")){
        igTextWrapped("Diagram labels use source case names, not proven runtime events. Brackets show guard-match status or conditional/unfinished annotation; inspect full declarations below. Gold marks conditional/unknown/unfinished proposals; green marks checked guard matches, not guaranteed outcomes. Explicit preservation is an internal-effect annotation, not a re-entry event. Arrow paging is presentation only.");
        igTextWrapped("Gold state/cell outlines filter retained evidence; blue marks the inspected hypothesis. Mode switches keep both. No analysis Run, source edit or reachability claim.");
        igTextWrapped("Same local binding only. Matching/unknown/excluded/unfinished counts are hypotheses, not events or frequency. Open vocabularies and unloaded/opaque declarations remain incomplete. No initial/final states, inferred hierarchy or reachability.");igTreePop();
    }
    if(igSmallButton("Clear state/cell filter")){state->row=state->column=-1;state->page=state->arrow_page=0;}
    if(!state->mode){int edges=0;for(i=0;i<state->edge_count;++i)if(chart_matches(state,state->edge_scenarios[i]))++edges;gd_explorer_evidence_pager("Diagram transitions",&state->arrow_page,edges,CHART_ARROWS_PER_PAGE);}
    for(i=0;i<state->query->state_chart->scenario_count;++i)if(chart_matches(state,i))++count;
    page=gd_explorer_evidence_pager("Hypothesis evidence",&state->page,count,GD_EVIDENCE_PAGE_ROWS);count=0;
    for(i=0;i<state->query->state_chart->scenario_count;++i)if(chart_matches(state,i)){
        const GD_StateChartScenario *scenario=&state->query->state_chart->scenarios[i];const GD_QueryBinding *binding=&state->query->bindings[scenario->binding];const GD_RuleCase *root=&state->query->rules->cases[binding->case_index];int ordinal=count++;
        if(ordinal<page.first || ordinal>=page.end)continue;igPushID_Int(i);
        if(igSelectable_Bool(gd_scene_format("%s :: %s | %s%s%s | %s%s",root->name,state->query->rules->participants[binding->participant].name,state->query->state_coverage->values[scenario->value].value,scenario->companion_value ? " + ":"",scenario->companion_value ? scenario->companion_value:"",gd_rule_truth_name(scenario->applicability),scenario->limited ? " [unfinished]":""),state->scenario==i,0,(ImVec2_c){0,0})){if(state->scenario!=i)state->proposal_page=state->activation_page=0;state->scenario=i;context->pane->selection.linked=state->query->rules->sources[root->source].record;}
        igPopID();
    }
}
static void chart_label(const GD_ExplorerContext *context,GD_Rect box,const char *text,int size,GD_Color color) {
    GD_Point position=gd_world_to_screen((GD_Point){box.x,box.y},context->pane->camera);GD_Rect clip=gd_rect_overlap((GD_Rect){position.x,position.y,box.width*context->pane->camera.zoom,box.height*context->pane->camera.zoom},context->viewport);
    if(clip.width<1 || clip.height<1 || size*context->pane->camera.zoom<8)return;
    gd_scene_clip((int)clip.x,(int)clip.y,(int)clip.width,(int)clip.height);gd_scene_text(text,(int)box.x+6,(int)box.y+3,size,color);
    gd_scene_unclip();
}
static GD_Rect chart_state_box(int region,int node) {return (GD_Rect){40+node*(float)CHART_NODE_STEP,80+region*(float)CHART_REGION_HEIGHT,174,62};}
static GD_Rect chart_grid_box(int row,int column) {
    return (GD_Rect){CHART_GRID_LEFT+column*(float)CHART_GRID_COLUMN,CHART_GRID_TOP+row*(float)CHART_GRID_ROW,CHART_GRID_COLUMN,CHART_GRID_ROW};
}
static int chart_focus(GD_ExplorerContext *context,int frame) {
    GD_ExplorerChartState *state=context->pane->chart_state;const GD_Query *query=state ? state->query:NULL;int row,column,i,region,selected[2];GD_Rect bounds={0},primary={0};int found=0;GD_ExplorerFocusResult result={0};
    if(!query)return 0;row=state->row;column=state->column;
    if(state->scenario>=0){const GD_StateChartScenario *scenario=&query->state_chart->scenarios[state->scenario];row=scenario->value;column=scenario->companion_value ? chart_node(state,1,scenario->companion_value,0):-1;}
    if(row<0 && column<0)return 0;
    if(state->mode){int rows=row>=0 ? 1:query->state_coverage->value_count,columns=column>=0 ? 1:query->request.companion_axis ? query->state_chart->companion_count:1;
        if(!rows || !columns)return 0;primary=chart_grid_box(row>=0 ? row:0,column>=0 ? column:0);bounds=primary;bounds.width+=(columns-1)*CHART_GRID_COLUMN;bounds.height+=(rows-1)*CHART_GRID_ROW;found=rows*columns;goto place;}
    selected[0]=row;selected[1]=column;primary=chart_state_box(row>=0 ? 0:1,row>=0 ? row:column);
    for(region=0;region<2;++region)for(i=0;i<state->node_count[region];++i){int include=i==selected[region];GD_Rect box=chart_state_box(region,i);
        if(state->scenario>=0){const GD_StateChartScenario *scenario=&query->state_chart->scenarios[state->scenario];int w;for(w=0;w<scenario->write_count && !include;++w){const GD_RuleWrite *write=&query->state_chart->writes[scenario->first_write+w];if(write->actor==query->bindings[scenario->binding].participant && write->axis==(region ? query->state_chart->companion_axis:query->request.axis) && write->kind==GD_WRITE_STATE && !strcmp(write->state,state->nodes[region][i]))include=1;}}
        if(include){if(!found)bounds=box;else{float right=fmaxf(bounds.x+bounds.width,box.x+box.width),bottom=fmaxf(bounds.y+bounds.height,box.y+box.height);bounds.x=fminf(bounds.x,box.x);bounds.y=fminf(bounds.y,box.y);bounds.width=right-bounds.x;bounds.height=bottom-bounds.y;}++found;}
    }
    if(!found)return 0;
place:
    result.selected=found;
    if(frame){context->pane->camera.target=(GD_Point){bounds.x+bounds.width/2,bounds.y+bounds.height/2};context->pane->camera.zoom=fmaxf(.002f,fminf(1.2f,fminf(context->viewport.width/(bounds.width+80),context->viewport.height/(bounds.height+100))));}
    else gd_explorer_focus_camera(&context->pane->camera,context->viewport,bounds,bounds,(GD_Point){primary.x+primary.width/2,primary.y+primary.height/2},&result);
    result.zoom=context->pane->camera.zoom;result.status=result.clipped ? GD_FOCUS_PARTIAL:GD_FOCUS_FRAMED;context->pane->focus_result=result;context->pane->fit=0;context->pane->focus_camera_override=true;return 1;
}
int gd_explorer_chart_fit(GD_ExplorerContext *context){return chart_focus(context,1);}
void gd_explorer_chart_fit_all(GD_ExplorerContext *context){
    GD_ExplorerPane *pane=context->pane;GD_ExplorerChartState *state=pane->chart_state;
    const GD_Query *query=state ? state->query:NULL;
    GD_Rect box=context->viewport;float width=400,height=400,visible_width,visible_height;
    if(query && state->mode){
        int columns=query->request.companion_axis ? query->state_chart->companion_count:1;
        /* Include row/column headers and title, not just populated cells. */
        width=CHART_GRID_LEFT+CHART_GRID_COLUMN*fmaxf(1,columns)-20;
        height=CHART_GRID_TOP+CHART_GRID_ROW*query->state_coverage->value_count-26;
        pane->camera.zoom=fmaxf(.0001f,fminf(1.2f,fminf(box.width/(width+40),box.height/(height+40))));
        pane->camera.target=(GD_Point){20+width/2,26+height/2};
        return;
    }else if(query){
        width=fmaxf(state->node_count[0],state->node_count[1])*CHART_NODE_STEP+280;
        height=(query->state_chart->companion_axis>=0 ? 2:1)*CHART_REGION_HEIGHT+20;
    }
    pane->camera.zoom=fmaxf(.72f,fminf(1,fminf(box.width/width,box.height/height)));
    visible_width=box.width/pane->camera.zoom;visible_height=box.height/pane->camera.zoom;
    /* Large diagrams start at their first readable rows. Pan reaches the rest. */
    pane->camera.target=(GD_Point){visible_width/2,visible_height/2};
}
int gd_explorer_chart_focus(GD_ExplorerContext *context){return chart_focus(context,0);}
static void chart_select(GD_ExplorerContext *context,int row,int column,int scenario) {
    GD_ExplorerChartState *state=context->pane->chart_state;state->row=row;state->column=column;if(state->scenario!=scenario)state->proposal_page=state->activation_page=0;state->scenario=scenario;if(scenario<0)state->page=state->arrow_page=0;
    context->pane->selection=(GD_ExplorerSelection){-1,-1,0,-1,-1};
    if(scenario>=0){const GD_QueryBinding *binding=&state->query->bindings[state->query->state_chart->scenarios[scenario].binding];context->pane->selection.linked=state->query->rules->sources[state->query->rules->cases[binding->case_index].source].record;}
}
void gd_explorer_chart_input(GD_ExplorerContext *context) {
    GD_ExplorerPane *pane=context->pane;GD_ExplorerChartState *state=pane->chart_state;GD_Point mouse=gd_mouse_position();float wheel;
    if(!state || igGetIO_Nil()->WantCaptureMouse || !gd_rect_hit(mouse,context->viewport))return;
    if(igIsMouseDown_Nil(ImGuiMouseButton_Middle) || igIsMouseDown_Nil(ImGuiMouseButton_Right)){GD_Point delta=gd_mouse_delta();pane->camera.target.x-=delta.x/pane->camera.zoom;pane->camera.target.y-=delta.y/pane->camera.zoom;}
    wheel=gd_mouse_wheel();if(wheel){GD_Point before=gd_screen_to_world(mouse,pane->camera),after;pane->camera.zoom=fmaxf(state->mode ? .0001f:.08f,fminf(3,pane->camera.zoom*powf(1.15f,wheel)));after=gd_screen_to_world(mouse,pane->camera);pane->camera.target.x+=before.x-after.x;pane->camera.target.y+=before.y-after.y;}
}
void gd_explorer_chart_draw(GD_ExplorerContext *context) {
    GD_ExplorerPane *pane=context->pane;GD_ExplorerChartState *state=pane->chart_state;const GD_Query *query=state->query;const GD_StateChart *chart=query ? query->state_chart:NULL;GD_Point mouse=gd_screen_to_world(gd_mouse_position(),pane->camera);int click=!igGetIO_Nil()->WantCaptureMouse && gd_rect_hit(gd_mouse_position(),context->viewport) && gd_mouse_pressed(ImGuiMouseButton_Left),region,i,j;
    if(!query){gd_scene_text("State charts: select noun/axes in Analysis, then Run.",(int)context->viewport.x+20,(int)context->viewport.y+20,18,chart_muted);return;}
    int inspected_row=-1,inspected_column=-1;
    if(state->scenario>=0 && state->scenario<chart->scenario_count){const GD_StateChartScenario *selected=&chart->scenarios[state->scenario];inspected_row=selected->value;inspected_column=selected->companion_value ? chart_node(state,1,selected->companion_value,0):0;}
    gd_scene_clip((int)context->viewport.x,(int)context->viewport.y,(int)context->viewport.width,(int)context->viewport.height);gd_scene_set_camera(pane->camera);
    if(state->mode){
        int rows=query->state_coverage->value_count,columns=query->request.companion_axis ? chart->companion_count:1;
        const GD_Color header={39,51,68,255},border={65,78,95,255};
        GD_CountRange range={0};float pixel=1/pane->camera.zoom;
        GD_Rect corner={20,54,CHART_GRID_LEFT-20,CHART_GRID_TOP-54};
        GD_Point lo=gd_screen_to_world((GD_Point){context->viewport.x,context->viewport.y},pane->camera);
        GD_Point hi=gd_screen_to_world((GD_Point){context->viewport.x+context->viewport.width,context->viewport.y+context->viewport.height},pane->camera);
        GD_Rect visible={lo.x,lo.y,hi.x-lo.x,hi.y-lo.y};
        for(i=0;i<rows*columns;++i)gd_count_range_add(&range,state->cells[i].matching);
        gd_scene_rect(corner,header);chart_label(context,corner,query->model->records[query->request.axis].reference,17,chart_text);
        chart_label(context,(GD_Rect){CHART_GRID_LEFT,26,fmaxf(1,columns)*CHART_GRID_COLUMN,24},
            query->request.companion_axis ? query->request.companion_axis:"No companion assumption",17,chart_muted);
        for(j=0;j<columns;++j){GD_Rect box=chart_grid_box(0,j);box.y=corner.y;box.height=corner.height;
            if(!gd_rect_intersects(box,visible))continue;
            gd_scene_rect(box,header);gd_scene_outline(box,pixel,border);
            chart_label(context,box,chart->companion_count ? chart->companion_values[j]:"(one axis)",18,chart_text);
        }
        for(i=0;i<rows;++i){GD_Rect row=chart_grid_box(i,0);row.x=corner.x;row.width=corner.width;
            if(row.y>=hi.y || row.y+row.height<=lo.y)continue;
            if(gd_rect_intersects(row,visible)){
                gd_scene_rect(row,header);gd_scene_outline(row,pixel,border);chart_label(context,row,query->state_coverage->values[i].value,18,chart_text);
            }
            for(j=0;j<columns;++j){
                int cell=i*columns+j,inspected=inspected_row==i && inspected_column==j,filtered=gd_explorer_chart_cell_filtered(state,i,j);
                GD_Rect box=chart_grid_box(i,j),line=box;
                int matching=state->cells[cell].matching,unknown=state->cells[cell].unknown,excluded=state->cells[cell].excluded,unfinished=state->cells[cell].unfinished;
                if(!gd_rect_intersects(box,visible))continue;
                gd_scene_rect(box,gd_count_color(range,matching));gd_scene_outline(box,pixel,border);
                if(filtered && box.width>2*pixel && box.height>2*pixel)gd_scene_outline((GD_Rect){box.x+pixel,box.y+pixel,box.width-2*pixel,box.height-2*pixel},2*pixel,chart_gold);
                if(inspected && box.width>8*pixel && box.height>8*pixel)gd_scene_outline((GD_Rect){box.x+4*pixel,box.y+4*pixel,box.width-8*pixel,box.height-8*pixel},2*pixel,chart_blue);
                line.x+=5;line.width-=10;line.y+=9;line.height=24;
                chart_label(context,line,gd_scene_format("%d matching%s",matching,filtered ? " [filter]":""),18,chart_text);line.y+=26;
                chart_label(context,line,gd_scene_format("%d unknown | %d excluded",unknown,excluded),15,chart_text);line.y+=23;
                chart_label(context,line,gd_scene_format("%d unfinished%s",unfinished,inspected ? " [inspected]":""),15,unfinished ? chart_gold:chart_muted);
                if(!igGetIO_Nil()->WantCaptureMouse && gd_rect_hit(gd_mouse_position(),context->viewport) && gd_rect_hit(mouse,box)){
                    igSetTooltip("%s / %s\n%d matching, %d unknown, %d excluded, %d unfinished\nCounts concern hypotheses on inventoried bindings, not outcomes.",
                        query->state_coverage->values[i].value,chart->companion_count ? chart->companion_values[j]:"no companion assumption",matching,unknown,excluded,unfinished);
                    if(click)chart_select(context,i,j,-1);
                }
            }
        }
        if(!rows || !columns)gd_scene_text("No declared finite values; no joint hypotheses evaluated.",20,105,18,chart_gold);
        gd_scene_screen();gd_scene_unclip();
        gd_scene_clip((int)context->viewport.x,(int)context->viewport.y-GD_COUNT_GRID_HEADER,(int)context->viewport.width,GD_COUNT_GRID_HEADER);
        gd_scene_text("Hypotheses on inventoried bindings; not outcomes",(int)context->viewport.x+6,(int)context->viewport.y-GD_COUNT_GRID_HEADER+5,14,chart_muted);
        gd_count_legend(range,context->viewport.x+6,context->viewport.y-GD_COUNT_GRID_HEADER+24,fminf(360,context->viewport.width-12),"Matching hypotheses");
        gd_scene_unclip();return;
    }else{
        struct { GD_Rect box; const char *root,*status,*target; GD_Color color; int scenario,preserve; } labels[CHART_ARROWS_PER_PAGE];
        int regions=chart->companion_axis>=0 ? 2:1,ordinal=0,shown=0,first=state->arrow_page*CHART_ARROWS_PER_PAGE,lane[2]={0};
        float width=fmaxf(state->node_count[0],state->node_count[1])*CHART_NODE_STEP+260;
        gd_scene_round_outline((GD_Rect){12,8,width,regions*(float)CHART_REGION_HEIGHT},.02f,2,chart_muted);
        gd_scene_text(query->model->records[query->request.noun].reference,30,22,20,chart_text);
        for(region=0;region<regions;++region){int axis=region ? chart->companion_axis:query->request.axis;
            if(region)gd_scene_line((GD_Point){22,region*(float)CHART_REGION_HEIGHT+5},(GD_Point){width,region*(float)CHART_REGION_HEIGHT+5},2,chart_muted);
            gd_scene_text(gd_scene_format("%s%s",query->model->records[axis].reference,query->model->records[axis].axis_closed ? " [closed vocabulary]":" [open vocabulary]"),40,54+region*CHART_REGION_HEIGHT,16,chart_muted);
            for(i=0;i<state->node_count[region];++i){GD_Rect box=chart_state_box(region,i);int selected=region ? state->column==i:state->row==i,inspected=region ? inspected_column==i:inspected_row==i;
                gd_scene_round_rect(box,.18f,(GD_Color){29,42,57,255});if(selected)gd_scene_round_outline((GD_Rect){box.x-3,box.y-3,box.width+6,box.height+6},.18f,2,chart_gold);gd_scene_round_outline(box,.18f,inspected ? 3:1,inspected ? chart_blue:chart_muted);chart_label(context,(GD_Rect){box.x+8,box.y+15,box.width-12,30},gd_scene_format("%s%s",state->nodes[region][i],i>=(region ? chart->companion_count:query->state_coverage->value_count) ? " [proposal]":""),18,chart_text);
                if(click && gd_rect_hit(mouse,box)){int hypotheses=region ? chart->companion_count:query->state_coverage->value_count;if(i<hypotheses)chart_select(context,region ? state->row:i,region ? i:state->column,-1);else {pane->selection.linked=axis;state->scenario=-1;}}
            }
        }
        for(i=0;i<state->edge_count && shown<CHART_ARROWS_PER_PAGE;++i){
            int scenario_index=state->edge_scenarios[i],index=state->edge_writes[i],source,target;const GD_StateChartScenario *scenario=&chart->scenarios[scenario_index];const GD_RuleWrite *write=&chart->writes[index];const GD_QueryBinding *binding=&query->bindings[scenario->binding];const GD_RuleCase *root=&query->rules->cases[binding->case_index];GD_Rect a,b,label;float y;GD_Color color;
            if(!chart_matches(state,scenario_index))continue;if(ordinal++<first)continue;
            region=write->axis==query->request.axis ? 0:1;source=region ? chart_node(state,1,scenario->companion_value,0):scenario->value;target=write->kind==GD_WRITE_PRESERVE ? source:chart_node(state,region,write->state,0);if(source<0 || target<0)continue;
            {int row=lane[region]++,preserve=write->kind==GD_WRITE_PRESERVE;
            float start,end,stroke=state->scenario==scenario_index ? 2.5f:1.5f;
            GD_LayoutRoute route={0};
            a=chart_state_box(region,source);b=chart_state_box(region,target);y=179+region*(float)CHART_REGION_HEIGHT+row*29;
            start=a.x+25+row*5;end=b.x+148-row*5;
            label=(GD_Rect){fminf(width-316,fmaxf(start,end)+12),y-12,304,25};
            color=state->scenario==scenario_index ? chart_blue:scenario->applicability==GD_RULE_TRUTH_TRUE && !scenario->limited && !write->conditional ? (GD_Color){124,210,166,255}:chart_gold;
            route.points[0]=(GD_LayoutPoint){start,a.y+a.height};route.points[1]=(GD_LayoutPoint){start,y};
            route.points[2]=(GD_LayoutPoint){preserve ? label.x:end,y};route.count=3;
            if(!preserve){route.points[3]=(GD_LayoutPoint){end,b.y+b.height+4};route.count=4;}
            gd_graph_route_line(&route,stroke,color,scenario->limited || write->conditional || scenario->applicability!=GD_RULE_TRUTH_TRUE);
            if(!preserve)gd_graph_route_arrow(&route,color);
            if(!preserve && label.x>end)gd_scene_line((GD_Point){end,y},(GD_Point){label.x,y},1,color);
            labels[shown].box=label;labels[shown].root=root->name;
            labels[shown].status=scenario->limited ? "unfinished":write->conditional ? "conditional":gd_rule_truth_name(scenario->applicability);
            labels[shown].target=preserve ? state->nodes[region][source]:write->state;
            labels[shown].color=color;labels[shown].scenario=scenario_index;labels[shown].preserve=preserve;++shown;}
        }
        for(i=0;i<shown;++i){
            GD_Rect label=labels[i].box;GD_Color color=labels[i].color;
            gd_scene_round_rect(label,.15f,(GD_Color){26,38,52,255});gd_scene_round_outline(label,.15f,labels[i].scenario==state->scenario ? 2:1,color);
            chart_label(context,label,gd_scene_format("[%s] %s: %s%s",labels[i].status,labels[i].root,labels[i].preserve ? "preserve ":"",labels[i].target),15,chart_text);
            if(!igGetIO_Nil()->WantCaptureMouse && gd_rect_hit(gd_mouse_position(),context->viewport) && gd_rect_hit(mouse,label)){
                igSetTooltip("%s [%s]\n%s%s\nSelect for declaration, guards and side effects.",labels[i].root,labels[i].status,labels[i].preserve ? "Preserve ":"Propose ",labels[i].target);
                if(click)chart_select(context,state->row,state->column,labels[i].scenario);
            }
        }
        if(!state->edge_count)gd_scene_text("No retained non-excluded state/preservation proposals. No self-loops implied.",40,180,17,chart_gold);
    }
    gd_scene_screen();gd_scene_unclip();
}
void gd_explorer_chart_details(GD_ExplorerContext *context) {
    GD_ExplorerChartState *state=context->pane->chart_state;const GD_Query *query=state->query;const GD_StateChartScenario *scenario;int i;
    if(context->pane->selection.linked>=0){const GD_Record *source=&gd_explorer_model(context->project,GD_EXPLORER_STATES)->records[context->pane->selection.linked];igTextWrapped("Source %s | %s",source->reference,source->source_path);}
    if(!query || state->scenario<0 || state->scenario>=query->state_chart->scenario_count){igTextWrapped("Select a hypothesis or transition label. State/cell selection filters evidence without Run.");return;}
    scenario=&query->state_chart->scenarios[state->scenario];
    igTextWrapped("Hypothesis %d: %s | requirements %s | context %s | constraints complete %s | values complete %s | unfinished %s",state->scenario,gd_rule_truth_name(scenario->applicability),gd_rule_truth_name(scenario->requirements),gd_rule_truth_name(scenario->context),scenario->constraints_complete ? "yes":"no",scenario->values_complete ? "yes":"no",scenario->limited ? "yes":"no");
    igTextWrapped("Only binding %s: %s=%s%s%s. Other bindings stay unknown.",query->rules->participants[query->bindings[scenario->binding].participant].name,state->applied.axis,query->state_coverage->values[scenario->value].value,scenario->companion_value ? " ; companion=":"",scenario->companion_value ? scenario->companion_value:"");
    igTextWrapped("%d opaque effects | %d cycles | %d conflicts | %d uncertain pairs. Proposals are not final outcomes.",scenario->opaque_effects,scenario->cycles,scenario->conflicts,scenario->uncertain_pairs);
    gd_explorer_rule_case_details(context,query->bindings[scenario->binding].case_index);
    if(!scenario->write_count)igTextWrapped("No retained proposals. Preservation is not implied.");
    GD_EvidencePage proposals=gd_explorer_evidence_pager("All participant proposals",&state->proposal_page,scenario->write_count,GD_EVIDENCE_PAGE_ROWS);
    for(i=proposals.first;i<proposals.end;++i){int index=scenario->first_write+i;const GD_RuleWrite *write=&query->state_chart->writes[index];const GD_RuleCase *origin=&query->rules->cases[write->origin_case];const GD_Record *source=&query->model->records[query->rules->sources[origin->source].record];
        igTextWrapped("Proposal %d: %s | %s | %s | %s%s | line %d%s",i,write->actor>=0 ? query->rules->participants[write->actor].name:"unresolved actor",write->axis>=0 ? query->model->records[write->axis].reference:"unresolved axis",gd_effect_kind_name(query->state_chart->families[index]),write->kind==GD_WRITE_PRESERVE ? "preserve":write->kind==GD_WRITE_STATE ? write->state:"quantity",write->kind==GD_WRITE_QUANTITY && write->value.known ? gd_scene_format(" %.9g",write->value.number):"",write->line,write->conditional ? " [conditional]":"");
        igTextWrapped("Origin %s#%s | %s:%d | activation %d | declared unit %s",source->reference,origin->name,source->source_path,write->line,query->state_chart->write_activations[index],write->axis>=0 && query->model->records[write->axis].axis.unit ? query->model->records[write->axis].axis.unit:"unspecified");
    }
    GD_EvidencePage activations=gd_explorer_evidence_pager("Mapped activations",&state->activation_page,scenario->activation_count,GD_EVIDENCE_PAGE_ROWS);
    for(i=activations.first;i<activations.end;++i){const GD_RuleActivation *activation=&query->state_chart->activations[scenario->first_activation+i];int step;igPushID_Int(i);
        if(igTreeNode_Str(gd_scene_format("Activation %d: %s | local %s",i,gd_rule_truth_name(activation->applicability),gd_rule_truth_name(activation->local_applicability)))){
            for(step=0;step<activation->path_count;++step){const GD_RuleCase *root=&query->rules->cases[activation->path[step]];igTextWrapped("Mapped ancestor %s#%s",query->model->records[query->rules->sources[root->source].record].reference,root->name);if(activation->calls[step]>=0){const GD_RuleCall *call=&query->analysis->calls[activation->calls[step]];igTextWrapped("Call line %d: %.*s",call->line,(int)call->length,call->text);}}
            gd_explorer_rule_case_details(context,activation->case_index);igTreePop();
        }igPopID();
    }
}
