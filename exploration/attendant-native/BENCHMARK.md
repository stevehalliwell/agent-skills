# CLI performance baseline

2026-10-28. Zig defaults to `zig cc -O3` for all sources, including SQLite. No fast-math or explicit assertion removal added. Native incremental/WAL changes remain uncommitted on top of `3ad2a79`.

Current adopted policy and native/CJIT/Node results: [SQLITE-ADOPTION.md](SQLITE-ADOPTION.md). Below retains earlier eager-FTS incremental/WAL and rebuild baselines.

## Historical incremental/WAL baseline

Native now updates affected rows/cache/FTS/trigram entries transactionally. Mtime-only changes refresh cache metadata without index churn. Native writers use WAL/NORMAL, 2000 ms busy timeout and default checkpointing. Full source hashing/parsing/validation remains; this is incremental SQLite writing, not `.mjs` stat-only source reuse. Query connections stay read-only; planner `PRAGMA optimize` runs after full builds.

Same two datasets, three runners, six cases, startup-inclusive wall clock: 20 measurements + 3 warmups each, compilation/reset/edit/check work excluded. All 720 measured + 108 warmups passed, source/pristine hashes unchanged. All body edits report `updated`; all mtime-only changes report `reused`, for every runner. Full build/unchanged states remain `rebuilt`/`reused` respectively.

Cells: median / p90 milliseconds. Ordinary shared machine: cross-run differences do not isolate WAL versus incremental algorithms or background activity. Separate safe read-tuning experiments and limits: [SQLITE-TUNING.md](SQLITE-TUNING.md). No experimental cache/mmap defaults adopted.

### proto-framework — 127 records

| Scenario | `.mjs` | CJIT | Zig `-O3` |
| --- | ---: | ---: | ---: |
| Fresh sync | 568.9 / 702.1 | 369.8 / 476.9 | 282.1 / 356.7 |
| Unchanged sync | 160.3 / 190.5 | 200.5 / 248.7 | 116.3 / 131.4 |
| Unchanged query | 236.6 / 240.8 | 193.9 / 197.6 | 114.3 / 118.0 |
| Unchanged search | 267.4 / 272.7 | 249.6 / 252.7 | 149.0 / 152.0 |
| Body edit, then sync | 214.7 / 220.4 | 361.2 / 366.1 | 206.0 / 213.4 |
| Mtime-only, then sync | 170.1 / 177.4 | 353.8 / 359.8 | 202.6 / 207.4 |

### godot-proto — 60 records

| Scenario | `.mjs` | CJIT | Zig `-O3` |
| --- | ---: | ---: | ---: |
| Fresh sync | 262.8 / 266.6 | 128.4 / 132.0 | 105.8 / 108.3 |
| Unchanged sync | 95.2 / 99.0 | 85.0 / 88.6 | 55.6 / 60.3 |
| Unchanged query | 131.6 / 138.1 | 90.1 / 93.2 | 60.9 / 64.1 |
| Unchanged search | 144.3 / 149.2 | 106.4 / 110.9 | 71.7 / 76.0 |
| Body edit, then sync | 152.4 / 188.1 | 169.1 / 204.4 | 114.9 / 148.8 |
| Mtime-only, then sync | 130.9 / 141.8 | 202.4 / 215.2 | 144.5 / 168.8 |

Zig body-edit sync is now competitive with `.mjs` on larger dataset and faster on smaller one. Metadata-only preparation still costs more than `.mjs`; native validates all source and replans under a writer transaction. Unoptimized CJIT remains slower on changed-source cases. Fresh-build timings did not improve here: WAL is not a universal acceleration switch.

Evidence: `build/bench/run-50cram/report.json`, `samples.jsonl`; summary `build/benchmark-incremental-wal.json`. Native source/read/prepare/CLI suites pass both compilers/configurations; 11 dedicated incremental/WAL cases cover rollback, ID swaps, global visibility, held readers, create/update, immutable doctor and replacement refusal. Latest harness tests 9 passed; existing runner tests 7 passed. Broader operational/real-data release acceptance remains pending.

## Historical rebuild results

Optimized Zig beats CJIT in all 12 tested dataset/scenario combinations. Compared with earlier unoptimized Zig run, median CLI times fall by factors of **1.95–3.18**. Those comparisons span separate runs on a shared machine, not a controlled estimate of optimization's exact effect.

Zig `-O3` beats `.mjs` on fresh sync and unchanged sync/query/search on both datasets. It also beats `.mjs` on smaller dataset's body edit. Larger dataset's changed-source rebuilds remain slower; smaller dataset's mtime-only change is close. No universal native speedup.

Earlier CJIT-versus-Zig ranking reflected an unoptimized Zig build. Do not use it to rank optimized compiler output. Historical measurements remain below and in original raw report.

## Historical rebuild runs — method

- Windows x64 `10.0.26200`; AMD Ryzen 5 7600, 12 logical CPUs, 34,080,960,512 bytes RAM. Ordinary shared machine; background activity uncontrolled.
- Node `v24.16.0`, CJIT `v1.2.0`, Zig `0.17.0`. All runners report SQLite `3.53.0`. Native uses DELETE/FULL and temporary DB publication; `.mjs` uses WAL/NORMAL. Not an isolated language/engine comparison.
- Build executables **before** correctness/timing runs. Only build change between runs: add `-O3` to Zig invocation in `build.sh`. `.mjs` and CJIT artifact hashes match between runs. Harness launches prebuilt executables directly; never invokes compilers.
- Same external monotonic timer for all runners: immediately before process launch through exit/output capture. Includes CLI startup and parent handoff. Excludes fixture reset/copy, source edits, JSON parsing and correctness checks.
- Warm filesystem, fresh process every invocation, serial rotating runner order. Per runner/dataset/scenario: 3 warmups, then 20 measured samples. No outlier trimming. Median and interpolated p90 are descriptive, not confidence intervals.
- Fresh owned copies only. Fixed integer-second source mtimes and closed per-runner DB snapshots make resets reproducible. Body-edit trial appends one unique token; mtime-only trial advances timestamp without changing bytes. Detailed protocol/commands: [README](README.md#cli-performance-baseline).
- Workloads: 25 tasks ordered by priority/creation/name; 20 `task` search hits. Both datasets return data. Native results match `.mjs`, with documented score rounding allowance.

## Historical optimized rerun — before incremental/WAL

Cells: **median / p90 milliseconds**, including startup. All three runners remeasured in same run; not old Node/CJIT values paired with new Zig values.

### proto-framework

127 records: 93 tasks, 34 decisions. 134 source files, 1,508,608 source bytes. Baseline DB: 9,969,664 bytes for each runner.

| Scenario | `.mjs` | CJIT | Zig `-O3` |
| --- | ---: | ---: | ---: |
| Fresh sync | 524.1 / 574.7 | 271.4 / 293.8 | 201.3 / 221.5 |
| Unchanged sync | 145.3 / 150.5 | 183.6 / 189.4 | 105.0 / 117.5 |
| Unchanged query | 232.4 / 249.5 | 186.7 / 189.6 | 107.1 / 120.9 |
| Unchanged search | 269.1 / 295.8 | 241.3 / 255.5 | 143.4 / 152.9 |
| Body edit, then sync | 213.4 / 226.1 | 420.8 / 444.8 | 276.1 / 300.9 |
| Mtime-only change, then sync | 165.6 / 170.9 | 414.0 / 421.9 | 271.1 / 288.5 |

### godot-proto

60 records: 55 tasks, 5 decisions. 67 source files, 471,535 source bytes. Baseline DB: 3,452,928 bytes for `.mjs`/Zig; 3,457,024 bytes for CJIT. Physical SQLite size differences are not logical-result differences.

| Scenario | `.mjs` | CJIT | Zig `-O3` |
| --- | ---: | ---: | ---: |
| Fresh sync | 263.7 / 270.4 | 105.2 / 111.6 | 82.9 / 90.1 |
| Unchanged sync | 91.4 / 98.0 | 76.9 / 81.2 | 51.8 / 56.8 |
| Unchanged query | 131.1 / 138.8 | 83.8 / 87.3 | 55.2 / 62.6 |
| Unchanged search | 147.7 / 151.3 | 99.5 / 104.7 | 65.2 / 73.5 |
| Body edit, then sync | 137.3 / 169.6 | 162.5 / 181.3 | 111.4 / 151.9 |
| Mtime-only change, then sync | 107.4 / 113.2 | 161.3 / 176.7 | 109.6 / 117.0 |

## Initial unoptimized baseline — historical only

Before `-O3` became Zig default. Same protocol; all three runners measured. Cells: median / p90 milliseconds. These are not current Zig results.

### proto-framework

| Scenario | `.mjs` | CJIT | Zig unoptimized |
| --- | ---: | ---: | ---: |
| Fresh sync | 526.2 / 572.3 | 270.4 / 293.5 | 493.7 / 519.7 |
| Unchanged sync | 138.2 / 144.9 | 171.2 / 178.8 | 332.8 / 344.4 |
| Unchanged query | 222.9 / 240.0 | 174.7 / 181.6 | 340.1 / 347.9 |
| Unchanged search | 248.9 / 266.4 | 224.1 / 238.1 | 447.8 / 463.1 |
| Body edit, then sync | 201.3 / 217.4 | 390.2 / 427.2 | 752.8 / 822.5 |
| Mtime-only change, then sync | 162.2 / 184.6 | 400.4 / 415.2 | 763.6 / 800.1 |

### godot-proto

| Scenario | `.mjs` | CJIT | Zig unoptimized |
| --- | ---: | ---: | ---: |
| Fresh sync | 250.3 / 260.0 | 101.1 / 104.7 | 162.0 / 167.5 |
| Unchanged sync | 89.0 / 92.7 | 76.2 / 79.6 | 138.5 / 144.7 |
| Unchanged query | 130.0 / 138.7 | 84.4 / 88.0 | 148.0 / 158.8 |
| Unchanged search | 134.1 / 136.8 | 92.6 / 96.5 | 170.0 / 173.0 |
| Body edit, then sync | 127.3 / 134.1 | 147.5 / 152.0 | 263.4 / 278.4 |
| Mtime-only change, then sync | 101.0 / 103.6 | 148.0 / 157.0 | 266.7 / 275.9 |

## Checks and evidence

- Optimized Zig passed relative/absolute source-contract, projection/read, preparation and CLI suites: 10 source groups, 23 read comparisons + 27 rejections, 34 lifecycle cases, 19 CLI cases + 18 rejected batches per configuration. Source-contract checks include exact integers/floats, mixed TEXT-affinity values, invalid numeric/date inputs, typed round trips and stale lossy projection detection. Node-free native execution and source/write failure checks passed.
- Harness unit tests: 8 passed; existing `.mjs` tests: 7 passed.
- Each full benchmark run: 720 measured samples + 108 warmups; all 828 passed checks, exited 0, emitted no stderr. Initial unoptimized smoke also passed both datasets, all runners/scenarios, one warmup + one measured sample. Capture refactor verified on project fixture.
- Strict validation, schema/usage preflight, query/search parity, record counts, projected hashes, cached mtime/size/hash, edited-body FTS hits, source preservation and unchanged pristine inventories passed. No CLI ran against original projects. Full operational/real-data acceptance remains pending.
- Independent read-only harness review found missing cached-mtime verification. Added check and stale-cache regression before both full runs. Bounded follow-up found no remaining actionable issue in fix. Review inspected source only; did not execute benchmarks.
- Source capture: `build/real-data/capture-C41Ig7/manifest.json`.
- Optimized evidence: `build/bench/run-U3wwg9/report.json` and `samples.jsonl`; entry-point summary `build/benchmark-zig-o3.json`.
- Initial unoptimized evidence, preserved: `build/bench/run-FRyLLI/report.json` and `samples.jsonl`; entry-point summary `build/benchmark-baseline.json`.
- Each report retains full precision, min/max/means, state counts, versions and executable/script hashes. Expected outputs and workspaces live alongside reports. Ignored, local evidence; this document retains summary if build artifacts are removed.

All 20 samples per dataset/runner agree on sync states in both historical rebuild runs:
- Fresh: all `rebuilt`.
- Unchanged: all `reused`.
- Body edit: `.mjs updated`; CJIT/Zig `rebuilt`.
- Mtime-only: `.mjs reused` with refreshed cache metadata; CJIT/Zig `rebuilt`.
- Query/search do not expose projection state; report records `not-reported`, not an inferred state.

## Implications

1. Use current incremental/WAL baseline for later comparisons. Keep earlier rebuild runs as historical evidence.
2. Retain exact source checks; source-cache fast paths remain separate work. Do not infer source-processing hotspots from total timings alone.
3. WAL publication/recovery and synchronous/busy/checkpoint settings are implemented together. Measurements do not isolate each setting or prove globally optimal tuning.
4. Keep launcher/skill integration last. No native rollout or production replacement authorized by these checks.
