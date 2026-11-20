#ifndef GD_SKILL_UNLOCKS_H
#define GD_SKILL_UNLOCKS_H
#include "gd.h"
#define GD_SKILL_QUERY_WORK (16 * 1024 * 1024)
#define GD_SKILL_COMPARISONS 128
/* All records retain their status. These are authored combinations under
   explicit assumptions, never observed knowledge or automatic learning. */
typedef enum { GD_SKILL_NO_PREREQUISITES, GD_SKILL_NEEDS_MORE, GD_SKILL_PREREQUISITES_SUPPLIED, GD_SKILL_UNRESOLVED } GD_SkillEligibility;
typedef struct { int candidate, root_edge, candidate_edge; } GD_SkillComparison;
typedef struct GD_SkillUnlocks {
    int root, work, limited, comparison_count, comparisons_omitted;
    unsigned char known[GD_MAX_RECORDS], ancestors[GD_MAX_RECORDS], descendants[GD_MAX_RECORDS];
    GD_SkillEligibility eligibility[GD_MAX_RECORDS];
    int supplied[GD_MAX_RECORDS], missing[GD_MAX_RECORDS];
    GD_SkillComparison comparisons[GD_SKILL_COMPARISONS];
} GD_SkillUnlocks;
/* Reset/compute caller storage. root=-1 means all; no ownership of model.
   Explicit known[] is not transitively expanded. No source or rank changes. */
int gd_skill_unlocks(const GD_Model *model, const unsigned char *known, int root, GD_SkillUnlocks *result);
const char *gd_skill_eligibility_name(GD_SkillEligibility state);
void gd_write_skill_unlocks(FILE *output, const GD_Model *model, const GD_SkillUnlocks *query);
int gd_write_skill_result(FILE *output, const GD_Model *model, const GD_SkillUnlocks *query, int json, const char *fields);
#endif
