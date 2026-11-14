/* Workspace-only, source-preserving discovery. Allocate before opening handles:
 * callbacks must not allocate through fail()/longjmp while search owns handles. */
#define BROWSE_PATH_BYTES (16 * 1024 * 1024)
typedef struct workspace_browse {
    char warning[8192];
    size_t warning_used, storage_used;
    unsigned warnings, shown;
    char *storage;
    const char *root;
    attendant_document_info *documents;
    int *count;
} workspace_browse;
static void browse_warning(workspace_browse *scan, const char *path, const char *reason) {
    ++scan->warnings;
    if (!scan->warning_used) scan->warning_used = (size_t)snprintf(scan->warning, sizeof(scan->warning), "Workspace discovery incomplete:\n");
    size_t remaining = sizeof(scan->warning) - scan->warning_used;
    if (scan->shown < 32 && strlen(path) + strlen(reason) + 5 < remaining - 128) {
        scan->warning_used += (size_t)snprintf(scan->warning + scan->warning_used, remaining, "%s: %s\n", path, reason);
        ++scan->shown;
    }
}
static int browse_document(workspace_browse *scan, attendant_document_info *documents, int *count, const char *root, const char *path) {
    for (int i = 0; i < *count; ++i) if (!source_path_compare(documents[i].path, path)) return 1;
    if (*count == MAX_RECORDS) { browse_warning(scan, path, "4096-document capacity reached"); return 0; }
    size_t length = strlen(path) + 1;
    if (length > BROWSE_PATH_BYTES - scan->storage_used) { browse_warning(scan, path, "navigation path storage exhausted"); return 0; }
    char *copy = scan->storage + scan->storage_used;
    memcpy(copy, path, length); scan->storage_used += length;
    attendant_document_info *document = &documents[(*count)++];
    document->path = copy; document->name = copy + strlen(root) + (root[strlen(root) - 1] == '/' || root[strlen(root) - 1] == '\\' ? 0 : 1);
    document->collection = "(workspace)"; return 1;
}
static void browse_search_issue(void *user, const char *path, file_search_problem problem) {
    workspace_browse *scan = user;
    char reason[64];
    switch (problem) {
    case file_search_result_limit: snprintf(reason, sizeof(reason), "4096-document capacity reached"); break;
    case file_search_path: snprintf(reason, sizeof(reason), "discovery path cannot be represented"); break;
    case file_search_open: snprintf(reason, sizeof(reason), "cannot read directory"); break;
    case file_search_read: snprintf(reason, sizeof(reason), "directory enumeration failed"); break;
    case file_search_close: snprintf(reason, sizeof(reason), "directory close failed"); break;
    default: snprintf(reason, sizeof(reason), "entry unavailable"); break;
    }
    browse_warning(scan, path, reason);
}
static file_error browse_search_document(void *user, const file_tree_entry *entry) {
    workspace_browse *scan = user;
    char joined[SOURCE_PATH_CAPACITY], path[SOURCE_PATH_CAPACITY];
    int length = snprintf(joined, sizeof(joined), "%s/%s", scan->root, entry->path);
    if (length < 0 || length >= (int)sizeof(joined) || !source_full_path(joined, path)) {
        browse_warning(scan, scan->root, "discovery path cannot be represented"); return file_error_ok;
    }
    return browse_document(scan, scan->documents, scan->count, scan->root, path) ? file_error_ok : file_error_capacity;
}
static const char *browse_workspace(context *c, const char *root, attendant_document_info *documents, int *count) {
    workspace_browse *scan = allocate(c, sizeof(*scan));
    arena_result storage = arena_alloc(c->memory, BROWSE_PATH_BYTES, 1);
    if (storage.error) fail(c, "Cannot allocate bounded workspace discovery storage");
    scan->storage = storage.ptr; scan->root = root; scan->documents = documents; scan->count = count;
    /* ponytail: root Markdown + configured records only until recursive
       discovery can run without blocking the UI. Keep shared walker reusable. */
    file_search_options options = {
        .pattern = "*.md", .max_results = MAX_RECORDS, .root_only = 1,
        .skip_dot_names = 1, .skip_unsupported = 1, .issue = browse_search_issue,
    };
    /* Result paths/warnings precede temporary frames. Visitor never allocates
       from this arena, so scratch can rewind without invalidating results. */
    arena_position_marker scratch = arena_position_mark(c->memory);
    file_error error = file_directory_search(c->memory, (cstring){(char *)root}, &options, browse_search_document, scan);
    (void)arena_pop_to_marker(scratch);
    /* Search has released every handle; hard allocation failure can now recover. */
    if (error == file_error_out_of_memory) fail(c, "Cannot allocate bounded workspace discovery storage");
    if (error != file_error_ok && !scan->warnings) browse_warning(scan, root, "cannot search workspace");
    if (scan->warnings > scan->shown)
        snprintf(scan->warning + scan->warning_used, sizeof(scan->warning) - scan->warning_used, "%u additional warning(s) omitted.", scan->warnings - scan->shown);
    return scan->warnings ? scan->warning : NULL;
}
