# Native Attendant exploration

C99 provides native `sync`, `schema`, `query`, and `search` over a compatible SQLite projection. Commands reuse unchanged projections or rebuild them from normalized Markdown when needed. Native and existing `.mjs` commands can consume either builder's projection. This is a read-only-source exploration, not full CLI or YAML parity.

## Build and check

Windows x64, Git Bash, installed CJIT or Zig. No downloads during build. `CJIT` and `ZIG` override compiler executable paths. Build paths are explicit and independent; Zig does not require CJIT or `build.h` bootstrap.

From repository root:

```sh
bash exploration/attendant-native/build.sh cjit
bash exploration/attendant-native/build.sh zig
node exploration/attendant-native/check.mjs \
  exploration/attendant-native/build/projection-cjit.exe \
  exploration/attendant-native/build/projection-zig.exe
```

Run native read-command checks as well:

```sh
node exploration/attendant-native/read-check.mjs \
  exploration/attendant-native/build/projection-cjit.exe \
  exploration/attendant-native/build/projection-zig.exe
```

Run lifecycle checks:

```sh
node exploration/attendant-native/prepare-check.mjs \
  exploration/attendant-native/build/projection-cjit.exe \
  exploration/attendant-native/build/projection-zig.exe
```

`read-check.mjs` runs projection checks first; `prepare-check.mjs` is independent. Node.js >=24.10.0 is needed only for the existing CLI and comparison harnesses. Native commands need no Node. Lifecycle checks run missing-projection commands with PATH restricted to Windows System32. Preferred `json-build` writes JSON; `libpdjson5` reads bound parameters in strict JSON mode with caller-backed allocation.

`check.mjs` copies fixtures into ignored `build/review/` paths, including spaces and Unicode. It rebuilds those generated copies on rerun; never point it at original source collections. Each compiler is checked with relative configuration and with absolute paths, tab-separated aliases and trailing separators. Copies include a real decision from `proto-framework`, synthetic task records, CRLF, no-final-newline, enums, defaults, dates, arrays, untyped values, references, hidden records, Unicode, block text and numeric edge cases.

For each scenario it:
1. Builds a reference DB using the current runner and freezes source hashes.
2. Removes disposable projection, generates native output, and compares collection/system/FTS logical rows and column declarations before the runner can refresh anything.
3. Requires existing `.mjs sync` to return `state: "reused"`.
4. Compares `schema`, parameterized query, join, ranked search and trigram search results with the reference; checks hidden content is excluded.
5. Verifies source hashes stay unchanged. Malformed YAML, duplicate keys, aliases and nested values must fail without changing source or an already-published projection.

Summary evidence: `build/review/results.json`. SQLite file bytes are not expected to match. No performance target or speedup claim.

## Review demo

After checks, from repository root:

```sh
project='exploration/attendant-native/build/review/projection-cjit-relative café Ω'
node skills/attendant/scripts/attendant.mjs sync --project "$project"
# Expect state: "reused", collections: 2, records: 3, diagnostics: []
node skills/attendant/scripts/attendant.mjs query --project "$project" \
  -s 'SELECT name, status, priority FROM tasks ORDER BY name'
node skills/attendant/scripts/attendant.mjs search --project "$project" -q lantern
```

Native invocation for a copied, normalized fixture project:

```sh
exploration/attendant-native/build/projection-cjit.exe "$project"
```

Builder writes only `.attendant/` under the supplied project. Existing source files are read-only. Builder creates a temporary database and publishes it after completion; consumers must not run concurrently. Existing WAL/SHM sidecars or rollback journal cause refusal rather than replacement. This is not a concurrent CLI/GUI publication protocol.

## Native commands

Use the same executable. Positional project invocation still forces a full build for backward compatibility with the first slice; normal commands prepare automatically.

```sh
native='exploration/attendant-native/build/projection-cjit.exe'
"$native" sync -p "$project"  # state: reused or rebuilt
"$native" schema -p "$project"
"$native" query -p "$project" \
  -s 'SELECT name, status FROM tasks WHERE status=:status' -P '{"status":"active"}'
"$native" search -p "$project" -q lantern -c tasks -l 10
```

- `--project`/`-p` defaults to current directory. Query uses `--sql`/`-s` and optional `--params`/`-P`. Search uses `--query`/`-q` and optional `--collections`/`-c`. Both accept `--limit`/`-l`, default 100.
- `@path` and `-` work for SQL, params, search text and collection filters. Stdin can supply only one option. Params are a strict JSON object with scalar/null values; named parameters support bare or prefixed names, plus numbered `?1` names. Unbound parameters remain NULL, as in the current runner.
- `sync`, `schema`, `query`, and `search` prepare the projection in process. Reuse requires compatible layout/config/schema metadata, a successful SQLite quick check, matching source paths/collections/content hashes/size/mtime, and matching table/cache counts. Each operation scans, hashes and validates bounded source. No Node subprocess and no incremental reconciliation.
- Missing, stale or broken projections trigger full rebuilds. Additions, edits, deletions, renames, hidden-state changes and config/schema changes are detected. Hash checks also catch equal-size edits with unchanged mtime. Mtime-only changes cause a full rebuild, unlike the runner's cache-metadata-only update. Usage text stays live without triggering rebuilds.
- Preparation writes only disposable `.attendant/` data. Build completes in a temporary DB before replacement; failed validation, SQL construction or publication leaves the published DB intact. Unsupported/malformed source fails instead of returning stale results or repairing Markdown. Sequential use only; WAL/SHM sidecars and rollback journals block replacement. Native does not recover journals or remove sidecars automatically; stop consumers and use SQLite recovery or remove the disposable projection before retrying.
- After preparation, SQLite opens read-only with query-only mode and an authorizer. User SQL cannot mutate data, attach databases, read private cache/metadata, or load extensions. Only one statement; SQL comments are unsupported. Unchanged reads leave DB bytes unchanged.
- Success preserves command-specific shapes. Sync returns `collections`, `records`, `diagnostics`, `state` (`reused` or `rebuilt`); diagnostics remain empty for supported normalized input. Query/search return `columns`, `rows`, `returned`, `truncated`. Errors return `{ "error": { "code": "...", "message": "..." } }` on stdout, explanation on stderr, exit 1. No TTY-dependent format changes or prompts.
- Search keeps complete matches first, then unseen trigram matches, ordered by score and byte-order collection/ID ties, with snippets. Deterministic ties are sufficient; reproducing `.mjs` locale collation is not required. A limited result can therefore select a different tied hit. Tiny floating-point score differences between SQLite compiler builds are allowed by tests.
- Deliberate fixes: empty results retain column metadata; limit zero returns zero rows; quoted semicolons with a trailing statement terminator work. These do not reproduce existing reporting/parser bugs.
- Limits: at most 10,000 returned rows and 10 MiB of serialized rows, bounded iteration with one-row lookahead for queries; at most 4,096 retained search hits. SQLite values limited to 12 MiB, SQL text to 4 MiB, 1,024 columns and expression depth 100. A 10-million-VM-instruction budget fails costly queries with `sql-budget`. No wall-clock timeout guarantee. Result buffering uses the shared arena; capacity failures are explicit.
- Windows NLS classifies search words. General Unicode-category/version parity remains unverified. No ICU dependency is needed for search ordering.
- SQLite TEXT may contain invalid UTF-8 bytes. Native output replaces malformed sequences with U+FFFD before JSON serialization, matching the Node encoding boundary. Oversized escaped text or expanded BLOB rows are reported as truncated rather than invalid JSON or capacity errors.

`read-check.mjs` compares native schema/queries/search with `.mjs`, tests both projection producers, live usage, input transports, duplicate parameter semantics, raw-byte UTF-8, structured failures, SQL write containment, budgets and byte/row limits. Tie fixtures check deterministic native order and matching full result sets, not locale-order parity. Evidence: `build/review/read-results.json`.

`prepare-check.mjs` checks missing DB through every command, unchanged reuse, equal-stat content edits, field edits, added/renamed/deleted records, hidden records/state, config/schema/collection changes, old/broken DB recovery, source/SQL/publication failures, active WAL refusal and a genuine hot rollback journal left by a force-killed test writer. It verifies source hashes and `.mjs` reuse after native preparation. Evidence: `build/review/prepare-results.json`.

## Compatibility contract

`projection.c` mirrors current runner layout version `2`: collection columns, metadata/cache, diagnostics table, reference edges, Unicode FTS5 and trigram FTS5. Schema hashes currently use literal `\\0` separators, not NUL bytes. Cache reuse needs configured directory spelling, absolute source paths, SHA-256 and filesystem timestamps. Windows CNG supplies SHA-256. Native conversion matches Node's cache millisecond arithmetic and rounded Stats Date values on tested fixtures.

Source fixture must already be valid and normalized. Native slice rejects unsupported or malformed input rather than performing source repair. Empty diagnostic tables represent valid fixtures, not implemented general diagnostic parity. `check.mjs` verifies reference diagnostics are empty before comparing output.

Limits and deferred behavior:
- Windows only. Tested CJIT v1.2.0 and Zig 0.17.0, SQLite 3.53.0, LibYAML pinned source 0.2.5, Node v24.16.0.
- 128 MiB caller-owned arena plus 12 MiB JSON workspace; parser/SQLite allocate independently. Up to 32 collections, 128 schema fields per collection, 256 front-matter keys, 1024 scalar items per array, and 4096 records total. Each source file is at most 4 MiB. Generated SQL/path capacities also fail explicitly. These are exploration bounds, not final product limits.
- Flat mapping with scalar or scalar-array values. Aliases, anchors, explicit tags, nested structures, nonfinite values and non-decimal numeric forms are not supported. UTC date-times require seconds and optional exactly 3-digit milliseconds. Date validation and scalar resolution are not complete equivalents of the runner.
- Numeric JSON uses a small round-trip formatting path checked against fixture values, including subnormal and exponent boundaries. It is not a proven general ECMAScript number serializer. Integer-index key ordering matches the runner for FTS content.
- General invalid-record diagnostics, default repair, missing-target diagnostics, cache incrementality, arbitrary path/locale parity, simultaneous writes, long-path and UNC coverage are not delivered. Do not use against live production datasets.
- No create/update, Markdown editing, migration commands, Git awareness or viewer. No change to existing runner.

## Dependency choices and checks

Vendored provenance and notices: `vendor/PROVENANCE.md` and per-library files. Copied `include/base.h` supplies arena allocation and UTF-8 validation. `build.h` and `test.h` remain available but were not required by this slice.

SQLite builds with FTS5 and `SQLITE_DISABLE_INTRINSIC`: CJIT otherwise fails to link `__umulh`. Windows SDK `bcrypt.h` annotations do not compile with CJIT's bundled headers, so the native source declares only the three stable CNG APIs it uses. Zig uses a local global cache by default after the previously observed default-cache link failure; no user cache is deleted or repaired.

Actual checks on Windows:
- Both native builds pass.
- Four end-to-end scenarios pass: two compiler paths times two configuration variants, each with 2 collections, 4 source records, 3 projected records and empty diagnostics. All native DBs are reused by the existing runner.
- Native read checks: all four compiler/configuration scenarios passed. Each runs 23 oracle comparisons and 27 rejection cases, plus raw-byte UTF-8, expanded-row byte limits, row cap, query budget, live usage, automatic metadata refresh, producer interoperability and deterministic ties. Source and unchanged snapshot bytes stay unchanged during reads.
- Native preparation checks: 34 named lifecycle cases passed in each of four compiler/configuration scenarios. Includes missing-projection operations with Node absent from PATH, equal-stat edits, collection changes, native-built DBs reused by `.mjs`, failed SQL steps, blocked publication, active WAL and hot-journal preservation. Generated copies only; no original source edited.
- Existing `node --test skills/attendant/test/attendant.test.js`: 7 passed.
- Independent read-only review found initial projection config/key-order issues and native read size/encoding/duplicate-binding issues. Fixes gained regression coverage. Exact search tie collation was withdrawn as a requirement; unnecessary ICU work removed. Final inspection found no remaining actionable issue within documented scope.

Projection and read slices reviewed and committed. Native preparation is implemented and reviewed. Independent read-only review found journal protection, statement-cleanup and missing-edge-table gaps; fixes gained executable regressions. Follow-up inspection found no remaining actionable defect in fixes. Source writes and broader parity remain separate work.
