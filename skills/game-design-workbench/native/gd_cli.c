#if !defined(_WIN32) && !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif
#include "gd.h"
#include "gd_rule_analysis.h"
#include "gd_resource_graph.h"
#include "gd_skill_unlocks.h"
#include "gd_explorer.h"
#include "gd_matrix_grid.h"
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#include <io.h>
#ifndef WC_ERR_INVALID_CHARS
#define WC_ERR_INVALID_CHARS 0x00000080
#endif
#else
#include <unistd.h>
#include <signal.h>
#endif

static void *system_allocate(void *context, size_t size) { (void)context; return malloc(size); }
static void system_release(void *context, void *memory) { (void)context; free(memory); }
static void help(void) {
    puts("Game Design Workbench — native domain tools\n"
         "Usage: gd matrix|connectivity|skill-atoms|scaffolding|mda|controls|rules|resources [options]\n"
         "       gd view matrix|skill-atoms|mda|resources|explorer [options]\n\n"
         "Unified explorer: gd view explorer with explicit collection bindings below.\n"
         "  One window; six projections; unavailable views explain missing roles.\n"
         "  --mode skills|challenges|relationships|matrix|mda|resources (default first available)\n"
         "  --layout grid|graph|columns             Explorer: requires --mode matrix\n"
         "  Supports --frames, --screenshot, --output and error-output format flags.\n"
         "  Standalone view commands also available. No automatic folder discovery.\n\n"
         "Resources: gd resources uses explicit rule bindings; retained direct declarations only.\n"
         "  --fields nodes,relations,routes,findings (JSON). Directed routes/composed outcomes unavailable.\n"
         "Matrix/connectivity collections (exactly one of each):\n"
         "  --nouns ALIAS=FOLDER\n  --verbs ALIAS=FOLDER\n  --interactions ALIAS=FOLDER\n"
         "Skill operations require: --skill-atoms ALIAS=FOLDER\n"
         "  --known ALIAS/NAME     Hypothetically known skill; repeatable, max 64\n"
         "  --from ALIAS/NAME      Skill root for prerequisite/unlock chains and comparison evidence\n"
         "  view skill-atoms also accepts --challenges ALIAS=FOLDER for switchable exploration\n"
         "  --mode skills|challenges|relationships  Initial explorer mode; default skills\n"
         "Scaffolding also requires: --challenges ALIAS=FOLDER\n"
         "Controls require --control-vars ALIAS=FOLDER; inspection by default\n"
         "  gd controls export --as json|csv|ini [options]  Raw resolved values on stdout\n"
         "  gd controls import --input FILE [--input-format json|csv|ini] [options]\n"
         "                         Read-only update plan; format defaults json; no baseline file\n"
         "  --status STATUS        Controls status filter; repeatable, or all alone\n"
         "  --include NAME         Only these control names; repeatable\n"
         "  --exclude NAME         Omit these control names; repeatable; exclusions win\n"
         "                         Export defaults accepted; inspection all. Explicit names\n"
         "                         bypass implicit status default, not explicit --status.\n"
         "  --set ALIAS/NAME=VALUE  Temporary typed override; repeatable, never source edits\n"
         "                         Enum VALUE is raw text; booleans true/false; numeric unit from definition\n"
         "Resource graph preview: gd view resources with the same rule inputs.\n"
         "  Preview is incomplete: no routes or composed outcome expansion.\n"
         "Rules require --interactions and/or --mechanics ALIAS=FOLDER\n"
         "  Optional --axes, --nouns, --verbs, --resources, --control-vars, --related; explicit applicability\n"
         "  Shared typed state/quantity logic; declaration inspection by default, no fabricated scene\n"
         "  --analyze              Compile explicit calls and compare bounded declared enum branches\n"
         "  --case ALIAS/NAME#CASE  Inspect conditional proposals under supplied assumptions\n"
         "  --assume LOCAL:ALIAS/AXIS=VALUE  Current value in declared axis unit; repeatable\n"
         "  --context \"A ALIAS/AXIS.STATE B\"  Explicit ordered relation assumption; repeatable\n"
         "  --context-not TRIPLE   Explicit false relation; reverse order never implies false\n"
         "MDA requires --mechanics, --dynamics, --aesthetics ALIAS=FOLDER; optional --systems\n"
         "  --from ALIAS/NAME       MDA/control root (default all), or Relationships root\n"
         "                         Relationships defaults first loaded skill, then other record\n"
         "  --direction forward|backward|both  MDA traversal; default both\n"
         "                         Forward: toward aesthetics; backward: authored basis\n"
         "                         Both: connected associations; not causal claims\n"
         "Optional:\n  --related ALIAS=FOLDER  Load linked rule/context text; repeatable\n"
         "  --json                 Structured output\n  --format json|text     Explicit output format\n"
         "  --fields KEY,...       Top-level data selection; JSON only\n"
         "                         matrix: nouns,verbs,cases,cells,components\n"
         "                         connectivity: nouns,verbs\n"
         "                         skill-atoms: skills,links,components,findings,exploration\n"
         "                         scaffolding: challenges,skills,checks,witnesses,findings\n"
         "                         mda: nodes,links,paths,gaps,findings; controls: controls\n"
         "                         rules: records,cases,axes,actors,findings,analysis\n"
         "                         controls import: changes,derived,items (flags/errors retained)\n"
         "                         MDA view: columns (default) or graph; source read-only\n"
         "  --layout table|graph|columns  View layout; graph settles dynamically\n"
         "  --layout prerequisites|rank  Skill view; prerequisite ordering is default\n"
         "  --direction down|right Skill view axis; down is default\n"
         "  --output FILE.json     Skill view change report instead of stdout; new file only\n"
         "                         Finish/close returns proposed edits; never writes Markdown\n"
         "  --show-other-pairs      Matrix overlay; does not merge active groups\n"
         "  --frames N             Close view after N frames (bounded smoke test)\n"
         "  --screenshot FILE.png  View capture on final bounded frame\n"
         "  --help                 Show help\n  --version              Show version\n\n"
         "No path defaults, registry, schema engine, generic queries, or source writes.\n"
         "Immediate non-hidden .md files only. name or filename identifies a record.\n"
         "Connectivity counts distinct partners from accepted, defined cases.\n"
         "Scaffolding checks prior introduction/practice on declared entry/next routes,\n"
         "including revisits; not evidence of learning or game-state reachability.\n"
         "Pipes/GD_AGENT=1/CI default to JSON; explicit --format takes precedence.\n"
         "Exit 0: operation completed (including empty); 2: input/runtime/load failure.\n"
         "Operation errors include partial data with coverage.complete=false; not schema validation.");
}
static int fields_valid(const char *fields, int operation) {
    int connectivity = operation == 1;
    const char *p = fields;
    if (!p || !*p) return 0;
    while (*p) {
        const char *end = strchr(p, ','); size_t n = end ? (size_t)(end-p) : strlen(p);
        if (operation == 8) {
            if (!((n==5 && !memcmp(p,"nodes",n)) || (n==6 && !memcmp(p,"routes",n)) || (n==9 && !memcmp(p,"relations",n)) || (n==8 && !memcmp(p,"findings",n)))) return 0;
        } else if (operation == 7) {
            if (!((n==7 && !memcmp(p,"records",n)) || (n==5 && !memcmp(p,"cases",n)) ||
                  (n==4 && !memcmp(p,"axes",n)) || (n==6 && !memcmp(p,"actors",n)) || (n==8 && (!memcmp(p,"findings",n) || !memcmp(p,"analysis",n))))) return 0;
        } else if (operation == 6) {
            if (!((n==7 && !memcmp(p,"changes",n)) || (n==7 && !memcmp(p,"derived",n)) || (n==5 && !memcmp(p,"items",n)))) return 0;
        } else if (operation == 5) {
            if (n != 8 || memcmp(p,"controls",n)) return 0;
        } else if (operation == 4) {
            if (!((n == 5 && !memcmp(p,"nodes",n)) || (n == 5 && !memcmp(p,"links",n)) ||
                  (n == 5 && !memcmp(p,"paths",n)) || (n == 4 && !memcmp(p,"gaps",n)) || (n == 8 && !memcmp(p,"findings",n)))) return 0;
        } else if (operation == 3) {
            if (!((n == 10 && !memcmp(p,"challenges",n)) || (n == 6 && !memcmp(p,"skills",n)) ||
                  (n == 6 && !memcmp(p,"checks",n)) || (n == 9 && !memcmp(p,"witnesses",n)) || (n == 8 && !memcmp(p,"findings",n)))) return 0;
        } else if (operation == 2) {
            if (!((n == 6 && !memcmp(p, "skills", n)) || (n == 5 && !memcmp(p, "links", n)) ||
                  (n == 10 && !memcmp(p, "components", n)) || (n == 8 && !memcmp(p, "findings", n)) || (n == 11 && !memcmp(p,"exploration",n)))) return 0;
        } else if (!((n == 5 && !memcmp(p, "nouns", n)) || (n == 5 && !memcmp(p, "verbs", n)) ||
              (!connectivity && n == 5 && !memcmp(p, "cases", n)) || (!connectivity && n == 5 && !memcmp(p, "cells", n)) ||
              (!connectivity && n == 10 && !memcmp(p, "components", n)))) return 0;
        if (!end) return 1;
        p = end + 1; if (!*p) return 0;
    }
    return 1;
}
static int absolute_folder(const char *path, char *result) {
#ifdef _WIN32
    wchar_t input[GD_MAX_PATH], absolute[GD_MAX_PATH];
    DWORD length;
    if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path, -1, input, GD_MAX_PATH)) return 0;
    length = GetFullPathNameW(input, GD_MAX_PATH, absolute, NULL);
    if (!length || length >= GD_MAX_PATH || !WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, absolute, -1,
                                                                result, GD_MAX_PATH, NULL, NULL)) return 0;
#else
    if (path[0] == '/') {
        if (strlen(path) >= GD_MAX_PATH) return 0;
        strcpy(result, path);
    } else {
        char current[GD_MAX_PATH];
        if (!getcwd(current, sizeof(current)) || snprintf(result, GD_MAX_PATH, "%s/%s", current, path) >= GD_MAX_PATH) return 0;
    }
#endif
    {
        char *p;
        size_t length;
        for (p = result; *p; ++p) if (*p == '\\') *p = '/';
        length = strlen(result);
        while (length > 3 && result[length-1] == '/') result[--length] = 0;
    }
    return 1;
}
static int application(int argc, char **argv) {
    GD_Allocator allocator = { NULL, system_allocate, system_release };
    GD_Collection collections[GD_MAX_COLLECTIONS];
    char aliases[GD_MAX_COLLECTIONS][129], folders[GD_MAX_COLLECTIONS][GD_MAX_PATH];
    const char *error = NULL, *fields = NULL, *screenshot = NULL, *output_path = NULL;
    GD_Model *model;
    GD_Routes *routes = NULL;
    GD_Mda *mda = NULL;
    GD_Controls *controls = NULL;
    GD_ControlImport *import_plan = NULL;
    GD_Rules *rules = NULL;
    GD_RuleAnalysis *rule_analysis=NULL;
    GD_ResourceGraph *resource_graph=NULL;
    GD_SkillUnlocks *skill_query=NULL;
    const char *known_names[64]; int known_count=0;
    GD_RuleScenario scenario={0};
    const char *case_name=NULL, *assumptions[64], *context_assumptions[64];
    GD_RuleTruth context_truths[64];
    int analyze=0, assumption_count=0, context_assumption_count=0;
    const char *input_path = NULL, *input_format = "json";
    int import_option = 0;
    GD_ControlOverride overrides[64]; char override_names[64][258];
    int override_count = 0;
    GD_ControlFilter control_filter = {0};
    const char *export_format = NULL;
    int as_supplied = 0, control_command;
    GD_MdaDirection mda_direction = GD_MDA_BOTH;
    const char *from = NULL;
    int route_command, mda_command, rule_command;
    int i, count = 0, roles = 0, command = 0, first = 2, frames = 0, explicit_format = -1;
    GD_ViewLayout layout = GD_VIEW_TABLE;
    GD_ExplorerMode explorer_mode = GD_EXPLORE_SKILLS;
    int explorer_mode_supplied = 0, unified_mode=-1, matrix_layout=-1;
    int layout_supplied = 0, other_pairs = 0, horizontal = 0, direction_supplied = 0;
    int skill_command, view_command;
    int json, status = 0;
#ifdef _WIN32
    json = !_isatty(_fileno(stdout));
#else
    json = !isatty(fileno(stdout));
#endif
    if (getenv("GD_AGENT") && !strcmp(getenv("GD_AGENT"), "1")) json = 1;
    if (getenv("CI") && *getenv("CI")) json = 1;
    for (i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--help")) { help(); return 0; }
        if (!strcmp(argv[i], "--version")) { puts("gd native 0.1 (output schema 1)"); return 0; }
    }
    if (argc < 2) error = "Supply matrix, connectivity, skill-atoms, scaffolding, mda, controls, rules, resources, or view; see --help";
    else if (!strcmp(argv[1], "connectivity")) command = 1;
    else if (!strcmp(argv[1], "view") && argc > 2 && !strcmp(argv[2], "matrix")) { command = 2; first = 3; }
    else if (!strcmp(argv[1], "skill-atoms")) command = 3;
    else if (!strcmp(argv[1], "scaffolding")) command = 5;
    else if (!strcmp(argv[1], "mda")) command = 6;
    else if (!strcmp(argv[1], "rules")) command = 11;
    else if (!strcmp(argv[1], "resources")) command = 14;
    else if (!strcmp(argv[1], "controls")) {
        command = 8;
        if (argc > 2 && !strcmp(argv[2],"export")) { command = 9; first = 3; export_format = "json"; }
        else if (argc > 2 && !strcmp(argv[2],"import")) { command = 10; first = 3; }
    }
    else if (!strcmp(argv[1], "view") && argc > 2 && !strcmp(argv[2], "explorer")) { command = 13; first = 3; }
    else if (!strcmp(argv[1], "view") && argc > 2 && !strcmp(argv[2], "resources")) { command = 12; first = 3; }
    else if (!strcmp(argv[1], "view") && argc > 2 && !strcmp(argv[2], "mda")) { command = 7; first = 3; layout = GD_VIEW_COLUMNS; }
    else if (!strcmp(argv[1], "view") && argc > 2 && !strcmp(argv[2], "skill-atoms")) { command = 4; first = 3; layout = GD_VIEW_PREREQUISITES; }
    else if (strcmp(argv[1], "matrix")) error = "Unsupported command; no generic inspect/validate/query commands exist";
    control_filter.root = -1; control_filter.accepted_default = command == 9 || command == 10;
    control_command = command >= 8 && command <= 10;
    route_command = command == 5;
    rule_command = command == 11 || command == 12 || command == 14;
    mda_command = command == 6 || command == 7;
    skill_command = command >= 3 && command <= 5; view_command = command == 2 || command == 4 || command == 7 || command == 12;
    for (i = first; i < argc && !error; ++i) {
        GD_Role role = GD_RELATED;
        int mapping = 1;
        if (!strcmp(argv[i], "--nouns")) role = GD_NOUN;
        else if (!strcmp(argv[i], "--verbs")) role = GD_VERB;
        else if (!strcmp(argv[i], "--interactions")) role = GD_CASE;
        else if (!strcmp(argv[i], "--skill-atoms")) role = GD_SKILL;
        else if (!strcmp(argv[i], "--challenges")) role = GD_CHALLENGE;
        else if (!strcmp(argv[i], "--mechanics")) role = GD_MDA_MECHANIC;
        else if (!strcmp(argv[i], "--systems")) role = GD_MDA_SYSTEM;
        else if (!strcmp(argv[i], "--dynamics")) role = GD_MDA_DYNAMIC;
        else if (!strcmp(argv[i], "--aesthetics")) role = GD_MDA_AESTHETIC;
        else if (!strcmp(argv[i], "--control-vars")) role = GD_CONTROL;
        else if (!strcmp(argv[i], "--axes")) role = GD_AXIS;
        else if (!strcmp(argv[i], "--resources")) role = GD_RESOURCE;
        else if (strcmp(argv[i], "--related")) mapping = 0;
        if (mapping) {
            char *equals; size_t length;
            if (++i == argc) { error = "Collection option requires ALIAS=FOLDER"; break; }
            equals = strchr(argv[i], '=');
            if (!equals || equals == argv[i] || !equals[1] || (length = (size_t)(equals - argv[i])) > 128) { error = "Collection option requires ALIAS=FOLDER (alias at most 128 bytes)"; break; }
            if (count == GD_MAX_COLLECTIONS) { error = "At most 16 explicit collections supported"; break; }
            if (role != GD_RELATED && (roles & (1 << role))) { error = "Supply each domain role exactly once"; break; }
            memcpy(aliases[count], argv[i], length); aliases[count][length] = 0;
            if (!absolute_folder(equals + 1, folders[count])) { error = "Collection path is invalid or exceeds 4095 UTF-8 bytes"; break; }
            collections[count].alias = aliases[count]; collections[count].directory = folders[count];
            collections[count].role = rule_command && (role == GD_CASE || role == GD_MDA_MECHANIC) ? GD_RULE :
                (command == 12 || command == 14) && role == GD_MDA_SYSTEM ? GD_RELATED:role;
            if (role != GD_RELATED) roles |= 1 << role;
            ++count;
        } else if (!strcmp(argv[i], "--status") || !strcmp(argv[i], "--include") || !strcmp(argv[i], "--exclude")) {
            const char *option = argv[i];
            int *length = !strcmp(option,"--status") ? &control_filter.status_count : !strcmp(option,"--include") ? &control_filter.include_count : &control_filter.exclude_count;
            const char **items = !strcmp(option,"--status") ? control_filter.statuses : !strcmp(option,"--include") ? control_filter.include : control_filter.exclude;
            if (++i == argc || !argv[i][0] || !strncmp(argv[i],"--",2) || *length == GD_MAX_CONTROL_FILTERS) { error = "Control filters need nonempty values, at most 64 per kind"; break; }
            items[(*length)++] = argv[i];
        } else if (!strcmp(argv[i], "--set")) {
            const char *equals; size_t length;
            if (++i == argc || !(equals = strchr(argv[i],'=')) || equals == argv[i] || (length = (size_t)(equals-argv[i])) > 257 || override_count == 64) { error = "--set requires ALIAS/NAME=VALUE, at most 64 overrides"; break; }
            memcpy(override_names[override_count],argv[i],length); override_names[override_count][length] = 0;
            overrides[override_count].reference = override_names[override_count]; overrides[override_count++].value = equals+1;
        } else if (!strcmp(argv[i], "--input")) {
            import_option = 1;
            if (++i == argc || !argv[i][0] || strlen(argv[i]) >= GD_MAX_PATH) { error = "--input requires a UTF-8 file path within 4095 bytes"; break; }
            input_path = argv[i];
        } else if (!strcmp(argv[i], "--input-format")) {
            import_option = 1;
            if (++i == argc || (strcmp(argv[i],"json") && strcmp(argv[i],"csv") && strcmp(argv[i],"ini"))) { error = "--input-format requires json, csv, or ini"; break; }
            input_format = argv[i];
        } else if (!strcmp(argv[i], "--as")) {
            as_supplied = 1;
            if (++i == argc || (strcmp(argv[i],"json") && strcmp(argv[i],"csv") && strcmp(argv[i],"ini"))) { error = "--as requires json, csv, or ini"; break; }
            export_format = argv[i];
        } else if (!strcmp(argv[i],"--analyze")) {
            analyze=1;
        } else if (!strcmp(argv[i],"--case")) {
            if (++i==argc || case_name || !argv[i][0] || strlen(argv[i])>386) { error="--case needs one ALIAS/NAME#CASE, at most 386 bytes"; break; }
            case_name=argv[i];
        } else if (!strcmp(argv[i],"--assume")) {
            if (++i==argc || !argv[i][0] || strlen(argv[i])>GD_MAX_RULE_LINE || assumption_count==64) { error="--assume needs LOCAL:ALIAS/AXIS=VALUE, at most 64 bounded inputs"; break; }
            assumptions[assumption_count++]=argv[i];
        } else if (!strcmp(argv[i],"--context") || !strcmp(argv[i],"--context-not")) {
            GD_RuleTruth truth=!strcmp(argv[i],"--context") ? GD_RULE_TRUTH_TRUE:GD_RULE_TRUTH_FALSE;
            if (++i==argc || !argv[i][0] || strlen(argv[i])>GD_MAX_RULE_LINE || context_assumption_count==64) { error="Context option needs one bounded ordered triple, at most 64 inputs"; break; }
            context_assumptions[context_assumption_count]=argv[i]; context_truths[context_assumption_count++]=truth;
        } else if (!strcmp(argv[i],"--known")) {
            if (++i==argc || !argv[i][0] || !strncmp(argv[i],"--",2) || known_count==64) { error="--known requires one qualified skill reference, at most 64 assumptions"; break; }
            known_names[known_count++]=argv[i];
        } else if (!strcmp(argv[i], "--json")) {
            if (explicit_format == 0) error = "Conflicting output formats";
            explicit_format = 1; json = 1;
        } else if (!strcmp(argv[i], "--format")) {
            int value;
            if (++i == argc) { error = "--format requires json or text"; break; }
            value = !strcmp(argv[i], "json") ? 1 : !strcmp(argv[i], "text") ? 0 : -1;
            if (value < 0 || (explicit_format >= 0 && explicit_format != value)) error = "Unsupported or conflicting output formats";
            else { explicit_format = value; json = value; }
        } else if (!strcmp(argv[i], "--output")) {
            if (++i == argc) { error = "--output requires a new .json report path"; break; }
            output_path = argv[i];
        } else if (!strcmp(argv[i], "--fields")) {
            if (++i == argc) { error = "--fields requires comma-separated data keys"; break; }
            fields = argv[i];
        } else if (!strcmp(argv[i], "--mode")) {
            explorer_mode_supplied = 1;
            if (++i == argc) { error="--mode requires a supported projection name"; break; }
            if (command==13) {
                const char *names[]={"skills","challenges","relationships","matrix","mda","resources"}; int m;
                unified_mode=-1;
                for (m=0;m<GD_EXPLORER_VIEWS;++m) if (!strcmp(argv[i],names[m])) unified_mode=m;
                if (unified_mode<0) { error="Explorer --mode requires skills, challenges, relationships, matrix, mda or resources"; break; }
            } else {
                if (strcmp(argv[i],"skills") && strcmp(argv[i],"challenges") && strcmp(argv[i],"relationships")) { error="--mode requires skills, challenges, or relationships"; break; }
                explorer_mode = !strcmp(argv[i],"relationships") ? GD_EXPLORE_RELATIONSHIPS : !strcmp(argv[i],"challenges") ? GD_EXPLORE_CHALLENGES : GD_EXPLORE_SKILLS;
            }
        } else if (!strcmp(argv[i], "--layout")) {
            if (++i == argc) { error = command==13 ? "Explorer --layout requires grid, graph or columns with --mode matrix":"--layout requires table, graph, columns, prerequisites, or rank"; break; }
            layout_supplied = 1;
            if (command==13) {
                matrix_layout=!strcmp(argv[i],"grid") ? GD_MATRIX_GRID_CHOICE:!strcmp(argv[i],"graph") ? GD_LAYOUT_FORCE:!strcmp(argv[i],"columns") ? GD_LAYOUT_COLUMNS:-1;
                if (matrix_layout<0) error="Explorer --layout requires grid, graph or columns with --mode matrix";
            }
            else if (!strcmp(argv[i], "table")) layout = GD_VIEW_TABLE;
            else if (!strcmp(argv[i], "graph")) layout = GD_VIEW_GRAPH;
            else if (!strcmp(argv[i], "columns")) layout = GD_VIEW_COLUMNS;
            else if (!strcmp(argv[i], "prerequisites")) layout = GD_VIEW_PREREQUISITES;
            else if (!strcmp(argv[i], "rank")) layout = GD_VIEW_RANK;
            else error = "--layout requires table, graph, columns, prerequisites, or rank";
        } else if (!strcmp(argv[i], "--from")) {
            if (++i == argc || !argv[i][0] || strlen(argv[i]) > 257) { error = "--from requires ALIAS/NAME (up to 257 bytes)"; break; }
            from = argv[i];
        } else if (!strcmp(argv[i], "--direction")) {
            direction_supplied = 1;
            if (++i == argc) { error = mda_command ? "--direction requires forward, backward, or both" : "--direction requires down or right"; break; }
            if (mda_command) {
                if (!strcmp(argv[i],"forward")) mda_direction = GD_MDA_FORWARD;
                else if (!strcmp(argv[i],"backward")) mda_direction = GD_MDA_BACKWARD;
                else if (!strcmp(argv[i],"both")) mda_direction = GD_MDA_BOTH;
                else error = "--direction requires forward, backward, or both";
            }
            else if (!strcmp(argv[i], "down")) horizontal = 0;
            else if (!strcmp(argv[i], "right")) horizontal = 1;
            else error = "--direction requires down or right";
        } else if (!strcmp(argv[i], "--show-other-pairs")) {
            other_pairs = 1;
        } else if (!strcmp(argv[i], "--frames")) {
            char *end; long value;
            if (++i == argc) { error = "--frames requires 1..600"; break; }
            value = strtol(argv[i], &end, 10);
            if (*end || value < 1 || value > 600) error = "--frames requires 1..600";
            else frames = (int)value;
        } else if (!strcmp(argv[i], "--screenshot")) {
            if (++i == argc) { error = "--screenshot requires a PNG path"; break; }
            screenshot = argv[i];
        } else error = "Unknown option; see --help";
    }
    if (command==13) {
        if (!error && (!count || fields || from || known_count || analyze || case_name || assumption_count || context_assumption_count ||
            import_option || as_supplied || override_count || control_filter.status_count || control_filter.include_count || control_filter.exclude_count ||
            (layout_supplied && unified_mode!=GD_EXPLORER_MATRIX) || direction_supplied || other_pairs))
            error="view explorer requires explicit collections; supports --mode, matrix-only --layout grid|graph|columns, --frames, --screenshot, --output and error-output format flags";
        if (!error && screenshot && (!frames || strlen(screenshot)<4 || strcmp(screenshot+strlen(screenshot)-4,".png")))
            error="--screenshot requires bounded --frames and a .png path";
        model=gd_create(allocator);
        if (!model) { fputs("gd: cannot allocate diagnostic snapshot\n",stderr); return 2; }
        if (error) gd_error(model,"cli.arguments",error,NULL,0);
        else {
#ifdef GD_WITH_GRAPH
            status=gd_explorer_view(model,collections,count,unified_mode,matrix_layout,frames,screenshot,output_path);
#else
            gd_error(model,"view.graph_unavailable","Unified explorer needs a -Graph build with prepared raylib and gd-ui.dll",NULL,0);
#endif
        }
        if (model->error_count) {
            gd_write_result(stdout,model,0,json,NULL);
            for (i=0;i<model->error_count;++i) fprintf(stderr,"gd: %s: %s | %s:%d\n",model->errors[i].code,model->errors[i].message,model->errors[i].path ? model->errors[i].path:"runtime",model->errors[i].line);
            status=2;
        }
        gd_destroy(model); return status;
    }
    if (!error && mda_command) {
        int required = (1 << GD_MDA_MECHANIC) | (1 << GD_MDA_DYNAMIC) | (1 << GD_MDA_AESTHETIC);
        if (roles != required && roles != (required | (1 << GD_MDA_SYSTEM))) error = "MDA requires --mechanics, --dynamics, --aesthetics ALIAS=FOLDER; optional --systems and --related";
    }
    if (!error && rule_command) {
        int roots = (1 << GD_CASE) | (1 << GD_MDA_MECHANIC);
        int permitted = roots | (1 << GD_NOUN) | (1 << GD_VERB) | (1 << GD_CONTROL) | (1 << GD_AXIS) | (1 << GD_RESOURCE) | ((command == 12 || command == 14) ? (1 << GD_MDA_SYSTEM):0);
        if (!(roles & roots) || (roles & ~permitted)) error = "Rules require --interactions and/or --mechanics, optional --axes, --nouns, --verbs, --resources, --control-vars and --related";
    }
    if (!error && (command == 12 || command == 14) && (analyze || case_name || assumption_count || context_assumption_count)) error="Resource declaration inspection does not accept scenario or comparison options; use rules";
    if (!error && !rule_command && (analyze || case_name || assumption_count || context_assumption_count)) error="--analyze, --case, --assume and context options apply only to rules";
    if (!error && (assumption_count || context_assumption_count) && !case_name) error="Rule assumptions require an explicit --case; no implicit scene or case selection";
    if (!error && command==11 && fields && strstr(fields,"analysis") && !analyze && !case_name) error="analysis field requires --analyze and/or --case";
    if (!error && control_command && roles != (1 << GD_CONTROL)) error = "Controls require --control-vars ALIAS=FOLDER, plus optional --related";
    if (!error && known_count && command!=3 && command!=4) error="--known applies only to skill-atoms or view skill-atoms";
    if (!error && explorer_mode_supplied && command != 4) error = "--mode applies only to view skill-atoms";
    if (!error && command == 4 && explorer_mode == GD_EXPLORE_CHALLENGES && !(roles & (1 << GD_CHALLENGE))) error = "Challenges mode requires explicit --challenges ALIAS=FOLDER";
    if (!error && !rule_command && !control_command && !mda_command && !(command == 4 && roles == ((1 << GD_SKILL) | (1 << GD_CHALLENGE))) && roles != (route_command ? (1 << GD_SKILL) | (1 << GD_CHALLENGE) : skill_command ? 1 << GD_SKILL : 7))
        error = route_command ? "Supply exactly --skill-atoms and --challenges ALIAS=FOLDER, plus optional --related collections" : skill_command ? "Supply exactly --skill-atoms ALIAS=FOLDER, plus optional --related collections" : "Explicit --nouns, --verbs, and --interactions ALIAS=FOLDER inputs are required";
    if (!error && !control_command && (control_filter.status_count || control_filter.include_count || control_filter.exclude_count)) error = "--status, --include, --exclude apply only to controls";
    if (!error && override_count && !control_command) error = "--set applies only to controls";
    if (!error && import_option && command != 10) error = "--input and --input-format apply only to controls import";
    if (!error && command == 10 && (!input_path || override_count)) error = "Controls import requires --input FILE; edit that file instead of --set";
    if (!error && as_supplied && command != 9) error = "--as applies only to controls export";
    if (!error && command == 9 && (fields || explicit_format >= 0)) error = "Controls export emits raw --as format; no --fields, --json, or --format";
    if (!error && output_path && command != 4) error = "--output applies only to view skill-atoms change reports";
    if (!error && command == 4 && explicit_format == 0) error = "Skill view change report supports JSON only";
    if (command == 4) json = 1;
    if (!error && output_path && (strlen(output_path) >= GD_MAX_PATH || strlen(output_path) < 5 || strcmp(output_path+strlen(output_path)-5, ".json"))) error = "Report path must end in .json and fit 4095 UTF-8 bytes";
    if (!error && fields && (!json || view_command || !fields_valid(fields, command==14 ? 8 : rule_command ? 7 : command == 10 ? 6 : control_command ? 5 : mda_command ? 4 : route_command ? 3 : skill_command ? 2 : command == 1))) error = "--fields needs JSON and supported top-level data keys";
    if (!error && !view_command && (frames || screenshot || layout_supplied || other_pairs || (direction_supplied && !mda_command))) error = "Layout/direction/frame/capture options apply only to views";
    if (!error && command == 2 && layout > GD_VIEW_COLUMNS) error = "Matrix view requires table, graph, or columns layout";
    if (!error && command == 4 && (layout < GD_VIEW_PREREQUISITES || other_pairs)) error = "Skill view requires prerequisites or rank layout; no matrix-pair overlay";
    if (!error && direction_supplied && command != 4 && !mda_command) error = "--direction applies only to MDA or view skill-atoms";
    if (!error && command==14 && from) error="resources currently reports direct declarations only; directed route selectors are unavailable";
    if (!error && from && !mda_command && !control_command && command!=3 && command!=4) error = "--from applies only to MDA, controls, or skill exploration";
    if (!error && command == 7 && (layout != GD_VIEW_COLUMNS && layout != GD_VIEW_GRAPH)) error = "MDA view requires columns or graph layout";
    if (!error && mda_command && other_pairs) error = "--show-other-pairs applies only to matrix views";
    if (!error && other_pairs && layout == GD_VIEW_TABLE) error = "--show-other-pairs requires graph or columns layout";
    if (!error && screenshot && !frames) error = "--screenshot requires bounded --frames";
    if (!error && screenshot && (strlen(screenshot) < 4 || strcmp(screenshot + strlen(screenshot) - 4, ".png"))) error = "Screenshot path must end in .png";
#ifndef GD_WITH_GRAPH
    (void)horizontal;
#endif
    model = gd_create(allocator);
    if (!model) { fputs("gd: cannot allocate bounded model storage\n", stderr); return 2; }
    if (error) gd_error(model, "cli.arguments", error, NULL, 0);
    else gd_load(model, collections, count);
    if (rule_command && !error) {
        rules=gd_read_rules(model);
        if (rules && (analyze || case_name || command == 12 || command == 14)) {
            rule_analysis=gd_analyze_rules(model,rules);
            if (rule_analysis && case_name) {
                int selected_case=gd_rule_case_find(model,rules,case_name), valid=1;
                if (selected_case<0) gd_error(model,"rules.case","--case must identify one parsed ALIAS/NAME#CASE; local name may be omitted only when unique",NULL,0);
                else {
                    rule_analysis->requested_case=selected_case; rule_analysis->scenario.root_case=selected_case; rule_analysis->scenario.exclusion.excluded_by=-1;
                    for (i=0;i<assumption_count;++i) if (!gd_rule_assume(model,rules,selected_case,assumptions[i],&scenario)) valid=0;
                    for (i=0;i<context_assumption_count;++i) if (!gd_rule_assume_context(model,rules,selected_case,context_assumptions[i],context_truths[i],&scenario)) valid=0;
                    if (valid) {
                        GD_RuleAssumptions inputs={scenario.facts,scenario.fact_count,scenario.contexts,scenario.context_count};
                        gd_rule_evaluate(rule_analysis,selected_case,inputs,&rule_analysis->scenario);
                        if (rule_analysis->scenario.input_valid==0) gd_error(model,"rules.scenario","Contradictory/invalid typed assumptions; no case applicability claim",NULL,0);
                    }
                }
            }
            if (rule_analysis && analyze) gd_rule_compare(rule_analysis);
        }
        if ((command == 12 || command == 14) && rules && rule_analysis && !model->error_count) {
            resource_graph=gd_resource_graph_build(model,rules);
            if (resource_graph) gd_resource_graph_bind_calls(resource_graph,rule_analysis);
        }
    }
    if ((command==3 || command==4) && !error && !model->error_count) {
        unsigned char known[GD_MAX_RECORDS]={0}; int selected=-1;
        for (i=0;i<known_count+(from ? 1:0);++i) {
            const char *name=i<known_count ? known_names[i]:from;
            int r,found=-1;
            for (r=0;r<model->record_count;++r) if (!strcmp(model->records[r].reference,name)) { if (found!=-1) { found=-2; break; } found=r; }
            if (found<0 || (model->records[found].role!=GD_SKILL && !(i==known_count && command==4 && explorer_mode==GD_EXPLORE_RELATIONSHIPS))) {
                gd_error(model,"skills.selector","Known skill/root must identify one unique loaded skill (Relationships roots may be other records)",name,0); continue;
            }
            if (i<known_count) known[found]=1; else selected=found;
        }
        if (!model->error_count) {
            skill_query=allocator.allocate(allocator.context,sizeof(*skill_query));
            if (!skill_query) gd_error(model,"skills.memory","Cannot allocate skill exploration",NULL,0);
            else gd_skill_unlocks(model,known,selected>=0 && model->records[selected].role==GD_SKILL ? selected:-1,skill_query);
        }
    }
    if (route_command && !error) routes = gd_analyze_routes(model);
    if (mda_command && !error) {
        mda = gd_analyze_mda(model);
        if (mda && !gd_mda_query(model,mda,from ? gd_mda_find(model,from):-1,mda_direction))
            gd_error(model,"mda.root","--from must identify a unique loaded MDA record",NULL,0);
    }
    if (control_command && !error) {
        int root = from ? gd_control_find(model,from) : -1;
        if (from && root < 0) gd_error(model,"controls.root","--from must identify a unique loaded control",NULL,0);
        else {
            GD_ControlSelection selection;
            control_filter.root = root;
            if (gd_select_controls(model,&control_filter,&selection)) {
                if (command == 10) {
                    size_t size;
                    if (gd_read_text(model,input_path,&size)) import_plan = gd_plan_control_import(model,&selection,model->input,size,input_format,input_path);
                    if (!model->error_count && gd_sources_changed(model)) gd_error(model,"input.changed","Source changed during import planning; retry explicitly",NULL,0);
                } else controls = gd_resolve_control_set(model,&selection,overrides,override_count);
            }
        }
    }
    if (!model->error_count && view_command) {
#ifdef GD_WITH_VIEW
        if (command == 12 && resource_graph) status = gd_resource_view(model,resource_graph,frames,screenshot);
        else if (layout == GD_VIEW_TABLE) status = gd_view(model, frames, screenshot);
        else {
#ifdef GD_WITH_GRAPH
            if (command == 7) status = gd_mda_view(model,mda,frames,screenshot,layout == GD_VIEW_COLUMNS);
            else status = gd_graph_view(model, frames, screenshot, layout, other_pairs, horizontal, output_path, explorer_mode, from, skill_query);
#else
            gd_error(model, "view.graph_unavailable", "Graph view needs a view build with prepared cimgui/rlImGui DLL", NULL, 0);
#endif
        }
#else
        gd_error(model, "view.unavailable", "Use gd.ps1 or build.ps1 -View with the raylib SDK", NULL, 0);
#endif
    }
    if (!view_command || model->error_count) {
        int written = command==14 ? gd_write_resource_result(stdout,model,resource_graph,json,error ? NULL:fields) : rule_command ? gd_write_rule_report(stdout,model,rules,rule_analysis,json,error ? NULL:fields) : command == 10 ? gd_write_control_import(stdout,model,import_plan,json,error ? NULL:fields) : control_command ? gd_write_controls_result(stdout,model,controls,json,error ? NULL:fields,error ? NULL:export_format) : mda_command ? gd_write_mda_result(stdout,model,mda,json,error ? NULL:fields) : route_command ? gd_write_routes_result(stdout,model,routes,json,error ? NULL : fields) :
            command==3 ? gd_write_skill_result(stdout,model,skill_query,json,error ? NULL:fields) : gd_write_result(stdout, model, skill_command ? 2 : command == 1, json, error ? NULL : fields);
        if (!written || fflush(stdout)) {
            fputs("gd: cannot write result\n", stderr); status = 2;
        }
    }
    for (i = 0; i < model->error_count; ++i) {
        const GD_Error *e = &model->errors[i];
        fprintf(stderr, "gd: %s: %s%s%s\n", e->code, e->message, e->path ? " — " : "", e->path ? e->path : "");
    }
    if (model->error_count || model->errors_omitted) status = 2;
    if (skill_query) allocator.release(allocator.context,skill_query);
    gd_resource_graph_destroy(resource_graph); gd_rule_analysis_destroy(rule_analysis); gd_rules_destroy(rules); gd_control_import_destroy(import_plan); gd_controls_destroy(controls); gd_mda_destroy(mda); gd_routes_destroy(routes); gd_destroy(model); return status;
}
int main(int argc, char **argv) {
#ifdef _WIN32
    wchar_t **wide;
    char arguments[131072], *utf8[256];
    int count, i, used = 0, result;
    (void)argc; (void)argv;
    /* The Windows CRT's narrow argv can lose Unicode before paths reach us. */
    wide = CommandLineToArgvW(GetCommandLineW(), &count);
    if (!wide || count > 256) { if (wide) LocalFree(wide); fputs("gd: unsupported argument count\n", stderr); return 2; }
    for (i = 0; i < count; ++i) {
        int bytes = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, wide[i], -1, arguments + used, (int)sizeof(arguments) - used, NULL, NULL);
        if (!bytes) { LocalFree(wide); fputs("gd: invalid or oversized Unicode arguments\n", stderr); return 2; }
        utf8[i] = arguments + used; used += bytes;
    }
    result = application(count, utf8); LocalFree(wide); return result;
#else
    signal(SIGPIPE, SIG_IGN); /* Report errors must return to viewer, not kill its draft. */
    return application(argc, argv);
#endif
}
