#ifndef GD_VIEWER_CAPTURE_H
#define GD_VIEWER_CAPTURE_H
#include <stddef.h>
#include <stdio.h>

#define GD_VIEWER_CAPTURE_PAGE_LIMIT 65536u
#define GD_VIEWER_CAPTURE_TOTAL_LIMIT (64u * 1024u * 1024u)

typedef int (*GD_ViewerCaptureEmit)(FILE *output, void *context);
typedef struct { size_t captured, total; } GD_ViewerCaptureResult;

/* Windows/macOS. Invoke emit synchronously on caller/GUI owner thread.
   Windows reader thread drains bytes without context/model pointers;
   macOS funopen sink captures/counts synchronously without reader thread.
   Capture [offset, offset+capacity) into caller-owned buffer, without NUL.
   capacity may be zero (buffer may then be NULL). Range must fit total limit.
   Offset beyond emitted EOF succeeds with captured=0 and exact total.
   Emit must return nonzero on success, must not close/retain output or its
   descriptor, and must stop on write failure. Output is binary, nonseekable.
   All borrowed arguments remain live until return; result/message/buffer must
   not overlap. No external thread may access buffer during this call.
   Success=1; failure=0 with concise message when message_capacity>0.
   Result is zero on failure; buffer contents then have no meaning. Over-limit
   output fails writes, never returns a partial page. Windows closes reader to
   break writes; macOS sink returns EFBIG.
   No timeout: finite native serializers must return. Windows join waits for
   reader to relinquish storage, including errors; never terminate threads.
   Uses stream resources plus Windows anonymous pipe/thread; no disk I/O. */
int gd_viewer_capture(GD_ViewerCaptureEmit emit, void *context,
    size_t offset, char *buffer, size_t capacity,
    GD_ViewerCaptureResult *result, char *message, size_t message_capacity);
#endif
