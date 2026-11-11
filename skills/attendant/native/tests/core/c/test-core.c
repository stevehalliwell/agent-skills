#include "test.h"

/* Migrated from api-check.mjs. Wrapper runs under UTC and Pacific/Auckland. */
TEST(datetime_roundtrip_and_exact_filetime,
    const struct { const char *text; u64 ticks; } cases[] = {
        {"1601-01-01T00:00:00.000Z", 0ULL},
        {"1969-12-31T23:59:59.000Z", 116444735990000000ULL},
        {"1970-01-01T00:00:00.000Z", 116444736000000000ULL},
        {"2026-10-04T23:45:59.000Z", 134356311590000000ULL},
        {"2028-02-29T01:02:03.123Z", 134798725231230000ULL},
        {"9999-12-31T23:59:59.999Z", 2650467743999990000ULL}
    };
    TEST_ASSERT(source_test_session != NULL);
    context *c = request_begin(source_test_session); c->recovery = NULL;
    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        source_time stamp;
        TEST_ASSERT(parse_datetime(cases[i].text, &stamp) && stamp == cases[i].ticks);
        TEST_ASSERT(!strcmp(iso_time(c, stamp), cases[i].text));
    }
)

TEST(datetime_rejects_invalid_calendar_and_noncanonical_input,
    const char *invalid[] = {
        "1600-12-31T23:59:59.999Z", "1900-02-29T00:00:00.000Z", "2026-04-31T00:00:00.000Z",
        "2026-01-01T24:00:00.000Z", "2026-01-01T00:00:60.000Z", "2026-01-01T00:00:00.000+01:00",
        "2026-01-01T00:00:00Z", "2026-01-01t00:00:00.000Z", "2026-01-01T00:00:00.000z", ""
    };
    source_time stamp;
    for (size_t i = 0; i < sizeof(invalid) / sizeof(invalid[0]); i++) TEST_ASSERT(!parse_datetime(invalid[i], &stamp));
    TEST_ASSERT(valid_date("2000-02-29") && valid_date("2028-02-29"));
    TEST_ASSERT(!valid_date("1900-02-29") && !valid_date("2026-02-29") && !valid_date("2026-13-01"));
)

TEST(decimal_integer_classification_and_overflow,
    value number = {0};
    TEST_ASSERT(parse_number("9223372036854775807", &number) == NUMBER_VALID);
    TEST_ASSERT(number.number_kind == NUMBER_I64 && number.number.signed_integer == INT64_MAX);
    TEST_ASSERT(parse_number("-9223372036854775808", &number) == NUMBER_VALID);
    TEST_ASSERT(number.number_kind == NUMBER_I64 && number.number.signed_integer == INT64_MIN);
    TEST_ASSERT(parse_number("18446744073709551615", &number) == NUMBER_VALID);
    TEST_ASSERT(number.number_kind == NUMBER_U64 && number.number.unsigned_integer == UINT64_MAX);
    TEST_ASSERT(parse_number("18446744073709551616", &number) == NUMBER_INVALID);
    TEST_ASSERT(parse_number("-9223372036854775809", &number) == NUMBER_INVALID);
    TEST_ASSERT(parse_number("+42", &number) == NUMBER_VALID && number.number.signed_integer == 42);
    TEST_ASSERT(parse_number("-0", &number) == NUMBER_VALID && number.number.signed_integer == 0);
)

TEST(decimal_float_selection_and_underflow,
    value number = {0};
    TEST_ASSERT(parse_number("1.0", &number) == NUMBER_VALID && number.number_kind == NUMBER_F64 && number.number.real == 1);
    TEST_ASSERT(parse_number("1e3", &number) == NUMBER_VALID && number.number_kind == NUMBER_F64 && number.number.real == 1000);
    TEST_ASSERT(parse_number("0e-999", &number) == NUMBER_VALID && number.number.real == 0);
    TEST_ASSERT(parse_number("1e-999", &number) == NUMBER_INVALID);
    TEST_ASSERT(parse_number("1e999", &number) == NUMBER_INVALID);
    TEST_ASSERT(parse_number("1e", &number) == NUMBER_NOT_NUMERIC);
    TEST_ASSERT(parse_number("0x10", &number) == NUMBER_NOT_NUMERIC);
    TEST_ASSERT(parse_number("", &number) == NUMBER_NOT_NUMERIC);
)

TEST(numeric_equality_does_not_round_large_integers,
    value integer = {0}, real = {0}, large = {0};
    TEST_ASSERT(parse_number("9007199254740993", &integer) == NUMBER_VALID);
    TEST_ASSERT(parse_number("9007199254740992.0", &real) == NUMBER_VALID);
    TEST_ASSERT(!numbers_equal(integer, real) && !numbers_equal(real, integer));
    TEST_ASSERT(parse_number("18446744073709551615", &large) == NUMBER_VALID);
    TEST_ASSERT(parse_number("18446744073709551616.0", &real) == NUMBER_VALID);
    TEST_ASSERT(!numbers_equal(large, real));
    TEST_ASSERT(parse_number("42", &integer) == NUMBER_VALID && parse_number("42.0", &real) == NUMBER_VALID);
    TEST_ASSERT(numbers_equal(integer, real) && numbers_equal(real, integer));
)

TEST(flat_yaml_preserves_unicode_and_value_byte_spans,
    const char *text = "---\r\ndesc: caf\xc3\xa9\r\ntags: [caf\xc3\xa9, \xce\xa9] # keep comment\r\nsummary: NEL\xc2\x85LS\xe2\x80\xa8PS\xe2\x80\xa9\r\n---\r\nBody bytes.\r\n";
    const char *body = NULL, *array = "[caf\xc3\xa9, \xce\xa9]";
    TEST_ASSERT(source_test_session != NULL);
    context *c = request_begin(source_test_session); c->recovery = NULL;
    mapping parsed = read_mapping(c, text, &body);
    TEST_ASSERT(!parsed.problem && parsed.present && parsed.count == 3 && !strcmp(body, "Body bytes.\r\n"));
    TEST_ASSERT(!strcmp(lookup(&parsed, "desc")->text, "caf\xc3\xa9"));
    TEST_ASSERT(!strcmp(lookup(&parsed, "summary")->text, "NEL\xc2\x85LS\xe2\x80\xa8PS\xe2\x80\xa9"));
    pair *tags = &parsed.pairs[1];
    TEST_ASSERT(!strcmp(tags->key, "tags") && tags->data.kind == VALUE_ARRAY && tags->data.count == 2);
    TEST_ASSERT(tags->value_start == (size_t)(strstr(text, array) - text));
    TEST_ASSERT(tags->value_end - tags->value_start == strlen(array) && !memcmp(text + tags->value_start, array, strlen(array)));
)

TEST(flat_yaml_rejects_unsupported_shapes_and_values,
    const char *invalid[] = {
        "---\na: 1\na: 2\n---\n", "---\na: {nested: 1}\n---\n", "---\na: [[1]]\n---\n",
        "---\na: &value 1\nb: *value\n---\n", "---\na: !!str 1\n---\n", "---\na: 0x10\n---\n",
        "---\na: .inf\n---\n", "---\na: 18446744073709551616\n---\n", "---\na: 1e-999\n---\n",
        "---\na: \"\\0\"\n---\n", "---\na: \xff\n---\n", "---\na: 1\n", "No front matter."
    };
    TEST_ASSERT(source_test_session != NULL);
    for (size_t i = 0; i < sizeof(invalid) / sizeof(invalid[0]); i++) {
        context *c = request_begin(source_test_session); c->recovery = NULL;
        TEST_ASSERT(read_mapping(c, invalid[i], NULL).problem != NULL);
    }
)

TEST(flat_yaml_array_capacity_is_explicit,
    char text[4096]; size_t used = 0;
    TEST_ASSERT(source_test_session != NULL);
    used += (size_t)snprintf(text + used, sizeof(text) - used, "---\nitems: [");
    for (int i = 0; i < MAX_ITEMS; i++) used += (size_t)snprintf(text + used, sizeof(text) - used, "%s0", i ? "," : "");
    snprintf(text + used, sizeof(text) - used, "]\n---\n");
    context *c = request_begin(source_test_session); c->recovery = NULL;
    mapping parsed = read_mapping(c, text, NULL);
    TEST_ASSERT(!parsed.problem && lookup(&parsed, "items")->count == MAX_ITEMS);
    snprintf(text + used, sizeof(text) - used, ",0]\n---\n");
    c = request_begin(source_test_session); c->recovery = NULL;
    TEST_ASSERT(read_mapping(c, text, NULL).problem != NULL);
)

TEST(sha256_known_vectors,
    TEST_ASSERT(source_test_session != NULL);
    context *c = request_begin(source_test_session); c->recovery = NULL;
    TEST_ASSERT(!strcmp(digest(c, ""), "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"));
    TEST_ASSERT(!strcmp(digest(c, "abc"), "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"));
)
