# Copied real-data workflow acceptance

2026-10-28. **66 workflow cases pass:** two captured projects × `.mjs`/CJIT/Zig × 11 cases. Uses same pristine capture as benchmarks: proto-framework 127 records/134 files; godot-proto 60 records/67 files. No production code change, launcher switch or new speed claim.

## Reproduce

Prebuild normal native binaries, then:

```sh
node exploration/attendant-native/tools/real-data-check.mjs \
  --manifest exploration/attendant-native/build/real-data/capture-C41Ig7/manifest.json
node --test exploration/attendant-native/tests/tools/real-data-check.test.mjs \
  exploration/attendant-native/tests/tools/benchmark.test.mjs \
  skills/attendant/test/attendant.test.js
```

Optional `--cjit <exe>` / `--zig <exe>` overrides. `--help` returns JSON usage. Argument errors exit 2; failed checks exit 1 with structured error and report directory. Every run creates unique `build/real-data-check/run-*/` with retained runner workspaces and `report.json`, including partial results on failure.

Requires Windows PowerShell for NLS audit. Node is test infrastructure only. Uses existing `tools/dataset-files.mjs` inventory, hash and containment checks. Reject changed capture before runner invocation. Copy only config and inventoried Markdown; reject escaping collection paths/links. Never invoke runners in live projects or pristine captures. Never use manifest `source`/`working` paths as mutation targets. Original capture inventory checked before/after each dataset.

## Eleven cases per runner/dataset

1. Strict validation, cold sync, schema/live usage, full task baseline query parity, collection counts and unchanged reuse. Read commands preserve source hashes.
2. First search plus all Unicode-containing words from capture: nonempty hits and matching collection/name result sets. Ignore ranking ties, absolute paths and compiler BM25 rounding here; focused read suite owns those checks.
3. Update existing captured task's description, numeric priority and tags. Verify bound SQL, indexed search and exact body preservation.
4. Reject mixed update batch containing missing target/invalid status without changing any source bytes.
5. Batch create/update, omitted schema defaults, exact template body, optional task reference arrays and projected reference edge. Strict validation afterward.
6. Create/update decision in second collection, including string-array fields.
7. External body append; automatic preparation/search finds new token.
8. External hidden/unhidden edits; sync removes/restores row visibility.
9. External filename rename and deletion; reconcile source paths, collection rows, search hits and reference edges. Strict validation afterward.
10. Remove owned disposable cache; search-first rebuild restores same logical data without changing source.
11. Indexed projection handoff between `.mjs` and native, unchanged reuse, fresh doctor and strict final validation. Selected logical columns (`name`, `status`, `priority`, `desc`, `tags`, `depends_on`) match for every task across runners. Generated IDs/times and workspace-dependent paths/hashes are not part of cross-run comparison. Verify final collection counts, exact edited body and every untouched original file hash.

Tests mutate only owned copies. Synthetic names/fields are test data, not real task/decision lifecycle changes or approvals. Both datasets' schemas and usage were read before selecting writes. Cases exercise existing schema fields only; no added collection or schema migration.

## Unicode applicability

Audit every non-ASCII character in **201 captured config/Markdown files** using same Windows `GetStringTypeW` predicate as native; compare with JavaScript Unicode Letter/Number categories. **No category mismatches in this capture. No supplementary-plane letters/numbers.**

Proto does contain `²`, e.g. `px²`, `ms²`, `count²`. Unlike `⁴` in earlier synthetic `²³⁴` example, Windows classifies `²` compatibly. Also search captured `Iñigo`, `s²` and `µs`; all result sets agree across runners. Godot has no non-ASCII Letter/Number words in capture. Other non-ASCII punctuation still participates in character audit.

Retain known synthetic Unicode gap; do not expand production Unicode implementation now. Revisit if affected characters enter actual data or user needs those searches. This is corpus-specific evidence on tested Windows/Unicode versions, not universal Unicode equivalence.

## Deferred work and rollout gate

- Note long-path/UNC/access-denied qualification as unverified. User expects low practical risk; do not expand this work now.
- Launcher/compiler discovery and skill integration remain **last**. User requires correctness evidence, representative speed advantage and useful additional capabilities first. These checks establish tested workflows, not every possible input or universal correctness.
- Native already exposes exact-number handling, visible corrections, structured failures and bounded lock waits; final feature-benefit acceptance remains separate. Do not invent new feature scope from rollout gate.
- Existing timing evidence remains in [SQLITE-ADOPTION.md](SQLITE-ADOPTION.md). This suite is correctness-only and does not replace benchmarks.

## Evidence

- Final run: `build/real-data-check/run-mYv34X/report.json`; all six runner/dataset combinations pass all 11 cases.
- Capture: `build/real-data/capture-C41Ig7/manifest.json`. Pristine inventories unchanged.
- Four workflow harness tests, ten benchmark tests and seven existing runner tests pass: **21 total**.
- No new native source changes. No power-loss qualification, exhaustive diagnostic-message equivalence or broader platform claim.
