/* Synthetic protocol/fault emitter, NOT a simulation. No route code, control
   formulas, random draws, expectation comparisons or game outcomes. Intentional
   hang/flood/child faults require owned watchdog; do not invoke unsupervised. */
#include "gd_experiment.h"
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif
static void *allocate(void *context,size_t size) { (void)context; return malloc(size); }
static void release(void *context,void *memory) { (void)context; free(memory); }
int main(int argc,char **argv) {
    GD_Allocator allocator={NULL,allocate,release};
    GD_ExperimentInput *input;
    const char *path=NULL,*checkpoint=NULL,*build=NULL,*toolchain=NULL,*mode,*model;
    char build_json[8192],toolchain_json[8192];
    FILE *file;
    int i;
#ifdef _WIN32
    _setmode(_fileno(stdout),_O_BINARY); _setmode(_fileno(stderr),_O_BINARY);
    if (argc==2 && !strcmp(argv[1],"--engineering-child")) { Sleep(60000); return 0; }
#endif
    if (argc!=9) return 20;
    for (i=1;i+1<argc;i+=2) {
        if (!strcmp(argv[i],"--input")) path=argv[i+1];
        else if (!strcmp(argv[i],"--checkpoint")) checkpoint=argv[i+1];
        else if (!strcmp(argv[i],"--build")) build=argv[i+1];
        else if (!strcmp(argv[i],"--toolchain")) toolchain=argv[i+1];
        else return 20;
    }
    if (!path || !checkpoint || !build || !toolchain) return 20;
    input=gd_experiment_input_create(allocator); if (!input) return 21;
    file=fopen(path,"rb"); if (!file) return 22;
    i=gd_experiment_read(input,file); fclose(file); if (!i) return 23;
    model=gd_input_string(input,gd_input_member(input,0,"model_id"));
    mode=gd_input_string(input,gd_input_member(input,0,"engineering_mode"));
    if (!model || strcmp(model,"engineering-fixture-v1") || !mode) return 24; /* Refuse real recipe. */
#ifdef _WIN32
    if (!strcmp(mode,"complete")) {
        char directory[4096];
        if (!GetCurrentDirectoryA(sizeof(directory),directory)) return 30;
        fprintf(stderr,"engineering_cwd=%s\n",directory); fflush(stderr);
    }
    if (!strcmp(mode,"hang")) { fputs("engineering hang\n",stderr); fflush(stderr); Sleep(60000); return 25; }
    if (!strcmp(mode,"child-parent-exit")) {
        STARTUPINFOA startup; PROCESS_INFORMATION process;
        char executable[4096],command[8192];
        memset(&startup,0,sizeof(startup)); memset(&process,0,sizeof(process)); startup.cb=sizeof(startup);
        startup.dwFlags=STARTF_USESTDHANDLES; startup.hStdInput=GetStdHandle(STD_INPUT_HANDLE);
        startup.hStdOutput=GetStdHandle(STD_OUTPUT_HANDLE); startup.hStdError=GetStdHandle(STD_ERROR_HANDLE);
        if (!GetModuleFileNameA(NULL,executable,sizeof(executable))) return 26;
        snprintf(command,sizeof(command),"\"%s\" --engineering-child",executable);
        if (!CreateProcessA(executable,command,NULL,NULL,TRUE,CREATE_NO_WINDOW,NULL,NULL,&startup,&process)) return 27;
        fprintf(stderr,"engineering_child_pid=%lu\n",(unsigned long)process.dwProcessId); fflush(stderr);
        CloseHandle(process.hThread); CloseHandle(process.hProcess); return 0;
    }
#endif
    if (!strcmp(mode,"stdout-flood") || !strcmp(mode,"stderr-flood") || !strcmp(mode,"both-flood")) {
        char bytes[4096]; FILE *target=!strcmp(mode,"stderr-flood") ? stderr : stdout;
        memset(bytes,'x',sizeof(bytes));
        for (i=0;i<65536;++i) {
            if (fwrite(bytes,1,sizeof(bytes),target)!=sizeof(bytes) || fflush(target)!=0) break;
            if (!strcmp(mode,"both-flood") && (fwrite(bytes,1,sizeof(bytes),stderr)!=sizeof(bytes) || fflush(stderr)!=0)) break;
        }
        gd_experiment_input_destroy(input); return 0;
    }
    if (!strcmp(mode,"preheader-error")) {
        puts("{\"type\":\"error\",\"protocol\":\"gd-experiment-ndjson-v1\",\"code\":\"engineering.input\",\"message\":\"Synthetic pre-header error\"}");
        gd_experiment_input_destroy(input); return 2;
    }
    if (!gd_event_quote(build_json,sizeof(build_json),build) || !gd_event_quote(toolchain_json,sizeof(toolchain_json),toolchain)) return 28;
    printf("{\"type\":\"header\",\"protocol\":\"%s\",\"model\":\"engineering-fixture-v1\",\"helper\":\"%s\",\"checkpoint_commit\":\"%s\",\"build\":%s,\"toolchain\":%s,\"dependencies\":{\"libyaml\":\"%s\",\"pcg-c-basic\":\"%s\"},\"input\":%s}\n",
        GD_EXPERIMENT_PROTOCOL,GD_EXPERIMENT_VERSION,checkpoint,build_json,toolchain_json,GD_EXPERIMENT_YAML_PIN,GD_EXPERIMENT_PCG_PIN,input->raw);
    if (!strcmp(mode,"crash")) { fputs("{\"type\":\"trial\"",stdout); fflush(stdout); gd_experiment_input_destroy(input); return 9; }
    if (!strcmp(mode,"binary")) { const char bytes[]="prefix\0suffix\n"; fwrite(bytes,1,sizeof(bytes)-1,stdout); gd_experiment_input_destroy(input); return 0; }
    if (!strcmp(mode,"error")) {
        puts("{\"type\":\"trial\",\"configuration_id\":\"fixture\",\"trial\":0,\"status\":\"error\",\"steps\":0,\"measurements\":{\"number\":null,\"integer\":0,\"boolean\":false,\"enum\":\"a\"},\"undefined\":{\"number\":\"engineering runtime error\"}}");
        puts("{\"type\":\"completion\",\"status\":\"error\",\"trials_requested\":2,\"trials_attempted\":1,\"trials_completed\":0,\"partial_evidence\":true}");
        gd_experiment_input_destroy(input); return 2;
    }
    if (!strcmp(mode,"stdin") && fgetc(stdin)!=EOF) return 29;
    if (!strcmp(mode,"input-lock")) {
        FILE *write_attempt=fopen(path,"r+b");
        if (write_attempt) { fclose(write_attempt); return 31; } /* Never write. */
    }
    if (!strcmp(mode,"trace")) {
        puts("{\"type\":\"step\",\"configuration_id\":\"fixture\",\"trial\":0,\"step\":1,\"state\":{\"synthetic\":true},\"measurements\":{\"number\":1.25}}");
        puts("{\"type\":\"step\",\"configuration_id\":\"fixture\",\"trial\":0,\"step\":2,\"state\":{\"synthetic\":true},\"measurements\":{\"number\":2.5}}");
    }
    puts("{\"type\":\"trial\",\"configuration_id\":\"fixture\",\"trial\":0,\"status\":\"complete\",\"steps\":2,\"measurements\":{\"number\":1.25,\"integer\":2,\"boolean\":true,\"enum\":\"a\"}}");
    if (!strcmp(mode,"limit")) {
        puts("{\"type\":\"trial\",\"configuration_id\":\"fixture\",\"trial\":1,\"status\":\"limit\",\"steps\":1,\"measurements\":{\"number\":null,\"integer\":1,\"boolean\":false,\"enum\":\"b\"},\"undefined\":{\"number\":\"engineering interruption\"}}");
        puts("{\"type\":\"completion\",\"status\":\"limit\",\"trials_requested\":2,\"trials_attempted\":2,\"trials_completed\":1,\"partial_evidence\":true}");
        gd_experiment_input_destroy(input); return 3;
    }
    puts("{\"type\":\"trial\",\"configuration_id\":\"fixture\",\"trial\":1,\"status\":\"complete\",\"steps\":2,\"measurements\":{\"number\":2.5,\"integer\":3,\"boolean\":false,\"enum\":\"b\"}}");
    puts("{\"type\":\"completion\",\"status\":\"complete\",\"trials_requested\":2,\"trials_attempted\":2,\"trials_completed\":2,\"partial_evidence\":false}");
    gd_experiment_input_destroy(input); return 0;
}
