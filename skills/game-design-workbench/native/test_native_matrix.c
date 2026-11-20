/* Qualification uses actual frontend/private presentation helpers, not a
   second renderer. Default is headless; --render explicitly opens owned hidden
   matrix for three frames. Source collections stay read-only. */
#include "gd_view.c"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stddef.h>
#include <stdlib.h>
#include <shellapi.h>
#ifndef WC_ERR_INVALID_CHARS
#define WC_ERR_INVALID_CHARS 0x00000080
#endif

static void *matrix_test_allocate(void *context,size_t size){(void)context;return malloc(size);}
static void matrix_test_release(void *context,void *memory){(void)context;free(memory);}
static int matrix_check_main(int argc,char **argv) {
    GD_Allocator allocator={NULL,matrix_test_allocate,matrix_test_release};GD_Model *model=NULL;
    GD_Collection collections[3];char paths[3][GD_MAX_PATH];const char *names[]={"nouns","verbs","interactions"};
    GD_Role roles[]={GD_NOUN,GD_VERB,GD_CASE};int nouns[GD_MAX_RECORDS],verbs[GD_MAX_RECORDS];
    int render=argc==4 && !strcmp(argv[3],"--render"),checks=0,result=2,verb_count=0,noun_count=0;
    void *snapshot=NULL;size_t retained_size=offsetof(GD_Model,input);
#define MATRIX_CHECK(value) do{++checks;if(!(value)){fprintf(stderr,"Native matrix check failed: %s (line %d)\n",#value,__LINE__);goto cleanup;}}while(0)
    if((argc!=2 && !render) || (render && !argv[2][0])){fputs("Usage: test-native-matrix SOURCE-DIRECTORY [OUTPUT.png --render]\n",stderr);return 2;}
    MATRIX_CHECK(!igGetCurrentContext());MATRIX_CHECK(!GetModuleHandleW(L"raylib.dll"));
    for(int i=0;i<3;++i) {
        int length=snprintf(paths[i],sizeof(paths[i]),"%s/%s",argv[1],names[i]);
        MATRIX_CHECK(length>0 && length<(int)sizeof(paths[i]));collections[i]=(GD_Collection){names[i],paths[i],roles[i],0};
    }
    model=gd_create(allocator);MATRIX_CHECK(model);
    MATRIX_CHECK(matrix_axes(model,nouns,verbs,&verb_count)==0 && verb_count==0);
    MATRIX_CHECK(gd_load(model,collections,3) && !model->error_count);
    noun_count=matrix_axes(model,nouns,verbs,&verb_count);MATRIX_CHECK(noun_count>0 && verb_count>0 && model->cell_count>0);
    for(int i=0;i<noun_count;++i)MATRIX_CHECK(model->records[nouns[i]].role==GD_NOUN);
    for(int i=0;i<verb_count;++i)MATRIX_CHECK(model->records[verbs[i]].role==GD_VERB);
    MATRIX_CHECK(matrix_count_color((GD_CountRange){0,0},0)==matrix_panel);
    MATRIX_CHECK(matrix_count_color((GD_CountRange){2,4},2)==0xff523020u);
    MATRIX_CHECK(matrix_count_color((GD_CountRange){2,4},4)==0xff607018u);
    MATRIX_CHECK(matrix_count_color((GD_CountRange){2,2},2)==0xff59501cu);
    MATRIX_CHECK(matrix_hit((ImVec2_c){10,20},(MatrixArea){10,20,30,40}));
    MATRIX_CHECK(matrix_hit((ImVec2_c){40,60},(MatrixArea){10,20,30,40}));
    MATRIX_CHECK(!matrix_hit((ImVec2_c){40.1f,60},(MatrixArea){10,20,30,40}));
    snapshot=allocator.allocate(allocator.context,retained_size);MATRIX_CHECK(snapshot);
    memcpy(snapshot,model,retained_size);MATRIX_CHECK(!gd_sources_changed(model));
    if(render)MATRIX_CHECK(native_matrix_view(model,3,argv[2],GD_WINDOW_HIDDEN)==0);
    MATRIX_CHECK(!memcmp(snapshot,model,retained_size));
    MATRIX_CHECK(!igGetCurrentContext() && !GetModuleHandleW(L"raylib.dll"));
    printf("{\"ok\":true,\"checks\":%d,\"rendered\":%s,\"records\":%d,\"nouns\":%d,\"verbs\":%d,\"cells\":%d,\"model_retained\":true,\"human_graphical_acceptance\":false}\n",checks,render ? "true":"false",model->record_count,noun_count,verb_count,model->cell_count);
    result=0;
cleanup:
    if(snapshot)allocator.release(allocator.context,snapshot);gd_destroy(model);return result;
#undef MATRIX_CHECK
}
/* Match production CLI's strict UTF-8 Windows argument boundary. */
int main(void) {
    int count=0,result=2;wchar_t **wide=CommandLineToArgvW(GetCommandLineW(),&count);
    char storage[4][GD_MAX_PATH],*arguments[4];
    if(!wide || count<1 || count>4){fputs("Invalid native matrix test arguments\n",stderr);goto cleanup;}
    for(int i=0;i<count;++i) {
        if(!WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,wide[i],-1,storage[i],sizeof(storage[i]),NULL,NULL)) {
            fputs("Native matrix test arguments require bounded valid UTF-8\n",stderr);goto cleanup;
        }
        arguments[i]=storage[i];
    }
    result=matrix_check_main(count,arguments);
cleanup:
    if(wide)LocalFree(wide);return result;
}
