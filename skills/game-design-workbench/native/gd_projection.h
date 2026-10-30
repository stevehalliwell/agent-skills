#ifndef GD_PROJECTION_H
#define GD_PROJECTION_H
#include "gd_explorer.h"
#include "gd_layout.h"
#include "gd_actor_inventory.h"
/* Shared graphics-free topology. Labels/model/project remain borrowed.
   Relationships uses supplied revealed scope; NULL means whole loaded context.
   Geometry is presentation only and does not restrict analytical scope. */
const char *gd_projection_name(GD_ExplorerView view);
int gd_projection_find(const char *name);
const char *gd_resource_label(const GD_ResourceGraph *graph, int index);
int gd_resource_layout(const GD_ResourceGraph *graph, GD_Layout *layout);
/* Canonical inventory topology for result tables; no canvas geometry policy. */
int gd_actor_inventory_layout(const GD_ActorInventory *inventory, GD_Layout *layout);
/* Original six project-owned projections. Actor inventory needs separately
   owned inventory + gd_actor_inventory_layout, prepared by shared result. */
int gd_projection_layout(const GD_ExplorerProject *project, GD_ExplorerView view, const GD_Focus *focus, GD_Layout *layout);
int gd_projection_record(const GD_ExplorerProject *project, GD_ExplorerView view, int key);
/* -1 for derived multi-source cells. Matrix case sources remain in cell pairs. */
int gd_projection_edge_record(const GD_ExplorerProject *project, GD_ExplorerView view, int key);
#endif
