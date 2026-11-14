#include "test.h"
#include "apps/gui/workspace.h"
#include "apps/gui/navigation.h"
#include "apps/gui/find.h"

static attendant_document_info *browse_test_read(const char *root, int *count, const char **warning, const char *managed) {
    context *c = request_begin(source_test_session);
    if (setjmp(source_test_session->recovery)) return NULL;
    const char *absolute = full_path(c, ".", root);
    attendant_document_info *documents = allocate(c, MAX_RECORDS * sizeof(*documents));
    *count = 0;
    if (managed) {
        documents[0].path = full_path(c, absolute, managed); documents[0].name = "managed"; documents[0].collection = "tasks"; *count = 1;
    }
    *warning = browse_workspace(c, absolute, documents, count);
    c->recovery = NULL; return documents;
}
TEST(workspace_discovery_is_root_only_and_preserves_configured_records,
    char root[NATIVE_PATH_SIZE], path[NATIVE_PATH_SIZE], link[NATIVE_PATH_SIZE], outside[NATIVE_PATH_SIZE];
    TEST_ASSERT_FILE_OK(file_tree_join(root, source_test_case_root, "tree")); TEST_ASSERT(file_directory_create(root, 1));
    const char *directories[] = {"docs", "build", "vendor", "vendor/lib", "reference", "node_modules", "node_modules/lib", "records", "records/tasks", ".hidden", "empty"};
    for (usize i = 0; i < sizeof(directories)/sizeof(directories[0]); ++i) {
        TEST_ASSERT_FILE_OK(file_tree_join(path, root, directories[i])); TEST_ASSERT(file_directory_create(path, 1));
    }
    const char *files[] = {"README.md", "notes caf\xc3\xa9.MD", "docs/guide caf\xc3\xa9.MD", "build/generated.md", "vendor/lib/README.md", "reference/README.md", "node_modules/lib/README.md", "records/tasks/managed.md", ".hidden/no.md", ".secret.md", "docs/not.txt"};
    for (usize i = 0; i < sizeof(files)/sizeof(files[0]); ++i) {
        TEST_ASSERT_FILE_OK(file_tree_join(path, root, files[i])); TEST_ASSERT_FILE_OK(file_create_text((cstring){path}, "unchanged\r\n"));
    }
    TEST_ASSERT_FILE_OK(file_tree_join(outside, source_test_case_root, "outside")); TEST_ASSERT(file_directory_create(outside, 1));
    TEST_ASSERT_FILE_OK(file_tree_join(path, outside, "outside.md")); TEST_ASSERT_FILE_OK(file_create_text((cstring){path}, "outside"));
    TEST_ASSERT_FILE_OK(file_tree_join(link, root, "linked")); TEST_ASSERT(filesystem_test_link(link, outside));
    int count = 0; const char *warning = NULL;
    attendant_document_info *documents = browse_test_read(root, &count, &warning, "records/tasks/managed.md");
    int removed = filesystem_test_unlink(link);
    TEST_ASSERT(removed && documents && !warning && count == 3);
    TEST_ASSERT(!strcmp(documents[0].collection, "tasks"));
    int found = 0;
    for (int i = 1; i < count; ++i) {
        TEST_ASSERT(!strcmp(documents[i].collection, "(workspace)"));
        TEST_ASSERT(!strchr(documents[i].name, '/') && !strchr(documents[i].name, '\\') && documents[i].name[0] != '.');
        if (strstr(documents[i].name, "caf\xc3\xa9.MD")) ++found;
    }
    TEST_ASSERT(found == 1);
    documents = browse_test_read(root, &count, &warning, "README.md");
    TEST_ASSERT(documents && !warning && count == 2 && !strcmp(documents[0].collection, "tasks"));
    TEST_ASSERT_FILE_TEXT(((cstring){path}), "outside");
)
TEST(workspace_discovery_skips_subfolders_without_warning_and_keeps_document_limit,
    char root[NATIVE_PATH_SIZE], path[NATIVE_PATH_SIZE];
    TEST_ASSERT_FILE_OK(file_tree_join(root, source_test_case_root, "tree")); TEST_ASSERT(file_directory_create(root, 1));
    TEST_ASSERT_FILE_OK(file_tree_join(path, root, "root.md")); TEST_ASSERT_FILE_OK(file_create_text((cstring){path}, "original"));
    TEST_ASSERT_FILE_OK(file_tree_join(path, root, "a")); TEST_ASSERT(file_directory_create(path, 1));
    TEST_ASSERT_FILE_OK(file_tree_join(path, root, "a/b")); TEST_ASSERT(file_directory_create(path, 1));
    TEST_ASSERT_FILE_OK(file_tree_join(path, root, "a/b/nested.md")); TEST_ASSERT_FILE_OK(file_create_text((cstring){path}, "deep"));
    int count = 0; const char *warning = NULL;
    attendant_document_info *documents = browse_test_read(root, &count, &warning, NULL);
    TEST_ASSERT(documents && count == 1 && !warning);
    for (int i = 0; i < 48; ++i) {
        char relative[32], noise[NATIVE_PATH_SIZE]; snprintf(relative, sizeof(relative), "noise-%03d.bin", i);
        TEST_ASSERT_FILE_OK(file_tree_join(noise, root, relative)); TEST_ASSERT_FILE_OK(file_create_text((cstring){noise}, "cache"));
    }
    char added[NATIVE_PATH_SIZE];
    TEST_ASSERT_FILE_OK(file_tree_join(added, root, "added.md")); TEST_ASSERT_FILE_OK(file_create_text((cstring){added}, "new root file"));
    documents = browse_test_read(root, &count, &warning, NULL);
    TEST_ASSERT(documents && !warning && count == 2);
    TEST_ASSERT_FILE_OK(file_remove((cstring){added}));
    documents = browse_test_read(root, &count, &warning, NULL);
    TEST_ASSERT(documents && !warning && count == 1);
    context *c = request_begin(source_test_session);
    TEST_ASSERT(setjmp(source_test_session->recovery) == 0);
    workspace_browse *scan = allocate(c, sizeof(*scan));
    documents = allocate(c, MAX_RECORDS * sizeof(*documents));
    for (int i = 0; i < MAX_RECORDS; ++i) documents[i].path = "existing.md";
    count = MAX_RECORDS;
    TEST_ASSERT(!browse_document(scan, documents, &count, root, path) && count == MAX_RECORDS && scan->warnings == 1);
    c->recovery = NULL;
    TEST_ASSERT_FILE_TEXT(((cstring){path}), "deep");
)
TEST(workspace_tree_is_ordered_collapsed_and_preserves_expansion,
    gui_navigation *first = native_memory_allocate(sizeof(*first)), *second = native_memory_allocate(sizeof(*second));
    TEST_ASSERT(first && second);
    const char *paths[] = {"/project/README.md", "/project/vendor/lib/README.md", "/project/docs/z.md", "/project/docs/A.md", "/project/records/tasks/managed.md"};
    for (int i = 0; i < 5; ++i) {
        first->documents[i].path = paths[i]; first->documents[i].collection = i == 4 ? "tasks" : "(workspace)";
    }
    first->count = 5;
    int ok = gui_tree_build(first, NULL, "/project") && first->tree_count == 7 && first->tree[0].document == -1 && !first->tree[0].expanded &&
        !strncmp(first->tree[0].path + first->tree[0].name_begin, "docs/", 5) && first->tree[1].document == 3 && first->tree[2].document == 2 && first->tree[6].document == 0;
    ok = ok && gui_tree_compact_end(first, 0) == 0 && gui_tree_compact_end(first, 3) == 5 && gui_tree_compact_end(first, 6) == 6;
    first->tree[0].expanded = 1;
    second->count = first->count; memcpy(second->documents, first->documents, sizeof(first->documents));
    ok = ok && gui_tree_build(second, first, "/project") && second->tree[0].expanded && !second->tree[3].expanded;
    arena_memory_release(first, sizeof(*first)); arena_memory_release(second, sizeof(*second)); TEST_ASSERT(ok);
)
TEST(literal_find_is_bounded_nonoverlapping_case_optional_and_revision_aware,
    text_find_result hit = text_find_scan("Plan PLAN plan", 14, "plan", 0, 2);
    TEST_ASSERT(hit.count == 3 && hit.begin == 10 && hit.end == 14);
    hit = text_find_scan("Plan PLAN plan", 14, "plan", 1, 0); TEST_ASSERT(hit.count == 1 && hit.begin == 10);
    hit = text_find_scan("banana", 6, "ana", 0, 0); TEST_ASSERT(hit.count == 1 && hit.begin == 1);
    hit = text_find_scan("caf\xc3\xa9 CAF\xc3\x89", 11, "caf\xc3\xa9", 0, 0); TEST_ASSERT(hit.count == 1 && hit.end == 5);
    TEST_ASSERT(gui_find_valid("\xf0\x9f\x98\x80") && !gui_find_valid("\xc0\xaf") && !gui_find_valid("two\nlines"));
    char oversized[TEXT_FIND_QUERY + 1]; memset(oversized, 'x', sizeof(oversized)); oversized[TEXT_FIND_QUERY] = 0;
    TEST_ASSERT(!gui_find_valid(oversized));
    gui_find find = {0}; gui_tab tab = {0}; tab.text = "Plan PLAN plan"; tab.length = 14; tab.generation = 1; tab.text_revision = 1;
    memcpy(find.query, "plan", 5); gui_find_source(&find, &tab, 0, 0);
    TEST_ASSERT(find.count == 3 && find.current == 1 && find.begin == 0 && find.apply);
    find.action = -1; gui_find_source(&find, &tab, 0, 0); TEST_ASSERT(find.current == 3 && find.begin == 10);
    find.action = 1; gui_find_source(&find, &tab, 0, 0); TEST_ASSERT(find.current == 1);
    gui_find_source(&find, &tab, 0, 1); TEST_ASSERT(find.count == 1 && find.begin == 10);
    tab.text = "no hits"; tab.length = 7; ++tab.text_revision; gui_find_source(&find, &tab, 0, 1);
    TEST_ASSERT(!find.count && !find.current && !find.apply);
    find.query[0] = 0; gui_find_source(&find, &tab, 0, 0); TEST_ASSERT(!find.count);
    char repetitive[8192]; memset(repetitive, 'a', sizeof(repetitive));
    hit = text_find_scan(repetitive, sizeof(repetitive), "aaaaaaaaab", 0, 0); TEST_ASSERT(!hit.count);
)
