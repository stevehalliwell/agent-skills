#include "gd.h"
#include "gd_rule_analysis.h"
#include <assert.h>
#include <string.h>

/* gd-rule is a line-oriented domain language, not YAML. In particular, a bare
   {control-vars/name} expression must not become a YAML flow mapping. */
static int same(const char *text, size_t size, const char *value) {
    return size == strlen(value) && !memcmp(text, value, size);
}
static int label(const char *text, size_t size) {
    size_t i;
    if (!size || size > 128) return 0;
    for (i = 0; i < size; ++i) {
        unsigned c = (unsigned char)text[i];
        if (!(c >= 'a' && c <= 'z') && !(c >= 'A' && c <= 'Z') &&
            !(c >= '0' && c <= '9') && (i == 0 || (c != '-' && c != '_'))) return 0;
    }
    return 1;
}
static const char *retain(GD_Model *model, GD_Rules *rules, const GD_Record *record,
                          const char *text, size_t size, int line) {
    char *value;
    if (size >= GD_RULE_TEXT_CAPACITY - rules->text_used) {
        gd_error(model,"rules.text_limit","4 MiB retained rule-text limit exceeded",record->source_path,line);
        return NULL;
    }
    value = rules->text + rules->text_used;
    memcpy(value,text,size); value[size] = 0; rules->text_used += size + 1;
    return value;
}
static int fail(GD_Model *model, const GD_Record *record, const char *code, const char *message, int line) {
    gd_error(model,code,message,record->source_path,line); return 0;
}
/* Markdown fences may use backticks or tildes, at most three leading spaces.
   Track other fences too: a quoted example inside another code block is not a rule. */
static size_t fence(const char *text, size_t size, char *marker, const char **tail, size_t *tail_size) {
    size_t indent = 0, width = 0, offset;
    while (indent < size && text[indent] == ' ') ++indent;
    if (indent > 3 || indent == size || (text[indent] != '`' && text[indent] != '~')) return 0;
    *marker = text[indent];
    while (indent + width < size && text[indent + width] == *marker) ++width;
    if (width < 3) return 0;
    offset = indent + width;
    while (offset < size && text[offset] == ' ') ++offset;
    *tail = text + offset; *tail_size = size - offset;
    return width;
}
static int declaration(GD_Model *model, GD_Rules *rules, GD_RuleSource *source,
                       const GD_Record *record, const char *text, size_t size,
                       int line, int *current_case, int *current_participant, int *in_set) {
    const char *key, *colon, *value;
    size_t indent = 0, key_size, value_size;
    GD_RuleCase *item;
    GD_RuleParticipant *participant;
    int i;
    while (indent < size && text[indent] == ' ') ++indent;
    if (indent == size || text[indent] == '#') return 1;
    if (text[indent] == '\t') return fail(model,record,"rules.indent","Use spaces, not tabs, for rule indentation",line);
    if (!indent) {
        if (size < 7 || memcmp(text,"case ",5) || text[size-1] != ':' || !label(text+5,size-6))
            return fail(model,record,"rules.case","Expected case NAME: with an ASCII technical name",line);
        for (i = source->first_case; i < rules->case_count; ++i)
            if (same(text+5,size-6,rules->cases[i].name))
                return fail(model,record,"rules.duplicate_case","Case names must be unique within a record",line);
        if (rules->case_count == GD_MAX_RULE_CASES)
            return fail(model,record,"rules.case_limit","4096 rule-case limit exceeded",line);
        item = &rules->cases[rules->case_count];
        memset(item,0,sizeof(*item));
        item->name = retain(model,rules,record,text+5,size-6,line);
        if (!item->name) return 0;
        item->source = (int)(source - rules->sources); item->line = line;
        item->first_participant = rules->participant_count;
        *current_case = rules->case_count++; ++source->case_count;
        *current_participant = -1; *in_set = 0; return 1;
    }
    if (*current_case < 0) return fail(model,record,"rules.case","Declare a named case before its fields",line);
    item = &rules->cases[*current_case];
    key = text + indent; colon = memchr(key,':',size-indent);
    if (!colon) return fail(model,record,"rules.field","Expected KEY: VALUE or participant NAME:",line);
    key_size = (size_t)(colon-key);
    while (key_size && key[key_size-1] == ' ') --key_size;
    value = colon + 1;
    while (value < text+size && *value == ' ') ++value;
    value_size = (size_t)(text+size-value);
    if (indent == 4) {
        *in_set = 0; *current_participant = -1;
        if (same(key,key_size,"context")) {
            if (item->context) return fail(model,record,"rules.duplicate_field","Duplicate case context",line);
            if (!value_size) return fail(model,record,"rules.value","context needs a value; omit it for unspecified context",line);
            item->context = retain(model,rules,record,value,value_size,line); item->context_line = line;
            return item->context != NULL;
        }
        if (same(key,key_size,"unknown_conditions") || same(key,key_size,"unknown_effects")) {
            int conditions=same(key,key_size,"unknown_conditions");
            const char **destination=conditions ? &item->unknown_conditions:&item->unknown_effects;
            if (*destination) return fail(model,record,"rules.duplicate_field","Duplicate case unknown coverage declaration",line);
            if (!value_size) return fail(model,record,"rules.value","Unknown coverage needs a readable reason, not an empty claim",line);
            *destination=retain(model,rules,record,value,value_size,line);
            if (conditions) item->unknown_conditions_line=line; else item->unknown_effects_line=line;
            return *destination!=NULL;
        }
        if (value_size || !label(key,key_size))
            return fail(model,record,"rules.participant","Expected participant NAME: or context: VALUE at four spaces",line);
        for (i = item->first_participant; i < rules->participant_count; ++i)
            if (same(key,key_size,rules->participants[i].name))
                return fail(model,record,"rules.duplicate_participant","Participant bindings must be unique within a case",line);
        if (rules->participant_count == GD_MAX_RULE_PARTICIPANTS)
            return fail(model,record,"rules.participant_limit","8192 rule-participant limit exceeded",line);
        participant = &rules->participants[rules->participant_count];
        memset(participant,0,sizeof(*participant));
        participant->name = retain(model,rules,record,key,key_size,line);
        if (!participant->name) return 0;
        participant->line = line; participant->first_assignment = rules->assignment_count;
        *current_participant = rules->participant_count++; ++item->participant_count; return 1;
    }
    if (*current_participant < 0) return fail(model,record,"rules.participant","Declare a participant before its fields",line);
    participant = &rules->participants[*current_participant];
    if (indent == 12 && *in_set) {
        GD_RuleAssignment *assignment;
        /* Axis qualification is retained for later target/axis resolution. */
        if (!key_size || !value_size) return fail(model,record,"rules.assignment","set needs AXIS: EXPRESSION entries",line);
        if (rules->assignment_count == GD_MAX_RULE_ASSIGNMENTS)
            return fail(model,record,"rules.assignment_limit","8192 rule-assignment limit exceeded",line);
        assignment = &rules->assignments[rules->assignment_count];
        assignment->axis = retain(model,rules,record,key,key_size,line);
        assignment->expression = retain(model,rules,record,value,value_size,line);
        assignment->line = line;
        if (!assignment->axis || !assignment->expression) return 0;
        ++rules->assignment_count; ++participant->assignment_count;
        /* Repeated assignments are declarations, not last-write-wins. Keep them
           for semantic conflict checking rather than silently overwriting. */
        return 1;
    }
    if (indent == 8) {
        static const char *keys[] = {"noun","requires","skip_if","other_axes","becomes","preserve","causes","set"};
        const char **destination;
        *in_set = 0;
        for (i = 0; i < 8; ++i) if (same(key,key_size,keys[i])) break;
        if (i == 8) return fail(model,record,"rules.field","Unknown participant field",line);
        if (participant->fields & (1u << i)) return fail(model,record,"rules.duplicate_field","Duplicate participant field",line);
        participant->fields |= 1u << i; participant->lines[i] = line;
        if (i == 7) {
            if (value_size) return fail(model,record,"rules.assignment","set: takes indented AXIS: EXPRESSION entries",line);
            *in_set = 1; return 1;
        }
        if (!value_size) return fail(model,record,"rules.value","Field needs a value; omit it for unspecified information",line);
        destination = i == 0 ? &participant->noun : i == 1 ? &participant->requires : i == 2 ? &participant->skip_if :
                      i == 3 ? &participant->other_axes : i == 4 ? &participant->becomes : i == 5 ? &participant->preserve : &participant->causes;
        *destination = retain(model,rules,record,value,value_size,line); return *destination != NULL;
    }
    return fail(model,record,"rules.indent","Use four spaces for case fields, eight for participant fields, twelve for set entries",line);
}
static int read_source(GD_Model *model, GD_Rules *rules, GD_RuleSource *source) {
    const GD_Record *record = &model->records[source->record];
    const char *cursor = record->body ? record->body : "";
    size_t fence_width = 0;
    char fence_marker = 0;
    int rule_fence = 0, seen = 0, line = record->body_line ? record->body_line : 1;
    int current_case = -1, current_participant = -1, in_set = 0;
    while (*cursor) {
        const char *end = strchr(cursor,'\n'), *tail = NULL;
        size_t size = end ? (size_t)(end-cursor) : strlen(cursor), width, tail_size = 0;
        char marker = 0;
        while (size && (cursor[size-1] == ' ' || cursor[size-1] == '\r' || cursor[size-1] == '\t')) --size;
        width = fence(cursor,size,&marker,&tail,&tail_size);
        if (fence_width && width >= fence_width && marker == fence_marker && !tail_size) {
            fence_width = 0; rule_fence = 0;
        } else if (!fence_width && width) {
            fence_width = width; fence_marker = marker; rule_fence = same(tail,tail_size,"gd-rule");
            if (rule_fence) {
                if (seen++) return fail(model,record,"rules.multiple_blocks","Use one authoritative gd-rule block per record",line);
                source->line = line;
            }
        } else if (rule_fence) {
            if (size > GD_MAX_RULE_LINE) return fail(model,record,"rules.line_limit","Rule line exceeds 8192 bytes",line);
            if (!declaration(model,rules,source,record,cursor,size,line,&current_case,&current_participant,&in_set)) return 0;
        }
        if (!end) break;
        cursor = end + 1; ++line;
    }
    if (rule_fence) return fail(model,record,"rules.unclosed_block","Close the gd-rule code fence",source->line);
    source->state = !seen ? GD_RULE_ABSENT : source->case_count ? GD_RULE_PARSED : GD_RULE_EMPTY;
    return 1;
}
GD_Rules *gd_read_rules(GD_Model *model) {
    GD_Rules *rules;
    int i;
    assert(model && model->allocator.allocate && model->allocator.release);
    rules = model->allocator.allocate(model->allocator.context,sizeof(*rules));
    if (!rules) { gd_error(model,"rules.memory","Cannot allocate bounded rule declarations",NULL,0); return NULL; }
    memset(rules,0,sizeof(*rules)); rules->allocator = model->allocator; rules->model=model;
    for (i = 0; i < model->record_count; ++i) if (model->records[i].role == GD_RULE) {
        int participants = rules->participant_count, assignments = rules->assignment_count;
        size_t text_used = rules->text_used;
        GD_RuleSource *source = &rules->sources[rules->source_count++];
        source->record = i; source->first_case = rules->case_count;
        if (!read_source(model,rules,source)) {
            /* A malformed record contributes no valid-looking partial cases. */
            rules->case_count = source->first_case; rules->participant_count = participants;
            rules->assignment_count = assignments; rules->text_used = text_used;
            source->case_count = 0; source->state = GD_RULE_INVALID;
        }
    }
    gd_resolve_rule_axes(model,rules);
    gd_resolve_rule_logic(model,rules);
    gd_resolve_rule_quantities(model,rules);
    gd_resolve_rule_controls(model,rules);
    return rules;
}
void gd_rules_destroy(GD_Rules *rules) {
    if (rules) { gd_controls_destroy(rules->controls); rules->allocator.release(rules->allocator.context,rules); }
}
static void member(FILE *output, const char *key, const char *value) {
    fputc(',',output); gd_json_string(output,key); fputc(':',output); gd_json_string(output,value);
}
static int selected(const char *fields, const char *key) {
    const char *cursor = fields;
    if (!cursor) return 1;
    while (*cursor) {
        const char *end = strchr(cursor,',');
        if (same(cursor,end ? (size_t)(end-cursor) : strlen(cursor),key)) return 1;
        if (!end) break;
        cursor = end+1;
    }
    return 0;
}
static const char *source_state(GD_RuleSourceState state) {
    return state == GD_RULE_PARSED ? "parsed" : state == GD_RULE_EMPTY ? "empty" : state == GD_RULE_INVALID ? "invalid" : "absent";
}
static void provenance(FILE *output, const GD_Record *record, int line) {
    fputs("{\"record\":",output); gd_json_string(output,record->reference);
    member(output,"record_id",record->id); member(output,"status",record->status);
    member(output,"coverage",record->coverage); member(output,"source_path",record->source_path);
    fprintf(output,",\"line\":%d",line);
}
static void effect_target(FILE *output, const GD_Model *model, GD_RuleTarget target) {
    fputs(",\"target\":{\"resolution\":",output); gd_json_string(output,gd_rule_resolution_name(target.resolution));
    member(output,"axis",target.axis_record >= 0 ? model->records[target.axis_record].reference:NULL);
    member(output,"state",target.state); fputc('}',output);
}
static void quantity_result(FILE *output, const GD_Rules *rules, int index) {
    const GD_RuleQuantity *quantity;
    const GD_RuleQuantityValue *value;
    fputs(",\"quantity\":",output);
    if (index<0 || index>=rules->quantity_count) { fputs("null",output); return; }
    quantity=&rules->quantities[index]; value=&quantity->inspection;
    fputs("{\"state\":",output); gd_json_string(output,gd_rule_condition_state_name(quantity->state));
    fputs(",\"text\":",output); gd_json_string_n(output,quantity->text,quantity->length);
    fprintf(output,",\"token_count\":%d,\"bindings_complete\":%s,\"inspection\":{\"state\":",quantity->token_count,quantity->bindings_complete ? "true":"false");
    gd_json_string(output,value->state==GD_EXPRESSION_VALUE ? "resolved":value->state==GD_EXPRESSION_UNIT_ERROR ? "unit_error":
                   value->state==GD_EXPRESSION_ARITHMETIC_ERROR ? "arithmetic_error":"unknown");
    fprintf(output,",\"checks_complete\":%s,\"number\":",value->checks_complete ? "true":"false");
    if (value->known) fprintf(output,"%.17g",value->number); else fputs("null",output);
    fputs(",\"bounds\":",output); gd_json_string(output,value->bounds==GD_BOUNDS_WITHIN ? "within_declared":
        value->bounds==GD_BOUNDS_OUTSIDE ? "outside_declared":value->bounds==GD_BOUNDS_UNDECLARED ? "not_declared":"unknown");
    fputs("}}",output);
}
static void condition_result(FILE *output, const GD_Model *model, const GD_Rules *rules,
                             const char *key, const GD_RuleCondition *condition, int line) {
    int i;
    fputc(',',output); gd_json_string(output,key); fputs(":{\"state\":",output);
    gd_json_string(output,gd_rule_condition_state_name(condition->state));
    fprintf(output,",\"line\":%d,\"bindings_complete\":%s,\"program\":[",line,condition->bindings_complete ? "true":"false");
    for (i = 0; i < condition->token_count; ++i) {
        const GD_RuleConditionToken *token = &rules->condition_tokens[condition->first_token+i];
        if (i) fputc(',',output); fputs("{\"op\":",output); gd_json_string(output,gd_rule_condition_op_name(token->operation));
        fputs(",\"text\":",output); gd_json_string_n(output,token->text,token->length);
        fprintf(output,",\"offset\":%d",token->offset);
        if (token->operation == GD_RULE_OP_STATE || token->operation == GD_RULE_OP_COMPARE) effect_target(output,model,token->target);
        if (token->operation==GD_RULE_OP_CONTROL) {
            member(output,"control",token->control_record>=0 ? model->records[token->control_record].reference:NULL);
            member(output,"control_resolution",token->control_record>=0 ? "resolved":token->control_record==GD_EXPRESSION_UNLOADED_REFERENCE ? "unloaded":"invalid");
        }
        if (token->operation == GD_RULE_OP_COMPARE) {
            member(output,"comparison",gd_rule_comparison_name(token->comparison)); quantity_result(output,rules,token->quantity);
        }
        fputc('}',output);
    }
    fputs("]}",output);
}
static void context_result(FILE *output, const GD_Model *model, const GD_Rules *rules, const GD_RuleContext *context) {
    fputs(",\"ordered_context\":{\"state\":",output); gd_json_string(output,gd_rule_condition_state_name(context->state));
    fprintf(output,",\"unrestricted\":%s,\"bindings_complete\":%s",context->unrestricted ? "true":"false",context->bindings_complete ? "true":"false");
    member(output,"left",context->left >= 0 ? rules->participants[context->left].name:NULL);
    member(output,"right",context->right >= 0 ? rules->participants[context->right].name:NULL);
    fprintf(output,",\"left_index\":%d,\"right_index\":%d,\"operator\":",context->left,context->right);
    gd_json_string_n(output,context->operator_text,context->operator_length);
    fprintf(output,",\"operator_offset\":%d",context->operator_offset);
    effect_target(output,model,context->target); fputc('}',output);
}
int gd_write_rule_report(FILE *output, const GD_Model *model, const GD_Rules *rules, const GD_RuleAnalysis *analysis, int json, const char *fields) {
    int i, j, k, comma = 0, parsed = 0;
    int complete = rules && !model->error_count && !model->errors_omitted;
    if (rules) for (i = 0; i < rules->source_count; ++i) parsed += rules->sources[i].state == GD_RULE_PARSED;
    if (!json) {
        fprintf(output,"Rules: %d source records; %d with cases; %d cases; %d participants; %d assignments. Declaration checks %s.\n",
                rules ? rules->source_count:0,parsed,rules ? rules->case_count:0,rules ? rules->participant_count:0,
                rules ? rules->assignment_count:0,complete ? "complete":"incomplete");
        fputs("Declarations only: explicit applicability, state predicate logic and effect bindings resolved where known; ordered context binds local participants and declared operator vocabulary only. Shared numeric math inspects constants and typed inputs; no actor starting values or gameplay transition inferred. Optional analysis reports separate bounded hypothetical checks.\n",output);
        fprintf(output,"Typed axes: %d; actors: %d; axis definitions %s; explicit bindings %s; findings: %d (+%d omitted).\n",
                rules ? rules->axis_count:0,rules ? rules->actor_count:0,rules && rules->axis_definitions_complete ? "complete":"incomplete",
                rules && rules->bindings_complete ? "complete":"incomplete",rules ? rules->finding_count:0,rules ? rules->findings_omitted:0);
        fprintf(output,"Effect bindings: %d state declarations; %s for authored targets; symbol work %d/%d. Quantity inspection is not a gameplay transition.\n",
                rules ? rules->state_effect_count:0,rules && rules->effect_bindings_complete ? "complete":"incomplete",
                rules ? rules->symbol_work:0,GD_RULE_SYMBOL_WORK_LIMIT);
        fprintf(output,"Conditions: %d authored; %d compiled tokens; bindings %s. Omitted predicates remain unspecified.\n",
                rules ? rules->condition_count:0,rules ? rules->condition_token_count:0,
                rules && rules->condition_bindings_complete ? "complete":"incomplete");
        fprintf(output,"Ordered context: %d authored; bindings %s. No contact, symmetry, reachability or scene truth inferred.\n",
                rules ? rules->context_count:0,rules && rules->context_bindings_complete ? "complete":"incomplete");
        if (rules) for (i = 0; i < model->record_count; ++i) if (model->records[i].role == GD_AXIS) {
            const GD_Record *record = &model->records[i]; const char *value = record->axis.allowed;
            fprintf(output,"%s: %s; %s; unit=%s",record->reference,record->axis.type ? record->axis.type:"unspecified",
                    record->axis_closed ? "closed vocabulary":"open vocabulary",record->axis.unit && *record->axis.unit ? record->axis.unit:"unspecified");
            for (j = 0; j < record->axis.allowed_count; ++j) { fprintf(output,"%s%s",j ? ", ":"; values=",value); value += strlen(value)+1; }
            fputc('\n',output);
        }
        if (rules) for (i = 0; i < rules->case_count; ++i) {
            const GD_RuleCase *item = &rules->cases[i];
            fprintf(output,"%s#%s — %s:%d\n",model->records[rules->sources[item->source].record].reference,
                    item->name,model->records[rules->sources[item->source].record].source_path,item->line);
        }
        if (analysis) {
            fprintf(output,"Rule analysis: %d explicit calls; %d compared pairs; %d pairs unexamined. No physical scene or universal gameplay claim.\n",analysis->call_count,analysis->pair_count,analysis->pairs_unexamined);
            if (analysis->requested_case>=0) {
                const GD_RuleEvaluation *result=&analysis->scenario;
                fprintf(output,"Scenario: applicability=%s; %d proposals; %d conflicts; %d uncertain overlap pairs; constraints=%s; limited=%s.\n",
                    gd_rule_truth_name(result->applicability),result->write_count,result->conflict_count,result->uncertain_write_pairs,result->constraints_complete ? "complete":"incomplete",result->limited ? "yes":"no");
                for (i=0;i<result->write_count;++i) {
                    const GD_RuleWrite *write=&result->writes[i];
                    fprintf(output,"  %s %s: %s",write->actor>=0 ? rules->participants[write->actor].name:"unknown actor",write->axis>=0 ? model->records[write->axis].reference:"unknown axis",write->state ? write->state:write->kind==GD_WRITE_PRESERVE ? "preserve starting value":"numeric proposal");
                    if (write->value.known) fprintf(output," %.17g",write->value.number);
                    fprintf(output,"; %s; %s#%s:%d\n",write->conditional ? "conditional":"under supplied assumptions",model->records[rules->sources[rules->cases[write->origin_case].source].record].reference,rules->cases[write->origin_case].name,write->line);
                }
            }
        }
        return !ferror(output);
    }
    fprintf(output,"{\"schema_version\":1,\"ok\":%s,\"data\":{",complete ? "true":"false");
    if (selected(fields,"records")) {
        fputs("\"records\":[",output); comma = 1;
        if (rules) for (i = 0; i < rules->source_count; ++i) {
            const GD_RuleSource *source = &rules->sources[i]; if (i) fputc(',',output);
            provenance(output,&model->records[source->record],source->line);
            member(output,"name",model->records[source->record].name);
            member(output,"description",model->records[source->record].description);
            member(output,"rule_block",source_state(source->state));
            fprintf(output,",\"context_axes_declared\":%s,\"context_axes_complete\":%s,\"context_axes_line\":%d,\"case_count\":%d,\"relationships\":[",
                    model->records[source->record].context_axes_declared ? "true":"false",
                    rules->context_axes_complete[source->record] ? "true":"false",model->records[source->record].context_axes_line,source->case_count);
            {
                int edge_comma = 0;
                for (j = 0; j < model->edge_count; ++j) if (model->edges[j].source == source->record) {
                    const GD_Edge *edge = &model->edges[j]; if (edge_comma++) fputc(',',output);
                    fputs("{\"field\":",output); gd_json_string(output,gd_field_name(edge->field));
                    member(output,"reference",edge->reference);
                    fprintf(output,",\"line\":%d,\"loaded\":%s,\"ambiguous\":%s}",edge->line,
                            edge->target >= 0 ? "true":"false",edge->target == -2 ? "true":"false");
                }
            }
            fputs("]}",output);
        }
        fputc(']',output);
    }
    if (selected(fields,"cases")) {
        if (comma++) fputc(',',output); fputs("\"cases\":[",output);
        if (rules) for (i = 0; i < rules->case_count; ++i) {
            const GD_RuleCase *item = &rules->cases[i]; if (i) fputc(',',output);
            provenance(output,&model->records[rules->sources[item->source].record],item->line);
            member(output,"case",item->name); member(output,"context",item->context);
            member(output,"unknown_conditions",item->unknown_conditions); member(output,"unknown_effects",item->unknown_effects);
            fprintf(output,",\"unknown_conditions_line\":%d,\"unknown_effects_line\":%d",item->unknown_conditions_line,item->unknown_effects_line);
            context_result(output,model,rules,&item->ordered_context);
            fprintf(output,",\"context_line\":%d,\"skip_if_count\":%d,\"participants\":[",item->context_line,item->skip_if_count);
            for (j = 0; j < item->participant_count; ++j) {
                const GD_RuleParticipant *participant = &rules->participants[item->first_participant+j];
                if (j) fputc(',',output); fputs("{\"name\":",output); gd_json_string(output,participant->name);
                member(output,"noun",participant->noun);
                member(output,"noun_resolution",gd_rule_resolution_name(participant->noun_resolution));
                fprintf(output,",\"applicable_axes_complete\":%s",participant->noun_resolution == GD_RULE_RESOLVED && rules->actor_axes_complete[participant->noun_record] ? "true":"false");
                member(output,"requires",participant->requires);
                member(output,"skip_if",participant->skip_if); member(output,"other_axes",participant->other_axes);
                member(output,"becomes",participant->becomes); member(output,"preserve",participant->preserve);
                member(output,"causes",participant->causes);
                member(output,"other_axes_mode",participant->other_axes_mode == GD_RULE_OTHER_ANY ? "any":
                       participant->other_axes_mode == GD_RULE_OTHER_INVALID ? "invalid":"unspecified");
                condition_result(output,model,rules,"requires_condition",&participant->required_condition,participant->lines[1]);
                condition_result(output,model,rules,"skip_if_condition",&participant->skip_condition,participant->lines[2]);
                fprintf(output,",\"line\":%d,\"field_lines\":{",participant->line);
                fprintf(output,"\"noun\":%d,\"requires\":%d,\"skip_if\":%d,\"other_axes\":%d,\"becomes\":%d,\"preserve\":%d,\"causes\":%d,\"set\":%d},\"set\":[",
                        participant->lines[0],participant->lines[1],participant->lines[2],participant->lines[3],
                        participant->lines[4],participant->lines[5],participant->lines[6],participant->lines[7]);
                for (k = 0; k < participant->assignment_count; ++k) {
                    const GD_RuleAssignment *assignment = &rules->assignments[participant->first_assignment+k];
                    if (k) fputc(',',output); fputs("{\"axis\":",output); gd_json_string(output,assignment->axis);
                    member(output,"expression",assignment->expression); fprintf(output,",\"line\":%d",assignment->line);
                    effect_target(output,model,assignment->target); quantity_result(output,rules,assignment->quantity); fputc('}',output);
                }
                fputs("],\"state_effects\":[",output);
                for (k = 0; k < participant->state_effect_count; ++k) {
                    const GD_RuleStateEffect *effect = &rules->state_effects[participant->first_state_effect+k];
                    if (k) fputc(',',output); fputs("{\"text\":",output); gd_json_string_n(output,effect->text,effect->length);
                    fprintf(output,",\"line\":%d",effect->line); effect_target(output,model,effect->target);
                    member(output,"control",effect->control_record>=0 ? model->records[effect->control_record].reference:NULL); fputc('}',output);
                }
                fputs("],\"preservations\":[",output);
                for (k=0;k<participant->preservation_count;++k) {
                    const GD_RulePreservation *promise=&rules->preservations[participant->first_preservation+k];
                    if (k) fputc(',',output); fputs("{\"text\":",output); gd_json_string_n(output,promise->text,promise->length);
                    member(output,"kind",promise->kind==GD_PRESERVE_ALL ? "all":promise->kind==GD_PRESERVE_OTHER ? "other":"axis");
                    fprintf(output,",\"line\":%d",promise->line); effect_target(output,model,promise->target); fputc('}',output);
                }
                fputs("]}",output);
            }
            fputs("]}",output);
        }
        fputc(']',output);
    }
    if (selected(fields,"axes")) {
        int axis_comma = 0;
        if (comma++) fputc(',',output); fputs("\"axes\":[",output);
        if (rules) for (i = 0; i < model->record_count; ++i) if (model->records[i].role == GD_AXIS) {
            const GD_Record *record = &model->records[i]; const GD_ValueDefinition *definition = &record->axis;
            const char *value = definition->allowed; double bound;
            if (axis_comma++) fputc(',',output); provenance(output,record,definition->lines[0]);
            member(output,"name",record->name); member(output,"description",record->description);
            member(output,"value_type",definition->type); member(output,"unit",definition->unit);
            fprintf(output,",\"definition_valid\":%s,\"type_known\":%s,\"unit_known\":%s,\"closed\":%s,\"closed_line\":%d,\"minimum\":",
                    rules->axes[i].valid ? "true":"false",rules->axes[i].type != GD_AXIS_UNSPECIFIED ? "true":"false",
                    rules->axes[i].unit_known ? "true":"false",record->axis_closed ? "true":"false",record->axis_closed_line);
            if (definition->minimum && gd_control_number(definition->minimum,&bound,0)) fprintf(output,"%.17g",bound); else fputs("null",output);
            fputs(",\"maximum\":",output);
            if (definition->maximum && gd_control_number(definition->maximum,&bound,0)) fprintf(output,"%.17g",bound); else fputs("null",output);
            fputs(",\"allowed_values\":[",output);
            for (j = 0; j < definition->allowed_count; ++j) {
                if (j) fputc(',',output); gd_json_string(output,value); value += strlen(value)+1;
            }
            fprintf(output,"],\"field_lines\":{\"value_type\":%d,\"unit\":%d,\"allowed_values\":%d,\"minimum\":%d,\"maximum\":%d}}",
                    definition->lines[0],definition->lines[3],definition->lines[4],definition->lines[5],definition->lines[6]);
        }
        fputc(']',output);
    }
    if (selected(fields,"actors")) {
        int actor_comma = 0;
        if (comma++) fputc(',',output); fputs("\"actors\":[",output);
        if (rules) for (i = 0; i < model->record_count; ++i) if (model->records[i].role == GD_NOUN) {
            const GD_Record *record = &model->records[i]; int edge, edge_comma = 0;
            if (actor_comma++) fputc(',',output); provenance(output,record,record->axes_line);
            member(output,"name",record->name); member(output,"description",record->description);
            fprintf(output,",\"axes_declared\":%s,\"applicable_axes_complete\":%s,\"axes\":[",record->axes_declared ? "true":"false",rules->actor_axes_complete[i] ? "true":"false");
            for (edge = rules->actor_first[i]; edge >= 0; edge = rules->next_axis[edge]) {
                if (edge_comma++) fputc(',',output); fputs("{\"reference\":",output); gd_json_string(output,model->edges[edge].reference);
                member(output,"resolution",gd_rule_resolution_name(rules->axis_resolution[edge]));
                fprintf(output,",\"line\":%d}",model->edges[edge].line);
            }
            fputs("]}",output);
        }
        fputc(']',output);
    }
    if (selected(fields,"findings")) {
        if (comma++) fputc(',',output); fputs("\"findings\":[",output);
        if (rules) for (i = 0; i < rules->finding_count; ++i) {
            const GD_Finding *item = &rules->findings[i]; if (i) fputc(',',output);
            provenance(output,&model->records[item->record],item->line);
            member(output,"code",item->code); member(output,"message",item->message);
            member(output,"field",item->field); member(output,"reference",item->reference); member(output,"severity","info");
            fputc('}',output);
        }
        fputc(']',output);
    }
    if (analysis && selected(fields,"analysis")) { if (comma++) fputc(',',output); fputs("\"analysis\":",output); gd_write_rule_analysis(output,analysis); }
    fprintf(output,"},\"coverage\":{\"operation\":\"rules\",\"complete\":%s,\"source_record_count\":%d,\"records_with_cases\":%d,\"case_count\":%d,\"participant_count\":%d,\"assignment_count\":%d,",
            complete ? "true":"false",rules ? rules->source_count:0,parsed,rules ? rules->case_count:0,
            rules ? rules->participant_count:0,rules ? rules->assignment_count:0);
    fprintf(output,"\"axis_count\":%d,\"actor_count\":%d,\"axis_definitions_complete\":%s,\"bindings_complete\":%s,\"findings_omitted\":%d,",
            rules ? rules->axis_count:0,rules ? rules->actor_count:0,rules && rules->axis_definitions_complete ? "true":"false",
            rules && rules->bindings_complete ? "true":"false",rules ? rules->findings_omitted:0);
    fprintf(output,"\"preservation_count\":%d,\"preservation_bindings_complete\":%s,",rules ? rules->preservation_count:0,rules && rules->preservation_bindings_complete ? "true":"false");
    fprintf(output,"\"state_effect_count\":%d,\"effect_bindings_complete\":%s,\"symbol_work\":%d,",
            rules ? rules->state_effect_count:0,rules && rules->effect_bindings_complete ? "true":"false",rules ? rules->symbol_work:0);
    fprintf(output,"\"condition_count\":%d,\"condition_token_count\":%d,\"condition_bindings_complete\":%s,",
            rules ? rules->condition_count:0,rules ? rules->condition_token_count:0,rules && rules->condition_bindings_complete ? "true":"false");
    fprintf(output,"\"context_count\":%d,\"context_bindings_complete\":%s,",rules ? rules->context_count:0,
            rules && rules->context_bindings_complete ? "true":"false");
    fprintf(output,"\"quantity_count\":%d,\"quantity_token_count\":%d,\"quantity_bindings_complete\":%s,\"quantity_work\":%d,\"quantity_limited\":%s,",
        rules ? rules->quantity_count:0,rules ? rules->quantity_token_count:0,rules && rules->quantity_bindings_complete ? "true":"false",
        rules ? rules->quantity_work:0,rules && rules->quantity_limited ? "true":"false");
    fputs("\"scope\":\"Declaration scan, explicit applicability, state predicate logic, ordered context bindings and effect bindings, all supplied statuses/dispositions. Missing rules remain absent; omitted fields remain null, not ANY or preserved. State conditions compile to derived Boolean programs, not an applicability verdict. Ordered context binds case-local endpoints and explicit context_axes vocabulary; no contact, symmetry, reachability or scene truth inferred. Numeric comparisons and assignment expressions use the shared bounded numeric compiler/evaluator. Inspection supplies no actor starting values: this remains unknown, constants are not a gameplay transition, and bounds never clamp/wrap/reject behavior. Preservation promises bind without inventing missing results; optional analysis expands explicit composition and checks conditional proposals. Effect bindings do not establish consistency or an applicable transition. Open vocabularies do not prove complete state sets or reachable combinations. Declaration inspection alone makes no equivalence, consistency or gameplay claim; optional analysis reports separate bounded logical checks.\",\"collections\":[",output);
    for (i = 0; i < model->collection_count; ++i) {
        if (i) fputc(',',output); fputs("{\"alias\":",output); gd_json_string(output,model->collections[i].alias);
        member(output,"directory",model->collections[i].directory);
        member(output,"role",model->collections[i].role == GD_RULE ? "rules":model->collections[i].role == GD_NOUN ? "nouns":
               model->collections[i].role == GD_VERB ? "verbs":model->collections[i].role == GD_CONTROL ? "control-vars":model->collections[i].role == GD_AXIS ? "axes":"related");
        fputc('}',output);
    }
    fprintf(output,"],\"limits\":{\"cases\":%d,\"participants\":%d,\"assignments\":%d,\"text_bytes\":%d,\"line_bytes\":%d,\"axis_values\":%d,\"findings\":%d,\"records\":%d,\"axis_references\":%d,\"state_effects\":%d,\"symbol_work\":%d,\"condition_tokens\":%d,\"condition_items\":%d,\"condition_depth\":%d,\"evaluation_work\":%d,\"quantities\":%d,\"quantity_tokens\":%d,\"quantity_work\":%d,\"preservations\":%d}},\"errors\":[",
            GD_MAX_RULE_CASES,GD_MAX_RULE_PARTICIPANTS,GD_MAX_RULE_ASSIGNMENTS,GD_RULE_TEXT_CAPACITY,GD_MAX_RULE_LINE,
            GD_MAX_ENUM_VALUES,GD_MAX_ERRORS,GD_MAX_RECORDS,GD_MAX_EDGES,GD_MAX_RULE_STATE_EFFECTS,GD_RULE_SYMBOL_WORK_LIMIT,GD_MAX_RULE_CONDITION_TOKENS,GD_MAX_RULE_CONDITION_ITEMS,GD_MAX_RULE_CONDITION_DEPTH,GD_RULE_EVALUATION_WORK_LIMIT,GD_MAX_RULE_QUANTITIES,GD_MAX_RULE_QUANTITY_TOKENS,GD_RULE_QUANTITY_WORK_LIMIT,GD_MAX_RULE_PRESERVATIONS);
    for (i = 0; i < model->error_count; ++i) {
        const GD_Error *error = &model->errors[i]; if (i) fputc(',',output);
        fputs("{\"code\":",output); gd_json_string(output,error->code); member(output,"message",error->message);
        member(output,"source_path",error->path); fprintf(output,",\"line\":%d}",error->line);
    }
    fprintf(output,"],\"errors_omitted\":%d,\"breadcrumbs\":[\"Edit the authoritative gd-rule block in source Markdown; rerun with the same explicit mappings.\"]}\n",model->errors_omitted);
    return !ferror(output);
}
int gd_write_rules_result(FILE *output, const GD_Model *model, const GD_Rules *rules, int json, const char *fields) {
    return gd_write_rule_report(output,model,rules,NULL,json,fields);
}
