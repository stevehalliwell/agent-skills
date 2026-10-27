/* Native domain checks. Build/run through test.ps1. Fixtures stay in ignored build/. */
#include "gd.h"
#include "gd_layout.h"
#include "gd_diagnostics.h"
#include <limits.h>
#include <math.h>
#include <time.h>
#define TEST_ENABLE 1
#include "vendor/proto-headers/test.h"

static unsigned char backing[64 * 1024 * 1024];
static free_list *storage;
static char root[GD_MAX_PATH];
static char folders[3][GD_MAX_PATH];
static GD_Collection collections[3];

static void *allocate(void *context, size_t size) {
    arena_result result = free_list_alloc(context, size);
    return result.error == arena_error_ok ? result.ptr : NULL;
}
static void release(void *context, void *memory) { free_list_free(context, memory); }
static GD_Model *model_create(void) {
    GD_Allocator allocator = { storage, allocate, release };
    return gd_create(allocator);
}
static int setup(const char *name) {
    const char *aliases[] = {"things", "actions", "cases"};
    char directory[GD_MAX_PATH];
    int i;
    if (snprintf(directory, sizeof(directory), "%s/%s", root, name) >= (int)sizeof(directory) ||
        !file_directory_create(directory, 1)) return 0;
    for (i = 0; i < 3; ++i) {
        if (snprintf(folders[i], sizeof(folders[i]), "%s/%s", directory, aliases[i]) >= (int)sizeof(folders[i]) ||
            !file_directory_create(folders[i], 1)) return 0;
        collections[i].alias = aliases[i]; collections[i].directory = folders[i]; collections[i].role = (GD_Role)i;
    }
    return 1;
}
static int write_record(int collection, const char *name, const char *text) {
    char path[GD_MAX_PATH];
    if (snprintf(path, sizeof(path), "%s/%s.md", folders[collection], name) >= (int)sizeof(path)) return 0;
    return file_write_bytes(cstring_from_raw(path), text, strlen(text)) == file_error_ok;
}
static int find_record(GD_Model *model, const char *reference) {
    int i;
    for (i = 0; i < model->record_count; ++i) if (!strcmp(model->records[i].reference, reference)) return i;
    return -1;
}
static int has_error(GD_Model *model, const char *code) {
    int i;
    for (i = 0; i < model->error_count; ++i) if (!strcmp(model->errors[i].code, code)) return 1;
    return 0;
}
TEST(rule_effect_bindings,
    GD_Model *model = model_create(); GD_Rules *rules; int life, form, time, amount;
    TEST_ASSERT(model && setup("rule-effect-bindings")); collections[1].role = GD_AXIS; collections[2].role = GD_RULE;
    TEST_ASSERT(write_record(1,"Life","---\nvalue_type: enum\nallowed_values: [alive, dead, dead]\nclosed: true\n---\nLife."));
    TEST_ASSERT(write_record(1,"Form","---\nvalue_type: enum\nallowed_values: [small, tall]\n---\nForm."));
    TEST_ASSERT(write_record(1,"Time","---\nvalue_type: number\nunit: s\n---\nTime."));
    TEST_ASSERT(write_record(1,"Amount","---\nvalue_type: integer\nunit: life\n---\nAmount."));
    TEST_ASSERT(write_record(0,"hero","---\naxes: [actions/Life, actions/Form, actions/Time, actions/Life]\n---\nHero."));
    TEST_ASSERT(write_record(0,"lives","---\naxes: [actions/Amount]\n---\nSeparate noun."));
    TEST_ASSERT(write_record(2,"contact","```gd-rule\ncase contact:\n    First:\n        noun: things/hero\n        becomes: small, Form.small, Life.dead\n        set:\n            Time: {controls/window}\n    Second:\n        noun: things/hero\n        preserve: ALL\n    Lives:\n        noun: things/lives\n        set:\n            Amount: this + 1[life]\n```\n"));
    TEST_ASSERT(gd_load(model,collections,3)); rules = gd_read_rules(model);
    TEST_ASSERT(rules && !model->error_count && rules->effect_bindings_complete && rules->state_effect_count == 3);
    life = find_record(model,"actions/Life"); form = find_record(model,"actions/Form");
    time = find_record(model,"actions/Time"); amount = find_record(model,"actions/Amount");
    TEST_ASSERT(rules->participants[0].state_effect_count == 3 && !rules->participants[1].state_effect_count);
    TEST_ASSERT(rules->state_effects[0].target.axis_record == form && rules->state_effects[1].target.axis_record == form);
    TEST_ASSERT(rules->state_effects[2].target.axis_record == life && !strcmp(rules->state_effects[2].target.state,"dead"));
    TEST_ASSERT(rules->state_effects[0].target.resolution == GD_RULE_RESOLVED && rules->state_effects[0].length == 5);
    TEST_ASSERT(!strncmp(rules->state_effects[0].text,"small",5) && !strcmp(rules->participants[0].becomes,"small, Form.small, Life.dead"));
    TEST_ASSERT(rules->assignments[0].target.resolution == GD_RULE_RESOLVED && rules->assignments[0].target.axis_record == time);
    TEST_ASSERT(rules->assignments[1].target.resolution == GD_RULE_RESOLVED && rules->assignments[1].target.axis_record == amount);
    TEST_ASSERT(!strcmp(rules->assignments[1].expression,"this + 1[life]"));
    TEST_ASSERT(rules->symbol_work > 0 && rules->symbol_work <= GD_RULE_SYMBOL_WORK_LIMIT && !gd_sources_changed(model));
    gd_rules_destroy(rules); gd_destroy(model);
)
TEST(rule_axis_definitions,
    GD_Model *model = model_create(); GD_Rules *rules; int life, amount, hero, isolated, empty, null_axes;
    TEST_ASSERT(model && setup("rule-axis-definitions")); collections[1].role = GD_AXIS; collections[2].role = GD_RULE;
    TEST_ASSERT(write_record(1,"Life","---\nvalue_type: enum\nallowed_values: [alive, dead, alive]\nclosed: false\n---\nKnown states, open set."));
    TEST_ASSERT(write_record(1,"Amount","---\nvalue_type: integer\nunit: life\nminimum: -0\nmaximum: 99\n---\nQuantity, not initial value."));
    TEST_ASSERT(write_record(1,"Time","---\nvalue_type: number\nunit: s\n---\nNo invented numeric bounds."));
    TEST_ASSERT(write_record(1,"Unknown","---\nvalue_type: unspecified\n---\nIncomplete definition allowed."));
    TEST_ASSERT(write_record(0,"hero","---\naxes: [actions/Life, actions/Time, actions/Life]\n---\nActor."));
    TEST_ASSERT(write_record(0,"lives","---\naxes: [actions/Amount]\n---\nPool stays a noun."));
    TEST_ASSERT(write_record(0,"isolated","Actor with unspecified axes."));
    TEST_ASSERT(write_record(0,"empty","---\naxes: []\n---\nNo axes currently declared."));
    TEST_ASSERT(write_record(0,"null","---\naxes: null\n---\nExplicitly unspecified."));
    TEST_ASSERT(write_record(2,"rule","```gd-rule\ncase contact:\n    A:\n        noun: things/hero\n    B:\n        noun: things/hero\n    Lives:\n        noun: things/lives\n```\n"));
    TEST_ASSERT(gd_load(model,collections,3)); rules = gd_read_rules(model); TEST_ASSERT(rules && !model->error_count);
    life = find_record(model,"actions/Life"); amount = find_record(model,"actions/Amount"); hero = find_record(model,"things/hero");
    isolated = find_record(model,"things/isolated"); empty = find_record(model,"things/empty"); null_axes = find_record(model,"things/null");
    TEST_ASSERT(rules->axis_count == 4 && rules->actor_count == 5 && !rules->axis_definitions_complete && !rules->bindings_complete);
    TEST_ASSERT(rules->axes[life].type == GD_AXIS_ENUM && rules->axes[life].valid && !model->records[life].axis_closed);
    TEST_ASSERT(model->records[life].axis.allowed_count == 3 && model->records[life].axis_closed_line == 4);
    TEST_ASSERT(rules->axes[amount].type == GD_AXIS_INTEGER && rules->axes[amount].unit_known && rules->axes[amount].valid);
    TEST_ASSERT(!strcmp(model->records[amount].axis.minimum,"-0") && !model->records[amount].axis_closed_line);
    TEST_ASSERT(rules->actor_axes_complete[hero] && !rules->actor_axes_complete[isolated] && rules->actor_axes_complete[empty] && !rules->actor_axes_complete[null_axes]);
    TEST_ASSERT(model->records[null_axes].axes_line == 2 && !model->records[null_axes].axes_declared);
    TEST_ASSERT(model->records[empty].axes_declared && rules->actor_first[empty] == -1);
    TEST_ASSERT(rules->participants[0].noun_resolution == GD_RULE_RESOLVED && rules->participants[0].noun_record == hero);
    TEST_ASSERT(rules->participants[1].noun_record == hero && strcmp(rules->participants[0].name,rules->participants[1].name));
    TEST_ASSERT(rules->next_axis[rules->next_axis[rules->actor_first[hero]]] >= 0); /* Duplicate declaration retained. */
    gd_rules_destroy(rules); gd_destroy(model);
)
TEST(rule_axis_errors,
    const char *definitions[] = {
        "value_type: boolean\n", "value_type: integer\nunit: life\nminimum: 1.5\n",
        "value_type: number\nunit: s\nminimum: 2\nmaximum: 1\n", "value_type: number\nunit: s+life\n",
        "value_type: enum\nunit: s\n", "value_type: integer\nunit: life\nclosed: true\n",
        "value_type: enum\nallowed_values: [alive, null]\n", "value_type: enum\nallowed_values: [\"\"]\n",
        "value_type: enum\nclosed: null\n", "value_type: enum\nclosed: false\nclosed: true\n",
        "value_type: integer\nunit: life\nminimum: 9007199254740992\n", "value_type: number\nminimum: \"0\"\n"
    };
    const char *codes[] = {"rules.axis_type","rules.axis_bound","rules.axis_bounds","rules.axis_unit","rules.axis_shape",
        "rules.axis_shape","rules.axis_value_type","rules.axis_value_empty","rules.axis_closed_type","input.duplicate_field","rules.axis_bound","rules.axis_field_type"};
    int i;
    for (i = 0; i < (int)(sizeof(definitions)/sizeof(definitions[0])); ++i) {
        GD_Model *model = model_create(); GD_Rules *rules; char name[64], text[1024];
        snprintf(name,sizeof(name),"rule-axis-error-%d",i);
        TEST_ASSERT(model && setup(name)); collections[1].role = GD_AXIS; collections[2].role = GD_RULE;
        snprintf(text,sizeof(text),"---\n%s---\nDefinition.",definitions[i]); TEST_ASSERT(write_record(1,"Bad",text));
        (void)gd_load(model,collections,3); rules = gd_read_rules(model);
        TEST_ASSERT(rules && has_error(model,codes[i]) && !rules->axis_definitions_complete && !rules->bindings_complete);
        TEST_ASSERT(model->errors[0].line > 0);
        gd_rules_destroy(rules); gd_destroy(model);
    }
)
TEST(rule_axis_references,
    GD_Model *model = model_create(); GD_Rules *rules; int hero, edge, count = 0;
    TEST_ASSERT(model && setup("rule-axis-references")); collections[1].role = GD_AXIS; collections[2].role = GD_RULE;
    TEST_ASSERT(write_record(1,"Life","---\nvalue_type: enum\nallowed_values: [alive, dead]\nclosed: true\n---\nStates."));
    TEST_ASSERT(write_record(0,"hero","---\naxes: [actions/Life, actions/absent, foreign/Life, things/hero]\n---\nActor."));
    TEST_ASSERT(write_record(2,"rule","```gd-rule\ncase refs:\n    A:\n        noun: things/hero\n    Missing:\n        noun: things/absent\n    Unloaded:\n        noun: foreign/actor\n    Wrong:\n        noun: actions/Life\n    Unknown:\n        requires: ANY\n```\n"));
    TEST_ASSERT(gd_load(model,collections,3)); rules = gd_read_rules(model); TEST_ASSERT(rules && !model->error_count);
    TEST_ASSERT(rules->axis_definitions_complete && !rules->bindings_complete);
    hero = find_record(model,"things/hero"); edge = rules->actor_first[hero];
    TEST_ASSERT(rules->axis_resolution[edge] == GD_RULE_RESOLVED); edge = rules->next_axis[edge];
    TEST_ASSERT(rules->axis_resolution[edge] == GD_RULE_MISSING); edge = rules->next_axis[edge];
    TEST_ASSERT(rules->axis_resolution[edge] == GD_RULE_UNLOADED); edge = rules->next_axis[edge];
    TEST_ASSERT(rules->axis_resolution[edge] == GD_RULE_WRONG_ROLE && rules->next_axis[edge] == -1);
    TEST_ASSERT(rules->participants[0].noun_resolution == GD_RULE_RESOLVED);
    TEST_ASSERT(rules->participants[1].noun_resolution == GD_RULE_MISSING);
    TEST_ASSERT(rules->participants[2].noun_resolution == GD_RULE_UNLOADED);
    TEST_ASSERT(rules->participants[3].noun_resolution == GD_RULE_WRONG_ROLE);
    TEST_ASSERT(rules->participants[4].noun_resolution == GD_RULE_UNSPECIFIED);
    for (edge = 0; edge < rules->finding_count; ++edge) if (!strcmp(rules->findings[edge].code,"rules.axis_reference")) ++count;
    TEST_ASSERT(count == 3);
    gd_rules_destroy(rules); gd_destroy(model);
)
TEST(rule_axis_incomplete,
    GD_Model *model = model_create(); GD_Rules *rules;
    TEST_ASSERT(model && setup("rule-axis-incomplete")); collections[1].role = GD_AXIS; collections[2].role = GD_RULE;
    TEST_ASSERT(write_record(1,"Time","---\nvalue_type: number\n---\nUnit not yet known."));
    TEST_ASSERT(write_record(1,"Open","---\nvalue_type: enum\nallowed_values: []\n---\nOpen, no invented states."));
    TEST_ASSERT(write_record(1,"ClosedEmpty","---\nvalue_type: enum\nallowed_values: []\nclosed: true\n---\nExplicit empty domain, not a parse failure."));
    TEST_ASSERT(write_record(0,"actor","---\naxes: [actions/Time]\n---\nActor."));
    TEST_ASSERT(gd_load(model,collections,3)); rules = gd_read_rules(model);
    TEST_ASSERT(rules && !model->error_count && !rules->axis_definitions_complete && rules->bindings_complete);
    TEST_ASSERT(rules->axes[find_record(model,"actions/Time")].valid && !rules->axes[find_record(model,"actions/Time")].unit_known);
    TEST_ASSERT(!model->records[find_record(model,"actions/Open")].axis_closed && model->records[find_record(model,"actions/ClosedEmpty")].axis_closed);
    gd_rules_destroy(rules); gd_destroy(model);
)
TEST(rule_axis_limits,
    {
        GD_Model *model = model_create(); GD_Rules *rules; size_t used; int i;
        TEST_ASSERT(model && setup("rule-axis-value-limit")); collections[1].role = GD_AXIS; collections[2].role = GD_RULE;
        used = (size_t)snprintf(model->input,sizeof(model->input),"---\nvalue_type: enum\nallowed_values: [");
        for (i = 0; i <= GD_MAX_ENUM_VALUES; ++i)
            used += (size_t)snprintf(model->input+used,sizeof(model->input)-used,"%sv%d",i ? ", ":"",i);
        strcpy(model->input+used,"]\n---\nValues."); TEST_ASSERT(write_record(1,"Many",model->input));
        TEST_ASSERT(!gd_load(model,collections,3)); rules = gd_read_rules(model);
        TEST_ASSERT(rules && has_error(model,"rules.axis_value_limit") && !rules->axis_definitions_complete);
        gd_rules_destroy(rules); gd_destroy(model);
    }
    {
        GD_Model *model = model_create(); GD_Rules *rules; int i;
        TEST_ASSERT(model && setup("rule-axis-finding-limit")); collections[1].role = GD_AXIS; collections[2].role = GD_RULE;
        for (i = 0; i <= GD_MAX_ERRORS; ++i) { char name[32]; snprintf(name,sizeof(name),"actor-%d",i); TEST_ASSERT(write_record(0,name,"Axes unspecified.")); }
        TEST_ASSERT(gd_load(model,collections,3)); rules = gd_read_rules(model);
        TEST_ASSERT(rules && !model->error_count && rules->finding_count == GD_MAX_ERRORS && rules->findings_omitted == 1 && !rules->bindings_complete);
        gd_rules_destroy(rules); gd_destroy(model);
    }
    {
        GD_Model *model = model_create(); GD_Rules *rules; int actor;
        TEST_ASSERT(model && setup("rule-axis-ambiguous")); collections[1].role = GD_AXIS; collections[2].role = GD_RULE;
        TEST_ASSERT(write_record(1,"a","---\nname: Life\nvalue_type: enum\n---\nAxis A."));
        TEST_ASSERT(write_record(1,"b","---\nname: Life\nvalue_type: enum\n---\nAxis B."));
        TEST_ASSERT(write_record(0,"a","---\nname: hero\naxes: [actions/Life]\n---\nActor A."));
        TEST_ASSERT(write_record(0,"b","---\nname: hero\naxes: [actions/Life]\n---\nActor B."));
        TEST_ASSERT(write_record(2,"refs","```gd-rule\ncase refs:\n    Ambiguous:\n        noun: things/hero\n    Missing:\n        noun: things/absent\n```\n"));
        TEST_ASSERT(!gd_load(model,collections,3)); rules = gd_read_rules(model);
        TEST_ASSERT(rules && has_error(model,"input.identity") && rules->participants[0].noun_resolution == GD_RULE_AMBIGUOUS);
        TEST_ASSERT(rules->participants[1].noun_resolution == GD_RULE_UNAVAILABLE);
        actor = find_record(model,"things/hero"); TEST_ASSERT(rules->axis_resolution[rules->actor_first[actor]] == GD_RULE_AMBIGUOUS);
        TEST_ASSERT(!rules->axis_definitions_complete && !rules->bindings_complete);
        gd_rules_destroy(rules); gd_destroy(model);
    }
)
TEST(rules_declarations,
    GD_Model *model = model_create();
    GD_Rules *rules;
    TEST_ASSERT(model && setup("rules-declarations"));
    collections[2].role = GD_RULE;
    TEST_ASSERT(write_record(0,"hero","Hero."));
    TEST_ASSERT(write_record(2,"contact",
        "---\nname: contact\nstatus: accepted\ncoverage: defined\n---\n# Intent\n\n```gd-rule\n"
        "case powered-side-contact:\n    context: Player sideContacts Walker\n\n"
        "    Player:\n        noun: things/hero\n        requires: alive AND (tall OR spark) AND NOT protected\n"
        "        other_axes: ANY\n        becomes: small, protected\n        set:\n"
        "            ProtectionTime: {control-vars/safety-window}\n            Amount: this + 1[life]\n"
        "            Amount: this + 2[life]\n        preserve: OTHER\n"
        "    Walker:\n        noun: things/hero\n        requires: alive\n"
        "    Lives:\n        requires: ANY\n        preserve: ALL\n"
        "case unknown-result:\n    Player:\n        skip_if: dead OR (hidden AND NOT revealed)\n"
        "        causes: mechanics/power-loss\n```\n"));
    TEST_ASSERT(write_record(2,"legacy","Legacy prose remains unknown."));
    TEST_ASSERT(write_record(2,"empty","```gd-rule\n# Design not filled in.\n```\n"));
    TEST_ASSERT(gd_load(model,collections,3));
    TEST_ASSERT(model->rule_mode && !model->cell_count && !model->component_count);
    rules = gd_read_rules(model);
    TEST_ASSERT(rules && !model->error_count);
    TEST_ASSERT(rules->source_count == 3 && rules->case_count == 2 && rules->participant_count == 4 && rules->assignment_count == 3);
    TEST_ASSERT(rules->sources[0].state == GD_RULE_PARSED && rules->sources[0].line == 8);
    TEST_ASSERT(rules->sources[1].state == GD_RULE_EMPTY && rules->sources[2].state == GD_RULE_ABSENT);
    TEST_ASSERT(rules->cases[0].line == 9 && rules->cases[0].context_line == 10 && rules->participants[0].line == 12);
    TEST_ASSERT(!strcmp(rules->cases[0].context,"Player sideContacts Walker"));
    TEST_ASSERT(!strcmp(rules->participants[0].requires,"alive AND (tall OR spark) AND NOT protected"));
    TEST_ASSERT(!strcmp(rules->assignments[0].expression,"{control-vars/safety-window}"));
    TEST_ASSERT(!strcmp(rules->assignments[1].expression,"this + 1[life]") && !strcmp(rules->assignments[2].expression,"this + 2[life]"));
    TEST_ASSERT(!rules->participants[1].preserve && !rules->participants[1].becomes && !rules->participants[1].other_axes);
    TEST_ASSERT(!rules->participants[2].noun && !strcmp(rules->participants[2].requires,"ANY"));
    TEST_ASSERT(!rules->cases[1].context && !rules->participants[3].requires && !rules->participants[3].noun);
    TEST_ASSERT(!strcmp(rules->participants[3].causes,"mechanics/power-loss"));
    gd_rules_destroy(rules); gd_destroy(model);
)
TEST(rules_fence_scope,
    GD_Model *model = model_create();
    GD_Rules *rules;
    TEST_ASSERT(model && setup("rules-fence-scope")); collections[2].role = GD_RULE;
    TEST_ASSERT(write_record(2,"quoted",
        "````markdown\r\n```gd-rule\r\ncase hidden:\r\n```\r\n````\r\n"
        "   ~~~ gd-rule\r\ncase real:\r\n    context: ANY\r\n    Actor:\r\n"
        "        noun: things/actor\r\n        becomes: unknown\r\n   ~~~~\r\n"));
    TEST_ASSERT(write_record(2,"z-duplicate","```gd-rule\ncase real:\n```\n"));
    TEST_ASSERT(gd_load(model,collections,3)); rules = gd_read_rules(model);
    TEST_ASSERT(rules && !model->error_count && rules->case_count == 2);
    TEST_ASSERT(!strcmp(rules->cases[0].name,"real") && rules->cases[0].line == 7);
    TEST_ASSERT(!strcmp(rules->cases[1].name,"real") && rules->cases[0].source != rules->cases[1].source);
    TEST_ASSERT(!strcmp(rules->cases[0].context,"ANY") && rules->participants[0].lines[4] == 11);
    gd_rules_destroy(rules); gd_destroy(model);
)
TEST(rules_invalid_records,
    const char *inputs[] = {
        "```gd-rule\ncase bad:\n    A:\n        noun: things/a\n        noun: things/b\n```\n",
        "```gd-rule\ncase bad:\n    A:\n    A:\n```\n",
        "```gd-rule\ncase bad:\ncase bad:\n```\n",
        "```gd-rule\ncase bad:\n    A:\n        mystery: wrong\n```\n",
        "```gd-rule\ncase bad:\n    A:\n        requires:\n```\n",
        "```gd-rule\ncase bad:\n    A:\n          noun: things/a\n```\n",
        "```gd-rule\ncase bad:\n\tA:\n```\n",
        "```gd-rule\ncase bad:\n    A:\n        set: Amount: 1\n```\n",
        "```gd-rule\ncase bad:\n    A:\n        set:\n            Amount:\n```\n",
        "```gd-rule\ncase bad:\n    context: ANY\n    context: ANY\n```\n",
        "```gd-rule\ncase bad:\n    A:\n        noun: things/a\n",
        "```gd-rule\ncase bad:\n```\n```gd-rule\ncase another:\n```\n",
        "```gd-rule\n    A:\n```\n"
    };
    const char *codes[] = {"rules.duplicate_field","rules.duplicate_participant","rules.duplicate_case","rules.field",
        "rules.value","rules.indent","rules.indent","rules.assignment","rules.assignment","rules.duplicate_field",
        "rules.unclosed_block","rules.multiple_blocks","rules.case"};
    int i;
    for (i = 0; i < (int)(sizeof(inputs)/sizeof(inputs[0])); ++i) {
        GD_Model *model = model_create(); GD_Rules *rules; char name[64];
        snprintf(name,sizeof(name),"rules-invalid-%d",i);
        TEST_ASSERT(model && setup(name)); collections[2].role = GD_RULE;
        TEST_ASSERT(write_record(2,"a-good","```gd-rule\ncase good:\n    Actor:\n        becomes: alive\n```\n"));
        TEST_ASSERT(write_record(2,"bad",inputs[i]));
        TEST_ASSERT(write_record(2,"z-good","```gd-rule\ncase later:\n    Actor:\n        preserve: OTHER\n```\n"));
        TEST_ASSERT(gd_load(model,collections,3)); rules = gd_read_rules(model);
        TEST_ASSERT(rules && has_error(model,codes[i]));
        TEST_ASSERT(rules->case_count == 2 && rules->participant_count == 2 && !rules->assignment_count);
        TEST_ASSERT(rules->sources[1].state == GD_RULE_INVALID && !rules->sources[1].case_count);
        TEST_ASSERT(!strcmp(rules->cases[0].name,"good") && !strcmp(rules->cases[1].name,"later"));
        TEST_ASSERT(!strcmp(rules->participants[0].becomes,"alive") && !strcmp(rules->participants[1].preserve,"OTHER"));
        TEST_ASSERT(model->errors[0].line > 0);
        gd_rules_destroy(rules); gd_destroy(model);
    }
)
TEST(rules_limits,
    int mode;
    const char *codes[] = {"rules.case_limit","rules.participant_limit","rules.assignment_limit","rules.line_limit"};
    for (mode = 0; mode < 4; ++mode) {
        GD_Model *model = model_create(); GD_Rules *rules; char name[64];
        size_t used; int i, count = mode == 0 ? GD_MAX_RULE_CASES : mode == 1 ? GD_MAX_RULE_PARTICIPANTS : GD_MAX_RULE_ASSIGNMENTS;
        snprintf(name,sizeof(name),"rules-limit-%d",mode);
        TEST_ASSERT(model && setup(name)); collections[2].role = GD_RULE;
        used = (size_t)snprintf(model->input,sizeof(model->input),"```gd-rule\n");
        if (mode) used += (size_t)snprintf(model->input+used,sizeof(model->input)-used,"case big:\n");
        if (mode == 2) used += (size_t)snprintf(model->input+used,sizeof(model->input)-used,"    Actor:\n        set:\n");
        if (mode == 3) {
            memset(model->input+used,'x',GD_MAX_RULE_LINE+1); used += GD_MAX_RULE_LINE+1;
            model->input[used++] = '\n';
        } else for (i = 0; i <= count; ++i)
            used += (size_t)snprintf(model->input+used,sizeof(model->input)-used,
                mode == 0 ? "case c%d:\n" : mode == 1 ? "    A%d:\n" : "            Amount: this + %d[life]\n",i);
        TEST_ASSERT(used+5 < sizeof(model->input));
        strcpy(model->input+used,"```\n");
        TEST_ASSERT(write_record(2,"limit",model->input));
        TEST_ASSERT(gd_load(model,collections,3)); rules = gd_read_rules(model);
        TEST_ASSERT(rules && has_error(model,codes[mode]));
        TEST_ASSERT(!rules->case_count && !rules->participant_count && !rules->assignment_count && !rules->text_used);
        gd_rules_destroy(rules); gd_destroy(model);
    }
    {
        GD_Model *model = model_create(); GD_Rules *rules; size_t used; int i, j;
        TEST_ASSERT(model && setup("rules-text-limit")); collections[2].role = GD_RULE;
        for (i = 0; i < 6; ++i) {
            char name[32]; snprintf(name,sizeof(name),"text-%d",i);
            used = (size_t)snprintf(model->input,sizeof(model->input),"```gd-rule\ncase text:\n    Actor:\n        set:\n");
            for (j = 0; j < 120; ++j) {
                used += (size_t)snprintf(model->input+used,sizeof(model->input)-used,"            Amount: ");
                TEST_ASSERT(used+7501+5 < sizeof(model->input));
                memset(model->input+used,'x',7500); used += 7500; model->input[used++] = '\n';
            }
            strcpy(model->input+used,"```\n"); TEST_ASSERT(write_record(2,name,model->input));
        }
        TEST_ASSERT(gd_load(model,collections,3)); rules = gd_read_rules(model);
        TEST_ASSERT(rules && has_error(model,"rules.text_limit") && model->error_count == 2);
        TEST_ASSERT(rules->source_count == 6 && rules->case_count == 4 && rules->assignment_count == 480);
        TEST_ASSERT(rules->sources[4].state == GD_RULE_INVALID && rules->sources[5].state == GD_RULE_INVALID);
        TEST_ASSERT(rules->text_used < GD_RULE_TEXT_CAPACITY && strlen(rules->assignments[479].expression) == 7500);
        gd_rules_destroy(rules); gd_destroy(model);
    }
)
TEST(empty,
    GD_Model *model = model_create();
    TEST_ASSERT(model && setup("empty"));
    TEST_ASSERT(gd_load(model, collections, 3));
    TEST_ASSERT(model->record_count == 0 && model->cell_count == 0);
    TEST_ASSERT(!gd_sources_changed(model));
    gd_destroy(model);
)
TEST(matrix,
    GD_Model *model = model_create();
    int box, wall, push;
    TEST_ASSERT(model && setup("matrix"));
    TEST_ASSERT(write_record(0, "box", "---\nname: box\nstatus: accepted\n---\nBox."));
    TEST_ASSERT(write_record(0, "wall", "Wall."));
    TEST_ASSERT(write_record(1, "push", "Push."));
    TEST_ASSERT(write_record(1, "pull", "---\nstatus: rejected\n---\nPull."));
    TEST_ASSERT(write_record(2, "first", "---\nstatus: accepted\ncoverage: defined\nnouns: [things/box, things/box]\nverb: actions/push\ncontext_nouns: [things/wall]\n---\nFirst condition."));
    TEST_ASSERT(write_record(2, "second", "---\nstatus: accepted\ncoverage: defined\nnouns: [things/box]\nverb: actions/push\n---\nOther condition."));
    TEST_ASSERT(write_record(2, "excluded", "---\nstatus: accepted\ncoverage: excluded\nnouns: [things/wall]\nverb: actions/push\n---\nNot available."));
    TEST_ASSERT(write_record(2, "proposal", "---\nstatus: proposed\ncoverage: defined\nnouns: [things/box, things/wall]\nverb: actions/pull\n---\nAlternative."));
    TEST_ASSERT(gd_load(model, collections, 3));
    TEST_ASSERT(model->cell_count == 4 && model->pair_count == 5 && !model->unplaced_cases);
    box = find_record(model, "things/box"); wall = find_record(model, "things/wall"); push = find_record(model, "actions/push");
    TEST_ASSERT(box >= 0 && wall >= 0 && push >= 0);
    TEST_ASSERT(model->connectivity[box].partners == 1 && model->connectivity[box].cases == 3);
    TEST_ASSERT(model->connectivity[wall].partners == 0 && model->connectivity[wall].cases == 2);
    TEST_ASSERT(model->connectivity[push].partners == 1 && model->connectivity[push].active_cases == 2);
    TEST_ASSERT(model->component_count == 3);
    TEST_ASSERT(model->component_of[box] == model->component_of[push]);
    TEST_ASSERT(model->component_of[wall] != model->component_of[box]);
    TEST_ASSERT(model->components[model->component_of[box]].edge_count == 1);
    TEST_ASSERT(!gd_sources_changed(model));
    TEST_ASSERT(write_record(0, "new", "New node."));
    TEST_ASSERT(gd_sources_changed(model));
    {
        GD_Model *fresh = model_create();
        TEST_ASSERT(fresh && gd_load(fresh, collections, 3));
        TEST_ASSERT(fresh->record_count == model->record_count + 1);
        gd_destroy(fresh);
        TEST_ASSERT(write_record(0, "new", "---\nname: [unsafe]\n---\n"));
        fresh = model_create();
        TEST_ASSERT(fresh && !gd_load(fresh, collections, 3));
        TEST_ASSERT(model->cell_count == 4 && model->connectivity[box].partners == 1);
        TEST_ASSERT(!strcmp(model->records[box].body, "Box."));
        gd_destroy(fresh);
    }
    gd_destroy(model);
)
TEST(unsafe_input,
    GD_Model *model = model_create();
    TEST_ASSERT(model && setup("unsafe"));
    TEST_ASSERT(write_record(0, "duplicate", "---\nname: box\nname: other\n---\n"));
    TEST_ASSERT(write_record(1, "invalid", "---\nname: [nested]\n---\n"));
    TEST_ASSERT(write_record(2, "broken", "---\nnouns: [broken\n---\n"));
    TEST_ASSERT(write_record(0, "escaped-nul", "---\nname: \"box\\0hidden\"\n---\n"));
    TEST_ASSERT(write_record(0, "merge", "---\n<<: {name: box}\n---\n"));
    TEST_ASSERT(!gd_load(model, collections, 3));
    TEST_ASSERT(has_error(model, "input.duplicate_field") && has_error(model, "input.field_type") && has_error(model, "input.yaml"));
    TEST_ASSERT(has_error(model, "input.field_nul") && has_error(model, "input.yaml_merge"));
    TEST_ASSERT(model->record_count == 0 && model->edge_count == 0);
    gd_destroy(model);
)
TEST(ambiguous_endpoints,
    GD_Model *model = model_create();
    TEST_ASSERT(model && setup("ambiguous"));
    TEST_ASSERT(write_record(0, "one", "---\nname: box\n---\n"));
    TEST_ASSERT(write_record(0, "two", "---\nname: box\n---\n"));
    TEST_ASSERT(write_record(1, "push", "Push."));
    TEST_ASSERT(write_record(2, "case", "---\nnouns: [things/box]\nverb: actions/push\n---\n"));
    TEST_ASSERT(!gd_load(model, collections, 3));
    TEST_ASSERT(has_error(model, "input.identity") && has_error(model, "matrix.endpoint"));
    TEST_ASSERT(model->cell_count == 0 && model->unplaced_cases == 1);
    gd_destroy(model);
)
TEST(text_types,
    GD_Model *model = model_create();
    int record;
    TEST_ASSERT(model && setup("types"));
    TEST_ASSERT(write_record(0, "fallback", "---\nname: !!str null\nid: null\nrank: -1\nmetadata: {ignored: [one, two]}\n---\nMeaning preserved."));
    TEST_ASSERT(write_record(1, "numeric", "---\nname: 123\n---\n"));
    TEST_ASSERT(!gd_load(model, collections, 3));
    TEST_ASSERT(has_error(model, "input.field_type"));
    record = find_record(model, "things/null");
    TEST_ASSERT(record >= 0 && !model->records[record].id);
    TEST_ASSERT(strstr(model->records[record].body, "Meaning preserved."));
    gd_destroy(model);
)
TEST(missing_endpoint,
    GD_Model *model = model_create();
    TEST_ASSERT(model && setup("missing"));
    TEST_ASSERT(write_record(0, "box", "Box."));
    TEST_ASSERT(write_record(1, "push", "Push."));
    TEST_ASSERT(write_record(2, "case", "---\nnouns: [things/missing]\nverb: things/box\n---\n"));
    TEST_ASSERT(!gd_load(model, collections, 3));
    TEST_ASSERT(has_error(model, "matrix.endpoint"));
    TEST_ASSERT(!model->cell_count && model->unplaced_cases == 1);
    gd_destroy(model);
)
TEST(input_limits,
    GD_Model *model = model_create();
    char path[GD_MAX_PATH];
    TEST_ASSERT(model && setup("limits"));
    memset(model->input, 'x', sizeof(model->input));
    snprintf(path, sizeof(path), "%s/oversized.md", folders[0]);
    TEST_ASSERT(file_write_bytes(cstring_from_raw(path), model->input, sizeof(model->input)) == file_error_ok);
    snprintf(path, sizeof(path), "%s/binary.md", folders[0]);
    TEST_ASSERT(file_write_bytes(cstring_from_raw(path), "x\0y", 3) == file_error_ok);
    memcpy(model->input, "---\nignored: ", 13);
    snprintf(path, sizeof(path), "%s/frontmatter.md", folders[1]);
    TEST_ASSERT(file_write_bytes(cstring_from_raw(path), model->input, GD_MAX_FRONTMATTER + 32) == file_error_ok);
    TEST_ASSERT(!gd_load(model, collections, 3));
    TEST_ASSERT(has_error(model, "input.read_limit") && has_error(model, "input.utf8") && has_error(model, "input.frontmatter"));
    gd_destroy(model);
)
TEST(component_chain,
    GD_Model *model = model_create(), *reordered = model_create();
    GD_Collection reversed[3];
    int a, b, c, x, y, i;
    TEST_ASSERT(model && reordered && setup("chain"));
    TEST_ASSERT(write_record(0, "a", "A.")); TEST_ASSERT(write_record(0, "b", "B.")); TEST_ASSERT(write_record(0, "c", "C."));
    TEST_ASSERT(write_record(1, "x", "X.")); TEST_ASSERT(write_record(1, "y", "Y."));
    TEST_ASSERT(write_record(2, "ax", "---\nstatus: accepted\ncoverage: defined\nnouns: [things/a]\nverb: actions/x\n---\n"));
    TEST_ASSERT(write_record(2, "bx", "---\nstatus: accepted\ncoverage: defined\nnouns: [things/b]\nverb: actions/x\n---\n"));
    TEST_ASSERT(write_record(2, "by", "---\nstatus: accepted\ncoverage: defined\nnouns: [things/b]\nverb: actions/y\n---\n"));
    TEST_ASSERT(gd_load(model, collections, 3));
    a = find_record(model, "things/a"); b = find_record(model, "things/b"); c = find_record(model, "things/c");
    x = find_record(model, "actions/x"); y = find_record(model, "actions/y");
    TEST_ASSERT(model->component_count == 2 && model->component_of[a] == model->component_of[b]);
    TEST_ASSERT(model->component_of[a] == model->component_of[x] && model->component_of[x] == model->component_of[y]);
    TEST_ASSERT(model->component_of[c] != model->component_of[a]);
    TEST_ASSERT(model->components[model->component_of[a]].edge_count == 3);
    reversed[0] = collections[2]; reversed[1] = collections[1]; reversed[2] = collections[0];
    TEST_ASSERT(gd_load(reordered, reversed, 3));
    TEST_ASSERT(reordered->component_count == model->component_count);
    for (i = 0; i < model->record_count; ++i) TEST_ASSERT(model->component_of[i] == reordered->component_of[i]);
    gd_destroy(reordered); gd_destroy(model);
)
static GD_Layout test_layout;
TEST(layout_modes,
    GD_Layout *layout = &test_layout;
    float first_x, first_y, other_x;
    int i;
    memset(layout, 0, sizeof(*layout)); layout->node_count = 3; layout->edge_count = 2; layout->group_count = 1;
    layout->nodes[0] = (GD_LayoutNode){.key=0, .label="zeta", .degree=1};
    layout->nodes[1] = (GD_LayoutNode){.key=1, .label="alpha", .side=1, .degree=2};
    layout->nodes[2] = (GD_LayoutNode){.key=2, .label="beta", .degree=3};
    layout->edges[0] = (GD_LayoutEdge){0,1,0,1}; layout->edges[1] = (GD_LayoutEdge){1,2,1,1};
    TEST_ASSERT(gd_layout_arrange(layout, GD_LAYOUT_COLUMNS, GD_SORT_NAME));
    TEST_ASSERT(layout->order[0] == 1 && layout->order[1] == 2 && layout->order[2] == 0);
    first_x = layout->nodes[0].x; first_y = layout->nodes[0].y;
    gd_layout_step(layout, -1);
    TEST_ASSERT(layout->nodes[0].x == first_x && layout->nodes[0].y == first_y && layout->tick == 0);
    TEST_ASSERT(gd_layout_arrange(layout, GD_LAYOUT_COLUMNS, GD_SORT_PARTNERS));
    TEST_ASSERT(layout->order[0] == 2 && layout->order[1] == 1 && layout->order[2] == 0);
    TEST_ASSERT(gd_layout_arrange(layout, GD_LAYOUT_FORCE, GD_SORT_NAME));
    layout->nodes[0].x = 100; layout->nodes[0].y = 200; other_x = layout->nodes[1].x;
    gd_layout_step(layout, 0);
    TEST_ASSERT(layout->nodes[0].x == 100 && layout->nodes[0].y == 200 && layout->nodes[1].x != other_x);
    TEST_ASSERT(gd_layout_arrange(layout, GD_LAYOUT_FORCE, GD_SORT_NAME));
    for (i = 0; i < 120; ++i) gd_layout_step(layout, -1);
    first_x = layout->nodes[0].x; first_y = layout->nodes[0].y;
    TEST_ASSERT(isfinite(first_x) && isfinite(first_y));
    TEST_ASSERT(gd_layout_arrange(layout, GD_LAYOUT_FORCE, GD_SORT_NAME));
    for (i = 0; i < 120; ++i) gd_layout_step(layout, -1);
    TEST_ASSERT(layout->nodes[0].x == first_x && layout->nodes[0].y == first_y);
    layout->edges[0].a = 99;
    TEST_ASSERT(!gd_layout_arrange(layout, GD_LAYOUT_FORCE, GD_SORT_NAME));
)
TEST(layout_capacity,
    GD_Layout *layout = &test_layout;
    clock_t start;
    int i;
    memset(layout, 0, sizeof(*layout)); layout->node_count = GD_MAX_RECORDS; layout->edge_count = GD_MAX_PAIRS; layout->group_count = 1;
    for (i = 0; i < layout->node_count; ++i) { layout->nodes[i].label = "node"; layout->nodes[i].key = i; layout->nodes[i].side = i%2; }
    for (i = 0; i < layout->edge_count; ++i) layout->edges[i] = (GD_LayoutEdge){i%GD_MAX_RECORDS, (i+1)%GD_MAX_RECORDS, i, 1};
    TEST_ASSERT(gd_layout_arrange(layout, GD_LAYOUT_FORCE, GD_SORT_NAME));
    start = clock();
    for (i = 0; i < 10; ++i) gd_layout_step(layout, -1);
    printf("layout capacity probe: 4096 nodes / 32768 edges / 10 steps: %.1f ms\n", 1000.0*(clock()-start)/CLOCKS_PER_SEC);
    for (i = 0; i < layout->node_count; ++i) TEST_ASSERT(isfinite(layout->nodes[i].x) && isfinite(layout->nodes[i].y));
    layout->node_count = GD_MAX_RECORDS+1;
    TEST_ASSERT(!gd_layout_arrange(layout, GD_LAYOUT_FORCE, GD_SORT_NAME));
)
static int setup_skills(const char *name) {
    if (!setup(name)) return 0;
    collections[0].role = GD_SKILL; collections[0].alias = "atoms";
    return 1;
}
static int has_finding(const GD_Model *model, const char *code) {
    int i;
    for (i = 0; i < model->finding_count; ++i) if (!strcmp(model->findings[i].code, code)) return 1;
    return 0;
}
TEST(skill_chain,
    GD_Model *model = model_create();
    GD_Layout *layout = &test_layout;
    int root_node, zero, branch, merge, join, late, unranked, isolated, rank_mode, horizontal, i, step;
    TEST_ASSERT(model && setup_skills("skill-chain"));
    TEST_ASSERT(write_record(0, "root", "---\nstatus: rejected\nrank: -4\nprerequisites: []\n---\nRoot body stays unchanged."));
    TEST_ASSERT(write_record(0, "zero", "---\nrank: 0\nprerequisites: [atoms/root]\n---\n"));
    TEST_ASSERT(write_record(0, "branch", "---\nrank: null\nprerequisites: [atoms/root]\n---\n"));
    TEST_ASSERT(write_record(0, "merge", "---\nrank: -1\nprerequisites: [atoms/zero, atoms/branch, atoms/zero]\n---\n"));
    TEST_ASSERT(write_record(0, "late", "---\nrank: 3\n---\n"));
    TEST_ASSERT(write_record(0, "join", "---\nrank: 2\nprerequisites: [atoms/merge, atoms/late]\n---\n"));
    TEST_ASSERT(write_record(0, "unranked", "---\nrank: null\n---\n"));
    TEST_ASSERT(write_record(0, "unranked-child", "---\nprerequisites: [atoms/unranked]\n---\n"));
    TEST_ASSERT(write_record(0, "isolated", "---\nstatus: proposed\nrank: 9007199254740991\n---\n"));
    TEST_ASSERT(gd_load(model, collections, 1));
    root_node = find_record(model, "atoms/root"); zero = find_record(model, "atoms/zero"); branch = find_record(model, "atoms/branch");
    merge = find_record(model, "atoms/merge"); join = find_record(model, "atoms/join"); late = find_record(model, "atoms/late");
    unranked = find_record(model, "atoms/unranked"); isolated = find_record(model, "atoms/isolated");
    TEST_ASSERT(model->record_count == 9 && model->component_count == 3 && model->skill_link_count == 7);
    TEST_ASSERT(model->skills[root_node].depth == 0 && !model->skills[root_node].has_minimum && model->skills[root_node].display_rank == -4);
    TEST_ASSERT(model->records[zero].has_rank && model->records[zero].rank == 0 && model->skills[zero].minimum_rank == -4);
    TEST_ASSERT(!model->records[branch].has_rank && model->skills[branch].has_display && model->skills[branch].display_rank == -4);
    TEST_ASSERT(model->skills[merge].depth == 2 && model->skills[merge].prerequisite_count == 2 && model->skills[merge].declared_prerequisites == 3);
    TEST_ASSERT(model->skills[merge].rank_conflict && model->skills[merge].display_rank == 0 && model->records[merge].rank == -1);
    TEST_ASSERT(model->skills[join].depth == 3 && model->skills[join].display_rank == 3 && model->skills[join].rank_conflict && model->records[join].rank == 2);
    TEST_ASSERT(model->skills[late].depth == 0 && model->skills[late].display_rank == 3);
    TEST_ASSERT(!model->skills[unranked].has_display && !model->skills[unranked].has_minimum && !model->skills[unranked].incomplete);
    TEST_ASSERT(model->skills[isolated].display_rank == INT64_C(9007199254740991));
    TEST_ASSERT(!model->skill_incomplete && model->finding_count == 2 && has_finding(model, "skill.rank_conflict"));
    for (rank_mode = 0; rank_mode < 2; ++rank_mode) for (horizontal = 0; horizontal < 2; ++horizontal) {
        TEST_ASSERT(gd_skill_layout(model, layout, rank_mode, horizontal, GD_SORT_NAME));
        TEST_ASSERT(layout->node_count == 9 && layout->edge_count == 7);
        if (rank_mode) TEST_ASSERT(horizontal ? layout->nodes[root_node].y != layout->nodes[late].y : layout->nodes[root_node].x != layout->nodes[late].x);
        for (step = 0; step < 120; ++step) {
            /* Pointer drag attempts to move a prerequisite past descendants,
               including paused input (before stepping) and held-node settling. */
            gd_layout_move_node(layout, step%layout->node_count, 100000, 100000);
            for (i = 0; i < layout->edge_count; ++i) {
                const GD_LayoutNode *a = &layout->nodes[layout->edges[i].a], *b = &layout->nodes[layout->edges[i].b];
                TEST_ASSERT(horizontal ? a->x < b->x : a->y < b->y);
            }
            gd_layout_step(layout, step%layout->node_count);
            for (i = 0; i < layout->edge_count; ++i) {
                const GD_LayoutNode *a = &layout->nodes[layout->edges[i].a], *b = &layout->nodes[layout->edges[i].b];
                TEST_ASSERT(horizontal ? a->x < b->x : a->y < b->y);
            }
        }
        TEST_ASSERT(gd_skill_layout(model, layout, rank_mode, horizontal, GD_SORT_PARTNERS));
    }
    TEST_ASSERT(!gd_sources_changed(model) && !strcmp(model->records[root_node].body, "Root body stays unchanged."));
    TEST_ASSERT(!strcmp(model->records[root_node].status, "rejected") && !model->records[branch].has_rank);
    {
        GD_Model *fresh = model_create();
        TEST_ASSERT(fresh && write_record(0, "zero", "---\nrank: 4\nprerequisites: [atoms/root]\n---\n"));
        TEST_ASSERT(gd_sources_changed(model) && gd_load(fresh, collections, 1));
        TEST_ASSERT(fresh->skills[merge].display_rank == 4 && fresh->records[merge].rank == -1 && fresh->records[join].rank == 2);
        TEST_ASSERT(fresh->skills[join].display_rank == 4 && fresh->skills[late].display_rank == 3 && !fresh->records[branch].has_rank);
        TEST_ASSERT(model->records[zero].rank == 0 && model->skills[join].display_rank == 3);
        gd_destroy(fresh);
        fresh = model_create();
        TEST_ASSERT(fresh && write_record(0, "zero", "---\nrank: 0\nprerequisites: [atoms/root]\n---\n"));
        TEST_ASSERT(gd_load(fresh, collections, 1) && fresh->skills[join].display_rank == 3 && fresh->records[join].rank == 2);
        gd_destroy(fresh);
    }
    gd_destroy(model);
)
TEST(skill_incomplete,
    GD_Model *model = model_create();
    int a, b, tail, self, missing, child, independent, wrong, i, rank_mode, horizontal;
    TEST_ASSERT(model && setup_skills("skill-incomplete"));
    collections[1].role = GD_RELATED; collections[1].alias = "context";
    TEST_ASSERT(write_record(1, "not-a-skill", "Context only."));
    TEST_ASSERT(write_record(0, "a", "---\nrank: 2\nprerequisites: [atoms/b, atoms/independent]\n---\n"));
    TEST_ASSERT(write_record(0, "b", "---\nprerequisites: [atoms/a]\n---\n"));
    TEST_ASSERT(write_record(0, "tail", "---\nrank: 9\nprerequisites: [atoms/a]\n---\n"));
    TEST_ASSERT(write_record(0, "self", "---\nprerequisites: [atoms/self]\n---\n"));
    TEST_ASSERT(write_record(0, "missing", "---\nprerequisites: [atoms/absent]\n---\n"));
    TEST_ASSERT(write_record(0, "child", "---\nprerequisites: [atoms/missing]\n---\n"));
    TEST_ASSERT(write_record(0, "independent", "---\nrank: 0\nmechanics: [context/not-loaded]\n---\n"));
    TEST_ASSERT(write_record(0, "wrong", "---\nprerequisites: [context/not-a-skill]\n---\n"));
    TEST_ASSERT(gd_load(model, collections, 2));
    a = find_record(model, "atoms/a"); b = find_record(model, "atoms/b"); tail = find_record(model, "atoms/tail");
    self = find_record(model, "atoms/self"); missing = find_record(model, "atoms/missing"); child = find_record(model, "atoms/child");
    independent = find_record(model, "atoms/independent"); wrong = find_record(model, "atoms/wrong");
    TEST_ASSERT(model->skills[a].cycle && model->skills[b].cycle && model->skills[self].cycle);
    TEST_ASSERT(!model->skills[tail].cycle && model->skills[tail].incomplete && model->skills[tail].depth == -1);
    TEST_ASSERT(model->skills[missing].incomplete && model->skills[child].incomplete && model->skills[wrong].incomplete);
    TEST_ASSERT(model->skills[missing].declared_prerequisites == 1 && model->skills[missing].prerequisite_count == 0);
    TEST_ASSERT(!model->skills[independent].incomplete && model->skills[independent].depth == 0);
    TEST_ASSERT(!model->skills[a].has_display && !model->skills[tail].has_minimum && model->records[tail].rank == 9);
    TEST_ASSERT(model->skill_link_count == 6 && model->skill_incomplete);
    TEST_ASSERT(has_finding(model, "skill.prerequisite_cycle") && has_finding(model, "skill.placement_unknown"));
    TEST_ASSERT(has_finding(model, "skill.prerequisite_missing") && has_finding(model, "skill.prerequisite_target"));
    TEST_ASSERT(model->skills[missing].layout_layer < model->skills[child].layout_layer);
    TEST_ASSERT(model->skills[a].layout_layer == model->skills[b].layout_layer && model->skills[a].layout_layer < model->skills[tail].layout_layer);
    for (rank_mode = 0; rank_mode < 2; ++rank_mode) for (horizontal = 0; horizontal < 2; ++horizontal) {
        TEST_ASSERT(gd_skill_layout(model, &test_layout, rank_mode, horizontal, GD_SORT_NAME));
        TEST_ASSERT(test_layout.edge_count == model->skill_link_count);
        for (i = 0; i < test_layout.edge_count; ++i) {
            const GD_LayoutNode *p = &test_layout.nodes[test_layout.edges[i].a], *d = &test_layout.nodes[test_layout.edges[i].b];
            const GD_SkillState *before = &model->skills[p->key], *after = &model->skills[d->key];
            if (before->cycle && after->cycle && before->layout_layer == after->layout_layer) continue;
            TEST_ASSERT(horizontal ? p->x < d->x : p->y < d->y);
        }
    }
    gd_destroy(model);
)
TEST(skill_rank_types,
    const struct { const char *value; int valid, ranked; int64_t expected; } samples[] = {
        {"null",1,0,0}, {"!!null 'null'",1,0,0}, {"!!null 'wrong'",0,0,0}, {"!!null \"null\\0hidden\"",0,0,0}, {"-9007199254740991",1,1,-INT64_C(9007199254740991)}, {"0",1,1,0},
        {"9007199254740991",1,1,INT64_C(9007199254740991)}, {"!!int '2'",1,1,2},
        {"!!str null",0,0,0}, {"'2'",0,0,0}, {"true",0,0,0}, {"1.5",0,0,0},
        {"9007199254740992",0,0,0}, {"-9007199254740992",0,0,0}, {"[1]",0,0,0},
        {"{n: 2}",0,0,0}, {"!!float 2",0,0,0}, {"!!int \"2\\0\"",0,0,0}
    };
    int i;
    TEST_ASSERT(setup_skills("skill-rank-types"));
    for (i = 0; i < (int)(sizeof(samples)/sizeof(samples[0])); ++i) {
        GD_Model *model = model_create(); char source[512];
        TEST_ASSERT(model);
        snprintf(source, sizeof(source), "---\nrank: %s\n---\n", samples[i].value);
        TEST_ASSERT(write_record(0, "value", source));
        TEST_ASSERT(gd_load(model, collections, 1) == samples[i].valid);
        if (samples[i].valid) {
            TEST_ASSERT(model->record_count == 1 && model->records[0].has_rank == samples[i].ranked);
            if (samples[i].ranked) TEST_ASSERT(model->records[0].rank == samples[i].expected);
        } else TEST_ASSERT(has_error(model, "skill.rank_type") && model->record_count == 0);
        gd_destroy(model);
    }
    {
        GD_Model *model = model_create();
        TEST_ASSERT(model && write_record(0, "value", "---\nrank: 1\nrank: 2\n---\n"));
        TEST_ASSERT(!gd_load(model, collections, 1) && has_error(model, "input.duplicate_field")); gd_destroy(model);
    }
)
TEST(skill_capacity,
    int phase, i;
    for (phase = 0; phase < 2; ++phase) {
        GD_Model *model = model_create(); clock_t start;
        TEST_ASSERT(model);
        model->record_count = GD_MAX_RECORDS; model->edge_count = GD_MAX_EDGES;
        for (i = 0; i < GD_MAX_RECORDS; ++i) model->records[i].role = GD_SKILL;
        for (i = 0; i < GD_MAX_EDGES; ++i) {
            int child = i < GD_MAX_RECORDS-1 ? i+1 : 1+i%(GD_MAX_RECORDS-1);
            model->edges[i].field = GD_PREREQUISITES; model->edges[i].source = child;
            model->edges[i].target = i < GD_MAX_RECORDS-1 ? i : (i/(GD_MAX_RECORDS-1))%child;
        }
        if (phase) { model->edges[GD_MAX_EDGES-1].source = 0; model->edges[GD_MAX_EDGES-1].target = GD_MAX_RECORDS-1; }
        start = clock(); gd_analyze_skills(model);
        printf("skill capacity probe: 4096 nodes / 32768 declarations / %s: %.1f ms\n", phase ? "cycle" : "chain", 1000.0*(clock()-start)/CLOCKS_PER_SEC);
        if (!phase) TEST_ASSERT(model->skills[GD_MAX_RECORDS-1].depth == GD_MAX_RECORDS-1 && !model->skill_incomplete);
        else {
            for (i = 0; i < GD_MAX_RECORDS; ++i) TEST_ASSERT(model->skills[i].cycle && model->skills[i].incomplete && model->skills[i].depth == -1);
            TEST_ASSERT(model->finding_count == GD_MAX_ERRORS && model->findings_omitted == GD_MAX_RECORDS-GD_MAX_ERRORS);
        }
        gd_destroy(model);
    }
)
static int draft_fixture(const char *name, GD_Model *model) {
    return setup_skills(name) &&
        write_record(0, "root", "---\nrank: 1\n---\nRoot body.") &&
        write_record(0, "child", "---\nrank: null\nprerequisites: [atoms/root, atoms/root]\nmechanics: [rules/keep]\n---\nChild body.") &&
        write_record(0, "leaf", "---\nrank: 2\nprerequisites: [atoms/child]\n---\nLeaf body.") && gd_load(model, collections, 1);
}
TEST(draft_ranks,
    GD_Model *base = model_create(); GD_Draft *draft; int parent, child, leaf;
    TEST_ASSERT(base && draft_fixture("draft-ranks", base));
    parent = find_record(base,"atoms/root"); child = find_record(base,"atoms/child"); leaf = find_record(base,"atoms/leaf");
    draft = gd_draft_create(base); TEST_ASSERT(draft && !draft->modified_count);
    TEST_ASSERT(gd_draft_rank(draft,parent,"4") == 1 && draft->modified_count == 1);
    TEST_ASSERT(draft->model->records[parent].rank == 4 && base->records[parent].rank == 1);
    TEST_ASSERT(!draft->model->records[child].has_rank && draft->model->records[leaf].rank == 2);
    TEST_ASSERT(draft->model->skills[leaf].display_rank == 4 && draft->model->skills[leaf].rank_conflict);
    TEST_ASSERT(!draft->rank_changed[child] && !draft->rank_changed[leaf] && !gd_sources_changed(base));
    TEST_ASSERT(!strcmp(draft->model->records[child].body,"Child body."));
    TEST_ASSERT(gd_draft_rank(draft,parent,"1.5") == -1 && gd_draft_rank(draft,parent,"9007199254740992") == -1);
    TEST_ASSERT(draft->history_count == 1 && gd_draft_rank(draft,parent,"4") == 0);
    TEST_ASSERT(gd_draft_rank(draft,parent,"-9007199254740991") == 1);
    TEST_ASSERT(gd_draft_rank(draft,parent,"0") == 1 && draft->model->skills[leaf].display_rank == 2);
    TEST_ASSERT(gd_draft_rank(draft,parent,"null") == 1 && !draft->model->records[parent].has_rank);
    TEST_ASSERT(gd_draft_rank(draft,parent,"9007199254740991") == 1);
    while (draft->history_count) TEST_ASSERT(gd_draft_undo(draft) == 1);
    TEST_ASSERT(!draft->modified_count && draft->model->records[parent].rank == 1 && !gd_draft_undo(draft));
    TEST_ASSERT(draft->model->finding_count == base->finding_count && !gd_sources_changed(base));
    gd_draft_destroy(draft); gd_destroy(base);
)
TEST(draft_connections,
    GD_Model *base = model_create(); GD_Draft *draft; int parent, child, leaf, count;
    TEST_ASSERT(base && draft_fixture("draft-connections",base));
    parent = find_record(base,"atoms/root"); child = find_record(base,"atoms/child"); leaf = find_record(base,"atoms/leaf");
    draft = gd_draft_create(base); TEST_ASSERT(draft);
    TEST_ASSERT(gd_draft_prerequisite(draft,parent,"atoms/child",1) == 1);
    TEST_ASSERT(draft->model->skills[parent].cycle && draft->model->skills[child].cycle && draft->model->skills[leaf].incomplete);
    TEST_ASSERT(!draft->model->skills[leaf].cycle && draft->modified_count == 1 && !draft->prerequisites_changed[child]);
    TEST_ASSERT(gd_draft_undo(draft) && !draft->model->skill_incomplete && !draft->modified_count);
    TEST_ASSERT(gd_draft_prerequisite(draft,parent,"atoms/root",1) == 1 && draft->model->skills[parent].cycle);
    TEST_ASSERT(gd_draft_undo(draft) && !draft->modified_count);
    TEST_ASSERT(gd_draft_prerequisite(draft,child,"atoms/root",0) == 1 && !draft->model->skills[child].prerequisite_count);
    TEST_ASSERT(gd_draft_prerequisite(draft,child,"atoms/root",1) == 1 && !draft->modified_count);
    TEST_ASSERT(gd_draft_undo(draft) && gd_draft_undo(draft));
    TEST_ASSERT(!memcmp(draft->model->edges,base->edges,base->edge_count*sizeof(base->edges[0])));
    TEST_ASSERT(gd_draft_prerequisite(draft,child,"atoms/unknown",1) == -1 && !draft->modified_count);
    count = base->edge_count; TEST_ASSERT(!gd_sources_changed(base));
    TEST_ASSERT(draft->model->edge_count == count && base->skills[child].declared_prerequisites == 2);
    gd_draft_destroy(draft); gd_destroy(base);
)
TEST(draft_limits,
    GD_Model *base = model_create(); GD_Draft *draft; int parent, child, i;
    TEST_ASSERT(base && draft_fixture("draft-limits",base));
    parent = find_record(base,"atoms/root"); child = find_record(base,"atoms/child");
    draft = gd_draft_create(base); TEST_ASSERT(draft);
    for (i = 0; i < GD_MAX_DRAFT_EDITS; ++i) TEST_ASSERT(gd_draft_rank(draft,parent,i%2 ? "1":"0") == 1);
    TEST_ASSERT(draft->history_count == GD_MAX_DRAFT_EDITS && !draft->modified_count);
    TEST_ASSERT(gd_draft_rank(draft,parent,"4") == -1 && draft->model->records[parent].rank == 1);
    for (i = 0; i < GD_MAX_DRAFT_EDITS; ++i) TEST_ASSERT(gd_draft_undo(draft) == 1);
    TEST_ASSERT(!draft->modified_count && !draft->history_count && !gd_sources_changed(base));
    gd_draft_destroy(draft);
    base->edge_count = GD_MAX_EDGES;
    for (i = 0; i < GD_MAX_EDGES; ++i) base->edges[i] = (GD_Edge){child,parent,GD_PREREQUISITES,base->records[parent].reference,0};
    gd_recompute_skills(base); draft = gd_draft_create(base); TEST_ASSERT(draft);
    TEST_ASSERT(gd_draft_prerequisite(draft,child,"atoms/leaf",1) == -1 && draft->model->edge_count == GD_MAX_EDGES);
    TEST_ASSERT(gd_draft_prerequisite(draft,child,"atoms/root",0) == 1 && !draft->model->edge_count);
    TEST_ASSERT(gd_draft_undo(draft) && draft->model->edge_count == GD_MAX_EDGES && !draft->modified_count);
    gd_draft_destroy(draft); gd_destroy(base);
)
TEST(draft_report,
    GD_Model *base = model_create(); GD_Draft *draft; int parent; char path[GD_MAX_PATH];
    TEST_ASSERT(base && draft_fixture("draft-report",base));
    parent = find_record(base,"atoms/root"); draft = gd_draft_create(base); TEST_ASSERT(draft);
    TEST_ASSERT(gd_draft_rank(draft,parent,"4") == 1);
    TEST_ASSERT(gd_draft_prerequisite(draft,parent,"atoms/leaf",1) == 1);
    TEST_ASSERT(write_record(0,"root","---\nrank: 9\n---\nExternal body."));
    snprintf(path,sizeof(path),"%s/draft-changes.json",root);
    TEST_ASSERT(gd_draft_finish(draft,path) == 0 && draft->modified_count == 1);
    TEST_ASSERT(gd_draft_finish(draft,path) == -1 && draft->modified_count == 1);
    snprintf(path,sizeof(path),"%s/no-folder/report.json",root);
    TEST_ASSERT(gd_draft_finish(draft,path) == -1 && draft->model->records[parent].rank == 4);
    snprintf(path,sizeof(path),"%s/not-a-report.md",root);
    TEST_ASSERT(gd_draft_finish(draft,path) == -1);
    TEST_ASSERT(write_record(0,"root","---\nrank: invalid\n---\nInvalid external state."));
    snprintf(path,sizeof(path),"%s/draft-unavailable.json",root);
    TEST_ASSERT(gd_draft_finish(draft,path) == 2 && draft->model->records[parent].rank == 4);
    TEST_ASSERT(write_record(0,"root","---\nname: renamed\nrank: null\n---\nRenamed external state."));
    snprintf(path,sizeof(path),"%s/draft-identity.json",root);
    TEST_ASSERT(gd_draft_finish(draft,path) == 2);
    TEST_ASSERT(write_record(0,"root","---\nrank: 1\n---\nRoot body."));
    TEST_ASSERT(!gd_sources_changed(base));
    while (draft->history_count) TEST_ASSERT(gd_draft_undo(draft));
    snprintf(path,sizeof(path),"%s/draft-empty.json",root);
    TEST_ASSERT(!draft->modified_count && gd_draft_finish(draft,path) == 0 && !gd_sources_changed(base));
    gd_draft_destroy(draft); gd_destroy(base);
)
static int route_setup(const char *name) {
    if (!setup(name)) return 0;
    collections[0].alias = "atoms"; collections[0].role = GD_SKILL;
    collections[1].alias = "challenges"; collections[1].role = GD_CHALLENGE;
    collections[2].alias = "context"; collections[2].role = GD_RELATED;
    return 1;
}
static GD_RouteCheck *route_check(GD_Model *model, GD_Routes *routes, const char *challenge, const char *skill) {
    int i, c = find_record(model,challenge), s = find_record(model,skill);
    for (i = 0; i < routes->check_count; ++i) if (routes->checks[i].challenge == c && routes->checks[i].skill == s) return &routes->checks[i];
    return NULL;
}
TEST(routes_semantics,
    GD_Model *model = model_create(); GD_Routes *routes; GD_RouteCheck *check; int i, witnessed = 0;
    TEST_ASSERT(model && route_setup("routes-semantics"));
    TEST_ASSERT(write_record(0,"a","---\nprerequisites: [atoms/b]\nrank: -1\n---\nA."));
    TEST_ASSERT(write_record(0,"b","---\nrank: null\n---\nB."));
    TEST_ASSERT(write_record(1,"entry","---\nentry: true\nintroduces: [atoms/b]\nnext: [challenges/practice, challenges/bypass]\n---\nEntry."));
    TEST_ASSERT(write_record(1,"practice","---\npractises: [atoms/a, atoms/a]\nstatus: rejected\nnext: [challenges/demand]\n---\nPractice."));
    TEST_ASSERT(write_record(1,"bypass","---\nrequires: [atoms/a]\nnext: [challenges/demand]\n---\nUse is not teaching."));
    TEST_ASSERT(write_record(1,"demand","---\nintroduces: [atoms/a]\nrequires: [atoms/a, atoms/a]\ntests_mastery: [atoms/a]\nnext: []\n---\nDemand."));
    TEST_ASSERT(write_record(1,"isolated","---\nrequires: [atoms/a]\n---\nNo declared route."));
    TEST_ASSERT(gd_load(model,collections,2)); routes = gd_analyze_routes(model); TEST_ASSERT(routes);
    check = route_check(model,routes,"challenges/demand","atoms/a");
    TEST_ASSERT(check && check->coverage == GD_SUPPORT_SOME && check->direct_requires && check->direct_mastery && check->current_introduction);
    TEST_ASSERT(route_check(model,routes,"challenges/demand","atoms/b")->coverage == GD_SUPPORT_ALL);
    TEST_ASSERT(route_check(model,routes,"challenges/bypass","atoms/a")->coverage == GD_SUPPORT_ABSENT);
    TEST_ASSERT(route_check(model,routes,"challenges/isolated","atoms/a")->coverage == GD_SUPPORT_UNKNOWN);
    for (i = 0; i < routes->witness_count; ++i) {
        GD_RouteWitness *w = &routes->witnesses[i];
        if (&routes->checks[w->check] == check && !w->supported) { TEST_ASSERT(w->length == 3 && w->path[1] == find_record(model,"challenges/bypass")); witnessed = 1; }
    }
    TEST_ASSERT(witnessed && model->edge_count == 14 && routes->check_count == 6 && routes->routes_complete && !gd_sources_changed(model));
    gd_routes_destroy(routes); gd_destroy(model);
)
TEST(routes_types_unknown,
    const char *bad[] = {"'true'","0","null","yes","[true]","!!str true","!!bool \"true\\0\""};
    int i; char text[512]; GD_Model *model; GD_Routes *routes;
    TEST_ASSERT(route_setup("routes-types")); TEST_ASSERT(write_record(0,"a","A."));
    for (i = 0; i < (int)(sizeof(bad)/sizeof(bad[0])); ++i) {
        model = model_create(); snprintf(text,sizeof(text),"---\nentry: %s\n---\nBad.",bad[i]);
        TEST_ASSERT(write_record(1,"entry",text)); TEST_ASSERT(!gd_load(model,collections,2)); gd_destroy(model);
    }
    TEST_ASSERT(write_record(1,"entry","---\nentry: !!bool 'true'\nrequires: [atoms/a]\nnext: [challenges/missing, context/wrong]\nintroduces: [atoms/missing]\n---\nEntry."));
    TEST_ASSERT(write_record(2,"wrong","Wrong role.")); model = model_create(); TEST_ASSERT(gd_load(model,collections,3));
    routes = gd_analyze_routes(model); TEST_ASSERT(routes && !routes->routes_complete && !routes->requirements_complete);
    TEST_ASSERT(routes->checks[0].coverage == GD_SUPPORT_UNKNOWN && routes->checks[0].unsupported_route && !routes->checks[0].supported_route);
    TEST_ASSERT(model->finding_count >= 4 && model->findings[0].line > 0 && !model->error_count);
    gd_routes_destroy(routes); gd_destroy(model);
    TEST_ASSERT(write_record(1,"entry","---\nentry: false\nrequires: [atoms/a]\nnext: null\nintroduces: null\npractises: []\n---\nNo entries."));
    model = model_create(); TEST_ASSERT(gd_load(model,collections,2)); routes = gd_analyze_routes(model);
    TEST_ASSERT(routes && !routes->entry_count && routes->checks[0].coverage == GD_SUPPORT_UNKNOWN && !routes->checks[0].unsupported_route);
    gd_routes_destroy(routes); gd_destroy(model);
    TEST_ASSERT(write_record(1,"entry","---\nentry: true\nentry: false\n---\nDuplicate."));
    model = model_create(); TEST_ASSERT(!gd_load(model,collections,2) && has_error(model,"input.duplicate_field")); gd_destroy(model);
    TEST_ASSERT(write_record(1,"entry","---\nentry: true\nnext: challenges/entry\n---\nScalar route."));
    model = model_create(); TEST_ASSERT(!gd_load(model,collections,2)); gd_destroy(model);
    TEST_ASSERT(write_record(0,"a","---\nprerequisites: [atoms/b]\n---\nA."));
    TEST_ASSERT(write_record(0,"b","---\nprerequisites: [atoms/a, atoms/missing]\n---\nB."));
    TEST_ASSERT(write_record(1,"entry","---\nentry: true\nintroduces: [atoms/a]\nrequires: [atoms/a]\nnext: [challenges/entry]\n---\nRevisit."));
    model = model_create(); TEST_ASSERT(gd_load(model,collections,2)); routes = gd_analyze_routes(model);
    TEST_ASSERT(routes && routes->routes_complete && !routes->requirements_complete && routes->check_count == 2);
    TEST_ASSERT(route_check(model,routes,"challenges/entry","atoms/a")->coverage == GD_SUPPORT_SOME);
    TEST_ASSERT(route_check(model,routes,"challenges/entry","atoms/b")->coverage == GD_SUPPORT_ABSENT);
    TEST_ASSERT(routes->checks[0].supported_route && routes->checks[0].unsupported_route);
    gd_routes_destroy(routes); gd_destroy(model);
)
static void route_edge(GD_Model *model, int source, int target, GD_Field field) {
    GD_Edge *edge = &model->edges[model->edge_count++];
    edge->source = source; edge->target = target; edge->field = field; edge->reference = model->records[target].reference; edge->line = 3;
}
TEST(routes_walk_oracle,
    GD_Model *model = model_create(); unsigned seed = 713; int trial, i, j, k;
    const char *names[] = {"atoms/a","challenges/a","challenges/b","challenges/c","challenges/d"};
    TEST_ASSERT(model); model->record_count = 5; model->route_mode = model->skill_mode = 1;
    for (i = 0; i < 5; ++i) { model->records[i].role = i ? GD_CHALLENGE:GD_SKILL; model->records[i].reference = names[i]; }
    for (trial = 0; trial < 96; ++trial) {
        unsigned char reach[8][8] = {{0}}, support[4], entry[4]; GD_Routes *routes;
        model->edge_count = model->finding_count = model->findings_omitted = 0;
        for (i = 0; i < 4; ++i) {
            seed = seed*1664525u+1013904223u; support[i] = (seed>>28)&1; entry[i] = (seed>>29)&1;
            model->records[i+1].entry = entry[i]; route_edge(model,i+1,0,GD_REQUIRES);
            if (support[i]) route_edge(model,i+1,0,trial&1 ? GD_INTRODUCES:GD_PRACTISES);
        }
        for (i = 0; i < 4; ++i) for (j = 0; j < 4; ++j) {
            seed = seed*1664525u+1013904223u;
            if ((seed>>28)%3 == 0) {
                route_edge(model,i+1,j+1,GD_NEXT);
                reach[2*i][2*j+support[i]] = 1; reach[2*i+1][2*j+1] = 1;
            }
        }
        for (i = 0; i < 8; ++i) reach[i][i] = 1;
        for (k = 0; k < 8; ++k) for (i = 0; i < 8; ++i) for (j = 0; j < 8; ++j) reach[i][j] |= reach[i][k] && reach[k][j];
        routes = gd_analyze_routes(model); TEST_ASSERT(routes && routes->check_count == 4);
        for (i = 0; i < 4; ++i) {
            int yes = 0, no = 0; GD_RouteCheck *check = &routes->checks[i];
            for (j = 0; j < 4; ++j) if (entry[j]) { no |= reach[2*j][2*i]; yes |= reach[2*j][2*i+1]; }
            TEST_ASSERT(check->supported_route == yes && check->unsupported_route == no);
            TEST_ASSERT(check->coverage == (!yes && !no ? GD_SUPPORT_UNKNOWN : yes ? no ? GD_SUPPORT_SOME:GD_SUPPORT_ALL:GD_SUPPORT_ABSENT));
        }
        gd_routes_destroy(routes);
    }
    gd_destroy(model);
)
typedef struct { GD_Allocator parent; int remaining; } RouteFailure;
static void *route_allocate(void *context, size_t size) {
    RouteFailure *failure = context;
    return failure->remaining-- > 0 ? failure->parent.allocate(failure->parent.context,size):NULL;
}
static void route_release(void *context, void *memory) { RouteFailure *failure = context; failure->parent.release(failure->parent.context,memory); }
TEST(routes_limits,
    GD_Model *model = model_create(); GD_Routes *routes; int i; char names[385][32]; GD_Allocator original; RouteFailure failure;
    TEST_ASSERT(model); model->route_mode = model->skill_mode = 1;
    for (i = 0; i < 385; ++i) {
        snprintf(names[i],sizeof(names[i]),i<256 ? "atoms/s%d":"challenges/c%d",i);
        model->records[i].reference = names[i]; model->records[i].role = i<256 ? GD_SKILL:GD_CHALLENGE;
        if (i > 0 && i < 256) route_edge(model,i,i-1,GD_PREREQUISITES);
        if (i >= 256) route_edge(model,i,255,GD_REQUIRES);
    }
    model->records[256].entry = 1; model->record_count = 384; --model->edge_count;
    routes = gd_analyze_routes(model); TEST_ASSERT(routes && routes->check_count == GD_MAX_ROUTE_CHECKS && !model->error_count);
    TEST_ASSERT(routes->witness_count == GD_MAX_ROUTE_WITNESSES && routes->witnesses_omitted > 0 && model->findings_omitted > 0);
    gd_routes_destroy(routes); model->record_count = 385; ++model->edge_count; model->finding_count = model->findings_omitted = 0;
    routes = gd_analyze_routes(model); TEST_ASSERT(routes && has_error(model,"scaffolding.check_limit") && routes->check_count == GD_MAX_ROUTE_CHECKS);
    gd_routes_destroy(routes);
    model->error_count = 0; model->finding_count = model->findings_omitted = 0; model->edge_count = 0; model->record_count = 301;
    model->records[0].role = GD_SKILL;
    for (i = 1; i < 301; ++i) { model->records[i].role = GD_CHALLENGE; model->records[i].entry = i==1; route_edge(model,i,0,GD_REQUIRES); if (i>1) route_edge(model,i-1,i,GD_NEXT); }
    route_edge(model,1,0,GD_INTRODUCES);
    routes = gd_analyze_routes(model); TEST_ASSERT(routes && !model->error_count);
    TEST_ASSERT(routes->witnesses[0].full_length == 300 && routes->witnesses[0].length == GD_MAX_ROUTE_PATH && routes->witnesses[0].path[GD_MAX_ROUTE_PATH-1] == 300);
    gd_routes_destroy(routes);
    original = model->allocator; failure.parent = original; failure.remaining = 0;
    model->allocator.context = &failure; model->allocator.allocate = route_allocate; model->allocator.release = route_release;
    TEST_ASSERT(!gd_analyze_routes(model) && has_error(model,"input.memory"));
    model->error_count = 0; failure.remaining = 1; routes = gd_analyze_routes(model);
    TEST_ASSERT(routes && has_error(model,"input.memory")); gd_routes_destroy(routes);
    model->allocator = original; gd_destroy(model);
)
static int mda_setup(const char *name, GD_Collection *result, char *system_folder) {
    int i;
    if (!setup(name)) return 0;
    collections[0].role = GD_MDA_MECHANIC; collections[1].role = GD_MDA_DYNAMIC; collections[2].role = GD_MDA_AESTHETIC;
    for (i = 0; i < 3; ++i) result[i] = collections[i];
    if (snprintf(system_folder,GD_MAX_PATH,"%s/%s/arrangements",root,name) >= GD_MAX_PATH || !file_directory_create(system_folder,1)) return 0;
    result[3].alias = "arrangements"; result[3].directory = system_folder; result[3].role = GD_MDA_SYSTEM; return 1;
}
static void mda_edge(GD_Model *model, int source, int target, GD_Field field) {
    GD_Edge *edge = &model->edges[model->edge_count++];
    edge->source = source; edge->target = target; edge->reference = model->records[target].reference; edge->field = field; edge->line = model->edge_count;
}
TEST(mda_traces,
    GD_Model *model = model_create(); GD_Mda *mda; GD_Collection inputs[4]; GD_Layout *layout;
    char directory[GD_MAX_PATH], path[GD_MAX_PATH]; int push, intent, i, nodes[GD_MAX_MDA_PATH], links[GD_MAX_MDA_PATH], full, count;
    const char *pack = "---\nstatus: accepted\nmechanics: [things/push]\n---\nConstructed arrangement, not dynamics.";
    TEST_ASSERT(model && mda_setup("mda-traces",inputs,directory));
    TEST_ASSERT(write_record(0,"push","---\nstatus: accepted\nnouns: [unloaded/crate]\ncontrol-vars: [unloaded/force]\n---\nRule, not measured emotion."));
    TEST_ASSERT(write_record(0,"isolated","---\nstatus: rejected\n---\nNo declared association."));
    TEST_ASSERT(write_record(1,"drift","---\nstatus: proposed\nmechanics: [things/push, things/push]\nsystems: [arrangements/pack]\nexperiments: [unloaded/experiment]\n---\nHypothesis only."));
    TEST_ASSERT(write_record(2,"intent","---\nstatus: accepted\ndynamics: [actions/drift]\nexperiments: [unloaded/experiment]\n---\nLocal intent, not whole player story."));
    TEST_ASSERT(write_record(2,"unlinked","---\nstatus: rejected\ndynamics: []\n---\nNo basis declared."));
    snprintf(path,sizeof(path),"%s/pack.md",directory); TEST_ASSERT(file_write_bytes(cstring_from_raw(path),pack,strlen(pack)) == file_error_ok);
    TEST_ASSERT(gd_load(model,inputs,4)); mda = gd_analyze_mda(model);
    TEST_ASSERT(mda && mda->node_count == 6 && mda->link_count == 5 && mda->graph_complete && mda->gap_count == 2);
    push = gd_mda_find(model,"things/push"); intent = gd_mda_find(model,"cases/intent");
    TEST_ASSERT(push >= 0 && intent >= 0 && mda->nodes[push].aesthetic == intent);
    TEST_ASSERT(!strcmp(model->records[gd_mda_find(model,"things/isolated")].status,"rejected"));
    TEST_ASSERT(gd_mda_query(model,mda,push,GD_MDA_FORWARD) && mda->selected_count == 4 && mda->gap_count == 0);
    count = gd_mda_path(mda,intent,nodes,links,GD_MAX_MDA_PATH,&full);
    TEST_ASSERT(count == 3 && full == 3 && nodes[0] == push && nodes[2] == intent);
    for (i = 0; i+1 < count; ++i) {
        const GD_Edge *edge = &model->edges[mda->links[links[i]].edge];
        TEST_ASSERT(edge->source == nodes[i+1] && edge->target == nodes[i]);
    }
    snprintf(path,sizeof(path),"%s/mda-forward.json",root);
    { FILE *output = fopen(path,"wb"); TEST_ASSERT(output && gd_write_mda_result(output,model,mda,1,NULL)); TEST_ASSERT(!fclose(output)); }
    TEST_ASSERT(gd_mda_query(model,mda,intent,GD_MDA_BACKWARD) && mda->selected_count == 4);
    TEST_ASSERT(gd_mda_query(model,mda,push,GD_MDA_BACKWARD) && mda->selected_count == 1);
    TEST_ASSERT(gd_mda_query(model,mda,push,GD_MDA_BOTH) && mda->selected_count == 4);
    layout = model->allocator.allocate(model->allocator.context,sizeof(*layout));
    TEST_ASSERT(layout && gd_mda_layout(model,mda,layout,1) && layout->node_count == 6 && layout->edge_count == 5);
    for (i = 0; i < layout->edge_count; ++i) {
        const GD_Edge *edge = &model->edges[mda->links[layout->edges[i].key].edge];
        TEST_ASSERT(layout->nodes[layout->edges[i].a].key == edge->source && layout->nodes[layout->edges[i].b].key == edge->target);
        TEST_ASSERT(layout->nodes[layout->edges[i].a].layer > layout->nodes[layout->edges[i].b].layer);
    }
    TEST_ASSERT(gd_mda_layout(model,mda,layout,0) && layout->mode == GD_LAYOUT_FORCE && layout->node_count == 6);
    TEST_ASSERT(!gd_sources_changed(model) && !strcmp(model->records[gd_mda_find(model,"actions/drift")].body,"Hypothesis only."));
    model->allocator.release(model->allocator.context,layout); gd_mda_destroy(mda); gd_destroy(model);
)
TEST(mda_resolution,
    GD_Model *model; GD_Mda *mda; GD_Collection inputs[4]; char directory[GD_MAX_PATH]; int i, unloaded = 0, missing = 0, wrong = 0;
    TEST_ASSERT(mda_setup("mda-resolution",inputs,directory));
    TEST_ASSERT(write_record(0,"push","Rule."));
    TEST_ASSERT(write_record(0,"other","Other rule."));
    TEST_ASSERT(write_record(1,"drift","---\nmechanics: [things/push, things/missing, cases/intent]\nsystems: [arrangements/pack]\n---\nDeclared but incomplete."));
    TEST_ASSERT(write_record(2,"intent","---\ndynamics: [actions/drift]\n---\nIntent."));
    model = model_create(); TEST_ASSERT(model && gd_load(model,inputs,3)); mda = gd_analyze_mda(model);
    TEST_ASSERT(mda && !mda->graph_complete && !model->error_count);
    for (i = 0; i < mda->link_count; ++i) { unloaded += mda->links[i].resolution == GD_MDA_UNLOADED; missing += mda->links[i].resolution == GD_MDA_MISSING; wrong += mda->links[i].resolution == GD_MDA_WRONG_ROLE; }
    TEST_ASSERT(unloaded == 1 && missing == 1 && wrong == 1);
    TEST_ASSERT(!strcmp(gd_mda_trace(mda,gd_mda_find(model,"things/other")),"unknown"));
    TEST_ASSERT(!strcmp(gd_mda_trace(mda,gd_mda_find(model,"things/push")),"declared_trace"));
    TEST_ASSERT(gd_mda_query(model,mda,gd_mda_find(model,"things/other"),GD_MDA_BOTH) && mda->selected_count == 1 && mda->finding_count == 1);
    TEST_ASSERT(!strcmp(mda->findings[0].code,"mda.trace_unknown"));
    TEST_ASSERT(!gd_mda_query(model,mda,-2,GD_MDA_BOTH) && !mda->selected_count && !mda->query_complete);
    gd_mda_destroy(mda); gd_destroy(model);
    TEST_ASSERT(write_record(0,"duplicate","---\nname: push\n---\nAmbiguous identity."));
    model = model_create(); TEST_ASSERT(model && !gd_load(model,inputs,3) && has_error(model,"input.identity"));
    mda = gd_analyze_mda(model); TEST_ASSERT(mda && mda->node_count == 5 && gd_mda_find(model,"things/push") == -2);
    for (i = 0; i < mda->link_count; ++i) {
        const char *reference = model->edges[mda->links[i].edge].reference;
        if (!strcmp(reference,"things/push")) TEST_ASSERT(mda->links[i].resolution == GD_MDA_AMBIGUOUS);
        if (!strcmp(reference,"things/missing")) TEST_ASSERT(mda->links[i].resolution == GD_MDA_UNAVAILABLE);
    }
    TEST_ASSERT(!mda->graph_complete && mda->selected_count == 5);
    gd_mda_destroy(mda); gd_destroy(model);
)
TEST(mda_types,
    GD_Model *model; GD_Mda *mda; GD_Collection inputs[4]; char directory[GD_MAX_PATH]; int i;
    const char *bad[] = {"mechanics: scalar", "systems: 7", "experiments: [false]", "mechanics: []\nmechanics: []", "systems: {bad: shape}", "systems: [!!str \"escaped\\0nul\"]"};
    TEST_ASSERT(mda_setup("mda-types",inputs,directory));
    TEST_ASSERT(write_record(0,"rule","Rule.")); TEST_ASSERT(write_record(2,"intent","---\ndynamics: null\n---\nNo basis declared."));
    for (i = 0; i < 6; ++i) {
        char text[256]; snprintf(text,sizeof(text),"---\n%s\n---\nInvalid typed field.",bad[i]);
        TEST_ASSERT(write_record(1,"broken",text)); model = model_create();
        TEST_ASSERT(model && !gd_load(model,inputs,3) && model->error_count);
        mda = gd_analyze_mda(model); TEST_ASSERT(mda && !mda->graph_complete && mda->node_count == 2 && mda->selected_count == 2);
        gd_mda_destroy(mda); gd_destroy(model);
    }
    TEST_ASSERT(write_record(1,"broken","---\nmechanics: null\nsystems: []\nexperiments: null\n---\nEmpty declarations are not parse errors."));
    model = model_create(); TEST_ASSERT(model && gd_load(model,inputs,3)); mda = gd_analyze_mda(model);
    TEST_ASSERT(mda && mda->graph_complete && mda->node_count == 3 && !mda->link_count && mda->gap_count == 3);
    gd_mda_destroy(mda); gd_destroy(model);
)
TEST(mda_oracle,
    GD_Model *model = model_create(); unsigned random = 7813; int trial, i, j, k, direction, root_index;
    char names[12][24];
    TEST_ASSERT(model); model->mda_mode = 1; model->record_count = 12;
    for (i = 0; i < 12; ++i) {
        snprintf(names[i],sizeof(names[i]),"test/r%d",i); model->records[i].reference = names[i]; model->records[i].name = names[i]+5;
        model->records[i].role = i < 3 ? GD_MDA_MECHANIC:i < 5 ? GD_MDA_SYSTEM:i < 9 ? GD_MDA_DYNAMIC:GD_MDA_AESTHETIC;
    }
    for (trial = 0; trial < 64; ++trial) {
        GD_Mda *mda; int authored[12][12] = {{0}};
        model->edge_count = 0;
        for (i = 0; i < 12; ++i) for (j = 0; j < 12; ++j) {
            GD_Role a = model->records[i].role, b = model->records[j].role;
            int allowed = (a == GD_MDA_SYSTEM && b == GD_MDA_MECHANIC) || (a == GD_MDA_DYNAMIC && (b == GD_MDA_MECHANIC || b == GD_MDA_SYSTEM)) || (a == GD_MDA_AESTHETIC && b == GD_MDA_DYNAMIC);
            random = random*1664525u+1013904223u;
            if (allowed && ((random>>24)&3) == 0) {
                mda_edge(model,i,j,b == GD_MDA_MECHANIC ? GD_MECHANICS:b == GD_MDA_SYSTEM ? GD_SYSTEMS:GD_DYNAMICS); authored[i][j] = 1;
                if ((random&7) == 0) mda_edge(model,i,j,b == GD_MDA_MECHANIC ? GD_MECHANICS:b == GD_MDA_SYSTEM ? GD_SYSTEMS:GD_DYNAMICS);
            }
        }
        mda = gd_analyze_mda(model); TEST_ASSERT(mda && mda->graph_complete);
        for (direction = 0; direction < 3; ++direction) {
            int distance[12][12];
            for (i = 0; i < 12; ++i) for (j = 0; j < 12; ++j) distance[i][j] = i == j ? 0:((direction != GD_MDA_FORWARD && authored[i][j]) || (direction != GD_MDA_BACKWARD && authored[j][i])) ? 1:13;
            for (k = 0; k < 12; ++k) for (i = 0; i < 12; ++i) for (j = 0; j < 12; ++j)
                if (distance[i][k]+distance[k][j] < distance[i][j]) distance[i][j] = distance[i][k]+distance[k][j];
            for (root_index = 0; root_index < 12; ++root_index) {
                TEST_ASSERT(gd_mda_query(model,mda,root_index,(GD_MdaDirection)direction));
                for (i = 0; i < 12; ++i) {
                    int nodes[GD_MAX_MDA_PATH], links[GD_MAX_MDA_PATH], full, length;
                    TEST_ASSERT(mda->selected[i] == (distance[root_index][i] < 13));
                    length = gd_mda_path(mda,i,nodes,links,GD_MAX_MDA_PATH,&full);
                    TEST_ASSERT(full == (distance[root_index][i] < 13 ? distance[root_index][i]+1:0) && length == full);
                    if (!length) continue;
                    TEST_ASSERT(nodes[0] == root_index && nodes[length-1] == i);
                    for (j = 0; j+1 < length; ++j) {
                        const GD_Edge *edge = &model->edges[mda->links[links[j]].edge];
                        TEST_ASSERT((direction != GD_MDA_FORWARD && edge->source == nodes[j] && edge->target == nodes[j+1]) || (direction != GD_MDA_BACKWARD && edge->target == nodes[j] && edge->source == nodes[j+1]));
                    }
                }
            }
        }
        for (k = 0; k < 12; ++k) for (i = 0; i < 12; ++i) for (j = 0; j < 12; ++j) authored[i][j] |= authored[i][k] && authored[k][j];
        for (i = 0; i < 12; ++i) {
            int has_aesthetic = model->records[i].role == GD_MDA_AESTHETIC;
            for (j = 9; j < 12; ++j) has_aesthetic |= authored[j][i];
            TEST_ASSERT((mda->nodes[i].aesthetic >= 0) == has_aesthetic);
        }
        gd_mda_destroy(mda);
    }
    gd_destroy(model);
)
TEST(mda_limits,
    GD_Model *model = model_create(); GD_Mda *mda; GD_Allocator original; RouteFailure failure;
    static char names[GD_MAX_RECORDS][24]; int i, nodes[GD_MAX_MDA_PATH], links[GD_MAX_MDA_PATH], full, count;
    TEST_ASSERT(model); model->mda_mode = 1; model->record_count = GD_MAX_RECORDS;
    for (i = 0; i < GD_MAX_RECORDS; ++i) {
        snprintf(names[i],sizeof(names[i]),"test/r%d",i); model->records[i].reference = names[i]; model->records[i].name = names[i]+5;
        model->records[i].role = i%2 ? GD_MDA_DYNAMIC:GD_MDA_MECHANIC;
    }
    for (i = 1; i < GD_MAX_RECORDS; i += 2) { mda_edge(model,i,i-1,GD_MECHANICS); if (i+1 < GD_MAX_RECORDS) mda_edge(model,i,i+1,GD_MECHANICS); }
    while (model->edge_count < GD_MAX_EDGES) mda_edge(model,1,0,GD_MECHANICS);
    mda = gd_analyze_mda(model); TEST_ASSERT(mda && mda->node_count == GD_MAX_RECORDS && mda->link_count == GD_MAX_EDGES);
    TEST_ASSERT(mda->gap_count == GD_MAX_RECORDS && mda->finding_count == GD_MAX_ERRORS && mda->findings_omitted == GD_MAX_RECORDS-GD_MAX_ERRORS);
    TEST_ASSERT(gd_mda_query(model,mda,0,GD_MDA_BOTH) && mda->selected_count == GD_MAX_RECORDS);
    count = gd_mda_path(mda,GD_MAX_RECORDS-1,nodes,links,GD_MAX_MDA_PATH,&full);
    TEST_ASSERT(full == GD_MAX_RECORDS && count == GD_MAX_MDA_PATH && nodes[0] == GD_MAX_RECORDS-GD_MAX_MDA_PATH && nodes[count-1] == GD_MAX_RECORDS-1);
    TEST_ASSERT(gd_mda_query(model,mda,0,GD_MDA_FORWARD) && mda->selected_count == 2 && !mda->findings_omitted);
    original = model->allocator; failure.parent = original; failure.remaining = 0;
    model->allocator.context = &failure; model->allocator.allocate = route_allocate; model->allocator.release = route_release;
    TEST_ASSERT(!gd_analyze_mda(model) && has_error(model,"input.memory"));
    model->allocator = original; gd_mda_destroy(mda); gd_destroy(model);
)
static void control_definition(GD_Model *model, int index, const char *reference, const char *type, const char *value, const char *expression, const char *unit) {
    GD_Record *record=&model->records[index];
    memset(record,0,sizeof(*record)); record->role=GD_CONTROL; record->reference=reference;
    record->name=strchr(reference,'/')+1; record->source_path=reference;
    record->control.type=type; record->control.value=value; record->control.expression=expression; record->control.unit=unit;
    record->control.kind=value ? GD_LITERAL_NUMBER:GD_LITERAL_NULL;
}
TEST(control_units_numbers,
    GD_Unit unit; double number;
    TEST_ASSERT(gd_control_unit("m/s",3,&unit) && unit.count==2 && unit.terms[1].power==-1);
    TEST_ASSERT(gd_control_unit("m*s^-1",6,&unit) && unit.count==2 && unit.terms[1].power==-1);
    TEST_ASSERT(gd_control_unit("energy/energy",13,&unit) && !unit.count);
    TEST_ASSERT(gd_control_unit("1",1,&unit) && !unit.count);
    TEST_ASSERT(!gd_control_unit("",0,&unit) && !gd_control_unit("m/",2,&unit) && !gd_control_unit("m^33",4,&unit));
    TEST_ASSERT(!gd_control_unit("m/(s*s)",7,&unit) && !gd_control_unit("m m",3,&unit));
    TEST_ASSERT(gd_control_number("-9007199254740991",&number,1));
    TEST_ASSERT(!gd_control_number("9007199254740992",&number,1));
    TEST_ASSERT(!gd_control_number("9007199254740991.1",&number,1));
    TEST_ASSERT(!gd_control_number("1.00000000000000000001",&number,1));
    TEST_ASSERT(!gd_control_number("1e-999",&number,0) && !gd_control_number("nan",&number,0));
    TEST_ASSERT(gd_control_number("1.250e2",&number,1) && number==125);
    TEST_ASSERT(!gd_control_number(".1",&number,1) && !gd_control_number("1.2.3",&number,0));
)
TEST(control_formulas,
    GD_Model *model=model_create(); GD_Controls *controls; int i;
    const char *expressions[]={"1+2*3","(1+2)*3","--2 + -3","min(3, 2)","max(-3,-2)","clamp(8,2,4)","abs(-3)","floor(-2.1)","ceil(-2.1)","round(-2.5)","round(2.5)","round(0.49999999999999994)","1e2 + .5", "-(min(2,max(3,4)))"};
    double expected[]={7,9,-1,2,-2,4,3,-3,-2,-3,3,0,100.5,-2};
    TEST_ASSERT(model); model->record_count=1; model->control_mode=1;
    control_definition(model,0,"config/value","number",NULL,NULL,"1");
    for (i=0;i<14;++i) {
        model->records[0].control.expression=expressions[i]; model->error_count=model->errors_omitted=0;
        controls=gd_resolve_controls(model,-1,NULL,0);
        TEST_ASSERT(controls && controls->complete && controls->values[0].number==expected[i]); gd_controls_destroy(controls);
    }
    model->records[0].control.expression="1[s] + 0.2[s]"; model->records[0].control.unit="s";
    controls=gd_resolve_controls(model,-1,NULL,0); TEST_ASSERT(controls && controls->complete && fabs(controls->values[0].number-1.2)<1e-15); gd_controls_destroy(controls);
    gd_destroy(model);
)
TEST(control_reference_syntax,
    GD_Model *model=model_create(); GD_Controls *controls; int i;
    const char *invalid[]={"${config/a}","{config/a} + ${config/a}","$ {config/a}","{}","{config/a","{config/a}}","{{config/a}}"};
    TEST_ASSERT(model); model->record_count=2;
    control_definition(model,0,"config/a","number","2",NULL,"s");
    control_definition(model,1,"config/b","number",NULL,"({config/a} + {config/a}) / 2","s");
    model->records[1].control.lines[2]=17;
    controls=gd_resolve_controls(model,-1,NULL,0);
    TEST_ASSERT(controls && controls->complete && controls->values[1].number==2 && controls->tokens[0].offset==1);
    gd_controls_destroy(controls);
    for (i=0;i<7;++i) {
        model->error_count=model->errors_omitted=0; model->records[1].control.expression=invalid[i];
        controls=gd_resolve_controls(model,-1,NULL,0);
        TEST_ASSERT(controls && !controls->complete && model->error_count);
        if (i<2) {
            TEST_ASSERT(has_error(model,"controls.reference_syntax") && model->errors[0].line==17);
            TEST_ASSERT(!strcmp(model->errors[0].path,"config/b") && strstr(model->errors[0].message,"{ALIAS/NAME}"));
        }
        gd_controls_destroy(controls);
    }
    model->error_count=model->errors_omitted=0; model->records[1].control.expression="{config/missing}";
    controls=gd_resolve_controls(model,-1,NULL,0);
    TEST_ASSERT(controls && !controls->complete && has_error(model,"controls.reference"));
    gd_controls_destroy(controls); gd_destroy(model);
)
TEST(control_errors,
    GD_Model *model=model_create(); GD_Controls *controls; int i;
    const char *invalid[]={"1/0","clamp(1,3,2)","1[s]+2[ms]","min(1,2,3)","pow(2,3)","1;system(2)","()","min()","1+","1e309","2[bad/]","1,2","(1,2)","1(2)","1[s]+1","1e-999"};
    TEST_ASSERT(model); model->record_count=1; control_definition(model,0,"config/value","number",NULL,NULL,"1");
    for (i=0;i<16;++i) {
        model->error_count=model->errors_omitted=0; model->records[0].control.expression=invalid[i];
        controls=gd_resolve_controls(model,-1,NULL,0); TEST_ASSERT(controls && !controls->complete && model->error_count); gd_controls_destroy(controls);
    }
    model->error_count=0; model->records[0].control.type="integer"; model->records[0].control.expression="3 / 2";
    controls=gd_resolve_controls(model,-1,NULL,0); TEST_ASSERT(controls && !controls->complete && has_error(model,"controls.type")); gd_controls_destroy(controls);
    model->error_count=0; model->records[0].control.expression="round(3/2)";
    controls=gd_resolve_controls(model,-1,NULL,0); TEST_ASSERT(controls && controls->complete && controls->values[0].number==2); gd_controls_destroy(controls);
    model->error_count=0; model->records[0].control.expression="9007199254740991+1-1";
    controls=gd_resolve_controls(model,-1,NULL,0); TEST_ASSERT(controls && !controls->complete); gd_controls_destroy(controls);
    model->error_count=0; model->records[0].control.expression=NULL; model->records[0].control.value="1";
    model->records[0].control.kind=GD_LITERAL_NUMBER; model->records[0].control.minimum="2";
    controls=gd_resolve_controls(model,-1,NULL,0); TEST_ASSERT(controls && !controls->complete && has_error(model,"controls.range")); gd_controls_destroy(controls);
    model->error_count=0; model->records[0].control.minimum=NULL; model->records[0].control.type="boolean";
    controls=gd_resolve_controls(model,-1,NULL,0); TEST_ASSERT(controls && !controls->complete && has_error(model,"controls.type")); gd_controls_destroy(controls);
    gd_destroy(model);
)
TEST(control_loading,
    GD_Model *model=model_create(); GD_Controls *controls; int flag,mode;
    TEST_ASSERT(model && setup("control-loading")); collections[0].role=GD_CONTROL; collections[0].alias="config";
    TEST_ASSERT(write_record(0,"a","---\nvalue_type: number\nvalue: 2\nunit: m/s\n---\n"));
    TEST_ASSERT(write_record(0,"b","---\nvalue_type: number\nvalue: null\nexpression: '1[m] / {config/a}'\nunit: s\n---\n"));
    TEST_ASSERT(write_record(0,"flag","---\nvalue_type: boolean\nvalue: false\n---\n"));
    TEST_ASSERT(write_record(0,"mode","---\nvalue_type: enum\nvalue: '=value'\nallowed_values: ['=value', 'other', '']\n---\n"));
    TEST_ASSERT(gd_load(model,collections,1)); controls=gd_resolve_controls(model,-1,NULL,0);
    flag=gd_control_find(model,"config/flag"); mode=gd_control_find(model,"config/mode");
    TEST_ASSERT(controls && controls->complete && controls->resolved_count==4 && controls->values[gd_control_find(model,"config/b")].number==0.5);
    TEST_ASSERT(controls->values[flag].kind==GD_LITERAL_BOOLEAN && !controls->values[flag].number && !strcmp(controls->values[mode].string,"=value"));
    TEST_ASSERT(model->records[mode].control.allowed_count==3 && !gd_sources_changed(model));
    gd_controls_destroy(controls); gd_destroy(model);
    model=model_create(); TEST_ASSERT(model && setup("control-null-enum")); collections[0].role=GD_CONTROL;
    TEST_ASSERT(write_record(0,"bad","---\nvalue_type: enum\nvalue: x\nallowed_values: [null]\n---\n"));
    TEST_ASSERT(!gd_load(model,collections,1) && has_error(model,"controls.enum_value")); gd_destroy(model);
    model=model_create(); TEST_ASSERT(model && setup("control-bad-metadata")); collections[0].role=GD_CONTROL;
    TEST_ASSERT(write_record(0,"duplicate","---\nvalue: 2\nvalue: 3\n---\n"));
    TEST_ASSERT(write_record(0,"unit","---\nunit: []\n---\n"));
    TEST_ASSERT(write_record(0,"enum","---\nallowed_values: [2]\n---\n"));
    TEST_ASSERT(write_record(0,"bounds","---\nminimum: false\n---\n"));
    TEST_ASSERT(write_record(0,"alias","---\nvalue: &number 2\nunit: *number\n---\n"));
    TEST_ASSERT(write_record(0,"finite","---\nvalue: .inf\n---\n"));
    TEST_ASSERT(write_record(0,"expression","---\nexpression: [1]\n---\n"));
    TEST_ASSERT(!gd_load(model,collections,1) && !model->record_count && model->error_count>=7 && has_error(model,"input.duplicate_field")); gd_destroy(model);
)
TEST(control_overrides_scope,
    GD_Model *model=model_create(); GD_Controls *controls; GD_ControlOverride override={"config/a","4"};
    TEST_ASSERT(model); model->record_count=3;
    control_definition(model,0,"config/a","number","2",NULL,"m/s");
    control_definition(model,1,"config/b","number",NULL,"1[m]/{config/a}","s");
    control_definition(model,2,"config/unknown","unspecified",NULL,NULL,NULL);
    controls=gd_resolve_controls(model,1,&override,1);
    TEST_ASSERT(controls && controls->complete && controls->selected_count==2 && controls->values[1].number==0.25);
    TEST_ASSERT(controls->overridden[0] && !strcmp(model->records[0].control.value,"2") && !controls->selected[2]); gd_controls_destroy(controls);
    override.reference="config/b"; override.value="0.75";
    controls=gd_resolve_controls(model,1,&override,1);
    TEST_ASSERT(controls && controls->complete && controls->selected_count==1 && controls->values[1].number==0.75 && !strcmp(model->records[1].control.expression,"1[m]/{config/a}")); gd_controls_destroy(controls);
    controls=gd_resolve_controls(model,-1,NULL,0); TEST_ASSERT(controls && !controls->complete && controls->resolved_count==2); gd_controls_destroy(controls); gd_destroy(model);
)
TEST(control_selection,
    GD_Model *model=model_create(); GD_Controls *controls; GD_ControlFilter filter={0}; GD_ControlSelection selection;
    int a,b,c,d;
    TEST_ASSERT(model && setup("control-selection")); collections[0].role=GD_CONTROL; collections[0].alias="config";
    TEST_ASSERT(write_record(0,"a","---\nstatus: accepted\nvalue_type: number\nvalue: 2\nunit: m\n---\n"));
    TEST_ASSERT(write_record(0,"b","---\nstatus: proposed\nvalue_type: number\nvalue: 3\nunit: m\n---\n"));
    TEST_ASSERT(write_record(0,"c","---\nstatus: accepted\nvalue_type: number\nexpression: '{config/a} + {config/b}'\nunit: m\n---\n"));
    TEST_ASSERT(write_record(0,"d","---\nstatus: rejected\nvalue_type: boolean\nvalue: false\n---\n"));
    TEST_ASSERT(gd_load(model,collections,1));
    a=gd_control_find(model,"config/a"); b=gd_control_find(model,"config/b"); c=gd_control_find(model,"config/c"); d=gd_control_find(model,"config/d");
    filter.root=-1; filter.accepted_default=1;
    TEST_ASSERT(gd_select_controls(model,&filter,&selection) && selection.roots[a] && selection.roots[c] && !selection.roots[b] && !selection.allowed[d]);
    controls=gd_resolve_control_set(model,&selection,NULL,0);
    TEST_ASSERT(controls && !controls->complete && has_error(model,"controls.status")); gd_controls_destroy(controls); model->error_count=0;
    filter.status_count=1; filter.statuses[0]="all";
    TEST_ASSERT(gd_select_controls(model,&filter,&selection)); controls=gd_resolve_control_set(model,&selection,NULL,0);
    TEST_ASSERT(controls && controls->complete && controls->selected_count==4 && controls->values[c].number==5); gd_controls_destroy(controls);
    filter.status_count=0; filter.include_count=1; filter.include[0]="c";
    TEST_ASSERT(gd_select_controls(model,&filter,&selection)); controls=gd_resolve_control_set(model,&selection,NULL,0);
    TEST_ASSERT(controls && controls->complete && controls->selected_count==3 && controls->requested[c] && !controls->requested[a] && !controls->requested[b]); gd_controls_destroy(controls);
    filter.exclude_count=1; filter.exclude[0]="config/c";
    TEST_ASSERT(gd_select_controls(model,&filter,&selection)); controls=gd_resolve_control_set(model,&selection,NULL,0);
    TEST_ASSERT(controls && controls->complete && !controls->selected_count); gd_controls_destroy(controls);
    filter.exclude_count=0; filter.include[0]="b"; filter.status_count=1; filter.statuses[0]="accepted";
    TEST_ASSERT(gd_select_controls(model,&filter,&selection) && !selection.roots[b]);
    filter.include[0]="missing"; TEST_ASSERT(!gd_select_controls(model,&filter,&selection) && has_error(model,"controls.selection"));
    gd_destroy(model);
)
TEST(control_limits,
    GD_Model *model=model_create(); GD_Controls *controls; GD_Allocator original; RouteFailure failure;
    static char names[GD_MAX_RECORDS][32],expressions[GD_MAX_RECORDS][64]; int i;
    TEST_ASSERT(model); model->record_count=1;
    {
        char too_many[601];
        for (i=0;i<599;++i) too_many[i]=i%2 ? '+':'1'; too_many[599]=0;
        control_definition(model,0,"config/value","number",NULL,too_many,"1");
        controls=gd_resolve_controls(model,-1,NULL,0);
        TEST_ASSERT(controls && !controls->complete && has_error(model,"controls.expression_limit")); gd_controls_destroy(controls);
        model->error_count=model->errors_omitted=0;
    }
    model->record_count=GD_MAX_RECORDS;
    for (i=0;i<GD_MAX_RECORDS;++i) {
        snprintf(names[i],sizeof(names[i]),"config/n%04d",i);
        if (i) snprintf(expressions[i],sizeof(expressions[i]),"{config/n%04d}+1",i-1);
        control_definition(model,i,names[i],"integer",i ? NULL:"0",i ? expressions[i]:NULL,"1");
    }
    controls=gd_resolve_controls(model,GD_MAX_RECORDS-1,NULL,0);
    TEST_ASSERT(controls && controls->complete && controls->selected_count==GD_MAX_RECORDS && controls->values[GD_MAX_RECORDS-1].number==GD_MAX_RECORDS-1); gd_controls_destroy(controls);
    model->records[0].control.value=NULL; model->records[0].control.kind=GD_LITERAL_NULL; model->records[0].control.expression="{config/n4095}";
    controls=gd_resolve_controls(model,-1,NULL,0);
    TEST_ASSERT(controls && !controls->complete && !controls->resolved_count && has_error(model,"controls.cycle") && model->errors_omitted==GD_MAX_RECORDS-GD_MAX_ERRORS); gd_controls_destroy(controls);
    model->error_count=model->errors_omitted=0; original=model->allocator; failure.parent=original; failure.remaining=0;
    model->allocator.context=&failure; model->allocator.allocate=route_allocate; model->allocator.release=route_release;
    TEST_ASSERT(!gd_resolve_controls(model,-1,NULL,0) && has_error(model,"input.memory")); model->allocator=original; gd_destroy(model);
)
TEST(skill_progression_modes,
    GD_Model *base=model_create(); GD_Draft *draft; GD_Routes *routes; GD_Layout *layout;
    GD_Focus focus;
    int a,b,demand,entry,isolated,i;
    TEST_ASSERT(base && route_setup("explorer-modes"));
    TEST_ASSERT(write_record(0,"a","---\nrank: 0\n---\nA."));
    TEST_ASSERT(write_record(0,"b","---\nrank: null\n---\nB."));
    TEST_ASSERT(write_record(1,"entry","---\nentry: true\nintroduces: [atoms/a]\nnext: [challenges/demand, challenges/demand]\n---\nEntry."));
    TEST_ASSERT(write_record(1,"demand","---\nrequires: [atoms/a]\nnext: [challenges/entry, challenges/demand]\n---\nDemand."));
    TEST_ASSERT(write_record(1,"isolated","---\nnext: [atoms/a, challenges/missing]\n---\nIsolated."));
    TEST_ASSERT(gd_load(base,collections,2)); draft=gd_draft_create(base); TEST_ASSERT(draft && draft->model->route_mode);
    layout=base->allocator.allocate(base->allocator.context,sizeof(*layout)); TEST_ASSERT(layout);
    a=find_record(base,"atoms/a"); b=find_record(base,"atoms/b"); demand=find_record(base,"challenges/demand"); entry=find_record(base,"challenges/entry"); isolated=find_record(base,"challenges/isolated");
    routes=gd_analyze_routes(draft->model); TEST_ASSERT(routes && route_check(draft->model,routes,"challenges/demand","atoms/a")->coverage==GD_SUPPORT_UNKNOWN);
    gd_routes_destroy(routes);
    TEST_ASSERT(gd_draft_prerequisite(draft,a,"atoms/b",1)==1 && gd_draft_rank(draft,a,"-2")==1);
    TEST_ASSERT(gd_focus_reset(draft->model,&focus,a));
    for (i=0;i<6;++i) {
        TEST_ASSERT(gd_focus_layout(draft->model,&focus,layout,GD_SORT_NAME) && focus.visible[b]);
        TEST_ASSERT(gd_skill_layout(draft->model,layout,i%2,i%2,GD_SORT_NAME) && layout->node_count==2);
        TEST_ASSERT(gd_route_layout(draft->model,layout,i%2,GD_SORT_PARTNERS) && layout->node_count==3 && layout->edge_count==4);
        TEST_ASSERT(layout->nodes[0].key>=0 && draft->modified_count==1 && draft->history_count==2 && draft->model->records[a].rank==-2);
    }
    for (i=0;i<layout->node_count;++i) {
        if (layout->nodes[i].key==entry) TEST_ASSERT(layout->nodes[i].layer==0);
        if (layout->nodes[i].key==demand) TEST_ASSERT(layout->nodes[i].layer==1);
        if (layout->nodes[i].key==isolated) TEST_ASSERT(layout->nodes[i].layer==-1);
    }
    routes=gd_analyze_routes(draft->model);
    TEST_ASSERT(routes && route_check(draft->model,routes,"challenges/demand","atoms/b") && routes->checks[1].skill>=0);
    TEST_ASSERT(base->records[a].rank==0 && !draft->model->records[b].has_rank && !gd_sources_changed(base));
    gd_routes_destroy(routes);
    TEST_ASSERT(gd_draft_undo(draft) && gd_draft_undo(draft) && draft->modified_count==0);
    TEST_ASSERT(gd_focus_layout(draft->model,&focus,layout,GD_SORT_NAME) && focus.visible[b]);
    for (i=0;i<layout->edge_count;++i) TEST_ASSERT(draft->model->edges[layout->edges[i].key].field!=GD_PREREQUISITES);
    TEST_ASSERT(gd_route_layout(draft->model,layout,0,GD_SORT_NAME));
    base->allocator.release(base->allocator.context,layout); gd_draft_destroy(draft); gd_destroy(base);
)
TEST(route_layout_capacity,
    GD_Model *model=model_create(); GD_Layout *layout; int i;
    TEST_ASSERT(model); layout=model->allocator.allocate(model->allocator.context,sizeof(*layout)); TEST_ASSERT(layout);
    TEST_ASSERT(gd_route_layout(model,layout,0,GD_SORT_NAME) && !layout->node_count);
    model->record_count=GD_MAX_RECORDS;
    for (i=0;i<GD_MAX_RECORDS;++i) { model->records[i].role=GD_CHALLENGE; model->records[i].name="node"; model->records[i].reference="challenges/node"; model->records[i].entry=i==0; }
    for (i=0;i<GD_MAX_RECORDS-1;++i) route_edge(model,i,i+1,GD_NEXT);
    while (model->edge_count<GD_MAX_EDGES) route_edge(model,GD_MAX_RECORDS-1,0,GD_NEXT);
    TEST_ASSERT(gd_route_layout(model,layout,1,GD_SORT_NAME) && layout->node_count==GD_MAX_RECORDS && layout->edge_count==GD_MAX_EDGES);
    TEST_ASSERT(layout->nodes[GD_MAX_RECORDS-1].layer==GD_MAX_RECORDS-1 && layout->nodes[0].layer==0);
    model->allocator.release(model->allocator.context,layout); gd_destroy(model);
)
static GD_Model *import_model(void) {
    GD_Model *model=model_create();
    if (!model) return NULL;
    model->record_count=4;
    control_definition(model,0,"config/a","number","2",NULL,"s");
    control_definition(model,1,"config/b","number","3",NULL,"s");
    control_definition(model,2,"config/c","number",NULL,"{config/a}+{config/b}","s");
    control_definition(model,3,"config/mode","enum","plain",NULL,NULL);
    model->records[3].control.kind=GD_LITERAL_STRING;
    model->records[3].control.allowed="plain\0\xe9\x9b\xaa\0\xf0\x9f\x98\x80\0line\n\"quoted\"\0";
    model->records[3].control.allowed_count=4;
    return model;
}
TEST(control_import_roundtrip,
    GD_Model *model=import_model(); GD_ControlImport *plan; int i;
    const char *formats[]={"json","csv","ini"};
    const char *data[]={"{\"a\":4,\"c\":5}","key,type,value,unit\r\na,number,4,s\r\nc,number,5,s\r\n","[controls]\na=4\nc=5\n"};
    TEST_ASSERT(model);
    for (i=0;i<3;++i) {
        plan=gd_plan_control_import(model,NULL,data[i],strlen(data[i]),formats[i],"game");
        TEST_ASSERT(plan && plan->complete && plan->changed_count==1 && plan->derived_count==1);
        TEST_ASSERT(plan->current->values[2].number==5 && plan->candidate->values[2].number==7 && !plan->inputs[1].changed && !strcmp(model->records[0].control.value,"2"));
        gd_control_import_destroy(plan);
    }
    {
        const char *matching="{\"a\":4,\"c\":7}";
        plan=gd_plan_control_import(model,NULL,matching,strlen(matching),"json","game");
        TEST_ASSERT(plan && plan->complete && plan->inputs[1].changed && !plan->inputs[1].conflict); gd_control_import_destroy(plan);
    }
    {
        const char *mismatch="{\"a\":4,\"c\":6}";
        plan=gd_plan_control_import(model,NULL,mismatch,strlen(mismatch),"json","game");
        TEST_ASSERT(plan && !plan->complete && plan->inputs[1].conflict && has_error(model,"controls.import_formula")); gd_control_import_destroy(plan); model->error_count=0;
    }
    {
        GD_ControlSelection selection; const char *changed="{\"a\":4}";
        memset(&selection,0,sizeof(selection)); selection.roots[0]=1;
        for (i=0;i<4;++i) selection.allowed[i]=1;
        model->records[2].control.maximum="6";
        plan=gd_plan_control_import(model,&selection,changed,strlen(changed),"json","game");
        TEST_ASSERT(plan && !plan->complete && has_error(model,"controls.range") && plan->candidate->selected[2]); gd_control_import_destroy(plan);
    }
    gd_destroy(model);
)
TEST(control_import_legacy_dependent,
    GD_Model *model=import_model(); GD_ControlImport *plan; GD_ControlSelection selection;
    const char *changed="{\"a\":4}"; int i;
    TEST_ASSERT(model); memset(&selection,0,sizeof(selection)); selection.roots[0]=1;
    for (i=0;i<4;++i) selection.allowed[i]=1;
    model->records[2].control.expression="${config/a}+{config/b}";
    plan=gd_plan_control_import(model,&selection,changed,strlen(changed),"json","game");
    TEST_ASSERT(plan && !plan->complete && plan->candidate && plan->candidate->selected[2]);
    TEST_ASSERT(has_error(model,"controls.reference_syntax") && !strcmp(model->records[0].control.value,"2"));
    gd_control_import_destroy(plan); gd_destroy(model);
)
TEST(control_import_strings,
    GD_Model *model=import_model(); GD_ControlImport *plan; int i;
    const char *data[]={"{\"mode\":\"\\u96ea\"}","{\"mode\":\"\\ud83d\\ude00\"}","key,type,value,unit\nmode,enum,\"line\n\"\"quoted\"\"\",\n","[controls]\nmode=\"line\\n\\\"quoted\\\"\"\n"};
    const char *formats[]={"json","json","csv","ini"};
    const char *expected[]={"\xe9\x9b\xaa","\xf0\x9f\x98\x80","line\n\"quoted\"","line\n\"quoted\""};
    TEST_ASSERT(model);
    for (i=0;i<4;++i) {
        plan=gd_plan_control_import(model,NULL,data[i],strlen(data[i]),formats[i],"game");
        TEST_ASSERT(plan && plan->complete && plan->changed_count==1 && !strcmp(plan->candidate->values[3].string,expected[i])); gd_control_import_destroy(plan);
    }
    gd_destroy(model);
)
TEST(control_import_failures,
    GD_Model *model=import_model(); GD_ControlImport *plan; int i;
    const char *invalid[]={"{\"a\":2,\"a\":3}","{\"absent\":1}","{\"a\":\"2\"}","{\"a\":01}","{\"a\":.5}","{\"a\":+1}","{\"a\":1e309}","{\"a\":true}","{\"a\":null}","{\"a\":1,}","{a:1}","{\"a\":[]}","{\"a\":1}extra","{\"mode\":\"\\u0000\"}","{\"mode\":\"\\ud800\"}","{\"mode\":\"\\udc00\"}","{\"mode\":\"\\x61\"}","{\"mode\":\"not-allowed\"}"};
    TEST_ASSERT(model);
    for (i=0;i<18;++i) {
        model->error_count=model->errors_omitted=0;
        plan=gd_plan_control_import(model,NULL,invalid[i],strlen(invalid[i]),"json","game");
        TEST_ASSERT(plan && !plan->complete && model->error_count); gd_control_import_destroy(plan);
    }
    model->error_count=0;
    TEST_ASSERT(!gd_plan_control_import(model,NULL,"{}",GD_MAX_FILE+1,"json","game") && has_error(model,"controls.import_input"));
    model->error_count=0;
    {
        GD_Allocator original=model->allocator; RouteFailure failure;
        failure.parent=original; failure.remaining=1; model->allocator.context=&failure; model->allocator.allocate=route_allocate; model->allocator.release=route_release;
        plan=gd_plan_control_import(model,NULL,"{}",2,"json","game");
        TEST_ASSERT(plan && !plan->complete && !plan->current && has_error(model,"input.memory")); gd_control_import_destroy(plan); model->allocator=original;
    }
    gd_destroy(model);
)
TEST(focused_relationships,
    GD_Model *model=model_create(); GD_Focus focus; int i;
    GD_Layout *layout=allocate(storage,sizeof(*layout));
    const char *references[]={"things/incoming","skills/root","mechanics/neighbor","cases/farther","things/hidden","cases/isolated"};
    TEST_ASSERT(model && layout);
    model->record_count=6; model->edge_count=7;
    for (i=0;i<6;++i) { model->records[i].reference=references[i]; model->records[i].status=i%2 ? "rejected":"accepted"; }
    model->edges[0]=(GD_Edge){0,1,GD_MECHANICS,references[1],10};
    model->edges[1]=(GD_Edge){1,2,GD_MECHANICS,references[2],11};
    model->edges[2]=(GD_Edge){2,3,GD_MECHANICS,references[3],12};
    model->edges[3]=model->edges[1];
    model->edges[4]=(GD_Edge){1,1,GD_PREREQUISITES,references[1],14};
    model->edges[5]=(GD_Edge){1,-1,GD_MECHANICS,"missing/unknown",15};
    model->edges[6]=(GD_Edge){4,5,GD_MECHANICS,references[5],16};
    TEST_ASSERT(gd_focus_reset(model,&focus,1) && gd_focus_layout(model,&focus,layout,GD_SORT_NAME));
    TEST_ASSERT(layout->node_count==3 && layout->edge_count==4 && focus.boundary_count==2);
    TEST_ASSERT(focus.visible[0] && focus.visible[1] && focus.visible[2] && !focus.visible[3] && !focus.visible[4] && !focus.visible[5]);
    TEST_ASSERT(focus.expanded[1] && !focus.expanded[0] && !focus.expanded[2]);
    TEST_ASSERT(layout->edges[0].key==0 && layout->edges[1].key==1 && layout->edges[2].key==3 && layout->edges[3].key==4);
    TEST_ASSERT(layout->nodes[layout->edges[0].a].key==0 && layout->nodes[layout->edges[0].b].key==1);
    TEST_ASSERT(gd_focus_expand(model,&focus,2) && gd_focus_layout(model,&focus,layout,GD_SORT_PARTNERS));
    TEST_ASSERT(layout->node_count==4 && layout->edge_count==5 && focus.boundary_count==1);
    TEST_ASSERT(gd_focus_reset(model,&focus,1));
    model->edges[1].target=5; model->edges[1].reference=references[5];
    TEST_ASSERT(gd_focus_layout(model,&focus,layout,GD_SORT_NAME) && focus.visible[5] && !focus.visible[4] && !focus.visible[3]);
    model->edges[1].target=-1;
    TEST_ASSERT(gd_focus_layout(model,&focus,layout,GD_SORT_NAME) && focus.visible[5]); /* Retain revealed isolate. */
    TEST_ASSERT(!gd_focus_reset(model,&focus,6) && focus.root==1);
    TEST_ASSERT(!gd_focus_expand(model,&focus,-1));
    TEST_ASSERT(gd_focus_reset(model,&focus,-1) && gd_focus_layout(model,&focus,layout,GD_SORT_NAME));
    TEST_ASSERT(!layout->node_count && !layout->edge_count && !focus.boundary_count);
    release(storage,layout); gd_destroy(model);
)
TEST(focused_relationship_capacity,
    GD_Model *model=model_create(); GD_Focus focus; int i;
    GD_Layout *layout=allocate(storage,sizeof(*layout));
    TEST_ASSERT(model && layout);
    model->record_count=GD_MAX_RECORDS; model->edge_count=GD_MAX_EDGES;
    for (i=0;i<GD_MAX_RECORDS;++i) model->records[i].reference="same visible label; distinct record indices";
    for (i=0;i<GD_MAX_EDGES;++i) model->edges[i]=(GD_Edge){0,i%GD_MAX_RECORDS,GD_MECHANICS,"reference",i};
    TEST_ASSERT(gd_focus_reset(model,&focus,0) && gd_focus_layout(model,&focus,layout,GD_SORT_PARTNERS));
    TEST_ASSERT(layout->node_count==GD_MAX_RECORDS && layout->edge_count==GD_MAX_EDGES && !focus.boundary_count);
    for (i=0;i<layout->edge_count;++i) TEST_ASSERT(layout->edges[i].key==i);
    gd_layout_step(layout,-1);
    for (i=0;i<layout->node_count;++i) TEST_ASSERT(isfinite(layout->nodes[i].x) && isfinite(layout->nodes[i].y));
    release(storage,layout); gd_destroy(model);
)
TEST(diagnostic_projection,
    GD_Model *model=model_create(); GD_DiagnosticItem item;
    GD_Finding findings[2]={{"skill.example","Investigate declared prerequisites","prerequisites","atoms/b",0,7},{"mda.question","Unbound question",NULL,NULL,-1,0}};
    GD_DiagnosticSet set;
    TEST_ASSERT(model);
    model->record_count=2; model->records[0].reference="atoms/a"; model->records[0].source_path="design/a.md";
    model->records[1].reference="atoms/b"; model->records[1].source_path="design/b.md";
    gd_error(model,"operation.example","Broken operation","design/b.md",9);
    set=(GD_DiagnosticSet){model,findings,2,3,NULL};
    TEST_ASSERT(gd_diagnostic_count(&set)==3 && gd_diagnostic_item(&set,0,&item));
    TEST_ASSERT(item.kind==GD_DIAGNOSTIC_FINDING && item.record==0 && item.line==7 && !strcmp(item.path,"design/a.md"));
    TEST_ASSERT(gd_diagnostic_matches(&item,GD_DIAGNOSTIC_ALL,"PREREQUISITES",-1));
    TEST_ASSERT(gd_diagnostic_matches(&item,GD_DIAGNOSTIC_FINDING,"ATOMS/B",0));
    TEST_ASSERT(gd_diagnostic_matches(&item,GD_DIAGNOSTIC_ALL,"DESIGN/A.MD",-1));
    TEST_ASSERT(!gd_diagnostic_matches(&item,GD_DIAGNOSTIC_ERROR,"",-1) && !gd_diagnostic_matches(&item,GD_DIAGNOSTIC_ALL,"",-2));
    TEST_ASSERT(!gd_diagnostic_matches(&item,GD_DIAGNOSTIC_ALL,"",1));
    TEST_ASSERT(gd_diagnostic_item(&set,1,&item) && item.record==-1 && !item.path && !item.record_reference);
    TEST_ASSERT(gd_diagnostic_item(&set,2,&item) && item.kind==GD_DIAGNOSTIC_ERROR && item.record==1 && item.line==9);
    model->records[0].source_path="design/b.md";
    TEST_ASSERT(gd_diagnostic_item(&set,2,&item) && item.record==-1);
    TEST_ASSERT(!gd_diagnostic_item(&set,-1,&item) && !gd_diagnostic_item(&set,3,&item));
    set.finding_count=0; model->error_count=0;
    TEST_ASSERT(!gd_diagnostic_count(&set));
    gd_destroy(model);
)
TEST(diagnostic_retention,
    GD_Model *model=model_create(); GD_DiagnosticErrors *snapshot; GD_DiagnosticItem item; GD_DiagnosticSet set;
    GD_Error errors[GD_MAX_ERRORS+1]; char message[4100], path[64]="design/rejected.md", code[140]; int i;
    TEST_ASSERT(model);
    memset(message,'x',4094); memcpy(message+4094,"\xf0\x9f\x98\x80",5);
    memset(code,'a',sizeof(code)-1); code[sizeof(code)-1]=0;
    for (i=0;i<GD_MAX_ERRORS+1;++i) errors[i]=(GD_Error){code,message,path,42};
    snapshot=gd_diagnostics_capture(model->allocator,errors,GD_MAX_ERRORS+1,2);
    TEST_ASSERT(snapshot && snapshot->count==GD_MAX_ERRORS && snapshot->omitted==3 && snapshot->truncated[0]);
    TEST_ASSERT(strlen(snapshot->errors[0].code)==128 && strlen(snapshot->errors[0].message)==4094);
    TEST_ASSERT(gd_valid_utf8((const unsigned char *)snapshot->errors[0].message,4094));
    message[0]='z'; code[0]='b'; path[0]='X';
    TEST_ASSERT(snapshot->errors[0].message[0]=='x' && snapshot->errors[0].code[0]=='a' && !strcmp(snapshot->errors[0].path,"design/rejected.md"));
    model->record_count=1; model->records[0].reference="atoms/old"; model->records[0].source_path="design/rejected.md";
    set=(GD_DiagnosticSet){model,NULL,0,0,snapshot};
    TEST_ASSERT(gd_diagnostic_count(&set)==GD_MAX_ERRORS && gd_diagnostic_item(&set,0,&item) && item.kind==GD_DIAGNOSTIC_RELOAD && item.record==0 && item.truncated && item.line==42);
    for (i=0;i<GD_MAX_ERRORS;++i) {
        model->findings[i]=(GD_Finding){"question","Retained domain question",NULL,NULL,0,0};
        gd_error(model,"operation","Retained error",NULL,0);
    }
    set.findings=model->findings; set.finding_count=GD_MAX_ERRORS;
    TEST_ASSERT(gd_diagnostic_count(&set)==3*GD_MAX_ERRORS && gd_diagnostic_item(&set,3*GD_MAX_ERRORS-1,&item) && item.kind==GD_DIAGNOSTIC_RELOAD);
    TEST_ASSERT(gd_diagnostic_item(&set,GD_MAX_ERRORS,&item) && item.kind==GD_DIAGNOSTIC_ERROR);
    set.findings=NULL; set.finding_count=0; model->error_count=0;
    gd_diagnostics_destroy(snapshot);
    {
        char long_path[4100], prefix[GD_MAX_PATH];
        memset(long_path,'p',sizeof(long_path)-1); long_path[sizeof(long_path)-1]=0;
        memcpy(prefix,long_path,GD_MAX_PATH-1); prefix[GD_MAX_PATH-1]=0;
        errors[0]=(GD_Error){"input.path","Path too long",long_path,0};
        snapshot=gd_diagnostics_capture(model->allocator,errors,1,0);
        TEST_ASSERT(snapshot && snapshot->path_truncated[0]);
        model->records[0].source_path=prefix; set.rejected=snapshot;
        TEST_ASSERT(gd_diagnostic_item(&set,0,&item) && item.record==-1); /* Shortened path is not an identity. */
        gd_diagnostics_destroy(snapshot);
    }
    snapshot=gd_diagnostics_capture(model->allocator,NULL,0,INT_MAX);
    TEST_ASSERT(snapshot && snapshot->omitted==INT_MAX && snapshot->count==1 && !strcmp(snapshot->errors[0].code,"view.reload") && !snapshot->errors[0].path && !snapshot->truncated[0]);
    gd_diagnostics_destroy(snapshot);
    TEST_ASSERT(!gd_diagnostics_capture(model->allocator,errors,GD_MAX_ERRORS+1,INT_MAX));
    {
        RouteFailure failure; GD_Allocator allocator=model->allocator;
        failure.parent=allocator; failure.remaining=0; allocator.context=&failure; allocator.allocate=route_allocate; allocator.release=route_release;
        TEST_ASSERT(!gd_diagnostics_capture(allocator,errors,1,0));
    }
    gd_destroy(model);
)
TEST(rules_allocation_failure,
    GD_Model *model = model_create();
    RouteFailure failure;
    GD_Allocator allocator;
    TEST_ASSERT(model);
    allocator = model->allocator;
    failure.parent = allocator; failure.remaining = 0;
    model->allocator.context = &failure; model->allocator.allocate = route_allocate; model->allocator.release = route_release;
    TEST_ASSERT(!gd_read_rules(model) && has_error(model,"rules.memory"));
    model->allocator = allocator; gd_destroy(model);
)
int main(int argc, char **argv) {
    int failed = 0;
    if (argc != 2 || strlen(argv[1]) >= sizeof(root)) { fputs("Usage: test-core NEW-FIXTURE-DIRECTORY\n", stderr); return 2; }
    strcpy(root, argv[1]);
    { char *p; for (p = root; *p; ++p) if (*p == '\\') *p = '/'; }
    if (!file_directory_create(root, 1)) { fputs("Fixture directory must be new\n", stderr); return 2; }
    storage = free_list_from_buffer(backing, sizeof(backing));
    if (!storage) return 2;
    failed += test_rule_effect_bindings();
    failed += test_rule_axis_definitions(); failed += test_rule_axis_errors(); failed += test_rule_axis_references(); failed += test_rule_axis_incomplete(); failed += test_rule_axis_limits();
    failed += test_rules_declarations(); failed += test_rules_fence_scope(); failed += test_rules_invalid_records(); failed += test_rules_limits(); failed += test_rules_allocation_failure();
    failed += test_empty(); failed += test_matrix(); failed += test_unsafe_input(); failed += test_ambiguous_endpoints();
    failed += test_text_types(); failed += test_missing_endpoint();
    failed += test_input_limits();
    failed += test_component_chain(); failed += test_layout_modes(); failed += test_layout_capacity();
    failed += test_skill_chain(); failed += test_skill_incomplete(); failed += test_skill_rank_types(); failed += test_skill_capacity();
    failed += test_draft_ranks(); failed += test_draft_connections(); failed += test_draft_limits(); failed += test_draft_report();
    failed += test_routes_semantics(); failed += test_routes_types_unknown(); failed += test_routes_walk_oracle(); failed += test_routes_limits();
    failed += test_mda_traces(); failed += test_mda_resolution(); failed += test_mda_types(); failed += test_mda_oracle(); failed += test_mda_limits();
    failed += test_control_units_numbers(); failed += test_control_formulas(); failed += test_control_reference_syntax(); failed += test_control_errors();
    failed += test_control_loading(); failed += test_control_overrides_scope(); failed += test_control_selection(); failed += test_control_limits();
    failed += test_control_import_roundtrip(); failed += test_control_import_legacy_dependent(); failed += test_control_import_strings(); failed += test_control_import_failures();
    failed += test_skill_progression_modes(); failed += test_route_layout_capacity();
    failed += test_focused_relationships(); failed += test_focused_relationship_capacity();
    failed += test_diagnostic_projection(); failed += test_diagnostic_retention();
    printf("native domain/layout/draft/route/MDA/control/rule checks: %d/56 passed\n", 56-failed);
    return failed ? 1 : 0;
}
