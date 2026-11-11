# Native Attendant

C99 CLI and Markdown workspace GUI over shared core. Markdown is authoritative; `.attendant/` is disposable SQLite projection state. Runtime needs no Node.js. Third-party GUI C++ stays behind C ABI/native library.

Status: active Attendant implementation for Windows x64 and macOS arm64. Skill invokes native CLI directly; GUI is separate. No JS runtime or fallback. Build on demand with installed compiler; see build reference. Use owned workspace copies for checks/evaluation.

Platform qualification before native test migration: macOS arm64 Zig/CJIT CLI, source/feedback API and interactive GLFW/OpenGL3 workspace run. Combined checks passed 20 CLI groups, 94 API cases, 20 GUI/model groups (194 macOS model assertions per compiler) and 16 control groups. Bridge checks cover Markdown/images, Retina screenshots and terminal render cleanup. Windows x64 keeps Win32/D3D11; post-port CJIT/Zig rebuilds and runtime checks passed 20 CLI groups, 94 API cases and 20 GUI/model groups (193 model assertions per compiler). See [macOS build checkpoint](../references/build.md#macos-bring-up-checkpoint). Current test organization is below; migrated C suite has Windows runtime proof, not a fresh macOS run.

## Build and run

Read [build reference](../references/build.md). It covers installed tools, explicit dependency acquisition, offline builds and missing-build handling. No automatic installs/downloads.

```sh
# From this directory, Windows Git Bash or macOS Bash.
# Omit .exe from executable names on macOS:
bash build.sh zig
build/attendant-zig.exe --help
build/attendant-zig.exe schema -p /path/to/owned/workspace
```

Commands: `doctor`, `validate`, `sync`, `schema`, `query`, `search`, `add-table`, `create`, `update`. Output is one JSON value; failures also explain on stderr. Exit 0 means success, 1 means operational/validation failure. No TTY-dependent output or prompts. CLI flags and JSON examples: [commands and feedback](../references/commands.md).

**Reads are not universally source-read-only.** `schema`, `query` and `search` apply mechanical metadata corrections before preparing projection, reporting corrections. `doctor` never edits source/state. `validate --no-correct` previews; `validate --strict` stays read-only and fails for diagnostics or pending corrections. Default `validate` applies corrections. `sync` prepares projection without source correction.

## Layout

- `apps/cli/`: CLI entry point and handlers.
- `apps/gui/`: GUI entry point, document workspace and reveal/inspection IPC. [GUI contract](apps/gui/README.md).
- `src/`: shared core, native source API, C support and locally extended `test.h`. App-specific C99 unity builds; no CLI entry point in shared core.
- `vendor/`: pinned C dependencies and maintained GUI bridge. [Dependency pins, licenses and patches](vendor/README.md).
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
- Add-table/first feedback bootstrap publish owned seeds before config registration. On macOS, failed setup retains published files and reports written paths; staging and empty owned directories are cleaned. Windows removes unchanged owned files through share-denying handles, preserving changed/busy files. Inspect reported files before retrying; never adopt conflicting managed data.
- Native source API is single-threaded and recoverable; see `src/native-api.h` for ownership, lifetimes and request/result contracts. GUI source reads/saves/feedback do not require preparing SQLite.

## Projection and limits

Full bounded source scan/hash/type validation remains required before cache reuse or changed-only reconciliation. Never reconstruct Markdown from cached JSON. Detect equal-stat edits, additions, deletions and hidden transitions. Cold caches defer FTS until first search; later edits maintain both indexes.

SQLite uses WAL/NORMAL, 2000 ms busy timeout and transactional projection updates. Recent cache commits may be lost after power failure; Markdown remains authoritative. Full rebuild uses temporary DB and safe replacement; active/nonempty sidecars are not blindly removed. Stop consumers before manual disposable-state removal.

After preparation, user SQL has a read-only connection and authorizer: one statement, no writes, attach, extensions or private cache/metadata access. Search combines full-text and trigram results with deterministic ties.

Bounds: 128 MiB caller arena plus 12 MiB JSON workspace; parser/SQLite allocate separately. At most 32 collections, 128 fields/schema, 256 keys/record, 1024 scalar array items, 4096 records and 4 MiB/source file. Combined arena may fail earlier. Query: 10,000 rows, 10 MiB serialized rows, 4 MiB SQL, 1024 columns, expression depth 100 and 10-million-VM-instruction budget. Search retains at most 4096 hits. Capacity failures are explicit; these are not hard wall-clock guarantees.

## Checks

From repository checkout, native root (GUI checks need builds described in build reference):

```sh
# C core/source, process contracts and CLI/API/workflow/install integration:
bash test.sh cjit
bash test.sh zig
# Retained desktop/IPC orchestration:
node tests/gui/check.mjs
# macOS transport-specific checks:
node tests/gui/control-macos-check.mjs
node tests/gui/bridge-check.mjs
node tools/benchmark.mjs 5
```

Native `test.sh` needs installed selected compiler/Bash, not Node, GUI, downloads or prebuilt CLI. It runs 47 C cases under UTC and Pacific/Auckland, 26 isolated framework scenarios, then five subprocess integration cases with 23 contract groups per compiler. Builds matching CLI/API probe automatically. Integration requires sibling `skills/init-project/templates/` from repository checkout; normal product builds do not.

Reusable `base.h` owns ordinary file/environment operations. `test.h` owns hooks, assertions, optional fixture-directory policy and bounded synchronous/caller-polled live test commands. Existing application `process_run` is unchanged. Neither header depends on Attendant. Actual CLI/API interoperability, stdin/output/deadlines, body preservation, workflows and cold isolated installation checks moved into C; four core `.mjs` harnesses were removed, not duplicated. GUI model already uses direct C assertions. Shared GUI acceptance runs all ten groups per compiler in generated C suite, including two caller-polled live previews with opposite-compiler inspection; JS keeps provenance/build/result coordination only. Total remains 20 GUI/model groups across both compilers. macOS control/bridge orchestration stays external. See [test interfaces/limits](tests/README.md) and [migration/progress](../../../TODO.md#native-test-migration).

Checks use owned fixtures, never live projects. Core evidence stays under `build/native-tests/run-*/`: timezone/framework/integration TSV, generated runners, diagnostics and build logs. Successful fixture folders are removed; failures retain/report paths. Isolated installation uses owned OS temp directory, also removed on success and retained on failure. Latest GUI report remains `build/gui-check-results.json`; macOS adds `build/control-check-results.json` and `build/bridge-check-results.json`. GUI manifests bind actual inputs/binaries to checks. macOS model checks include maximum 4 MiB selection plus 4 MiB context, delayed-reader backpressure and frozen-reply independence from later selection changes. Assertions remain active under `NDEBUG`.

Benchmark measures warm synthetic fixture launch-to-exit, excluding build/copy/warmup/result checks, rotating serial compiler order. Reports samples and min/median/max. It is not real-project or GUI footprint qualification.

## Remaining limits and history

macOS control transport is same-UID across login sessions, not same-desktop isolation or application authentication. Same-UID processes can inspect selections or impersonate endpoints. Discovery rejects ambiguous/unresponsive/unsafe endpoints; stale endpoints are not deleted during discovery. A stale same-PID endpoint can require manual removal after a crash.

Known renderer failure abandons unsafe frame but loses unsaved tabs/composer drafts. Recovery explicitly deferred. Git integration, migration/import, coordinated concurrent source writers, wider Unicode/glyph/long-path/access-denied/DPI/device-loss and long-run/power-loss qualification remain outside demonstrated coverage. Human workflow and distribution/license review complete; native skill cutover approved. Known platform/failure limits remain documented, not an exhaustive rollout gate.

Windows small-workspace footprint sanity check completed; CPU varies and is not near-zero. Measurements and limits: [current checks](../../../TODO.md#checks). No first-paint, GPU, human-input latency or large-project qualification claim. macOS footprint measurement waived; no macOS footprint claim. Run `tools/footprint-windows.ps1` with PowerShell 7; see build reference.

Accepted runtime spike: `330136285cb1470ccecc00ca244894c9de2122a3` on `feat/shared-markdown-gui`. Retired JS Attendant is available in Git history at `330136285cb1470ccecc00ca244894c9de2122a3:skills/attendant/scripts/attendant.mjs`. It is not maintained or used as fallback. Native has no migration/import command. Current decisions/delivery: [TODO](../../../TODO.md).
