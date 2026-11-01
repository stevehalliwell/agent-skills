/* Test-only feedback failure and concurrent-edit injection. */
static void feedback_fault(context *c, const char *point) {
    char fault[128];
    DWORD length = GetEnvironmentVariableA("ATTENDANT_FEEDBACK_FAULT", fault, sizeof(fault));
    if (length && length < sizeof(fault) && !strcmp(fault, point)) fail(c, "Injected feedback failure: %s", point);
}
static void feedback_external_edit(context *c, const char *target, const char *config, int after_publication) {
    char fault[128];
    DWORD length = GetEnvironmentVariableA("ATTENDANT_FEEDBACK_FAULT", fault, sizeof(fault)), written;
    const char *path, *edit = "\n# Concurrent external edit\n";
    HANDLE file;
    if (!length || length >= sizeof(fault)) return;
    path = after_publication ? (!strcmp(fault, "published-edit") ? target : NULL) :
        !strcmp(fault, "target-change") ? target : !strcmp(fault, "config-change") ? config : NULL;
    if (!path) return;
    file = CreateFileW(wide(c, path), FILE_APPEND_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) fail(c, "Cannot inject external source edit");
    if (!WriteFile(file, edit, (DWORD)strlen(edit), &written, NULL) || written != strlen(edit)) { CloseHandle(file); fail(c, "Cannot inject external source bytes"); }
    CloseHandle(file);
    if (after_publication) fail(c, "Injected failure after external edit of published seed");
}
