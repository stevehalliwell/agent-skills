#include "gd_resource_graph.h"
#include "gd_projection.h"
#include "gd_layout.h"
#include "gd_graph_analysis.h"
#include "gd_graph_analysis_draw.h"
#include "gd_graph_overview_draw.h"
#include "raylib.h"
#include <math.h>
#include <string.h>

static const Color background={18,24,34,255}, foreground={225,233,241,255}, muted={155,174,194,255};
static const Color role_color={193,155,228,255}, facet_color={108,171,230,255}, case_color={224,160,109,255};
static const Color selected_color={255,217,125,255};
static const char *node_label(const GD_ResourceGraph *graph, int index) {
    return gd_resource_label(graph,index);
}
static const char *relation_name(GD_ResourceRelationKind kind) {
    switch (kind) {
        case GD_RESOURCE_REQUIREMENT: return "condition";
        case GD_RESOURCE_EXCLUSION: return "exclusion";
        case GD_RESOURCE_STATE_WRITE: return "state write";
        case GD_RESOURCE_QUANTITY_WRITE: return "quantity write";
        case GD_RESOURCE_ROLE_BINDING: return "role binding (not flow)";
        case GD_RESOURCE_PRESERVATION: return "preservation";
        case GD_RESOURCE_INVOCATION: return "conditional invocation";
        default: return "unknown";
    }
}
static int resource_layout(const GD_ResourceGraph *graph, GD_Layout *layout) { return gd_resource_layout(graph,layout); }
static void fit(Camera2D *camera, const GD_Layout *layout, Rectangle viewport) {
    float left=0,right=1,top=0,bottom=1; int i;
    for (i=0;i<layout->node_count;++i) {
        float x=layout->nodes[i].x,y=layout->nodes[i].y;
        if (!i || x-110<left) left=x-110; if (!i || x+110>right) right=x+110;
        if (!i || y-45<top) top=y-45; if (!i || y+45>bottom) bottom=y+45;
    }
    camera->target=(Vector2){(left+right)/2,(top+bottom)/2};
    camera->offset=(Vector2){viewport.x+viewport.width/2,viewport.y+viewport.height/2};
    camera->zoom=fminf(viewport.width/(right-left+50),viewport.height/(bottom-top+50));
    if (camera->zoom>1.2f) camera->zoom=1.2f; if (camera->zoom<0.002f) camera->zoom=0.002f;
}
static int shared_choice(const GD_GraphAnalysis *analysis, int selected, int *peer, int next) {
    int current=gd_graph_match(analysis,selected,*peer),i,start=current>=0 ? current+1:0;
    if (current>=0 && !next) return current;
    for (i=0;i<analysis->pair_count;++i) {
        int index=(start+i)%analysis->pair_count; const GD_GraphMatch *match=&analysis->pairs[index];
        if (match->a!=selected && match->b!=selected) continue;
        *peer=match->a==selected ? match->b:match->a; return index;
    }
    *peer=-1; return -1;
}
static const GD_Record *relation_source(const GD_ResourceGraph *graph, int index) {
    const GD_ResourceRelation *relation=&graph->relations[index];
    int c=relation->source_case;
    return c>=0 ? &graph->model->records[graph->rules->sources[graph->rules->cases[c].source].record]:NULL;
}
#include "gd_resource_evidence.h"
static int source_row(const GD_ResourceGraph *graph,int index,int x,int y) {
    const GD_ResourceRelation *relation=&graph->relations[index];const GD_Record *source=relation_source(graph,index);
    y=resource_text(TextFormat("relation #%d | %s | line %d | case #%d",index,relation_name(relation->kind),relation->line,relation->source_case),x,y,13,muted);
    if(source)y=resource_text(source->reference,x,y,13,foreground);
    return y;
}
static int path_sources(const GD_ResourceGraph *graph,const GD_Layout *layout,const int *arcs,int retained,GD_EvidencePage page,int x,int y) {
    for(int i=page.first;i<page.end;++i)y=source_row(graph,layout->edges[arcs[retained-1-i]/2].key,x,y);
    return y;
}
static int overview_details(const GD_ResourceGraph *graph, const GD_Layout *layout, GD_GraphAnalysis *analysis,
                             GD_OverviewControls *controls, int x,GD_ResourceEvidence *evidence) {
    const GD_GraphOverview *overview=gd_graph_overview(analysis,controls,1); int i,types[7]={0},y=390;
    DrawText(controls->enabled ? "[x] Incidence overview (click)":"[ ] Incidence overview (click)",x,114,16,foreground);
    if (!overview) return y;
    DrawText("Row + (click)",x,146,14,foreground); DrawText("Column + (click)",x+160,146,14,foreground);
    DrawText("Next occupied cell (click)",x,178,14,foreground); DrawText("Clear cell (click)",x,210,14,foreground);
    DrawText("Symmetric incidence, NOT flow. Linear brightness:",x,244,13,muted);
    DrawText("projected link rows, not quantities/strength.",x,264,13,muted);
    DrawText("Index bins, not clusters. Diagonal is not",x,284,13,muted);
    DrawText("necessarily a self-link. Gold: contributors.",x,304,13,muted);
    DrawText(TextFormat("%d identities / %d bins / %d input rows",overview->node_count,overview->bins,overview->link_count),x,330,13,muted);
    if (controls->row<0 || controls->column<0 || !overview->bins) return y;
    DrawText(TextFormat("Cell %d,%d: %d rows | arrows: move cell",controls->row,controls->column,overview->selected_count),x,352,13,foreground);
    DrawText(TextFormat("Row ordinals %d..%d | column %d..%d",overview->start[controls->row],overview->start[controls->row+1]-1,overview->start[controls->column],overview->start[controls->column+1]-1),x,372,12,muted);
    for (i=0;i<analysis->edge_count;++i) if (overview->selected_edges[i]) {
        int kind=graph->relations[layout->edges[i].key].kind;
        if (kind>=0 && kind<7) ++types[kind];

    }
    for (i=0;i<7;++i) if (types[i]) { DrawText(TextFormat("%s: %d",relation_name((GD_ResourceRelationKind)i),types[i]),x,y,12,muted); y+=18; }
    for(int side=0;side<2;++side){
        int bin=side ? controls->column:controls->row,begin=overview->start[bin],end=overview->start[bin+1];
        GD_EvidencePage page=resource_pager(side ? "Column identities":"Row identities",&evidence->bin[side],end-begin,x,&y,evidence);
        for(i=begin+page.first;i<begin+page.end;++i){int node=overview->nodes[i];y=resource_text(TextFormat("#%d",node),x,y,12,muted);y=resource_text(node_label(graph,node),x,y,13,foreground);}
    }
    GD_EvidencePage page=resource_pager("Contributing link rows",&evidence->sources,overview->selected_count,x,&y,evidence);int ordinal=0;
    for(i=0;i<analysis->edge_count;++i)if(overview->selected_edges[i] && gd_evidence_page_take(&page,&ordinal))y=source_row(graph,layout->edges[i].key,x,y);
    return resource_text("Retained shown incidences only; source omissions remain separate.",x,y,12,muted);
}
static const GD_GraphShapes *resource_shapes(const GD_ResourceGraph *graph, GD_GraphAnalysis *analysis, int enabled) {
    int roles[GD_MAX_RECORDS],i;
    if (!enabled) return NULL;
    for (i=0;i<analysis->node_count;++i) roles[i]=graph->nodes[i].kind;
    return gd_graph_shapes(analysis,roles,GD_GRAPH_ANALYSIS_WORK);
}
static int shape_details(const GD_ResourceGraph *graph, const GD_GraphShapes *shapes, int enabled, int selected, int x,GD_ResourceEvidence *evidence) {
    int i,shown=0,y=290;
    DrawText(enabled ? "[x] Structural shapes (click)":"[ ] Structural shapes (click)",x,114,16,foreground);
    DrawText("Two exact typed endpoint-neighborhood rounds.",x,146,13,muted);
    DrawText("Next repeated family (click)",x,178,14,foreground);
    DrawText("Transient IDs; NOT identity or equivalence.",x,210,13,muted);
    DrawText("Shown incidences, NOT resource flow.",x,232,13,muted);
    if (!shapes) return y;
    if (!shapes->complete) { DrawText("Unavailable: shape work limit.",x,256,14,muted); return y; }
    DrawText(TextFormat("%d families | gold outline: selected family",shapes->family_count),x,256,13,muted);
    if (selected<0 || selected>=shapes->node_count) return y;
    DrawText(TextFormat("shape %d / %d members",shapes->colors[selected]+1,shapes->size[shapes->colors[selected]]),x,y,14,foreground); y+=24;
    GD_EvidencePage page=resource_pager("Family members",&evidence->members,shapes->size[shapes->colors[selected]],x,&y,evidence);
    for(i=0;i<shapes->node_count;++i)if(shapes->colors[i]==shapes->colors[selected] && gd_evidence_page_take(&page,&shown)){
        y=resource_text(TextFormat("member #%d (snapshot-local)",i),x,y,12,muted);y=resource_text(node_label(graph,i),x,y,13,foreground);
    }
    return resource_text("Missing data can change families; rarity is not a design defect. Member keys are local.",x,y+12,13,muted);
}
static void next_reach_kind(const GD_GraphAnalysis *analysis, GD_ReachControls *controls) {
    int i,first=-1,next=-1;
    for (i=0;i<analysis->edge_count;++i) {
        const GD_LayoutEdge *edge=&analysis->edges[i]; int kind=edge->relation;
        if (!analysis->shown[edge->a] || !analysis->shown[edge->b] || (!edge->active && !analysis->include_inactive)) continue;
        if (first<0 || kind<first) first=kind;
        if (kind>controls->relation && (next<0 || kind<next)) next=kind;
    }
    controls->relation=next>=0 ? next:first;
}
static int reach_details(const GD_ResourceGraph *graph, const GD_Layout *layout, GD_GraphAnalysis *analysis,
                          GD_ReachControls *controls, int selected, int x,GD_ResourceEvidence *evidence) {
    const GD_GraphClosure *closure=gd_graph_reachability_prepare(analysis,controls,1);
    const GD_GraphComparison *comparison=closure ? gd_graph_closure_choose_into(analysis,&analysis->reachability,selected,&controls->peer):NULL;
    int i,j,y=350;
    DrawText(controls->enabled ? "[x] Typed multi-hop (click)":"[ ] Typed multi-hop (click)",x,114,16,foreground);
    if (!controls->enabled) return y;
    DrawText(TextFormat("Kind: %.36s (click)",controls->relation>=0 ? gd_graph_relation_name(controls->relation):"none"),x,146,13,foreground);
    DrawText(TextFormat("B: %.24s | %d peers (click)",comparison && comparison->b>=0 ? node_label(graph,comparison->b):"none",closure ? closure->peer_count:0),x,178,13,foreground);
    DrawText("Same-kind incidence connections, NOT flow.",x,211,14,muted);
    DrawText("T: other origins reaching this identity.",x,233,14,muted);
    DrawText("Purple shared | blue A-only | amber B-only",x,255,13,muted);
    if (!closure) { DrawText("No shown relation kind.",x,278,14,muted); return y; }
    if (!comparison) { DrawText("Select node with completed local traversal.",x,278,14,muted); return y; }
    DrawText(TextFormat("A: %.22s | %d targets%s",node_label(graph,selected),closure->count[selected],closure->complete ? "":" | partial"),x,278,13,muted);
    DrawText("Witness target (click to cycle)",x,302,14,foreground);
    if (controls->target<0 || controls->target>=layout->node_count || !comparison->nodes[controls->target]) {
        controls->target=-1; for (i=0;i<layout->node_count;++i) if (comparison->nodes[i]) { controls->target=i; break; }
    }
    if (controls->target>=0){y=resource_text(TextFormat("Target #%d (snapshot-local)",controls->target),x,324,12,muted);y=resource_text(node_label(graph,controls->target),x,y,13,foreground);}
    for (j=0;j<2 && controls->target>=0;++j) {
        int arcs[GD_RESOURCE_EVIDENCE_ROWS],length=gd_graph_closure_path(analysis,closure,j,controls->target,NULL,0);
        if (length<0) continue;
        DrawText(TextFormat("%c witness: %d retained parent arcs",j ? 'B':'A',length),x,y,13,foreground);y+=20;
        GD_EvidencePage page=resource_pager(j ? "B witness":"A witness",&evidence->witness[j],length,x,&y,evidence);int retained=page.end-page.first;
        gd_graph_closure_path_slice(analysis,closure,j,controls->target,length-page.end,arcs,retained);
        for(i=retained-1;i>=0;--i)y=source_row(graph,layout->edges[arcs[i]/2].key,x,y);
        y+=12;
    }
    return resource_text("Shown links only; not complete source semantics or execution.",x,y,12,muted);
}
static int path_details(const GD_ResourceGraph *graph, const GD_Layout *layout, GD_GraphAnalysis *analysis,
                         const GD_GraphPaths *paths, const GD_PathControls *controls, int selected, int x,GD_ResourceEvidence *evidence) {
    const GD_PathSelection *support; int y=controls->compare ? 280:248;
    DrawText(controls->enabled ? "[x] Mandatory connections (click)":"[ ] Mandatory connections (click)",x,114,14,foreground);
    if (!paths) { DrawText("Enable, then choose analysis starts.",x,148,14,muted); return y; }
    DrawRectangleLines(x,144,320,25,muted); DrawText("Add selection as analysis start",x+5,150,14,foreground);
    DrawRectangleLines(x,176,240,25,muted); DrawText("Clear analysis starts",x+5,182,14,foreground);
    DrawText(controls->compare ? "[x] Compare support / bypass (click)":"[ ] Compare support / bypass (click)",x,212,14,foreground);
    if (controls->compare) DrawText("Next support-path candidate (click)",x,244,14,foreground);
    DrawText(TextFormat("Configured %d | shown starts %d",controls->count,paths->root_count),x,y,14,muted); y+=22;
    DrawText("Virtual start; inspection does not re-root.",x,y,13,muted); y+=20;
    DrawText("D: necessary connections, NOT resource flow.",x,y,13,muted); y+=20;
    DrawText("D-: unreachable | D?: unavailable",x,y,13,muted); y+=20;
    DrawText("Pink: support | cyan: bypass | !: mandatory",x,y,13,muted); y+=20;
    DrawText("White triangle: chosen analysis start",x,y,13,muted); y+=20;
    if (!paths->root_count) { DrawText("Unavailable: no shown analysis starts.",x,y,14,selected_color); return y+22; }
    if (!paths->complete) { DrawText("Work limit: intermediate results NOT proofs.",x,y,13,selected_color); return y+22; }
    DrawText(TextFormat("%d / %d shown nodes reachable",paths->reachable_count,analysis->shown_nodes),x,y,14,muted); y+=23;
    DrawText("Shown incidence only; no composed outcomes.",x,y,13,muted); y+=23;
    support=gd_graph_path_selection(analysis,selected);
    if (!support) { DrawText("Selection has no path from starts.",x,y,14,muted); return y+22; }
    DrawText(TextFormat("Selected D%d | %d mandatory ancestors",paths->dominated[selected],support->ancestor_count),x,y,14,foreground); y+=23;
    y=resource_text(TextFormat("Support: %d total links | %d native retained | %d earlier arcs not retained",support->length,support->retained,support->length-support->retained),x,y,13,muted);
    GD_EvidencePage page=resource_pager("Support arcs",&evidence->support,support->retained,x,&y,evidence);
    y=path_sources(graph,layout,support->arcs,support->retained,page,x,y);
    if (controls->compare) {
        const GD_PathBypass *bypass=gd_graph_path_bypass(analysis,selected,-1,GD_GRAPH_ANALYSIS_WORK);
        if (!bypass) { DrawText("No nonmandatory node on this support path.",x,y,13,muted); DrawText("Alternatives elsewhere are not ruled out.",x,y+18,13,muted); return y+40; }
        if (!bypass->complete || !bypass->found) { DrawText("Bypass witness unavailable; no partial proof.",x,y,13,muted); return y+22; }
        y=resource_text(TextFormat("C (avoided) #%d:",bypass->candidate),x,y,13,gd_bypass_color);y=resource_text(node_label(graph,bypass->candidate),x,y,13,gd_bypass_color);
        y=resource_text(TextFormat("F (fork) #%d:",bypass->fork),x,y,13,gd_bypass_color);y=resource_text(bypass->fork>=0 ? node_label(graph,bypass->fork):"virtual root (separate starts)",x,y,13,gd_bypass_color);
        y=resource_text(TextFormat("R (rejoin) #%d:",bypass->rejoin),x,y,13,gd_bypass_color);y=resource_text(node_label(graph,bypass->rejoin),x,y,13,gd_bypass_color);
        y=resource_text(TextFormat("Bypass: %d total links | %d native retained | %d earlier arcs not retained",bypass->length,bypass->retained,bypass->length-bypass->retained),x,y,13,muted);
        page=resource_pager("Bypass arcs",&evidence->bypass,bypass->retained,x,&y,evidence);
        y=path_sources(graph,layout,bypass->arcs,bypass->retained,page,x,y);
    }
    return y+20;
}
static int shared_details(const GD_ResourceGraph *graph, const GD_Layout *layout, const GD_GraphAnalysis *analysis, const GD_GraphComparison *comparison, int selected, int pair, int x) {
    int i,y=292;
    DrawText(TextFormat("Shown: %d nodes / %d links",analysis->shown_nodes,analysis->shown_edges),x,198,14,muted);
    if (selected<0) { DrawText("Select a node for shared neighbors",x,220,14,muted); return y; }
    DrawText(TextFormat("%d%s related | %d other neighbors",analysis->matched[selected],analysis->complete ? "":"+ retained",analysis->neighbors[selected]),x,220,14,muted);
    DrawText("Next retained match",x+6,249,14,foreground);
    if (pair>=0) {
        const GD_GraphMatch *match=&analysis->pairs[pair];
        int other=match->a==selected ? match->b:match->a;
        DrawText(TextFormat("%.31s | %d%s shared",node_label(graph,other),match->shared,analysis->complete ? "":"+"),x,274,14,foreground);
        for (i=0;i<2;++i) {
            const GD_GraphFeature *feature=&analysis->features[i ? match->first_b:match->first_a];
            y=resource_text(i ? "B native example declaration:":"A native example declaration:",x,y,12,muted);
            y=source_row(graph,layout->edges[feature->edge].key,x,y);
        }
    } else DrawText(analysis->complete ? "No shared-neighbor match":"No retained match; result is partial",x,274,14,muted);
    if(y<372)y=372;
    y=resource_text(analysis->complete ? (comparison ? "S: global matches | pair branches: one hop":"S: matches | thick purple: example links"):
        comparison ? "PARTIAL discovery | exact one-hop branches":"PARTIAL: counts are lower bounds",x,y,13,muted);
    y=resource_text("Native examples do not retain all parallel declarations. Incident role is not resource flow.",x,y,13,muted);
    if (comparison) {
        y=resource_text("A:",x,y,13,foreground);y=resource_text(node_label(graph,comparison->a),x,y,13,foreground);
        y=resource_text("B:",x,y,13,foreground);y=resource_text(node_label(graph,comparison->b),x,y,13,foreground);
        y=resource_text(TextFormat("Shared %d | A only %d | B only %d | other role %d",comparison->shared,comparison->a_only,comparison->b_only,comparison->other_role),x,y,13,muted);
        y=resource_text("Purple shared | blue A | amber B | gray other role | white mixed links",x,y,13,muted);
    }
    return y+16;
}
static int detail(const GD_ResourceGraph *graph, int selected, int x, int y,GD_ResourceEvidence *evidence) {
    const GD_ResourceNode *node; const GD_Record *source=NULL;
    DrawText("Selection",x,y,20,foreground); y+=32;
    if (selected<0 || selected>=graph->node_count) { DrawText("Select a node",x,y,16,muted); return y+22; }
    node=&graph->nodes[selected];
    DrawText(node->kind==GD_RESOURCE_CASE ? "Rule case":node->kind==GD_RESOURCE_ROLE ? "Resource role":"Actor facet",x,y,17,selected_color); y+=27;
    y=resource_text(node_label(graph,selected),x,y,16,foreground);
    if (node->kind==GD_RESOURCE_CASE) {
        const GD_RuleCase *item=&graph->rules->cases[node->case_index];
        source=&graph->model->records[graph->rules->sources[item->source].record];
        DrawText(TextFormat("case line: %d",item->line),x,y,15,muted); y+=23;
        if (node->instance) {
            y=resource_text("Placement in:",x,y,15,selected_color);y=resource_text(node_label(graph,node->parent),x,y,15,selected_color);
            DrawText(TextFormat("call line %d | template origin above",graph->calls->calls[node->call].line),x,y,14,muted); y+=23;
        }
        y=resource_text("Context:",x,y,15,muted);y=resource_text(item->context,x,y,15,muted);
    } else if (node->kind==GD_RESOURCE_ROLE) {
        if (node->record>=0) source=&graph->model->records[node->record];
        DrawText(TextFormat("resolution: %s",gd_rule_resolution_name(node->resolution)),x,y,15,muted); y+=23;
    } else {
        const GD_RuleParticipant *actor=&graph->rules->participants[node->participant];
        source=&graph->model->records[graph->rules->sources[graph->rules->cases[node->case_index].source].record];
        y=resource_text("Participant:",x,y,15,muted);y=resource_text(actor->name,x,y,15,muted);
        y=resource_text("Noun:",x,y,15,muted);y=resource_text(gd_rule_noun_label(graph->rules,actor),x,y,15,muted);
        y=resource_text("Axis:",x,y,15,muted);y=resource_text(node->axis>=0 ? graph->model->records[node->axis].reference:"unresolved",x,y,15,muted);
    }
    if (source) {
        y=resource_text("Source:",x,y,15,foreground);y=resource_text(source->reference,x,y,15,foreground);
        y=resource_text("Status:",x,y,15,muted);y=resource_text(source->status,x,y,15,muted);
        y=resource_text("Path:",x,y,14,muted);y=resource_text(source->source_path,x,y,14,muted);
    }
    DrawText("Incident declarations:",x,y,16,foreground); y+=23;
    { int i,total=0,ordinal=0;
      for(i=0;i<graph->relation_count;++i)total+=graph->relations[i].facet==selected || graph->relations[i].activation==selected;
      GD_EvidencePage page=resource_pager("Incident declarations",&evidence->incidents,total,x,&y,evidence);
      for (i=0;i<graph->relation_count;++i) {
          const GD_ResourceRelation *edge=&graph->relations[i];
          if ((edge->facet!=selected && edge->activation!=selected) || !gd_evidence_page_take(&page,&ordinal)) continue;
          y=source_row(graph,i,x,y);
          if (edge->kind==GD_RESOURCE_QUANTITY_WRITE) {
              const GD_RuleAssignment *assignment=&graph->rules->assignments[edge->declaration];
              y=resource_text(assignment->expression,x,y,14,muted);
              if (edge->delta_recognized && edge->delta.known) { DrawText(TextFormat("  signed delta: %g",edge->delta.number),x,y,14,muted); y+=20; }
          } else if (edge->kind==GD_RESOURCE_STATE_WRITE) {
              const GD_RuleStateEffect *effect=&graph->rules->state_effects[edge->declaration];
              y=resource_text(graph->rules->participants[edge->mapped_participant].name,x,y,14,muted);y=resource_text("becomes:",x,y,14,muted);y=resource_text(effect->target.state ? effect->target.state:"unresolved state",x,y,14,muted);
          } else if (edge->kind==GD_RESOURCE_REQUIREMENT || edge->kind==GD_RESOURCE_EXCLUSION) {
              const GD_RuleParticipant *actor=&graph->rules->participants[edge->participant];
              const char *condition=edge->kind==GD_RESOURCE_REQUIREMENT ? actor->requires:actor->skip_if;
              y=resource_text(condition,x,y,14,muted);
          }
      }
      if (!total) {DrawText("None retained",x,y,14,muted);y+=20;}
    }
    return resource_text("Presentation pages cover retained declarations, not omitted or unexamined source.",x,y,12,muted);
}
#ifdef GD_EXPLORER_TEST
/* Hidden-window helper checks only; never opens a window or sends OS input. */
int gd_resource_evidence_checks(const GD_ResourceGraph *graph,GD_Layout *layout,GD_GraphAnalysis *analysis,int *calls) {
    int checks=0,allocations=*calls,y,cursor=0,selected=-1,total=0;GD_ResourceEvidence evidence={0};GD_EvidencePage page;
#define RESOURCE_PAGE_CHECK(test) do{++checks;if(!(test)){fprintf(stderr,"Resource paging failed: %s (line %d)\n",#test,__LINE__);return 0;}}while(0)
    RESOURCE_PAGE_CHECK(resource_layout(graph,layout));gd_graph_analyze(layout,NULL,1,GD_GRAPH_ANY,analysis);
    const unsigned char *bytes=(const unsigned char *)layout;uint64_t before=1,after=1;
    for(size_t i=0;i<sizeof(*layout);++i)before=(before^bytes[i])*UINT64_C(1099511628211);
    Rectangle panel={400,100,380,400};resource_sidebar_input(&evidence,panel,0,1000,(Vector2){450,200},-2,1);
    RESOURCE_PAGE_CHECK(evidence.scroll[0]==84 && evidence.mouse.y==284 && evidence.clicked);
    resource_sidebar_input(&evidence,panel,1,1000,(Vector2){450,200},-1,0);RESOURCE_PAGE_CHECK(evidence.scroll[0]==84 && evidence.scroll[1]==42);
    resource_sidebar_input(&evidence,panel,0,1000,(Vector2){200,200},-100,1);RESOURCE_PAGE_CHECK(evidence.scroll[0]==84 && evidence.mouse.y==200 && !evidence.clicked);
    resource_sidebar_input(&evidence,panel,0,200,(Vector2){450,200},0,0);RESOURCE_PAGE_CHECK(evidence.scroll[0]==0 && evidence.mouse.y==200);
    BeginDrawing();evidence.clicked=1;evidence.mouse=(Vector2){104,108};y=100;page=resource_pager("Test",&cursor,9,14,&y,&evidence);EndDrawing();
    RESOURCE_PAGE_CHECK(cursor==1 && page.first==4 && page.end==8);
    BeginDrawing();y=100;page=resource_pager("Test",&cursor,9,14,&y,&evidence);EndDrawing();RESOURCE_PAGE_CHECK(cursor==2 && page.first==8 && page.end==9);
    BeginDrawing();y=100;page=resource_pager("Test",&cursor,9,14,&y,&evidence);EndDrawing();RESOURCE_PAGE_CHECK(cursor==2 && page.end==9);
    evidence.mouse.x=20;BeginDrawing();y=100;page=resource_pager("Test",&cursor,9,14,&y,&evidence);EndDrawing();RESOURCE_PAGE_CHECK(cursor==1 && page.first==4);
    evidence.clicked=0;
    for(int rows=0;rows<=9;++rows){cursor=100;BeginDrawing();y=100;page=resource_pager("Boundary",&cursor,rows,14,&y,&evidence);EndDrawing();RESOURCE_PAGE_CHECK(page.end==rows && cursor==(rows ? (rows-1)/4:0));}
    for(int node=0;node<graph->node_count;++node){int matches=0;for(int row=0;row<graph->relation_count;++row)matches+=graph->relations[row].facet==node || graph->relations[row].activation==node;if(matches>total){total=matches;selected=node;}}
    if(selected>=0){
        int reached=0;
        for(int index=0;index<(total+3)/4;++index){int ordinal=0,returned=0;page=gd_evidence_page(total,4,index);
            for(int row=0;row<graph->relation_count;++row)if(graph->relations[row].facet==selected || graph->relations[row].activation==selected){if(gd_evidence_page_take(&page,&ordinal)){RESOURCE_PAGE_CHECK(ordinal-1==reached);++reached;++returned;}}
            evidence.incidents=index;BeginDrawing();y=detail(graph,selected,414,100,&evidence);EndDrawing();RESOURCE_PAGE_CHECK(evidence.incidents==index && returned==page.end-page.first && y>100);
        }
        RESOURCE_PAGE_CHECK(reached==total);
        int arcs[64]={0};evidence.support=15;BeginDrawing();y=100;page=resource_pager("Native retained support",&evidence.support,64,14,&y,&evidence);y=path_sources(graph,layout,arcs,64,page,414,y);EndDrawing();
        RESOURCE_PAGE_CHECK(page.first==60 && page.end==64 && evidence.support==15 && y>100);
    }
    for(size_t i=0;i<sizeof(*layout);++i)after=(after^bytes[i])*UINT64_C(1099511628211);
    RESOURCE_PAGE_CHECK(before==after && *calls==allocations);fprintf(stderr,"Resource paging: %d checks; %d matching incident rows; presentation helpers only\n",checks,total);
#undef RESOURCE_PAGE_CHECK
    return checks;
}
#endif
static void draw_resource_graph(const GD_ResourceGraph *graph,GD_Layout *raw, GD_Layout *layout, GD_GraphAnalysis *analysis,
    Camera2D camera, Rectangle viewport, int selected, int shared, int pair, const GD_GraphComparison *comparison,
    const GD_GraphPaths *paths, const GD_PathBypass *bypass, const GD_GraphClosure *reachability,
    const GD_GraphComparison *reach_pair, const GD_GraphShapes *shapes, const GD_GraphOverview *overview) {
    int i; const GD_PathSelection *path_selection=paths ? gd_graph_path_selection(analysis,selected):NULL;
    BeginScissorMode((int)viewport.x,(int)viewport.y,(int)viewport.width,(int)viewport.height); BeginMode2D(camera);
    for (i=0;i<layout->edge_count;++i) {
        const GD_LayoutEdge *edge=&layout->edges[i]; const GD_ResourceRelation *rel=&graph->relations[edge->key];
        const GD_LayoutNode *a=&layout->nodes[edge->a],*b=&layout->nodes[edge->b];
        Color color=rel->kind==GD_RESOURCE_ROLE_BINDING ? role_color : rel->kind==GD_RESOURCE_INVOCATION ? selected_color :
            rel->kind==GD_RESOURCE_EXCLUSION ? (Color){230,125,125,255} : rel->kind==GD_RESOURCE_REQUIREMENT ? (Color){81,194,178,255} :
            rel->kind==GD_RESOURCE_PRESERVATION ? (Color){180,201,119,255} : facet_color;
        if (!analysis->shown[edge->a] || !analysis->shown[edge->b]) continue;
        if (overview && overview->selected_edges[i]) DrawLineEx((Vector2){a->x,a->y},(Vector2){b->x,b->y},21,selected_color);
        if (reach_pair && reach_pair->edges[i]) DrawLineEx((Vector2){a->x,a->y},(Vector2){b->x,b->y},17,gd_graph_branch_color(reach_pair->edges[i]));
        if (bypass && bypass->complete && bypass->found && bypass->edges[i]) DrawLineEx((Vector2){a->x,a->y},(Vector2){b->x,b->y},15,gd_bypass_color);
        if (path_selection && path_selection->edges[i]) DrawLineEx((Vector2){a->x,a->y},(Vector2){b->x,b->y},11,gd_path_color);
        if (comparison && comparison->edges[i]) DrawLineEx((Vector2){a->x,a->y},(Vector2){b->x,b->y},7,gd_graph_branch_color(comparison->edges[i]));
        else if (!comparison && shared && pair>=0 && (i==analysis->features[analysis->pairs[pair].first_a].edge || i==analysis->features[analysis->pairs[pair].first_b].edge))
            DrawLineEx((Vector2){a->x,a->y},(Vector2){b->x,b->y},7,role_color);
        DrawLineEx((Vector2){a->x,a->y},(Vector2){b->x,b->y},1.5f,color);
    }
    for (i=0;i<layout->node_count;++i) {
        GD_LayoutNode *n=&layout->nodes[i],*physical=&raw->nodes[i]; const GD_ResourceNode *item=&graph->nodes[n->key];
        Color color=item->kind==GD_RESOURCE_CASE ? case_color:item->kind==GD_RESOURCE_ROLE ? role_color:facet_color;
        if (!analysis->shown[i]) continue;
        gd_layout_set_footprint(physical,-106,-43,106,45);
        DrawRectangleRounded((Rectangle){n->x-90,n->y-24,180,48},0.15f,5,(Color){28,38,50,255});
        DrawRectangleRoundedLinesEx((Rectangle){n->x-90,n->y-24,180,48},0.15f,5,selected==n->key ? 3:1.5f,selected==n->key ? selected_color:color);
        gd_graph_node_text_at(physical,n,item->kind==GD_RESOURCE_CASE ? "case":item->kind==GD_RESOURCE_ROLE ? "resource role":"facet",(int)n->x-83,(int)n->y-19,11,color,1);
        gd_graph_node_text_at(physical,n,TextFormat("%.23s",node_label(graph,n->key)),(int)n->x-83,(int)n->y-3,14,foreground,1);
        if (shared) gd_graph_node_text_at(physical,n,TextFormat("S%d%s",analysis->matched[i],analysis->complete ? "":"+"),(int)n->x-83,(int)n->y+28,13,role_color,1);
        if (comparison) {
            unsigned category=comparison->nodes[i];
            if (category) {
                Color branch_color=gd_graph_branch_color(category);
                DrawRectangle((int)n->x+86,(int)n->y-18,2,36,branch_color);
                gd_graph_footprint_rect_at(physical,n,gd_graph_branch_badge(category,(int)n->x-35,(int)n->y+28,background),1);
            }
            if (i==comparison->a || i==comparison->b) gd_graph_node_text_at(physical,n,i==comparison->a ? "A":"B",(int)n->x+70,(int)n->y+28,13,foreground,1);
        }
        if (overview && overview->selected_nodes[i]) DrawRectangleLinesEx((Rectangle){n->x-106,n->y-40,212,80},2,selected_color);
        if (shapes) {
            gd_graph_footprint_rect_at(physical,n,gd_graph_shape_badge(shapes,i,(int)n->x-83,(int)n->y+45,background),1);
            if (gd_graph_shape_peer(shapes,i,selected)) DrawRectangleLinesEx((Rectangle){n->x-102,n->y-36,204,72},2,(Color){220,205,148,255});
        }
        if (reachability) {
            gd_graph_footprint_rect_at(physical,n,gd_graph_reachability_badge(reachability,i,(int)n->x-83,(int)n->y-43,background),1);
            if (reach_pair && reach_pair->nodes[i]) DrawRectangleLinesEx((Rectangle){n->x-98,n->y-32,196,64},2,gd_graph_branch_color(reach_pair->nodes[i]));
        }
        if (paths) {
            gd_graph_footprint_rect_at(physical,n,gd_graph_path_badge(paths,path_selection,bypass,i,(int)n->x,(int)n->y-20,background),1);
            if (paths->roots[i]) gd_graph_path_start_marker((int)n->x+76,(int)n->y-15);
        }
    }
    EndMode2D(); EndScissorMode();
}
int gd_resource_view(GD_Model *model, GD_ResourceGraph *graph, int frames, const char *screenshot) {
    GD_Layout *layout=model->allocator.allocate(model->allocator.context,sizeof(*layout));
    GD_GraphAnalysis *analysis=model->allocator.allocate(model->allocator.context,sizeof(*analysis));
    Camera2D camera={0}; int frame=0, selected=graph->node_count ? 0:-1,held=-1,stale=0,result=0,paused=1,i;
    int shared=1,branches=0,peer=-1,pair=-1; double next_check=0;
    GD_GraphLens shared_lens=GD_GRAPH_ANY;
    GD_PathControls path_controls={0}; int path_tab=0;
    GD_ReachControls reach_controls={0,-1,0,-1,-1}; int shape_enabled=0;
    GD_OverviewControls overview_controls={1,-1,-1};
    GD_ResourceEvidence evidence={0};float content_bottom[6]={0};
    evidence.selected=evidence.row=evidence.column=evidence.target=evidence.relation=evidence.peer=-1;
    if (!layout || !analysis || !resource_layout(graph,layout) || !gd_layout_prepare_free(layout,model->allocator,1.4f,1)) {
        if (analysis) model->allocator.release(model->allocator.context,analysis);
        if (layout) model->allocator.release(model->allocator.context,layout);
        gd_error(model,"resources.layout","Cannot allocate or arrange resource preview",NULL,0); return 2;
    }
    memset(analysis,0,sizeof(*analysis)); gd_graph_analyze(layout,NULL,1,shared_lens,analysis);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE|FLAG_MSAA_4X_HINT); InitWindow(1440,900,"Resource graph preview"); SetExitKey(KEY_NULL);SetTargetFPS(60);
    for (i=0;i<120;++i) gd_layout_step(layout,-1);
    fit(&camera,layout,(Rectangle){0,100,1060,755});
    while (!WindowShouldClose() && (!frames || frame<frames)) {
        int width=GetScreenWidth(),height=GetScreenHeight(),split=width>700 ? width-380:width*2/3,i;
        Rectangle viewport={0,100,(float)split,(float)(height-145)};
        Vector2 mouse=GetMousePosition();
        camera.offset=(Vector2){viewport.x+viewport.width/2,viewport.y+viewport.height/2};
        if (GetTime()>=next_check) { if (gd_sources_changed(model)) stale=1; next_check=GetTime()+1; }
        if(IsKeyPressed(KEY_ESCAPE)){selected=held=-1;overview_controls.row=overview_controls.column=-1;analysis->initialized=0;}
        if(IsKeyPressed(KEY_F)){gd_graph_analyze(layout,NULL,1,shared_lens,analysis);if(!gd_graph_frame_selection(&camera,layout,analysis,&overview_controls,selected,-1,viewport,1))fit(&camera,layout,viewport);}
        if (IsKeyPressed(KEY_SPACE)) paused=!paused;
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse,(Rectangle){(float)split+14,12,340,28})){
            gd_layout_islands_set(layout,!layout->islands.preferred);if(layout->islands.enabled)paused=0;
        }
        if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) held=-1;
        if (gd_graph_overview_hit(&overview_controls,viewport,mouse)) held=-1;
        if (CheckCollisionPointRec(mouse,viewport) && !gd_graph_overview_hit(&overview_controls,viewport,mouse)) {
            Vector2 world=GetScreenToWorld2D(mouse,camera);
            float wheel=GetMouseWheelMove();
            if (wheel) { Vector2 after; camera.zoom*=powf(1.15f,wheel); if(camera.zoom<0.002f)camera.zoom=0.002f; if(camera.zoom>4)camera.zoom=4; after=GetScreenToWorld2D(mouse,camera); camera.target.x+=world.x-after.x; camera.target.y+=world.y-after.y; }
            if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) || IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) { Vector2 delta=GetMouseDelta(); camera.target.x-=delta.x/camera.zoom; camera.target.y-=delta.y/camera.zoom; }
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) for (i=layout->node_count-1;i>=0;--i) {
                const GD_LayoutNode *n=&layout->nodes[i];
                if (CheckCollisionPointRec(world,(Rectangle){n->x-90,n->y-24,180,48})) { selected=n->key; held=i; gd_layout_pointer_begin(layout,i,world.x,world.y,mouse.x,mouse.y); break; }
            }
            if (held>=0 && IsMouseButtonDown(MOUSE_BUTTON_LEFT) && gd_layout_pointer_dragged(layout,held,mouse.x,mouse.y)) gd_layout_drag_node(layout,held,world.x,world.y);
        }
        if (!paused) gd_layout_step(layout,held);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse,(Rectangle){(float)split+14,72,100,25})) path_tab=0;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse,(Rectangle){(float)split+124,72,70,25})) path_tab=1;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse,(Rectangle){(float)split+204,72,140,25})) path_tab=path_tab>=2 && path_tab<5 ? path_tab+1:2;
        Rectangle sidebar={(float)split,100,(float)(width-split),(float)(height-145)};
        resource_sidebar_input(&evidence,sidebar,path_tab,content_bottom[path_tab],mouse,GetMouseWheelMove(),IsMouseButtonPressed(MOUSE_BUTTON_LEFT));mouse=evidence.mouse;
        if (path_tab==5) {
            if (evidence.clicked && CheckCollisionPointRec(mouse,(Rectangle){(float)split+14,110,340,25})) overview_controls.enabled=!overview_controls.enabled;
            if (overview_controls.enabled) {
                const GD_GraphOverview *overview=gd_graph_overview(analysis,&overview_controls,1);
                int row=overview_controls.row<0 ? 0:overview_controls.row,column=overview_controls.column<0 ? 0:overview_controls.column,changed=0;
                if (evidence.clicked) {
                    if (overview->bins && CheckCollisionPointRec(mouse,(Rectangle){(float)split+14,140,150,25})) { row=(row+1)%overview->bins; changed=1; }
                    if (overview->bins && CheckCollisionPointRec(mouse,(Rectangle){(float)split+174,140,170,25})) { column=(column+1)%overview->bins; changed=1; }
                    if (overview->bins && CheckCollisionPointRec(mouse,(Rectangle){(float)split+14,172,340,25})) {
                        int cells=overview->bins*overview->bins,start=overview_controls.row<0 ? -1:row*overview->bins+column;
                        for (i=1;i<=cells;++i) { int next=(start+i)%cells; if (overview->counts[next]) { row=next/overview->bins; column=next%overview->bins; changed=1; break; } }
                    }
                    if (CheckCollisionPointRec(mouse,(Rectangle){(float)split+14,204,340,25})) { row=column=-1; changed=1; }
                }
                if (overview->bins && (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_DOWN))) {
                    column+=IsKeyPressed(KEY_RIGHT)-IsKeyPressed(KEY_LEFT); row+=IsKeyPressed(KEY_DOWN)-IsKeyPressed(KEY_UP);
                    if (column<0) column=0; if (row<0) row=0; if (column>=overview->bins) column=overview->bins-1; if (row>=overview->bins) row=overview->bins-1; changed=1;
                }
                if (changed) { overview_controls.row=row; overview_controls.column=column; gd_graph_overview_select(analysis,row,column); }
            }
        }
        if (path_tab==4 && evidence.clicked) {
            if (CheckCollisionPointRec(mouse,(Rectangle){(float)split+14,110,340,25})) shape_enabled=!shape_enabled;
            if (shape_enabled && CheckCollisionPointRec(mouse,(Rectangle){(float)split+14,172,340,25})) {
                const GD_GraphShapes *shapes=resource_shapes(graph,analysis,1); int family=selected>=0 ? shapes->colors[selected]:-1;
                if (shapes->complete) for (i=0;i<shapes->family_count;++i) {
                    int next=(family+1+i)%shapes->family_count;
                    if (shapes->size[next]>1) { selected=shapes->representative[next]; break; }
                }
            }
        }
        if (path_tab==3 && evidence.clicked) {
            if (CheckCollisionPointRec(mouse,(Rectangle){(float)split+14,110,340,25})) reach_controls.enabled=!reach_controls.enabled;
            if (reach_controls.enabled && CheckCollisionPointRec(mouse,(Rectangle){(float)split+14,140,340,25})) next_reach_kind(analysis,&reach_controls);
            if (reach_controls.enabled && CheckCollisionPointRec(mouse,(Rectangle){(float)split+14,172,340,25})) {
                const GD_GraphClosure *closure=gd_graph_reachability_prepare(analysis,&reach_controls,1); int i;
                if (closure && gd_graph_closure_choose_into(analysis,&analysis->reachability,selected,&reach_controls.peer) && closure->peer_count) {
                    for (i=0;i<closure->peer_count && closure->peers[i]!=reach_controls.peer;++i) {}
                    reach_controls.peer=closure->peers[(i+1)%closure->peer_count];
                }
            }
            if (reach_controls.enabled && CheckCollisionPointRec(mouse,(Rectangle){(float)split+14,296,340,25})) {
                const GD_GraphClosure *closure=gd_graph_reachability_prepare(analysis,&reach_controls,1);
                const GD_GraphComparison *comparison=closure ? gd_graph_closure_choose_into(analysis,&analysis->reachability,selected,&reach_controls.peer):NULL;
                if (comparison) for (i=0;i<layout->node_count;++i) {
                    int node=(reach_controls.target+1+i)%layout->node_count;
                    if (comparison->nodes[node]) { reach_controls.target=node; break; }
                }
            }
        }
        if (path_tab==1 && evidence.clicked) {
            if (CheckCollisionPointRec(mouse,(Rectangle){(float)split+14,110,340,25})) path_controls.enabled=!path_controls.enabled;
            if (path_controls.enabled && selected>=0 && CheckCollisionPointRec(mouse,(Rectangle){(float)split+14,144,320,25})) gd_graph_path_start(&path_controls,selected);
            if (path_controls.enabled && CheckCollisionPointRec(mouse,(Rectangle){(float)split+14,176,240,25})) path_controls.count=0;
            if (path_controls.enabled && CheckCollisionPointRec(mouse,(Rectangle){(float)split+14,208,340,25})) path_controls.compare=!path_controls.compare;
            if (path_controls.enabled && path_controls.compare && CheckCollisionPointRec(mouse,(Rectangle){(float)split+14,240,340,25})) {
                const GD_PathSelection *support=gd_graph_path_selection(analysis,selected); int i;
                if (support && support->candidate_count) {
                    for (i=0;i<support->candidate_count;++i) if (support->candidates[i]==support->candidate) break;
                    analysis->paths.selection.candidate=support->candidates[(i+1)%support->candidate_count];
                }
            }
        }
        if (!path_tab && evidence.clicked && CheckCollisionPointRec(mouse,(Rectangle){(float)split+14,110,340,25})) shared=!shared;
        if (!path_tab && shared && evidence.clicked && CheckCollisionPointRec(mouse,(Rectangle){(float)split+14,136,340,25})) shared_lens=(GD_GraphLens)((shared_lens+1)%GD_GRAPH_LENS_COUNT);
        if (!path_tab && shared && evidence.clicked && CheckCollisionPointRec(mouse,(Rectangle){(float)split+14,162,340,25})) branches=!branches;
        if (gd_graph_analyze(layout,NULL,1,shared_lens,analysis)) peer=-1;
        if (!frame && analysis->pair_count) selected=analysis->pairs[0].a;
        pair=shared_choice(analysis,selected,&peer,!path_tab && shared && evidence.clicked && CheckCollisionPointRec(mouse,(Rectangle){(float)split+14,242,190,25}));
        const GD_GraphComparison *comparison=shared && branches && pair>=0 ? gd_graph_compare(analysis,selected,peer):NULL;
        const GD_GraphPaths *paths=gd_graph_paths_prepare(analysis,&path_controls,NULL,1);
        const GD_PathBypass *bypass=paths && path_controls.compare ? gd_graph_path_bypass(analysis,selected,-1,GD_GRAPH_ANALYSIS_WORK):NULL;
        const GD_GraphClosure *reachability=gd_graph_reachability_prepare(analysis,&reach_controls,1);
        const GD_GraphComparison *reach_pair=reachability ? gd_graph_closure_choose_into(analysis,&analysis->reachability,selected,&reach_controls.peer):NULL;
        const GD_GraphShapes *shapes=resource_shapes(graph,analysis,shape_enabled);
        const GD_GraphOverview *overview=gd_graph_overview(analysis,&overview_controls,1);
        BeginDrawing(); ClearBackground(background);
        DrawText("Resource graph preview",18,14,26,foreground);
        DrawRectangleLines(split+14,12,340,28,muted);
        DrawText(layout->islands.enabled ? "[x] Separate connected islands":"[ ] Separate connected islands",split+22,18,16,foreground);
        DrawText("Direct and instantiated declarations; not final outcomes. Incidence lines are not proof of flow.",18,49,16,selected_color);
        DrawText(TextFormat("%d nodes | %d relations | %d unresolved facets | %d unexamined calls%s",graph->node_count,graph->relation_count,graph->unresolved_facets,graph->composition_unexamined,graph->limited ? " | GRAPH LIMIT REACHED":""),18,74,15,muted);
        draw_resource_graph(graph,layout,layout,analysis,camera,viewport,selected,shared,pair,comparison,
                            paths,bypass,reachability,reach_pair,shapes,overview);
        gd_graph_workspace_draw(layout,layout,analysis->shown,camera,viewport,1);
        gd_graph_islands_draw(layout,layout,analysis->shown,camera,viewport,1);
        gd_graph_overview_draw(analysis,&overview_controls,viewport,selected,1);
        DrawRectangle(split,100,width-split,height-100,(Color){29,38,51,255});
        DrawText(path_tab==0 ? "[Context]":"Context",split+14,76,14,foreground);
        DrawText(path_tab==1 ? "[Paths]":"Paths",split+124,76,14,foreground);
        DrawText(path_tab==5 ? "[Overview]":path_tab==4 ? "[Shapes]": path_tab==3 ? "[Typed paths]":path_tab==2 ? "[Foundations]":"Layers (click)",split+204,76,12,foreground);
        BeginScissorMode(split,100,width-split,height-145);
        Camera2D sidebar_camera={0};sidebar_camera.zoom=1;sidebar_camera.offset.y=-evidence.scroll[path_tab];BeginMode2D(sidebar_camera);
        int bottom=280;
        if(selected!=evidence.selected){evidence.members=evidence.incidents=evidence.support=evidence.bypass=evidence.witness[0]=evidence.witness[1]=0;evidence.selected=selected;}
        if(overview_controls.row!=evidence.row || overview_controls.column!=evidence.column){evidence.sources=evidence.bin[0]=evidence.bin[1]=0;evidence.row=overview_controls.row;evidence.column=overview_controls.column;}
        if(reach_controls.target!=evidence.target || reach_controls.relation!=evidence.relation || reach_controls.peer!=evidence.peer){evidence.witness[0]=evidence.witness[1]=0;evidence.target=reach_controls.target;evidence.relation=reach_controls.relation;evidence.peer=reach_controls.peer;}
        if (path_tab==2) {
            DrawText("Prerequisite foundations: unavailable",split+14,114,15,foreground);
            DrawText("This projection has resource/rule facets,",split+14,145,14,muted);
            DrawText("not shown skill-prerequisite declarations.",split+14,165,14,muted);
            DrawText("No hidden skill graph is analyzed.",split+14,195,14,muted);
            DrawText("Resource incidences are not AND skills,",split+14,225,14,muted);
            DrawText("learning requirements or proof of flow.",split+14,245,14,muted);
        }
        else if (path_tab==5) bottom=overview_details(graph,layout,analysis,&overview_controls,split+14,&evidence);
        else if (path_tab==4) bottom=shape_details(graph,shapes,shape_enabled,selected,split+14,&evidence);
        else if (path_tab==3) bottom=reach_details(graph,layout,analysis,&reach_controls,selected,split+14,&evidence);
        else if (path_tab==1) bottom=path_details(graph,layout,analysis,paths,&path_controls,selected,split+14,&evidence);
        else {
        DrawText(shared ? "[x] Shared neighbors (click)":"[ ] Shared neighbors (click)",split+14,114,16,foreground);
        if (shared) {
            DrawText(TextFormat("Lens: %s (click)",gd_graph_lens_names[shared_lens]),split+14,140,14,foreground);
            DrawText(branches ? "[x] Compare branches (click)":"[ ] Compare branches (click)",split+14,166,14,foreground);
            DrawRectangleLines(split+14,242,190,25,muted);bottom=shared_details(graph,layout,analysis,comparison,selected,pair,split+14);
        }
        bottom=detail(graph,selected,split+14,shared ? bottom:150,&evidence);
        }
        content_bottom[path_tab]=(float)bottom+16;
        EndMode2D();EndScissorMode();
        if(content_bottom[path_tab]>height-45)DrawText("Wheel over sidebar: scroll evidence",split+14,height-59,12,muted);
        if (stale) DrawText("STALE: sources changed. Close and reopen preview.",18,height-38,15,selected_color);
        else DrawText("Drag nodes | Right drag: pan | Wheel: zoom | F: fit | Space: settle/pause",18,height-38,15,muted);
        DrawText("Edges: teal condition | red exclusion | blue write | green preservation | purple role | gold call",18,height-20,13,muted);
        EndDrawing(); ++frame;
        if (screenshot && frame==frames && !gd_save_screenshot(screenshot)) { gd_error(model,"view.screenshot","Resource preview screenshot failed",screenshot,0); result=2; }
    }
    CloseWindow(); model->allocator.release(model->allocator.context,analysis); model->allocator.release(model->allocator.context,layout); return result;
}

#ifdef GD_WITH_GRAPH
#include "gd_explorer_controls.h"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_NO_EXPORT
#include "cimgui.h"
int gd_explorer_resource_prepare(GD_ExplorerContext *context) {
    return resource_layout(context->project->resources,context->pane->layout);
}
void gd_explorer_resource_draw(GD_ExplorerContext *context, const GD_ExplorerEvidence *e) {
    GD_ExplorerPane *p=context->pane;const GD_Layout *shown=gd_explorer_display_layout(context);
    int pair=p->shared ? gd_graph_match(context->analysis,p->selection.node,p->peer):-1;
    const GD_GraphComparison *comparison=p->branches && pair>=0 ? gd_graph_compare(context->analysis,p->selection.node,p->peer):NULL;
    if (p->resource_routes && p->resource_routes->graph==context->project->resources) {
        const GD_ResourceRoutes *query=p->resource_routes; int i;
        BeginScissorMode((int)context->viewport.x,(int)context->viewport.y,(int)context->viewport.width,(int)context->viewport.height); BeginMode2D(p->camera);
        for (i=0;i<p->layout->edge_count;++i) if (query->marked_relations[i] || query->side_relations[i]) {
            const GD_LayoutEdge *edge=&p->layout->edges[i];
            if (context->analysis->shown[edge->a] && context->analysis->shown[edge->b])
                DrawLineEx((Vector2){shown->nodes[edge->a].x,shown->nodes[edge->a].y},(Vector2){shown->nodes[edge->b].x,shown->nodes[edge->b].y},
                    (query->marked_relations[i] ? 9:5)/p->camera.zoom,query->marked_relations[i] ? (Color){45,214,199,150}:(Color){243,186,87,115});
        }
        if (p->resource_witness>=0 && p->resource_witness<query->witness_count) {
            const GD_ResourceWitness *w=&query->witnesses[p->resource_witness];
            for (i=0;i<w->count;++i) {
                const GD_ResourceArc *arc=&query->arcs[w->arcs[i]];
                const GD_LayoutNode *a=&shown->nodes[arc->from],*b=&shown->nodes[arc->to];
                float dx=b->x-a->x,dy=b->y-a->y,length=hypotf(dx,dy),size=8/p->camera.zoom;
                if (!context->analysis->shown[arc->from] || !context->analysis->shown[arc->to] || length<2*size) continue;
                dx/=length; dy/=length;
                { Vector2 tip={a->x+(b->x-a->x)*0.6f,a->y+(b->y-a->y)*0.6f};
                  DrawTriangle(tip,(Vector2){tip.x-dx*size-dy*size/2,tip.y-dy*size+dx*size/2},(Vector2){tip.x-dx*size+dy*size/2,tip.y-dy*size-dx*size/2},(Color){45,214,199,255}); }
            }
        }
        EndMode2D(); EndScissorMode();
    }
    draw_resource_graph(context->project->resources,p->layout,gd_explorer_display_layout(context),context->analysis,p->camera,context->viewport,p->selection.node,
                        p->shared,pair,comparison,e->paths,e->bypass,e->reachability,e->reach_pair,e->shapes,e->overview);
    /* Separate provenance overlay: canonical declarations only, never a claim
       that each mapped placement applies to the independent rule scenario. */
    if (p->rule_state && (p->rule_state->result || p->rule_state->bindings)) {
        int i; BeginScissorMode((int)context->viewport.x,(int)context->viewport.y,(int)context->viewport.width,(int)context->viewport.height); BeginMode2D(p->camera);
        for (i=0;i<p->layout->node_count;++i) {
            const GD_LayoutNode *node=&shown->nodes[i]; const GD_ResourceNode *source=&context->project->resources->nodes[node->key];
            if (context->analysis->shown[i] && source->kind==GD_RESOURCE_CASE && !source->instance && gd_explorer_rules_marked(context,source->case_index))
                DrawRectangleLinesEx((Rectangle){node->x-97,node->y-31,194,62},3/p->camera.zoom,(Color){198,133,245,255});
        }
        EndMode2D(); EndScissorMode();
    }
}
void gd_explorer_resource_source(GD_ExplorerContext *context, int edge) {
    const GD_ResourceGraph *graph=context->project->resources;
    int key=context->pane->layout->edges[edge].key;
    const GD_Record *source=relation_source(graph,key);
    igTextWrapped("%s | line %d | %s",relation_name(graph->relations[key].kind),graph->relations[key].line,
                  source ? source->reference:"unresolved source");
    if (source) igTextWrapped("%s",source->source_path);
}
int gd_explorer_resource_run(GD_ExplorerContext *context) {
    GD_ExplorerPane *p=context->pane; GD_ResourceRoutes *next;
    if (p->resource_pending.to<0) { snprintf(p->resource_message,sizeof(p->resource_message),"Select a concrete target node first."); return 0; }
    next=gd_resource_routes_create(context->project->allocator);
    if (!next) { snprintf(p->resource_message,sizeof(p->resource_message),"Cannot allocate bounded route scratch; previous result retained."); return 0; }
    if (!gd_resource_query(next,context->project->resources,p->resource_pending.from,p->resource_pending.to,p->resource_pending.accepted,p->resource_pending.work_limit)) {
        gd_resource_routes_destroy(next); snprintf(p->resource_message,sizeof(p->resource_message),"Invalid endpoints or work ceiling; previous result retained."); return 0;
    }
    gd_resource_routes_destroy(p->resource_routes); p->resource_routes=next; p->resource_applied=p->resource_pending;
    p->resource_witness=next->witness_count ? 0:-1; *p->resource_message=0; context->analysis->initialized=0; return 1;
}
void gd_explorer_resource_controls(GD_ExplorerContext *context) {
    GD_ExplorerPane *p=context->pane; const GD_ResourceGraph *graph=context->project->resources;
    int selected=p->selection.node,i;
    igSeparator(); igTextUnformatted("Conditional resource witnesses",NULL);
    igTextWrapped("Whole loaded Resource snapshot, not camera/filter scope. Teal arrows: selected witness. Amber: side guard/write declarations, not necessarily consumed or AND inputs. Feasibility is not checked. Mapped calls expose concrete placements. Unresolved/limited calls and unknown-direction writes remain frontiers.");
    igTextWrapped("Pending from: %s | to: %s",p->resource_pending.from>=0 ? node_label(graph,p->resource_pending.from):"any upstream declaration",p->resource_pending.to>=0 ? node_label(graph,p->resource_pending.to):"not selected");
    igBeginDisabled(selected<0 || selected>=graph->node_count || p->selection.linked>=0);
    if (igButton("Use selected as FROM",(ImVec2_c){0,0})) p->resource_pending.from=selected;
    igSameLine(0,8);
    if (igButton("Use selected as TO",(ImVec2_c){0,0})) p->resource_pending.to=selected;
    igEndDisabled();
    if (igButton("Any upstream (clear FROM)",(ImVec2_c){0,0})) p->resource_pending.from=-1;
    gd_explorer_form_widget(GD_FORM_RESOURCE,&p->resource_pending,"accepted",NULL);
    gd_explorer_form_widget(GD_FORM_RESOURCE,&p->resource_pending,"work_limit",NULL);
    if (p->resource_routes) {
        const GD_ExplorerResourceForm *applied=&p->resource_applied;
        igTextWrapped("Applied from: %s | to: %s | accepted-only: %s | work ceiling: %d",applied->from>=0 ? node_label(graph,applied->from):"any upstream declaration",node_label(graph,applied->to),applied->accepted ? "yes":"no",applied->work_limit);
        if (!gd_explorer_form_equal(GD_FORM_RESOURCE,&p->resource_pending,applied)) igTextWrapped("Inputs pending. Witnesses and highlights retain applied scope until a successful Run.");
    }
    gd_explorer_action_button(context,GD_ACTION_RESOURCE_RUN);
    if (*p->resource_message) igTextWrapped("Last Run: %s",p->resource_message);
    if (p->resource_routes) {
        GD_ResourceRoutes *query=p->resource_routes;
        igTextWrapped("%d retained witnesses | work %d/%d | work/witness limit %s | depth limit %s | cycles skipped %d",query->witness_count,query->work,query->work_limit,query->limited ? "yes":"no",query->depth_limited ? "yes":"no",query->cycle_edges);
        igTextWrapped("Unknown-direction writes %d | unexpanded calls %d | excluded cases %d. No known witness is not impossibility; raw/capped expressions remain in source inspector.",query->unknown_writes,query->unexpanded_calls,query->excluded_cases);
        if (!query->to_bindings || (query->from>=0 && !query->from_bindings)) igTextWrapped("Endpoint has no explicit facet binding. Resource noun/type alone does not identify a shared pool.");
        if (query->witness_count && igSliderInt("Witness",&p->resource_witness,0,query->witness_count-1,"%d",0)) gd_resource_route_mark(query,p->resource_witness);
        if (p->resource_witness>=0) {
            if (igCheckbox("Only witness + side declarations (display filter)",&p->resource_only_witness)) p->fit=GD_EXPLORER_FIT_RESOURCE_WITNESS;
            if (igButton("Frame witness + side declarations",(ImVec2_c){0,0})) p->fit=GD_EXPLORER_FIT_RESOURCE_WITNESS;
            const GD_ResourceWitness *w=&query->witnesses[p->resource_witness]; int hidden=0;
            for (i=0;i<graph->node_count;++i) if (query->marked_nodes[i] && !context->analysis->shown[i]) ++hidden;
            igTextWrapped("Conditional, feasibility not checked | proposal reliance %s | unresolved side facts %s | %d highlighted nodes hidden by display filters",w->proposed ? "yes":"no",w->unresolved ? "yes":"no",hidden);
            for (i=0;i<w->count;++i) {
                const GD_ResourceArc *arc=&query->arcs[w->arcs[i]];
                igTextWrapped("%s: %s -> %s",gd_resource_arc_name(arc->kind),node_label(graph,arc->from),node_label(graph,arc->to));
            }
            if (igCollapsingHeader_TreeNodeFlags("Whole ancestor/instance guards + side declarations",0)) {
              unsigned char guards[GD_RESOURCE_GRAPH_NODES]={0};
              for (i=0;i<=w->count;++i) if (graph->nodes[w->nodes[i]].kind==GD_RESOURCE_CASE) {
                  int current=w->nodes[i],depth=0;
                  do { guards[current]=1; if (!graph->nodes[current].instance) break; current=graph->nodes[current].parent; } while (++depth<GD_MAX_RULE_EXPANSION_DEPTH);
              }
              for (i=0;i<graph->node_count;++i) if (guards[i]) {
                const GD_ResourceNode *node=&graph->nodes[i]; int j;
                { const GD_RuleCase *rule=&graph->rules->cases[node->case_index];
                  igTextWrapped("Case %s | context %s | unknown conditions %s | unknown effects %s",rule->name,rule->context ? rule->context:"unspecified",rule->unknown_conditions ? rule->unknown_conditions:"none declared",rule->unknown_effects ? rule->unknown_effects:"none declared");
                  igTextWrapped("Primary target %s | case-level becomes %s | preserves %s | calls %s",rule->primary_target ? rule->primary_target:"unspecified",rule->default_effects[0] ? rule->default_effects[0]:"unspecified",rule->default_effects[1] ? rule->default_effects[1]:"unspecified",rule->default_effects[2] ? rule->default_effects[2]:"none");
                  for (j=rule->first_participant;j<rule->first_participant+rule->participant_count;++j) {
                      const GD_RuleParticipant *actor=&graph->rules->participants[j];
                      igTextWrapped("%s requires %s | excludes %s | preserves %s | calls %s",actor->name,actor->requires ? actor->requires:"unspecified",actor->skip_if ? actor->skip_if:"unspecified",actor->preserve ? actor->preserve:"unspecified",actor->causes ? actor->causes:"none");
                  }
                }
                if (node->instance) igTextWrapped("Concrete placement in %s | template %s",node_label(graph,node->parent),graph->rules->cases[node->case_index].name);
                for (j=query->case_head[i];j>=0;j=query->case_next[j]) {
                    gd_explorer_resource_source(context,j);
                    if (graph->relations[j].kind==GD_RESOURCE_QUANTITY_WRITE) igTextWrapped("%s",graph->rules->assignments[graph->relations[j].declaration].expression);
                }
              }
            }
        }
    }
    igSeparator();
}
void gd_explorer_resource_details(GD_ExplorerContext *context) {
    const GD_ResourceGraph *graph=context->project->resources;
    int selected=context->pane->selection.node,record=gd_explorer_resource_record(graph,selected),i;
    const GD_ResourceNode *node; char identity[GD_MAX_PATH];
    if (context->pane->selection.linked>=0) {
        const GD_Record *source=&graph->model->records[context->pane->selection.linked];
        igTextWrapped("Loaded source inspection (not a projected node): %s",source->reference);
        igTextWrapped("Status: %s | %s",source->status ? source->status:"unspecified",source->source_path);
        if (igButton("Copy source path",(ImVec2_c){0,0})) SetClipboardText(source->source_path);
        igPushTextWrapPos(0); igTextUnformatted(source->body,NULL); igPopTextWrapPos(); return;
    }
    if (selected<0 || selected>=graph->node_count) { igTextWrapped("Select a case, facet or resource role to inspect source."); return; }
    node=&graph->nodes[selected];
    if (gd_resource_node_id(graph,selected,-1,identity,sizeof(identity))) {
        igTextWrapped("Node ID: %s",identity);
        if (igButton("Copy node ID",(ImVec2_c){0,0})) SetClipboardText(identity);
    }
    igTextWrapped("#%d %s | %s",selected,node_label(graph,selected),node->kind==GD_RESOURCE_CASE ? "Rule case":node->kind==GD_RESOURCE_ROLE ? "Resource role":"Actor facet");
    if (node->instance) {
        const GD_RuleCase *item=&graph->rules->cases[node->case_index];
        igTextWrapped("Concrete placement in %s | template origin retained | call line %d",node_label(graph,node->parent),graph->calls->calls[node->call].line);
        for (i=item->first_participant;i<item->first_participant+item->participant_count;++i) {
            int actor=gd_resource_instance_actor(graph,selected,i);
            igTextWrapped("Template %s = concrete %s",graph->rules->participants[i].name,actor>=0 ? graph->rules->participants[actor].name:"unresolved");
        }
        if (node->binding_conflict) igTextWrapped("Conflicting explicit mapped pool bindings: activation unavailable; no source wins.");
        if (node->frontier) igTextWrapped("Expansion incomplete: inspect unresolved/cycle/depth/work frontier.");
    }
    if (node->pool_reference) igTextWrapped("Pool: %s | shared identity, not a unique case/participant",node->pool_reference);
    if (node->case_index>=0 && !node->pool_reference) {
        const GD_RuleCase *item=&graph->rules->cases[node->case_index];
        igTextWrapped("Case line %d | context: %s",item->line,item->context ? item->context:"unspecified");
    }
    if (node->participant>=0 && !node->pool_reference) {
        const GD_RuleParticipant *actor=&graph->rules->participants[node->participant];
        igTextWrapped("Participant: %s | noun: %s",actor->name,gd_rule_noun_label(graph->rules,actor));
    }
    if (node->axis>=0) igTextWrapped("Axis: %s",graph->model->records[node->axis].reference);
    if (node->kind==GD_RESOURCE_ROLE) igTextWrapped("Resolution: %s",gd_rule_resolution_name(node->resolution));
    if (record>=0) {
        const GD_Record *source=&graph->model->records[record];
        igTextWrapped("Source: %s | status: %s",source->reference,source->status ? source->status:"unspecified");
        igTextWrapped("%s",source->source_path);
        if (igButton("Copy source path",(ImVec2_c){0,0})) SetClipboardText(source->source_path);
    }
    igSeparator(); igTextUnformatted("Direct/instantiated declarations (not final outcomes):",NULL);
    for (i=0;i<graph->relation_count;++i) {
        const GD_ResourceRelation *edge=&graph->relations[i];
        if (edge->facet!=selected && edge->activation!=selected) continue;
        igPushID_Int(i); gd_explorer_resource_source(context,i);
        if (edge->mapped_participant>=0 && edge->mapped_participant<graph->rules->participant_count)
            igTextWrapped("Concrete participant: %s%s",graph->rules->participants[edge->mapped_participant].name,edge->contextual_input ? " | ancestor input in this placement":"");
        if (edge->kind==GD_RESOURCE_QUANTITY_WRITE) {
            igTextWrapped("%s := %s",graph->rules->assignments[edge->declaration].axis,graph->rules->assignments[edge->declaration].expression);
            if (edge->delta_recognized && edge->delta.known) igText("Signed delta: %g",edge->delta.number);
        } else if (edge->kind==GD_RESOURCE_STATE_WRITE) {
            const GD_RuleStateEffect *effect=&graph->rules->state_effects[edge->declaration];
            igTextWrapped("%s := %s",effect->target.axis_record>=0 ? graph->model->records[effect->target.axis_record].reference:"unresolved axis",effect->target.state ? effect->target.state:"unresolved state");
        } else if (edge->kind==GD_RESOURCE_REQUIREMENT || edge->kind==GD_RESOURCE_EXCLUSION) {
            const GD_RuleParticipant *actor=&graph->rules->participants[edge->participant];
            const char *condition=edge->kind==GD_RESOURCE_REQUIREMENT ? actor->requires:actor->skip_if;
            igTextWrapped("%s",condition ? condition:"unspecified condition");
        }
        igPopID();
    }
    if (record>=0) {
        igSeparator(); igPushTextWrapPos(0); igTextUnformatted(graph->model->records[record].body,NULL); igPopTextWrapPos();
    }
}
#endif
