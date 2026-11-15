#include "gd_decisions.h"
#include "yaml.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define DECISION_COUNT 4096
#define DECISION_LOCAL 16
#define DECISION_ENTRIES 16
#define DECISION_TOTAL_ENTRIES 32768
#define DECISION_TEXT (4 * 1024 * 1024)
#define DECISION_EVENTS 2048
#define DECISION_DEPTH 8

typedef struct { const char *kind; int intent, line; } Demand;
typedef struct {
    const char *id, *task, *context, *evidence;
    int record, line, first_demand, demand_count, first_reference, reference_count;
} Assessment;
typedef struct {
    GD_Model *model; int count, demand_count, reference_count;
    size_t text_used;
    Assessment assessments[DECISION_COUNT];
    Demand demands[DECISION_TOTAL_ENTRIES];
    const char *references[DECISION_TOTAL_ENTRIES];
    char text[DECISION_TEXT];
} Decisions;
static const char *const intents[] = { "wanted", "unwanted", "undecided" };
static int intent_index(const char *value) {
    int i; for (i=0;i<3;++i) if (value && !strcmp(value,intents[i])) return i; return -1;
}
static int technical(const char *value) {
    size_t i, length=value ? strlen(value):0;
    if (!length || length>128) return 0;
    for (i=0;i<length;++i) {
        unsigned c=(unsigned char)value[i];
        if (!((c>='a' && c<='z') || (c>='A' && c<='Z') || (c>='0' && c<='9') || (i && (c=='-' || c=='_')))) return 0;
    }
    return 1;
}
static int error(Decisions *result, const GD_Record *record, const char *code, const char *message, int line) {
    gd_error(result->model,code,message,record->source_path,line); return 0;
}
static int is_null(const yaml_node_t *node) {
    const char *value;
    if (!node || node->type!=YAML_SCALAR_NODE || node->data.scalar.style!=YAML_PLAIN_SCALAR_STYLE) return 0;
    value=(const char *)node->data.scalar.value;
    return !*value || !strcmp(value,"null") || !strcmp(value,"Null") || !strcmp(value,"NULL") || !strcmp(value,"~");
}
static int is_string(const yaml_node_t *node) {
    const char *value,*number; char *end;
    if (!node || node->type!=YAML_SCALAR_NODE || memchr(node->data.scalar.value,0,node->data.scalar.length)) return 0;
    if (node->data.scalar.style!=YAML_PLAIN_SCALAR_STYLE) return 1;
    value=(const char *)node->data.scalar.value;
    if (is_null(node) || !strcmp(value,"true") || !strcmp(value,"false") || !strcmp(value,"True") || !strcmp(value,"False") ||
        !strcmp(value,"TRUE") || !strcmp(value,"FALSE")) return 0;
    number=value; if (*number=='+' || *number=='-') ++number;
    if (!strcmp(number,".inf") || !strcmp(number,".Inf") || !strcmp(number,".INF") || !strcmp(number,".nan") || !strcmp(number,".NaN") || !strcmp(number,".NAN")) return 0;
    if (number[0]=='0' && (number[1]=='o' || number[1]=='b')) {
        const char *digit=number+2; char maximum=number[1]=='o' ? '7':'1';
        if (*digit) {
            while (*digit>='0' && *digit<=maximum) ++digit;
            if (!*digit) return 0;
        }
    }
    strtod(value,&end); return end==value || *end;
}
static const char *retain(Decisions *result, const GD_Record *record, const yaml_node_t *node, int base) {
    char *value; size_t size;
    if (!is_string(node) || !node->data.scalar.length || node->data.scalar.length>=GD_MAX_PATH) {
        error(result,record,"decision.field_type","Assessment field needs nonempty text of at most 4095 bytes, not coercion",base+(node ? (int)node->start_mark.line:0)); return NULL;
    }
    size=node->data.scalar.length;
    if (size>=DECISION_TEXT-result->text_used) {
        error(result,record,"decision.text_limit","4 MiB retained assessment text exceeded",base+(int)node->start_mark.line); return NULL;
    }
    value=result->text+result->text_used; memcpy(value,node->data.scalar.value,size); value[size]=0; result->text_used+=size+1; return value;
}
/* The vendored libyaml subset provides events/document primitives, not its
   recursive loader. Build a bounded document iteratively; parse YAML only once. */
static int load_document(Decisions *result, const GD_Record *record, const char *text, size_t size, int base, yaml_document_t *document) {
    typedef struct { int node, key, mapping; } Frame;
    Frame stack[DECISION_DEPTH];
    yaml_parser_t parser; yaml_event_t event; int events=0, depth=0, documents=0, root=0, valid=1, done=0;
    if (size>GD_MAX_FRONTMATTER) return error(result,record,"decision.block_limit","Assessment block exceeds 64 KiB",base);
    if (!yaml_parser_initialize(&parser)) return error(result,record,"decision.allocation","Cannot initialize assessment YAML reader",base);
    if (!yaml_document_initialize(document,NULL,NULL,NULL,1,1)) { yaml_parser_delete(&parser); return error(result,record,"decision.allocation","Cannot initialize assessment document",base); }
    yaml_parser_set_input_string(&parser,(const unsigned char *)text,size);
    while (!done && valid && events<DECISION_EVENTS) {
        int node=0,container=0,mapping=0;
        if (!yaml_parser_parse(&parser,&event)) { valid=error(result,record,"decision.yaml","Invalid assessment YAML",base+(int)parser.problem_mark.line); break; }
        ++events;
        if (event.type==YAML_DOCUMENT_START_EVENT && ++documents>1) valid=0;
        if (event.type==YAML_ALIAS_EVENT) valid=0;
        if (event.type==YAML_SCALAR_EVENT) {
            if (event.data.scalar.anchor || event.data.scalar.tag) valid=0;
            else node=yaml_document_add_scalar(document,NULL,event.data.scalar.value,(int)event.data.scalar.length,event.data.scalar.style);
        } else if (event.type==YAML_MAPPING_START_EVENT) {
            container=1; mapping=1;
            if (event.data.mapping_start.anchor || event.data.mapping_start.tag) valid=0;
            else node=yaml_document_add_mapping(document,NULL,event.data.mapping_start.style);
        } else if (event.type==YAML_SEQUENCE_START_EVENT) {
            container=1;
            if (event.data.sequence_start.anchor || event.data.sequence_start.tag) valid=0;
            else node=yaml_document_add_sequence(document,NULL,event.data.sequence_start.style);
        }
        if (valid && (event.type==YAML_SCALAR_EVENT || container)) {
            if (!node) valid=error(result,record,"decision.allocation","Cannot retain bounded assessment YAML node",base+(int)event.start_mark.line);
            else {
                yaml_node_t *retained=yaml_document_get_node(document,node);
                retained->start_mark=event.start_mark;
                if (!depth) { if (root) valid=0; else root=node; }
                else {
                    Frame *parent=&stack[depth-1];
                    if (!parent->mapping) valid=yaml_document_append_sequence_item(document,parent->node,node);
                    else if (!parent->key) parent->key=node;
                    else { valid=yaml_document_append_mapping_pair(document,parent->node,parent->key,node); parent->key=0; }
                }
                if (!valid && !result->model->error_count) error(result,record,"decision.document","Cannot attach assessment YAML node",base+(int)event.start_mark.line);
                if (valid && container) {
                    if (depth==DECISION_DEPTH) valid=0;
                    else { stack[depth].node=node; stack[depth].key=0; stack[depth].mapping=mapping; ++depth; }
                }
            }
        } else if (event.type==YAML_SEQUENCE_END_EVENT || event.type==YAML_MAPPING_END_EVENT) {
            if (!depth || stack[depth-1].key) valid=0;
            else --depth;
        }
        done=event.type==YAML_STREAM_END_EVENT;
        if (!valid && !result->model->error_count) error(result,record,"decision.yaml_contract","Use one bounded YAML document without aliases, anchors or explicit tags",base+(int)event.start_mark.line);
        yaml_event_delete(&event);
    }
    if (valid && !done) valid=error(result,record,"decision.event_limit","Assessment exceeds 2048 YAML events",base);
    yaml_parser_delete(&parser);
    if (!valid) yaml_document_delete(document);
    return valid;
}
static int parse_demands(Decisions *result, const GD_Record *record, yaml_document_t *document, yaml_node_t *sequence, Assessment *assessment, int base) {
    yaml_node_item_t *entry;
    if (!sequence || sequence->type!=YAML_SEQUENCE_NODE) return error(result,record,"decision.demands","demands requires a sequence",base);
    for (entry=sequence->data.sequence.items.start;entry<sequence->data.sequence.items.top;++entry) {
        yaml_node_t *mapping=yaml_document_get_node(document,*entry); yaml_node_pair_t *pair;
        Demand demand={NULL,2,base}; int seen=0, i;
        if (assessment->demand_count==DECISION_ENTRIES || result->demand_count==DECISION_TOTAL_ENTRIES)
            return error(result,record,"decision.demand_limit","At most 16 demands per assessment and 32768 total",base);
        if (!mapping || mapping->type!=YAML_MAPPING_NODE) return error(result,record,"decision.demand_type","Demand entry needs a mapping",base);
        demand.line=base+(int)mapping->start_mark.line;
        for (pair=mapping->data.mapping.pairs.start;pair<mapping->data.mapping.pairs.top;++pair) {
            yaml_node_t *key=yaml_document_get_node(document,pair->key), *value=yaml_document_get_node(document,pair->value); int field;
            if (!is_string(key)) return error(result,record,"decision.demand_field","Demand keys must be kind or intent",demand.line);
            field=!strcmp((const char *)key->data.scalar.value,"kind") ? 1:!strcmp((const char *)key->data.scalar.value,"intent") ? 2:0;
            if (!field || (seen&field)) return error(result,record,"decision.demand_field","Unknown or duplicate demand field",base+(int)key->start_mark.line);
            seen|=field;
            if (field==1) {
                demand.kind=retain(result,record,value,base); if (!demand.kind) return 0;
                if (!technical(demand.kind)) return error(result,record,"decision.demand_kind","Demand kind needs a technical name of at most 128 bytes",demand.line);
            } else {
                if (!is_string(value) || (demand.intent=intent_index((const char *)value->data.scalar.value))<0)
                    return error(result,record,"decision.intent","Intent needs wanted, unwanted or undecided; omit for undecided",base+(int)value->start_mark.line);
            }
        }
        if (!(seen&1)) return error(result,record,"decision.demand_kind","Demand entry needs kind",demand.line);
        for (i=assessment->first_demand;i<result->demand_count;++i) if (!strcmp(demand.kind,result->demands[i].kind))
            return error(result,record,"decision.duplicate_demand","Duplicate demand kind within assessment",demand.line);
        result->demands[result->demand_count++]=demand; ++assessment->demand_count;
    }
    return 1;
}
static int parse_block(Decisions *result, int record_index, const char *text, size_t size, int base) {
    const GD_Record *record=&result->model->records[record_index];
    yaml_document_t document; yaml_node_t *mapping; yaml_node_pair_t *pair;
    Assessment assessment={0}; int valid=1, seen=0, i;
    static const char *const keys[]={"id","task","context","evidence_status","rule_cases","demands"};
    if (result->count==DECISION_COUNT) return error(result,record,"decision.assessment_limit","4096 total assessments exceeded",base);
    if (!load_document(result,record,text,size,base,&document)) return 0;
    mapping=yaml_document_get_root_node(&document);
    assessment.record=record_index; assessment.line=base-1; assessment.first_demand=result->demand_count; assessment.first_reference=result->reference_count;
    if (!mapping || mapping->type!=YAML_MAPPING_NODE) valid=error(result,record,"decision.mapping","Assessment requires one mapping",base);
    if (valid) for (pair=mapping->data.mapping.pairs.start;pair<mapping->data.mapping.pairs.top && valid;++pair) {
        yaml_node_t *key=yaml_document_get_node(&document,pair->key), *value=yaml_document_get_node(&document,pair->value); int field=-1;
        if (is_string(key)) for (i=0;i<6;++i) if (!strcmp((const char *)key->data.scalar.value,keys[i])) { field=i; break; }
        if (field<0 || (seen&(1<<field))) { valid=error(result,record,"decision.field","Unknown or duplicate assessment field",base+(int)key->start_mark.line); break; }
        seen|=1<<field;
        if (field==4) {
            yaml_node_item_t *reference;
            if (value->type!=YAML_SEQUENCE_NODE) { valid=error(result,record,"decision.references","rule_cases requires a sequence of authored references",base+(int)value->start_mark.line); break; }
            for (reference=value->data.sequence.items.start;reference<value->data.sequence.items.top && valid;++reference) {
                const char *retained;
                if (assessment.reference_count==DECISION_ENTRIES || result->reference_count==DECISION_TOTAL_ENTRIES) { valid=error(result,record,"decision.reference_limit","At most 16 references per assessment and 32768 total",base); break; }
                retained=retain(result,record,yaml_document_get_node(&document,*reference),base);
                if (!retained) { valid=0; break; }
                result->references[result->reference_count++]=retained; ++assessment.reference_count;
            }
        } else if (field==5) valid=parse_demands(result,record,&document,value,&assessment,base);
        else {
            const char *retained=NULL;
            if ((field!=2 && field!=3) || !is_null(value)) { retained=retain(result,record,value,base); if (!retained) { valid=0; break; } }
            if (field==0) assessment.id=retained;
            else if (field==1) assessment.task=retained;
            else if (field==2) assessment.context=retained;
            else assessment.evidence=retained;
        }
    }
    if (valid && (!technical(assessment.id) || !assessment.task || !(seen&(1<<5)))) valid=error(result,record,"decision.required","Assessment requires technical id, task and demands sequence",base);
    if (valid) for (i=result->count-1;i>=0 && result->assessments[i].record==record_index;--i) if (!strcmp(result->assessments[i].id,assessment.id)) {
        valid=error(result,record,"decision.duplicate_id","Assessment id repeats within Challenge",base); break;
    }
    if (valid) result->assessments[result->count++]=assessment;
    yaml_document_delete(&document); return valid;
}
/* Match ordinary Markdown fences, including other languages so nested examples
   do not become authoritative assessments. At most three leading spaces. */
static size_t fence(const char *text, size_t size, char *marker, const char **tail, size_t *tail_size) {
    size_t indent=0,width=0,offset;
    while (indent<size && text[indent]==' ') ++indent;
    if (indent>3 || indent==size || (text[indent]!='`' && text[indent]!='~')) return 0;
    *marker=text[indent]; while (indent+width<size && text[indent+width]==*marker) ++width;
    if (width<3) return 0;
    offset=indent+width; while (offset<size && (text[offset]==' ' || text[offset]=='\t')) ++offset;
    *tail=text+offset; *tail_size=size-offset; return width;
}
static int read_record(Decisions *result, int index) {
    const GD_Record *record=&result->model->records[index]; const char *cursor=record->body ? record->body:"", *start=NULL;
    size_t width=0; char marker=0; int assessment=0,base=0,line=record->body_line,local=0;
    while (*cursor) {
        const char *end=strchr(cursor,'\n'), *tail; size_t size=end ? (size_t)(end-cursor):strlen(cursor), length, tail_size; char current;
        if (size && cursor[size-1]=='\r') --size;
        while (size && (cursor[size-1]==' ' || cursor[size-1]=='\t')) --size;
        length=fence(cursor,size,&current,&tail,&tail_size);
        if (width && length>=width && current==marker && !tail_size) {
            if (assessment && !parse_block(result,index,start,(size_t)(cursor-start),base)) return 0;
            width=0; assessment=0;
        } else if (!width && length) {
            width=length; marker=current; assessment=tail_size==19 && !memcmp(tail,"decision-assessment",19);
            if (assessment) {
                if (++local>DECISION_LOCAL) return error(result,record,"decision.local_limit","16 assessments per Challenge exceeded",line);
                start=end ? end+1:cursor+strlen(cursor); base=line+1;
            }
        }
        if (!end) break; cursor=end+1; ++line;
    }
    if (assessment) return error(result,record,"decision.unclosed_block","Close decision-assessment Markdown fence",base-1);
    return 1;
}
static int entry_matches(const Demand *entry, const char *demand, int intent) {
    assert(entry && entry->kind && entry->intent>=0 && entry->intent<3 && intent>=-1 && intent<3);
    return (!demand || !strcmp(demand,entry->kind)) && (intent<0 || intent==entry->intent);
}
static int matches(const Decisions *result, const Assessment *assessment, const char *demand, int intent, int *entries) {
    int i; *entries=0;
    for (i=assessment->first_demand;i<assessment->first_demand+assessment->demand_count;++i) if (entry_matches(&result->demands[i],demand,intent)) ++*entries;
    return (!demand && intent<0) || *entries;
}
static int fields_valid(const char *fields) {
    const char *p=fields;
    if (!p) return 1; if (!*p) return 0;
    while (*p) {
        const char *end=strchr(p,','); size_t size=end ? (size_t)(end-p):strlen(p);
        if (!((size==7 && !memcmp(p,"summary",7)) || (size==11 && !memcmp(p,"assessments",11)))) return 0;
        if (!end) return 1; p=end+1; if (!*p) return 0;
    }
    return 0;
}
static int selected(const char *fields, const char *key) {
    const char *p=fields; size_t size=strlen(key);
    if (!p) return 1;
    while (*p) { const char *end=strchr(p,','); if ((end ? (size_t)(end-p):strlen(p))==size && !memcmp(p,key,size)) return 1; if (!end) break; p=end+1; }
    return 0;
}
static void member(FILE *out, const char *key, const char *value) { fputc(',',out); gd_json_string(out,key); fputc(':',out); gd_json_string(out,value); }
static void write_row(FILE *out, const Decisions *result, const Assessment *assessment, const char *demand, int intent) {
    const GD_Record *record=&result->model->records[assessment->record]; int i;
    fputs("{\"record\":",out); gd_json_string(out,record->reference); member(out,"record_id",record->id); member(out,"status",record->status);
    member(out,"assessment_id",assessment->id); member(out,"task",assessment->task); member(out,"context",assessment->context); member(out,"evidence_status",assessment->evidence);
    member(out,"source_path",record->source_path); fprintf(out,",\"line\":%d,\"rule_cases\":[",assessment->line);
    for (i=0;i<assessment->reference_count;++i) { if (i) fputc(',',out); gd_json_string(out,result->references[assessment->first_reference+i]); }
    fputs("],\"demands\":[",out);
    for (i=0;i<assessment->demand_count;++i) {
        const Demand *entry=&result->demands[assessment->first_demand+i]; if (i) fputc(',',out);
        fputs("{\"kind\":",out); gd_json_string(out,entry->kind); member(out,"intent",intents[entry->intent]);
        fprintf(out,",\"matched\":%s,\"line\":%d}",entry_matches(entry,demand,intent) ? "true":"false",entry->line);
    }
    fputs("]}",out);
}
int gd_decisions_cli(GD_Allocator allocator, const GD_Collection *collections, int count, const char *projection,
                     const char *demand, const char *intent, int json, const char *fields, const char *initial_error) {
    GD_Model *model=gd_create(allocator); Decisions *result=NULL; int i,complete=0,rows=0,entries=0,intent_value=intent_index(intent),comma=0,status;
    if (!model) {
        if (json) {
            fputs("{\"schema_version\":1,\"ok\":false,\"data\":{",stdout);
            if (selected(fields,"summary")) { fputs("\"summary\":null",stdout); comma=1; }
            if (selected(fields,"assessments")) { if (comma) fputc(',',stdout); fputs("\"assessments\":[]",stdout); }
            fputs("},\"question\":\"decision-assessments\",\"projection\":\"challenges\",\"parameters\":{\"demand\":",stdout); gd_json_string(stdout,demand); member(stdout,"intent",intent);
            fputs("},\"coverage\":{\"complete\":false,\"read_only\":true,\"classification_inferred\":false},\"errors\":[{\"code\":\"decision.allocation\",\"message\":\"Cannot allocate diagnostic snapshot\",\"source_path\":null,\"line\":0}],\"errors_omitted\":0,\"breadcrumbs\":[\"gd query --list\"]}\n",stdout);
        }
        fputs("gd: cannot allocate decision diagnostic snapshot\n",stderr); return 2;
    }
    if (initial_error) gd_error(model,"cli.arguments",initial_error,NULL,0);
    else if (!projection || strcmp(projection,"challenges")) gd_error(model,"query.projection_unavailable","decision-assessments requires projection challenges",NULL,0);
    else if (count!=1 || collections[0].role!=GD_CHALLENGE) gd_error(model,"cli.arguments","decision-assessments requires exactly one --challenges ALIAS=FOLDER binding",NULL,0);
    else if ((demand && !technical(demand)) || (intent && intent_value<0)) gd_error(model,"cli.arguments","Use technical --demand (at most 128 bytes) and --intent wanted|unwanted|undecided",NULL,0);
    else if (!fields_valid(fields) || (fields && !json)) gd_error(model,"cli.arguments","--fields needs JSON keys summary,assessments",NULL,0);
    else if (gd_load_challenges(model,collections,count)) {
        result=allocator.allocate(allocator.context,sizeof(*result));
        if (!result) gd_error(model,"decision.allocation","Cannot allocate bounded assessment result",NULL,0);
        else {
            memset(result,0,sizeof(*result)); result->model=model;
            for (i=0;i<model->record_count;++i) if (model->records[i].role==GD_CHALLENGE && !read_record(result,i)) break;
            complete=!model->error_count && !model->errors_omitted;
        }
    }
    if (complete) for (i=0;i<result->count;++i) { int matched; if (matches(result,&result->assessments[i],demand,intent_value,&matched)) { ++rows; entries+=matched; } }
    if (json) {
        fprintf(stdout,"{\"schema_version\":1,\"ok\":%s,\"data\":{",complete ? "true":"false");
        if (selected(fields,"summary")) {
            fputs("\"summary\":",stdout);
            if (!complete) fputs("null",stdout);
            else fprintf(stdout,"{\"source_records\":%d,\"retained_assessments\":%d,\"retained_demand_entries\":%d,\"matched_assessments\":%d,\"matched_demand_entries\":%d}",model->record_count,result->count,result->demand_count,rows,entries);
            comma=1;
        }
        if (selected(fields,"assessments")) {
            int emitted=0; if (comma) fputc(',',stdout); fputs("\"assessments\":[",stdout);
            if (complete) for (i=0;i<result->count && !ferror(stdout);++i) { int matched; if (!matches(result,&result->assessments[i],demand,intent_value,&matched)) continue; if (emitted++) fputc(',',stdout); write_row(stdout,result,&result->assessments[i],demand,intent_value); }
            fputc(']',stdout);
        }
        fputs("},\"question\":\"decision-assessments\",\"projection\":\"challenges\",\"parameters\":{\"demand\":",stdout); gd_json_string(stdout,demand); member(stdout,"intent",intent);
        fprintf(stdout,"},\"coverage\":{\"complete\":%s,\"read_only\":true,\"classification_inferred\":false,\"scope\":\"Explicit authored assessment blocks only. Intent/evidence are declared claims; references do not prove applicability. Empty matches do not establish absent gameplay demands. Counts are assessments/demand entries, not frequency, difficulty or quality.\",\"limits\":{\"assessments\":4096,\"per_record\":16,\"demands_per_assessment\":16,\"references_per_assessment\":16,\"demands\":32768,\"references\":32768,\"block_bytes\":65536,\"yaml_events\":2048,\"yaml_depth\":8,\"text_bytes\":4194304}},\"errors\":[",complete ? "true":"false");
        for (i=0;i<model->error_count;++i) { const GD_Error *item=&model->errors[i]; if (i) fputc(',',stdout); fputs("{\"code\":",stdout); gd_json_string(stdout,item->code); member(stdout,"message",item->message); member(stdout,"source_path",item->path); fprintf(stdout,",\"line\":%d}",item->line); }
        fprintf(stdout,"],\"errors_omitted\":%d,\"breadcrumbs\":[\"gd query --list\",\"Review authoritative Challenge assessment and whether each demand is wanted\"]}\n",model->errors_omitted);
    } else if (complete) {
        fprintf(stdout,"decision-assessments [challenges]: %d assessments; %d matching demand entries\n",rows,entries);
        for (i=0;i<result->count;++i) {
            int matched,j; const Assessment *item=&result->assessments[i];
            if (!matches(result,item,demand,intent_value,&matched)) continue;
            fprintf(stdout,"%s#%s | %s | %s:%d\n",model->records[item->record].reference,item->id,item->task,model->records[item->record].source_path,item->line);
            fprintf(stdout,"  Context: %s | Declared evidence: %s\n",item->context ? item->context:"unknown",item->evidence ? item->evidence:"unknown");
            for (j=0;j<item->demand_count;++j) {
                const Demand *entry=&result->demands[item->first_demand+j];
                fprintf(stdout,"  %s: %s%s\n",entry->kind,intents[entry->intent],entry_matches(entry,demand,intent_value) ? " [match]":"");
            }
            for (j=0;j<item->reference_count;++j) fprintf(stdout,"  Declared reference: %s\n",result->references[item->first_reference+j]);
        }
    }
    for (i=0;i<model->error_count;++i) fprintf(stderr,"gd: %s: %s | %s:%d\n",model->errors[i].code,model->errors[i].message,model->errors[i].path ? model->errors[i].path:"runtime",model->errors[i].line);
    status=complete && !ferror(stdout) && !fflush(stdout) ? 0:2;
    if (result) allocator.release(allocator.context,result);
    gd_destroy(model); return status;
}
int gd_write_decision_catalogue(FILE *out, int json, int comma) {
    if (!json) fputs("decision-assessments [challenges]: --challenges required; optional exact --demand, --intent wanted|unwanted|undecided; explicit authored blocks, CLI only\n",out);
    else {
        if (comma) fputc(',',out);
        fputs("{\"name\":\"decision-assessments\",\"projection\":\"challenges\",\"required_bindings\":{\"all_of\":[\"challenges\"]},\"required_parameters\":[],\"optional_parameters\":[\"--demand\",\"--intent\"],\"repeatable_parameters\":[],\"fields\":[\"summary\",\"assessments\"],\"ui_available\":false,\"ipc_available\":false,\"scope\":\"Explicit per-situation authored performance/intent assessments; no automatic classification or applicability evaluation\"}",out);
    }
    return !ferror(out);
}
