#include "gd_named_query.h"
#include "gd_query.h"
#include <string.h>

static const char *const names[]={"conditional-outcomes","conditional-consistency","skill-unlocks","resource-declarations","resource-routes"};
const char *gd_named_name(GD_NamedKind kind) { return kind>=0 && kind<GD_NAMED_COUNT ? names[kind]:NULL; }
int gd_named_kind(const char *name) { int i; if (name) for (i=0;i<GD_NAMED_COUNT;++i) if (!strcmp(name,names[i])) return i; return -1; }
const char *gd_named_projection(GD_NamedKind kind) { return kind==GD_NAMED_OUTCOMES || kind==GD_NAMED_CONSISTENCY ? "rules":kind==GD_NAMED_SKILLS ? "skills":kind==GD_NAMED_RESOURCE_DECLARATIONS || kind==GD_NAMED_RESOURCE_ROUTES ? "resources":NULL; }
int gd_named_fields(GD_NamedKind kind,const char *fields) {
    static const char *const rule[]={"records","cases","axes","actors","findings","analysis"},*const skill[]={"skills","links","components","findings","exploration"},*const resource[]={"nodes","relations","routes","findings"};
    const char *const *values=kind==GD_NAMED_SKILLS ? skill:kind<=GD_NAMED_CONSISTENCY ? rule:resource; int count=kind==GD_NAMED_SKILLS ? 5:kind<=GD_NAMED_CONSISTENCY ? 6:4;
    const char *start=fields;
    if (kind<0 || kind>=GD_NAMED_COUNT || !fields || !*fields) return 0;
    while (*start) {
        const char *end=strchr(start,','); size_t length=end ? (size_t)(end-start):strlen(start); int i,found=0;
        for (i=0;i<count;++i) if (length==strlen(values[i]) && !strncmp(start,values[i],length)) found=1;
        if (!found) return 0; if (!end) return 1; start=end+1;
    }
    return 0;
}
static int bounded(const char *text) { size_t length=0; if (!text) return 1; while (length<GD_MAX_RULE_LINE && text[length]) ++length; return length>0 && length<GD_MAX_RULE_LINE; }
static int valid(GD_Model *diagnostic,const GD_NamedRequest *request) {
    int i,rules,skill,routes;
    if (!request || request->kind<0 || request->kind>=GD_NAMED_COUNT || !request->projection || strcmp(request->projection,gd_named_projection(request->kind))) { gd_error(diagnostic,"query.projection_unavailable","Named operation requires its advertised projection; inspect query --list",NULL,0); return 0; }
    rules=request->kind==GD_NAMED_OUTCOMES; skill=request->kind==GD_NAMED_SKILLS; routes=request->kind==GD_NAMED_RESOURCE_ROUTES;
    if (request->assumption_count<0 || request->assumption_count>GD_NAMED_SELECTORS || request->context_count<0 || request->context_count>GD_NAMED_SELECTORS || request->known_count<0 || request->known_count>GD_NAMED_SELECTORS || request->accepted_only<0 || request->accepted_only>1 || request->work_limit<-1 ||
        (!rules && (request->case_name || request->assumption_count || request->context_count)) || (!skill && request->known_count) || (!skill && !routes && request->from) || (!routes && (request->to || request->accepted_only)) ||
        (rules && !request->case_name) || (routes && !request->to) || (!rules && !routes && request->work_limit!=-1) || ((rules || routes) && request->work_limit>(rules ? GD_RULE_EVALUATION_WORK_LIMIT:GD_RESOURCE_ROUTE_WORK))) { gd_error(diagnostic,"query.arguments","Use bounded operation-specific selectors/assumptions; outcomes need case, routes need to; fixed-budget delegates reject overrides",NULL,0); return 0; }
    if (!bounded(request->case_name) || !bounded(request->from) || !bounded(request->to)) { gd_error(diagnostic,"query.arguments","Selectors need bounded nonempty native identities",NULL,0); return 0; }
    for (i=0;i<request->assumption_count;++i) if (!request->assumptions[i] || !bounded(request->assumptions[i])) { gd_error(diagnostic,"query.arguments","Assumptions need bounded literal input",NULL,0); return 0; }
    for (i=0;i<request->context_count;++i) if (!request->contexts[i] || !bounded(request->contexts[i]) || (request->context_truths[i]!=GD_RULE_TRUTH_TRUE && request->context_truths[i]!=GD_RULE_TRUTH_FALSE)) { gd_error(diagnostic,"query.arguments","Context needs bounded triple and explicit true/false truth",NULL,0); return 0; }
    for (i=0;i<request->known_count;++i) if (!request->known[i] || !bounded(request->known[i])) { gd_error(diagnostic,"query.arguments","Known assumptions need bounded exact skill references",NULL,0); return 0; }
    return 1;
}
static int scenario(GD_Model *diagnostic,GD_Model *model,const GD_Rules *rules,int root,const GD_NamedRequest *request,GD_RuleScenario *input) {
    GD_Error saved[GD_MAX_ERRORS]; int before=model->error_count,omitted=model->errors_omitted,i,ok=1;
    /* Legacy parser combines source lookup and diagnostic channel. Single-threaded
       adapter has no observer callbacks; restore every diagnostic slot, not only
       counts, before returning borrowed source to other readers. Facts untouched. */
    if (model!=diagnostic) memcpy(saved,model->errors,sizeof(saved));
    for (i=0;i<request->assumption_count;++i) if (!gd_rule_assume(model,rules,root,request->assumptions[i],input)) ok=0;
    for (i=0;i<request->context_count;++i) if (!gd_rule_assume_context(model,rules,root,request->contexts[i],request->context_truths[i],input)) ok=0;
    if (model!=diagnostic) {
        for (i=before;i<model->error_count;++i) { const GD_Error *error=&model->errors[i]; gd_error(diagnostic,error->code,error->message,error->path,error->line); }
        diagnostic->errors_omitted+=model->errors_omitted-omitted;
        memcpy(model->errors,saved,sizeof(saved)); model->error_count=before; model->errors_omitted=omitted;
    }
    return ok;
}
GD_NamedQuery *gd_named_build(GD_Model *diagnostic,const GD_ExplorerProject *project,const GD_NamedRequest *request) {
    GD_NamedQuery *result; GD_Model *model; int i;
    if (!diagnostic || !valid(diagnostic,request)) return NULL;
    model=project ? (request->kind==GD_NAMED_SKILLS ? gd_explorer_model(project,GD_EXPLORER_SKILLS):project->models[GD_EXPLORER_RULE_DATA]):NULL;
    if (!project || !model || (request->kind!=GD_NAMED_SKILLS && (!project->rules || !project->calls)) || (request->kind>=GD_NAMED_RESOURCE_DECLARATIONS && !project->resources)) { gd_error(diagnostic,"query.projection_unavailable","Required explicit source bindings for named operation are unavailable",NULL,0); return NULL; }
    if (model->error_count) { gd_error(diagnostic,"query.source_invalid","Selected native reader has errors; no delegated result adopted",NULL,0); return NULL; }
    result=project->allocator.allocate(project->allocator.context,sizeof(*result));
    if (!result) { gd_error(diagnostic,"query.memory","Cannot allocate named native result",NULL,0); return NULL; }
    memset(result,0,sizeof(*result)); result->allocator=project->allocator; result->project=project; result->model=model; result->request=*request; result->root=-1; result->revision=project->revision; result->owned_bytes=sizeof(*result);
    if (request->kind<=GD_NAMED_CONSISTENCY) {
        result->analysis=gd_rule_analysis_workspace(project->allocator,project->calls);
        if (!result->analysis) goto memory;
        result->owned_bytes+=sizeof(*result->analysis);
        if (request->kind==GD_NAMED_CONSISTENCY) gd_rule_compare(result->analysis);
        else {
            result->root=gd_rule_case_find(model,project->rules,request->case_name);
            if (result->root<0) { gd_error(diagnostic,"rules.case","Case must identify one parsed source#case; local name omitted only when unique",request->case_name,0); goto failure; }
            if (!scenario(diagnostic,model,project->rules,result->root,request,&result->scenario)) goto failure;
            result->analysis->requested_case=result->root;
            { GD_RuleAssumptions inputs={result->scenario.facts,result->scenario.fact_count,result->scenario.contexts,result->scenario.context_count};
              gd_rule_evaluate_bounded(result->analysis,result->root,inputs,&result->analysis->scenario,request->work_limit<0 ? GD_RULE_EVALUATION_WORK_LIMIT:request->work_limit,NULL);
              if (!result->analysis->scenario.input_valid) { gd_error(diagnostic,"rules.scenario","Contradictory or invalid typed assumptions; no applicability claim",NULL,0); goto failure; }
            }
        }
    } else if (request->kind==GD_NAMED_SKILLS) {
        unsigned char known[GD_MAX_RECORDS]={0};
        for (i=0;i<request->known_count+(request->from ? 1:0);++i) {
            const char *name=i<request->known_count ? request->known[i]:request->from; int record=gd_query_record(model,name,GD_SKILL);
            if (record<0) { gd_error(diagnostic,"skills.selector","Known skill/root must identify one unique loaded skill",name,0); goto failure; }
            if (i<request->known_count) known[record]=1; else result->root=record;
        }
        result->skills=project->allocator.allocate(project->allocator.context,sizeof(*result->skills)); if (!result->skills) goto memory;
        result->owned_bytes+=sizeof(*result->skills); gd_skill_unlocks(model,known,result->root,result->skills);
    } else if (request->kind==GD_NAMED_RESOURCE_ROUTES) {
        int from=-1,to;
        result->routes=gd_resource_routes_create(project->allocator); if (!result->routes) goto memory;
        result->owned_bytes+=sizeof(*result->routes);
        if (request->from) { from=gd_resource_select(result->routes,project->resources,request->from); result->selector_work+=result->routes->work; }
        if (request->from && from<0) { gd_error(diagnostic,from==-2 ? "resources.selector_ambiguous":from==-3 ? "resources.selector_limit":"resources.selector_missing","Resource source selector missing, ambiguous or limited",request->from,0); goto failure; }
        to=gd_resource_select(result->routes,project->resources,request->to); result->selector_work+=result->routes->work;
        if (to<0) { gd_error(diagnostic,to==-2 ? "resources.selector_ambiguous":to==-3 ? "resources.selector_limit":"resources.selector_missing","Resource target selector missing, ambiguous or limited",request->to,0); goto failure; }
        if (!gd_resource_query(result->routes,project->resources,from,to,request->accepted_only,request->work_limit<0 ? GD_RESOURCE_ROUTE_WORK:request->work_limit)) { gd_error(diagnostic,"resources.query","Invalid native route parameters",NULL,0); goto failure; }
    }
    return result;
memory:
    gd_error(diagnostic,"query.memory","Cannot allocate private native delegate workspace; prior result retained",NULL,0);
failure:
    gd_named_destroy(result); return NULL;
}
void gd_named_destroy(GD_NamedQuery *query) {
    if (!query) return;
    if (query->skills) query->allocator.release(query->allocator.context,query->skills);
    gd_rule_analysis_destroy(query->analysis); gd_resource_routes_destroy(query->routes); query->allocator.release(query->allocator.context,query);
}
static void strings(FILE *out,const char *const *values,int count) { int i; fputc('[',out); for (i=0;i<count;++i) { if (i) fputc(',',out); gd_json_string(out,values[i]); } fputc(']',out); }
int gd_write_named_result(FILE *out,const GD_NamedQuery *query,int json,const char *fields) {
    const GD_NamedRequest *request; int i,written=0;
    if (!out || !query || !query->model || (fields && (!json || !gd_named_fields(query->request.kind,fields)))) return 0;
    request=&query->request;
    if (request->kind==GD_NAMED_SKILLS && (query->revision!=query->project->revision || query->model!=gd_explorer_model(query->project,GD_EXPLORER_SKILLS))) return 0;
    if (!json) {
        fprintf(out,"%s [%s]: delegated native report; explicit assumptions, no execution or observed knowledge proof\n",gd_named_name(request->kind),request->projection);
        fprintf(out,"case: %s | from: %s | to: %s | accepted-only: %s | work override: %d (-1 native default)\n",request->case_name ? request->case_name:"none",request->from ? request->from:"none",request->to ? request->to:"none",request->accepted_only ? "yes":"no",request->work_limit);
        for (i=0;i<request->assumption_count;++i) fprintf(out,"assume: %s\n",request->assumptions[i]);
        for (i=0;i<request->known_count;++i) fprintf(out,"known assumption: %s\n",request->known[i]);
        for (i=0;i<request->context_count;++i) fprintf(out,"context %s: %s\n",gd_rule_truth_name(request->context_truths[i]),request->contexts[i]);
        return request->kind<=GD_NAMED_CONSISTENCY ? gd_write_rule_report(out,query->model,query->project->rules,query->analysis,0,NULL):request->kind==GD_NAMED_SKILLS ? gd_write_skill_result(out,query->model,query->skills,0,NULL):gd_write_resource_query(out,query->model,query->project->resources,query->routes,0,NULL);
    }
    fputs("{\"schema_version\":1,\"ok\":true,\"data\":{\"question\":",out); gd_json_string(out,gd_named_name(request->kind)); fputs(",\"projection\":",out); gd_json_string(out,request->projection);
    fputs(",\"parameters\":{\"case\":",out); gd_json_string(out,request->case_name); fputs(",\"from\":",out); gd_json_string(out,request->from); fputs(",\"to\":",out); gd_json_string(out,request->to);
    fprintf(out,",\"accepted_only\":%s,\"work_limit_override\":%d,\"assumptions\":",request->accepted_only ? "true":"false",request->work_limit); strings(out,request->assumptions,request->assumption_count);
    fputs(",\"known\":",out); strings(out,request->known,request->known_count); fputs(",\"contexts\":[",out);
    for (i=0;i<request->context_count;++i) { if (i) fputc(',',out); fputs("{\"literal\":",out); gd_json_string(out,request->contexts[i]); fputs(",\"truth\":",out); gd_json_string(out,gd_rule_truth_name(request->context_truths[i])); fputc('}',out); }
    fputs("]},\"native_result\":",out);
    if (request->kind<=GD_NAMED_CONSISTENCY) written=gd_write_rule_report(out,query->model,query->project->rules,query->analysis,1,fields);
    else if (request->kind==GD_NAMED_SKILLS) written=gd_write_skill_result(out,query->model,query->skills,1,fields);
    else written=gd_write_resource_query(out,query->model,query->project->resources,query->routes,1,fields);
    if (!written) return 0;
    fprintf(out,"},\"coverage\":{\"native_result_authoritative\":true,\"scope\":\"whole supplied native reader context, never camera/display filters\",\"fixed_preparation_separate_from_search\":true,\"native_selector_work\":%d,\"owned_result_bytes\":%llu,\"execution_proven\":false,\"observed_learning_proven\":false},\"errors\":[],\"breadcrumbs\":[\"Inspect native_result coverage, source paths, conditions and limits; no second outcome/knowledge/Resource evaluator.\",\"Known skills are explicit assumptions, not transitively learned; prerequisites remain AND. Resource requirements enable without consumption, pool+axis joins and conditional witnesses do not establish execution.\",\"Rule comparisons align uniquely typed local roles hypothetically; existential conflict and bounded equivalence do not establish shared runtime actors or gameplay occurrence.\"]}\n",query->selector_work,(unsigned long long)query->owned_bytes);
    return !ferror(out);
}
int gd_write_named_catalogue(FILE *out,int json,int comma) {
    int kind;
    for (kind=0;kind<GD_NAMED_COUNT;++kind) {
        if (!json) { fprintf(out,"%s [%s]: delegated native semantics; query --list --json for exact parameters\n",names[kind],gd_named_projection((GD_NamedKind)kind)); continue; }
        if (comma++) fputc(',',out); fputs("{\"name\":",out); gd_json_string(out,names[kind]); fputs(",\"projection\":",out); gd_json_string(out,gd_named_projection((GD_NamedKind)kind));
        fputs(",\"required_bindings\":{\"all_of\":[",out); if (kind==GD_NAMED_SKILLS) gd_json_string(out,"skill-atoms"); fputs("],\"any_of\":[",out); if (kind!=GD_NAMED_SKILLS) fputs("\"interactions\",\"mechanics\"",out); fputs("]},\"required_parameters\":[",out);
        if (kind==GD_NAMED_OUTCOMES) gd_json_string(out,"--case"); if (kind==GD_NAMED_RESOURCE_ROUTES) gd_json_string(out,"--to");
        fputs("],\"parameters\":[",out);
        if (kind==GD_NAMED_OUTCOMES) fputs("\"--case\",\"--assume (repeat)\",\"--context (repeat)\",\"--context-not (repeat)\",\"--work-limit 0..16777216\"",out);
        if (kind==GD_NAMED_SKILLS) fputs("\"--known (repeat)\",\"--from (optional selected skill)\"",out);
        if (kind==GD_NAMED_RESOURCE_ROUTES) fputs("\"--from (optional; blank upstream)\",\"--to\",\"--accepted-only\",\"--work-limit 0..16777216\"",out);
        fputs("],\"fields\":[",out);
        if (kind<=GD_NAMED_CONSISTENCY) fputs("\"records\",\"cases\",\"axes\",\"actors\",\"findings\",\"analysis\"",out);
        else if (kind==GD_NAMED_SKILLS) fputs("\"skills\",\"links\",\"components\",\"findings\",\"exploration\"",out);
        else fputs("\"nodes\",\"relations\",\"routes\",\"findings\"",out);
        fputs("],\"field_scope\":\"native_result.data; question/parameters/coverage remain in envelope\",\"ui_available\":true,\"ui_scope\":",out);
        gd_json_string(out,kind==GD_NAMED_SKILLS ? "Skills: Skill opportunities / knowledge; explicit known assumptions, selected chains, combinations and shared-link candidates":kind>=GD_NAMED_RESOURCE_DECLARATIONS ? "Resources: whole declarations and conditional route controls with native witness highlights; not standalone preview":kind==GD_NAMED_CONSISTENCY ? "Resources > Object bindings / rule checks: whole-reader consistency, native pairs/witnesses, exact source; not Resource applicability/flow":"Selection > Inspect conditional outcomes in current pane, or Resources > Object bindings / rule checks: exact-case staging and separate Run, native proposals and exact source; not execution or final outcome proof");
        fputs(",\"ui_mode\":",out); gd_json_string(out,kind==GD_NAMED_SKILLS ? "skills":"resources");
        fputs(",\"ui_validation\":\"bounded app-local window/render checks; real-pointer/manual usefulness deferred\",\"ipc_available\":false,\"native_work_limit\":",out);
        if (kind==GD_NAMED_RESOURCE_DECLARATIONS) fputs("null",out); else fprintf(out,"%d",kind==GD_NAMED_CONSISTENCY ? GD_RULE_ANALYSIS_WORK_LIMIT:kind==GD_NAMED_OUTCOMES ? GD_RULE_EVALUATION_WORK_LIMIT:kind==GD_NAMED_SKILLS ? GD_SKILL_QUERY_WORK:GD_RESOURCE_ROUTE_WORK);
        fprintf(out,",\"work_limit_override_available\":%s,\"repeat_limit\":64,\"status_filter_available\":false,\"direction_scope\":\"native domain-specific semantics; no generic graph direction/filter\"}",kind==GD_NAMED_OUTCOMES || kind==GD_NAMED_RESOURCE_ROUTES ? "true":"false");
    }
    return !ferror(out);
}
int gd_named_cli(GD_Allocator allocator,const GD_Collection *collections,int count,const GD_NamedRequest *request,int json,const char *fields,const char *initial_error) {
    GD_Model *diagnostic=gd_create(allocator); GD_ExplorerProject *project=NULL; GD_NamedQuery *query=NULL; int status=2,i;
    if (!diagnostic) { fputs("gd: cannot allocate native delegate diagnostic\n",stderr); return 2; }
    if (initial_error) gd_error(diagnostic,"cli.arguments",initial_error,NULL,0);
    else if (!count || (fields && (!json || !request || !gd_named_fields(request->kind,fields)))) gd_error(diagnostic,"query.arguments","Explicit bindings and supported native fields required",NULL,0);
    else (void)valid(diagnostic,request);
    if (!diagnostic->error_count) {
        project=gd_explorer_load(allocator,collections,count);
        if (!project) gd_error(diagnostic,"query.memory","Cannot allocate native source snapshots",NULL,0);
        else { const GD_Error *error=gd_explorer_error(project); if (error) gd_error(diagnostic,error->code,error->message,error->path,error->line); else query=gd_named_build(diagnostic,project,request); }
    }
    if (!diagnostic->error_count && query) status=gd_write_named_result(stdout,query,json,fields) && !fflush(stdout) ? 0:2;
    else { gd_write_result(stdout,diagnostic,0,json,NULL); for (i=0;i<diagnostic->error_count;++i) fprintf(stderr,"gd: %s: %s\n",diagnostic->errors[i].code,diagnostic->errors[i].message); }
    gd_named_destroy(query); gd_explorer_destroy(project); gd_destroy(diagnostic); return status;
}
