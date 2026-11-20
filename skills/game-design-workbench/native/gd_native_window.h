#ifndef GD_NATIVE_WINDOW_H
#define GD_NATIVE_WINDOW_H
#include <stdint.h>

/* C99 window/context owner. One open host per process, caller-owned storage.
   Open requires no current ImGui context. Default state is all zeroes.
   Hidden mode never shows/activates window; visible mode shows without focus.
   Close requests are retained for caller's draft/exit policy, not acted on here.
   No model, source, query, draft, save or reload ownership. Win32 / macOS GLFW. */
typedef enum { GD_WINDOW_HIDDEN, GD_WINDOW_VISIBLE } GD_WindowVisibility;
typedef struct {
    void *handle, *context;
    uint32_t thread;
    int minimum_width, minimum_height, close_requested, backend_ready, frame_active, failed;
    int64_t start_ticks, ticks_per_second;
    char error[256];
} GD_NativeWindow;
int gd_native_window_open(GD_NativeWindow *window, const char *title, int width, int height,
    int minimum_width, int minimum_height, GD_WindowVisibility visibility);
/* Pump ownership/close messages without opening an ImGui frame. */
int gd_native_window_poll(GD_NativeWindow *window);
void gd_native_window_size(const GD_NativeWindow *window,int *width,int *height);
/* Pumps platform events (Win32: at most 256 messages); 16 ms idle wait.
   1 ready, 0 minimized, -1 error.
   Close request does not prevent drawing a caller-owned exit/draft dialog.
   Failed host still pumps close messages with 16 ms idle-wait timeout, but returns -1
   without GPU retry or ImGui input forwarding. Caller retains model/IPC. */
int gd_native_window_begin(GD_NativeWindow *window);
int gd_native_window_end(GD_NativeWindow *window, float red, float green, float blue, float alpha);
int gd_native_window_close(GD_NativeWindow *window);
double gd_native_window_seconds(const GD_NativeWindow *window);
#endif
