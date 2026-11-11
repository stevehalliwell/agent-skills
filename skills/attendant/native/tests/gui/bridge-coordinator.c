/* Native bridge coordinator: real ABI/lifetime, exact P6 markers and provenance. */
#include "tools/native/support.h"
#include "tools/native/manifests.h"
#include "test.h"
static u8 bridge_colors[2097152];
typedef struct bridge_image { unsigned width, height; usize colors; } bridge_image;
static int bridge_ppm_token(FILE *file, char *text, usize capacity) {
    int byte; do { byte = fgetc(file); } while (byte != EOF && strchr(" \t\r\n", byte));
    usize count = 0;
    while (byte != EOF && !strchr(" \t\r\n", byte)) { if (count + 1 >= capacity) return 0; text[count++] = (char)byte; byte = fgetc(file); }
    text[count] = 0; return count > 0 && byte != EOF;
}
static int bridge_ppm_number(FILE *file, unsigned *out) {
    char text[32], *end; if (!bridge_ppm_token(file, text, sizeof(text)) || text[0] < '0' || text[0] > '9') return 0;
    unsigned long number = strtoul(text, &end, 10); if (*end || !number || number > 16384) return 0; *out = (unsigned)number; return 1;
}
static int bridge_screenshot(const char *path, bridge_image *image) {
    FILE *file = file_open_utf8(path, "rb"); TOOL_REQUIRE(file, "Bridge P6 screenshot missing");
    char magic[8]; unsigned maximum = 0; int ok = bridge_ppm_token(file, magic, sizeof(magic)) && !strcmp(magic, "P6") && bridge_ppm_number(file, &image->width) && bridge_ppm_number(file, &image->height) && bridge_ppm_number(file, &maximum) && maximum == 255;
    ok = ok && image->width >= 640 && image->height >= 480 && image->width * 480u == image->height * 640u;
    long start = ftell(file); if (start < 0 || fseek(file, 0, SEEK_END)) ok = 0; long end = ftell(file);
    usize pixels = (usize)image->width * image->height;
    if (end < start || (usize)(end - start) != pixels * 3 || fseek(file, 0, SEEK_SET)) ok = 0;
    char expected_header[64], actual_header[64]; int header_size = snprintf(expected_header, sizeof(expected_header), "P6\n%u %u\n255\n", image->width, image->height);
    if (start < 0 || start >= (long)sizeof(actual_header) || header_size != start || fread(actual_header, 1, (usize)start, file) != (usize)start || memcmp(actual_header, expected_header, (usize)start)) ok = 0;
    memset(bridge_colors, 0, sizeof(bridge_colors)); image->colors = 0; unsigned markers = 0;
    if (ok) for (usize index = 0; index < pixels; ++index) {
        u8 pixel[3]; if (fread(pixel, 1, 3, file) != 3) { ok = 0; break; }
        unsigned color = ((unsigned)pixel[0] << 16) | ((unsigned)pixel[1] << 8) | pixel[2]; u8 mask = (u8)(1u << (color & 7));
        if (!(bridge_colors[color >> 3] & mask)) { bridge_colors[color >> 3] |= mask; ++image->colors; }
        unsigned x = (unsigned)(index % image->width), y = (unsigned)(index / image->width), marker_x = 20u * image->width / 640u;
        if (x == marker_x && y == 20u * image->height / 480u && pixel[0] == 255 && !pixel[1] && !pixel[2]) markers |= 1;
        if (x == marker_x && y == 460u * image->height / 480u && !pixel[0] && !pixel[1] && pixel[2] == 255) markers |= 2;
    }
    if (fclose(file)) ok = 0;
    TOOL_REQUIRE(ok && image->colors > 32 && markers == 3, "Bridge P6 dimensions/aspect/length/colors/exact orientation markers invalid"); return 1;
}
static int bridge_receipt(const void *bytes, usize size) {
    const char *expected = "{\"ok\":true,\"frames\":4,\"image_seen\":true,\"live_images\":0}";
    TOOL_REQUIRE(integration_json_parse(0, bytes, size) == 0 && integration_json_matches("", expected, ""), "Unexpected native bridge lifetime/ABI result"); return 1;
}
#ifndef BRIDGE_VALIDATE_ONLY
#if defined(__APPLE__)
static u8 bridge_memory[262144];
static char bridge_output[1048577], bridge_errors[1048577];
static test_command_workspace bridge_workspace;
static test_command_result bridge_result;
static char bridge_run[NATIVE_PATH_SIZE], bridge_versions[2][65536], bridge_commands[2][NATIVE_PATH_SIZE];
static char bridge_executables[2][NATIVE_PATH_SIZE], bridge_screenshots[2][NATIVE_PATH_SIZE], bridge_binary_hashes[2][65], bridge_screenshot_hashes[2][65];
static bridge_image bridge_images[2];
static unsigned bridge_count;
static int bridge_execute(const char *const *arguments, usize count, u32 timeout) {
    test_command command = {.arguments = arguments, .argument_count = count, .timeout_ms = timeout};
    test_command_error error = test_command_run(&bridge_workspace, &command, &bridge_result);
    TOOL_REQUIRE(error == test_command_ok && bridge_result.exited && !bridge_result.exit_code, "Bridge build/check child failed"); return 1;
}
static int bridge_suite(void) {
    TOOL_REQUIRE(tool_verify_stack(), "Stale GUI stack; rebuild before bridge acceptance");
    char supplied[NATIVE_PATH_SIZE], bash[NATIVE_PATH_SIZE];
    TOOL_REQUIRE(tool_environment("ATTENDANT_BRIDGE_DIRECTORY", supplied, sizeof(supplied)) && *supplied && file_tree_path((cstring){supplied}, bridge_run, 0) == file_error_ok && tool_environment("ATTENDANT_TOOL_BASH", bash, sizeof(bash)) && *bash, "Missing owned bridge directory/Bash");
    for (unsigned i = 0; i < 2; ++i) {
        const char *compiler = i ? "cjit" : "zig"; const char *build[] = {bash, "tools/check-gui-bridge.sh", compiler};
        int child_ok = bridge_execute(build, 3, 120000);
        char log[NATIVE_PATH_SIZE];
        TOOL_REQUIRE(snprintf(log, sizeof(log), "%s/%s.stdout", bridge_run, compiler) < (int)sizeof(log) && file_create_bytes((cstring){log}, bridge_result.output.data, bridge_result.output.len) == file_error_ok && snprintf(log, sizeof(log), "%s/%s.stderr", bridge_run, compiler) < (int)sizeof(log) && file_create_bytes((cstring){log}, bridge_result.error_output.data, bridge_result.error_output.len) == file_error_ok, "Cannot retain bridge captures");
        TOOL_REQUIRE(child_ok && bridge_receipt(bridge_result.output.data, bridge_result.output.len), "Unexpected native bridge lifetime/ABI result");
        char relative[NATIVE_PATH_SIZE]; snprintf(relative, sizeof(relative), "build/bridge-check/%s.ppm", compiler);
        TOOL_REQUIRE(file_tree_path((cstring){relative}, bridge_screenshots[i], 0) == file_error_ok && bridge_screenshot(bridge_screenshots[i], &bridge_images[i]) && tool_sha256(bridge_screenshots[i], bridge_screenshot_hashes[i]), "Bridge screenshot failed");
        snprintf(relative, sizeof(relative), "build/bridge-check/bridge-%s", compiler); TOOL_REQUIRE(file_tree_path((cstring){relative}, bridge_executables[i], 0) == file_error_ok && tool_sha256(bridge_executables[i], bridge_binary_hashes[i]), "Cannot identify bridge executable");
        TOOL_REQUIRE(tool_environment(i ? "CJIT" : "ZIG", bridge_commands[i], sizeof(bridge_commands[i])), "Cannot read compiler command"); if (!*bridge_commands[i]) strcpy(bridge_commands[i], compiler);
        /* Version probes preserve stdout + stderr, outside measured/rendered work. */
        const char *version[] = {bash, "-c", "\"$@\" 2>&1", "native-version", bridge_commands[i], i ? "--help" : "version"};
        TOOL_REQUIRE(bridge_execute(version, 6, 10000) && bridge_result.output.len < sizeof(bridge_versions[i]) && !memchr(bridge_output, 0, bridge_result.output.len), "Cannot identify bridge compiler");
        usize size = bridge_result.output.len; memcpy(bridge_versions[i], bridge_output, size + 1); tool_normalize(bridge_versions[i], &size, 1); ++bridge_count;
    }
    return 1;
}
int main(int argc, char **argv) {
    if (argc == 2 && !strcmp(argv[1], "--help")) { puts("Usage: bash tools/check-bridge.sh\nmacOS bridge lifetime/ABI/exact P6 orientation checks, Zig and CJIT. Stdout JSON; build/bridge-check-results.json."); return 0; }
    if (argc != 1) { puts("{\"ok\":false,\"code\":\"usage\"}"); return 2; }
    bridge_workspace = (test_command_workspace){arena_from_buffer(bridge_memory, sizeof(bridge_memory)), bridge_output, bridge_errors, sizeof(bridge_output), sizeof(bridge_errors)};
    int ok = tool_initialize() && environment_set(tool_files.memory, "DYLD_LIBRARY_PATH", NULL) == environment_error_ok && bridge_suite();
    char timestamp[32]; if (!tool_timestamp(timestamp)) ok = 0;
    tool_path_count = 0; const char *inputs[] = {"tests/gui/bridge-check.c", "tests/gui/bridge-coordinator.c", "tools/check-bridge.sh", "tools/check-gui-bridge.sh", "tools/native/support.h", "tools/native/manifests.h", "tools/native-build.sh", "tools/capture-command.sh"};
    for (usize i = 0; i < sizeof(inputs)/sizeof(inputs[0]); ++i) if (!tool_add_path(inputs[i])) ok = 0;
    if (!tool_hash_paths()) ok = 0;
    FILE *out = file_open_utf8("build/bridge-check-results.json", "wb");
    if (out) {
        fprintf(out, "{\"ok\":%s,\"platform\":\"darwin\",\"architecture\":", ok ? "true" : "false"); tool_json_string(out, tool_architecture()); fputs(",\"testedAt\":", out); tool_json_string(out, timestamp); fputs(",\"run\":", out); tool_json_string(out, bridge_run); fputs(",\"inputs\":", out); if (!tool_write_files(out)) ok = 0;
        fputs(",\"stack\":", out); if (!tool_stack_summary(out)) { fputs("null", out); ok = 0; }
        fputs(",\"cases\":[", out);
        for (unsigned i = 0; i < bridge_count; ++i) { if (i) fputc(',', out); fprintf(out, "{\"compiler\":\"%s\",\"compilerCommand\":", i ? "cjit" : "zig"); tool_json_string(out, bridge_commands[i]); fputs(",\"version\":", out); tool_json_string(out, bridge_versions[i]); fputs(",\"bridge\":{\"ok\":true,\"frames\":4,\"image_seen\":true,\"live_images\":0},\"executable\":{\"path\":", out); tool_json_string(out, bridge_executables[i]); fputs(",\"sha256\":", out); tool_json_string(out, bridge_binary_hashes[i]); fputs("},\"screenshot\":{\"path\":", out); tool_json_string(out, bridge_screenshots[i]); fputs(",\"sha256\":", out); tool_json_string(out, bridge_screenshot_hashes[i]); fprintf(out, "},\"width\":%u,\"height\":%u,\"colors\":%zu}", bridge_images[i].width, bridge_images[i].height, bridge_images[i].colors); }
        fputs("]}\n", out); if (ferror(out)) ok = 0; if (fclose(out)) ok = 0;
    } else ok = 0;
    if (!ok) fprintf(stderr, "%s\n%.*s\n", tool_problem, 4096, bridge_errors);
    printf("{\"ok\":%s,\"cases\":%u}\n", ok ? "true" : "false", bridge_count); return ok ? 0 : 1;
}
#else
int main(void) { fputs("macOS bridge acceptance requires native macOS; not validated on Windows.\n", stderr); puts("{\"ok\":false,\"code\":\"unsupported-platform\"}"); return 2; }
#endif
#endif
