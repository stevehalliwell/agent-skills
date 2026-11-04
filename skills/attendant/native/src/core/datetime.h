/* Canonical UTC timestamps only: YYYY-MM-DDTHH:mm:ss.sssZ.
   Calendar-only validation is shared with YYYY-MM-DD fields. FILETIME bounds apply. */
static int datetime_digits(const char *text, int count) {
    int result = 0, i;
    for (i = 0; i < count; i++) { if (text[i] < '0' || text[i] > '9') return -1; result = result * 10 + text[i] - '0'; }
    return result;
}
static int valid_date(const char *text) {
    static const int days[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    int year, month, day, maximum;
    if (strlen(text) < 10 || text[4] != '-' || text[7] != '-') return 0;
    year = datetime_digits(text, 4); month = datetime_digits(text + 5, 2); day = datetime_digits(text + 8, 2);
    if (year < 0 || month < 1 || month > 12 || day < 1) return 0;
    maximum = days[month] + (month == 2 && year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
    return day <= maximum;
}
static int parse_datetime(const char *text, source_time *out) {
    int hour, minute, second, milliseconds, year;
    if (strlen(text) != 24 || !valid_date(text) || text[10] != 'T' || text[13] != ':' || text[16] != ':' || text[19] != '.' || text[23] != 'Z') return 0;
    year = datetime_digits(text, 4); hour = datetime_digits(text + 11, 2); minute = datetime_digits(text + 14, 2);
    second = datetime_digits(text + 17, 2); milliseconds = datetime_digits(text + 20, 3);
    if (year < 1601 || year > 9999 || hour < 0 || hour > 23 || minute < 0 || minute > 59 || second < 0 || second > 59 || milliseconds < 0) return 0;
#if defined(_WIN32)
    SYSTEMTIME time = {0}; FILETIME native;
    time.wYear = (WORD)year; time.wMonth = (WORD)datetime_digits(text + 5, 2); time.wDay = (WORD)datetime_digits(text + 8, 2);
    time.wHour = (WORD)hour; time.wMinute = (WORD)minute; time.wSecond = (WORD)second; time.wMilliseconds = (WORD)milliseconds;
    if (!SystemTimeToFileTime(&time, &native)) return 0;
    *out = source_filetime(native);
#else
    /* Gregorian day count from 1601. Avoid Darwin timegm's pre-1900 limits
     * and timezone/error-state behavior; validated range fits u64 ticks. */
    static const int preceding_days[] = {0,31,59,90,120,151,181,212,243,273,304,334};
    int month = datetime_digits(text + 5, 2), prior_year = year - 1;
    u64 days = (u64)(365LL * (year - 1601) + prior_year / 4 - 400 -
        (prior_year / 100 - 16) + prior_year / 400 - 4 + preceding_days[month - 1] +
        datetime_digits(text + 8, 2) - 1 +
        (month > 2 && year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)));
    *out = (((days * 24 + hour) * 60 + minute) * 60 + second) * 10000000ULL + (u64)milliseconds * 10000ULL;
#endif
    return 1;
}
static const char *validated_datetime(context *c, const char *text) {
    source_time time;
    if (!parse_datetime(text, &time)) fail(c, "Datetime must be YYYY-MM-DDTHH:mm:ss.sssZ: %s", text);
    return text;
}
