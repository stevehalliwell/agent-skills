/* Prescribed legal-route timing, not a puzzle solver or player model.
   Reads only --input. Writes only stdout/stderr. No shell/network/source writes.
   Compile/check helpers before reviewed checkpoint; do not run trials yet. */
#include "gd_experiment.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

static void *allocate(void *context, size_t size) { (void)context; return malloc(size); }
static void release(void *context, void *memory) { (void)context; free(memory); }
static int member(const GD_ExperimentInput *input, int object, const char *key) { return gd_input_member(input,object,key); }
static int equals(const GD_ExperimentInput *input, int object, const char *key, const char *expected) {
    const char *text=gd_input_string(input,member(input,object,key));
    return text && !strcmp(text,expected);
}
static int number(const GD_ExperimentInput *input, int object, const char *key, double *value) {
    return gd_input_number(input,member(input,object,key),value);
}
static int integer(const GD_ExperimentInput *input, int object, const char *key, int64_t *value) {
    return gd_input_integer(input,member(input,object,key),value);
}
static int boolean_true(const GD_ExperimentInput *input, int object, const char *key) {
    int node=member(input,object,key);
    return node>=0 && input->nodes[node].kind==GD_INPUT_BOOLEAN && !strcmp(input->nodes[node].text,"true");
}
static int control(const GD_ExperimentInput *input, int controls, const char *key,
                   const char *type, const char *unit, double *value) {
    int node=member(input,controls,key);
    int64_t whole;
    return node>=0 && equals(input,node,"type",type) && equals(input,node,"unit",unit) &&
        boolean_true(input,node,"resolved") && number(input,node,"value",value) &&
        (strcmp(type,"integer") || gd_input_integer(input,member(input,node,"value"),&whole));
}
static int commit_id(const char *text) {
    size_t i,n=strlen(text);
    if (n!=40 && n!=64) return 0;
    for (i=0;i<n;++i) if (!strchr("0123456789abcdef",text[i])) return 0;
    return 1;
}
static int error(const char *message) {
    char quoted[2048];
    if (!gd_event_quote(quoted,sizeof(quoted),message)) strcpy(quoted,"\"Input/runtime error\"");
    fprintf(stderr,"route-timing: %s\n",message);
    printf("{\"type\":\"error\",\"protocol\":\"%s\",\"code\":\"route.input\",\"message\":%s}\n",GD_EXPERIMENT_PROTOCOL,quoted);
    return 2;
}
int main(int argc, char **argv) {
    GD_Allocator allocator={NULL,allocate,release};
    GD_ExperimentInput *input=NULL;
    GD_TrialBudget budget;
    FILE *stream=NULL;
    const char *path=NULL,*checkpoint=NULL,*build=NULL,*toolchain=NULL;
    char build_json[8192],toolchain_json[8192],line[4096];
    char *header=NULL;
    int i,controls,assumptions,limits,measures,sequence,trace_node,trace=-1,result=2;
    int64_t trials,steps,max_bytes,trace_trial,version;
    uint64_t seed,trial=0,completed=0,attempted=0;
    int actions_completed=0;
    int64_t energy=0;
    double elapsed=0;
    double walk,push,initial,capacity,cost,tile,speed,timeout,delay_min,delay_max;
    memset(&budget,0,sizeof(budget));
#ifdef _WIN32
    /* Count actual stdout bytes, not text-mode CRLF expansion. */
    if (_setmode(_fileno(stdout),_O_BINARY)==-1) return error("Cannot set binary stdout for exact byte accounting");
#endif
    for (i=1;i<argc;++i) {
        const char **target=NULL;
        if (!strcmp(argv[i],"--help") && argc==2) {
            puts("Usage: route-timing --input FROZEN.json --checkpoint COMMIT --build COMMAND --toolchain VERSION\nEmits gd-experiment-ndjson-v1; no prompts. Exit 0 complete, 2 input/error, 3 limit.\nExecution requires reviewed committed inputs/code and explicit approval. No current-design reads."); return 0;
        }
        if (!strcmp(argv[i],"--input")) target=&path;
        else if (!strcmp(argv[i],"--checkpoint")) target=&checkpoint;
        else if (!strcmp(argv[i],"--build")) target=&build;
        else if (!strcmp(argv[i],"--toolchain")) target=&toolchain;
        if (!target || *target || i+1==argc) return error("Unknown, duplicate or missing argument; use --help");
        *target=argv[++i];
    }
    if (!path || !checkpoint || !build || !*build || !toolchain || !*toolchain || !commit_id(checkpoint) ||
        strlen(build)>1024 || strlen(toolchain)>1024 || !gd_event_quote(build_json,sizeof(build_json),build) ||
        !gd_event_quote(toolchain_json,sizeof(toolchain_json),toolchain)) return error("Supply input, actual commit, build command and toolchain (metadata strings at most 1024 bytes)");
    input=gd_experiment_input_create(allocator);
    if (!input) return error("Cannot allocate frozen-input storage");
    stream=fopen(path,"rb");
    if (!stream) { error("Cannot open frozen input"); goto done; }
    i=gd_experiment_read(input,stream); fclose(stream); stream=NULL;
    if (!i) { error(input->error); goto done; }
    controls=member(input,0,"controls"); assumptions=member(input,0,"assumptions");
    limits=member(input,0,"limits"); measures=member(input,0,"measures");
    if (!integer(input,0,"schema_version",&version) || version!=1 ||
        !equals(input,0,"model_id","crate-route-timing-v1") ||
        !equals(input,0,"recipe_id","crate-route-cost-baseline-v1") ||
        !equals(input,0,"configuration_id","baseline-speed-2") ||
        !equals(input,member(input,0,"experiment"),"reference","experiments/route-cost") ||
        !equals(input,member(input,0,"experiment"),"id","01a15175-3c00-7e68-8f3b-b760984a9a2d") ||
        !equals(input,0,"random_generator",GD_EXPERIMENT_RANDOM) ||
        !gd_input_seed(gd_input_string(input,member(input,0,"seed")),&seed) ||
        !integer(input,0,"trials",&trials) || trials<1 || trials>1000000 ||
        !integer(input,limits,"steps_per_trial",&steps) || steps<1 ||
        !integer(input,limits,"output_bytes",&max_bytes) || max_bytes<4096 || max_bytes>1073741824 ||
        !number(input,limits,"wall_seconds",&timeout) || timeout<=0 ||
        !control(input,controls,"walk-duration","number","s",&walk) || walk<=0 ||
        !control(input,controls,"push-duration","number","s",&push) || push<=0 ||
        !control(input,controls,"initial-energy","integer","energy",&initial) ||
        !control(input,controls,"energy-capacity","integer","energy",&capacity) ||
        !control(input,controls,"push-cost","integer","energy",&cost) ||
        !control(input,controls,"tile-distance","number","m",&tile) || tile<=0 ||
        !control(input,controls,"walk-speed","number","m/s",&speed) || speed!=2 ||
        initial<0 || capacity<initial || cost<0 || cost>initial/3 ||
        !isfinite(4*walk+3*push+7)) { error("Unsupported/missing typed controls, identity, seed, trial count or limits; prepare inputs through resolver"); goto done; }
    if (!equals(input,assumptions,"clock_start","before-first-delay") ||
        !equals(input,assumptions,"clock_stop","after-final-push") ||
        !equals(input,assumptions,"delay_distribution","uniform-independent") ||
        !number(input,assumptions,"delay_min_seconds",&delay_min) || delay_min!=0 ||
        !number(input,assumptions,"delay_max_seconds",&delay_max) || delay_max!=1 ||
        !boolean_true(input,assumptions,"route_legal") || !boolean_true(input,assumptions,"decisions_correct") ||
        !equals(input,assumptions,"energy_policy","require-sufficient-energy-successful-pushes-only")) {
        error("Unsupported route, clock, delay or energy assumptions; insufficient-energy behavior is not modeled"); goto done;
    }
    sequence=member(input,assumptions,"action_sequence");
    if (sequence<0 || input->nodes[sequence].kind!=GD_INPUT_ARRAY) { error("Missing prescribed action sequence"); goto done; }
    {
        int count=0;
        for (i=sequence+1;i<input->count;++i) if (input->nodes[i].parent==sequence) {
            const char *action=gd_input_string(input,i);
            if (count>=7 || !action || strcmp(action,count<4 ? "walk" : "push")) { error("Only four walks then three successful pushes are modeled"); goto done; }
            ++count;
        }
        if (count!=7) { error("Expected seven prescribed actions"); goto done; }
    }
    {
        const char *names[]={"elapsed-time","completion-time","route-complete","remaining-energy"};
        const char *types[]={"number","number","boolean","integer"};
        const char *units[]={"s","s","","energy"};
        for (i=0;i<4;++i) {
            int measure=member(input,measures,names[i]);
            if (!equals(input,measure,"value_type",types[i]) || !equals(input,measure,"unit",units[i])) {
                error("Missing or mismatched local measure type/unit"); goto done;
            }
        }
    }
    trace_node=member(input,0,"trace_trial");
    if (trace_node<0 || (input->nodes[trace_node].kind!=GD_INPUT_NULL &&
        (!gd_input_integer(input,trace_node,&trace_trial) || trace_trial<0 || trace_trial>=trials))) {
        error("trace_trial must be null or one valid zero-based trial index"); goto done;
    }
    if (input->nodes[trace_node].kind!=GD_INPUT_NULL) trace=(int)trace_trial;
    budget.output=stdout; budget.max_steps=(uint64_t)steps; budget.max_bytes=(size_t)max_bytes;
    budget.timeout=timeout; budget.started=gd_experiment_clock();
    header=allocator.allocate(NULL,input->raw_size+32768);
    if (!header) { error("Cannot allocate event buffer"); goto done; }
    i=snprintf(header,input->raw_size+32768,
        "{\"type\":\"header\",\"protocol\":\"%s\",\"model\":\"crate-route-timing-v1\",\"helper\":\"%s\",\"checkpoint_commit\":\"%s\",\"build\":%s,\"toolchain\":%s,\"dependencies\":{\"libyaml\":\"" GD_EXPERIMENT_YAML_PIN "\",\"pcg-c-basic\":\"" GD_EXPERIMENT_PCG_PIN "\"},\"input\":%s}",
        GD_EXPERIMENT_PROTOCOL,GD_EXPERIMENT_VERSION,checkpoint,build_json,toolchain_json,input->raw);
    if (i<0 || (size_t)i>=input->raw_size+32768) { budget.status="error"; goto finish; }
    if (!gd_trial_event(&budget,header,(size_t)i,0)) goto finish;
    for (trial=0;trial<(uint64_t)trials;++trial) {
        pcg32_random_t random;
        char time_text[64];
        elapsed=0; energy=(int64_t)initial; actions_completed=0; ++attempted;
        budget.steps=0; gd_trial_random(&random,seed,trial);
        for (i=0;i<7;++i) {
            double delay;
            if (!gd_trial_step(&budget)) goto finish;
            delay=gd_trial_uniform(&random);
            elapsed+=delay+(i<4 ? walk : push);
            if (i>=4) energy-=(int64_t)cost;
            actions_completed=i+1;
            if (trace==(int)trial) {
                char delay_text[64]; int size;
                if (!gd_measure_number(time_text,sizeof(time_text),elapsed) || !gd_measure_number(delay_text,sizeof(delay_text),delay)) { budget.status="error"; goto finish; }
                size=snprintf(line,sizeof(line),"{\"type\":\"step\",\"configuration_id\":\"baseline-speed-2\",\"trial\":%llu,\"step\":%d,\"action\":\"%s\",\"delay_seconds\":%s,\"state\":{\"actions_completed\":%d,\"remaining_energy\":%lld},\"measurements\":{\"elapsed-time\":%s}}",
                    (unsigned long long)trial,i+1,i<4 ? "walk" : "push",delay_text,i+1,(long long)energy,time_text);
                if (size<0 || (size_t)size>=sizeof(line)) { budget.status="error"; goto finish; }
                if (!gd_trial_event(&budget,line,(size_t)size,0)) goto finish;
            }
        }
        if (!gd_measure_number(time_text,sizeof(time_text),elapsed)) { budget.status="error"; goto finish; }
        i=snprintf(line,sizeof(line),"{\"type\":\"trial\",\"configuration_id\":\"baseline-speed-2\",\"trial\":%llu,\"status\":\"complete\",\"steps\":7,\"measurements\":{\"elapsed-time\":%s,\"completion-time\":%s,\"route-complete\":true,\"remaining-energy\":%lld}}",
            (unsigned long long)trial,time_text,time_text,(long long)energy);
        if (i<0 || (size_t)i>=sizeof(line)) { budget.status="error"; goto finish; }
        if (!gd_trial_event(&budget,line,(size_t)i,0)) goto finish;
        ++completed;
    }
finish:
    if (!budget.status) gd_trial_budget_check(&budget);
    if (attempted>completed) {
        char time_text[64];
        if (gd_measure_number(time_text,sizeof(time_text),elapsed)) {
            i=snprintf(line,sizeof(line),"{\"type\":\"trial\",\"configuration_id\":\"baseline-speed-2\",\"trial\":%llu,\"status\":\"%s\",\"steps\":%d,\"measurements\":{\"elapsed-time\":%s,\"completion-time\":%s,\"route-complete\":%s,\"remaining-energy\":%lld},\"undefined\":%s}",
                (unsigned long long)trial,budget.status ? budget.status : "error",actions_completed,time_text,
                actions_completed==7 ? time_text : "null",actions_completed==7 ? "true" : "false",(long long)energy,
                actions_completed==7 ? "{}" : "{\"completion-time\":\"execution-interrupted-before-route-completion\"}");
            if (i<0 || (size_t)i>=sizeof(line) || !gd_trial_event(&budget,line,(size_t)i,1)) budget.status="error";
        } else budget.status="error";
    }
    i=snprintf(line,sizeof(line),"{\"type\":\"completion\",\"status\":\"%s\",\"trials_requested\":%lld,\"trials_attempted\":%llu,\"trials_completed\":%llu,\"partial_evidence\":%s}",
        budget.status ? budget.status : "complete",(long long)trials,(unsigned long long)attempted,(unsigned long long)completed,budget.status ? "true" : "false");
    if (i<0 || (size_t)i>=sizeof(line) || !gd_trial_event(&budget,line,(size_t)i,1)) budget.status="error";
    result=!budget.status ? 0 : !strcmp(budget.status,"limit") ? 3 : 2;
    if (result) fprintf(stderr,"route-timing: incomplete %s; retain available output, never infer criterion satisfaction\n",budget.status);
done:
    if (stream) fclose(stream);
    if (header) allocator.release(NULL,header);
    gd_experiment_input_destroy(input);
    return result;
}
