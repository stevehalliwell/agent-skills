#include "gd.h"
#include <assert.h>
#include <string.h>

typedef struct {
    GD_Model *model;
    GD_Rules *rules;
    int seen[GD_MAX_RECORDS], generation, exhausted, effects_limit, condition_limit;
    int record, line;
} RuleReader;

/* Charge byte-comparison upper bounds and vocabulary traversal, not just
   lookup calls. Large strings and repeated references share the same budget. */
static int charge(RuleReader *reader, size_t amount) {
    if (reader->exhausted) return 0;
    if (amount > (size_t)(GD_RULE_SYMBOL_WORK_LIMIT-reader->rules->symbol_work)) {
        reader->exhausted = 1;
        gd_error(reader->model,"rules.symbol_work_limit","Rule symbol work limit exceeded; remaining bindings unavailable",
                 reader->model->records[reader->record].source_path,reader->line);
        return 0;
    }
    reader->rules->symbol_work += (int)amount;
    return 1;
}
static int equal(RuleReader *reader, const char *text, size_t size, const char *value) {
    if (!charge(reader,size+1)) return 0;
    return !strncmp(text,value,size) && !value[size];
}
static const char *named_state(RuleReader *reader, int axis, const char *text, size_t size) {
    const GD_ValueDefinition *definition = &reader->model->records[axis].axis;
    const char *value = definition->allowed;
    int i;
    for (i = 0; i < definition->allowed_count && !reader->exhausted; ++i) {
        if (equal(reader,text,size,value)) return value;
        while (*value) { if (!charge(reader,1)) return NULL; ++value; }
        if (!charge(reader,1)) return NULL;
        ++value;
    }
    return NULL;
}
static GD_RuleTarget lookup(RuleReader *reader, const GD_RuleParticipant *participant,
                            const char *text, size_t size, int state) {
    GD_RuleTarget target = {GD_RULE_UNDETERMINED,-1,NULL};
    const char *dot = state ? memchr(text,'.',size):NULL;
    size_t axis_size = dot ? (size_t)(dot-text):size;
    int edge, found = -1, uncertain = 0, open = 0, ambiguous = 0;
    int canonical = (!state || dot) && memchr(text,'/',dot ? axis_size:size) != NULL;
    const char *value = NULL;
    if (reader->exhausted) { target.resolution = GD_RULE_UNAVAILABLE; return target; }
    if (participant->noun_resolution != GD_RULE_RESOLVED) return target;
    if (!reader->rules->actor_axes_complete[participant->noun_record]) uncertain = 1;
    ++reader->generation;
    assert(reader->generation <= GD_MAX_RULE_STATE_EFFECTS + GD_MAX_RULE_ASSIGNMENTS + GD_MAX_RULE_CONDITION_TOKENS);
    for (edge = reader->rules->actor_first[participant->noun_record]; edge >= 0; edge = reader->rules->next_axis[edge]) {
        int axis = reader->model->edges[edge].target;
        const GD_RuleAxis *definition;
        const char *match;
        if (!charge(reader,1)) break;
        if (reader->rules->axis_resolution[edge] != GD_RULE_RESOLVED) { uncertain = 1; continue; }
        if (reader->seen[axis] == reader->generation) continue;
        reader->seen[axis] = reader->generation;
        definition = &reader->rules->axes[axis];
        if (!state || dot) {
            const GD_Record *record = &reader->model->records[axis];
            if (!equal(reader,text,axis_size,record->name) && !equal(reader,text,axis_size,record->reference)) continue;
            if (found >= 0) ambiguous = 1;
            found = axis;
        } else {
            if (!definition->valid || definition->type == GD_AXIS_UNSPECIFIED) { uncertain = 1; continue; }
            if (definition->type != GD_AXIS_ENUM) continue;
            if (!reader->model->records[axis].axis_closed) open = 1;
            match = named_state(reader,axis,text,size);
            if (!match) continue;
            if (found >= 0) ambiguous = 1;
            found = axis; value = match;
        }
    }
    if (reader->exhausted) { target.resolution = GD_RULE_UNAVAILABLE; return target; }
    if (ambiguous) { target.resolution = GD_RULE_AMBIGUOUS; return target; }
    /* Qualification can select a known canonical reference despite other
       unloaded axes; a bare name cannot exclude an unseen competing name. */
    if (uncertain && !canonical) return target;
    if (found < 0) {
        target.resolution = uncertain || (state && !dot && open) ? GD_RULE_UNDETERMINED:GD_RULE_MISSING;
        return target;
    }
    target.axis_record = found;
    if (!reader->rules->axes[found].valid) { target.resolution = GD_RULE_INVALID_DEFINITION; return target; }
    if (reader->rules->axes[found].type == GD_AXIS_UNSPECIFIED) return target;
    if ((state && reader->rules->axes[found].type != GD_AXIS_ENUM) ||
        (!state && reader->rules->axes[found].type != GD_AXIS_INTEGER && reader->rules->axes[found].type != GD_AXIS_NUMBER)) {
        target.resolution = GD_RULE_WRONG_TYPE; return target;
    }
    if (state && dot) {
        value = named_state(reader,found,dot+1,size-axis_size-1);
        if (reader->exhausted) { target.resolution = GD_RULE_UNAVAILABLE; return target; }
        if (!value) {
            target.resolution = reader->model->records[found].axis_closed ? GD_RULE_MISSING:GD_RULE_UNDETERMINED;
            return target;
        }
    }
    target.state = value; target.resolution = GD_RULE_RESOLVED;
    return target;
}
static void diagnose(RuleReader *reader, GD_RuleTarget target, const char *field,
                     const char *text, GD_RuleCondition *condition) {
    GD_Finding *finding;
    const char *code, *message;
    if (target.resolution == GD_RULE_RESOLVED) return;
    if (condition) { condition->bindings_complete = 0; reader->rules->condition_bindings_complete = 0; }
    else reader->rules->effect_bindings_complete = 0;
    if (target.resolution == GD_RULE_UNAVAILABLE) return; /* Budget diagnostic already retained. */
    if (target.resolution == GD_RULE_UNDETERMINED) {
        if (reader->rules->finding_count == GD_MAX_ERRORS) { ++reader->rules->findings_omitted; return; }
        finding = &reader->rules->findings[reader->rules->finding_count++];
        finding->record = reader->record; finding->line = reader->line;
        finding->code = condition ? "rules.condition_undetermined":"rules.effect_undetermined";
        finding->field = field; finding->reference = text;
        finding->message = "State/target remains undetermined; incomplete or open definitions do not infer a binding";
        return;
    }
    code = target.resolution == GD_RULE_AMBIGUOUS ? (condition ? "rules.condition_ambiguous":"rules.effect_ambiguous") :
           target.resolution == GD_RULE_WRONG_TYPE ? (condition ? "rules.condition_type":"rules.effect_type") :
           target.resolution == GD_RULE_INVALID_DEFINITION ? (condition ? "rules.condition_definition":"rules.effect_definition") :
           condition ? "rules.condition_missing":"rules.effect_missing";
    message = target.resolution == GD_RULE_AMBIGUOUS ? "Name has multiple declared matches; qualify the target, never guess" :
              target.resolution == GD_RULE_WRONG_TYPE ? "State predicates/becomes require enum axes; set requires integer or number" :
              target.resolution == GD_RULE_INVALID_DEFINITION ? "Name targets an invalid axis definition" :
              "Name is absent from the declared actor inventory or closed vocabulary";
    gd_error(reader->model,code,message,reader->model->records[reader->record].source_path,reader->line);
}
static void states(RuleReader *reader, GD_RuleParticipant *participant) {
    const char *start = participant->becomes, *cursor;
    int before = reader->rules->state_effect_count;
    participant->first_state_effect = before;
    if (!start) return;
    if (reader->effects_limit) { reader->rules->effect_bindings_complete = 0; return; }
    /* Validate/retain the entire comma list before resolving any item. An empty
       entry or capacity failure contributes no valid-looking partial list. */
    for (cursor = start;; ++cursor) if (!*cursor || *cursor == ',') {
        GD_RuleStateEffect *effect;
        const char *end = cursor, *dot;
        int malformed;
        while (start < end && (*start == ' ' || *start == '\t')) ++start;
        while (end > start && (end[-1] == ' ' || end[-1] == '\t')) --end;
        dot = memchr(start,'.',(size_t)(end-start));
        malformed = start == end || (dot && (dot == start || dot == end-1));
        if (malformed || reader->rules->state_effect_count == GD_MAX_RULE_STATE_EFFECTS) {
            int full = !malformed;
            reader->rules->state_effect_count = before; reader->rules->effect_bindings_complete = 0;
            reader->effects_limit = full;
            gd_error(reader->model,full ? "rules.state_effect_limit":"rules.state_list",
                     full ? "8192 state-effect limit exceeded; current and later effect lists not resolved" :
                            "becomes requires nonempty comma-separated states; qualification requires AXIS.STATE",
                     reader->model->records[reader->record].source_path,reader->line);
            return;
        }
        effect = &reader->rules->state_effects[reader->rules->state_effect_count++];
        effect->text = start; effect->length = (size_t)(end-start); effect->line = reader->line;
        if (!*cursor) break;
        start = cursor+1;
    }
    participant->state_effect_count = reader->rules->state_effect_count-before;
    for (before = participant->first_state_effect; before < reader->rules->state_effect_count; ++before) {
        GD_RuleStateEffect *effect = &reader->rules->state_effects[before];
        effect->target = lookup(reader,participant,effect->text,effect->length,1);
        diagnose(reader,effect->target,"becomes",participant->becomes,NULL);
    }
}
typedef struct { int operation, offset; const char *text; size_t length; } ConditionOperator;
static int word(const char *text, size_t size, const char *keyword) {
    size_t i;
    if (size != strlen(keyword)) return 0;
    for (i = 0; i < size; ++i) {
        unsigned c = (unsigned char)text[i];
        if (c >= 'A' && c <= 'Z') c += 'a'-'A';
        if (c != (unsigned char)keyword[i]) return 0;
    }
    return 1;
}
static int precedence(int operation) {
    return operation == GD_RULE_OP_NOT ? 3:operation == GD_RULE_OP_AND ? 2:1;
}
static int emit(RuleReader *reader, ConditionOperator item) {
    GD_RuleConditionToken *token;
    if (reader->rules->condition_token_count == GD_MAX_RULE_CONDITION_TOKENS) {
        reader->condition_limit = 1; return 0;
    }
    token = &reader->rules->condition_tokens[reader->rules->condition_token_count++];
    memset(token,0,sizeof(*token)); token->target.axis_record = -1;
    token->operation = (GD_RuleConditionOp)item.operation; token->offset = item.offset;
    token->text = item.text; token->length = item.length;
    return 1;
}
static void condition_failure(RuleReader *reader, GD_RuleCondition *condition,
                              GD_RuleConditionState state, const char *code, const char *message) {
    reader->rules->condition_token_count = condition->first_token;
    condition->token_count = 0; condition->bindings_complete = 0; condition->state = state;
    reader->rules->condition_bindings_complete = 0;
    gd_error(reader->model,code,message,reader->model->records[reader->record].source_path,reader->line);
}
static void condition(RuleReader *reader, GD_RuleParticipant *participant, const char *text,
                      const char *field, GD_RuleCondition *result) {
    ConditionOperator operators[GD_MAX_RULE_CONDITION_ITEMS];
    const char *cursor = text;
    int count = 0, depth = 0, items = 0, operand = 1, i;
    result->first_token = reader->rules->condition_token_count;
    result->participant = (int)(participant-reader->rules->participants);
    if (!text) return;
    ++reader->rules->condition_count;
    if (reader->condition_limit) {
        result->state = GD_CONDITION_LIMITED; reader->rules->condition_bindings_complete = 0; return;
    }
    if (strpbrk(text,"<>=") != NULL) {
        GD_Finding *finding;
        result->state = GD_CONDITION_UNSUPPORTED; reader->rules->condition_bindings_complete = 0;
        if (reader->rules->finding_count == GD_MAX_ERRORS) { ++reader->rules->findings_omitted; return; }
        finding = &reader->rules->findings[reader->rules->finding_count++];
        finding->record = reader->record; finding->line = reader->line;
        finding->code = "rules.condition_unsupported"; finding->field = field; finding->reference = text;
        finding->message = "Numeric comparisons are not compiled yet; condition retained and truth remains unknown";
        return;
    }
    while (*cursor) {
        ConditionOperator item;
        const char *start;
        if (*cursor == ' ' || *cursor == '\t') { ++cursor; continue; }
        if (++items > GD_MAX_RULE_CONDITION_ITEMS) goto token_limit;
        start = cursor; item.offset = (int)(start-text); item.text = start; item.length = 1;
        if (*cursor == '(') {
            if (!operand) goto syntax;
            if (++depth > GD_MAX_RULE_CONDITION_DEPTH) {
                condition_failure(reader,result,GD_CONDITION_LIMITED,"rules.condition_depth","Condition nesting exceeds 64 groups"); return;
            }
            item.operation = -1; operators[count++] = item; ++cursor; continue;
        }
        if (*cursor == ')') {
            if (operand || !depth) goto syntax;
            while (count && operators[count-1].operation != -1) if (!emit(reader,operators[--count])) goto token_limit;
            assert(count); --count; --depth; ++cursor; continue;
        }
        if (*cursor == '!' || *cursor == '&' || *cursor == '|') {
            item.operation = *cursor == '!' ? GD_RULE_OP_NOT:*cursor == '&' ? GD_RULE_OP_AND:GD_RULE_OP_OR;
            if (*cursor != '!') {
                if (cursor[1] != *cursor) goto syntax;
                item.length = 2; ++cursor;
            }
            ++cursor;
        } else {
            const char *dot;
            while (*cursor && *cursor != ' ' && *cursor != '\t' && !strchr("()!&|",*cursor)) ++cursor;
            item.length = (size_t)(cursor-start);
            item.operation = word(start,item.length,"not") ? GD_RULE_OP_NOT:word(start,item.length,"and") ? GD_RULE_OP_AND:
                             word(start,item.length,"or") ? GD_RULE_OP_OR:word(start,item.length,"any") ? GD_RULE_OP_ANY:GD_RULE_OP_STATE;
            dot = memchr(start,'.',item.length);
            if (item.operation == GD_RULE_OP_STATE && dot && (dot == start || dot == cursor-1)) goto syntax;
        }
        if (item.operation == GD_RULE_OP_STATE || item.operation == GD_RULE_OP_ANY) {
            if (!operand) goto syntax;
            if (!emit(reader,item)) goto token_limit;
            operand = 0;
        } else if (item.operation == GD_RULE_OP_NOT) {
            if (!operand) goto syntax;
            operators[count++] = item;
        } else {
            if (operand) goto syntax;
            while (count && operators[count-1].operation != -1 && precedence(operators[count-1].operation) >= precedence(item.operation))
                if (!emit(reader,operators[--count])) goto token_limit;
            operators[count++] = item; operand = 1;
        }
    }
    if (operand || depth) goto syntax;
    while (count) if (!emit(reader,operators[--count])) goto token_limit;
    result->state = GD_CONDITION_PARSED; result->bindings_complete = 1;
    result->token_count = reader->rules->condition_token_count-result->first_token;
    for (i = result->first_token; i < reader->rules->condition_token_count; ++i) {
        GD_RuleConditionToken *token = &reader->rules->condition_tokens[i];
        if (token->operation != GD_RULE_OP_STATE) continue;
        token->target = lookup(reader,participant,token->text,token->length,1);
        diagnose(reader,token->target,field,text,result);
    }
    return;
syntax:
    condition_failure(reader,result,GD_CONDITION_INVALID,"rules.condition_syntax","Expected state names/ANY, grouped AND/OR/NOT (or &&/||/!); no implicit operators"); return;
token_limit:
    condition_failure(reader,result,GD_CONDITION_LIMITED,"rules.condition_tokens","Condition exceeds 256 input tokens or shared 65536 compiled-token capacity");
}
const char *gd_rule_condition_state_name(GD_RuleConditionState state) {
    return state == GD_CONDITION_PARSED ? "parsed":state == GD_CONDITION_INVALID ? "invalid":
           state == GD_CONDITION_UNSUPPORTED ? "unsupported":state == GD_CONDITION_LIMITED ? "limited":"unspecified";
}
const char *gd_rule_condition_op_name(GD_RuleConditionOp operation) {
    return operation == GD_RULE_OP_STATE ? "state":operation == GD_RULE_OP_ANY ? "any":operation == GD_RULE_OP_NOT ? "not":
           operation == GD_RULE_OP_AND ? "and":operation == GD_RULE_OP_OR ? "or":"unknown";
}
static GD_RuleTruth negate(GD_RuleTruth value) {
    return value == GD_RULE_TRUTH_TRUE ? GD_RULE_TRUTH_FALSE:value == GD_RULE_TRUTH_FALSE ? GD_RULE_TRUTH_TRUE:GD_RULE_TRUTH_UNKNOWN;
}
static GD_RuleTruth predicate(const GD_RuleCondition *condition, GD_RuleTarget target,
                              const GD_RulePredicateFact *facts, int count) {
    GD_RuleTruth value = GD_RULE_TRUTH_UNKNOWN;
    int i, found = 0, conflict = 0;
    if (target.resolution != GD_RULE_RESOLVED || !target.state) return value;
    for (i = 0; i < count; ++i) {
        GD_RuleTruth next;
        if (facts[i].participant != condition->participant || facts[i].axis_record != target.axis_record ||
            !facts[i].state || strcmp(facts[i].state,target.state)) continue;
        next = facts[i].truth >= GD_RULE_TRUTH_UNKNOWN && facts[i].truth <= GD_RULE_TRUTH_TRUE ? facts[i].truth:GD_RULE_TRUTH_UNKNOWN;
        if (found && next != value) conflict = 1;
        value = next; found = 1;
    }
    return conflict ? GD_RULE_TRUTH_UNKNOWN:value;
}
GD_RuleTruth gd_rule_condition_value(const GD_Rules *rules, const GD_RuleCondition *condition,
                                    const GD_RulePredicateFact *facts, int count) {
    GD_RuleTruth stack[GD_MAX_RULE_CONDITION_ITEMS];
    int i, used = 0;
    if (!rules || !condition || rules->condition_token_count < 0 || rules->condition_token_count > GD_MAX_RULE_CONDITION_TOKENS ||
        condition->state != GD_CONDITION_PARSED || count < 0 || count > GD_MAX_RULE_CONDITION_ITEMS ||
        (count && !facts) || condition->first_token < 0 || condition->token_count < 1 ||
        condition->token_count > GD_MAX_RULE_CONDITION_ITEMS || condition->first_token > rules->condition_token_count-condition->token_count)
        return GD_RULE_TRUTH_UNKNOWN;
    for (i = 0; i < condition->token_count; ++i) {
        const GD_RuleConditionToken *token = &rules->condition_tokens[condition->first_token+i];
        GD_RuleTruth a, b;
        switch (token->operation) {
            case GD_RULE_OP_STATE: stack[used++] = predicate(condition,token->target,facts,count); break;
            case GD_RULE_OP_ANY: stack[used++] = GD_RULE_TRUTH_TRUE; break;
            case GD_RULE_OP_NOT:
                if (!used) return GD_RULE_TRUTH_UNKNOWN;
                stack[used-1] = negate(stack[used-1]); break;
            case GD_RULE_OP_AND: case GD_RULE_OP_OR:
                if (used < 2) return GD_RULE_TRUTH_UNKNOWN;
                b = stack[--used]; a = stack[used-1];
                stack[used-1] = token->operation == GD_RULE_OP_AND ?
                    (a == GD_RULE_TRUTH_FALSE || b == GD_RULE_TRUTH_FALSE ? GD_RULE_TRUTH_FALSE:
                     a == GD_RULE_TRUTH_TRUE && b == GD_RULE_TRUTH_TRUE ? GD_RULE_TRUTH_TRUE:GD_RULE_TRUTH_UNKNOWN) :
                    (a == GD_RULE_TRUTH_TRUE || b == GD_RULE_TRUTH_TRUE ? GD_RULE_TRUTH_TRUE:
                     a == GD_RULE_TRUTH_FALSE && b == GD_RULE_TRUTH_FALSE ? GD_RULE_TRUTH_FALSE:GD_RULE_TRUTH_UNKNOWN);
                break;
            default: return GD_RULE_TRUTH_UNKNOWN;
        }
    }
    return used == 1 ? stack[0]:GD_RULE_TRUTH_UNKNOWN;
}
void gd_resolve_rule_logic(GD_Model *model, GD_Rules *rules) {
    RuleReader reader;
    int i, j, k;
    assert(model && rules);
    memset(&reader,0,sizeof(reader)); reader.model = model; reader.rules = rules;
    rules->effect_bindings_complete = 1; rules->condition_bindings_complete = 1;
    for (i = 0; i < rules->case_count; ++i) {
        GD_RuleCase *item = &rules->cases[i];
        reader.record = rules->sources[item->source].record;
        for (j = 0; j < item->participant_count; ++j) {
            GD_RuleParticipant *participant = &rules->participants[item->first_participant+j];
            reader.line = participant->lines[1]; condition(&reader,participant,participant->requires,"requires",&participant->required_condition);
            reader.line = participant->lines[2]; condition(&reader,participant,participant->skip_if,"skip_if",&participant->skip_condition);
            if (participant->other_axes) {
                const char *value = participant->other_axes;
                while (*value == ' ' || *value == '\t') ++value;
                participant->other_axes_mode = word(value,strlen(value),"any") ? GD_RULE_OTHER_ANY:GD_RULE_OTHER_INVALID;
                if (participant->other_axes_mode == GD_RULE_OTHER_INVALID) {
                    rules->condition_bindings_complete = 0;
                    gd_error(model,"rules.other_axes","other_axes supports ANY only; omit for unspecified remaining axes",
                             model->records[reader.record].source_path,participant->lines[3]);
                }
            }
            reader.line = participant->lines[4]; states(&reader,participant);
            for (k = 0; k < participant->assignment_count; ++k) {
                GD_RuleAssignment *assignment = &rules->assignments[participant->first_assignment+k];
                reader.line = assignment->line;
                assignment->target = lookup(&reader,participant,assignment->axis,strlen(assignment->axis),0);
                diagnose(&reader,assignment->target,"set",assignment->axis,NULL);
            }
        }
    }
}
