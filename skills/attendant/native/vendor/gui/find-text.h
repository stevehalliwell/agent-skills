/* Bounded literal matching shared by C UI and C++ preview. UTF-8 validity is
 * checked by callers. ASCII folding never changes UTF-8 byte boundaries.
 * KMP gives linear work even for repetitive 4 MiB inputs. No allocations. */
#ifndef ATTENDANT_FIND_TEXT_H
#define ATTENDANT_FIND_TEXT_H
#include <stddef.h>
#define TEXT_FIND_QUERY 1024
typedef struct text_find_result { int count; size_t begin, end; } text_find_result;
static unsigned text_find_fold(unsigned c, int match_case) {
    return !match_case && c >= 'A' && c <= 'Z' ? c + ('a' - 'A') : c;
}
static int text_find_query_length(const char *query) {
    for (int i = 0; i < TEXT_FIND_QUERY; ++i) {
        if (!query[i]) return i;
        if (query[i] == '\r' || query[i] == '\n') return -1;
    }
    return -1;
}
static text_find_result text_find_scan(const char *text, size_t length, const char *query, int match_case, int target) {
    text_find_result result = {0, 0, 0}; unsigned short prefix[TEXT_FIND_QUERY];
    int size = text_find_query_length(query);
    if (size <= 0) return result;
    prefix[0] = 0;
    for (int i = 1, matched = 0; i < size; ++i) {
        unsigned c = text_find_fold((unsigned char)query[i], match_case);
        while (matched && c != text_find_fold((unsigned char)query[matched], match_case)) matched = prefix[matched - 1];
        if (c == text_find_fold((unsigned char)query[matched], match_case)) ++matched;
        prefix[i] = (unsigned short)matched;
    }
    for (size_t i = 0, matched = 0; i < length; ++i) {
        unsigned c = text_find_fold((unsigned char)text[i], match_case);
        while (matched && c != text_find_fold((unsigned char)query[matched], match_case)) matched = prefix[matched - 1];
        if (c == text_find_fold((unsigned char)query[matched], match_case)) ++matched;
        if (matched == (size_t)size) {
            if (result.count == target) { result.begin = i + 1 - matched; result.end = i + 1; }
            ++result.count; matched = 0;
        }
    }
    return result;
}
#endif
