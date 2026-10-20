# Setup and updates

## Establish source owners

1. Inspect project instructions, `.pi/attendant.tables`, live schemas/usages, and existing design documents. Read loaded Attendant skill's collection workflow. Existing Markdown stays in place. Done: existing collections and possible migration sources are known.
2. Identify one live concept owner; default `game-design/concept.md`, or explicitly bind an existing document, including README. Identify current project mode from that document. Ask only when mode or ownership is unresolved. Done: concept location and mode are explicit.
3. Propose concrete collection paths/aliases and source changes. Default is `game-design/<role>` as `<role>` for the 14 roles in distribution templates. Reuse compatible existing collections after reading their contracts. Existing records require Attendant's migration workflow, not blank setup over them. Done: user can see additions, conflicts, and preserved sources.
4. Follow Attendant's confirmation and empty-collection workflow. Author approved schemas/usages/templates afterward from distribution templates; adapt references to actual aliases. No tutorial facts enter live design. Done: approved collections exist and match the proposed contracts.
5. Link concept near README's beginning without replacing existing navigation. In project `AGENTS.md`, identify concept path, any non-default role aliases, and use of installed `game-design-workbench` and `attendant` skills. Keep `.pi/attendant.tables` authoritative for collection discovery. Done: another agent can find actual owners without a manifest.
6. Inspect diff and use non-correcting Attendant validation. Commit completed authorized concept changes under project Git policy. Report unresolved contracts and checks not implemented. Done: changes are reviewable and no unrelated source was altered.

Partial models remain useful without setup. Empty optional collections are valid. Missing/incompatible contracts are not empty query results.

## Copying and updating

Use installed skill directly. If project needs local copies, copy only wanted guidance/templates and actual domain-tool sources; record their location in project instructions. Do not create a second configured tutorial model, automatic package installer, or hash manifest.

For an update, compare existing project files with new skill material and available Git history. Merge useful changes with agent judgment. Preserve local schemas, usages, records, unknown files, and project-specific edits. Present material conflicts rather than silently resolving them. Source migration and deletion need their own authorized scope; a skill update does not authorize either.

Review exact diff, validate affected records, and make a scoped commit when authorized. On partial failure, inspect actual files and Git diff before continuing. Do not reset unrelated work or copy a stale whole-file version over new edits.

## Older workbench installations

Earlier releases installed `tools/game-design-workbench/` with a CLI and manifest. This layer is retired; use Attendant and Git directly. Before removing an old copy, inspect it for project-specific edits, unique data/code, and references from instructions. Merge or relocate wanted content first. Remove only identified obsolete files within authorized scope. Preserve reports, experiment evidence, and unrelated project tools.

Earlier model changes still require deliberate data migration:

- `skills` became `skill-atoms`.
- Hypotheses, expectations, and observations became headings in owning experiments.
- `controls` became `control-vars`; update field names, schema references, record references, and formula text together.
- Measure definitions/provenance belong in owning experiments; no separate measures collection.
- No workbench decisions collection. Retain wanted alternatives in domain records or existing project decisions, or delete only when requested.

Preserve UUIDs, authored nulls, rationale, and historical evidence. Follow live schema/usage and Attendant's migration workflow. Do not re-run historical fixture-conversion scripts against user projects.
