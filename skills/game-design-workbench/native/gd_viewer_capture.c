#if defined(__APPLE__) && !defined(_DARWIN_C_SOURCE)
#define _DARWIN_C_SOURCE 1
#endif
#include "gd_viewer_capture.h"
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <io.h>
#include <fcntl.h>

typedef struct {
    HANDLE read;
    char *buffer;
    size_t offset, capacity, captured, total;
    int limited;
    DWORD error;
    volatile LONG done;
} GD_ViewerCaptureReader;

static DWORD WINAPI gd_viewer_capture_read(void *argument) {
    GD_ViewerCaptureReader *reader=argument;
    unsigned char bytes[8192];
    for (;;) {
        DWORD count=0;
        size_t begin,end;
        if (!ReadFile(reader->read,bytes,sizeof(bytes),&count,NULL)) {
            DWORD error=GetLastError();
            if (error!=ERROR_BROKEN_PIPE) reader->error=error;
            break;
        }
        if (!count) break;
        if (count>GD_VIEWER_CAPTURE_TOTAL_LIMIT-reader->total) {
            reader->limited=1;
            break;
        }
        begin=reader->offset>reader->total ? reader->offset-reader->total:0;
        end=reader->offset+reader->capacity>reader->total ? reader->offset+reader->capacity-reader->total:0;
        if (end>count) end=count;
        if (begin<end) {
            memcpy(reader->buffer+reader->captured,bytes+begin,end-begin);
            reader->captured+=end-begin;
        }
        reader->total+=count;
    }
    /* Reader exclusively owns read handle after CreateThread succeeds. Closing
       it on ceiling/read failure releases a blocked CRT pipe writer. */
    if (!CloseHandle(reader->read) && !reader->error) reader->error=GetLastError();
    /* Final state access. Wait failure fallback can safely reclaim caller storage
       after observing this flag even if thread return epilogue is still running. */
    InterlockedExchange(&reader->done,1);
    return 0;
}
#endif

#ifdef __APPLE__
#include <errno.h>
/* funopen keeps serialization on owner thread. Retain only requested page;
   count/discard other bytes, fail rather than exceed total output ceiling. */
typedef struct {
    char *buffer;
    size_t offset,capacity,captured,total;
    int limited;
} GD_ViewerCaptureSink;
static int gd_viewer_capture_write(void *context,const char *bytes,int size) {
    GD_ViewerCaptureSink *sink=context;
    size_t begin,end,count;
    if (size<0) { errno=EINVAL; return -1; }
    count=(size_t)size;
    if (sink->limited || count>GD_VIEWER_CAPTURE_TOTAL_LIMIT-sink->total) {
        sink->limited=1; errno=EFBIG; return -1;
    }
    begin=sink->offset>sink->total ? sink->offset-sink->total:0;
    end=sink->offset+sink->capacity>sink->total ? sink->offset+sink->capacity-sink->total:0;
    if (end>count) end=count;
    if (begin<end) {
        memcpy(sink->buffer+sink->captured,bytes+begin,end-begin);
        sink->captured+=end-begin;
    }
    sink->total+=count;
    return size;
}
#endif

int gd_viewer_capture(GD_ViewerCaptureEmit emit, void *context,
    size_t offset, char *buffer, size_t capacity,
    GD_ViewerCaptureResult *result, char *message, size_t message_capacity) {
    const char *failure=NULL;
    if (result) memset(result,0,sizeof(*result));
    if (message && message_capacity) *message=0;
    if (!emit || !result || (!buffer && capacity) || (!message && message_capacity) ||
        offset>GD_VIEWER_CAPTURE_TOTAL_LIMIT || capacity>GD_VIEWER_CAPTURE_PAGE_LIMIT ||
        capacity>GD_VIEWER_CAPTURE_TOTAL_LIMIT-offset) failure="Invalid capture arguments or byte range";
#ifdef _WIN32
    if (!failure) {
        HANDLE read=NULL,write=NULL,thread=NULL;
        FILE *output=NULL;
        int descriptor=-1,emitted=0,stream_error=0,close_error=0,wait_error=0;
        GD_ViewerCaptureReader reader;
        memset(&reader,0,sizeof(reader));
        reader.buffer=buffer; reader.offset=offset; reader.capacity=capacity;
        if (!CreatePipe(&read,&write,NULL,0)) failure="Cannot create capture pipe";
        if (!failure) {
            /* Successful descriptor conversion transfers write handle ownership. */
            descriptor=_open_osfhandle((intptr_t)write,_O_WRONLY|_O_BINARY);
            if (descriptor<0) failure="Cannot open capture pipe descriptor";
            else write=NULL;
        }
        if (!failure) {
            output=_fdopen(descriptor,"wb");
            if (!output) failure="Cannot open capture pipe stream";
            else descriptor=-1;
        }
        if (!failure) {
            reader.read=read;
            thread=CreateThread(NULL,0,gd_viewer_capture_read,&reader,0,NULL);
            if (!thread) failure="Cannot start capture reader";
            else read=NULL;
        }
        if (!failure) {
            emitted=emit(output,context);
            stream_error=ferror(output)!=0;
        }
        /* Always close writer before joining, including callback failure. fclose
           flushes buffered bytes and owns the descriptor/handle even on error. */
        if (output) close_error=fclose(output)!=0;
        if (descriptor>=0 && _close(descriptor)!=0 && !failure) failure="Cannot close capture descriptor";
        if (write && !CloseHandle(write) && !failure) failure="Cannot close capture writer";
        if (read && !CloseHandle(read) && !failure) failure="Cannot close capture reader";
        if (thread) {
            if (WaitForSingleObject(thread,INFINITE)!=WAIT_OBJECT_0) {
                wait_error=1;
                /* Exceptional OS wait failure must not return while reader still
                   uses caller storage. Intentional unbounded join policy, not a
                   retry of serialization. Writer is already closed. */
                while (!InterlockedCompareExchange(&reader.done,0,0)) SwitchToThread();
            }
            if (!CloseHandle(thread) && !failure) failure="Cannot close capture thread";
            if (!failure) {
                if (reader.limited) failure="Capture exceeds 64 MiB output ceiling";
                else if (reader.error) failure="Capture pipe read/close failed";
                else if (wait_error) failure="Capture reader wait failed";
                else if (!emitted) failure="Capture serializer failed";
                else if (stream_error || close_error) failure="Capture stream write/close failed";
                else { result->captured=reader.captured; result->total=reader.total; }
            }
        }
    }
#elif defined(__APPLE__)
    if (!failure) {
        GD_ViewerCaptureSink sink={buffer,offset,capacity,0,0,0};
        FILE *output=funopen(&sink,NULL,gd_viewer_capture_write,NULL,NULL);
        if (!output) failure="Cannot open capture stream";
        else {
            int emitted=emit(output,context),stream_error=ferror(output)!=0;
            int close_error=fclose(output)!=0;
            if (sink.limited) failure="Capture exceeds 64 MiB output ceiling";
            else if (!emitted) failure="Capture serializer failed";
            else if (stream_error || close_error) failure="Capture stream write/close failed";
            else { result->captured=sink.captured; result->total=sink.total; }
        }
    }
#else
    (void)context;
    if (!failure) failure="Viewer serializer capture unsupported on this platform";
#endif
    if (failure && message && message_capacity) snprintf(message,message_capacity,"%s",failure);
    return failure==NULL;
}
