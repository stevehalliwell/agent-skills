#ifndef ATTENDANT_GUI_FIND_H
#define ATTENDANT_GUI_FIND_H
#include "../../vendor/gui/find-text.h"
typedef enum gui_find_target { GUI_FIND_SIDEBAR, GUI_FIND_SOURCE, GUI_FIND_PREVIEW } gui_find_target;
typedef struct gui_find {
    char query[TEXT_FIND_QUERY], applied[TEXT_FIND_QUERY];
    int open, focus, action, error, rejected, cached, count, current, apply;
    int tab, generation, match_case;
    uint64_t revision;
    size_t begin, end;
    void *input_state;
} gui_find;
static int gui_find_valid(const char *query) {
    return text_find_query_length(query) >= 0 && string_view_utf8_valid(string_view_from_cstr(query));
}
static void gui_find_source(gui_find *find, const gui_tab *tab, int index, int match_case) {
    if (find->error || !gui_find_valid(find->query)) { find->count = find->current = find->apply = find->cached = 0; return; }
    int identity = find->tab != index || find->generation != tab->generation;
    int query_changed = !find->cached || strcmp(find->query, find->applied) || find->match_case != match_case;
    if (!identity && !query_changed && find->revision == tab->text_revision && !find->action) return;
    if (!identity && !query_changed && find->revision != tab->text_revision && !find->action) find->apply = 0;
    int current = identity || query_changed || find->action == 2 ? 0 : (find->current ? find->current - 1 : 0);
    if (!identity && !query_changed && find->action && find->action != 2 && find->count)
        current = (current + (find->action < 0 ? -1 : 1) + find->count) % find->count;
    text_find_result hit = text_find_scan(tab->text, tab->length, find->query, match_case, current);
    if (current >= hit.count && hit.count) { current = 0; hit = text_find_scan(tab->text, tab->length, find->query, match_case, 0); }
    find->count = hit.count; find->current = hit.count ? current + 1 : 0; find->begin = hit.begin; find->end = hit.end;
    if (identity) find->apply = 0;
    if ((query_changed || identity || find->action) && hit.count) find->apply = 1;
    if (!hit.count) find->apply = 0;
    find->action = 0; find->tab = index; find->generation = tab->generation; find->revision = tab->text_revision;
    find->match_case = match_case; find->cached = 1; memcpy(find->applied, find->query, strlen(find->query) + 1);
}
static int gui_find_contains(const char *text, const char *query, int match_case) {
    return !*query || text_find_scan(text, strlen(text), query, match_case, 0).count > 0;
}
#endif
