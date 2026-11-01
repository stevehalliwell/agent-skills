/* Native CLI entry point. Shared core has no application entry point. */
#include "../../src/core/core.c"
#include "../../src/core/prepare.h"
#include "read.h"

int main(int argc, char **argv) {
    static unsigned char backing[128 * 1024 * 1024];
    static context c;
    const char *root, *state;
    int i;
    active_context = &c;
    c.memory = arena_from_buffer(backing, sizeof(backing));
    {
        LPWSTR *arguments = CommandLineToArgvW(GetCommandLineW(), &argc);
        if (!arguments) fail(&c, "Cannot read Windows command line");
        argv = allocate(&c, (argc + 1) * sizeof(char *));
        for (i = 0; i < argc; i++) argv[i] = (char *)utf8(&c, arguments[i]);
        LocalFree(arguments);
    }
    if (argc < 2 || !strcmp(argv[1], "--help") || !strcmp(argv[1], "-h")) return native_help();
    if (!strcmp(argv[1], "doctor") || !strcmp(argv[1], "validate") || !strcmp(argv[1], "add-table") || !strcmp(argv[1], "create") || !strcmp(argv[1], "update") ||
        !strcmp(argv[1], "sync") || !strcmp(argv[1], "schema") || !strcmp(argv[1], "query") || !strcmp(argv[1], "search"))
        return native_read_main(&c, argc, argv);
    if (argc != 2) { c.error_code = "cli-command"; fail(&c, "Expected a native command or one fixture-project path"); }
    root = full_path(&c, ".", argv[1]);
    state = prepare_projection(&c, root, PROJECTION_REBUILD);
    print_sync(&c, state);
    return 0;
}
