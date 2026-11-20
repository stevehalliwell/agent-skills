/* Private spatial projection, not a query/evidence graph. Typed scene remains
   unchanged. Only intentionally stacked variant cards share a geometric unit. */
#ifndef GD_QUERY_SCENE_FREE_H
#define GD_QUERY_SCENE_FREE_H
#include <float.h>
struct GD_QueryFree { GD_Layout layout; int unit[GD_MAX_RECORDS]; };
enum { QUERY_FREE_HASH_SLOTS=GD_MAX_PAIRS*2, QUERY_FREE_HASH_WORK=262144 };
typedef struct { uint64_t key[QUERY_FREE_HASH_SLOTS]; int entry[QUERY_FREE_HASH_SLOTS]; } QueryFreeHash;
typedef char QueryFreeScratchBound[sizeof(QueryFreeHash)+sizeof(((GD_Layout *)0)->collision_order)+sizeof(((GD_Layout *)0)->collision_remaining)<=1048576 ? 1:-1];
static void query_free_scatter(GD_QueryScene *scene) {
    GD_QueryFree *free=scene->free_geometry; int i;
    for(i=0;i<scene->layout.node_count;++i) {
        GD_LayoutNode *node=&scene->layout.nodes[i];const GD_LayoutNode *unit=&free->layout.nodes[free->unit[i]];int anchor=unit->key;
        node->x=unit->x+(scene->native_points[i].x-scene->native_points[anchor].x);
        node->y=unit->y+(scene->native_points[i].y-scene->native_points[anchor].y);node->vx=unit->vx;node->vy=unit->vy;
    }
    if(scene->layout.group_count) {GD_LayoutGroup *group=&scene->layout.groups[0];const GD_LayoutGroup *spatial=&free->layout.groups[0];group->x=spatial->x;group->y=spatial->y;group->width=spatial->width;group->height=spatial->height;}
    scene->layout.width=free->layout.width;scene->layout.height=free->layout.height;scene->layout.tick=free->layout.tick;scene->layout.workspace=free->layout.workspace;
    scene->layout.islands.preferred=free->layout.islands.preferred;scene->layout.islands.enabled=free->layout.islands.enabled;
    scene->layout.islands.count=free->layout.islands.count;scene->layout.islands.limited=free->layout.islands.limited;scene->layout.islands.checks=free->layout.islands.checks;scene->layout.islands.overlaps=free->layout.islands.overlaps;
    if(free->layout.islands.enabled)for(i=0;i<scene->layout.node_count;++i)scene->layout.islands.component[i]=free->layout.islands.component[free->unit[i]];
    scene->layout.collision_checks=free->layout.collision_checks;scene->layout.collision_moves=free->layout.collision_moves;scene->layout.collision_limited=free->layout.collision_limited;
}
static unsigned query_free_slot(uint64_t key) {
    key^=key>>33;key*=UINT64_C(0xff51afd7ed558ccd);key^=key>>33;key*=UINT64_C(0xc4ceb9fe1a85ec53);key^=key>>33;
    return (unsigned)key&(QUERY_FREE_HASH_SLOTS-1);
}
int gd_query_scene_place(GD_QueryScene *scene,GD_LayoutMode mode,int workspace,float aspect) {
    GD_QueryFree *fresh;QueryFreeHash *hash=NULL;GD_Allocator allocator;int i,primary=-1;unsigned work=0;
    if(scene && scene->overview && scene->overview->mode==GD_ACTOR_INTERACTIONS) mode=GD_LAYOUT_FORCE;
    if(!scene || (mode!=GD_LAYOUT_FORCE && mode!=GD_LAYOUT_DIRECTED) || (workspace!=0 && workspace!=1) || !isfinite(aspect) || aspect<=0) return 0;
    if(scene->layout.node_count<0 || scene->layout.node_count>GD_MAX_RECORDS || scene->layout.edge_count<0 || scene->layout.edge_count>GD_MAX_PAIRS || scene->layout.group_count!=(scene->layout.node_count ? 1:0)) return 0;
    for(i=0;i<scene->layout.node_count;++i) if(scene->layout.nodes[i].key!=i || !isfinite(scene->native_points[i].x) || !isfinite(scene->native_points[i].y)) return 0;
    for(i=0;i<scene->layout.edge_count;++i) if(scene->layout.edges[i].a<0 || scene->layout.edges[i].a>=scene->layout.node_count || scene->layout.edges[i].b<0 || scene->layout.edges[i].b>=scene->layout.node_count) return 0;
    allocator=scene->allocator;
    if(!allocator.allocate || !allocator.release) return 0;
    if(mode==GD_LAYOUT_DIRECTED) {
        for(i=0;i<scene->layout.node_count;++i) {scene->layout.nodes[i].x=scene->native_points[i].x;scene->layout.nodes[i].y=scene->native_points[i].y;scene->layout.nodes[i].vx=scene->layout.nodes[i].vy=0;}
        scene->layout.groups[0]=scene->native_group;scene->layout.width=scene->native_width;scene->layout.height=scene->native_height;scene->layout.mode=mode;
        scene->layout.tick=scene->layout.collision_cursor=scene->layout.collision_checks=scene->layout.collision_moves=0;scene->layout.collision_limited=0;memset(&scene->layout.workspace,0,sizeof(scene->layout.workspace));gd_layout_islands_set(&scene->layout,0);
        if(scene->free_geometry) allocator.release(allocator.context,scene->free_geometry);scene->free_geometry=NULL;return 1;
    }
    fresh=allocator.allocate(allocator.context,sizeof(*fresh));if(!fresh) return 0;memset(fresh,0,sizeof(*fresh));
    if(scene->ghosts==GD_QUERY_STACKED_VARIANTS) for(i=0;i<scene->layout.node_count;++i)
        if(scene->nodes[i].kind==GD_QUERY_NODE_VARIANT && scene->nodes[i].variant==scene->primary) primary=i;
    /* Roots first, then map folded cards. Original scene index remains unit key. */
    for(i=0;i<scene->layout.node_count;++i) {
        if(primary>=0 && scene->nodes[i].kind==GD_QUERY_NODE_VARIANT && i!=primary) {fresh->unit[i]=-1;continue;}
        fresh->unit[i]=fresh->layout.node_count;fresh->layout.nodes[fresh->layout.node_count++]=scene->layout.nodes[i];
    }
    for(i=0;i<scene->layout.node_count;++i) if(fresh->unit[i]<0) fresh->unit[i]=fresh->unit[primary];
    for(i=0;i<fresh->layout.node_count;++i) {
        GD_LayoutNode *node=&fresh->layout.nodes[i];node->group=0;node->degree=0;node->footprint=(GD_LayoutBounds){FLT_MAX,FLT_MAX,-FLT_MAX,-FLT_MAX};
    }
    for(i=0;i<scene->layout.node_count;++i) {
        GD_LayoutNode *unit=&fresh->layout.nodes[fresh->unit[i]];const GD_LayoutNode *member=&scene->layout.nodes[i];
        float dx=scene->native_points[i].x-scene->native_points[unit->key].x,dy=scene->native_points[i].y-scene->native_points[unit->key].y;
        unit->footprint.left=fminf(unit->footprint.left,dx+member->footprint.left);unit->footprint.top=fminf(unit->footprint.top,dy+member->footprint.top);
        unit->footprint.right=fmaxf(unit->footprint.right,dx+member->footprint.right);unit->footprint.bottom=fmaxf(unit->footprint.bottom,dy+member->footprint.bottom);
    }
    if(scene->layout.edge_count) {hash=allocator.allocate(allocator.context,sizeof(*hash));if(!hash) goto failed;memset(hash,0,sizeof(*hash));}
    for(i=0;i<scene->layout.edge_count;++i) {
        GD_LayoutEdge edge=scene->layout.edges[i];int folded,index,entry;uint64_t key;unsigned slot;
        int a=fresh->unit[edge.a],b=fresh->unit[edge.b];
        folded=fresh->layout.nodes[a].key!=edge.a || fresh->layout.nodes[b].key!=edge.b;edge.a=a;edge.b=b;
        if(a==b) continue;
        key=(((uint64_t)(unsigned)edge.relation<<32)|((uint64_t)(unsigned)a<<16)|(unsigned)b)+1;slot=query_free_slot(key);
        for(;;) {if(++work>QUERY_FREE_HASH_WORK) goto failed;if(!hash->key[slot] || hash->key[slot]==key) break;slot=(slot+1)&(QUERY_FREE_HASH_SLOTS-1);}
        entry=hash->entry[slot];
        if(entry && folded) continue; /* Ghost copies cannot add parallel spring weight. */
        if(entry<0) {index=-entry-1;fresh->layout.edges[index]=edge;hash->entry[slot]=index+1;continue;}
        index=fresh->layout.edge_count++;fresh->layout.edges[index]=edge;
        if(!entry) {hash->key[slot]=key;hash->entry[slot]=folded ? -index-1:index+1;}
    }
    if(hash) {allocator.release(allocator.context,hash);hash=NULL;}
    for(i=0;i<fresh->layout.edge_count;++i) {++fresh->layout.nodes[fresh->layout.edges[i].a].degree;++fresh->layout.nodes[fresh->layout.edges[i].b].degree;}
    fresh->layout.group_count=fresh->layout.node_count ? 1:0;
    if(!gd_layout_arrange(&fresh->layout,GD_LAYOUT_FORCE,GD_SORT_NAME)) goto failed;
    {
        float left=FLT_MAX,top=FLT_MAX,right=-FLT_MAX,bottom=-FLT_MAX;
        for(i=0;i<fresh->layout.node_count;++i) {
            GD_LayoutNode *node=&fresh->layout.nodes[i];node->x=scene->native_points[node->key].x;node->y=scene->native_points[node->key].y;node->vx=node->vy=0;
            left=fminf(left,node->x+node->footprint.left);top=fminf(top,node->y+node->footprint.top);right=fmaxf(right,node->x+node->footprint.right);bottom=fmaxf(bottom,node->y+node->footprint.bottom);
        }
        if(fresh->layout.node_count) {GD_LayoutGroup *group=&fresh->layout.groups[0];group->x=left-16;group->y=top-16;group->width=right-left+32;group->height=bottom-top+32;fresh->layout.width=group->width;fresh->layout.height=group->height;}
    }
    if(!gd_layout_prepare_free(&fresh->layout,allocator,aspect,workspace))goto failed;
    if(scene->free_geometry) allocator.release(allocator.context,scene->free_geometry);
    scene->free_geometry=fresh;scene->layout.mode=GD_LAYOUT_FORCE;query_free_scatter(scene);return 1;
failed:
    if(hash) allocator.release(allocator.context,hash);allocator.release(allocator.context,fresh);return 0;
}
int gd_query_scene_anchor(const GD_QueryScene *scene,int node) {
    if(!scene || node<0 || node>=scene->layout.node_count)return -1;
    return scene->free_geometry ? scene->free_geometry->layout.nodes[scene->free_geometry->unit[node]].key:node;
}
int gd_query_scene_islands(GD_QueryScene *scene,int enabled){
    if(!scene || !scene->free_geometry)return 0;
    if(!gd_layout_islands_set(&scene->free_geometry->layout,enabled))return 0;
    query_free_scatter(scene);return 1;
}
int gd_query_scene_workspace(GD_QueryScene *scene,int enabled,float aspect) {
    if(!scene)return 0;
    if(!scene->free_geometry)return enabled ? 0:gd_layout_workspace(&scene->layout,0,aspect);
    if(!gd_layout_workspace(&scene->free_geometry->layout,enabled,aspect))return 0;
    query_free_scatter(scene);return 1;
}
void gd_query_scene_step(GD_QueryScene *scene,int held_node) {
    if(!scene || !scene->free_geometry) return;
    gd_layout_step_weighted(&scene->free_geometry->layout,held_node>=0 && held_node<scene->layout.node_count ? scene->free_geometry->unit[held_node]:-1,scene->overview ? scene->overview->counts:NULL);query_free_scatter(scene);
}
void gd_query_scene_drag(GD_QueryScene *scene,int node,float x,float y) {
    GD_QueryFree *free;
    if(!scene || node<0 || node>=scene->layout.node_count || !isfinite(x) || !isfinite(y) || (scene->overview && scene->overview->mode==GD_ACTOR_USAGE && !scene->free_geometry)) return;
    gd_layout_drag_node(&scene->layout,node,x,y);free=scene->free_geometry;
    if(free) {
        int index=free->unit[node],anchor=free->layout.nodes[index].key;
        gd_layout_move_node(&free->layout,index,scene->layout.nodes[node].x-(scene->native_points[node].x-scene->native_points[anchor].x),scene->layout.nodes[node].y-(scene->native_points[node].y-scene->native_points[anchor].y));query_free_scatter(scene);
    }
}
#endif
