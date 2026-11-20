#include "gd_named_query.h"
#include "test_structure_memory.h"
static unsigned long long named_hash(const void *data,size_t size) {
    const unsigned char *bytes=data; unsigned long long value=1469598103934665603ULL; size_t i;
    for (i=0;i<size;++i) { value^=bytes[i]; value*=1099511628211ULL; } return value;
}
TEST(named_rule_delegates_private_slots_inputs_limits_and_failures,
    StructureMemory memory={0,-1}; GD_Allocator allocator={&memory,structure_allocate,structure_release}; GD_Model *diagnostic=model_create(); GD_ExplorerProject *project; GD_NamedQuery *query; GD_NamedRequest request={0};
    GD_RuleEvaluation *expected; GD_RuleAnalysis *comparison; unsigned long long source_hash,compiled_hash; int baseline,stage; FILE *out;
    TEST_ASSERT(diagnostic && setup("named-rules")); collections[1].role=GD_AXIS; collections[2].role=GD_RULE;
    TEST_ASSERT(write_record(0,"unit","---\naxes: [actions/Life]\n---\n")); TEST_ASSERT(write_record(0,"empty","---\naxes: []\n---\n"));
    TEST_ASSERT(write_record(1,"Life","---\nvalue_type: enum\nallowed_values: [alive, dead]\nclosed: true\n---\n"));
    TEST_ASSERT(write_record(2,"rules","---\nstatus: accepted\n---\n```gd-rule\ncase shared:\n    target: A\n    context: ANY\n    A:\n        noun: things/unit\n        requires: ANY\n        preserve: ALL\ncase caller:\n    target: Left\n    context: ANY\n    Left:\n        noun: things/unit\n        requires: ANY\n        causes: cases/rules#shared with A=Left\n        preserve: ALL\ncase silent:\n    target: A\n    context: ANY\n    A:\n        noun: things/empty\n        requires: ANY\n        preserve: ALL\n```\n"));
    project=gd_explorer_load(allocator,collections,3); TEST_ASSERT(project && !gd_explorer_error(project)); baseline=memory.outstanding;
    source_hash=named_hash(project->models[GD_EXPLORER_RULE_DATA],sizeof(GD_Model)); compiled_hash=named_hash(project->calls,sizeof(*project->calls));
    request.kind=GD_NAMED_OUTCOMES; request.projection="rules"; request.case_name="cases/rules#caller"; request.work_limit=-1; request.assumptions[0]="Left:actions/Life=alive"; request.assumption_count=1;
    query=gd_named_build(diagnostic,project,&request); TEST_ASSERT(query && !diagnostic->error_count && query->analysis!=project->calls && query->analysis->model==project->calls->model && query->analysis->scenario.input_valid==1);
    expected=calloc(1,sizeof(*expected)); TEST_ASSERT(expected);
    gd_rule_evaluate_bounded(project->calls,query->root,query->analysis->scenario.assumptions,expected,GD_RULE_EVALUATION_WORK_LIMIT,NULL);
    TEST_ASSERT(expected->work==query->analysis->scenario.work && expected->write_count==query->analysis->scenario.write_count && !memcmp(expected->writes,query->analysis->scenario.writes,(size_t)expected->write_count*sizeof(*expected->writes)));
    out=tmpfile(); TEST_ASSERT(out && gd_write_named_result(out,query,1,"analysis") && !gd_write_named_result(out,query,1,"unsupported") && !gd_write_named_result(out,query,0,"analysis")); fclose(out); free(expected); gd_named_destroy(query);
    TEST_ASSERT(memory.outstanding==baseline && source_hash==named_hash(project->models[GD_EXPLORER_RULE_DATA],sizeof(GD_Model)) && compiled_hash==named_hash(project->calls,sizeof(*project->calls)));
    request.case_name="cases/rules#silent"; request.assumption_count=0; query=gd_named_build(diagnostic,project,&request); TEST_ASSERT(query && !query->analysis->scenario.write_count && !query->analysis->scenario.limited); gd_named_destroy(query); request.case_name="cases/rules#caller"; request.assumption_count=1;
    for (stage=0;stage<2;++stage) { memory.remaining=stage; query=gd_named_build(diagnostic,project,&request); memory.remaining=-1; TEST_ASSERT(!query && diagnostic->error_count && memory.outstanding==baseline); diagnostic->error_count=0; }
    request.assumptions[0]="Left:actions/Life=not-authored"; TEST_ASSERT(!gd_named_build(diagnostic,project,&request) && diagnostic->error_count && memory.outstanding==baseline && source_hash==named_hash(project->models[GD_EXPLORER_RULE_DATA],sizeof(GD_Model))); diagnostic->error_count=0;
    request.assumptions[0]="Left:actions/Life=alive"; request.assumptions[1]="Left:actions/Life=dead"; request.assumption_count=2;
    TEST_ASSERT(!gd_named_build(diagnostic,project,&request) && diagnostic->error_count && memory.outstanding==baseline); diagnostic->error_count=0;
    request.assumption_count=1; request.work_limit=0; query=gd_named_build(diagnostic,project,&request); TEST_ASSERT(query && query->analysis->scenario.limited && query->analysis->scenario.input_valid==-1 && !query->analysis->scenario.write_count); gd_named_destroy(query);
    memset(&request,0,sizeof(request)); request.kind=GD_NAMED_CONSISTENCY; request.projection="rules"; request.work_limit=-1;
    comparison=gd_rule_analysis_workspace(allocator,project->calls); TEST_ASSERT(comparison); gd_rule_compare(comparison);
    query=gd_named_build(diagnostic,project,&request); TEST_ASSERT(query && query->analysis->comparison_count==comparison->comparison_count && query->analysis->pairs_unexamined==comparison->pairs_unexamined && !memcmp(query->analysis->comparisons,comparison->comparisons,(size_t)comparison->comparison_count*sizeof(*comparison->comparisons)));
    gd_named_destroy(query); gd_rule_analysis_destroy(comparison);
    request.work_limit=0; TEST_ASSERT(!gd_named_build(diagnostic,project,&request) && diagnostic->error_count && memory.outstanding==baseline); diagnostic->error_count=0;
    TEST_ASSERT(source_hash==named_hash(project->models[GD_EXPLORER_RULE_DATA],sizeof(GD_Model)) && compiled_hash==named_hash(project->calls,sizeof(*project->calls)));
    gd_explorer_destroy(project); gd_destroy(diagnostic); TEST_ASSERT(memory.outstanding==0);
)
TEST(named_resource_delegates_native_graph_and_zero_work,
    StructureMemory memory={0,-1}; GD_Allocator allocator={&memory,structure_allocate,structure_release}; GD_Model *diagnostic=model_create(); GD_ExplorerProject *project; GD_NamedQuery *query; GD_NamedRequest request={0}; char id[GD_MAX_PATH]; int baseline; FILE *out;
    TEST_ASSERT(diagnostic && setup("named-resources")); collections[1].role=GD_AXIS; collections[2].role=GD_RULE;
    TEST_ASSERT(write_record(0,"unit","---\naxes: [actions/Count]\n---\n")); TEST_ASSERT(write_record(1,"Count","---\nvalue_type: integer\nunit: items\n---\n"));
    TEST_ASSERT(write_record(2,"rules","---\nstatus: accepted\n---\n```gd-rule\ncase add:\n    target: A\n    context: ANY\n    A:\n        noun: things/unit\n        requires: ANY\n        set:\n            Count: this + 1[items]\n```\n"));
    project=gd_explorer_load(allocator,collections,3); TEST_ASSERT(project && !gd_explorer_error(project) && project->resources->node_count>0); baseline=memory.outstanding;
    request.kind=GD_NAMED_RESOURCE_DECLARATIONS; request.projection="resources"; request.work_limit=-1;
    query=gd_named_build(diagnostic,project,&request); TEST_ASSERT(query && !query->routes && query->owned_bytes==sizeof(*query)); out=tmpfile(); TEST_ASSERT(out && gd_write_named_result(out,query,1,"nodes,relations,routes")); fclose(out); gd_named_destroy(query);
    TEST_ASSERT(gd_resource_node_id(project->resources,0,-1,id,sizeof(id))); request.kind=GD_NAMED_RESOURCE_ROUTES; request.to=id; request.work_limit=0;
    query=gd_named_build(diagnostic,project,&request); TEST_ASSERT(query && query->routes->queried && query->routes->limited && !query->routes->witness_count && query->routes->to==0); gd_named_destroy(query);
    request.to="missing/resource"; TEST_ASSERT(!gd_named_build(diagnostic,project,&request) && diagnostic->error_count && memory.outstanding==baseline); diagnostic->error_count=0;
    gd_explorer_destroy(project); gd_destroy(diagnostic); TEST_ASSERT(memory.outstanding==0);
)
TEST(named_skill_delegate_explicit_knowledge_not_learning,
    StructureMemory memory={0,-1}; GD_Allocator allocator={&memory,structure_allocate,structure_release}; GD_Model *diagnostic=model_create(); GD_ExplorerProject *project; GD_NamedQuery *query; GD_NamedRequest request={0}; GD_SkillUnlocks expected; unsigned char known[GD_MAX_RECORDS]={0}; int root_record,prerequisite; FILE *out;
    TEST_ASSERT(diagnostic && setup("named-skills")); collections[0].role=GD_SKILL;
    TEST_ASSERT(write_record(0,"pre","---\nprerequisites: []\n---\n")); TEST_ASSERT(write_record(0,"root","---\nprerequisites: [things/pre]\n---\n")); TEST_ASSERT(write_record(0,"child","---\nprerequisites: [things/root, things/pre]\n---\n"));
    project=gd_explorer_load(allocator,collections,1); TEST_ASSERT(project && !gd_explorer_error(project));
    request.kind=GD_NAMED_SKILLS; request.projection="skills"; request.work_limit=-1; request.from="things/root"; request.known[0]="things/pre"; request.known_count=1;
    query=gd_named_build(diagnostic,project,&request); TEST_ASSERT(query && !diagnostic->error_count); root_record=gd_query_record(query->model,"things/root",GD_SKILL); prerequisite=gd_query_record(query->model,"things/pre",GD_SKILL); TEST_ASSERT(root_record>=0 && prerequisite>=0); known[prerequisite]=1;
    TEST_ASSERT(gd_skill_unlocks(query->model,known,root_record,&expected) && !memcmp(&expected,query->skills,sizeof(expected)) && !query->skills->known[root_record] && query->skills->eligibility[root_record]==GD_SKILL_PREREQUISITES_SUPPLIED);
    TEST_ASSERT(project->draft && gd_draft_prerequisite(project->draft,root_record,"things/pre",0)); gd_explorer_edited(project);
    out=tmpfile(); TEST_ASSERT(out && !gd_write_named_result(out,query,1,NULL) && ftell(out)==0); fclose(out); gd_named_destroy(query);
    query=gd_named_build(diagnostic,project,&request); TEST_ASSERT(query && query->model==gd_explorer_model(project,GD_EXPLORER_SKILLS) && query->skills->eligibility[root_record]==GD_SKILL_NO_PREREQUISITES);
    gd_named_destroy(query); gd_explorer_destroy(project); gd_destroy(diagnostic); TEST_ASSERT(memory.outstanding==0);
)
