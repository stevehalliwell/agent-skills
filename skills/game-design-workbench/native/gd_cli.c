#if !defined(_WIN32) && !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif
#include "gd.h"
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
         "Usage: gd matrix|connectivity|skill-atoms|scaffolding|mda|controls|rules [options]\n"
         "       gd view matrix|skill-atoms|mda [options]\n\n"
         "Matrix/connectivity collections (exactly one of each):\n"
         "  --nouns ALIAS=FOLDER\n  --verbs ALIAS=FOLDER\n  --interactions ALIAS=FOLDER\n"
         "Skill operations require: --skill-atoms ALIAS=FOLDER\n"
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
         "Rules require --interactions and/or --mechanics ALIAS=FOLDER\n"
         "  Optional --axes, --nouns, --verbs, --control-vars, --related; explicit applicability\n"
         "  Reads labelled gd-rule body blocks; no state/formula evaluation or comparison\n"
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
         "                         skill-atoms: skills,links,components,findings\n"
         "                         scaffolding: challenges,skills,checks,witnesses,findings\n"
         "                         mda: nodes,links,paths,gaps,findings; controls: controls\n"
         "                         rules: records,cases,axes,actors,findings\n"
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
        if (operation == 7) {
            if (!((n==7 && !memcmp(p,"records",n)) || (n==5 && !memcmp(p,"cases",n)) ||
                  (n==4 && !memcmp(p,"axes",n)) || (n==6 && !memcmp(p,"actors",n)) || (n==8 && !memcmp(p,"findings",n)))) return 0;
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
                  (n == 10 && !memcmp(p, "components", n)) || (n == 8 && !memcmp(p, "findings", n)))) return 0;
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
    int explorer_mode_supplied = 0;
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
    if (argc < 2) error = "Supply matrix, connectivity, skill-atoms, scaffolding, mda, controls, rules, or view; see --help";
    else if (!strcmp(argv[1], "connectivity")) command = 1;
    else if (!strcmp(argv[1], "view") && argc > 2 && !strcmp(argv[2], "matrix")) { command = 2; first = 3; }
    else if (!strcmp(argv[1], "skill-atoms")) command = 3;
    else if (!strcmp(argv[1], "scaffolding")) command = 5;
    else if (!strcmp(argv[1], "mda")) command = 6;
    else if (!strcmp(argv[1], "rules")) command = 11;
    else if (!strcmp(argv[1], "controls")) {
        command = 8;
        if (argc > 2 && !strcmp(argv[2],"export")) { command = 9; first = 3; export_format = "json"; }
        else if (argc > 2 && !strcmp(argv[2],"import")) { command = 10; first = 3; }
    }
    else if (!strcmp(argv[1], "view") && argc > 2 && !strcmp(argv[2], "mda")) { command = 7; first = 3; layout = GD_VIEW_COLUMNS; }
    else if (!strcmp(argv[1], "view") && argc > 2 && !strcmp(argv[2], "skill-atoms")) { command = 4; first = 3; layout = GD_VIEW_PREREQUISITES; }
    else if (strcmp(argv[1], "matrix")) error = "Unsupported command; no generic inspect/validate/query commands exist";
    control_filter.root = -1; control_filter.accepted_default = command == 9 || command == 10;
    control_command = command >= 8 && command <= 10;
    route_command = command == 5;
    rule_command = command == 11;
    mda_command = command == 6 || command == 7;
    skill_command = command >= 3 && command <= 5; view_command = command == 2 || command == 4 || command == 7;
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
            collections[count].role = rule_command && (role == GD_CASE || role == GD_MDA_MECHANIC) ? GD_RULE : role;
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
            if (++i == argc || (strcmp(argv[i],"skills") && strcmp(argv[i],"challenges") && strcmp(argv[i],"relationships"))) { error = "--mode requires skills, challenges, or relationships"; break; }
            explorer_mode = !strcmp(argv[i],"relationships") ? GD_EXPLORE_RELATIONSHIPS : !strcmp(argv[i],"challenges") ? GD_EXPLORE_CHALLENGES : GD_EXPLORE_SKILLS;
        } else if (!strcmp(argv[i], "--layout")) {
            if (++i == argc) { error = "--layout requires table, graph, columns, prerequisites, or rank"; break; }
            layout_supplied = 1;
            if (!strcmp(argv[i], "table")) layout = GD_VIEW_TABLE;
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
    if (!error && mda_command) {
        int required = (1 << GD_MDA_MECHANIC) | (1 << GD_MDA_DYNAMIC) | (1 << GD_MDA_AESTHETIC);
        if (roles != required && roles != (required | (1 << GD_MDA_SYSTEM))) error = "MDA requires --mechanics, --dynamics, --aesthetics ALIAS=FOLDER; optional --systems and --related";
    }
    if (!error && rule_command) {
        int roots = (1 << GD_CASE) | (1 << GD_MDA_MECHANIC);
        int permitted = roots | (1 << GD_NOUN) | (1 << GD_VERB) | (1 << GD_CONTROL) | (1 << GD_AXIS);
        if (!(roles & roots) || (roles & ~permitted)) error = "Rules require --interactions and/or --mechanics, optional --axes, --nouns, --verbs, --control-vars and --related";
    }
    if (!error && control_command && roles != (1 << GD_CONTROL)) error = "Controls require --control-vars ALIAS=FOLDER, plus optional --related";
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
    if (!error && fields && (!json || view_command || !fields_valid(fields, rule_command ? 7 : command == 10 ? 6 : control_command ? 5 : mda_command ? 4 : route_command ? 3 : skill_command ? 2 : command == 1))) error = "--fields needs JSON and supported top-level data keys";
    if (!error && !view_command && (frames || screenshot || layout_supplied || other_pairs || (direction_supplied && !mda_command))) error = "Layout/direction/frame/capture options apply only to views";
    if (!error && command == 2 && layout > GD_VIEW_COLUMNS) error = "Matrix view requires table, graph, or columns layout";
    if (!error && command == 4 && (layout < GD_VIEW_PREREQUISITES || other_pairs)) error = "Skill view requires prerequisites or rank layout; no matrix-pair overlay";
    if (!error && direction_supplied && command != 4 && !mda_command) error = "--direction applies only to MDA or view skill-atoms";
    if (!error && from && !mda_command && !control_command && !(command == 4 && explorer_mode == GD_EXPLORE_RELATIONSHIPS)) error = "--from applies only to MDA, controls, or view skill-atoms --mode relationships";
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
    if (rule_command && !error) rules = gd_read_rules(model);
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
        if (layout == GD_VIEW_TABLE) status = gd_view(model, frames, screenshot);
        else {
#ifdef GD_WITH_GRAPH
            if (command == 7) status = gd_mda_view(model,mda,frames,screenshot,layout == GD_VIEW_COLUMNS);
            else status = gd_graph_view(model, frames, screenshot, layout, other_pairs, horizontal, output_path, explorer_mode, from);
#else
            gd_error(model, "view.graph_unavailable", "Graph view needs a view build with prepared cimgui/rlImGui DLL", NULL, 0);
#endif
        }
#else
        gd_error(model, "view.unavailable", "Use gd.ps1 or build.ps1 -View with the raylib SDK", NULL, 0);
#endif
    }
    if (!view_command || model->error_count) {
        int written = rule_command ? gd_write_rules_result(stdout,model,rules,json,error ? NULL:fields) : command == 10 ? gd_write_control_import(stdout,model,import_plan,json,error ? NULL:fields) : control_command ? gd_write_controls_result(stdout,model,controls,json,error ? NULL:fields,error ? NULL:export_format) : mda_command ? gd_write_mda_result(stdout,model,mda,json,error ? NULL:fields) : route_command ? gd_write_routes_result(stdout,model,routes,json,error ? NULL : fields) :
            gd_write_result(stdout, model, skill_command ? 2 : command == 1, json, error ? NULL : fields);
        if (!written || fflush(stdout)) {
            fputs("gd: cannot write result\n", stderr); status = 2;
        }
    }
    for (i = 0; i < model->error_count; ++i) {
        const GD_Error *e = &model->errors[i];
        fprintf(stderr, "gd: %s: %s%s%s\n", e->code, e->message, e->path ? " — " : "", e->path ? e->path : "");
    }
    if (model->error_count || model->errors_omitted) status = 2;
    gd_rules_destroy(rules); gd_control_import_destroy(import_plan); gd_controls_destroy(controls); gd_mda_destroy(mda); gd_routes_destroy(routes); gd_destroy(model); return status;
}
int main(int argc, char **argv) {
#ifdef _WIN32
    wchar_t **wide;
    char arguments[131072], *utf8[128];
    int count, i, used = 0, result;
    (void)argc; (void)argv;
    /* The Windows CRT's narrow argv can lose Unicode before paths reach us. */
    wide = CommandLineToArgvW(GetCommandLineW(), &count);
    if (!wide || count > 128) { if (wide) LocalFree(wide); fputs("gd: unsupported argument count\n", stderr); return 2; }
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
