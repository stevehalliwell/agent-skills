#include "gd.h"
#include "yaml.h"
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
/* CJIT 1.2.0 ships older Win32 headers; value is defined by the Windows API. */
#ifndef WC_ERR_INVALID_CHARS
#define WC_ERR_INVALID_CHARS 0x00000080
#endif
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

static const char *field_names[] = {
    "nouns", "verb", "context_nouns", "mechanics", "control-vars", "resources", "verbs", "prerequisites",
    "introduces", "practises", "requires", "tests_mastery", "next", "systems", "dynamics", "experiments", "axes", "context_axes"
};
enum { RANK_KEY = 5 + GD_CONTEXT_AXES + 1, ENTRY_KEY, CONTROL_KEY, AXIS_CLOSED_KEY = CONTROL_KEY + 7, RESOURCE_NOUN_KEY }; /* Consumed-field indices; byte markers avoid width-dependent bit shifts. */
const char *gd_field_name(GD_Field field) { return field_names[field]; }

GD_Model *gd_create(GD_Allocator allocator) {
    GD_Model *model;
    assert(allocator.allocate && allocator.release);
    model = allocator.allocate(allocator.context, sizeof(*model));
    if (model) { memset(model, 0, sizeof(*model)); model->allocator = allocator; }
    return model;
}
void gd_destroy(GD_Model *model) {
    if (model) model->allocator.release(model->allocator.context, model);
}
void gd_error(GD_Model *model, const char *code, const char *message,
              const char *path, int line) {
    if (model->error_count < GD_MAX_ERRORS) {
        GD_Error *error = &model->errors[model->error_count++];
        error->code = code; error->message = message;
        error->path = path; error->line = line;
    } else model->errors_omitted++;
}
static const char *copy_text(GD_Model *model, const char *text, size_t size) {
    char *result;
    if (size >= GD_TEXT_CAPACITY - model->text_used) {
        gd_error(model, "input.text_limit", "16 MiB retained-text limit exceeded", NULL, 0);
        return NULL;
    }
    result = model->text + model->text_used;
    memcpy(result, text, size); result[size] = 0;
    model->text_used += size + 1;
    return result;
}
static uint64_t hash_bytes(uint64_t hash, const char *bytes, size_t size) {
    size_t i;
    for (i = 0; i < size; ++i) { hash ^= (unsigned char)bytes[i]; hash *= UINT64_C(1099511628211); }
    return hash;
}
int gd_valid_utf8(const unsigned char *text, size_t size) {
    size_t i = 0;
    while (i < size) {
        unsigned c = text[i++], count, value, minimum;
        if (!c) return 0;
        if (c < 128) continue;
        if (c >= 194 && c <= 223) { count = 1; value = c & 31; minimum = 128; }
        else if (c >= 224 && c <= 239) { count = 2; value = c & 15; minimum = 2048; }
        else if (c >= 240 && c <= 244) { count = 3; value = c & 7; minimum = 65536; }
        else return 0;
        if (count > size - i) return 0;
        while (count--) {
            c = text[i++]; if ((c & 192) != 128) return 0;
            value = (value << 6) | (c & 63);
        }
        if (value < minimum || value > 0x10ffff || (value >= 0xd800 && value <= 0xdfff)) return 0;
    }
    return 1;
}
static int technical_name(const char *name) {
    size_t i, size = strlen(name);
    if (!size || size > 128) return 0;
    for (i = 0; i < size; ++i) {
        unsigned c = (unsigned char)name[i];
        if (!(c >= 'a' && c <= 'z') && !(c >= 'A' && c <= 'Z') &&
            !(c >= '0' && c <= '9') && (i == 0 || (c != '-' && c != '_'))) return 0;
    }
    return 1;
}
static FILE *source_open(const char *path) {
#ifdef _WIN32
    wchar_t wide[GD_MAX_PATH];
    if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path, -1, wide, GD_MAX_PATH)) return NULL;
    return _wfopen(wide, L"rb");
#else
    return fopen(path, "rb");
#endif
}
static int read_file(GD_Model *model, const char *path, size_t *size, int checking) {
    FILE *file = source_open(path);
    long length;
    int valid = 0;
    if (!file) {
        if (!checking) gd_error(model, "input.read", "Cannot open source file", path, 0);
        return 0;
    }
    if (!fseek(file, 0, SEEK_END) && (length = ftell(file)) >= 0 && length <= GD_MAX_FILE &&
        !fseek(file, 0, SEEK_SET)) {
        *size = (size_t)length;
        valid = fread(model->input, 1, *size, file) == *size && fgetc(file) == EOF && !ferror(file);
        model->input[*size] = 0;
    }
    fclose(file);
    if (!valid && !checking)
        gd_error(model, "input.read_limit", "Cannot read a stable file within the 1 MiB file limit", path, 0);
    return valid;
}

int gd_read_text(GD_Model *model, const char *path, size_t *size) {
    if (!read_file(model,path,size,0)) return 0;
    if (!gd_valid_utf8((const unsigned char *)model->input,*size)) {
        gd_error(model,"input.utf8","Input must be UTF-8 text without NUL",path,0); return 0;
    }
    return 1;
}

/* No implicit YAML type coercion for consumed text/reference fields. */
static int scalar_null(const yaml_event_t *event) {
    const char *s = (const char *)event->data.scalar.value;
    if (event->data.scalar.tag && strcmp((const char *)event->data.scalar.tag, "tag:yaml.org,2002:null")) return 0;
    if (memchr(event->data.scalar.value, 0, event->data.scalar.length)) return 0;
    return (event->data.scalar.tag || event->data.scalar.style == YAML_PLAIN_SCALAR_STYLE) &&
        (!*s || !strcmp(s, "~") || !strcmp(s, "null") || !strcmp(s, "Null") || !strcmp(s, "NULL"));
}
static int scalar_string(const yaml_event_t *event) {
    const char *s = (const char *)event->data.scalar.value;
    char *end;
    if (event->data.scalar.tag && strcmp((const char *)event->data.scalar.tag, "tag:yaml.org,2002:str")) return 0;
    if (event->data.scalar.tag || event->data.scalar.style != YAML_PLAIN_SCALAR_STYLE) return 1;
    if (scalar_null(event) || !strcmp(s, "true") || !strcmp(s, "false") ||
        !strcmp(s, "True") || !strcmp(s, "False") || !strcmp(s, "TRUE") || !strcmp(s, "FALSE") ||
        !strcmp(s, ".inf") || !strcmp(s, ".nan") || !strcmp(s, ".Inf") || !strcmp(s, ".NaN")) return 0;
    strtod(s, &end);
    return end == s || *end != 0;
}
typedef struct { GD_Model *model; yaml_parser_t parser; const char *path; int events; } Reader;
static int next_event(Reader *reader, yaml_event_t *event) {
    if (++reader->events > 65536) {
        gd_error(reader->model, "input.yaml_limit", "Frontmatter event limit exceeded", reader->path, 0); return 0;
    }
    if (!yaml_parser_parse(&reader->parser, event)) {
        const char *message = copy_text(reader->model, reader->parser.problem ? reader->parser.problem : "Invalid YAML",
                                        strlen(reader->parser.problem ? reader->parser.problem : "Invalid YAML"));
        gd_error(reader->model, "input.yaml", message ? message : "Invalid YAML", reader->path,
                 (int)reader->parser.problem_mark.line + 2);
        return 0;
    }
    return 1;
}
/* Iterative skip of unconsumed metadata, not a schema interpreter. */
static int skip_value(Reader *reader, yaml_event_t *event) {
    int depth = event->type == YAML_SEQUENCE_START_EVENT || event->type == YAML_MAPPING_START_EVENT;
    yaml_event_delete(event);
    while (depth) {
        if (!next_event(reader, event)) return 0;
        if (event->type == YAML_SEQUENCE_START_EVENT || event->type == YAML_MAPPING_START_EVENT) ++depth;
        if (event->type == YAML_SEQUENCE_END_EVENT || event->type == YAML_MAPPING_END_EVENT) --depth;
        if (depth > 32 || event->type == YAML_STREAM_END_EVENT) {
            yaml_event_delete(event);
            gd_error(reader->model, "input.yaml_depth", "Unsupported metadata depth", reader->path, 0); return 0;
        }
        yaml_event_delete(event);
    }
    return 1;
}
static int consume_string(Reader *reader, yaml_event_t *event, const char **value) {
    if (event->type == YAML_SCALAR_EVENT && memchr(event->data.scalar.value, 0, event->data.scalar.length)) {
        gd_error(reader->model, "input.field_nul", "Consumed text cannot contain an escaped NUL", reader->path,
                 (int)event->start_mark.line + 2); return 0;
    }
    if (event->type == YAML_SCALAR_EVENT && scalar_null(event)) { *value = NULL; return 1; }
    if (event->type != YAML_SCALAR_EVENT || !scalar_string(event)) {
        gd_error(reader->model, "input.field_type", "Consumed field requires text, not coercion or an alias",
                 reader->path, (int)event->start_mark.line + 2); return 0;
    }
    *value = copy_text(reader->model, (const char *)event->data.scalar.value, event->data.scalar.length);
    return *value != NULL;
}
static int add_edge(Reader *reader, int source, GD_Field field, yaml_event_t *event) {
    const char *value = NULL;
    GD_Edge *edge;
    if (!consume_string(reader, event, &value) || !value || !*value || strlen(value) > 257) {
        gd_error(reader->model, "input.reference", "Relationship item requires a nonempty alias/name string",
                 reader->path, (int)event->start_mark.line + 2); return 0;
    }
    if (reader->model->edge_count == GD_MAX_EDGES) {
        gd_error(reader->model, "input.edge_limit", "32768 relationship limit exceeded", reader->path, 0); return 0;
    }
    edge = &reader->model->edges[reader->model->edge_count++];
    edge->source = source; edge->target = -1; edge->field = field; edge->reference = value;
    edge->line = (int)event->start_mark.line + 2;
    return 1;
}
static int consume_links(Reader *reader, int source, GD_Field field, yaml_event_t *event) {
    if (event->type == YAML_SCALAR_EVENT && scalar_null(event)) { yaml_event_delete(event); return 1; }
    if (field == GD_VERB_REF) {
        int ok = add_edge(reader, source, field, event); yaml_event_delete(event); return ok;
    }
    if (event->type != YAML_SEQUENCE_START_EVENT) {
        gd_error(reader->model, "input.field_type", "Relationship field requires a sequence or null", reader->path,
                 (int)event->start_mark.line + 2); yaml_event_delete(event); return 0;
    }
    yaml_event_delete(event);
    for (;;) {
        int ok;
        if (!next_event(reader, event)) return 0;
        if (event->type == YAML_SEQUENCE_END_EVENT) { yaml_event_delete(event); return 1; }
        ok = add_edge(reader, source, field, event); yaml_event_delete(event);
        if (!ok) return 0;
    }
}
static int consume_rank(Reader *reader, yaml_event_t *event, GD_Record *record) {
    const unsigned char *text;
    size_t i = 0;
    uint64_t magnitude = 0, limit = UINT64_C(9007199254740991);
    int negative = 0;
    if (event->type != YAML_SCALAR_EVENT) goto invalid;
    if (scalar_null(event)) return 1;
    if ((event->data.scalar.tag && strcmp((const char *)event->data.scalar.tag, "tag:yaml.org,2002:int")) ||
        (!event->data.scalar.tag && event->data.scalar.style != YAML_PLAIN_SCALAR_STYLE)) goto invalid;
    text = event->data.scalar.value;
    if (i < event->data.scalar.length && (text[i] == '-' || text[i] == '+')) negative = text[i++] == '-';
    if (i == event->data.scalar.length) goto invalid;
    for (; i < event->data.scalar.length; ++i) {
        unsigned digit = text[i] - '0';
        if (digit > 9 || magnitude > (limit-digit)/10) goto invalid;
        magnitude = magnitude*10+digit;
    }
    record->has_rank = 1; record->rank = negative ? -(int64_t)magnitude : (int64_t)magnitude;
    record->rank_line = (int)event->start_mark.line+2;
    return 1;
invalid:
    gd_error(reader->model, "skill.rank_type", "Rank requires null or a JSON-safe signed decimal integer; no coercion", reader->path, (int)event->start_mark.line+2);
    return 0;
}
static int consume_boolean(Reader *reader, yaml_event_t *event, int *result, int *line,
                           const char *code, const char *message) {
    const char *value;
    if (event->type != YAML_SCALAR_EVENT ||
        memchr(event->data.scalar.value, 0, event->data.scalar.length) ||
        (event->data.scalar.tag ? strcmp((const char *)event->data.scalar.tag, "tag:yaml.org,2002:bool") :
         event->data.scalar.style != YAML_PLAIN_SCALAR_STYLE)) goto invalid;
    value = (const char *)event->data.scalar.value;
    if (!strcmp(value,"true") || !strcmp(value,"True") || !strcmp(value,"TRUE")) *result = 1;
    else if (!strcmp(value,"false") || !strcmp(value,"False") || !strcmp(value,"FALSE")) *result = 0;
    else goto invalid;
    *line = (int)event->start_mark.line+2; return 1;
invalid:
    gd_error(reader->model,code,message,reader->path,(int)event->start_mark.line+2);
    return 0;
}
static int consume_definition(Reader *reader, yaml_event_t *event, GD_ValueDefinition *definition, int field, int axis) {
    const char **destination = field == 0 ? &definition->type : field == 1 ? &definition->value :
        field == 2 ? &definition->expression : field == 3 ? &definition->unit :
        field == 5 ? &definition->minimum : &definition->maximum;
    definition->lines[field] = (int)event->start_mark.line + 2;
    if (field == 4) {
        if (event->type == YAML_SCALAR_EVENT && scalar_null(event)) { yaml_event_delete(event); return 1; }
        if (event->type != YAML_SEQUENCE_START_EVENT) goto invalid;
        yaml_event_delete(event);
        for (;;) {
            const char *value = NULL;
            if (!next_event(reader,event)) return 0;
            if (event->type == YAML_SEQUENCE_END_EVENT) { yaml_event_delete(event); return 1; }
            if (definition->allowed_count == GD_MAX_ENUM_VALUES) {
                gd_error(reader->model,axis ? "rules.axis_value_limit":"controls.enum_limit",axis ? "At most 64 named values per axis":"At most 64 enum values per control",reader->path,definition->lines[field]);
                yaml_event_delete(event); return 0;
            }
            if (!consume_string(reader,event,&value) || !value) {
                gd_error(reader->model,axis ? "rules.axis_value_type":"controls.enum_value","Enum entries require strings, not null",reader->path,(int)event->start_mark.line+2);
                yaml_event_delete(event); return 0;
            }
            if (!definition->allowed_count) definition->allowed = value;
            ++definition->allowed_count; yaml_event_delete(event);
        }
    }
    if (field == 1 || field >= 5) {
        const char *text;
        if (event->type != YAML_SCALAR_EVENT || memchr(event->data.scalar.value,0,event->data.scalar.length)) goto invalid;
        if (scalar_null(event)) { yaml_event_delete(event); return 1; }
        text = (const char *)event->data.scalar.value;
        if (scalar_string(event)) {
            if (field != 1 || !consume_string(reader,event,destination)) goto invalid;
            definition->kind = GD_LITERAL_STRING;
        } else {
            double number;
            int boolean = !strcmp(text,"true") || !strcmp(text,"false") || !strcmp(text,"True") || !strcmp(text,"False") || !strcmp(text,"TRUE") || !strcmp(text,"FALSE");
            const char *tag = (const char *)event->data.scalar.tag;
            if (event->data.scalar.style != YAML_PLAIN_SCALAR_STYLE ||
                (tag && strcmp(tag,boolean ? "tag:yaml.org,2002:bool":"tag:yaml.org,2002:int") &&
                 (boolean || strcmp(tag,"tag:yaml.org,2002:float"))) ||
                (boolean ? field != 1 : !gd_control_number(text,&number,0))) goto invalid;
            *destination = copy_text(reader->model,text,event->data.scalar.length);
            if (!*destination) { yaml_event_delete(event); return 0; }
            if (field == 1) definition->kind = boolean ? GD_LITERAL_BOOLEAN : GD_LITERAL_NUMBER;
        }
        yaml_event_delete(event); return 1;
    }
    {
        int ok = consume_string(reader,event,destination); yaml_event_delete(event); return ok;
    }
invalid:
    gd_error(reader->model,axis ? "rules.axis_field_type":"controls.field_type",axis ? "Axis metadata requires typed scalars or a string state list; no coercion/aliases":"Control metadata requires typed scalars or a string enum list; no coercion/aliases",reader->path,definition->lines[field]);
    yaml_event_delete(event); return 0;
}
static int parse_metadata(GD_Model *model, int index, const char *start, size_t length) {
    Reader reader;
    yaml_event_t event;
    GD_Record *record = &model->records[index];
    unsigned char seen[RESOURCE_NOUN_KEY+1] = {0};
    int stage, success = 0;
    memset(&reader, 0, sizeof(reader)); reader.model = model; reader.path = record->source_path;
    if (!yaml_parser_initialize(&reader.parser)) {
        gd_error(model, "input.memory", "Cannot initialize YAML parser", record->source_path, 0); return 0;
    }
    yaml_parser_set_input_string(&reader.parser, (const unsigned char *)start, length);
    for (stage = 0; stage < 3; ++stage) {
        yaml_event_type_t expected = stage == 0 ? YAML_STREAM_START_EVENT :
                                    stage == 1 ? YAML_DOCUMENT_START_EVENT : YAML_MAPPING_START_EVENT;
        if (!next_event(&reader, &event)) goto done;
        if (event.type != expected) {
            yaml_event_delete(&event);
            gd_error(model, "input.frontmatter", "Frontmatter must be one mapping", record->source_path, 2); goto done;
        }
        yaml_event_delete(&event);
    }
    for (;;) {
        char key[64];
        int key_index = -1, i;
        const char **destination = NULL;
        if (!next_event(&reader, &event)) goto done;
        if (event.type == YAML_MAPPING_END_EVENT) { yaml_event_delete(&event); break; }
        if (event.type != YAML_SCALAR_EVENT) {
            yaml_event_delete(&event);
            gd_error(model, "input.frontmatter", "Frontmatter keys must be scalar", record->source_path, 0); goto done;
        }
        if (memchr(event.data.scalar.value, 0, event.data.scalar.length)) {
            yaml_event_delete(&event);
            gd_error(model, "input.field_nul", "Frontmatter key cannot contain an escaped NUL", record->source_path, 0); goto done;
        }
        if (event.data.scalar.length < sizeof(key)) {
            memcpy(key, event.data.scalar.value, event.data.scalar.length); key[event.data.scalar.length] = 0;
        } else key[0] = 0;
        yaml_event_delete(&event);
        if (!strcmp(key, "<<")) {
            gd_error(model, "input.yaml_merge", "YAML merges can alter consumed fields; expand them explicitly", record->source_path, 0); goto done;
        }
        if (!strcmp(key, "name")) { key_index = 0; destination = &record->name; }
        else if (!strcmp(key, "id")) { key_index = 1; destination = &record->id; }
        else if (!strcmp(key, "desc")) { key_index = 2; destination = &record->description; }
        else if (!strcmp(key, "status")) { key_index = 3; destination = &record->status; }
        else if (!strcmp(key, "coverage") && (record->role == GD_CASE || record->role == GD_RULE)) { key_index = 4; destination = &record->coverage; }
        else if (!strcmp(key, "rank") && record->role == GD_SKILL) key_index = RANK_KEY;
        else if (!strcmp(key, "entry") && record->role == GD_CHALLENGE) key_index = ENTRY_KEY;
        else if (!strcmp(key, "noun") && record->role == GD_RESOURCE) { key_index = RESOURCE_NOUN_KEY; destination = &record->resource_noun; }
        else if (!strcmp(key,"axes") && record->role == GD_NOUN) key_index = 5 + GD_AXES;
        else if (!strcmp(key,"context_axes") && (record->role == GD_RULE || record->role == GD_CASE)) key_index = 5 + GD_CONTEXT_AXES;
        else if (record->role == GD_AXIS && !strcmp(key,"closed")) key_index = AXIS_CLOSED_KEY;
        else if (record->role == GD_CONTROL || record->role == GD_AXIS) {
            static const char *keys[] = { "value_type", "value", "expression", "unit", "allowed_values", "minimum", "maximum" };
            for (i = 0; i < 7; ++i) if ((record->role == GD_CONTROL || (i != 1 && i != 2)) && !strcmp(key,keys[i])) { key_index = CONTROL_KEY+i; break; }
        }
        else if (gd_mda_role(record->role)) {
            for (i = 0; i <= GD_EXPERIMENTS; ++i) {
                int allowed = record->role == GD_MDA_MECHANIC ? (i == GD_NOUNS || i == GD_VERBS || i == GD_CONTROLS || i == GD_RESOURCES) :
                    record->role == GD_MDA_SYSTEM ? i == GD_MECHANICS :
                    record->role == GD_MDA_DYNAMIC ? (i == GD_MECHANICS || i == GD_SYSTEMS || i == GD_EXPERIMENTS) :
                    (i == GD_DYNAMICS || i == GD_EXPERIMENTS);
                if (allowed && !strcmp(key,field_names[i])) { key_index = 5+i; break; }
            }
        }
        else if (record->role == GD_CHALLENGE) {
            for (i = GD_INTRODUCES; i <= GD_NEXT; ++i) if (!strcmp(key, field_names[i])) { key_index = 5+i; break; }
        }
        else if (record->role == GD_SKILL && (!strcmp(key, "prerequisites") || !strcmp(key, "mechanics")))
            key_index = 5 + (!strcmp(key, "prerequisites") ? GD_PREREQUISITES : GD_MECHANICS);
        else if (record->role == GD_CASE || record->role == GD_RELATED || record->role == GD_RULE) {
            for (i = 0; i < 7; ++i) if (!strcmp(key, field_names[i])) { key_index = 5 + i; break; }
        }
        if (!next_event(&reader, &event)) goto done;
        if (key_index < 0) { if (!skip_value(&reader, &event)) goto done; continue; }
        if (seen[key_index]) {
            gd_error(model, "input.duplicate_field", "Duplicate consumed field is ambiguous", record->source_path,
                     (int)event.start_mark.line + 2); yaml_event_delete(&event); goto done;
        }
        seen[key_index] = 1;
        if (key_index == RESOURCE_NOUN_KEY) {
            int ok = consume_string(&reader,&event,destination); yaml_event_delete(&event); if (!ok) goto done;
        } else if (key_index == AXIS_CLOSED_KEY) {
            int ok = consume_boolean(&reader,&event,&record->axis_closed,&record->axis_closed_line,
                "rules.axis_closed_type","Axis closed requires boolean true or false; no coercion");
            yaml_event_delete(&event); if (!ok) goto done;
        } else if (key_index >= CONTROL_KEY) {
            int axis = record->role == GD_AXIS;
            if (!consume_definition(&reader,&event,axis ? &record->axis:&record->control,key_index-CONTROL_KEY,axis)) goto done;
        } else if (key_index == ENTRY_KEY) {
            int ok = consume_boolean(&reader,&event,&record->entry,&record->entry_line,
                "challenge.entry_type","Entry requires a boolean true or false; no coercion");
            yaml_event_delete(&event); if (!ok) goto done;
        } else if (key_index == RANK_KEY) {
            int ok = consume_rank(&reader, &event, record); yaml_event_delete(&event); if (!ok) goto done;
        } else if (destination) {
            int ok = consume_string(&reader, &event, destination); yaml_event_delete(&event); if (!ok) goto done;
        } else {
            if (key_index == 5 + GD_AXES) {
                record->axes_line = (int)event.start_mark.line+2;
                record->axes_declared = event.type == YAML_SEQUENCE_START_EVENT;
            } else if (key_index == 5 + GD_CONTEXT_AXES) {
                record->context_axes_line = (int)event.start_mark.line+2;
                record->context_axes_declared = event.type == YAML_SEQUENCE_START_EVENT;
            }
            if (!consume_links(&reader, index, (GD_Field)(key_index - 5), &event)) goto done;
        }
    }
    for (stage = 0; stage < 2; ++stage) {
        if (!next_event(&reader, &event)) goto done;
        if (event.type != (stage ? YAML_STREAM_END_EVENT : YAML_DOCUMENT_END_EVENT)) {
            yaml_event_delete(&event);
            gd_error(model, "input.frontmatter", "Multiple YAML documents are unsupported", record->source_path, 0); goto done;
        }
        yaml_event_delete(&event);
    }
    success = 1;
done:
    yaml_parser_delete(&reader.parser); return success;
}
static int fence(const char *s, size_t remaining, size_t *advance) {
    size_t n = 3;
    if (remaining < 3 || memcmp(s, "---", 3)) return 0;
    if (n < remaining && s[n] == '\r') ++n;
    if (n < remaining && s[n] != '\n') return 0;
    if (n < remaining) ++n;
    *advance = n; return 1;
}
static int load_record(GD_Model *model, int collection, const char *path, size_t size) {
    GD_Record *record;
    size_t offset = 0, advance, header = 0, body = 0, p;
    int index = model->record_count, edge_start = model->edge_count;
    char reference[260];
    const char *filename;
    if (index == GD_MAX_RECORDS) {
        gd_error(model, "input.record_limit", "4096 record limit exceeded", path, 0); return 0;
    }
    if (!gd_valid_utf8((const unsigned char *)model->input, size)) {
        gd_error(model, "input.utf8", "Source must be UTF-8 text without NUL", path, 0); return 0;
    }
    record = &model->records[index]; memset(record, 0, sizeof(*record));
    record->source_path = path; record->role = model->collections[collection].role;
    record->collection = collection; record->load_index = index;
    if (size >= 3 && !memcmp(model->input, "\xef\xbb\xbf", 3)) offset = 3;
    body = offset;
    if (fence(model->input + offset, size - offset, &advance)) {
        int closed = 0;
        header = offset + advance;
        for (p = header; p < size; ++p) {
            if ((p == header || model->input[p - 1] == '\n') && fence(model->input + p, size - p, &advance)) {
                body = p + advance; closed = 1; break;
            }
            if (p - header > GD_MAX_FRONTMATTER) break;
        }
        if (!closed || p - header > GD_MAX_FRONTMATTER) {
            gd_error(model, "input.frontmatter", "Missing closing fence or frontmatter exceeds 64 KiB", path, 0); return 0;
        }
        if (!parse_metadata(model, index, model->input + header, p - header)) {
            model->edge_count = edge_start; return 0;
        }
    }
    filename = strrchr(path, '/'); filename = filename ? filename + 1 : path;
    if (!record->name) record->name = copy_text(model, filename, strlen(filename) - 3);
    if (!record->name || !technical_name(record->name)) {
        gd_error(model, "input.name", "Consumed identity must be an ASCII technical name, at most 128 bytes", path, 0);
        model->edge_count = edge_start; return 0;
    }
    if (record->coverage && strcmp(record->coverage, "defined") && strcmp(record->coverage, "unknown") &&
        strcmp(record->coverage, "excluded") && strcmp(record->coverage, "not_applicable")) {
        gd_error(model, "matrix.coverage", "Unsupported case coverage; cannot classify it", path, 0);
        model->edge_count = edge_start; return 0;
    }
    snprintf(reference, sizeof(reference), "%s/%s", model->collections[collection].alias, record->name);
    record->reference = copy_text(model, reference, strlen(reference));
    record->body = copy_text(model, model->input + body, size - body);
    record->body_line = 1;
    for (p = 0; p < body; ++p) if (model->input[p] == '\n') ++record->body_line;
    if (!record->reference || !record->body) { model->edge_count = edge_start; return 0; }
    model->record_count++; return 1;
}
static int md_filename(const char *name) {
    size_t n = strlen(name);
    return name[0] != '.' && n > 3 && name[n-3] == '.' &&
           tolower((unsigned char)name[n-2]) == 'm' && tolower((unsigned char)name[n-1]) == 'd';
}
static int visit_file(GD_Model *model, int collection, const char *filename, int checking, uint64_t *digest) {
    char path[GD_MAX_PATH];
    size_t size;
    uint64_t hash = UINT64_C(14695981039346656037);
    const char *stored;
    int length = snprintf(path, sizeof(path), "%s/%s", model->collections[collection].directory, filename);
    if (length < 0 || length >= (int)sizeof(path)) {
        if (!checking) gd_error(model, "input.path_limit", "Source path exceeds 4095 UTF-8 bytes", NULL, 0);
        return 0;
    }
    stored = checking ? path : copy_text(model, path, strlen(path));
    if (!stored || !read_file(model, stored, &size, checking)) return 0;
    hash = hash_bytes(hash, model->collections[collection].alias, strlen(model->collections[collection].alias));
    hash = hash_bytes(hash, path, strlen(path)); hash = hash_bytes(hash, model->input, size);
    *digest += hash; /* Order independent across filesystem enumeration. Not a security hash. */
    if (!checking) load_record(model, collection, stored, size);
    return 1;
}
static int scan(GD_Model *model, int checking, uint64_t *digest) {
    int i, success = 1, files = 0, entries = 0;
    *digest = 0;
    for (i = 0; i < model->collection_count; ++i) {
#ifdef _WIN32
        wchar_t pattern[GD_MAX_PATH];
        WIN32_FIND_DATAW data;
        HANDLE handle;
        char filename[GD_MAX_PATH], search[GD_MAX_PATH];
        int n = snprintf(search, sizeof(search), "%s/*", model->collections[i].directory);
        if (n < 0 || n >= (int)sizeof(search) ||
            !MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, search, -1, pattern, GD_MAX_PATH)) {
            success = 0;
            if (!checking) gd_error(model, "input.path", "Invalid or oversized UTF-8 collection path", model->collections[i].directory, 0);
            continue;
        }
        handle = FindFirstFileW(pattern, &data);
        if (handle == INVALID_HANDLE_VALUE) {
            /* FindFirstFile reports FILE_NOT_FOUND for a genuinely empty folder. */
            DWORD attributes, find_error = GetLastError();
            int length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, model->collections[i].directory, -1, pattern, GD_MAX_PATH);
            attributes = length ? GetFileAttributesW(pattern) : INVALID_FILE_ATTRIBUTES;
            if (find_error != ERROR_FILE_NOT_FOUND || attributes == INVALID_FILE_ATTRIBUTES || !(attributes & FILE_ATTRIBUTE_DIRECTORY)) {
                if (!checking) gd_error(model, "input.folder", "Cannot enumerate collection folder", model->collections[i].directory, 0);
                success = 0;
            }
            continue;
        }
        do {
            if (++entries > GD_MAX_DIRECTORY_ENTRIES) { success = 0; break; }
            if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
            if (!WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, data.cFileName, -1, filename, sizeof(filename), NULL, NULL)) { success = 0; continue; }
            if (!md_filename(filename)) continue;
            if (++files > GD_MAX_RECORDS) { success = 0; break; }
            if (!visit_file(model, i, filename, checking, digest)) success = 0;
        } while (FindNextFileW(handle, &data));
        if (GetLastError() != ERROR_NO_MORE_FILES && files <= GD_MAX_RECORDS) success = 0;
        FindClose(handle);
#else
        DIR *folder = opendir(model->collections[i].directory);
        struct dirent *entry;
        if (!folder) { success = 0; if (!checking) gd_error(model, "input.folder", "Cannot enumerate collection folder", model->collections[i].directory, 0); continue; }
        while ((entry = readdir(folder)) != NULL) {
            struct stat attributes; char path[GD_MAX_PATH];
            if (++entries > GD_MAX_DIRECTORY_ENTRIES) { success = 0; break; }
            if (!md_filename(entry->d_name)) continue;
            if (snprintf(path, sizeof(path), "%s/%s", model->collections[i].directory, entry->d_name) >= (int)sizeof(path) ||
                stat(path, &attributes) || !S_ISREG(attributes.st_mode)) continue;
            if (++files > GD_MAX_RECORDS) { success = 0; break; }
            if (!visit_file(model, i, entry->d_name, checking, digest)) success = 0;
        }
        closedir(folder);
#endif
    }
    if (entries > GD_MAX_DIRECTORY_ENTRIES && !checking)
        gd_error(model, "input.entry_limit", "65536 directory entry limit exceeded", NULL, 0);
    if (files > GD_MAX_RECORDS && !checking) gd_error(model, "input.record_limit", "4096 source file limit exceeded", NULL, 0);
    return success;
}
static int compare_records(const void *a, const void *b) {
    const GD_Record *left = a, *right = b;
    int order = left->role - right->role;
    if (!order) order = strcmp(left->reference, right->reference);
    return order ? order : strcmp(left->source_path, right->source_path);
}
static int compare_pairs(const void *a, const void *b) {
    const GD_Pair *left = a, *right = b;
    if (left->noun != right->noun) return left->noun - right->noun;
    if (left->verb != right->verb) return left->verb - right->verb;
    return left->interaction - right->interaction;
}
int gd_active_case(const GD_Record *record) {
    return record->status && !strcmp(record->status, "accepted") && record->coverage && !strcmp(record->coverage, "defined");
}
static void resolve_edges(GD_Model *model) {
    int i, j;
    for (i = 0; i < model->edge_count; ++i) {
        GD_Edge *edge = &model->edges[i];
        for (j = 0; j < model->record_count; ++j) if (!strcmp(edge->reference, model->records[j].reference)) {
            edge->target = edge->target == -1 ? j : -2;
            if (edge->target == -2) break;
        }
    }
}
static void build_matrix(GD_Model *model) {
    int i, j, k;
    for (i = 0; i < model->record_count; ++i) {
        GD_Record *record = &model->records[i];
        int verb = -1, nouns[GD_MAX_RECORDS], noun_count = 0, active;
        if (record->role != GD_CASE) continue;
        active = gd_active_case(record);
        for (j = 0; j < model->edge_count; ++j) {
            GD_Edge *edge = &model->edges[j];
            if (edge->source != i || (edge->field != GD_NOUNS && edge->field != GD_VERB_REF)) continue;
            if (edge->target < 0 || model->records[edge->target].role != (edge->field == GD_NOUNS ? GD_NOUN : GD_VERB)) {
                gd_error(model, "matrix.endpoint", "Cannot place case: unresolved, ambiguous, or wrong-role endpoint", record->source_path, 0);
                continue;
            }
            if (edge->field == GD_VERB_REF) verb = edge->target;
            else {
                for (k = 0; k < noun_count && nouns[k] != edge->target; ++k) {}
                if (k == noun_count) {
                    if (noun_count == GD_MAX_RECORDS) { gd_error(model, "matrix.target_limit", "Case target limit exceeded", record->source_path, 0); break; }
                    nouns[noun_count++] = edge->target;
                }
            }
        }
        if (verb < 0 || !noun_count) { model->unplaced_cases++; continue; }
        model->connectivity[verb].cases++; model->connectivity[verb].active_cases += active;
        for (j = 0; j < noun_count; ++j) {
            GD_Pair *pair;
            if (model->pair_count == GD_MAX_PAIRS) { gd_error(model, "matrix.pair_limit", "32768 case-pair limit exceeded", record->source_path, 0); return; }
            pair = &model->pairs[model->pair_count++]; pair->noun = nouns[j]; pair->verb = verb; pair->interaction = i;
            model->connectivity[nouns[j]].cases++; model->connectivity[nouns[j]].active_cases += active;
        }
    }
    qsort(model->pairs, model->pair_count, sizeof(model->pairs[0]), compare_pairs);
    for (i = 0; i < model->pair_count; ++i) {
        GD_Pair *pair = &model->pairs[i];
        GD_Cell *cell;
        if (!model->cell_count || model->cells[model->cell_count-1].noun != pair->noun || model->cells[model->cell_count-1].verb != pair->verb) {
            cell = &model->cells[model->cell_count++];
            cell->noun = pair->noun; cell->verb = pair->verb; cell->first_pair = i;
        }
        cell = &model->cells[model->cell_count-1]; cell->pair_count++;
        if (gd_active_case(&model->records[pair->interaction])) cell->active = 1;
    }
    for (i = 0; i < model->cell_count; ++i) if (model->cells[i].active) {
        model->connectivity[model->cells[i].noun].partners++;
        model->connectivity[model->cells[i].verb].partners++;
    }
}
/* Weak groups of active matrix pairs or declared skill prerequisites only. */
static int component_root(int *parent, int node) {
    int root = node;
    while (parent[root] != root) root = parent[root];
    while (node != root) { int next = parent[node]; parent[node] = root; node = next; }
    return root;
}
static void build_components(GD_Model *model) {
    int parent[GD_MAX_RECORDS], group[GD_MAX_RECORDS], cursor[GD_MAX_RECORDS];
    int i, offset = 0;
    for (i = 0; i < model->record_count; ++i) {
        parent[i] = i; group[i] = -1; model->component_of[i] = -1;
    }
    for (i = 0; i < model->cell_count; ++i) if (model->cells[i].active) {
        int a = component_root(parent, model->cells[i].noun);
        int b = component_root(parent, model->cells[i].verb);
        if (a < b) parent[b] = a; else parent[a] = b;
    }
    for (i = 0; i < model->skill_link_count; ++i) {
        int a = component_root(parent, model->skill_links[i].prerequisite);
        int b = component_root(parent, model->skill_links[i].dependent);
        if (a < b) parent[b] = a; else parent[a] = b;
    }
    for (i = 0; i < model->record_count; ++i) {
        int root, index;
        if (model->skill_mode ? model->records[i].role != GD_SKILL :
            (model->records[i].role != GD_NOUN && model->records[i].role != GD_VERB)) continue;
        root = component_root(parent, i);
        if (group[root] < 0) {
            group[root] = model->component_count++;
            model->components[group[root]].anchor = i;
        }
        index = group[root]; model->component_of[i] = index;
        model->components[index].member_count++;
    }
    for (i = 0; i < model->component_count; ++i) {
        model->components[i].first_member = cursor[i] = offset;
        offset += model->components[i].member_count;
    }
    for (i = 0; i < model->record_count; ++i) if (model->component_of[i] >= 0)
        model->component_members[cursor[model->component_of[i]]++] = i;
    for (i = 0; i < model->cell_count; ++i) if (model->cells[i].active)
        model->components[model->component_of[model->cells[i].noun]].edge_count++;
    for (i = 0; i < model->skill_link_count; ++i)
        model->components[model->component_of[model->skill_links[i].prerequisite]].edge_count++;
}
void gd_recompute_skills(GD_Model *model) {
    assert(model->skill_mode);
    memset(model->skills, 0, sizeof(model->skills));
    memset(model->components, 0, sizeof(model->components));
    model->skill_link_count = model->finding_count = model->findings_omitted = 0;
    model->skill_incomplete = model->component_count = 0;
    gd_analyze_skills(model);
    build_components(model);
}
int gd_sources_changed(GD_Model *model) {
    uint64_t digest;
    return !scan(model, 1, &digest) || digest != model->source_digest;
}
int gd_load(GD_Model *model, const GD_Collection *collections, int count) {
    int i, j, roles = 0, remap[GD_MAX_RECORDS];
    if (count < 1 || count > GD_MAX_COLLECTIONS) { gd_error(model, "input.collections", "Supply explicit domain roles and at most 16 collections", NULL, 0); return 0; }
    model->collection_count = count;
    for (i = 0; i < count; ++i) {
        model->collections[i] = collections[i];
        if (collections[i].role == GD_SKILL) model->skill_mode = 1;
        if (collections[i].role == GD_CHALLENGE) model->route_mode = 1;
        if (gd_mda_role(collections[i].role)) model->mda_mode = 1;
        if (collections[i].role == GD_CONTROL) model->control_mode = 1;
        if (collections[i].role == GD_RULE || collections[i].role == GD_AXIS) model->rule_mode = 1;
        if (collections[i].role != GD_RELATED && collections[i].role != GD_RULE) {
            if (collections[i].role < GD_NOUN || collections[i].role > GD_RESOURCE || (roles & (1 << collections[i].role)))
                gd_error(model, "input.collections", "Supply each domain role exactly once", collections[i].directory, 0);
            else roles |= 1 << collections[i].role;
        }
        if (!technical_name(collections[i].alias)) gd_error(model, "input.alias", "Collection alias must be an ASCII technical name", collections[i].directory, 0);
        for (j = 0; j < i; ++j) if (!strcmp(collections[i].alias, collections[j].alias)) gd_error(model, "input.alias", "Collection aliases must be unique", collections[i].directory, 0);
    }
    if (model->rule_mode) {
        int permitted = (1 << GD_NOUN) | (1 << GD_VERB) | (1 << GD_CONTROL) | (1 << GD_AXIS) | (1 << GD_RESOURCE);
        if (roles & ~permitted)
            gd_error(model,"input.collections","Rules require rule sources, optional axes/nouns/verbs/control-vars and related collections",NULL,0);
    } else if (model->mda_mode) {
        int required = (1 << GD_MDA_MECHANIC) | (1 << GD_MDA_DYNAMIC) | (1 << GD_MDA_AESTHETIC);
        if (roles != required && roles != (required | (1 << GD_MDA_SYSTEM)))
            gd_error(model,"input.collections","MDA requires mechanics/dynamics/aesthetics roles, optional systems; no other domain roles",NULL,0);
    } else if (model->control_mode && roles != (1 << GD_CONTROL))
        gd_error(model,"input.collections","Controls require control-vars alone, plus optional related collections",NULL,0);
    else if (!model->control_mode && roles != (model->route_mode ? (1 << GD_SKILL) | (1 << GD_CHALLENGE) : model->skill_mode ? 1 << GD_SKILL : 7))
        gd_error(model, "input.collections", "Supply matrix roles, skill-atoms alone, or skill-atoms plus challenges; no other role mixtures", NULL, 0);
    if (model->error_count) return 0;
    if (!scan(model, 0, &model->source_digest) && !model->error_count)
        gd_error(model, "input.scan", "Source enumeration was incomplete", NULL, 0);
    qsort(model->records, model->record_count, sizeof(model->records[0]), compare_records);
    for (i = 0; i < model->record_count; ++i) remap[model->records[i].load_index] = i;
    for (i = 0; i < model->edge_count; ++i) model->edges[i].source = remap[model->edges[i].source];
    for (i = 1; i < model->record_count; ++i) if (!strcmp(model->records[i-1].reference, model->records[i].reference))
        gd_error(model, "input.identity", "Duplicate identity: cannot select a unique record", model->records[i].source_path, 0);
    resolve_edges(model);
    if (model->skill_mode) gd_recompute_skills(model);
    else if (!model->rule_mode && !model->mda_mode && !model->control_mode) { build_matrix(model); build_components(model); }
    if (!model->error_count && gd_sources_changed(model)) gd_error(model, "input.changed", "Source changed while loading; retry explicitly", NULL, 0);
    return model->error_count == 0 && model->errors_omitted == 0;
}
