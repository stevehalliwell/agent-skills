#ifndef GD_EXPERIMENT_RESULT_H
#define GD_EXPERIMENT_RESULT_H
#include "gd_experiment.h"

#define GD_RESULT_TRIALS 1000000
#define GD_RESULT_MEASURES 64
#define GD_RESULT_BYTES ((size_t)1073741824)

typedef struct {
    const char *name, *identity, *type, *unit;
    int definition;
} GD_ResultMeasure;
typedef struct {
    const GD_ExperimentInput *input; /* Borrowed; immutable during validation. */
    const char *model, *recipe, *configuration;
    int64_t trials, trace_trial, max_steps;
    size_t max_bytes;
    double wall_seconds;
    GD_ResultMeasure measures[GD_RESULT_MEASURES];
    int measure_count;
} GD_ResultContract;
typedef struct {
    const char *checkpoint, *build, *toolchain; /* Actual execution provenance. */
} GD_ResultProvenance;
typedef struct {
    int protocol_valid, complete, has_exit_code, process_exit_code;
    const char *status, *code, *message;
    uint64_t line, records, trials, completed_trials, steps, undefined_measurements;
    size_t bytes;
} GD_ResultReport;

/* One frozen configuration per stream. A future sweep validates each captured
   configuration separately; trial indices are scoped to configuration, not global.
   Report strings are static; contract strings borrow frozen input storage. */
int gd_result_contract(GD_ResultContract *contract, const GD_ExperimentInput *input, GD_ResultReport *report);
int gd_result_validate(FILE *stream, const GD_ResultContract *contract,
    const GD_ResultProvenance *provenance, int process_exit_code, int externally_stopped,
    GD_Allocator allocator, GD_ResultReport *report);
void gd_result_write(FILE *output, const GD_ResultReport *report);
int gd_result_commit(const char *text);
#endif
