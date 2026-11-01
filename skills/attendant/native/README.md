# Native Attendant

Windows-first C99 CLI and Markdown workspace GUI over shared core. Markdown is authoritative; `.attendant/` is disposable SQLite projection state. Runtime needs no Node.js. Third-party GUI C++ stays behind C ABI/DLL.

Status: product import under qualification. Existing JS skill launcher remains active; native rollout requires explicit approval. Use owned workspace copies for evaluation, not authoritative projects.

## Build and run

Read [build reference](../references/build.md). It covers installed tools, explicit dependency acquisition, offline builds and missing-build handling. No automatic installs/downloads.

```sh
# From this directory, Windows Git Bash:
bash build.sh zig
build/attendant-zig.exe --help
build/attendant-zig.exe schema -p /path/to/owned/workspace
```

Commands: `doctor`, `validate`, `sync`, `schema`, `query`, `search`, `add-table`, `create`, `update`. Output is one JSON value; failures also explain on stderr. Exit 0 means success, 1 means operational/validation failure. No TTY-dependent output or prompts. CLI flags and JSON examples: [native commands](../references/native-commands.md).

**Reads are not universally source-read-only.** `schema`, `query` and `search` apply mechanical metadata corrections before preparing projection, reporting corrections. `doctor` never edits source/state. `validate --no-correct` previews; `validate --strict` stays read-only and fails for diagnostics or pending corrections. Default `validate` applies corrections. `sync` prepares projection without source correction.

## Layout

- `apps/cli/`: CLI entry point and handlers.
- `apps/gui/`: GUI entry point, document workspace and reveal/inspection IPC. [GUI contract](apps/gui/README.md).
- `src/`: shared core, native source API and C support header. App-specific C99 unity builds; no CLI entry point in shared core.
- `vendor/`: pinned C dependencies and maintained GUI bridge. [Provenance](vendor/PROVENANCE.md), [GUI pins and patches](vendor/gui/PROVENANCE.md).
- `tests/`: focused direct native contracts, feedback fault checks, GUI model/input checks and owned fixtures. No JS equivalence oracle.
- `tools/`: explicit GUI acquisition/generation, build support and repeatable performance probe.
- `build/`: ignored binaries, external GUI checkouts/assets, caches and current check output.

No build/runtime dependency on historical exploration/spike trees. Do not overwrite a binary/DLL mapped by a running viewer; use a fresh `ATTENDANT_GUI_VARIANT` for GUI builds/checks.

## Source and data contract

- Flat YAML front matter: scalar values and scalar arrays only. Reject duplicate keys, nested structures, aliases, anchors, explicit tags/documents, embedded NUL and invalid UTF-8. Preserve Unicode without normalization; CR/LF are line breaks, NEL/LS/PS remain scalar content. Bodies remain human Markdown.
- Date: `YYYY-MM-DD`. Datetime: `YYYY-MM-DDTHH:mm:ss.sssZ`, valid calendar/time, years 1601–9999. No offset guessing or date normalization.
- Decimal integers retain exact `i64`/`u64`; decimal point/exponent selects finite `f64`. Reject overflow, nonfinite values and nonzero float underflow to zero. SQLite scalar values/parameters reject integers above `INT64_MAX`; JSON-backed arrays retain `u64`. Use lossless JSON parsing for large integer results.
- `create`/`update` accept nonempty arrays of `{name, fields}`. Only declared fields plus `desc`/`tags` are writable. Defaults/template apply to create. Reject invalid types/references, immutable fields, unsafe names, collisions and invalid prospective batches before publication.
- Update/correction splices changed value spans; preserve unrelated bytes, comments, newline style and body. Formatting inside replaced values may change. Parse/validate serialized values before staging.
- Flush exclusive sibling temporary files; compare captured source before replacement. Publication is atomic per file, not across a batch. Completed writes survive later failures and are reported as `written`; source remains committed if projection refresh fails. No universal concurrent-writer guarantee.
- Add-table/first feedback bootstrap publish owned seeds before config registration. Failed cleanup preserves changed/busy files and reports survivors. Never adopt conflicting managed data.
- Native source API is single-threaded and recoverable; see `src/native-api.h` for ownership, lifetimes and request/result contracts. GUI source reads/saves/feedback do not require preparing SQLite.

## Projection and limits

Full bounded source scan/hash/type validation remains required before cache reuse or changed-only reconciliation. Never reconstruct Markdown from cached JSON. Detect equal-stat edits, additions, deletions and hidden transitions. Cold caches defer FTS until first search; later edits maintain both indexes.

SQLite uses WAL/NORMAL, 2000 ms busy timeout and transactional projection updates. Recent cache commits may be lost after power failure; Markdown remains authoritative. Full rebuild uses temporary DB and safe replacement; active/nonempty sidecars are not blindly removed. Stop consumers before manual disposable-state removal.

After preparation, user SQL has a read-only connection and authorizer: one statement, no writes, attach, extensions or private cache/metadata access. Search combines full-text and trigram results with deterministic ties.

Bounds: 128 MiB caller arena plus 12 MiB JSON workspace; parser/SQLite allocate separately. At most 32 collections, 128 fields/schema, 256 keys/record, 1024 scalar array items, 4096 records and 4 MiB/source file. Combined arena may fail earlier. Query: 10,000 rows, 10 MiB serialized rows, 4 MiB SQL, 1024 columns, expression depth 100 and 10-million-VM-instruction budget. Search retains at most 4096 hits. Capacity failures are explicit; these are not hard wall-clock guarantees.

## Checks

From this directory, after builds described in build reference:

```sh
node tests/core/cli-check.mjs
node tests/core/api-check.mjs
node tests/gui/check.mjs
node tools/benchmark.mjs 5
```

Checks create owned copies under `build/`; never target live projects. Current results: `build/cli-check-results.json`, `build/api-check-results.json`, `build/gui-check-results.json`. GUI manifests bind actual inputs/binaries to checks. Model assertions remain active even if compiler defines `NDEBUG`.

Benchmark measures warm synthetic fixture launch-to-exit, excluding build/copy/warmup/result checks, rotating serial compiler order. Reports samples and min/median/max. It is not real-project or GUI footprint qualification.

## Remaining limits and history

Known renderer failure abandons unsafe frame but loses unsaved tabs/composer drafts. Recovery explicitly deferred. Git integration, migration/import, coordinated concurrent source writers, wider Unicode/glyph/long-path/access-denied/DPI/device-loss and long-run/power-loss qualification remain outside demonstrated coverage. Automated GUI checks are not final human rollout acceptance. Startup/idle/interaction footprint is not qualified.

Accepted runtime spike: `330136285cb1470ccecc00ca244894c9de2122a3` on `feat/shared-markdown-gui`. Prior JS Attendant remains available at `330136285cb1470ccecc00ca244894c9de2122a3:skills/attendant/scripts/attendant.mjs`; no separate JS fallback is planned after native rollout. Current decisions/delivery: [TODO](../../../TODO.md).
