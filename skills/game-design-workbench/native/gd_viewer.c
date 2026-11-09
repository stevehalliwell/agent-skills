/* Local viewer control. Platform I/O owns buffers through cancellation completion. */
#if defined(__APPLE__) && !defined(_DARWIN_C_SOURCE)
#define _DARWIN_C_SOURCE 1
#endif
#include "gd_viewer.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void gd_viewer_write(GD_ViewerWriter *w, const char *format, ...) {
    va_list arguments; int count;
    if (w->failed) return;
    va_start(arguments,format);
    count=vsnprintf(w->text+w->size,w->capacity-w->size,format,arguments);
    va_end(arguments);
    if (count<0 || (size_t)count>=w->capacity-w->size) { w->failed=1; return; }
    w->size+=(size_t)count;
}
void gd_viewer_quote(GD_ViewerWriter *w, const char *text) {
    const unsigned char *p=(const unsigned char *)text;
    if (!text) { gd_viewer_write(w,"null"); return; }
    gd_viewer_write(w,"\"");
    for (;*p && !w->failed;++p) {
        if (*p=='"' || *p=='\\') gd_viewer_write(w,"\\%c",*p);
        else if (*p<32) gd_viewer_write(w,"\\u%04x",*p);
        else gd_viewer_write(w,"%c",*p);
    }
    gd_viewer_write(w,"\"");
}
void gd_viewer_error(GD_ViewerWriter *w, const char *code, const char *message) {
    w->size=0; w->failed=0;
    gd_viewer_write(w,"{\"ok\":false,\"code\":"); gd_viewer_quote(w,code);
    gd_viewer_write(w,",\"message\":"); gd_viewer_quote(w,message); gd_viewer_write(w,"}");
}
/* Flat protocol only. JSON escapes decode before duplicate/field/value checks. */
typedef struct { const char *p,*end; } Reader;
static void spaces(Reader *r) { while (r->p<r->end && (*r->p==' ' || *r->p=='\r' || *r->p=='\n' || *r->p=='\t')) ++r->p; }
static int take(Reader *r, char c) { spaces(r); if (r->p==r->end || *r->p!=c) return 0; ++r->p; return 1; }
static int hex4(Reader *r, unsigned *value) {
    int i; *value=0;
    for (i=0;i<4;++i) {
        unsigned c,d;
        if (r->p==r->end) return 0;
        c=(unsigned char)*r->p++; d=c>='0'&&c<='9'?c-'0':c>='a'&&c<='f'?c-'a'+10:c>='A'&&c<='F'?c-'A'+10:16;
        if (d==16) return 0;
        *value=(*value<<4)|d;
    }
    return 1;
}
static int string(Reader *r, char *output, size_t capacity, int multiline) {
    size_t length=0;
    if (!take(r,'"')) return 0;
    while (r->p<r->end) {
        unsigned c=(unsigned char)*r->p++; unsigned char bytes[4]; size_t n=1,i; int unicode=0;
        if (c=='"') { output[length]=0; return gd_valid_utf8((const unsigned char *)output,length); }
        if (c<32) return 0;
        if (c=='\\') {
            if (r->p==r->end) return 0;
            c=(unsigned char)*r->p++;
            if (c=='b') c=8; else if (c=='f') c=12; else if (c=='n') c=10; else if (c=='r') c=13; else if (c=='t') c=9;
            else if (c=='u') {
                unsigned low; unicode=1;
                if (!hex4(r,&c)) return 0;
                if (c>=0xd800 && c<=0xdbff) {
                    if (r->end-r->p<2 || r->p[0]!='\\' || r->p[1]!='u') return 0;
                    r->p+=2;
                    if (!hex4(r,&low) || low<0xdc00 || low>0xdfff) return 0;
                    c=0x10000+((c-0xd800)<<10)+(low-0xdc00);
                } else if (c>=0xdc00 && c<=0xdfff) return 0;
            } else if (c!='"' && c!='\\' && c!='/') return 0;
        }
        /* Only literal control values accept escaped LF/CR/TAB. NUL and other
           control bytes remain forbidden; raw JSON control bytes always fail. */
        if (c<32 && (!multiline || (c!=10 && c!=13 && c!=9))) return 0;
        if (c<128) bytes[0]=(unsigned char)c;
        else if (c<256 && !unicode) bytes[0]=(unsigned char)c; /* Raw UTF-8 bytes validated on close. */
        else if (c<2048) { bytes[0]=0xc0|(c>>6); bytes[1]=0x80|(c&63); n=2; }
        else if (c<65536) { bytes[0]=0xe0|(c>>12); bytes[1]=0x80|((c>>6)&63); bytes[2]=0x80|(c&63); n=3; }
        else { bytes[0]=0xf0|(c>>18); bytes[1]=0x80|((c>>12)&63); bytes[2]=0x80|((c>>6)&63); bytes[3]=0x80|(c&63); n=4; }
        if (n>=capacity-length) return 0;
        for (i=0;i<n;++i) output[length++]=(char)bytes[i];
    }
    return 0;
}
int gd_viewer_field(const char *fields, const char *name) {
    const char *p=fields;
    if (!fields || !*fields) return 1;
    while (*p) {
        const char *end=strchr(p,','); size_t size=end?(size_t)(end-p):strlen(p);
        if (size==strlen(name) && !memcmp(p,name,size)) return 1;
        if (!end) break;
        p=end+1;
    }
    return 0;
}
static int unsigned_value(Reader *r, unsigned maximum, unsigned *output) {
    unsigned value=0; const char *start;
    spaces(r); start=r->p;
    while (r->p<r->end && *r->p>='0' && *r->p<='9') {
        unsigned digit=(unsigned)(*r->p++-'0');
        if (digit>maximum || value>(maximum-digit)/10) return 0;
        value=value*10+digit;
    }
    if (r->p==start || (r->p-start>1 && *start=='0')) return 0;
    *output=value; return 1;
}
static int fields_valid(const char *fields) {
    const char *p=fields; unsigned seen=0;
    const char *const names[]={"selection","snapshots","panes","agent_hint","source"};
    if (!*p) return 0;
    for (;;) {
        const char *end=strchr(p,','); size_t size=end?(size_t)(end-p):strlen(p); unsigned i;
        for (i=0;i<5;++i) if (size==strlen(names[i]) && !memcmp(p,names[i],size)) break;
        if (i==5 || (seen&(1u<<i))) return 0;
        seen|=1u<<i;
        if (!end) return 1;
        p=end+1;
    }
}
int gd_viewer_parse(const char *text, size_t size, GD_ViewerRequest *request) {
    Reader r; unsigned seen=0;
    if (!text || !request) return 0;
    r=(Reader){text,text+size};
    if (!size || size>GD_VIEWER_REQUEST_LIMIT || memchr(text,0,size) || !gd_valid_utf8((const unsigned char *)text,size)) return 0;
    memset(request,0,sizeof(*request)); request->limit=GD_VIEWER_NODE_PAGE;
    if (!take(&r,'{')) return 0;
    for (;;) {
        char key[32]; unsigned bit; char *target=NULL; size_t capacity=0;
        if (!string(&r,key,sizeof(key),0) || !take(&r,':')) return 0;
        if (!strcmp(key,"protocol")) bit=1;
        else if (!strcmp(key,"command")) { bit=2; target=request->command; capacity=sizeof(request->command); }
        else if (!strcmp(key,"target")) { bit=4; target=request->target; capacity=sizeof(request->target); }
        else if (!strcmp(key,"projection")) { bit=8; target=request->projection; capacity=sizeof(request->projection); }
        else if (!strcmp(key,"reference")) { bit=16; target=request->reference; capacity=sizeof(request->reference); }
        else if (!strcmp(key,"fields")) { bit=32; target=request->fields; capacity=sizeof(request->fields); }
        else if (!strcmp(key,"id")) { bit=64; target=request->id; capacity=sizeof(request->id); }
        else if (!strcmp(key,"offset")) bit=128;
        else if (!strcmp(key,"limit")) bit=256;
        else if (!strcmp(key,"action")) { bit=512; target=request->action; capacity=sizeof(request->action); }
        else if (!strcmp(key,"form")) { bit=1024; target=request->form; capacity=sizeof(request->form); }
        else if (!strcmp(key,"control")) { bit=2048; target=request->control; capacity=sizeof(request->control); }
        else if (!strcmp(key,"value")) { bit=4096; target=request->value; capacity=sizeof(request->value); }
        else if (!strcmp(key,"expect")) { bit=8192; target=request->expect; capacity=sizeof(request->expect); }
        else return 0;
        if (seen&bit) return 0;
        seen|=bit;
        if (bit==1) { spaces(&r); if (r.p==r.end || *r.p++!='1') return 0; }
        else if (bit==128) { if (!unsigned_value(&r,64u*1024u*1024u,&request->offset)) return 0; }
        else if (bit==256) { if (!unsigned_value(&r,GD_VIEWER_NODE_PAGE,&request->limit) || !request->limit) return 0; }
        else if (!string(&r,target,capacity,bit==4096) || (!*target && bit!=4096)) return 0;
        spaces(&r);
        if (take(&r,'}')) break;
        if (!take(&r,',')) return 0;
    }
    spaces(&r);
    if (r.p!=r.end || (seen&3)!=3) return 0;
    if (!strcmp(request->command,"inspect")) return !(seen&~(7u|32u)) && (!(seen&32) || fields_valid(request->fields));
    if (!strcmp(request->command,"nodes")) return request->offset<=GD_MAX_RECORDS && !(seen&~(7u|8u|128u|256u));
    if (!strcmp(request->command,"select")) return (seen&(8|1024|64|8192))==(8|1024|64|8192) && !(seen&~(7u|8u|1024u|64u|8192u|16u));
    if (!strcmp(request->command,"result")) return (seen&(8|1024))==(8|1024) && !(seen&~(7u|8u|1024u|32u|128u|256u|8192u));
    if (!strcmp(request->command,"evidence")) return request->offset<=GD_MAX_EDGES && !(seen&~(7u|8u|1024u|128u|256u|16u));
    if (!strcmp(request->command,"controls")) return request->offset<=GD_VIEWER_CONTROL_OFFSET && !(seen&~(7u|8u|1024u|2048u|128u|256u)) && (!(seen&2048) || (seen&1024)) && (!(seen&(128|256)) || (seen&2048));
    if (!strcmp(request->command,"configure")) return (seen&(8|1024|2048|4096|8192))==(8|1024|2048|4096|8192) && !(seen&~(7u|8u|1024u|2048u|4096u|8192u));
    if (!strcmp(request->command,"actions")) return !(seen&~(7u|8u));
    if (!strcmp(request->command,"action")) return (seen&(8|512))==(8|512) && !(seen&~(7u|8u|512u|8192u));
    if (!strcmp(request->command,"refresh")) return !(seen&~7u);
    if ((seen&(16|64))==(16|64) || (seen&~(7u|8u|16u|64u))) return 0;
    if (!strcmp(request->command,"hint")) return (seen&8) && (seen&(16|64));
    if (!strcmp(request->command,"navigate")) return (seen&8)!=0;
    return 0;
}

#if defined(_WIN32) || defined(__APPLE__)
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
/* Installed SDK sddl.h uses annotations CJIT cannot parse. Keep only these
   documented Win32 ABI declarations; no alternate implementation. */
WINADVAPI BOOL WINAPI ConvertSidToStringSidA(PSID sid, LPSTR *text);
WINADVAPI BOOL WINAPI ConvertStringSecurityDescriptorToSecurityDescriptorA(LPCSTR text, DWORD revision, PSECURITY_DESCRIPTOR *descriptor, PULONG size);
#ifndef PIPE_REJECT_REMOTE_CLIENTS
#define PIPE_REJECT_REMOTE_CLIENTS 8
#endif
#define PIPE_PREFIX "gd-workbench-"
#define PEER_TIMEOUT 10000
/* One overlapped operation/channel. Immediate completion is recorded explicitly. */
typedef struct {
    HANDLE pipe,event; OVERLAPPED operation;
    int pending,ready; DWORD count,error;
} Channel;
struct GD_ViewerServer {
    GD_Allocator allocator; Channel channel;
    char session[GD_VIEWER_SESSION],error[256];
    char input[GD_VIEWER_REQUEST_LIMIT],output[GD_VIEWER_REPLY_LIMIT];
    int stage; DWORD started;
};
static void channel_init(Channel *c) { memset(c,0,sizeof(*c)); c->pipe=INVALID_HANDLE_VALUE; }
static int settled(Channel *c) {
    DWORD count=0;
    if (!c->pending) return 1;
    CancelIo(c->pipe);
    if (WaitForSingleObject(c->event,2000)!=WAIT_OBJECT_0) return 0;
    GetOverlappedResult(c->pipe,&c->operation,&count,FALSE); c->pending=0; return 1;
}
static int channel_close(Channel *c) {
    if (!settled(c)) return 0;
    if (c->pipe!=INVALID_HANDLE_VALUE) CloseHandle(c->pipe);
    if (c->event) CloseHandle(c->event);
    channel_init(c); return 1;
}
static void begin(Channel *c, int operation, char *bytes, DWORD size) {
    BOOL okay;
    memset(&c->operation,0,sizeof(c->operation)); c->operation.hEvent=c->event;
    ResetEvent(c->event); c->ready=0; c->error=0; c->count=0;
    if (!operation) okay=ConnectNamedPipe(c->pipe,&c->operation);
    else if (operation==1) okay=ReadFile(c->pipe,bytes,size,&c->count,&c->operation);
    else okay=WriteFile(c->pipe,bytes,size,&c->count,&c->operation);
    if (okay) { c->ready=1; return; }
    c->error=GetLastError();
    if (!operation && c->error==ERROR_PIPE_CONNECTED) { c->error=0; c->ready=1; }
    else if (c->error==ERROR_IO_PENDING) { c->error=0; c->pending=1; }
    else c->ready=1;
}
static int poll(Channel *c) {
    if (c->pending) {
        if (!GetOverlappedResult(c->pipe,&c->operation,&c->count,FALSE)) {
            c->error=GetLastError(); if (c->error==ERROR_IO_INCOMPLETE) { c->error=0; return 0; }
        }
        c->pending=0; c->ready=1;
    }
    return c->ready;
}
static void reset(GD_ViewerServer *s) {
    if (!settled(&s->channel)) { snprintf(s->error,sizeof(s->error),"IPC cancellation did not settle; backing retained until exit"); return; }
    DisconnectNamedPipe(s->channel.pipe); s->stage=0; s->started=GetTickCount(); begin(&s->channel,0,NULL,0);
}
GD_ViewerServer *gd_viewer_server_open(GD_Allocator a, char *error, size_t capacity) {
    GD_ViewerServer *s=a.allocate(a.context,sizeof(*s));
    HANDLE token=NULL; TOKEN_USER *user=NULL; DWORD needed=0;
    LPSTR sid=NULL; PSECURITY_DESCRIPTOR descriptor=NULL; SECURITY_ATTRIBUTES attributes;
    char security[512],path[128]; unsigned char nonce[8]; int i; HMODULE library=NULL;
    typedef BOOLEAN (WINAPI *RandomBytes)(PVOID,ULONG); RandomBytes random_bytes=NULL;
    if (!s) { snprintf(error,capacity,"Cannot allocate bounded viewer IPC storage"); return NULL; }
    memset(s,0,sizeof(*s)); s->allocator=a; channel_init(&s->channel);
    library=LoadLibraryA("advapi32.dll");
    if (library) random_bytes=(RandomBytes)GetProcAddress(library,"SystemFunction036");
    if (!random_bytes || !random_bytes(nonce,sizeof(nonce))) goto fail;
    snprintf(s->session,sizeof(s->session),"%lu-",(unsigned long)GetCurrentProcessId());
    for (i=0;i<8;++i) { size_t n=strlen(s->session); snprintf(s->session+n,sizeof(s->session)-n,"%02x",nonce[i]); }
    if (!OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&token)) goto fail;
    GetTokenInformation(token,TokenUser,NULL,0,&needed);
    if (!needed || needed>65536) goto fail;
    user=a.allocate(a.context,needed);
    if (!user || !GetTokenInformation(token,TokenUser,user,needed,&needed) || !ConvertSidToStringSidA(user->User.Sid,&sid)) goto fail;
    if (snprintf(security,sizeof(security),"D:P(A;;GA;;;%s)",sid)>=(int)sizeof(security) ||
        !ConvertStringSecurityDescriptorToSecurityDescriptorA(security,1,&descriptor,NULL)) goto fail;
    memset(&attributes,0,sizeof(attributes)); attributes.nLength=sizeof(attributes); attributes.lpSecurityDescriptor=descriptor;
    snprintf(path,sizeof(path),"\\\\.\\pipe\\%s%s",PIPE_PREFIX,s->session);
    s->channel.event=CreateEventA(NULL,TRUE,FALSE,NULL);
    s->channel.pipe=CreateNamedPipeA(path,PIPE_ACCESS_DUPLEX|FILE_FLAG_OVERLAPPED|FILE_FLAG_FIRST_PIPE_INSTANCE,
        PIPE_TYPE_MESSAGE|PIPE_READMODE_MESSAGE|PIPE_WAIT|PIPE_REJECT_REMOTE_CLIENTS,1,65536,8192,0,&attributes);
    if (!s->channel.event || s->channel.pipe==INVALID_HANDLE_VALUE) goto fail;
    begin(&s->channel,0,NULL,0);
    if (s->channel.error) goto fail;
    if (descriptor) LocalFree(descriptor); if (sid) LocalFree(sid); if (user) a.release(a.context,user);
    CloseHandle(token); FreeLibrary(library); return s;
fail:
    snprintf(error,capacity,"Viewer IPC unavailable (OS error %lu); ordinary viewer remains usable",(unsigned long)GetLastError());
    if (descriptor) LocalFree(descriptor); if (sid) LocalFree(sid); if (user) a.release(a.context,user);
    if (token) CloseHandle(token); if (library) FreeLibrary(library);
    gd_viewer_server_close(s); return NULL;
}
const char *gd_viewer_session(const GD_ViewerServer *s) { return s?s->session:NULL; }
const char *gd_viewer_server_error(const GD_ViewerServer *s) { return s && *s->error?s->error:NULL; }
void gd_viewer_server_poll(GD_ViewerServer *s, GD_ViewerHandler handler, void *context) {
    Channel *c; GD_ViewerWriter w; GD_ViewerRequest request;
    if (!s || *s->error) return;
    c=&s->channel;
    if (s->stage && GetTickCount()-s->started>=PEER_TIMEOUT) { reset(s); return; }
    if (!poll(c)) return;
    if (s->stage==3) {
        /* Extra/pipelined input is not receipt acknowledgement. Drain at most
           one bounded read/poll, retaining reply until peer close/deadline. */
        if (c->error && c->error!=ERROR_MORE_DATA) reset(s);
        else begin(c,1,s->input,sizeof(s->input));
        return;
    }
    if (c->error) {
        if (!s->stage) { snprintf(s->error,sizeof(s->error),"IPC accept failed (OS error %lu)",(unsigned long)c->error); return; }
        if (s->stage==1 && c->error==ERROR_MORE_DATA) {
            w=(GD_ViewerWriter){s->output,0,sizeof(s->output),0};
            gd_viewer_error(&w,"viewer.capacity","Request exceeds 8192 bytes");
            s->stage=2; begin(c,2,s->output,(DWORD)w.size); return;
        }
        reset(s); return;
    }
    if (!s->stage) { s->stage=1; s->started=GetTickCount(); begin(c,1,s->input,sizeof(s->input)); return; }
    if (s->stage==1) {
        w=(GD_ViewerWriter){s->output,0,sizeof(s->output),0};
        if (!gd_viewer_parse(s->input,c->count,&request)) gd_viewer_error(&w,"viewer.protocol","Invalid v1 viewer request, command or fields");
        else if (*request.target && strcmp(request.target,s->session)) gd_viewer_error(&w,"viewer.target","Session target does not match this viewer");
        else handler(context,&request,&w);
        if (w.failed || !w.size) gd_viewer_error(&w,"viewer.capacity","Context exceeds reply capacity; no partial result returned");
        s->stage=2; begin(c,2,s->output,(DWORD)w.size); return;
    }
    if (s->stage==2) {
        /* Buffered write completion is not receipt. Wait for peer close; no flush on owner thread. */
        s->stage=3; begin(c,1,s->input,sizeof(s->input)); return;
    }
    reset(s);
}
void gd_viewer_server_close(GD_ViewerServer *s) {
    GD_Allocator a;
    if (!s) return;
    a=s->allocator;
    if (!channel_close(&s->channel)) return; /* Kernel still owns overlapped backing until process exit. */
    a.release(a.context,s);
}
typedef struct { Channel channel; char output[GD_VIEWER_REQUEST_LIMIT],input[GD_VIEWER_REPLY_LIMIT]; } Exchange;
static int wait_operation(Channel *c, DWORD started, DWORD timeout) {
    DWORD elapsed=GetTickCount()-started;
    if (!c->pending) return !c->error;
    if (elapsed>=timeout || WaitForSingleObject(c->event,timeout-elapsed)!=WAIT_OBJECT_0) return 0;
    return poll(c) && !c->error;
}
static Exchange *exchange(GD_Allocator a, const char *session, const char *request, unsigned timeout, char *error, size_t capacity) {
    Exchange *e=a.allocate(a.context,sizeof(*e)); DWORD started=GetTickCount(),mode=PIPE_READMODE_MESSAGE;
    char path[128]; size_t size=strlen(request);
    if (!e) { snprintf(error,capacity,"Cannot allocate exchange storage"); return NULL; }
    channel_init(&e->channel);
    snprintf(path,sizeof(path),"\\\\.\\pipe\\%s%s",PIPE_PREFIX,session);
    if (!size || size>=sizeof(e->output)) goto fail;
    memcpy(e->output,request,size);
    e->channel.event=CreateEventA(NULL,TRUE,FALSE,NULL);
    if (!e->channel.event) goto fail;
    for (;;) {
        DWORD elapsed;
        e->channel.pipe=CreateFileA(path,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_FLAG_OVERLAPPED,NULL);
        if (e->channel.pipe!=INVALID_HANDLE_VALUE) break;
        if (GetLastError()!=ERROR_PIPE_BUSY) goto fail;
        elapsed=GetTickCount()-started;
        if (elapsed>=timeout || !WaitNamedPipeA(path,timeout-elapsed)) goto fail;
    }
    if (!SetNamedPipeHandleState(e->channel.pipe,&mode,NULL,NULL)) goto fail;
    begin(&e->channel,2,e->output,(DWORD)size);
    if (!wait_operation(&e->channel,started,timeout) || e->channel.count!=size) goto fail;
    begin(&e->channel,1,e->input,sizeof(e->input)-1);
    if (!wait_operation(&e->channel,started,timeout) || !e->channel.count || memchr(e->input,0,e->channel.count)) goto fail;
    e->input[e->channel.count]=0;
    if (!gd_valid_utf8((const unsigned char *)e->input,e->channel.count)) goto fail;
    if (!channel_close(&e->channel)) goto fail;
    return e;
fail:
    snprintf(error,capacity,"Viewer exchange failed/timed out; mutation outcome may be unknown. Do not automatically retry mutations.");
    if (!channel_close(&e->channel)) { snprintf(error,capacity,"IPC cancellation unsettled; outcome unknown, backing retained until exit"); return NULL; }
    a.release(a.context,e); return NULL;
}
#else
#include "gd_viewer_macos.h"
#endif
static int session_valid(const char *text) {
    size_t i=0,digits=0;
    while (text[i]>='0' && text[i]<='9') { ++i; ++digits; }
    if (!digits || digits>10 || text[0]=='0' || text[i++]!='-') return 0;
    for (digits=0;digits<16;++digits,++i) if (!((text[i]>='0'&&text[i]<='9') || (text[i]>='a'&&text[i]<='f'))) return 0;
    return !text[i];
}
#ifdef _WIN32
static int instances(char sessions[GD_VIEWER_INSTANCES][GD_VIEWER_SESSION], int *count) {
    WIN32_FIND_DATAA data; HANDLE search;
    *count=0; search=FindFirstFileA("\\\\.\\pipe\\" PIPE_PREFIX "*",&data);
    if (search==INVALID_HANDLE_VALUE) return GetLastError()==ERROR_FILE_NOT_FOUND || GetLastError()==ERROR_NO_MORE_FILES;
    do {
        const char *name=data.cFileName;
        if (strncmp(name,PIPE_PREFIX,strlen(PIPE_PREFIX)) || !session_valid(name+strlen(PIPE_PREFIX))) continue;
        if (*count==GD_VIEWER_INSTANCES) { FindClose(search); return 0; }
        snprintf(sessions[(*count)++],GD_VIEWER_SESSION,"%s",name+strlen(PIPE_PREFIX));
    } while (FindNextFileA(search,&data));
    { DWORD error=GetLastError(); FindClose(search); return error==ERROR_NO_MORE_FILES; }
}
#endif
static int cli_failure(const char *code, const char *message) {
    char output[1024]; GD_ViewerWriter w={output,0,sizeof(output),0};
    gd_viewer_error(&w,code,message); puts(output); fprintf(stderr,"%s: %s\n",code,message); return 2;
}
int gd_viewer_command(int argc, char **argv, GD_Allocator a) {
    const char *command=argc>2?argv[2]:NULL,*target=NULL,*projection=NULL,*reference=NULL,*fields=NULL,*id=NULL,*offset=NULL,*limit=NULL,*action=NULL,*form=NULL,*control=NULL,*value=NULL,*expect=NULL;
    unsigned page_offset=0,page_limit=GD_VIEWER_NODE_PAGE;
    char sessions[GD_VIEWER_INSTANCES][GD_VIEWER_SESSION],request[GD_VIEWER_REQUEST_LIMIT],error[512];
    GD_ViewerWriter w={request,0,sizeof(request),0}; GD_ViewerRequest parsed;
    int i,count=0; Exchange *e;
    if (command && !strcmp(command,"--help")) {
        puts("Usage: gd viewer list|inspect|nodes|hint|navigate|actions|action|refresh|controls|configure|evidence|select|result [--target SESSION] [--projection VIEW]\n"
             "hint/navigate --reference ALIAS/NAME (Find) or --id NATIVE_ID (exact select/frame); never both.\n"
             "nodes [--projection VIEW] [--offset 0..4096] [--limit 1..64] reads prepared canvas; default active view, page 64.\n"
             "inspect --fields selection,snapshots,panes,agent_hint,source narrows optional sections; source body opt-in.\n"
             "hint requires projection and reference/id; navigate requires projection.\n"
             "actions [--projection VIEW] lists shared application actions; action --projection VIEW --action NAME runs one.\n"
             "controls [--projection VIEW] [--form FORM [--control NAME --offset N --limit N]] reads types/values/choice keys.\n"
             "configure --projection VIEW --form FORM --control NAME --value TEXT --expect TOKEN stages analytical input; presentation applies immediately like GUI. Empty text clears.\n"
             "evidence [--projection VIEW] [--form links|cells|cases|diagnostics|layers|starts|known] [--offset N --limit N] reads retained pages.\n"
             "select --projection VIEW --form link|cell|case|source --id KEY --expect TOKEN changes exact inspection; cell also accepts --reference NOUN --id VERB.\n"
             "result --projection VIEW --form query|structure|rules|bindings|resource|mda|challenges|actor-overview|noun-grid [--fields NATIVE_FIELDS --offset BYTE --limit KIB --expect TOKEN] reads base64 native JSON chunks; no evaluation.\n"
             "refresh reloads original source bindings now; active edits/dirty drafts block, never discard.\n"
             "JSON stdout; diagnostics stderr; exit 0 success / 2 failure. No auto-start, source writes or OS focus changes.\n"
             "inspect/list are non-consuming. Multiple viewers require exact --target from list. Timeout outcome may be unknown; never auto-retry mutations."); return 0;
    }
    if (!command) return cli_failure("viewer.arguments","Supply list, inspect, nodes, hint, navigate, actions, action, refresh, controls or configure; see gd viewer --help");
    for (i=3;i<argc;++i) {
        const char **field=!strcmp(argv[i],"--target")?&target:!strcmp(argv[i],"--projection")?&projection:!strcmp(argv[i],"--reference")?&reference:!strcmp(argv[i],"--fields")?&fields:!strcmp(argv[i],"--id")?&id:!strcmp(argv[i],"--offset")?&offset:!strcmp(argv[i],"--limit")?&limit:!strcmp(argv[i],"--action")?&action:!strcmp(argv[i],"--form")?&form:!strcmp(argv[i],"--control")?&control:!strcmp(argv[i],"--value")?&value:!strcmp(argv[i],"--expect")?&expect:NULL;
        if (!strcmp(argv[i],"--json")) continue;
        if (!field || *field || i+1==argc || (!*argv[i+1] && field!=&value)) return cli_failure("viewer.arguments","Unknown, duplicate or missing option; see gd viewer --help");
        *field=argv[++i];
    }
    if (target && !session_valid(target)) return cli_failure("viewer.target","Use exact session identity from gd viewer list");
    if (!strcmp(command,"list")) {
        if (target || projection || reference || fields || id || offset || limit || action || form || control || value || expect) return cli_failure("viewer.arguments","list accepts only --json");
        if (!instances(sessions,&count)) return cli_failure("viewer.discovery","Discovery failed or exceeds 32 candidates");
        /* All candidates remain explicit, including unresponsive endpoints. */
        fputs("{\"ok\":true,\"viewers\":[",stdout);
        for (i=0;i<count;++i) {
            if (i) fputc(',',stdout);
            printf("{\"session\":\"%s\"}",sessions[i]);
        }
        puts("]}"); return ferror(stdout)?2:0;
    }
    if (offset) {
        Reader r={offset,offset+strlen(offset)};
        if (!unsigned_value(&r,!strcmp(command,"result") ? 64u*1024u*1024u:!strcmp(command,"evidence") ? GD_MAX_EDGES:!strcmp(command,"controls") ? GD_VIEWER_CONTROL_OFFSET:GD_MAX_RECORDS,&page_offset) || r.p!=r.end) return cli_failure("viewer.arguments","offset outside command's bounded source-slot range");
    }
    if (limit) {
        Reader r={limit,limit+strlen(limit)};
        if (!unsigned_value(&r,GD_VIEWER_NODE_PAGE,&page_limit) || !page_limit || r.p!=r.end) return cli_failure("viewer.arguments","limit must be integer 1..64");
    }
    gd_viewer_write(&w,"{\"protocol\":1,\"command\":"); gd_viewer_quote(&w,command);
    if (target) { gd_viewer_write(&w,",\"target\":"); gd_viewer_quote(&w,target); }
    if (projection) { gd_viewer_write(&w,",\"projection\":"); gd_viewer_quote(&w,projection); }
    if (reference) { gd_viewer_write(&w,",\"reference\":"); gd_viewer_quote(&w,reference); }
    if (fields) { gd_viewer_write(&w,",\"fields\":"); gd_viewer_quote(&w,fields); }
    if (id) { gd_viewer_write(&w,",\"id\":"); gd_viewer_quote(&w,id); }
    if (action) { gd_viewer_write(&w,",\"action\":"); gd_viewer_quote(&w,action); }
    if (form) { gd_viewer_write(&w,",\"form\":"); gd_viewer_quote(&w,form); }
    if (control) { gd_viewer_write(&w,",\"control\":"); gd_viewer_quote(&w,control); }
    if (value) { gd_viewer_write(&w,",\"value\":"); gd_viewer_quote(&w,value); }
    if (expect) { gd_viewer_write(&w,",\"expect\":"); gd_viewer_quote(&w,expect); }
    if (offset) gd_viewer_write(&w,",\"offset\":%u",page_offset);
    if (limit) gd_viewer_write(&w,",\"limit\":%u",page_limit);
    gd_viewer_write(&w,"}");
    if (w.failed) return cli_failure("viewer.capacity","Encoded request exceeds 8 KiB limit; no request sent");
    if (!gd_viewer_parse(request,w.size,&parsed)) return cli_failure("viewer.arguments","Invalid command/fields; see gd viewer --help");
    if (!target) {
        if (!instances(sessions,&count)) return cli_failure("viewer.discovery","Discovery failed or exceeds 32 candidates; use exact --target");
        if (count!=1) return cli_failure(count?"viewer.ambiguous":"viewer.not_running",count?"Several viewers exist; use exact --target from gd viewer list":"No running IPC-capable viewer; open explorer explicitly");
        target=sessions[0];
    }
    e=exchange(a,target,request,2000,error,sizeof(error));
    if (!e) return cli_failure("viewer.exchange",error);
    /* Replies originate at trusted same-user endpoint. Fixed envelope is emitted by owner. */
    count=strncmp(e->input,"{\"ok\":true",10)?2:0;
    puts(e->input); if (count) fprintf(stderr,"Viewer rejected request; see structured stdout\n");
    a.release(a.context,e); return ferror(stdout)?2:count;
}
#else
struct GD_ViewerServer { int unused; };
GD_ViewerServer *gd_viewer_server_open(GD_Allocator a, char *error, size_t capacity) { (void)a; snprintf(error,capacity,"Viewer IPC supports Windows and macOS only"); return NULL; }
const char *gd_viewer_session(const GD_ViewerServer *s) { (void)s; return NULL; }
const char *gd_viewer_server_error(const GD_ViewerServer *s) { (void)s; return NULL; }
void gd_viewer_server_poll(GD_ViewerServer *s, GD_ViewerHandler h, void *c) { (void)s; (void)h; (void)c; }
void gd_viewer_server_close(GD_ViewerServer *s) { (void)s; }
int gd_viewer_command(int argc, char **argv, GD_Allocator a) { (void)argc; (void)argv; (void)a; puts("{\"ok\":false,\"code\":\"viewer.unsupported\",\"message\":\"Viewer IPC supports Windows and macOS only\"}"); return 2; }
#endif
