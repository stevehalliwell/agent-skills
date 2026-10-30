#include "gd_structure_query.h"
#include "gd_projection.h"
TEST(structural_summary_parallel_roles_and_scope,
    GD_Model *model=model_create(); GD_Allocator allocator=model->allocator; GD_Layout *layout=allocator.allocate(allocator.context,sizeof(*layout)); GD_StructureQuery *query; GD_StructureRequest request={0}; int i;
    TEST_ASSERT(layout); memset(layout,0,sizeof(*layout)); layout->node_count=6; layout->edge_count=6;
    for (i=0;i<6;++i) { layout->nodes[i].key=i; layout->nodes[i].label="fixture"; }
    layout->edges[0]=(GD_LayoutEdge){0,1,0,1,GD_REQUIRES}; layout->edges[1]=(GD_LayoutEdge){1,2,1,1,GD_PREREQUISITES}; layout->edges[2]=layout->edges[1]; layout->edges[2].key=2;
    layout->edges[3]=(GD_LayoutEdge){2,3,3,1,GD_MECHANICS}; layout->edges[4]=(GD_LayoutEdge){0,2,4,1,GD_NEXT}; layout->edges[5]=(GD_LayoutEdge){5,5,5,1,GD_REQUIRES};
    request.source=request.target=request.avoid=-1; request.work_limit=GD_GRAPH_ANALYSIS_WORK;
    query=gd_structure_query_build(allocator,layout,NULL,NULL,&request); TEST_ASSERT(query && query->complete && query->isolated==1 && query->shown_edges==6);
    TEST_ASSERT(query->nodes[1].incoming==1 && query->nodes[1].outgoing==2 && query->nodes[1].incident==3 && query->nodes[1].partners==2 && query->nodes[1].kinds==2);
    TEST_ASSERT(query->nodes[5].partners==0 && query->nodes[5].incident==1 && query->nodes[5].incoming==1 && query->nodes[5].outgoing==1);
    TEST_ASSERT(!gd_graph_analyze_bounded(layout,NULL,0,GD_GRAPH_ANY,0,&query->analysis));
    TEST_ASSERT(gd_graph_analyze_bounded(layout,NULL,0,GD_GRAPH_ANY,GD_GRAPH_ANALYSIS_WORK,&query->analysis) && query->analysis.complete && query->analysis.pair_count>0);
    TEST_ASSERT(gd_graph_analyze_bounded(layout,NULL,0,GD_GRAPH_ANY,0,&query->analysis) && !query->analysis.complete && !query->analysis.pair_count); gd_structure_query_destroy(query);
    request.kind=GD_STRUCTURE_RELATED; request.source=0; request.target=1;
    query=gd_structure_query_build(allocator,layout,NULL,NULL,&request); TEST_ASSERT(query && query->complete && query->analysis.comparison.shared==1 && query->analysis.comparison.a_only==1 && query->analysis.comparison.b_only==1); gd_structure_query_destroy(query);
    request.lens=GD_GRAPH_SAME_ROLE; query=gd_structure_query_build(allocator,layout,NULL,NULL,&request); TEST_ASSERT(query && !query->analysis.comparison.shared && query->analysis.comparison.other_role==1); gd_structure_query_destroy(query);
    request.source=request.target=-1; request.work_limit=0; query=gd_structure_query_build(allocator,layout,NULL,NULL,&request); TEST_ASSERT(query && !query->complete && !query->analysis.pair_count && query->nodes[1].partners==2); gd_structure_query_destroy(query);
    request.kind=GD_STRUCTURE_CONNECTIONS; request.lens=GD_GRAPH_ANY; request.incidence=1; query=gd_structure_query_build(allocator,layout,NULL,NULL,&request); TEST_ASSERT(query && query->complete && query->undirected && query->nodes[1].incoming==-1 && query->nodes[1].outgoing==-1); gd_structure_query_destroy(query);
    { unsigned char eligible[GD_MAX_PAIRS]; memset(eligible,1,sizeof(eligible)); eligible[1]=eligible[2]=0; request.edge_mask=eligible; query=gd_structure_query_build(allocator,layout,NULL,NULL,&request); TEST_ASSERT(query && query->shown_edges==4 && query->nodes[1].incident==1 && query->nodes[1].partners==1 && !query->request.edge_mask); gd_structure_query_destroy(query); }
    allocator.release(allocator.context,layout); gd_destroy(model);
)
TEST(structural_mixed_kind_paths_dominators_and_boundaries,
    GD_Model *model=model_create(); GD_Allocator allocator=model->allocator; GD_Layout *layout=allocator.allocate(allocator.context,sizeof(*layout)); GD_StructureQuery *query; GD_StructureRequest request={0}; unsigned char shown[GD_MAX_RECORDS]; int i;
    TEST_ASSERT(layout); memset(layout,0,sizeof(*layout)); layout->node_count=5; layout->edge_count=5;
    for (i=0;i<5;++i) { layout->nodes[i].key=i; layout->nodes[i].label="fixture"; }
    layout->edges[0]=(GD_LayoutEdge){0,1,0,1,GD_REQUIRES}; layout->edges[1]=(GD_LayoutEdge){1,2,1,1,GD_PREREQUISITES}; layout->edges[2]=layout->edges[1]; layout->edges[2].key=2;
    layout->edges[3]=(GD_LayoutEdge){2,3,3,1,GD_MECHANICS}; layout->edges[4]=(GD_LayoutEdge){0,2,4,1,GD_NEXT};
    request.kind=GD_STRUCTURE_REACHABILITY; request.source=0; request.target=3; request.avoid=-1; request.work_limit=GD_GRAPH_ANALYSIS_WORK;
    request.relation_count=3; request.relations[0]=GD_REQUIRES; request.relations[1]=GD_PREREQUISITES; request.relations[2]=GD_MECHANICS;
    query=gd_structure_query_build(allocator,layout,NULL,NULL,&request); TEST_ASSERT(query && query->complete && query->marked_nodes[3] && query->marked_edges[0] && query->marked_edges[3] && !query->marked_edges[4]); gd_structure_query_destroy(query);
    request.direction=GD_STRUCTURE_BACKWARD; request.source=3; request.target=0;
    query=gd_structure_query_build(allocator,layout,NULL,NULL,&request); TEST_ASSERT(query && query->complete && query->marked_nodes[0] && query->nodes[0].outgoing==1 && query->nodes[0].incoming==0 && layout->edges[0].a==0); gd_structure_query_destroy(query);
    request.direction=GD_STRUCTURE_FORWARD; request.source=0; request.target=3; request.relation_count=0; request.kind=GD_STRUCTURE_DOMINATORS;
    query=gd_structure_query_build(allocator,layout,NULL,NULL,&request); TEST_ASSERT(query && query->complete && query->available && query->analysis.paths.selection.ancestors[2] && !query->analysis.paths.selection.ancestors[1]); gd_structure_query_destroy(query);
    request.kind=GD_STRUCTURE_BYPASS; request.avoid=1;
    query=gd_structure_query_build(allocator,layout,NULL,NULL,&request); TEST_ASSERT(query && query->complete && query->analysis.paths.bypass.found && query->marked_edges[4]); gd_structure_query_destroy(query);
    request.relation_count=3; query=gd_structure_query_build(allocator,layout,NULL,NULL,&request); TEST_ASSERT(query && !query->complete && !query->available && !query->analysis.paths.bypass.valid); gd_structure_query_destroy(query);
    request.relation_count=0; request.avoid=4; query=gd_structure_query_build(allocator,layout,NULL,NULL,&request); TEST_ASSERT(query && !query->available && !query->complete && !query->analysis.paths.bypass.valid); gd_structure_query_destroy(query);
    request.relation_count=3; request.kind=GD_STRUCTURE_REACHABILITY; request.avoid=-1; memset(shown,1,sizeof(shown)); shown[2]=0;
    query=gd_structure_query_build(allocator,layout,shown,NULL,&request); TEST_ASSERT(query && query->complete && !query->marked_nodes[3] && query->isolated==2); gd_structure_query_destroy(query);
    request.work_limit=0; query=gd_structure_query_build(allocator,layout,NULL,NULL,&request); TEST_ASSERT(query && !query->complete && !query->marked_nodes[3]); gd_structure_query_destroy(query);
    request.kind=GD_STRUCTURE_SHAPES; request.source=request.target=-1; query=gd_structure_query_build(allocator,layout,NULL,NULL,&request); TEST_ASSERT(query && !query->complete); gd_structure_query_destroy(query);
    request.kind=GD_STRUCTURE_ADJACENCY; request.relation_count=0; request.work_limit=GD_GRAPH_ANALYSIS_WORK; query=gd_structure_query_build(allocator,layout,NULL,NULL,&request); TEST_ASSERT(query && query->complete && query->analysis.overview.link_count==5); gd_structure_query_destroy(query);
    for (i=0;i<2;++i) { RouteFailure failure={allocator,i}; GD_Allocator failing={&failure,route_allocate,route_release}; TEST_ASSERT(!gd_structure_query_build(failing,layout,NULL,NULL,&request)); }
    request.source=99; TEST_ASSERT(!gd_structure_query_build(allocator,layout,NULL,NULL,&request)); request.source=-1; request.relations[0]=GD_STRUCTURE_KINDS; request.relation_count=1; TEST_ASSERT(!gd_structure_query_build(allocator,layout,NULL,NULL,&request));
    printf("structural query owned storage: %llu bytes; temporary filtered layout %llu bytes; borrowed source excluded\n",(unsigned long long)sizeof(GD_StructureQuery),(unsigned long long)sizeof(GD_Layout));
    allocator.release(allocator.context,layout); gd_destroy(model);
)
TEST(structural_capacity_and_empty_scope,
    GD_Model *model=model_create(); GD_Allocator allocator=model->allocator; GD_Layout *layout=allocator.allocate(allocator.context,sizeof(*layout)); GD_StructureQuery *query; GD_StructureRequest request={0}; int i;
    TEST_ASSERT(layout); memset(layout,0,sizeof(*layout)); request.source=request.target=request.avoid=-1;
    query=gd_structure_query_build(allocator,layout,NULL,NULL,&request); TEST_ASSERT(query && query->complete && !query->shown_nodes && !query->shown_edges); gd_structure_query_destroy(query);
    layout->node_count=GD_MAX_RECORDS; layout->edge_count=GD_MAX_PAIRS;
    for (i=0;i<layout->node_count;++i) { layout->nodes[i].key=i; layout->nodes[i].label="bounded"; }
    for (i=0;i<layout->edge_count;++i) layout->edges[i]=(GD_LayoutEdge){0,1+i%(GD_MAX_RECORDS-1),i,1,GD_REQUIRES};
    query=gd_structure_query_build(allocator,layout,NULL,NULL,&request); TEST_ASSERT(query && query->complete && query->shown_edges==GD_MAX_PAIRS && query->nodes[0].incident==GD_MAX_PAIRS && query->nodes[0].partners==GD_MAX_RECORDS-1 && !query->work); gd_structure_query_destroy(query);
    request.kind=GD_STRUCTURE_ADJACENCY; request.source=0; TEST_ASSERT(!gd_structure_request_valid(layout,NULL,&request)); request.source=-1; request.avoid=1; TEST_ASSERT(!gd_structure_request_valid(layout,NULL,&request)); request.avoid=-1; request.root_count=1; TEST_ASSERT(!gd_structure_request_valid(layout,NULL,&request));
    allocator.release(allocator.context,layout); gd_destroy(model);
)
