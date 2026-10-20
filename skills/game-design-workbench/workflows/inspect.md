# Inspection and change questions

Read live schemas/usages before querying collection fields. Use Attendant directly; write SQL suited to actual aliases and the question, not a workbench query wrapper. Validate with `--no-correct` first: if preparation would correct source, inspect Markdown directly or use an ordinary temporary copy when SQL is necessary. Do not authorize collateral edits merely to obtain a projection.

## Inspection topics

- **Matrix:** accepted interaction cases grouped by applicable noun and verb. Preserve multiple cases and contextual nouns. Coverage distinguishes defined, unknown, excluded, and not applicable; read prose to distinguish no effect from unknown effect. Empty Cartesian cells are not automatically defects.
- **Skill atoms:** authored ranks, prerequisite AND sets, and mechanic links. Preserve null/negative/zero ranks. Inventory does not compute placement, detect all cycles, or estimate capability.
- **Routes:** entry/next and introduction/practice/required/mastery links. Edges are declared opportunities, not physical or game-state reachability. List order is not a route.
- **MDA:** inspect aesthetics, dynamics, systems, and mechanics; follow/reverse their declared links. Systems are not dynamics; links do not prove emotion.
- **Control variables:** named configuration types, literal values, units, and formula text. Resolver/import/export remain deferred; do not evaluate expressions using shell or arbitrary code.
- **Glossary:** canonical terms/aliases, definition targets, and status, including inactive records.
- **Evidence:** experiment stage/kind and source; read local measures, hypotheses, expectations, method, observations, results, and provenance. Proposed is not executed.
- **Records:** inspect all relevant fields/dispositions, including resources, experiment-local measurements, existing project decisions, and case context omitted by a narrow inventory.

SQLite array cells are JSON text. Inspect complete arrays; do not infer missing relationships from truncated rows. Narrow query or increase limit and disclose incomplete coverage. Invalid/missing configuration is not zero results.

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
