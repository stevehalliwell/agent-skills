# Changelog

## [Unreleased]

### Added

- Native Attendant GUI: default-on raw **Wrap** toggle, context-aware Ctrl+F for file/source/preview panes, and bounded recursive Markdown tree with explicit incomplete-discovery warnings. Windows CJIT/Zig checks pass; usability changes await macOS validation.
- Native GUI follow-up: compact single-child file paths, Ctrl+F-only sidebar filter, draggable pane widths, guarded Close All, and explicit CLI `--find` for existing viewers without source writes or OS focus stealing.

### Changed

- Temporarily disable recursive workspace discovery on open/Refresh files. Show root Markdown plus configured Attendant files only; shared `base.h` retains recursive search and adds explicit warning-free `root_only` scope.
- Native filesystem helpers now share one bounded recursive walker for strict tree operations and glob-filtered search such as `*.md`. Attendant discovery uses shared search; duplicate application walker/platform directory iterators removed. Windows passes `*.md` to native search; separate match/directory limits prevent unrelated cache files from exhausting Markdown results.
- **Breaking:** Attendant skill now invokes native CLI on Windows x64/macOS arm64. Build with installed CJIT or Zig when missing; no JS fallback or Node.js runtime for record operations. Native GUI supports source editing, human feedback and reveal/inspection controls.
- Attendant command/setup guidance and `init-project` now match native behavior, including reported mechanical corrections during schema/query/search and partial-write handling.
- Replaced remaining Attendant JS preparation/patch/manifest, GUI/control/bridge and benchmark tools with C/Bash entry points. Windows CJIT/Zig checks pass; new macOS replacements need user compilation/runtime validation. See Attendant build reference for commands and helper-compiler selection.

### Removed

- Removed all nine remaining maintained Attendant `.mjs` tools/tests and their Node invocations; no Attendant runtime/build/test/benchmark Node dependency.
- Retired Attendant JS runtime, vendored JS YAML parser, transition tests and migration-only guidance. Native has no migration/import command; existing record folders remain untouched. Former JS version remains linked in native README through Git history.

## [0.2.0]

### Added

- `agent-browser` for shared browser workflows, `cli-design` for human and machine interfaces, `gltf-assets` for GLB/glTF inspection and optimization, and `session-skill-invocations` for saved Pi session reports.
- `herdr`, `herdr-parallel-work`, and `herdr-docker-agents` for pane control, coordinated worktree-isolated implementation, and isolated container agents.
- `spike-comparison` for comparative technical spikes with offline HTML reports.
- `systems-coding` for low-level engineering guidance, `itch-page` for draft project pages, and `offline-html-report` for self-contained reports with bundled Chart.js.
- `refine-the-backlog` for queue-wide refinement, separate from single-task `task-refinement`.
- Attendant batch metadata updates and collection `.usage.md` discovery through `schema`; default project templates now include task and decision usage guidance.

### Changed

- **Breaking:** Attendant `create` now requires `--items`/`-i` containing an array for both single and batch creation; `--name`/`-n` and `--fields`/`-f` are no longer supported.
- `delegate-tasks` uses Herdr for independent asides when available; coordinated implementation uses `herdr-parallel-work`. The job runner exposes the signed-in Codex model catalogue.
- Project and task workflows read collection usage guidance instead of assuming a shared lifecycle. `init-project` interviews users before documenting empty projects; `implementation` lists ready tasks when no focus is supplied.
- `backlog-capture` supports batch capture, duplicate checks, and project-local `TODO.md` fallback when Attendant tasks are not configured.
- YouTube transcript downloads reject invalid video IDs, empty transcripts, language mismatches, and existing output files.

### Fixed

- Delegated background jobs record actual exit status and require non-empty results after successful exit; cancellation checks recorded process identities.
- README skill index links to current skills and includes invocation guidance.

### Removed

- `add-todo`, consolidated into `backlog-capture`.
- `backlog-refinement`, replaced by `refine-the-backlog`.
- `task-lifecycle`; collection-specific lifecycle rules now live in `.usage.md`.
- `threejs-performance`; `gltf-assets` covers asset-file work, not general Three.js scene performance.

## [0.1.0]

### Added

- Initial public release of 38 Pi-compatible agent skills.
