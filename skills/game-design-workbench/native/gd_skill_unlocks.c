#include "gd_skill_unlocks.h"
#include <string.h>

static int charge(GD_SkillUnlocks *query, size_t amount) {
    if (query->limited) return 0;
    if (amount>(size_t)(GD_SKILL_QUERY_WORK-query->work)) { query->limited=1; return 0; }
    query->work+=(int)amount; return 1;
}
const char *gd_skill_eligibility_name(GD_SkillEligibility state) {
    switch (state) {
        case GD_SKILL_NO_PREREQUISITES: return "no_declared_prerequisites";
        case GD_SKILL_NEEDS_MORE: return "needs_more";
        case GD_SKILL_PREREQUISITES_SUPPLIED: return "all_declared_prerequisites_supplied";
        default: return "unresolved";
    }
}
static void closure(const GD_Model *model, GD_SkillUnlocks *query, unsigned char *marked, int reverse) {
    int queue[GD_MAX_RECORDS], first=0,last=0,head[GD_MAX_RECORDS],next[GD_MAX_EDGES],i;
    for (i=0;i<GD_MAX_RECORDS;++i) head[i]=-1;
    for (i=model->skill_link_count-1;i>=0;--i) {
        const GD_SkillLink *link=&model->skill_links[i];
        int source=reverse ? link->dependent:link->prerequisite;
        if (!charge(query,1)) return;
        next[i]=head[source]; head[source]=i;
    }
    queue[last++]=query->root; marked[query->root]=1;
    while (first<last && !query->limited) {
        int source=queue[first++];
        for (i=head[source];i>=0;i=next[i]) {
            const GD_SkillLink *link=&model->skill_links[i];
            int target=reverse ? link->prerequisite:link->dependent;
            if (!charge(query,1)) return;
            if (!marked[target]) { marked[target]=1; queue[last++]=target; }
        }
    }
    marked[query->root]=0;
}
int gd_skill_unlocks(const GD_Model *model, const unsigned char *known, int root, GD_SkillUnlocks *query) {
    int i,j,root_first=-1,first[GD_MAX_RECORDS],next[GD_MAX_EDGES];
    if (!model || !query || !model->skill_mode || root < -1 || root>=model->record_count || (root>=0 && model->records[root].role!=GD_SKILL)) return 0;
    /* known may point to the previous result's known array. */
    { unsigned char assumptions[GD_MAX_RECORDS]={0};
      if (known) memcpy(assumptions,known,(size_t)model->record_count);
      memset(query,0,sizeof(*query)); memcpy(query->known,assumptions,(size_t)model->record_count);
    }
    query->root=root; query->work=4*GD_MAX_RECORDS+model->record_count+model->skill_link_count;
    for (i=0;i<model->record_count;++i) {
        query->known[i]=query->known[i] && model->records[i].role==GD_SKILL;
        query->eligibility[i]=GD_SKILL_UNRESOLVED; first[i]=-1;
    }
    for (i=0;i<model->skill_link_count;++i) {
        const GD_SkillLink *link=&model->skill_links[i];
        if (query->known[link->prerequisite]) ++query->supplied[link->dependent]; else ++query->missing[link->dependent];
    }
    for (i=0;i<model->record_count;++i) if (model->records[i].role==GD_SKILL && !model->skills[i].incomplete) {
        query->eligibility[i]=!model->skills[i].declared_prerequisites ? GD_SKILL_NO_PREREQUISITES :
            query->missing[i] ? GD_SKILL_NEEDS_MORE:GD_SKILL_PREREQUISITES_SUPPLIED;
    }
    if (root<0) return 1;
    closure(model,query,query->ancestors,1); closure(model,query,query->descendants,0);
    /* Compare one selected root, never all pairs. Keep exact source edges as
       evidence, including unresolved mechanic references. No score or ranking. */
    for (i=model->edge_count-1;i>=0;--i) {
        const GD_Edge *edge=&model->edges[i];
        if (!charge(query,1)) break;
        if (model->records[edge->source].role!=GD_SKILL || (edge->field!=GD_PREREQUISITES && edge->field!=GD_MECHANICS)) continue;
        next[i]=first[edge->source]; first[edge->source]=i;
    }
    root_first=first[root];
    for (i=0;i<model->record_count && !query->limited;++i) if (i!=root && model->records[i].role==GD_SKILL) {
        int a;
        for (a=root_first;a>=0 && !query->limited;a=next[a]) for (j=first[i];j>=0;j=next[j]) {
            const GD_Edge *left=&model->edges[a],*right=&model->edges[j];
            size_t length=strlen(left->reference);
            if (!charge(query,3*(length+1))) break;
            if (left->field!=right->field || strcmp(left->reference,right->reference)) continue;
            if (query->comparison_count==GD_SKILL_COMPARISONS) ++query->comparisons_omitted;
            else query->comparisons[query->comparison_count++]=(GD_SkillComparison){i,a,j};
        }
    }
    return 1;
}
static void reference(FILE *output, const GD_Model *model, int index) {
    gd_json_string(output,index>=0 ? model->records[index].reference:NULL);
}
void gd_write_skill_unlocks(FILE *output, const GD_Model *model, const GD_SkillUnlocks *query) {
    int i,j,comma=0;
    fputs("{\"root\":",output); reference(output,model,query->root);
    fprintf(output,",\"complete\":%s,\"work\":%d,\"work_limit\":%d,\"comparison_limit\":%d,\"comparisons_omitted\":%d,\"skills\":[",query->limited || query->comparisons_omitted ? "false":"true",query->work,GD_SKILL_QUERY_WORK,GD_SKILL_COMPARISONS,query->comparisons_omitted);
    for (i=0;i<model->record_count;++i) if (model->records[i].role==GD_SKILL) {
        if (comma++) fputc(',',output);
        fputs("{\"reference\":",output); reference(output,model,i);
        fputs(",\"eligibility\":",output); gd_json_string(output,gd_skill_eligibility_name(query->eligibility[i]));
        fprintf(output,",\"known_assumption\":%s,\"prerequisite_of_root\":%s,\"unlocked_from_root\":%s,\"supplied_count\":%d,\"missing_count\":%d,\"missing\":[",query->known[i] ? "true":"false",query->ancestors[i] ? "true":"false",query->descendants[i] ? "true":"false",query->supplied[i],query->missing[i]);
        { int count=0; for (j=0;j<model->skill_link_count;++j) if (model->skill_links[j].dependent==i && !query->known[model->skill_links[j].prerequisite]) {
            if (count++) fputc(',',output); reference(output,model,model->skill_links[j].prerequisite);
        }}
        fputs("]}",output);
    }
    fputs("],\"comparison_candidates\":[",output);
    for (i=0;i<query->comparison_count;++i) {
        const GD_SkillComparison *item=&query->comparisons[i];
        const GD_Edge *edge=&model->edges[item->root_edge];
        if (i) fputc(',',output);
        fputs("{\"candidate\":",output); reference(output,model,item->candidate);
        fputs(",\"field\":",output); gd_json_string(output,gd_field_name(edge->field));
        fputs(",\"shared_reference\":",output); gd_json_string(output,edge->reference);
        fprintf(output,",\"root_line\":%d,\"candidate_line\":%d}",edge->line,model->edges[item->candidate_edge].line);
    }
    fputs("],\"scope\":\"Explicit hypothetical knowledge only. No automatic prerequisite inference or learning. Structural unlock chains and shared-link comparison candidates do not prove teaching contrast, transfer or mastery. All statuses retained; unresolved ancestry and cycles remain unresolved.\"}",output);
}
