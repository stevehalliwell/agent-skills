/* Bootstrap with tools/native-tool.sh; all commands run from native root. */
#include "tools/native/support.h"
#include "tools/native/dependencies.h"
#include "tools/native/manifests.h"
int main(int argc, char **argv) {
    if (argc == 2 && !strcmp(argv[1], "--help")) {
        puts("Usage: bash tools/native-tool.sh COMMAND\n"
             "  prepare [--verify]       Explicit pinned acquisition/patch, or offline verification\n"
             "  fix-generated            Strict C99 cimgui generated-header fixes\n"
             "  stack-manifest           Record completed native stack build\n"
             "  verify-stack             Verify stack inputs and library\n"
             "  gui-manifest cjit|zig [--check]\n"
             "  verify-gui cjit|zig [--check]\n"
             "  hash FILE                Stream binary SHA-256 (JSON)\n"
             "Stdout: JSON. Stderr: diagnostics. Exit 0 success, 1 failure, 2 usage.\n"
             "Builds need installed compiler/Bash/Git, never Node or implicit downloads."); return 0;
    }
    int usage = argc < 2, ok = 0;
    if (!usage && tool_initialize()) {
        if (!strcmp(argv[1], "prepare") && (argc == 2 || (argc == 3 && !strcmp(argv[2], "--verify")))) ok = tool_prepare(argc == 3);
        else if (!strcmp(argv[1], "fix-generated") && argc == 2) ok = tool_fix_generated();
        else if (!strcmp(argv[1], "stack-manifest") && argc == 2) ok = tool_stack_manifest();
        else if (!strcmp(argv[1], "verify-stack") && argc == 2) ok = tool_verify_stack();
        else if ((!strcmp(argv[1], "gui-manifest") || !strcmp(argv[1], "verify-gui")) && (argc == 3 || (argc == 4 && !strcmp(argv[3], "--check"))) && (!strcmp(argv[2], "zig") || !strcmp(argv[2], "cjit")))
            ok = !strcmp(argv[1], "gui-manifest") ? tool_gui_manifest(argv[2], argc == 4) : tool_verify_gui(argv[2], argc == 4);
        else if (!strcmp(argv[1], "hash") && argc == 3) { char hash[65]; ok = tool_sha256(argv[2], hash); if (ok) printf("{\"ok\":true,\"sha256\":\"%s\"}\n", hash); }
        else usage = 1;
    }
    if (usage) { fprintf(stderr, "Usage: bash tools/native-tool.sh --help\n"); puts("{\"ok\":false,\"code\":\"usage\"}"); return 2; }
    if (!ok) { fprintf(stderr, "%s\nRetained tooling directory: %s\n", tool_problem, tool_directory); fputs("{\"ok\":false,\"code\":\"tool-failed\",\"message\":", stdout); tool_json_string(stdout, tool_problem); puts("}"); return 1; }
    if (strcmp(argv[1], "prepare") && strcmp(argv[1], "fix-generated") && strcmp(argv[1], "hash")) puts("{\"ok\":true}");
    return 0;
}
