#ifndef GD_DIAGNOSTICS_H
#define GD_DIAGNOSTICS_H
#include "gd.h"

typedef enum { GD_DIAGNOSTIC_ALL, GD_DIAGNOSTIC_FINDING, GD_DIAGNOSTIC_ERROR, GD_DIAGNOSTIC_RELOAD } GD_DiagnosticKind;
/* One owned allocation. No pointers into rejected model; release before allocator
   context expires. Strings capped at 128-byte code / 4095-byte message/path,
   on UTF-8 boundaries; shortened entries remain explicitly marked. */
typedef struct {
    GD_Allocator allocator;
    GD_Error errors[GD_MAX_ERRORS];
    unsigned char truncated[GD_MAX_ERRORS], path_truncated[GD_MAX_ERRORS];
    int count, omitted;
    char text[];
} GD_DiagnosticErrors;
typedef struct {
    const GD_Model *model;
    const GD_Finding *findings;
    int finding_count, findings_omitted;
    const GD_DiagnosticErrors *rejected;
} GD_DiagnosticSet;
typedef struct {
    const char *code, *message, *path, *record_reference, *field, *reference;
    GD_DiagnosticKind kind;
    int record, line, truncated;
} GD_DiagnosticItem;
GD_DiagnosticErrors *gd_diagnostics_capture(GD_Allocator allocator, const GD_Error *errors, int count, int omitted);
void gd_diagnostics_destroy(GD_DiagnosticErrors *errors);
/* Borrowed items last only while source set/snapshot stays unchanged. Invalid
   finding indices remain visible without invented record identity. Error paths
   navigate only when a unique loaded record shares the exact path. */
int gd_diagnostic_count(const GD_DiagnosticSet *set);
int gd_diagnostic_item(const GD_DiagnosticSet *set, int index, GD_DiagnosticItem *item);
/* ASCII-case-insensitive substring over metadata, not source bodies. Kind ALL
   and record=-1 are unrestricted; record=-2 represents empty selection. */
int gd_diagnostic_matches(const GD_DiagnosticItem *item, GD_DiagnosticKind kind, const char *text, int record);
#endif
