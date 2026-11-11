#ifndef GD_QUERY_SCENE_H
#define GD_QUERY_SCENE_H
#include "gd_query.h"
#include "gd_layout.h"
enum { GD_QUERY_SCENE_LABEL=192 };
typedef enum { GD_QUERY_ALL_VARIANTS, GD_QUERY_PRIMARY_VARIANT, GD_QUERY_STACKED_VARIANTS } GD_QueryGhostMode;
typedef enum { GD_QUERY_NODE_BINDING, GD_QUERY_NODE_VARIANT, GD_QUERY_NODE_ACTIVATION, GD_QUERY_NODE_EFFECT, GD_QUERY_NODE_PARTNER, GD_QUERY_NODE_COVERAGE, GD_QUERY_NODE_OBJECT, GD_QUERY_NODE_AXIS } GD_QueryNodeKind;
typedef enum { GD_QUERY_EDGE_ASSUMPTION, GD_QUERY_EDGE_INVOCATION, GD_QUERY_EDGE_EFFECT, GD_QUERY_EDGE_PARTICIPANT, GD_QUERY_EDGE_FRONTIER, GD_QUERY_EDGE_INTERACTION, GD_QUERY_EDGE_USAGE, GD_QUERY_EDGE_AXIS } GD_QueryEdgeKind;
typedef struct { GD_QueryNodeKind kind; int source,variant,incoming,ghost; GD_RuleTruth applicability; int comparison; } GD_QuerySceneNode;
typedef struct { GD_QueryEdgeKind kind; int source,variant; } GD_QuerySceneEdge;
typedef struct GD_QueryFree GD_QueryFree;
typedef struct GD_ActorOverview GD_ActorOverview;
typedef enum { GD_ACTOR_INTERACTIONS, GD_ACTOR_USAGE } GD_ActorOverviewMode;
typedef struct {
    GD_Allocator allocator; const GD_Query *query; const GD_EffectQuery *effect_scope;
    GD_Layout layout;
    GD_QuerySceneNode nodes[GD_MAX_RECORDS]; GD_QuerySceneEdge edges[GD_MAX_PAIRS];
    char labels[GD_MAX_RECORDS][GD_QUERY_SCENE_LABEL];
    GD_LayoutPoint native_points[GD_MAX_RECORDS]; GD_LayoutGroup native_group;
    float native_width,native_height; GD_QueryFree *free_geometry;
    GD_ActorOverview *overview;
    GD_QueryGhostMode ghosts; int primary,weighted,include_excluded;
    int work,work_limit,limited,capacity_limited,unexamined_items_upper_bound,label_truncations,excluded_activations;
} GD_QueryScene;
/* Graphics-free presentation of native query evidence. Borrow query until
   destruction; own layout/labels/metadata. No evaluator or source writes.
   Binding query presents loaded noun definitions with distinct local-binding
   counts, including unused definitions; no global runtime actor merge.
   Incoming weights only apply to direct source declarations in Effects;
   variant/ghost copies never supply convergence weights. Partial presentation
   remains marked; allocation/invalid input returns NULL, never mutates old scene. */
GD_QueryScene *gd_query_scene_build(const GD_Query *query, GD_QueryGhostMode ghosts, int primary, int weighted, int include_excluded, int work_limit);
/* Explicit-actor overview over a binding query; no query evaluation or filtering
   changes. Interaction records only, same-case pairs, record-deduplicated counts.
   Cloud uses actors' own declared axes; no referenced-mechanic/call expansion. */
GD_QueryScene *gd_query_scene_overview(const GD_Query *query,GD_ActorOverviewMode mode,int work_limit);
int gd_query_scene_overview_mode(const GD_QueryScene *scene); /* -1 for query scenes. */
int gd_query_scene_overview_count(const GD_QueryScene *scene,int edge);
int gd_query_scene_overview_case(const GD_QueryScene *scene,int edge,int case_index);
int gd_query_scene_overview_unclassified(const GD_QueryScene *scene);
int gd_query_scene_overview_incomplete(const GD_QueryScene *scene);
int gd_query_scene_overview_write(FILE *out,const GD_QueryScene *scene);
/* Canonical finite actor topology: all retained variants/excluded checks,
   independent of drawing controls. Caller allocator owns scene, query borrowed. */
GD_QueryScene *gd_query_scene_actor_topology(GD_Allocator allocator, const GD_Query *query, int work_limit);
const char *gd_query_scene_node_kind_name(GD_QueryNodeKind kind);
const char *gd_query_scene_edge_kind_name(GD_QueryEdgeKind kind);
/* Exact contributor context plus all direct side effects. Counts use native
   context mask; scope must outlive scene. Wrapped rows are display, not time. */
GD_QueryScene *gd_query_scene_effect_context(const GD_Query *query, const GD_EffectQuery *scope, int weighted, int work_limit);
/* Color membership only; no source topology, semantic facts or counts change. */
int gd_query_scene_compare(GD_QueryScene *scene, int first, int second);
/* Geometry only: retain typed scene/query/metadata. Native restores build pose.
   Free uses shared solver with compound stacked-variant footprints. Candidate
   allocation/work failure leaves prior geometry intact; no per-frame allocation. */
int gd_query_scene_place(GD_QueryScene *scene,GD_LayoutMode mode,int neighborhood,float aspect);
/* Optional shared workspace over spatial units, including compound stacks. */
int gd_query_scene_workspace(GD_QueryScene *scene,int enabled,float aspect);
void gd_query_scene_step(GD_QueryScene *scene,int held_node);
void gd_query_scene_drag(GD_QueryScene *scene,int node,float x,float y);
void gd_query_scene_destroy(GD_QueryScene *scene);
/* Snapshot record for source inspection/Find; -1 if no resolved source. */
int gd_query_scene_record(const GD_QueryScene *scene, int node);
/* Reverse painter order: visible non-ghost cards before exposed ghost strips. */
int gd_query_scene_hit(const GD_QueryScene *scene, float x, float y);
/* Same typed hit ordering, supplied presentation geometry only. */
int gd_query_scene_hit_layout(const GD_QueryScene *scene,const GD_Layout *layout,float x,float y);
/* Visible anchor of compound spatial unit; no geometry mutation. */
int gd_query_scene_anchor(const GD_QueryScene *scene,int node);
#endif
