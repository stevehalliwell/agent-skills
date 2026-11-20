/* ImGui-only frames: no native window, renderer, OS input or source writes. */
#ifndef TEST_EXPLORER_VIEWER_FEEDBACK_H
#define TEST_EXPLORER_VIEWER_FEEDBACK_H
static int viewer_feedback_checks(ExplorerViewer *viewer) {
    ImGuiIO *io=igGetIO_Nil();ImGuiWindow *hint,*toast,*human;double start,until;float style=igGetStyle()->Alpha;
    int checks=0,result=0,had_hint=viewer->has_hint;
#define FEEDBACK_CHECK(value) do { ++checks; if (!(value)) { fprintf(stderr,"Viewer feedback check failed: %s (line %d)\n",#value,__LINE__); goto cleanup; } } while (0)
    io->DisplaySize=(ImVec2_c){800,600};io->DeltaTime=1.0f/60.0f;
    io->BackendFlags|=ImGuiBackendFlags_RendererHasTextures; /* Font atlas stays CPU-only. */
    *viewer->toast=0;viewer_toast(viewer,"CLI redirected view to","Actors / states");start=viewer->toast_started;
    FEEDBACK_CHECK(viewer->toast_until-start==6 && viewer_toast_alpha(viewer,start)==0);
    FEEDBACK_CHECK(fabsf(viewer_toast_alpha(viewer,start+0.1)-0.5f)<0.0001f);
    FEEDBACK_CHECK(viewer_toast_alpha(viewer,start+0.2)==1 && viewer_toast_alpha(viewer,start+3)==1);
    FEEDBACK_CHECK(fabsf(viewer_toast_alpha(viewer,start+5.7)-1)<0.0001f);
    FEEDBACK_CHECK(fabsf(viewer_toast_alpha(viewer,start+5.85)-0.5f)<0.0001f);
    FEEDBACK_CHECK(viewer_toast_alpha(viewer,start+6)==0 && viewer_toast_alpha(viewer,start+7)==0);
    FEEDBACK_CHECK(viewer_toast_alpha(viewer,start-1)==0 && viewer_toast_alpha(viewer,NAN)==0);
    igNewFrame();
    igBegin("Human context",NULL,ImGuiWindowFlags_NoSavedSettings);igTextUnformatted("Human context",NULL);igEnd();
    igSetWindowFocus_Str("Human context");human=igGetCurrentContext()->NavWindow;
    viewer_feedback(viewer);igEndFrame();
    hint=igFindWindowByName("Agent hint");toast=igFindWindowByName("CLI feedback");
    FEEDBACK_CHECK(hint && hint->Active && !hint->ParentWindow);
    FEEDBACK_CHECK((hint->Flags&ImGuiWindowFlags_NoFocusOnAppearing)!=0);
    FEEDBACK_CHECK(hint->Size.x==360 && hint->Size.y==160);
    FEEDBACK_CHECK(toast && toast->Active && (toast->Flags&ImGuiWindowFlags_NoInputs)==ImGuiWindowFlags_NoInputs);
    FEEDBACK_CHECK((toast->Flags&ImGuiWindowFlags_NoFocusOnAppearing)!=0);
    FEEDBACK_CHECK(human && igGetCurrentContext()->NavWindow==human);
    FEEDBACK_CHECK(viewer->has_hint==had_hint && !strcmp(viewer->hint.command,"hint") && igGetStyle()->Alpha==style);
    io->DeltaTime=3.0f;igNewFrame();
    igBegin("Human context",NULL,ImGuiWindowFlags_NoSavedSettings);igEnd();viewer_feedback(viewer);igEndFrame();
    FEEDBACK_CHECK(toast->Active && hint->Active); /* Regression: former 2.5s toast already vanished. */
    until=viewer->toast_until;viewer_toast(viewer,"CLI action in","MDA");
    FEEDBACK_CHECK(!strcmp(viewer->toast,"CLI action in MDA") && viewer->toast_until>until && viewer_toast_alpha(viewer,igGetTime())==1);
    FEEDBACK_CHECK(viewer_toast_alpha(viewer,igGetTime()+0.1)==1); /* No repeated entrance. */
    io->DeltaTime=5.85f;igNewFrame();
    igBegin("Human context",NULL,ImGuiWindowFlags_NoSavedSettings);igEnd();viewer_feedback(viewer);igEndFrame();
    {
        float alpha=viewer_toast_alpha(viewer,igGetTime());int i,maximum=0;
        FEEDBACK_CHECK(fabsf(alpha-0.5f)<0.0001f && toast->Active && igGetStyle()->Alpha==style);
        for(i=0;i<toast->DrawList->VtxBuffer.Size;++i){int value=(int)(toast->DrawList->VtxBuffer.Data[i].col>>24);if(value>maximum)maximum=value;}
        FEEDBACK_CHECK(maximum>0 && maximum<=ceilf(255*alpha*style)+1); /* Background/text vertices fade, not just window fill. */
        viewer_toast(viewer,"CLI configured controls in","Resources");
        FEEDBACK_CHECK(fabsf(viewer_toast_alpha(viewer,igGetTime())-alpha)<0.0001f);
        FEEDBACK_CHECK(fabsf(viewer_toast_alpha(viewer,igGetTime()+0.1)-(alpha+(1-alpha)*0.5f))<0.0001f);
        FEEDBACK_CHECK(viewer_toast_alpha(viewer,igGetTime()+0.2)==1);
    }
    io->DeltaTime=6.1f;igNewFrame();
    igBegin("Human context",NULL,ImGuiWindowFlags_NoSavedSettings);igEnd();viewer_feedback(viewer);igEndFrame();
    FEEDBACK_CHECK(!toast->Active && hint->Active && igGetCurrentContext()->NavWindow==human && igGetStyle()->Alpha==style);
    viewer->human_go=1;viewer_toast(viewer,"CLI redirected view to","Actors / states");viewer->human_go=0;
    FEEDBACK_CHECK(!*viewer->toast && viewer_toast_alpha(viewer,igGetTime())==0);
    viewer->has_hint=0;io->DeltaTime=1.0f/60.0f;igNewFrame();
    igBegin("Human context",NULL,ImGuiWindowFlags_NoSavedSettings);igEnd();viewer_feedback(viewer);igEndFrame();
    FEEDBACK_CHECK(!hint->Active && !toast->Active);
    result=checks;
cleanup:
    viewer->has_hint=had_hint;viewer->human_go=0;igGetStyle()->Alpha=style;io->DeltaTime=1.0f/60.0f;
#undef FEEDBACK_CHECK
    return result;
}
#endif
