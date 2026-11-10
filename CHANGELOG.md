# Changelog

## [Unreleased]

### Changed

- **Breaking:** Attendant skill now invokes native CLI on Windows x64/macOS arm64. Build with installed CJIT or Zig when missing; no JS fallback or Node.js runtime for record operations. Native GUI supports source editing, human feedback and reveal/inspection controls.
- Attendant command/setup guidance and `init-project` now match native behavior, including reported mechanical corrections during schema/query/search and partial-write handling.

### Removed

- Retired Attendant JS runtime, vendored JS YAML parser, transition tests and migration-only guidance. Native has no migration/import command; existing record folders remain untouched. Former JS version remains linked in native README through Git history.

## [0.3.0]

### Added

- `yt-dlp` for video and audio downloads, format and metadata inspection, subtitle retrieval, and FFmpeg processing. Includes an audio-only download helper; speech recognition requires a separate tool.
- `github-image-attachments` for uploading local images into GitHub PRs, issues, and comments through GitHub CLI.
- `herdr-open-in-vscode` and `herdr-open-in-fork` for opening local files and repositories in their project context.
- `herdr-needs-attention` for keeping a pane blocked on an open question while user review, a decision, or external work is pending.
- Independent developer persona for explicitly delegated implementation in a dedicated branch and worktree, with authorized step commits and an offline decision report.

### Changed

- `persona` supports on-demand agent selection of useful perspectives without requiring user-role selection or a separate session setup. Task workflows and approval gates still apply.
- Herdr and Herdr-based delegation use labeled tabs for substantial work, work likely to need user help, or later inspection; small transient helpers may use split panes.
- Default task collection guidance limits active work to one task per working context, not one task across the entire project; independent contexts may work concurrently.
- `coding`, `implementation`, and `systems-coding` share proportionate verification guidance. Implementation checks acceptance explicitly and updates task records at meaningful checkpoints rather than after every feedback exchange.

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
