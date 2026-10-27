#include "gd.h"
#include <assert.h>
#include <string.h>

typedef struct {
    GD_Model *model;
    GD_Rules *rules;
    int seen[GD_MAX_RECORDS], generation, exhausted, effects_limit;
    int record, line;
} EffectReader;

/* Charge byte-comparison upper bounds and vocabulary traversal, not just
   lookup calls. Large strings and repeated references share the same budget. */
static int charge(EffectReader *reader, size_t amount) {
    if (reader->exhausted) return 0;
    if (amount > (size_t)(GD_RULE_SYMBOL_WORK_LIMIT-reader->rules->symbol_work)) {
        reader->exhausted = 1;
        gd_error(reader->model,"rules.symbol_work_limit","Effect symbol work limit exceeded; remaining bindings unavailable",
                 reader->model->records[reader->record].source_path,reader->line);
        return 0;
    }
    reader->rules->symbol_work += (int)amount;
    return 1;
}
static int equal(EffectReader *reader, const char *text, size_t size, const char *value) {
    if (!charge(reader,size+1)) return 0;
    return !strncmp(text,value,size) && !value[size];
}
static const char *named_state(EffectReader *reader, int axis, const char *text, size_t size) {
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
static GD_RuleTarget lookup(EffectReader *reader, const GD_RuleParticipant *participant,
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
    assert(reader->generation <= GD_MAX_RULE_STATE_EFFECTS + GD_MAX_RULE_ASSIGNMENTS);
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
static void diagnose(EffectReader *reader, GD_RuleTarget target, const char *field, const char *text) {
    GD_Finding *finding;
    const char *code, *message;
    if (target.resolution == GD_RULE_RESOLVED) return;
    reader->rules->effect_bindings_complete = 0;
    if (target.resolution == GD_RULE_UNAVAILABLE) return; /* Budget diagnostic already retained. */
    if (target.resolution == GD_RULE_UNDETERMINED) {
        if (reader->rules->finding_count == GD_MAX_ERRORS) { ++reader->rules->findings_omitted; return; }
        finding = &reader->rules->findings[reader->rules->finding_count++];
        finding->record = reader->record; finding->line = reader->line;
        finding->code = "rules.effect_undetermined"; finding->field = field; finding->reference = text;
        finding->message = "Effect target/state remains undetermined; incomplete or open definitions do not infer a binding";
        return;
    }
    code = target.resolution == GD_RULE_AMBIGUOUS ? "rules.effect_ambiguous" :
           target.resolution == GD_RULE_WRONG_TYPE ? "rules.effect_type" :
           target.resolution == GD_RULE_INVALID_DEFINITION ? "rules.effect_definition":"rules.effect_missing";
    message = target.resolution == GD_RULE_AMBIGUOUS ? "Effect name has multiple declared matches; qualify the target, never guess" :
              target.resolution == GD_RULE_WRONG_TYPE ? "becomes requires an enum axis; set requires an integer or number axis" :
              target.resolution == GD_RULE_INVALID_DEFINITION ? "Effect targets an invalid axis definition" :
              "Effect name is absent from the declared actor inventory or closed vocabulary";
    gd_error(reader->model,code,message,reader->model->records[reader->record].source_path,reader->line);
}
static void states(EffectReader *reader, GD_RuleParticipant *participant) {
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
        diagnose(reader,effect->target,"becomes",participant->becomes);
    }
}
void gd_resolve_rule_effects(GD_Model *model, GD_Rules *rules) {
    EffectReader reader;
    int i, j, k;
    assert(model && rules);
    memset(&reader,0,sizeof(reader)); reader.model = model; reader.rules = rules;
    rules->effect_bindings_complete = 1;
    for (i = 0; i < rules->case_count; ++i) {
        GD_RuleCase *item = &rules->cases[i];
        reader.record = rules->sources[item->source].record;
        for (j = 0; j < item->participant_count; ++j) {
            GD_RuleParticipant *participant = &rules->participants[item->first_participant+j];
            reader.line = participant->lines[4]; states(&reader,participant);
            for (k = 0; k < participant->assignment_count; ++k) {
                GD_RuleAssignment *assignment = &rules->assignments[participant->first_assignment+k];
                reader.line = assignment->line;
                assignment->target = lookup(&reader,participant,assignment->axis,strlen(assignment->axis),0);
                diagnose(&reader,assignment->target,"set",assignment->axis);
            }
        }
    }
}
