# Style verifier

Inspect Markdown against selected profile.

## Required read

Read `../references/style-profiles.md` before work. Done when profile resolution and metric limits are known.

## Workflow

1. Gather target Markdown path(s), profile name, and optional explicit `apply` request. Use single-review mode for one target; use corpus-review mode when multiple items must be compared or ranked. Default report-only. Apply mode is single-review only. Done when review mode, targets, and edit authority are known.
2. Resolve profile project-first, then global. If missing, report both checked paths; direct user to the learn workflow or another name. Resolve the same-named `.metrics.json` sidecar. Done when profile and metric source are resolved or a blocker is stated.
3. Assess profile rules with observable support: voice, vocabulary, sentence rhythm, composition, structure, punctuation, and anti-patterns. In single-review mode, each finding needs target location or verbatim anchor, profile rule, evidence, severity, and suggested correction. In corpus-review mode, assess every item and record one concise qualitative signal per item; give location-level evidence for the highest-deviance items and any item with a material qualitative mismatch. Mark meaning/accessibility/exact-technical-text/user-requirement conflicts as low-severity judgement calls. Done when every finding is grounded or omitted.
4. When the sidecar is available, run `../references/style-profile-compare.mjs <profile.metrics.json> <target Markdown files...>`. It runs the metrics script for each target and returns grouped, weighted heuristic statistical-deviation results across cadence and structure, voice and punctuation, lexicon, and POS composition. The profile's optional `Verification calibration` section supplies these weights and standalone-label threshold; otherwise documented defaults apply. In single-review mode, report relevant corpus/target values, metric deltas, and the group scores that materially influence the total. In corpus-review mode, rank every item by score and include the strongest statistical differences in the table. Scores identify candidates for review; they are not conformance grades or automatic exclusion rules. If the sidecar is unavailable, report statistical comparison unavailable; never estimate. Done when metric evidence or limitation is explicit.
5. In single-review apply mode, state exact declared pattern and replacement, then change exact matches only. Preserve Markdown syntax, code blocks, URLs, commands, quoted excerpts, meaning. Show diff or before/after; report skipped subjective findings. If no exact anti-pattern rule exists, make no edit. Done when each edit is deterministic and reported.
6. Return verification report. For corpus-review mode, include every item in a table with rank, statistical-deviation score, strongest statistical differences, and qualitative signal; follow with detailed evidence for review candidates. Done when profile path, findings, matches, limits, metric source, and any apply result are explicit.

## Output shape

```markdown
# Style verification: <target>

## Profile
- <resolved path>

## Statistical comparison
### Weighted groups
| Group | Weight | Deviation |
| --- | ---: | ---: |
| <group> | <profile weight> | <heuristic delta> |

### Metrics
| Metric | Group | Corpus | Target | Deviation |
| --- | --- | --- | --- | --- |
| <metric> | <group or unweighted> | <expected value/range> | <target value> | <heuristic delta> |

### Tracked separately
- <standalone-label rate or other unweighted measurement>

## Findings
- <location or anchor> — <severity: high|medium|low>
  - Rule: <profile rule>
  - Evidence: <target text or measurable result>
  - Suggested correction: <specific change>

## Matches
- <grounded profile match>

## Limits
- <rules not assessable from target or profile>

For corpus-review mode:

```markdown
# Corpus style review: <profile>

| Rank | Item | Statistical deviance | Strongest statistical differences | Qualitative signal |
| ---: | --- | ---: | --- | --- |
| 1 | <path> | <score> | <metric deltas> | <one grounded observation> |

## Review candidates
- <item> — <statistical and qualitative evidence; whether to exclude, split into a mode, or retain>

## Limits
- Scores are heuristic outlier signals, not quality grades.
```
```

## Boundaries

- Markdown targets only.
- No automatic profile selection or profile modification.
- Use the generate workflow for rewrites beyond deterministic replacements.
