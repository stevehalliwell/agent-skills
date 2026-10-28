# Attendant native discussion

Explore moving Attendant from Node.js to C99 + CJIT. Core native commands now implemented within Windows/resource/flat-YAML bounds; production replacement and launcher rollout remain undecided. Current contract, commands and evidence: `exploration/attendant-native/README.md`.

## Goals

- Remove Node.js runtime requirement.
- Support C99 through `cjit` and `zig cc`; prefer CJIT when both work. Discover compilers on PATH unless an explicit user-configured path overrides discovery. Try `zig cc` if CJIT is unavailable or cannot build Attendant. If neither path works, report missing or failed components and stop; do not install tools or dependencies automatically.
- Vendor SQLite source. Verify builds, FTS5/trigram support, and linking for both compiler paths during feasibility work.
- Run toolchain and required dependency checks automatically on first Attendant use; do not require a separate setup command. Exact probes and build-artifact reuse remain open.
- Serve user's environment first. For broader use, skill must flag when it cannot find a way to compile a C99 program; standalone binary distribution is not a first-release requirement.
- Support Windows first; other operating systems are future work. Aim for small footprint and faster startup; measure responsiveness against the Node runner, but do not require a fixed speedup.
- Preserve Markdown as source of truth and `.attendant/` as disposable projection state.
- Keep each dataset self-sufficient and regeneratable. Markdown remains source of truth; define exact regeneration boundary during refinement.
- Preserve useful CLI capabilities, except migration commands; redesign interactions against `cli-design` skill.
- Keep transparent automatic repair for copied Markdown records. Report applied repairs through CLI and, if built, viewer warnings.
- Human and agent own Git operations, diffs, and recovery. No Git awareness is needed in native Attendant for now; retain possible future uses as exploration.
- Explore a reusable native core that could later support a raylib/cimgui visual collaboration layer without duplicating record rules. Primary value: shared context between human and agent, not a better standalone data editor.

## Anti-goals

- Do not start a rewrite before assessing feasibility and agreeing scope.
- Do not build the viewer/editor as part of this discussion. Decide its release scope after native Attendant reaches approximate parity with the existing runner.
- Do not replace Markdown with a database as source of truth.
- Do not choose Zig without a concrete benefit over preferred C99 + CJIT path.
- Do not assume smaller size or better performance without evidence.
- Do not change unrelated skills or tools.
- Do not include dataset import, schema migration, or migration-plan execution in native CLI. Human/agent workflows can transform Markdown outside tool.
- Do not invoke Git, create commits, or make CLI responsible for Git-based rollback.
- Do not make GUI exploration a commitment to build or ship GUI.

## Current behavior inventory

Inspected bundled runner, YAML dependency, CLI help, references, and all seven shipped integration tests. All seven tests pass on Node.js v24.16.0. Temporary-fixture probes confirmed behaviors below; no project records were modified.

### Product surface

- `add-table`: create and register an empty collection with schema, usage, and template files. Bootstrap project setup remains skill-guided, not a CLI command.
- `create` / `update`: JSON-array batches; generated UUIDv7 identity and timestamps; declared fields plus writable `desc` and `tags`; template body on create, existing body preserved on update. Update cannot change identity or edit body. Replacement is atomic per file, not across batch.
- `validate`: validate schemas, metadata, field types, dates, duplicates, and references. Default applies mechanical corrections. `--no-correct` reports; `--strict` also fails for diagnostics or pending corrections.
- `sync`: build, reuse, or incrementally reconcile disposable SQLite projection. Does not run source corrections itself.
- `schema`: source locations, live usage text, field declarations, SQL columns, FTS metadata, diagnostic and reference-edge table metadata.
- `query`: one constrained read-only SQL statement with bound parameters and SQLite authorizer restrictions.
- `search`: ranked full-text matches followed by additional trigram partial matches; collection filters and snippets.
- `doctor`: read-only runtime, SQLite FTS5/trigram, configuration, write access, ignore rule, projection health, and validation checks.
- `migrate check/apply`: reviewed Markdown plans, source hashes, exact text-removal spans, path/collision checks, and Git prerequisite for apply. Check covers mapping mechanics, not full resulting schema/record/reference validity. Apply can leave partial writes.

### Data and implementation

- Flat schema uses scalar defaults, non-empty scalar enum arrays, optional date/datetime, string arrays, references, and untyped fields. Migration plans themselves use nested YAML.
- Collections scan immediate non-hidden `.md` files. `.usage.md` supplies guidance, not enforced policy; `.template.md` supplies body copy.
- Projection includes record fields, source paths/hashes, filesystem-derived `updated_at`, diagnostics, reference edges, and two FTS5 indexes per collection. Hidden records are excluded from projected collection rows and search.
- Cache uses file metadata and hashes, with incremental reconciliation and rebuild fallback. Matching file size and modification time can avoid content reads.
- Dependencies to replace or supply: bundled YAML parser/serializer, SQLite with FTS5/trigram and authorizer, JSON, SHA-256, secure random UUID generation, dates, Unicode handling, filesystem/path operations, and Git subprocess for migration apply.
- YAML updates use the vendored `yaml` v2.9.0 document model rather than rebuilding metadata solely from plain objects. Record front matter is delimited in `attendant.mjs`; its body is carried as text, not parsed by a Markdown library. Preservation of comments, formatting, line endings, and body bytes needs explicit compatibility tests.
- C YAML candidates need evaluation, not adoption. [libfyaml](https://github.com/pantoniou/libfyaml) has a document API and experimental opt-in comment handling; its own documentation describes round-trip comment placement as best effort. [LibYAML](https://github.com/yaml/libyaml) is another parser/emitter candidate; comment-preserving edits need separate investigation. Verify Windows, CJIT, `zig cc`, licensing, supported YAML semantics, and source-preservation fixtures before selection.
- A C Markdown parser such as [MD4C](https://github.com/mity/md4c) is a candidate only if future functionality needs Markdown structure. Current record processing does not establish that dependency.
- Preferred JSON dependencies already exist in `D:/Repos/proto-framework/vendor/`: `libpdjson5` for reading and `json-build` for writing. Reuse these as the starting point for native JSON needs; check their pinned versions, licenses, strict JSON behavior and compiler compatibility before copying. `include/json.h` provides optional fixed-shape native bindings, but excludes maps and optional values and requires object roots; do not assume that wrapper fits Attendant's schema-driven fields and array-root batch inputs. The underlying libraries can be evaluated independently.
- Completed YAML spike: `spikes/c99-yaml-frontmatter/RESULTS.md`, probes, fixture and raw results retained here. Parent independently reproduced LibYAML builds and identical probe output with CJIT v1.2.0 and Zig 0.17.0. LibYAML can parse and emit tested mappings but drops comments, changes formatting, accepts duplicate keys, and needs explicit Attendant subset and scalar resolution logic. It is not a drop-in source-preserving editor. libfyaml runtime behavior remains untested: direct compilation hit missing `xxhash.h`; supported CMake build was not attempted. A narrow JS splice preserves simple fixtures but rejects the real flow-sequence example; it is not a native editing solution. Current runner-equivalent serialization also changes some formatting and produces a mixed CRLF closing delimiter. LibYAML now supplies bounded native reads and edit spans, never whole-document emission. CLI completion exercises deliberately scoped source-preserving edits; full YAML equivalence remains unproven. libfyaml remains unadopted and runtime-untested.
- Copied `base.h`, `build.h`, and `test.h` from `D:/Repos/proto-framework/include/` at commit `f0b848e2541903ac99696f1f8a71d93e72ef4c5b` into `exploration/attendant-native/include/`. `base.h` supplies allocators, strings, collections, file and process helpers; `build.h` supplies a bounded CJIT/Zig build driver; `test.h` supplies assertions and runner generation. These remain exploration dependencies, not a final packaging choice. `base.h` collection macros require `__typeof__`; `build.h` bootstraps via CJIT even when targeting Zig, so it does not by itself satisfy a Zig-only fallback.
- A minimal combined-header assertion ran with CJIT v1.2.0 and Zig 0.17.0. Zig's default global cache failed to link due to missing cache artifacts; a fresh isolated global cache passed. This does not verify SQLite, YAML, build driver operation, or full header behavior.

### Native behavior baseline

Preserve existing Attendant core features and broad behavior in native exploration, except the agreed removal of migration commands. Reporting changes aim for consistency and visibility, not feature redesign. Treat Markdown authors as responsible for content and intent; Attendant makes configured records usable, reports problems, and does not invent corrections to malformed content.

- Dataset source: `.pi/attendant.tables` identifies collection directories and optional aliases. Each collection has `.schema.md` and Markdown records; `.usage.md` supplies human/agent guidance and `.template.md` supplies optional new-record body text. Usage and template files are not projected records. Configured external paths remain dependencies of the dataset.
- Projection: `.attendant/` is disposable. Normal operations prepare or refresh a SQLite projection from configured source files; reuse or reconcile when possible, rebuild when needed. Projection includes queryable fields, source paths and hashes, diagnostics, reference edges, and full-text indexes. Hidden or invalid records can be omitted. Rebuilding does not guarantee identical filesystem-derived `updated_at` values or absolute source paths after files move.
- Inspection and lookup: `schema` exposes collection fields, usage and projected tables; `query` runs constrained read-only SQL; `search` searches metadata and Markdown body text. `doctor` diagnoses health without changing source or projection. Routine data actions prepare projection without requiring a separate sync.
- Records: create uses schema defaults and optional template body; update changes declared metadata while retaining the body and immutable identity fields. Validate checks source against schemas and can apply mechanical corrections. Current normal preparation can also apply corrections; native output must make applied corrections visible. Batch writes are not cross-file transactions.
- Git: current non-migration operations do not invoke Git; `doctor` checks whether `.attendant/` is ignored. Current migration apply invokes Git, but migration commands are excluded from the native CLI. Further Git-context reporting is an exploration, not existing behavior.
- Usability boundary: report malformed YAML and skip that file; continue with other records. Keep useful schema diagnostics and deterministic best-effort duplicate correction without making Attendant responsible for the author's intent. Implementation should handle ordinary failures without turning each edge case into a product decision.

### Agreed direction

- Remove `migrate check/apply` from native CLI. Importing non-Attendant data and updating records for schema changes remain external human/agent workflows.
- Native CLI does not need Git awareness or invoke Git. Human and agent own diff and recovery; Git presence alone does not guarantee source is recoverable. Possible future exploration: report worktree context or warn when source changes occur outside Git; neither is first-release scope.
- Keep automatic correction: models may copy Markdown and edit directly instead of using CLI. Make records usable and avoid confusing duplicate identities or names on a best-effort basis. Markdown authors own correctness; Attendant does not guarantee or repair author intent.
- Make corrections visible without treating successful repair as operation failure. Structured results identify what was corrected and the affected record; human-facing warnings go to stderr. Do not require old/new values in repair output; human or agent can inspect Git diffs for detail. The CLI does not invoke Git. Future viewer may surface the same events.
- For duplicate IDs and names, prefer valid `created_at`, then filesystem creation time, then path as practical tie-breaks. Unknown dates sort after valid dates; if any filesystem creation time in a tied group is unavailable, use path order for that group. These are heuristics, not proof of original identity. Disambiguate duplicate names in metadata only; do not rename files.
- Report malformed YAML, ignore the affected file, and continue processing other records as usual. Never repair malformed YAML; its author owns correction.
- If a repair write fails after other repairs succeeded, fail the command and report completed and failed repairs in structured output. Leave completed repairs in place; do not attempt rollback.
- Apply `cli-design` skill. Preserve existing successful command-specific JSON shapes where practical; extend them with consistent repair and diagnostic reporting rather than wrapping all results in a new envelope. Preserve stdout data/stderr display separation; define stable machine-readable errors, field selection, empty results, agent mode, and exit semantics. Existing always-JSON output is established behavior; do not silently switch defaults based on TTY.
- Viewer remains exploratory during this discussion. Decide whether to include it in a release after native Attendant reaches approximate parity with the existing runner. Show collections and their files, render and edit Markdown, and follow links between records. Investigate current cimgui-related Markdown preview/editing tooling; exact component and compatibility need verification. Agent can reveal locations without changing human selection; viewer reports the current human selection back to the tool for agent retrieval. Selection granularity and transport remain open.
- Showing a proposed edit location or reporting a selection does not itself mean edit approval. Define approval separately if needed.

### Existing `.mjs` findings

- Inspection can write source: `schema`, `query`, and `search` prepare project, applying missing/default metadata and resolving duplicate names/IDs when projection is not fresh. These corrections are not included in those commands' returned results.
- Success is one JSON value; operation failures generally emit text on stderr with exit 1 and no stdout JSON. Validation and projection diagnostics are embedded in successful JSON results. Non-strict validation can exit 0 with diagnostics.
- Query/search return `columns`, `rows`, `returned`, and `truncated`. Default limit is 100; shaping caps output at 10,000 rows and 10 MiB of serialized rows, but SQL/search results are fully materialized before shaping.
- Confirmed inconsistencies: `--limit 0` returns up to one row; empty query results lose column metadata; reused projection diagnostics use `source_path`/`collection_name` while rebuilt/updated diagnostics use `path`.
- Lock acquisition and some generated-file cleanup retries have no deadline. Future GUI alongside CLI makes bounded waiting and concurrent-write handling important.
- Native prototype now exposes structured errors/diagnostics and automatic repairs; broader product CLI contract remains open. Possible additions: optional correction preview, body read/edit through shared core, and stale-edit conflict detection. These additions are not approved scope. Migration improvements are excluded from native CLI.

## First slice: native projection, existing CLI consumer

Build a C99 projection builder for copied sample datasets. Existing `.mjs` CLI consumes the generated database unchanged. This tests the native read path before source editing or CLI replacement.

Scope:
- Read `.pi/attendant.tables`, collection schemas, and valid, already-normalized Markdown records from isolated fixture datasets.
- Generate `.attendant/attendant.sqlite` matching the current runner's layout: collection tables, metadata/cache, diagnostics, reference edges, and FTS5 Unicode/trigram indexes. Match source hashes, paths and filesystem metadata needed for cache reuse.
- Vendor SQLite source. Use the YAML spike's LibYAML evidence as a starting point, not proof of complete scalar or schema compatibility. Reuse preferred JSON libraries and copied framework headers where they fit.
- Build and test Windows paths with CJIT and `zig cc`. Keep any blocker explicit; do not silently drop either path.
- Do not change fixture source files or original sample records. Existing runner may normalize fixture copies during fixture preparation; freeze and hash them before native-build acceptance checks.
- Exclude source repair, create/update, incremental reconciliation, CLI replacement, and viewer implementation. Existing CLI and native builder operate sequentially for this slice.

Acceptance:
- From no projection, native builder generates the database for a fixture with expected records, schema field kinds, cross-record references, and searchable metadata/body text.
- Existing `.mjs sync` reports `state: "reused"`, not `rebuilt` or `updated`. A successful query alone is not sufficient because the runner can silently replace incompatible output.
- Existing `schema`, parameterized read-only `query`, and `search` return expected fixture results. Compare with an independently runner-built projection for the same frozen fixture paths; semantic comparison, not identical SQLite file bytes.
- Frozen source hashes remain unchanged through generation and consumption. Report compiler versions, commands, results, and remaining limits.
- Present code, repeatable checks, and a short demo to the user. Stop at user review before expanding scope toward broader parity.

## Steps

Check steps only when completed. Complete the first slice through user review before returning to the remaining refinement and parity work.

### First slice through user review

- [x] Prepare isolated copied fixtures and expected results from existing Attendant; freeze source and record hashes after any fixture normalization.
- [x] Trace the current projection contract and build a reference database for comparison; document required layout, hashes, paths, cache metadata, and FTS behavior.
- [x] Establish reproducible Windows native builds with vendored SQLite, YAML reading, and required JSON support; verify CJIT and `zig cc` or report blockers.
- [x] Implement full projection generation from frozen fixture source without editing Markdown. Match existing projection contract rather than redesign it.
- [x] Run end-to-end acceptance: `.mjs sync` reuses native output; schema/query/search give expected results; frozen source stays unchanged. Compare native and runner-built logical results.
- [x] Prepare concise review evidence and demo commands; report limitations and compiler-path results. See `exploration/attendant-native/README.md`; four compiler/configuration scenarios passed and existing runner's 7 tests passed. Build outputs and machine-readable check results remain under ignored `exploration/attendant-native/build/`.
- [x] User reviewed the native-generation/existing-CLI result and requested committing this slice. Further implementation scope remains a separate decision.

### Native read slice

User approved native `schema`, `query`, and `search` over compatible projection, without further product refinement. This slice initially deferred repair/writes/incremental refresh; CLI completion below adds source writes. Read slice initially required an existing snapshot; preparation slice below replaces that boundary. Current implementation and acceptance commands: `exploration/attendant-native/README.md`.

- [x] Add native read commands with familiar arguments and command-specific JSON results; use preferred strict JSON reader for parameters.
- [x] Enforce read-only SQLite and authorizer containment; support parameter binding, file/stdin inputs, limits and structured errors.
- [x] Preserve useful reporting for empty/limited results and valid UTF-8 JSON at the SQLite TEXT boundary. Malformed byte sequences become U+FFFD; oversized serialized rows truncate.
- [x] Keep deterministic search tie-breaking. Exact Node locale collation is not required; do not add ICU for this distinction.
- [x] Finish final build, regression and independent review checks. Both compiler paths passed four configuration scenarios, each with 23 oracle comparisons and 27 rejection cases, plus byte/row limits, UTF-8, live usage, snapshot producer interoperability and stable-tie checks. Existing runner's 7 tests passed. Independent read-only review found no remaining actionable issue in scope. Evidence: `exploration/attendant-native/build/review/read-results.json` (generated).

### Native preparation slice

At `5e2d32a`, user-approved native `sync` and automatic read preparation reused unchanged projection or fully rebuilt changed/missing projection without source writes. CLI completion below supersedes normalized-only/untouched-source boundary: reads can correct metadata; malformed records are diagnosed/skipped. Incremental reconciliation remains excluded.

Acceptance: native reads build a missing projection without Node; unchanged operations reuse it; adding, editing or deleting source appears on the next read; existing `.mjs sync` reuses native output; source hashes stay unchanged. Preserve the published projection on rebuild failure. No concurrent producer/consumer guarantee in this slice.

- [x] Extract reusable in-process projection preparation and add native `sync`; positional project invocation still forces a build. Prior progress and this slice's plan committed as `3012b65`.
- [x] Detect source changes and reuse or fully rebuild; invoke preparation before native reads. Scan/hash/validate all bounded source; compare content even when size/mtime are unchanged. No Node subprocess or Markdown writes.
- [x] Verify missing/unchanged/added/edited/deleted/config/schema cases, `.mjs` interoperability, failure preservation and unchanged source on both compiler paths. Each of four compiler/configuration scenarios passed 34 named lifecycle cases, including Node absent from PATH, collection changes, missing/corrupt DB parts, failed SQL steps, blocked publication, active WAL and genuine hot-journal preservation. Existing read checks passed 23 comparisons and 27 rejections per scenario; runner's 7 tests passed.
- [x] Update commands and evidence. See `exploration/attendant-native/README.md` and generated `build/review/prepare-results.json`. Independent read-only review found journal protection, statement-cleanup and missing-edge-table gaps; fixes gained executable regressions. Follow-up inspection found no remaining actionable defect in fixes. Preparation slice completed and reviewed. Source repair, incremental reconciliation and concurrent publication remain separate work.

### Native CLI completion slice

User approved completing all remaining core CLI commands in order within one slice. Preparation progress committed as `5e2d32a`. Scope: `doctor`, `validate` (including `--no-correct` and `--strict`), `add-table`, `create`, and `update`. Migration/import commands remain excluded.

Agreed sequence: `doctor`; `validate` diagnostics; source-preserving mechanical correction; `add-table`; record `create`/`update`. Keep existing command-specific success shapes, batch preflight, immutable metadata and body preservation. Malformed YAML must be reported and skipped, never repaired. Completing command names alone does not establish parity: reads/preparation must also handle invalid records and expose corrections consistently. Source writes use bounded top-level YAML edits rather than a whole-document emitter: preserve unrelated source bytes, comments, newline style and body. Preflight all requested records before publication; replacements atomic per file, not across an entire update batch. Report already-written paths if later publication or projection refresh fails. Do not add Git checks to doctor. Retain exploration resource/platform/YAML boundaries and document tested parity rather than claim full YAML equivalence.

- [x] Implement native doctor and non-writing validation with structured diagnostics. Doctor inspects SQLite immutably; strict/no-correct do not touch source/state.
- [x] Implement mechanical correction and malformed-record skipping in preparation; expose corrections. Prefer valid source dates, then available filesystem creation times within tied cohorts, then path; never rename files. Event preflight rejects nested/root-sequence input and counts implicit-null/indentless arrays before recursive document load.
- [x] Implement add-table with safe preflight and cleanup. Reject unrepresentable config entries, disposable-state paths and capacity overflow before writes.
- [x] Implement create/update, strict transport, batch preflight and source-preserving writes. Escape YAML-sensitive Unicode; verify parsed meaning. Report `failed` source path and committed `written` paths on staging/publication failure; preserve source commits after projection-refresh failure.
- [x] Verify both compiler paths, `.mjs` interoperability, source preservation and failure paths. Four compiler/config scenarios each pass 19 CLI cases plus 18 rejected batches, 34 preparation cases, 23 read comparisons and 27 read rejections; existing runner's 7 tests pass. CLI checks cover Node absent from PATH, exact body/unrelated comments, CRLF, YAML-sensitive Unicode, duplicate creation-time heuristics, 1024/1025 array boundaries, locked-file partial publication, config rollback, projection-refresh reporting and 32-collection capacity. Independent read-only review drove comment-span, encoding, config grammar, capacity and event-bound fixes; final bounded follow-up found no remaining actionable issue. README and generated `build/review/cli-results.json` retain commands/evidence. Slice complete; included in CLI/source-contract checkpoint.

### Source contract task

Status: complete; included in CLI/source-contract checkpoint.

Checks: CJIT/Zig × relative/absolute config passed focused source, projection/read, preparation and CLI suites. Existing runner's 7 tests passed. Independent read-only review drove precision regressions; final bounded follow-up found no remaining actionable issue in inspected fixes. Commands, limits and generated evidence: exploration README.

Goal: enforce Attendant's deliberate source contract, not JavaScript/YAML-library permissiveness.

Contract:
- Datetimes exactly `YYYY-MM-DDTHH:mm:ss.sssZ`; date-only fields exactly `YYYY-MM-DD`. Check calendar/time validity. Reject alternate spellings without guessing or rewriting author values.
- Keep flat YAML mappings, scalars and scalar arrays. No nested objects, anchors, aliases, tags or extra documents. Preserve unrelated formatting/comments and Markdown bodies.
- Read decimal numbers into `i64`, `u64` or finite `f64`; preserve integer precision through YAML/JSON, validation, writes, projection and query output. No JavaScript safe-integer restriction or general JavaScript serialization requirement.
- Preserve valid UTF-8 values without locale collation or Unicode normalization. Reject invalid source encoding. Treat literal NEL/LS/PS as scalar content, not line breaks; YAML syntax remains enforced. Local LibYAML patch and provenance document this boundary.
- Paths are local inputs/derived DB values, not cross-machine portable identifiers. Keep configured source spelling untouched; resolve separately for filesystem access. No UNC/long-path expansion unless actual checks reveal an in-scope defect.

Storage: reject scalar numeric-column values above `INT64_MAX` explicitly; never round or change numeric columns to text. Full `u64` remains valid inside JSON-backed values such as arrays. Oversized identifiers can use explicitly quoted string fields.

Acceptance:
- [x] Canonical dates pass; alternate timestamp formats fail with diagnostics and no guessed repair.
- [x] Integer boundaries and finite float cases retain intended values across supported transports, source writes, cache/projection and query output; overflow fails explicitly.
- [x] UTF-8 text and unrelated source/body bytes remain unchanged through read/write round trips.
- [x] Existing restricted YAML forms remain supported; elaborate forms stay rejected.
- [x] Both compiler paths pass focused regressions and affected existing suites. Document intentional differences from `.mjs`.

Implementation: `numbers.h` retains exact integers and checked mixed numeric equality; SQLite INTEGER binding/output stays exact. Strict date parser validates without normalization. Source serialization preserves float syntax; typed numeric readback catches meaning changes that JSON string comparison misses. Cache reuse checks decoded front matter and precision-sensitive projected numbers, catching old rounding even when cached JSON digits match. Honor TEXT affinity in mixed enums; explicitly serialize numeric TEXT values to avoid SQLite's lower-precision double conversion. Focused suite: `exploration/attendant-native/source-check.mjs`; generated evidence: `build/review/source-results.json`. README owns detailed limits and intentional `.mjs` differences.

Excludes: migration/import, broader YAML grammar, locale matching, path portability, incremental reconciliation, launcher rollout, viewer and commits.

### Incremental projection and WAL task

Status: complete; implemented and compared. No commit or launcher rollout.

User authorized partial updates after measured full-rebuild costs, WAL and read-heavy settings, then same timing comparison.

Contract:
- Keep full source scan/hash/parse/validation and exact typed-value checks. Incremental scope is SQLite projection writes, not stat-only source-cache trust.
- Reconcile added/edited/deleted/renamed records and hidden/blocked transitions in one SQLite transaction. Delete whole affected set before reinsertion for ID swaps. Preserve unchanged rows/FTS/trigram entries; refresh global diagnostics/reference edges after projected changes.
- Mtime-only change refreshes cache metadata and returns `reused`, without rewriting projected rows/indexes. Content changes return `updated`; missing/config/schema/incompatible state returns `rebuilt`. Create/update share same post-publication refresh path.
- Writers use WAL/NORMAL, 2000 ms busy timeout, default 1000-page automatic checkpoint; replan under `BEGIN IMMEDIATE`. NORMAL trades projection power-loss durability, not SQLite consistency; Markdown stays authoritative. Do not use OFF modes or disable checkpoints. Run planner `PRAGMA optimize` after full builds; retain read-only/query-only consumers. No unmeasured mmap/cache/index tuning.
- Full rebuilds still stage/checkpoint/close before atomic replacement. SQLite may checkpoint/close existing WAL, but native never deletes published sidecars; active consumers can block replacement. Protect hot rollback journals. Doctor stays non-writing and conservatively marks active/nonempty WAL stale.
- No concurrent source-writer guarantee, cross-file source transaction, launcher rollout or production switch.

Checks: both compilers/config modes pass source/projection/read suites, 34 lifecycle cases, 19 CLI cases plus 18 rejected batches, and 11 incremental cases. Audit triggers prove changed-only writes; mid-delta fault proves transaction rollback; held-reader fixtures verify sync/create/update snapshots, doctor immutability and replacement refusal. Independent review drove shared create/update refresh and fail-closed sidecar inspection. Dedicated doctor cases cover empty sidecars and invalid WAL directory; access-denied metadata injection not separately exercised. Full three-runner timing comparison passed 720 measured + 108 warmup samples; all body edits `updated`, all mtime-only cases `reused`. Evidence: `exploration/attendant-native/build/bench/run-50cram/`; results in `BENCHMARK.md`.

User supplied Google bulk-ingest/read-only recipe and requested tests. Safe experiments keep WAL/NORMAL and normal locking. Six Zig profiles (control, 16 MiB cache, 64 MiB mmap, combined, large caps, extra VACUUM/ANALYZE) interleaved with Node: 1764 measured + 252 warmups passed. Unchanged-read medians favor mmap by roughly 7–13%; huge caps and vacuum add little here. Background drift invalidated naive sequential pilot rankings, so repeated in one balanced interleaved run. Experimental defaults not adopted. `journal_mode=OFF`, synchronous OFF, exclusive locking, delayed index construction and private unsafe bulk ingest not tested/adopted. Method, sources, limits, memory probes and results: `SQLITE-TUNING.md`; raw evidence `build/bench/run-x87VUS/` under same exploration. That safe-setting stage favored mmap 64 MiB, not wholesale recipe; later experiments below supersede its next-step priority.

User then authorizes disposable-cache safety/speed experiments and proposes lazy FTS because agent usage reportedly favors SQL/ripgrep. Implemented 12 isolated generated C variants, not normal defaults: control, WAL/OFF, private bulk OFF/OFF/exclusive, MEMORY/OFF, lazy, lazy+WAL/OFF, no routine quick_check, 8/16/32 KiB pages, eager FTS populated after rows, final FTS5 optimize. Main interleaved matrix passed 3042 measured + 468 warmups; never-searched sync/edit follow-up passed 676 + 104. Lazy WAL/NORMAL fresh medians 71.5/67.4 ms versus eager 300.4/202.1 ms; first search pays deferred work. Warm-query no-quick-check medians 56.9/42.7 ms versus 134.3/68.7 ms. Never-searched lazy edit medians 89.5/64.1 ms; lazy+WAL/OFF 57.3/43.7 ms. Absolute values across separate runs not interchangeable.

All 12 profiles pass source-contract checks in both config modes and injected delta SQL rollback. Lazy variants pass CLI suites and 8 focused lifecycle cases, including old reader snapshot during invalidation/repopulation. Selected killed incremental writes/private builds/population recover in tested process-death cases; MEMORY writer correctly fails busy with held reader. No power-loss qualification. Independent source review found missing search-after-edit setup verification; fixed and regression added before full run; follow-up no actionable finding within inspected scope. Normal launchers/defaults untouched. Method, profiles, evidence, limits and remaining combined-candidate/recovery work: `SQLITE-DISPOSABLE.md`. Main raw `build/bench/run-4KfwN5/`; follow-up `build/bench/run-ZbT1h4/`, both under exploration directory.

### Task: adopt deferred FTS and read policy

Status: complete; normal native implementation updated. No launcher change or commit.

- User approves recommended lazy FTS/reduced routine checks/mmap, specifying full indexed rebuild on first search rather than late population. Preserve enabled mode afterward; do not invalidate/drop indexes on later edits.
- Cold layout `3`, exact metadata `fts_enabled=0`, no FTS objects. First search reuses loaded source for full private indexed build, validation/checkpoint/close/publication. Indexed layout `2`, flag `1`; legacy layout 2 without flag inferred enabled. Legacy `.mjs` rebuilds cold format safely, native reuses indexed legacy format without writing. Source/JSON contracts unchanged; cold schema describes declared FTS columns.
- Shared opens request 64 MiB mmap. Full builds run optimize/integrity/snapshot checks before publication; doctor retains immutable inspection. Remove per-preparation quick_check; keep full source hashing/parsing/validation and exact value checks. Default WAL/NORMAL/checkpoint/busy policy unchanged. First-search replacement retains active-consumer refusal; subsequent enabled deltas coexist with WAL readers.
- Both compilers/configs pass source/read, 34 lifecycle, 11 incremental, 19 CLI + 18 rejected batches and 9 mode cases. Sentinel proves first full replacement and later incremental generation retention. Mid-construction failure leaves old cache intact and removes owned temporary state. Independent source review found no actionable adoption issue within inspected scope. Ten harness + seven runner tests pass.
- Main current CJIT/Zig/Node + matched late/OFF comparisons pass 2376 measured + 264 warmups; first-search post-commit-validation focus passes 216 + 24. All exit 0/no stderr, captures unchanged. Zig NORMAL proto fresh/query/search medians 55.3/38.9/69.4 ms; godot 61.0/32.7/40.2 ms.
- Correction: earlier bare-build versus search-plus-population timing did not establish full rebuild faster. Final matched first-search NORMAL full 330.3/132.2 ms versus late 309.9/121.2 ms. Full rebuild remains requested policy; late comparison stays experimental. Mode flag persists under both; later edits no longer trigger repeated FTS population.
- Earlier experiment inputs frozen under `fixtures/sqlite-experiment-base/`, avoiding silent redefinition of historical variants. Detailed behavior, profiles, limits, numbers and evidence: `exploration/attendant-native/SQLITE-ADOPTION.md`. Main raw `build/bench/run-fzhcMF/`; focus `build/bench/run-hBsV60/`, under exploration. WAL/OFF restart/corruption policy and broader operational release acceptance still pending.

### Remaining refinement and parity work

Current review direction:
- Leave native launcher/compiler discovery and skill integration until last.
- Current native/CJIT/Node and first-search strategy comparisons complete; retain workload/variance evidence in adoption report. Further tuning requires measured benefit; do not infer full-rebuild speed from bare-build versus first-search totals.
- Native writers now target WAL/NORMAL with bounded lock waits and unchanged default checkpoint policy. Preserve temporary publication for full rebuilds and safety around live readers/sidecars; measure actual workload effects. Details and limits belong to incremental task above.
- Keep current explicit capacities as starting limits. Growable arenas or expanding allocators remain possible later work, not a blocker for current datasets.
- Defer query hints. Run full representative-data acceptance near end of review, before launcher rollout.
- Use isolated copies of active `D:/Repos/proto-framework` and `D:/Repos/godot-proto` datasets for analysis; never run correction/preparation in originals. Capture source config and collection Markdown, not existing `.attendant/` projections. Keep pristine references separate from disposable runner workspaces.

Dataset capture: `exploration/attendant-native/capture-datasets.mjs`; shared read-only inventory/containment checks in `dataset-files.mjs`. Commands/evidence in exploration README. Initial captured datasets contain 127 and 60 records. Both native compilers and `.mjs` pass strict validation without diagnostics or pending corrections. Full real-data acceptance remains pending.

Historical pre-incremental performance baseline: `exploration/attendant-native/benchmark.mjs`; method/results in `BENCHMARK.md` beside it. Both datasets × `.mjs`/CJIT/Zig × six scenarios passed 720 measured samples plus 108 separate warmups. External wall clock includes process/CLI startup through exit/output capture; excludes native compilation, fixture resets, edits and result checks. No `bench.h` needed for this boundary. Zig now defaults to `zig cc -O3` for all sources, including SQLite; no fast-math or explicit assertion removal added. Optimized rerun beats CJIT in all tested cases and `.mjs` on fresh/unchanged commands; larger dataset's changed-source rebuilds remain slower than `.mjs`. Optimized median CLI times are 1.95–3.18 times faster than initial unoptimized Zig run; separate shared-machine runs limit exact causal attribution. Record counts, query/search parity, projected/cache hashes, cache mtimes/sizes, FTS edits and source/pristine preservation passed. Optimized Zig passes source, projection/read, preparation and CLI suites in both config modes; eight harness tests and seven existing runner tests pass. Independent review drove cached-mtime regression. Optimized raw evidence: `exploration/attendant-native/build/bench/run-U3wwg9/`; original unoptimized evidence preserved at `build/bench/run-FRyLLI/` under same exploration. Those baseline runs predate incremental/WAL task above; launcher remains deferred.

- [x] Create isolated discussion worktree: `D:/Repos/agent-skills-attendant-native`, branch `discuss/attendant-native`.
- [x] Capture motivation, goals, and anti-goals in this file.
- [x] Define first-release audience and delivery expectation: user first; broader users need a discoverable C99 compilation path, with skill reporting when none is available.
- [ ] Define how skill detects and reports missing compiler or required dependencies on Windows. Prefer `cjit`; fall back to `zig cc` if CJIT is unavailable or cannot build Attendant. SQLite source is vendored; exact checks and other dependency packaging remain open.
- [ ] Define observable benefits separately for Node removal, command responsiveness, and possible visual companion. Measure responsiveness against Node without a fixed speedup requirement; workloads and visual outcomes remain open.
- [x] Inventory current Attendant commands, behavior, dependencies, and shipped tests; verify CLI help and representative outputs.
- [x] Review capability removal: exclude migration/import/schema-transform commands from native CLI; keep other capabilities under review.
- [x] Decide inspection correction direction: keep automatic repair and expose applied changes.
- [x] Set repair boundary: best-effort automatic correction and visible reports; authors own source correctness. Ignore and report malformed YAML. Duplicate ordering is a practical heuristic, not a guarantee of author intent.
- [x] Establish Git boundary: no Git awareness or invocation for now; human/agent own Git operations and recovery. Worktree context and non-Git warnings are possible future exploration.
- [x] Capture current dataset source and disposable projection regeneration at broad-strokes level; configured external paths and filesystem-derived metadata limit exact portability.
- [ ] Refine human and agent CLI contract against `cli-design`: output defaults/flags, field selection, errors, warnings, agent mode, exit status, and applicable breadcrumbs.
- [x] Define native prototype error/diagnostic shapes, empty-result metadata and limit semantics. See exploration README; final product contract remains reviewable.
- [ ] Assess body read/edit and stale-edit conflict detection as possible initial core capabilities.
- [ ] Define bounded resource use and lock waits, including future CLI/GUI coexistence.
- [x] Decide migration responsibility: external workflow, not native CLI.
- [x] Define write-failure reporting/recovery without tool-managed Git: atomic per-file replacements, no cross-file update transaction, structured failed/written paths, source remains authoritative after projection-refresh failure.
- [ ] Sketch visual collaboration loops: agent shows intended edit locations; human selects context; agent retrieves selection through CLI. Exploratory only, no build commitment.
- [ ] Define selection granularity and addressing: records, fields, body sections, or text ranges; alternatives, not agreed requirements.
- [ ] Define CLI/viewer communication, session ownership, stale selections, and behavior when viewer is absent.
- [ ] Separate navigation, selection, proposed changes, and explicit approval; no implicit edit consent.
- [ ] Expand compatibility fixtures for YAML preservation, Unicode, references, hidden/invalid records, concurrency, and failure paths.
- [ ] Locate and inspect existing C99 + CJIT tool in another worktree; record reusable patterns and constraints.
- [ ] Assess C99 + CJIT support for current Attendant needs, including SQLite, YAML, Markdown, filesystem operations, and cross-platform execution. Evaluate C YAML candidates and source-preserving edits against real fixtures; add a Markdown parser only if behavior requires it.
- [ ] Identify compatibility requirements and any proposed behavior changes.
- [ ] Outline CLI/core boundaries that leave room for a future raylib/cimgui viewer/editor.
- [ ] Compare C99 + CJIT with keeping Node.js; consider Zig only where relevant to a concrete constraint.
- [ ] Define measurements for startup time, footprint, and runtime requirements.
- [ ] Identify replacement rollout, data safety, recovery, and test requirements; no CLI migration machinery.
- [x] Exercise source-preserving feasibility within user-approved CLI completion slice: bounded YAML splices, exact body/unrelated-comment checks, Unicode semantic readback and locked-file failure regressions.
- [x] Select first implementation slice: native projection generation consumed by existing `.mjs` CLI, with explicit user review before further scope.

## Comments

- Current implementation: `skills/attendant/scripts/attendant.mjs` and `yaml.mjs`; tests: `skills/attendant/test/attendant.test.js`.
- Current documented runtime requirement: Node.js `>=24.10.0`.
- Local shell finds CJIT v1.2.0 and Zig 0.17.0. `D:/Repos/frametime-matters-website/tools/performance-faker/` documents both paths; its dependency and build setup is not yet assessed for Attendant.
