# Setup, binding, and updates

## Plan before apply



1. Inspect target root, existing registry/schemas/usages, manifest, and unconfigured Markdown. Read required Attendant skill's `references/collections.md`. Existing source stays in place.
2. Resolve `<workbench>` to supplied skill folder. Prepare combined proposal:

   ```sh
   node <workbench>/scripts/workbench.mjs setup plan --project <project> --json
   ```

   Plan lists new collections, role mapping, exact file changes, dependency checks, conflicts, and candidate sources needing migration. Plan itself writes nothing. Default mapping: `game-design/<role>` configured as `<role>`.
3. Present scope: empty role collections; live schema/usage/body templates; project-local `tools/game-design-workbench/` snapshot; manifest; managed `AGENTS.md` block; generated-cache ignore rules. No tutorial records are added. Ask for combined proposal approval using focused choice. Approval must include any source migration/fresh decision.
4. Apply same options with returned content-derived plan ID:

   ```sh
   node <workbench>/scripts/workbench.mjs setup apply --project <project> --approve <plan-id> --json
   ```

   Stale approval fails before writes. CLI never prompts; an ID is a mechanical guard, not independent proof of human consent. Agent must obtain the actual approval first.
5. Inspect returned paths and registry. Run installed read-only `validate` and one appropriate empty query. Report unknown/unsupported semantic checks. Stop on source conflicts; do not overwrite or silently widen schema.

## Existing collections

Default reuse occurs when a role already has a collection alias matching its role name. For other aliases, prepare JSON mapping with only known roles:

```json
{"skill-atoms":"abilities","challenges":"encounters"}
```

Pass `--mapping <map.json>` to both plan and apply. Existing aliases must already be configured; new aliases are created at `game-design/<alias>`. Role aliases must be distinct. Existing collection contracts must include compatible required fields; extra fields/local usage/templates remain untouched. Query helpers read installed mapping, not guessed directory names.

Unconfigured frontmatter candidates block apply until migration or an explicit fresh-design choice, including root notes alongside an existing registry. Candidate scan descends at most two directories and skips hidden paths, dependencies, tools/vendor/build, and configured collections; inspect supplied deeper source paths separately. No candidates is not proof no prior design exists. Use required Attendant skill's `references/migration.md` for migration; review/adopt its plan separately. To intentionally create empty workbench collections while preserving old notes unchanged, include `--fresh` in both approved setup commands. This is an explicit acknowledgment, not a default.

An existing partial model can still be discussed without setup. Helpers needing absent/incompatible contracts return a named blocker. Empty optional collections are valid; absent core collection configuration is not treated as an empty query result.

## Legacy collection name

Earlier snapshots used `skills` for game-design skill atoms. Current role/default alias, directory, and query are `skill-atoms`; no old query alias remains. Existing legacy manifests fail `COLLECTION_RENAME_REQUIRED` before setup can create a duplicate collection.

Explicitly migrate collection directory/config, schema references, record references, glossary targets, role mapping, and managed guidance together. Preserve record UUIDs and frozen reports/baselines; old evidence remains historical/stale. Do not bypass local-edit guards by changing hashes. Canonical repository maintainer tool `scripts/rename-workbench-skill-atoms.mjs --help` handles only its fixture/source, not arbitrary user projects.

## Experiment consolidation

Model version 2 replaces separate hypotheses, expectations, and observations collections with experiment headings and frontmatter `stage`. Old installed roles fail `EXPERIMENT_MIGRATION_REQUIRED` before setup writes. Explicitly migrate records/references/schema/registry and retain source identities, criteria, provenance, and authored unknowns. Do not merely remove roles from manifest or delete collections to bypass this gate.

Canonical maintainer migration `scripts/consolidate-workbench-experiments.mjs` operates only on this repository's pristine fixture; not arbitrary user projects. Older saved reports remain historical/stale. Native execution and structured expectation analysis contracts need refinement before implementation.

## Update and local edits

Run newer supplied package's `setup plan` against project; inspect changes before approval/apply. Installed helper alone has no newer upstream bytes and cannot update itself to an unseen version.

Manifest records role mapping, capabilities, source-package hashes, and distribution hash. Existing config and records are preserved. Generated schema/usage/templates are not silently replaced. Locally edited installed tool/guidance files block replacement, including apparently compatible local modifications. Retired managed files are proposed as explicit deletions only when their hashes still match prior manifest. Modified retired files block update; unknown local files remain. This also removes legacy vendored Attendant copies. Reconcile deliberately; do not change stored hashes merely to bypass conflict detection.

Apply is ordered and per-file guarded, **not a cross-file transaction**. On partial failure, inspect reported applied paths, registry, and manifest. Do not delete user sources or blindly retry previous approval. Re-plan actual state; preserve incomplete installation evidence. Manifest is written after its planned dependencies; retry remains subject to reviewed conflict handling.

Only rebuildable `.attendant/` and `game-design-results/**/artifacts/` are ignored. Review metadata/findings are not automatically ignored or force-added to Git. No setup command commits, merges, pushes, or executes a simulation.
