/* Disposable LibYAML document probe. Input and output are YAML only, not Markdown. */
#include <stdio.h>
#include <string.h>
#include <yaml.h>

int main(int argc, char **argv) {
    yaml_parser_t parser;
    yaml_document_t doc;
    yaml_emitter_t emitter;
    yaml_node_t *root;
    yaml_node_pair_t *pair;
    int found = 0, key, value;
    if (argc != 2 || !yaml_parser_initialize(&parser)) return 2;
    yaml_parser_set_input_file(&parser, stdin);
    if (!yaml_parser_load(&parser, &doc)) {
        fprintf(stderr, "parse: %s at %lu:%lu\n", parser.problem ? parser.problem : "unknown", (unsigned long)parser.problem_mark.line + 1, (unsigned long)parser.problem_mark.column + 1);
        yaml_parser_delete(&parser);
        return 1;
    }
    yaml_parser_delete(&parser);
    root = yaml_document_get_root_node(&doc);
    if (!root || root->type != YAML_MAPPING_NODE) { fprintf(stderr, "not a mapping\n"); yaml_document_delete(&doc); return 1; }
    if (!strcmp(argv[1], "parse")) { yaml_document_delete(&doc); puts("mapping accepted"); return 0; }
    if (strcmp(argv[1], "edit")) { yaml_document_delete(&doc); return 2; }
    for (pair = root->data.mapping.pairs.start; pair < root->data.mapping.pairs.top; ++pair) {
        yaml_node_t *k = yaml_document_get_node(&doc, pair->key);
        if (k && k->type == YAML_SCALAR_NODE && k->data.scalar.length == 6 && !memcmp(k->data.scalar.value, "status", 6)) {
            if (found++) { fprintf(stderr, "duplicate status\n"); yaml_document_delete(&doc); return 1; }
            value = yaml_document_add_scalar(&doc, NULL, (yaml_char_t *)"updated", -1, YAML_PLAIN_SCALAR_STYLE);
            if (!value) { yaml_document_delete(&doc); return 2; }
            /* Adding a node may reallocate the node stack; pair storage is separate. */
            pair->value = value;
        }
    }
    if (!found) { fprintf(stderr, "status missing\n"); yaml_document_delete(&doc); return 1; }
    key = yaml_document_add_scalar(&doc, NULL, (yaml_char_t *)"spike_added", -1, YAML_PLAIN_SCALAR_STYLE);
    value = yaml_document_add_scalar(&doc, NULL, (yaml_char_t *)"yes", -1, YAML_DOUBLE_QUOTED_SCALAR_STYLE);
    if (!key || !value || !yaml_document_append_mapping_pair(&doc, 1, key, value)) { yaml_document_delete(&doc); return 2; }
    if (!yaml_emitter_initialize(&emitter)) { yaml_document_delete(&doc); return 2; }
    yaml_emitter_set_output_file(&emitter, stdout);
    yaml_emitter_set_unicode(&emitter, 1);
    if (!yaml_emitter_dump(&emitter, &doc)) { fprintf(stderr, "emit: %s\n", emitter.problem ? emitter.problem : "unknown"); yaml_emitter_delete(&emitter); return 1; }
    yaml_emitter_delete(&emitter);
    return 0;
}
