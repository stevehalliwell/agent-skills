#ifndef GD_EXPERIMENT_H
#define GD_EXPERIMENT_H
#include "gd.h"
#include "vendor/pcg-basic/pcg_basic.h"

#define GD_EXPERIMENT_VERSION "gd-experiment-v1"
#define GD_EXPERIMENT_PROTOCOL "gd-experiment-ndjson-v1"
#define GD_EXPERIMENT_RANDOM "pcg32-xsh-rr-v1"
#define GD_INPUT_NODES 4096
#define GD_INPUT_DEPTH 32
/* Header embeds a maximum-sized input plus provenance: one extra level and
   bounded metadata nodes/bytes, without weakening frozen-input limits. */
#define GD_EVENT_NODES (GD_INPUT_NODES+128)
#define GD_EVENT_DEPTH (GD_INPUT_DEPTH+1)
#define GD_EVENT_BYTES (GD_MAX_FILE+32768)
#define GD_EXPERIMENT_YAML_PIN "2c891fc7a770e8ba2fec34fc6b545c672beb37e6"
#define GD_EXPERIMENT_PCG_PIN "bc39cd76ac3d541e618606bcc6e1e5ba5e5e6aa3"

typedef enum { GD_INPUT_OBJECT, GD_INPUT_ARRAY, GD_INPUT_STRING,
    GD_INPUT_NUMBER, GD_INPUT_BOOLEAN, GD_INPUT_NULL } GD_InputKind;
typedef struct {
    GD_InputKind kind;
    int parent;
    const char *key, *text; /* Borrowed from owning GD_ExperimentInput. */
} GD_InputNode;
typedef struct {
    GD_Allocator allocator;
    char raw[GD_EVENT_BYTES+1], text[GD_EVENT_BYTES+1];
    GD_InputNode nodes[GD_EVENT_NODES];
    size_t raw_size, text_used;
    int count;
    const char *error; /* Static diagnostic; remains valid after parse failure. */
} GD_ExperimentInput;
/* Input owns one allocation; parser's bounded LibYAML events are temporary.
   Parsing invalidates all earlier node/text pointers. No live design loading. */
GD_ExperimentInput *gd_experiment_input_create(GD_Allocator allocator);
void gd_experiment_input_destroy(GD_ExperimentInput *input);
int gd_experiment_parse(GD_ExperimentInput *input, const char *bytes, size_t size);
int gd_experiment_read(GD_ExperimentInput *input, FILE *stream);
int gd_experiment_parse_event(GD_ExperimentInput *input, const char *bytes, size_t size);
int gd_input_member(const GD_ExperimentInput *input, int object, const char *key);
const char *gd_input_string(const GD_ExperimentInput *input, int node);
int gd_input_number(const GD_ExperimentInput *input, int node, double *value);
/* JSON integer lexeme only; no decimal/exponent coercion, magnitude <= 2^53-1. */
int gd_input_integer(const GD_ExperimentInput *input, int node, int64_t *value);
int gd_input_seed(const char *hex, uint64_t *value); /* Exactly 0x + 16 hex digits. */

/* Seed = master state initializer; zero-based trial index = stream selector.
   Trial indices must be < 2^63. Corresponding configurations reuse streams. */
void gd_trial_random(pcg32_random_t *random, uint64_t seed, uint64_t trial);
double gd_trial_uniform(pcg32_random_t *random);
/* Rejection sampling; failure after 128 draws, not modulo-biased fallback. */
int gd_trial_bounded(pcg32_random_t *random, uint32_t bound, uint32_t *value);

/* Monotonic wall clock; failure is negative, never fabricated zero elapsed. */
double gd_experiment_clock(void);
typedef struct {
    FILE *output;
    uint64_t steps, max_steps;
    size_t bytes, max_bytes;
    double started, timeout;
    const char *status; /* NULL, limit, error. Terminal status is sticky. */
} GD_TrialBudget;
int gd_trial_budget_check(GD_TrialBudget *budget);
int gd_trial_step(GD_TrialBudget *budget); /* Account before model action. */
/* Single already-encoded JSON record, without newline. Caller supplies binary
   stream (including Windows stdout) so byte cap includes actual newline bytes.
   Completion reserves 1024 bytes; normal records cannot consume that reserve.
   No hidden flush retry. */
int gd_trial_event(GD_TrialBudget *budget, const char *json, size_t size, int completion);
/* Finite numeric measurement and JSON string encoding into caller buffers.
   String caller supplies valid NUL-terminated UTF-8, not arbitrary bytes. */
int gd_measure_number(char *output, size_t capacity, double value);
int gd_event_quote(char *output, size_t capacity, const char *text);
#endif
