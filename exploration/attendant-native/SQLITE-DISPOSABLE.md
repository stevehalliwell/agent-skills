# Disposable-cache and lazy-search experiments

Historical experiment stage. Current normal implementation uses persistent deferred FTS with first-search full rebuild, bounded mmap and reduced routine integrity scans: [SQLITE-ADOPTION.md](SQLITE-ADOPTION.md). Below records earlier invalidating-lazy prototype, not current enabled-mode lifecycle. Bare-build versus first-search figures below do not establish faster rebuilding; matched comparison in adoption report corrects that inference.

2026-10-28. User authorizes trading projection durability for speed and proposes lazy FTS because agents reportedly use SQL/ripgrep more often. Usage frequency is supplied assumption, not measured telemetry. Markdown remains authoritative. **Experimental binaries only; normal application defaults and launchers unchanged.**

## Findings

1. Lazy FTS has largest build benefit without weakening durability: fresh median 300.4 → 71.5 ms on proto, 202.1 → 67.4 ms on godot. Before any search, SQL query median 132.4 → 53.4 ms and 74.4 → 45.3 ms.
2. Per-preparation `PRAGMA quick_check` is expensive once FTS exists. Removing routine checks lowers warm SQL median 134.3 → 56.9 ms and 68.7 → 42.7 ms. Doctor still checks integrity. This variant does **not** add replacement post-build/recovery checks; that policy remains adoption work.
3. WAL/OFF improves write latency here. Private OFF/OFF/exclusive bulk build also helps. Neither claim establishes power-loss safety.
4. Lazy mode moves work to first search: normal-sync median 510.0/196.4 ms, versus eager control 170.9/81.7 ms. Lazy + WAL/OFF lowers that to 298.5/119.7 ms. Later unchanged searches reuse indexes.
5. Lazy invalidation after an already-indexed cache is not free. Separate never-searched edit test shows much larger benefit than edits starting from ready indexes.
6. Larger pages offer smaller read gains and larger files. Deferred eager FTS population and final FTS5 optimize are not clear winners here. Final optimize increases allocated file size; no subsequent VACUUM included, so this is not proof of greater live index size.

Recommendation for next candidate: lazy FTS, bounded mmap and integrity checks at build/repair/doctor rather than every lookup. Test combination; do not multiply isolated speedups. WAL/OFF needs explicit restart/corruption invalidation policy before adoption. Keep served-result correctness separate from disposable-cache durability.

## Implementation and isolation

`sqlite-experiment-source.mjs` generates exact-checked source transformations under ignored `build/sqlite-experiments/<profile>/`. Assertion fails if expected source fragment changes. `sqlite-experiment-build.sh` compiles separate Zig `-O3` executables. No experimental policy macros or generated changes added to normal C sources. Node is generation/test infrastructure only; experimental CLI remains native.

Profiles, all other settings held at current defaults:
- `control`: current WAL/NORMAL, 4096-byte pages, eager FTS and routine integrity checks. Disabled fault hooks shared with other variants.
- `wal-off`: WAL with synchronous OFF.
- `bulk-off`: private full builds use journal OFF, synchronous OFF, exclusive locking, 64 MiB cache, cache spilling OFF and memory temporary storage. After commit, select WAL, checkpoint/close, then publish. Later incremental writes use WAL/OFF. Bundled profile does not isolate each setting.
- `memory`: MEMORY rollback journal with synchronous OFF on writers. Mode is per connection; after close, reported persisted mode is DELETE, not WAL. Reader/writer overlap differs from WAL.
- `lazy`: WAL/NORMAL; no FTS objects until first search.
- `lazy-wal-off`: lazy plus WAL/OFF.
- `no-quick-check`: eager WAL/NORMAL; remove normal preparation integrity scans, retain doctor scans, source validation and compatibility/value checks.
- `page8`, `page16`, `page32`: set 8192/16384/32768-byte pages before new database enters WAL. Readback confirms sizes.
- `fts-deferred`: create eager FTS tables, but populate after all base rows are loaded, before build commit. Incremental behavior unchanged.
- `fts-optimize`: normal eager build plus FTS5 `optimize` command on both indexes before commit. This is not planner `PRAGMA optimize`.

### Lazy behavior

- Global readiness metadata distinguishes intentionally absent indexes from complete ones. All collections use same readiness state for this prototype.
- Sync/schema/SQL never populate FTS.
- First search opens writer transaction, verifies loaded source matches complete compatible projection under lock, creates/populates normal and trigram indexes, then commits readiness atomically with them.
- Projected source changes drop/invalidate both indexes transactionally. Next search repopulates current content. Mtime-only refresh leaves indexes ready.
- Existing WAL readers retain old complete snapshot during invalidation/repopulation.
- Failed or killed population cannot publish ready state with partial indexes in tested cases.
- Once populated, SQLite file need not shrink when indexes are dropped; freed pages can remain allocated. Pre-search sizes below describe never-searched cache, not guaranteed post-invalidation disk reclamation.

## Method

Two captured datasets, six core CLI cases plus SQL-before-search, first search and search after edit. Twelve experimental native profiles plus Node control, serial rotating order. **13 measured rounds + 2 warmups**, balancing all 13 measured ordering positions. Fresh process each command; warm filesystem. Clock includes startup/output capture/exit. Excludes compilation, fixture reset/edit, correctness checks and explicit setup sync for search-after-edit.

Cold baseline saved after initial sync/schema/query **before oracle search**. Ready baseline saved after search. SQL-before-search and first-search restore cold baseline; ordinary unchanged cases and body-edit case restore ready baseline. Search-after-edit performs untimed sync, then verifies updated state, cache mtime/size/hash, projected hash and lazy index absence **before** timed search. Timed query searches edit marker. No source-update work hidden in claimed search-only timing.

Main run: **3042 measured + 468 warmups**, all 3510 valid, exit 0/no stderr. Source/pristine hashes unchanged except deliberate owned edits, restored afterward. No live project preparation.

Cells below: median milliseconds. Full nine-case distributions, p90/min/max/means and samples remain in raw report. Shared-machine noise and write outliers remain; no trimming or confidence-interval claim. Compare alternatives within same scenario/run, not absolute values across reports.

### proto-framework — 127 records

| Profile | Fresh sync | SQL before search | Warm SQL | Edit with ready baseline | First search |
| --- | ---: | ---: | ---: | ---: | ---: |
| Node | 645.8 | 270.1 | 273.1 | 252.5 | 310.4 |
| control | 300.4 | 132.4 | 134.3 | 236.6 | 170.9 |
| wal-off | 257.7 | 130.2 | 129.9 | 213.8 | 170.0 |
| bulk-off | 207.3 | 129.4 | 133.3 | 212.4 | 169.5 |
| memory | 212.4 | 124.2 | 127.1 | 203.6 | 165.0 |
| lazy | 71.5 | 53.4 | 127.7 | 239.3 | 510.0 |
| lazy-wal-off | 56.7 | 53.0 | 127.6 | 210.4 | 298.5 |
| no-quick-check | 318.4 | 55.5 | 56.9 | 92.9 | 95.8 |
| page8 | 286.7 | 123.6 | 125.6 | 221.3 | 165.4 |
| page16 | 272.1 | 120.4 | 119.8 | 219.8 | 162.1 |
| page32 | 274.6 | 119.1 | 114.5 | 213.8 | 156.2 |
| fts-deferred | 307.4 | 132.9 | 134.9 | 259.7 | 173.4 |
| fts-optimize | 363.2 | 128.3 | 131.4 | 245.1 | 165.5 |

### godot-proto — 60 records

| Profile | Fresh sync | SQL before search | Warm SQL | Edit with ready baseline | First search |
| --- | ---: | ---: | ---: | ---: | ---: |
| Node | 390.6 | 164.8 | 154.3 | 180.0 | 174.1 |
| control | 202.1 | 74.4 | 68.7 | 137.1 | 81.7 |
| wal-off | 113.4 | 76.0 | 67.9 | 98.9 | 83.7 |
| bulk-off | 85.2 | 74.3 | 67.7 | 98.6 | 82.7 |
| memory | 83.3 | 67.9 | 62.1 | 91.6 | 77.3 |
| lazy | 67.4 | 45.3 | 67.7 | 126.0 | 196.4 |
| lazy-wal-off | 49.2 | 45.4 | 67.5 | 98.6 | 119.7 |
| no-quick-check | 200.2 | 47.0 | 42.7 | 81.3 | 57.7 |
| page8 | 184.9 | 71.9 | 65.9 | 148.4 | 81.7 |
| page16 | 194.5 | 70.7 | 65.0 | 158.8 | 81.9 |
| page32 | 210.0 | 69.6 | 64.6 | 171.0 | 79.6 |
| fts-deferred | 190.7 | 73.1 | 67.7 | 126.5 | 83.7 |
| fts-optimize | 257.9 | 73.7 | 67.4 | 148.7 | 82.7 |

Warm means search previously populated; lazy therefore does not avoid scans of existing FTS pages. No-quick-check fresh build path is essentially unchanged: small fresh-build differences should not be assigned causal meaning.

Never-searched main DB sizes:
- Proto: control 9,973,760 bytes; lazy 1,871,872 bytes. After search both 9,973,760 bytes.
- Godot: control 3,469,312; lazy 643,072 bytes. Lazy after search 3,461,120 bytes; small allocation differences are not logical parity differences.
- 32 KiB pages: proto 11,468,800; godot 4,390,912 bytes.
- FTS5 final optimize without VACUUM: proto 13,111,296; godot 4,681,728 allocated bytes.

## Follow-up: never-searched edits

Same profiles/rotation, separate run restoring pre-search baseline. **676 measured + 104 warmups**, all valid. All edits report updated; unchanged sync reused. Post-timing search verifies content, outside measured interval. Cells median milliseconds; do not directly compare absolute edit values with main run.

| Dataset / case | Node | control | lazy | lazy-wal-off |
| --- | ---: | ---: | ---: | ---: |
| Proto unchanged sync | 165.4 | 118.5 | 43.9 | 44.1 |
| Proto body edit + sync | 353.5 | 357.7 | 89.5 | 57.3 |
| Godot unchanged sync | 119.2 | 69.0 | 39.1 | 39.6 |
| Godot body edit + sync | 211.2 | 168.1 | 64.1 | 43.7 |

This is closer to supplied SQL/ripgrep-dominant workflow. Main ready-baseline edit test remains relevant after user has searched once.

## Checks and limitations

- All 12 profiles pass source-contract suites in relative and absolute config modes: exact numbers/dates, source preservation and typed round trips.
- Both lazy variants additionally pass 19 CLI cases + 18 rejected batches per config mode, and 8 focused lifecycle cases: no-index SQL/doctor, failed population rollback, first population, invalidation, old-reader snapshot, reuse and mtime-only preservation.
- All 12 pass injected mid-delta SQL failure: old row/cache/FTS restored, authored source edit preserved.
- Control, WAL/OFF, private-bulk-OFF and MEMORY/OFF pass selected killed incremental-write and unpublished-build tests. Test hook calls `sqlite3_db_cacheflush`, signals parent, pauses before commit; parent terminates only owned process. Subsequent native sync/search returns complete current data. Killed private builder does not change published main DB.
- Both lazy variants pass killed first-population test: readiness remains 0, no partial search tables served, next search rebuilds.
- MEMORY/OFF held reader blocks writer; command returns bounded structured busy error and old reader remains unchanged. This loses WAL coexistence; not recommended default.
- Ten benchmark unit tests and seven existing runner tests pass. Independent source review found missing untimed-sync preflight check; fixed with regression before full measurement. Follow-up found no actionable issue within inspected lazy/timing-separation scope. Fault hooks not independently reviewed.
- These are selected process-death tests, **not** power-loss, OS-crash, storage-reordering or silent-corruption qualification. OFF sync may corrupt cache after those failures. Automatic trustworthy invalidation after OS restart/corruption remains adoption requirement; ability to manually rebuild does not prove detection of every invalid cache.
- No combination of lazy + reduced integrity checks + mmap measured yet. No long-lived GUI session, scaled corpus, cold disk, RSS measurement, FTS automerge/crisismerge tuning, published journal-OFF mutation policy or cross-platform qualification. Publication still refuses active consumers on full replacement.

## Reproduce and evidence

```sh
for profile in control wal-off bulk-off memory lazy lazy-wal-off no-quick-check \
  page8 page16 page32 fts-deferred fts-optimize; do
  bash exploration/attendant-native/sqlite-experiment-build.sh "$profile"
done
node exploration/attendant-native/sqlite-experiment-check.mjs \
  --manifest exploration/attendant-native/build/real-data/capture-C41Ig7/manifest.json \
  --runs 13 --warmup 2
# Same flags plus --unsearched-only for never-searched sync/edit cases.
node exploration/attendant-native/lazy-search-check.mjs \
  exploration/attendant-native/build/real-data/capture-C41Ig7/manifest.json
node exploration/attendant-native/sqlite-experiment-faults.mjs \
  exploration/attendant-native/build/real-data/capture-C41Ig7/manifest.json
```

- Main: `build/bench/run-4KfwN5/report.json`, `samples.jsonl`, `sqlite-experiments.json`; entry `build/sqlite-experiments.json`.
- Never-searched: `build/bench/run-ZbT1h4/`; entry `build/sqlite-experiments-unsearched.json`.
- Faults: `build/sqlite-experiment-faults.json`; lazy checks `build/lazy-search-results.jsonl`; source/CLI checks `build/sqlite-experiments-source-results.jsonl` and `build/sqlite-experiments-cli-results.jsonl`.
- Reports retain executable/generated-source hashes, environment, states, distributions and owned workspaces. Local evidence ignored; conclusions retained here.
- Timing driver rejects experimental pause environment variables. Hooks activate only with `ATTENDANT_EXPERIMENT_PHASE` and `ATTENDANT_EXPERIMENT_SIGNAL`, used by fault tests outside timings.

SQLite semantics: [synchronous](https://www.sqlite.org/pragma.html#pragma_synchronous), [journal modes](https://www.sqlite.org/pragma.html#pragma_journal_mode), [cache spilling](https://www.sqlite.org/pragma.html#pragma_cache_spill), [FTS5 optimize](https://www.sqlite.org/fts5.html#the_optimize_command). Earlier safe-profile comparison: [SQLITE-TUNING.md](SQLITE-TUNING.md). Normal native/Node/CJIT baselines: [BENCHMARK.md](BENCHMARK.md).
