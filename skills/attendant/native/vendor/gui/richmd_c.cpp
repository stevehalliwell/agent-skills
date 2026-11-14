/* Native Win32/D3D11 or macOS GLFW/OpenGL3 adapter plus narrow RichMd C ABI. */
#define RICHMD_C_BUILD
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "richmd_c.h"
#if defined(_WIN32)
#include <windows.h>
#include <d3d11.h>
#elif defined(__APPLE__)
#ifndef GL_SILENCE_DEPRECATION
#define GL_SILENCE_DEPRECATION
#endif
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>
#include <vector>
#else
#error Unsupported RichMd host platform
#endif
#include <cstdio>
#include <cstring>
#include <exception>
#include <stdexcept>
#include <string>
#include "imgui.h"
#include "imgui_internal.h"
namespace ImStb {
#include "imstb_textedit.h"
}
#include <utility>
#if defined(_WIN32)
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx11.h"
#else
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#endif
#include "imgui_rich_md/rich_md.h"
#include "imgui_rich_md/rich_md_host.h"
#if defined(_WIN32)
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

static ID3D11Device *device;
static ID3D11DeviceContext *commands;
static IDXGISwapChain *swap_chain;
static ID3D11RenderTargetView *target;
static HWND window_handle;
#else
static GLFWwindow *window_handle;
#endif
#if defined(__APPLE__)
/* CPU transfers must not inherit PBO offsets, padded rows or skipped pixels.
 * Restore caller state on all exits, including allocation/renderer exceptions. */
struct pixel_transfer_state {
    GLenum target, properties[4];
    GLint buffer, values[4];
    explicit pixel_transfer_state(GLenum buffer_target) : target(buffer_target) {
        const bool unpack = target == GL_PIXEL_UNPACK_BUFFER;
        static const GLenum layouts[2][4] = {
            {GL_PACK_ALIGNMENT, GL_PACK_ROW_LENGTH, GL_PACK_SKIP_ROWS, GL_PACK_SKIP_PIXELS},
            {GL_UNPACK_ALIGNMENT, GL_UNPACK_ROW_LENGTH, GL_UNPACK_SKIP_ROWS, GL_UNPACK_SKIP_PIXELS}};
        const GLenum *selected = layouts[unpack ? 1 : 0];
        glGetIntegerv(unpack ? GL_PIXEL_UNPACK_BUFFER_BINDING : GL_PIXEL_PACK_BUFFER_BINDING, &buffer);
        glBindBuffer(target, 0);
        for (size_t i = 0; i < 4; ++i) {
            properties[i] = selected[i];
            glGetIntegerv(properties[i], &values[i]);
            glPixelStorei(properties[i], i == 0 ? 1 : 0);
        }
    }
    ~pixel_transfer_state() {
        for (size_t i = 0; i < 4; ++i) glPixelStorei(properties[i], values[i]);
        glBindBuffer(target, (GLuint)buffer);
    }
    pixel_transfer_state(const pixel_transfer_state &) = delete;
    pixel_transfer_state &operator=(const pixel_transfer_state &) = delete;
};
#endif
static bool platform_ready, renderer_ready;
static RichMd::Context *markdown_context;
static char error_text[1024];
static int vertices, image_textures;
static crmd_link_callback on_link;
static void *link_user;
static std::string raw_document, raw_source;
static uint64_t raw_revision, raw_interaction;
static bool raw_selectable;
static RichMd::PreviewSelection preview_snapshot;
static bool have_raw_input, render_failed;
/* Render exceptions can leave upstream ImGui scopes/splitters open. Do not resume
 * normal frame teardown; caller shuts down context without rendering another frame. */
static void render_named(const char *key, const std::string& input, bool selectable, bool inject_failure = false) {
    try {
        ImGui::PushID(key);
        if (inject_failure) {
            ImGui::PushID("nested-render-scope"); ImGui::BeginChild("render-failure", ImVec2(20, 20));
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
            throw std::runtime_error("Injected nested render failure; frame must be abandoned");
        }
        if (selectable) RichMd::RenderSelectable(input); else RichMd::RenderRaw(input);
        ImGui::PopID();
    } catch (...) { render_failed = true; throw; }
}
static_assert(static_cast<int>(RichMd::SourceSelectionStatus::None) == CRMD_SELECTION_NONE &&
    static_cast<int>(RichMd::SourceSelectionStatus::Exact) == CRMD_SELECTION_EXACT &&
    static_cast<int>(RichMd::SourceSelectionStatus::Unsupported) == CRMD_SELECTION_UNSUPPORTED, "selection status ABI");

static int fail(const char *message) {
    std::snprintf(error_text, sizeof(error_text), "%s", message);
    return 0;
}
static int exception_error() {
    try { throw; }
    catch (const std::exception &error) { return fail(error.what()); }
    catch (...) { return fail("Unknown C++ exception"); }
}
#if defined(_WIN32)
static int make_target() {
    ID3D11Texture2D *buffer = nullptr;
    HRESULT result = swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&buffer);
    if (FAILED(result)) return fail("Cannot get D3D11 back buffer");
    result = device->CreateRenderTargetView(buffer, nullptr, &target);
    buffer->Release();
    return SUCCEEDED(result) ? 1 : fail("Cannot create D3D11 render target");
}
#endif
const char *crmd_version(void) { return IMGUI_RICHMD_VERSION; }
const char *crmd_error(void) { return error_text; }
int crmd_check_cimgui_layout(const size_t *layout, size_t count) {
    const size_t expected[] = {sizeof(ImGuiStoragePair), offsetof(ImGuiStoragePair, val_i),
        sizeof(ImGuiStyleMod), offsetof(ImGuiStyleMod, BackupInt),
        sizeof(ImGuiInputEvent), offsetof(ImGuiInputEvent, MousePos)};
    if (!layout || count != sizeof(expected) / sizeof(expected[0])) return fail("Invalid ABI probe");
    for (size_t i = 0; i < count; ++i)
        if (layout[i] != expected[i]) return fail("C99 union layout mismatch");
    return 1;
}

int crmd_shutdown(void) {
    int ok = 1;
#if defined(__APPLE__)
    // RichMd texture owners release GL objects during context destruction.
    if (window_handle) glfwMakeContextCurrent(window_handle);
#endif
    try {
        if (markdown_context) RichMd::DestroyContext(markdown_context);
    } catch (...) { exception_error(); ok = 0; }
    markdown_context = nullptr;
#if defined(_WIN32)
    if (renderer_ready) ImGui_ImplDX11_Shutdown();
    if (platform_ready) ImGui_ImplWin32_Shutdown();
    if (target) target->Release();
    if (swap_chain) swap_chain->Release();
    if (commands) commands->Release();
    if (device) device->Release();
    target = nullptr; swap_chain = nullptr; commands = nullptr; device = nullptr;
#else
    if (window_handle) glfwMakeContextCurrent(window_handle);
    if (renderer_ready) ImGui_ImplOpenGL3_Shutdown();
    if (platform_ready) ImGui_ImplGlfw_Shutdown();
#endif
    renderer_ready = platform_ready = false;
    window_handle = nullptr; on_link = nullptr; link_user = nullptr;
    std::string().swap(raw_document); std::string().swap(raw_source); preview_snapshot = {};
    have_raw_input = raw_selectable = render_failed = false; raw_revision = raw_interaction = 0;
    return ok;
}
int crmd_init(void *window, const char *assets_folder, crmd_link_callback link, void *user) {
    if (!window || !assets_folder || !ImGui::GetCurrentContext() || window_handle)
        return fail("Expected ImGui context, window, assets folder, and no active adapter");
    try {
        on_link = link; link_user = user;
#if defined(_WIN32)
        window_handle = (HWND)window;
        DXGI_SWAP_CHAIN_DESC description = {};
        description.BufferCount = 2;
        description.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        description.OutputWindow = window_handle;
        description.SampleDesc.Count = 1;
        description.Windowed = TRUE;
        description.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        D3D_FEATURE_LEVEL level;
        HRESULT result = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
            0, nullptr, 0, D3D11_SDK_VERSION, &description, &swap_chain, &device, &level, &commands);
        if (FAILED(result)) {
            result = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr,
                0, nullptr, 0, D3D11_SDK_VERSION, &description, &swap_chain, &device, &level, &commands);
        }
        if (FAILED(result)) { fail("Cannot initialize D3D11 hardware or WARP device"); crmd_shutdown(); return 0; }
        if (!make_target()) { crmd_shutdown(); return 0; }
        platform_ready = ImGui_ImplWin32_Init(window_handle);
        if (!platform_ready) { fail("Cannot initialize Win32 backend"); crmd_shutdown(); return 0; }
        renderer_ready = ImGui_ImplDX11_Init(device, commands);
        if (!renderer_ready) { fail("Cannot initialize DX11 backend"); crmd_shutdown(); return 0; }
#else
        window_handle = static_cast<GLFWwindow *>(window);
        glfwMakeContextCurrent(window_handle);
        platform_ready = ImGui_ImplGlfw_InitForOpenGL(window_handle, true);
        if (!platform_ready) { fail("Cannot initialize GLFW backend"); crmd_shutdown(); return 0; }
        renderer_ready = ImGui_ImplOpenGL3_Init("#version 150");
        if (!renderer_ready) { fail("Cannot initialize OpenGL3 backend"); crmd_shutdown(); return 0; }
#endif
        // Own image textures directly. RichMd's default uploader defers CPU-side
        // texture destruction until a later render; final teardown has no later frame.
        RichMd::HostServices services;
        services.UploadRgba = [](const unsigned char *rgba, int width, int height) {
            RichMd::MarkdownTexture result;
#if defined(_WIN32)
            if (!device || !rgba || width <= 0 || height <= 0 || width > 4096 || height > 4096) return result;
            D3D11_TEXTURE2D_DESC image = {};
            image.Width = (UINT)width; image.Height = (UINT)height;
            image.MipLevels = image.ArraySize = image.SampleDesc.Count = 1;
            image.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            image.Usage = D3D11_USAGE_IMMUTABLE;
            image.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            D3D11_SUBRESOURCE_DATA initial = {};
            initial.pSysMem = rgba; initial.SysMemPitch = (UINT)width * 4;
            ID3D11Texture2D *texture = nullptr;
            if (FAILED(device->CreateTexture2D(&image, &initial, &texture))) return result;
            ID3D11ShaderResourceView *view = nullptr;
            HRESULT status = device->CreateShaderResourceView(texture, nullptr, &view);
            texture->Release();
            if (FAILED(status)) return result;
            ++image_textures;
            result.keepAlive = std::shared_ptr<void>(view, [](void *pointer) {
                static_cast<ID3D11ShaderResourceView *>(pointer)->Release();
                --image_textures;
            });
            result.ref = ImTextureRef((ImTextureID)(uintptr_t)view);
#else
            if (!window_handle || !rgba || width <= 0 || height <= 0 || width > 4096 || height > 4096) return result;
            pixel_transfer_state transfer(GL_PIXEL_UNPACK_BUFFER);
            GLint previous = 0;
            glGetIntegerv(GL_TEXTURE_BINDING_2D, &previous);
            GLuint texture = 0;
            glGenTextures(1, &texture);
            if (!texture) return result;
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
            GLenum status = glGetError();
            glBindTexture(GL_TEXTURE_2D, (GLuint)previous);
            if (status != GL_NO_ERROR) { glDeleteTextures(1, &texture); return result; }
            ++image_textures;
            result.keepAlive = std::shared_ptr<void>((void *)(uintptr_t)texture, [](void *pointer) {
                GLuint texture = (GLuint)(uintptr_t)pointer;
                glDeleteTextures(1, &texture); --image_textures;
            });
            result.ref = ImTextureRef((ImTextureID)texture);
#endif
            result.size = ImVec2((float)width, (float)height);
            return result;
        };
        RichMd::SetHostServices(services);
        RichMd::SetAssetsFolder(assets_folder);
        RichMd::MarkdownOptions options;
        options.selectableText = true;
        options.callbacks.OnOpenLink = [](const std::string &url) {
            if (on_link) on_link(url.c_str(), link_user);
        };
        markdown_context = RichMd::CreateContext(options);
        error_text[0] = 0;
        return 1;
    } catch (...) { exception_error(); crmd_shutdown(); return 0; }
}
intptr_t crmd_message(void *window, unsigned int message, uintptr_t wparam, intptr_t lparam) {
#if defined(_WIN32)
    try { return ImGui_ImplWin32_WndProcHandler((HWND)window, message, (WPARAM)wparam, (LPARAM)lparam); }
    catch (...) { exception_error(); return 0; }
#else
    (void)window; (void)message; (void)wparam; (void)lparam;
    return 0;
#endif
}
int crmd_begin_frame(void) {
    if (!renderer_ready || render_failed) return fail("Adapter is not initialized or render frame failed; shutdown required");
    try {
#if defined(_WIN32)
        RECT rect;
        if (!GetClientRect(window_handle, &rect)) return fail("Cannot read client size");
        D3D11_TEXTURE2D_DESC description;
        ID3D11Texture2D *buffer = nullptr;
        if (FAILED(swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&buffer)))
            return fail("Cannot read back buffer size");
        buffer->GetDesc(&description); buffer->Release();
        UINT width = (UINT)(rect.right - rect.left), height = (UINT)(rect.bottom - rect.top);
        if (width && height && (width != description.Width || height != description.Height)) {
            commands->OMSetRenderTargets(0, nullptr, nullptr);
            if (target) { target->Release(); target = nullptr; }
            if (FAILED(swap_chain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0)))
                return fail("Cannot resize back buffer");
            if (!make_target()) return 0;
        }
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
#else
        glfwMakeContextCurrent(window_handle);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
#endif
        ImGui::NewFrame();
        return 1;
    } catch (...) { return exception_error(); }
}
int crmd_render(const char *markdown, size_t length) {
    if (!markdown_context || !markdown || length > 4 * 1024 * 1024)
        return fail("Expected initialized context and Markdown up to 4 MiB");
    have_raw_input = false;
    try { RichMd::Render(std::string(markdown, length)); return 1; }
    catch (...) { render_failed = true; return exception_error(); }
}
int crmd_render_raw(const char *document_key, uint64_t revision, const char *markdown, size_t length, crmd_source_selection *selection) {
    bool render_complete = false;
    if (selection) *selection = {};
    raw_interaction = 0;
    if (!markdown_context || !document_key || !*document_key || std::strlen(document_key) > 16384 || !markdown ||
        length > 4 * 1024 * 1024 || !selection || std::memchr(markdown, 0, length))
        return fail("Expected document identity, initialized context, NUL-free Markdown up to 4 MiB and selection output");
    try {
        const std::string input(markdown, length);
        if (raw_selectable || !have_raw_input || raw_document != document_key || raw_revision != revision || raw_source != input) {
            raw_selectable = false;
            RichMd::ClearSourceSelection(); raw_document = document_key; raw_revision = revision; raw_source = input; have_raw_input = true;
        }
        render_named(document_key, input, false); render_complete = true;
        const auto result = RichMd::LastSourceSelection(input);
        selection->status = static_cast<int>(result.status); selection->begin = result.begin; selection->end = result.end; raw_interaction = result.interaction;
        return 1;
    } catch (...) { if (!render_complete) render_failed = true; have_raw_input = false; return exception_error(); }
}
uint64_t crmd_selection_interaction(void) { return raw_interaction; }
int crmd_render_failed(void) { return render_failed ? 1 : 0; }
int crmd_check_render_failure(void) {
    if (!markdown_context || !GImGui || !GImGui->WithinFrameScope || !GImGui->CurrentWindow) return fail("Expected active test render frame");
    try { render_named("failure-probe", "", true, true); }
    catch (...) { return exception_error(); }
    return fail("Render failure diagnostic did not throw");
}
int crmd_render_preview(const char *document_key, uint64_t revision, const char *markdown, size_t length, crmd_preview_selection *selection, size_t selection_size) {
    return crmd_render_preview_find(document_key, revision, markdown, length, selection, selection_size, nullptr, 0, 0, nullptr);
}
int crmd_render_preview_find(const char *document_key, uint64_t revision, const char *markdown, size_t length, crmd_preview_selection *selection, size_t selection_size,
    const char *query, int match_case, int action, crmd_find_result *find) {
    if (find) *find = {};
    bool render_complete = false;
    if (!selection || selection_size != sizeof(*selection)) return fail("Unexpected preview-selection layout");
    *selection = {}; preview_snapshot = {}; raw_interaction = 0;
    if (!markdown_context || !document_key || !*document_key || std::strlen(document_key) > 16384 || !markdown ||
        length > 4 * 1024 * 1024 || std::memchr(markdown, 0, length)) return fail("Expected bounded named UTF-8 preview input");
    try {
        const std::string input(markdown, length);
        if (!raw_selectable || !have_raw_input || raw_document != document_key || raw_revision != revision || raw_source != input) {
            RichMd::ClearSourceSelection(); raw_document = document_key; raw_revision = revision; raw_source = input; have_raw_input = raw_selectable = true;
        }
        render_named(document_key, input, true); render_complete = true;
        if (query && find && !RichMd::FindPreview(query, match_case != 0, action, find->count, find->current)) {
            find->error = 1; fail("Preview find capacity exceeded or query invalid");
        }
        raw_interaction = selection->interaction = RichMd::LastSelectionInteraction();
        preview_snapshot = RichMd::LastPreviewSelection(input);
        selection->has_source_span = preview_snapshot.has_source_span;
        selection->begin = preview_snapshot.begin; selection->end = preview_snapshot.end;
        selection->text = preview_snapshot.text.c_str(); selection->context = preview_snapshot.context.c_str();
        selection->text_length = preview_snapshot.text.size(); selection->context_length = preview_snapshot.context.size();
        return 1;
    } catch (...) { if (!render_complete) render_failed = true; preview_snapshot = {}; return exception_error(); }
}
int crmd_get_text_view(uint32_t widget_id, crmd_text_view *view, size_t view_size) {
    if (!view || view_size != sizeof(*view)) return fail("Unexpected C99 text-view layout");
    *view = {}; view->caret = view->selection_start = view->selection_end = -1;
    if (!ImGui::GetCurrentContext() || !GImGui->WithinFrameScope || !GImGui->CurrentWindow) return 0;
    try {
        for (ImGuiWindow *child : GImGui->CurrentWindow->DC.ChildWindows) {
            if (child->ChildId != widget_id || child->SkipItems) continue;
            view->window = child; view->draw_list = child->DrawList;
            view->text_y = child->DC.CursorStartPos.y + ImGui::GetStyle().FramePadding.y;
            view->line_height = ImGui::GetFontSize(); view->scroll_y = child->Scroll.y;
            /* EndChild popped transient ClipRect; InnerClipRect retains widget bounds. */
            view->clip_min_x = child->InnerClipRect.Min.x; view->clip_min_y = child->InnerClipRect.Min.y;
            view->clip_max_x = child->InnerClipRect.Max.x; view->clip_max_y = child->InnerClipRect.Max.y;
            view->active_id = GImGui->ActiveId;
            if (ImGuiInputTextState *state = ImGui::GetInputTextState(widget_id)) {
                view->caret = state->GetCursorPos(); view->selection_start = state->GetSelectionStart(); view->selection_end = state->GetSelectionEnd();
            }
            return 1;
        }
        return 0;
    } catch (...) { return exception_error(); }
}
int crmd_text_position(uint32_t widget_id, const char *text, size_t length, size_t offset, int wrapped, float *x, float *y) {
    crmd_text_view view;
    if (!text || !x || !y || offset > length || (offset < length && ((unsigned char)text[offset] & 0xc0) == 0x80) ||
        !crmd_get_text_view(widget_id, &view, sizeof(view))) return 0;
    try {
        ImGuiWindow *window = (ImGuiWindow *)view.window;
        const ImGuiStyle& style = ImGui::GetStyle();
        float width = wrapped ? ImMax(1.0f, window->WorkRect.Max.x - window->DC.CursorStartPos.x - style.FramePadding.x +
            (window->ScrollbarY ? 0.0f : -style.ScrollbarSize)) : 0.0f;
        if (ImGuiInputTextState *state = ImGui::GetInputTextState(widget_id))
            if (wrapped && state->WrapWidth > 0) width = state->WrapWidth;
        const char *at = text, *end = text + length, *target = text + offset;
        unsigned row = 0;
        while (at < end) {
            const char *stop = width > 0 ? ImFontCalcWordWrapPositionEx(ImGui::GetFont(), ImGui::GetFontSize(), at, end, width, ImDrawTextFlags_WrapKeepBlanks) : (const char *)memchr(at, '\n', (size_t)(end - at));
            if (!stop) stop = end;
            if (stop == at && *at != '\n') { unsigned int character; int size = ImTextCharFromUtf8(&character, at, end); if (size <= 0) return 0; stop += size; }
            const char *next = stop < end && *stop == '\n' ? stop + 1 : stop;
            if (target < next || next == end) {
                *x = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, 0, at, target).x;
                *y = row * view.line_height;
                if (target == next && stop < end && *stop == '\n') { *x = 0; *y += view.line_height; }
                return 1;
            }
            at = next; ++row;
        }
        *x = 0; *y = row * view.line_height; return 1;
    } catch (...) { return exception_error(); }
}
void *crmd_input_state_create(void) {
    if (!GImGui) return nullptr;
    auto *state = IM_NEW(ImGuiInputTextState); state->Ctx = GImGui; return state;
}
void crmd_input_state_destroy(void *opaque) { IM_DELETE((ImGuiInputTextState *)opaque); }
void crmd_input_state_exchange(void *opaque) {
    if (!opaque || !GImGui) return;
    static_assert(IMGUI_VERSION_NUM == 19300, "Review input-state ownership on ImGui upgrade");
    auto& a = GImGui->InputTextState; auto& b = *(ImGuiInputTextState *)opaque;
    /* Swap each owner, not an implicit copy of the owning Stb pointer. Pinned
       internal layout; review this list when upgrading ImGui. */
    using std::swap;
    swap(a.Ctx, b.Ctx); swap(a.Stb, b.Stb); swap(a.Flags, b.Flags); swap(a.ID, b.ID);
    swap(a.TextLen, b.TextLen); swap(a.TextSrc, b.TextSrc);
    a.TextA.swap(b.TextA); a.TextToRevertTo.swap(b.TextToRevertTo); a.CallbackTextBackup.swap(b.CallbackTextBackup);
    swap(a.BufCapacity, b.BufCapacity); swap(a.Scroll, b.Scroll); swap(a.LineCount, b.LineCount); swap(a.WrapWidth, b.WrapWidth);
    swap(a.CursorAnim, b.CursorAnim); swap(a.CursorFollow, b.CursorFollow); swap(a.CursorCenterY, b.CursorCenterY);
    swap(a.SelectedAllMouseLock, b.SelectedAllMouseLock); swap(a.EditedBefore, b.EditedBefore); swap(a.EditedThisFrame, b.EditedThisFrame);
    swap(a.ValidatedThisFrame, b.ValidatedThisFrame); swap(a.WantReloadUserBuf, b.WantReloadUserBuf);
    swap(a.LastMoveDirectionLR, b.LastMoveDirectionLR); swap(a.ReloadSelectionStart, b.ReloadSelectionStart); swap(a.ReloadSelectionEnd, b.ReloadSelectionEnd);
}
int crmd_source_select(uint32_t widget, const char *text, size_t length, size_t begin, size_t end) {
    if (!GImGui || !text || length > 4 * 1024 * 1024 || begin >= end || end > length ||
        ((unsigned char)text[begin] & 0xc0) == 0x80 || (end < length && ((unsigned char)text[end] & 0xc0) == 0x80)) return 0;
    auto& state = GImGui->InputTextState;
    if (state.ID != widget) {
        state.ID = widget; state.TextLen = (int)length; state.WantReloadUserBuf = false;
        state.Flags = ImGuiInputTextFlags_None;
        state.TextA.resize((int)length + 1); memcpy(state.TextA.Data, text, length + 1); state.TextSrc = state.TextA.Data;
        state.Stb->single_line = 1; /* Next multiline draw initializes its undo state without taking focus. */
        return 0;
    }
    if (state.Stb->single_line) return 0;
    state.SetSelection((int)begin, (int)end); state.CursorFollow = true; return 1;
}
int crmd_end_frame(void) {
    if (render_failed) return 0;
    if (!renderer_ready || !window_handle) return fail("Adapter has no render target");
#if defined(_WIN32)
    if (!target) return fail("Adapter has no render target");
#endif
    try {
        ImGui::Render();
#if defined(_WIN32)
        const float clear[4] = {0.08f, 0.09f, 0.12f, 1.0f};
        commands->OMSetRenderTargets(1, &target, nullptr);
        commands->ClearRenderTargetView(target, clear);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#else
        int width = 0, height = 0;
        glfwGetFramebufferSize(window_handle, &width, &height);
        glViewport(0, 0, width, height);
        glClearColor(0.08f, 0.09f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif
        vertices = ImGui::GetDrawData()->TotalVtxCount;
        return 1;
    } catch (...) { return exception_error(); }
}
int crmd_present(void) {
    if (render_failed) return 0;
#if defined(_WIN32)
    return swap_chain && SUCCEEDED(swap_chain->Present(1, 0)) ? 1 : fail("Cannot present frame");
#else
    if (!window_handle) return fail("Cannot present frame");
    glfwSwapBuffers(window_handle);
    return 1;
#endif
}
int crmd_vertex_count(void) { return vertices; }
int crmd_live_images(void) { return image_textures; }
int crmd_screenshot(const char *path) {
    if (render_failed) return 0;
#if defined(_WIN32)
    if (!swap_chain || !path) return fail("Expected initialized adapter and screenshot path");
    ID3D11Texture2D *source = nullptr, *staging = nullptr;
    if (FAILED(swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&source)))
        return fail("Cannot get screenshot buffer");
    D3D11_TEXTURE2D_DESC description;
    source->GetDesc(&description);
    description.BindFlags = 0; description.MiscFlags = 0;
    description.Usage = D3D11_USAGE_STAGING; description.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    HRESULT result = device->CreateTexture2D(&description, nullptr, &staging);
    if (FAILED(result)) { source->Release(); return fail("Cannot create screenshot staging buffer"); }
    commands->CopyResource(staging, source); source->Release();
    D3D11_MAPPED_SUBRESOURCE mapped;
    result = commands->Map(staging, 0, D3D11_MAP_READ, 0, &mapped);
    if (FAILED(result)) { staging->Release(); return fail("Cannot map screenshot buffer"); }
    FILE *file = std::fopen(path, "wb");
    bool ok = file && std::fprintf(file, "P6\n%u %u\n255\n", description.Width, description.Height) > 0;
    for (UINT y = 0; ok && y < description.Height; ++y) {
        const unsigned char *row = (const unsigned char *)mapped.pData + y * mapped.RowPitch;
        for (UINT x = 0; ok && x < description.Width; ++x) ok = std::fwrite(row + x * 4, 1, 3, file) == 3;
    }
    if (file && std::fclose(file)) ok = false;
    commands->Unmap(staging, 0); staging->Release();
    return ok ? 1 : fail("Cannot write screenshot");
#else
    if (!window_handle || !path) return fail("Expected initialized adapter and screenshot path");
    try {
        int width = 0, height = 0;
        glfwGetFramebufferSize(window_handle, &width, &height);
        if (width <= 0 || height <= 0 || width > 16384 || height > 16384) return fail("Screenshot dimensions exceed bounds");
        std::vector<unsigned char> pixels((size_t)width * (size_t)height * 3);
        pixel_transfer_state transfer(GL_PIXEL_PACK_BUFFER);
        GLint previous_buffer = 0;
        glGetIntegerv(GL_READ_BUFFER, &previous_buffer);
        glReadBuffer(GL_BACK);
        glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
        GLenum status = glGetError();
        glReadBuffer((GLenum)previous_buffer);
        if (status != GL_NO_ERROR) return fail("Cannot read screenshot buffer");
        FILE *file = std::fopen(path, "wb");
        bool ok = file && std::fprintf(file, "P6\n%d %d\n255\n", width, height) > 0;
        for (int y = height - 1; ok && y >= 0; --y)
            ok = std::fwrite(pixels.data() + (size_t)y * (size_t)width * 3, 3, (size_t)width, file) == (size_t)width;
        if (file && std::fclose(file)) ok = false;
        return ok ? 1 : fail("Cannot write screenshot");
    } catch (...) { return exception_error(); }
#endif
}
