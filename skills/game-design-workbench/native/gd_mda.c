#include "gd_layout.h"
#include <assert.h>
#include <string.h>

int gd_mda_role(GD_Role role) { return role >= GD_MDA_MECHANIC && role <= GD_MDA_AESTHETIC; }
const char *gd_mda_role_name(GD_Role role) {
    switch (role) {
        case GD_MDA_MECHANIC: return "mechanics";
        case GD_MDA_SYSTEM: return "systems";
        case GD_MDA_DYNAMIC: return "dynamics";
        case GD_MDA_AESTHETIC: return "aesthetics";
        default: return "context";
    }
}
const char *gd_mda_resolution_name(GD_MdaResolution resolution) {
    switch (resolution) {
        case GD_MDA_RESOLVED: return "resolved";
        case GD_MDA_UNLOADED: return "unloaded";
        case GD_MDA_MISSING: return "missing";
        case GD_MDA_WRONG_ROLE: return "wrong_role";
        case GD_MDA_UNAVAILABLE: return "unavailable";
        default: return "ambiguous";
    }
}
const char *gd_mda_direction_name(GD_MdaDirection direction) {
    return direction == GD_MDA_FORWARD ? "forward" : direction == GD_MDA_BACKWARD ? "backward" : "both";
}
static int expected_role(GD_Role source, GD_Field field) {
    if ((source == GD_MDA_SYSTEM || source == GD_MDA_DYNAMIC) && field == GD_MECHANICS) return GD_MDA_MECHANIC;
    if (source == GD_MDA_DYNAMIC && field == GD_SYSTEMS) return GD_MDA_SYSTEM;
    if (source == GD_MDA_AESTHETIC && field == GD_DYNAMICS) return GD_MDA_DYNAMIC;
    return -1;
}
static GD_MdaResolution resolve(const GD_Model *model, const GD_Edge *edge, int expected) {
    int i; const char *slash;
    if (edge->target >= 0) return model->records[edge->target].role == (GD_Role)expected ? GD_MDA_RESOLVED:GD_MDA_WRONG_ROLE;
    if (edge->target == -2) return GD_MDA_AMBIGUOUS;
    slash = strchr(edge->reference,'/');
    if (!slash || slash == edge->reference || !slash[1]) return GD_MDA_MISSING;
    for (i = 0; i < model->collection_count; ++i)
        if (strlen(model->collections[i].alias) == (size_t)(slash-edge->reference) &&
            !memcmp(model->collections[i].alias,edge->reference,(size_t)(slash-edge->reference)))
            return model->error_count || model->errors_omitted ? GD_MDA_UNAVAILABLE:GD_MDA_MISSING;
    return GD_MDA_UNLOADED;
}
GD_Mda *gd_analyze_mda(GD_Model *model) {
    GD_Mda *mda = model->allocator.allocate(model->allocator.context,sizeof(*mda));
    int i, e, queue[GD_MAX_RECORDS], first = 0, last = 0;
    if (!mda) { gd_error(model,"input.memory","Cannot allocate bounded MDA analysis",NULL,0); return NULL; }
    memset(mda,0,sizeof(*mda)); mda->allocator = model->allocator; mda->root = -1;
    for (i = 0; i < GD_MAX_RECORDS; ++i) { mda->outgoing[i] = mda->incoming[i] = -1; mda->nodes[i].aesthetic = -1; }
    if (!model->mda_mode) { gd_error(model,"mda.collections","MDA analysis requires explicit MDA collection roles",NULL,0); return mda; }
    mda->graph_complete = !model->error_count && !model->errors_omitted;
    /* Bucket original edges by sorted source record. Preserve authored list
       order within a record; filesystem enumeration cannot choose witnesses. */
    for (e = model->edge_count-1; e >= 0; --e) {
        i = model->edges[e].source; mda->next_out[e] = mda->outgoing[i]; mda->outgoing[i] = e;
    }
    for (i = 0; i < model->record_count; ++i) if (gd_mda_role(model->records[i].role)) {
        ++mda->node_count;
        for (e = mda->outgoing[i]; e >= 0; e = mda->next_out[e]) {
            const GD_Edge *edge = &model->edges[e]; int expected = expected_role(model->records[i].role,edge->field);
            GD_MdaLink *link;
            if (expected < 0) continue;
            assert(mda->link_count < GD_MAX_EDGES);
            link = &mda->links[mda->link_count++]; link->edge = e; link->resolution = resolve(model,edge,expected);
            ++mda->nodes[i].basis_declared;
            if (link->resolution == GD_MDA_RESOLVED) ++mda->nodes[i].basis_resolved;
            else mda->graph_complete = 0;
        }
    }
    for (i = 0; i < model->record_count; ++i) mda->outgoing[i] = -1;
    for (i = mda->link_count-1; i >= 0; --i) {
        const GD_Edge *edge = &model->edges[mda->links[i].edge];
        mda->next_out[i] = mda->outgoing[edge->source]; mda->outgoing[edge->source] = i;
        if (mda->links[i].resolution == GD_MDA_RESOLVED) { mda->next_in[i] = mda->incoming[edge->target]; mda->incoming[edge->target] = i; }
    }
    /* Multi-source authored-basis traversal proves a DECLARED aesthetic path,
       not causality or an observed emotional response. */
    for (i = 0; i < model->record_count; ++i) if (model->records[i].role == GD_MDA_AESTHETIC) {
        mda->nodes[i].aesthetic = i; queue[last++] = i;
    }
    while (first < last) {
        int v = queue[first++];
        for (i = mda->outgoing[v]; i >= 0; i = mda->next_out[i]) if (mda->links[i].resolution == GD_MDA_RESOLVED) {
            int target = model->edges[mda->links[i].edge].target;
            if (mda->nodes[target].aesthetic >= 0) continue;
            assert(last < GD_MAX_RECORDS); mda->nodes[target].aesthetic = mda->nodes[v].aesthetic; queue[last++] = target;
        }
    }
    gd_mda_query(model,mda,-1,GD_MDA_BOTH); return mda;
}
void gd_mda_destroy(GD_Mda *mda) { if (mda) mda->allocator.release(mda->allocator.context,mda); }
int gd_mda_find(const GD_Model *model, const char *reference) {
    int i, found = -2;
    for (i = 0; i < model->record_count; ++i) if (!strcmp(reference,model->records[i].reference)) {
        if (found >= 0 || !gd_mda_role(model->records[i].role)) return -2;
        found = i;
    }
    return found;
}
const char *gd_mda_basis(const GD_Model *model, const GD_Mda *mda, int record) {
    const GD_MdaNode *node = &mda->nodes[record];
    return model->records[record].role == GD_MDA_MECHANIC ? "not_applicable" :
        !node->basis_declared ? "none_declared" : node->basis_declared == node->basis_resolved ? "resolved":"incomplete";
}
/* ponytail: unresolved core context makes every negative trace unknown.
   Narrow uncertainty only with a proved scope for unloaded arrangements. */
const char *gd_mda_trace(const GD_Mda *mda, int record) {
    return mda->nodes[record].aesthetic >= 0 ? "declared_trace" : mda->graph_complete ? "none_declared":"unknown";
}
static void finding(GD_Mda *mda, int record, const char *code, const char *message, const char *field, const char *reference, int line) {
    GD_Finding *item;
    if (mda->finding_count == GD_MAX_ERRORS) { ++mda->findings_omitted; return; }
    item = &mda->findings[mda->finding_count++]; item->record = record; item->code = code; item->message = message;
    item->field = field; item->reference = reference; item->line = line;
}
static void visit(GD_Mda *mda, int v, int target, int link, int *queue, int *last) {
    if (mda->selected[target]) return;
    assert(*last < GD_MAX_RECORDS); mda->selected[target] = 1;
    mda->distance[target] = mda->distance[v]+1; mda->parent[target] = v; mda->parent_link[target] = link;
    queue[(*last)++] = target;
}
/* ponytail: retain one shortest parent witness, not combinatorial path lists.
   All raw links remain available if explicit alternative-path selection is added. */
int gd_mda_query(const GD_Model *model, GD_Mda *mda, int root, GD_MdaDirection direction) {
    int i, queue[GD_MAX_RECORDS], first = 0, last = 0;
    mda->root = root; mda->direction = direction; mda->selected_count = mda->query_complete = 0;
    mda->gap_count = mda->finding_count = mda->findings_omitted = 0;
    memset(mda->selected,0,sizeof(mda->selected));
    for (i = 0; i < GD_MAX_RECORDS; ++i) mda->distance[i] = mda->parent[i] = mda->parent_link[i] = -1;
    if (root < -1 || root >= model->record_count ||
        (root >= 0 && !gd_mda_role(model->records[root].role)) || direction < GD_MDA_BOTH || direction > GD_MDA_BACKWARD) return 0;
    if (root < 0) {
        for (i = 0; i < model->record_count; ++i) if (gd_mda_role(model->records[i].role)) mda->selected[i] = 1;
    } else {
        mda->selected[root] = 1; mda->distance[root] = 0; queue[last++] = root;
        while (first < last) {
            int v = queue[first++];
            if (direction != GD_MDA_FORWARD) for (i = mda->outgoing[v]; i >= 0; i = mda->next_out[i])
                if (mda->links[i].resolution == GD_MDA_RESOLVED) visit(mda,v,model->edges[mda->links[i].edge].target,i,queue,&last);
            if (direction != GD_MDA_BACKWARD) for (i = mda->incoming[v]; i >= 0; i = mda->next_in[i])
                visit(mda,v,model->edges[mda->links[i].edge].source,i,queue,&last);
        }
    }
    for (i = 0; i < mda->link_count; ++i) {
        const GD_MdaLink *link = &mda->links[i]; const GD_Edge *edge = &model->edges[link->edge];
        if (!mda->selected[edge->source] || link->resolution == GD_MDA_RESOLVED) continue;
        finding(mda,edge->source,link->resolution == GD_MDA_UNLOADED ? "mda.reference_unloaded" : link->resolution == GD_MDA_UNAVAILABLE ? "mda.reference_unavailable" : link->resolution == GD_MDA_MISSING ? "mda.reference_missing" : link->resolution == GD_MDA_WRONG_ROLE ? "mda.reference_target":"mda.reference_ambiguous",
                link->resolution == GD_MDA_UNAVAILABLE ? "Source load is incomplete; target absence cannot be established" : "Declared basis target is not a unique loaded record of the expected role; no link is repaired",gd_field_name(edge->field),edge->reference,edge->line);
    }
    for (i = 0; i < model->record_count; ++i) if (mda->selected[i]) {
        int gap = 0; ++mda->selected_count;
        if (model->records[i].role != GD_MDA_MECHANIC && !mda->nodes[i].basis_declared) {
            gap = 1; finding(mda,i,"mda.no_declared_basis","No basis link declared; review intent, not an automatic design defect",NULL,NULL,0);
        }
        if (mda->nodes[i].basis_declared != mda->nodes[i].basis_resolved) gap = 1;
        if (mda->nodes[i].aesthetic < 0) {
            gap = 1; finding(mda,i,mda->graph_complete ? "mda.no_aesthetic_trace":"mda.trace_unknown",
                mda->graph_complete ? "No declared path to an aesthetic in supplied graph; review whether one is intended":"Unresolved graph references prevent a negative aesthetic-trace claim",NULL,NULL,0);
        }
        if (gap) mda->gaps[mda->gap_count++] = i;
    }
    mda->query_complete = 1; return 1;
}
int gd_mda_path(const GD_Mda *mda, int target, int *nodes, int *links, int capacity, int *full_length) {
    int reverse_nodes[GD_MAX_MDA_PATH], reverse_links[GD_MAX_MDA_PATH], count = 0, i, v = target;
    *full_length = 0;
    if (capacity < 1 || capacity > GD_MAX_MDA_PATH || target < 0 || target >= GD_MAX_RECORDS || mda->root < 0 || mda->distance[target] < 0) return 0;
    *full_length = mda->distance[target]+1;
    while (v >= 0 && count < capacity) { reverse_nodes[count] = v; reverse_links[count++] = mda->parent_link[v]; v = mda->parent[v]; }
    for (i = 0; i < count; ++i) { nodes[i] = reverse_nodes[count-1-i]; if (i+1 < count) links[i] = reverse_links[count-2-i]; }
    return count;
}
int gd_mda_layout(const GD_Model *model, const GD_Mda *mda, GD_Layout *layout, int bands) {
    int mapping[GD_MAX_RECORDS], i;
    int has_systems = 0;
    memset(layout,0,sizeof(*layout)); layout->group_count = 1; layout->horizontal = 1;
    for (i = 0; i < model->record_count; ++i) if (model->records[i].role == GD_MDA_SYSTEM) has_systems = 1;
    for (i = 0; i < model->record_count; ++i) {
        mapping[i] = -1;
        if (gd_mda_role(model->records[i].role)) {
            GD_LayoutNode *node = &layout->nodes[layout->node_count]; int role = model->records[i].role-GD_MDA_MECHANIC;
            mapping[i] = layout->node_count++; node->key = i; node->label = model->records[i].name;
            node->layer = role-(!has_systems && role > 1); node->width=220; node->height=96;
        }
    }
    for (i = 0; i < mda->link_count; ++i) if (mda->links[i].resolution == GD_MDA_RESOLVED) {
        const GD_Edge *edge = &model->edges[mda->links[i].edge]; GD_LayoutEdge *line = &layout->edges[layout->edge_count++];
        line->a = mapping[edge->source]; line->b = mapping[edge->target]; line->key = i; line->active = 1; line->relation=edge->field;
    }
    return gd_layout_arrange(layout,bands ? GD_LAYOUT_DIRECTED:GD_LAYOUT_FORCE,GD_SORT_NAME);
}
