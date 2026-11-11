/* Warm synthetic process benchmark; not real-project or GUI footprint evidence. */
#include "tools/native/support.h"
#include "test.h"
typedef struct benchmark_sample { unsigned round, compiler, command; double milliseconds; } benchmark_sample;
static benchmark_sample benchmark_samples[6000];
static unsigned benchmark_count;
static u8 benchmark_memory[262144];
static char benchmark_output[1048577], benchmark_errors[1048577];
static test_command_workspace benchmark_workspace;
static const char *benchmark_names[] = {"sync", "query", "search"};
static char benchmark_run[NATIVE_PATH_SIZE], benchmark_projects[2][NATIVE_PATH_SIZE];
static int benchmark_invoke(unsigned compiler, unsigned operation, double *milliseconds) {
    char executable[NATIVE_PATH_SIZE];
#if defined(_WIN32)
    const char *suffix = ".exe";
#else
    const char *suffix = "";
#endif
    TOOL_REQUIRE(snprintf(executable, sizeof(executable), "build/attendant-%s%s", compiler ? "zig" : "cjit", suffix) < (int)sizeof(executable), "Benchmark executable path exceeds capacity");
    const char *arguments[8] = {executable, benchmark_names[operation]}; usize count = 2;
    if (operation) { arguments[count++] = operation == 1 ? "-s" : "-q"; arguments[count++] = operation == 1 ? "SELECT name,status FROM tasks ORDER BY name" : "projection"; }
    arguments[count++] = "-p"; arguments[count++] = benchmark_projects[compiler];
    test_command command = {.arguments = arguments, .argument_count = count, .timeout_ms = 30000}; test_command_result result;
    u64 start = clock_now_ns(); test_command_error error = test_command_run(&benchmark_workspace, &command, &result); u64 end = clock_now_ns();
    TOOL_REQUIRE(start && end >= start && error == test_command_ok && result.exited && !result.exit_code, "Benchmark command failed/expired; no timing accepted");
    *milliseconds = (double)(end - start) / 1000000.0;
    TOOL_REQUIRE(integration_json_parse(0, result.output.data, result.output.len) == 0, "Benchmark output is not valid JSON");
    if (!operation) TOOL_REQUIRE(integration_text_is("state", "rebuilt") || integration_text_is("state", "reused"), "Unexpected sync result");
    else TOOL_REQUIRE(integration_number_is("returned", operation == 1 ? 2 : 1), "Unexpected benchmark row count"); return 1;
}
static int benchmark_compare(const void *a, const void *b) { double left = *(const double *)a, right = *(const double *)b; return (left > right) - (left < right); }
static int benchmark_summary(FILE *out) {
    fputc('[', out);
    for (unsigned compiler = 0; compiler < 2; ++compiler) for (unsigned command = 0; command < 3; ++command) {
        double values[1000]; usize count = 0;
        for (unsigned i = 0; i < benchmark_count; ++i) if (benchmark_samples[i].compiler == compiler && benchmark_samples[i].command == command) values[count++] = benchmark_samples[i].milliseconds;
        if (compiler || command) fputc(',', out);
        fprintf(out, "{\"compiler\":\"%s\",\"command\":\"%s\",\"samples\":%zu", compiler ? "zig" : "cjit", benchmark_names[command], count);
        if (count) { qsort(values, count, sizeof(values[0]), benchmark_compare); usize middle = count / 2; double median = count % 2 ? values[middle] : (values[middle - 1] + values[middle]) / 2;
            fprintf(out, ",\"minimumMs\":%.9f,\"medianMs\":%.9f,\"maximumMs\":%.9f", values[0], median, values[count - 1]); }
        fputc('}', out);
    }
    fputc(']', out); return !ferror(out);
}
int main(int argc, char **argv) {
    if (argc == 2 && !strcmp(argv[1], "--help")) { puts("Usage: bash tools/benchmark.sh [runs=10]\nInteger 1..1000. Owned synthetic copies; warm sync/query/search; CJIT/Zig. Stdout JSON; results under build/bench/. Builds/copy/warmup/JSON checking excluded; process launch through exit included."); return 0; }
    unsigned runs = 10;
    if (argc > 2) { puts("{\"ok\":false,\"code\":\"usage\"}"); return 2; }
    if (argc == 2) { char *end; unsigned long number = strtoul(argv[1], &end, 10); if (!argv[1][0] || *end || number < 1 || number > 1000 || argv[1][0] == '-') { puts("{\"ok\":false,\"code\":\"usage\"}"); return 2; } runs = (unsigned)number; }
    char supplied[NATIVE_PATH_SIZE]; int ok = tool_initialize() && tool_environment("ATTENDANT_BENCHMARK_DIRECTORY", supplied, sizeof(supplied)) && *supplied && file_tree_path((cstring){supplied}, benchmark_run, 0) == file_error_ok;
    benchmark_workspace = (test_command_workspace){arena_from_buffer(benchmark_memory, sizeof(benchmark_memory)), benchmark_output, benchmark_errors, sizeof(benchmark_output), sizeof(benchmark_errors)};
    for (unsigned i = 0; ok && i < 2; ++i) ok = tool_path(benchmark_projects[i], benchmark_run, i ? "zig" : "cjit") && file_directory_copy(&tool_files, (cstring){"tests/fixtures/project"}, (cstring){benchmark_projects[i]}) == file_error_ok;
    double ignored;
    for (unsigned i = 0; ok && i < 2; ++i) for (unsigned j = 0; ok && j < 3; ++j) ok = benchmark_invoke(i, j, &ignored);
    for (unsigned round = 0; ok && round < runs; ++round) for (unsigned offset = 0; ok && offset < 2; ++offset) for (unsigned command = 0; ok && command < 3; ++command) {
        unsigned compiler = (round + offset) % 2; double milliseconds;
        ok = benchmark_invoke(compiler, command, &milliseconds);
        if (ok) benchmark_samples[benchmark_count++] = (benchmark_sample){round, compiler, command, milliseconds};
    }
    char path[NATIVE_PATH_SIZE]; FILE *out = tool_path(path, benchmark_run, "results.json") ? file_open_utf8(path, "wb") : NULL;
    if (out) {
        fprintf(out, "{\"ok\":%s,\"platform\":", ok ? "true" : "false"); tool_json_string(out, tool_platform()); fputs(",\"architecture\":", out); tool_json_string(out, tool_architecture());
        fprintf(out, ",\"runs\":%u,\"scope\":\"warm synthetic fixture, process launch through exit; rotating serial compiler order\",\"samples\":[", runs);
        for (unsigned i = 0; i < benchmark_count; ++i) { benchmark_sample *sample = &benchmark_samples[i]; if (i) fputc(',', out); fprintf(out, "{\"round\":%u,\"compiler\":\"%s\",\"command\":\"%s\",\"milliseconds\":%.9f}", sample->round, sample->compiler ? "zig" : "cjit", benchmark_names[sample->command], sample->milliseconds); }
        fputs("],\"summary\":", out); benchmark_summary(out); fputs("}\n", out); if (ferror(out)) ok = 0; if (fclose(out)) ok = 0;
    } else ok = 0;
    if (!ok) fprintf(stderr, "%s\nBenchmark evidence: %s\n%.*s\n", tool_problem, benchmark_run, (int)(strlen(benchmark_errors) > 4096 ? 4096 : strlen(benchmark_errors)), benchmark_errors);
    fprintf(stdout, "{\"ok\":%s,\"run\":", ok ? "true" : "false"); tool_json_string(stdout, benchmark_run); fputs(",\"summary\":", stdout); benchmark_summary(stdout); puts("}"); return ok ? 0 : 1;
}
