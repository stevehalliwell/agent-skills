# Deferred FTS and read-path adoption

2026-10-28. Lazy FTS, reduced routine integrity scans and bounded mmap checkpoint committed as `daddf13`. User then approves **in-place FTS population on first search**, retaining persistent enabled mode. Follow-up implemented and validated in normal CJIT/Zig builds; user approved commit. No launcher rollout. WAL/NORMAL remains default; WAL/OFF combination measured separately, not adopted.

## Current behavior

- Cold `sync`, `schema`, SQL `query`, `create`, `update`: collection rows/cache/diagnostics/edges only. No normal-token/trigram FTS objects.
- First `search` against compatible cold cache: reconcile pending source changes, acquire `BEGIN IMMEDIATE`, recheck current source snapshot, create/populate both FTS indexes in place. Validate before commit; indexes, `fts_enabled=1` and layout `2` become visible together. Failure rolls back all population/mode changes. Collection rows/cache/diagnostics/edges retain generation.
- Missing/incompatible cache still requires full private rebuild. Search as first command directly builds indexed cache, without intermediate cold build.
- Once enabled, ordinary deltas maintain both indexes transactionally. Never drop them merely because source changed. Full contract rebuild preserves known enabled state. Removing/corrupting disposable cache can reset inferred state.
- Healthy first-search upgrade coexists with WAL readers: held reader retains complete cold schema/mode; new readers see complete indexed state after commit. Competing writers have bounded busy failure. Actual full replacement still refuses active consumers.
- Metadata `fts_enabled` is exact TEXT `0`/`1`. Cold layout `3` requires flag `0` and absent indexes. Indexed layout `2` supports explicit `1` or missing flag for legacy-produced DBs.
- Distinct cold layout makes unchanged `.mjs` rebuild it safely instead of trying missing FTS. Indexed layout stays interoperable. Native can consume legacy indexed DB without modifying it.
- `schema.fts` describes supported table/column contract even before physical creation. Successful schema JSON shape remains unchanged.
- Shared SQLite open helper requests **64 MiB mmap** for preparation, writers and query-only consumers. Default cache/temp settings unchanged.
- No per-preparation `quick_check`. Full private builds run planner optimize, full quick check and snapshot validation before publication. Doctor retains immutable/non-writing integrity inspection. Source hashing/parsing/validation, equal-stat detection and exact value checks remain.

## Important timing correction

Earlier bare-build timings versus first-search timings did **not** establish that full rebuild is faster: first search also includes preparation and actual search. Matched profiles retain same mmap, integrity, persistent-mode and synchronous settings.

**Matched measurements do not favor full rebuild.** In-place population is now normal path; full replacement remains comparison variant. Private full build validates after commit before publication; in-place upgrade validates before commit to preserve atomic rollback on validation failure.

## Current in-place first-search result

Production follow-up versus full replacement, median / p90 milliseconds:

| Dataset | In-place, NORMAL | Full rebuild, NORMAL | In-place, OFF | Full rebuild, OFF |
| --- | ---: | ---: | ---: | ---: |
| Proto — 127 records | 322.9 / 333.6 | 345.0 / 368.1 | 276.0 / 288.2 | 295.7 / 303.4 |
| Godot — 60 records | 121.7 / 124.6 | 134.7 / 140.4 | 103.9 / 106.7 | 115.6 / 120.2 |

Six runners × two datasets × 18 measured + 2 warmups: **216 measured + 24 warmups**, all valid. Normal native, CJIT, Node, WAL/OFF and full/full-OFF variants share current harness. Current default in-place reduces median first-search latency about 6–10% versus full in this run. Checks occur at different transaction boundaries by design; not an isolated SQL insertion microbenchmark. Source capture/pristine preservation and result parity pass. No new full eleven-case matrix; historical non-first-search numbers below predate this follow-up.

Evidence: `build/bench/run-kUJbeU/`, entry `build/sqlite-population-first-search.json`. Current driver outputs `sqlite-population*.json`, preserving earlier entries. Full comparison profiles are now `full`/`full-wal-off`; former `late` generators remain reproducible from checkpoint `daddf13`.

## Historical matched first-search comparison

Checkpoint-era final first-search run, median / p90 milliseconds:

| Dataset | Full rebuild, NORMAL | Late population, NORMAL | Full rebuild, OFF | Late population, OFF |
| --- | ---: | ---: | ---: | ---: |
| Proto — 127 records | 330.3 / 340.1 | 309.9 / 316.0 | 282.4 / 286.7 | 265.5 / 269.0 |
| Godot — 60 records | 132.2 / 143.2 | 121.2 / 128.5 | 114.1 / 116.8 | 104.0 / 107.5 |

Both strategies perform post-commit optimize/integrity/snapshot validation, including committed FTS segments. Earlier main-matrix late variant checked integrity before commit; focused run supersedes that first-search comparison. Other normal-build results below unchanged. No power-loss qualification for OFF.

## Historical checkpoint native versus Node

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
- 13 search-mode cases. Sentinel table proves first search retains collection/cache generation. Injected metadata failure after all FTS rows inserted rolls back indexes and enabled flag. Injected collection deletion during upgrade forces validation failure and proves pre-commit rollback restores rows/cold mode. Held WAL reader retains cold snapshot while upgrade succeeds; competing writer fails bounded. Also covers pending edit plus first search, cold SQL/schema/doctor/create/update, failed private contract rebuild, enabled contract rebuild, cold legacy rebuild and search-first startup.
- 10 benchmark unit tests + 7 existing runner tests pass. Search-after-edit preflight now checks persistent index state before timing, so hidden rebuilding cannot masquerade as ready-search latency.

Independent source review found no actionable issue in inspected checkpoint adoption paths. Review covered mode/version compatibility, connections, full validation/publication, incremental maintenance, schema descriptors and test transitions. Source-only review; benchmarks not independently reproduced. In-place follow-up has fresh full correctness suites and focused timing, not a fresh independent review.

Current-mode explicit SQL-failure and active-reader tests establish tested failure behavior. Earlier process-kill experiments remain historical evidence for their own binaries, not fresh crash/power-loss qualification of this adoption. mmap I/O fault injection, long-lived GUI coordination and wider Unicode category parity remain unverified. WAL/OFF automatic invalidation after OS crash/restart/corruption remains separate work.

## Reproduce and evidence

```sh
bash exploration/attendant-native/build.sh cjit
bash exploration/attendant-native/build.sh zig
node exploration/attendant-native/search-mode-check.mjs \
  exploration/attendant-native/build/projection-cjit.exe \
  exploration/attendant-native/build/projection-zig.exe
for profile in wal-off full full-wal-off; do
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
- `build/review/search-mode-results.json` plus `build/population-{source,read,prepare,incremental,mode,cli}-results.jsonl` retain current correctness output; checkpoint results remain in `build/adoption-*.jsonl`.
- Historical pre-adoption experiment C inputs frozen under `fixtures/sqlite-experiment-base/`; older generators/adapters use that baseline rather than silently measuring new defaults. Other source-contract/vendor headers remain shared; original binary hashes define exact historical measurement.

Prior experiments: [SQLITE-DISPOSABLE.md](SQLITE-DISPOSABLE.md), [SQLITE-TUNING.md](SQLITE-TUNING.md). Historical native/CJIT/Node baselines: [BENCHMARK.md](BENCHMARK.md).
