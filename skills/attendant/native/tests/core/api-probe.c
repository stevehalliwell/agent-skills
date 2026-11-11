/* Feedback fixture producer for retained API/CLI subprocess integration.
   Direct source, failure-recovery and datetime tests live in tests/core/c/. */
#include "../../src/native-api.c"
int main(int argc, char **argv) {
    attendant_session *session = attendant_open();
    attendant_result result;
#if defined(_WIN32)
    LPWSTR *arguments;
    char *owned[16] = {0};
#endif
    int i, code;
    jsonb output;
    char *json;
    if (!session) return 3;
#if defined(_WIN32)
    arguments = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!arguments || argc > 16) return 2;
    for (i = 0; i < argc; i++) {
        int size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, arguments[i], -1, NULL, 0, NULL, NULL);
        owned[i] = HeapAlloc(GetProcessHeap(), 0, size);
        if (!owned[i] || !WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, arguments[i], -1, owned[i], size, NULL, NULL)) return 3;
    }
    LocalFree(arguments); argv = owned;
#endif
    if (argc == 10 && (!strcmp(argv[1], "send") || !strcmp(argv[1], "send-context"))) {
        attendant_feedback input;
        memset(&input, 0, sizeof(input));
        input.project = argv[2]; input.target_path = argv[3]; input.target_hash = argv[4];
#if defined(_WIN32)
        input.range_start = _strtoui64(argv[5], NULL, 10); input.range_end = _strtoui64(argv[6], NULL, 10);
#else
        input.range_start = strtoull(argv[5], NULL, 10); input.range_end = strtoull(argv[6], NULL, 10);
#endif
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
    puts("}");
    attendant_close(session);
#if defined(_WIN32)
    for (i = 0; i < argc; i++) HeapFree(GetProcessHeap(), 0, owned[i]);
#endif
    return code;
}
