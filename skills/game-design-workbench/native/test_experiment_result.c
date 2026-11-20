/* Result-reader engineering checks. No model linkage, execution or outcomes. */
#include "gd_experiment_result.h"
#include <stdlib.h>
#include <string.h>
#define CHECK(test) do { if (!(test)) { fprintf(stderr,"Result helper failure line %d: %s\n",__LINE__,#test); return 1; } ++checks; } while (0)
typedef struct { int limit,calls,releases; } AllocationCheck;
static void *allocate(void *context,size_t size) {
    AllocationCheck *check=context;
    if (check && check->calls++>=check->limit) return NULL;
    return malloc(size);
}
static void release(void *context,void *memory) {
    AllocationCheck *check=context;
    if (check) ++check->releases;
    free(memory);
}
int main(int argc,char **argv) {
    GD_Allocator allocator={NULL,allocate,release};
    GD_ExperimentInput *input=gd_experiment_input_create(allocator);
    GD_ResultContract contract;
    GD_ResultReport report;
    GD_ResultProvenance provenance={"0123456789012345678901234567890123456789","engineering only","engineering only"};
    FILE *stream;
    int checks=0,i,count;
    CHECK(argc==2 && input!=NULL);
    stream=fopen(argv[1],"rb"); CHECK(stream!=NULL);
    CHECK(gd_experiment_read(input,stream)); fclose(stream);
    CHECK(gd_result_contract(&contract,input,&report));
    stream=tmpfile(); CHECK(stream!=NULL);
    for (i=0;i<3;++i) {
        AllocationCheck state={i,0,0};
        GD_Allocator limited={&state,allocate,release};
        CHECK(!gd_result_validate(stream,&contract,&provenance,0,0,limited,&report));
        CHECK(!report.complete && !report.protocol_valid && report.has_exit_code);
        CHECK(state.releases==i);
        CHECK(!strcmp(report.code,i<2 ? "results.memory" : "results.completion"));
        rewind(stream);
    }
    fclose(stream);
    count=input->count; input->count=GD_INPUT_NODES+1;
    CHECK(!gd_result_contract(&contract,input,&report) && !strcmp(report.code,"results.input"));
    input->count=count;
    CHECK(gd_result_contract(&contract,input,&report));
    CHECK(gd_result_commit(provenance.checkpoint) && !gd_result_commit("not-a-commit"));
    gd_experiment_input_destroy(input);
    printf("{\"engineering_checks\":%d,\"experiment_trials_executed\":0}\n",checks); return 0;
}
