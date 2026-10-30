#ifndef GD_QUERY_SCENE_H
#define GD_QUERY_SCENE_H
#include "gd_query.h"
#include "gd_layout.h"
enum { GD_QUERY_SCENE_LABEL=192 };
typedef enum { GD_QUERY_ALL_VARIANTS, GD_QUERY_PRIMARY_VARIANT, GD_QUERY_STACKED_VARIANTS } GD_QueryGhostMode;
typedef enum { GD_QUERY_NODE_BINDING, GD_QUERY_NODE_VARIANT, GD_QUERY_NODE_ACTIVATION, GD_QUERY_NODE_EFFECT, GD_QUERY_NODE_PARTNER, GD_QUERY_NODE_COVERAGE } GD_QueryNodeKind;
typedef enum { GD_QUERY_EDGE_ASSUMPTION, GD_QUERY_EDGE_INVOCATION, GD_QUERY_EDGE_EFFECT, GD_QUERY_EDGE_PARTICIPANT, GD_QUERY_EDGE_FRONTIER } GD_QueryEdgeKind;
typedef struct { GD_QueryNodeKind kind; int source,variant,incoming,ghost; GD_RuleTruth applicability; int comparison; } GD_QuerySceneNode;
typedef struct { GD_QueryEdgeKind kind; int source,variant; } GD_QuerySceneEdge;
typedef struct {
    GD_Allocator allocator; const GD_Query *query; const GD_EffectQuery *effect_scope;
    GD_Layout layout;
    GD_QuerySceneNode nodes[GD_MAX_RECORDS]; GD_QuerySceneEdge edges[GD_MAX_PAIRS];
    char labels[GD_MAX_RECORDS][GD_QUERY_SCENE_LABEL];
    GD_QueryGhostMode ghosts; int primary,weighted,include_excluded;
    int work,work_limit,limited,capacity_limited,unexamined_items_upper_bound,label_truncations,excluded_activations;
} GD_QueryScene;
/* Graphics-free presentation of native query evidence. Borrow query until
   destruction; own layout/labels/metadata. No evaluator or source writes.
   Incoming weights only apply to direct source declarations in Effects;
   variant/ghost copies never supply convergence weights. Partial presentation
   remains marked; allocation/invalid input returns NULL, never mutates old scene. */
GD_QueryScene *gd_query_scene_build(const GD_Query *query, GD_QueryGhostMode ghosts, int primary, int weighted, int include_excluded, int work_limit);
/* Exact contributor context plus all direct side effects. Counts use native
   context mask; scope must outlive scene. Wrapped rows are display, not time. */
GD_QueryScene *gd_query_scene_effect_context(const GD_Query *query, const GD_EffectQuery *scope, int weighted, int work_limit);
/* Color membership only; no source topology, semantic facts or counts change. */
int gd_query_scene_compare(GD_QueryScene *scene, int first, int second);
void gd_query_scene_destroy(GD_QueryScene *scene);
/* Snapshot record for source inspection/Find; -1 if no resolved source. */
int gd_query_scene_record(const GD_QueryScene *scene, int node);
/* Reverse painter order: visible non-ghost cards before exposed ghost strips. */
int gd_query_scene_hit(const GD_QueryScene *scene, float x, float y);
#endif
