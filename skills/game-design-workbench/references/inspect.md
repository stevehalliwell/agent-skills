# Inspection and change questions

Use scoped recipes for declared relationships and change questions.

```sh
node <workbench>/scripts/workbench.mjs query matrix --project <project> --json
node <workbench>/scripts/workbench.mjs query skill-atoms --project <project> --fields name,rank,prerequisites --json
node <workbench>/scripts/workbench.mjs query routes --project <project> --json
node <workbench>/scripts/workbench.mjs query mda --project <project> --json
node <workbench>/scripts/workbench.mjs query decisions --project <project> --json
node <workbench>/scripts/workbench.mjs resolve --term "box" --project <project> --json
```

## Query semantics

- `matrix`: accepted explicit cases expanded per applicable noun. Multiple cases per pair remain separate; contextual nouns stay in source. Empty Cartesian cells are not generated. `coverage` distinguishes defined, unknown, excluded, not_applicable; inspect prose to distinguish no effect from unknown effect.
- `skill-atoms`: accepted authored ranks/prerequisites/mechanic links. No computed rank placement, cycle detection, descendant rewrite, capability estimate, or graph layout.
- `routes`: accepted entry/next links and introduction/practice/required/mastery lists. Every next edge is declared structural opportunity, not geometry or game-state reachability. Do not assume list order is a route.
- `mda`: accepted aesthetics/dynamics/systems/mechanics inventory with `link_kind` and declared links; one dynamics row per relationship kind. Agent can follow/reverse links or notice disconnected inventory records. No emotional inference.
- `controls`: accepted typed declarations, literal values, units, and formula text. No evaluation, conversion, bound validation, temporary overrides, or exports.
- `decisions`: all dispositions, including rejected/proposed. Unchosen is not rejected.
- `glossary`: all canonical terms/aliases/definition targets with disposition.
- `evidence`: experiment inventory with stage, kind, disposition, and source path. Read hypothesis/expectation/observation/result headings for evidence and provenance.
- `query records --collection <alias>`: all fields/all dispositions for configured collection. Use for resources, experiments, measures, context links, and other details not selected by a recipe.

SQLite link arrays remain JSON text in query cells. This is a projection format, not a second source schema. Respect `truncated`/coverage; increase limit or narrow source inspection before drawing completeness conclusions. Missing config/contracts fail; invalid source is never disguised as zero rows.

## Source-safe projections

Helpers copy source into a temporary project and call installed Attendant skill's public runner against that copy. No Attendant files are copied. Any mechanical corrections occur only in that copy and are disclosed as `projection_corrections`. Sources are hashed before/after; intervening changes abort. No live `.attendant/` is required or created by workbench query/validate. Raw Attendant query preparation can mechanically correct live records; do not substitute it for read-only review.

## Term resolution

`resolve` trims/collapses whitespace and compares case-insensitively. It preserves punctuation/plural distinctions and reports unknown terms instead of guessing. Multiple matching identities return ambiguous with `term.collision`; do not choose one automatically. Resolved inactive records retain their disposition and are not reactivated. A collision elsewhere in glossary is returned even if the selected term itself resolves.

## Scoped impact

For `What changes if pushing costs two energy?`, inspect control, resource, mechanic/case references, affected challenge descriptions, and experiment expectations. Explain declared dependencies and conditional hypotheses. Do not adopt proposed value or execute a model. No transitive impact checker is installed; use complete scoped source inspection and report its limits. Request explicit revision or experiment authority only if needed to continue beyond inspection.
