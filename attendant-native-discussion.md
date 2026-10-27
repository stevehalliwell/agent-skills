# Attendant native discussion

Explore moving Attendant from Node.js to C99 + CJIT. No rewrite decision yet.

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
- Completed YAML spike: `spikes/c99-yaml-frontmatter/RESULTS.md`, probes, fixture and raw results retained here. Parent independently reproduced LibYAML builds and identical probe output with CJIT v1.2.0 and Zig 0.17.0. LibYAML can parse and emit tested mappings but drops comments, changes formatting, accepts duplicate keys, and needs explicit Attendant subset and scalar resolution logic. It is not a drop-in source-preserving editor. libfyaml runtime behavior remains untested: direct compilation hit missing `xxhash.h`; supported CMake build was not attempted. A narrow JS splice preserves simple fixtures but rejects the real flow-sequence example; it is not a native editing solution. Current runner-equivalent serialization also changes some formatting and produces a mixed CRLF closing delimiter. No parser adopted; next YAML evidence needed is a supported libfyaml build or a deliberately scoped source-preserving native edit experiment.
- Copied `base.h`, `build.h`, and `test.h` from `D:/Repos/proto-framework/include/` at commit `f0b848e2541903ac99696f1f8a71d93e72ef4c5b` into `exploration/attendant-native/include/`. `base.h` supplies allocators, strings, collections, file and process helpers; `build.h` supplies a bounded CJIT/Zig build driver; `test.h` supplies assertions and runner generation. These are exploration dependencies, not an approved native implementation. `base.h` collection macros require `__typeof__`; `build.h` bootstraps via CJIT even when targeting Zig, so it does not by itself satisfy a Zig-only fallback.
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

### Remaining discussion findings

- Inspection can write source: `schema`, `query`, and `search` prepare project, applying missing/default metadata and resolving duplicate names/IDs when projection is not fresh. These corrections are not included in those commands' returned results.
- Success is one JSON value; operation failures generally emit text on stderr with exit 1 and no stdout JSON. Validation and projection diagnostics are embedded in successful JSON results. Non-strict validation can exit 0 with diagnostics.
- Query/search return `columns`, `rows`, `returned`, and `truncated`. Default limit is 100; shaping caps output at 10,000 rows and 10 MiB of serialized rows, but SQL/search results are fully materialized before shaping.
- Confirmed inconsistencies: `--limit 0` returns up to one row; empty query results lose column metadata; reused projection diagnostics use `source_path`/`collection_name` while rebuilt/updated diagnostics use `path`.
- Lock acquisition and some generated-file cleanup retries have no deadline. Future GUI alongside CLI makes bounded waiting and concurrent-write handling important.
- Stable machine-readable errors/diagnostics and visible automatic repairs are agreed direction; exact contract remains open. Possible additions: optional correction preview, body read/edit through shared core, and stale-edit conflict detection. These additions are not approved scope. Migration improvements are excluded from native CLI.

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

- [ ] Prepare isolated copied fixtures and expected results from existing Attendant; freeze source and record hashes after any fixture normalization.
- [ ] Trace the current projection contract and build a reference database for comparison; document required layout, hashes, paths, cache metadata, and FTS behavior.
- [ ] Establish reproducible Windows native builds with vendored SQLite, YAML reading, and required JSON support; verify CJIT and `zig cc` or report blockers.
- [ ] Implement full projection generation from frozen fixture source without editing Markdown. Match existing projection contract rather than redesign it.
- [ ] Run end-to-end acceptance: `.mjs sync` reuses native output; schema/query/search give expected results; frozen source stays unchanged. Compare native and runner-built logical results.
- [ ] Prepare concise review evidence and demo commands; report limitations and compiler-path results.
- [ ] User reviews the slice. Stop here pending review; do not proceed into writes, CLI replacement, or viewer work.

### Remaining refinement and parity work

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
- [ ] Define stable error/diagnostic shapes, empty-result metadata, and limit semantics.
- [ ] Assess body read/edit and stale-edit conflict detection as possible initial core capabilities.
- [ ] Define bounded resource use and lock waits, including future CLI/GUI coexistence.
- [x] Decide migration responsibility: external workflow, not native CLI.
- [ ] Define write-failure reporting and recovery expectations without tool-managed Git operations.
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
- [ ] Evaluate a source-preserving feasibility milestone before implementing writes; proposal, not agreed scope.
- [x] Select first implementation slice: native projection generation consumed by existing `.mjs` CLI, with explicit user review before further scope.

## Comments

- Current implementation: `skills/attendant/scripts/attendant.mjs` and `yaml.mjs`; tests: `skills/attendant/test/attendant.test.js`.
- Current documented runtime requirement: Node.js `>=24.10.0`.
- Local shell finds CJIT v1.2.0 and Zig 0.17.0. `D:/Repos/frametime-matters-website/tools/performance-faker/` documents both paths; its dependency and build setup is not yet assessed for Attendant.
