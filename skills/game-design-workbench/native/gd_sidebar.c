#include "gd_sidebar.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
/* CJIT headers predate this Windows API constant; retain strict UTF-8 conversion. */
#ifndef WC_ERR_INVALID_CHARS
#define WC_ERR_INVALID_CHARS 0x00000080
#endif
#include <wchar.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif
const char *const gd_sidebar_labels[GD_SIDEBAR_COUNT]={"View & layout","Find selected in another view","Display & filters","Analysis","Browse shown nodes","Selection & evidence","Sources & changes","Diagnostics"};
enum { GD_SIDEBAR_FILE_LIMIT=512 };
static const char *const keys[GD_SIDEBAR_COUNT]={"view","find","filters","analysis","browse","selection","sources","diagnostics"};
void gd_sidebar_defaults(GD_Sidebar *s){memset(s,0,sizeof(*s));s->open=(1u<<GD_SIDEBAR_VIEW)|(1u<<GD_SIDEBAR_FILTERS)|(1u<<GD_SIDEBAR_ANALYSIS)|(1u<<GD_SIDEBAR_SELECTION);}
#ifdef _WIN32
static int wide_path(const char *path,wchar_t *wide){return MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,path,-1,wide,GD_MAX_PATH)!=0;}
static FILE *open_file(const char *path,int writing){wchar_t wide[GD_MAX_PATH];return wide_path(path,wide)?_wfopen(wide,writing?L"wb":L"rb"):NULL;}
static int make_directory(const char *path){wchar_t wide[GD_MAX_PATH];DWORD attributes;if(!wide_path(path,wide))return 0;if(CreateDirectoryW(wide,NULL))return 1;attributes=GetFileAttributesW(wide);return attributes!=INVALID_FILE_ATTRIBUTES && (attributes&FILE_ATTRIBUTE_DIRECTORY);}
static int replace_file(const char *from,const char *to){wchar_t a[GD_MAX_PATH],b[GD_MAX_PATH];return wide_path(from,a)&&wide_path(to,b)&&MoveFileExW(a,b,MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH);}
static void remove_file(const char *path){wchar_t wide[GD_MAX_PATH];if(wide_path(path,wide))DeleteFileW(wide);}
#else
static FILE *open_file(const char *path,int writing){return fopen(path,writing?"wb":"rb");}
static int make_directory(const char *path){struct stat status;if(!mkdir(path,0700))return 1;return !stat(path,&status)&&S_ISDIR(status.st_mode);}
static int replace_file(const char *from,const char *to){return rename(from,to)==0;}
static void remove_file(const char *path){remove(path);}
#endif
void gd_sidebar_init(GD_Sidebar *s){
    char base[GD_MAX_PATH];int length=-1;gd_sidebar_defaults(s);*base=0;
#ifdef _WIN32
    {wchar_t wide[GD_MAX_PATH];DWORD n=GetEnvironmentVariableW(L"LOCALAPPDATA",wide,GD_MAX_PATH);if(n && n<GD_MAX_PATH && !WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,wide,-1,base,sizeof(base),NULL,NULL))*base=0;}
    if(*base)length=snprintf(s->path,sizeof(s->path),"%s/game-design-workbench/sidebar-folds.ini",base);
#elif defined(__APPLE__)
    {const char *home=getenv("HOME");if(home && *home=='/')length=snprintf(s->path,sizeof(s->path),"%s/Library/Application Support/game-design-workbench/sidebar-folds.ini",home);}
#else
    {const char *config=getenv("XDG_CONFIG_HOME"),*home=getenv("HOME");if(config && *config=='/')length=snprintf(s->path,sizeof(s->path),"%s/game-design-workbench/sidebar-folds.ini",config);else if(home && *home=='/')length=snprintf(s->path,sizeof(s->path),"%s/.config/game-design-workbench/sidebar-folds.ini",home);}
#endif
    if(length<0 || (size_t)length>=sizeof(s->path)){*s->path=0;snprintf(s->message,sizeof(s->message),"UI fold settings path unavailable; preferences remembered for this session only.");return;}
    {char *p;for(p=s->path;*p;++p)if(*p=='\\')*p='/';}
    gd_sidebar_load(s);
}
int gd_sidebar_load(GD_Sidebar *s){
    char text[GD_SIDEBAR_FILE_LIMIT+1],*line;size_t size;unsigned bits=s->open,seen=0;int version=0,ok,i;FILE *file;
    if(!*s->path)return 0;errno=0;file=open_file(s->path,0);
    if(!file){if(errno==ENOENT){*s->message=0;return 1;}goto failed;}
    size=fread(text,1,sizeof(text),file);ok=!ferror(file)&&size<=GD_SIDEBAR_FILE_LIMIT;if(fclose(file))ok=0;
    if(!ok || memchr(text,0,size))goto failed;text[size]=0;line=text;
    while(*line){
        char *end=strchr(line,'\n');size_t length=end?(size_t)(end-line):strlen(line);int found=0;
        if(length && line[length-1]=='\r')--length;
        if(length==9 && !memcmp(line,"version=1",9)){if(version)goto failed;version=1;found=1;}
        for(i=0;i<GD_SIDEBAR_COUNT;++i){size_t n=strlen(keys[i]);if(length==n+2 && !memcmp(line,keys[i],n) && line[n]=='=' && (line[n+1]=='0'||line[n+1]=='1')){if(seen&(1u<<i))goto failed;seen|=1u<<i;if(line[n+1]=='1')bits|=1u<<i;else bits&=~(1u<<i);found=1;break;}}
        if(length && !found)goto failed;if(!end)break;line=end+1;
    }
    if(!version)goto failed;s->open=bits;*s->message=0;return 1;
failed:
    snprintf(s->message,sizeof(s->message),"Cannot read UI fold settings; session preferences retained. No project data affected.");return 0;
}
int gd_sidebar_save(GD_Sidebar *s){
    char directory[GD_MAX_PATH],temporary[GD_MAX_PATH],*slash;FILE *file;int i,length,ok;unsigned long process;
    if(!*s->path)return 0;
#ifdef _WIN32
    process=(unsigned long)GetCurrentProcessId();
#else
    process=(unsigned long)getpid();
#endif
    length=snprintf(temporary,sizeof(temporary),"%s.%lu.tmp",s->path,process);if(length<0 || (size_t)length>=sizeof(temporary))goto failed;
    snprintf(directory,sizeof(directory),"%s",s->path);for(slash=directory;*slash;++slash)if(*slash=='\\')*slash='/';
    slash=strrchr(directory,'/');if(!slash)goto failed;
    if(slash==directory || (slash==directory+2 && directory[1]==':'))slash[1]=0;else *slash=0;
    /* Bounded path walk; create UI settings directories only, never source folders. */
    for(slash=directory+1;*slash;++slash)if(*slash=='/' && slash[-1]!=':'){*slash=0;ok=make_directory(directory);*slash='/';if(!ok)goto failed;}
    if(!make_directory(directory))goto failed;
    file=open_file(temporary,1);if(!file)goto failed;
    ok=fputs("version=1\n",file)>=0;for(i=0;i<GD_SIDEBAR_COUNT && ok;++i)ok=fprintf(file,"%s=%u\n",keys[i],(s->open>>i)&1u)>0;
    if(fflush(file))ok=0;if(fclose(file))ok=0;
    if(!ok || !replace_file(temporary,s->path)){remove_file(temporary);goto failed;}
    *s->message=0;return 1;
failed:
    snprintf(s->message,sizeof(s->message),"Cannot save UI fold settings; preferences retained for this session. No project data affected.");return 0;
}
