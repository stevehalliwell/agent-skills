#ifndef GD_SIMILARITY_PROJECTION_H
#define GD_SIMILARITY_PROJECTION_H
#include "gd_similarity.h"
#include "gd_structure_cli.h"
#define GD_SIMILARITY_STORAGE (64u*1024u*1024u)
/* Finite discoverable feature IDs, not a query grammar. Directions describe
   canonical projection endpoints; incidence projections offer adjacent sets. */
typedef enum {
    GD_SIMILARITY_PARTNERS, GD_SIMILARITY_ROWS, GD_SIMILARITY_RELATIONS,
    GD_SIMILARITY_INCOMING, GD_SIMILARITY_OUTGOING, GD_SIMILARITY_RANK,
    GD_SIMILARITY_STATUS, GD_SIMILARITY_ROLE, GD_SIMILARITY_STATE,
    GD_SIMILARITY_DIRECT_SET, GD_SIMILARITY_REACHED_SET
} GD_SimilarityProperty;
typedef struct {
    char id[128]; GD_SimilarityFeature feature;
    GD_SimilarityProperty property; int relation,reverse;
} GD_SimilarityFeatureInfo;
typedef enum { GD_SIMILARITY_KNOWN, GD_SIMILARITY_ABSENT_FIELD, GD_SIMILARITY_NO_VARIANT } GD_SimilarityMissing;
typedef struct GD_SimilaritySelection {
    const char *const *features; const double *weights; int feature_count,work_limit;
    const unsigned char *selected; /* Layout-index mask, consumed during build; NULL all shown. */
    const char *const *entries; int entry_count; /* Optional <=64 exact native IDs; cannot combine with mask. */
} GD_SimilaritySelection;
typedef struct GD_SimilarityProjection {
    GD_Allocator allocator; const GD_StructureResult *context;
    GD_SimilarityFeatureInfo catalogue[GD_SIMILARITY_FEATURES]; int catalogue_count;
    GD_SimilarityFeature features[GD_SIMILARITY_FEATURES]; int selected_features[GD_SIMILARITY_FEATURES];
    const char *ids[GD_MAX_RECORDS]; int nodes[GD_MAX_RECORDS],entry_count,feature_count;
    char *identity_storage; GD_SimilarityCell *cells; unsigned char *missing;
    int *sets[GD_SIMILARITY_FEATURES]; GD_Similarity *map;
    int work,work_limit,limited,storage_limited,source_limited,table_complete;
    size_t owned_bytes,peak_owned_bytes;
} GD_SimilarityProjection;
/* Caller supplies storage for at least GD_SIMILARITY_FEATURES rows. Catalogue
   depends on native projection capabilities, not whether a relation has rows. */
int gd_similarity_feature_catalogue(const GD_StructureResult *context,GD_SimilarityFeatureInfo *features);
/* Shared CLI/form qualifier parser: finite catalogue ID optionally =WEIGHT.
   No feature inference or expression evaluation. Output ID needs 128 bytes. */
int gd_similarity_feature_spec(const char *spec,char *id,size_t size,double *weight);
/* Borrow immutable canonical forward context/project until destruction. Entry
   selection never erases feature links to other shown context nodes. Feature
   IDs must be advertised, unique and explicitly selected. Weight default 1.
   Work/storage exhaustion returns limited projection without coordinates;
   invalid input/allocation failure returns NULL with native diagnostic. */
GD_SimilarityProjection *gd_similarity_projection_build(GD_Model *diagnostic,const GD_StructureResult *context,const GD_SimilaritySelection *selection);
void gd_similarity_projection_destroy(GD_SimilarityProjection *projection);
#endif
