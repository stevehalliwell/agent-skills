#include "gd_picker.h"
TEST(picker_search_and_lists,
    char list[32]="nouns/hero\nnouns/brick",before[32];
    TEST_ASSERT(gd_picker_match("Hero | nouns/hero","") && gd_picker_match("Hero | nouns/hero","HRO"));
    TEST_ASSERT(gd_picker_match("Form | axes/Form","axfrm") && !gd_picker_match("Hero","heroX") && !gd_picker_match("","a"));
    TEST_ASSERT(gd_picker_match("nouns/hero","n hr") && !gd_picker_match("abc","cba"));
    TEST_ASSERT(gd_picker_contains(list,"nouns/hero") && !gd_picker_contains(list,"nouns/her"));
    TEST_ASSERT(gd_picker_toggle(list,sizeof(list),"nouns/hero") && !strcmp(list,"nouns/brick"));
    TEST_ASSERT(gd_picker_toggle(list,sizeof(list),"nouns/hero") && !strcmp(list,"nouns/brick\nnouns/hero"));
    memcpy(before,list,sizeof(list)); TEST_ASSERT(!gd_picker_toggle(list,sizeof(list),"nouns/too-long-to-fit") && !memcmp(list,before,sizeof(list)));
    TEST_ASSERT(!gd_picker_toggle(list,sizeof(list),"invalid\nidentity") && !memcmp(list,before,sizeof(list)));
    TEST_ASSERT(gd_picker_toggle(list,sizeof(list),"nouns/hero") && gd_picker_toggle(list,sizeof(list),"nouns/brick") && !*list);
    TEST_ASSERT(gd_picker_toggle_limit(list,sizeof(list),"Hero",1) && !gd_picker_contains(list,"hero"));
    TEST_ASSERT(!gd_picker_toggle_limit(list,sizeof(list),"Brick",1) && !strcmp(list,"Hero"));
    TEST_ASSERT(gd_picker_toggle_limit(list,sizeof(list),"Hero",1) && !*list);
    TEST_ASSERT(gd_picker_match("Café","é") && gd_picker_match("Hero"," \t "));
    { char text[]="one;two\n three ;four ",*cursor=text; TEST_ASSERT(!strcmp(gd_picker_next_item(&cursor,1),"one;two") && !strcmp(gd_picker_next_item(&cursor,1)," three ;four ") && !gd_picker_next_item(&cursor,1)); }
    { char text[]="one;two\n three ;four ",*cursor=text; TEST_ASSERT(!strcmp(gd_picker_next_item(&cursor,0),"one") && !strcmp(gd_picker_next_item(&cursor,0),"two") && !strcmp(gd_picker_next_item(&cursor,0),"three") && !strcmp(gd_picker_next_item(&cursor,0),"four") && !gd_picker_next_item(&cursor,0)); }
)
