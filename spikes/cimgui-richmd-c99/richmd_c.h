#ifndef SPIKE_RICHMD_C_H
#define SPIKE_RICHMD_C_H
/* C99 ABI. No C++ types, allocations, or exceptions cross this boundary.
 * Single UI thread/context. UTF-8 strings borrowed for duration of each call.
 * Raw selection export accepts only proven contiguous original-input spans. */
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
typedef enum crmd_selection_status { CRMD_SELECTION_NONE, CRMD_SELECTION_EXACT, CRMD_SELECTION_UNSUPPORTED } crmd_selection_status;
typedef struct crmd_source_selection {
    int status;
    size_t begin, end; /* Raw input UTF-8 byte offsets, end-exclusive; valid only for EXACT. */
} crmd_source_selection;
/* No unindent/transclusions. Full key/revision/input identity invalidates old selection.
 * Snapshot copied while named render input remains alive. Output cleared on failure. */
CRMD_API int crmd_render_raw(const char *document_key, uint64_t revision, const char *markdown, size_t length, crmd_source_selection *selection);
/* Immediately after successful raw render: renderer selection-interaction serial.
 * Includes context menus and child-widget ownership; no text/range proof by itself. */
CRMD_API uint64_t crmd_selection_interaction(void);
/* Borrowed multiline-input geometry, immediate frame-local use only.
 * Caller owns decoration/navigation policy. Query never changes focus/selection. */
typedef struct crmd_text_view {
    void *window, *draw_list;
    float text_y, line_height, clip_min_x, clip_min_y, clip_max_x, clip_max_y, scroll_y;
    uint32_t active_id;
    int caret, selection_start, selection_end; /* -1 when no retained state for widget. */
} crmd_text_view;
CRMD_API int crmd_get_text_view(uint32_t widget_id, crmd_text_view *view, size_t view_size);
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
