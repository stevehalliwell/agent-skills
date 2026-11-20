#include "gd_named_query.h"
#include "test_structure_memory.h"
TEST(named_empty_readers_and_unavailable_selectors,
    StructureMemory memory={0,-1}; GD_Allocator allocator={&memory,structure_allocate,structure_release}; GD_Model *diagnostic=model_create(); GD_ExplorerProject *project; GD_NamedQuery *query; GD_NamedRequest request={0};
    TEST_ASSERT(diagnostic && setup("named-empty-skills")); collections[0].role=GD_SKILL;
    project=gd_explorer_load(allocator,collections,1); TEST_ASSERT(project && !gd_explorer_error(project));
    request.kind=GD_NAMED_SKILLS; request.projection="skills"; request.work_limit=-1;
    query=gd_named_build(diagnostic,project,&request); TEST_ASSERT(query && !query->model->record_count && !query->skills->limited && !query->skills->comparison_count); gd_named_destroy(query);
    gd_explorer_destroy(project); TEST_ASSERT(memory.outstanding==0);
    TEST_ASSERT(setup("named-empty-rules")); collections[1].role=GD_AXIS; collections[2].role=GD_RULE;
    project=gd_explorer_load(allocator,collections,3); TEST_ASSERT(project && !gd_explorer_error(project) && project->rules && project->calls && project->resources && !project->resources->node_count);
    request.kind=GD_NAMED_CONSISTENCY; request.projection="rules";
    query=gd_named_build(diagnostic,project,&request); TEST_ASSERT(query && !query->analysis->comparison_count && !query->analysis->pairs_unexamined && !query->analysis->comparison_limited); gd_named_destroy(query);
    request.kind=GD_NAMED_RESOURCE_DECLARATIONS; request.projection="resources";
    query=gd_named_build(diagnostic,project,&request); TEST_ASSERT(query && !query->routes); gd_named_destroy(query);
    request.kind=GD_NAMED_RESOURCE_ROUTES; request.to="missing/target";
    TEST_ASSERT(!gd_named_build(diagnostic,project,&request) && diagnostic->error_count); diagnostic->error_count=0;
    request.kind=GD_NAMED_OUTCOMES; request.projection="rules"; request.to=NULL; request.case_name="cases/missing#absent";
    TEST_ASSERT(!gd_named_build(diagnostic,project,&request) && diagnostic->error_count); diagnostic->error_count=0;
    gd_explorer_destroy(project); gd_destroy(diagnostic); TEST_ASSERT(memory.outstanding==0);
)
