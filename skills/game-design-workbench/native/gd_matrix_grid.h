#ifndef GD_MATRIX_GRID_H
#define GD_MATRIX_GRID_H
#include "gd_layout.h"

/* Presentation only. No dense Cartesian allocation; every loaded axis retained.
   order: nouns then verbs. slot: record key -> index within its own axis. */
enum { GD_MATRIX_GRID_CHOICE=2, GD_MATRIX_COLUMN_WIDTH=176, GD_MATRIX_ROW_HEIGHT=44,
       GD_MATRIX_NOUN_WIDTH=240, GD_MATRIX_HEADER_HEIGHT=64 };
typedef struct {
    int nouns, verbs, noun, verb;
    int order[GD_MAX_RECORDS], slot[GD_MAX_RECORDS];
} GD_MatrixGrid;
typedef struct { int first_row, end_row, first_column, end_column; } GD_MatrixGridRange;
int gd_matrix_grid_prepare(GD_MatrixGrid *grid, const GD_Model *model, GD_LayoutSort sort);
int gd_matrix_grid_cell(const GD_Model *model, int noun, int verb);
/* Half-open cell bounds. Headers and points outside axes are not cells. */
int gd_matrix_grid_hit(const GD_MatrixGrid *grid, float x, float y, int *row, int *column);
GD_MatrixGridRange gd_matrix_grid_range(const GD_MatrixGrid *grid, float left, float top, float right, float bottom);
/* Clamped navigation. No selection starts at first cell, including blank cells. */
int gd_matrix_grid_move(GD_MatrixGrid *grid, int rows, int columns);
#endif
