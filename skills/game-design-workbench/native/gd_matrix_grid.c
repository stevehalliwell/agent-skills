#include "gd_matrix_grid.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

typedef struct { int record, side, degree; const char *label; } GridOrder;
static int compare_grid_order(const void *a, const void *b) {
    const GridOrder *left=a,*right=b; int order;
    if (left->side!=right->side) return left->side-right->side;
    if (left->degree!=right->degree) return left->degree>right->degree ? -1:1;
    order=strcmp(left->label,right->label);
    return order ? order:(left->record>right->record)-(left->record<right->record);
}
int gd_matrix_grid_prepare(GD_MatrixGrid *grid, const GD_Model *model, GD_LayoutSort sort) {
    GridOrder order[GD_MAX_RECORDS]; int i,count=0,nouns=0;
    if (!grid || !model || model->record_count<0 || model->record_count>GD_MAX_RECORDS ||
        (sort!=GD_SORT_NAME && sort!=GD_SORT_PARTNERS)) return 0;
    for (i=0;i<model->record_count;++i) if (model->records[i].role==GD_NOUN || model->records[i].role==GD_VERB) {
        if (!model->records[i].name || model->connectivity[i].partners<0) return 0;
        order[count++]=(GridOrder){i,model->records[i].role==GD_VERB,
            sort==GD_SORT_PARTNERS ? model->connectivity[i].partners:0,model->records[i].name};
        if (model->records[i].role==GD_NOUN) ++nouns;
    }
    qsort(order,(size_t)count,sizeof(order[0]),compare_grid_order);
    for (i=0;i<GD_MAX_RECORDS;++i) grid->slot[i]=-1;
    grid->nouns=nouns; grid->verbs=count-nouns;
    for (i=0;i<count;++i) { grid->order[i]=order[i].record; grid->slot[order[i].record]=i<nouns ? i:i-nouns; }
    if (grid->noun<0 || grid->noun>=model->record_count || model->records[grid->noun].role!=GD_NOUN) grid->noun=-1;
    if (grid->verb<0 || grid->verb>=model->record_count || model->records[grid->verb].role!=GD_VERB) grid->verb=-1;
    return 1;
}
int gd_matrix_grid_cell(const GD_Model *model, int noun, int verb) {
    int low=0,high=model->cell_count;
    if (noun<0 || verb<0) return -1;
    while (low<high) {
        int middle=low+(high-low)/2; const GD_Cell *cell=&model->cells[middle];
        if (cell->noun<noun || (cell->noun==noun && cell->verb<verb)) low=middle+1; else high=middle;
    }
    return low<model->cell_count && model->cells[low].noun==noun && model->cells[low].verb==verb ? low:-1;
}
int gd_matrix_grid_hit(const GD_MatrixGrid *grid, float x, float y, int *row, int *column) {
    if (!isfinite(x) || !isfinite(y) || x<0 || y<0 ||
        x>=grid->verbs*(float)GD_MATRIX_COLUMN_WIDTH || y>=grid->nouns*(float)GD_MATRIX_ROW_HEIGHT) return 0;
    *row=(int)(y/GD_MATRIX_ROW_HEIGHT); *column=(int)(x/GD_MATRIX_COLUMN_WIDTH); return 1;
}
GD_MatrixGridRange gd_matrix_grid_range(const GD_MatrixGrid *grid, float left, float top, float right, float bottom) {
    GD_MatrixGridRange range={0};
    if (!isfinite(left) || !isfinite(top) || !isfinite(right) || !isfinite(bottom) || right<=left || bottom<=top) return range;
    /* Clamp floats before conversion, including extreme panned camera values. */
    range.first_row=(int)fminf(grid->nouns,fmaxf(0,floorf(top/GD_MATRIX_ROW_HEIGHT)));
    range.end_row=(int)fminf(grid->nouns,fmaxf(0,ceilf(bottom/GD_MATRIX_ROW_HEIGHT)));
    range.first_column=(int)fminf(grid->verbs,fmaxf(0,floorf(left/GD_MATRIX_COLUMN_WIDTH)));
    range.end_column=(int)fminf(grid->verbs,fmaxf(0,ceilf(right/GD_MATRIX_COLUMN_WIDTH)));
    return range;
}
int gd_matrix_grid_move(GD_MatrixGrid *grid, int rows, int columns) {
    int row,column;
    if (!grid->nouns || !grid->verbs) return 0;
    if (rows<-1 || rows>1 || columns<-1 || columns>1) return 0;
    if (grid->noun<0 || grid->verb<0) row=column=0;
    else {
        row=grid->slot[grid->noun]+rows; column=grid->slot[grid->verb]+columns;
        if (row<0) row=0; if (row>=grid->nouns) row=grid->nouns-1;
        if (column<0) column=0; if (column>=grid->verbs) column=grid->verbs-1;
    }
    grid->noun=grid->order[row]; grid->verb=grid->order[grid->nouns+column]; return 1;
}
