/*
build.h - single-file C99 build declarations, generator, and native driver

README
Copy this header and base.h into a C project. No game library, project shell
script, or generated binding is required. Shared native filesystem/process calls
live in base.h. Hosting supports Windows and macOS. This is reusable C99 code,
not an incremental dependency graph. On macOS CJIT callers must supply SDK headers
through BUILD_SDK_INCLUDE (typically $(xcrun --show-sdk-path)/usr/include);
bootstrap CJIT calls also need -I "$BUILD_SDK_INCLUDE". CJIT output executables
require codesign --sign - before native arm64 execution; driver signs its targets.

Define BUILD_GENERATOR before including to expose source discovery and C driver
emission. Define BUILD_DRIVER to expose bounded process/filesystem helpers and
build_run(). The generated driver defines BUILD_DRIVER itself. Definitions are
static; include once per translation unit and choose only the features needed.
Define both for build_entry(workspace, argc, argv, project). build_project is
borrowed declaration data: command/default target, targets/count, header include
path, configuration option spelling, compiler/optimization/configuration defaults,
optional source discoveries, preparation hint and extra agent environment name.
The entry owns parsing, help, target/configuration resolution, discovery and
failure reporting. Project main only supplies caller-owned workspace and calls
build_entry. No project target/library names, setup commands or mode policy are
built in. Unsupported configuration names/compiler combinations fail against
actual target declarations; dependency-free targets reject explicit selection.
Generation and driver compilation always remain unoptimized CJIT; target compiler
is independent. build_orchestrate(workspace, options, project) is the lower-level
path after build_begin() and option parsing. Both paths return one human/JSON
result, including early phase failures. build_default_json() supports pipe/CI/
BUILD_AGENT detection. Lists of configuration defaults and discoveries terminate
with NULL and have BUILD_MAX_ITEMS capacity.

Minimal project (save as build.c beside this header and your own main.c):

    #define BUILD_GENERATOR
    #define BUILD_DRIVER
    #include "build.h"
    static const char *sources[] = {"main.c", NULL};
    static const char *flags[] = {"-std=c99", NULL};
    static const char *empty[] = {NULL};
    static const build_target targets[] = {
        {"hello", {sources, empty, flags, empty},
         "hello.exe", NULL, NULL, NULL},
    };
    static const build_project project = {
        .default_target = "hello", .targets = targets, .target_count = 1,
        .include_directory = ".",
    };
    int main(int argc, char **argv) {
        static build_workspace workspace;
        return build_entry(&workspace, argc, argv, &project);
    }

Example Windows/Git Bash bootstrap with installed CJIT and Zig on PATH:

    cjit -q -C '' -I . build.c -- --compiler zig

The entry creates a new driver/artifact run each time. For emission-only callers,
build_generate(stdout, targets, count) still emits a standalone C driver; compile
it and invoke with a fresh --run build/build-driver/<name>. CJIT's CLI is not GCC-compatible.
Supported compiler commands are CJIT (unoptimized only) and Zig cc (-O0/-O2).
Override executable paths with CJIT and ZIG. Tools must already be installed;
the header never downloads or installs them.

Declarations
- build_inputs: NULL-terminated C sources, include directories, flags, and library
  names. Sources are explicit, or filled by build_find_sources() from immediate
  lowercase .c files in declared roots. Discovery is sorted, nonrecursive, and
  does not follow reparse points. Missing/empty discovery fails explicitly.
- build_link_config: optional named, compiler-specific prepared link files,
  additional flags, and copies. First matching compiler entry is the default;
  --configuration selects a name. No library names/paths or release policy are
  built in. Configuration flags/files follow C sources on the compile/link line.
- build_copy: source path and destination filename inside the artifact directory.
  Use for DLLs or other assets. Targets and configurations may declare copies.
- build_target: name, inputs, output filename, optional link configurations,
  copies, and required additional output filenames. No application filename is assumed.

Operation and limits
The caller owns discovery/workspace storage; there is no project malloc/backing
growth. At most 16 targets, 32 entries per list/configuration/copy set, 32 source
roots/files per discovery, 4096 visited directory entries per discovery, 192 argv
elements, 1024-byte strings,
and 32768-byte quoted Windows commands. Expected failures return errors; capacity
exhaustion never grows storage. Child compiler internals allocate independently.

Generation returns 0 on success, 2 on invalid declarations/output failure.
Execution returns 0 only for a fresh successful compile/link and declared copies/
output checks, otherwise 2. Outputs are under build/<target>/<compiler>-<mode>/
<run-name>/; versions and effective commands stay in compiler.log. Children use
argv-safe native invocation (CreateProcessW/posix_spawnp), never a shell, with
no retries or timeouts.

--help prints usage. Interactive stdout prints a short result; --json, piped
stdout, nonempty CI, or BUILD_AGENT=1 emits one JSON object. Diagnostics and child
compiler output do not contaminate structured stdout. Failure has artifact:null.
Missing prepared inputs are errors, never triggers for rebuilding/fallback.
Long-path Windows settings and non-ASCII paths are not broadly validated.
*/
#ifndef BUILD_H
#define BUILD_H

#include "base.h"

enum {
    BUILD_MAX_TARGETS = 16,
    BUILD_MAX_ITEMS = 64,
    BUILD_MAX_DIRECTORY_ENTRIES = 4096,
    BUILD_MAX_ARGUMENTS = 192,
    BUILD_PATH_SIZE = 1024,
    BUILD_COMMAND_SIZE = 32768,
    BUILD_RUN_PREFIX_SIZE = sizeof("build/build-driver/") - 1,
};

typedef struct build_inputs {
    const char *const *sources;
    const char *const *includes;
    const char *const *arguments;
    const char *const *libraries;
} build_inputs;

typedef struct build_copy {
    const char *source;
    const char *name;
} build_copy;

typedef struct build_link_config {
    const char *name;
    const char *compiler; /* cjit or zig; matches the actual compiler. */
    const char *const *files;
    const char *const *arguments;
    const build_copy *copies; /* Optional, terminated by source == NULL. */
} build_link_config;

typedef struct build_target {
    const char *name;
    build_inputs inputs;
    const char *output;
    const build_link_config *configurations; /* Optional, terminated by name == NULL. */
    const build_copy *copies;
    const char *const *required_outputs; /* Optional additional output filenames. */
} build_target;

#if defined(__APPLE__)
#define _stricmp strcasecmp
#define _strnicmp strncasecmp
#define _isatty isatty
#define _fileno fileno
#elif !defined(_WIN32)
#error Build generation and execution require Windows or macOS.
#endif

#ifdef BUILD_GENERATOR
#include <wchar.h>

/* Discovery fills caller-owned paths and non-owning pointers, valid until that
   storage is reused. Generated C bakes the result; it does not rescan at run time. */
typedef struct build_source_list {
    char paths[BUILD_MAX_ITEMS][BUILD_PATH_SIZE];
    const char *files[BUILD_MAX_ITEMS + 1];
    size_t count;
} build_source_list;

/* Validation precedes emission; lists must be NULL terminated within the bound. */
static int build_valid_list(const char *const *items, int required) {
    size_t i;
    if (!items)
        return 0;
    for (i = 0; i <= BUILD_MAX_ITEMS; i++) {
        if (!items[i])
            return !required || i != 0;
        if (i == BUILD_MAX_ITEMS || !*items[i] || strlen(items[i]) >= BUILD_PATH_SIZE)
            return 0;
    }
    return 0;
}

static int build_compare_sources(const void *left, const void *right) {
    return strcmp(*(const char *const *)left, *(const char *const *)right);
}

static int build_find_sources(build_source_list *sources, const char *const *directories) {
    size_t entries = 0;
    sources->count = 0;
    sources->files[0] = NULL;
    if (!build_valid_list(directories, 1)) {
        fprintf(stderr, "source discovery: invalid source directories\n");
        return 0;
    }
    for (size_t root = 0; directories[root]; ++root) {
        file_directory directory = {0};
        if (!file_directory_open(&directory, directories[root])) {
            fprintf(stderr, "source discovery: missing, linked, or invalid directory: %s\n", directories[root]);
            return 0;
        }
        int success = 1, next;
        while ((next = file_directory_read(&directory)) > 0) {
            if (++entries > BUILD_MAX_DIRECTORY_ENTRIES) {
                success = 0;
                break;
            }
            size_t size = strlen(directory.name);
            if (size < 2 || strcmp(directory.name + size - 2, ".c"))
                continue;
            char path[BUILD_PATH_SIZE];
            int length = snprintf(path, sizeof(path), "%s/%s", directories[root], directory.name);
            if (length < 0 || length >= BUILD_PATH_SIZE) {
                success = 0;
                break;
            }
            if (!file_is_regular(path, 0))
                continue;
            size_t other;
            for (other = 0; other < sources->count; ++other)
                if (!strcmp(sources->files[other], path))
                    break;
            if (other != sources->count)
                continue;
            if (sources->count == BUILD_MAX_ITEMS) {
                success = 0;
                break;
            }
            memcpy(sources->paths[sources->count], path, (size_t)length + 1);
            sources->files[sources->count] = sources->paths[sources->count];
            sources->files[++sources->count] = NULL;
        }
        if (next < 0)
            success = 0;
        if (!file_directory_close(&directory))
            success = 0;
        if (!success) {
            fprintf(stderr, "source discovery: enumeration or capacity failure: %s\n", directories[root]);
            return 0;
        }
    }
    if (!sources->count) {
        fprintf(stderr, "source discovery: no immediate .c sources found\n");
        return 0;
    }
    qsort(sources->files, sources->count, sizeof(sources->files[0]), build_compare_sources);
    return 1;
}

static int build_valid_inputs(const build_inputs *inputs) {
    return build_valid_list(inputs->sources, 1) && build_valid_list(inputs->includes, 0) &&
           build_valid_list(inputs->arguments, 0) && build_valid_list(inputs->libraries, 0);
}

static int build_valid_name(const char *name) {
    size_t i;
    if (!name || !*name || strlen(name) >= BUILD_PATH_SIZE)
        return 0;
    for (i = 0; name[i]; i++) {
        char c = name[i];
        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-'))
            return 0;
    }
    return 1;
}

static int build_write_literal(FILE *out, const char *text) {
    const unsigned char *byte = (const unsigned char *)text;
    if (!text)
        return fputs("NULL", out) >= 0;
    if (fputc('"', out) == EOF)
        return 0;
    for (; *byte; byte++) {
        if (*byte == '"' || *byte == '\\' || *byte == '?') {
            if (fputc('\\', out) == EOF || fputc(*byte, out) == EOF)
                return 0;
        } else if (*byte < 32 || *byte >= 127) {
            if (fprintf(out, "\\%03o", *byte) < 0)
                return 0;
        } else if (fputc(*byte, out) == EOF)
            return 0;
    }
    return fputc('"', out) != EOF;
}

static int build_valid_filename(const char *name) {
    size_t i, length, stem;
    if (!name || !*name)
        return 0;
    length = strlen(name);
    if (length >= BUILD_PATH_SIZE || strpbrk(name, "/\\:<>\"|?*") || name[length - 1] == '.' ||
        name[length - 1] == ' ')
        return 0;
    for (i = 0; i < length; ++i)
        if ((unsigned char)name[i] < 32)
            return 0;
    stem = strcspn(name, ".");
    /* Windows reserves device names even with an extension or trailing spaces. */
    while (stem && name[stem - 1] == ' ')
        --stem;
    if ((stem == 3 && (!_strnicmp(name, "con", 3) || !_strnicmp(name, "prn", 3) ||
                       !_strnicmp(name, "aux", 3) || !_strnicmp(name, "nul", 3))) ||
        (stem == 6 && !_strnicmp(name, "conin$", 6)) ||
        (stem == 7 && !_strnicmp(name, "conout$", 7)))
        return 0;
    if (!_strnicmp(name, "com", 3) || !_strnicmp(name, "lpt", 3)) {
        if (stem == 4 && name[3] >= '1' && name[3] <= '9')
            return 0;
        /* Windows also treats superscript 1, 2, and 3 as device digits (UTF-8). */
        if (stem == 5 && (unsigned char)name[3] == 0xc2 &&
            ((unsigned char)name[4] == 0xb9 || (unsigned char)name[4] == 0xb2 ||
             (unsigned char)name[4] == 0xb3))
            return 0;
    }
    return 1;
}

static int build_valid_copies(const build_copy *copies) {
    size_t i;
    if (!copies)
        return 1;
    for (i = 0; i <= BUILD_MAX_ITEMS; ++i) {
        if (!copies[i].source)
            return 1;
        if (i == BUILD_MAX_ITEMS || !*copies[i].source ||
            strlen(copies[i].source) >= BUILD_PATH_SIZE || !build_valid_filename(copies[i].name))
            return 0;
    }
    return 0;
}

/* Target and selected configuration copies share one artifact directory. */
static int build_valid_copy_destinations(const build_target *target, const build_copy *extra) {
    const build_copy *sets[] = {target->copies, extra};
    size_t set, i, previous_set, other;
    for (set = 0; set < 2; ++set) {
        if (!sets[set])
            continue;
        for (i = 0; sets[set][i].source; ++i) {
            const char *name = sets[set][i].name;
            if (!_stricmp(name, target->output) || !_stricmp(name, "compiler.log"))
                return 0;
            if (target->required_outputs)
                for (other = 0; target->required_outputs[other]; ++other)
                    if (!_stricmp(name, target->required_outputs[other]))
                        return 0;
            for (previous_set = 0; previous_set <= set; ++previous_set)
                if (sets[previous_set])
                    for (other = 0;
                         sets[previous_set][other].source && (previous_set != set || other < i);
                         ++other)
                        if (!_stricmp(name, sets[previous_set][other].name))
                            return 0;
        }
    }
    return 1;
}

static int build_valid_configurations(const build_link_config *configurations) {
    size_t i, other;
    if (!configurations)
        return 1;
    for (i = 0; i <= BUILD_MAX_ITEMS; ++i) {
        const build_link_config *configuration = &configurations[i];
        if (!configuration->name)
            return 1;
        if (i == BUILD_MAX_ITEMS || !build_valid_name(configuration->name) ||
            !configuration->compiler ||
            (strcmp(configuration->compiler, "cjit") && strcmp(configuration->compiler, "zig")) ||
            !build_valid_list(configuration->files, 0) ||
            !build_valid_list(configuration->arguments, 0) ||
            !build_valid_copies(configuration->copies))
            return 0;
        for (other = 0; other < i; ++other)
            if (!strcmp(configuration->name, configurations[other].name) &&
                !strcmp(configuration->compiler, configurations[other].compiler))
                return 0;
    }
    return 0;
}

static int build_write_list(FILE *out, const char *name, const char *const *items) {
    size_t i;
    if (fprintf(out, "static const char *%s[] = {\n", name) < 0)
        return 0;
    if (items)
        for (i = 0; items[i]; ++i)
            if (!build_write_literal(out, items[i]) || fputs(",\n", out) < 0)
                return 0;
    return fputs("NULL};\n", out) >= 0;
}

static int build_write_copies(FILE *out, const char *name, const build_copy *copies) {
    size_t i;
    if (fprintf(out, "static const build_copy %s[] = {\n", name) < 0)
        return 0;
    if (copies)
        for (i = 0; copies[i].source; ++i)
            if (fputc('{', out) == EOF || !build_write_literal(out, copies[i].source) ||
                fputs(", ", out) < 0 || !build_write_literal(out, copies[i].name) ||
                fputs("},\n", out) < 0)
                return 0;
    return fputs("{NULL, NULL}};\n", out) >= 0;
}

static int build_write_inputs(FILE *out, size_t index, const build_inputs *inputs) {
    const char *const *lists[] = {
        inputs->sources,
        inputs->includes,
        inputs->arguments,
        inputs->libraries,
    };
    size_t list;
    for (list = 0; list < 4; list++) {
        char name[64];
        snprintf(name, sizeof(name), "inputs_%zu_%zu", index, list);
        if (!build_write_list(out, name, lists[list]))
            return 0;
    }
    return 1;
}

static int build_generate(FILE *out, const build_target *targets, size_t count) {
    size_t i, other;
    if (!out || !targets || !count || count > BUILD_MAX_TARGETS) {
        fprintf(stderr, "invalid build declarations or capacity exceeded\n");
        return 2;
    }
    for (i = 0; i < count; i++) {
        const build_target *target = &targets[i];
        if (!build_valid_name(target->name) || !build_valid_inputs(&target->inputs) ||
            !build_valid_filename(target->output) ||
            !build_valid_configurations(target->configurations) ||
            !build_valid_copies(target->copies) ||
            (target->required_outputs && !build_valid_list(target->required_outputs, 0))) {
            fprintf(stderr, "invalid declaration for target %zu\n", i);
            return 2;
        }
        if (target->required_outputs)
            for (other = 0; target->required_outputs[other]; ++other)
                if (!build_valid_filename(target->required_outputs[other])) {
                    fprintf(stderr, "invalid required output for target %zu\n", i);
                    return 2;
                }
        if (!_stricmp(target->output, "compiler.log") ||
            !build_valid_copy_destinations(target, NULL)) {
            fprintf(stderr, "conflicting output/copy destination for target %zu\n", i);
            return 2;
        }
        if (target->configurations)
            for (other = 0; target->configurations[other].name; ++other)
                if (!build_valid_copy_destinations(target, target->configurations[other].copies)) {
                    fprintf(stderr,
                            "conflicting configuration copy destination for target %zu\n",
                            i);
                    return 2;
                }
        for (other = 0; other < i; other++) {
            if (!strcmp(target->name, targets[other].name)) {
                fprintf(stderr, "duplicate build target: %s\n", target->name);
                return 2;
            }
        }
    }
    if (fputs("/* Generated C build driver; regenerate from your declarations. */\n"
              "#define BUILD_DRIVER\n#include \"build.h\"\n",
              out) < 0)
        goto output_error;
    for (i = 0; i < count; i++) {
        char name[64];
        const build_target *target = &targets[i];
        if (!build_write_inputs(out, i, &target->inputs))
            goto output_error;
        snprintf(name, sizeof(name), "copies_%zu", i);
        if (!build_write_copies(out, name, target->copies))
            goto output_error;
        snprintf(name, sizeof(name), "outputs_%zu", i);
        if (!build_write_list(out, name, target->required_outputs))
            goto output_error;
        if (target->configurations) {
            size_t selected;
            for (selected = 0; target->configurations[selected].name; ++selected) {
                const build_link_config *configuration = &target->configurations[selected];
                snprintf(name, sizeof(name), "link_files_%zu_%zu", i, selected);
                if (!build_write_list(out, name, configuration->files))
                    goto output_error;
                snprintf(name, sizeof(name), "link_arguments_%zu_%zu", i, selected);
                if (!build_write_list(out, name, configuration->arguments))
                    goto output_error;
                snprintf(name, sizeof(name), "link_copies_%zu_%zu", i, selected);
                if (!build_write_copies(out, name, configuration->copies))
                    goto output_error;
            }
            if (fprintf(out, "static const build_link_config configurations_%zu[] = {\n", i) < 0)
                goto output_error;
            for (selected = 0; target->configurations[selected].name; ++selected) {
                const build_link_config *configuration = &target->configurations[selected];
                if (fputc('{', out) == EOF || !build_write_literal(out, configuration->name) ||
                    fputs(", ", out) < 0 || !build_write_literal(out, configuration->compiler) ||
                    fprintf(out,
                            ", link_files_%zu_%zu, link_arguments_%zu_%zu, link_copies_%zu_%zu},\n",
                            i,
                            selected,
                            i,
                            selected,
                            i,
                            selected) < 0)
                    goto output_error;
            }
            if (fputs("{NULL, NULL, NULL, NULL, NULL}};\n", out) < 0)
                goto output_error;
        }
    }
    if (fputs("static const build_target targets[] = {\n", out) < 0)
        goto output_error;
    for (i = 0; i < count; i++) {
        const build_target *target = &targets[i];
        if (fputc('{', out) == EOF || !build_write_literal(out, target->name) ||
            fprintf(out,
                    ", {inputs_%zu_0, inputs_%zu_1, inputs_%zu_2, inputs_%zu_3}, ",
                    i,
                    i,
                    i,
                    i) < 0 ||
            !build_write_literal(out, target->output) || fputs(", ", out) < 0)
            goto output_error;
        if (target->configurations) {
            if (fprintf(out, "configurations_%zu", i) < 0)
                goto output_error;
        } else if (fputs("NULL", out) < 0)
            goto output_error;
        if (fprintf(out, ", copies_%zu, outputs_%zu},\n", i, i) < 0)
            goto output_error;
    }
    if (fputs("};\nint main(int argc, char **argv) {\n"
              "static build_workspace workspace;\n"
              "return build_run(&workspace, argc, argv, targets,\n"
              "sizeof(targets) / sizeof(targets[0]));\n}\n",
              out) < 0 ||
        fflush(out) != 0 || ferror(out))
        goto output_error;
    return 0;
output_error:
    fprintf(stderr, "failed to write generated build driver\n");
    return 2;
}
#endif

#ifdef BUILD_DRIVER
#if defined(_WIN32)
#include <io.h>
#endif

/* Caller-owned fixed workspace. No project malloc or hidden growable storage. */
typedef struct build_workspace {
    const char *arguments[BUILD_MAX_ARGUMENTS];
    size_t argument_count;
    process_workspace process;
    char directory[BUILD_PATH_SIZE];
    char output[BUILD_PATH_SIZE];
    char log[BUILD_PATH_SIZE];
    char destination[BUILD_PATH_SIZE];
    char message[BUILD_PATH_SIZE];
    const char *stage;
    const char *prepared_hint; /* Optional caller-owned setup instruction. */
    const char *generation;
    const char *driver_compilation;
    unsigned long child_exit;
    unsigned int commands;
} build_workspace;

static int build_error(build_workspace *workspace, const char *stage, const char *message) {
    workspace->stage = stage;
#if defined(_WIN32)
    snprintf(workspace->message,
             sizeof(workspace->message),
             "%s (Windows error %lu)",
             message,
             (unsigned long)GetLastError());
#else
    snprintf(workspace->message, sizeof(workspace->message), "%s (errno %d)", message, errno);
#endif
    fprintf(stderr, "%s: %s\n", stage, workspace->message);
    return 0;
}

static int
build_path(build_workspace *workspace, char *out, const char *directory, const char *leaf) {
    int length = snprintf(out, BUILD_PATH_SIZE, "%s/%s", directory, leaf);
    return (length >= 0 && length < BUILD_PATH_SIZE) ||
           build_error(workspace, "configuration", "path exceeds 1023 bytes");
}

static int build_directory(build_workspace *workspace, const char *path) {
    return file_directory_create(path, 0) ||
           build_error(workspace, "storage", "could not create build directory");
}

static int build_file_exists(build_workspace *workspace, const char *path) {
    if (file_is_regular(path, 1))
        return 1;
    snprintf(workspace->message,
             sizeof(workspace->message),
             "missing required file: %.800s%s%.150s",
             path,
             workspace->prepared_hint ? "; " : "",
             workspace->prepared_hint ? workspace->prepared_hint : "");
    workspace->stage = "prerequisites";
    fprintf(stderr, "%s\n", workspace->message);
    return 0;
}

static int build_add(build_workspace *workspace, const char *argument) {
    if (!argument || strlen(argument) >= BUILD_PATH_SIZE ||
        workspace->argument_count == BUILD_MAX_ARGUMENTS)
        return build_error(workspace, "configuration", "compiler argument capacity exceeded");
    workspace->arguments[workspace->argument_count++] = argument;
    return 1;
}

static int build_process(build_workspace *workspace) {
    if (!process_run(&workspace->process, workspace->arguments, workspace->argument_count,
                     workspace->log))
        return build_error(workspace,
                           !strcmp(workspace->process.error, "process command capacity exceeded") ?
                               "configuration" : "process", workspace->process.error);
    ++workspace->commands;
    workspace->child_exit = workspace->process.exit_code;
    if (workspace->child_exit) {
        workspace->stage = "compilation";
        snprintf(workspace->message, sizeof(workspace->message), "compiler exited %lu; see %.900s",
                 workspace->child_exit, workspace->log);
        fprintf(stderr, "%s\n", workspace->message);
        return 0;
    }
    return 1;
}

static const char *build_environment(const char *name, const char *fallback) {
    const char *value = getenv(name);
    return value && *value ? value : fallback;
}

static int build_compiler(build_workspace *workspace, const char *compiler, int optimized) {
    workspace->argument_count = 0;
    if (!strcmp(compiler, "zig")) {
        return build_add(workspace, build_environment("ZIG", "zig")) &&
               build_add(workspace, "cc") && build_add(workspace, optimized ? "-O2" : "-O0");
    }
    if (strcmp(compiler, "cjit") || optimized)
        return build_error(workspace, "configuration", "unsupported compiler or optimization");
    /* CJIT reads CFLAGS unless overridden. Fixed empty flags prevent hidden settings. */
    if (!build_add(workspace, build_environment("CJIT", "cjit")) || !build_add(workspace, "-q") ||
        !build_add(workspace, "-C") || !build_add(workspace, ""))
        return 0;
#if defined(__APPLE__)
    /* Wrappers discover SDK headers; standalone callers provide the same input. */
    const char *headers = build_environment("BUILD_SDK_INCLUDE", "");
    if (!*headers)
        return build_error(workspace, "prerequisites", "set BUILD_SDK_INCLUDE to macOS SDK usr/include");
    if (!build_add(workspace, "-I") || !build_add(workspace, headers))
        return 0;
#endif
    return 1;
}

static int
build_arguments(build_workspace *workspace, const build_inputs *inputs, const char *compiler) {
    size_t i;
    for (i = 0; inputs->includes[i]; i++) {
        if (!build_add(workspace, "-I") || !build_add(workspace, inputs->includes[i]))
            return 0;
    }
    for (i = 0; inputs->arguments[i]; i++) {
        const char *argument = inputs->arguments[i];
        /* CJIT has its own language defaults and no GCC warning switches. */
        if (!strcmp(compiler, "cjit") &&
            (!strncmp(argument, "-std=", 5) || !strcmp(argument, "-Wall") ||
             !strcmp(argument, "-Wextra")))
            continue;
        if (!build_add(workspace, argument))
            return 0;
    }
    return 1;
}

static int build_libraries(build_workspace *workspace, const build_inputs *inputs) {
    size_t i;
    for (i = 0; inputs->libraries[i]; i++) {
        if (!build_add(workspace, "-l") || !build_add(workspace, inputs->libraries[i]))
            return 0;
    }
    return 1;
}

static int build_check_inputs(build_workspace *workspace, const build_inputs *inputs) {
    size_t i;
    for (i = 0; inputs->sources[i]; i++) {
        if (!build_file_exists(workspace, inputs->sources[i]))
            return 0;
    }
    for (i = 0; inputs->includes[i]; i++) {
        if (!file_is_directory(inputs->includes[i], 1))
            return build_error(workspace, "prerequisites", "missing declared include directory");
    }
    return 1;
}

static int build_select_configuration(build_workspace *workspace,
                                      const build_target *target,
                                      const char *compiler,
                                      const char *name,
                                      const build_link_config **selected) {
    size_t i;
    *selected = NULL;
    if (!target->configurations)
        return !name ||
               build_error(workspace, "configuration", "target has no link configurations");
    for (i = 0; i < BUILD_MAX_ITEMS && target->configurations[i].name; ++i) {
        const build_link_config *configuration = &target->configurations[i];
        if (!strcmp(configuration->compiler, compiler) &&
            (!name || !strcmp(configuration->name, name))) {
            *selected = configuration;
            return 1;
        }
    }
    return build_error(workspace, "configuration", "no matching compiler/link configuration");
}

static int build_copy_files(build_workspace *workspace, const build_copy *copies) {
    size_t i;
    if (!copies)
        return 1;
    for (i = 0; i < BUILD_MAX_ITEMS && copies[i].source; ++i) {
        if (!build_file_exists(workspace, copies[i].source) ||
            !build_path(workspace, workspace->destination, workspace->directory, copies[i].name))
            return 0;
        if (!file_copy(copies[i].source, workspace->destination, 1))
            return build_error(workspace, "storage", "could not copy declared artifact");
    }
    return !copies[i].source || build_error(workspace, "configuration", "copy capacity exceeded");
}

static int build_compile(build_workspace *workspace,
                         const build_target *target,
                         const char *compiler,
                         int optimized,
                         const build_link_config *configuration) {
    size_t i;
    if (!build_check_inputs(workspace, &target->inputs))
        return 0;
    if (configuration)
        for (i = 0; configuration->files[i]; ++i)
            if (!build_file_exists(workspace, configuration->files[i]))
                return 0;
    if (!build_compiler(workspace, compiler, optimized) ||
        !build_arguments(workspace, &target->inputs, compiler))
        return 0;
    for (i = 0; target->inputs.sources[i]; ++i)
        if (!build_add(workspace, target->inputs.sources[i]))
            return 0;
    if (configuration) {
        for (i = 0; configuration->arguments[i]; ++i)
            if (!build_add(workspace, configuration->arguments[i]))
                return 0;
        for (i = 0; configuration->files[i]; ++i)
            if (!build_add(workspace, configuration->files[i]))
                return 0;
    }
    if (!build_libraries(workspace, &target->inputs) || !build_add(workspace, "-o") ||
        !build_add(workspace, workspace->output) || !build_process(workspace) ||
        !build_file_exists(workspace, workspace->output))
        return 0;
#if defined(__APPLE__)
    if (!strcmp(compiler, "cjit")) {
        /* Apple Silicon requires a signature even for local native executables. */
        workspace->argument_count = 0;
        if (!build_add(workspace, "codesign") || !build_add(workspace, "--force") ||
            !build_add(workspace, "--sign") || !build_add(workspace, "-") ||
            !build_add(workspace, workspace->output) || !build_process(workspace))
            return 0;
    }
#endif
    if (target->required_outputs)
        for (i = 0; target->required_outputs[i]; ++i)
            if (!build_path(workspace,
                            workspace->destination,
                            workspace->directory,
                            target->required_outputs[i]) ||
                !build_file_exists(workspace, workspace->destination))
                return 0;
    return build_copy_files(workspace, target->copies) &&
           (!configuration || build_copy_files(workspace, configuration->copies));
}

static void build_json_string(const char *text) {
    const unsigned char *byte = (const unsigned char *)(text ? text : "");
    putchar('"');
    for (; *byte; byte++) {
        if (*byte == '"' || *byte == '\\') {
            putchar('\\');
            putchar(*byte);
        } else if (*byte < 32)
            printf("\\u%04x", *byte);
        else
            putchar(*byte);
    }
    putchar('"');
}

typedef struct build_options {
    const char *name;
    const char *compiler;
    const char *mode;
    const char *configuration;
    const char *run;
    int json;
    int help;
} build_options;

typedef struct build_configuration_default {
    const char *compiler;
    const char *optimization;
    const char *configuration;
} build_configuration_default;

/* Shared result path also reports failures before the generated driver exists. */
static int build_result(build_workspace *workspace, int success, const build_options *options) {
    const int json = options->json;
    const char *name = options->name, *compiler = options->compiler, *mode = options->mode;
    const char *configuration_name = options->configuration, *run = options->run;
    if (json) {
        printf("{\"status\":\"%s\",\"generation\":", success ? "complete" : "error");
        build_json_string(workspace->generation ? workspace->generation : "complete");
        printf(",\"driver_compilation\":");
        build_json_string(workspace->driver_compilation ? workspace->driver_compilation
                                                        : "complete");
        printf(",\"stage\":");
        build_json_string(success ? "target" : workspace->stage);
        printf(",\"target\":");
        build_json_string(name);
        printf(",\"compiler\":");
        build_json_string(compiler);
        printf(",\"optimization\":");
        build_json_string(mode);
        printf(",\"configuration\":");
        if (configuration_name)
            build_json_string(configuration_name);
        else
            printf("null");
        printf(",\"run\":");
        build_json_string(run);
        printf(",\"artifact\":");
        if (success)
            build_json_string(workspace->output);
        else
            printf("null");
        printf(",\"log\":");
        build_json_string(workspace->log);
        printf(",\"commands\":%u,\"child_exit\":%lu,\"error\":",
               workspace->commands,
               workspace->child_exit);
        if (success)
            printf("null");
        else
            build_json_string(workspace->message);
        puts("}");
    } else {
        printf("%s: %s (%s, %s)\n", success ? "Built" : "Failed", name, compiler, mode);
        if (success)
            printf("Artifact: %s\n", workspace->output);
        printf("Build details: %s\n", run ? run : "none");
    }
    return fflush(stdout) != 0 || ferror(stdout) || !success ? 2 : 0;
}

typedef enum build_option_scope {
    build_entry_options,
    build_driver_options,
} build_option_scope;

static int build_parse_options(build_workspace *workspace,
                               build_options *options,
                               int argc,
                               char **argv,
                               const char *configuration_option,
                               build_option_scope scope) {
    const int driver = scope == build_driver_options;
    /* Scan output switches first so even an earlier bad option honors --json. */
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--json"))
            options->json = 1;
        if (!strcmp(argv[i], "--human") && driver)
            options->json = 0;
    }
    for (int i = 1; i < argc; ++i) {
        const char *option = argv[i];
        if (!strcmp(option, "--json") || (!strcmp(option, "--human") && driver))
            continue;
        if (!strcmp(option, "--help") || !strcmp(option, "-h")) {
            options->help = 1;
            continue;
        }
        const char **value = NULL;
        if (!strcmp(option, "--target"))
            value = &options->name;
        else if (!strcmp(option, "--compiler"))
            value = &options->compiler;
        else if (!strcmp(option, "--optimization"))
            value = &options->mode;
        else if (!strcmp(option, configuration_option))
            value = &options->configuration;
        else if (!strcmp(option, "--run") && driver)
            value = &options->run;
        else
            return build_error(workspace, "configuration", "unknown option; use --help");
        if (++i == argc || !strncmp(argv[i], "--", 2))
            return build_error(workspace, "configuration", "missing option value");
        if (strlen(argv[i]) >= BUILD_PATH_SIZE)
            return build_error(workspace, "configuration", "option value exceeds capacity");
        *value = argv[i];
    }
    if ((strcmp(options->compiler, "cjit") && strcmp(options->compiler, "zig")) ||
        (strcmp(options->mode, "unoptimized") && strcmp(options->mode, "optimized")) ||
        (!strcmp(options->compiler, "cjit") && !strcmp(options->mode, "optimized")))
        return build_error(workspace,
                           "configuration",
                           "use unoptimized CJIT or optimized/unoptimized Zig");
    return 1;
}

static int build_default_json(void) {
    return !_isatty(_fileno(stdout)) || build_environment("CI", "")[0] != 0 ||
           !strcmp(build_environment("BUILD_AGENT", "0"), "1");
}

/* One target lookup/configuration resolution path for entry and direct driver.
   Defaults are caller-owned data; the driver may omit them to use first match. */
static const build_target *build_resolve(build_workspace *workspace,
                                         build_options *options,
                                         const build_target *targets,
                                         size_t count,
                                         const build_configuration_default *defaults,
                                         const build_link_config **configuration) {
    const build_target *target = NULL;
    if (!targets || !count || count > BUILD_MAX_TARGETS) {
        build_error(workspace, "configuration", "invalid target declarations or capacity exceeded");
        return NULL;
    }
    for (size_t i = 0; i < count; ++i)
        if (!strcmp(targets[i].name, options->name)) {
            target = &targets[i];
            break;
        }
    if (!target) {
        build_error(workspace, "configuration", "unknown target; use --help");
        return NULL;
    }
    if (target->configurations && !options->configuration && defaults) {
        size_t i;
        for (i = 0; i < BUILD_MAX_ITEMS && defaults[i].compiler; ++i)
            if (!strcmp(defaults[i].compiler, options->compiler) &&
                !strcmp(defaults[i].optimization, options->mode)) {
                options->configuration = defaults[i].configuration;
                break;
            }
        if (i == BUILD_MAX_ITEMS && defaults[i].compiler) {
            build_error(workspace, "configuration", "configuration default capacity exceeded");
            return NULL;
        }
    }
    if (!build_select_configuration(workspace,
                                    target,
                                    options->compiler,
                                    options->configuration,
                                    configuration))
        return NULL;
    options->configuration = *configuration ? (*configuration)->name : NULL;
    return target;
}

static build_options build_begin(build_workspace *workspace, const char *default_target) {
    workspace->stage = "configuration";
    workspace->generation = "not-started";
    workspace->driver_compilation = "not-started";
    workspace->commands = 0;
    workspace->child_exit = 0;
    workspace->log[0] = 0;
    workspace->message[0] = 0;
    build_options options =
        {default_target, "cjit", "unoptimized", NULL, NULL, build_default_json(), 0};
    return options;
}

static int build_help(const char *command,
                      const char *configuration_option,
                      int driver,
                      const build_target *targets,
                      size_t count) {
    printf("Usage: %s%s [--target <name>] [--compiler cjit|zig] "
           "[--optimization unoptimized|optimized] [%s <name>] [--json]\n",
           command,
           driver ? " --run build/build-driver/<run>" : "",
           configuration_option);
    puts("Targets (compiler:configuration when linked):");
    for (size_t i = 0; i < count; ++i) {
        printf("%s", targets[i].name);
        if (targets[i].configurations)
            for (size_t j = 0; j < BUILD_MAX_ITEMS && targets[i].configurations[j].name; ++j)
                printf(" %s:%s",
                       targets[i].configurations[j].compiler,
                       targets[i].configurations[j].name);
        putchar('\n');
    }
    return ferror(stdout) ? 2 : 0;
}

static int build_run(build_workspace *workspace,
                     int argc,
                     char **argv,
                     const build_target *targets,
                     size_t count) {
    build_options options = build_begin(workspace, count && targets ? targets[0].name : "");
    /* This executable only exists after successful generation/driver compilation. */
    workspace->generation = workspace->driver_compilation = "complete";
    const build_link_config *link_configuration = NULL;
    const build_target *target;
    int success = 0;
    if (!build_parse_options(workspace,
                             &options,
                             argc,
                             argv,
                             "--configuration",
                             build_driver_options))
        goto finish;
    if (options.help)
        return build_help("build-driver", "--configuration", 1, targets, count);
    if (!options.run || strncmp(options.run, "build/build-driver/", BUILD_RUN_PREFIX_SIZE) ||
        strpbrk(options.run + BUILD_RUN_PREFIX_SIZE, "/\\:") ||
        strstr(options.run + BUILD_RUN_PREFIX_SIZE, "..") || !options.run[BUILD_RUN_PREFIX_SIZE]) {
        build_error(workspace,
                    "configuration",
                    "--run must name a fresh build/build-driver/<run> directory");
        goto finish;
    }
    target = build_resolve(workspace, &options, targets, count, NULL, &link_configuration);
    if (!target)
        goto finish;
    if (!build_path(workspace, workspace->log, options.run, "compiler.log") ||
        !build_directory(workspace, "build") ||
        !build_path(workspace, workspace->destination, "build", options.name) ||
        !build_directory(workspace, workspace->destination))
        goto finish;
    {
        char artifact_mode[64];
        snprintf(artifact_mode, sizeof(artifact_mode), "%s-%s", options.compiler, options.mode);
        if (!build_path(workspace, workspace->output, workspace->destination, artifact_mode) ||
            !build_directory(workspace, workspace->output) ||
            !build_path(workspace,
                        workspace->directory,
                        workspace->output,
                        options.run + BUILD_RUN_PREFIX_SIZE))
            goto finish;
    }
    if (!file_directory_create(workspace->directory, 1)) {
        build_error(
            workspace,
            "storage",
            "could not create fresh artifact directory (existing runs are not overwritten)");
        goto finish;
    }
    if (!build_path(workspace, workspace->output, workspace->directory, target->output))
        goto finish;
    /* Retain actual versions in the same log as every effective command. */
    if (!strcmp(options.compiler, "zig")) {
        workspace->argument_count = 0;
        if (!build_add(workspace, build_environment("ZIG", "zig")) ||
            !build_add(workspace, "version") || !build_process(workspace))
            goto finish;
    }
    if (!strcmp(options.compiler, "cjit")) {
        workspace->argument_count = 0;
        if (!build_add(workspace, build_environment("CJIT", "cjit")) ||
            !build_add(workspace, "-h") || !build_process(workspace))
            goto finish;
    }
    success = build_compile(workspace,
                            target,
                            options.compiler,
                            !strcmp(options.mode, "optimized"),
                            link_configuration);
    if (success) {
        if (!build_path(workspace, workspace->destination, workspace->directory, "compiler.log") ||
            !file_copy(workspace->log, workspace->destination, 1)) {
            build_error(workspace, "storage", "could not retain target compiler configuration/log");
            success = 0;
        }
    }
finish:
    return build_result(workspace, success, &options);
}
#endif

#if defined(BUILD_GENERATOR) && defined(BUILD_DRIVER)
typedef struct build_discovery {
    build_source_list *sources;
    const char *const *directories;
} build_discovery;

/* Borrowed project declarations. Lists terminate with NULL; all storage belongs
   to caller and remains live through build_entry(). No project policy in header. */
typedef struct build_project {
    const char *command;
    const char *default_target;
    const build_target *targets;
    size_t target_count;
    const char *include_directory;
    const char *configuration_option;
    const build_configuration_default *configuration_defaults; /* compiler == NULL terminates */
    const build_discovery *discoveries;                        /* sources == NULL terminates */
    const char *prepared_hint;
    const char *agent_environment;
} build_project;

/* Resolve declarations, generate, compile with CJIT, execute once. */
static int build_orchestrate(build_workspace *workspace,
                             build_options *options,
                             const build_project *project) {
    const build_target *targets = project->targets;
    const size_t count = project->target_count;
    workspace->prepared_hint = project->prepared_hint;
    char run[BUILD_PATH_SIZE] = "", source[BUILD_PATH_SIZE], driver[BUILD_PATH_SIZE];
    const build_link_config *configuration = NULL;
    const build_target *target = NULL;
    FILE *file;
    target = build_resolve(workspace,
                           options,
                           targets,
                           count,
                           project->configuration_defaults,
                           &configuration);
    if (!target)
        goto failure;
    if (!build_directory(workspace, "build") || !build_directory(workspace, "build/build-driver"))
        goto failure;
    snprintf(run,
             sizeof(run),
             "build/build-driver/%s-%s-%lu-%llu",
             options->compiler,
             options->mode,
             (unsigned long)process_id(),
             (unsigned long long)system_milliseconds());
    options->run = run;
    if (!file_directory_create(run, 1)) {
        build_error(workspace,
                    "storage",
                    "could not create fresh driver run (existing runs are not overwritten)");
        goto failure;
    }
    if (!build_path(workspace, workspace->log, run, "generator.log"))
        goto failure;
    workspace->argument_count = 0;
    if (!build_add(workspace, build_environment("CJIT", "cjit")) || !build_add(workspace, "-h") ||
        !build_process(workspace))
        goto failure;
    if (configuration)
        for (size_t i = 0; configuration->files[i]; ++i)
            if (!build_file_exists(workspace, configuration->files[i]))
                goto failure;
    if (project->discoveries) {
        workspace->generation = "failed";
        size_t i;
        for (i = 0; i < BUILD_MAX_ITEMS && project->discoveries[i].sources; ++i)
            if (!build_find_sources(project->discoveries[i].sources,
                                    project->discoveries[i].directories)) {
                workspace->generation = "failed";
                build_error(workspace, "generation", "source discovery failed");
                goto failure;
            }
        if (i == BUILD_MAX_ITEMS && project->discoveries[i].sources) {
            build_error(workspace, "generation", "source discovery list capacity exceeded");
            goto failure;
        }
    }
    if (!build_path(workspace, source, run, "driver.c"))
        goto failure;
    workspace->generation = "failed";
    file = fopen(source, "wb");
    if (!file) {
        build_error(workspace, "generation", "could not open generated source");
        goto failure;
    }
    int generated = build_generate(file, targets, count) == 0;
    if (fclose(file))
        generated = 0;
    if (!generated) {
        build_error(workspace,
                    "generation",
                    "driver generation failed; see generator.log and stderr");
        goto failure;
    }
    workspace->generation = "complete";
#if defined(_WIN32)
    const char *driver_name = "driver.exe";
#else
    const char *driver_name = "driver";
#endif
    if (!build_path(workspace, driver, run, driver_name) ||
        !build_path(workspace, workspace->log, run, "driver-compilation.log"))
        goto failure;
    workspace->driver_compilation = "failed";
    if (!build_compiler(workspace, "cjit", 0) || !build_add(workspace, "-I") ||
        !build_add(workspace, project->include_directory) || !build_add(workspace, source) ||
        !build_add(workspace, "-o") || !build_add(workspace, driver) || !build_process(workspace) ||
        !build_file_exists(workspace, driver)) {
        workspace->stage = "driver-compilation";
        goto failure;
    }
#if defined(__APPLE__)
    workspace->argument_count = 0;
    if (!build_add(workspace, "codesign") || !build_add(workspace, "--force") ||
        !build_add(workspace, "--sign") || !build_add(workspace, "-") ||
        !build_add(workspace, driver) || !build_process(workspace)) {
        workspace->stage = "driver-compilation";
        goto failure;
    }
#endif
    workspace->driver_compilation = "complete";
    workspace->argument_count = 0;
    if (!build_add(workspace, driver) || !build_add(workspace, "--run") ||
        !build_add(workspace, run) || !build_add(workspace, "--target") ||
        !build_add(workspace, options->name) || !build_add(workspace, "--compiler") ||
        !build_add(workspace, options->compiler) || !build_add(workspace, "--optimization") ||
        !build_add(workspace, options->mode) ||
        !build_add(workspace, options->json ? "--json" : "--human"))
        goto failure;
    if (options->configuration &&
        (!build_add(workspace, "--configuration") || !build_add(workspace, options->configuration)))
        goto failure;
    /* Child owns the sole terminal/JSON result; never mix compiler output into it. */
    if (!process_run(&workspace->process, workspace->arguments, workspace->argument_count, NULL)) {
        build_error(workspace, "driver-execution", workspace->process.error);
        goto failure;
    }
    return workspace->process.exit_code == 0 ? 0 : 2;
failure:
    return build_result(workspace, 0, options);
}

/* Project entry is declarations plus this call. Policy is supplied as data. */
static int
build_entry(build_workspace *workspace, int argc, char **argv, const build_project *project) {
    build_options options = build_begin(workspace, project->default_target);
    const char *configuration_option =
        project->configuration_option ? project->configuration_option : "--configuration";
    if (project->agent_environment &&
        !strcmp(build_environment(project->agent_environment, "0"), "1"))
        options.json = 1;
    if (!build_parse_options(workspace,
                             &options,
                             argc,
                             argv,
                             configuration_option,
                             build_entry_options))
        return build_result(workspace, 0, &options);
    if (options.help) {
        if (build_help(project->command ? project->command : "build",
                       configuration_option,
                       0,
                       project->targets,
                       project->target_count))
            return 2;
        printf("Default: %s, unoptimized CJIT. Build tooling always uses CJIT.\n",
               project->default_target);
        puts("Build only: no target execution. Piped output, --json, CI or BUILD_AGENT=1 selects "
             "JSON.");
        if (project->agent_environment)
            printf("%s=1 also selects JSON.\n", project->agent_environment);
        if (project->prepared_hint)
            printf("Prepared inputs: %s\n", project->prepared_hint);
        if (project->configuration_defaults) {
            puts("Default link configurations (compiler / optimization / name):");
            for (size_t i = 0; i < BUILD_MAX_ITEMS && project->configuration_defaults[i].compiler;
                 ++i) {
                const build_configuration_default *item = &project->configuration_defaults[i];
                printf("%s / %s / %s\n", item->compiler, item->optimization, item->configuration);
            }
        }
        puts("Exit 0: success; exit 2: controlled build failure.");
        return fflush(stdout) != 0 || ferror(stdout) ? 2 : 0;
    }
    return build_orchestrate(workspace, &options, project);
}
#endif
#endif
