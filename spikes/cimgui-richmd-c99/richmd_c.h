#ifndef SPIKE_RICHMD_C_H
#define SPIKE_RICHMD_C_H
/* C99 ABI. No C++ types, allocations, or exceptions cross this boundary.
 * Single UI thread/context. UTF-8 strings borrowed for duration of each call.
 * This experimental ABI does not export source selection ranges. */
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#ifdef RICHMD_C_BUILD
#define CRMD_API __declspec(dllexport)
#else
#define CRMD_API __declspec(dllimport)
#endif
typedef void (*crmd_link_callback)(const char *url, void *user);
CRMD_API const char *crmd_version(void);
CRMD_API const char *crmd_error(void);
/* Six entries: sizeof + union offset for ImGuiStoragePair, ImGuiStyleMod,
 * ImGuiInputEvent. Checks patched C99 named unions against original C++ ABI. */
CRMD_API int crmd_check_cimgui_layout(const size_t *layout, size_t count);
/* HWND and Windows message forwarding, kept opaque to C consumers. */
CRMD_API int crmd_init(void *window, const char *assets_folder, crmd_link_callback link, void *user);
CRMD_API intptr_t crmd_message(void *window, unsigned int message, uintptr_t wparam, intptr_t lparam);
CRMD_API int crmd_begin_frame(void);
CRMD_API int crmd_render(const char *markdown, size_t length);
/* Render, then optionally screenshot, then present. */
CRMD_API int crmd_end_frame(void);
CRMD_API int crmd_present(void);
CRMD_API int crmd_screenshot(const char *path);
CRMD_API int crmd_vertex_count(void);
CRMD_API int crmd_live_images(void);
CRMD_API int crmd_shutdown(void);
#ifdef __cplusplus
}
#endif
#endif
