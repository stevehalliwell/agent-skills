/* Private build-directory file only; no user preferences, source writes or OS input. */
#ifndef TEST_SIDEBAR_H
#define TEST_SIDEBAR_H
#include <limits.h>
static int viewer_sidebar_checks(void){
    GD_Sidebar sidebar,restarted;char path[GD_MAX_PATH];int checks=0,result=0,created=0,v,i;FILE *file;
#define SIDEBAR_CHECK(value) do{++checks;if(!(value)){fprintf(stderr,"Sidebar check failed: %s (line %d)\n",#value,__LINE__);goto cleanup;}}while(0)
    gd_sidebar_defaults(&sidebar);gd_sidebar_defaults(&restarted);
    SIDEBAR_CHECK(snprintf(path,sizeof(path),"%ssidebar-checks-%u.ini",GetApplicationDirectory(),(unsigned)GetRandomValue(1,INT_MAX))<(int)sizeof(path));
    SIDEBAR_CHECK(!FileExists(path));snprintf(sidebar.path,sizeof(sidebar.path),"%s",path);snprintf(restarted.path,sizeof(restarted.path),"%s",path);
    SIDEBAR_CHECK(gd_sidebar_load(&sidebar) && !*sidebar.message);
    SIDEBAR_CHECK(gd_sidebar_save(&sidebar));created=1;
    SIDEBAR_CHECK(gd_sidebar_load(&restarted) && restarted.open==sidebar.open);
    for(v=0;v<GD_EXPLORER_VIEWS;++v){
        unsigned wanted=v<GD_SIDEBAR_COUNT ? 1u<<v:(1u<<GD_SIDEBAR_COUNT)-1;sidebar.open=wanted;
        SIDEBAR_CHECK(gd_sidebar_save(&sidebar) && gd_sidebar_load(&restarted) && restarted.open==wanted);
        igGetIO_Nil()->DeltaTime=1.0f/60.0f;igNewFrame();igBegin("Sidebar folds",NULL,ImGuiWindowFlags_NoSavedSettings);
        for(i=0;i<GD_SIDEBAR_COUNT;++i){int open=sidebar_section(&restarted,(GD_SidebarSection)i);if(open!=((wanted&(1u<<i))!=0)){igEnd();igEndFrame();SIDEBAR_CHECK(0);}}
        igEnd();igEndFrame();SIDEBAR_CHECK(restarted.open==wanted && !*restarted.message);
    }
    igNewFrame();igSetNextWindowCollapsed(true,ImGuiCond_Always);igBegin("Sidebar folds",NULL,ImGuiWindowFlags_NoSavedSettings);
    for(i=0;i<GD_SIDEBAR_COUNT;++i)sidebar_section(&restarted,(GD_SidebarSection)i);
    igEnd();igEndFrame();SIDEBAR_CHECK(restarted.open==sidebar.open);
    SIDEBAR_CHECK(gd_sidebar_load(&sidebar) && sidebar.open==restarted.open); /* Hidden window must not save every section as closed. */
    {unsigned before=restarted.open;file=fopen(path,"wb");SIDEBAR_CHECK(file!=NULL);fputs("version=2\nview=0\n",file);fclose(file);SIDEBAR_CHECK(!gd_sidebar_load(&restarted) && restarted.open==before && *restarted.message);}
    file=fopen(path,"wb");SIDEBAR_CHECK(file!=NULL);fputs("version=1\nview=1\nview=0\n",file);fclose(file);
    SIDEBAR_CHECK(!gd_sidebar_load(&restarted));
    file=fopen(path,"wb");SIDEBAR_CHECK(file!=NULL);for(i=0;i<600;++i)fputc('x',file);fclose(file);SIDEBAR_CHECK(!gd_sidebar_load(&restarted));
    SIDEBAR_CHECK(gd_sidebar_save(&sidebar) && gd_sidebar_load(&restarted) && restarted.open==sidebar.open);
    snprintf(restarted.path,sizeof(restarted.path),"%s/child.ini",path);SIDEBAR_CHECK(!gd_sidebar_save(&restarted) && *restarted.message);
    snprintf(restarted.path,sizeof(restarted.path),"%s",path);SIDEBAR_CHECK(gd_sidebar_load(&restarted) && restarted.open==sidebar.open);
    result=checks;
cleanup:
    if(created)remove(path);
#undef SIDEBAR_CHECK
    return result;
}
#endif
