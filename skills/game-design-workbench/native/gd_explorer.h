#ifndef GD_EXPLORER_H
#define GD_EXPLORER_H
#include "gd_resource_graph.h"
#include "gd_skill_unlocks.h"

/* Existing typed readers remain independent. Snapshot families borrow no memory
   from another family; qualified source references bridge their local indices. */
typedef enum { GD_EXPLORER_SKILLS, GD_EXPLORER_CHALLENGES, GD_EXPLORER_RELATIONSHIPS,
               GD_EXPLORER_MATRIX, GD_EXPLORER_MDA, GD_EXPLORER_RESOURCES, GD_EXPLORER_VIEWS } GD_ExplorerView;
typedef enum { GD_EXPLORER_SKILL_DATA, GD_EXPLORER_MATRIX_DATA, GD_EXPLORER_MDA_DATA,
               GD_EXPLORER_RULE_DATA, GD_EXPLORER_FAMILIES } GD_ExplorerFamily;
typedef struct {
    GD_Allocator allocator;
    GD_Model *models[GD_EXPLORER_FAMILIES];
    GD_Draft *draft;
    GD_Routes *routes;
    GD_Mda *mda;
    GD_Rules *rules;
    GD_RuleAnalysis *calls;
    GD_ResourceGraph *resources;
    unsigned available;
    unsigned revision;
} GD_ExplorerProject;
extern const char *const gd_explorer_view_names[GD_EXPLORER_VIEWS];
GD_ExplorerFamily gd_explorer_family(GD_ExplorerView view);
/* Explicit role bindings only; no path guessing, registry or schema mutation.
   Return an owned partial object on load failure for diagnostics; check errors
   before use. Destroy accepts NULL and partially constructed projects. */
GD_ExplorerProject *gd_explorer_load(GD_Allocator allocator, const GD_Collection *collections, int count);
void gd_explorer_destroy(GD_ExplorerProject *project);
GD_Model *gd_explorer_model(const GD_ExplorerProject *project, GD_ExplorerView view);
const GD_Error *gd_explorer_error(const GD_ExplorerProject *project);
int gd_explorer_record(const GD_Model *model, const char *reference);
int gd_explorer_resource_record(const GD_ResourceGraph *graph, int node);
/* Exact source/case/participant/axis/pool identity; -1 when gone or ambiguous. */
int gd_explorer_resource_remap(const GD_ResourceGraph *old, const GD_ResourceGraph *fresh, int node);
/* Recompute derived skill results after an edit, not an additional evaluator. */
int gd_explorer_edited(GD_ExplorerProject *project);
#ifdef GD_WITH_GRAPH
int gd_explorer_view(GD_Model *diagnostic, const GD_Collection *collections, int count,
                     int initial_view, int matrix_layout, int frames, const char *screenshot, const char *output_path);
#endif
#endif
