#include "gd_similarity.h"
#include <float.h>
TEST(similarity_typed_distances_missing_and_reorder,
    GD_Allocator allocator={storage,allocate,release};
    const char *ids[]={"a","b","c","missing"},*reordered_ids[]={"c","missing","a","b"};
    int ab[]={1,2},abcd[]={1,2,3,4},c[]={3},permutation[]={2,3,0,1},i,j,work=0;
    GD_SimilarityFeature features[]={{"number","unit",GD_SIMILARITY_NUMBER,1},{"name",NULL,GD_SIMILARITY_NAME,2},{"targets",NULL,GD_SIMILARITY_SET,3}};
    GD_SimilarityCell cells[12]={{0}},reordered[12];
    GD_SimilarityInput input={4,3,GD_SIMILARITY_WORK,ids,features,cells};
    GD_Similarity *result,*again; double distance,components[3];
    for(i=0;i<4;++i) for(j=0;j<3;++j) cells[i*3+j].present=1;
    cells[0].number=0; cells[3].number=5; cells[6].number=10; cells[9].number=1000;
    cells[1].name="same"; cells[4].name="same"; cells[7].name="other"; cells[10].present=0;
    cells[2].members=ab; cells[2].count=2; cells[5].members=abcd; cells[5].count=4; cells[8].members=c; cells[8].count=1;
    result=gd_similarity_build(allocator,&input);
    TEST_ASSERT(result && result->available && result->input_complete && !result->limited && result->complete_entries==3 && result->missing_entries==1 && !result->placed[3]);
    TEST_ASSERT(result->low[0]==0 && result->high[0]==10 && result->distortion_complete && result->distortion_pairs==3 && result->squared_error<1e-18);
    TEST_ASSERT(gd_similarity_distance(result,0,1,components,&distance,&work,GD_SIMILARITY_WORK));
    TEST_ASSERT(fabs(components[0]-0.5)<1e-12 && components[1]==0 && fabs(components[2]-0.5)<1e-12 && fabs(distance-1.0/3)<1e-12);
    TEST_ASSERT(!gd_similarity_distance(result,0,3,NULL,&distance,&work,GD_SIMILARITY_WORK));
    for(i=0;i<4;++i) for(j=0;j<3;++j) reordered[i*3+j]=cells[permutation[i]*3+j];
    input.ids=reordered_ids; input.cells=reordered; again=gd_similarity_build(allocator,&input);
    TEST_ASSERT(again && again->available && again->work==result->work);
    for(i=0;i<4;++i) { TEST_ASSERT(fabs(again->x[i]-result->x[permutation[i]])<1e-10 && fabs(again->y[i]-result->y[permutation[i]])<1e-10); }
    for(i=0;i<3;++i) { TEST_ASSERT(!strcmp(again->input.ids[again->landmarks[i]],result->input.ids[result->landmarks[i]])); }
    gd_similarity_destroy(again); gd_similarity_destroy(result);
    input.ids=ids; input.cells=cells; cells[0].number=10000000000000000.0; cells[3].number=10000000000000002.0; cells[6].number=10000000000000006.0;
    result=gd_similarity_build(allocator,&input); TEST_ASSERT(result && result->available); work=0;
    TEST_ASSERT(gd_similarity_distance(result,0,1,components,&distance,&work,GD_SIMILARITY_WORK) && fabs(components[0]-1.0/3)<1e-12); gd_similarity_destroy(result);
    cells[0].number=-DBL_MAX; cells[3].number=0; cells[6].number=DBL_MAX;
    result=gd_similarity_build(allocator,&input); TEST_ASSERT(result && result->available); work=0;
    TEST_ASSERT(gd_similarity_distance(result,0,1,components,&distance,&work,GD_SIMILARITY_WORK) && fabs(components[0]-0.5)<1e-12); gd_similarity_destroy(result);
)
TEST(similarity_degenerate_non_euclidean_and_bounds,
    GD_Allocator allocator={storage,allocate,release};
    const char *ids[]={"a","b","c","d"};
    GD_SimilarityFeature features[]={{"x",NULL,GD_SIMILARITY_NUMBER,1},{"y",NULL,GD_SIMILARITY_NUMBER,1}};
    GD_SimilarityCell cells[8]={{0}}; GD_SimilarityInput input={4,2,GD_SIMILARITY_WORK,ids,features,cells};
    GD_Similarity *result; int i,work=0,exact; double distance;
    for(i=0;i<8;++i) cells[i].present=1;
    cells[2].number=1; cells[5].number=1; cells[6].number=1; cells[7].number=1;
    result=gd_similarity_build(allocator,&input);
    TEST_ASSERT(result && result->available && result->negative_eigenvalues==1 && result->negative_eigenvalue_mass>0.2 && result->squared_error>0);
    exact=result->work; gd_similarity_destroy(result);
    input.work_limit=exact; result=gd_similarity_build(allocator,&input); TEST_ASSERT(result && result->available && result->work==exact); gd_similarity_destroy(result);
    input.work_limit=exact-1; result=gd_similarity_build(allocator,&input);
    TEST_ASSERT(result && !result->available && result->limited && result->input_complete && !result->distortion_complete);
    for(i=0;i<4;++i) { TEST_ASSERT(!result->placed[i] && !result->x[i] && !result->y[i]); } gd_similarity_destroy(result);
    input.work_limit=0; result=gd_similarity_build(allocator,&input); TEST_ASSERT(result && result->limited && !result->input_complete && !result->available); gd_similarity_destroy(result);
    input.work_limit=GD_SIMILARITY_WORK;
    for(i=0;i<8;++i) cells[i].number=7;
    result=gd_similarity_build(allocator,&input); TEST_ASSERT(result && result->available && !result->negative_eigenvalues && !result->squared_error);
    for(i=0;i<4;++i) { TEST_ASSERT(result->placed[i] && result->x[i]==0 && result->y[i]==0); }
    TEST_ASSERT(gd_similarity_distance(result,0,1,NULL,&distance,&work,GD_SIMILARITY_WORK) && distance==0); gd_similarity_destroy(result);
    input.entry_count=1; result=gd_similarity_build(allocator,&input); TEST_ASSERT(result && result->available && result->placed[0] && result->x[0]==0 && !result->distortion_pairs); gd_similarity_destroy(result);
    cells[0].present=0; result=gd_similarity_build(allocator,&input); TEST_ASSERT(result && result->available && !result->complete_entries && result->missing_entries==1 && !result->placed[0]); gd_similarity_destroy(result);
    input.entry_count=0; result=gd_similarity_build(allocator,&input); TEST_ASSERT(result && result->available && !result->complete_entries); gd_similarity_destroy(result);
    input.entry_count=2; cells[0].present=1;
    ids[1]="a"; TEST_ASSERT(!gd_similarity_build(allocator,&input)); ids[1]="b";
    features[0].weight=-1; TEST_ASSERT(!gd_similarity_build(allocator,&input)); features[0].weight=0; features[1].weight=0; TEST_ASSERT(!gd_similarity_build(allocator,&input));
    features[0].weight=1; input.entry_count=GD_MAX_RECORDS+1; TEST_ASSERT(!gd_similarity_build(allocator,&input)); input.entry_count=2;
    input.feature_count=GD_SIMILARITY_FEATURES+1; TEST_ASSERT(!gd_similarity_build(allocator,&input));
)
TEST(similarity_small_representable_distances,
    GD_Allocator allocator={storage,allocate,release}; const char *ids[]={"a","b"};
    GD_SimilarityFeature features[]={{"varying",NULL,GD_SIMILARITY_NUMBER,1e-200},{"constant",NULL,GD_SIMILARITY_NUMBER,1}};
    GD_SimilarityCell cells[4]={{0}}; GD_SimilarityInput input={2,2,GD_SIMILARITY_WORK,ids,features,cells};
    GD_Similarity *result; int i,work=0; double distance,projected;
    for(i=0;i<4;++i) cells[i].present=1;
    cells[1].number=cells[3].number=7; cells[2].number=1;
    result=gd_similarity_build(allocator,&input); TEST_ASSERT(result && result->available && result->distance_scale>0);
    TEST_ASSERT(gd_similarity_distance(result,0,1,NULL,&distance,&work,GD_SIMILARITY_WORK) && distance>0);
    projected=fabs(result->x[0]-result->x[1]);
    TEST_ASSERT(fabs(projected/distance-1)<1e-12 && result->squared_distance>0.9 && result->squared_error<1e-20 && result->distortion_complete);
    gd_similarity_destroy(result);
)
TEST(similarity_landmarks_all_entries_and_capacity,
    GD_Allocator allocator={storage,allocate,release};
    static const char *ids[GD_MAX_RECORDS]; static char names[GD_MAX_RECORDS][16];
    static GD_SimilarityCell cells[GD_MAX_RECORDS];
    GD_SimilarityFeature feature={"position","unit",GD_SIMILARITY_NUMBER,1};
    GD_SimilarityInput input={GD_MAX_RECORDS,1,GD_SIMILARITY_WORK,ids,&feature,cells};
    GD_Similarity *result; int i,placed=0,work=0; double distance,dx,dy;
    for(i=0;i<GD_MAX_RECORDS;++i) { snprintf(names[i],sizeof(names[i]),"entry-%04d",i); ids[i]=names[i]; cells[i].present=1; cells[i].number=i; }
    result=gd_similarity_build(allocator,&input);
    TEST_ASSERT(result && result->available && !result->limited && result->complete_entries==GD_MAX_RECORDS && result->landmark_count==32 && !result->distortion_complete && result->distortion_pairs==GD_SIMILARITY_PAIRS);
    for(i=0;i<GD_MAX_RECORDS;++i) placed+=result->placed[i];
    TEST_ASSERT(placed==GD_MAX_RECORDS && result->squared_error<1e-17);
    TEST_ASSERT(gd_similarity_distance(result,0,GD_MAX_RECORDS-1,NULL,&distance,&work,GD_SIMILARITY_WORK));
    dx=result->x[0]-result->x[GD_MAX_RECORDS-1]; dy=result->y[0]-result->y[GD_MAX_RECORDS-1];
    TEST_ASSERT(fabs(distance-1)<1e-12 && fabs(sqrt(dx*dx+dy*dy)-1)<1e-10);
    gd_similarity_destroy(result);
)
