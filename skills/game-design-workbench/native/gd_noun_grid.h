#ifndef GD_NOUN_GRID_H
#define GD_NOUN_GRID_H
#include "gd_layout.h"
/* Immutable, sparse same-case projection. Borrows rules; owns all other storage.
   Records remain canonical keys. No evaluator, verb dependency or dense matrix. */
enum { GD_NOUN_GRID_WORK=64*1024*1024 };
enum { GD_NOUN_LAYOUT_GRID,GD_NOUN_LAYOUT_FREE };
/* Match MDA's rendered minimum spring span without scaling noun identities. */
enum { GD_NOUN_GRAPH_MIN_REST_LENGTH=330,GD_NOUN_GRAPH_CARD_HEIGHT=48 };
typedef struct { int a,b,count,first_case,last_case; } GD_NounPair;
typedef struct {
    GD_Allocator allocator;const GD_Rules *rules;
    int noun_count,pair_count,work,work_limit,limited,incomplete;
    int nouns[GD_MAX_RECORDS],slot[GD_MAX_RECORDS],hash[GD_MAX_PAIRS*2];
    GD_NounPair pairs[GD_MAX_PAIRS];
} GD_NounGrid;
GD_NounGrid *gd_noun_grid_build(const GD_Rules *rules,int work_limit);
void gd_noun_grid_destroy(GD_NounGrid *grid);
/* All noun identities, one undirected edge per retained pair, including loops.
   Borrows labels; caller owns layout. No new counts, filtering or evaluation. */
int gd_noun_grid_layout(const GD_NounGrid *grid,GD_Layout *layout);
int gd_noun_grid_pair(const GD_NounGrid *grid,int a,int b);
int gd_noun_grid_case(const GD_NounGrid *grid,int pair,int case_index);
int gd_noun_grid_write(FILE *out,const GD_NounGrid *grid);
#endif
