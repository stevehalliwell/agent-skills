/* Private macOS transport for gd_viewer.c. Reuses bounded local message pipes.
   Frame: little-endian u32 byte count, then UTF-8 JSON. No registry or TCP. */
#include "vendor/local-pipe/native.h"
#include <sys/random.h>
#define PIPE_PREFIX "gd-workbench-"
#define PEER_TIMEOUT 10000
struct GD_ViewerServer {
    GD_Allocator allocator;
    pipe_channel channel;
    char session[GD_VIEWER_SESSION],error[256];
    char input[GD_VIEWER_REQUEST_LIMIT],output[GD_VIEWER_REPLY_LIMIT];
    int stage;
    u32 started;
};
static int session_valid(const char *text);
static int endpoint(char *path,size_t capacity,const char *session) {
    int length=snprintf(path,capacity,"/tmp/proto-pipes-%lu/" PIPE_PREFIX "%s",(unsigned long)getuid(),session);
    return length>0 && (size_t)length<capacity;
}
static void reset(GD_ViewerServer *s) {
    usize ignored;
    if (s->channel.pending) { pipe_cancel(&s->channel); pipe_poll(&s->channel,&ignored); }
    if (pipe_server_disconnect(&s->channel)!=pipe_error_ok ||
        pipe_begin(&s->channel,pipe_operation_connect,NULL,0)!=pipe_error_pending)
        snprintf(s->error,sizeof(s->error),"Viewer IPC cannot accept next peer");
    s->stage=0;
}
GD_ViewerServer *gd_viewer_server_open(GD_Allocator a,char *error,size_t capacity) {
    GD_ViewerServer *s=a.allocate(a.context,sizeof(*s));
    unsigned char nonce[8]; char path[104]; size_t i; pipe_error status=pipe_error_io;
    if (!s) { snprintf(error,capacity,"Cannot allocate bounded viewer IPC storage"); return NULL; }
    memset(s,0,sizeof(*s)); s->allocator=a;
    if (getentropy(nonce,sizeof(nonce))) goto fail;
    snprintf(s->session,sizeof(s->session),"%lu-",(unsigned long)getpid());
    for (i=0;i<sizeof(nonce);++i) { size_t n=strlen(s->session); snprintf(s->session+n,sizeof(s->session)-n,"%02x",nonce[i]); }
    if (!endpoint(path,sizeof(path),s->session)) goto fail;
    status=pipe_server_open(path,sizeof(s->input),sizeof(s->output),&s->channel);
    if (status!=pipe_error_ok) goto fail;
    status=pipe_begin(&s->channel,pipe_operation_connect,NULL,0);
    if (status==pipe_error_pending) return s;
fail:
    snprintf(error,capacity,"Viewer IPC unavailable (%s); ordinary viewer remains usable",pipe_error_message(status));
    pipe_close(&s->channel,0); a.release(a.context,s); return NULL;
}
const char *gd_viewer_session(const GD_ViewerServer *s) { return s?s->session:NULL; }
const char *gd_viewer_server_error(const GD_ViewerServer *s) { return s && *s->error?s->error:NULL; }
void gd_viewer_server_close(GD_ViewerServer *s) {
    GD_Allocator a;
    if (!s) return;
    a=s->allocator; pipe_close(&s->channel,0); a.release(a.context,s);
}
void gd_viewer_server_poll(GD_ViewerServer *s,GD_ViewerHandler handler,void *context) {
    pipe_error status; usize count=0; GD_ViewerWriter w; GD_ViewerRequest request;
    if (!s || *s->error) return;
    if (s->stage && system_milliseconds()-s->started>=PEER_TIMEOUT) { reset(s); return; }
    status=pipe_poll(&s->channel,&count);
    if (status==pipe_error_pending) return;
    if (s->stage==3) {
        /* Completed write is not receipt. Retain backing until close/deadline;
           ignore at most one bounded framed input per owner poll. */
        if (status!=pipe_error_ok) reset(s);
        else if (pipe_begin(&s->channel,pipe_operation_read,s->input,sizeof(s->input))!=pipe_error_pending) reset(s);
        return;
    }
    if (status!=pipe_error_ok && !(s->stage==1 && status==pipe_error_capacity)) { reset(s); return; }
    if (!s->stage) {
        s->stage=1; s->started=system_milliseconds();
        if (pipe_begin(&s->channel,pipe_operation_read,s->input,sizeof(s->input))!=pipe_error_pending) reset(s);
        return;
    }
    if (s->stage==1) {
        w=(GD_ViewerWriter){s->output,0,sizeof(s->output),0};
        if (status==pipe_error_capacity) gd_viewer_error(&w,"viewer.capacity","Request exceeds 8192 bytes");
        else if (!gd_viewer_parse(s->input,count,&request)) gd_viewer_error(&w,"viewer.protocol","Invalid v1 viewer request, command or fields");
        else if (*request.target && strcmp(request.target,s->session)) gd_viewer_error(&w,"viewer.target","Session target does not match this viewer");
        else handler(context,&request,&w);
        if (w.failed || !w.size) gd_viewer_error(&w,"viewer.capacity","Context exceeds reply capacity; no partial result returned");
        s->stage=2;
        if (pipe_begin(&s->channel,pipe_operation_write,s->output,w.size)!=pipe_error_pending) reset(s);
        return;
    }
    s->stage=3;
    if (pipe_begin(&s->channel,pipe_operation_read,s->input,sizeof(s->input))!=pipe_error_pending) reset(s);
}
typedef struct { pipe_channel channel; char output[GD_VIEWER_REQUEST_LIMIT],input[GD_VIEWER_REPLY_LIMIT]; } Exchange;
static int exchange_operation(Exchange *e,pipe_operation operation,void *bytes,size_t size,u32 started,unsigned timeout,usize *count) {
    u32 elapsed=system_milliseconds()-started;
    return elapsed<timeout && pipe_begin(&e->channel,operation,bytes,size)==pipe_error_pending &&
        pipe_wait(&e->channel,timeout-elapsed,count)==pipe_error_ok;
}
static Exchange *exchange(GD_Allocator a,const char *session,const char *request,unsigned timeout,char *error,size_t capacity) {
    Exchange *e=a.allocate(a.context,sizeof(*e)); u32 started=system_milliseconds();
    usize count=0; char path[104]; size_t size=strlen(request);
    if (!e) { snprintf(error,capacity,"Cannot allocate exchange storage"); return NULL; }
    memset(&e->channel,0,sizeof(e->channel));
    if (!size || size>=sizeof(e->output) || !endpoint(path,sizeof(path),session)) goto fail;
    memcpy(e->output,request,size);
    if (pipe_client_open(path,timeout,&e->channel)!=pipe_error_ok ||
        !exchange_operation(e,pipe_operation_write,e->output,size,started,timeout,&count) || count!=size ||
        !exchange_operation(e,pipe_operation_read,e->input,sizeof(e->input)-1,started,timeout,&count) ||
        !count || memchr(e->input,0,count) || !gd_valid_utf8((const unsigned char *)e->input,count)) goto fail;
    e->input[count]=0;
    if (pipe_close(&e->channel,0)!=pipe_error_ok) goto fail;
    return e;
fail:
    snprintf(error,capacity,"Viewer exchange failed/timed out; mutation outcome may be unknown. Do not automatically retry mutations.");
    pipe_close(&e->channel,0); a.release(a.context,e); return NULL;
}
static int instances(char sessions[GD_VIEWER_INSTANCES][GD_VIEWER_SESSION],int *count) {
    char directory[104],path[104]; DIR *folder; struct dirent *entry;
    unsigned entries=0; int okay=1;
    *count=0;
    snprintf(directory,sizeof(directory),"/tmp/proto-pipes-%lu",(unsigned long)getuid());
    if (!pipe_macos_directory(directory)) {
        struct stat status;
        return lstat(directory,&status)!=0 && errno==ENOENT;
    }
    folder=opendir(directory); if (!folder) return 0;
    for (;;) {
        struct stat status; const char *session;
        errno=0; entry=readdir(folder);
        if (!entry) { if (errno) okay=0; break; }
        if (++entries>GD_MAX_DIRECTORY_ENTRIES) { okay=0; break; }
        if (strncmp(entry->d_name,PIPE_PREFIX,strlen(PIPE_PREFIX))) continue;
        session=entry->d_name+strlen(PIPE_PREFIX);
        if (!session_valid(session)) continue;
        if (!endpoint(path,sizeof(path),session)) { okay=0; break; }
        if (lstat(path,&status)) { if (errno==ENOENT) continue; okay=0; break; }
        if (!S_ISSOCK(status.st_mode) || status.st_uid!=getuid() || (status.st_mode&0777)!=0600) continue;
        if (*count==GD_VIEWER_INSTANCES) { okay=0; break; }
        snprintf(sessions[(*count)++],GD_VIEWER_SESSION,"%s",session);
    }
    if (closedir(folder)) okay=0;
    return okay;
}
