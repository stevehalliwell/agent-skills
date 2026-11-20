/* Shared bounded literal matching: ASCII case-insensitive, other UTF-8 bytes exact. */
#ifndef GD_TEXT_MATCH_H
#define GD_TEXT_MATCH_H
#include <string.h>
enum { GD_FIND_TEXT=129 };
typedef struct {unsigned char text[GD_FIND_TEXT];int prefix[GD_FIND_TEXT],length;} GD_FindPattern;
static inline unsigned char find_lower(unsigned char value){return value>='A' && value<='Z' ? (unsigned char)(value+'a'-'A'):value;}
static inline void find_pattern(GD_FindPattern *pattern,const char *text){
    int i,j=0;memset(pattern,0,sizeof(*pattern));
    for(i=0;text[i] && i<GD_FIND_TEXT-1;++i)pattern->text[i]=find_lower((unsigned char)text[i]);pattern->length=i;
    for(i=1;i<pattern->length;++i){while(j && pattern->text[i]!=pattern->text[j])j=pattern->prefix[j-1];if(pattern->text[i]==pattern->text[j])++j;pattern->prefix[i]=j;}
}
static inline int find_contains(const GD_FindPattern *pattern,const char *text){
    int j=0;if(!text || !pattern->length)return 0;
    while(*text){unsigned char value=find_lower((unsigned char)*text++);while(j && value!=pattern->text[j])j=pattern->prefix[j-1];if(value==pattern->text[j])++j;if(j==pattern->length)return 1;}return 0;
}
static inline int gd_text_contains(const char *text,const char *needle){
    GD_FindPattern pattern;if(!*needle)return 1;if(strlen(needle)>=GD_FIND_TEXT)return 0;find_pattern(&pattern,needle);return find_contains(&pattern,text);
}
#endif
