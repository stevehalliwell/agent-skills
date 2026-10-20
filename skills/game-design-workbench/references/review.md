# Selected review and provenance

## Review

1. Use requested lens and scope. If selection remains material, ask one focused question; no all-lenses sweep. Read selected provisional guide; identify relevant coverage gaps before using it. Done: relevant questions/limits are known.
2. Inspect actual source/usage and prior alternatives/rationale. Capture snapshot **before** review; include relevant unconfigured documents explicitly:

   ```sh
   node <workbench>/scripts/workbench.mjs snapshot --project <project> --include design-notes.md --output <new-snapshot.json> --json
   ```

   Include project experience brief (for example `--include README.md`) when it informs review. Compare overall intent/player voice with individual MDA aesthetics; record support, contradiction, drift, and evidence without equating those lenses. Omit `--include` only when configured sources suffice. Output file must be new and outside canonical collections/config/tools. Snapshot inventories source/schema/usage/templates, registry, project instructions, installed manifest, tool/guidance hashes, and installed Attendant skill fingerprint. Dependency changes make old reviews stale even when workbench files are unchanged. Done: review has frozen hash inventory; this is not a frozen copy of file content or a Git checkpoint.
3. Run only useful available queries, then inspect source bodies. Separate malformed/broken data, required missing relationships, unknowns, opportunities, conflicts, unverified hypotheses, and deliberate exclusions. Missing optional data is not invalid. Done: findings cite exact source field/section and distinguish fact from interpretation.
4. Keep deterministic findings separate from agent judgments. A bootstrap recipe is a data projection, not a framework evaluator. Do not label manually derived rank/route observations deterministic C99 findings. Done: reasons/evidence/coverage are explicit.
5. Present useful questions and limitations. Save dated report when requested using [finding input](../templates/review-input.json), or return concise review in chat. Done: reviewed scope, findings, missing inputs, and unsupported checks remain visible.

## Save and reopen

```sh
node <workbench>/scripts/workbench.mjs report save --project <project> --lens skill-atoms --input <findings.json> --snapshot <snapshot.json> --json
node <workbench>/scripts/workbench.mjs report show <run-id> --project <project> --json
```

Submit plain input JSON; snapshot may be raw metadata or CLI envelope. A lens must match one catalog ID. Fields:

- `scope`: string array naming reviewed records/questions.
- `findings`: array with `code`, `origin` (`agent` or `deterministic`), `kind`, `severity` (`error`, `warning`, `info`), `records` (stable lowercase UUID IDs, not mutable terms/names), `sources`, `reason`, `suggested_investigation`; optional stable `trigger_key` separates distinct triggers for same rule/records/fields.
- `sources`: project-relative captured path plus `field` or `section`. Include external-to-collections documents in pre-review snapshot; uncaptured sources are rejected.
- `kind`: `invalid_broken`, `missing_required_relationship`, `unknown_unresolved`, `possible_opportunity`, `conflicting_claims`, `unverified_hypothesis`, `deliberately_excluded`.
- `coverage`: object with `status: partial|complete` **within declared scope**, performed/omitted checks and assumptions. `limitations`: string array. No finding is not completeness evidence.

`origin` is a declaration, not certification. Use deterministic only with actual repeatable tool evidence; cite tool/check and output in reason. Native semantic checks are unavailable in this release regardless of input labeling. Saver validates structure/source match, not design truth or evidentiary adequacy.

Results: `game-design-results/YYYYMMDDTHHMMSSmmmZ-<suffix>/run.json`, `review.json`, `review.md`. Exclusive directories prevent overwriting older runs; incomplete writes retain `writing` metadata and cannot be reopened as complete reviews. Historical findings and timestamps are never rewritten during freshness checks.

Finding IDs hash rule code, stable record IDs, source field/section keys, and optional trigger key—not timestamp, severity, prose, or file path. Retain stable trigger keys. Reports record source hashes and package fingerprint, pack version, creation time, and available Git context. Git context is not a reproducibility checkpoint and saving never commits.

`report show` returns original report with separate `historical`, `matches_current_source`, and `freshly_run: false`. A hash match does not turn old interpretation into a new review. Edited source/guidance makes comparison false. Saving against changed baseline fails; re-inspect rather than relabel stale findings. Hash inventory alone cannot reconstruct deleted source; use ordinary Git or an explicitly approved snapshot-copy workflow for reproducibility.
