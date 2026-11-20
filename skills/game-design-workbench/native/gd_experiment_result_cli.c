#include "gd_experiment_result.h"
#include <limits.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

static void *allocate(void *context,size_t size) { (void)context; return malloc(size); }
static void release(void *context,void *memory) { (void)context; free(memory); }
int main(int argc,char **argv) {
    GD_Allocator allocator={NULL,allocate,release};
    GD_ExperimentInput *input=NULL;
    GD_ResultContract contract;
    GD_ResultProvenance provenance={NULL,NULL,NULL};
    GD_ResultReport report;
    const char *path=NULL,*results=NULL,*exit_text=NULL;
    int i,check=0,stopped=0,code=2;
    FILE *stream=NULL;
    memset(&report,0,sizeof(report)); report.status="invalid";
    report.code="results.arguments"; report.message="Use --help for required explicit inputs";
    for (i=1;i<argc;++i) {
        const char **target=NULL;
        if (!strcmp(argv[i],"--help") && argc==2) {
            puts("Usage: gd-experiment-result --check-input FROZEN.json\n   or: gd-experiment-result --input FROZEN.json --results CAPTURE.ndjson --exit-code N\n       --checkpoint COMMIT --build COMMAND --toolchain VERSION [--stopped]\nRead-only JSON output. Exit 0 complete/valid input, 3 valid producer limit, 2 invalid/error/incomplete.\nNo model execution, criteria evaluation, Git changes or source writes. One configuration per stream."); return 0;
        }
        if (!strcmp(argv[i],"--stopped")) { if (stopped) goto done; stopped=1; continue; }
        if (!strcmp(argv[i],"--check-input")) { if (check) goto done; check=1; target=&path; }
        else if (!strcmp(argv[i],"--input")) target=&path;
        else if (!strcmp(argv[i],"--results")) target=&results;
        else if (!strcmp(argv[i],"--exit-code")) target=&exit_text;
        else if (!strcmp(argv[i],"--checkpoint")) target=&provenance.checkpoint;
        else if (!strcmp(argv[i],"--build")) target=&provenance.build;
        else if (!strcmp(argv[i],"--toolchain")) target=&provenance.toolchain;
        if (!target || *target || i+1==argc) goto done;
        *target=argv[++i];
    }
    if (!path || (check && (results || exit_text || stopped || provenance.checkpoint || provenance.build || provenance.toolchain)) ||
        (!check && (!results || !exit_text || !provenance.checkpoint || !provenance.build || !provenance.toolchain))) goto done;
    input=gd_experiment_input_create(allocator);
    if (!input) { report.code="results.memory"; report.message="Cannot allocate input reader"; goto done; }
    stream=fopen(path,"rb");
    if (!stream || !gd_experiment_read(input,stream)) { report.code="results.input"; report.message="Cannot read strict bounded frozen JSON input"; goto done; }
    fclose(stream); stream=NULL;
    if (!gd_result_contract(&contract,input,&report)) goto done;
    if (check) {
        printf("{\"schema_version\":1,\"ok\":true,\"input_valid\":true,\"trials\":%lld,\"measure_count\":%d,\"node_count\":%d,\"wall_seconds\":%.17g,\"output_bytes\":%llu}\n",
            (long long)contract.trials,contract.measure_count,input->count,contract.wall_seconds,(unsigned long long)contract.max_bytes);
        code=0; goto cleanup;
    }
    {
        char *end; long value;
        errno=0; value=strtol(exit_text,&end,10);
        if (!*exit_text || *end || errno==ERANGE || value<INT_MIN || value>INT_MAX) { report.code="results.arguments"; report.message="exit-code must fit a signed 32-bit process code"; goto done; }
        stream=fopen(results,"rb");
        if (!stream) { report.code="results.io"; report.message="Cannot open captured NDJSON"; goto done; }
        gd_result_validate(stream,&contract,&provenance,(int)value,stopped,allocator,&report);
        code=report.complete ? 0 : report.protocol_valid && !strcmp(report.status,"limit") ? 3 : 2;
    }
done:
    gd_result_write(stdout,&report);
    if (report.code) fprintf(stderr,"gd-experiment-result: %s: %s\n",report.code,report.message);
cleanup:
    if (stream) fclose(stream);
    gd_experiment_input_destroy(input);
    if (fflush(stdout)!=0 || ferror(stdout)) { fputs("gd-experiment-result: cannot deliver JSON summary on stdout\n",stderr); return 2; }
    return code;
}
