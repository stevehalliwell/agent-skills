#ifndef GD_SIMILARITY_OUTPUT_H
#define GD_SIMILARITY_OUTPUT_H
#include "gd_similarity_projection.h"
/* JSON/text share native feature/point/evidence payloads. Default map fields:
   features, points, evidence. Large inputs/source require explicit selection. */
int gd_similarity_fields(int catalogue,const char *fields);
int gd_write_similarity_features_retained(FILE *out,const GD_StructureResult *context,int json,const char *fields);
int gd_write_similarity_result_retained(FILE *out,const GD_SimilarityProjection *projection,int json,const char *fields);
int gd_write_similarity_features(FILE *out,const GD_StructureResult *context,int json,const char *fields);
int gd_write_similarity_result(FILE *out,const GD_SimilarityProjection *projection,int json,const char *fields,int first,int second);
#endif
