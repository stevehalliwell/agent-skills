#include "gd_experiment_result.h"
#include <math.h>
#include <string.h>

static int fail(GD_ResultReport *report, const char *code, const char *message) {
    report->protocol_valid=0; report->complete=0; report->status="invalid";
    report->code=code; report->message=message; return 0;
}
static int member(const GD_ExperimentInput *input, int parent, const char *key) { return gd_input_member(input,parent,key); }
static const char *string(const GD_ExperimentInput *input, int parent, const char *key) { return gd_input_string(input,member(input,parent,key)); }
static int equal(const char *a, const char *b) { return a && b && !strcmp(a,b); }
static int bounded(const char *text,size_t limit) { return text && *text && strlen(text)<=limit; }
static int object(const GD_ExperimentInput *input,int node) { return node>=0 && input->nodes[node].kind==GD_INPUT_OBJECT; }
static int integer(const GD_ExperimentInput *input,int parent,const char *key,int64_t *value) { return gd_input_integer(input,member(input,parent,key),value); }
static int boolean(const GD_ExperimentInput *input,int parent,const char *key,int *value) {
    int node=member(input,parent,key);
    if (node<0 || input->nodes[node].kind!=GD_INPUT_BOOLEAN) return 0;
    *value=!strcmp(input->nodes[node].text,"true"); return 1;
}
int gd_result_commit(const char *text) {
    size_t i,n=text ? strlen(text) : 0;
    if (n!=40 && n!=64) return 0;
    for (i=0;i<n;++i) if (!strchr("0123456789abcdef",text[i])) return 0;
    return 1;
}
int gd_result_contract(GD_ResultContract *contract,const GD_ExperimentInput *input,GD_ResultReport *report) {
    int i,j,definitions,limits,trace,experiment;
    int64_t version,bytes;
    uint64_t seed;
    memset(contract,0,sizeof(*contract)); memset(report,0,sizeof(*report)); report->status="invalid";
    contract->input=input; contract->trace_trial=-1;
    if (!input || input->error || input->count<1 || input->count>GD_INPUT_NODES || input->nodes[0].kind!=GD_INPUT_OBJECT)
        return fail(report,"results.input","Expected a successfully parsed frozen input within 4096 nodes");
    contract->model=string(input,0,"model_id"); contract->recipe=string(input,0,"recipe_id");
    contract->configuration=string(input,0,"configuration_id");
    limits=member(input,0,"limits"); definitions=member(input,0,"measures"); experiment=member(input,0,"experiment");
    if (!integer(input,0,"schema_version",&version) || version!=1 || !bounded(contract->model,128) ||
        !bounded(contract->recipe,128) || !bounded(contract->configuration,128) ||
        !bounded(string(input,experiment,"id"),128) || !bounded(string(input,experiment,"reference"),128) ||
        !equal(string(input,0,"random_generator"),GD_EXPERIMENT_RANDOM) || !gd_input_seed(string(input,0,"seed"),&seed) ||
        !integer(input,0,"trials",&contract->trials) || contract->trials<1 || contract->trials>GD_RESULT_TRIALS ||
        !integer(input,limits,"steps_per_trial",&contract->max_steps) || contract->max_steps<1 ||
        !integer(input,limits,"output_bytes",&bytes) || bytes<1 || (uint64_t)bytes>GD_RESULT_BYTES ||
        !gd_input_number(input,member(input,limits,"wall_seconds"),&contract->wall_seconds) || contract->wall_seconds<=0 ||
        !object(input,definitions)) return fail(report,"results.input","Invalid frozen identity, seed, limits or measure definitions");
    contract->max_bytes=(size_t)bytes;
    trace=member(input,0,"trace_trial");
    if (trace<0 || (input->nodes[trace].kind!=GD_INPUT_NULL &&
        (!gd_input_integer(input,trace,&contract->trace_trial) || contract->trace_trial<0 || contract->trace_trial>=contract->trials)))
        return fail(report,"results.input","trace_trial must be null or a valid trial index");
    for (i=definitions+1;i<input->count;++i) if (input->nodes[i].parent==definitions) {
        GD_ResultMeasure *measure;
        const char *type=string(input,i,"value_type");
        if (contract->measure_count==GD_RESULT_MEASURES || !bounded(input->nodes[i].key,128) || !object(input,i) ||
            !bounded(string(input,i,"identity"),512) || !bounded(string(input,i,"boundary"),4096) ||
            !string(input,i,"unit") || strlen(string(input,i,"unit"))>128 ||
            (!equal(type,"number") && !equal(type,"integer") && !equal(type,"boolean") && !equal(type,"enum")))
            return fail(report,"results.definitions","At most 64 named measures with unique identities, type, unit and boundary are required");
        for (j=0;j<contract->measure_count;++j) if (equal(contract->measures[j].identity,string(input,i,"identity")))
            return fail(report,"results.definitions","Measure identity is duplicated");
        if ((equal(type,"boolean") || equal(type,"enum")) && *string(input,i,"unit"))
            return fail(report,"results.definitions","Boolean/enum measures must be unitless");
        if (equal(type,"enum")) {
            int choices=member(input,i,"allowed_values"),n=0,k;
            if (choices<0 || input->nodes[choices].kind!=GD_INPUT_ARRAY) return fail(report,"results.definitions","Enum measure needs allowed_values array");
            for (j=choices+1;j<input->count;++j) if (input->nodes[j].parent==choices) {
                if (!gd_input_string(input,j) || ++n>64) return fail(report,"results.definitions","Enum requires 1 to 64 unique string choices");
                for (k=choices+1;k<j;++k) if (input->nodes[k].parent==choices && equal(input->nodes[k].text,input->nodes[j].text))
                    return fail(report,"results.definitions","Enum choice is duplicated");
            }
            if (!n) return fail(report,"results.definitions","Enum requires choices");
        }
        measure=&contract->measures[contract->measure_count++];
        measure->name=input->nodes[i].key; measure->identity=string(input,i,"identity");
        measure->type=type; measure->unit=string(input,i,"unit"); measure->definition=i;
    }
    if (!contract->measure_count) return fail(report,"results.definitions","At least one measure required");
    return 1;
}
/* Iterative tree comparison: object order/JSON string escapes may differ;
   number lexemes and every metadata/value field must match frozen snapshot.
   Mapping follows preorder parents; no recursion or regenerated formulas. */
static int same_input(const GD_ExperimentInput *expected,const GD_ExperimentInput *record,int root) {
    int mapping[GD_INPUT_NODES],i,j;
    if (!object(record,root)) return 0;
    mapping[0]=root;
    for (i=0;i<expected->count;++i) {
        int target=mapping[i],children_a=0,children_b=0;
        const GD_InputNode *a=&expected->nodes[i],*b;
        if (target<0 || target>=record->count) return 0;
        b=&record->nodes[target];
        if (a->kind!=b->kind || ((a->text || b->text) && !equal(a->text,b->text))) return 0;
        for (j=i+1;j<expected->count;++j) if (expected->nodes[j].parent==i) {
            int other=-1,k;
            if (a->kind==GD_INPUT_OBJECT) other=member(record,target,expected->nodes[j].key);
            else {
                int ordinal=0;
                for (k=target+1;k<record->count;++k) if (record->nodes[k].parent==target && ordinal++==children_a) { other=k; break; }
            }
            mapping[j]=other; ++children_a;
        }
        for (j=target+1;j<record->count;++j) if (record->nodes[j].parent==target) ++children_b;
        if (children_a!=children_b) return 0;
    }
    return 1;
}
static int measurements(const GD_ExperimentInput *record,const GD_ResultContract *contract,int full,GD_ResultReport *report) {
    int values=member(record,0,"measurements"),undefined=member(record,0,"undefined"),i,j,n=0;
    uint64_t undefined_count=0;
    if (!object(record,values) || (undefined>=0 && !object(record,undefined))) return fail(report,"results.measurements","Measurements and optional undefined reasons must be objects");
    for (i=values+1;i<record->count;++i) if (record->nodes[i].parent==values) {
        const GD_ResultMeasure *measure=NULL;
        int reason=undefined>=0 ? member(record,undefined,record->nodes[i].key) : -1;
        double number; int64_t whole;
        for (j=0;j<contract->measure_count;++j) if (equal(record->nodes[i].key,contract->measures[j].name)) { measure=&contract->measures[j]; break; }
        if (!measure) return fail(report,"results.measurement_key","Undeclared measurement key");
        ++n;
        if (record->nodes[i].kind==GD_INPUT_NULL) {
            if (!full || !bounded(gd_input_string(record,reason),4096)) return fail(report,"results.undefined","Null trial measurement needs nonempty undefined reason");
            ++undefined_count; continue;
        }
        if (reason>=0) return fail(report,"results.undefined","Defined value must not have undefined reason");
        if ((equal(measure->type,"number") && !gd_input_number(record,i,&number)) ||
            (equal(measure->type,"integer") && !gd_input_integer(record,i,&whole)) ||
            (equal(measure->type,"boolean") && record->nodes[i].kind!=GD_INPUT_BOOLEAN))
            return fail(report,"results.measurement_type","Measurement has wrong declared type");
        if (equal(measure->type,"enum")) {
            int choices=member(contract->input,measure->definition,"allowed_values"),found=0;
            const char *value=gd_input_string(record,i);
            for (j=choices+1;j<contract->input->count;++j) if (contract->input->nodes[j].parent==choices && equal(value,contract->input->nodes[j].text)) found=1;
            if (!found) return fail(report,"results.measurement_type","Enum measurement is not a declared choice");
        }
    }
    if (full && n!=contract->measure_count) return fail(report,"results.measurements","Trial must include every declared measure, including explicitly undefined ones");
    if (undefined>=0) for (i=undefined+1;i<record->count;++i) if (record->nodes[i].parent==undefined) {
        int value=member(record,values,record->nodes[i].key);
        if (value<0 || record->nodes[value].kind!=GD_INPUT_NULL) return fail(report,"results.undefined","Unexpected undefined reason");
    }
    report->undefined_measurements+=undefined_count;
    return 1;
}
int gd_result_validate(FILE *stream,const GD_ResultContract *contract,const GD_ResultProvenance *provenance,
    int process_exit_code,int externally_stopped,GD_Allocator allocator,GD_ResultReport *report) {
    GD_ExperimentInput *record=NULL;
    unsigned char *seen=NULL;
    int header=0,completion=0,ok=1,trial_error=0;
    int64_t last_step=0;
    const char *final_status=NULL;
    memset(report,0,sizeof(*report)); report->has_exit_code=1; report->process_exit_code=process_exit_code; report->status="invalid";
    if (!provenance || !gd_result_commit(provenance->checkpoint) || !bounded(provenance->build,1024) || !bounded(provenance->toolchain,1024))
        return fail(report,"results.provenance","Expected actual checkpoint, build and toolchain required");
    record=gd_experiment_input_create(allocator);
    if (record) seen=allocator.allocate(allocator.context,(size_t)(contract->trials+7)/8);
    if (!record || !seen) { fail(report,"results.memory","Cannot allocate bounded result reader"); ok=0; goto done; }
    memset(seen,0,(size_t)(contract->trials+7)/8);
    for (;;) {
        size_t length=0; int c=0;
        while ((c=fgetc(stream))!=EOF) {
            if (report->bytes==contract->max_bytes) { fail(report,"results.bytes","Captured stream exceeds declared output cap"); ok=0; break; }
            ++report->bytes;
            if (c=='\n') break;
            if (length==GD_EVENT_BYTES) { fail(report,"results.record_limit","NDJSON record exceeds header/event bound"); ok=0; break; }
            record->raw[length++]=(char)c;
        }
        if (!ok) break;
        if (c==EOF && !length) break;
        ++report->line;
        if (c==EOF) { ok=fail(report,"results.truncated","Final NDJSON record lacks newline"); break; }
        if (length && record->raw[length-1]=='\r') --length;
        if (completion) { ok=fail(report,"results.trailing","Record after completion"); break; }
        if (!gd_experiment_parse_event(record,record->raw,length)) { ok=fail(report,"results.json","Invalid bounded UTF-8 JSON record, duplicate key or non-finite number"); break; }
        ++report->records;
        {
            const char *type=string(record,0,"type");
            if (!header) {
                int dependencies=member(record,0,"dependencies");
                if (equal(type,"error")) { ok=fail(report,"results.producer_error","Producer reported pre-header error; inspect retained stdout/stderr"); break; }
                if (!equal(type,"header") || !equal(string(record,0,"protocol"),GD_EXPERIMENT_PROTOCOL) ||
                    !equal(string(record,0,"model"),contract->model) || !equal(string(record,0,"helper"),GD_EXPERIMENT_VERSION) ||
                    !equal(string(record,0,"checkpoint_commit"),provenance->checkpoint) || !equal(string(record,0,"build"),provenance->build) ||
                    !equal(string(record,0,"toolchain"),provenance->toolchain) ||
                    !equal(string(record,dependencies,"libyaml"),GD_EXPERIMENT_YAML_PIN) ||
                    !equal(string(record,dependencies,"pcg-c-basic"),GD_EXPERIMENT_PCG_PIN) ||
                    !same_input(contract->input,record,member(record,0,"input"))) {
                    ok=fail(report,"results.header","First record must match protocol, provenance, dependencies and entire frozen input"); break;
                }
                header=1; continue;
            }
            if (equal(type,"trial") || equal(type,"step")) {
                int64_t trial,steps;
                if (!equal(string(record,0,"configuration_id"),contract->configuration) || !integer(record,0,"trial",&trial) || trial<0 || trial>=contract->trials ||
                    (seen[trial/8]&(1U<<(trial%8)))) { ok=fail(report,"results.identity","Wrong configuration, duplicate/invalid trial identity, or trace after trial"); break; }
                if (equal(type,"step")) {
                    if (trial!=contract->trace_trial || !integer(record,0,"step",&steps) || steps<=last_step || steps>contract->max_steps ||
                        !object(record,member(record,0,"state"))) { ok=fail(report,"results.step","Trace requires selected trial, increasing bounded step and state object"); break; }
                    if (!measurements(record,contract,0,report)) { ok=0; break; }
                    last_step=steps; ++report->steps;
                } else {
                    const char *status=string(record,0,"status");
                    if ((!equal(status,"complete") && !equal(status,"limit") && !equal(status,"error")) ||
                        !integer(record,0,"steps",&steps) || steps<0 || steps>contract->max_steps ||
                        (trial==contract->trace_trial && (last_step>steps || (steps>0 && last_step==0)))) {
                        ok=fail(report,"results.trial","Trial requires recognized execution status, bounded steps and requested trace"); break;
                    }
                    if (!measurements(record,contract,1,report)) { ok=0; break; }
                    seen[trial/8]|=(unsigned char)(1U<<(trial%8)); ++report->trials;
                    if (equal(status,"complete")) ++report->completed_trials;
                    if (equal(status,"error")) trial_error=1;
                }
            } else if (equal(type,"completion")) {
                int64_t requested,attempted,completed; int partial;
                const char *status=string(record,0,"status");
                if ((!equal(status,"complete") && !equal(status,"limit") && !equal(status,"error")) ||
                    !integer(record,0,"trials_requested",&requested) || requested!=contract->trials ||
                    !integer(record,0,"trials_attempted",&attempted) || attempted!=(int64_t)report->trials ||
                    !integer(record,0,"trials_completed",&completed) || completed!=(int64_t)report->completed_trials ||
                    !boolean(record,0,"partial_evidence",&partial) || partial==equal(status,"complete") ||
                    (equal(status,"limit") && trial_error) ||
                    (equal(status,"complete") && (completed!=requested || attempted!=completed))) {
                    ok=fail(report,"results.completion","Final status/counts/partial flag disagree with retained trial records"); break;
                }
                final_status=equal(status,"complete") ? "complete" : equal(status,"limit") ? "limit" : "error";
                completion=1;
            } else { ok=fail(report,"results.record","Unexpected record kind or repeated header"); break; }
        }
    }
    if (ok && ferror(stream)) ok=fail(report,"results.io","Cannot read captured results");
    if (ok && (!header || !completion)) ok=fail(report,"results.completion","Missing header or explicit final completion; evidence incomplete");
    if (ok && !externally_stopped && process_exit_code!=(equal(final_status,"complete") ? 0 : equal(final_status,"limit") ? 3 : 2))
        ok=fail(report,"results.exit","Process exit disagrees with producer completion status");
    if (ok) {
        report->protocol_valid=1; report->complete=!externally_stopped && equal(final_status,"complete");
        report->status=externally_stopped ? "incomplete" : final_status;
        report->code=externally_stopped ? "results.stopped" : NULL;
        report->message=externally_stopped ? "External supervision stopped execution; producer completion cannot establish success" : NULL;
    }
done:
    if (seen) allocator.release(allocator.context,seen);
    gd_experiment_input_destroy(record); return ok;
}
void gd_result_write(FILE *output,const GD_ResultReport *report) {
    char exit_code[32];
    if (report->has_exit_code) snprintf(exit_code,sizeof(exit_code),"%d",report->process_exit_code);
    else strcpy(exit_code,"null");
    /* Only static diagnostics/status strings are written here. No user text is
       interpolated into JSON; full source evidence remains in captured files. */
    fprintf(output,"{\"schema_version\":1,\"ok\":%s,\"protocol_valid\":%s,\"complete\":%s,\"status\":\"%s\",\"process_exit_code\":%s,\"records\":%llu,\"trials\":%llu,\"completed_trials\":%llu,\"step_records\":%llu,\"undefined_measurements\":%llu,\"bytes\":%llu,\"error\":",
        report->complete ? "true" : "false",report->protocol_valid ? "true" : "false",report->complete ? "true" : "false",report->status ? report->status : "invalid",exit_code,
        (unsigned long long)report->records,(unsigned long long)report->trials,(unsigned long long)report->completed_trials,
        (unsigned long long)report->steps,(unsigned long long)report->undefined_measurements,(unsigned long long)report->bytes);
    if (report->code) fprintf(output,"{\"code\":\"%s\",\"message\":\"%s\",\"line\":%llu}",report->code,report->message,(unsigned long long)report->line);
    else fputs("null",output);
    fputs("}\n",output);
}
