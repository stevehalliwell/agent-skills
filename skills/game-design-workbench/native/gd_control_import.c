#include "gd.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    GD_Model *model;
    GD_ControlImport *plan;
    const char *p, *end, *path;
    int line;
    unsigned char seen[GD_MAX_RECORDS];
} ImportReader;
static int import_error(ImportReader *reader, const char *code, const char *message) {
    gd_error(reader->model,code,message,reader->path,reader->line); return 0;
}
static int take(ImportReader *reader) {
    unsigned char c=(unsigned char)*reader->p++;
    if (c=='\n') ++reader->line;
    return c;
}
static void space(ImportReader *reader) {
    while (reader->p<reader->end && (*reader->p==' ' || *reader->p=='\t' || *reader->p=='\r' || *reader->p=='\n')) take(reader);
}
static int byte(ImportReader *reader, unsigned value) {
    if (reader->plan->text_used>=sizeof(reader->plan->text)) return 0;
    reader->plan->text[reader->plan->text_used++]=(char)value; return 1;
}
static const char *copy_span(ImportReader *reader, const char *start, size_t size) {
    char *target;
    if (size>=sizeof(reader->plan->text)-reader->plan->text_used) return NULL;
    target=reader->plan->text+reader->plan->text_used;
    memcpy(target,start,size); target[size]=0; reader->plan->text_used+=size+1; return target;
}
static int hex4(ImportReader *reader, unsigned *value) {
    int i;
    *value=0;
    for (i=0;i<4;++i) {
        unsigned digit,c;
        if (reader->p==reader->end) return 0;
        c=(unsigned)take(reader);
        if (c>='0' && c<='9') digit=c-'0';
        else if (c>='A' && c<='F') digit=c-'A'+10;
        else if (c>='a' && c<='f') digit=c-'a'+10;
        else return 0;
        *value=*value*16+digit;
    }
    return 1;
}
/* Flat game-value JSON needs no tree/general configuration parser. Decode full
   JSON string escapes here, including surrogate pairs; never YAML coercion. */
static const char *json_string(ImportReader *reader) {
    size_t start=reader->plan->text_used;
    if (reader->p==reader->end || take(reader)!='"') return NULL;
    while (reader->p<reader->end) {
        unsigned c=(unsigned)take(reader);
        if (c=='"') { if (!byte(reader,0)) return NULL; return reader->plan->text+start; }
        if (c<32) return NULL;
        if (c=='\\') {
            if (reader->p==reader->end) return NULL;
            c=(unsigned)take(reader);
            if (c=='u') {
                unsigned code,low;
                if (!hex4(reader,&code) || !code) return NULL;
                if (code>=0xd800 && code<=0xdbff) {
                    if (reader->end-reader->p<2 || take(reader)!='\\' || take(reader)!='u' || !hex4(reader,&low) || low<0xdc00 || low>0xdfff) return NULL;
                    code=0x10000+((code-0xd800)<<10)+(low-0xdc00);
                } else if (code>=0xdc00 && code<=0xdfff) return NULL;
                if (code>=0x10000) {
                    if (!byte(reader,0xf0|(code>>18)) || !byte(reader,0x80|((code>>12)&63))) return NULL;
                } else if (code>=0x800) {
                    if (!byte(reader,0xe0|(code>>12))) return NULL;
                }
                if (code>=0x80 && !byte(reader,(code>=0x800 ? 0x80:0xc0)|((code>>6)&63))) return NULL;
                if (!byte(reader,code>=0x80 ? 0x80|(code&63):code)) return NULL;
                continue;
            }
            if (c=='b') c='\b'; else if (c=='f') c='\f'; else if (c=='n') c='\n'; else if (c=='r') c='\r'; else if (c=='t') c='\t';
            else if (c!='"' && c!='\\' && c!='/') return NULL;
        }
        if (!byte(reader,c)) return NULL;
    }
    return NULL;
}
static int json_scalar(ImportReader *reader, GD_ControlInput *input) {
    const char *start=reader->p;
    if (reader->p==reader->end) return 0;
    if (*reader->p=='"') { input->kind=GD_LITERAL_STRING; input->text=json_string(reader); return input->text!=NULL; }
    if (reader->end-reader->p>=4 && !memcmp(reader->p,"true",4)) { reader->p+=4; input->kind=GD_LITERAL_BOOLEAN; input->text="true"; return 1; }
    if (reader->end-reader->p>=5 && !memcmp(reader->p,"false",5)) { reader->p+=5; input->kind=GD_LITERAL_BOOLEAN; input->text="false"; return 1; }
    if (*reader->p=='-') ++reader->p;
    if (reader->p==reader->end) return 0;
    if (*reader->p=='0') ++reader->p;
    else {
        if (*reader->p<'1' || *reader->p>'9') return 0;
        while (reader->p<reader->end && isdigit((unsigned char)*reader->p)) ++reader->p;
    }
    if (reader->p<reader->end && *reader->p=='.') {
        ++reader->p; if (reader->p==reader->end || !isdigit((unsigned char)*reader->p)) return 0;
        while (reader->p<reader->end && isdigit((unsigned char)*reader->p)) ++reader->p;
    }
    if (reader->p<reader->end && (*reader->p=='e' || *reader->p=='E')) {
        ++reader->p; if (reader->p<reader->end && (*reader->p=='+' || *reader->p=='-')) ++reader->p;
        if (reader->p==reader->end || !isdigit((unsigned char)*reader->p)) return 0;
        while (reader->p<reader->end && isdigit((unsigned char)*reader->p)) ++reader->p;
    }
    input->kind=GD_LITERAL_NUMBER; input->text=copy_span(reader,start,(size_t)(reader->p-start)); return input->text!=NULL;
}
static int input_error(ImportReader *reader, const GD_ControlInput *input, const char *code, const char *message) {
    gd_error(reader->model,code,message,reader->path,input->line); return 0;
}
static int add_input(ImportReader *reader, const char *key, const GD_ControlInput *input) {
    int record=gd_control_find_name(reader->model,key);
    GD_ControlInput *stored;
    if (record<0) return input_error(reader,input,"controls.import_key","Import key must identify a unique existing control");
    if (!reader->plan->current->requested[record]) return input_error(reader,input,"controls.import_scope","Imported key is outside the selected export set");
    if (reader->seen[record]) return input_error(reader,input,"controls.import_duplicate","Import repeats a control key");
    if (reader->plan->count==GD_MAX_RECORDS) return input_error(reader,input,"controls.import_limit","At most 4096 imported controls");
    reader->seen[record]=1; stored=&reader->plan->inputs[reader->plan->count++]; *stored=*input; stored->record=record; return 1;
}
static int parse_json(ImportReader *reader) {
    space(reader); if (reader->p==reader->end || take(reader)!='{') return 0;
    space(reader);
    if (reader->p<reader->end && *reader->p=='}') { ++reader->p; space(reader); return reader->p==reader->end; }
    for (;;) {
        const char *key;
        GD_ControlInput input;
        memset(&input,0,sizeof(input)); input.line=reader->line;
        key=json_string(reader); if (!key) return 0;
        space(reader); if (reader->p==reader->end || take(reader)!=':') return 0;
        space(reader); if (!json_scalar(reader,&input)) return 0;
        if (!add_input(reader,key,&input)) return 0;
        space(reader); if (reader->p==reader->end) return 0;
        if (*reader->p=='}') { ++reader->p; space(reader); return reader->p==reader->end; }
        if (take(reader)!=',') return 0;
        space(reader);
    }
}
static const char *csv_field(ImportReader *reader) {
    size_t start=reader->plan->text_used;
    int quoted=reader->p<reader->end && *reader->p=='"',closed=!quoted;
    if (quoted) ++reader->p;
    while (reader->p<reader->end) {
        unsigned c=(unsigned char)*reader->p;
        if (quoted && c=='"') {
            ++reader->p;
            if (reader->p==reader->end || *reader->p!='"') { closed=1; break; }
            ++reader->p;
        } else {
            if (!quoted && (c==',' || c=='\r' || c=='\n')) break;
            if (!quoted && c=='"') return NULL;
            take(reader);
        }
        if (!byte(reader,c)) return NULL;
    }
    if (!closed || !byte(reader,0)) return NULL;
    return reader->plan->text+start;
}
static int parse_csv(ImportReader *reader) {
    static const char *headers[]={"key","type","value","unit"};
    int header=1;
    while (reader->p<reader->end) {
        const char *fields[4]; int i;
        GD_ControlInput input;
        memset(&input,0,sizeof(input)); input.line=reader->line;
        for (i=0;i<4;++i) {
            fields[i]=csv_field(reader); if (!fields[i]) return 0;
            if (i<3 && (reader->p==reader->end || take(reader)!=',')) return 0;
        }
        if (reader->p<reader->end) {
            if (*reader->p=='\r') ++reader->p;
            if (reader->p==reader->end || take(reader)!='\n') return 0;
        }
        if (header) {
            for (i=0;i<4;++i) if (strcmp(fields[i],headers[i])) return 0;
            header=0; continue;
        }
        input.type=fields[1]; input.text=fields[2]; input.unit=fields[3];
        input.kind=!strcmp(input.type,"enum") ? GD_LITERAL_STRING : !strcmp(input.type,"boolean") ? GD_LITERAL_BOOLEAN:GD_LITERAL_NUMBER;
        if (!add_input(reader,fields[0],&input)) return 0;
    }
    return !header;
}
static int parse_ini(ImportReader *reader) {
    int section=0;
    while (reader->p<reader->end) {
        const char *end=reader->p,*key,*key_end; int line=reader->line;
        GD_ControlInput input; ImportReader scalar;
        while (end<reader->end && *end!='\n' && *end!='\r') ++end;
        while (reader->p<end && (*reader->p==' ' || *reader->p=='\t')) ++reader->p;
        if (reader->p<end && *reader->p!=';' && *reader->p!='#') {
            if (*reader->p=='[') {
                const char *last=end; while (last>reader->p && (last[-1]==' ' || last[-1]=='\t')) --last;
                if (section || last-reader->p!=10 || memcmp(reader->p,"[controls]",10)) return 0;
                section=1;
            } else {
                if (!section) return 0;
                key_end=reader->p; while (key_end<end && *key_end!='=') ++key_end;
                if (key_end==end) return 0;
                key=reader->p; reader->p=key_end+1;
                while (key_end>key && (key_end[-1]==' ' || key_end[-1]=='\t')) --key_end;
                key=copy_span(reader,key,(size_t)(key_end-key)); if (!key || !*key) return 0;
                memset(&input,0,sizeof(input)); input.line=line;
                /* Bound scalar parsing to this line; no multiline INI dialect. */
                scalar.model=reader->model; scalar.plan=reader->plan; scalar.path=reader->path;
                scalar.p=reader->p; scalar.end=end; scalar.line=reader->line; space(&scalar);
                if (!json_scalar(&scalar,&input)) return 0;
                space(&scalar); if (scalar.p!=end) return 0;
                if (!add_input(reader,key,&input)) return 0;
            }
        }
        reader->p=end;
        if (reader->p<reader->end && *reader->p=='\r') ++reader->p;
        if (reader->p<reader->end && *reader->p=='\n') take(reader);
    }
    return section;
}
static int same_value(const GD_ControlValue *a, const GD_ControlValue *b) {
    if (a->kind!=b->kind) return 0;
    return a->kind==GD_LITERAL_STRING ? !strcmp(a->string,b->string):a->number==b->number;
}
static int input_value(GD_Model *model, const GD_ControlInput *input, GD_ControlValue *value, const GD_Controls *current) {
    const GD_ControlDefinition *definition=&model->records[input->record].control;
    *value=current->values[input->record];
    if (input->kind!=value->kind || (input->type && strcmp(input->type,definition->type))) return 0;
    if (input->unit) {
        GD_Unit unit;
        int i;
        if (!*input->unit) { if (input->kind==GD_LITERAL_NUMBER) return 0; memset(&unit,0,sizeof(unit)); }
        else if (!gd_control_unit(input->unit,strlen(input->unit),&unit)) return 0;
        if (unit.count!=value->unit.count) return 0;
        for (i=0;i<unit.count;++i) if (unit.terms[i].power!=value->unit.terms[i].power || strcmp(unit.terms[i].symbol,value->unit.terms[i].symbol)) return 0;
    }
    if (input->kind==GD_LITERAL_NUMBER) return gd_control_number(input->text,&value->number,!strcmp(definition->type,"integer"));
    if (input->kind==GD_LITERAL_BOOLEAN) {
        if (!strcmp(input->text,"true")) value->number=1;
        else if (!strcmp(input->text,"false")) value->number=0;
        else return 0;
    } else value->string=input->text;
    return 1;
}
/* Validate status-admitted dependents too, even when omitted from game output.
   Braced reference candidates include invalid prefixed forms so an affected
   legacy formula is diagnosed by the resolver, not silently skipped. */
static int affected_selection(ImportReader *reader, const GD_ControlSelection *original, GD_ControlSelection *expanded) {
    int head[GD_MAX_RECORDS],next[GD_MAX_EDGES],source[GD_MAX_EDGES],queue[GD_MAX_RECORDS];
    unsigned char seen[GD_MAX_RECORDS]={0};
    int i,edges=0,read=0,write=0;
    if (original) *expanded=*original;
    else {
        memset(expanded,0,sizeof(*expanded));
        for (i=0;i<reader->model->record_count;++i) if (reader->model->records[i].role==GD_CONTROL) expanded->roots[i]=expanded->allowed[i]=1;
    }
    for (i=0;i<reader->model->record_count;++i) head[i]=-1;
    for (i=0;i<reader->model->record_count;++i) if (expanded->allowed[i] && reader->model->records[i].role==GD_CONTROL) {
        const char *p=reader->model->records[i].control.expression;
        if (!p) continue;
        while ((p=strchr(p,'{'))!=NULL) {
            const char *end=strchr(p+1,'}'); char reference[258]; int target;
            if (!end) break;
            if (end-p-1<=257) {
                memcpy(reference,p+1,(size_t)(end-p-1)); reference[end-p-1]=0;
                target=gd_control_find(reader->model,reference);
                if (target>=0) {
                    if (edges==GD_MAX_EDGES) return import_error(reader,"controls.import_impact_limit","Dependent validation exceeds 32768 declared references");
                    source[edges]=i; next[edges]=head[target]; head[target]=edges++;
                }
            }
            p=end+1;
        }
    }
    for (i=0;i<reader->plan->count;++i) if (reader->plan->inputs[i].changed && !reader->plan->inputs[i].derived) {
        int record=reader->plan->inputs[i].record;
        if (!seen[record]) { seen[record]=1; queue[write++]=record; }
    }
    while (read<write) {
        int edge,record=queue[read++];
        for (edge=head[record];edge>=0;edge=next[edge]) if (!seen[source[edge]]) {
            seen[source[edge]]=1; expanded->roots[source[edge]]=1; queue[write++]=source[edge];
        }
    }
    return 1;
}
GD_ControlImport *gd_plan_control_import(GD_Model *model, const GD_ControlSelection *selection,
                                        const char *input, size_t size, const char *format, const char *path) {
    GD_ControlImport *plan;
    ImportReader reader;
    GD_ControlOverride overrides[GD_MAX_RECORDS];
    int i,count=0,parsed,errors_before;
    if (!input || size>GD_MAX_FILE || !gd_valid_utf8((const unsigned char *)input,size) || !format ||
        (strcmp(format,"json") && strcmp(format,"csv") && strcmp(format,"ini"))) {
        gd_error(model,"controls.import_input","Import requires bounded UTF-8 json/csv/ini input without NUL",path,0); return NULL;
    }
    plan=model->allocator.allocate(model->allocator.context,sizeof(*plan));
    if (!plan) { gd_error(model,"input.memory","Cannot allocate control import plan",path,0); return NULL; }
    memset(plan,0,sizeof(*plan)); plan->allocator=model->allocator;
    plan->current=gd_resolve_control_set(model,selection,NULL,0);
    if (!plan->current || !plan->current->complete) return plan;
    memset(&reader,0,sizeof(reader)); reader.model=model; reader.plan=plan;
    reader.p=input; reader.end=input+size; reader.path=path; reader.line=1;
    if (size>=3 && !memcmp(reader.p,"\xef\xbb\xbf",3)) reader.p+=3;
    errors_before=model->error_count+model->errors_omitted;
    parsed=!strcmp(format,"json") ? parse_json(&reader):!strcmp(format,"csv") ? parse_csv(&reader):parse_ini(&reader);
    if (!parsed) {
        if (model->error_count+model->errors_omitted==errors_before) import_error(&reader,"controls.import_syntax","Invalid flat export dialect, escape, field count, or input complexity");
        return plan;
    }
    for (i=0;i<plan->count;++i) {
        GD_ControlInput *entry=&plan->inputs[i]; GD_ControlValue value;
        const GD_ControlDefinition *definition=&model->records[entry->record].control;
        if (!input_value(model,entry,&value,plan->current)) {
            gd_error(model,"controls.import_type","Imported type/unit/value must match current definition; no coercion or definition changes",path,entry->line); continue;
        }
        entry->derived=definition->expression && *definition->expression;
        entry->changed=!same_value(&value,&plan->current->values[entry->record]);
        if (entry->derived) { ++plan->derived_count; continue; }
        if (entry->changed) {
            overrides[count].reference=model->records[entry->record].reference; overrides[count++].value=entry->text; ++plan->changed_count;
        }
    }
    if (model->error_count || model->errors_omitted) return plan;
    {
        GD_ControlSelection expanded;
        reader.line=0;
        if (count && !affected_selection(&reader,selection,&expanded)) return plan;
        plan->candidate=gd_resolve_control_set(model,count ? &expanded:selection,overrides,count);
    }
    if (!plan->candidate || !plan->candidate->complete) return plan;
    for (i=0;i<plan->count;++i) if (plan->inputs[i].derived && plan->inputs[i].changed) {
        GD_ControlInput *entry=&plan->inputs[i]; GD_ControlValue value;
        if (!input_value(model,entry,&value,plan->current) || !same_value(&value,&plan->candidate->values[entry->record])) {
            entry->conflict=1;
            gd_error(model,"controls.import_formula","Derived-value tweak does not match recomputed formula; change inputs or explicitly revise design, never overwrite formula",path,entry->line);
        }
    }
    plan->complete=!model->error_count && !model->errors_omitted; return plan;
}
void gd_control_import_destroy(GD_ControlImport *plan) {
    if (!plan) return;
    gd_controls_destroy(plan->candidate); gd_controls_destroy(plan->current); plan->allocator.release(plan->allocator.context,plan);
}
