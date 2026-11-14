/* Maintained native-tool regression: owned files/repos only; no downloads. */
#define COORDINATOR_VALIDATE_ONLY
#include "tests/gui/coordinator.c"
#define BRIDGE_VALIDATE_ONLY
#include "tests/gui/bridge-coordinator.c"
#include "tools/native/dependencies.h"
static test_directory validation_fixture;
static char validation_tool[NATIVE_PATH_SIZE];
static int validation_path(char out[NATIVE_PATH_SIZE], const char *label) { return test_directory_next(&validation_fixture, label, out) == file_error_ok || tool_error("Cannot reserve validation path"); }
static int validation_hash(void) {
    char path[NATIVE_PATH_SIZE], hash[65]; TOOL_REQUIRE(validation_path(path, "hash") && file_create_text((cstring){path}, "") == file_error_ok && tool_sha256(path, hash) && !strcmp(hash, "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"), "Empty SHA-256 mismatch");
    TOOL_REQUIRE(file_write_bytes((cstring){path}, "abc", 3) == file_error_ok && tool_sha256(path, hash) && !strcmp(hash, "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"), "ABC SHA-256 mismatch");
    static u8 bytes[65792]; for (usize i = 0; i < sizeof(bytes); ++i) bytes[i] = (u8)i;
    TOOL_REQUIRE(file_write_bytes((cstring){path}, bytes, sizeof(bytes)) == file_error_ok && tool_sha256(path, hash) && !strcmp(hash, "120c518a83325c66464701a6ee080302f332bc768ea3f60473b209f1bfb091df"), "Binary/multi-chunk SHA-256 mismatch"); return 1;
}
static int validation_capture(void) {
    char bash[NATIVE_PATH_SIZE], output[256]; usize size;
    TOOL_REQUIRE(tool_environment("ATTENDANT_TOOL_BASH", bash, sizeof(bash)), "Missing Bash");
    const char *value = "café Ω; $HOME \" \\ x";
    const char *arguments[] = {bash, "-c", "printf '%s' \"$1\"; printf '%s' notice >&2", "capture-test", value};
    TOOL_REQUIRE(tool_capture(arguments, 5, output, sizeof(output), &size) && size == strlen(value) && !strcmp(output, value), "Argument/Unicode/separate stderr capture failed");
    const char *failure[] = {bash, "-c", "printf diagnostic >&2; exit 7", "capture-test"};
    TOOL_REQUIRE(!tool_capture(failure, 4, output, sizeof(output), &size), "Nonzero ordinary child accepted"); return 1;
}
static int validation_replacements(void) {
    char text[64] = "anchor middle anchor";
    TOOL_REQUIRE(tool_replace(text, sizeof(text), "anchor", "replacement", 2) && !strcmp(text, "anchor middle replacement"), "Second duplicate replacement wrong");
    strcpy(text, "anchor anchor anchor"); TOOL_REQUIRE(!tool_replace(text, sizeof(text), "anchor", "replacement", 2) && !strcmp(text, "anchor anchor anchor"), "Third duplicate accepted/mutated");
    strcpy(text, "anchor anchor"); TOOL_REQUIRE(!tool_replace(text, sizeof(text), "anchor", "replacement", 1) && !strcmp(text, "anchor anchor"), "Nonunique strict anchor accepted/mutated");
    strcpy(text, "anchor"); TOOL_REQUIRE(!tool_replace(text, sizeof(text), "missing", "x", 1) && !strcmp(text, "anchor"), "Missing anchor mutated");
    TOOL_REQUIRE(!tool_replace(text, 8, "anchor", "too large", 1) && !strcmp(text, "anchor"), "Patch capacity failure mutated input"); return 1;
}
static int validation_generated(void) {
    const char *names[] = {"ImDrawTextFlags", "ImTextureRef", "ImVec2", "ImVec2i", "ImVec4", "ImColor", "ImRect"};
    tool_bytes[0][0] = 0;
    for (unsigned i = 0; i < 7; ++i) { char declaration[128]; snprintf(declaration, sizeof(declaration), i ? "typedef struct %s_c %s;\n" : "typedef int %s;\n", names[i], names[i]); strcat(tool_bytes[0], declaration); strcat(tool_bytes[0], declaration); }
    strcat(tool_bytes[0], "union { int val_i; float val_f; void* val_p; };\nunion { int BackupInt[2]; float BackupFloat[2]; };\nImGuiInputEventAppFocused AppFocused;\n    };\n");
    strcpy(tool_bytes[2], tool_bytes[0]);
    TOOL_REQUIRE(tool_fix_generated_text() && strstr(tool_bytes[0], "} value;") && strstr(tool_bytes[0], "} backup;") && strstr(tool_bytes[0], "} event;"), "Generated C99 transformations missing");
    for (unsigned i = 0; i < 7; ++i) { char declaration[128]; snprintf(declaration, sizeof(declaration), i ? "typedef struct %s_c %s;" : "typedef int %s;", names[i], names[i]); char *first = strstr(tool_bytes[0], declaration); TOOL_REQUIRE(first && !strstr(first + strlen(declaration), declaration), "Generated duplicate count wrong"); }
    TOOL_REQUIRE(!tool_fix_generated_text(), "Already-fixed input accepted without exact original counts");
    strcpy(tool_bytes[0], tool_bytes[2]); strcat(tool_bytes[0], "typedef int ImDrawTextFlags;\n"); TOOL_REQUIRE(!tool_fix_generated_text(), "Third typedef drift accepted");
    strcpy(tool_bytes[0], tool_bytes[2]); TOOL_REQUIRE(tool_replace(tool_bytes[0], sizeof(tool_bytes[0]), "float val_f", "float changed", 1) && !tool_fix_generated_text(), "Union anchor drift accepted"); return 1;
}
static int validation_patches(void) {
    char folder[NATIVE_PATH_SIZE], source[NATIVE_PATH_SIZE], patch[NATIVE_PATH_SIZE], hash[65], after[65];
    TOOL_REQUIRE(validation_path(folder, "repository") && file_directory_create(folder, 1) && tool_path(source, folder, "input.c") && file_create_text((cstring){source}, "old\n") == file_error_ok, "Cannot create patch fixture");
    const char *initialize[] = {"git", "init", "--template=", "-q", folder}; TOOL_REQUIRE(tool_command(initialize, 5), "Cannot initialize owned fixture repo");
    const char *configure[] = {"git", "-C", folder, "config", "core.autocrlf", "false"}; TOOL_REQUIRE(tool_command(configure, 6), "Cannot configure fixture line endings");
    const char *add[] = {"git", "-C", folder, "add", "input.c"}; TOOL_REQUIRE(tool_command(add, 5), "Cannot index fixture source");
    const char *commit[] = {"git", "-C", folder, "-c", "user.name=Native Tool Test", "-c", "user.email=native-test@example.invalid", "-c", "commit.gpgsign=false", "-c", "core.hooksPath=.git/native-hooks", "commit", "-q", "-m", "Owned patch fixture"};
    TOOL_REQUIRE(tool_command(commit, 15) && validation_path(patch, "patch.json") && file_create_text((cstring){patch}, "[{\"path\":\"input.c\",\"operations\":[{\"prefix\":\"prefix\\n\"},{\"before\":\"old\",\"after\":\"new\"}]}]") == file_error_ok, "Cannot commit/configure isolated patch fixture");
    TOOL_REQUIRE(!tool_apply_patch(folder, patch, 1), "Offline verification accepted missing patch");
    TOOL_REQUIRE(tool_apply_patch(folder, patch, 0) && tool_sha256(source, hash) && tool_apply_patch(folder, patch, 0) && tool_sha256(source, after) && !strcmp(hash, after) && tool_apply_patch(folder, patch, 1), "Patch/idempotence/offline verification failed");
    usize size; TOOL_REQUIRE(tool_read(source, tool_bytes[2], sizeof(tool_bytes[2]), &size) && !strcmp(tool_bytes[2], "prefix\nnew\n"), "Patch byte output differs");
    TOOL_REQUIRE(file_write_bytes((cstring){source}, "unknown edit\n", sizeof("unknown edit\n") - 1) == file_error_ok && tool_sha256(source, hash) && !tool_apply_patch(folder, patch, 0) && tool_sha256(source, after) && !strcmp(hash, after), "Unknown source edit overwritten");
    const char bytes[] = "old\0hidden"; TOOL_REQUIRE(file_write_bytes((cstring){source}, bytes, sizeof(bytes) - 1) == file_error_ok && tool_sha256(source, hash) && !tool_apply_patch(folder, patch, 0) && tool_sha256(source, after) && !strcmp(hash, after), "NUL-tail source edit overwritten"); return 1;
}
static void validation_tsv(void) {
    usize count = (usize)snprintf(coordinator_output, sizeof(coordinator_output), "kind\tstatus\tsource\tname\nsuite\t10\t\t\n");
    for (unsigned i = 0; i < 10; ++i) count += (usize)snprintf(coordinator_output + count, sizeof(coordinator_output) - count, "test\tok\t%s\t%s\n", coordinator_source(i), coordinator_names[i]);
    count += (usize)snprintf(coordinator_output + count, sizeof(coordinator_output) - count, "complete\t0\t10\t10\n"); coordinator_result.output.len = count;
    strcpy(coordinator_errors, "diagnostic\nmodel-checks:193\n"); coordinator_result.error_output.len = strlen(coordinator_errors);
}
static int validation_results(void) {
    unsigned order[10]; int checks;
    validation_tsv(); TOOL_REQUIRE(coordinator_validate(order, &checks) && checks == 193, "Valid GUI completion rejected");
    for (unsigned i = 0; i < 10; ++i) TOOL_REQUIRE(order[i] == i, "GUI case identity order wrong");
    const char *before[] = {"suite\t10", "test\tok", "complete\t0\t10\t10", "tests/gui/integration/checks.c", "production_render_abi_screenshot_without_workspace_writes"};
    const char *after[] = {"suite\t9", "test\tfail", "complete\t0\t9\t10", "unknown/source.c", "existing_c_document_selection_ipc_model"};
    for (unsigned i = 0; i < 5; ++i) {
        validation_tsv(); char *at = strstr(coordinator_output, before[i]); TOOL_REQUIRE(at, "Missing mutation anchor"); usize old = strlen(before[i]), replacement = strlen(after[i]); memmove(at + replacement, at + old, strlen(at + old) + 1); memcpy(at, after[i], replacement); coordinator_result.output.len = strlen(coordinator_output);
        TOOL_REQUIRE(!coordinator_validate(order, &checks), "Corrupt GUI result accepted");
    }
    validation_tsv(); coordinator_output[coordinator_result.output.len - 1] = 0; --coordinator_result.output.len; TOOL_REQUIRE(!coordinator_validate(order, &checks), "Incomplete TSV accepted");
    validation_tsv(); coordinator_output[10] = 0; TOOL_REQUIRE(!coordinator_validate(order, &checks), "NUL TSV accepted");
    const char *markers[] = {"", "model-checks:0\n", "model-checks:-1\n", "model-checks:2147483648\n", "model-checks:193bad\n", "model-checks:1\nmodel-checks:1\n", "model-checks:\n"};
    for (unsigned i = 0; i < sizeof(markers)/sizeof(markers[0]); ++i) { validation_tsv(); strcpy(coordinator_errors, markers[i]); coordinator_result.error_output.len = strlen(coordinator_errors); TOOL_REQUIRE(!coordinator_validate(order, &checks), "Bad/duplicate model count accepted"); }
    validation_tsv(); strcpy(coordinator_errors, "model-checks:2147483647\n"); coordinator_result.error_output.len = strlen(coordinator_errors); TOOL_REQUIRE(coordinator_validate(order, &checks) && checks == INT_MAX, "Valid signed-int model count rejected"); return 1;
}
static int validation_ppm_write(const char *path, unsigned width, unsigned height, int colorful, int markers) {
    FILE *file = file_open_utf8(path, "wb"); TOOL_REQUIRE(file, "Cannot open P6 fixture");
    int ok = fprintf(file, "P6\n%u %u\n255\n", width, height) > 0;
    for (unsigned y = 0; ok && y < height; ++y) for (unsigned x = 0; ok && x < width; ++x) {
        u8 pixel[3] = {(u8)(colorful ? x : 0), (u8)(colorful ? y : 0), (u8)(colorful ? x + y : 0)};
        if (x == 20u * width / 640u && y == 20u * height / 480u) { pixel[0] = markers ? 255 : 254; pixel[1] = pixel[2] = 0; }
        if (x == 20u * width / 640u && y == 460u * height / 480u) { pixel[0] = pixel[1] = 0; pixel[2] = 255; }
        if (fwrite(pixel, 1, 3, file) != 3) ok = 0;
    }
    if (fclose(file)) ok = 0; return ok;
}
static int validation_ppm(void) {
    const char *valid = "{\"ok\":true,\"frames\":4,\"image_seen\":true,\"live_images\":0}";
    TOOL_REQUIRE(bridge_receipt(valid, strlen(valid)), "Valid bridge receipt rejected");
    const char nul_tail[] = "{\"ok\":true,\"frames\":4,\"image_seen\":true,\"live_images\":0}\0hidden";
    TOOL_REQUIRE(!bridge_receipt(nul_tail, sizeof(nul_tail) - 1), "NUL-tail bridge receipt accepted");
    const char *invalid[] = {"{}", "{\"ok\":true,\"frames\":3,\"image_seen\":true,\"live_images\":0}", "{\"ok\":true,\"frames\":4,\"image_seen\":false,\"live_images\":0}", "{\"ok\":true,\"frames\":4,\"image_seen\":true,\"live_images\":1}", "{\"ok\":true,\"frames\":4,\"image_seen\":true,\"live_images\":0,\"extra\":1}"};
    for (unsigned i = 0; i < sizeof(invalid)/sizeof(invalid[0]); ++i) TOOL_REQUIRE(!bridge_receipt(invalid[i], strlen(invalid[i])), "Invalid bridge ABI/lifetime receipt accepted");
    char path[NATIVE_PATH_SIZE]; bridge_image image = {0}; TOOL_REQUIRE(validation_path(path, "screenshot.ppm"), "No screenshot fixture path");
    TOOL_REQUIRE(validation_ppm_write(path, 640, 480, 1, 1) && bridge_screenshot(path, &image) && image.colors > 32 && image.width == 640 && image.height == 480, "Valid bridge P6 rejected");
    TOOL_REQUIRE(validation_ppm_write(path, 1280, 960, 1, 1) && bridge_screenshot(path, &image), "Retina bridge P6 rejected");
    TOOL_REQUIRE(validation_ppm_write(path, 640, 480, 1, 0) && !bridge_screenshot(path, &image), "Inexact orientation marker accepted");
    TOOL_REQUIRE(validation_ppm_write(path, 640, 480, 0, 1) && !bridge_screenshot(path, &image), "Clear-color bridge P6 accepted");
    TOOL_REQUIRE(validation_ppm_write(path, 640, 481, 1, 1) && !bridge_screenshot(path, &image), "Wrong bridge aspect ratio accepted");
    TOOL_REQUIRE(file_write_bytes((cstring){path}, "P6\n16385 480\n255\n", sizeof("P6\n16385 480\n255\n") - 1) == file_error_ok && !bridge_screenshot(path, &image), "Oversize bridge P6 accepted");
    TOOL_REQUIRE(file_write_bytes((cstring){path}, "P6\n640 480\n255\nshort", sizeof("P6\n640 480\n255\nshort") - 1) == file_error_ok && !bridge_screenshot(path, &image), "Truncated bridge P6 accepted"); return 1;
}
static int validation_child_capture(const char *root, const char *label, test_command_result *result) {
    char path[NATIVE_PATH_SIZE];
    TOOL_REQUIRE(snprintf(path, sizeof(path), "%s/%s.stdout", root, label) < (int)sizeof(path) && file_create_bytes((cstring){path}, result->output.data, result->output.len) == file_error_ok && snprintf(path, sizeof(path), "%s/%s.stderr", root, label) < (int)sizeof(path) && file_create_bytes((cstring){path}, result->error_output.data, result->error_output.len) == file_error_ok, "Cannot retain acquisition child captures"); return 1;
}
static int validation_local_acquisition(void) {
    char root[NATIVE_PATH_SIZE], logs[NATIVE_PATH_SIZE], path[NATIVE_PATH_SIZE];
    TOOL_REQUIRE(validation_path(root, "local-acquisition") && file_directory_create(root, 1) && tool_path(logs, root, "logs") && file_directory_create(logs, 1), "Cannot create isolated acquisition fixture");
    const char *directories[] = {"tools", "tools/native", "tools/native/patches", "vendor", "vendor/gui"};
    for (unsigned i = 0; i < sizeof(directories)/sizeof(directories[0]); ++i) TOOL_REQUIRE(tool_path(path, root, directories[i]) && file_directory_create(path, 1), "Cannot create owned acquisition support directory");
    const char *inputs[] = {"tools/capture-command.sh", "tools/native/patches/imgui.json", "tools/native/patches/richmd.json", "vendor/gui/preview-selection.cpp.inc"};
    for (unsigned i = 0; i < sizeof(inputs)/sizeof(inputs[0]); ++i) { usize size; TOOL_REQUIRE(tool_read(inputs[i], tool_bytes[2], sizeof(tool_bytes[2]), &size) && tool_path(path, root, inputs[i]) && file_create_bytes((cstring){path}, tool_bytes[2], size) == file_error_ok, "Cannot copy acquisition support input"); }
    enum { CONFIGURATION_CAPACITY = 6 };
    char keys[CONFIGURATION_CAPACITY * 2][64], values[CONFIGURATION_CAPACITY * 2][4096], total[16];
    test_command_environment environment[CONFIGURATION_CAPACITY * 2 + 4] = {{"ATTENDANT_TOOL_DIRECTORY", logs}, {"GIT_ALLOW_PROTOCOL", "file"}, {"GIT_TERMINAL_PROMPT", "0"}};
    usize count = sizeof(tool_dependencies)/sizeof(tool_dependencies[0]); TOOL_REQUIRE(count <= CONFIGURATION_CAPACITY, "Acquisition test configuration capacity exceeded"); snprintf(total, sizeof(total), "%zu", count); environment[3] = (test_command_environment){"GIT_CONFIG_COUNT", total};
    for (usize i = 0; i < count; ++i) {
        char relative[NATIVE_PATH_SIZE], source[NATIVE_PATH_SIZE], uri[3079];
        TOOL_REQUIRE(tool_path(relative, "build/deps", tool_dependencies[i][0]) && file_tree_path((cstring){relative}, source, 0) == file_error_ok, "Missing local pinned mirror for acquisition test");
        usize used = 0; const char *prefix = source[0] == '/' ? "file://" : "file:///"; strcpy(uri, prefix); used = strlen(uri);
        for (const u8 *at = (const u8 *)source; *at; ++at) {
            TOOL_REQUIRE(used + 4 < sizeof(uri), "Local mirror URI exceeds capacity");
            if ((*at >= 'a' && *at <= 'z') || (*at >= 'A' && *at <= 'Z') || (*at >= '0' && *at <= '9') || strchr("/:._-", *at)) uri[used++] = (char)*at;
            else { static const char hex[] = "0123456789ABCDEF"; uri[used++] = '%'; uri[used++] = hex[*at >> 4]; uri[used++] = hex[*at & 15]; }
        }
        uri[used] = 0;
        snprintf(keys[i * 2], sizeof(keys[0]), "GIT_CONFIG_KEY_%zu", i); snprintf(values[i * 2], sizeof(values[0]), "url.%s.insteadOf", uri);
        snprintf(keys[i * 2 + 1], sizeof(keys[0]), "GIT_CONFIG_VALUE_%zu", i); snprintf(values[i * 2 + 1], sizeof(values[0]), "%s", tool_dependencies[i][1]);
        environment[i * 2 + 4] = (test_command_environment){keys[i * 2], values[i * 2]}; environment[i * 2 + 5] = (test_command_environment){keys[i * 2 + 1], values[i * 2 + 1]};
    }
    const char *arguments[] = {validation_tool, "prepare"}; test_command_result result;
    test_command command = {.arguments = arguments, .argument_count = 2, .working_directory = root, .environment = environment, .environment_count = count * 2 + 4, .timeout_ms = 180000};
    test_command_error error = test_command_run(&coordinator_commands, &command, &result);
    TOOL_REQUIRE(validation_child_capture(root, "prepare", &result) && error == test_command_ok && result.exited && result.exit_code == 0 && integration_json_parse(0, result.output.data, result.output.len) == 0 && integration_json_get(0, "prepared") && integration_json_get(0, "prepared")->type == PDJSON_TRUE, "Cold local-only pinned acquisition failed");
    /* Each helper invocation gets a fresh capture directory, like bootstrap. */
    TOOL_REQUIRE(tool_path(logs, root, "verify-logs") && file_directory_create(logs, 1), "Cannot create fresh offline-verification captures");
    const char *verify[] = {validation_tool, "prepare", "--verify"}; command.arguments = verify; command.argument_count = 3;
    error = test_command_run(&coordinator_commands, &command, &result);
    TOOL_REQUIRE(validation_child_capture(root, "verify", &result) && error == test_command_ok && result.exited && result.exit_code == 0 && integration_json_parse(0, result.output.data, result.output.len) == 0 && integration_json_get(0, "verified") && integration_json_get(0, "verified")->type == PDJSON_TRUE, "Fresh locally acquired pins/patches do not verify offline"); return 1;
}
static int validation_provenance(void) {
    tool_path_count = 0;
    TOOL_REQUIRE(tool_native_inputs() && tool_hash_paths(), "Cannot inventory/hash native tooling inputs");
    const char *required[] = {"tools/build-inputs.sh", "tools/build-platform.sh", "tools/native-tool.c", "tools/native/support.h", "tools/native/patches/imgui.json", "src/native-api.c", "src/core/core.c", "src/include/base.h", "tests/support/integration-json.h", "vendor/sqlite/sqlite3.c", "vendor/libyaml/src/api.c", "vendor/libpdjson5/pdjson5.c", "vendor/gui/preview-selection.cpp.inc", "vendor/gui/richmd_c.h", "vendor/gui/find-text.h"};
    for (usize i = 0; i < sizeof(required) / sizeof(required[0]); ++i) {
        usize found = 0;
        for (usize j = 0; j < tool_path_count; ++j) if (!strcmp(required[i], tool_paths[j])) ++found;
        TOOL_REQUIRE(found == 1, "Missing/duplicate native tooling provenance input");
    }
    TOOL_REQUIRE(tool_gui_inputs(), "Cannot inventory GUI inputs");
    for (usize i = 0; i < tool_path_count; ++i)
        TOOL_REQUIRE(strncmp(tool_paths[i], "tests/core/", 11), "Unrelated core tests remain in GUI provenance");
    char path[NATIVE_PATH_SIZE], root[NATIVE_PATH_SIZE], hash[65], records[4096];
    TOOL_REQUIRE(validation_path(path, "provenance") && file_create_text((cstring){path}, "abc") == file_error_ok && file_tree_path((cstring){"."}, root, 0) == file_error_ok && tool_sha256(path, hash), "Cannot create provenance fixture");
    usize prefix = strlen(root); TOOL_REQUIRE(!strncmp(path, root, prefix) && path[prefix] == '/', "Provenance fixture outside root"); const char *relative = path + prefix + 1;
    TOOL_REQUIRE(tool_relative(relative) && snprintf(records, sizeof(records), "{\"files\":[{\"path\":\"%s\",\"sha256\":\"%s\"}]}", relative, hash) < (int)sizeof(records), "Provenance fixture path invalid");
    TOOL_REQUIRE(integration_json_parse(0, records, strlen(records)) == 0 && tool_verify_records(0, "files"), "Valid binary provenance rejected");
    TOOL_REQUIRE(file_write_bytes((cstring){path}, "edited", 6) == file_error_ok && !tool_verify_records(0, "files"), "Changed binary provenance accepted");
    const char *invalid[] = {"{\"files\":[]}", "{\"files\":[{}]}", "{\"files\":[{\"path\":\"../outside\",\"sha256\":\"bad\"}]}", "{}"};
    for (unsigned i = 0; i < sizeof(invalid)/sizeof(invalid[0]); ++i) TOOL_REQUIRE(integration_json_parse(0, invalid[i], strlen(invalid[i])) == 0 && !tool_verify_records(0, "files"), "Invalid provenance record accepted");
    TOOL_REQUIRE(!tool_relative("../outside") && !tool_relative("build/../../outside") && !tool_relative("C:/outside") && !tool_relative("/outside"), "Escaping provenance path accepted"); return 1;
}
static int validation_cli(void) {
    const char *bad[] = {validation_tool, "prepare", "--verify", "extra"};
    test_command command = {.arguments = bad, .argument_count = 4, .timeout_ms = 10000}; test_command_result result;
    TOOL_REQUIRE(test_command_run(&coordinator_commands, &command, &result) == test_command_ok && result.exited && result.exit_code == 2 && integration_json_parse(0, result.output.data, result.output.len) == 0 && integration_text_is("code", "usage"), "Malformed tool arguments not rejected");
    char isolated[NATIVE_PATH_SIZE], child_logs[NATIVE_PATH_SIZE]; TOOL_REQUIRE(validation_path(isolated, "offline-missing") && file_directory_create(isolated, 1) && tool_path(child_logs, isolated, "logs") && file_directory_create(child_logs, 1), "Cannot create isolated offline fixture");
    const char *verify[] = {validation_tool, "prepare", "--verify"}; test_command_environment environment[] = {{"ATTENDANT_TOOL_DIRECTORY", child_logs}};
    command = (test_command){.arguments = verify, .argument_count = 3, .working_directory = isolated, .environment = environment, .environment_count = 1, .timeout_ms = 10000};
    TOOL_REQUIRE(test_command_run(&coordinator_commands, &command, &result) == test_command_ok && result.exited && result.exit_code == 1 && integration_json_parse(0, result.output.data, result.output.len) == 0 && integration_text_is("code", "tool-failed"), "Missing offline dependency not rejected");
    arena_reset(tool_files.memory); file_tree tree = {0}; TOOL_REQUIRE(file_directory_list_tree(&tool_files, (cstring){isolated}, &tree) == file_error_ok && tree.count == 1, "Offline verification acquired/modified missing dependency"); arena_reset(tool_files.memory);
    const char *hash[] = {validation_tool, "hash", "missing-file"}; command = (test_command){.arguments = hash, .argument_count = 3, .working_directory = isolated, .environment = environment, .environment_count = 1, .timeout_ms = 10000};
    TOOL_REQUIRE(test_command_run(&coordinator_commands, &command, &result) == test_command_ok && result.exited && result.exit_code == 1, "Missing binary hash input accepted"); return 1;
}
int main(void) {
    char source[NATIVE_PATH_SIZE], fixture[NATIVE_PATH_SIZE];
    if (!tool_initialize() || !tool_environment("ATTENDANT_NATIVE_TOOL_HELPER", source, sizeof(source)) || file_tree_path((cstring){source}, validation_tool, 0) != file_error_ok || !tool_temporary(fixture, ".validation") || test_directory_create(&validation_fixture, (cstring){fixture}) != file_error_ok) { fprintf(stderr, "Native-tool validation setup: %s\n", tool_problem); return 2; }
    coordinator_commands = (test_command_workspace){arena_from_buffer(coordinator_memory, sizeof(coordinator_memory)), coordinator_output, coordinator_errors, sizeof(coordinator_output), sizeof(coordinator_errors)};
    int (*tests[])(void) = {validation_hash, validation_capture, validation_replacements, validation_generated, validation_patches, validation_results, validation_ppm, validation_provenance, validation_cli, validation_local_acquisition};
    const char *names[] = {"binary SHA-256", "argv/separate captures", "strict replacements", "C99 generated fixes", "source patches/idempotence/drift", "exact GUI results", "bridge ABI receipt/P6 markers/aspect/colors", "provenance tamper/path/list refusal", "CLI/offline missing input", "cold local-only pinned acquisition"};
    unsigned passed = 0; int ok = 1;
    for (unsigned i = 0; i < sizeof(tests)/sizeof(tests[0]); ++i) { tool_problem[0] = 0; if (!tests[i]()) { fprintf(stderr, "%s: %s\n", names[i], tool_problem); ok = 0; break; } fprintf(stderr, "ok: %s\n", names[i]); ++passed; }
    /* Git creates read-only object files. Only relax flags in this exclusively
       owned disposable fixture, after every child has finished. */
    arena_reset(tool_files.memory);
    if (ok) {
        file_tree tree = {0}; if (file_directory_list_tree(&tool_files, (cstring){validation_fixture.path}, &tree) != file_error_ok) ok = 0;
        for (usize i = 0; ok && i < tree.count; ++i) if (tree.entries[i].kind == file_entry_regular) { char path[NATIVE_PATH_SIZE]; if (!tool_path(path, validation_fixture.path, tree.entries[i].path) || file_set_read_only((cstring){path}, 0) != file_error_ok) ok = 0; }
    }
    arena_reset(tool_files.memory); if (test_directory_finish(&validation_fixture, &tool_files, !ok) != file_error_ok) ok = 0;
    printf("{\"ok\":%s,\"groups\":%u}\n", ok ? "true" : "false", passed); return ok ? 0 : 1;
}
