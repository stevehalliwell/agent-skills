# Inspection and change questions

Read live schemas/usages before querying collection fields. Use Attendant directly; write SQL suited to actual aliases and the question, not a workbench query wrapper. Validate with `--no-correct` first: if preparation would correct source, inspect Markdown directly or use an ordinary temporary copy when SQL is necessary. Do not authorize collateral edits merely to obtain a projection.

## Inspection topics

- **Matrix:** interaction cases of all dispositions grouped by applicable noun and verb; active counts use accepted, defined pairs only. Preserve multiple cases and contextual nouns. Coverage distinguishes defined, unknown, excluded, and not applicable; read prose to distinguish no effect from unknown effect. Empty Cartesian cells are not automatically defects.
- **Skill atoms:** authored ranks, prerequisite AND sets, and mechanic links. Preserve null/negative/zero ranks. SQL inventory alone does not compute placement or detect cycles. Native skill operation below computes declared prerequisite/rank consistency, not player capability.
- **Routes:** entry/next and introduction/practice/required/mastery links. Edges are declared opportunities, not physical or game-state reachability. List order is not a route.
- **MDA:** inspect aesthetics, dynamics, systems, and mechanics; follow/reverse their declared links. Systems are not dynamics; links do not prove emotion.
- **Control variables:** named configuration types, literal values, units and formula text. Use shared native resolution/import/export below; never shell/eval expressions.
- **Actor/state rules:** readable `gd-rule` cases, explicit noun/axis applicability, predicates, quantities, preservation and mapped calls. Use conditional analysis below; no inferred scene, inherited outcome or file winner.
- **Glossary:** canonical terms/aliases, definition targets, and status, including inactive records.
- **Evidence:** experiment stage/kind and source; read local measures, hypotheses, expectations, method, observations, results, and provenance. Proposed is not executed.
- **Records:** inspect all relevant fields/dispositions, including resources, experiment-local measurements, existing project decisions, and case context omitted by a narrow inventory.

SQLite array cells are JSON text. Inspect complete arrays; do not infer missing relationships from truncated rows. Narrow query or increase limit and disclose incomplete coverage. Invalid/missing configuration is not zero results.

## Native matrix and connectivity

Read [native tools](../native/README.md) to build/run `matrix`, `connectivity`, or `view matrix`. Resolve actual role aliases/folders through project instructions and Attendant, then pass every mapping explicitly. Native commands do not discover collections or interpret schemas. Use `view matrix --layout graph` for force-directed connected groups, or `--layout columns` for stable noun/verb columns. Headless matrix `components` exposes the same active grouping, including isolated nodes. Other-pair overlay does not merge groups. No record writes, automatic framework review, or repair.

Matrix includes all dispositions and multiple cases; active connectivity counts accepted, defined pairs only. Context nouns stay separate. JSON coverage concerns supplied source loading/computation, not schema validity or exhaustive design coverage. Omitted related folders leave links visible but not loaded. Use `--related` only for wanted context.

## Native actor/state hypotheses

Read [rule contract](../native/rules.md) and [conditional outcome/comparison contract](../native/outcomes.md). Run `rules` with explicit interactions/mechanics and optional noun/axis/control/verb/related mappings. Default inspects declarations; `--case ALIAS/NAME#CASE` plus `--assume LOCAL:ALIAS/AXIS=VALUE` and ordered `--context`/`--context-not` supplies hypothetical current facts. Never infer actor initialization from configuration, context from geometry/links, or omitted predicates/effects as ANY/preserved.

`--analyze` expands explicit calls and compares bounded known enum products. Inspect `--fields analysis` for conditional proposals, simultaneous-write checks, source paths, witnesses, limits and unknown/unexamined scope. Keep mechanics/interactions as separate sources; reference-derived agreement is not independent corroboration. No priority, automatic merge/repair, numeric algebra proof, physical reachability, gameplay simulation or fun score. All supplied statuses/dispositions remain visible; do not label proposed alternatives as adopted active behavior.

## Native control resolution

Use `controls --control-vars ALIAS=FOLDER --json` to resolve typed values and formulas. Read [control contract](../native/README.md#control-values-and-exports). `--from ALIAS/NAME` limits scope to one control and required dependencies; repeatable `--set ALIAS/NAME=VALUE` applies temporary typed overrides without changing source. `controls export --as json|csv|ini` emits resolved values only; source-backed errors block game-facing export rather than inventing values. `controls import --input FILE --input-format json|csv|ini` plans typed value updates; use [tuning workflow](tune-controls.md) for authorized public-Attendant adoption and freshness checks. No simulation or gameplay verdict follows from successful value resolution.

## Native MDA declared traces

For requested MDA tracing/exploration, use `mda` or `view mda` with explicit mechanics/dynamics/aesthetics mappings; systems mapping optional. Read [MDA native contract](../native/README.md#mda-declared-traces-and-questions). Optional `--from ALIAS/NAME` and `--direction forward|backward|both` select query; forward moves toward aesthetics, backward follows authored basis, both explores connected associations. Stored/drawn arrows always preserve authored references.

Inspect nodes, raw/boundary links, one shortest witness per reached node, scoped questions, and source details. Systems are not dynamics or a mandatory bridge. Missing association is a review question, not automatic defect. Unloaded aliases differ from mapped missing/wrong-role/ambiguous targets; unresolved core graph makes negative traces unknown. Preserve source status/ID/path/line and path truncation. Do not infer observed behavior, emotion, causality, guaranteed experience, or whole player story. Viewer remains read-only; no automatic repair or framework review.

## Native skill prerequisites and ranks

Use `skill-atoms --skill-atoms ALIAS=FOLDER --json` or `view skill-atoms` with the same explicit input. Optional `--from ALIAS/NAME` roots prerequisite/unlock chains and shared-link comparison candidates; repeated `--known ALIAS/NAME` supplies explicit hypothetical knowledge. Inspect `exploration` for eligibility and missing prerequisites; never promote an unlock to observed learning. See [combination contract](../native/README.md#skill-understanding-combinations-and-unlocks). See [native skill contract](../native/README.md#skill-prerequisites-and-ranks). Default graph enforces top-to-bottom prerequisite chains during settling/dragging; left-to-right and conceptual rank layouts are available. All statuses, null/negative/zero ranks, isolated nodes, and raw unresolved links remain visible. Cycles are conflicts, not repaired edges. Authored rank, prerequisite depth, minimum/display rank, and incomplete ancestry stay distinct.

CLI source analysis and viewer draft analysis share deterministic computation and source paths. Skill view can edit rank/prerequisite connections in memory, show divergence from loaded baseline, undo to disk sync point, and emit JSON proposals on finish or to explicit report file. It never writes authoritative records. Headless CLI still reports disk state, not an unreported draft. No observed learning judgment or generic validation.

Before applying a handoff, inspect `applied:false`, net changed fields, loaded/current-disk/edited values, availability, errors, and source identity. Report is not a lock or approval to overwrite. Recheck current source and use [Capture](capture.md) for authorized Attendant/precise edits; do not turn unrelated external changes into reversions. Refresh clean viewer or reopen afterward.

## Native challenge route opportunities

Use `scaffolding --skill-atoms ALIAS=FOLDER --challenges ALIAS=FOLDER --json`. Read [route contract](../native/README.md#challenge-route-opportunities). `requires`/`tests_mastery` are demands; only earlier `introduces`/`practises` count as declared support. Check transitive prerequisites on every declared entry-to-visit route, including revisits. Rank and levels do not establish traversal.

Inspect `checks`, witness paths, source role links, and coverage flags. All/some/absent concern prior opportunities for named skill, not observed learning or overall readiness. Same-challenge introduction/demand stays an informational single-shot question; do not demand earlier teaching. Missing entries/broken route links/unreachable demands yield unknown coverage. Inspect `requirements_complete` for missing/cyclic prerequisite ancestry. Witness omission/path-prefix truncation is explicit; do not present retained suffix as a whole route. No game-state gate evaluator, physical layout solver, automatic repair, or source writes.

## Example SQL

After confirming default aliases and fields, these can be sent to Attendant's public `query --sql` command:

```sql
SELECT name, rank, prerequisites, mechanics, source_path
FROM "skill-atoms" WHERE status = 'accepted' ORDER BY name;
```

```sql
SELECT name, nouns, verb, context_nouns, coverage, mechanics, source_path
FROM interactions WHERE status = 'accepted' ORDER BY name;
```

```sql
SELECT name, entry, next, introduces, practises, requires, tests_mastery, source_path
FROM challenges WHERE status = 'accepted' ORDER BY name;
```

```sql
SELECT name, value_type, value, expression, unit, source_path
FROM "control-vars" WHERE status = 'accepted' ORDER BY name;
```

Use ordinary SELECTs for other relevant collections after inspecting their fields. Quoted identifiers matter for aliases containing hyphens. These inventories are source retrieval, not design judgments or native semantic checks.

## Term resolution

Compare declared canonical terms/aliases after trimming/collapsing whitespace and case-insensitive comparison only. Preserve punctuation and plural distinctions. Inspect full relevant glossary; if matching identities conflict, report ambiguity and do not pick one. Unknown is not permission to invent a synonym. Expose inactive status rather than silently reactivating a term. Do not claim a global uniqueness check from a partial lookup.

## Scoped impact

For `What changes if pushing costs two energy?`, inspect variable, resource, mechanic/case references, affected challenges, and experiment expectations. Explain declared dependencies and conditional hypotheses. Do not adopt value or execute a model. No transitive/game-state impact engine exists yet; inspect scoped source and state limits.
