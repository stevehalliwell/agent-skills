#include "gd_matrix_grid.h"
TEST(matrix_grid,
    GD_Model *model=model_create(); GD_MatrixGrid *grid=allocate(storage,sizeof(*grid));
    GD_MatrixGridRange range; int i,row=-1,column=-1;
    TEST_ASSERT(model && grid); memset(grid,0,sizeof(*grid)); grid->noun=grid->verb=-1;
    model->record_count=6;
    for (i=0;i<6;++i) { model->records[i].name="axis"; model->records[i].role=i%2 ? GD_VERB:GD_NOUN; }
    model->records[0].name="Zulu"; model->records[2].name="Alpha"; model->records[4].name="Beta";
    model->records[1].name="Touch"; model->records[3].name="Act"; model->records[5].role=GD_CASE;
    model->connectivity[0].partners=2; model->connectivity[1].partners=1;
    model->cell_count=2; model->cells[0]=(GD_Cell){.noun=0,.verb=1,.pair_count=3,.active=1};
    model->cells[1]=(GD_Cell){.noun=2,.verb=3,.pair_count=1,.active=0};
    TEST_ASSERT(gd_matrix_grid_prepare(grid,model,GD_SORT_NAME));
    TEST_ASSERT(grid->nouns==3 && grid->verbs==2 && grid->order[0]==2 && grid->order[3]==3 && grid->slot[5]==-1);
    TEST_ASSERT(gd_matrix_grid_cell(model,0,1)==0 && gd_matrix_grid_cell(model,2,3)==1 && gd_matrix_grid_cell(model,4,3)==-1);
    TEST_ASSERT(!gd_matrix_grid_hit(grid,-1,0,&row,&column));
    TEST_ASSERT(!gd_matrix_grid_hit(grid,NAN,0,&row,&column));
    TEST_ASSERT(gd_matrix_grid_hit(grid,176,44,&row,&column) && row==1 && column==1);
    TEST_ASSERT(!gd_matrix_grid_hit(grid,352,0,&row,&column) && !gd_matrix_grid_hit(grid,0,132,&row,&column));
    TEST_ASSERT(gd_matrix_grid_move(grid,1,0) && grid->noun==2 && grid->verb==3); /* First, blank or declared. */
    TEST_ASSERT(gd_matrix_grid_move(grid,1,1) && grid->noun==4 && grid->verb==1);
    TEST_ASSERT(gd_matrix_grid_cell(model,grid->noun,grid->verb)==-1);
    TEST_ASSERT(gd_matrix_grid_prepare(grid,model,GD_SORT_PARTNERS) && grid->noun==4 && grid->verb==1 && grid->order[0]==0);
    for (i=0;i<10;++i) TEST_ASSERT(gd_matrix_grid_move(grid,1,1));
    TEST_ASSERT(grid->slot[grid->noun]==2 && grid->slot[grid->verb]==1);
    range=gd_matrix_grid_range(grid,-1e30f,-1e30f,1e30f,1e30f);
    TEST_ASSERT(range.first_row==0 && range.end_row==3 && range.first_column==0 && range.end_column==2);
    range=gd_matrix_grid_range(grid,176,44,352,88);
    TEST_ASSERT(range.first_row==1 && range.end_row==2 && range.first_column==1 && range.end_column==2);
    /* No 512-column limit, and every record can belong to one axis. */
    model->record_count=GD_MAX_RECORDS;
    for (i=0;i<GD_MAX_RECORDS;++i) { model->records[i].role=i==0 ? GD_NOUN:GD_VERB; model->records[i].name="axis"; }
    TEST_ASSERT(gd_matrix_grid_prepare(grid,model,GD_SORT_NAME) && grid->verbs==GD_MAX_RECORDS-1);
    TEST_ASSERT(gd_matrix_grid_hit(grid,(GD_MAX_RECORDS-2)*176.0f,0,&row,&column) && column==GD_MAX_RECORDS-2);
    range=gd_matrix_grid_range(grid,0,0,1e30f,1e30f); TEST_ASSERT(range.end_column==GD_MAX_RECORDS-1);
    model->record_count=0; TEST_ASSERT(gd_matrix_grid_prepare(grid,model,GD_SORT_NAME) && !gd_matrix_grid_move(grid,1,0));
    TEST_ASSERT(!gd_matrix_grid_prepare(grid,model,(GD_LayoutSort)99));
    release(storage,grid); gd_destroy(model);
)
