#ifndef ATTENDANT_NATIVE_TOOL_MANIFESTS_H
#define ATTENDANT_NATIVE_TOOL_MANIFESTS_H
/* Content provenance, not runtime product state. JSON budgets remain explicit. */
enum { TOOL_MANIFEST_FILES = 1200 };
static char tool_paths[TOOL_MANIFEST_FILES][NATIVE_PATH_SIZE], tool_hashes[TOOL_MANIFEST_FILES][65];
static usize tool_path_count;
static const char *tool_library(void) {
#if defined(_WIN32)
    return "cimgui_richmd.dll";
#else
    return "libcimgui_richmd.dylib";
#endif
}
static int tool_relative(const char *path) {
    return path && path[0] && path[0] != '/' && !strchr(path, '\\') && !strchr(path, ':') &&
        strcmp(path, "..") && strncmp(path, "../", 3) && !strstr(path, "/../");
}
static int tool_add_path(const char *path) {
    TOOL_REQUIRE(tool_relative(path) && strlen(path) < NATIVE_PATH_SIZE, "Unsafe/overlong provenance path");
    for (usize i = 0; i < tool_path_count; ++i) if (!strcmp(path, tool_paths[i])) return 1;
    TOOL_REQUIRE(tool_path_count < TOOL_MANIFEST_FILES, "Provenance file capacity exceeded (1200)");
    strcpy(tool_paths[tool_path_count++], path); return 1;
}
static int tool_sort_path(const void *left, const void *right) { return strcmp(left, right); }
static int tool_hash_paths(void) {
    qsort(tool_paths, tool_path_count, sizeof(tool_paths[0]), tool_sort_path);
    for (usize i = 0; i < tool_path_count; ++i) TOOL_REQUIRE(tool_sha256(tool_paths[i], tool_hashes[i]), "Cannot hash provenance input");
    return 1;
}
static int tool_walk_inputs(const char *folder) {
    arena_reset(tool_files.memory); file_tree tree = {0};
    TOOL_REQUIRE(file_directory_list_tree(&tool_files, (cstring){(char *)folder}, &tree) == file_error_ok, "Cannot inventory provenance inputs");
    for (usize i = 0; i < tree.count; ++i) if (tree.entries[i].kind == file_entry_regular) {
        const char *extension = strrchr(tree.entries[i].path, '.');
        if (extension && (!strcmp(extension, ".c") || !strcmp(extension, ".h") || !strcmp(extension, ".inc") || (!strcmp(folder, "tools/native") && !strcmp(extension, ".json")))) {
            char path[NATIVE_PATH_SIZE]; TOOL_REQUIRE(tool_path(path, folder, tree.entries[i].path) && tool_add_path(path), "Cannot record provenance path");
        }
    }
    arena_reset(tool_files.memory); return 1;
}
static int tool_gui_inputs(void) {
    tool_path_count = 0;
    const char *paths[] = {"build-gui.sh", "tools/build-inputs.sh", "tools/build-platform.sh", "tools/build-gui-macos.sh", "tools/native-tool.c", "tools/native-tool.sh", "tools/native-build.sh", "tools/build-gui-tests.sh", "tools/check-gui.sh", "tools/check-bridge.sh", "tools/check-control-macos.sh", "tools/check-gui-bridge.sh", "tools/capture-command.sh"};
    for (usize i = 0; i < sizeof(paths) / sizeof(paths[0]); ++i) TOOL_REQUIRE(tool_add_path(paths[i]), "Cannot add GUI build input");
    const char *folders[] = {"src", "apps/gui", "tests/gui", "tests/core", "tests/support", "vendor", "tools/native"};
    for (usize i = 0; i < sizeof(folders) / sizeof(folders[0]); ++i) TOOL_REQUIRE(tool_walk_inputs(folders[i]), "Cannot scan GUI build inputs");
    return tool_hash_paths();
}
static int tool_write_files(FILE *out) {
    fputc('[', out);
    for (usize i = 0; i < tool_path_count; ++i) {
        if (i) fputc(',', out); fputs("{\"path\":", out); TOOL_REQUIRE(tool_json_string(out, tool_paths[i]), "Invalid UTF-8 provenance path");
        fputs(",\"sha256\":", out); tool_json_string(out, tool_hashes[i]); fputc('}', out);
    }
    fputc(']', out); return !ferror(out);
}
static int tool_verify_file(const char *path, const char *expected) {
    char hash[65];
    if (!expected || strlen(expected) != 64 || !tool_sha256(path, hash) || strcmp(hash, expected)) {
        fprintf(stderr, "Provenance mismatch: %s\n", path ? path : "(missing path)"); return tool_error("Provenance mismatch; rebuild before acceptance");
    }
    return 1;
}
static int tool_verify_records(unsigned slot, const char *field) {
    integration_json_node *files = integration_json_get(slot, field);
    TOOL_REQUIRE(files && files->type == PDJSON_ARRAY && files->count > 0 && files->count <= TOOL_MANIFEST_FILES, "Invalid provenance file list");
    for (int i = 0; i < files->count; ++i) {
        char key[128]; snprintf(key, sizeof(key), "%s.%d.path", field, i); const char *path = tool_text(slot, key);
        snprintf(key, sizeof(key), "%s.%d.sha256", field, i); const char *hash = tool_text(slot, key);
        TOOL_REQUIRE(tool_relative(path) && tool_verify_file(path, hash), "Invalid/changed provenance input");
    }
    return 1;
}
static int tool_verify_stack(void) {
    TOOL_REQUIRE(tool_json_read(1, "build/build-manifest.json"), "Missing/invalid stack manifest; rebuild GUI stack");
    const char *platform = tool_text(1, "platform"), *architecture = tool_text(1, "architecture");
    TOOL_REQUIRE((!platform && !strcmp(tool_platform(), "win32")) || (platform && !strcmp(platform, tool_platform())), "GUI stack targets another platform");
    TOOL_REQUIRE(!architecture || !strcmp(architecture, tool_architecture()), "GUI stack targets another architecture");
    TOOL_REQUIRE(tool_verify_records(1, "files"), "GUI stack inputs changed");
    const char *library = tool_text(1, "library"); char fallback[NATIVE_PATH_SIZE];
    TOOL_REQUIRE(tool_path(fallback, "build/bin", tool_library()), "Library path exceeds capacity");
    TOOL_REQUIRE(tool_verify_file(library ? library : fallback, tool_text(1, "dllSha256")), "GUI stack binary changed");
    return 1;
}
static int tool_gui_directory(char out[NATIVE_PATH_SIZE]) {
    char variant[128]; TOOL_REQUIRE(tool_environment("ATTENDANT_GUI_VARIANT", variant, sizeof(variant)), "Cannot read GUI variant");
    for (usize i = 0; variant[i]; ++i) TOOL_REQUIRE((variant[i] >= 'a' && variant[i] <= 'z') || (variant[i] >= '0' && variant[i] <= '9') || (i && variant[i] == '-'), "Invalid GUI variant");
    TOOL_REQUIRE(snprintf(out, NATIVE_PATH_SIZE, "build/gui%s%s", *variant ? "-" : "", variant) < NATIVE_PATH_SIZE, "GUI variant exceeds capacity"); return 1;
}
static int tool_verify_gui(const char *compiler, int testing) {
    TOOL_REQUIRE(!strcmp(compiler, "cjit") || !strcmp(compiler, "zig"), "Invalid GUI compiler");
    char directory[NATIVE_PATH_SIZE], path[NATIVE_PATH_SIZE];
    TOOL_REQUIRE(tool_gui_directory(directory), "Cannot select GUI directory");
    TOOL_REQUIRE(snprintf(path, sizeof(path), "%s/attendant-gui-%s%s.json", directory, compiler, testing ? "-check" : "") < (int)sizeof(path), "GUI manifest path exceeds capacity");
    TOOL_REQUIRE(tool_json_read(0, path), "Missing/invalid GUI manifest; rebuild GUI");
    const char *actual = tool_text(0, "compiler"), *platform = tool_text(0, "platform"), *architecture = tool_text(0, "architecture");
    integration_json_node *mode = integration_json_get(0, "testing");
    TOOL_REQUIRE(actual && !strcmp(actual, compiler) && platform && !strcmp(platform, tool_platform()) && architecture && !strcmp(architecture, tool_architecture()) && mode && mode->type == (testing ? PDJSON_TRUE : PDJSON_FALSE), "GUI compiler/platform/mode mismatch");
    TOOL_REQUIRE(tool_gui_inputs(), "Cannot inventory GUI inputs");
    integration_json_node *inputs = integration_json_get(0, "inputs");
    TOOL_REQUIRE(inputs && inputs->type == PDJSON_ARRAY && inputs->count == (int)tool_path_count, "GUI input identities changed; rebuild GUI");
    for (usize i = 0; i < tool_path_count; ++i) {
        char key[128]; snprintf(key, sizeof(key), "inputs.%zu.path", i); const char *name = tool_text(0, key);
        snprintf(key, sizeof(key), "inputs.%zu.sha256", i); const char *hash = tool_text(0, key);
        TOOL_REQUIRE(name && hash && !strcmp(name, tool_paths[i]) && !strcmp(hash, tool_hashes[i]), "GUI build inputs changed; rebuild GUI");
    }
    TOOL_REQUIRE(tool_verify_stack(), "GUI stack changed");
    TOOL_REQUIRE(tool_verify_file("build/build-manifest.json", tool_text(0, "stack.sha256")), "GUI stack manifest changed");
    const char *dll = tool_text(0, "stack.dllSha256"), *stack_dll = tool_text(1, "dllSha256");
    TOOL_REQUIRE(dll && stack_dll && !strcmp(dll, stack_dll), "GUI stack library identity changed");
    TOOL_REQUIRE(integration_json_equal(&integration_documents[0], integration_json_get(0, "stack.dependencies"), &integration_documents[1], integration_json_get(1, "dependencies"), 0), "GUI dependency identity changed");
    TOOL_REQUIRE(tool_path(path, directory, tool_library()) && tool_verify_file(path, stack_dll), "Copied GUI library changed");
    return tool_verify_records(0, "binaries");
}
static int tool_stack_summary(FILE *out) {
    char hash[65]; TOOL_REQUIRE(tool_verify_stack() && tool_sha256("build/build-manifest.json", hash), "Cannot summarize verified stack");
    fputs("{\"sha256\":", out); tool_json_string(out, hash); fputs(",\"dllSha256\":", out); tool_json_string(out, tool_text(1, "dllSha256"));
    fputs(",\"dependencies\":", out); TOOL_REQUIRE(tool_json_node(out, 1, integration_json_get(1, "dependencies")), "Cannot serialize dependency identity"); fputc('}', out); return !ferror(out);
}
static int tool_gui_manifest(const char *compiler, int testing) {
    TOOL_REQUIRE(!strcmp(compiler, "cjit") || !strcmp(compiler, "zig"), "Invalid GUI compiler");
    TOOL_REQUIRE(tool_gui_inputs(), "Cannot hash GUI inputs");
    char directory[NATIVE_PATH_SIZE], path[NATIVE_PATH_SIZE], executable[NATIVE_PATH_SIZE], command[NATIVE_PATH_SIZE], timestamp[32], version[65536]; usize size;
    TOOL_REQUIRE(tool_gui_directory(directory) && tool_timestamp(timestamp), "Cannot prepare GUI manifest");
    TOOL_REQUIRE(tool_environment(!strcmp(compiler, "zig") ? "ZIG" : "CJIT", command, sizeof(command)), "Cannot read compiler command"); if (!*command) strcpy(command, compiler);
    TOOL_REQUIRE(tool_version(command, !strcmp(compiler, "zig") ? "version" : "--help", version, sizeof(version), &size), "Cannot identify compiler");
    if (size > 1000) { string_view text = string_view_from_raw((u8 *)version, size); size = string_view_utf8_prefix(text, 1000); version[size] = 0; }
    TOOL_REQUIRE(snprintf(path, sizeof(path), "%s/attendant-gui-%s%s.json", directory, compiler, testing ? "-check" : "") < (int)sizeof(path), "GUI manifest path exceeds capacity");
    FILE *out = file_open_utf8(path, "wb"); TOOL_REQUIRE(out, "Cannot open GUI manifest"); int ok = 1;
    fputs("{\"builtAt\":", out); tool_json_string(out, timestamp); fputs(",\"platform\":", out); tool_json_string(out, tool_platform()); fputs(",\"architecture\":", out); tool_json_string(out, tool_architecture());
    fputs(",\"compiler\":", out); tool_json_string(out, compiler); fprintf(out, ",\"testing\":%s,\"compilerCommand\":", testing ? "true" : "false"); tool_json_string(out, command);
    fputs(",\"version\":{\"status\":0,\"text\":", out); tool_json_string(out, version); fputs("},\"inputs\":", out); ok = tool_write_files(out);
    fputs(",\"stack\":", out); if (ok) ok = tool_stack_summary(out);
    fputs(",\"binaries\":[", out);
#if defined(_WIN32)
    const char *suffix = ".exe";
#else
    const char *suffix = "";
#endif
    unsigned count = testing ? 2 : 1;
#if defined(__APPLE__)
    if (!strcmp(compiler, "cjit")) ++count;
#endif
    for (unsigned i = 0; ok && i < count; ++i) {
        if (i) fputc(',', out);
        if (!i) snprintf(executable, sizeof(executable), "%s/attendant-gui-%s%s%s", directory, compiler, testing ? "-check" : "", suffix);
        else if (testing && i == 1) snprintf(executable, sizeof(executable), "%s/model-check-%s%s", directory, compiler, suffix);
        else snprintf(executable, sizeof(executable), "%s/link-cjit%s/%s", directory, testing ? "-check" : "", tool_library());
        char hash[65]; ok = tool_sha256(executable, hash);
        if (ok) { fputs("{\"path\":", out); tool_json_string(out, executable); fputs(",\"sha256\":", out); tool_json_string(out, hash); fputc('}', out); }
    }
    fputs("]}\n", out); if (ferror(out)) ok = 0; if (fclose(out)) ok = 0;
    TOOL_REQUIRE(ok, "Cannot complete GUI manifest"); return tool_verify_gui(compiler, testing);
}
static int tool_stack_manifest(void) {
    TOOL_REQUIRE(tool_json_read(0, "build/dependencies.json"), "Missing/invalid dependency metadata");
    tool_path_count = 0;
    const char *paths[] = {"vendor/gui/richmd_c.cpp", "vendor/gui/richmd_c.h", "vendor/gui/preview-selection.cpp.inc", "tools/build-gui-stack.sh", "tools/native-tool.c", "tools/native-tool.sh", "tools/native-build.sh", "tools/capture-command.sh"};
    for (usize i = 0; i < sizeof(paths) / sizeof(paths[0]); ++i) TOOL_REQUIRE(tool_add_path(paths[i]), "Cannot record stack source");
    TOOL_REQUIRE(tool_walk_inputs("tools/native"), "Cannot inventory native tooling");
#if defined(__APPLE__)
    TOOL_REQUIRE(tool_add_path("tools/build-gui-stack-macos.sh"), "Cannot record macOS stack script");
#endif
    integration_json_node *dependencies = integration_json_get(0, "dependencies");
    TOOL_REQUIRE(dependencies && dependencies->type == PDJSON_ARRAY && dependencies->count > 0, "Invalid dependency metadata");
    for (int i = 0; i < dependencies->count; ++i) {
        char key[80], folder[NATIVE_PATH_SIZE]; snprintf(key, sizeof(key), "dependencies.%d.0", i); const char *name = tool_text(0, key);
        TOOL_REQUIRE(name && tool_path(folder, "build/deps", name), "Invalid dependency name");
        const char *arguments[] = {"git", "-C", folder, "ls-files", "-z"}; usize size;
        TOOL_REQUIRE(tool_capture(arguments, 5, tool_bytes[2], sizeof(tool_bytes[2]), &size), "Cannot inventory pinned dependency");
        for (usize offset = 0; offset < size;) {
            const char *relative = tool_bytes[2] + offset; usize length = strlen(relative); char path[NATIVE_PATH_SIZE];
            TOOL_REQUIRE(length < size - offset && tool_path(path, folder, relative), "Invalid/overlong Git input path");
            if (file_is_regular(path, 0)) TOOL_REQUIRE(tool_add_path(path), "Cannot record dependency input"); offset += length + 1;
        }
    }
    const char *generated[] = {"luajit.h", "host/buildvm_arch.h", "lj_bcdef.h", "lj_ffdef.h", "lj_libdef.h", "lj_recdef.h", "lj_folddef.h", "jit/vmdef.lua"};
    for (unsigned i = 0; i < 8; ++i) { char path[NATIVE_PATH_SIZE]; TOOL_REQUIRE(tool_path(path, "build/deps/luajit/src", generated[i]) && tool_add_path(path), "Cannot record LuaJIT generator input"); }
    TOOL_REQUIRE(tool_hash_paths(), "Cannot hash stack sources");
    char files[NATIVE_PATH_SIZE], aggregate[65]; TOOL_REQUIRE(tool_temporary(files, ".files.json"), "Cannot reserve canonical file list");
    FILE *out = file_open_utf8(files, "wb"); TOOL_REQUIRE(out, "Cannot write canonical file list"); int ok = tool_write_files(out); if (fclose(out)) ok = 0;
    TOOL_REQUIRE(ok && tool_sha256(files, aggregate), "Cannot hash canonical file list");
    char timestamp[32], zig[NATIVE_PATH_SIZE], version[65536], generator_version[65536]; usize size;
    TOOL_REQUIRE(tool_environment("ZIG", zig, sizeof(zig)) && tool_timestamp(timestamp), "Cannot prepare stack identity"); if (!*zig) strcpy(zig, "zig");
    const char *version_args[] = {zig, "version"}; TOOL_REQUIRE(tool_capture(version_args, 2, version, sizeof(version), &size), "Cannot identify Zig"); tool_normalize(version, &size, 1);
    const char *generator_args[] = {"zig", "version"}; TOOL_REQUIRE(tool_capture(generator_args, 2, generator_version, sizeof(generator_version), &size), "Cannot identify generator Zig"); tool_normalize(generator_version, &size, 1);
    char library[NATIVE_PATH_SIZE], library_hash[65], generator_hash[65]; TOOL_REQUIRE(tool_path(library, "build/bin", tool_library()) && tool_sha256(library, library_hash), "Cannot hash stack library");
#if defined(_WIN32)
    const char *generator = "build/bin/luajit.exe";
#else
    const char *generator = "build/bin/luajit";
#endif
    TOOL_REQUIRE(tool_sha256(generator, generator_hash), "Cannot hash LuaJIT generator");
    out = file_open_utf8("build/build-manifest.json", "wb"); TOOL_REQUIRE(out, "Cannot open stack manifest");
    fputs("{\"builtAt\":", out); tool_json_string(out, timestamp); fputs(",\"zig\":", out); tool_json_string(out, version); fputs(",\"generatorZig\":", out); tool_json_string(out, generator_version);
    fputs(",\"dependencies\":", out); ok = tool_json_node(out, 0, integration_json_get(0, "")); fputs(",\"files\":", out); if (!tool_copy_stream(out, files)) ok = 0;
    fputs(",\"sourceTreeSha256\":", out); tool_json_string(out, aggregate); fputs(",\"platform\":", out); tool_json_string(out, tool_platform()); fputs(",\"architecture\":", out); tool_json_string(out, tool_architecture());
#if defined(_WIN32)
    fputs(",\"backend\":\"win32-d3d11\"", out);
#else
    char sdk[NATIVE_PATH_SIZE], target[128]; tool_environment("SDKROOT", sdk, sizeof(sdk)); tool_environment("MACOSX_DEPLOYMENT_TARGET", target, sizeof(target));
    if (!*sdk) { const char *args[] = {"xcrun", "--show-sdk-path"}; if (!tool_capture(args, 2, sdk, sizeof(sdk), &size)) ok = 0; else tool_normalize(sdk, &size, 1); }
    fputs(",\"backend\":\"glfw-opengl3\",\"sdk\":", out); tool_json_string(out, sdk); if (*target) { fputs(",\"deploymentTarget\":", out); tool_json_string(out, target); }
#endif
    fputs(",\"library\":", out); tool_json_string(out, library); fputs(",\"generator\":", out); tool_json_string(out, generator); fputs(",\"dllSha256\":", out); tool_json_string(out, library_hash); fputs(",\"generatorSha256\":", out); tool_json_string(out, generator_hash); fputs("}\n", out);
    if (ferror(out)) ok = 0; if (fclose(out)) ok = 0; TOOL_REQUIRE(ok, "Cannot complete stack manifest"); return tool_verify_stack();
}
#endif
