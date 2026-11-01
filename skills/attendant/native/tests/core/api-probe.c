/* Headless test executable only; not a new public CLI command. */
#include "../../src/native-api.c"
int main(int argc, char **argv) {
    attendant_session *session = attendant_open();
    attendant_result result;
    LPWSTR *arguments;
    char *owned[16] = {0};
    const char *target;
    int i, code;
    jsonb output;
    char *json;
    if (!session) return 3;
    arguments = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!arguments || argc > 16) return 2;
    for (i = 0; i < argc; i++) {
        int size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, arguments[i], -1, NULL, 0, NULL, NULL);
        owned[i] = HeapAlloc(GetProcessHeap(), 0, size);
        if (!owned[i] || !WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, arguments[i], -1, owned[i], size, NULL, NULL)) return 3;
    }
    LocalFree(arguments); argv = owned;
    if (argc == 3 && !strcmp(argv[1], "read")) { target = argv[2]; result = attendant_read_document(session, target); }
    else if (argc == 5 && !strcmp(argv[1], "save")) { target = argv[2]; result = attendant_save_document(session, target, argv[3], argv[4]); }
    else if (argc == 10 && (!strcmp(argv[1], "send") || !strcmp(argv[1], "send-context"))) {
        attendant_feedback input;
        memset(&input, 0, sizeof(input));
        input.project = argv[2]; input.target_path = target = argv[3]; input.target_hash = argv[4];
        input.range_start = _strtoui64(argv[5], NULL, 10); input.range_end = _strtoui64(argv[6], NULL, 10);
        input.selected_text = argv[7]; input.comment = argv[8];
        input.mode = !strcmp(argv[1], "send-context") ? ATTENDANT_FEEDBACK_CONTEXT : ATTENDANT_FEEDBACK_SOURCE;
        if (strcmp(argv[9], "probe")) return 2;
        result = attendant_send_feedback(session, &input);
    } else return 2;
    code = result.ok ? 0 : 1;
    json = allocate(&session->state, OUTPUT_CAPACITY); jsonb_init(&output);
    json_check(jsonb_object(&output, json, OUTPUT_CAPACITY));
    json_check(jsonb_key(&output, json, OUTPUT_CAPACITY, "ok", 2));
    json_check(jsonb_bool(&output, json, OUTPUT_CAPACITY, result.ok));
    if (result.code) json_property(&output, json, OUTPUT_CAPACITY, "code", result.code);
    if (result.message) json_property(&output, json, OUTPUT_CAPACITY, "message", result.message);
    if (result.path) json_property(&output, json, OUTPUT_CAPACITY, "path", result.path);
    if (result.hash) json_property(&output, json, OUTPUT_CAPACITY, "hash", result.hash);
    if (result.text) json_property(&output, json, OUTPUT_CAPACITY, "text", result.text);
    json_check(jsonb_key(&output, json, OUTPUT_CAPACITY, "written", 7)); json_check(jsonb_array(&output, json, OUTPUT_CAPACITY));
    for (i = 0; i < result.written_count; i++) json_check(jsonb_string(&output, json, OUTPUT_CAPACITY, result.written[i], strlen(result.written[i])));
    json_check(jsonb_array_pop(&output, json, OUTPUT_CAPACITY));
    printf("%.*s", (int)output.pos, json);
    /* Prove an operational failure does not poison next request in same process. */
    if (!result.ok) {
        attendant_result next = attendant_read_document(session, target);
        printf(",\"recovered\":%s", next.ok ? "true" : "false");
    }
    puts("}");
    attendant_close(session);
    for (i = 0; i < argc; i++) HeapFree(GetProcessHeap(), 0, owned[i]);
    return code;
}
