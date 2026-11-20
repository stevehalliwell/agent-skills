#ifndef GD_SIMILARITY_H
#define GD_SIMILARITY_H
#include "gd.h"
#define GD_SIMILARITY_FEATURES 256
#define GD_SIMILARITY_LANDMARKS 32
#define GD_SIMILARITY_WORK 67108864
#define GD_SIMILARITY_PAIRS 8192
typedef enum { GD_SIMILARITY_NUMBER, GD_SIMILARITY_NAME, GD_SIMILARITY_SET } GD_SimilarityKind;
typedef enum { GD_SIMILARITY_NO_FAILURE, GD_SIMILARITY_WORK_LIMIT, GD_SIMILARITY_FIT_LIMIT, GD_SIMILARITY_NUMERIC_FAILURE } GD_SimilarityFailure;
typedef struct { const char *name,*unit; GD_SimilarityKind kind; double weight; } GD_SimilarityFeature;
/* Set members are sorted unique nonnegative native identities within one
   borrowed snapshot, not positions in selected entries. Empty differs missing. */
typedef struct { int present,count; double number; const char *name; const int *members; } GD_SimilarityCell;
typedef struct {
    int entry_count,feature_count,work_limit;
    const char *const *ids;
    const GD_SimilarityFeature *features;
    const GD_SimilarityCell *cells; /* Row-major entry_count * feature_count. */
} GD_SimilarityInput;
typedef struct {
    GD_Allocator allocator; GD_SimilarityInput input;
    double x[GD_MAX_RECORDS],y[GD_MAX_RECORDS];
    double low[GD_SIMILARITY_FEATURES],high[GD_SIMILARITY_FEATURES];
    int order[GD_MAX_RECORDS],landmarks[GD_SIMILARITY_LANDMARKS];
    unsigned char placed[GD_MAX_RECORDS];
    int input_complete,complete_entries,missing_entries,landmark_count,work,limited,available,negative_eigenvalues;
    GD_SimilarityFailure failure;
    int distortion_pairs,distortion_complete;
    /* Fit normalizes by distance_scale before squaring. Error/distance sums
       and negative eigenvalue mass use that normalized squared scale; stress
       ratios are unchanged. Coordinates retain original distance units. */
    double distance_scale,squared_error,squared_distance,negative_eigenvalue_mass;
} GD_Similarity;
/* Borrow immutable input/IDs/cells/set members until destruction. Explicit
   allocator owns result and bounded temporary fit scratch. NULL invalid input
   or allocation failure; limited result has no published coordinates. Counts
   and ranges are final only with input_complete. Complete
   entries alone define numeric ranges, landmarks, fit and distortion samples.
   Pure typed distances/MDS, no feature inference or source writes. */
GD_Similarity *gd_similarity_build(GD_Allocator allocator, const GD_SimilarityInput *input);
void gd_similarity_destroy(GD_Similarity *result);
/* Fixed result plus temporary fit scratch, excluding borrowed inputs. */
size_t gd_similarity_peak_owned_bytes(void);
/* Unweighted components plus weighted mean. No comparisons for missing rows.
   Independent bounded call; does not mutate result or its construction budget. */
int gd_similarity_distance(const GD_Similarity *result, int first, int second, double *components, double *distance, int *work, int work_limit);
#endif
