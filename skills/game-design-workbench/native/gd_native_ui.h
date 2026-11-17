#ifndef GD_NATIVE_UI_H
#define GD_NATIVE_UI_H
#include <stddef.h>
#include <stdint.h>
#if defined(_WIN32) && defined(GD_NATIVE_UI_BUILD)
#define GD_NATIVE_UI_API __declspec(dllexport)
#elif defined(_WIN32)
#define GD_NATIVE_UI_API __declspec(dllimport)
#else
#define GD_NATIVE_UI_API
#endif
#ifdef __cplusplus
extern "C" {
#endif
/* Single UI thread/context. Caller owns native window and ImGui context; initialize
   a context before init, destroy it after shutdown. No application/model ownership.
   Window is HWND on Windows. macOS GLFW/OpenGL3 integration remains pending.
   Failures return 0 except begin_frame (-1 error, 0 minimized, 1 ready).
   After a frame failure, shut down and destroy context; never retry that frame.
   No source writes, window creation/focus, or implicit application state changes. */
GD_NATIVE_UI_API const char *gd_native_ui_backend(void);
GD_NATIVE_UI_API const char *gd_native_ui_error(void); /* Borrowed until next error. */
GD_NATIVE_UI_API int gd_native_ui_check_layout(const size_t *layout, size_t count);
GD_NATIVE_UI_API int gd_native_ui_init(void *window);
GD_NATIVE_UI_API intptr_t gd_native_ui_message(void *window, unsigned message, uintptr_t wparam, intptr_t lparam);
GD_NATIVE_UI_API int gd_native_ui_begin_frame(void);
GD_NATIVE_UI_API int gd_native_ui_end_frame(float red, float green, float blue, float alpha);
GD_NATIVE_UI_API int gd_native_ui_shutdown(void);
#ifdef __cplusplus
}
#endif
#endif
