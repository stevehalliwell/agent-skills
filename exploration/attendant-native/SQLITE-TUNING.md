# SQLite tuning experiment

2026-10-28. Safe connection-setting experiments on copies of both captured datasets. **Normal native defaults unchanged by experiment:** WAL/NORMAL writers, 2000 ms busy timeout, default checkpointing, read-only/query-only consumers. Incremental projection implementation is measured separately in [BENCHMARK.md](BENCHMARK.md).

## Result

Historical safe-setting comparison. Later authorized disposable-cache/lazy-search experiments: [SQLITE-DISPOSABLE.md](SQLITE-DISPOSABLE.md). Those show larger gains from avoiding unused FTS work and routine full integrity scans. Subsequent normal-build adoption: [SQLITE-ADOPTION.md](SQLITE-ADOPTION.md).

Recommend **64 MiB mmap as next candidate**, not entire supplied recipe. Interleaved results show roughly 7–13% lower median unchanged CLI read times versus experimental control. Larger caches and temporary-memory storage add little to those reads. No default tuning adoption in this slice.

- Giant cache/mmap caps do not materially beat modest combined profile on these 3–10 MB DBs.
- `VACUUM; ANALYZE` after fresh build costs extra time with negligible space/read benefit here.
- Mutation timings have substantial outliers. Do not rank small mutation differences or claim globally optimal settings.
- Results apply to startup-inclusive warm-filesystem commands, not massive ingest, cold disk, persistent GUI sessions or scaled datasets.

## Recipe assessment

- `journal_mode=OFF` disables SQLite atomic commit and rollback. Unsuitable for published incremental DB: mid-update failure must preserve prior projection. Could be a separate unpublished-temporary-build experiment with verification, final durable flush and safe publication; **not implemented or timed here**.
- `synchronous=OFF` and `locking_mode=EXCLUSIVE` not tested/adopted. OFF changes failure safety; exclusive mode conflicts with tested WAL reader/writer coexistence. No claim these settings are required for best safe performance.
- Read-only open is appropriate; synchronous setting does not accelerate nonexistent writes. Cannot switch persisted journal mode from a genuinely read-only connection. Journal mode changes locking/journaling, not a special read-optimized query planner.
- Cache/mmap/temp settings are per connection. Apply to freshness probe and actual query, not just writer. Cache caps are not up-front RAM allocations. mmap is a virtual mapping limit, not resident RAM allocation.
- `VACUUM` rebuilds/reclaims pages; it does not compress arbitrary data or defragment the filesystem. Keep its cost in ingest time. Planner `PRAGMA optimize` is already used after normal full builds; full `ANALYZE` is a distinct experiment.
- Essential PK/unique constraints and FTS indexes remain enabled. Delayed index construction not tested; this dataset is small, not a massive one-shot import.

## Profiles

All experimental binaries use same current native code, SQLite `3.53.0`, Zig `0.17.0`, `-O3`, WAL/NORMAL and normal locking. Adapter only intercepts application SQLite opens; normal production build never includes adapter. Control has same adapter overhead without tuning.

| Profile | Cache | mmap requested | Temporary storage | Full-build cleanup |
| --- | --- | --- | --- | --- |
| control | default `-2000` KiB | 0 | default | normal `PRAGMA optimize` |
| cache | `-16384` KiB = 16 MiB | 0 | default | normal |
| mmap | default | 64 MiB | default | normal |
| combined | 16 MiB | 64 MiB | MEMORY | normal |
| large | `-500000` KiB = 512,000,000 bytes | 4,294,967,296 bytes | MEMORY | normal |
| vacuum | 16 MiB | 64 MiB | MEMORY | extra `VACUUM; ANALYZE`, then checkpoint |

Readback confirmed requested settings, except large mmap: compiled SQLite cap limits effective value to **2,147,418,112 bytes**, not requested 4 GiB. Actual files are much smaller. Vacuum cleanup occurs only on committed private full-build DB before publication, never on published incremental DB.

## Method and results

Initial six separate 5-sample pilots had large background drift; retained as screening evidence, not basis for ranking. Follow-up interleaves six profiles plus Node control in one sequence: **21 measured rounds + 3 warmups per dataset/scenario/runner**. Rotating order balances seven positions across measured rounds. Same fixture-reset, result/hash/FTS/cache checks and external wall clock as [baseline harness](README.md#cli-performance-baseline). Compilation, resets, edits, checks and reporting probes excluded. Every command starts fresh process.

Cells: median milliseconds. Full p90/min/max/means and individual samples in raw report.

### proto-framework — 127 records, approximately 10 MB DB

| Scenario | Node | control | cache | mmap | combined | large | vacuum |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Fresh sync | 462.9 | 239.1 | 220.9 | 233.7 | 220.5 | 223.1 | 299.9 |
| Unchanged sync | 127.5 | 95.2 | 91.2 | 82.9 | 82.6 | 82.5 | 83.2 |
| Unchanged query | 207.8 | 100.8 | 97.3 | 87.8 | 88.1 | 88.0 | 87.6 |
| Unchanged search | 235.7 | 130.9 | 127.4 | 117.9 | 117.6 | 117.7 | 117.4 |
| Body edit, then sync | 188.9 | 184.2 | 176.6 | 159.3 | 162.8 | 163.7 | 162.2 |
| Mtime-only, then sync | 227.5 | 257.3 | 271.2 | 251.7 | 245.8 | 253.6 | 262.8 |

### godot-proto — 60 records, approximately 3.5 MB DB

| Scenario | Node | control | cache | mmap | combined | large | vacuum |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Fresh sync | 308.2 | 165.0 | 160.0 | 167.7 | 166.5 | 168.9 | 249.1 |
| Unchanged sync | 81.2 | 48.9 | 47.9 | 45.1 | 45.5 | 45.2 | 45.1 |
| Unchanged query | 117.8 | 54.5 | 53.5 | 49.8 | 50.7 | 50.2 | 49.9 |
| Unchanged search | 127.5 | 62.9 | 61.4 | 58.7 | 58.3 | 58.0 | 57.9 |
| Body edit, then sync | 166.2 | 114.3 | 134.0 | 100.9 | 128.2 | 111.3 | 109.3 |
| Mtime-only, then sync | 145.0 | 124.2 | 102.3 | 110.6 | 105.2 | 104.7 | 126.6 |

Unchanged reads were stable: proto control query p90 102.9 ms versus mmap 90.3 ms; godot 55.8 versus 50.7 ms. Writes/fresh builds were not uniformly stable: godot fresh max exceeded 2 seconds in some profiles; proto mtime-only p90 ranges 287.8–328.2 ms across native profiles. No outliers removed. No statistical-equivalence or confidence-interval claim.

Vacuum shrank proto DB from 9,973,760 to 9,957,376 bytes: **16,384 bytes**, about 0.16%. Godot combined/vacuum both 3,469,312 bytes. Fresh median increased from combined 220.5 to 299.9 ms and 166.5 to 249.1 ms respectively.

## Memory and checks

Untimed unchanged-query probes report peak **SQLite allocator** bytes, not process RSS, caller arena, OS page cache or mmap-resident pages:
- Proto control: 2,433,576; cache-only: 10,992,952; mmap: 414,800; combined: 331,824.
- Godot control: 2,380,480; cache-only: 3,978,888; mmap: 361,704; combined: 278,728.
- Large profile has approximately same allocator peak as combined; requested cache capacity is not eagerly allocated. These figures do not establish total memory savings from mmap.

Checks:
- All **1764 measured + 252 warmup samples** passed parity/source/cache/FTS checks; exits 0, no stderr. Pristine captures unchanged; no live project commands.
- All six experimental profiles pass 11 incremental/WAL regressions in both config modes, including injected rollback, ID swaps, global visibility, held-reader sync/create/update and non-writing doctor.
- mmap and combined additionally pass source-contract and projection/read suites in both config modes, including exact numeric and SQL/read-limit cases.
- Harness tests: 9 passed. Existing runner tests: 7 passed. Earlier code review fixed create/update bypass and fail-open doctor-sidecar metadata handling; metadata-access-denied injection not separately exercised.
- No full default adoption, fault-injected mmap I/O tests, crash/power-loss proof, long-lived readers, large synthetic corpus or arbitrary user sort workload covered. Memory-backed sorting can increase worst-case RAM; not recommended blindly.

## Reproduce and evidence

```sh
for profile in control cache mmap combined large vacuum; do
  bash exploration/attendant-native/experiments/sqlite/sqlite-tuning-build.sh "$profile"
done
node exploration/attendant-native/experiments/sqlite/sqlite-tuning-check.mjs \
  --manifest exploration/attendant-native/build/real-data/capture-C41Ig7/manifest.json \
  --runs 21 --warmup 3
```

- Driver: `experiments/sqlite/sqlite-tuning-check.mjs`; adapter: `experiments/sqlite/sqlite-tuning.c`; compiler flags shared through `tools/build-inputs.sh`. No new runtime dependency or production CLI option.
- Interleaved evidence: `build/bench/run-x87VUS/report.json`, `samples.jsonl`, `sqlite-tuning.json`; entry summary `build/sqlite-tuning-interleaved.json`.
- Initial screening retained: `build/sqlite-tuning-pilot.json`; do not compare its sequential profile times without accounting for drift.
- Probe logging uses `ATTENDANT_TUNING_REPORT=1` only outside timers; normal timings disable it. Raw reports retain executable hashes, environment, states, distributions and per-profile workspaces. All evidence ignored/local; summary retained here.

## Sources

SQLite documentation retrieved 2026-10-28:
- [Journal mode](https://www.sqlite.org/pragma.html#pragma_journal_mode): “The OFF journaling mode disables the atomic commit and rollback capabilities of SQLite.”
- [WAL](https://www.sqlite.org/wal.html): WAL can be slightly slower for mostly-read workloads; automatic checkpoint threshold defaults to 1000 pages; exclusive WAL access assumes only one accessing process.
- [Locking mode](https://www.sqlite.org/pragma.html#pragma_locking_mode): exclusive mode retains acquired locks, not a universally safe read acceleration switch.
- [mmap](https://www.sqlite.org/pragma.html#pragma_mmap_size): requested limit cannot exceed compiled/startup hard limit.
- [Planner optimize](https://www.sqlite.org/pragma.html#pragma_optimize): modern bounded planner-statistics maintenance; not a promise of faster arbitrary queries.
