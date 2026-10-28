/* Focused unified-loader checks. Larger than the 64 MiB single-model arena:
   measured allocator tracks simultaneous typed snapshots and every failure path. */
#include "gd_explorer.h"
#include <stdlib.h>
typedef union { long double alignment; void *pointer; size_t size; } ExplorerAllocation;
typedef struct { size_t live, peak; int calls, allocations, fail_at; } ExplorerAllocator;
static void *explorer_test_allocate(void *context, size_t size) {
    ExplorerAllocator *state=context; ExplorerAllocation *memory;
    if (++state->calls==state->fail_at) return NULL;
    memory=malloc(sizeof(*memory)+size); if (!memory) return NULL;
    memory->size=size; state->live+=size; ++state->allocations;
    if (state->live>state->peak) state->peak=state->live;
    return memory+1;
}
static void explorer_test_release(void *context, void *pointer) {
    ExplorerAllocator *state=context; ExplorerAllocation *memory;
    if (!pointer) return;
    memory=(ExplorerAllocation *)pointer-1; state->live-=memory->size; --state->allocations; free(memory);
}
static char explorer_directories[12][GD_MAX_PATH];
static GD_Collection explorer_collections[12];
static int explorer_write(int folder, const char *name, const char *text) {
    char path[GD_MAX_PATH];
    return snprintf(path,sizeof(path),"%s/%s.md",explorer_directories[folder],name)<(int)sizeof(path) &&
        file_write_bytes(cstring_from_raw(path),text,strlen(text))==file_error_ok;
}
static int explorer_fixture(void) {
    const char *names[]={"skill-atoms","challenges","nouns","verbs","interactions","moves","systems","dynamics","aesthetics","axes","resources","control-vars"};
    const GD_Role roles[]={GD_SKILL,GD_CHALLENGE,GD_NOUN,GD_VERB,GD_CASE,GD_MDA_MECHANIC,GD_MDA_SYSTEM,GD_MDA_DYNAMIC,GD_MDA_AESTHETIC,GD_AXIS,GD_RESOURCE,GD_CONTROL};
    int i; char directory[GD_MAX_PATH];
    if (snprintf(directory,sizeof(directory),"%s/explorer",root)>=(int)sizeof(directory) || !file_directory_create(directory,1)) return 0;
    for (i=0;i<12;++i) {
        if (snprintf(explorer_directories[i],GD_MAX_PATH,"%s/explorer/%s",root,names[i])>=GD_MAX_PATH || !file_directory_create(explorer_directories[i],1)) return 0;
        explorer_collections[i]=(GD_Collection){names[i],explorer_directories[i],roles[i]};
    }
    return explorer_write(0,"root","---\nrank: 0\nmechanics: [moves/action]\n---\nRoot.") &&
        explorer_write(0,"leaf","---\nrank: 1\nprerequisites: [skill-atoms/root]\n---\nLeaf.") &&
        explorer_write(1,"start","---\nentry: true\nintroduces: [skill-atoms/root]\nnext: [challenges/end]\n---\nStart.") &&
        explorer_write(1,"end","---\nrequires: [skill-atoms/leaf]\n---\nEnd.") &&
        explorer_write(2,"player","---\naxes: [axes/Life]\n---\nPlayer.") &&
        explorer_write(3,"act","---\nstatus: accepted\n---\nAct.") &&
        explorer_write(4,"contact","---\nnouns: [nouns/player]\nverb: verbs/act\ncoverage: defined\nstatus: accepted\n---\nContact.") &&
        explorer_write(5,"action","---\nnouns: [nouns/player]\nverbs: [verbs/act]\n---\n```gd-rule\ncase hit:\n    context: ANY\n    Player:\n        noun: nouns/player\n        requires: alive\n        becomes: dead\n        preserve: OTHER\n```") &&
        explorer_write(6,"loop","---\nmechanics: [moves/action]\n---\nLoop.") &&
        explorer_write(7,"play","---\nmechanics: [moves/action]\nsystems: [systems/loop]\n---\nPlay.") &&
        explorer_write(8,"feel","---\ndynamics: [dynamics/play]\n---\nFeel.") &&
        explorer_write(9,"Life","---\nvalue_type: enum\nallowed_values: [alive, dead]\nclosed: true\n---\nLife.");
}
TEST(explorer_loading,
    ExplorerAllocator memory={0}; GD_Allocator allocator={&memory,explorer_test_allocate,explorer_test_release};
    GD_ExplorerProject *project,*fresh; GD_Model *skills,*mda; int i,record,changed=0;
    TEST_ASSERT(explorer_fixture());
    project=gd_explorer_load(allocator,explorer_collections,12);
    TEST_ASSERT(project && !gd_explorer_error(project) && project->available==63 && project->draft && project->routes && project->mda && project->resources);
    skills=gd_explorer_model(project,GD_EXPLORER_SKILLS); mda=gd_explorer_model(project,GD_EXPLORER_MDA);
    TEST_ASSERT(skills==gd_explorer_model(project,GD_EXPLORER_CHALLENGES) && skills==gd_explorer_model(project,GD_EXPLORER_RELATIONSHIPS));
    record=gd_explorer_record(mda,"moves/action"); TEST_ASSERT(record>=0 && mda->records[record].role==GD_MDA_MECHANIC);
    record=gd_explorer_record(gd_explorer_model(project,GD_EXPLORER_RESOURCES),"moves/action");
    TEST_ASSERT(record>=0 && project->models[GD_EXPLORER_RULE_DATA]->records[record].role==GD_RULE);
    record=gd_explorer_record(skills,"skill-atoms/root"); TEST_ASSERT(record>=0 && gd_draft_rank(project->draft,record,"7")>0);
    TEST_ASSERT(gd_explorer_edited(project) && project->revision>2 && project->draft->modified_count==1);
    TEST_ASSERT(project->models[GD_EXPLORER_SKILL_DATA]->records[gd_explorer_record(project->models[GD_EXPLORER_SKILL_DATA],"skill-atoms/root")].rank==0);
    TEST_ASSERT(gd_draft_undo(project->draft) && gd_explorer_edited(project) && !project->draft->modified_count);
    TEST_ASSERT(explorer_write(5,"a-new","```gd-rule\ncase extra:\n    context: ANY\n    Player:\n        noun: nouns/player\n        requires: ANY\n        preserve: ALL\n```"));
    fresh=gd_explorer_load(allocator,explorer_collections,12); TEST_ASSERT(fresh && !gd_explorer_error(fresh));
    for (i=0;i<project->resources->node_count;++i) {
        int target=gd_explorer_resource_remap(project->resources,fresh->resources,i);
        TEST_ASSERT(target>=0 && gd_explorer_resource_record(fresh->resources,target)>=0);
        if (target!=i) changed=1;
    }
    TEST_ASSERT(changed && gd_sources_changed(project->models[0]));
    TEST_ASSERT(gd_explorer_resource_remap(project->resources,fresh->resources,-1)==-1 && gd_explorer_resource_remap(project->resources,fresh->resources,project->resources->node_count)==-1);
    TEST_ASSERT(gd_explorer_record(mda,"not-loaded/missing")==-1);
    gd_explorer_destroy(fresh); gd_explorer_destroy(project); TEST_ASSERT(!memory.live && !memory.allocations);
    fprintf(stderr,"Explorer typed snapshot peak: %zu bytes (two projects, reload overlap).\n",memory.peak);
)
TEST(explorer_allocation_failures,
    ExplorerAllocator memory={0}; GD_Allocator allocator={&memory,explorer_test_allocate,explorer_test_release};
    GD_ExplorerProject *project; int i,calls;
    project=gd_explorer_load(allocator,explorer_collections,12); TEST_ASSERT(project && !gd_explorer_error(project));
    calls=memory.calls; gd_explorer_destroy(project); TEST_ASSERT(!memory.live && !memory.allocations);
    for (i=1;i<=calls;++i) {
        memory=(ExplorerAllocator){0}; memory.fail_at=i;
        project=gd_explorer_load(allocator,explorer_collections,12);
        gd_explorer_destroy(project); TEST_ASSERT(!memory.live && !memory.allocations);
    }
    memory=(ExplorerAllocator){0};
    project=gd_explorer_load(allocator,explorer_collections,1);
    TEST_ASSERT(project && !gd_explorer_error(project) && project->available==((1u<<GD_EXPLORER_SKILLS)|(1u<<GD_EXPLORER_RELATIONSHIPS)));
    gd_explorer_destroy(project); TEST_ASSERT(!memory.live && !memory.allocations);
    TEST_ASSERT(!gd_explorer_load(allocator,explorer_collections,0) && !gd_explorer_load(allocator,explorer_collections,GD_MAX_COLLECTIONS+1));
)
