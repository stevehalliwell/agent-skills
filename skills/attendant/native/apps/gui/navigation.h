/* Stable preorder tree over navigation-owned paths. No filesystem operations. */
#ifndef ATTENDANT_NAVIGATION_H
#define ATTENDANT_NAVIGATION_H
static int gui_path_separator(char c) { return c == '/' || c == '\\'; }
static unsigned gui_ascii_fold(unsigned c) { return c >= 'A' && c <= 'Z' ? c + ('a' - 'A') : c; }
static int gui_tree_order(const char *a, int al, int ad, const char *b, int bl, int bd) {
    int ai = 0, bi = 0;
    while (ai < al && bi < bl) {
        int ae = ai, be = bi;
        while (ae < al && !gui_path_separator(a[ae])) ++ae;
        while (be < bl && !gui_path_separator(b[be])) ++be;
        int af = ae < al || ad, bf = be < bl || bd;
        if (af != bf) return af ? -1 : 1;
        int n = ae - ai < be - bi ? ae - ai : be - bi, order = 0;
        for (int i = 0; i < n; ++i) {
            unsigned ac = gui_ascii_fold((unsigned char)a[ai + i]), bc = gui_ascii_fold((unsigned char)b[bi + i]);
            if (ac != bc) { order = ac < bc ? -1 : 1; break; }
        }
        if (!order && ae - ai != be - bi) order = ae - ai < be - bi ? -1 : 1;
        if (!order) order = memcmp(a + ai, b + bi, (size_t)n);
        if (order) return order;
        if (ae == al || be == bl) return ae == al ? (be == bl ? 0 : -1) : 1;
        ai = ae + 1; bi = be + 1;
    }
    return (ai < al) - (bi < bl);
}
typedef struct gui_tree_file { const char *path; int document; } gui_tree_file;
static int gui_tree_file_order(const void *a, const void *b) {
    const gui_tree_file *left = a, *right = b;
    return gui_tree_order(left->path, (int)strlen(left->path), 0, right->path, (int)strlen(right->path), 0);
}
static int gui_tree_was_expanded(const gui_navigation *old, const char *path, int length) {
    if (!old) return 0;
    int low = 0, high = old->tree_count;
    while (low < high) {
        int mid = low + (high - low) / 2;
        const gui_tree_node *node = &old->tree[mid];
        int order = gui_tree_order(node->path, node->length, node->document < 0, path, length, 1);
        if (order < 0) low = mid + 1; else high = mid;
    }
    return low < old->tree_count && old->tree[low].document < 0 && old->tree[low].length == length &&
        !memcmp(old->tree[low].path, path, (size_t)length) && old->tree[low].expanded;
}
/* Collapse only real single-child chains, never branches hidden by a filter. */
static int gui_tree_compact_end(const gui_navigation *navigation, int first) {
    int at = first;
    while (navigation->tree[at].document < 0) {
        int child = at + 1;
        if (child >= navigation->tree[at].end || navigation->tree[child].parent != at ||
            navigation->tree[child].end != navigation->tree[at].end) break;
        at = child;
    }
    return at;
}
static int gui_tree_build(gui_navigation *navigation, const gui_navigation *old, const char *root) {
    gui_tree_file files[GUI_DOCUMENTS]; int count = 0, stack[32], depth = 0;
    size_t root_length = strlen(root);
    int begin = (int)root_length + (gui_path_separator(root[root_length - 1]) ? 0 : 1);
    for (int i = 0; i < navigation->count; ++i)
        if (!strcmp(navigation->documents[i].collection, "(workspace)")) {
            files[count].path = navigation->documents[i].path; files[count++].document = i;
        }
    qsort(files, (size_t)count, sizeof(*files), gui_tree_file_order);
    for (int i = 0; i < count; ++i) {
        const char *path = files[i].path; int ends[32], starts[32], next_depth = 0, start = begin, shared = 0;
        int length = (int)strlen(path);
        for (int at = begin; at < length; ++at) if (gui_path_separator(path[at])) {
            if (next_depth == 32) return 0;
            starts[next_depth] = start; ends[next_depth++] = at; start = at + 1;
        }
        while (shared < depth && shared < next_depth && navigation->tree[stack[shared]].length == ends[shared] &&
            !memcmp(navigation->tree[stack[shared]].path, path, (size_t)ends[shared])) ++shared;
        while (depth > shared) navigation->tree[stack[--depth]].end = navigation->tree_count;
        while (depth < next_depth) {
            if (navigation->tree_count == GUI_TREE_NODES) return 0;
            int index = navigation->tree_count++; gui_tree_node *node = &navigation->tree[index];
            node->path = path; node->length = ends[depth]; node->name_begin = starts[depth]; node->document = -1;
            node->parent = depth ? stack[depth - 1] : -1;
            node->expanded = gui_tree_was_expanded(old, path, ends[depth]); stack[depth++] = index;
        }
        if (navigation->tree_count == GUI_TREE_NODES) return 0;
        gui_tree_node *file = &navigation->tree[navigation->tree_count++];
        file->path = path; file->length = length; file->name_begin = start; file->document = files[i].document;
        file->parent = depth ? stack[depth - 1] : -1; file->end = navigation->tree_count;
    }
    while (depth) navigation->tree[stack[--depth]].end = navigation->tree_count;
    return 1;
}
#endif
