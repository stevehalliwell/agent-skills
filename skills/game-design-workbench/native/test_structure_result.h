#include "gd_structure_cli.h"
TEST(shared_structural_result_scope_ownership_and_failures,
    GD_Model *model=model_create(); GD_ExplorerProject project={0}; GD_StructureOptions options={0}; GD_StructureResult *result;
    GD_Allocator allocator; RouteFailure failure; int i;
    TEST_ASSERT(model && setup("structure-result")); collections[0].role=GD_SKILL; model->skill_mode=1;
    TEST_ASSERT(write_record(0,"a","---\nid: first\nstatus: accepted\n---\n"));
    TEST_ASSERT(write_record(0,"b","---\nid: second\nstatus: accepted\nprerequisites: [things/a]\n---\n"));
    TEST_ASSERT(write_record(0,"c","---\nid: third\nstatus: accepted\n---\n"));
    TEST_ASSERT(write_record(0,"d","---\nid: fourth\nstatus: accepted\nprerequisites: [things/c]\n---\n"));
    TEST_ASSERT(gd_load(model,collections,1) && !model->error_count);
    project.allocator=model->allocator; project.models[GD_EXPLORER_SKILL_DATA]=model; project.available=(1u<<GD_EXPLORER_SKILLS)|(1u<<GD_EXPLORER_RELATIONSHIPS);
    options.question="reachability"; options.projection="skills"; options.from="things/a"; options.to="things/b"; options.work_limit=GD_GRAPH_ANALYSIS_WORK;
    result=gd_structure_result_build(model,&project,&options);
    TEST_ASSERT(result && result->layout && result->query && result->query->complete && !result->source_incomplete && result->query->shown_nodes==4 && result->query->shown_edges==2);
    TEST_ASSERT(result->query->request.edge_mask==NULL && result->query->layout==result->layout && result->project==&project && result->selector_work>0);
    TEST_ASSERT(gd_graph_closure_contains(&result->query->analysis.reachability,result->query->request.source,result->query->request.target));
    { FILE *output=tmpfile(); TEST_ASSERT(output && !gd_write_structure_result(output,NULL,1,NULL) && !gd_write_structure_result(output,result,1,"missing") && gd_write_structure_result(output,result,1,"evidence") && ftell(output)>0); fclose(output); }
    printf("shared structural result: %llu bytes; owned layout %llu; owned query %llu bytes\n",(unsigned long long)sizeof(*result),(unsigned long long)sizeof(*result->layout),(unsigned long long)sizeof(*result->query));
    gd_structure_result_destroy(result);
    {
        GD_StructureResult *first,*second; int a,c;
        options.question="direct-connections"; options.to=NULL;
        first=gd_structure_result_build(model,&project,&options); options.from="things/c"; second=gd_structure_result_build(model,&project,&options);
        TEST_ASSERT(first && second); a=first->query->request.source; c=second->query->request.source;
        TEST_ASSERT(first->query->marked_nodes[a] && !first->query->marked_nodes[c] && !second->query->marked_nodes[a] && second->query->marked_nodes[c]);
        TEST_ASSERT(!memcmp(first->query->nodes,second->query->nodes,sizeof(first->query->nodes)) && memcmp(first->query->marked_edges,second->query->marked_edges,sizeof(first->query->marked_edges)));
        gd_structure_result_destroy(first); gd_structure_result_destroy(second);
        options.question="reachability"; options.from="things/a"; options.to="things/b";
    }
    allocator=project.allocator;
    for (i=0;i<4;++i) {
        failure=(RouteFailure){allocator,i}; project.allocator=(GD_Allocator){&failure,route_allocate,route_release};
        result=gd_structure_result_build(model,&project,&options); project.allocator=allocator;
        TEST_ASSERT(!result && model->error_count && !strcmp(model->errors[model->error_count-1].code,"query.memory")); model->error_count=0;
    }
    options.root_count=1; TEST_ASSERT(!gd_structure_result_build(model,&project,&options) && !strcmp(model->errors[0].code,"query.arguments")); model->error_count=0; options.root_count=0;
    options.work_limit=0; result=gd_structure_result_build(model,&project,&options);
    TEST_ASSERT(result && !result->query->complete && !result->source_incomplete);
    { FILE *output=tmpfile(); char text[8192]; size_t length; TEST_ASSERT(output && gd_write_structure_result(output,result,1,"evidence")); rewind(output); length=fread(text,1,sizeof(text)-1,output); text[length]=0; TEST_ASSERT(strstr(text,"\"cycle\":null")); fclose(output); }
    gd_structure_result_destroy(result);
    options.scope_root="things/a"; TEST_ASSERT(!gd_structure_result_build(model,&project,&options) && !strcmp(model->errors[0].code,"query.scope")); model->error_count=0;
    options.projection="relationships"; options.work_limit=GD_GRAPH_ANALYSIS_WORK; options.from="things/b"; options.to="things/a";
    result=gd_structure_result_build(model,&project,&options); TEST_ASSERT(result && result->query->shown_nodes==2 && result->query->complete); gd_structure_result_destroy(result);
    gd_destroy(model);
)
TEST(declaration_gap_inventory_native_identities_and_failures,
    GD_Model *model=model_create(); GD_ExplorerProject project={0}; GD_StructureOptions options={0}; GD_StructureResult *result;
    GD_Allocator allocator; RouteFailure failure; int i,missing=-1;
    TEST_ASSERT(model && setup("declaration-gaps")); collections[0].role=GD_SKILL; collections[1].role=GD_RELATED; model->skill_mode=1;
    TEST_ASSERT(write_record(0,"root","---\nstatus: accepted\nprerequisites: [things/known, things/missing, outside/absent, actions/other]\n---\n"));
    TEST_ASSERT(write_record(0,"known","---\nstatus: accepted\n---\n"));
    TEST_ASSERT(write_record(1,"other","---\nstatus: draft\n---\n"));
    TEST_ASSERT(gd_load(model,collections,2) && !model->error_count && model->finding_count>0);
    project.allocator=model->allocator; project.models[GD_EXPLORER_SKILL_DATA]=model; project.available=1u<<GD_EXPLORER_RELATIONSHIPS;
    options.question="declaration-gaps"; options.projection="relationships";
    result=gd_structure_result_build(model,&project,&options);
    TEST_ASSERT(result && result->gaps && result->gaps->rows==4 && result->query->complete && !result->query->work && result->source_incomplete);
    TEST_ASSERT(result->gaps->counts[GD_DECLARATION_RESOLVED]==2 && result->gaps->counts[GD_DECLARATION_MISSING]==1 && result->gaps->counts[GD_DECLARATION_UNLOADED]==1 && !result->gaps->counts[GD_DECLARATION_AMBIGUOUS]);
    { FILE *output=tmpfile(); char text[16384]; size_t length; TEST_ASSERT(output && gd_write_structure_result(output,result,1,"findings")); rewind(output); length=fread(text,1,sizeof(text)-1,output); text[length]=0; TEST_ASSERT(strstr(text,"\"missing_metadata_checked\":false") && strstr(text,"\"identity_resolution\":\"missing\"") && strstr(text,"\"identity_resolution\":\"unloaded\"")); fclose(output); }
    for (i=0;i<result->gaps->rows;++i) if (result->gaps->identities[i]==GD_DECLARATION_MISSING) missing=i;
    gd_structure_result_destroy(result); TEST_ASSERT(missing>=0);
    /* Synthetic ambiguous native identity marker, not a new resolver. */
    model->edges[missing].target=-2; result=gd_structure_result_build(model,&project,&options);
    TEST_ASSERT(result && result->gaps->counts[GD_DECLARATION_AMBIGUOUS]==1 && !result->gaps->counts[GD_DECLARATION_MISSING]); gd_structure_result_destroy(result); model->edges[missing].target=-1;
    options.status_count=1; options.statuses[0]="accepted"; TEST_ASSERT(!gd_structure_result_build(model,&project,&options) && !strcmp(model->errors[0].code,"query.arguments")); model->error_count=0; options.status_count=0;
    options.from="things/root"; TEST_ASSERT(!gd_structure_result_build(model,&project,&options)); model->error_count=0; options.from=NULL;
    allocator=project.allocator;
    for (i=0;i<5;++i) { failure=(RouteFailure){allocator,i}; project.allocator=(GD_Allocator){&failure,route_allocate,route_release}; result=gd_structure_result_build(model,&project,&options); project.allocator=allocator; TEST_ASSERT(!result && model->error_count && !strcmp(model->errors[0].code,"query.memory")); model->error_count=0; }
    printf("declaration inventory owned storage: %llu bytes\n",(unsigned long long)sizeof(GD_DeclarationGaps));
    gd_destroy(model);
    model=model_create(); TEST_ASSERT(model); project.models[GD_EXPLORER_SKILL_DATA]=model;
    result=gd_structure_result_build(model,&project,&options); TEST_ASSERT(result && result->gaps && !result->gaps->rows && !result->source_incomplete && result->query->complete); gd_structure_result_destroy(result); gd_destroy(model);
)
