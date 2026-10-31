#ifndef GD_VIEWER_H
#define GD_VIEWER_H
#include "gd.h"

#define GD_VIEWER_REQUEST_LIMIT 8192
#define GD_VIEWER_REPLY_LIMIT (1024 * 1024)
#define GD_VIEWER_INSTANCES 32
#define GD_VIEWER_SESSION 64
#define GD_VIEWER_NODE_PAGE 64
/* Local control v1: no source writes, execution, arbitrary paths or auto-start. */
typedef struct {
    char command[16], target[GD_VIEWER_SESSION], projection[32], reference[260], fields[128];
    char id[GD_MAX_PATH];
    unsigned offset,limit;
} GD_ViewerRequest;
typedef struct { char *text; size_t size, capacity; int failed; } GD_ViewerWriter;
void gd_viewer_write(GD_ViewerWriter *writer, const char *format, ...);
void gd_viewer_quote(GD_ViewerWriter *writer, const char *text);
void gd_viewer_error(GD_ViewerWriter *writer, const char *code, const char *message);
int gd_viewer_parse(const char *text, size_t size, GD_ViewerRequest *request);
int gd_viewer_field(const char *fields, const char *name);

typedef struct GD_ViewerServer GD_ViewerServer;
/* Handler runs only on owner poll, never on a transport callback/thread.
   Request is valid for this call. Writer storage belongs to server until reply settles. */
typedef void (*GD_ViewerHandler)(void *context, const GD_ViewerRequest *request, GD_ViewerWriter *reply);
GD_ViewerServer *gd_viewer_server_open(GD_Allocator allocator, char *error, size_t capacity);
const char *gd_viewer_session(const GD_ViewerServer *server);
void gd_viewer_server_poll(GD_ViewerServer *server, GD_ViewerHandler handler, void *context);
const char *gd_viewer_server_error(const GD_ViewerServer *server);
void gd_viewer_server_close(GD_ViewerServer *server);
int gd_viewer_command(int argc, char **argv, GD_Allocator allocator);
#endif
