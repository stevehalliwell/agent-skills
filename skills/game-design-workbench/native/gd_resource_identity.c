#include "gd_resource_graph.h"
#include <string.h>

typedef struct { char *text; size_t size,used; int valid; } Identity;
/* Serializer and reload matching share this exact identity definition. Loaded
   definition indices and resolution status are provenance, not identity. */
typedef struct {
    GD_ResourceNodeKind kind;
    const char *source,*name,*participant,*pool,*axis,*declaration_participant,*declaration_source,*declaration_case;
    int shared,unresolved_kind,unresolved_position,placement,declaration_depth;
    unsigned long long placement_identity[2];
} IdentityKey;
static const char *const kinds[]={"requirement","exclusion","state_write","quantity_write","role_binding","preservation","invocation"};
static int key(const GD_ResourceGraph *graph, int index, int hint, IdentityKey *value) {
    const GD_ResourceNode *node;
    memset(value,0,sizeof(*value)); value->unresolved_kind=-1;
    if (!graph || index<0 || index>=graph->node_count) return 0;
    node=&graph->nodes[index]; value->kind=node->kind;
    value->placement=node->kind==GD_RESOURCE_CASE && node->instance ? index+1:node->placement;
    if (value->placement) {
        value->placement_identity[0]=graph->nodes[value->placement-1].identity[0];
        value->placement_identity[1]=graph->nodes[value->placement-1].identity[1];
    }
    if (node->kind==GD_RESOURCE_ROLE) { value->pool=node->pool_reference; return 1; }
    if (node->kind==GD_RESOURCE_CASE || !node->pool_reference) {
        const GD_RuleCase *item=&graph->rules->cases[node->case_index];
        value->source=graph->model->records[graph->rules->sources[item->source].record].reference; value->name=item->name;
    }
    if (node->kind==GD_RESOURCE_CASE) return 1;
    value->shared=node->pool_reference!=NULL; value->pool=node->pool_reference;
    if (!value->shared) value->participant=graph->rules->participants[node->participant].name;
    if (node->axis>=0) value->axis=graph->model->records[node->axis].reference;
    else {
        const GD_ResourceRelation *edge; const GD_RuleParticipant *actor;
        if (hint<0) for (hint=0;hint<graph->relation_count && graph->relations[hint].facet!=index;++hint) {}
        if (hint>=graph->relation_count || graph->relations[hint].facet!=index) return 0;
        edge=&graph->relations[hint]; actor=&graph->rules->participants[edge->participant]; value->unresolved_kind=edge->kind;
        if (value->placement) {
            const GD_RuleCase *origin=&graph->rules->cases[edge->source_case];
            value->declaration_participant=actor->name; value->declaration_case=origin->name;
            value->declaration_source=graph->model->records[graph->rules->sources[origin->source].record].reference;
            {
                int current=value->placement-1;
                while (graph->nodes[current].case_index!=edge->source_case) {
                    if (!graph->nodes[current].instance || ++value->declaration_depth>=GD_MAX_RULE_EXPANSION_DEPTH) return 0;
                    current=graph->nodes[current].parent;
                }
            }
        }
        switch (edge->kind) {
            case GD_RESOURCE_REQUIREMENT: case GD_RESOURCE_EXCLUSION: value->unresolved_position=graph->rules->condition_tokens[edge->declaration].offset; break;
            case GD_RESOURCE_STATE_WRITE: value->unresolved_position=edge->declaration-actor->first_state_effect; break;
            case GD_RESOURCE_QUANTITY_WRITE: value->unresolved_position=edge->declaration-actor->first_assignment; break;
            case GD_RESOURCE_PRESERVATION: value->unresolved_position=edge->declaration-actor->first_preservation; break;
            default: break;
        }
    }
    return 1;
}
static int same_text(const char *a, const char *b) { return !strcmp(a ? a:"",b ? b:""); }
static int same_placement(const GD_ResourceGraph *a, int left, const GD_ResourceGraph *b, int right) {
    int depth=0;
    if (!left || !right) return left==right;
    --left; --right;
    do {
        const GD_ResourceNode *x=&a->nodes[left],*y=&b->nodes[right];
        const GD_RuleCase *xc=&a->rules->cases[x->case_index],*yc=&b->rules->cases[y->case_index];
        const char *xs=a->model->records[a->rules->sources[xc->source].record].reference;
        const char *ys=b->model->records[b->rules->sources[yc->source].record].reference;
        if (strcmp(xs,ys) || strcmp(xc->name,yc->name) || x->instance!=y->instance) return 0;
        if (!x->instance) return 1;
        if (x->ordinal!=y->ordinal || strcmp(a->rules->participants[a->calls->calls[x->call].caller].name,b->rules->participants[b->calls->calls[y->call].caller].name)) return 0;
        left=x->parent; right=y->parent;
    } while (++depth<GD_MAX_RULE_EXPANSION_DEPTH);
    return 0;
}
static int same_key(const IdentityKey *a, const IdentityKey *b) {
    return a->placement_identity[0]==b->placement_identity[0] && a->placement_identity[1]==b->placement_identity[1] && a->kind==b->kind && a->shared==b->shared && a->unresolved_kind==b->unresolved_kind && a->unresolved_position==b->unresolved_position &&
        same_text(a->source,b->source) && same_text(a->name,b->name) && same_text(a->participant,b->participant) && same_text(a->pool,b->pool) && same_text(a->axis,b->axis) && same_text(a->declaration_participant,b->declaration_participant) && same_text(a->declaration_source,b->declaration_source) && same_text(a->declaration_case,b->declaration_case);
}
int gd_resource_node_remap(const GD_ResourceGraph *old, int node, const GD_ResourceGraph *fresh) {
    IdentityKey before,after; int hints[GD_RESOURCE_GRAPH_NODES],i,found=-1;
    if (!fresh || !key(old,node,-1,&before)) return -1;
    for (i=0;i<fresh->node_count;++i) hints[i]=-1;
    for (i=0;i<fresh->relation_count;++i) if (hints[fresh->relations[i].facet]<0) hints[fresh->relations[i].facet]=i;
    for (i=0;i<fresh->node_count;++i) if (key(fresh,i,hints[i],&after) && same_key(&before,&after) && same_placement(old,before.placement,fresh,after.placement)) {
        if (found>=0) return -1;
        found=i;
    }
    return found;
}
static void append(Identity *id, const char *text, int escaped) {
    const unsigned char *p=(const unsigned char *)(text ? text:""); const char hex[]="0123456789ABCDEF";
    for (;*p && id->valid;++p) {
        int plain=!escaped || (*p>='a' && *p<='z') || (*p>='A' && *p<='Z') || (*p>='0' && *p<='9') || *p=='-' || *p=='_' || *p=='.';
        size_t count=plain ? 1:3;
        if (id->used+count>=id->size) { id->valid=0; break; }
        if (plain) id->text[id->used++]=(char)*p;
        else { id->text[id->used++]='%'; id->text[id->used++]=hex[*p>>4]; id->text[id->used++]=hex[*p&15]; }
    }
}
int gd_resource_node_id(const GD_ResourceGraph *graph, int index, int hint, char *text, size_t size) {
    static const char *const names[]={"case:","facet:","resource_role:"};
    IdentityKey value; Identity id={text,size,0,1};
    if (!text || !size) return 0; text[0]=0;
    if (!key(graph,index,hint,&value)) return 0;
    append(&id,names[value.kind],0);
    if (value.kind==GD_RESOURCE_ROLE) append(&id,value.pool,1);
    else {
        if (value.shared) { append(&id,"pool:",0); append(&id,value.pool,1); }
        else { append(&id,value.source,1); append(&id,"#",0); append(&id,value.name,1); }
        if (value.kind==GD_RESOURCE_FACET) {
            append(&id,":",0);
            if (!value.shared) { append(&id,value.participant,1); append(&id,":",0); }
            if (value.axis) append(&id,value.axis,1);
            else {
                char number[32]; snprintf(number,sizeof(number),"%d",value.unresolved_position);
                append(&id,"unresolved:",0); append(&id,kinds[value.unresolved_kind],0); append(&id,":",0); append(&id,number,0);
            }
        }
    }
    if (value.placement) {
        const GD_ResourceNode *placement=&graph->nodes[value.placement-1]; char number[40];
        snprintf(number,sizeof(number),"%016llx%016llx",placement->identity[0],placement->identity[1]);
        append(&id,"@placement:",0); append(&id,number,0);
        if (value.declaration_participant) {
            /* Placement already identifies every qualified ancestor case.
               Relative origin depth avoids repeating long source names in IDs. */
            char origin[24]; snprintf(origin,sizeof(origin),"%d",value.declaration_depth);
            append(&id,":origin:",0); append(&id,origin,0); append(&id,":",0); append(&id,value.declaration_participant,1);
        }
    }
    if (!id.valid) { text[0]=0; return 0; }
    text[id.used]=0; return 1;
}
