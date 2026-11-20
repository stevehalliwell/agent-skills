/* Engineering checks only. Does not link or execute route model/trials. */
#include "gd_experiment.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(test) do { if (!(test)) { fprintf(stderr,"FAIL line %d: %s (%s)\n",__LINE__,#test,input && input->error ? input->error : "no input error"); return 1; } ++checks; } while (0)
static void *allocate(void *context,size_t size) { (void)context; return malloc(size); }
static void release(void *context,void *memory) { (void)context; free(memory); }
static void *refuse(void *context,size_t size) { (void)context; (void)size; return NULL; }
int main(int argc,char **argv) {
    GD_Allocator allocator={NULL,allocate,release},failure={NULL,refuse,release};
    GD_ExperimentInput *input=gd_experiment_input_create(allocator);
    pcg32_random_t random,copy;
    const uint32_t golden[]={0xa15c02b7,0x7b47f409,0xba1d3330,0x83d2f293,0xbfa4784b,0xcbed606e};
    const char *invalid[]={"", "[]", "null", "{x:1}","{\"x\":01}","{\"x\":1.}","{\"x\":.1}","{\"x\":1e}","{\"x\":truex}","{\"x\":1,}","{\"x\":[1,]}","{}{}", "{\"x\":1,\"x\":2}","{\"x\":1,\"\\u0078\":2}","{\"x\":1e999}","{\"x\":1e-999}","{\"x\":\"\\u0000\"}","{\"x\":\"\\ud800\"}","{\"x\":\"\\udc00\"}","{\"x\":\"\\z\"}","{\"x\":TRUE}","{\"x\":&a 1}","{\"x\": [*a]}","{\"x\":\"\xff\"}"};
    const char *valid="{\"a\":{\"x\":1},\"b\":{\"x\":2},\"array\":[true,false,null,\"\\ud83d\\ude00\"],\"tiny\":5e-324,\"big\":9007199254740992,\"n\":1.25,\"zero\":0,\"q\":\"\\\"\\\\\\n\"}";
    int checks=0,i,a,b;
    int64_t whole;
    double number;
    uint64_t seed;
    uint32_t bounded;
    char text[256],deep[256],line[256];
    FILE *stream;
    GD_TrialBudget budget;
    CHECK(input!=NULL); CHECK(gd_experiment_input_create(failure)==NULL);
    CHECK(gd_experiment_parse(input,valid,strlen(valid)));
    a=gd_input_member(input,0,"a"); b=gd_input_member(input,0,"b");
    CHECK(gd_input_integer(input,gd_input_member(input,a,"x"),&whole) && whole==1);
    CHECK(gd_input_integer(input,gd_input_member(input,b,"x"),&whole) && whole==2);
    CHECK(!gd_input_integer(input,gd_input_member(input,0,"big"),&whole));
    CHECK(!gd_input_integer(input,gd_input_member(input,0,"n"),&whole));
    CHECK(gd_input_number(input,gd_input_member(input,0,"tiny"),&number) && number>0);
    CHECK(!gd_input_number(input,gd_input_member(input,0,"q"),&number));
    CHECK(gd_input_member(input,-1,"x")==-1 && gd_input_member(input,GD_INPUT_NODES,"x")==-1);
    CHECK(!gd_input_string(input,-1) && !gd_input_string(input,GD_INPUT_NODES));
    for (i=0;i<(int)(sizeof(invalid)/sizeof(*invalid));++i) CHECK(!gd_experiment_parse(input,invalid[i],strlen(invalid[i])) && input->error);
    memset(deep,0,sizeof(deep)); strcpy(deep,"{\"x\":");
    for (i=0;i<32;++i) strcat(deep,"[");
    strcat(deep,"0"); for (i=0;i<32;++i) strcat(deep,"]"); strcat(deep,"}");
    CHECK(!gd_experiment_parse(input,deep,strlen(deep)));
    deep[5]='0'; deep[6]='}'; deep[7]=0;
    CHECK(gd_experiment_parse(input,deep,strlen(deep)));
    CHECK(!gd_experiment_parse(input,"{}",GD_MAX_FILE+1));
    CHECK(gd_experiment_parse(input,"{\"x\":9007199254740990.5}",strlen("{\"x\":9007199254740990.5}")));
    CHECK(!gd_input_integer(input,gd_input_member(input,0,"x"),&whole));
    CHECK(gd_experiment_parse(input,"{\"x\":1e0}",9));
    CHECK(!gd_input_integer(input,gd_input_member(input,0,"x"),&whole));
    CHECK(gd_experiment_parse(input,"{\"x\":-9007199254740991}",strlen("{\"x\":-9007199254740991}")));
    CHECK(gd_input_integer(input,gd_input_member(input,0,"x"),&whole) && whole==-INT64_C(9007199254740991));
    {
        size_t used=0;
        used+=(size_t)snprintf(input->raw+used,sizeof(input->raw)-used,"{\"a\":[");
        for (i=0;i<GD_INPUT_NODES-2;++i) used+=(size_t)snprintf(input->raw+used,sizeof(input->raw)-used,"%s0",i ? "," : "");
        used+=(size_t)snprintf(input->raw+used,sizeof(input->raw)-used,"]}");
        CHECK(gd_experiment_parse(input,input->raw,used) && input->count==GD_INPUT_NODES);
        input->raw[used-2]=','; input->raw[used-1]='0'; input->raw[used++]=']'; input->raw[used++]='}';
        CHECK(!gd_experiment_parse(input,input->raw,used));
    }
    CHECK(gd_input_seed("0xffffffffffffffff",&seed) && seed==UINT64_MAX);
    CHECK(gd_input_seed("0x000000000000002a",&seed) && seed==42);
    CHECK(!gd_input_seed("42",&seed) && !gd_input_seed("0x000000000000002g",&seed));
    gd_trial_random(&random,42,54);
    for (i=0;i<6;++i) CHECK(pcg32_random_r(&random)==golden[i]);
    gd_trial_random(&random,42,54); gd_trial_random(&copy,42,54);
    CHECK(gd_trial_uniform(&random)==ldexp(pcg32_random_r(&copy),-32));
    CHECK(!gd_trial_bounded(&random,0,&bounded));
    for (i=0;i<20;++i) CHECK(gd_trial_bounded(&random,7,&bounded) && bounded<7);
    CHECK(gd_measure_number(text,sizeof(text),0.25) && !strcmp(text,"0.25"));
    CHECK(!gd_measure_number(text,sizeof(text),HUGE_VAL));
    CHECK(!gd_measure_number(text,1,0.25));
    CHECK(gd_event_quote(text,4,"a") && !strcmp(text,"\"a\""));
    CHECK(!gd_event_quote(text,3,"a"));
    CHECK(gd_event_quote(text,sizeof(text),"\"\\\n\t") && !strcmp(text,"\"\\\"\\\\\\u000a\\u0009\""));
    stream=tmpfile(); CHECK(stream!=NULL);
    memset(&budget,0,sizeof(budget)); budget.output=stream; budget.max_steps=1; budget.max_bytes=4096;
    budget.started=gd_experiment_clock(); budget.timeout=30;
    CHECK(budget.started>=0 && gd_trial_step(&budget) && budget.steps==1);
    CHECK(!gd_trial_step(&budget) && !strcmp(budget.status,"limit") && budget.steps==1);
    CHECK(gd_trial_event(&budget,"{\"status\":\"limit\"}",18,1));
    rewind(stream); CHECK(fgets(line,sizeof(line),stream)!=NULL && !strcmp(line,"{\"status\":\"limit\"}\n"));
    fclose(stream);
    stream=tmpfile(); CHECK(stream!=NULL);
    memset(&budget,0,sizeof(budget)); budget.output=stream; budget.max_bytes=1030; budget.started=gd_experiment_clock(); budget.timeout=30;
    CHECK(!gd_trial_event(&budget,"{\"x\":0}",7,0) && !strcmp(budget.status,"limit") && budget.bytes==0);
    CHECK(gd_trial_event(&budget,"{\"x\":0}",7,1) && budget.bytes==8);
    fclose(stream);
    memset(&budget,0,sizeof(budget)); budget.started=gd_experiment_clock()-1; budget.timeout=0.0001;
    CHECK(!gd_trial_budget_check(&budget) && !strcmp(budget.status,"limit"));
    memset(&budget,0,sizeof(budget)); budget.started=-1; budget.timeout=30;
    CHECK(!gd_trial_budget_check(&budget) && !strcmp(budget.status,"error"));
    /* Engineering fixture preflight: parse/type check frozen inputs, no draws,
       route actions, time measurements, expectation comparisons or outcomes. */
    CHECK(argc==2);
    stream=fopen(argv[1],"rb"); CHECK(stream!=NULL);
    CHECK(gd_experiment_read(input,stream)); fclose(stream);
    a=gd_input_member(input,0,"controls");
    b=gd_input_member(input,a,"walk-duration");
    CHECK(gd_input_number(input,gd_input_member(input,b,"value"),&number) && number==0.5);
    CHECK(gd_input_seed(gd_input_string(input,gd_input_member(input,0,"seed")),&seed) && seed==42);
    CHECK(gd_input_integer(input,gd_input_member(input,0,"trials"),&whole) && whole==1000);
    CHECK(strchr(input->raw,'\n')==NULL);
    gd_experiment_input_destroy(input);
    printf("{\"engineering_checks\":%d,\"experiment_trials_executed\":0}\n",checks);
    return 0;
}
