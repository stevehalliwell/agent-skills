# Bootstrap CLI contract

Execute `node <workbench>/scripts/workbench.mjs <command> --project <root>`.

## Commands

```text
help | --help
capabilities
lenses
setup plan [--mapping map.json] [--fresh]
setup apply --approve <plan-id> [same mapping/fresh options]
query <recipe>
query records --collection <configured-alias>
resolve --term <text>
validate
snapshot [--include <comma-separated-extra-source-paths>]
report save --lens <id> --input <findings.json> --snapshot <snapshot.json>
report show <run-id>
```

Common: `--project <root>`, `--json`, `--format json|text`, `--output <new-file>`. Queries also accept `--fields <comma-separated-row-fields>` and `--limit <1..10000>` (default 1000). Row field names are projected column names, not guessed domain fields. Extra positions, duplicate/unknown flags, conflicting formats, and invalid limits fail explicitly. Use absolute paths for unambiguous input/output; relative input/output paths resolve from shell working directory, not `--project`.

JSON is default for non-TTY stdout or `GD_AGENT=1`; explicit `--format text` overrides environment. Explicit `--json` conflicts with `--format text`. No prompts, ANSI, progress, or decoration in machine output. Diagnostics go to stderr; one versioned JSON envelope goes to stdout. `--output` writes envelope exclusively to a new file and leaves stdout empty on success; it cannot target canonical source/config/tool files. Errors still produce stdout envelope and stderr diagnostic. No overwrite option.

```json
{"schema_version":1,"ok":true,"data":{"rows":[],"returned":0,"truncated":false},"coverage":{"status":"partial"},"errors":[]}
```

Query/validate `provenance` is compact fingerprint/tool hash/source count/includes; `snapshot` emits full path/hash inventory. Results contain no aggregate design score. `query` results are inventories; code 0 does not mean valid design or empirical success.

Exit codes:

- `0`: requested operation completed; may include unresolved agent questions or ambiguous term results.
- `1`: read-only validation completed with source diagnostics.
- `2`: usage/approval/contract/dependency/runtime failure; `errors[].code` and message identify cause. Pending corrections alone are disclosed, not automatically applied by workbench validation.

## Boundaries and recovery

`validate` performs Attendant flat-schema/supplied type/reference checks against temporary copy. Optional/untyped null declarations do not enforce workbench scalar subtypes. Native alias/identity invariant, rank/cycle/route, formula/unit, and expectation semantics are not implemented here. Read coverage; do not call generic validation a full model conformance check.

Recipe definitions and read-only mechanism: [inspection query semantics](inspect.md#query-semantics) and [source-safe projections](inspect.md#source-safe-projections). `SOURCE_CHANGED` requires fresh inspection, not stale-write retry. Busy/filesystem cleanup failures remain errors.

Setup supports compatible dedicated in-project collection directories and unique role aliases. Root-as-collection, path escapes, symlinks/reparse entries, unsupported entry kinds, and malformed registries fail. No automatic local schema migration or conflict override. Update conflict/recovery procedure: [setup update and local edits](setup.md#update-and-local-edits).

Initial engineering limits: 5000 source files, 2 MiB per source/JSON-input file, 32 MiB total snapshot, 30 s per Attendant child, 16 MiB child output. They bound bootstrap cost, not game quality. Oversize input fails rather than silently truncating source. Query row limits explicitly mark partial output; matrix expansion obeys same limit. Source recursion includes all files in configured collection folders; keep private/unrelated large files outside them. Extra snapshot documents must be regular in-project files.

Unavailable commands fail `CAPABILITY_UNAVAILABLE`: native C99 inspect/impact/semantic validation, formula resolution/control exports, native views, simulation/Monte Carlo/sweeps/replay, and expectation comparison. Guidance may describe a proposed experiment; helper does not compile or execute it. Later delivery must add real implementations, not aliases that pretend projections are analysis.

## Maintainer tools

Not installed in project snapshot:

```sh
node scripts/build-packs.mjs --check
node scripts/build-packs.mjs --write
node scripts/build-example.mjs --templates --output <empty-directory>
node --test test/workbench.test.mjs
```

Run from canonical skill folder. Pack sources/citations/prompts live in `scripts/pack-data.mjs`; expanded explanations, procedures, examples, counterchecks, and traps live in `scripts/pack-guidance.mjs`; `--check` is read-only, `--write` regenerates pack Markdown/catalog. Fixture generator requires an empty output directory and creates deterministic tutorial identities/data, not real play evidence. Generated tutorial source is committed; installed runtime excludes maintainer generators/data and tests.
