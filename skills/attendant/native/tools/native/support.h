#ifndef ATTENDANT_NATIVE_TOOL_SUPPORT_H
#define ATTENDANT_NATIVE_TOOL_SUPPORT_H
/* Project-local native tooling. Shared bounded JSON uses existing numeric API;
   ordinary commands use base.h, never test-owned supervision. */
#include <time.h>
#include "src/native-api.c"
#include "tests/support/integration-json.h"
#if defined(_WIN32)
/* Match core's minimal Win32 crypto declarations; CJIT does not consume full SDK bcrypt.h. */
typedef HANDLE BCRYPT_HASH_HANDLE;
__declspec(dllimport) LONG WINAPI BCryptCreateHash(BCRYPT_ALG_HANDLE, BCRYPT_HASH_HANDLE *, PUCHAR, ULONG, PUCHAR, ULONG, ULONG);
__declspec(dllimport) LONG WINAPI BCryptHashData(BCRYPT_HASH_HANDLE, PUCHAR, ULONG, ULONG);
__declspec(dllimport) LONG WINAPI BCryptFinishHash(BCRYPT_HASH_HANDLE, PUCHAR, ULONG, ULONG);
__declspec(dllimport) LONG WINAPI BCryptDestroyHash(BCRYPT_HASH_HANDLE);
#else
#include <CommonCrypto/CommonDigest.h>
#include <sys/utsname.h>
#endif

static char tool_problem[4096], tool_directory[NATIVE_PATH_SIZE];
static unsigned tool_sequence;
static char tool_bytes[3][8388609];
static u8 tool_memory[8388608];
static file_tree_workspace tool_files;
static int tool_error(const char *message) { snprintf(tool_problem, sizeof(tool_problem), "%s", message); return 0; }
#define TOOL_REQUIRE(condition, message) do { if (!(condition)) return tool_error(message); } while (0)
static int tool_environment(const char *name, char *out, usize capacity) {
    static u8 memory[131072]; usize size = 0;
    environment_error error = environment_get(arena_from_buffer(memory, sizeof(memory)), name, out, capacity, &size);
    if (error == environment_error_not_found) { if (capacity) out[0] = 0; return 1; }
    return error == environment_error_ok;
}
static int tool_initialize(void) {
    tool_files = (file_tree_workspace){arena_from_buffer(tool_memory, sizeof(tool_memory)), 32, 8192};
    TOOL_REQUIRE(tool_environment("ATTENDANT_TOOL_DIRECTORY", tool_directory, sizeof(tool_directory)) && tool_directory[0] && file_is_directory(tool_directory, 0), "Missing owned ATTENDANT_TOOL_DIRECTORY; use Bash entry point");
    return 1;
}
static int tool_path(char out[NATIVE_PATH_SIZE], const char *root, const char *relative) {
    return file_tree_join(out, root, relative) == file_error_ok;
}
static int tool_temporary(char out[NATIVE_PATH_SIZE], const char *suffix) {
    int count = snprintf(out, NATIVE_PATH_SIZE, "%s/%u%s", tool_directory, ++tool_sequence, suffix);
    return count > 0 && count < NATIVE_PATH_SIZE;
}
static int tool_read(const char *path, char *out, usize capacity, usize *size) {
    TOOL_REQUIRE(capacity && file_read_into((cstring){(char *)path}, out, capacity - 1, size) == file_error_ok, "Cannot read file or file exceeds tooling capacity");
    out[*size] = 0; return 1;
}
static int tool_copy_stream(FILE *out, const char *path);
/* Bash opens both captures itself. MSYS cannot reliably write base.h's Win32
   append-only log handle. Keep stderr bytes separately and append them after
   wait. base.h remains unchanged; no test supervision in ordinary build tools. */
static int tool_capture(const char *const *arguments, usize count, char *out, usize capacity, usize *size) {
    char path[NATIVE_PATH_SIZE], output[NATIVE_PATH_SIZE], errors[NATIVE_PATH_SIZE]; process_workspace process = {0};
    TOOL_REQUIRE(count + 4 <= PROCESS_MAX_ARGUMENTS && tool_temporary(path, ".command.log") && tool_temporary(output, ".stdout") && tool_temporary(errors, ".stderr") &&
        file_create_text((cstring){path}, "") == file_error_ok && file_create_text((cstring){output}, "") == file_error_ok && file_create_text((cstring){errors}, "") == file_error_ok, "Cannot exclusively create command captures");
    char bash[NATIVE_PATH_SIZE];
    TOOL_REQUIRE(tool_environment("ATTENDANT_TOOL_BASH", bash, sizeof(bash)) && bash[0], "Missing explicit Bash path from native bootstrap");
    const char *wrapped[PROCESS_MAX_ARGUMENTS] = {bash, "tools/capture-command.sh", output, errors};
    for (usize i = 0; i < count; ++i) wrapped[i + 4] = arguments[i];
    int launched = process_run(&process, wrapped, count + 4, path);
    FILE *log = file_open_utf8(path, "ab"); int retained = log && tool_copy_stream(log, errors); if (log && fclose(log)) retained = 0;
    if (!launched || process.exit_code || !retained) {
        fprintf(stderr, "Command failed: %s (exit %lu, OS %lu); log: %s\n", arguments[0], process.exit_code, process.native_error, path);
        return tool_error("Native tool command failed; see retained command log");
    }
    return tool_read(output, out, capacity, size);
}
static int tool_command(const char *const *arguments, usize count) { usize size; return tool_capture(arguments, count, tool_bytes[2], sizeof(tool_bytes[2]), &size); }
static int tool_version(const char *command, const char *argument, char *out, usize capacity, usize *size) {
    char bash[NATIVE_PATH_SIZE]; TOOL_REQUIRE(tool_environment("ATTENDANT_TOOL_BASH", bash, sizeof(bash)) && *bash, "Missing explicit version-probe Bash");
    const char *arguments[] = {bash, "-c", "\"$@\" 2>&1", "native-version", command, argument};
    return tool_capture(arguments, 6, out, capacity, size);
}
static void tool_normalize(char *text, usize *size, int trim) {
    usize used = 0;
    for (usize i = 0; i < *size; ++i) if (!(text[i] == '\r' && i + 1 < *size && text[i + 1] == '\n')) text[used++] = text[i];
    if (trim) while (used && strchr(" \t\r\n\v\f", text[used - 1])) --used;
    text[used] = 0; *size = used;
}
static int tool_json_string(FILE *out, const char *text) {
    if (!text || !string_view_utf8_valid(string_view_from_cstr(text)) || fputc('"', out) == EOF) return 0;
    for (const u8 *p = (const u8 *)text; *p; ++p) {
        if (*p == '"' || *p == '\\') { if (fputc('\\', out) == EOF || fputc(*p, out) == EOF) return 0; }
        else if (*p < 32) { if (fprintf(out, "\\u%04x", (unsigned)*p) < 0) return 0; }
        else if (fputc(*p, out) == EOF) return 0;
    }
    return fputc('"', out) != EOF;
}
static const char *tool_text(unsigned slot, const char *path) {
    integration_json_node *node = integration_json_get(slot, path);
    return node && node->type == PDJSON_STRING ? node->text : NULL;
}
static int tool_json_read(unsigned slot, const char *path) {
    usize size;
    return tool_read(path, tool_bytes[slot], sizeof(tool_bytes[slot]), &size) && integration_json_parse(slot, tool_bytes[slot], size) == 0;
}
static int tool_json_node(FILE *out, unsigned slot, integration_json_node *node) {
    if (!node) return 0;
    if (node->type == PDJSON_STRING) return tool_json_string(out, node->text);
    if (node->type == PDJSON_NUMBER) return fputs(node->text, out) >= 0;
    if (node->type == PDJSON_TRUE || node->type == PDJSON_FALSE || node->type == PDJSON_NULL)
        return fputs(node->type == PDJSON_TRUE ? "true" : node->type == PDJSON_FALSE ? "false" : "null", out) >= 0;
    int object = node->type == PDJSON_OBJECT;
    if (!object && node->type != PDJSON_ARRAY) return 0;
    if (fputc(object ? '{' : '[', out) == EOF) return 0;
    for (int child = node->first, first = 1; child >= 0; child = integration_documents[slot].nodes[child].next) {
        integration_json_node *value = &integration_documents[slot].nodes[child];
        if (!first && fputc(',', out) == EOF) return 0; first = 0;
        if (object && (!tool_json_string(out, value->name) || fputc(':', out) == EOF)) return 0;
        if (!tool_json_node(out, slot, value)) return 0;
    }
    return fputc(object ? '}' : ']', out) != EOF;
}
typedef struct tool_hash {
#if defined(_WIN32)
    BCRYPT_ALG_HANDLE algorithm; BCRYPT_HASH_HANDLE hash;
#else
    CC_SHA256_CTX context;
#endif
} tool_hash;
static int tool_hash_begin(tool_hash *hash) {
    memset(hash, 0, sizeof(*hash));
#if defined(_WIN32)
    if (BCryptOpenAlgorithmProvider(&hash->algorithm, L"SHA256", NULL, 0) < 0) return 0;
    if (BCryptCreateHash(hash->algorithm, &hash->hash, NULL, 0, NULL, 0, 0) < 0) { BCryptCloseAlgorithmProvider(hash->algorithm, 0); hash->algorithm = NULL; return 0; }
    return 1;
#else
    return CC_SHA256_Init(&hash->context) != 0;
#endif
}
static int tool_hash_update(tool_hash *hash, const void *data, usize size) {
    for (usize offset = 0; offset < size;) {
        usize count = size - offset > 16384 ? 16384 : size - offset;
#if defined(_WIN32)
        if (BCryptHashData(hash->hash, (PUCHAR)((const u8 *)data + offset), (ULONG)count, 0) < 0) return 0;
#else
        if (!CC_SHA256_Update(&hash->context, (const u8 *)data + offset, (CC_LONG)count)) return 0;
#endif
        offset += count;
    }
    return 1;
}
static int tool_hash_finish(tool_hash *hash, char text[65], int ok) {
    u8 bytes[32];
#if defined(_WIN32)
    if (ok && BCryptFinishHash(hash->hash, bytes, 32, 0) < 0) ok = 0;
    if (hash->hash && BCryptDestroyHash(hash->hash) < 0) ok = 0;
    if (hash->algorithm && BCryptCloseAlgorithmProvider(hash->algorithm, 0) < 0) ok = 0;
#else
    if (ok && !CC_SHA256_Final(bytes, &hash->context)) ok = 0;
#endif
    if (ok) { for (unsigned i = 0; i < 32; ++i) snprintf(text + 2 * i, 3, "%02x", bytes[i]); text[64] = 0; }
    return ok;
}
static int tool_sha256(const char *path, char text[65]) {
    FILE *file = file_open_utf8(path, "rb"); TOOL_REQUIRE(file, "Cannot open file for SHA-256");
    tool_hash hash; int ok = tool_hash_begin(&hash);
    if (ok) {
        u8 bytes[16384]; usize size;
        while ((size = fread(bytes, 1, sizeof(bytes), file))) if (!tool_hash_update(&hash, bytes, size)) { ok = 0; break; }
        if (ferror(file)) ok = 0;
        ok = tool_hash_finish(&hash, text, ok);
    }
    if (fclose(file)) ok = 0;
    return ok || tool_error("SHA-256 read/hash/cleanup failed");
}
static const char *tool_platform(void) {
#if defined(_WIN32)
    return "win32";
#else
    return "darwin";
#endif
}
static const char *tool_architecture(void) {
#if defined(_M_ARM64) || defined(__aarch64__)
    return "arm64";
#else
    return "x64";
#endif
}
static int tool_timestamp(char out[32]) {
    time_t now = time(NULL); struct tm *utc = gmtime(&now);
    return utc && strftime(out, 32, "%Y-%m-%dT%H:%M:%S.000Z", utc) != 0;
}
static int tool_copy_stream(FILE *out, const char *path) {
    FILE *input = file_open_utf8(path, "rb"); if (!input) return 0;
    u8 bytes[16384]; usize count; int ok = 1;
    while ((count = fread(bytes, 1, sizeof(bytes), input))) if (fwrite(bytes, 1, count, out) != count) { ok = 0; break; }
    if (ferror(input) || ferror(out)) ok = 0;
    if (fclose(input)) ok = 0; return ok;
}
#endif
