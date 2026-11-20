#include "gd_noun_grid.h"
#include <string.h>
#include <stdint.h>
#include <math.h>
static unsigned noun_pair_slot(int a,int b) {
    uint64_t key=((uint64_t)(unsigned)a<<32)|(unsigned)b;
    key^=key>>33;key*=UINT64_C(0xff51afd7ed558ccd);key^=key>>33;
    return (unsigned)key&(GD_MAX_PAIRS*2-1);
}
static int noun_grid_charge(GD_NounGrid *grid) {
    if(grid->work==grid->work_limit) {grid->limited=grid->incomplete=1;return 0;}++grid->work;return 1;
}
static int noun_interaction_source(const GD_Rules *rules,int source) {
    return rules->model->collections[rules->model->records[rules->sources[source].record].collection].role==GD_CASE;
}
GD_NounGrid *gd_noun_grid_build(const GD_Rules *rules,int work_limit) {
    GD_NounGrid *grid;int *scratch,*members,*multiplicity,i,c,p,j,k,count;
    if(!rules || !rules->model || work_limit<0 || work_limit>GD_NOUN_GRID_WORK) return NULL;
    grid=rules->allocator.allocate(rules->allocator.context,sizeof(*grid));if(!grid)return NULL;
    memset(grid,0,sizeof(*grid));grid->allocator=rules->allocator;grid->rules=rules;grid->work_limit=work_limit;
    scratch=grid->allocator.allocate(grid->allocator.context,GD_MAX_RECORDS*2*sizeof(*scratch));
    if(!scratch){gd_noun_grid_destroy(grid);return NULL;}members=scratch;multiplicity=scratch+GD_MAX_RECORDS;
    for(i=0;i<GD_MAX_RECORDS;++i)grid->slot[i]=-1;
    grid->incomplete=rules->model->error_count || rules->model->errors_omitted || rules->findings_omitted || !rules->bindings_complete;
    /* Axis inventory is always complete and bounded by GD_MAX_RECORDS.
       Pair-construction work limits never hide loaded noun definitions. */
    for(i=0;i<rules->model->record_count;++i) {
        if(rules->model->records[i].role==GD_NOUN) {grid->slot[i]=grid->noun_count;grid->nouns[grid->noun_count++]=i;}
    }
    for(i=0;i<rules->source_count && !grid->limited;++i) {
        if(!noun_grid_charge(grid))break;
        if(noun_interaction_source(rules,i) && rules->sources[i].state!=GD_RULE_PARSED)grid->incomplete=1;
    }
    for(c=0;c<rules->case_count && !grid->limited;++c) {
        const GD_RuleCase *item=&rules->cases[c];if(!noun_grid_charge(grid))break;
        if(!noun_interaction_source(rules,item->source))continue;
        memset(multiplicity,0,GD_MAX_RECORDS*sizeof(*multiplicity));count=0;
        for(p=item->first_participant;p<item->first_participant+item->participant_count;++p) {
            const GD_RuleParticipant *participant=&rules->participants[p];int record=participant->noun_record;
            if(!noun_grid_charge(grid))break;
            if(participant->noun_resolution!=GD_RULE_RESOLVED || record<0 || record>=rules->model->record_count || grid->slot[record]<0){grid->incomplete=1;continue;}
            if(!multiplicity[record]++)members[count++]=record;
        }
        for(j=0;j<count && !grid->limited;++j)for(k=j;k<count && !grid->limited;++k) {
            int a=members[j],b=members[k],pair;unsigned slot;
            if(!noun_grid_charge(grid))break;
            if(a==b && multiplicity[a]<2)continue;
            if(a>b){int swap=a;a=b;b=swap;}slot=noun_pair_slot(a,b);
            for(;;){if(!noun_grid_charge(grid))break;pair=grid->hash[slot]-1;if(pair<0 || (grid->pairs[pair].a==a && grid->pairs[pair].b==b))break;slot=(slot+1)&(GD_MAX_PAIRS*2-1);}
            if(grid->limited)break;
            if(pair<0){if(grid->pair_count==GD_MAX_PAIRS){grid->limited=grid->incomplete=1;break;}pair=grid->pair_count++;grid->pairs[pair]=(GD_NounPair){a,b,0,c,c};grid->hash[slot]=pair+1;}
            ++grid->pairs[pair].count;grid->pairs[pair].last_case=c;
        }
    }
    grid->allocator.release(grid->allocator.context,scratch);return grid;
}
void gd_noun_grid_destroy(GD_NounGrid *grid){if(grid)grid->allocator.release(grid->allocator.context,grid);}
int gd_noun_grid_layout(const GD_NounGrid *grid,GD_Layout *layout) {
    int i;
    if(!grid || !layout)return 0;
    memset(layout,0,sizeof(*layout));layout->direct_routes=1;
    layout->spring_gap=GD_NOUN_GRAPH_MIN_REST_LENGTH-GD_NOUN_GRAPH_CARD_HEIGHT;
    layout->node_count=grid->noun_count;layout->edge_count=grid->pair_count;
    for(i=0;i<grid->noun_count;++i){GD_LayoutNode *node=&layout->nodes[i];const GD_Record *record=&grid->rules->model->records[grid->nouns[i]];node->key=grid->nouns[i];node->label=record->display_name ? record->display_name:record->name;node->width=fmaxf(240,20+10*(float)strlen(node->label));node->height=GD_NOUN_GRAPH_CARD_HEIGHT;gd_layout_set_footprint(node,-node->width/2,-node->height/2,node->width/2,node->height/2);}
    for(i=0;i<grid->pair_count;++i){const GD_NounPair *pair=&grid->pairs[i];layout->edges[i]=(GD_LayoutEdge){grid->slot[pair->a],grid->slot[pair->b],i,1,GD_RELATION_CASE_PAIR};}
    return gd_layout_components(layout) && gd_layout_arrange(layout,GD_LAYOUT_FORCE,GD_SORT_NAME);
}
int gd_noun_grid_pair(const GD_NounGrid *grid,int a,int b) {
    unsigned slot;int steps;
    if(!grid || a<0 || b<0 || a>=grid->rules->model->record_count || b>=grid->rules->model->record_count)return -1;
    if(a>b){int swap=a;a=b;b=swap;}slot=noun_pair_slot(a,b);
    for(steps=0;steps<GD_MAX_PAIRS*2;++steps){int pair=grid->hash[slot]-1;if(pair<0)return -1;if(grid->pairs[pair].a==a && grid->pairs[pair].b==b)return pair;slot=(slot+1)&(GD_MAX_PAIRS*2-1);}return -1;
}
int gd_noun_grid_case(const GD_NounGrid *grid,int pair,int case_index) {
    const GD_RuleCase *item;const GD_NounPair *cell;int a=0,b=0,p;
    if(!grid || pair<0 || pair>=grid->pair_count || case_index<0 || case_index>=grid->rules->case_count)return 0;
    /* Each retained pair contains a case prefix, including partial constructions. */
    if(case_index>grid->pairs[pair].last_case)return 0;
    item=&grid->rules->cases[case_index];if(!noun_interaction_source(grid->rules,item->source))return 0;cell=&grid->pairs[pair];
    for(p=item->first_participant;p<item->first_participant+item->participant_count;++p){const GD_RuleParticipant *participant=&grid->rules->participants[p];if(participant->noun_resolution==GD_RULE_RESOLVED){a+=participant->noun_record==cell->a;b+=participant->noun_record==cell->b;}}
    return cell->a==cell->b ? a>=2:a>0 && b>0;
}
int gd_noun_grid_write(FILE *out,const GD_NounGrid *grid) {
    int i;if(!out || !grid)return 0;
    fprintf(out,"{\"ok\":true,\"metric\":\"distinct declared cases\",\"symmetric\":true,\"limited\":%s,\"incomplete\":%s,\"work\":%d,\"work_limit\":%d,\"nouns\":[",grid->limited ? "true":"false",grid->incomplete ? "true":"false",grid->work,grid->work_limit);
    for(i=0;i<grid->noun_count && !ferror(out);++i){const GD_Record *record=&grid->rules->model->records[grid->nouns[i]];if(i)fputc(',',out);fputs("{\"reference\":",out);gd_json_string(out,record->reference);fputs(",\"label\":",out);gd_json_string(out,record->display_name);fprintf(out,",\"is_actor\":%s}",!record->is_actor_line ? "null":record->is_actor ? "true":"false");}
    fputs("],\"cells\":[",out);
    for(i=0;i<grid->pair_count && !ferror(out);++i){const GD_NounPair *pair=&grid->pairs[i];if(i)fputc(',',out);fprintf(out,"{\"row\":%d,\"column\":%d,\"case_count\":%d,\"first_case\":%d}",grid->slot[pair->a],grid->slot[pair->b],pair->count,pair->first_case);}
    fputs("],\"claim\":\"shared declared cases, not direct modification, applicability or reachability\"}",out);return !ferror(out);
}
