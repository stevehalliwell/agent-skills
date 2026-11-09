/* Included by gd_query_scene.c. Bounded source projection, never an evaluator.
   One scratch allocation (<1 MiB); scene owns counts until destruction. */
#ifndef GD_QUERY_SCENE_OVERVIEW_H
#define GD_QUERY_SCENE_OVERVIEW_H
typedef struct {
    int slots[GD_MAX_PAIRS*2],last_record[GD_MAX_PAIRS];
    int node[GD_MAX_RECORDS],seen[GD_MAX_RECORDS],members[GD_MAX_RECORDS];
    unsigned char cases[GD_MAX_RULE_CASES];
} OverviewScratch;
typedef char OverviewScratchBound[sizeof(OverviewScratch)<=1048576 ? 1:-1];
static int overview_actor(const GD_Record *record) { return record->role==GD_NOUN && record->is_actor_line && record->is_actor; }
static int overview_node(GD_QueryScene *scene,OverviewScratch *scratch,int record) {
    const GD_Record *item=&scene->query->model->records[record]; int node=scratch->node[record];
    if(node>=0) return node;
    node=add_node(scene,item->role==GD_AXIS ? GD_QUERY_NODE_AXIS:GD_QUERY_NODE_OBJECT,record,-1,0,GD_RULE_TRUTH_UNKNOWN,"%s\n%s",item->display_name,item->role==GD_AXIS ? "axis":overview_actor(item) ? "actor":item->is_actor_line ? "non-actor noun":"unclassified noun");
    scratch->node[record]=node; return node;
}
static void overview_pair(GD_QueryScene *scene,OverviewScratch *scratch,int a,int b,GD_QueryEdgeKind kind,int source,int record) {
    unsigned slot; int edge; uint64_t key;
    if(a<0 || b<0 || a==b || scene->limited) return;
    key=((uint64_t)(unsigned)a<<32)|(unsigned)b;slot=query_free_slot(key);
    for(;;) {
        if(!charge(scene,1)) return;
        edge=scratch->slots[slot]-1;
        if(edge<0 || (scene->layout.edges[edge].a==a && scene->layout.edges[edge].b==b)) break;
        slot=(slot+1)&(GD_MAX_PAIRS*2-1);
    }
    if(edge<0) {
        edge=scene->layout.edge_count;add_edge(scene,a,b,kind,source,-1);
        if(scene->layout.edge_count==edge) return;
        scratch->slots[slot]=edge+1;scratch->last_record[edge]=-1;
        ++scene->layout.nodes[a].degree;++scene->layout.nodes[b].degree;
    }
    /* Caller walks each source's contiguous cases together. A pair contributes
       once per source record regardless of cases or repeated local bindings. */
    if(scratch->last_record[edge]!=record) { ++scene->overview->counts[edge];scratch->last_record[edge]=record; }
}
static void overview_cloud_pose(GD_QueryScene *scene) {
    int i,actors=0,concepts=0,a=0,c=0,columns;float radius,width,height,cell_width=450,cell_height=200;
    for(i=0;i<scene->layout.node_count;++i) { if(overview_actor(&scene->query->model->records[scene->nodes[i].source])) ++actors;else ++concepts; }
    columns=concepts ? (int)ceilf(sqrtf((float)concepts)):1;
    width=columns*cell_width;height=((concepts+columns-1)/columns)*cell_height;
    radius=fmaxf(400,hypotf(width,height)/2+230);radius=fmaxf(radius,actors*300.0f/6.28318530718f);
    for(i=0;i<scene->layout.node_count;++i) {
        GD_LayoutNode *node=&scene->layout.nodes[i];
        if(overview_actor(&scene->query->model->records[scene->nodes[i].source])) {float angle=6.28318530718f*a++/(actors ? actors:1)-1.57079632679f;node->x=radius*cosf(angle);node->y=radius*sinf(angle);}
        else {node->x=((c%columns)-(columns-1)/2.0f)*cell_width;node->y=((c/columns)-(((concepts+columns-1)/columns)-1)/2.0f)*cell_height;++c;}
        node->vx=node->vy=0;
    }
    scene->layout.width=scene->layout.height=radius*2+450;
    scene->layout.groups[0]=(GD_LayoutGroup){-radius-225,-radius-225,scene->layout.width,scene->layout.height,0,scene->layout.node_count};
}
GD_QueryScene *gd_query_scene_overview(const GD_Query *query,GD_ActorOverviewMode mode,int work_limit) {
    GD_QueryScene *scene;OverviewScratch *scratch;int i,s,c,p,j;
    if(!query || query->request.kind!=GD_QUERY_BINDINGS || mode<GD_ACTOR_INTERACTIONS || mode>GD_ACTOR_USAGE || work_limit<0 || work_limit>GD_EFFECT_WORK) return NULL;
    scene=query->allocator.allocate(query->allocator.context,sizeof(*scene));if(!scene) return NULL;
    memset(scene,0,sizeof(*scene));scene->allocator=query->allocator;scene->query=query;scene->primary=-1;scene->work_limit=work_limit;
    scene->overview=query->allocator.allocate(query->allocator.context,sizeof(*scene->overview));
    scratch=query->allocator.allocate(query->allocator.context,sizeof(*scratch));
    if(!scene->overview || !scratch) {if(scratch) query->allocator.release(query->allocator.context,scratch);gd_query_scene_destroy(scene);return NULL;}
    memset(scene->overview,0,sizeof(*scene->overview));scene->overview->mode=mode;
    memset(scratch,0,sizeof(*scratch));for(i=0;i<GD_MAX_RECORDS;++i) scratch->node[i]=scratch->seen[i]=-1;
    scene->overview->incomplete=query->limited || query->model->error_count || query->model->errors_omitted || query->rules->findings_omitted || !query->rules->bindings_complete;
    for(i=0;i<query->model->record_count && !scene->limited;++i) {
        const GD_Record *record=&query->model->records[i];if(!charge(scene,1)) break;
        if(record->role==GD_NOUN && !record->is_actor_line) ++scene->overview->unclassified;
        if(overview_actor(record)) overview_node(scene,scratch,i);
    }
    for(i=0;i<query->binding_count && !scene->limited;++i) {if(!charge(scene,1)) break;scratch->cases[query->bindings[i].case_index]=1;}
    for(s=0;s<query->rules->source_count && !scene->limited;++s) {
        const GD_RuleSource *source=&query->rules->sources[s];
        if(!charge(scene,1)) break;
        /* Rule snapshots normalize record roles; original bound collection keeps source scope. */
        if(query->model->collections[query->model->records[source->record].collection].role!=GD_CASE) continue;
        if(source->state!=GD_RULE_PARSED) scene->overview->incomplete=1;
        for(c=source->first_case;c<source->first_case+source->case_count && !scene->limited;++c) {
            const GD_RuleCase *item=&query->rules->cases[c];int count=0;
            if(!charge(scene,1)) break;if(!scratch->cases[c]) continue;
            for(p=item->first_participant;p<item->first_participant+item->participant_count && !scene->limited;++p) {
                const GD_RuleParticipant *participant=&query->rules->participants[p];int record=participant->noun_record;
                if(!charge(scene,1)) break;
                if(participant->noun_resolution!=GD_RULE_RESOLVED) {scene->overview->incomplete=1;continue;}
                if(scratch->seen[record]==c) continue;scratch->seen[record]=c;scratch->members[count++]=record;
            }
            for(i=0;i<count && !scene->limited;++i) if(overview_actor(&query->model->records[scratch->members[i]])) {
                int a=overview_node(scene,scratch,scratch->members[i]);
                for(j=0;j<count && !scene->limited;++j) {
                    int other=scratch->members[j],actor=overview_actor(&query->model->records[other]);
                    if(!charge(scene,1)) break;
                    if(mode==GD_ACTOR_INTERACTIONS ? (!actor || scratch->members[i]>=other):actor) continue;
                    overview_pair(scene,scratch,a,overview_node(scene,scratch,other),mode==GD_ACTOR_INTERACTIONS ? GD_QUERY_EDGE_INTERACTION:GD_QUERY_EDGE_USAGE,c,source->record);
                }
            }
        }
    }
    if(mode==GD_ACTOR_USAGE) for(i=0;i<query->model->edge_count && !scene->limited;++i) {
        const GD_Edge *edge=&query->model->edges[i];if(!charge(scene,1)) break;
        if(edge->field!=GD_AXES || !overview_actor(&query->model->records[edge->source])) continue;
        if(query->rules->axis_resolution[i]!=GD_RULE_RESOLVED) {scene->overview->incomplete=1;continue;}
        overview_pair(scene,scratch,scratch->node[edge->source],overview_node(scene,scratch,edge->target),GD_QUERY_EDGE_AXIS,i,edge->source);
    }
    if(mode==GD_ACTOR_USAGE) for(i=0;i<scene->layout.edge_count;++i) ++scene->nodes[scene->layout.edges[i].b].incoming;
    if(mode==GD_ACTOR_USAGE) for(i=0;i<scene->layout.node_count;++i) {
        GD_LayoutNode *node=&scene->layout.nodes[i];int incoming=scene->nodes[i].incoming;
        if(incoming) {float scale=log2f((float)incoming+1);
            node->width=250+fminf(150,35*scale);node->height=90+fminf(70,15*scale);
        }
    }
    query->allocator.release(query->allocator.context,scratch);
    scene->layout.group_count=scene->layout.node_count ? 1:0;scene->layout.direct_routes=1;
    if(!gd_layout_arrange(&scene->layout,mode==GD_ACTOR_INTERACTIONS ? GD_LAYOUT_FORCE:GD_LAYOUT_DIRECTED,GD_SORT_NAME)) {gd_query_scene_destroy(scene);return NULL;}
    if(mode==GD_ACTOR_USAGE) overview_cloud_pose(scene);
    for(i=0;i<scene->layout.node_count;++i) scene->native_points[i]=(GD_LayoutPoint){scene->layout.nodes[i].x,scene->layout.nodes[i].y};
    scene->native_group=scene->layout.groups[0];scene->native_width=scene->layout.width;scene->native_height=scene->layout.height;
    if(scene->limited) scene->unexamined_items_upper_bound=query->rules->participant_count+query->model->edge_count;
    return scene;
}
int gd_query_scene_overview_mode(const GD_QueryScene *scene) {return scene && scene->overview ? (int)scene->overview->mode:-1;}
int gd_query_scene_overview_count(const GD_QueryScene *scene,int edge) {return scene && scene->overview && edge>=0 && edge<scene->layout.edge_count ? scene->overview->counts[edge]:0;}
int gd_query_scene_overview_unclassified(const GD_QueryScene *scene) {return scene && scene->overview ? scene->overview->unclassified:0;}
int gd_query_scene_overview_incomplete(const GD_QueryScene *scene) {return scene && scene->overview && (scene->overview->incomplete || scene->limited);}
int gd_query_scene_overview_case(const GD_QueryScene *scene,int edge,int case_index) {
    const GD_RuleCase *item;const GD_Record *record;int a,b,has_a=0,has_b=0,p,i;
    if(!scene || !scene->overview || edge<0 || edge>=scene->layout.edge_count || scene->edges[edge].kind==GD_QUERY_EDGE_AXIS || case_index<0 || case_index>=scene->query->rules->case_count) return 0;
    item=&scene->query->rules->cases[case_index];record=&scene->query->model->records[scene->query->rules->sources[item->source].record];if(scene->query->model->collections[record->collection].role!=GD_CASE) return 0;
    for(i=0;i<scene->query->request.status_count;++i) if(record->status && !strcmp(record->status,scene->query->request.statuses[i])) break;
    if(scene->query->request.status_count && i==scene->query->request.status_count) return 0;
    a=scene->nodes[scene->layout.edges[edge].a].source;b=scene->nodes[scene->layout.edges[edge].b].source;
    for(p=item->first_participant;p<item->first_participant+item->participant_count;++p) {const GD_RuleParticipant *participant=&scene->query->rules->participants[p];if(participant->noun_resolution==GD_RULE_RESOLVED) {has_a|=participant->noun_record==a;has_b|=participant->noun_record==b;}}
    return has_a && has_b;
}
int gd_query_scene_overview_write(FILE *out,const GD_QueryScene *scene) {
    int i;if(!out || !scene || !scene->overview) return 0;
    fprintf(out,"{\"ok\":true,\"mode\":\"%s\",\"unclassified_nouns\":%d,\"incomplete\":%s,\"limited\":%s,\"work\":%d,\"work_limit\":%d,\"size_metric\":\"distinct connected actors\",\"nodes\":[",scene->overview->mode==GD_ACTOR_INTERACTIONS ? "interactions":"usage",scene->overview->unclassified,gd_query_scene_overview_incomplete(scene) ? "true":"false",scene->limited ? "true":"false",scene->work,scene->work_limit);
    for(i=0;i<scene->layout.node_count && !ferror(out);++i) {const GD_Record *record=&scene->query->model->records[scene->nodes[i].source];if(i) fputc(',',out);fputs("{\"reference\":",out);gd_json_string(out,record->reference);fprintf(out,",\"kind\":\"%s\",\"is_actor\":%s,\"connected_actors\":%d}",record->role==GD_AXIS ? "axis":"noun",record->role!=GD_NOUN || !record->is_actor_line ? "null":record->is_actor ? "true":"false",scene->nodes[i].incoming);}
    fputs("],\"links\":[",out);
    for(i=0;i<scene->layout.edge_count && !ferror(out);++i) {const GD_LayoutEdge *edge=&scene->layout.edges[i];if(i) fputc(',',out);fprintf(out,"{\"from\":%d,\"to\":%d,\"kind\":",edge->a,edge->b);gd_json_string(out,gd_query_scene_edge_kind_name(scene->edges[i].kind));fprintf(out,",\"record_count\":%d,\"evidence_source\":",scene->edges[i].kind==GD_QUERY_EDGE_AXIS ? 0:scene->overview->counts[i]);gd_json_string(out,scene->query->model->records[scene->edges[i].kind==GD_QUERY_EDGE_AXIS ? scene->nodes[edge->a].source:scene->query->rules->sources[scene->query->rules->cases[scene->edges[i].source].source].record].reference);fputc('}',out);}
    fputs("],\"claim\":\"same-case participation or declared axis use; not direct modification, frequency, applicability or reachability proof\"}",out);return !ferror(out);
}
#endif
