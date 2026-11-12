/* App-local checks, not OS input automation. Run native/test-explorer.ps1.
   Only private explorer-checks-* copies may be used for reload-failure probes.
   --ipc-helpers GAME-DESIGN-DIRECTORY [--deep-checks] [--layout-research] runs source-read-only CPU checks.
   Extended presentation/refresh and comparative research require explicit flags;
   research implies deep checks. Private preferences fixture, no window or user-settings writes. */
#include "gd_explorer.h"
#include <stdlib.h>
#include <string.h>
typedef union { long double alignment; void *pointer; size_t size; } Allocation;
typedef struct { int calls,fail_at,allocations; size_t live,peak; } Memory;
static void *allocate(void *context, size_t size) {
    Memory *state=context; Allocation *memory;
    if (++state->calls==state->fail_at) return NULL;
    memory=malloc(sizeof(*memory)+size); if (!memory) return NULL;
    memory->size=size; ++state->allocations; state->live+=size; if (state->live>state->peak) state->peak=state->live;
    return memory+1;
}
static void release(void *context, void *pointer) {
    Memory *state=context; Allocation *memory;
    if (!pointer) return;
    memory=(Allocation *)pointer-1; --state->allocations; state->live-=memory->size; free(memory);
}
int gd_explorer_checks(GD_Allocator allocator, const GD_Collection *collections, int count,
                       const char *output, int *fail_at, int *calls);
int gd_explorer_viewer_checks(GD_Allocator allocator, const GD_Collection *collections, int count, int *fail_at, int *calls,int run_deep_checks,int run_layout_research);
int gd_explorer_find_checks(GD_Allocator allocator,const GD_Collection *collections,int count,int *fail_at,int *calls);
int main(int argc, char **argv) {
    const char *names[]={"skill-atoms","challenges","nouns","verbs","interactions","mechanics","systems","dynamics","aesthetics","axes","resources","control-vars"};
    const GD_Role roles[]={GD_SKILL,GD_CHALLENGE,GD_NOUN,GD_VERB,GD_CASE,GD_MDA_MECHANIC,GD_MDA_SYSTEM,GD_MDA_DYNAMIC,GD_MDA_AESTHETIC,GD_AXIS,GD_RESOURCE,GD_CONTROL};
    GD_Collection collections[12]; char paths[12][GD_MAX_PATH];
    Memory memory={0}; GD_Allocator allocator={&memory,allocate,release}; int i,result;
    int ipc_helpers=argc>=3 && argc<=5 && !strcmp(argv[1],"--ipc-helpers");
    int run_deep_checks=0,run_layout_research=0,find_helpers=argc==3 && !strcmp(argv[1],"--find-helpers");
    if(ipc_helpers)for(i=3;i<argc;++i){
        if(!strcmp(argv[i],"--deep-checks") && !run_deep_checks)run_deep_checks=1;
        else if(!strcmp(argv[i],"--layout-research") && !run_layout_research)run_layout_research=1;
        else{ipc_helpers=0;break;}
    }
    if(run_layout_research)run_deep_checks=1;
    const char *input=ipc_helpers||find_helpers?argv[2]:argc>1?argv[1]:"";
    if (!ipc_helpers && !find_helpers && (argc!=3 || !strstr(argv[1],"explorer-checks-") || !strstr(argv[2],"explorer-checks-"))) {
        fputs("Use --find-helpers GAME-DESIGN-DIRECTORY, --ipc-helpers GAME-DESIGN-DIRECTORY [--deep-checks] [--layout-research], or test-explorer.ps1 with private explorer-checks-* fixture/output paths.\n",stderr); return 2;
    }
    for (i=0;i<12;++i) {
        if (snprintf(paths[i],sizeof(paths[i]),"%s/%s",input,names[i])>=(int)sizeof(paths[i])) return 2;
        collections[i]=(GD_Collection){names[i],paths[i],roles[i]};
    }
    result=find_helpers?gd_explorer_find_checks(allocator,collections,12,&memory.fail_at,&memory.calls):ipc_helpers?gd_explorer_viewer_checks(allocator,collections,12,&memory.fail_at,&memory.calls,run_deep_checks,run_layout_research):gd_explorer_checks(allocator,collections,12,argv[2],&memory.fail_at,&memory.calls);
    if (memory.allocations || memory.live) { fputs("Explorer test leaked owned memory.\n",stderr); result=2; }
    if (!ipc_helpers && !find_helpers) printf("{\"ok\":%s,\"allocations_remaining\":%d,\"peak_bytes\":%zu,\"input\":\"app-local state transitions; no OS input posted\"}\n",result ? "false":"true",memory.allocations,memory.peak);
    return result;
}
