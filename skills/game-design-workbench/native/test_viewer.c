/* CJIT headless protocol/owner fixture. No window, renderer or project writes.
   build.ps1 -ViewerTest; test-viewer.exe checks; --serve used by native-viewer.test.mjs. */
#include "gd_viewer.h"
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#endif
typedef struct { int live,calls,fail; } Memory;
static void *allocate(void *context, size_t size) {
    Memory *state=context; void *memory;
    if (state && ++state->calls==state->fail) return NULL;
    memory=malloc(size); if (memory && state) ++state->live; return memory;
}
static void release(void *context, void *memory) { Memory *state=context; if (memory && state) --state->live; free(memory); }
#ifdef _WIN32
static int lifecycle(void);
#endif
static int checks(void) {
    const char *valid[]={
        "{\"protocol\":1,\"command\":\"inspect\"}",
        " { \"command\":\"navigate\",\"projection\":\"skills\",\"protocol\":1 } ",
        "{\"protocol\":1,\"command\":\"nodes\",\"offset\":0,\"limit\":64}",
        "{\"protocol\":1,\"command\":\"nodes\",\"projection\":\"effects\",\"offset\":4096,\"limit\":1}",
        "{\"protocol\":1,\"command\":\"hint\",\"projection\":\"effects\",\"id\":\"effect:Ω\"}",
        "{\"protocol\":1,\"command\":\"inspect\",\"fields\":\"source\"}",
        "{\"protocol\":1,\"command\":\"inspect\",\"fields\":\"selection,panes\"}",
        "{\"protocol\":1,\"command\":\"hint\",\"projection\":\"skills\",\"reference\":\"skills/café\"}",
        "{\"protocol\":1,\"command\":\"hint\",\"projection\":\"skills\",\"reference\":\"skills/caf\\u00e9\\u03a9\\ud83d\\ude00\"}"
    };
    const char *invalid[]={
        "{}","[]","null","{\"protocol\":2,\"command\":\"inspect\"}",
        "{\"protocol\":1.0,\"command\":\"inspect\"}",
        "{\"protocol\":1,\"command\":\"inspect\",\"command\":\"inspect\"}",
        "{\"protocol\":1,\"command\":\"inspect\",\"comm\\u0061nd\":\"inspect\"}",
        "{\"protocol\":1,\"command\":\"inspect\",\"unknown\":false}",
        "{\"protocol\":1,\"command\":\"inspect\",\"reference\":\"x\"}",
        "{\"protocol\":1,\"command\":\"inspect\",}",
        "{\"protocol\":1,\"command\":\"navigate\"}",
        "{\"protocol\":1,\"command\":\"hint\",\"projection\":\"skills\"}",
        "{\"protocol\":1,\"command\":\"hint\",\"projection\":\"skills\",\"reference\":\"\\u0000\"}",
        "{\"protocol\":1,\"command\":\"hint\",\"projection\":\"skills\",\"reference\":\"\\ud800\"}",
        "{\"protocol\":1,\"command\":\"hint\",\"projection\":\"skills\",\"reference\":\"\\udc00\"}",
        "{\"protocol\":1,\"command\":\"inspect\"} trailing",
        "{\"protocol\":1,\"command\":\"inspect\",\"target\":\"\"}",
        "{\"protocol\":1,\"command\":\"quit\"}",
        "{\"protocol\":1,\"command\":\"navigate\",\"projection\":\"effects\",\"id\":\"x\",\"reference\":\"x\"}",
        "{\"protocol\":1,\"command\":\"hint\",\"id\":\"x\"}",
        "{\"protocol\":1,\"command\":\"inspect\",\"id\":\"x\"}",
        "{\"protocol\":1,\"command\":\"nodes\",\"id\":\"x\"}",
        "{\"protocol\":1,\"command\":\"nodes\",\"fields\":\"selection\"}",
        "{\"protocol\":1,\"command\":\"nodes\",\"offset\":-1}",
        "{\"protocol\":1,\"command\":\"nodes\",\"offset\":4097}",
        "{\"protocol\":1,\"command\":\"nodes\",\"offset\":1.0}",
        "{\"protocol\":1,\"command\":\"nodes\",\"offset\":01}",
        "{\"protocol\":1,\"command\":\"nodes\",\"offset\":\"1\"}",
        "{\"protocol\":1,\"command\":\"nodes\",\"limit\":0}",
        "{\"protocol\":1,\"command\":\"nodes\",\"limit\":65}",
        "{\"protocol\":1,\"command\":\"inspect\",\"limit\":1}",
        "{\"protocol\":1,\"command\":\"navigate\",\"projection\":\"effects\",\"offset\":0}",
        "{\"protocol\":1,\"command\":\"inspect\",\"fields\":\"unknown\"}",
        "{\"protocol\":1,\"command\":\"inspect\",\"fields\":\"selection,selection\"}",
        "{\"protocol\":1,\"command\":\"inspect\",\"fields\":\"selection,\"}",
        "{\"protocol\":1,\"command\":\"navigate\",\"projection\":\"skills\",\"fields\":\"panes\"}",
        "{\"protocol\":1,\"command\":\"inspect\",\"target\":{}}",
        "{\"protocol\":1,\"command\":\"inspect\",\"target\":\"\xff\"}"
    };
    GD_ViewerRequest request; size_t i; char bytes[64]; GD_ViewerWriter w={bytes,0,sizeof(bytes),0};
    for (i=0;i<sizeof(valid)/sizeof(*valid);++i) if (!gd_viewer_parse(valid[i],strlen(valid[i]),&request)) { fprintf(stderr,"Valid protocol case %zu rejected\n",i); return 2; }
    if (strcmp(request.reference,"skills/caféΩ😀")) { fputs("Unicode decoding mismatch\n",stderr); return 2; }
    for (i=0;i<sizeof(invalid)/sizeof(*invalid);++i) if (gd_viewer_parse(invalid[i],strlen(invalid[i]),&request)) { fprintf(stderr,"Invalid protocol case %zu accepted\n",i); return 2; }
    if (gd_viewer_parse(valid[0],strlen(valid[0])+1,&request)) return 2;
    gd_viewer_quote(&w,"a\n\"\\Ω");
    if (w.failed || strcmp(bytes,"\"a\\u000a\\\"\\\\Ω\"")) return 2;
    w.capacity=4; w.size=0; gd_viewer_quote(&w,"long"); if (!w.failed) return 2;
    w.capacity=sizeof(bytes); gd_viewer_error(&w,"capacity","full");
    if (w.failed || strcmp(bytes,"{\"ok\":false,\"code\":\"capacity\",\"message\":\"full\"}")) return 2;
#ifdef _WIN32
    if (!lifecycle()) { fputs("Transport lifecycle/allocation check failed\n",stderr); return 2; }
#endif
    printf("{\"ok\":true,\"protocol_cases\":%zu,\"writer_checks\":3}\n",sizeof(valid)/sizeof(*valid)+sizeof(invalid)/sizeof(*invalid)+1); return 0;
}
#ifdef _WIN32
/* Verify actual kernel-object DACL, not merely a successful same-user connect. */
static int server_security(const char *session) {
    char path[128]; DWORD needed=0;
    union { void *alignment; unsigned char bytes[4096]; } descriptor,token_bytes;
    HANDLE pipe,token=NULL; PACL acl=NULL; BOOL present=FALSE,defaulted=FALSE;
    ACL_SIZE_INFORMATION information; ACCESS_ALLOWED_ACE *ace=NULL; int okay=0;
    snprintf(path,sizeof(path),"\\\\.\\pipe\\gd-workbench-%s",session);
    pipe=CreateFileA(path,READ_CONTROL,0,NULL,OPEN_EXISTING,0,NULL);
    if (pipe==INVALID_HANDLE_VALUE) return 0;
    if (!GetKernelObjectSecurity(pipe,DACL_SECURITY_INFORMATION,(PSECURITY_DESCRIPTOR)descriptor.bytes,sizeof(descriptor.bytes),&needed) ||
        !GetSecurityDescriptorDacl((PSECURITY_DESCRIPTOR)descriptor.bytes,&present,&acl,&defaulted) || !present || !acl ||
        !GetAclInformation(acl,&information,sizeof(information),AclSizeInformation) || information.AceCount!=1 ||
        !GetAce(acl,0,(void **)&ace) || ace->Header.AceType!=ACCESS_ALLOWED_ACE_TYPE ||
        !OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&token) ||
        !GetTokenInformation(token,TokenUser,token_bytes.bytes,sizeof(token_bytes.bytes),&needed)) goto done;
    okay=EqualSid((PSID)&ace->SidStart,((TOKEN_USER *)token_bytes.bytes)->User.Sid)!=0;
done:
    if (token) CloseHandle(token); CloseHandle(pipe); return okay;
}
static int lifecycle(void) {
    int i;
    for (i=0;i<5;++i) {
        Memory memory={0}; GD_Allocator a={&memory,allocate,release}; char error[256]; GD_ViewerServer *server;
        memory.fail=i<2?i+1:0; server=gd_viewer_server_open(a,error,sizeof(error));
        if (i<2 && server) { gd_viewer_server_close(server); return 0; }
        if (i>=2 && (!server || !server_security(gd_viewer_session(server)))) { gd_viewer_server_close(server); return 0; }
        gd_viewer_server_close(server); if (memory.live) return 0;
    }
    return 1;
}
typedef struct { int mutations; GD_ViewerServer *server; } Owner;
static void dispatch(void *context, const GD_ViewerRequest *request, GD_ViewerWriter *w) {
    Owner *owner=context;
    if (!strcmp(request->command,"inspect")) {
        gd_viewer_write(w,"{\"ok\":true,\"mutations\":%d,\"session\":",owner->mutations); gd_viewer_quote(w,gd_viewer_session(owner->server)); gd_viewer_write(w,"}");
    } else if (!strcmp(request->command,"nodes")) {
        gd_viewer_write(w,"{\"ok\":true,\"nodes\":[],\"total\":0,\"offset\":%u,\"limit\":%u,\"next_offset\":null}",request->offset,request->limit);
    } else {
        ++owner->mutations;
        if (!strcmp(request->reference,"slow")) Sleep(2300); /* Test-only delayed owner reply. */
        if (!strcmp(request->reference,"large") || !strcmp(request->reference,"overflow")) {
            size_t i,limit=!strcmp(request->reference,"large")?256*1024:GD_VIEWER_REPLY_LIMIT;
            gd_viewer_write(w,"{\"ok\":true,\"text\":\"");
            for (i=0;i<limit && !w->failed;++i) gd_viewer_write(w,"x");
            gd_viewer_write(w,"\"}");
        } else { gd_viewer_write(w,"{\"ok\":true,\"reference\":"); gd_viewer_quote(w,request->reference); gd_viewer_write(w,",\"id\":"); gd_viewer_quote(w,request->id); gd_viewer_write(w,"}"); }
    }
}
#endif
int main(int argc, char **argv) {
    if (argc==1) return checks();
#ifdef _WIN32
    if (argc==2 && !strcmp(argv[1],"--serve")) {
        GD_Allocator a={NULL,allocate,release}; char error[512]; DWORD start=GetTickCount(); Owner owner={0};
        owner.server=gd_viewer_server_open(a,error,sizeof(error));
        if (!owner.server) { fprintf(stderr,"%s\n",error); return 2; }
        if (!server_security(gd_viewer_session(owner.server))) { fputs("Server DACL check failed\n",stderr); gd_viewer_server_close(owner.server); return 2; }
        printf("{\"session\":\"%s\"}\n",gd_viewer_session(owner.server)); fflush(stdout);
        while (GetTickCount()-start<30000 && !gd_viewer_server_error(owner.server)) {
            gd_viewer_server_poll(owner.server,dispatch,&owner); Sleep(1);
        }
        gd_viewer_server_close(owner.server); return 0;
    }
#endif
    fputs("Usage: test-viewer.exe [--serve]\n",stderr); return 2;
}
