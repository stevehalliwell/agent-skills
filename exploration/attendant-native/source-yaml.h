/* Bounded flat YAML reader. Keep LibYAML marks for surgical value edits.
   Parser errors are data, not process exits; schema callers choose fatal policy. */
static int yaml_linebreak_width(const unsigned char *p) {
    if (p[0] == 0xc2 && p[1] == 0x85) return 2;
    if (p[0] == 0xe2 && p[1] == 0x80 && (p[2] == 0xa8 || p[2] == 0xa9)) return 3;
    return 0;
}
static size_t yaml_byte_offset(const char *text, size_t characters) {
    const unsigned char *p = (const unsigned char *)text;
    while (characters-- && *p) {
        p++;
        while ((*p & 0xc0) == 0x80) p++;
    }
    return (size_t)(p - (const unsigned char *)text);
}
static value yaml_scalar(context *c, yaml_node_t *node, mapping *result) {
    value v = {0};
    const char *text = (const char *)node->data.scalar.value;
    v.kind = VALUE_STRING; v.text = copy_text(c, text, node->data.scalar.length);
    if (node->data.scalar.length != strlen(text)) { result->problem = "NUL in YAML scalar is unsupported."; return v; }
    if (node->data.scalar.style != YAML_PLAIN_SCALAR_STYLE) return v;
    if (!*text || !strcmp(text, "~") || !_stricmp(text, "null")) v.kind = VALUE_NULL;
    else if (!_stricmp(text, "true") || !_stricmp(text, "false")) { v.kind = VALUE_BOOL; v.number.real = !_stricmp(text, "true"); }
    else if (strchr("0123456789-+.", *text)) {
        const char *number_text = *text == '+' || *text == '-' ? text + 1 : text;
        number_parse parsed;
        if (number_text[0] == '0' && (number_text[1] == 'x' || number_text[1] == 'X' || number_text[1] == 'o' || number_text[1] == 'O')) {
            result->problem = "Non-decimal YAML number is unsupported."; return v;
        }
        parsed = parse_number(text, &v);
        if (parsed == NUMBER_INVALID) result->problem = "Decimal number exceeds i64/u64 or finite f64 range.";
        else if (parsed == NUMBER_NOT_NUMERIC && (!strncmp(text, "0x", 2) || !strncmp(text, "0o", 2) || !_stricmp(text, ".nan") || !_stricmp(text, ".inf") || !_stricmp(text, "+.inf") || !_stricmp(text, "-.inf")))
            result->problem = "Non-decimal or nonfinite YAML number is unsupported.";
    }
    return v;
}
static mapping read_mapping(context *c, const char *text, const char **body) {
    mapping result = {0};
    const char *start, *end = NULL, *line;
    yaml_parser_t parser, scanner;
    yaml_document_t document;
    yaml_node_t *root;
    yaml_node_pair_t *entry;
    int loaded = 0, scanning = 0, initialized = 0, depth = 0, keys = 0, array_items = 0, expect_key = 1, documents = 0;
    result.problem_code = "record-frontmatter";
    if (!strncmp(text, "---\r\n", 5)) start = text + 5;
    else if (!strncmp(text, "---\n", 4)) start = text + 4;
    else { result.problem = "Record must begin with YAML front matter."; return result; }
    for (line = start; *line; ) {
        const char *next = strchr(line, '\n');
        size_t length = next ? (size_t)(next - line) : strlen(line);
        if (length && line[length - 1] == '\r') length--;
        if (length == 3 && !memcmp(line, "---", 3)) { end = line; if (body) *body = next ? next + 1 : line + strlen(line); break; }
        if (!next) break;
        line = next + 1;
    }
    if (!end) { result.problem = "Record front matter is missing closing delimiter."; return result; }
    result.problem_code = "record-yaml";
    /* Event preflight counts real nodes, including implicit nulls/indentless
       sequences, before LibYAML's recursive document loader allocates nodes. */
    if (!yaml_parser_initialize(&scanner)) fail(c, "YAML scanner allocation failed");
    scanning = 1;
    yaml_parser_set_input_string(&scanner, (const unsigned char *)start, end - start);
    for (;;) {
        yaml_event_t event;
        yaml_event_type_t type;
        int finished;
        if (!yaml_parser_parse(&scanner, &event)) { result.problem = "YAML event parsing failed."; goto done; }
        type = event.type; finished = type == YAML_STREAM_END_EVENT;
        if (type == YAML_ALIAS_EVENT ||
            (type == YAML_SCALAR_EVENT && (event.data.scalar.anchor || event.data.scalar.tag)) ||
            (type == YAML_MAPPING_START_EVENT && (event.data.mapping_start.anchor || event.data.mapping_start.tag)) ||
            (type == YAML_SEQUENCE_START_EVENT && (event.data.sequence_start.anchor || event.data.sequence_start.tag)) ||
            (type == YAML_DOCUMENT_START_EVENT && (++documents > 1 || !event.data.document_start.implicit || event.data.document_start.version_directive || event.data.document_start.tag_directives.start != event.data.document_start.tag_directives.end)) ||
            (type == YAML_DOCUMENT_END_EVENT && !event.data.document_end.implicit))
            result.problem = "Aliases, anchors, explicit tags and extra documents are unsupported.";
        if (type == YAML_MAPPING_START_EVENT) {
            if (depth) { result.problem_code = "record-nested"; result.problem = "Nested mappings are unsupported."; }
            else if (event.data.mapping_start.style != YAML_BLOCK_MAPPING_STYLE) result.problem = "Front matter root must be a block mapping.";
            depth++;
        } else if (type == YAML_SEQUENCE_START_EVENT) {
            if (!depth) result.problem = "Front matter root must be a block mapping.";
            else if (depth != 1) { result.problem_code = "record-nested"; result.problem = "Nested sequences are unsupported."; }
            else if (expect_key) { result.problem_code = "record-key"; result.problem = "Front matter keys must be scalar strings."; }
            depth++; array_items = 0; expect_key = 1;
        } else if (type == YAML_MAPPING_END_EVENT || type == YAML_SEQUENCE_END_EVENT) { if (depth) depth--; }
        else if (type == YAML_SCALAR_EVENT) {
            if (!depth && !(event.data.scalar.style == YAML_PLAIN_SCALAR_STYLE && !event.data.scalar.length)) result.problem = "Front matter root must be a block mapping.";
            if (depth == 1) {
                if (expect_key && ++keys > MAX_PAIRS) result.problem = "Front matter key capacity exceeded before document load.";
                expect_key = !expect_key;
            } else if (depth == 2 && ++array_items > MAX_ITEMS) result.problem = "YAML array capacity exceeded before document load.";
        }
        yaml_event_delete(&event);
        if (result.problem || finished) break;
    }
    if (result.problem) goto done;
    if (!yaml_parser_initialize(&parser)) fail(c, "YAML parser allocation failed");
    initialized = 1;
    yaml_parser_set_input_string(&parser, (const unsigned char *)start, end - start);
    if (!yaml_parser_load(&parser, &document)) {
        result.problem = format_text(c, "%s at line %lu", parser.problem ? parser.problem : "YAML parse failure", (unsigned long)parser.problem_mark.line + 2);
        goto done;
    }
    loaded = 1;
    root = yaml_document_get_root_node(&document);
    result.pairs = allocate(c, MAX_PAIRS * sizeof(pair));
    if (!root) goto done; /* Empty schemas declare no fields. */
    if (root->type != YAML_MAPPING_NODE || root->data.mapping.style == YAML_FLOW_MAPPING_STYLE) {
        result.problem = "Front matter must be a block mapping."; goto done;
    }
    result.present = 1;
    for (entry = root->data.mapping.pairs.start; entry < root->data.mapping.pairs.top; entry++) {
        yaml_node_t *key = yaml_document_get_node(&document, entry->key), *node = yaml_document_get_node(&document, entry->value);
        pair *p;
        value key_value;
        if (result.count == MAX_PAIRS) { result.problem = "Front matter key capacity exceeded."; goto done; }
        if (!key || key->type != YAML_SCALAR_NODE) { result.problem_code = "record-key"; result.problem = "Front matter keys must be non-empty strings."; goto done; }
        key_value = yaml_scalar(c, key, &result);
        if (result.problem) goto done;
        if (key_value.kind != VALUE_STRING || !*key_value.text) { result.problem_code = "record-key"; result.problem = "Front matter keys must be non-empty strings."; goto done; }
        if (lookup(&result, key_value.text)) { result.problem = format_text(c, "Duplicate YAML key %s.", key_value.text); goto done; }
        p = &result.pairs[result.count++]; p->key = key_value.text;
        p->value_start = (size_t)(start - text) + yaml_byte_offset(start, node->start_mark.index);
        p->value_end = (size_t)(start - text) + yaml_byte_offset(start, node->end_mark.index);
        if (node->type == YAML_SCALAR_NODE) p->data = yaml_scalar(c, node, &result);
        else if (node->type == YAML_SEQUENCE_NODE) {
            yaml_node_item_t *item;
            p->indentless_sequence = node->data.sequence.style == YAML_BLOCK_SEQUENCE_STYLE && node->start_mark.column <= key->start_mark.column;
            p->data.kind = VALUE_ARRAY;
            p->data.count = (int)(node->data.sequence.items.top - node->data.sequence.items.start);
            if (p->data.count > MAX_ITEMS) { result.problem = "YAML array capacity exceeded."; goto done; }
            p->data.items = allocate(c, (p->data.count + 1) * sizeof(value));
            for (item = node->data.sequence.items.start; item < node->data.sequence.items.top; item++) {
                yaml_node_t *child = yaml_document_get_node(&document, *item);
                if (child->type != YAML_SCALAR_NODE) { result.problem_code = "record-nested"; result.problem = "Values must be top-level scalars or scalar arrays."; goto done; }
                p->data.items[item - node->data.sequence.items.start] = yaml_scalar(c, child, &result);
                if (result.problem) goto done;
            }
            /* BLOCK_END is emitted after skipping following comments. The last
               scalar's end mark excludes that unrelated trailing trivia. */
            if (node->data.sequence.style == YAML_BLOCK_SEQUENCE_STYLE && p->data.count) {
                yaml_node_t *last = yaml_document_get_node(&document, node->data.sequence.items.top[-1]);
                p->value_end = (size_t)(start - text) + yaml_byte_offset(start, last->end_mark.index);
            }
        } else { result.problem_code = "record-nested"; result.problem = "Values must be top-level scalars or scalar arrays."; goto done; }
        if (result.problem) goto done;
    }
done:
    if (scanning) yaml_parser_delete(&scanner);
    if (loaded) yaml_document_delete(&document);
    if (initialized) yaml_parser_delete(&parser);
    return result;
}
