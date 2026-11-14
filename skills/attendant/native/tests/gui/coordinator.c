/* Shared GUI acceptance: native provenance/build/result coordination only. */
#include "tools/native/support.h"
#include "tools/native/manifests.h"
#include "test.h"
static u8 coordinator_memory[262144];
static char coordinator_output[1048577], coordinator_errors[1048577];
static test_command_workspace coordinator_commands;
static test_command_result coordinator_result;
static char coordinator_report[NATIVE_PATH_SIZE];
static const char *coordinator_names[] = {
    "production_render_abi_screenshot_without_workspace_writes", "existing_c_document_selection_ipc_model",
    "terminal_render_failure_preserves_source", "partial_discovery_diagnostics_preserve_source",
    "reference_navigation_and_tab_reuse_without_writes", "literal_path_undo_isolation_and_redo",
    "real_edit_feedback_and_explicit_resolution", "failed_queue_keeps_save_and_other_files",
    "preview_multi_cross_compiler_inspection_context_save_send_and_resolution",
    "preview_context_cross_compiler_inspection_context_save_send_and_resolution"
};
static const char *coordinator_source(unsigned index) { return index < 8 ? "tests/gui/integration/checks.c" : "tests/gui/integration/preview.c"; }
static int coordinator_execute(const char *const *arguments, usize count, u32 timeout, const test_command_environment *environment, usize environment_count) {
    test_command command = {.arguments = arguments, .argument_count = count, .environment = environment, .environment_count = environment_count, .timeout_ms = timeout};
    test_command_error error = test_command_run(&coordinator_commands, &command, &coordinator_result);
    if (error || !coordinator_result.exited || coordinator_result.exit_code) {
        fprintf(stderr, "GUI coordinator child: %s, OS %lu, exit %lu\n%.*s\n", test_command_error_message(error), coordinator_result.native_error, coordinator_result.exit_code,
            (int)(coordinator_result.error_output.len > 4096 ? 4096 : coordinator_result.error_output.len), coordinator_result.error_output.data);
        return tool_error("GUI child command failed; inspect retained captures");
    }
    return 1;
}
static int coordinator_validate(unsigned order[10], int *checks) {
    TOOL_REQUIRE(!memchr(coordinator_output, 0, coordinator_result.output.len) && !memchr(coordinator_errors, 0, coordinator_result.error_output.len), "Embedded NUL in GUI runner result");
    unsigned line_number = 0, seen = 0, count = 0; char *line = coordinator_output;
    while (*line) {
        char *end = strchr(line, '\n'); TOOL_REQUIRE(end, "Incomplete GUI TSV line"); *end = 0;
        usize length = strlen(line); if (length && line[length - 1] == '\r') line[length - 1] = 0;
        if (!line_number) TOOL_REQUIRE(!strcmp(line, "kind\tstatus\tsource\tname"), "Invalid GUI TSV header");
        else if (line_number == 1) TOOL_REQUIRE(!strcmp(line, "suite\t10\t\t"), "Invalid GUI suite identity/count");
        else if (line_number < 12) {
            const char *prefix = "test\tok\t"; TOOL_REQUIRE(!strncmp(line, prefix, strlen(prefix)), "GUI case did not pass");
            char *source = line + strlen(prefix), *name = strchr(source, '\t'); TOOL_REQUIRE(name && !strchr(name + 1, '\t'), "Invalid GUI TSV columns"); *name++ = 0;
            unsigned index; for (index = 0; index < 10; ++index) if (!strcmp(name, coordinator_names[index]) && !strcmp(source, coordinator_source(index))) break;
            TOOL_REQUIRE(index < 10 && !(seen & (1u << index)), "Unknown/duplicate GUI case identity"); seen |= 1u << index; order[count++] = index;
        } else TOOL_REQUIRE(line_number == 12 && !strcmp(line, "complete\t0\t10\t10"), "GUI runner did not complete successfully");
        ++line_number; line = end + 1;
    }
    TOOL_REQUIRE(line_number == 13 && seen == 1023 && count == 10, "Missing/extra GUI TSV rows");
    unsigned markers = 0; const char *at = coordinator_errors;
    while (*at) {
        const char *end = strchr(at, '\n'); usize length = end ? (usize)(end - at) : strlen(at);
        if (length && at[length - 1] == '\r') --length;
        if (length >= 13 && !strncmp(at, "model-checks:", 13)) {
            TOOL_REQUIRE(length > 13 && length - 13 < 16, "Invalid model count marker");
            unsigned long number = 0;
            for (usize i = 13; i < length; ++i) { TOOL_REQUIRE(at[i] >= '0' && at[i] <= '9' && number <= (INT_MAX - (unsigned)(at[i] - '0')) / 10u, "Invalid model count"); number = number * 10 + (unsigned)(at[i] - '0'); }
            TOOL_REQUIRE(number > 0, "Invalid model count"); *checks = (int)number; ++markers;
        }
        if (!end) break; at = end + 1;
    }
    TOOL_REQUIRE(markers == 1, "Missing/duplicate model count marker"); return 1;
}
typedef struct coordinator_run { char runner[NATIVE_PATH_SIZE], directory[NATIVE_PATH_SIZE]; unsigned order[10]; int checks, status, complete; } coordinator_run;
static coordinator_run coordinator_runs[2];
static unsigned coordinator_count;
static int coordinator_suite(void) {
    const char *compilers[] = {"cjit", "zig"};
    for (unsigned i = 0; i < 2; ++i) TOOL_REQUIRE(tool_verify_gui(compilers[i], 0) && tool_verify_gui(compilers[i], 1), "GUI provenance verification failed");
    char bash[NATIVE_PATH_SIZE]; TOOL_REQUIRE(tool_environment("ATTENDANT_TOOL_BASH", bash, sizeof(bash)) && *bash, "Missing native bootstrap Bash");
    for (unsigned i = 0; i < 2; ++i) {
        coordinator_run *run = &coordinator_runs[i]; run->status = -1;
        const char *build[] = {bash, "tools/build-gui-tests.sh", compilers[i]};
        TOOL_REQUIRE(coordinator_execute(build, 3, 180000, NULL, 0), "GUI runner build failed");
        usize size = coordinator_result.output.len;
        TOOL_REQUIRE(!memchr(coordinator_output, 0, size), "Invalid runner path output");
        while (size && strchr("\r\n\t ", coordinator_output[size - 1])) --size;
        TOOL_REQUIRE(size > 0 && size < sizeof(run->runner), "Invalid GUI runner path"); memcpy(run->runner, coordinator_output, size); run->runner[size] = 0;
        TOOL_REQUIRE(file_is_regular(run->runner, 0), "GUI runner executable missing");
        TOOL_REQUIRE(snprintf(run->directory, sizeof(run->directory), "%s/native-%s", coordinator_report, compilers[i]) < (int)sizeof(run->directory) && file_directory_create(run->directory, 1), "Cannot exclusively create compiler evidence directory");
        ++coordinator_count;
        test_command_environment environment[] = {{"ATTENDANT_TEST_DIRECTORY", run->directory}, {"ATTENDANT_TEST_COMPILER", compilers[i]}};
        const char *arguments[] = {run->runner, "-tsv"}; int ok = coordinator_execute(arguments, 2, 900000, environment, 2);
        run->status = coordinator_result.exited && coordinator_result.exit_code <= INT_MAX ? (int)coordinator_result.exit_code : -1;
        char path[NATIVE_PATH_SIZE];
        TOOL_REQUIRE(tool_path(path, run->directory, "results.tsv") && file_create_bytes((cstring){path}, coordinator_result.output.data, coordinator_result.output.len) == file_error_ok, "Cannot retain GUI TSV");
        TOOL_REQUIRE(tool_path(path, run->directory, "diagnostics.log") && file_create_bytes((cstring){path}, coordinator_result.error_output.data, coordinator_result.error_output.len) == file_error_ok, "Cannot retain GUI diagnostics");
        TOOL_REQUIRE(ok && coordinator_validate(run->order, &run->checks), "GUI runner result validation failed"); run->complete = 1;
    }
    return 1;
}
static int coordinator_write(int success) {
    FILE *out = file_open_utf8("build/gui-check-results.json", "wb"); TOOL_REQUIRE(out, "Cannot write GUI report");
    fprintf(out, "{\"ok\":%s,\"platform\":", success ? "true" : "false"); tool_json_string(out, tool_platform()); fputs(",\"architecture\":", out); tool_json_string(out, tool_architecture()); fputs(",\"run\":", out); tool_json_string(out, coordinator_report); fputs(",\"cases\":[", out);
    unsigned cases = 0;
    for (unsigned i = 0; i < coordinator_count; ++i) if (coordinator_runs[i].complete) for (unsigned j = 0; j < 10; ++j) {
        unsigned index = coordinator_runs[i].order[j]; if (cases++) fputc(',', out);
        fputs("{\"compiler\":", out); tool_json_string(out, i ? "zig" : "cjit"); fputs(",\"name\":", out); tool_json_string(out, coordinator_names[index]); fputs(",\"source\":", out); tool_json_string(out, coordinator_source(index));
        if (index == 1) fprintf(out, ",\"result\":{\"ok\":true,\"checks\":%d}", coordinator_runs[i].checks); fputc('}', out);
    }
    fputs("],\"native\":[", out);
    for (unsigned i = 0; i < coordinator_count; ++i) {
        if (i) fputc(',', out); fputs("{\"compiler\":", out); tool_json_string(out, i ? "zig" : "cjit"); fputs(",\"runner\":", out); tool_json_string(out, coordinator_runs[i].runner); fputs(",\"directory\":", out); tool_json_string(out, coordinator_runs[i].directory); fprintf(out, ",\"status\":%d}", coordinator_runs[i].status);
    }
    fputs("]}\n", out); int ok = !ferror(out); if (fclose(out)) ok = 0; return ok;
}
#ifndef COORDINATOR_VALIDATE_ONLY
int main(int argc, char **argv) {
    if (argc == 2 && !strcmp(argv[1], "--help")) { puts("Usage: bash tools/check-gui.sh\nBuild both compiler CLI/production/check GUI variants first. Stdout JSON; retained report build/gui-check-results.json."); return 0; }
    if (argc != 1) { puts("{\"ok\":false,\"code\":\"usage\"}"); return 2; }
    coordinator_commands = (test_command_workspace){arena_from_buffer(coordinator_memory, sizeof(coordinator_memory)), coordinator_output, coordinator_errors, sizeof(coordinator_output), sizeof(coordinator_errors)};
    char supplied[NATIVE_PATH_SIZE];
    if (!tool_initialize() || !tool_environment("ATTENDANT_GUI_REPORT_DIRECTORY", supplied, sizeof(supplied)) || !*supplied || file_tree_path((cstring){supplied}, coordinator_report, 0) != file_error_ok) { fprintf(stderr, "Use tools/check-gui.sh: %s\n", tool_problem); return 2; }
#if defined(__APPLE__)
    if (environment_set(tool_files.memory, "DYLD_LIBRARY_PATH", NULL) != environment_error_ok) return 2;
#endif
    int ok = coordinator_suite(); if (!coordinator_write(ok)) ok = 0;
    if (!ok) fprintf(stderr, "%s\nGUI report: build/gui-check-results.json\n", tool_problem);
    fprintf(stdout, "{\"ok\":%s,\"cases\":%u,\"run\":", ok ? "true" : "false", ok ? 20u : 0u); tool_json_string(stdout, coordinator_report); puts("}"); return ok ? 0 : 1;
}
#endif
