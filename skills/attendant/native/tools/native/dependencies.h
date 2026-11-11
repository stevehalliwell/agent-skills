#ifndef ATTENDANT_NATIVE_TOOL_DEPENDENCIES_H
#define ATTENDANT_NATIVE_TOOL_DEPENDENCIES_H
/* Pinned dependency policy and strict ordered source patches. */
static const char *tool_dependencies[][3] = {
    {"cimgui", "https://github.com/cimgui/cimgui.git", "2a6a93fccd10f275ee9206167dc9698f6f00fac1"},
    {"cimgui/imgui", "https://github.com/ocornut/imgui.git", "c1aaac65e896a45f81f47a4d68f9959e0f86c859"},
    {"richmd", "https://github.com/pthom/imgui_rich_md.git", "609039721f838108251520079fff79011cf46659"},
    {"richmd/external/md4c", "https://github.com/mity/md4c.git", "481fbfbdf72daab2912380d62bb5f2187d438408"},
    {"luajit", "https://github.com/LuaJIT/LuaJIT.git", "a4c064539ea931274d407e1249b4bb575424a0f4"},
#if defined(__APPLE__)
    {"glfw", "https://github.com/glfw/glfw.git", "7b6aead9fb88b3623e3b3725ebb42670cbe4c579"},
#endif
};
static int tool_replace(char *text, usize capacity, const char *before, const char *after, int occurrence) {
    TOOL_REQUIRE(before && before[0] && after, "Invalid patch operation");
    char *first = strstr(text, before), *second = first ? strstr(first + strlen(before), before) : NULL;
    TOOL_REQUIRE(first && ((occurrence == 1 && !second) || (occurrence == 2 && second && !strstr(second + strlen(before), before))), "Missing/nonunique patch anchor; refusing source drift");
    char *at = occurrence == 2 ? second : first;
    usize old = strlen(before), replacement = strlen(after), length = strlen(text), offset = (usize)(at - text);
    TOOL_REQUIRE(length - old + replacement < capacity, "Patched source exceeds capacity");
    memmove(at + replacement, at + old, length - offset - old + 1); memcpy(at, after, replacement); return 1;
}
static int tool_apply_patch(const char *repository, const char *description, int verify) {
    TOOL_REQUIRE(tool_json_read(0, description), "Invalid patch data");
    integration_json_node *patches = integration_json_get(0, "");
    TOOL_REQUIRE(patches && patches->type == PDJSON_ARRAY && patches->count > 0, "Patch description must be nonempty array");
    for (int i = 0; i < patches->count; ++i) {
        char key[128], path[NATIVE_PATH_SIZE], specification[NATIVE_PATH_SIZE];
        snprintf(key, sizeof(key), "%d.path", i); const char *relative = tool_text(0, key);
        TOOL_REQUIRE(relative && relative[0] && !strstr(relative, "..") && relative[0] != '/' && !strchr(relative, '\\'), "Unsafe patch path");
        TOOL_REQUIRE(tool_path(path, repository, relative), "Patch path exceeds capacity");
        TOOL_REQUIRE(snprintf(specification, sizeof(specification), "HEAD:%s", relative) < (int)sizeof(specification), "Git source path exceeds capacity");
        const char *arguments[] = {"git", "-C", repository, "show", specification}; usize original_size;
        TOOL_REQUIRE(tool_capture(arguments, 5, tool_bytes[0], sizeof(tool_bytes[0]), &original_size), "Cannot read pinned patch input");
        TOOL_REQUIRE(!memchr(tool_bytes[0], 0, original_size) && string_view_utf8_valid(string_view_from_raw((u8 *)tool_bytes[0], original_size)), "Invalid UTF-8/NUL in pinned source");
        tool_normalize(tool_bytes[0], &original_size, 1); memcpy(tool_bytes[1], tool_bytes[0], original_size + 1);
        snprintf(key, sizeof(key), "%d.operations", i); integration_json_node *operations = integration_json_get(0, key);
        TOOL_REQUIRE(operations && operations->type == PDJSON_ARRAY && operations->count > 0, "Missing patch operations");
        for (int j = 0; j < operations->count; ++j) {
            snprintf(key, sizeof(key), "%d.operations.%d.prefix", i, j); const char *prefix = tool_text(0, key);
            if (prefix) {
                usize count = strlen(prefix), length = strlen(tool_bytes[1]);
                TOOL_REQUIRE(length + count < sizeof(tool_bytes[1]), "Patch prefix exceeds capacity");
                memmove(tool_bytes[1] + count, tool_bytes[1], length + 1); memcpy(tool_bytes[1], prefix, count);
            } else {
                snprintf(key, sizeof(key), "%d.operations.%d.before", i, j); const char *before = tool_text(0, key);
                snprintf(key, sizeof(key), "%d.operations.%d.after", i, j); const char *after = tool_text(0, key);
                TOOL_REQUIRE(before && after && strlen(after) < sizeof(tool_bytes[2]), "Invalid replacement patch data");
                strcpy(tool_bytes[2], after);
                const char *marker = "__NATIVE_PREVIEW_METHODS__";
                if (strstr(tool_bytes[2], marker)) {
                    static char preview[262145]; usize size;
                    TOOL_REQUIRE(tool_read("vendor/gui/preview-selection.cpp.inc", preview, sizeof(preview), &size), "Cannot read authoritative preview methods");
                    TOOL_REQUIRE(tool_replace(tool_bytes[2], sizeof(tool_bytes[2]), marker, preview, 1), "Invalid preview include marker");
                }
                TOOL_REQUIRE(tool_replace(tool_bytes[1], sizeof(tool_bytes[1]), before, tool_bytes[2], 1), "Strict source patch failed");
            }
        }
        usize current_size; TOOL_REQUIRE(tool_read(path, tool_bytes[2], sizeof(tool_bytes[2]), &current_size), "Cannot read patch target");
        TOOL_REQUIRE(!memchr(tool_bytes[2], 0, current_size) && string_view_utf8_valid(string_view_from_raw((u8 *)tool_bytes[2], current_size)), "Invalid UTF-8/NUL in patch target; refusing overwrite");
        tool_normalize(tool_bytes[2], &current_size, 1);
        int expected = !strcmp(tool_bytes[2], tool_bytes[1]);
        TOOL_REQUIRE(expected || !strcmp(tool_bytes[2], tool_bytes[0]), "Unknown local source edits; refusing overwrite");
        if (verify) TOOL_REQUIRE(expected, "Missing expected source patch; run tools/native-tool.sh prepare explicitly");
        else {
            usize size = strlen(tool_bytes[1]); TOOL_REQUIRE(size + 1 < sizeof(tool_bytes[1]), "Patch output exceeds capacity");
            tool_bytes[1][size++] = '\n'; tool_bytes[1][size] = 0;
            TOOL_REQUIRE(file_write_bytes((cstring){path}, tool_bytes[1], size) == file_error_ok, "Cannot write patched source");
        }
    }
    return 1;
}
static int tool_allowed_dependency_edit(const char *name, const char *path) {
    if (!strcmp(name, "cimgui")) {
        const char *allowed[] = {"imgui", "cimgui.h", "cimgui.cpp", "generator/output/constants.json", "generator/output/constants.lua", "generator/output/definitions.json", "generator/output/definitions.lua", "generator/output/structs_and_enums.json", "generator/output/structs_and_enums.lua", "generator/output/overloads.txt"};
        for (usize i = 0; i < sizeof(allowed) / sizeof(allowed[0]); ++i) if (!strcmp(path, allowed[i])) return 1;
    } else if (!strcmp(name, "cimgui/imgui")) return !strcmp(path, "imgui.h");
    else if (!strcmp(name, "richmd")) {
        const char *allowed[] = {"imgui_rich_md/rich_md.h", "imgui_rich_md/internal/rich_md_renderer.h", "imgui_rich_md/internal/rich_md_renderer.cpp", "imgui_rich_md/rich_md.cpp"};
        for (usize i = 0; i < 4; ++i) if (!strcmp(path, allowed[i])) return 1;
    }
    return 0;
}
static int tool_prepare(int verify) {
    for (usize i = 0; i < sizeof(tool_dependencies) / sizeof(tool_dependencies[0]); ++i) {
        const char *name = tool_dependencies[i][0], *url = tool_dependencies[i][1], *commit = tool_dependencies[i][2];
        char folder[NATIVE_PATH_SIZE], git_path[NATIVE_PATH_SIZE];
        TOOL_REQUIRE(tool_path(folder, "build/deps", name) && tool_path(git_path, folder, ".git"), "Dependency path exceeds capacity");
        if (!file_is_directory(git_path, 0) && !file_is_regular(git_path, 0)) {
            TOOL_REQUIRE(!verify, "Missing dependency; run tools/native-tool.sh prepare explicitly");
            const char *initialize[] = {"git", "init", folder}; TOOL_REQUIRE(tool_command(initialize, 3), "Cannot initialize dependency");
            const char *remote[] = {"git", "-C", folder, "remote", "add", "origin", url}; TOOL_REQUIRE(tool_command(remote, 7), "Cannot configure dependency remote");
            const char *fetch[] = {"git", "-C", folder, "fetch", "--depth", "1", "origin", commit}; TOOL_REQUIRE(tool_command(fetch, 8), "Cannot fetch pinned dependency");
            const char *checkout[] = {"git", "-C", folder, "checkout", "--detach", "FETCH_HEAD"}; TOOL_REQUIRE(tool_command(checkout, 6), "Cannot check out pinned dependency");
        }
        const char *revision[] = {"git", "-C", folder, "rev-parse", "HEAD"}; usize size;
        TOOL_REQUIRE(tool_capture(revision, 5, tool_bytes[2], sizeof(tool_bytes[2]), &size), "Cannot inspect dependency revision");
        tool_normalize(tool_bytes[2], &size, 1); TOOL_REQUIRE(!strcmp(tool_bytes[2], commit), "Wrong pinned dependency revision");
        const char *diff[] = {"git", "-C", folder, "diff", "HEAD", "--name-only", "-z"};
        TOOL_REQUIRE(tool_capture(diff, 7, tool_bytes[2], sizeof(tool_bytes[2]), &size), "Cannot inspect tracked dependency edits");
        for (usize offset = 0; offset < size;) {
            const char *path = tool_bytes[2] + offset; usize length = strlen(path);
            TOOL_REQUIRE(length < size - offset && tool_allowed_dependency_edit(name, path), "Unexpected tracked dependency edit; refusing overwrite");
            offset += length + 1;
        }
    }
    TOOL_REQUIRE(tool_apply_patch("build/deps/cimgui/imgui", "tools/native/patches/imgui.json", verify), "ImGui source verification/patch failed");
    TOOL_REQUIRE(tool_apply_patch("build/deps/richmd", "tools/native/patches/richmd.json", verify), "RichMd source verification/patch failed");
    FILE *out = file_open_utf8("build/dependencies.json", "wb"); TOOL_REQUIRE(out, "Cannot write dependency manifest");
    fputs("{\"dependencies\":[", out);
    for (usize i = 0; i < sizeof(tool_dependencies) / sizeof(tool_dependencies[0]); ++i) {
        if (i) fputc(',', out); fputc('[', out);
        for (unsigned j = 0; j < 3; ++j) { if (j) fputc(',', out); tool_json_string(out, tool_dependencies[i][j]); }
        fputc(']', out);
    }
    fputs("],\"patch\":\"Guard IM_NODEBUGSTEP; strict original-input RichMd source-selection snapshot with explicit run provenance\"}\n", out);
    int ok = !ferror(out); if (fclose(out)) ok = 0; TOOL_REQUIRE(ok, "Cannot finish dependency manifest");
    puts(verify ? "{\"ok\":true,\"verified\":true}" : "{\"ok\":true,\"prepared\":true}"); return 1;
}
static int tool_fix_generated_text(void) {
    const char *names[] = {"ImDrawTextFlags", "ImTextureRef", "ImVec2", "ImVec2i", "ImVec4", "ImColor", "ImRect"};
    for (unsigned i = 0; i < 7; ++i) {
        char declaration[128];
        snprintf(declaration, sizeof(declaration), i ? "typedef struct %s_c %s;" : "typedef int %s;", names[i], names[i]);
        TOOL_REQUIRE(tool_replace(tool_bytes[0], sizeof(tool_bytes[0]), declaration, "", 2), "Generated typedef count changed");
    }
    const char *before[] = {"union { int val_i; float val_f; void* val_p; };", "union { int BackupInt[2]; float BackupFloat[2]; };", "ImGuiInputEventAppFocused AppFocused;\n    };"};
    const char *after[] = {"union { int val_i; float val_f; void* val_p; } value;", "union { int BackupInt[2]; float BackupFloat[2]; } backup;", "ImGuiInputEventAppFocused AppFocused;\n    } event;"};
    for (unsigned i = 0; i < 3; ++i) TOOL_REQUIRE(tool_replace(tool_bytes[0], sizeof(tool_bytes[0]), before[i], after[i], 1), "Anonymous union anchor changed");
    return 1;
}
static int tool_fix_generated(void) {
    const char *path = "build/deps/cimgui/cimgui.h"; usize size;
    TOOL_REQUIRE(tool_read(path, tool_bytes[0], sizeof(tool_bytes[0]), &size), "Cannot read generated cimgui header");
    TOOL_REQUIRE(!memchr(tool_bytes[0], 0, size) && string_view_utf8_valid(string_view_from_raw((u8 *)tool_bytes[0], size)), "Invalid UTF-8/NUL in generated header");
    tool_normalize(tool_bytes[0], &size, 0);
    TOOL_REQUIRE(tool_fix_generated_text(), "Generated header drift; output not written");
    TOOL_REQUIRE(file_write_bytes((cstring){(char *)path}, tool_bytes[0], strlen(tool_bytes[0])) == file_error_ok, "Cannot write generated header");
    puts("{\"ok\":true,\"fixed\":true}"); return 1;
}
#endif
