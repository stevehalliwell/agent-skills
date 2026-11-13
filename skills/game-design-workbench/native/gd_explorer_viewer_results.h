/* Native retained serializers through bounded, disk-free byte pages. */
#ifndef GD_EXPLORER_VIEWER_RESULTS_H
#define GD_EXPLORER_VIEWER_RESULTS_H
#include "gd_viewer_capture.h"
static void result_ints(FILE *out,const int *values,int count) {int i;fputc('[',out);for(i=0;i<count;++i){if(i)fputc(',',out);fprintf(out,"%d",values[i]);}fputc(']',out);}
static void result_bytes(FILE *out,const unsigned char *values,int count) {int i;fputc('[',out);for(i=0;i<count;++i){if(i)fputc(',',out);fprintf(out,"%u",(unsigned)values[i]);}fputc(']',out);}
static void result_closure(FILE *out,const GD_GraphClosure *closure,int nodes,int edges) {
    int i,j;if(!closure->valid){fputs("null",out);return;}fprintf(out,"{\"valid\":%s,\"complete\":%s,\"work\":%d,\"limit\":%d,\"relation\":%d,\"reverse\":%d,\"undirected\":%d,\"rows\":[",closure->valid ? "true":"false",closure->complete ? "true":"false",closure->work,closure->limit,closure->relation,closure->reverse,closure->undirected);
    for(i=0;i<nodes && closure->valid && !ferror(out);++i){if(i)fputc(',',out);fprintf(out,"{\"node\":%d,\"finished\":%s,\"eligible\":%s,\"boundary\":%s,\"unknown\":%s,\"cycle\":%s,\"direct\":%d,\"targets\":%d,\"users\":%d,\"target_bits\":[",i,closure->finished[i] ? "true":"false",closure->eligible[i] ? "true":"false",closure->boundary[i] ? "true":"false",closure->unknown[i] ? "true":"false",closure->cycle[i] ? "true":"false",closure->direct[i],closure->count[i],closure->users[i]);for(j=0;j<(nodes+63)/64 && !ferror(out);++j){if(j)fputc(',',out);fprintf(out,"\"%016llx\"",(unsigned long long)closure->rows[i][j]);}fputs("]}",out);}
    fputs("],\"comparison\":{\"valid\":",out);fputs(closure->comparison.valid ? "true":"false",out);fprintf(out,",\"first\":%d,\"second\":%d,\"shared\":%d,\"first_only\":%d,\"second_only\":%d,\"nodes\":",closure->comparison.a,closure->comparison.b,closure->comparison.shared,closure->comparison.a_only,closure->comparison.b_only);result_bytes(out,closure->comparison.nodes,nodes);fputs(",\"links\":",out);result_bytes(out,closure->comparison.edges,edges);fputs(",\"first_parent_arcs\":",out);result_ints(out,closure->comparison.first_a,nodes);fputs(",\"second_parent_arcs\":",out);result_ints(out,closure->comparison.first_b,nodes);fputs("}}",out);
}
static int viewer_write_layers(FILE *out,GD_ExplorerContext *c,const char *fields) {
    const GD_GraphAnalysis *a=c->analysis; const GD_ExplorerPane *p=c->pane; int i;
    if(overview_only(p) && !fields)fields="nodes,links,overview";
    fprintf(out,"{\"ok\":true,\"scope\":\"last explicitly prepared shown-canvas layer cache, not canonical question scope\",\"complete\":%s,\"work\":%d,\"limit\":%d,\"shown_nodes\":%d,\"shown_links\":%d,\"bitset_encoding\":\"hex uint64 words; bit (node %% 64) of word (node / 64); snapshot-local node slots\"",a->complete ? "true":"false",a->work,a->work_limit,a->shown_nodes,a->shown_edges);
    if(gd_viewer_field(fields,"nodes")){fputs(",\"nodes\":[",out);for(i=0;i<a->node_count && !ferror(out);++i){char id[GD_MAX_PATH];if(i)fputc(',',out);fprintf(out,"{\"slot\":%d,\"id\":",i);gd_json_string(out,presentation_node_id(c,i,id,sizeof(id)) ? id:NULL);fprintf(out,",\"shown\":%s",a->shown[i] ? "true":"false");if(!overview_only(p))fprintf(out,",\"neighbors\":%d,\"incident\":%d,\"shared_targets\":%d",a->neighbors[i],a->incident[i],a->matched[i]);fputc('}',out);}fputc(']',out);}
    if(gd_viewer_field(fields,"links")){fputs(",\"links\":[",out);for(i=0;i<a->edge_count && !ferror(out);++i){const GD_LayoutEdge *e=&a->edges[i];char key[64];if(i)fputc(',',out);viewer_row_key(c,"link",i,key,sizeof(key));fprintf(out,"{\"slot\":%d,\"key\":",i);gd_json_string(out,key);fprintf(out,",\"from\":%d,\"to\":%d,\"source_key\":%d,\"relation\":%d,\"active\":%s}",e->a,e->b,e->key,e->relation,e->active ? "true":"false");}fputc(']',out);}
    if(gd_viewer_field(fields,"shared")){fprintf(out,",\"shared\":{\"lens\":%d,\"pairs\":[",a->lens);for(i=0;i<a->pair_count && !ferror(out);++i){const GD_GraphMatch *m=&a->pairs[i];if(i)fputc(',',out);fprintf(out,"{\"first\":%d,\"second\":%d,\"shared\":%d,\"first_feature\":%d,\"second_feature\":%d}",m->a,m->b,m->shared,m->first_a,m->first_b);}fputs("],\"features\":[",out);for(i=0;i<a->feature_count && !ferror(out);++i){const GD_GraphFeature *f=&a->features[i];if(i)fputc(',',out);fprintf(out,"{\"owner\":%d,\"neighbor\":%d,\"link\":%d,\"endpoint\":%d,\"relation\":%d,\"owner_end\":%d}",f->owner,f->neighbor,f->edge,f->endpoint,f->relation,f->owner_end);}fprintf(out,"],\"comparison\":{\"valid\":%s,\"first\":%d,\"second\":%d,\"shared\":%d,\"first_only\":%d,\"second_only\":%d,\"other_role\":%d,\"nodes\":",a->comparison.valid ? "true":"false",a->comparison.a,a->comparison.b,a->comparison.shared,a->comparison.a_only,a->comparison.b_only,a->comparison.other_role);result_bytes(out,a->comparison.nodes,a->node_count);fputs(",\"links\":",out);result_bytes(out,a->comparison.edges,a->edge_count);fputs("}}",out);}
    if(gd_viewer_field(fields,"paths")){const GD_GraphPaths *v=&a->paths;if(!v->valid)fputs(",\"paths\":null",out);else {const GD_PathSelection *s=&v->selection;const GD_PathBypass *b=&v->bypass;fprintf(out,",\"paths\":{\"enabled\":%s,\"valid\":%s,\"complete\":%s,\"work\":%d,\"limit\":%d,\"root_count\":%d,\"roots\":",p->paths.enabled ? "true":"false",v->valid ? "true":"false",v->complete ? "true":"false",v->work,v->limit,v->root_count);result_bytes(out,v->roots,a->node_count);fputs(",\"reached\":",out);result_bytes(out,v->reached,a->node_count);fputs(",\"dominated\":",out);result_ints(out,v->dominated,a->node_count);fprintf(out,",\"selected\":%d,\"support_length\":%d,\"support_suffix\":",s->node,s->length);result_ints(out,s->arcs,s->retained);fputs(",\"ancestors\":",out);result_bytes(out,s->ancestors,a->node_count);fputs(",\"support_links\":",out);result_bytes(out,s->edges,a->edge_count);fputs(",\"bypass_candidates\":",out);result_ints(out,s->candidates,s->candidate_count);fprintf(out,",\"bypass\":{\"valid\":%s,\"complete\":%s,\"found\":%s,\"candidate\":%d,\"fork\":%d,\"rejoin\":%d,\"length\":%d,\"work\":%d,\"limit\":%d,\"suffix\":",b->valid ? "true":"false",b->complete ? "true":"false",b->found ? "true":"false",b->candidate,b->fork,b->rejoin,b->length,b->work,b->limit);result_ints(out,b->arcs,b->retained);fputs(",\"links\":",out);result_bytes(out,b->edges,a->edge_count);fputs("}}",out);}}
    if(gd_viewer_field(fields,"foundations")){fputs(",\"foundations\":",out);result_closure(out,&a->foundations,a->node_count,a->edge_count);}
    if(gd_viewer_field(fields,"reachability")){fputs(",\"reachability\":",out);result_closure(out,&a->reachability,a->node_count,a->edge_count);}
    if(gd_viewer_field(fields,"shapes")){const GD_GraphShapes *s=&a->shapes;if(!s->valid)fputs(",\"shapes\":null",out);else {fprintf(out,",\"shapes\":{\"valid\":%s,\"complete\":%s,\"work\":%d,\"limit\":%d,\"family_count\":%d,\"colors\":",s->valid ? "true":"false",s->complete ? "true":"false",s->work,s->limit,s->family_count);result_ints(out,s->colors,a->node_count);fputs(",\"roles\":",out);result_ints(out,s->roles,a->node_count);fputs(",\"previous_colors\":",out);result_ints(out,s->previous,a->node_count);fputs(",\"family_sizes\":",out);result_ints(out,s->size,s->family_count);fputs(",\"representatives\":",out);result_ints(out,s->representative,s->family_count);fputs(",\"feature_begin\":",out);result_ints(out,s->begin,a->node_count+1);fputs(",\"signature_features\":[",out);for(i=0;s->valid && i<s->begin[a->node_count] && !ferror(out);++i){if(i)fputc(',',out);fprintf(out,"{\"owner\":%d,\"relation\":%d,\"endpoint\":%d,\"color\":%d}",s->features[i].owner,s->features[i].relation,s->features[i].endpoint,s->features[i].color);}fputs("]}",out);}}
    if(gd_viewer_field(fields,"overview")){const GD_GraphOverview *v=&a->overview;if(!v->valid)fputs(",\"overview\":null",out);else {fputs(",\"overview\":{\"metric\":",out);gd_json_string(out,overview_metric(p));fprintf(out,",\"link_count\":%d,\"selected_count\":%d",v->link_count,v->selected_count);fprintf(out,",\"valid\":%s,\"bins\":%d,\"undirected\":%s,\"selected_row\":%d,\"selected_column\":%d,\"counts\":",v->valid ? "true":"false",v->bins,v->undirected ? "true":"false",v->selected_row,v->selected_column);result_ints(out,v->counts,v->bins*v->bins);fputs(",\"node_bins\":",out);result_ints(out,v->node_bin,a->node_count);fputs(",\"selected_nodes\":",out);result_bytes(out,v->selected_nodes,a->node_count);fputs(",\"selected_links\":",out);result_bytes(out,v->selected_edges,a->edge_count);fputc('}',out);}}
    fputs(",\"arc_encoding\":\"link=arc/2; reverse=arc%2; support suffixes retain native order and omission limits\",\"claim\":\"structural evidence, not AND acquisition, causal flow, learning or final outcomes\"}",out);return !ferror(out);
}
static int viewer_write_comparison(FILE *out,GD_ExplorerContext *c) {
    GD_ExplorerQueryState *q=c->pane->query_state; GD_ExplorerStructureState *s=c->pane->structure_state;int i;
    if(gd_explorer_similarity_active(c) || (s && s->result && s->result->similarity && c->pane->view!=GD_EXPLORER_ACTORS)) {
        if(!s || !s->comparison_available || s->comparison_first!=s->similarity_row || s->comparison_second!=s->similarity_peer) return 0;
        fprintf(out,"{\"ok\":true,\"family\":\"similarity\",\"first\":%d,\"second\":%d,\"work\":%d,\"distance\":%.17g,\"components\":[",s->comparison_first,s->comparison_second,s->comparison_work,s->comparison_distance);
        for(i=0;i<s->result->similarity->feature_count;++i){if(i)fputc(',',out);fputs("{\"feature\":",out);gd_json_string(out,s->result->similarity->features[i].name);fprintf(out,",\"weight\":%.17g,\"unweighted\":%.17g}",s->result->similarity->features[i].weight,s->comparison_components[i]);}fputs("]}",out);return !ferror(out);
    }
    if(!q || !q->comparison_ready || q->comparison_first!=q->applied.primary || q->comparison_second!=q->applied.compare) return 0;
    {const GD_ActorComparison *v=&q->comparison;fprintf(out,"{\"ok\":true,\"family\":\"actor\",\"first\":%d,\"second\":%d,\"complete\":%s,\"cases\":{\"shared\":%d,\"first_only\":%d,\"second_only\":%d},\"partners\":{\"shared\":%d,\"first_only\":%d,\"second_only\":%d},\"effects\":{\"shared\":%d,\"first_only\":%d,\"second_only\":%d}}",q->comparison_first,q->comparison_second,v->complete ? "true":"false",v->cases.shared,v->cases.first_only,v->cases.second_only,v->partners.shared,v->partners.first_only,v->partners.second_only,v->effects.shared,v->effects.first_only,v->effects.second_only);}return !ferror(out);
}
typedef struct { GD_ExplorerContext *context; const GD_ViewerRequest *request; } ViewerResult;
static int viewer_result_emit(FILE *out,void *data) {
    ViewerResult *result=data;GD_ExplorerContext *c=result->context;const GD_ViewerRequest *r=result->request;GD_ExplorerPane *p=c->pane;const char *fields=*r->fields ? r->fields:NULL;GD_Model *model=gd_explorer_model(c->project,p->view);
    if(!strcmp(r->form,"noun-grid")) return !fields && p->view==GD_EXPLORER_NOUNS && gd_noun_grid_write(out,c->project->noun_grid);
    if(!strcmp(r->form,"actor-overview")) return !fields && p->query_state && gd_query_scene_overview_write(out,p->query_state->scene);
    if(!strcmp(r->form,"query")) return p->query_state && p->query_state->query && (!fields || gd_query_fields(p->query_state->query->request.kind,fields)) && gd_write_query_selection(out,model,p->query_state->query,p->query_state->selected,1,fields);
    if(!strcmp(r->form,"structure")) return p->structure_state && p->structure_state->result && gd_write_structure_retained(out,p->structure_state->result,1,fields);
    if(!strcmp(r->form,"rules")) return p->rule_state && p->rule_state->result && (!fields || gd_named_fields(p->rule_state->result->request.kind,fields)) && gd_write_named_result(out,p->rule_state->result,1,fields);
    if(!strcmp(r->form,"bindings")) return p->rule_state && p->rule_state->bindings && (!fields || gd_query_fields(GD_QUERY_BINDINGS,fields)) && gd_write_query_retained(out,p->rule_state->bindings->model,p->rule_state->bindings,1,fields);
    if(!strcmp(r->form,"resource")) return p->view==GD_EXPLORER_RESOURCES && p->resource_routes && gd_write_resource_query(out,model,c->project->resources,p->resource_routes,1,fields);
    if(!strcmp(r->form,"mda")) return p->view==GD_EXPLORER_MDA && gd_write_mda_result(out,model,c->project->mda,1,fields);
    if(!strcmp(r->form,"challenges")) return p->view==GD_EXPLORER_CHALLENGES && gd_write_routes_result(out,model,c->project->routes,1,fields);
    if(!strcmp(r->form,"skills")) return p->view==GD_EXPLORER_SKILLS && c->query && gd_write_skill_result(out,model,c->query,1,fields);
    if(!strcmp(r->form,"layers")) return viewer_write_layers(out,c,fields);
    if(!strcmp(r->form,"comparison")) return viewer_write_comparison(out,c);
    return 0;
}
static void viewer_base64(GD_ViewerWriter *w,const unsigned char *bytes,size_t size) {
    const char *alphabet="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";size_t i;gd_viewer_write(w,"\"");
    for(i=0;i<size;i+=3){unsigned value=(unsigned)bytes[i]<<16;size_t remaining=size-i;if(remaining>1)value|=(unsigned)bytes[i+1]<<8;if(remaining>2)value|=bytes[i+2];gd_viewer_write(w,"%c%c%c%c",alphabet[(value>>18)&63],alphabet[(value>>12)&63],remaining>1 ? alphabet[(value>>6)&63]:'=',remaining>2 ? alphabet[value&63]:'=');}gd_viewer_write(w,"\"");
}
static int viewer_result_field_names(const char *fields,const char *names) {
    const char *p=fields;char key[64];if(!*p)return 1;
    for(;;){const char *end=strchr(p,',');size_t length=end ? (size_t)(end-p):strlen(p);if(!length || length>=sizeof(key))return 0;memcpy(key,p,length);key[length]=0;if(!gd_viewer_field(names,key))return 0;if(!end)return 1;p=end+1;}
}
static int viewer_result_fields(GD_ExplorerContext *c,const GD_ViewerRequest *r) {
    GD_ExplorerPane *p=c->pane;if(!*r->fields)return 1;
    if(!strcmp(r->form,"noun-grid"))return 0;
    if(!strcmp(r->form,"actor-overview"))return 0; /* Bounded byte pages, whole retained overview only. */
    if(!strcmp(r->form,"query"))return p->query_state && p->query_state->query && gd_query_fields(p->query_state->query->request.kind,r->fields);
    if(!strcmp(r->form,"bindings"))return gd_query_fields(GD_QUERY_BINDINGS,r->fields);
    if(!strcmp(r->form,"structure"))return p->structure_state && p->structure_state->result && gd_structure_question_fields(p->structure_state->result->options.question,r->fields);
    if(!strcmp(r->form,"rules"))return p->rule_state && p->rule_state->result && gd_named_fields(p->rule_state->result->request.kind,r->fields);
    if(!strcmp(r->form,"layers"))return viewer_result_field_names(r->fields,overview_only(p) ? "nodes,links,overview":"nodes,links,shared,paths,foundations,reachability,shapes,overview");
    if(!strcmp(r->form,"skills"))return viewer_result_field_names(r->fields,"skills,links,components,findings,exploration");
    if(!strcmp(r->form,"challenges"))return viewer_result_field_names(r->fields,"challenges,skills,checks,witnesses,findings");
    if(!strcmp(r->form,"mda"))return viewer_result_field_names(r->fields,"nodes,links,paths,gaps,findings");
    if(!strcmp(r->form,"resource"))return viewer_result_field_names(r->fields,"nodes,routes,relations,findings");
    return 0;
}
static void viewer_result(ExplorerViewer *viewer,const GD_ViewerRequest *request,GD_ExplorerContext *c,GD_ViewerWriter *w) {
    char token[64],message[256],bytes[GD_VIEWER_CAPTURE_PAGE_LIMIT];GD_ViewerCaptureResult page;ViewerResult input={c,request};size_t capacity=(size_t)request->limit*1024;
    if(!viewer_result_fields(c,request)){gd_viewer_error(w,"viewer.result_fields","Unsupported field for retained result family; inspect native query catalogue");return;}
    viewer_control_token(viewer,c,GD_FORM_PRESENTATION,token,sizeof(token));
    { uint64_t hash=viewer_control_hash(UINT64_C(14695981039346656037),token,strlen(token)); hash=viewer_control_hash(hash,request->form,strlen(request->form)+1);hash=viewer_control_hash(hash,request->fields,strlen(request->fields)+1);
      if(*request->expect) {char expected[64];snprintf(expected,sizeof(expected),"%016llx",(unsigned long long)hash);if(strcmp(expected,request->expect)){gd_viewer_error(w,"viewer.stale_result","Result/input/field scope changed; restart byte pagination");return;}}
    }
    if((!strcmp(request->form,"layers") && !viewer_layers_current(viewer,c)) || (!strcmp(request->form,"skills") && (!*viewer->knowledge_token || strcmp(viewer->knowledge_token,token)))){gd_viewer_error(w,"viewer.stale_result","No current shown-layer/knowledge cache; explicitly run view.analyze on target projection");return;}
    if(!strcmp(request->form,"query") && *request->fields && gd_viewer_field(request->fields,"comparisons")){gd_viewer_error(w,"viewer.result","All-pair evaluation is not a read; use view.compare then result --form comparison");return;}
    if(capacity>GD_VIEWER_CAPTURE_TOTAL_LIMIT-request->offset)capacity=GD_VIEWER_CAPTURE_TOTAL_LIMIT-request->offset;
    if(!gd_viewer_capture(viewer_result_emit,&input,request->offset,bytes,capacity,&page,message,sizeof(message))){gd_viewer_error(w,"viewer.result",message);return;}
    {uint64_t hash=viewer_control_hash(UINT64_C(14695981039346656037),token,strlen(token));hash=viewer_control_hash(hash,request->form,strlen(request->form)+1);hash=viewer_control_hash(hash,request->fields,strlen(request->fields)+1);snprintf(token,sizeof(token),"%016llx",(unsigned long long)hash);}
    gd_viewer_write(w,"{\"ok\":true,");viewer_member(w,"projection",viewer_projections[c->pane->view]);gd_viewer_write(w,",");viewer_member(w,"form",request->form);gd_viewer_write(w,",");viewer_member(w,"result_token",token);
    gd_viewer_write(w,",\"encoding\":\"base64\",\"media_type\":\"application/json; charset=utf-8\",\"offset\":%u,\"bytes\":%llu,\"total_bytes\":%llu,\"data\":",request->offset,(unsigned long long)page.captured,(unsigned long long)page.total);viewer_base64(w,(const unsigned char *)bytes,page.captured);
    gd_viewer_write(w,",\"next_offset\":");if((size_t)request->offset+page.captured<page.total)gd_viewer_write(w,"%llu",(unsigned long long)((size_t)request->offset+page.captured));else gd_viewer_write(w,"null");
    gd_viewer_write(w,",\"scope\":\"retained native result; byte pages require result_token; no source reload, query Run or hidden comparison evaluation\"}");
}
#endif
