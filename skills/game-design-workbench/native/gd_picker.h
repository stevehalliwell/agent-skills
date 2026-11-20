#ifndef GD_PICKER_H
#define GD_PICKER_H
#include <stddef.h>
#include <string.h>
/* UI-only ASCII-case-insensitive subsequence search. UTF-8 bytes otherwise exact;
   never normalizes or substitutes native identities. Empty search matches all. */
static inline int gd_picker_match(const char *text,const char *search) {
    size_t i=0,j=0; if (!text || !search) return 0;
    while (search[j]) {
        int found=0; unsigned char wanted=(unsigned char)search[j];
        if (wanted==' ' || wanted=='\t') { ++j; continue; }
        if (wanted>='A' && wanted<='Z') wanted+=(unsigned char)('a'-'A');
        while (text[i]) { unsigned char value=(unsigned char)text[i++]; if (value>='A' && value<='Z') value+=(unsigned char)('a'-'A'); if (value==wanted) { found=1; break; } }
        if (!found) return 0; ++j;
    }
    return 1;
}
/* Destructive form-token iterator. Exact newline mode preserves bytes;
   legacy semicolon shorthand trims surrounding ASCII whitespace. */
static inline char *gd_picker_next_item(char **cursor,int literal) {
    char *start=*cursor,*end,*last;
    while (start && *start) {
        end=literal ? strchr(start,'\n'):strpbrk(start,";\n"); if (end) { *end=0; *cursor=end+1; } else *cursor=NULL;
        if (!literal) { while (*start==' ' || *start=='\t' || *start=='\r') ++start;
            last=start+strlen(start); while (last>start && (last[-1]==' ' || last[-1]=='\t' || last[-1]=='\r')) *--last=0; }
        if (*start) return start; start=*cursor;
    }
    return NULL;
}
/* Exact newline-list membership; mutations validate full capacity before write. */
static inline int gd_picker_contains(const char *list,const char *value) {
    const char *line=list; size_t length=strlen(value);
    while (*line) { const char *end=strchr(line,'\n'); size_t size=end ? (size_t)(end-line):strlen(line); if (size==length && !memcmp(line,value,size)) return 1; if (!end) break; line=end+1; } return 0;
}
static inline int gd_picker_toggle(char *list,size_t capacity,const char *value) {
    size_t length=strlen(value),used=strlen(list); char *line=list;
    if (!length || strchr(value,'\n') || strchr(value,'\r') || used>=capacity) return 0;
    while (*line) { char *end=strchr(line,'\n'); size_t size=end ? (size_t)(end-line):strlen(line);
        if (size==length && !memcmp(line,value,size)) { if (end) memmove(line,end+1,strlen(end+1)+1); else if (line>list) line[-1]=0; else *line=0; return 1; } if (!end) break; line=end+1;
    }
    if (length+(used ? 1:0)>=capacity-used) return 0;
    if (used) list[used++]='\n'; memcpy(list+used,value,length+1); return 1;
}
static inline int gd_picker_toggle_limit(char *list,size_t capacity,const char *value,int maximum) {
    const char *cursor=list; int count=0;
    if (gd_picker_contains(list,value)) return gd_picker_toggle(list,capacity,value);
    if (*cursor) { count=1; while (*cursor) if (*cursor++=='\n') ++count; }
    return count<maximum && gd_picker_toggle(list,capacity,value);
}
#endif
