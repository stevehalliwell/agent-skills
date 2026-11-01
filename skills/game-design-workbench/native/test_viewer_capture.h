#ifndef TEST_VIEWER_CAPTURE_H
#define TEST_VIEWER_CAPTURE_H
#include "gd_viewer_capture.h"
typedef struct {size_t count;int fail,calls;
#ifdef _WIN32
    DWORD owner;
#endif
} CaptureInput;
static int capture_emit(FILE *out,void *context) {
    CaptureInput *input=context;unsigned char block[8192];size_t i,position=0;input->calls++;
#ifdef _WIN32
    if(GetCurrentThreadId()!=input->owner)return 0;
#endif
    for(i=0;i<sizeof(block);++i)block[i]=(unsigned char)i;
    while(position<input->count){size_t count=input->count-position;if(count>sizeof(block))count=sizeof(block);if(fwrite(block,1,count,out)!=count)return 0;position+=count;}
    return !input->fail && !ferror(out);
}
static int capture_checks(void) {
    char bytes[65536],message[256];GD_ViewerCaptureResult result;CaptureInput input={200000,0,0};int checks=0;
#define CAPTURE_CHECK(value) do {++checks;if(!(value)){fprintf(stderr,"Capture check failed: %s (%d)\n",#value,__LINE__);return 0;}}while(0)
#ifdef _WIN32
    DWORD before,after;size_t i;input.owner=GetCurrentThreadId();
    CAPTURE_CHECK(gd_viewer_capture(capture_emit,&input,0,bytes,64,&result,message,sizeof(message)));
    CAPTURE_CHECK(GetProcessHandleCount(GetCurrentProcess(),&before));
    CAPTURE_CHECK(gd_viewer_capture(capture_emit,&input,8000,bytes,sizeof(bytes),&result,message,sizeof(message)) && result.captured==sizeof(bytes) && result.total==input.count);
    for(i=0;i<result.captured;++i)if((unsigned char)bytes[i]!=(unsigned char)(8000+i))return 0;++checks;
    CAPTURE_CHECK(gd_viewer_capture(capture_emit,&input,199999,bytes,64,&result,message,sizeof(message)) && result.captured==1 && (unsigned char)bytes[0]==(unsigned char)199999);
    CAPTURE_CHECK(gd_viewer_capture(capture_emit,&input,200001,bytes,64,&result,message,sizeof(message)) && !result.captured && result.total==input.count);
    CAPTURE_CHECK(gd_viewer_capture(capture_emit,&input,0,NULL,0,&result,message,sizeof(message)) && !result.captured && result.total==input.count);
    input.count=0;CAPTURE_CHECK(gd_viewer_capture(capture_emit,&input,0,bytes,64,&result,message,sizeof(message)) && !result.total);
    input.fail=1;CAPTURE_CHECK(!gd_viewer_capture(capture_emit,&input,0,bytes,64,&result,message,sizeof(message)) && !result.total && *message);input.fail=0;
    input.count=GD_VIEWER_CAPTURE_TOTAL_LIMIT;CAPTURE_CHECK(gd_viewer_capture(capture_emit,&input,0,bytes,64,&result,message,sizeof(message)) && result.total==input.count);
    ++input.count;CAPTURE_CHECK(!gd_viewer_capture(capture_emit,&input,0,bytes,64,&result,message,sizeof(message)) && !result.total && !result.captured);
    CAPTURE_CHECK(!gd_viewer_capture(capture_emit,&input,0,NULL,1,&result,message,sizeof(message)));
    CAPTURE_CHECK(!gd_viewer_capture(capture_emit,&input,0,bytes,sizeof(bytes)+1,&result,message,sizeof(message)));
    CAPTURE_CHECK(!gd_viewer_capture(capture_emit,&input,GD_VIEWER_CAPTURE_TOTAL_LIMIT+1,bytes,0,&result,message,sizeof(message)));
    input.count=777;for(i=0;i<100;++i)CAPTURE_CHECK(gd_viewer_capture(capture_emit,&input,3,bytes,17,&result,message,sizeof(message)) && result.captured==17 && result.total==input.count);
    CAPTURE_CHECK(GetProcessHandleCount(GetCurrentProcess(),&after) && before==after);
#else
    CAPTURE_CHECK(!gd_viewer_capture(capture_emit,&input,0,bytes,sizeof(bytes),&result,message,sizeof(message)) && !input.calls && !result.total && *message);
#endif
#undef CAPTURE_CHECK
    return checks;
}
#endif
