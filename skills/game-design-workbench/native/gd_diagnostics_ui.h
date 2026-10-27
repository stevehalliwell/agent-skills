#ifndef GD_DIAGNOSTICS_UI_H
#define GD_DIAGNOSTICS_UI_H
#include "gd_diagnostics.h"
#include <stdbool.h>
typedef struct {
    char filter[257];
    int kind;
    bool selected_only, open_next, collapse_next;
    GD_DiagnosticErrors *rejected;
    int capture_failed;
} GD_DiagnosticBrowser;
void gd_diagnostics_rejected(GD_DiagnosticBrowser *browser, GD_Allocator allocator, const GD_Model *failed);
void gd_diagnostics_clear(GD_DiagnosticBrowser *browser);
/* Returns requested loaded record or -1; never mutates model/query/draft. */
int gd_diagnostics_panel(GD_DiagnosticBrowser *browser, const GD_Model *model,
    const GD_Finding *findings, int count, int omitted, int selected);
#endif
