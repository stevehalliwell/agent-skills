#ifndef GD_DECISIONS_H
#define GD_DECISIONS_H
#include "gd.h"
/* CLI-only authored assessments; no inference, rule evaluation or viewer mutation. */
int gd_decisions_cli(GD_Allocator allocator, const GD_Collection *collections, int count,
                     const char *projection, const char *demand, const char *intent,
                     int json, const char *fields, const char *initial_error);
int gd_write_decision_catalogue(FILE *output, int json, int comma);
#endif
