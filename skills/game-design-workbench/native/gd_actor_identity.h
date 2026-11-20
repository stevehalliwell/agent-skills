#ifndef GD_ACTOR_IDENTITY_H
#define GD_ACTOR_IDENTITY_H
#include "gd_query_scene.h"
/* State-specific opaque semantic identity, not scene/variant/declaration array
   offsets. Borrow finite actor scene/query/snapshots. Charge framed bytes and
   native call/write lookup rows to caller budget; never mutate query. Return 0
   with empty text for invalid/unavailable/overlong/limited identity. Caller must
   supplement reload matching with exact source/body/type checks; authored call
   and effect ordinals identify declarations, not durable runtime actors. */
int gd_actor_scene_node_id(const GD_QueryScene *scene, int node, char *text, size_t size, int *work, int work_limit);
#endif
