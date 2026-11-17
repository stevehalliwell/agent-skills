/* Native graphics dependency only. Application and window owner remain C99.
   Uses upstream ImGui Win32/D3D11 backends, following Attendant's host split. */
#define GD_NATIVE_UI_BUILD
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "gd_native_ui.h"
#if !defined(_WIN32)
#error Native macOS GLFW/OpenGL3 host is not implemented yet
#endif
#include <windows.h>
#include <d3d11.h>
#include <cmath>
#include "imgui.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx11.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

static ID3D11Device *device;
static ID3D11DeviceContext *commands;
static IDXGISwapChain *swap_chain;
static ID3D11RenderTargetView *target;
static HWND window_handle;
static ImGuiContext *owner_context;
static DWORD owner_thread;
static unsigned buffer_width, buffer_height;
static bool platform_ready, renderer_ready, frame_active, frame_failed;
static const char *error_text = "";

static int fail(const char *message) { error_text = message; return 0; }
static bool owner(void) {
    return owner_context && owner_context == ImGui::GetCurrentContext() && owner_thread == GetCurrentThreadId();
}
static int make_target(void) {
    ID3D11Texture2D *buffer = nullptr;
    HRESULT result = swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&buffer));
    if (FAILED(result)) return fail("Cannot get D3D11 back buffer");
    D3D11_TEXTURE2D_DESC description;
    buffer->GetDesc(&description);
    result = device->CreateRenderTargetView(buffer, nullptr, &target);
    buffer->Release();
    if (FAILED(result)) return fail("Cannot create D3D11 render target");
    buffer_width = description.Width; buffer_height = description.Height;
    return 1;
}
const char *gd_native_ui_backend(void) { return "Win32/D3D11"; }
const char *gd_native_ui_error(void) { return error_text; }
int gd_native_ui_check_layout(const size_t *layout, size_t count) {
    const size_t expected[] = {sizeof(ImGuiStoragePair), offsetof(ImGuiStoragePair, val_i),
        sizeof(ImGuiStyleMod), offsetof(ImGuiStyleMod, BackupInt),
        sizeof(ImGuiInputEvent), offsetof(ImGuiInputEvent, MousePos)};
    if (!layout || count != sizeof(expected)/sizeof(expected[0])) return fail("Invalid C99 layout probe");
    for (size_t i = 0; i < count; ++i) if (layout[i] != expected[i]) return fail("C99 union layout mismatch");
    return 1;
}
int gd_native_ui_shutdown(void) {
    if (owner_context && !owner()) return fail("Native GUI shutdown requires owning thread/context");
    if (renderer_ready) ImGui_ImplDX11_Shutdown();
    if (platform_ready) ImGui_ImplWin32_Shutdown();
    if (commands) commands->OMSetRenderTargets(0, nullptr, nullptr);
    if (target) target->Release();
    if (swap_chain) swap_chain->Release();
    if (commands) commands->Release();
    if (device) device->Release();
    target = nullptr; swap_chain = nullptr; commands = nullptr; device = nullptr;
    window_handle = nullptr; owner_context = nullptr; owner_thread = 0;
    buffer_width = buffer_height = 0;
    platform_ready = renderer_ready = frame_active = frame_failed = false;
    return 1;
}
int gd_native_ui_init(void *window) {
    if (owner_context || !window || !IsWindow(static_cast<HWND>(window)) || !ImGui::GetCurrentContext())
        return fail("Expected valid HWND, current ImGui context and no active native adapter");
    window_handle = static_cast<HWND>(window);
    owner_context = ImGui::GetCurrentContext(); owner_thread = GetCurrentThreadId();
    DXGI_SWAP_CHAIN_DESC description = {};
    description.BufferCount = 2;
    description.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    description.OutputWindow = window_handle;
    description.SampleDesc.Count = 1;
    description.Windowed = TRUE;
    description.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    HRESULT result = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0,
        D3D11_SDK_VERSION, &description, &swap_chain, &device, nullptr, &commands);
    if (FAILED(result)) {
        /* Release any partial outputs before hardware-to-software fallback. */
        if (swap_chain) { swap_chain->Release(); swap_chain = nullptr; }
        if (commands) { commands->Release(); commands = nullptr; }
        if (device) { device->Release(); device = nullptr; }
        result = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0, nullptr, 0,
            D3D11_SDK_VERSION, &description, &swap_chain, &device, nullptr, &commands);
    }
    if (FAILED(result)) { fail("Cannot initialize D3D11 hardware or WARP device"); gd_native_ui_shutdown(); return 0; }
    if (!make_target()) { gd_native_ui_shutdown(); return 0; }
    platform_ready = ImGui_ImplWin32_Init(window_handle);
    if (!platform_ready) { fail("Cannot initialize ImGui Win32 backend"); gd_native_ui_shutdown(); return 0; }
    renderer_ready = ImGui_ImplDX11_Init(device, commands);
    if (!renderer_ready) { fail("Cannot initialize ImGui D3D11 backend"); gd_native_ui_shutdown(); return 0; }
    error_text = "";
    return 1;
}
intptr_t gd_native_ui_message(void *window, unsigned message, uintptr_t wparam, intptr_t lparam) {
    if (!platform_ready || !owner() || window != window_handle) return 0;
    return ImGui_ImplWin32_WndProcHandler(window_handle, message, static_cast<WPARAM>(wparam), static_cast<LPARAM>(lparam));
}
int gd_native_ui_begin_frame(void) {
    if (!renderer_ready || !owner() || frame_active || frame_failed) {
        fail("Native frame requires ready adapter, owning context and no active/failed frame"); return -1;
    }
    RECT rect;
    if (!GetClientRect(window_handle, &rect)) { frame_failed = true; fail("Cannot read native client size"); return -1; }
    unsigned width = static_cast<unsigned>(rect.right - rect.left), height = static_cast<unsigned>(rect.bottom - rect.top);
    if (!width || !height || IsIconic(window_handle)) return 0;
    if (width != buffer_width || height != buffer_height) {
        commands->OMSetRenderTargets(0, nullptr, nullptr);
        target->Release(); target = nullptr;
        if (FAILED(swap_chain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0)) || !make_target()) {
            frame_failed = true; fail("Cannot resize native back buffer"); return -1;
        }
    }
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame(); frame_active = true;
    return 1;
}
int gd_native_ui_end_frame(float red, float green, float blue, float alpha) {
    if (!renderer_ready || !owner() || !frame_active || frame_failed)
        return fail("Native rendering requires active frame and owning context");
    if (!std::isfinite(red) || !std::isfinite(green) || !std::isfinite(blue) || !std::isfinite(alpha)) {
        frame_failed = true; return fail("Native clear color must be finite");
    }
    ImGui::Render(); frame_active = false;
    const float clear[4] = {red, green, blue, alpha};
    commands->OMSetRenderTargets(1, &target, nullptr);
    commands->ClearRenderTargetView(target, clear);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    HRESULT result = swap_chain->Present(1, 0);
    if (FAILED(result)) { frame_failed = true; return fail("Native presentation failed; shut down adapter without retrying frame"); }
    return 1;
}
