#ifndef GD_NOUN_LABEL_H
#define GD_NOUN_LABEL_H
#include "gd.h"
#include <string.h>
/* Presentation only. Never infer classification from a label or current role;
   is_noun survives normalized Relationships snapshots. */
typedef enum { GD_NOUN_LABEL_REGULAR,GD_NOUN_LABEL_BOLD,GD_NOUN_LABEL_UNDERLINE } GD_NounLabelStyle;
static inline GD_NounLabelStyle gd_noun_label_style(const GD_Record *record){
    if(!record || !record->is_noun || !record->is_actor_line)return GD_NOUN_LABEL_REGULAR;
    return record->is_actor ? GD_NOUN_LABEL_UNDERLINE:GD_NOUN_LABEL_BOLD;
}
/* Locate this known record's complete display label, even inside a multiline
   card. Prefix copy is bounded and never truncates UTF-8; unresolved has no record. */
static inline int gd_noun_label_prefix(const GD_Record *record,const char *text,char *prefix,size_t capacity){
    const char *found;size_t length;
    if(!prefix || !capacity)return 0;prefix[0]=0;
    if(gd_noun_label_style(record)==GD_NOUN_LABEL_REGULAR || !text || !record->display_name || !*record->display_name)return 0;
    found=strstr(text,record->display_name);if(!found)return 0;length=(size_t)(found-text);
    if(length>=capacity)return 0;memcpy(prefix,text,length);prefix[length]=0;return 1;
}
#endif
