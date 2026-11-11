/* Project-local bounded command JSON. Include after native-api.c for numeric
   comparison. No fixture hooks or core/GUI suite completion policy. */
#include "vendor/libpdjson5/pdjson5.h"

typedef struct integration_json_node {
    enum pdjson_type type;
    const char *name, *text;
    int first, last, next, count;
} integration_json_node;
typedef struct integration_json_document {
    integration_json_node nodes[4096];
    int count;
    arena *memory;
} integration_json_document;
static u8 integration_json_memory[2][2097152];
static integration_json_document integration_documents[2];
static void *integration_json_allocate(size_t size, void *opaque) {
    if (size > SIZE_MAX - 16) return NULL;
    arena_result allocation = arena_alloc(opaque, size + 16, 16);
    if (allocation.error != arena_error_ok) return NULL;
    *(size_t *)allocation.ptr = size;
    return (u8 *)allocation.ptr + 16;
}
static void *integration_json_reallocate(void *old, size_t size, void *opaque) {
    void *next = integration_json_allocate(size, opaque);
    if (next && old) { size_t prior = *(size_t *)((u8 *)old - 16); memcpy(next, old, prior < size ? prior : size); }
    return next;
}
static void integration_json_release(void *memory, size_t size, void *opaque) { (void)memory; (void)size; (void)opaque; }
static const char *integration_json_copy(integration_json_document *document, const char *text, usize size) {
    if (!text || !size || text[size - 1] || memchr(text, 0, size - 1)) return NULL;
    arena_result allocation = arena_alloc(document->memory, size, 1);
    if (allocation.error != arena_error_ok) return NULL;
    memcpy(allocation.ptr, text, size); return allocation.ptr;
}
static int integration_json_value(integration_json_document *document, pdjson_stream *parser, unsigned depth, const char *name) {
    if (depth > 16 || document->count == 4096) return -1;
    int index = document->count++;
    integration_json_node *node = &document->nodes[index];
    *node = (integration_json_node){pdjson_next(parser), name, NULL, -1, -1, -1, 0};
    if (node->type == PDJSON_OBJECT || node->type == PDJSON_ARRAY) {
        enum pdjson_type ending = node->type == PDJSON_OBJECT ? PDJSON_OBJECT_END : PDJSON_ARRAY_END;
        while (pdjson_peek(parser) != ending) {
            const char *key = NULL;
            if (node->type == PDJSON_OBJECT) {
                if (pdjson_next(parser) != PDJSON_NAME) return -1;
                size_t size; const char *text = pdjson_get_name(parser, &size);
                key = integration_json_copy(document, text, size); if (!key) return -1;
                for (int prior = node->first; prior >= 0; prior = document->nodes[prior].next)
                    if (!strcmp(key, document->nodes[prior].name)) return -1;
            }
            int child = integration_json_value(document, parser, depth + 1, key);
            if (child < 0) return -1;
            if (node->last >= 0) document->nodes[node->last].next = child; else node->first = child;
            node->last = child; ++node->count;
        }
        if (pdjson_next(parser) != ending) return -1;
    } else if (node->type == PDJSON_STRING || node->type == PDJSON_NUMBER) {
        size_t size; const char *text = pdjson_get_value(parser, &size);
        node->text = integration_json_copy(document, text, size); if (!node->text) return -1;
    } else if (node->type != PDJSON_TRUE && node->type != PDJSON_FALSE && node->type != PDJSON_NULL) return -1;
    return index;
}
static int integration_json_parse(unsigned slot, const void *bytes, usize size) {
    if (slot >= 2 || (!bytes && size) || (size && memchr(bytes, 0, size)) || !string_view_utf8_valid(string_view_from_raw((u8 *)bytes, size))) return 1;
    integration_json_document *document = &integration_documents[slot];
    document->count = 0; document->memory = arena_from_buffer(integration_json_memory[slot], sizeof(integration_json_memory[slot]));
    pdjson_stream parser; pdjson_open_buffer(&parser, bytes, size);
    pdjson_allocator allocator = {integration_json_allocate, integration_json_reallocate, integration_json_release};
    pdjson_set_allocator(&parser, &allocator, document->memory);
    pdjson_set_language(&parser, PDJSON_LANGUAGE_JSON); pdjson_set_streaming(&parser, false);
    int index = integration_json_value(document, &parser, 0, NULL);
    int ok = index == 0 && pdjson_next(&parser) == PDJSON_DONE;
    if (!ok) { const char *problem = pdjson_get_error(&parser); fprintf(stderr, "Invalid/beyond-capacity command JSON: %s\n", problem ? problem : "tree/shape limit"); }
    pdjson_close(&parser); return !ok;
}
static integration_json_node *integration_json_get(unsigned slot, const char *path) {
    integration_json_document *document = &integration_documents[slot];
    if (!document->count) return NULL;
    integration_json_node *node = &document->nodes[0];
    while (path && *path) {
        const char *end = strchr(path, '.'); usize size = end ? (usize)(end - path) : strlen(path);
        int child = node->first;
        if (node->type == PDJSON_ARRAY) {
            usize index = 0;
            if (!size || size > 8) return NULL;
            for (usize i = 0; i < size; ++i) { if (path[i] < '0' || path[i] > '9') return NULL; index = index * 10 + (usize)(path[i] - '0'); }
            if (index >= (usize)node->count) return NULL;
            while (index--) child = document->nodes[child].next;
        } else if (node->type == PDJSON_OBJECT) {
            while (child >= 0) {
                const char *key = document->nodes[child].name;
                if (strlen(key) == size && !memcmp(key, path, size)) break;
                child = document->nodes[child].next;
            }
        } else return NULL;
        if (child < 0) return NULL;
        node = &document->nodes[child]; path = end ? end + 1 : NULL;
    }
    return node;
}
static const char *integration_text(const char *path) {
    integration_json_node *node = integration_json_get(0, path);
    return node && node->type == PDJSON_STRING ? node->text : NULL;
}
static int integration_text_is(const char *path, const char *text) {
    const char *actual = integration_text(path); return actual && text && !strcmp(actual, text);
}
static int integration_number_is(const char *path, long long integer) {
    integration_json_node *node = integration_json_get(0, path); value actual = {0}, expected = {0};
    expected.kind = VALUE_NUMBER; expected.number_kind = NUMBER_I64; expected.number.signed_integer = integer;
    return node && node->type == PDJSON_NUMBER && parse_number(node->text, &actual) == NUMBER_VALID && numbers_equal(actual, expected);
}
static int integration_count(const char *path) {
    integration_json_node *node = integration_json_get(0, path);
    return node && (node->type == PDJSON_ARRAY || node->type == PDJSON_OBJECT) ? node->count : -1;
}
static int integration_json_equal(integration_json_document *left, integration_json_node *a,
                                   integration_json_document *right, integration_json_node *b, unsigned depth) {
    if (!a || !b || depth > 16 || a->type != b->type) return 0;
    if (a->type == PDJSON_STRING) return !strcmp(a->text, b->text);
    if (a->type == PDJSON_NUMBER) {
        value first = {0}, second = {0};
        return parse_number(a->text, &first) == NUMBER_VALID && parse_number(b->text, &second) == NUMBER_VALID && numbers_equal(first, second);
    }
    if (a->count != b->count) return 0;
    int other = b->first;
    for (int child = a->first; child >= 0; child = left->nodes[child].next) {
        if (a->type == PDJSON_OBJECT) {
            other = b->first;
            while (other >= 0 && strcmp(left->nodes[child].name, right->nodes[other].name)) other = right->nodes[other].next;
        }
        if (other < 0 || !integration_json_equal(left, &left->nodes[child], right, &right->nodes[other], depth + 1)) return 0;
        other = right->nodes[other].next;
    }
    return 1;
}
static int integration_json_matches(const char *path, const char *expected, const char *expected_path) {
    if (integration_json_parse(1, expected, strlen(expected))) return 0;
    return integration_json_equal(&integration_documents[0], integration_json_get(0, path), &integration_documents[1], integration_json_get(1, expected_path), 0);
}
