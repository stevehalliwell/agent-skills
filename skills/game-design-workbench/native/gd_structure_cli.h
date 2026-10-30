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
typedef enum { GD_DECLARATION_RESOLVED, GD_DECLARATION_MISSING, GD_DECLARATION_UNLOADED, GD_DECLARATION_AMBIGUOUS, GD_DECLARATION_STATES } GD_DeclarationIdentity;
typedef struct {
    int rows,counts[GD_DECLARATION_STATES];
    unsigned char identities[GD_MAX_EDGES];
} GD_DeclarationGaps;
const char *gd_declaration_identity_name(GD_DeclarationIdentity identity);
typedef struct {
    GD_Allocator allocator; const GD_ExplorerProject *project; GD_ExplorerView view;
    GD_StructureOptions options; /* Strings remain borrowed until destruction. */
    GD_Layout *layout; GD_StructureQuery *query;
    GD_DeclarationGaps *gaps; /* Whole loaded reader context; no graph filters. */
    GD_ActorInventory *inventory; /* Canonical declarations, not finite hypotheses. */
    int selector_work,source_incomplete;
} GD_StructureResult;
/* Shared native preparation for CLI and viewer. Owns canonical projection and
   query; borrows immutable project/options strings. Diagnostic receives errors.
   Destroy result before project. Failure retains no partial owned allocation. */
GD_StructureResult *gd_structure_result_build(GD_Model *diagnostic, const GD_ExplorerProject *project, const GD_StructureOptions *options);
void gd_structure_result_destroy(GD_StructureResult *result);
int gd_structure_result_record(const GD_StructureResult *result, int key);
int gd_structure_result_edge_record(const GD_StructureResult *result, int key);
int gd_structure_result_node_id(const GD_StructureResult *result, int key, char *text, size_t size);
int gd_write_structure_result(FILE *output, const GD_StructureResult *result, int json, const char *fields);
/* CLI parser supplies explicit collections/original roles. No second parser. */
int gd_structure_cli(GD_Allocator allocator, const GD_Collection *collections, int count,
    const GD_StructureOptions *options, int json, const char *fields, const char *initial_error);
int gd_structure_fields(const char *fields);
int gd_write_structure_catalogue(FILE *output, int json, int comma);
#endif
