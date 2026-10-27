#include "gd.h"
#include <assert.h>
#include <string.h>

typedef enum { BIND_EFFECT, BIND_CONDITION, BIND_CONTEXT, BIND_PRESERVE } BindingKind;
typedef enum { LOOKUP_QUANTITY_AXIS, LOOKUP_ENUM_STATE, LOOKUP_ANY_AXIS } LookupKind;
typedef struct {
    GD_Model *model;
    GD_Rules *rules;
    int seen[GD_MAX_RECORDS], generation, exhausted, effects_limit, condition_limit, preservation_limit;
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
static GD_RuleTarget lookup(RuleReader *reader, int owner, GD_Field inventory,
                            const char *text, size_t size, LookupKind kind) {
    int state=kind==LOOKUP_ENUM_STATE;
    GD_RuleTarget target = {GD_RULE_UNDETERMINED,-1,NULL};
    const char *dot = state ? memchr(text,'.',size):NULL;
    size_t axis_size = dot ? (size_t)(dot-text):size;
    int edge, found = -1, uncertain = 0, open = 0, ambiguous = 0;
    int canonical = (!state || dot) && memchr(text,'/',dot ? axis_size:size) != NULL;
    const char *value = NULL;
    if (reader->exhausted) { target.resolution = GD_RULE_UNAVAILABLE; return target; }
    if (owner < 0) return target;
    assert(owner < reader->model->record_count && (inventory == GD_AXES || inventory == GD_CONTEXT_AXES));
    if (!(inventory == GD_AXES ? reader->rules->actor_axes_complete[owner]:reader->rules->context_axes_complete[owner])) uncertain = 1;
    ++reader->generation;
    assert(reader->generation <= GD_MAX_RULE_STATE_EFFECTS + GD_MAX_RULE_ASSIGNMENTS + GD_MAX_RULE_CONDITION_TOKENS + GD_MAX_RULE_CASES + GD_MAX_RULE_PRESERVATIONS + GD_MAX_RULE_POOLS);
    for (edge = inventory == GD_AXES ? reader->rules->actor_first[owner]:reader->rules->context_first[owner]; edge >= 0; edge = reader->rules->next_axis[edge]) {
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
    if (kind==LOOKUP_ANY_AXIS) { target.resolution=GD_RULE_RESOLVED; return target; }
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
static GD_RuleTarget actor_target(RuleReader *reader, const GD_RuleParticipant *participant,
                                  const char *text, size_t size, LookupKind kind) {
    return lookup(reader,participant->noun_resolution == GD_RULE_RESOLVED ? participant->noun_record:-1,GD_AXES,text,size,kind);
}
static void resolve_pool(RuleReader *reader, const GD_RuleParticipant *participant, GD_RulePool *pool) {
    const GD_Model *model = reader->model;
    const char *slash = strchr(pool->reference,'/');
    int i, found = -1, loaded = 0;
    pool->target = actor_target(reader,participant,pool->axis,strlen(pool->axis),LOOKUP_ANY_AXIS);
    pool->resource_record = -1; pool->resource_resolution = GD_RULE_UNLOADED;
    for (i = 0; i < model->collection_count; ++i) {
        if (!charge(reader,(size_t)(slash-pool->reference)+1)) goto limited;
        if (strlen(model->collections[i].alias) == (size_t)(slash-pool->reference) &&
            !memcmp(model->collections[i].alias,pool->reference,(size_t)(slash-pool->reference))) loaded = 1;
    }
    for (i = 0; i < model->record_count; ++i) {
        if (!equal(reader,pool->reference,strlen(pool->reference),model->records[i].reference)) {
            if (reader->exhausted) goto limited;
            continue;
        }
        if (found >= 0) { pool->resource_resolution = GD_RULE_AMBIGUOUS; goto done; }
        found = i;
    }
    if (found < 0) pool->resource_resolution = loaded ? (model->error_count || model->errors_omitted ? GD_RULE_UNAVAILABLE:GD_RULE_MISSING):GD_RULE_UNLOADED;
    else {
        pool->resource_record = found;
        pool->resource_resolution = model->records[found].role == GD_RESOURCE ? GD_RULE_RESOLVED:GD_RULE_WRONG_ROLE;
    }
    goto done;
limited:
    pool->resource_resolution = GD_RULE_UNAVAILABLE;
done:
    pool->identity_resolution = pool->target.resolution != GD_RULE_RESOLVED ? pool->target.resolution:pool->resource_resolution;
    if (pool->target.resolution != GD_RULE_RESOLVED || pool->resource_resolution != GD_RULE_RESOLVED) {
        GD_Finding *item;
        reader->rules->pool_bindings_complete = 0;
        if (reader->rules->finding_count == GD_MAX_ERRORS) { ++reader->rules->findings_omitted; return; }
        item = &reader->rules->findings[reader->rules->finding_count++];
        item->record = reader->record; item->line = pool->line; item->field = "pools"; item->reference = pool->reference;
        item->code = "rules.pool_unresolved";
        item->message = "Pool axis or resource reference is unresolved; no shared identity inferred";
    }
}
static void diagnose(RuleReader *reader, GD_RuleTarget target, const char *field,
                     const char *text, BindingKind kind) {
    GD_Finding *finding;
    const char *code, *message;
    if (target.resolution == GD_RULE_RESOLVED) return;
    if (kind == BIND_CONDITION) reader->rules->condition_bindings_complete = 0;
    else if (kind == BIND_CONTEXT) reader->rules->context_bindings_complete = 0;
    else if (kind == BIND_PRESERVE) reader->rules->preservation_bindings_complete=0;
    else reader->rules->effect_bindings_complete = 0;
    if (target.resolution == GD_RULE_UNAVAILABLE) return; /* Budget diagnostic already retained. */
    if (target.resolution == GD_RULE_UNDETERMINED) {
        if (reader->rules->finding_count == GD_MAX_ERRORS) { ++reader->rules->findings_omitted; return; }
        finding = &reader->rules->findings[reader->rules->finding_count++];
        finding->record = reader->record; finding->line = reader->line;
        finding->code = kind == BIND_PRESERVE ? "rules.preserve_undetermined":kind == BIND_CONTEXT ? "rules.context_undetermined":kind == BIND_CONDITION ? "rules.condition_undetermined":"rules.effect_undetermined";
        finding->field = field; finding->reference = text;
        finding->message = "State/target remains undetermined; incomplete or open definitions do not infer a binding";
        return;
    }
    code = target.resolution == GD_RULE_AMBIGUOUS ? (kind == BIND_PRESERVE ? "rules.preserve_ambiguous":kind == BIND_CONTEXT ? "rules.context_ambiguous":kind == BIND_CONDITION ? "rules.condition_ambiguous":"rules.effect_ambiguous") :
           target.resolution == GD_RULE_WRONG_TYPE ? (kind == BIND_PRESERVE ? "rules.preserve_type":kind == BIND_CONTEXT ? "rules.context_type":kind == BIND_CONDITION ? "rules.condition_type":"rules.effect_type") :
           target.resolution == GD_RULE_INVALID_DEFINITION ? (kind == BIND_PRESERVE ? "rules.preserve_definition":kind == BIND_CONTEXT ? "rules.context_definition":kind == BIND_CONDITION ? "rules.condition_definition":"rules.effect_definition") :
           kind == BIND_PRESERVE ? "rules.preserve_missing":kind == BIND_CONTEXT ? "rules.context_missing":kind == BIND_CONDITION ? "rules.condition_missing":"rules.effect_missing";
    message = target.resolution == GD_RULE_AMBIGUOUS ? "Name has multiple declared matches; qualify the target, never guess" :
              target.resolution == GD_RULE_WRONG_TYPE ? "Named predicates/becomes require enum axes; numeric predicates/set require integer or number" :
              target.resolution == GD_RULE_INVALID_DEFINITION ? "Name targets an invalid axis definition" :
              "Name is absent from the declared axis inventory or closed vocabulary";
    gd_error(reader->model,code,message,reader->model->records[reader->record].source_path,reader->line);
}
static int control_reference(RuleReader *reader, const char *text, size_t length, int *record) {
    GD_ControlToken token;
    GD_Expression expression={&token,0,1};
    GD_ExpressionSite site={reader->record,reader->line,GD_EXPRESSION_PREDICATE};
    if (!length || (text[0]!='{' && !(length>1 && text[0]=='$' && text[1]=='{'))) return 0;
    *record=-1;
    if (!charge(reader,(length+1)*32)) return 1;
    if (gd_compile_expression(reader->model,&expression,text,length,site) && expression.count==1)
        *record=token.target;
    return 1;
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
        effect->text = start; effect->length = (size_t)(end-start); effect->line = reader->line; effect->control_record=-1;
        if (!*cursor) break;
        start = cursor+1;
    }
    participant->state_effect_count = reader->rules->state_effect_count-before;
    for (before = participant->first_state_effect; before < reader->rules->state_effect_count; ++before) {
        GD_RuleStateEffect *effect = &reader->rules->state_effects[before];
        if (control_reference(reader,effect->text,effect->length,&effect->control_record)) {
            effect->target=(GD_RuleTarget){GD_RULE_UNDETERMINED,-1,NULL}; continue;
        }
        effect->target = actor_target(reader,participant,effect->text,effect->length,1);
        diagnose(reader,effect->target,"becomes",participant->becomes,BIND_EFFECT);
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
static void preservation(RuleReader *reader, GD_RuleParticipant *participant) {
    const char *cursor=participant->preserve;
    int before=reader->rules->preservation_count, i;
    participant->first_preservation=before;
    if (!cursor) return;
    if (reader->preservation_limit) { reader->rules->preservation_bindings_complete=0; return; }
    while (*cursor) {
        const char *start,*end;
        GD_RulePreservation *item;
        while (*cursor==' ' || *cursor=='\t') ++cursor;
        start=cursor; while (*cursor && *cursor!=',') ++cursor; end=cursor;
        while (end>start && (end[-1]==' ' || end[-1]=='\t')) --end;
        if (end==start || (*cursor==',' && !cursor[1])) {
            reader->rules->preservation_count=before; reader->rules->preservation_bindings_complete=0;
            gd_error(reader->model,"rules.preserve_list","preserve requires nonempty comma-separated axis names, ALL or OTHER",reader->model->records[reader->record].source_path,reader->line);
            return;
        }
        if (reader->rules->preservation_count==GD_MAX_RULE_PRESERVATIONS) {
            reader->rules->preservation_count=before; reader->rules->preservation_bindings_complete=0; reader->preservation_limit=1;
            gd_error(reader->model,"rules.preserve_limit","8192 preservation declaration limit exceeded; current and later lists unavailable",reader->model->records[reader->record].source_path,reader->line);
            return;
        }
        item=&reader->rules->preservations[reader->rules->preservation_count++]; memset(item,0,sizeof(*item));
        item->text=start; item->length=(size_t)(end-start); item->line=reader->line;
        item->kind=word(start,item->length,"all") ? GD_PRESERVE_ALL:word(start,item->length,"other") ? GD_PRESERVE_OTHER:GD_PRESERVE_AXIS;
        if (*cursor==',') ++cursor;
    }
    participant->preservation_count=reader->rules->preservation_count-before;
    for (i=before;i<reader->rules->preservation_count;++i) {
        GD_RulePreservation *item=&reader->rules->preservations[i];
        if (item->kind==GD_PRESERVE_AXIS) item->target=actor_target(reader,participant,item->text,item->length,LOOKUP_ANY_AXIS);
        else item->target=(GD_RuleTarget){participant->noun_resolution==GD_RULE_RESOLVED && reader->rules->actor_axes_complete[participant->noun_record] ? GD_RULE_RESOLVED:GD_RULE_UNDETERMINED,-1,NULL};
        diagnose(reader,item->target,"preserve",participant->preserve,BIND_PRESERVE);
    }
}
static int context_participant(RuleReader *reader, const GD_RuleCase *item, const char *text, size_t length) {
    int i;
    for (i = item->first_participant; i < item->first_participant+item->participant_count; ++i)
        if (equal(reader,text,length,reader->rules->participants[i].name)) return i;
        else if (reader->exhausted) break;
    if (!reader->exhausted)
        gd_error(reader->model,"rules.context_participant","Ordered context endpoints must name declared participants in this case; never guess by noun or another case",
                 reader->model->records[reader->record].source_path,reader->line);
    return -1;
}
static void ordered_context(RuleReader *reader, GD_RuleCase *item) {
    GD_RuleContext *context = &item->ordered_context;
    const char *cursor = item->context, *parts[3], *dot;
    size_t lengths[3]; int count = 0;
    context->left = context->right = context->target.axis_record = -1;
    if (!cursor) return;
    ++reader->rules->context_count;
    while (*cursor) {
        if (*cursor == ' ' || *cursor == '\t') { ++cursor; continue; }
        if (count == 3) goto syntax;
        parts[count] = cursor;
        while (*cursor && *cursor != ' ' && *cursor != '\t') ++cursor;
        lengths[count] = (size_t)(cursor-parts[count]); ++count;
    }
    if (count == 1 && word(parts[0],lengths[0],"any")) {
        context->state = GD_CONDITION_PARSED; context->unrestricted = context->bindings_complete = 1; return;
    }
    if (count != 3) goto syntax;
    dot = memchr(parts[1],'.',lengths[1]);
    if (dot && (dot == parts[1] || dot == parts[1]+lengths[1]-1)) goto syntax;
    context->state = GD_CONDITION_PARSED; context->operator_text = parts[1]; context->operator_length = lengths[1];
    context->operator_offset = (int)(parts[1]-item->context);
    context->left = context_participant(reader,item,parts[0],lengths[0]);
    context->right = context_participant(reader,item,parts[2],lengths[2]);
    context->target = lookup(reader,reader->record,GD_CONTEXT_AXES,parts[1],lengths[1],1);
    context->bindings_complete = context->left >= 0 && context->right >= 0 && context->target.resolution == GD_RULE_RESOLVED;
    if (!context->bindings_complete) reader->rules->context_bindings_complete = 0;
    diagnose(reader,context->target,"context",item->context,BIND_CONTEXT);
    return;
syntax:
    context->state = GD_CONDITION_INVALID; reader->rules->context_bindings_complete = 0;
    gd_error(reader->model,"rules.context_syntax","Context requires ANY or one ordered LEFT OPERATOR RIGHT predicate; no implicit participants or Boolean composition",
             reader->model->records[reader->record].source_path,reader->line);
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
    token->quantity = token->control_record = -1;
    token->operation = (GD_RuleConditionOp)item.operation; token->offset = item.offset;
    token->text = item.text; token->length = item.length;
    return 1;
}
const char *gd_rule_comparison_name(int comparison) {
    static const char *names[]={"==","!=","<","<=",">",">="};
    return comparison>=GD_RULE_EQUAL && comparison<=GD_RULE_GREATER_EQUAL ? names[comparison]:"unknown";
}
/* Arithmetic parentheses/functions belong to a numeric atom. Stop at a Boolean
   boundary outside those parentheses, never inside a unit or control reference. */
static const char *quantity_end(const char *cursor) {
    const char *start=cursor;
    int depth=0;
    while (*cursor) {
        if (*cursor=='[' || *cursor=='{') {
            char close=*cursor=='[' ? ']':'}'; ++cursor;
            while (*cursor && *cursor!=close) ++cursor;
            if (*cursor) ++cursor;
            continue;
        }
        if (*cursor=='(') ++depth;
        else if (*cursor==')') { if (!depth) break; --depth; }
        if (!depth && (*cursor=='&' || *cursor=='|')) break;
        if (!depth && (cursor==start || cursor[-1]==' ' || cursor[-1]=='\t')) {
            const char *end=cursor;
            while ((*end>='A' && *end<='Z') || (*end>='a' && *end<='z')) ++end;
            if ((!*end || *end==' ' || *end=='\t' || *end==')') &&
                (word(cursor,(size_t)(end-cursor),"and") || word(cursor,(size_t)(end-cursor),"or"))) break;
        }
        ++cursor;
    }
    return cursor;
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
            while (*cursor && *cursor != ' ' && *cursor != '\t' && !strchr("()!&|<>=",*cursor)) ++cursor;
            item.length = (size_t)(cursor-start);
            item.operation = word(start,item.length,"not") ? GD_RULE_OP_NOT:word(start,item.length,"and") ? GD_RULE_OP_AND:
                             word(start,item.length,"or") ? GD_RULE_OP_OR:word(start,item.length,"any") ? GD_RULE_OP_ANY:GD_RULE_OP_STATE;
            dot = memchr(start,'.',item.length);
            if (!item.length || (item.operation == GD_RULE_OP_STATE && dot && (dot == start || dot == cursor-1))) goto syntax;
            {
                const char *operator=cursor;
                while (*operator==' ' || *operator=='\t') ++operator;
                if (*operator=='<' || *operator=='>' || *operator=='=' || (*operator=='!' && operator[1]=='=')) {
                    const char *rhs,*end,*trim;
                    GD_RuleConditionToken *token;
                    int comparison=*operator=='<' ? GD_RULE_LESS:*operator=='>' ? GD_RULE_GREATER:*operator=='!' ? GD_RULE_NOT_EQUAL:GD_RULE_EQUAL;
                    if (!operand || item.operation!=GD_RULE_OP_STATE) goto syntax;
                    rhs=operator+1;
                    if (*rhs=='=') { if (comparison==GD_RULE_LESS || comparison==GD_RULE_GREATER) ++comparison; ++rhs; }
                    while (*rhs==' ' || *rhs=='\t') ++rhs;
                    end=quantity_end(rhs); trim=end;
                    while (trim>rhs && (trim[-1]==' ' || trim[-1]=='\t')) --trim;
                    if (trim==rhs) goto syntax;
                    item.operation=GD_RULE_OP_COMPARE;
                    if (!emit(reader,item)) goto token_limit;
                    token=&reader->rules->condition_tokens[reader->rules->condition_token_count-1];
                    token->axis_length=(int)item.length; token->rhs_offset=(int)(rhs-start); token->comparison=comparison;
                    token->length=(size_t)(trim-start);
                    cursor=end; operand=0; continue;
                }
            }
        }
        if (item.operation == GD_RULE_OP_STATE || item.operation == GD_RULE_OP_ANY) {
            int control=-1, reference=item.operation==GD_RULE_OP_STATE && control_reference(reader,item.text,item.length,&control);
            if (!operand) goto syntax;
            if (reference && control==-1) {
                condition_failure(reader,result,reader->exhausted ? GD_CONDITION_LIMITED:GD_CONDITION_INVALID,
                    "rules.condition_control","Invalid/unavailable boolean control reference; see sourced lookup/compiler error");
                return;
            }
            if (reference) item.operation=GD_RULE_OP_CONTROL;
            if (!emit(reader,item)) goto token_limit;
            if (reference) reader->rules->condition_tokens[reader->rules->condition_token_count-1].control_record=control;
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
        if (token->operation==GD_RULE_OP_CONTROL) { if (token->control_record<0) result->bindings_complete=0; continue; }
        if (token->operation != GD_RULE_OP_STATE && token->operation != GD_RULE_OP_COMPARE) continue;
        token->target = actor_target(reader,participant,token->text,token->operation==GD_RULE_OP_COMPARE ? (size_t)token->axis_length:token->length,
                                     token->operation==GD_RULE_OP_STATE);
        if (token->target.resolution != GD_RULE_RESOLVED) result->bindings_complete = 0;
        diagnose(reader,token->target,field,text,BIND_CONDITION);
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
           operation == GD_RULE_OP_AND ? "and":operation == GD_RULE_OP_OR ? "or":operation == GD_RULE_OP_COMPARE ? "compare":operation == GD_RULE_OP_CONTROL ? "control":"unknown";
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
            !facts[i].value || strcmp(facts[i].value,target.state)) continue;
        next = facts[i].truth >= GD_RULE_TRUTH_UNKNOWN && facts[i].truth <= GD_RULE_TRUTH_TRUE ? facts[i].truth:GD_RULE_TRUTH_UNKNOWN;
        if (found && next != value) conflict = 1;
        value = next; found = 1;
    }
    return conflict ? GD_RULE_TRUTH_UNKNOWN:value;
}
static GD_RuleTruth numeric_predicate(const GD_Rules *rules, const GD_RuleCondition *condition,
                                      const GD_RuleConditionToken *token, const GD_RulePredicateFact *facts, int count) {
    GD_ExpressionInput left=gd_rule_starting_quantity(rules,condition->participant,token->target.axis_record,facts,count);
    GD_RuleQuantityValue right=gd_rule_quantity_value(rules,token->quantity,facts,count);
    int value;
    if (!left.known || !left.unit_known || right.state!=GD_EXPRESSION_VALUE || !right.known || !right.checks_complete) return GD_RULE_TRUTH_UNKNOWN;
    switch (token->comparison) {
        case GD_RULE_EQUAL: value=left.value.number==right.number; break;
        case GD_RULE_NOT_EQUAL: value=left.value.number!=right.number; break;
        case GD_RULE_LESS: value=left.value.number<right.number; break;
        case GD_RULE_LESS_EQUAL: value=left.value.number<=right.number; break;
        case GD_RULE_GREATER: value=left.value.number>right.number; break;
        case GD_RULE_GREATER_EQUAL: value=left.value.number>=right.number; break;
        default: return GD_RULE_TRUTH_UNKNOWN;
    }
    return value ? GD_RULE_TRUTH_TRUE:GD_RULE_TRUTH_FALSE;
}
static int compiled_condition(const GD_Rules *rules, const GD_RuleCondition *condition) {
    return rules && condition && rules->condition_token_count >= 0 && rules->condition_token_count <= GD_MAX_RULE_CONDITION_TOKENS &&
        condition->state == GD_CONDITION_PARSED && condition->first_token >= 0 && condition->token_count >= 1 &&
        condition->token_count <= GD_MAX_RULE_CONDITION_ITEMS && condition->first_token <= rules->condition_token_count-condition->token_count;
}
GD_RuleTruth gd_rule_condition_value(const GD_Rules *rules, const GD_RuleCondition *condition,
                                    const GD_RulePredicateFact *facts, int count) {
    GD_RuleTruth stack[GD_MAX_RULE_CONDITION_ITEMS];
    int i, used = 0;
    if (!compiled_condition(rules,condition) || count < 0 || count > GD_MAX_RULE_CONDITION_ITEMS || (count && !facts) ||
        gd_rule_condition_work(rules,condition,count)>GD_RULE_EVALUATION_WORK_LIMIT)
        return GD_RULE_TRUTH_UNKNOWN;
    for (i = 0; i < condition->token_count; ++i) {
        const GD_RuleConditionToken *token = &rules->condition_tokens[condition->first_token+i];
        GD_RuleTruth a, b;
        switch (token->operation) {
            case GD_RULE_OP_STATE: stack[used++] = predicate(condition,token->target,facts,count); break;
            case GD_RULE_OP_COMPARE: stack[used++] = numeric_predicate(rules,condition,token,facts,count); break;
            case GD_RULE_OP_CONTROL:
                stack[used++]=rules->controls && token->control_record>=0 && token->control_record<GD_MAX_RECORDS && rules->controls->state[token->control_record]==1 &&
                    rules->controls->values[token->control_record].kind==GD_LITERAL_BOOLEAN ?
                    (rules->controls->values[token->control_record].number ? GD_RULE_TRUTH_TRUE:GD_RULE_TRUTH_FALSE):GD_RULE_TRUTH_UNKNOWN;
                break;
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
/* Reserve byte-comparison upper bounds before evaluating a whole guard. A
   budget failure leaves that guard unexamined, not partially false. */
size_t gd_rule_condition_work(const GD_Rules *rules, const GD_RuleCondition *condition, int count) {
    size_t cost = 1;
    int i;
    if (count<0 || count>GD_MAX_RULE_CONDITION_ITEMS) return GD_RULE_EVALUATION_WORK_LIMIT+1u;
    if (!compiled_condition(rules,condition)) return cost;
    for (i = 0; i < condition->token_count; ++i) {
        const GD_RuleConditionToken *token = &rules->condition_tokens[condition->first_token+i];
        if (token->length > GD_MAX_RULE_LINE) return GD_RULE_EVALUATION_WORK_LIMIT+1u;
        ++cost;
        if (token->operation == GD_RULE_OP_STATE && token->target.resolution == GD_RULE_RESOLVED && token->target.state)
            cost += (size_t)count*(token->length+2); /* Fact-key check plus bounded spelling comparison. */
        if (token->operation==GD_RULE_OP_COMPARE) {
            int tokens=token->quantity>=0 && token->quantity<rules->quantity_count ? rules->quantities[token->quantity].token_count:0;
            cost += (size_t)(tokens+1)*GD_RULE_NUMERIC_TOKEN_WORK+(size_t)count*GD_RULE_NUMERIC_FACT_WORK;
        }
    }
    return cost;
}
GD_RuleExclusionCheck gd_rule_case_exclusions(const GD_Rules *rules, int case_index,
                                            const GD_RulePredicateFact *facts, int count) {
    GD_RuleExclusionCheck result = {GD_EXCLUSION_UNKNOWN,0,0,-1,0,0};
    const GD_RuleCase *item;
    int i, unknown = 0;
    if (!rules || rules->case_count < 0 || rules->case_count > GD_MAX_RULE_CASES || case_index < 0 || case_index >= rules->case_count ||
        rules->participant_count < 0 || rules->participant_count > GD_MAX_RULE_PARTICIPANTS || count < 0 || count > GD_MAX_RULE_CONDITION_ITEMS ||
        (count && !facts)) return result;
    item = &rules->cases[case_index];
    if (item->first_participant < 0 || item->participant_count < 0 || item->first_participant > rules->participant_count-item->participant_count ||
        item->skip_if_count < 0 || item->skip_if_count > item->participant_count) return result;
    result.guard_count = item->skip_if_count;
    if (!result.guard_count) { result.state = GD_EXCLUSION_ABSENT; return result; }
    for (i = item->first_participant; i < item->first_participant+item->participant_count; ++i) {
        const GD_RuleParticipant *participant = &rules->participants[i];
        GD_RuleTruth value;
        size_t cost = participant->skip_if ? gd_rule_condition_work(rules,&participant->skip_condition,count):1;
        if (cost > (size_t)(GD_RULE_EVALUATION_WORK_LIMIT-result.work)) { result.limited = 1; return result; }
        result.work += (int)cost;
        if (!participant->skip_if) continue; /* Absence is no authored guard, never a fabricated false fact. */
        value = gd_rule_condition_value(rules,&participant->skip_condition,facts,count);
        ++result.checked_count;
        if (value == GD_RULE_TRUTH_TRUE) { result.state = GD_EXCLUSION_EXCLUDED; result.excluded_by = i; return result; }
        if (value == GD_RULE_TRUTH_UNKNOWN) unknown = 1;
    }
    result.state = unknown ? GD_EXCLUSION_UNKNOWN:GD_EXCLUSION_NOT_EXCLUDED;
    return result;
}
void gd_resolve_rule_controls(GD_Model *model, GD_Rules *rules) {
    RuleReader reader;
    int c,p,i,k;
    memset(&reader,0,sizeof(reader)); reader.model=model; reader.rules=rules;
    for (c=0;c<rules->case_count;++c) {
        const GD_RuleCase *item=&rules->cases[c]; reader.record=rules->sources[item->source].record;
        for (p=item->first_participant;p<item->first_participant+item->participant_count;++p) {
            GD_RuleParticipant *participant=&rules->participants[p];
            for (i=participant->first_state_effect;i<participant->first_state_effect+participant->state_effect_count;++i) {
                GD_RuleStateEffect *effect=&rules->state_effects[i];
                const GD_ControlValue *value;
                if (effect->text[0]!='{' && !(effect->length>1 && effect->text[0]=='$' && effect->text[1]=='{')) continue;
                reader.line=effect->line;
                value=rules->controls && effect->control_record>=0 && rules->controls->state[effect->control_record]==1 ? &rules->controls->values[effect->control_record]:NULL;
                if (value && value->kind!=GD_LITERAL_STRING) {
                    effect->target.resolution=GD_RULE_WRONG_TYPE; rules->effect_bindings_complete=0;
                    gd_error(model,"rules.effect_control_type","becomes control reference requires an enum value; no numeric/boolean coercion",model->records[reader.record].source_path,reader.line);
                    continue;
                }
                if (value) effect->target=actor_target(&reader,participant,value->string,strlen(value->string),1);
                diagnose(&reader,effect->target,"becomes",participant->becomes,BIND_EFFECT);
            }
            for (k=0;k<2;++k) {
                GD_RuleCondition *condition=k ? &participant->skip_condition:&participant->required_condition;
                for (i=condition->first_token;i<condition->first_token+condition->token_count;++i) {
                    const GD_RuleConditionToken *token=&rules->condition_tokens[i];
                    const GD_ControlValue *value;
                    if (token->operation!=GD_RULE_OP_CONTROL) continue;
                    reader.line=participant->lines[k ? 2:1];
                    value=rules->controls && token->control_record>=0 && rules->controls->state[token->control_record]==1 ? &rules->controls->values[token->control_record]:NULL;
                    if (!value || value->kind!=GD_LITERAL_BOOLEAN) {
                        condition->bindings_complete=0; rules->condition_bindings_complete=0;
                        if (value) gd_error(model,"rules.condition_control_type","Control predicate requires a boolean; no numeric/enum truth coercion",model->records[reader.record].source_path,reader.line);
                    }
                }
            }
        }
    }
}
void gd_resolve_rule_logic(GD_Model *model, GD_Rules *rules) {
    RuleReader reader;
    int i, j, k;
    assert(model && rules);
    memset(&reader,0,sizeof(reader)); reader.model = model; reader.rules = rules;
    rules->effect_bindings_complete = 1; rules->condition_bindings_complete = 1; rules->context_bindings_complete = 1;
    rules->preservation_bindings_complete=1; rules->pool_bindings_complete=1;
    for (i = 0; i < rules->case_count; ++i) {
        GD_RuleCase *item = &rules->cases[i];
        reader.record = rules->sources[item->source].record;
        reader.line = item->context_line; ordered_context(&reader,item);
        for (j = 0; j < item->participant_count; ++j) {
            GD_RuleParticipant *participant = &rules->participants[item->first_participant+j];
            reader.line = participant->lines[1]; condition(&reader,participant,participant->requires,"requires",&participant->required_condition);
            if (participant->skip_if) ++item->skip_if_count;
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
                assignment->target = actor_target(&reader,participant,assignment->axis,strlen(assignment->axis),0);
                diagnose(&reader,assignment->target,"set",assignment->axis,BIND_EFFECT);
            }
            for (k = 0; k < participant->pool_count; ++k) {
                GD_RulePool *pool = &rules->pools[participant->first_pool+k];
                reader.line = pool->line;
                resolve_pool(&reader,participant,pool);
            }
            for (k = 0; k < participant->pool_count && !reader.exhausted; ++k) {
                GD_RulePool *pool = &rules->pools[participant->first_pool+k];
                int previous;
                for (previous = 0; previous < k; ++previous) {
                    GD_RulePool *other = &rules->pools[participant->first_pool+previous];
                    reader.line = pool->line;
                    if (!charge(&reader,strlen(pool->reference)+strlen(other->reference)+1)) break;
                    if (pool->target.resolution == GD_RULE_RESOLVED && other->target.resolution == GD_RULE_RESOLVED &&
                        pool->target.axis_record == other->target.axis_record && strcmp(pool->reference,other->reference)) {
                        if (pool->identity_resolution != GD_RULE_AMBIGUOUS) {
                            if (rules->finding_count < GD_MAX_ERRORS) {
                                GD_Finding *finding = &rules->findings[rules->finding_count++];
                                finding->record = reader.record; finding->line = pool->line; finding->field = "pools";
                                finding->reference = pool->reference; finding->code = "rules.pool_conflict";
                                finding->message = "Multiple resource references bind the same participant axis; no shared identity selected";
                            } else ++rules->findings_omitted;
                        }
                        pool->identity_resolution = other->identity_resolution = GD_RULE_AMBIGUOUS;
                        rules->pool_bindings_complete = 0;
                    }
                }
            }
            if (reader.exhausted) {
                for (k = 0; k < participant->pool_count; ++k) rules->pools[participant->first_pool+k].identity_resolution = GD_RULE_UNAVAILABLE;
                rules->pool_bindings_complete = 0;
            }
            reader.line=participant->lines[5]; preservation(&reader,participant);
        }
    }
}
