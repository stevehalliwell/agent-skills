# Native projection review slice

C99 reads frozen sample Attendant files and generates a SQLite projection that the existing `.mjs` CLI consumes unchanged. This is a read-only-source exploration, not a replacement CLI or a general YAML implementation.

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

Node.js >=24.10.0 is needed only for the existing CLI and comparison harness. The generated native executable reads YAML and writes SQLite without Node. Preferred `json-build` writes cached metadata and result JSON; there is no JSON input in this slice, so `libpdjson5` is not needed yet.

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

Writes only `.attendant/` under the supplied project. Existing source files are read-only. Builder creates a temporary database and publishes it after completion; consumers must not run concurrently. Existing WAL/SHM sidecars cause refusal rather than replacement. This is not a concurrent CLI/GUI publication protocol.

## Compatibility contract

`projection.c` mirrors current runner layout version `2`: collection columns, metadata/cache, diagnostics table, reference edges, Unicode FTS5 and trigram FTS5. Schema hashes currently use literal `\\0` separators, not NUL bytes. Cache reuse needs configured directory spelling, absolute source paths, SHA-256 and filesystem timestamps. Windows CNG supplies SHA-256. Native conversion matches Node's cache millisecond arithmetic and rounded Stats Date values on tested fixtures.

Source fixture must already be valid and normalized. Native slice rejects unsupported or malformed input rather than performing source repair. Empty diagnostic tables represent valid fixtures, not implemented general diagnostic parity. `check.mjs` verifies reference diagnostics are empty before comparing output.

Limits and deferred behavior:
- Windows only. Tested CJIT v1.2.0 and Zig 0.17.0, SQLite 3.53.0, LibYAML pinned source 0.2.5, Node v24.16.0.
- 128 MiB caller-owned arena plus 4 MiB JSON workspace; parser/SQLite allocate independently. Up to 32 collections, 128 schema fields per collection, 256 front-matter keys, 1024 scalar items per array, and 4096 records total. Each source file is at most 4 MiB. Generated SQL/path capacities also fail explicitly. These are exploration bounds, not final product limits.
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
- Existing `node --test skills/attendant/test/attendant.test.js`: 7 passed.
- Independent read-only review found config spelling/alias/BOM and integer-key ordering gaps; fixes gained regression coverage. Follow-up review found no remaining actionable issue within documented scope. Parent reran all four scenarios after the final BOM fix.

Stop at user review of this slice before broadening parity or implementing source writes.
