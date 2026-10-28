# Deferred FTS and read-path adoption

2026-10-28. User approved lazy FTS, reduced routine integrity scans and bounded mmap. User specifies **full indexed rebuild on first search**, then persistent enabled mode. Implemented in normal CJIT/Zig builds; no launcher rollout or commit. WAL/NORMAL remains default; WAL/OFF combination measured separately, not adopted.

## Current behavior

- Cold `sync`, `schema`, SQL `query`, `create`, `update`: collection rows/cache/diagnostics/edges only. No normal-token/trigram FTS objects.
- First `search`: reuse already loaded/validated source to build complete new indexed DB privately. Validate, checkpoint/close, then replace published cache. No initial unindexed build if search is first command.
- Once enabled, ordinary deltas maintain both indexes transactionally. Never drop them merely because source changed. Full contract rebuild preserves known enabled state. Removing/corrupting disposable cache can reset inferred state.
- First-search full replacement retains existing active-consumer refusal. Failed replacement preserves old cache and source; stop reader and retry. Later incremental updates retain WAL reader coexistence.
- Metadata `fts_enabled` is exact TEXT `0`/`1`. Cold layout `3` requires flag `0` and absent indexes. Indexed layout `2` supports explicit `1` or missing flag for legacy-produced DBs.
- Distinct cold layout makes unchanged `.mjs` rebuild it safely instead of trying missing FTS. Indexed layout stays interoperable. Native can consume legacy indexed DB without modifying it.
- `schema.fts` describes supported table/column contract even before physical creation. Successful schema JSON shape remains unchanged.
- Shared SQLite open helper requests **64 MiB mmap** for preparation, writers and query-only consumers. Default cache/temp settings unchanged.
- No per-preparation `quick_check`. Full private builds run planner optimize, full quick check and snapshot validation before publication. Doctor retains immutable/non-writing integrity inspection. Source hashing/parsing/validation, equal-stat detection and exact value checks remain.

## Important timing correction

Earlier bare-build timings versus first-search timings did **not** establish that full rebuild is faster: first search also includes preparation and actual search. Matched profiles retain same mmap, integrity, persistent-mode and synchronous settings.

**Matched measurements do not favor full rebuild.** Requested full rebuild remains implemented: private validation/publication isolates replacement failure, but active readers can block it. Late population is modestly faster and can coexist with WAL readers; it remains comparison variant, not normal path.

Focused final first-search run, median / p90 milliseconds:

| Dataset | Full rebuild, NORMAL | Late population, NORMAL | Full rebuild, OFF | Late population, OFF |
| --- | ---: | ---: | ---: | ---: |
| Proto — 127 records | 330.3 / 340.1 | 309.9 / 316.0 | 282.4 / 286.7 | 265.5 / 269.0 |
| Godot — 60 records | 132.2 / 143.2 | 121.2 / 128.5 | 114.1 / 116.8 | 104.0 / 107.5 |

Both strategies perform post-commit optimize/integrity/snapshot validation, including committed FTS segments. Earlier main-matrix late variant checked integrity before commit; focused run supersedes that first-search comparison. Other normal-build results below unchanged. No power-loss qualification for OFF.

## Current native versus Node

Same warm-filesystem, startup-inclusive harness. Compilation/reset/edit/check work excluded. All work in owned copied captures, never live projects. Six runners: Node, normal CJIT, normal Zig `-O3`, and Zig variants `wal-off`, `late`, `late-wal-off`. Main matrix **18 measured + 2 warmups** × 11 cases × 2 datasets × 6 runners: **2376 measured + 264 warmups**, all valid, exit 0/no stderr. Rotating order balances six measured positions.

Cells median milliseconds. Shared machine; compare within run, not absolute times against earlier reports. Mutation/first-build outliers remain, no trimming or confidence-interval claim.

### proto-framework

| Operation | `.mjs` | CJIT | Zig default NORMAL | Zig experimental OFF |
| --- | ---: | ---: | ---: | ---: |
| Fresh unindexed sync | 447.5 | 60.4 | 55.3 | 43.0 |
| Unchanged sync, indexed | 123.0 | 38.7 | 33.9 | 34.6 |
| SQL query, indexed | 199.6 | 44.4 | 38.9 | 38.6 |
| Search, indexed | 235.5 | 94.6 | 69.4 | 70.2 |
| Body edit + sync, indexed | 189.0 | 75.6 | 67.5 | 42.6 |
| Mtime-only + sync, indexed | 151.2 | 70.0 | 62.8 | 39.5 |
| SQL query before search | 205.0 | 42.5 | 37.1 | 37.3 |
| Search after untimed edit sync | 207.2 | 40.6 | 34.2 | 34.0 |
| Sync before search | 126.4 | 37.3 | 33.3 | 33.3 |
| Body edit + sync before search | 296.0 | 67.9 | 60.5 | 35.8 |

### godot-proto

| Operation | `.mjs` | CJIT | Zig default NORMAL | Zig experimental OFF |
| --- | ---: | ---: | ---: | ---: |
| Fresh unindexed sync | 297.4 | 69.1 | 61.0 | 40.5 |
| Unchanged sync, indexed | 82.6 | 30.4 | 28.1 | 28.1 |
| SQL query, indexed | 117.9 | 35.6 | 32.7 | 32.9 |
| Search, indexed | 125.9 | 46.9 | 40.2 | 40.6 |
| Body edit + sync, indexed | 142.0 | 71.5 | 80.1 | 33.5 |
| Mtime-only + sync, indexed | 124.6 | 93.9 | 59.7 | 30.6 |
| SQL query before search | 119.4 | 34.1 | 31.9 | 32.1 |
| Search after untimed edit sync | 120.9 | 26.0 | 23.1 | 23.4 |
| Sync before search | 83.8 | 29.0 | 27.5 | 27.7 |
| Body edit + sync before search | 171.5 | 52.9 | 43.7 | 29.3 |

Small fresh-build and mutation results had larger outliers; e.g. godot Zig fresh p90 176.1 ms versus median 61.0. Do not infer CJIT wins mutations generally from individual cells.

Focus first-search rerun: **216 measured + 24 warmups**, same six runners, all valid. First-search strategy table above uses this run. Node already paid eager indexing during initial sync, so Node first-search latency alone is not total indexing cost.

## Checks and review

Normal builds pass both compilers × relative/absolute configs:
- 10 source-contract groups; exact numbers/dates and source preservation.
- 23 read comparisons + 27 rejected SQL cases; indexed logical projection/schema/search parity and bidirectional legacy consumption.
- 34 lifecycle cases, including corrupt/incompatible cache, SQL construction failure, bounded busy writers and hot-journal refusal.
- 19 CLI cases + 18 rejected batches. Indexed interoperability explicitly enabled before asserting legacy reuse; cold format handoff separately checked.
- 11 incremental/WAL regressions; changed-only row/FTS writes, mtime-only no index churn, rollback, ID swaps, global validation, held readers and shared create/update path.
- 9 search-mode cases, including private-construction failure preserving old cache and cleaning owned temporary files. Sentinel table proves first search replaces entire DB, while later edits retain existing generation. Also tests cold SQL/schema/doctor/create/update, reader-blocked upgrade, enabled contract rebuild, cold legacy rebuild and search-first startup.
- 10 benchmark unit tests + 7 existing runner tests pass. Search-after-edit preflight now checks persistent index state before timing, so hidden rebuilding cannot masquerade as ready-search latency.

Independent source review found no actionable issue in inspected adoption paths. Review covered mode/version compatibility, connections, full validation/publication, incremental maintenance, schema descriptors and test transitions. Source-only review; benchmarks not independently reproduced.

Current-mode explicit SQL-failure and active-reader tests establish tested failure behavior. Earlier process-kill experiments remain historical evidence for their own binaries, not fresh crash/power-loss qualification of this adoption. mmap I/O fault injection, long-lived GUI coordination and wider Unicode category parity remain unverified. WAL/OFF automatic invalidation after OS crash/restart/corruption remains separate work.

## Reproduce and evidence

```sh
bash exploration/attendant-native/build.sh cjit
bash exploration/attendant-native/build.sh zig
node exploration/attendant-native/search-mode-check.mjs \
  exploration/attendant-native/build/projection-cjit.exe \
  exploration/attendant-native/build/projection-zig.exe
for profile in wal-off late late-wal-off; do
  bash exploration/attendant-native/sqlite-adoption-build.sh "$profile"
done
node exploration/attendant-native/sqlite-adoption-check.mjs \
  --manifest exploration/attendant-native/build/real-data/capture-C41Ig7/manifest.json \
  --runs 18 --warmup 2
# Add --first-search-only for final matched first-search protocol.
```

- Main evidence: `build/bench/run-fzhcMF/`, entry `build/sqlite-adoption.json`.
- Final first-search evidence: `build/bench/run-hBsV60/`, entry `build/sqlite-adoption-first-search.json`.
- Per-run `report.json`, every sample, `sqlite-adoption.json`, binary/source hashes and owned workspaces retained locally/ignored.
- `build/review/search-mode-results.json` plus `build/adoption-{source,read,prepare,incremental,mode,cli}-results.jsonl` retain correctness output.
- Historical pre-adoption experiment C inputs frozen under `fixtures/sqlite-experiment-base/`; older generators/adapters use that baseline rather than silently measuring new defaults. Other source-contract/vendor headers remain shared; original binary hashes define exact historical measurement.

Prior experiments: [SQLITE-DISPOSABLE.md](SQLITE-DISPOSABLE.md), [SQLITE-TUNING.md](SQLITE-TUNING.md). Historical native/CJIT/Node baselines: [BENCHMARK.md](BENCHMARK.md).
