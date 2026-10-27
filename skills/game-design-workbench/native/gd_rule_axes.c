#include "gd.h"
#include <assert.h>
#include <string.h>

const char *gd_rule_resolution_name(GD_RuleResolution resolution) {
    switch (resolution) {
        case GD_RULE_RESOLVED: return "resolved";
        case GD_RULE_UNLOADED: return "unloaded";
        case GD_RULE_MISSING: return "missing";
        case GD_RULE_WRONG_ROLE: return "wrong_role";
        case GD_RULE_AMBIGUOUS: return "ambiguous";
        case GD_RULE_UNAVAILABLE: return "unavailable";
        default: return "unspecified";
    }
}
static void finding(GD_Rules *rules, int record, int line, const char *code,
                    const char *message, const char *field, const char *reference) {
    GD_Finding *item;
    if (rules->finding_count == GD_MAX_ERRORS) { ++rules->findings_omitted; return; }
    item = &rules->findings[rules->finding_count++];
    item->record = record; item->line = line; item->code = code; item->message = message;
    item->field = field; item->reference = reference;
}
static void axis_error(GD_Model *model, GD_Rules *rules, int record, int line, const char *code, const char *message) {
    gd_error(model,code,message,model->records[record].source_path,line);
    rules->axes[record].valid = 0; rules->axis_definitions_complete = 0;
}
static GD_RuleResolution unresolved(const GD_Model *model, const char *reference) {
    const char *slash = strchr(reference,'/');
    int i;
    if (!slash || slash == reference || !slash[1]) return GD_RULE_MISSING;
    for (i = 0; i < model->collection_count; ++i)
        if (strlen(model->collections[i].alias) == (size_t)(slash-reference) &&
            !memcmp(model->collections[i].alias,reference,(size_t)(slash-reference)))
            return model->error_count || model->errors_omitted ? GD_RULE_UNAVAILABLE:GD_RULE_MISSING;
    return GD_RULE_UNLOADED;
}
static GD_RuleResolution noun_binding(const GD_Model *model, const char *reference, int *target) {
    int i, found = -1;
    *target = -1;
    if (!reference) return GD_RULE_UNSPECIFIED;
    for (i = 0; i < model->record_count; ++i) if (!strcmp(model->records[i].reference,reference)) {
        if (found >= 0) return GD_RULE_AMBIGUOUS;
        found = i;
    }
    if (found < 0) return unresolved(model,reference);
    *target = found;
    return model->records[found].role == GD_NOUN ? GD_RULE_RESOLVED:GD_RULE_WRONG_ROLE;
}
static void validate_axis(GD_Model *model, GD_Rules *rules, int index) {
    const GD_Record *record = &model->records[index];
    const GD_ValueDefinition *definition = &record->axis;
    GD_RuleAxis *axis = &rules->axes[index];
    double minimum = 0, maximum = 0;
    int minimum_valid = 0, maximum_valid = 0;
    axis->valid = 1;
    if (!definition->type || !*definition->type || !strcmp(definition->type,"unspecified")) {
        rules->axis_definitions_complete = 0;
        finding(rules,index,definition->lines[0],"rules.axis_type_unspecified",
                "Axis type remains unspecified; no state or quantity type inferred","value_type",record->reference);
        return;
    }
    axis->type = !strcmp(definition->type,"enum") ? GD_AXIS_ENUM : !strcmp(definition->type,"integer") ? GD_AXIS_INTEGER :
                 !strcmp(definition->type,"number") ? GD_AXIS_NUMBER:GD_AXIS_UNSPECIFIED;
    if (!axis->type) {
        axis_error(model,rules,index,definition->lines[0],"rules.axis_type","Axis value_type requires unspecified, enum, integer or number");
        return;
    }
    if (axis->type == GD_AXIS_ENUM) {
        const char *value = definition->allowed;
        int i, j;
        if ((definition->unit && *definition->unit) || definition->minimum || definition->maximum)
            axis_error(model,rules,index,definition->unit && *definition->unit ? definition->lines[3] : definition->minimum ? definition->lines[5] : definition->lines[6],
                       "rules.axis_shape","Named-state axes do not take numeric units or bounds");
        for (i = 0; i < definition->allowed_count; ++i) {
            const char *previous = definition->allowed;
            if (!*value) axis_error(model,rules,index,definition->lines[4],"rules.axis_value_empty","Named states require nonempty strings");
            for (j = 0; j < i; ++j) {
                if (!strcmp(value,previous)) {
                    finding(rules,index,definition->lines[4],"rules.axis_duplicate_value",
                            "Repeated named-state declaration retained; not an additional independent state","allowed_values",value);
                    break;
                }
                previous += strlen(previous)+1;
            }
            value += strlen(value)+1;
        }
        return;
    }
    if (definition->allowed_count || record->axis_closed)
        axis_error(model,rules,index,definition->allowed_count ? definition->lines[4] : record->axis_closed_line,
                   "rules.axis_shape","Numeric axes use units and bounds, not named-state values or closed vocabularies");
    if (definition->unit && *definition->unit) {
        GD_Unit unit;
        axis->unit_known = gd_control_unit(definition->unit,strlen(definition->unit),&unit);
        if (!axis->unit_known) axis_error(model,rules,index,definition->lines[3],"rules.axis_unit","Axis unit does not match shared unit grammar");
    } else {
        rules->axis_definitions_complete = 0;
        finding(rules,index,definition->lines[3],"rules.axis_unit_unspecified",
                "Numeric axis unit remains unspecified; use 1 for explicit dimensionless quantities","unit",record->reference);
    }
    if (definition->minimum) {
        minimum_valid = gd_control_number(definition->minimum,&minimum,axis->type == GD_AXIS_INTEGER);
        if (!minimum_valid) axis_error(model,rules,index,definition->lines[5],"rules.axis_bound","Axis minimum requires a finite value of its declared numeric type");
    }
    if (definition->maximum) {
        maximum_valid = gd_control_number(definition->maximum,&maximum,axis->type == GD_AXIS_INTEGER);
        if (!maximum_valid) axis_error(model,rules,index,definition->lines[6],"rules.axis_bound","Axis maximum requires a finite value of its declared numeric type");
    }
    if (minimum_valid && maximum_valid && minimum > maximum)
        axis_error(model,rules,index,definition->lines[6],"rules.axis_bounds","Axis minimum exceeds maximum; no implicit correction");
}
void gd_resolve_rule_axes(GD_Model *model, GD_Rules *rules) {
    int i, j, source;
    assert(model && rules && model->record_count <= GD_MAX_RECORDS && model->edge_count <= GD_MAX_EDGES);
    rules->axis_definitions_complete = 0; rules->bindings_complete = 1;
    for (i = 0; i < model->collection_count; ++i) if (model->collections[i].role == GD_AXIS) rules->axis_definitions_complete = 1;
    for (i = 0; i < model->record_count; ++i) rules->actor_first[i] = -1;
    for (i = 0; i < model->record_count; ++i) {
        const GD_Record *record = &model->records[i];
        if (record->role == GD_AXIS) { ++rules->axis_count; validate_axis(model,rules,i); }
        else if (record->role == GD_NOUN) {
            ++rules->actor_count; rules->actor_axes_complete[i] = (unsigned char)record->axes_declared;
            if (!record->axes_declared) {
                rules->bindings_complete = 0;
                finding(rules,i,record->axes_line,"rules.actor_axes_unspecified",
                        "Actor axes remain unspecified; interaction mentions do not declare applicability","axes",record->reference);
            }
        }
    }
    /* Bucket only explicitly declared noun-to-axis links. Preserve authored
       order and duplicates; do not infer applicability from body state names. */
    for (i = model->edge_count-1; i >= 0; --i) if (model->edges[i].field == GD_AXES) {
        const GD_Edge *edge = &model->edges[i];
        source = edge->source;
        rules->next_axis[i] = rules->actor_first[source]; rules->actor_first[source] = i;
        rules->axis_resolution[i] = edge->target == -2 ? GD_RULE_AMBIGUOUS : edge->target < 0 ? unresolved(model,edge->reference) :
            model->records[edge->target].role == GD_AXIS ? GD_RULE_RESOLVED:GD_RULE_WRONG_ROLE;
        if (rules->axis_resolution[i] != GD_RULE_RESOLVED) {
            rules->actor_axes_complete[source] = 0; rules->bindings_complete = 0;
            finding(rules,source,edge->line,"rules.axis_reference",
                    "Declared actor axis is unresolved; inspect reference resolution","axes",edge->reference);
        }
    }
    for (i = 0; i < rules->case_count; ++i) {
        const GD_RuleCase *item = &rules->cases[i];
        source = rules->sources[item->source].record;
        for (j = 0; j < item->participant_count; ++j) {
            GD_RuleParticipant *participant = &rules->participants[item->first_participant+j];
            participant->noun_resolution = noun_binding(model,participant->noun,&participant->noun_record);
            if (participant->noun_resolution != GD_RULE_RESOLVED) {
                rules->bindings_complete = 0;
                finding(rules,source,participant->lines[0] ? participant->lines[0]:participant->line,"rules.noun_reference",
                        "Participant noun binding is unresolved; no binding guessed from its local label","noun",participant->noun);
            } else if (!rules->actor_axes_complete[participant->noun_record]) rules->bindings_complete = 0;
        }
    }
    if (model->error_count || model->errors_omitted) {
        rules->axis_definitions_complete = 0; rules->bindings_complete = 0;
    }
}
