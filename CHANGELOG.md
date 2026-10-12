# Changelog

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
