/* Entire application is C99. DLL owns only C++ dependencies and native renderer.
 * Usage: host --assets DIR [--frames 1..6000 --shot FILE.ppm] [--no-preview] [--exercise]
 * No file writes except an explicitly requested screenshot. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cimgui.h"
#include "richmd_c.h"

static char markdown[65536] =
    "# C99 + cimgui + RichMd\n\n"
    "Application: **C99**, compiled by CJIT or Zig. Dependencies: Zig-built DLL.\n\n"
    "## Live preview\n\n"
    "Edit Markdown on the left. Select text here, then use **Ctrl+C**.\n\n"
    "A [local reference](attendant://tasks/example) invokes a **C callback**, not a browser.\n\n"
    "- **Bold**, *italic*, `inline code`, ~~strike~~\n"
    "- UTF-8: caf\xc3\xa9, I\xc3\xb1igo, \xc2\xb5s\n"
    "- Nested content stays in the same ImGui context\n\n"
    "| Component | Version | Boundary |\n|---|---|---|\n"
    "| Dear ImGui | 1.93.0 | regenerated cimgui |\n"
    "| RichMd | 1.0.0 | narrow extern-C API |\n\n"
    "```c\ncrmd_render(markdown, strlen(markdown));\n```\n\n"
    "> [!NOTE]\n> Text selection is available. Exact source-range export is not yet implemented.\n\n"
    "![Bundled image](images/world.png)\n";
static char last_link[1024] = "No link clicked";
static char copied_text[4096];
/* Test clipboard stays in process: never overwrite user's Windows clipboard. */
static const char *clipboard_get(ImGuiContext *context) { (void)context; return copied_text; }
static void clipboard_set(ImGuiContext *context, const char *text) {
    (void)context;
    snprintf(copied_text, sizeof(copied_text), "%s", text);
}
static void exercise_input(HWND window, int frame) {
    ImGuiIO *io = igGetIO_Nil();
    /* Fixed 1280x940 test window, default fonts. Inject backend/ImGui events,
     * never move OS cursor or send global keyboard input. */
    if (frame >= 3 && frame <= 6) crmd_message(window, WM_MOUSEMOVE, 0, MAKELPARAM(30, 64));
    if (frame == 3) ImGuiIO_AddMouseButtonEvent(io, 0, true);
    if (frame == 4) ImGuiIO_AddMouseButtonEvent(io, 0, false);
    if (frame == 6) ImGuiIO_AddInputCharactersUTF8(io, "!");
    if (frame >= 10 && frame <= 13) crmd_message(window, WM_MOUSEMOVE, 0, MAKELPARAM(658, 216));
    if (frame == 10) ImGuiIO_AddMouseButtonEvent(io, 0, true);
    if (frame == 11) ImGuiIO_AddMouseButtonEvent(io, 0, false);
    if (frame >= 15 && frame <= 24) crmd_message(window, WM_MOUSEMOVE, 0, MAKELPARAM(frame < 17 ? 599 : 920, 188));
    if (frame == 15) ImGuiIO_AddMouseButtonEvent(io, 0, true);
    if (frame == 19) ImGuiIO_AddMouseButtonEvent(io, 0, false);
    if (frame == 22) {
        ImGuiIO_AddKeyEvent(io, ImGuiMod_Ctrl, true);
        ImGuiIO_AddKeyEvent(io, ImGuiKey_C, true);
    }
    if (frame == 24) {
        ImGuiIO_AddKeyEvent(io, ImGuiKey_C, false);
        ImGuiIO_AddKeyEvent(io, ImGuiMod_Ctrl, false);
    }
}
static void link_clicked(const char *url, void *user) {
    int *count = (int *)user;
    ++*count;
    snprintf(last_link, sizeof(last_link), "%s", url);
    fprintf(stderr, "link: %s\n", url);
}
static LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    if (crmd_message(window, message, wparam, lparam)) return 1;
    if (message == WM_DESTROY) { PostQuitMessage(0); return 0; }
    return DefWindowProcA(window, message, wparam, lparam);
}
static int usage(void) {
    fprintf(stderr, "Usage: host --assets DIR [--frames 1..6000 --shot FILE.ppm] [--no-preview] [--exercise]\n");
    return 2;
}
int main(int argc, char **argv) {
    const char *assets = NULL, *shot = NULL;
    const size_t original_length = strlen(markdown);
    int frame_limit = 0, frame = 0, status = 1, links = 0, i;
    bool preview = true, running = true, exercise = false;
    HWND window;
    WNDCLASSA description;
    ImGuiContext *context;
    MSG message;
    for (i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--assets") && i + 1 < argc) assets = argv[++i];
        else if (!strcmp(argv[i], "--shot") && i + 1 < argc) shot = argv[++i];
        else if (!strcmp(argv[i], "--frames") && i + 1 < argc) {
            char *end;
            long value = strtol(argv[++i], &end, 10);
            if (*end || value < 1 || value > 6000) return usage();
            frame_limit = (int)value;
        } else if (!strcmp(argv[i], "--no-preview")) preview = false;
        else if (!strcmp(argv[i], "--exercise")) exercise = true;
        else return usage();
    }
    if (!assets || (shot && !frame_limit) || (exercise && (frame_limit < 30 || !preview))) return usage();
    memset(&description, 0, sizeof(description));
    description.lpfnWndProc = window_proc;
    description.hInstance = GetModuleHandleA(NULL);
    description.lpszClassName = "C99RichMdSpike";
    description.hCursor = LoadCursorA(NULL, IDC_ARROW);
    if (!RegisterClassA(&description)) { fprintf(stderr, "Cannot register window\n"); return 1; }
    context = igCreateContext(NULL);
    if (!igDebugCheckVersionAndDataLayout(igGetVersion(), sizeof(ImGuiIO), sizeof(ImGuiStyle),
            sizeof(ImVec2), sizeof(ImVec4), sizeof(ImDrawVert), sizeof(ImDrawIdx))) {
        fprintf(stderr, "cimgui ABI layout mismatch\n"); goto cleanup_context;
    }
    {
        const size_t layout[] = {sizeof(ImGuiStoragePair), offsetof(ImGuiStoragePair, value),
            sizeof(ImGuiStyleMod), offsetof(ImGuiStyleMod, backup),
            sizeof(ImGuiInputEvent), offsetof(ImGuiInputEvent, event)};
        if (!crmd_check_cimgui_layout(layout, sizeof(layout) / sizeof(layout[0]))) {
            fprintf(stderr, "%s\n", crmd_error()); goto cleanup_context;
        }
    }
    igGetIO_Nil()->IniFilename = NULL;
    igStyleColorsDark(NULL);
    window = CreateWindowExA(0, description.lpszClassName, "C99 / CJIT + Zig DLL: ImGui 1.93 + RichMd 1.0",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1280, 940,
        NULL, NULL, description.hInstance, NULL);
    if (!window) { fprintf(stderr, "Cannot create window\n"); goto cleanup_context; }
    if (!crmd_init(window, assets, link_clicked, &links)) goto cleanup_window;
    if (exercise) {
        igGetPlatformIO_Nil()->Platform_GetClipboardTextFn = clipboard_get;
        igGetPlatformIO_Nil()->Platform_SetClipboardTextFn = clipboard_set;
    }
    ShowWindow(window, SW_SHOWDEFAULT);
    UpdateWindow(window);
    while (running && (!frame_limit || frame < frame_limit)) {
        while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
            if (message.message == WM_QUIT) running = false;
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }
        if (!running) break;
        if (exercise) exercise_input(window, frame);
        if (!crmd_begin_frame()) goto cleanup_adapter;
        {
            ImGuiViewport *viewport = igGetMainViewport();
            ImVec2 zero = {0, 0}, editor_size, remaining;
            char summary[256];
            igSetNextWindowPos(viewport->WorkPos, ImGuiCond_Always, zero);
            igSetNextWindowSize(viewport->WorkSize, ImGuiCond_Always);
            igBegin("Shared Markdown spike", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
            snprintf(summary, sizeof(summary), "Dear ImGui %s | RichMd %s | application: C99 | link callbacks: %d",
                igGetVersion(), crmd_version(), links);
            igTextUnformatted(summary, NULL);
            igCheckbox("RichMd preview", &preview);
            igSameLine(0, -1);
            igTextUnformatted(last_link, NULL);
            igSeparator();
            remaining = igGetContentRegionAvail();
            editor_size.x = remaining.x * 0.46f;
            editor_size.y = remaining.y;
            igInputTextMultiline("##source", markdown, sizeof(markdown), editor_size, ImGuiInputTextFlags_AllowTabInput, NULL, NULL);
            igSameLine(0, -1);
            remaining.x = 0; remaining.y = 0;
            igBeginChild_Str("preview", remaining, ImGuiChildFlags_Borders, 0);
            if (preview && !crmd_render(markdown, strlen(markdown))) {
                igEndChild(); igEnd(); goto cleanup_adapter;
            }
            igEndChild();
            igEnd();
        }
        if (!crmd_end_frame()) goto cleanup_adapter;
        ++frame;
        if (shot && frame == frame_limit && !crmd_screenshot(shot)) goto cleanup_adapter;
        if (!crmd_present()) goto cleanup_adapter;
    }
    if (frame_limit && frame != frame_limit) { fprintf(stderr, "Window closed before smoke completed\n"); goto cleanup_adapter; }
    if (exercise) {
        bool edited = strlen(markdown) == original_length + 1 && strchr(markdown, '!') < strchr(markdown, '\n');
        if (!edited || links != 1 || !strstr(copied_text, "Markdown")) {
            fprintf(stderr, "Interaction exercise failed: edited=%d links=%d clipboard='%s'\n", edited, links, copied_text);
            goto cleanup_adapter;
        }
    }
    status = 0;
cleanup_adapter:
    if (!crmd_shutdown()) status = 1;
    if (crmd_live_images() != 0) { fprintf(stderr, "Image textures survived shutdown\n"); status = 1; }
cleanup_window:
    if (status) fprintf(stderr, "RichMd adapter: %s\n", crmd_error());
    DestroyWindow(window);
cleanup_context:
    igDestroyContext(context);
    UnregisterClassA(description.lpszClassName, description.hInstance);
    if (!status) printf("{\"ok\":true,\"imgui\":\"%s\",\"richmd\":\"%s\",\"frames\":%d,\"vertices\":%d,\"links\":%d,\"exercise\":%s,\"copied_bytes\":%u}\n",
        igGetVersion(), crmd_version(), frame, crmd_vertex_count(), links, exercise ? "true" : "false", (unsigned)strlen(copied_text));
    return status;
}
