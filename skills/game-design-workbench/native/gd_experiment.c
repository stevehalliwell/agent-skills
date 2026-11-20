#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif
#include "gd_experiment.h"
#include "vendor/libyaml/include/yaml.h"
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#endif

static void whitespace(const char **p, const char *end) {
    while (*p<end && (**p==' ' || **p=='\t' || **p=='\r' || **p=='\n')) ++*p;
}
static int digit(char c) { return c>='0' && c<='9'; }
static int hex(char c) {
    return digit(c) ? c-'0' : c>='a' && c<='f' ? c-'a'+10 : c>='A' && c<='F' ? c-'A'+10 : -1;
}
static int string_end(const char **p, const char *end) {
    ++*p;
    while (*p<end) {
        unsigned char c=(unsigned char)*(*p)++;
        if (c=='"') return 1;
        if (c<32) return 0;
        if (c=='\\') {
            if (*p==end) return 0;
            c=(unsigned char)*(*p)++;
            if (c=='u') {
                int i;
                for (i=0;i<4;++i) if (*p==end || hex(*(*p)++)<0) return 0;
            } else if (!strchr("\"\\/bfnrt",c)) return 0;
        }
    }
    return 0;
}
static int number_end(const char **p, const char *end) {
    if (*p<end && **p=='-') ++*p;
    if (*p==end) return 0;
    if (**p=='0') ++*p;
    else {
        if (**p<'1' || **p>'9') return 0;
        while (*p<end && digit(**p)) ++*p;
    }
    if (*p<end && **p=='.') {
        ++*p; if (*p==end || !digit(**p)) return 0;
        while (*p<end && digit(**p)) ++*p;
    }
    if (*p<end && (**p=='e' || **p=='E')) {
        ++*p; if (*p<end && (**p=='+' || **p=='-')) ++*p;
        if (*p==end || !digit(**p)) return 0;
        while (*p<end && digit(**p)) ++*p;
    }
    return 1;
}
/* LibYAML decodes UTF-8/escapes, but accepts YAML flow syntax. This finite
   grammar gate excludes YAML, trailing commas, tags, aliases and extra roots. */
static int json_syntax(const char *p, size_t size, int max_depth) {
    enum { KEY_OR_END, KEY, COLON, VALUE, OBJECT_END, ARRAY_OR_END, ARRAY_VALUE, ARRAY_END, DONE };
    int states[GD_EVENT_DEPTH+1], depth=0;
    const char *end=p+size;
    states[0]=VALUE;
    while (p<end) {
        int state=states[depth];
        whitespace(&p,end); if (p==end) break;
        if (state==DONE) return 0;
        if (state==KEY_OR_END || state==KEY) {
            if (state==KEY_OR_END && *p=='}') { ++p; --depth; continue; }
            if (*p!='"' || !string_end(&p,end)) return 0;
            states[depth]=COLON; continue;
        }
        if (state==COLON) { if (*p++!=':') return 0; states[depth]=VALUE; continue; }
        if (state==OBJECT_END || state==ARRAY_END) {
            if (*p==(state==OBJECT_END ? '}' : ']')) { ++p; --depth; continue; }
            if (*p++!=',') return 0;
            states[depth]=state==OBJECT_END ? KEY : ARRAY_VALUE; continue;
        }
        if (state==ARRAY_OR_END && *p==']') { ++p; --depth; continue; }
        states[depth]=depth==0 ? DONE : (state==ARRAY_OR_END || state==ARRAY_VALUE) ? ARRAY_END : OBJECT_END;
        if (*p=='{' || *p=='[') {
            char c=*p++;
            if (depth==max_depth) return 0;
            states[++depth]=c=='{' ? KEY_OR_END : ARRAY_OR_END;
        } else if (*p=='"') { if (!string_end(&p,end)) return 0; }
        else if (end-p>=4 && (!memcmp(p,"true",4) || !memcmp(p,"null",4))) p+=4;
        else if (end-p>=5 && !memcmp(p,"false",5)) p+=5;
        else if (!number_end(&p,end)) return 0;
    }
    return depth==0 && states[0]==DONE;
}
GD_ExperimentInput *gd_experiment_input_create(GD_Allocator allocator) {
    GD_ExperimentInput *input;
    if (!allocator.allocate || !allocator.release) return NULL;
    input=allocator.allocate(allocator.context,sizeof(*input));
    if (input) { memset(input,0,sizeof(*input)); input->allocator=allocator; }
    return input;
}
void gd_experiment_input_destroy(GD_ExperimentInput *input) {
    if (input) input->allocator.release(input->allocator.context,input);
}
static const char *copy_scalar(GD_ExperimentInput *input, const unsigned char *value, size_t size) {
    char *text;
    if (memchr(value,0,size) || size>=sizeof(input->text)-input->text_used) return NULL;
    text=input->text+input->text_used;
    memcpy(text,value,size); text[size]=0; input->text_used+=size+1; return text;
}
int gd_input_member(const GD_ExperimentInput *input, int object, const char *key) {
    int i;
    if (object<0 || object>=input->count || input->nodes[object].kind!=GD_INPUT_OBJECT) return -1;
    for (i=object+1;i<input->count;++i)
        if (input->nodes[i].parent==object && input->nodes[i].key && !strcmp(input->nodes[i].key,key)) return i;
    return -1;
}
const char *gd_input_string(const GD_ExperimentInput *input, int node) {
    return node>=0 && node<input->count && input->nodes[node].kind==GD_INPUT_STRING ? input->nodes[node].text : NULL;
}
int gd_input_number(const GD_ExperimentInput *input, int node, double *value) {
    char *end;
    if (node<0 || node>=input->count || input->nodes[node].kind!=GD_INPUT_NUMBER) return 0;
    errno=0; *value=strtod(input->nodes[node].text,&end);
    return !*end && isfinite(*value) && !(errno==ERANGE && *value==0);
}
int gd_input_integer(const GD_ExperimentInput *input, int node, int64_t *value) {
    const char *p; uint64_t magnitude=0; int negative;
    if (node<0 || node>=input->count || input->nodes[node].kind!=GD_INPUT_NUMBER) return 0;
    p=input->nodes[node].text; negative=*p=='-'; if (negative) ++p;
    if (!*p) return 0;
    for (;*p;++p) {
        unsigned d;
        if (!digit(*p)) return 0;
        d=(unsigned)(*p-'0');
        if (magnitude>(UINT64_C(9007199254740991)-d)/10) return 0;
        magnitude=magnitude*10+d;
    }
    *value=negative ? -(int64_t)magnitude : (int64_t)magnitude; return 1;
}
int gd_input_seed(const char *text, uint64_t *value) {
    int i;
    if (!text || strlen(text)!=18 || text[0]!='0' || text[1]!='x') return 0;
    *value=0;
    for (i=2;i<18;++i) { int d=hex(text[i]); if (d<0) return 0; *value=(*value<<4)|(unsigned)d; }
    return 1;
}
/* YAML rejects JSON's UTF-16 surrogate-pair escapes. Normalize only those
   escapes to their UTF-8 scalar before LibYAML; retain all values and types.
   In-place output never grows. Escaped backslashes are consumed together. */
static int json_surrogates(char *text, size_t *size) {
    size_t i=0,w=0; int quoted=0;
    while (i<*size) {
        char c=text[i++];
        if (c=='"') quoted=!quoted;
        if (quoted && c=='\\' && i<*size) {
            size_t escape=i;
            if (text[i]=='u') {
                unsigned high=0,low=0; int j;
                for (j=1;j<=4;++j) high=high*16+(unsigned)hex(text[i+j]);
                if (high>=0xdc00 && high<=0xdfff) return 0;
                if (high>=0xd800 && high<=0xdbff) {
                    unsigned code;
                    if (*size-i<11 || text[i+5]!='\\' || text[i+6]!='u') return 0;
                    for (j=7;j<=10;++j) { int d=hex(text[i+j]); if (d<0) return 0; low=low*16+(unsigned)d; }
                    if (low<0xdc00 || low>0xdfff) return 0;
                    code=0x10000+((high-0xd800)<<10)+(low-0xdc00);
                    text[w++]=(char)(0xf0|(code>>18)); text[w++]=(char)(0x80|((code>>12)&63));
                    text[w++]=(char)(0x80|((code>>6)&63)); text[w++]=(char)(0x80|(code&63));
                    i+=11; continue;
                }
            }
            text[w++]=c; text[w++]=text[escape]; ++i; continue;
        }
        text[w++]=c;
    }
    text[w]=0; *size=w; return 1;
}
static int parse(GD_ExperimentInput *input, const char *bytes, size_t size, size_t max_bytes, int max_nodes, int max_depth) {
    yaml_parser_t parser;
    int stack[GD_EVENT_DEPTH], depth=0, ok=1, finished=0;
    const char *keys[GD_EVENT_DEPTH]={0};
    input->count=0; input->text_used=0; input->error=NULL;
    if (size>max_bytes || !json_syntax(bytes,size,max_depth)) { input->error="Expected one strict JSON object within byte/depth limits"; return 0; }
    if (bytes!=input->raw) memmove(input->raw,bytes,size);
    input->raw[size]=0;
    if (!json_surrogates(input->raw,&size)) { input->error="Unpaired JSON surrogate escape"; return 0; }
    input->raw_size=size;
    if (!yaml_parser_initialize(&parser)) { input->error="Cannot allocate input parser"; return 0; }
    yaml_parser_set_input_string(&parser,(const unsigned char *)input->raw,size);
    while (ok && !finished) {
        yaml_event_t event;
        if (!yaml_parser_parse(&parser,&event)) { ok=0; break; }
        if (event.type==YAML_STREAM_END_EVENT) finished=1;
        else if (event.type==YAML_MAPPING_END_EVENT || event.type==YAML_SEQUENCE_END_EVENT) {
            if (!depth || keys[depth-1]) ok=0; else --depth;
        } else if (event.type==YAML_SCALAR_EVENT || event.type==YAML_MAPPING_START_EVENT || event.type==YAML_SEQUENCE_START_EVENT) {
            int parent=depth ? stack[depth-1] : -1;
            const char *text=NULL;
            if (event.type==YAML_SCALAR_EVENT) {
                text=copy_scalar(input,event.data.scalar.value,event.data.scalar.length);
                if (!text) { ok=0; yaml_event_delete(&event); break; }
            }
            if (parent>=0 && input->nodes[parent].kind==GD_INPUT_OBJECT && !keys[depth-1]) {
                if (!text || event.data.scalar.style!=YAML_DOUBLE_QUOTED_SCALAR_STYLE || gd_input_member(input,parent,text)>=0) ok=0;
                else keys[depth-1]=text;
            } else {
                GD_InputNode *node;
                if (input->count==max_nodes) { ok=0; yaml_event_delete(&event); break; }
                node=&input->nodes[input->count++]; node->parent=parent; node->text=text;
                node->key=parent>=0 ? keys[depth-1] : NULL;
                if (parent>=0) keys[depth-1]=NULL;
                if (text) {
                    double number;
                    node->kind=event.data.scalar.style==YAML_DOUBLE_QUOTED_SCALAR_STYLE ? GD_INPUT_STRING :
                        !strcmp(text,"null") ? GD_INPUT_NULL : !strcmp(text,"true") || !strcmp(text,"false") ? GD_INPUT_BOOLEAN : GD_INPUT_NUMBER;
                    if (node->kind==GD_INPUT_NUMBER && !gd_input_number(input,input->count-1,&number)) ok=0;
                } else {
                    node->kind=event.type==YAML_MAPPING_START_EVENT ? GD_INPUT_OBJECT : GD_INPUT_ARRAY;
                    if (depth==max_depth) ok=0;
                    else { stack[depth]=input->count-1; keys[depth++]=NULL; }
                }
            }
        }
        yaml_event_delete(&event);
    }
    yaml_parser_delete(&parser);
    if (!ok || !finished || depth || !input->count || input->nodes[0].kind!=GD_INPUT_OBJECT) {
        input->error="Invalid UTF-8/escape/number, duplicate key, non-object root, or input storage limit"; return 0;
    }
    /* Compact raw JSON for an embedded single-line NDJSON header. Owned decoded
       nodes remain valid; no number formatting or type conversion occurs here. */
    {
        size_t i,w=0; int quoted=0,escape=0;
        for (i=0;i<size;++i) {
            char c=input->raw[i];
            if (quoted || (c!=' ' && c!='\n' && c!='\r' && c!='\t')) input->raw[w++]=c;
            if (escape) escape=0;
            else if (quoted && c=='\\') escape=1;
            else if (c=='"') quoted=!quoted;
        }
        input->raw[w]=0; input->raw_size=w;
    }
    return 1;
}
int gd_experiment_parse(GD_ExperimentInput *input, const char *bytes, size_t size) {
    return parse(input,bytes,size,GD_MAX_FILE,GD_INPUT_NODES,GD_INPUT_DEPTH);
}
int gd_experiment_parse_event(GD_ExperimentInput *input, const char *bytes, size_t size) {
    return parse(input,bytes,size,GD_EVENT_BYTES,GD_EVENT_NODES,GD_EVENT_DEPTH);
}
int gd_experiment_read(GD_ExperimentInput *input, FILE *stream) {
    size_t size=fread(input->raw,1,GD_MAX_FILE+1,stream);
    if (ferror(stream) || size>GD_MAX_FILE) { input->error="Cannot read input or input exceeds 1 MiB"; return 0; }
    return gd_experiment_parse(input,input->raw,size);
}
void gd_trial_random(pcg32_random_t *random, uint64_t seed, uint64_t trial) { pcg32_srandom_r(random,seed,trial); }
double gd_trial_uniform(pcg32_random_t *random) { return ldexp(pcg32_random_r(random),-32); }
int gd_trial_bounded(pcg32_random_t *random, uint32_t bound, uint32_t *value) {
    unsigned i; uint32_t threshold;
    if (!bound) return 0;
    threshold=(uint32_t)(0U-bound)%bound;
    for (i=0;i<128;++i) { uint32_t draw=pcg32_random_r(random); if (draw>=threshold) { *value=draw%bound; return 1; } }
    return 0;
}
double gd_experiment_clock(void) {
#ifdef _WIN32
    LARGE_INTEGER count,frequency;
    if (!QueryPerformanceFrequency(&frequency) || !QueryPerformanceCounter(&count) || frequency.QuadPart<=0) return -1;
    return (double)count.QuadPart/(double)frequency.QuadPart;
#else
    struct timespec time;
    if (clock_gettime(CLOCK_MONOTONIC,&time)!=0) return -1;
    return (double)time.tv_sec+(double)time.tv_nsec/1e9;
#endif
}
int gd_trial_budget_check(GD_TrialBudget *budget) {
    double now;
    if (budget->status) return 0;
    now=gd_experiment_clock();
    if (now<0 || !isfinite(budget->started) || budget->started<0 || now<budget->started || !isfinite(budget->timeout) || budget->timeout<=0) budget->status="error";
    else if (now-budget->started>=budget->timeout) budget->status="limit";
    return budget->status==NULL;
}
int gd_trial_step(GD_TrialBudget *budget) {
    if (!gd_trial_budget_check(budget)) return 0;
    if (budget->steps>=budget->max_steps) { budget->status="limit"; return 0; }
    ++budget->steps; return 1;
}
int gd_trial_event(GD_TrialBudget *budget, const char *json, size_t size, int completion) {
    size_t reserve=completion ? 0 : 1024;
    if (!completion && !gd_trial_budget_check(budget)) return 0;
    if (size==SIZE_MAX || budget->bytes>budget->max_bytes || reserve>budget->max_bytes-budget->bytes ||
        size+1>budget->max_bytes-budget->bytes-reserve) { if (!budget->status) budget->status="limit"; return 0; }
    if (fwrite(json,1,size,budget->output)!=size || fputc('\n',budget->output)==EOF || fflush(budget->output)!=0) { budget->status="error"; return 0; }
    budget->bytes+=size+1; return 1;
}
int gd_measure_number(char *output, size_t capacity, double value) {
    int count;
    if (!isfinite(value)) return 0;
    count=snprintf(output,capacity,"%.17g",value);
    return count>=0 && (size_t)count<capacity;
}
int gd_event_quote(char *output, size_t capacity, const char *text) {
    const unsigned char *p=(const unsigned char *)text;
    size_t used=0;
    if (capacity<3) return 0;
    output[used++]='"';
    while (*p) {
        unsigned c=*p++;
        size_t needed=c<32 ? 6 : (c=='"' || c=='\\') ? 2 : 1;
        if (used+needed+2>capacity) return 0;
        if (c<32) { snprintf(output+used,7,"\\u%04x",c); used+=6; }
        else { if (c=='"' || c=='\\') output[used++]='\\'; output[used++]=(char)c; }
    }
    output[used++]='"'; output[used]=0; return 1;
}
