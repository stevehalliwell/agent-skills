# Native Attendant exploration

C99 implements core commands: `doctor`, `validate`, `sync`, `schema`, `query`, `search`, `add-table`, `create`, and `update`. No Node runtime needed. Native and existing `.mjs` commands consume either builder's layout-2 projection on shared fixtures. Use native consumers for lossless extended integer handling.

Windows exploration, not full YAML parity or replacement rollout. **Reads can now correct Markdown metadata.** Use generated datasets for review; do not use live production collections yet. Migration/import, Git awareness, incremental reconciliation, concurrent writers and viewer remain excluded. Existing skill still uses `.mjs`.

## Build and check

Requires Windows x64, Git Bash, installed CJIT or Zig. No automatic downloads or installs. `CJIT` and `ZIG` override compiler paths. Compiler paths are independent; Zig does not need CJIT or `build.h` bootstrap.

From repository root:

```sh
bash exploration/attendant-native/build.sh cjit
bash exploration/attendant-native/build.sh zig
node exploration/attendant-native/read-check.mjs \
  exploration/attendant-native/build/projection-cjit.exe \
  exploration/attendant-native/build/projection-zig.exe
node exploration/attendant-native/prepare-check.mjs \
  exploration/attendant-native/build/projection-cjit.exe \
  exploration/attendant-native/build/projection-zig.exe
node exploration/attendant-native/cli-check.mjs \
  exploration/attendant-native/build/projection-cjit.exe \
  exploration/attendant-native/build/projection-zig.exe
node exploration/attendant-native/source-check.mjs \
  exploration/attendant-native/build/projection-cjit.exe \
  exploration/attendant-native/build/projection-zig.exe
node --test skills/attendant/test/attendant.test.js
```

`read-check.mjs` runs `check.mjs` projection comparisons first. Other suites are independent. Node >=24.10.0 is test/oracle infrastructure only. CLI acceptance also uses Windows PowerShell for read locks and fixture creation timestamps; `hold-file.ps1` holds files read-only, never edits bytes. Native command checks restrict PATH to System32 to prove Node is not needed.

Suites replace generated copies under ignored `build/review/`, including paths with spaces and Unicode. They never edit original fixture collections. Each executable runs relative and absolute config scenarios. Evidence:

- `results.json`: native versus independent `.mjs` projection rows, columns, cache, hashes, FTS and edges. Requires `.mjs sync` to report `reused`, not silently rebuild.
- `read-results.json`: 23 oracle comparisons and 27 rejection cases per scenario; both projection producers, input transports, live usage, duplicate parameter semantics, UTF-8, byte/row limits, budgets and deterministic ties.
- `prepare-results.json`: 34 lifecycle cases per scenario; missing/changed/broken projection, equal-stat edits, collection changes, malformed-source skipping, SQL/publication failure, active WAL and genuine hot rollback journal preservation.
- `cli-results.json`: 19 cases and 18 rejected batches per scenario. Covers read-only doctor/validation, strict parity, corrections, source/body preservation, Unicode encoding, defaults/templates/UUIDs, malformed and 5000-deep YAML, duplicate heuristics, schema errors, collection capacity, partial publication and post-write projection failure.

- `source-results.json`: 10 focused source-contract groups per scenario; exact integer limits/SQL params, JSON-backed `u64`, mixed enums including TEXT-affinity precision/reuse, finite floats and source round-trip, overflow rejection, 10 invalid timestamp cases, UTF-8/NEL/LS/PS preservation, cache/projection precision invalidation and unchanged configured path spelling.

SQLite file bytes need not match across builders. Source bytes and logical results are compared where applicable. No performance or footprint claim.

## Commands

After checks, use generated fixtures:

```sh
native='exploration/attendant-native/build/projection-cjit.exe'
project='exploration/attendant-native/build/review/projection-cjit-relative café Ω'
"$native" doctor -p "$project"
"$native" validate --strict -p "$project"
"$native" sync -p "$project"  # state: reused or rebuilt
"$native" schema -p "$project"
"$native" query -p "$project" \
  -s 'SELECT name,status FROM tasks WHERE status=:status' -P '{"status":"active"}'
"$native" search -p "$project" -q lantern -c tasks -l 10
```

Write demo uses separate generated CLI dataset:

```sh
write_project='exploration/attendant-native/build/review/projection-cjit-relative CLI café Ω'
"$native" validate --no-correct -p "$write_project"  # preview only
"$native" validate -p "$write_project"               # apply mechanical corrections
"$native" create -p "$write_project" -c notes \
  -i '[{"name":"demo","fields":{"desc":"New record"}}]'
"$native" update -p "$write_project" -c notes \
  -i '[{"name":"demo","fields":{"priority":2}}]'
"$native" add-table -p "$write_project" -d records/ideas -a ideas
```

Demo create/add-table names must not already exist; rerun acceptance to reset generated data.

- `--project`/`-p` defaults to current directory. `--help` prints JSON help without preparation. Positional project invocation forces a rebuild, preserving first-slice invocation.
- Query: `--sql`/`-s`, optional `--params`/`-P`. Search: `--query`/`-q`, optional `--collections`/`-c`. Both accept `--limit`/`-l`, default 100.
- Writes: `--collection`/`-c`, `--items`/`-i`. Add-table: `--directory`/`-d`, optional `--alias`/`-a`.
- `@path` and `-` read file/stdin for SQL, params, search text, collection filters and items. Only one option may consume stdin. JSON transport is strict, not JSON5. Parameters allow scalar/null values, bare or prefixed named keys and numbered `?1` names. Unbound parameters remain NULL.

## Diagnostics and corrections

- `doctor` never prepares projection or edits source/state. Checks native runtime, in-memory SQLite FTS5/trigram, config/schema, root write access, projection freshness and diagnostics. Immutable SQLite inspection avoids journal recovery or WAL/SHM creation. No Git or ignore-rule checks.
- `validate` never creates/refreshes projection. Default applies mechanical corrections. `--no-correct` previews without writes. `--strict` also stays read-only and exits 1 for diagnostics or pending corrections.
- `validate` returns `collections`, `diagnostics`, `records`, `corrections`, `corrected`, `strict`. `corrected` marks correction mode, not a count: true even when no correction is needed. `corrections` contains `{path, fields}` entries; default reports applied values, dry-run reports planned values.
- Mechanical scope: replace missing/invalid UUIDv7 IDs; fill absent name, created_at, desc, tags and schema defaults; disambiguate duplicate IDs/names within a collection. Do not guess invalid author field values or repair malformed YAML.
- Duplicate preference: valid source `created_at` first; unknown dates last. Within a tied date cohort, use filesystem creation time if every member has one, otherwise byte-order path. Metadata names gain ` (2)`, ` (3)`, etc., avoiding occupied names. Files are never renamed. Heuristic does not prove author intent.
- `schema`, `query`, and `search` run correction before preparation and include `corrections` when applied. Diagnostics are exposed consistently. `sync` and positional rebuild do not correct source.
- Malformed/unsupported records are diagnosed, cached as raw source, and omitted from projected rows. Never repair them; continue reading valid records. Missing reference targets are diagnostics but do not block otherwise valid rows. Invalid config/schema or inaccessible collections prevent preparation.
- Successful correction is not failure. A later correction/publication/refresh failure reports paths already written; completed repairs remain in place.

## Source writes

- Markdown remains source of truth; `.attendant/` stays disposable. Do not place source collections inside `.attendant/`.
- Add-table accepts missing or empty collection directory, writes schema/template/usage seeds, then appends config. Validate paths, names, representable config grammar and collection capacity before creating anything. Publication failure before config commit removes owned seed files/empty directories, not pre-existing files.
- Create/update use non-empty JSON arrays of `{name, fields}` objects. Create permits omitted fields and uses defaults plus template body. Update requires non-empty fields. `name` selects `<name>.md`, not the possibly repaired metadata `name`.
- Only declared fields plus `desc`/`tags` are writable. Reject immutable/unknown fields, unsafe/device filenames, repeated targets, collisions, invalid types/dates/references and invalid prospective datasets before any source publication. Transport and whole-batch target checks happen before applying pending corrections.
- Create generates UUIDv7 `id`, name and UTC `created_at`. Optional template front matter is discarded; body copied byte-for-byte. After serialization, compare complete parsed mapping with intended values before staging.
- Updates and corrections splice bounded top-level value spans. Preserve unrelated bytes, comments, newline style, closing delimiter and exact body. Formatting/comments inside replaced values may change. Escape YAML-sensitive Unicode; parse edits again and check changed field values before staging. No whole-document emitter.
- Stage all writes in exclusive sibling temporary files; flush before publication. Compare existing source with captured text before replacing it. Replacement atomic per file, **not across an update/correction batch**. No concurrent-writer guarantee; comparison does not close every race.
- On update/correction failure, keep completed replacements and report `written` paths. On create I/O failure, remove newly created files where possible; already-published repairs to existing records remain. After successful source publication, projection refresh failure leaves source committed and reports `error.code: "projection-refresh"` plus `written`. Retry after resolving projection issue.

## Projection and read safety

`prepare.h` scans/hashes/validates bounded source, then reuses or fully rebuilds in process. No Node subprocess. Reuse checks layout/config/schema metadata, SQLite quick check, source path/collection/hash/size/mtime, decoded front matter, precision-sensitive scalar values and required table/cache counts. Compare large integers directly with SQLite INTEGER values, or exact decimal TEXT for string-first mixed enums: old float JSON can have identical digits yet a rounded projected value. Check floats stored in TEXT too; serialize numbers explicitly rather than use SQLite's lower-precision double-to-TEXT conversion. Changed interpretation invalidates stale projections even when source bytes/stat match.

- Detect additions, edits, deletion, rename, hidden state and config/schema changes, including equal-size/equal-mtime edits. Mtime-only changes fully rebuild, unlike `.mjs` metadata-only reconciliation. Usage text remains live without rebuild.
- Build temporary DB before replacing published DB. SQL/build/publication failure preserves old DB. Source corrections committed before failure are separate and reported.
- Sequential use only. WAL/SHM sidecars and rollback journals block replacement. Native never removes sidecars or recovers journals. Stop consumers; use SQLite recovery or remove disposable projection before retrying.
- After preparation, user SQL uses read-only/query-only connection and authorizer. Cannot mutate data, attach DBs, read private cache/metadata or load extensions. One statement; SQL comments unsupported.
- Sync returns `collections`, `records`, `diagnostics`, `state`. Query/search retain `columns`, `rows`, `returned`, `truncated`, with correction/diagnostic reports when relevant. Empty results keep columns; zero limit returns zero rows; quoted semicolons plus trailing terminator work.
- Success emits one JSON value. Failures emit `{ "error": { "code": "...", "message": "..." }, "failed": "path", "written": ["..."] }`; `failed` identifies failed source publication/staging when known, `written` omitted when empty. Explanation on stderr, exit 1. Strict validation/unhealthy doctor return their diagnostic result with exit 1. No TTY-dependent output or prompts.
- Search: complete matches first, then unseen trigram matches; score and byte-order collection/ID ties. Deterministic ties suffice; exact `.mjs` locale collation not required. Limited results may pick different tied hits. Tests allow tiny BM25 floating-point differences between compilers.
- Malformed SQLite TEXT becomes U+FFFD before JSON serialization. Oversized escaped text/BLOB rows truncate rather than corrupt JSON.

## Compatibility and limits

`projection.c` mirrors layout `2`: collection columns, metadata/cache, diagnostics, reference edges, Unicode and trigram FTS5. Schema hashes use literal `\\0` separators. Reuse needs configured directory spelling, absolute paths, SHA-256 and filesystem timestamps. Windows CNG supplies hashes and secure UUID randomness.

- Tested Windows x64, CJIT v1.2.0, Zig 0.17.0, SQLite 3.53.0, LibYAML 0.2.5, Node v24.16.0.
- 128 MiB caller-owned arena plus 12 MiB JSON workspace; parser/SQLite allocate independently. Upper bounds: 32 collections, 128 schema fields each, 256 front-matter keys, 1024 scalar items per array, 4096 records total, 4 MiB per source file. Combined arena limits may fail earlier. SQL/path/output capacities also fail explicitly.
- YAML root must be block mapping; values scalars or scalar arrays. Empty schema declares no fields; empty record is invalid. Reject aliases, anchors, explicit tags/documents, nested structures, duplicate keys, nonfinite/non-decimal numeric forms and embedded NUL. Preserve valid UTF-8, including literal NEL/LS/PS as scalar content; local LibYAML patch limits line breaks to CR/LF. No Unicode normalization. Invalid source UTF-8 fails without encoding repair. Body stays unchanged.
- Date fields require exactly `YYYY-MM-DD` and valid calendar days. Datetimes require exactly `YYYY-MM-DDTHH:mm:ss.sssZ`, valid calendar/time values and FILETIME years 1601–9999. Reject offsets, lowercase separators, omitted seconds, and other fractional widths. Never guess, truncate or normalize author timestamps; generated timestamps use this same format.
- Query/search: at most 10,000 returned rows, 10 MiB serialized rows, 4096 retained search hits. SQLite value limit 12 MiB; SQL 4 MiB; 1024 columns; expression depth 100; 10-million-VM-instruction budget (`sql-budget`). No wall-clock timeout guarantee.
- Decimal integer tokens parse exactly as `i64`/`u64`; decimal-point/exponent tokens select finite `f64`. Reject integer overflow, nonfinite floats and nonzero float underflow to zero. SQLite scalar columns/defaults/enums and SQL parameters reject integers above `INT64_MAX`; no rounding or text fallback. Full `u64` works in JSON-backed arrays. Quote oversized identifiers in string fields. SQL INTEGER results emit exact tokens, including values outside JavaScript's safe range; consumers must use lossless JSON parsing when needed.
- Float JSON uses tested round-trip formatting, not full ECMAScript serialization equivalence. Retain float syntax when a plain integer token would change value or select a rejected unsigned scalar. Source YAML always retains float syntax. Readback compares typed values, not serialized JSON text. Private cache/FTS keeps prior float spelling to preserve search terms; cache is not source reconstruction. Integer-index key ordering matches runner for FTS content. Differential tests compare shared-fixture JSON arrays semantically; focused tests separately check exact typed numbers.
- Configured path source spelling stays untouched. Resolve paths for local filesystem access; DB paths need not travel across machines or repositories. Windows NLS classifies search words; general Unicode-category/version, long-path and UNC behavior remain unverified. No locale collation or ICU dependency.
- Intentional `.mjs` differences: strict canonical timestamps instead of general date parsing; restricted flat YAML; exact typed integer handling without JavaScript safe-integer limits; deterministic byte-order search ties. Layout reuse remains tested on shared fixtures, not every input the JavaScript runner accepts.
- No incremental reconciliation, cross-file transaction, simultaneous CLI/GUI protocol, body editor, migration/import, Git invocation, viewer or skill launcher rollout. Diagnostic messages and unsupported-input behavior are not complete `.mjs` equivalence.

## Dependencies and review

Provenance/notices: `vendor/PROVENANCE.md` and per-library files. Preferred `json-build` writes JSON; `libpdjson5` reads strict JSON using caller-backed allocation. Copied `base.h` supplies arena and UTF-8 helpers. `build.h`/`test.h` remain available, not required for current build.

SQLite uses FTS5 and `SQLITE_DISABLE_INTRINSIC`; CJIT otherwise cannot link `__umulh`. Native declares stable Windows CNG functions because SDK `bcrypt.h` annotations fail with CJIT headers. Zig uses isolated local cache; never deletes/repairs user's cache.

Independent read-only reviews found and drove regressions for projection compatibility, read encoding/limits, journal protection, builder statement cleanup, required edge tables, YAML comment spans/Unicode encoding, config round-trip, collection-capacity preflight, YAML event bounds, integer-looking float serialization and old rounded projections whose cache JSON digits still match, and mixed-enum TEXT affinity/precision. Final bounded follow-up found no remaining actionable issue; review was source inspection only. Acceptance suites above passed with both compiler paths. Generated evidence defines tested coverage; no release/rollout approval implied.
