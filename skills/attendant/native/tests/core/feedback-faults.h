/* Test-only feedback failure and concurrent-edit injection. */
static const char *feedback_environment(char output[128]) {
#if defined(_WIN32)
    /* Model checks change OS environment after startup; CRT getenv may cache it. */
    DWORD length = GetEnvironmentVariableA("ATTENDANT_FEEDBACK_FAULT", output, 128);
    return length && length < 128 ? output : NULL;
#else
    (void)output;
    const char *value = getenv("ATTENDANT_FEEDBACK_FAULT");
    return value && *value && strlen(value) < 128 ? value : NULL;
#endif
}
static void feedback_fault(context *c, const char *point) {
    char environment[128];
    const char *fault = feedback_environment(environment);
    if (fault && !strcmp(fault, point)) fail(c, "Injected feedback failure: %s", point);
}
static void feedback_external_edit(context *c, const char *target, const char *config, int after_publication) {
    char environment[128];
    const char *fault = feedback_environment(environment);
    const char *path, *edit = "\n# Concurrent external edit\n";
    void *file;
    if (!fault) return;
    path = after_publication ? (!strcmp(fault, "published-edit") ? target : NULL) :
        !strcmp(fault, "target-change") ? target : !strcmp(fault, "config-change") ? config : NULL;
    if (!path) return;
#if defined(_WIN32)
    file = CreateFileW(wide(c, path), FILE_APPEND_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) file = NULL;
#else
    int descriptor = open(path, O_WRONLY | O_APPEND | O_CREAT | O_NOFOLLOW | O_CLOEXEC, 0600);
    file = descriptor < 0 ? NULL : (void *)(uptr)(descriptor + 1);
#endif
    if (!file) fail(c, "Cannot inject external source edit");
    int ok = file_stream_transfer(file, (void *)edit, strlen(edit), 1);
    if (!file_stream_close(file)) ok = 0;
    if (!ok) fail(c, "Cannot inject external source bytes");
    if (after_publication) fail(c, "Injected failure after external edit of published seed");
}
