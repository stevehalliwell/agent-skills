/* Spike-only Win32/D3D11 host adapter plus narrow RichMd C ABI. */
#define RICHMD_C_BUILD
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "richmd_c.h"
#include <windows.h>
#include <d3d11.h>
#include <cstdio>
#include <cstring>
#include <exception>
#include <string>
#include "imgui.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx11.h"
#include "imgui_rich_md/rich_md.h"
#include "imgui_rich_md/rich_md_host.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

static ID3D11Device *device;
static ID3D11DeviceContext *commands;
static IDXGISwapChain *swap_chain;
static ID3D11RenderTargetView *target;
static HWND window_handle;
static bool win32_ready, renderer_ready;
static RichMd::Context *markdown_context;
static char error_text[1024];
static int vertices, image_textures;
static crmd_link_callback on_link;
static void *link_user;

static int fail(const char *message) {
    std::snprintf(error_text, sizeof(error_text), "%s", message);
    return 0;
}
static int exception_error() {
    try { throw; }
    catch (const std::exception &error) { return fail(error.what()); }
    catch (...) { return fail("Unknown C++ exception"); }
}
static int make_target() {
    ID3D11Texture2D *buffer = nullptr;
    HRESULT result = swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&buffer);
    if (FAILED(result)) return fail("Cannot get D3D11 back buffer");
    result = device->CreateRenderTargetView(buffer, nullptr, &target);
    buffer->Release();
    return SUCCEEDED(result) ? 1 : fail("Cannot create D3D11 render target");
}
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
    try {
        if (markdown_context) RichMd::DestroyContext(markdown_context);
    } catch (...) { exception_error(); ok = 0; }
    markdown_context = nullptr;
    if (renderer_ready) ImGui_ImplDX11_Shutdown();
    if (win32_ready) ImGui_ImplWin32_Shutdown();
    renderer_ready = win32_ready = false;
    if (target) target->Release();
    if (swap_chain) swap_chain->Release();
    if (commands) commands->Release();
    if (device) device->Release();
    target = nullptr; swap_chain = nullptr; commands = nullptr; device = nullptr;
    window_handle = nullptr; on_link = nullptr; link_user = nullptr;
    return ok;
}
int crmd_init(void *window, const char *assets_folder, crmd_link_callback link, void *user) {
    if (!window || !assets_folder || !ImGui::GetCurrentContext() || window_handle)
        return fail("Expected ImGui context, window, assets folder, and no active adapter");
    try {
        window_handle = (HWND)window;
        on_link = link; link_user = user;
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
        win32_ready = ImGui_ImplWin32_Init(window_handle);
        if (!win32_ready) { fail("Cannot initialize Win32 backend"); crmd_shutdown(); return 0; }
        renderer_ready = ImGui_ImplDX11_Init(device, commands);
        if (!renderer_ready) { fail("Cannot initialize DX11 backend"); crmd_shutdown(); return 0; }
        // Own image textures directly. RichMd's default uploader defers CPU-side
        // texture destruction until a later render; final teardown has no later frame.
        RichMd::HostServices services;
        services.UploadRgba = [](const unsigned char *rgba, int width, int height) {
            RichMd::MarkdownTexture result;
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
    try { return ImGui_ImplWin32_WndProcHandler((HWND)window, message, (WPARAM)wparam, (LPARAM)lparam); }
    catch (...) { exception_error(); return 0; }
}
int crmd_begin_frame(void) {
    if (!renderer_ready) return fail("Adapter is not initialized");
    try {
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
        ImGui::NewFrame();
        return 1;
    } catch (...) { return exception_error(); }
}
int crmd_render(const char *markdown, size_t length) {
    if (!markdown_context || !markdown || length > 4 * 1024 * 1024)
        return fail("Expected initialized context and Markdown up to 4 MiB");
    try { RichMd::Render(std::string(markdown, length)); return 1; }
    catch (...) { return exception_error(); }
}
int crmd_end_frame(void) {
    if (!renderer_ready || !target) return fail("Adapter has no render target");
    try {
        ImGui::Render();
        const float clear[4] = {0.08f, 0.09f, 0.12f, 1.0f};
        commands->OMSetRenderTargets(1, &target, nullptr);
        commands->ClearRenderTargetView(target, clear);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        vertices = ImGui::GetDrawData()->TotalVtxCount;
        return 1;
    } catch (...) { return exception_error(); }
}
int crmd_present(void) {
    return swap_chain && SUCCEEDED(swap_chain->Present(1, 0)) ? 1 : fail("Cannot present frame");
}
int crmd_vertex_count(void) { return vertices; }
int crmd_live_images(void) { return image_textures; }
int crmd_screenshot(const char *path) {
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
}
