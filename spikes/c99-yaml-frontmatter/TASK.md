# Spike: C99 YAML front matter

## Outcome

Test whether C YAML libraries can read and safely update Attendant Markdown front matter on Windows. Produce reproducible probes and evidence, not a production parser or a library adoption decision.

Compare [libfyaml](https://github.com/pantoniou/libfyaml) and [LibYAML](https://github.com/yaml/libyaml) where feasible. If a candidate cannot build or cannot support a required check, report the blocker and continue with the other candidate. Do not substitute an untested conclusion for a failed build.

## Context

- Attendant's current runner is `skills/attendant/scripts/attendant.mjs`; its vendored YAML library is `skills/attendant/scripts/yaml.mjs`.
- Current runner finds front matter delimiters, parses YAML as a document, changes fields, serializes front matter, and retains the Markdown body as text. See `parseRecord`, `buildText`, and validation code. It does not establish a need for a Markdown parser.
- Target Windows first. Both `cjit` and `zig cc` are intended compiler paths. Prefer CJIT for normal use, with Zig as fallback. Do not assume both work without testing.
- `exploration/attendant-native/include/` may contain copies of `base.h`, `build.h`, and `test.h`. If absent in the worker branch, copy them from `D:/Repos/proto-framework/include/` and record source revision. These are optional helpers; `build.h` itself bootstraps through CJIT, so it does not prove a Zig-only path.
- Example real records are under `D:/Repos/proto-framework/records/tasks/` and `D:/Repos/proto-framework/records/decisions/`. Use copies or read-only inputs; never modify source records. The worker branch may not contain the discussion record or copied headers if they are untracked in the parent worktree.
- Prior investigation: libfyaml documents experimental opt-in comment parsing/emission with best-effort placement. Verify behavior rather than assume preservation. Vendoring SQLite is a separate decision and outside this spike.

## Work

1. Inspect current parsing, update, and validation behavior and existing Attendant tests. Identify the supported front matter subset and use current runner as a comparison where practical. Record any differences between current behavior and desired source preservation; do not present desired behavior as already guaranteed.
2. Select representative real Attendant records from local repos, plus small synthetic fixtures for cases real records do not cover. Copy only non-sensitive examples into the spike; otherwise retain paths and generate safe fixtures. Include records with scalar values, sequences, comments, Unicode, and Markdown bodies, where available.
3. Probe candidates for:
   - Delimiters and mapping values: strings, booleans, null, numeric values, dates/datetimes as applicable, arrays, quoted values, malformed YAML, and duplicate keys. Compare semantic results or diagnostics with current behavior.
   - A targeted metadata change and insertion. Check comments and formatting before and after; check LF and CRLF, and compare body bytes exactly. Check empty and missing front matter and input without a trailing newline.
   - Windows build/run with CJIT and `zig cc` independently. Record exact commands, versions, dependencies, relevant compile flags, and output. If compilation fails, capture the decisive error and distinguish compiler, library, and dependency failures.
4. Explore a source-preserving edit approach if a parser's emitter changes unrelated text. Do not claim byte preservation without testing. Document unsupported or ambiguous YAML cases and whether the approach rejects them safely. No production implementation or automatic changes to real records.
5. Save reproducible probes, fixtures, commands, and a concise results report under `spikes/c99-yaml-frontmatter/`. Record candidate revisions, licenses and provenance; preserve required notices for any copied third-party files. Do not install tools or dependencies automatically.

## Acceptance

- Report identifies tested candidates and versions, local compiler versions, sample sources, and exact commands to repeat each check.
- Results distinguish observed pass, fail, and untested; list blockers and shortest useful error. Describe comparison to current runner without claiming full YAML compatibility.
- At least one real copied record and synthetic edge cases are exercised, or the report explains why a case or candidate could not be exercised. No source record is modified.
- For each tested edit, report whether semantic values, comments, unrelated formatting, newline style, and body bytes were preserved. Include a failing or unsupported case when found.
- Report recommends whether either candidate merits further work, with evidence and unresolved compatibility or licensing risks. No adoption decision, rewrite, CLI change, or Markdown parser requirement is implied.
- Changes stay in the spike folder, except copying the three optional reusable headers into `exploration/attendant-native/include/` if missing. Do not commit, merge, or alter the parent worktree without explicit approval.
