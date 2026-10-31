#ifndef GD_NAMED_QUERY_H
#define GD_NAMED_QUERY_H
#include "gd_explorer.h"
#include "gd_resource_routes.h"
#include "gd_skill_unlocks.h"
#define GD_NAMED_SELECTORS 64
typedef enum { GD_NAMED_OUTCOMES,GD_NAMED_CONSISTENCY,GD_NAMED_SKILLS,GD_NAMED_RESOURCE_DECLARATIONS,GD_NAMED_RESOURCE_ROUTES,GD_NAMED_COUNT } GD_NamedKind;
typedef struct {
    GD_NamedKind kind; const char *projection,*case_name,*from,*to;
    const char *assumptions[GD_NAMED_SELECTORS],*contexts[GD_NAMED_SELECTORS],*known[GD_NAMED_SELECTORS];
    GD_RuleTruth context_truths[GD_NAMED_SELECTORS];
    int assumption_count,context_count,known_count,accepted_only,work_limit; /* -1 native default; fixed-budget operations reject overrides. */
} GD_NamedRequest;
typedef struct {
    GD_Allocator allocator; const GD_ExplorerProject *project; const GD_Model *model; GD_NamedRequest request;
    GD_RuleAnalysis *analysis; /* Private mutable slots; project->calls never used as result storage. */
    GD_RuleScenario scenario;
    GD_SkillUnlocks *skills; GD_ResourceRoutes *routes;
    int root,selector_work; unsigned revision; size_t owned_bytes;
} GD_NamedQuery;
const char *gd_named_name(GD_NamedKind kind);
int gd_named_kind(const char *name);
const char *gd_named_projection(GD_NamedKind kind);
int gd_named_fields(GD_NamedKind kind,const char *fields);
int gd_write_named_catalogue(FILE *out,int json,int comma);
/* Single-threaded native adapter. Borrow project and request strings until
   destruction; source records/rules stay unchanged. Any skill draft edit
   invalidates retained skill results; rebuild before reading borrowed model. Legacy parser diagnostics
   are captured/forwarded/restored before return. Caller owns candidate; no
   existing result mutated on failure. Native writers/algorithms remain sole
   source of outcome, consistency, knowledge and Resource semantics. */
GD_NamedQuery *gd_named_build(GD_Model *diagnostic,const GD_ExplorerProject *project,const GD_NamedRequest *request);
void gd_named_destroy(GD_NamedQuery *query);
/* Native report nested intact; fields prune native_result.data, not envelope. */
int gd_write_named_result(FILE *out,const GD_NamedQuery *query,int json,const char *fields);
int gd_named_cli(GD_Allocator allocator,const GD_Collection *collections,int count,const GD_NamedRequest *request,int json,const char *fields,const char *initial_error);
#endif
