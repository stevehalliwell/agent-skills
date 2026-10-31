/* ImGui-only frames: no native window, renderer, OS input or source writes. */
#ifndef TEST_EXPLORER_VIEWER_FEEDBACK_H
#define TEST_EXPLORER_VIEWER_FEEDBACK_H
static int viewer_feedback_checks(ExplorerViewer *viewer) {
    ImGuiIO *io=igGetIO_Nil(); ImGuiWindow *hint,*toast,*human;
    int checks=0,result=0,had_hint=viewer->has_hint;
#define FEEDBACK_CHECK(value) do { ++checks; if (!(value)) { fprintf(stderr,"Viewer feedback check failed: %s (line %d)\n",#value,__LINE__); goto cleanup; } } while (0)
    io->DisplaySize=(ImVec2_c){800,600}; io->DeltaTime=1.0f/60.0f;
    io->BackendFlags|=ImGuiBackendFlags_RendererHasTextures; /* Font atlas stays CPU-only. */
    viewer_toast(viewer,"CLI redirected view to","Actors / states");
    igNewFrame();
    igBegin("Human context",NULL,ImGuiWindowFlags_NoSavedSettings); igTextUnformatted("Human context",NULL); igEnd();
    igSetWindowFocus_Str("Human context"); human=igGetCurrentContext()->NavWindow;
    viewer_feedback(viewer); igEndFrame();
    hint=igFindWindowByName("Agent hint"); toast=igFindWindowByName("CLI feedback");
    FEEDBACK_CHECK(hint && hint->Active && !hint->ParentWindow);
    FEEDBACK_CHECK((hint->Flags&ImGuiWindowFlags_NoFocusOnAppearing)!=0);
    FEEDBACK_CHECK(hint->Size.x==360 && hint->Size.y==160);
    FEEDBACK_CHECK(toast && toast->Active && (toast->Flags&ImGuiWindowFlags_NoInputs)==ImGuiWindowFlags_NoInputs);
    FEEDBACK_CHECK((toast->Flags&ImGuiWindowFlags_NoFocusOnAppearing)!=0);
    FEEDBACK_CHECK(human && igGetCurrentContext()->NavWindow==human);
    FEEDBACK_CHECK(viewer->has_hint==had_hint && !strcmp(viewer->hint.command,"hint"));
    io->DeltaTime=3.0f; igNewFrame();
    igBegin("Human context",NULL,ImGuiWindowFlags_NoSavedSettings); igEnd(); viewer_feedback(viewer); igEndFrame();
    FEEDBACK_CHECK(!toast->Active && hint->Active);
    viewer->human_go=1; viewer_toast(viewer,"CLI redirected view to","Actors / states"); viewer->human_go=0;
    FEEDBACK_CHECK(!*viewer->toast);
    viewer->has_hint=0; io->DeltaTime=1.0f/60.0f; igNewFrame();
    igBegin("Human context",NULL,ImGuiWindowFlags_NoSavedSettings); igEnd(); viewer_feedback(viewer); igEndFrame();
    FEEDBACK_CHECK(!hint->Active && !toast->Active);
    result=checks;
cleanup:
    viewer->has_hint=had_hint; viewer->human_go=0;
#undef FEEDBACK_CHECK
    return result;
}
#endif
