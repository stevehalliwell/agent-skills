/* Explicit local process supervision, not Git/approval/lifecycle automation.
   Never invokes a shell or compiler. Reviewed code/input and execution approval
   remain agent/human gates. Synthetic emitters exercise this infrastructure. */
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif
#include "gd_experiment_result.h"
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#include <fcntl.h>
#include <io.h>

/* CJIT's older Windows headers omit startup-attribute declarations. Resolve
   installed Kernel32 API with its documented ABI; never weaken handle list. */
typedef BOOL (WINAPI *GD_InitializeAttributes)(void *,DWORD,DWORD,SIZE_T *);
typedef BOOL (WINAPI *GD_UpdateAttribute)(void *,DWORD,DWORD_PTR,void *,SIZE_T,void *,SIZE_T *);
typedef void (WINAPI *GD_DeleteAttributes)(void *);
typedef struct { STARTUPINFOW StartupInfo; void *lpAttributeList; } GD_StartupInfoEx;
#define GD_HANDLE_LIST ((DWORD_PTR)0x00020002)
#ifndef EXTENDED_STARTUPINFO_PRESENT
#define EXTENDED_STARTUPINFO_PRESENT 0x00080000
#endif

typedef struct {
    HANDLE job,out_read,out_write,err_read,err_write,null_input;
    HANDLE out_file,err_file,watchdog,watchdog_stop;
    volatile LONG watchdog_status;
    double started,timeout;
    PROCESS_INFORMATION process;
    void *attributes;
    GD_DeleteAttributes delete_attributes;
    int attributes_ready;
} OwnedProcess;
static void *allocate(void *context,size_t size) { (void)context; return malloc(size); }
static void release(void *context,void *memory) { (void)context; free(memory); }
static void close_handle(HANDLE *handle) {
    if (*handle && *handle!=INVALID_HANDLE_VALUE) CloseHandle(*handle);
    *handle=NULL;
}
static int ascii(const char *text,size_t limit) {
    const unsigned char *p=(const unsigned char *)text;
    if (!text || !*text || strlen(text)>limit) return 0;
    while (*p) if (*p++>=128) return 0;
    return 1;
}
static int full_path(const char *source,char *target) {
    DWORD n=GetFullPathNameA(source,GD_MAX_PATH,target,NULL);
    char drive[4]; UINT type;
    if (!n || n>=GD_MAX_PATH || !ascii(target,GD_MAX_PATH-1) || n<3 || target[1]!=':' || target[2]!='\\') return 0;
    memcpy(drive,target,3); drive[3]=0; type=GetDriveTypeA(drive);
    return type==DRIVE_FIXED || type==DRIVE_RAMDISK;
}
static int join(char *path,const char *folder,const char *name) {
    int n=snprintf(path,GD_MAX_PATH,"%s\\%s",folder,name);
    return n>=0 && n<GD_MAX_PATH;
}
static int write_bytes(HANDLE file,const void *bytes,DWORD count) {
    DWORD written=0;
    return WriteFile(file,bytes,count,&written,NULL) && written==count;
}
/* Quote each argument for Windows CRT, including trailing backslashes and
   embedded quotes. lpApplicationName selects executable separately: no shell. */
static int quote_argument(wchar_t *command,size_t *used,const char *argument) {
    size_t i,slashes=0;
    if (*used && *used<32767) command[(*used)++]=L' ';
    if (*used>=32766) return 0;
    command[(*used)++]=L'"';
    for (i=0;;++i) {
        unsigned char c=(unsigned char)argument[i]; size_t count;
        if (c=='\\') { ++slashes; continue; }
        count=(c=='"' || !c) ? 2*slashes : slashes;
        if (c=='"') ++count;
        if (count>32765-*used) return 0;
        while (count--) command[(*used)++]=L'\\';
        slashes=0;
        if (!c) break;
        if (*used>=32765) return 0;
        command[(*used)++]=(wchar_t)c;
    }
    if (*used>=32767) return 0;
    command[(*used)++]=L'"'; command[*used]=0; return 1;
}
static int make_pipe(HANDLE *read,HANDLE *write) {
    SECURITY_ATTRIBUTES security;
    memset(&security,0,sizeof(security)); security.nLength=sizeof(security); security.bInheritHandle=TRUE;
    return CreatePipe(read,write,&security,0) && SetHandleInformation(*read,HANDLE_FLAG_INHERIT,0);
}
/* Independent watchdog can stop model/job even while capture thread waits on
   local disk I/O. It never writes files or touches processes outside this job. */
static DWORD WINAPI watchdog(void *context) {
    OwnedProcess *owned=context;
    for (;;) {
        double now=gd_experiment_clock(),remaining=owned->timeout-(now-owned->started);
        DWORD wait,milliseconds;
        if (now<owned->started || remaining<=0) {
            InterlockedExchange(&owned->watchdog_status,now<owned->started ? 2 : 1);
            TerminateJobObject(owned->job,2); return 0;
        }
        milliseconds=remaining>=1 ? 1000 : (DWORD)(remaining*1000)+1;
        wait=WaitForSingleObject(owned->watchdog_stop,milliseconds);
        if (wait==WAIT_OBJECT_0) return 0;
        if (wait!=WAIT_TIMEOUT) { InterlockedExchange(&owned->watchdog_status,2); TerminateJobObject(owned->job,2); return 0; }
    }
}
static void stop_watchdog(OwnedProcess *owned) {
    if (owned->watchdog) {
        SetEvent(owned->watchdog_stop);
        if (WaitForSingleObject(owned->watchdog,1000)!=WAIT_OBJECT_0) {
            /* Do not release borrowed job/state while thread is alive. Process
               exit closes job and stops owned model; evidence is incomplete. */
            TerminateJobObject(owned->job,2); ExitProcess(2);
        }
    }
    close_handle(&owned->watchdog); close_handle(&owned->watchdog_stop);
}
static int launch(OwnedProcess *owned,const char *program,const char *directory,wchar_t *command,GD_Allocator allocator) {
    GD_StartupInfoEx startup;
    HMODULE kernel=GetModuleHandleW(L"kernel32.dll");
    GD_InitializeAttributes initialize_attributes=(GD_InitializeAttributes)(void *)GetProcAddress(kernel,"InitializeProcThreadAttributeList");
    GD_UpdateAttribute update_attribute=(GD_UpdateAttribute)(void *)GetProcAddress(kernel,"UpdateProcThreadAttribute");
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION job_limits;
    SIZE_T size=0;
    HANDLE inherited[3];
    wchar_t wide_program[GD_MAX_PATH],wide_directory[GD_MAX_PATH];
    SECURITY_ATTRIBUTES security;
    int i;
    memset(&startup,0,sizeof(startup)); memset(&job_limits,0,sizeof(job_limits));
    memset(&security,0,sizeof(security)); security.nLength=sizeof(security); security.bInheritHandle=TRUE;
    for (i=0;program[i];++i) wide_program[i]=(wchar_t)(unsigned char)program[i]; wide_program[i]=0;
    for (i=0;directory[i];++i) wide_directory[i]=(wchar_t)(unsigned char)directory[i]; wide_directory[i]=0;
    owned->job=CreateJobObjectW(NULL,NULL);
    job_limits.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    if (!owned->job || !SetInformationJobObject(owned->job,JobObjectExtendedLimitInformation,&job_limits,sizeof(job_limits)) ||
        !make_pipe(&owned->out_read,&owned->out_write) || !make_pipe(&owned->err_read,&owned->err_write)) return 0;
    owned->null_input=CreateFileA("NUL",GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,&security,OPEN_EXISTING,0,NULL);
    if (owned->null_input==INVALID_HANDLE_VALUE) return 0;
    owned->delete_attributes=(GD_DeleteAttributes)(void *)GetProcAddress(kernel,"DeleteProcThreadAttributeList");
    if (!initialize_attributes || !update_attribute || !owned->delete_attributes) return 0;
    initialize_attributes(NULL,1,0,&size);
    if (!size || size>65536) return 0;
    owned->attributes=allocator.allocate(allocator.context,size);
    if (!owned->attributes || !initialize_attributes(owned->attributes,1,0,&size)) return 0;
    owned->attributes_ready=1;
    inherited[0]=owned->null_input; inherited[1]=owned->out_write; inherited[2]=owned->err_write;
    if (!update_attribute(owned->attributes,0,GD_HANDLE_LIST,inherited,sizeof(inherited),NULL,NULL)) return 0;
    startup.StartupInfo.cb=sizeof(startup); startup.StartupInfo.dwFlags=STARTF_USESTDHANDLES;
    startup.StartupInfo.hStdInput=inherited[0]; startup.StartupInfo.hStdOutput=inherited[1]; startup.StartupInfo.hStdError=inherited[2];
    startup.lpAttributeList=owned->attributes;
    if (!CreateProcessW(wide_program,command,NULL,NULL,TRUE,CREATE_SUSPENDED|CREATE_NO_WINDOW|EXTENDED_STARTUPINFO_PRESENT,NULL,wide_directory,&startup.StartupInfo,&owned->process)) return 0;
    /* Never resume outside owned kill-on-close job. Assignment failure leaves
       suspended child for explicit cleanup; no fallback unsupervised launch. */
    if (!AssignProcessToJobObject(owned->job,owned->process.hProcess)) return 0;
    owned->watchdog_stop=CreateEventW(NULL,TRUE,FALSE,NULL);
    if (!owned->watchdog_stop) return 0;
    owned->watchdog=CreateThread(NULL,0,watchdog,owned,0,NULL);
    if (!owned->watchdog || ResumeThread(owned->process.hThread)==(DWORD)-1) return 0;
    close_handle(&owned->out_write); close_handle(&owned->err_write); close_handle(&owned->null_input);
    return 1;
}
/* One bounded chunk per stream/tick; stderr cannot starve stdout or deadlock
   behind it. Return 0 I/O error, 1 drained/pending, 2 output cap. */
static int capture(HANDLE pipe,HANDLE file,size_t *stream_bytes,size_t *total,size_t limit,int *closed) {
    DWORD available=0,read=0,written=0,take; unsigned char bytes[8192];
    if (*closed) return 1;
    if (!PeekNamedPipe(pipe,NULL,0,NULL,&available,NULL)) {
        if (GetLastError()==ERROR_BROKEN_PIPE) { *closed=1; return 1; }
        return 0;
    }
    if (!available) return 1;
    take=available>sizeof(bytes) ? (DWORD)sizeof(bytes) : available;
    if ((size_t)take>limit-*total) take=(DWORD)(limit-*total);
    if (take) {
        BOOL success;
        if (!ReadFile(pipe,bytes,take,&read,NULL) || read!=take) return 0;
        success=WriteFile(file,bytes,read,&written,NULL);
        *stream_bytes+=written; *total+=written;
        if (!success || written!=read) return 0;
    }
    return available>take && *total==limit ? 2 : 1;
}
static int stop_job(OwnedProcess *owned) {
    double started; int ok=0;
    stop_watchdog(owned);
    if (!owned->job) return 1;
    started=gd_experiment_clock();
    if (TerminateJobObject(owned->job,2) && started>=0) {
        for (;;) {
            JOBOBJECT_BASIC_ACCOUNTING_INFORMATION accounting;
            double now;
            if (!QueryInformationJobObject(owned->job,JobObjectBasicAccountingInformation,&accounting,sizeof(accounting),NULL)) break;
            if (!accounting.ActiveProcesses) { ok=1; break; }
            now=gd_experiment_clock(); if (now<started || now-started>=1) break;
            Sleep(1);
        }
    }
    close_handle(&owned->job); /* Kill-on-close remains active on all failures. */
    return ok;
}
static int cleanup(OwnedProcess *owned,GD_Allocator allocator) {
    int ok=1;
    /* Explicit termination also handles parent exit with inherited pipes held
       by descendants. Closing job is second kill guarantee, not process search. */
    if (!stop_job(owned)) ok=0;
    if (owned->process.hProcess) {
        if (WaitForSingleObject(owned->process.hProcess,0)!=WAIT_OBJECT_0) TerminateProcess(owned->process.hProcess,2);
        if (WaitForSingleObject(owned->process.hProcess,1000)!=WAIT_OBJECT_0) ok=0;
    }
    close_handle(&owned->job);
    close_handle(&owned->process.hThread); close_handle(&owned->process.hProcess);
    close_handle(&owned->out_read); close_handle(&owned->out_write); close_handle(&owned->err_read); close_handle(&owned->err_write);
    close_handle(&owned->null_input); close_handle(&owned->out_file); close_handle(&owned->err_file);
    if (owned->attributes_ready) owned->delete_attributes(owned->attributes);
    if (owned->attributes) allocator.release(allocator.context,owned->attributes);
    owned->attributes=NULL; return ok;
}
static void json_string(FILE *output,const char *text) {
    char quoted[GD_MAX_PATH*6+3];
    if (!text) fputs("null",output);
    else if (gd_event_quote(quoted,sizeof(quoted),text)) fputs(quoted,output);
    else fputs("null",output);
}
static void summary(FILE *output,const char *status,int code,DWORD windows_error,int launched,int exit_code,double elapsed,
                    size_t stdout_bytes,size_t stderr_bytes,const char *program,const char *input,const char *folder,
                    const GD_ResultProvenance *provenance,const GD_ResultReport *validation) {
    fprintf(output,"{\"schema_version\":1,\"ok\":%s,\"supervision_status\":",code==0 ? "true" : "false"); json_string(output,status);
    fprintf(output,",\"windows_error\":%lu,\"launched\":%s,\"process_exit_code\":",(unsigned long)windows_error,launched ? "true" : "false");
    if (launched) fprintf(output,"%d",exit_code); else fputs("null",output);
    fprintf(output,",\"elapsed_seconds\":%.17g,\"stdout_bytes\":%llu,\"stderr_bytes\":%llu,\"program\":",
        elapsed,(unsigned long long)stdout_bytes,(unsigned long long)stderr_bytes);
    json_string(output,program); fputs(",\"source_input\":",output); json_string(output,input); fputs(",\"output_directory\":",output); json_string(output,folder); fputs(",\"working_directory\":",output); json_string(output,folder);
    fputs(",\"checkpoint_commit\":",output); json_string(output,provenance->checkpoint);
    fputs(",\"build\":",output); json_string(output,provenance->build); fputs(",\"toolchain\":",output); json_string(output,provenance->toolchain);
    fputs(",\"validation\":",output); gd_result_write(output,validation); fputs("}\n",output);
}
int main(int argc,char **argv) {
    GD_Allocator allocator={NULL,allocate,release};
    GD_ExperimentInput *input=NULL;
    GD_ResultContract contract;
    GD_ResultReport validation;
    GD_ResultProvenance provenance={NULL,NULL,NULL};
    OwnedProcess owned;
    const char *program_arg=NULL,*input_arg=NULL,*output_arg=NULL,*status="arguments";
    char program[GD_MAX_PATH]={0},source[GD_MAX_PATH]={0},folder[GD_MAX_PATH]={0};
    char copy[GD_MAX_PATH],out_path[GD_MAX_PATH],err_path[GD_MAX_PATH],report_path[GD_MAX_PATH];
    HANDLE source_file=NULL,copy_file=NULL;
    FILE *stream=NULL;
    wchar_t *command=NULL;
    size_t used=0,total=0,out_bytes=0,err_bytes=0;
    double started=-1,elapsed=0;
    DWORD windows_error=0,process_exit=2,read=0;
    int i,code=2,created=0,launched=0,out_closed=0,err_closed=0;
    memset(&owned,0,sizeof(owned)); memset(&validation,0,sizeof(validation));
    validation.status="invalid"; validation.code="supervisor.not_started"; validation.message="No model execution; inspect supervision status";
    for (i=1;i<argc;++i) {
        const char **target=NULL;
        if (!strcmp(argv[i],"--help") && argc==2) {
            puts("Usage: gd-experiment-run --program MODEL.exe --input FROZEN.json --output NEW_DIRECTORY\n       --checkpoint COMMIT --build COMMAND --toolchain VERSION\nWindows local ASCII paths/arguments; no shell, compiler, Git or approval automation.\nUses frozen wall/output limits; captures raw stdout/stderr, validates NDJSON.\nExit 0 validated complete, 3 limit, 2 error/invalid. Requires prior checkpoint/execution approval."); return 0;
        }
        if (!strcmp(argv[i],"--program")) target=&program_arg;
        else if (!strcmp(argv[i],"--input")) target=&input_arg;
        else if (!strcmp(argv[i],"--output")) target=&output_arg;
        else if (!strcmp(argv[i],"--checkpoint")) target=&provenance.checkpoint;
        else if (!strcmp(argv[i],"--build")) target=&provenance.build;
        else if (!strcmp(argv[i],"--toolchain")) target=&provenance.toolchain;
        if (!target || *target || i+1==argc) goto finish;
        *target=argv[++i];
    }
    if (!ascii(program_arg,GD_MAX_PATH-1) || !ascii(input_arg,GD_MAX_PATH-1) || !ascii(output_arg,GD_MAX_PATH-1) ||
        !gd_result_commit(provenance.checkpoint) || !ascii(provenance.build,1024) || !ascii(provenance.toolchain,1024)) goto finish;
    status="paths";
    if (!full_path(program_arg,program) || !full_path(input_arg,source) || !full_path(output_arg,folder) ||
        !join(copy,folder,"input.json") || !join(out_path,folder,"stdout.ndjson") || !join(err_path,folder,"stderr.log") || !join(report_path,folder,"supervision.json")) goto finish;
    input=gd_experiment_input_create(allocator);
    if (!input) { status="memory"; goto finish; }
    status="input";
    source_file=CreateFileA(source,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
    if (source_file==INVALID_HANDLE_VALUE || GetFileType(source_file)!=FILE_TYPE_DISK ||
        !ReadFile(source_file,input->raw,GD_MAX_FILE+1,&read,NULL)) {
        windows_error=GetLastError(); validation.code="supervisor.input_io"; validation.message="Cannot read declared frozen input as a local disk file"; goto finish;
    }
    if (read>GD_MAX_FILE) { validation.code="supervisor.input_limit"; validation.message="Frozen input exceeds 1 MiB"; goto finish; }
    status="output-directory";
    if (!CreateDirectoryA(folder,NULL)) goto finish;
    created=1;
    copy_file=CreateFileA(copy,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);
    if (copy_file==INVALID_HANDLE_VALUE || !write_bytes(copy_file,input->raw,read)) { status="io"; goto finish; }
    close_handle(&source_file);
    if (!gd_experiment_parse(input,input->raw,read) || !gd_result_contract(&contract,input,&validation)) { status="input"; goto finish; }
    /* Hold immutable recorded snapshot open against writes/deletion throughout
       child run and result validation; child receives this exact copy only. */
    owned.out_file=CreateFileA(out_path,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);
    owned.err_file=CreateFileA(err_path,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);
    if (copy_file==INVALID_HANDLE_VALUE || owned.out_file==INVALID_HANDLE_VALUE || owned.err_file==INVALID_HANDLE_VALUE) { status="io"; goto finish; }
    command=allocator.allocate(NULL,32768*sizeof(*command));
    if (!command) { status="memory"; goto finish; }
    {
        const char *arguments[]={program,"--input",copy,"--checkpoint",provenance.checkpoint,"--build",provenance.build,"--toolchain",provenance.toolchain};
        for (i=0;i<9;++i) if (!quote_argument(command,&used,arguments[i])) { status="arguments"; goto finish; }
    }
    status="start"; started=gd_experiment_clock();
    owned.started=started; owned.timeout=contract.wall_seconds;
    if (started<0 || !launch(&owned,program,folder,command,allocator)) {
        if (InterlockedCompareExchange(&owned.watchdog_status,0,0)==1) { status="timeout"; code=3; }
        goto finish;
    }
    launched=1; status="exited";
    for (;;) {
        int first,second; DWORD wait;
        double now=gd_experiment_clock();
        if (now<started) { status="clock"; break; }
        elapsed=now-started;
        if (elapsed>=contract.wall_seconds) { status="timeout"; break; }
        first=capture(owned.out_read,owned.out_file,&out_bytes,&total,contract.max_bytes,&out_closed);
        second=capture(owned.err_read,owned.err_file,&err_bytes,&total,contract.max_bytes,&err_closed);
        if (!first || !second) { status="io"; break; }
        if (first==2 || second==2) { status="output-limit"; break; }
        wait=WaitForSingleObject(owned.process.hProcess,0);
        if (wait==WAIT_FAILED) { status="wait"; break; }
        if (wait==WAIT_OBJECT_0 && out_closed && err_closed) break;
        Sleep(1);
    }
    {
        LONG watchdog_status=InterlockedCompareExchange(&owned.watchdog_status,0,0);
        if (watchdog_status) status=watchdog_status==1 ? "timeout" : "clock";
    }
    /* Stop owned job before parsing output, even if parent already exited.
       Descendants cannot keep running while result validation reads evidence. */
    if (!stop_job(&owned)) status="cleanup";
    else {
        LONG watchdog_status=InterlockedCompareExchange(&owned.watchdog_status,0,0);
        if (watchdog_status) status=watchdog_status==1 ? "timeout" : "clock";
    }
    if (WaitForSingleObject(owned.process.hProcess,1000)!=WAIT_OBJECT_0 || !GetExitCodeProcess(owned.process.hProcess,&process_exit)) { status="cleanup"; process_exit=2; }
    /* After stopping the job, retain buffered pipe tail within existing cap.
       No live writer remains; one-second drain bound handles OS/I/O failure. */
    {
        double drain_started=gd_experiment_clock();
        while (total<contract.max_bytes && (!out_closed || !err_closed)) {
            int first,second; double now=gd_experiment_clock();
            if (now<drain_started || drain_started<0 || now-drain_started>=1) { status="drain"; break; }
            first=capture(owned.out_read,owned.out_file,&out_bytes,&total,contract.max_bytes,&out_closed);
            second=capture(owned.err_read,owned.err_file,&err_bytes,&total,contract.max_bytes,&err_closed);
            if (!first || !second) { status="io"; break; }
            if (first==2 || second==2) break;
            if (!out_closed || !err_closed) Sleep(1);
        }
    }
    close_handle(&owned.out_file); close_handle(&owned.err_file);
    stream=fopen(out_path,"rb");
    if (stream) { gd_result_validate(stream,&contract,&provenance,(int)process_exit,strcmp(status,"exited")!=0,allocator,&validation); fclose(stream); stream=NULL; }
    else { validation.protocol_valid=0; validation.complete=0; validation.status="invalid"; validation.code="results.io"; validation.message="Cannot reopen captured stdout"; }
    code=!strcmp(status,"exited") && validation.complete ? 0 :
        !strcmp(status,"timeout") || !strcmp(status,"output-limit") || (!strcmp(status,"exited") && validation.protocol_valid && !strcmp(validation.status,"limit")) ? 3 : 2;
finish:
    if (!windows_error) windows_error=(!strcmp(status,"start") || !strcmp(status,"io") || !strcmp(status,"paths") ||
        !strcmp(status,"output-directory") || !strcmp(status,"wait")) ? GetLastError() : 0;
    close_handle(&source_file); close_handle(&copy_file);
    if (!cleanup(&owned,allocator)) { status="cleanup"; code=2; validation.complete=0; }
    if (command) allocator.release(NULL,command);
    if (created) {
        /* Directory was created by this invocation; never replace any evidence. */
        HANDLE report_file=CreateFileA(report_path,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);
        if (report_file!=INVALID_HANDLE_VALUE) {
            int descriptor=_open_osfhandle((intptr_t)report_file,_O_BINARY|_O_WRONLY);
            if (descriptor==-1) { close_handle(&report_file); stream=NULL; }
            else { stream=_fdopen(descriptor,"wb"); if (!stream) _close(descriptor); }
            if (stream) {
                summary(stream,status,code,windows_error,launched,(int)process_exit,elapsed,out_bytes,err_bytes,program,source,folder,&provenance,&validation);
                if (ferror(stream)) { status="metadata-io"; code=2; validation.complete=0; }
                if (fclose(stream)!=0) { status="metadata-io"; code=2; validation.complete=0; }
                stream=NULL;
            } else { status="metadata-io"; code=2; validation.complete=0; }
        } else { status="metadata-io"; code=2; validation.complete=0; }
    }
    summary(stdout,status,code,windows_error,launched,(int)process_exit,elapsed,out_bytes,err_bytes,*program ? program : NULL,*source ? source : NULL,*folder ? folder : NULL,&provenance,&validation);
    if (code) fprintf(stderr,"gd-experiment-run: %s; validation %s; raw evidence retained when output directory was created\n",status,validation.code ? validation.code : validation.status);
    gd_experiment_input_destroy(input);
    if (fflush(stdout)!=0 || ferror(stdout)) { fputs("gd-experiment-run: cannot deliver JSON summary on stdout\n",stderr); return 2; }
    return code;
}
#else
int main(void) {
    fputs("{\"ok\":false,\"error\":{\"code\":\"supervisor.platform\",\"message\":\"Windows process supervision only; no model executed\"}}\n",stdout); return 2;
}
#endif
