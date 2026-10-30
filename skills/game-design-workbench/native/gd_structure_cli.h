#ifndef GD_STRUCTURE_CLI_H
#define GD_STRUCTURE_CLI_H
#include "gd_projection.h"
#include "gd_structure_query.h"
typedef struct {
    const char *question,*projection,*from,*to,*avoid,*scope_root;
    const char *roots[GD_STRUCTURE_SELECTORS],*relations[GD_STRUCTURE_KINDS],*statuses[GD_STRUCTURE_SELECTORS];
    int root_count,relation_count,status_count,include_inactive,work_limit;
    GD_StructureDirection direction; GD_GraphLens lens;
} GD_StructureOptions;
/* CLI parser supplies explicit collections/original roles. No second parser. */
int gd_structure_cli(GD_Allocator allocator, const GD_Collection *collections, int count,
    const GD_StructureOptions *options, int json, const char *fields, const char *initial_error);
int gd_structure_fields(const char *fields);
int gd_write_structure_catalogue(FILE *output, int json, int comma);
#endif
