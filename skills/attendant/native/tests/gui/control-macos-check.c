/* Native real-viewer/Unix-socket acceptance. macOS runtime qualification pending. */
#include "tools/native/support.h"
#include "tools/native/manifests.h"
#include "test.h"
#if defined(__APPLE__)
#include "apps/gui/reveal.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <poll.h>
#include <signal.h>
static test_command_live control_children[3];
static u8 control_memory[262144];
static char control_output[3][1048577], control_errors[3][1048577];
static test_command_workspace control_workspaces[3];
static usize control_cursor[2];
static unsigned control_events[2];
static pid_t control_pids[2];
static int control_checks[2], control_markers[2], control_dirty[2];
static test_directory control_case;
static char control_root[NATIVE_PATH_SIZE], control_gui[2][NATIVE_PATH_SIZE], control_testing[2][NATIVE_PATH_SIZE], control_assets[NATIVE_PATH_SIZE];
static char control_fake_path[sizeof(((struct sockaddr_un *)0)->sun_path)];
static int control_fake = -1, control_peer = -1, control_raw = -1, control_fake_owned, control_fake_seen;
static unsigned control_command_number, control_groups;
static const char *control_names[16];
static int control_model_checks[16];
enum { CONTROL_READY = 1, CONTROL_INSPECTED = 2, CONTROL_RECEIVED = 4, CONTROL_COMPLETE = 8 };
static int control_integer(const char *path, int *out) {
    integration_json_node *node = integration_json_get(0, path); value number = {0};
    if (!node || node->type != PDJSON_NUMBER || parse_number(node->text, &number) != NUMBER_VALID || number.number_kind != NUMBER_I64 || number.number.signed_integer < 0 || number.number.signed_integer > INT_MAX) return 0;
    *out = (int)number.number.signed_integer; return 1;
}
static int control_capture(unsigned child) {
    char path[NATIVE_PATH_SIZE];
    TOOL_REQUIRE(snprintf(path, sizeof(path), "%s/command-%u-%u.stdout", control_case.path, ++control_command_number, child) < (int)sizeof(path) && file_create_bytes((cstring){path}, control_children[child].result.output.data, control_children[child].result.output.len) == file_error_ok, "Cannot retain control stdout");
    TOOL_REQUIRE(snprintf(path, sizeof(path), "%s/command-%u-%u.stderr", control_case.path, control_command_number, child) < (int)sizeof(path) && file_create_bytes((cstring){path}, control_children[child].result.error_output.data, control_children[child].result.error_output.len) == file_error_ok, "Cannot retain control stderr"); return 1;
}
static int control_tick(void) {
    for (unsigned i = 0; i < 3; ++i) if (control_children[i].active) TOOL_REQUIRE(test_command_poll(&control_children[i]) == test_command_ok, "Live control/viewer deadline or I/O failure");
    for (unsigned i = 0; i < 2; ++i) {
        test_command_result *result = &control_children[i].result;
        while (control_cursor[i] < result->output.len) {
            const char *line = (const char *)result->output.data + control_cursor[i], *end = memchr(line, '\n', result->output.len - control_cursor[i]); if (!end) break;
            TOOL_REQUIRE(integration_json_parse(0, line, (usize)(end - line)) == 0, "Invalid viewer event JSON");
            const char *event = integration_text("event");
            if (event) {
                unsigned flag = !strcmp(event, "ready") ? CONTROL_READY : !strcmp(event, "inspected") ? CONTROL_INSPECTED : !strcmp(event, "received") ? CONTROL_RECEIVED : !strcmp(event, "complete") ? CONTROL_COMPLETE : 0;
                control_events[i] |= flag;
                if (flag == CONTROL_COMPLETE) {
                    TOOL_REQUIRE(control_integer("checks", &control_checks[i]) && control_checks[i] > 0 && control_integer("markerDraws", &control_markers[i]), "Invalid control model count");
                    integration_json_node *dirty = integration_json_get(0, "dirty"); TOOL_REQUIRE(dirty && (dirty->type == PDJSON_TRUE || dirty->type == PDJSON_FALSE), "Invalid control dirty flag"); control_dirty[i] = dirty->type == PDJSON_TRUE;
                }
            }
            control_cursor[i] += (usize)(end - line) + 1;
        }
    }
    if (control_fake >= 0 && control_peer < 0) {
        control_peer = accept(control_fake, NULL, NULL);
        if (control_peer >= 0) TOOL_REQUIRE(fcntl(control_peer, F_SETFL, O_NONBLOCK) == 0, "Cannot make fake peer nonblocking");
        else TOOL_REQUIRE(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR, "Fake server accept failed");
    }
    if (control_peer >= 0) {
        u8 bytes[65536]; ssize_t count = recv(control_peer, bytes, sizeof(bytes), 0);
        if (count > 0) {
            uint32_t reply[] = {4, GUI_REVEAL_ALLOWED};
            TOOL_REQUIRE(send(control_peer, reply, sizeof(reply), 0) == (ssize_t)sizeof(reply), "Cannot send fake discovery reply");
            TOOL_REQUIRE(close(control_peer) == 0, "Cannot close fake peer"); control_peer = -1; control_fake_seen = 1;
        } else if (count < 0) TOOL_REQUIRE(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR, "Fake peer read failed");
        else { close(control_peer); control_peer = -1; }
    }
    return 1;
}
static int control_event(unsigned child, unsigned events) {
    while ((control_events[child] & events) != events) {
        TOOL_REQUIRE(control_tick(), "Viewer polling failed");
        TOOL_REQUIRE((control_events[child] & events) == events || control_children[child].active, "Viewer ended before required event"); test_command_sleep(1);
    }
    return 1;
}
static int control_done(unsigned child) {
    while (control_children[child].active) { TOOL_REQUIRE(control_tick(), "Viewer completion failed"); test_command_sleep(1); }
    TOOL_REQUIRE(control_children[child].result.error == test_command_ok && control_children[child].result.exited && control_children[child].result.exit_code == 0, "Viewer failed"); return 1;
}
static int control_fixture(const char *label, char project[NATIVE_PATH_SIZE], char baseline[NATIVE_PATH_SIZE]) {
    TOOL_REQUIRE(tool_path(project, control_case.path, label) && file_directory_copy(&tool_files, (cstring){"tests/fixtures/project"}, (cstring){project}) == file_error_ok, "Cannot create owned control fixture");
    char path[NATIVE_PATH_SIZE], text[32768]; usize used = 0;
    TOOL_REQUIRE(tool_path(path, project, ".gui-test-owned") && file_create_text((cstring){path}, "Owned native control fixture.\n") == file_error_ok, "Cannot mark owned fixture");
    for (unsigned document = 0; document < 2; ++document) {
        used = 0;
        for (unsigned i = 1; i <= 180; ++i) { int count = snprintf(text + used, sizeof(text) - used, document ? "Other document line %u.\n" : "Line %u: café Ω 😀 saved source.\n", i); TOOL_REQUIRE(count > 0 && (usize)count < sizeof(text) - used, "Fixture exceeds capacity"); used += (usize)count; }
        TOOL_REQUIRE(tool_path(path, project, document ? "other.md" : "README.md") && file_write_bytes((cstring){path}, text, used) == file_error_ok, "Cannot write control fixture");
    }
    TOOL_REQUIRE(test_directory_next(&control_case, "baseline", baseline) == file_error_ok && file_directory_copy(&tool_files, (cstring){project}, (cstring){baseline}) == file_error_ok, "Cannot snapshot control fixture"); return 1;
}
static int control_unchanged(const char *project, const char *baseline) {
    int equal = 0; TOOL_REQUIRE(file_directory_equal(&tool_files, (cstring){(char *)project}, (cstring){(char *)baseline}, &equal) == file_error_ok && equal, "Control changed source tree"); return 1;
}
static int control_viewer(unsigned child, unsigned compiler, const char *project, const char *mode, const char *frames) {
    const char *arguments[] = {control_testing[compiler], "--project", project, "--assets", control_assets, "--file", "README.md", "--frames", frames, "--exercise-reveal", mode};
    test_command command = {.arguments = arguments, .argument_count = 11, .timeout_ms = 20000};
    TOOL_REQUIRE(test_command_start(&control_children[child], &control_workspaces[child], &command) == test_command_ok, "Cannot start owned control viewer");
    control_pids[child] = control_children[child].process; return control_event(child, CONTROL_READY);
}
static int control_request(unsigned compiler, const char *project, const char *const *extra, usize count, unsigned long expected, int stale) {
    if (control_children[2].complete) TOOL_REQUIRE(control_capture(2), "Cannot retain prior client captures");
    const char *arguments[16] = {stale ? control_testing[compiler] : control_gui[compiler], "--project", project};
    TOOL_REQUIRE(count + 3 <= 16, "Control argv exceeds capacity"); for (usize i = 0; i < count; ++i) arguments[i + 3] = extra[i];
    test_command_environment hash = {"ATTENDANT_GUI_TEST_REVEAL_HASH", "0000000000000000000000000000000000000000000000000000000000000000"};
    test_command command = {.arguments = arguments, .argument_count = count + 3, .timeout_ms = 10000, .environment = stale ? &hash : NULL, .environment_count = stale ? 1 : 0};
    TOOL_REQUIRE(test_command_start(&control_children[2], &control_workspaces[2], &command) == test_command_ok, "Cannot start control client");
    while (control_children[2].active) { TOOL_REQUIRE(control_tick(), "Control request/viewer failed"); test_command_sleep(1); }
    TOOL_REQUIRE(control_children[2].result.error == test_command_ok && control_children[2].result.exited && control_children[2].result.exit_code == expected, "Unexpected control client exit");
    return integration_json_parse(0, control_children[2].result.output.data, control_children[2].result.output.len) == 0 || tool_error("Invalid control JSON");
}
#define CONTROL_REQUEST(compiler, project, status, ...) do { const char *extra[] = {__VA_ARGS__}; TOOL_REQUIRE(control_request((compiler), (project), extra, sizeof(extra)/sizeof(extra[0]), (status), 0), "Control request failed"); } while (0)
static int control_endpoint(char *out, usize capacity, pid_t process) {
    int count = snprintf(out, capacity, "/tmp/proto-pipes-%lu/attendant-gui-%ld", (unsigned long)getuid(), (long)process); return count > 0 && count < (int)capacity;
}
static int control_case_finish(int failed) {
    int ok = 1;
    for (unsigned i = 0; i < 3; ++i) { test_command_error error = test_command_finish(&control_children[i]); if (control_children[i].result.cleanup_failed || (!failed && error)) ok = 0; }
    if (control_raw >= 0) { if (close(control_raw)) ok = 0; control_raw = -1; }
    if (control_peer >= 0) { if (close(control_peer)) ok = 0; control_peer = -1; }
    if (control_fake >= 0) { if (close(control_fake)) ok = 0; control_fake = -1; }
    if (control_fake_owned) { if (unlink(control_fake_path)) ok = 0; control_fake_owned = 0; }
    for (unsigned i = 0; i < 3; ++i) if (control_children[i].complete && control_case.owned && !control_capture(i)) ok = 0;
    arena_reset(tool_files.memory); if (test_directory_finish(&control_case, &tool_files, failed || !ok) != file_error_ok) ok = 0; return ok;
}
static int control_case_begin(void) {
    memset(control_children, 0, sizeof(control_children)); memset(control_cursor, 0, sizeof(control_cursor)); memset(control_events, 0, sizeof(control_events)); memset(control_checks, 0, sizeof(control_checks)); memset(control_markers, 0, sizeof(control_markers)); memset(control_dirty, 0, sizeof(control_dirty)); control_command_number = 0; control_fake_seen = 0;
    char path[NATIVE_PATH_SIZE]; TOOL_REQUIRE(snprintf(path, sizeof(path), "%s/case-%u", control_root, control_groups + 1) < (int)sizeof(path) && test_directory_create(&control_case, (cstring){path}) == file_error_ok, "Cannot exclusively create control case"); return 1;
}
static int control_mode(unsigned compiler, const char *mode) {
    char project[NATIVE_PATH_SIZE], baseline[NATIVE_PATH_SIZE]; TOOL_REQUIRE(control_fixture("project café Ω", project, baseline) && control_viewer(0, compiler, project, mode, "120"), "Cannot start control fixture/viewer");
    unsigned other = 1 - compiler;
    CONTROL_REQUEST(other, project, 0, "--inspect");
    integration_json_node *dirty = integration_json_get(0, "document.dirty");
    const char *text = integration_text("selection.text");
    TOOL_REQUIRE(dirty && dirty->type == (!strcmp(mode, "dirty") ? PDJSON_TRUE : PDJSON_FALSE) && integration_text_is("selection.status", "exact") && integration_text_is("selection.view", "source") && text && strstr(text, "café Ω 😀") && integration_number_is("selection.range_start", 0), "Unexpected live selection/dirty state");
    usize characters = 0; for (const u8 *at = (const u8 *)text; *at; ++at) if ((*at & 0xc0) != 0x80) ++characters;
    TOOL_REQUIRE(integration_number_is("selection.range_end", (long long)characters), "Selection range is not scalar UTF-8 length");
    const char *reveal[] = {"--reveal", !strcmp(mode, "other") ? "other.md" : "README.md", "--line", "80"};
    TOOL_REQUIRE(control_request(other, project, reveal, 4, 0, !strcmp(mode, "stale")), "Reveal failed");
    TOOL_REQUIRE(integration_json_parse(1, "{\"ok\":true,\"hint_updated\":true,\"persistent\":false,\"focus_changed\":false}", 72) == 0, "Internal reveal expectation invalid");
    TOOL_REQUIRE(integration_json_equal(&integration_documents[0], integration_json_get(0, ""), &integration_documents[1], integration_json_get(1, ""), 0), "Unexpected reveal receipt");
    TOOL_REQUIRE(control_event(0, CONTROL_INSPECTED | CONTROL_RECEIVED | CONTROL_COMPLETE) && control_done(0), "Missing control interaction checks");
    TOOL_REQUIRE(control_dirty[0] == !strcmp(mode, "dirty") && (control_markers[0] > 0) == (!strcmp(mode, "clean") || !strcmp(mode, "other")), "Unexpected Go guard/marker result");
    char endpoint[108]; struct stat status; TOOL_REQUIRE(control_endpoint(endpoint, sizeof(endpoint), control_pids[0]) && lstat(endpoint, &status) && errno == ENOENT, "Viewer endpoint leaked after exit");
    return control_unchanged(project, baseline);
}
static int control_rejected(unsigned compiler) {
    char project[NATIVE_PATH_SIZE], baseline[NATIVE_PATH_SIZE], sibling[NATIVE_PATH_SIZE], sibling_baseline[NATIVE_PATH_SIZE], path[NATIVE_PATH_SIZE], alias[NATIVE_PATH_SIZE];
    TOOL_REQUIRE(control_fixture("project café Ω", project, baseline) && control_fixture("sibling", sibling, sibling_baseline) && control_viewer(0, compiler, project, "idle", "240"), "Cannot start rejected-control fixture"); unsigned other = 1 - compiler;
    CONTROL_REQUEST(other, sibling, 1, "--inspect"); TOOL_REQUIRE(integration_text_is("code", "gui-not-running"), "Unrelated viewer accepted");
    TOOL_REQUIRE(tool_path(path, sibling, "README.md"), "Sibling path exceeds capacity"); CONTROL_REQUEST(other, project, 1, "--reveal", path); TOOL_REQUIRE(integration_text_is("code", "reveal-not-allowed"), "Sibling path accepted");
    CONTROL_REQUEST(other, project, 1, "--reveal", "README.md", "--line", "999999"); TOOL_REQUIRE(integration_text_is("code", "unmapped-line"), "Out-of-range line accepted");
    CONTROL_REQUEST(other, project, 2, "--reveal", "README.md", "--line", "0"); TOOL_REQUIRE(integration_text_is("code", "usage"), "Line zero accepted");
    TOOL_REQUIRE(tool_path(path, project, "bare-cr.md") && file_create_text((cstring){path}, "a\rb") == file_error_ok, "Cannot create bare-CR fixture");
    CONTROL_REQUEST(other, project, 1, "--reveal", "bare-cr.md"); TOOL_REQUIRE(integration_text_is("code", "unmapped-line"), "Bare CR accepted");
    TOOL_REQUIRE(tool_path(path, project, "README.md") && tool_path(alias, project, "alias.md") && symlink(path, alias) == 0, "Cannot create owned application symlink");
    CONTROL_REQUEST(other, project, 1, "--reveal", "alias.md"); TOOL_REQUIRE(integration_text_is("code", "unsupported-path"), "Application symlink accepted");
    TOOL_REQUIRE(control_viewer(1, other, project, "idle", "90"), "Cannot start ambiguous viewer");
    CONTROL_REQUEST(other, project, 1, "--inspect"); TOOL_REQUIRE(integration_text_is("code", "gui-ambiguous"), "Ambiguous inspection accepted");
    CONTROL_REQUEST(other, project, 1, "--reveal", "README.md"); TOOL_REQUIRE(integration_text_is("code", "gui-ambiguous"), "Ambiguous reveal accepted");
    TOOL_REQUIRE(control_done(1) && control_done(0) && unlink(alias) == 0 && tool_path(path, project, "bare-cr.md") && file_remove((cstring){path}) == file_error_ok, "Cannot finish rejected-control fixture");
    return control_unchanged(project, baseline) && control_unchanged(sibling, sibling_baseline);
}
static int control_connect(const char *endpoint) {
    control_raw = socket(AF_UNIX, SOCK_STREAM, 0); TOOL_REQUIRE(control_raw >= 0 && fcntl(control_raw, F_SETFL, O_NONBLOCK) == 0, "Cannot create raw control client");
    struct sockaddr_un address = {0}; address.sun_family = AF_UNIX; TOOL_REQUIRE(strlen(endpoint) < sizeof(address.sun_path), "Raw endpoint exceeds capacity"); strcpy(address.sun_path, endpoint);
    int status = connect(control_raw, (struct sockaddr *)&address, sizeof(address)); TOOL_REQUIRE(!status || errno == EINPROGRESS, "Cannot connect raw client");
    u64 start; TOOL_REQUIRE(test_command_clock(&start), "No monotonic clock");
    while (status) { struct pollfd item = {control_raw, POLLOUT, 0}; if (poll(&item, 1, 0) > 0) { int error = 0; socklen_t size = sizeof(error); TOOL_REQUIRE(getsockopt(control_raw, SOL_SOCKET, SO_ERROR, &error, &size) == 0 && !error, "Raw connect failed"); break; } u64 now; TOOL_REQUIRE(test_command_clock(&now) && now - start < 5000 && control_tick(), "Raw connect deadline"); test_command_sleep(1); }
    return 1;
}
static int control_pause(unsigned milliseconds) { u64 start, now; TOOL_REQUIRE(test_command_clock(&start), "No monotonic clock"); do { TOOL_REQUIRE(control_tick() && test_command_clock(&now), "Viewer failed during fragmented write"); test_command_sleep(1); } while (now - start < milliseconds); return 1; }
static int control_raw_request(const char *endpoint, const void *payload, usize size, uint32_t advertised, int fragmented, u8 reply[16], usize *received, u64 *elapsed) {
    TOOL_REQUIRE(control_connect(endpoint), "Raw connect failed"); u64 start, now; TOOL_REQUIRE(test_command_clock(&start), "No monotonic clock");
    u8 packet[sizeof(gui_reveal) + 8]; TOOL_REQUIRE(size + 4 <= sizeof(packet), "Raw packet exceeds capacity"); memcpy(packet, &advertised, 4); if (size) memcpy(packet + 4, payload, size);
    usize sent = 0, total = size + 4; *received = 0;
    while (sent < total) {
        usize wanted = total - sent; if (fragmented && sent < 2) wanted = 2 - sent; else if (fragmented && sent < 21) wanted = 21 - sent;
        ssize_t count = send(control_raw, packet + sent, wanted, 0);
        if (count > 0) { sent += (usize)count; if (fragmented && (sent == 2 || sent == 21)) TOOL_REQUIRE(control_pause(10), "Fragment pause failed"); }
        else TOOL_REQUIRE(count < 0 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR), "Raw write failed");
        TOOL_REQUIRE(control_tick() && test_command_clock(&now) && now - start < 5000, "Raw write deadline"); test_command_sleep(1);
    }
    for (;;) {
        u8 bytes[32]; ssize_t count = recv(control_raw, bytes, sizeof(bytes), 0);
        if (!count) break;
        if (count > 0) { TOOL_REQUIRE(*received + (usize)count <= 16, "Oversize raw reply"); memcpy(reply + *received, bytes, (usize)count); *received += (usize)count; }
        else TOOL_REQUIRE(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR, "Raw read failed");
        TOOL_REQUIRE(control_tick() && test_command_clock(&now) && now - start < 5000, "Raw read deadline"); test_command_sleep(1);
    }
    TOOL_REQUIRE(test_command_clock(&now) && close(control_raw) == 0, "Cannot close raw client"); control_raw = -1; *elapsed = now - start; return 1;
}
static int control_transport(unsigned compiler) {
    char project[NATIVE_PATH_SIZE], baseline[NATIVE_PATH_SIZE], endpoint[108]; TOOL_REQUIRE(control_fixture("project café Ω", project, baseline) && control_viewer(0, compiler, project, "idle", "360") && control_endpoint(endpoint, sizeof(endpoint), control_pids[0]), "Cannot start framed transport fixture");
    struct { uint32_t operation; gui_reveal hint; } request = {0}; request.operation = GUI_REVEAL_QUERY; request.hint.version = 1; snprintf(request.hint.project, sizeof(request.hint.project), "%s", project);
    u8 reply[16]; usize received; u64 elapsed; uint32_t header, operation;
    TOOL_REQUIRE(control_raw_request(endpoint, &request, sizeof(request), sizeof(request), 1, reply, &received, &elapsed) && received == 8, "Fragmented query failed"); memcpy(&header, reply, 4); memcpy(&operation, reply + 4, 4); TOOL_REQUIRE(header == 4 && operation == GUI_REVEAL_MATCH, "Invalid framed match reply");
    request.operation = 7; TOOL_REQUIRE(control_raw_request(endpoint, &request, sizeof(request), sizeof(request), 0, reply, &received, &elapsed) && received == 8, "Unknown request framing failed"); memcpy(&operation, reply + 4, 4); TOOL_REQUIRE(operation == 0, "Unknown operation accepted");
    u8 short_payload = 1; TOOL_REQUIRE(control_raw_request(endpoint, &short_payload, 1, 1, 0, reply, &received, &elapsed) && !received, "Short request accepted");
    TOOL_REQUIRE(control_raw_request(endpoint, NULL, 0, sizeof(request) + 1, 0, reply, &received, &elapsed) && !received, "Oversize request accepted");
    TOOL_REQUIRE(control_connect(endpoint), "Cannot connect silent client"); u64 start, now; TOOL_REQUIRE(test_command_clock(&start), "No monotonic clock");
    for (;;) { u8 byte; ssize_t count = recv(control_raw, &byte, 1, 0); if (!count) break; TOOL_REQUIRE(count < 0 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR), "Unexpected silent-client response"); TOOL_REQUIRE(control_tick() && test_command_clock(&now) && now - start < 4000, "Silent-client deadline exceeded"); test_command_sleep(1); }
    TOOL_REQUIRE(test_command_clock(&now) && now - start >= 1500 && now - start < 4000 && close(control_raw) == 0, "Silent-client timeout outside expected bounds"); control_raw = -1;
    CONTROL_REQUEST(1 - compiler, project, 0, "--inspect"); integration_json_node *dirty = integration_json_get(0, "document.dirty"); TOOL_REQUIRE(dirty && dirty->type == PDJSON_FALSE, "Reconnect failed");
    return control_done(0) && control_unchanged(project, baseline);
}
static int control_endpoint_policy(unsigned compiler) {
    char project[NATIVE_PATH_SIZE], baseline[NATIVE_PATH_SIZE], target[108];
    TOOL_REQUIRE(control_fixture("project café Ω", project, baseline) && control_viewer(0, compiler, project, "idle", "150") && control_endpoint(control_fake_path, sizeof(control_fake_path), getpid()), "Cannot start endpoint-policy fixture");
    struct stat status; TOOL_REQUIRE(lstat(control_fake_path, &status) && errno == ENOENT, "Fake endpoint already exists; refusing adoption");
    control_fake = socket(AF_UNIX, SOCK_STREAM, 0); TOOL_REQUIRE(control_fake >= 0 && fcntl(control_fake, F_SETFL, O_NONBLOCK) == 0, "Cannot create fake server");
    struct sockaddr_un address = {0}; address.sun_family = AF_UNIX; strcpy(address.sun_path, control_fake_path);
    TOOL_REQUIRE(bind(control_fake, (struct sockaddr *)&address, sizeof(address)) == 0, "Cannot exclusively bind fake endpoint"); control_fake_owned = 1;
    TOOL_REQUIRE(listen(control_fake, 4) == 0 && chmod(control_fake_path, 0600) == 0, "Cannot listen on private fake endpoint");
    CONTROL_REQUEST(1 - compiler, project, 1, "--inspect"); TOOL_REQUIRE(control_fake_seen && integration_text_is("code", "gui-unavailable"), "Unexpected discovery status accepted");
    TOOL_REQUIRE(chmod(control_fake_path, 0666) == 0, "Cannot set unsafe endpoint permissions"); CONTROL_REQUEST(1 - compiler, project, 1, "--inspect"); TOOL_REQUIRE(integration_text_is("code", "gui-unavailable"), "Unsafe endpoint accepted");
    TOOL_REQUIRE(close(control_fake) == 0, "Cannot close fake listener"); control_fake = -1; TOOL_REQUIRE(unlink(control_fake_path) == 0, "Cannot remove owned fake endpoint"); control_fake_owned = 0;
    TOOL_REQUIRE(control_endpoint(target, sizeof(target), control_pids[0]) && symlink(target, control_fake_path) == 0, "Cannot create owned endpoint symlink"); control_fake_owned = 1;
    CONTROL_REQUEST(1 - compiler, project, 1, "--inspect"); TOOL_REQUIRE(integration_text_is("code", "gui-unavailable"), "Endpoint symlink accepted");
    TOOL_REQUIRE(unlink(control_fake_path) == 0, "Cannot remove endpoint symlink"); control_fake_owned = 0;
    CONTROL_REQUEST(1 - compiler, project, 0, "--inspect"); integration_json_node *dirty = integration_json_get(0, "document.dirty"); TOOL_REQUIRE(dirty && dirty->type == PDJSON_FALSE, "Valid viewer unusable after rejected endpoints");
    return control_done(0) && control_unchanged(project, baseline);
}
static int control_suite(void) {
    TOOL_REQUIRE(tool_verify_gui("cjit", 0) && tool_verify_gui("cjit", 1) && tool_verify_gui("zig", 0) && tool_verify_gui("zig", 1), "GUI provenance failed");
    char directory[NATIVE_PATH_SIZE], supplied[NATIVE_PATH_SIZE]; TOOL_REQUIRE(tool_gui_directory(directory) && tool_environment("ATTENDANT_CONTROL_DIRECTORY", supplied, sizeof(supplied)) && *supplied && file_tree_path((cstring){supplied}, control_root, 0) == file_error_ok && file_tree_path((cstring){"build/deps/richmd/assets"}, control_assets, 0) == file_error_ok, "Missing owned control directory/assets");
    for (unsigned i = 0; i < 2; ++i) {
        char path[NATIVE_PATH_SIZE]; snprintf(path, sizeof(path), "%s/attendant-gui-%s", directory, i ? "zig" : "cjit"); TOOL_REQUIRE(file_tree_path((cstring){path}, control_gui[i], 0) == file_error_ok, "Missing GUI executable"); snprintf(path, sizeof(path), "%s/attendant-gui-%s-check", directory, i ? "zig" : "cjit"); TOOL_REQUIRE(file_tree_path((cstring){path}, control_testing[i], 0) == file_error_ok, "Missing check GUI executable");
    }
    arena *scratch = arena_from_buffer(control_memory, sizeof(control_memory)); for (unsigned i = 0; i < 3; ++i) control_workspaces[i] = (test_command_workspace){scratch, control_output[i], control_errors[i], sizeof(control_output[i]), sizeof(control_errors[i])};
    for (unsigned compiler = 0; compiler < 2; ++compiler) for (unsigned group = 0; group < 8; ++group) {
        TOOL_REQUIRE(control_case_begin(), "Control case setup failed"); int ok = 0; const char *name = "";
        if (!group) { char absent[NATIVE_PATH_SIZE]; name = "absent viewer"; ok = tool_path(absent, control_case.path, "not-running") && file_directory_create(absent, 1); if (ok) { const char *args[] = {"--inspect"}; ok = control_request(1 - compiler, absent, args, 1, 1, 0) && integration_text_is("code", "gui-not-running"); } }
        else if (group < 5) { const char *modes[] = {"clean", "dirty", "stale", "other"}; name = modes[group - 1]; ok = control_mode(compiler, name); }
        else if (group == 5) { name = "rejected controls and ambiguous viewers"; ok = control_rejected(compiler); }
        else if (group == 6) { name = "endpoint policy and unexpected discovery"; ok = control_endpoint_policy(compiler); }
        else { name = "framed transport and silent-client deadline"; ok = control_transport(compiler); }
        int checks = control_checks[0]; int cleanup = control_case_finish(!ok);
        TOOL_REQUIRE(ok && cleanup, "Native macOS control case failed; retained fixture/captures"); control_names[control_groups] = name; control_model_checks[control_groups++] = checks;
    }
    return 1;
}
int main(int argc, char **argv) {
    if (argc == 2 && !strcmp(argv[1], "--help")) { puts("Usage: bash tools/check-control-macos.sh\nReal owned macOS viewers and Unix-socket acceptance; both compiler builds required. Stdout JSON; build/control-check-results.json."); return 0; }
    if (argc != 1) { puts("{\"ok\":false,\"code\":\"usage\"}"); return 2; }
    int ok = signal(SIGPIPE, SIG_IGN) != SIG_ERR && tool_initialize() && environment_set(tool_files.memory, "DYLD_LIBRARY_PATH", NULL) == environment_error_ok && control_suite();
    FILE *out = file_open_utf8("build/control-check-results.json", "wb");
    if (out) { fprintf(out, "{\"ok\":%s,\"platform\":\"darwin\",\"architecture\":", ok ? "true" : "false"); tool_json_string(out, tool_architecture()); fputs(",\"run\":", out); tool_json_string(out, control_root); fputs(",\"cases\":[", out);
        for (unsigned i = 0; i < control_groups; ++i) { if (i) fputc(',', out); fprintf(out, "{\"compiler\":\"%s\",\"name\":", i < 8 ? "cjit" : "zig"); tool_json_string(out, control_names[i]); fprintf(out, ",\"checks\":%d}", control_model_checks[i]); } fputs("]}\n", out); if (ferror(out)) ok = 0; if (fclose(out)) ok = 0;
    } else ok = 0;
    if (!ok) fprintf(stderr, "%s\n", tool_problem);
    printf("{\"ok\":%s,\"cases\":%u,\"run\":", ok ? "true" : "false", control_groups); tool_json_string(stdout, control_root); puts("}"); return ok ? 0 : 1;
}
#else
int main(void) { fputs("macOS control acceptance requires native macOS; not validated on Windows.\n", stderr); puts("{\"ok\":false,\"code\":\"unsupported-platform\"}"); return 2; }
#endif
