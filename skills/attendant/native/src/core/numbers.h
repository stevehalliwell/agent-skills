/* Decimal source/JSON numbers: exact i64/u64 integer tokens, finite f64 otherwise.
   A decimal point or exponent explicitly selects f64. No integer-to-float fallback. */
typedef enum number_parse { NUMBER_NOT_NUMERIC, NUMBER_VALID, NUMBER_INVALID } number_parse;
static number_parse parse_number(const char *text, value *out) {
    const char *p = text, *digits;
    int negative = 0, count = 0, fractional = 0, nonzero = 0;
    unsigned long long magnitude = 0, maximum;
    if (*p == '+' || *p == '-') { negative = *p == '-'; p++; }
    digits = p;
    while (*p >= '0' && *p <= '9') { nonzero |= *p != '0'; count++; p++; }
    if (*p == '.') {
        fractional = 1; p++;
        while (*p >= '0' && *p <= '9') { nonzero |= *p != '0'; count++; p++; }
    }
    if (!count) return NUMBER_NOT_NUMERIC;
    if (*p == 'e' || *p == 'E') {
        int exponent_digits = 0;
        fractional = 1; p++;
        if (*p == '+' || *p == '-') p++;
        while (*p >= '0' && *p <= '9') { exponent_digits++; p++; }
        if (!exponent_digits) return NUMBER_NOT_NUMERIC;
    }
    if (*p) return NUMBER_NOT_NUMERIC;
    out->kind = VALUE_NUMBER;
    if (fractional) {
        out->number_kind = NUMBER_F64;
        out->number.real = strtod(text, NULL);
        return isfinite(out->number.real) && (out->number.real != 0 || !nonzero) ? NUMBER_VALID : NUMBER_INVALID;
    }
    maximum = negative ? 9223372036854775808ULL : 18446744073709551615ULL;
    for (p = digits; *p; p++) {
        unsigned digit = (unsigned)(*p - '0');
        if (magnitude > (maximum - digit) / 10) return NUMBER_INVALID;
        magnitude = magnitude * 10 + digit;
    }
    if (negative) {
        out->number_kind = NUMBER_I64;
        out->number.signed_integer = magnitude ? -(long long)(magnitude - 1) - 1 : 0;
    } else if (magnitude <= 9223372036854775807ULL) {
        out->number_kind = NUMBER_I64; out->number.signed_integer = (long long)magnitude;
    } else {
        out->number_kind = NUMBER_U64; out->number.unsigned_integer = magnitude;
    }
    return NUMBER_VALID;
}
static int numbers_equal(value a, value b) {
    if (a.number_kind == NUMBER_F64 && b.number_kind == NUMBER_F64) return a.number.real == b.number.real;
    if (b.number_kind == NUMBER_F64) { value swap = a; a = b; b = swap; }
    if (a.number_kind == NUMBER_F64) {
        double real = a.number.real;
        if (b.number_kind == NUMBER_I64) return real >= -9223372036854775808.0 && real < 9223372036854775808.0 && floor(real) == real && (long long)real == b.number.signed_integer;
        return real >= 0 && real < 18446744073709551616.0 && floor(real) == real && (unsigned long long)real == b.number.unsigned_integer;
    }
    if (a.number_kind == NUMBER_I64 && b.number_kind == NUMBER_I64) return a.number.signed_integer == b.number.signed_integer;
    if (a.number_kind == NUMBER_U64 && b.number_kind == NUMBER_U64) return a.number.unsigned_integer == b.number.unsigned_integer;
    if (a.number_kind == NUMBER_U64) { value swap = a; a = b; b = swap; }
    return a.number.signed_integer >= 0 && (unsigned long long)a.number.signed_integer == b.number.unsigned_integer;
}
static void json_integer(jsonb *b, char *out, size_t capacity, long long integer) {
    char token[32];
    int length = snprintf(token, sizeof(token), "%lld", integer);
    json_check(jsonb_token(b, out, capacity, token, length));
}
typedef enum number_format { JSON_NUMBERS, YAML_NUMBERS, INDEX_NUMBERS } number_format;
static void json_numeric(context *c, jsonb *b, char *out, size_t capacity, value v, number_format format) {
    if (v.number_kind == NUMBER_I64) json_integer(b, out, capacity, v.number.signed_integer);
    else if (v.number_kind == NUMBER_U64) {
        char token[32];
        int length = snprintf(token, sizeof(token), "%llu", v.number.unsigned_integer);
        json_check(jsonb_token(b, out, capacity, token, length));
    } else {
        char token[128]; jsonb number;
        value reparsed = {0};
        /* Cache/FTS float spelling defines search terms, not source reconstruction. */
        if (format == INDEX_NUMBERS || !isfinite(v.number.real)) { json_number(c, b, out, capacity, v.number.real); return; }
        jsonb_init(&number); json_number(c, &number, token, sizeof(token) - 3, v.number.real); token[number.pos] = 0;
        /* Shortest float digits can denote a different exact integer. Preserve
           float selection on source reload, or when JSON would lose value or
           select an unsigned scalar that SQLite cannot accept. */
        if (!memchr(token, '.', number.pos) && !memchr(token, 'e', number.pos) &&
            (format == YAML_NUMBERS || parse_number(token, &reparsed) != NUMBER_VALID || reparsed.number_kind == NUMBER_U64 || !numbers_equal(v, reparsed))) {
            token[number.pos++] = '.'; token[number.pos++] = '0';
        }
        json_check(jsonb_token(b, out, capacity, token, number.pos));
    }
}
static void json_real(context *c, jsonb *b, char *out, size_t capacity, double number) {
    value v = {0}; v.kind = VALUE_NUMBER; v.number_kind = NUMBER_F64; v.number.real = number;
    json_numeric(c, b, out, capacity, v, JSON_NUMBERS);
}
