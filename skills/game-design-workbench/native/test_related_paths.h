#include "gd_similarity_output.h"
TEST(structural_related_mixed_paths_native_and_shared_result,
    GD_Model *model=model_create(); GD_ExplorerProject project={0}; GD_StructureOptions options={0}; GD_StructureResult *result;
    const GD_GraphComparison *comparison; const GD_GraphClosure *closure; int a,b,mechanic,leaf,shared,arcs[64],length,i;
    TEST_ASSERT(model && setup("related-paths"));
    collections[0].role=GD_SKILL; collections[0].alias="atoms";
    collections[1].role=GD_CHALLENGE; collections[1].alias="challenges";
    collections[2].role=GD_RELATED; collections[2].alias="context";
    TEST_ASSERT(write_record(0,"root","---\nid: root\nmechanics: [context/rule]\n---\n"));
    TEST_ASSERT(write_record(0,"leaf","---\nid: leaf\nprerequisites: [atoms/root, atoms/root]\n---\n"));
    TEST_ASSERT(write_record(0,"peer","---\nid: peer\nprerequisites: [atoms/root]\n---\n"));
    TEST_ASSERT(write_record(1,"first","---\nid: first\nrequires: [atoms/leaf]\nnext: [challenges/later]\n---\n"));
    TEST_ASSERT(write_record(1,"second","---\nid: second\nrequires: [atoms/peer]\n---\n"));
    TEST_ASSERT(write_record(1,"later","---\nid: later\nmechanics: [context/later]\n---\n"));
    TEST_ASSERT(write_record(2,"rule","---\nid: rule\n---\n")); TEST_ASSERT(write_record(2,"later","---\nid: later-rule\n---\n"));
    TEST_ASSERT(gd_load(model,collections,3) && !model->error_count);
    project.allocator=model->allocator; project.models[GD_EXPLORER_SKILL_DATA]=model; project.available=1u<<GD_EXPLORER_RELATIONSHIPS;
    options.question="related-paths"; options.projection="relationships"; options.from="challenges/first"; options.to="challenges/second"; options.work_limit=GD_GRAPH_ANALYSIS_WORK;
    result=gd_structure_result_build(model,&project,&options); TEST_ASSERT(result && result->query->complete && result->query->available);
    a=result->query->request.source; b=result->query->request.target; mechanic=leaf=shared=-1;
    for(i=0;i<result->layout->node_count;++i) {
        int key=result->layout->nodes[i].key;
        if(!strcmp(model->records[key].reference,"context/rule")) mechanic=i;
        if(!strcmp(model->records[key].reference,"atoms/leaf")) leaf=i;
        if(!strcmp(model->records[key].reference,"atoms/root")) shared=i;
    }
    comparison=gd_structure_comparison(result->query); closure=&result->query->analysis.reachability;
    TEST_ASSERT(a>=0 && b>=0 && mechanic>=0 && leaf>=0 && shared>=0 && comparison && comparison->shared==2 && comparison->a_only==1 && comparison->b_only==1);
    TEST_ASSERT(comparison->nodes[mechanic]==GD_GRAPH_SHARED && comparison->nodes[shared]==GD_GRAPH_SHARED && comparison->nodes[leaf]==GD_GRAPH_A_ONLY && closure->peer_count>0);
    length=gd_graph_closure_path(&result->query->analysis,closure,0,mechanic,arcs,64); TEST_ASSERT(length==3);
    TEST_ASSERT(result->layout->edges[arcs[0]/2].relation==GD_MECHANICS && result->layout->edges[arcs[1]/2].relation==GD_PREREQUISITES && result->layout->edges[arcs[2]/2].relation==GD_REQUIRES);
    TEST_ASSERT(!gd_sources_changed(model));
    { FILE *output=tmpfile(); char text[32768]; size_t count; TEST_ASSERT(output && gd_write_structure_result(output,result,1,"evidence")); rewind(output); count=fread(text,1,sizeof(text)-1,output); text[count]=0; TEST_ASSERT(strstr(text,"\"related_paths\"") && strstr(text,"\"shared\":2") && strstr(text,"\"omitted_member_witnesses\":0") && strstr(text,"\"length\":3")); fclose(output); }
    {
        unsigned char selected[GD_MAX_RECORDS]={0}; const char *features[]={"reached-outgoing:all-selected-kinds"};
        GD_SimilaritySelection selection={features,NULL,1,GD_SIMILARITY_WORK,selected,NULL,0}; GD_SimilarityProjection *projection;
        int work=0,stage; double distance; GD_Allocator original=project.allocator; StructureMemory memory={0,-1};
        selected[a]=selected[b]=1;
        projection=gd_similarity_projection_build(model,result,&selection);
        TEST_ASSERT(projection && projection->table_complete && !projection->limited && projection->map && projection->map->available && projection->entry_count==2);
        TEST_ASSERT(projection->cells[0].count==3 && projection->cells[1].count==3 && projection->peak_owned_bytes<=GD_SIMILARITY_STORAGE);
        TEST_ASSERT(gd_similarity_distance(projection->map,0,1,NULL,&distance,&work,GD_SIMILARITY_WORK) && fabs(distance-0.5)<1e-12);
        TEST_ASSERT(projection->cells[0].members[0]>=0 && gd_structure_comparison(result->query)->shared==2 && !gd_sources_changed(model));
        { FILE *output; char path[GD_MAX_PATH],text[32768]; size_t count;
          TEST_ASSERT(gd_similarity_fields(0,"features,points,inputs,evidence,source") && !gd_similarity_fields(1,"points") && !gd_similarity_fields(0,"points,"));
          snprintf(path,sizeof(path),"%s/similarity-map.json",root); output=fopen(path,"wb"); TEST_ASSERT(output && gd_write_similarity_result(output,projection,1,NULL,0,1)); fclose(output);
          snprintf(path,sizeof(path),"%s/similarity-inputs.json",root); output=fopen(path,"wb"); TEST_ASSERT(output && gd_write_similarity_result(output,projection,1,"inputs,source",-1,-1)); fclose(output);
          snprintf(path,sizeof(path),"%s/similarity-features.json",root); output=fopen(path,"wb"); TEST_ASSERT(output && gd_write_similarity_features(output,result,1,NULL)); fclose(output);
          output=tmpfile(); TEST_ASSERT(output && gd_write_similarity_result(output,projection,1,"evidence",0,1)); rewind(output); count=fread(text,1,sizeof(text)-1,output); text[count]=0;
          TEST_ASSERT(strstr(text,"\"distance\":0.5") && !strstr(text,"\"points\"") && !strstr(text,"\"inputs\"") && strstr(text,"\"unweighted_distance\":0.5")); fclose(output);
          TEST_ASSERT(!gd_write_similarity_result(NULL,projection,1,NULL,-1,-1)); }
        { char id[128]; double weight; const char *ids[]={projection->ids[0],projection->ids[1]}; GD_SimilaritySelection named=selection; GD_SimilarityProjection *copy;
          TEST_ASSERT(gd_similarity_feature_spec("reached-outgoing:all-selected-kinds=0.5",id,sizeof(id),&weight) && weight==0.5 && !strcmp(id,features[0]));
          TEST_ASSERT(gd_similarity_feature_spec("source-status",id,sizeof(id),&weight) && weight==1 && !strcmp(id,"source-status"));
          TEST_ASSERT(gd_similarity_feature_spec("source-status=1.0",id,sizeof(id),&weight) && weight==1);
          TEST_ASSERT(!gd_similarity_feature_spec("source-status=-1",id,sizeof(id),&weight) && !gd_similarity_feature_spec("=1",id,sizeof(id),&weight) && !gd_similarity_feature_spec("source-status=nan",id,sizeof(id),&weight) && !gd_similarity_feature_spec("source-status=1/2",id,sizeof(id),&weight));
          named.selected=NULL; named.entries=ids; named.entry_count=2; copy=gd_similarity_projection_build(model,result,&named);
          TEST_ASSERT(copy && copy->map && copy->map->available && copy->entry_count==2 && !strcmp(copy->ids[0],projection->ids[0]) && copy->cells[0].count==3); gd_similarity_projection_destroy(copy);
          named.selected=selected; TEST_ASSERT(!gd_similarity_projection_build(model,result,&named) && model->error_count); model->error_count=0; named.selected=NULL;
          ids[1]=ids[0]; copy=gd_similarity_projection_build(model,result,&named); TEST_ASSERT(copy && copy->map && copy->entry_count==1); gd_similarity_projection_destroy(copy);
          ids[1]="missing/native-id"; TEST_ASSERT(!gd_similarity_projection_build(model,result,&named) && model->error_count); model->error_count=0; }
        printf("similarity core peak owned: %llu bytes; feature extraction/fit fixture peak: %llu bytes; borrowed context excluded\n",(unsigned long long)gd_similarity_peak_owned_bytes(),(unsigned long long)projection->peak_owned_bytes);
        gd_similarity_projection_destroy(projection);
        features[0]="direct-outgoing:requires"; projection=gd_similarity_projection_build(model,result,&selection); work=0;
        TEST_ASSERT(projection && projection->map && projection->map->available && gd_similarity_distance(projection->map,0,1,NULL,&distance,&work,GD_SIMILARITY_WORK) && distance==1); gd_similarity_projection_destroy(projection);
        features[0]="direct-incoming:requires"; projection=gd_similarity_projection_build(model,result,&selection); work=0;
        TEST_ASSERT(projection && projection->map && projection->cells[0].present && !projection->cells[0].count && gd_similarity_distance(projection->map,0,1,NULL,&distance,&work,GD_SIMILARITY_WORK) && distance==0); gd_similarity_projection_destroy(projection);
        features[0]="source-status"; projection=gd_similarity_projection_build(model,result,&selection);
        TEST_ASSERT(projection && projection->map && projection->map->available && projection->map->missing_entries==2 && !projection->map->complete_entries && projection->missing[0]==GD_SIMILARITY_ABSENT_FIELD); gd_similarity_projection_destroy(projection);
        features[0]="source-unsupported"; TEST_ASSERT(!gd_similarity_projection_build(model,result,&selection) && model->error_count); model->error_count=0;
        features[0]="reached-outgoing:all-selected-kinds"; selection.work_limit=0; projection=gd_similarity_projection_build(model,result,&selection);
        TEST_ASSERT(projection && projection->limited && !projection->map && !projection->table_complete);
        { FILE *output; char path[GD_MAX_PATH]; snprintf(path,sizeof(path),"%s/similarity-limited.json",root); output=fopen(path,"wb"); TEST_ASSERT(output && gd_write_similarity_result(output,projection,1,NULL,-1,-1)); fclose(output); }
        gd_similarity_projection_destroy(projection); selection.work_limit=GD_SIMILARITY_WORK;
        project.allocator=(GD_Allocator){&memory,structure_allocate,structure_release};
        for(stage=0;stage<8;++stage) {
            memory.remaining=stage; projection=gd_similarity_projection_build(model,result,&selection);
            TEST_ASSERT(!projection && model->error_count && !memory.outstanding); model->error_count=0;
        }
        memory.remaining=-1; projection=gd_similarity_projection_build(model,result,&selection);
        TEST_ASSERT(projection && projection->map && projection->map->available && memory.outstanding>0); gd_similarity_projection_destroy(projection); TEST_ASSERT(!memory.outstanding);
        project.allocator=original;
    }
    gd_structure_result_destroy(result);
    options.question="related-structure"; result=gd_structure_result_build(model,&project,&options); TEST_ASSERT(result && gd_structure_comparison(result->query) && !gd_structure_comparison(result->query)->shared); gd_structure_result_destroy(result);
    options.question="related-paths"; options.to=NULL; result=gd_structure_result_build(model,&project,&options); TEST_ASSERT(result && result->query->available && gd_structure_comparison(result->query)->a_only==3 && !gd_structure_comparison(result->query)->shared); gd_structure_result_destroy(result);
    options.relation_count=1; options.relations[0]="all"; options.to="challenges/second"; result=gd_structure_result_build(model,&project,&options); TEST_ASSERT(result && gd_structure_comparison(result->query)->a_only==3 && gd_structure_comparison(result->query)->shared==2); gd_structure_result_destroy(result);
    options.relation_count=1; options.relations[0]="requires"; result=gd_structure_result_build(model,&project,&options); TEST_ASSERT(result && gd_structure_comparison(result->query)->shared==0 && gd_structure_comparison(result->query)->a_only==1); gd_structure_result_destroy(result);
    options.relation_count=0; options.work_limit=0; result=gd_structure_result_build(model,&project,&options); TEST_ASSERT(result && !result->query->available && !result->query->complete && !gd_structure_comparison(result->query)); gd_structure_result_destroy(result);
    options.work_limit=GD_GRAPH_ANALYSIS_WORK; options.from=NULL; TEST_ASSERT(!gd_structure_result_build(model,&project,&options) && model->error_count); model->error_count=0;
    options.from="context/rule"; options.to="atoms/root"; options.direction=GD_STRUCTURE_BACKWARD;
    result=gd_structure_result_build(model,&project,&options); TEST_ASSERT(result && result->query->complete && result->query->available && gd_structure_comparison(result->query)->shared==4); gd_structure_result_destroy(result);
    TEST_ASSERT(!gd_sources_changed(model)); gd_destroy(model);
)
