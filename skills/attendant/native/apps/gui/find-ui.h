/* Included after gui_app. All input-state exchanges are balanced in this frame. */
static void find_close(gui_app *app, int target) {
    gui_find *find = &app->find[target];
    if (target == GUI_FIND_SIDEBAR) { find->query[0] = 0; find->error = 0; find->cached = 0; find->open = find->focus = 0; }
    else {
        find->open = 0;
        if (target == GUI_FIND_SOURCE && app->workspace.active == find->tab && find->count) {
            gui_tab *tab = &app->workspace.tabs[find->tab];
            if (tab->generation == find->generation && tab->text_revision == find->revision && find->begin < find->end && find->end <= tab->length) {
                tab->selection_start = (int)find->begin; tab->selection_end = (int)find->end; tab->reveal_selection = 1;
            }
        }
    }
    find->apply = find->action = 0;
    app->find_return = target; igClearActiveID();
}
static void find_keys(gui_app *app) {
    if (app->composer_active || app->pending_action != GUI_ACTION_NONE) return;
    int target = app->workspace.active < 0 ? GUI_FIND_SIDEBAR : app->find_target < 0 ? GUI_FIND_SOURCE : app->find_target;
    if (app->workspace.active < 0 && app->find_return != GUI_FIND_SIDEBAR) app->find_return = -1;
    if (igGetIO_Nil()->KeyCtrl && igIsKeyPressed_Bool(ImGuiKey_F, false)) {
        app->find_target = target; app->find[target].open = app->find[target].focus = 1; app->find[target].action = 2;
    }
    if (target != GUI_FIND_SIDEBAR && igIsKeyPressed_Bool(ImGuiKey_F3, true)) {
        app->find[target].open = 1; app->find[target].action = igGetIO_Nil()->KeyShift ? -1 : 1;
    }
    if (igIsKeyPressed_Bool(ImGuiKey_Escape, false) &&
        (app->find[target].open || (target == GUI_FIND_SIDEBAR && *app->find[0].query))) {
        igSetKeyOwner(ImGuiKey_Escape, igGetID_Str("find-escape"), ImGuiInputFlags_LockThisFrame);
        find_close(app, target);
    }
}
static void apply_agent_search(gui_app *app) {
    if (!app->search_ready) return;
    app->search_ready = 0;
    gui_search *request = &app->pending_search;
    if (app->composer_active || app->pending_action != GUI_ACTION_NONE || app->close_all) {
        snprintf(app->workspace.error, sizeof(app->workspace.error), "Agent search not applied while dialog/close-all is active"); return;
    }
    if (*request->path && (!gui_reveal_ordinary_path(request->path) || !gui_open_document(&app->workspace, request->path))) {
        if (!*app->workspace.error) snprintf(app->workspace.error, sizeof(app->workspace.error), "Agent search file is unavailable or has link ancestry");
        return;
    }
    if (request->view && app->workspace.active < 0) {
        snprintf(app->workspace.error, sizeof(app->workspace.error), "Agent search needs an open document"); return;
    }
    igClearActiveID();
    gui_find *find = &app->find[request->view];
    memcpy(find->query, request->query, strlen(request->query) + 1);
    find->open = find->focus = 1; find->action = 2; find->apply = find->error = 0;
    app->find_target = (int)request->view; app->find_match_case = request->match_case != 0;
    for (int i = 0; i < 3; ++i) app->find[i].cached = 0;
#ifdef ATTENDANT_GUI_TESTING
    if (app->selection_exercise == 17) app->search_event_pending = 1;
#endif
}
#ifdef ATTENDANT_GUI_TESTING
static void search_observe(gui_app *app, int target) {
    gui_find *find = &app->find[target];
    if (app->selection_exercise != 17 || !app->search_event_pending || app->find_target != target ||
        find->error || (target != GUI_FIND_PREVIEW && !find->cached) || (target == GUI_FIND_SOURCE && find->apply) ||
        (target != GUI_FIND_SIDEBAR && find->tab != app->workspace.active)) return;
    app->search_event_pending = 0;
    printf("{\"event\":\"search-applied\",\"serial\":%d,\"view\":%d,\"count\":%d}\n", ++app->search_event_serial, target, find->count); fflush(stdout);
}
#endif
static int find_character(ImGuiInputTextCallbackData *data) {
    gui_find *find = data->UserData;
    ImGuiInputTextState *state = igGetInputTextState(igGetActiveID());
    int bytes = data->EventChar < 0x80 ? 1 : data->EventChar < 0x800 ? 2 : data->EventChar < 0x10000 ? 3 : 4;
    int selected = state ? ImGuiInputTextState_GetSelectionEnd(state) - ImGuiInputTextState_GetSelectionStart(state) : 0;
    if (selected < 0) selected = -selected;
    if (data->EventChar == '\n' || data->EventChar == '\r' || (state && state->TextLen - selected + bytes >= TEXT_FIND_QUERY)) {
        find->rejected = 1; return 1;
    }
    return 0;
}
static void find_bar(gui_app *app, int target) {
    gui_find *find = &app->find[target];
    if (!find->open && !find->focus) return;
    find->open = 1;
    igPushID_Int(target);
    if (!find->input_state) find->input_state = crmd_input_state_create();
    if (!find->input_state) { igTextUnformatted("Cannot allocate find input state", NULL); igPopID(); return; }
    crmd_input_state_exchange(find->input_state);
    find->rejected = 0;
    bool requested_focus = find->focus != 0;
    if (find->focus) { igSetKeyboardFocusHere(0); find->focus = 0; }
    ImGuiID id = igGetID_Str("##query");
    ImGuiIO *io = igGetIO_Nil();
    ImGuiKey paste = (io->ConfigMacOSXBehaviors ? io->KeySuper : io->KeyCtrl) && igIsKeyPressed_Bool(ImGuiKey_V, false) ? ImGuiKey_V :
        io->KeyShift && igIsKeyPressed_Bool(ImGuiKey_Insert, false) ? ImGuiKey_Insert : ImGuiKey_None;
    if ((igGetActiveID() == id || requested_focus) && paste != ImGuiKey_None) {
        const char *clipboard = igGetClipboardText();
        ImGuiInputTextState *state = igGetInputTextState(id);
        int length = clipboard ? text_find_query_length(clipboard) : 0;
        int selected = state ? ImGuiInputTextState_GetSelectionEnd(state) - ImGuiInputTextState_GetSelectionStart(state) : 0;
        if (selected < 0) selected = -selected;
        if (length < 0 || (clipboard && !gui_find_valid(clipboard)) || (state && state->TextLen - selected + length >= TEXT_FIND_QUERY)) {
            find->rejected = 1;
            igSetKeyOwner(paste, igGetID_Str("rejected-paste"), ImGuiInputFlags_LockThisFrame);
        }
    }
    bool was_active = igGetActiveID() == id;
    igTextUnformatted(target == GUI_FIND_SIDEBAR ? "Filter files" : target == GUI_FIND_SOURCE ? "Find source" : "Find preview", NULL);
    igSetNextItemWidth(-1);
    bool changed = igInputText("##query", find->query, sizeof(find->query), ImGuiInputTextFlags_CallbackCharFilter | ImGuiInputTextFlags_AllowTabInput, find_character, find);
    bool active = igIsItemActive();
    if (active || was_active) {
        app->find_target = target;
        if (target != GUI_FIND_SIDEBAR && (igIsKeyPressed_Bool(ImGuiKey_Enter, false) || igIsKeyPressed_Bool(ImGuiKey_KeypadEnter, false))) {
            find->action = igGetIO_Nil()->KeyShift ? -1 : 1;
            find->focus = 1; /* Single-line InputText validates/deactivates on Enter. */
        }
    }
    crmd_input_state_exchange(find->input_state);
    if (changed) { find->error = 0; find->cached = 0; }
    if (find->rejected || !gui_find_valid(find->query)) { find->error = 1; find->cached = 0; find->count = find->current = find->apply = 0; }
    if (igCheckbox("Match case", &app->find_match_case)) for (int i = 0; i < 3; ++i) app->find[i].cached = 0;
    igSameLine(0, -1);
    if (igSmallButton("Clear")) {
        find->query[0] = 0; find->error = 0; find->cached = 0;
        if (target == GUI_FIND_SIDEBAR) find_close(app, target);
    }
#ifdef ATTENDANT_GUI_TESTING
    if (target == GUI_FIND_SIDEBAR) {
        ImVec2 a = igGetItemRectMin(), b = igGetItemRectMax();
        app->sidebar_clear_position = (ImVec2){(a.x + b.x) * 0.5f, (a.y + b.y) * 0.5f};
    }
#endif
    if (target != GUI_FIND_SIDEBAR) {
        igSameLine(0, -1); if (igSmallButton("Close")) find_close(app, target);
        igSameLine(0, -1); igText("%d/%d", find->current, find->count);
    }
    if (find->error) igTextUnformatted(find->error == 2 ? "Preview find unavailable: capacity exceeded. Query not searched." : "Use one UTF-8 line, at most 1023 bytes. Query not searched.", NULL);
    igPopID();
}
static void navigation_filter(gui_app *app) {
    gui_navigation *navigation = app->workspace.navigation; gui_find *find = &app->find[GUI_FIND_SIDEBAR];
    if (find->cached && app->filtered_navigation == navigation && find->generation == app->feedback_filter &&
        find->match_case == app->find_match_case && !strcmp(find->query, find->applied)) return;
    memset(app->tree_visible, 0, sizeof(app->tree_visible));
    for (int i = 0; i < navigation->count; ++i) {
        attendant_document_info *document = &navigation->documents[i];
        int feedback = !strcmp(document->collection, "_meta_comments");
        const char *relative = document->path;
        size_t root_length = strlen(app->workspace.root);
        if (!strncmp(relative, app->workspace.root, root_length)) { relative += root_length; if (*relative == '/' || *relative == '\\') ++relative; }
        app->document_visible[i] = !find->error &&
            (!feedback || !app->feedback_filter || (app->feedback_filter == 1 ? document->pending : document->resolved)) &&
            (gui_find_contains(document->name, find->query, app->find_match_case) || gui_find_contains(relative, find->query, app->find_match_case));
    }
    for (int i = navigation->tree_count; i-- > 0;) {
        gui_tree_node *node = &navigation->tree[i];
        if (node->document >= 0) app->tree_visible[i] = app->document_visible[node->document];
        if (app->tree_visible[i] && node->parent >= 0) app->tree_visible[node->parent] = 1;
    }
    app->filtered_navigation = navigation; find->generation = app->feedback_filter; find->match_case = app->find_match_case;
    memcpy(find->applied, find->query, strlen(find->query) + 1); find->cached = 1;
}
static bool navigation_group(gui_app *app, const char *name) {
    ImGuiID id = igGetID_Str(name); int i;
    for (i = 0; i < app->group_count; ++i) if (app->groups[i].id == id) break;
    if (i == app->group_count && i < 33) { app->groups[i].id = id; app->groups[i].open = 1; ++app->group_count; }
    int filtering = *app->find[GUI_FIND_SIDEBAR].query != 0;
    igSetNextItemOpen(filtering || i == 33 || app->groups[i].open, ImGuiCond_Always);
    bool open = igCollapsingHeader_TreeNodeFlags(name, ImGuiTreeNodeFlags_DefaultOpen);
    if (!filtering && i < 33) app->groups[i].open = open;
    return open;
}
