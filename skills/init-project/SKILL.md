---
name: init-project
description: "Initialize project, set up project docs, bootstrap Attendant tasks and decisions, create AGENTS.md, or add handoff/status docs. User-run setup that inspects existing docs and repo stack, then creates or carefully merges concise project-local guidance and default Attendant record collections."
disable-model-invocation: true
---

# Init Project

Set up high-signal project guidance plus Attendant-backed records.

## Rules

- Do not scaffold app code, install deps, or change build config unless user asks.
- Preserve existing docs; merge carefully or ask before reorganizing them.
- Adapt templates to repo facts; remove irrelevant sections; mark unknowns `TBD`.
- Treat `README.md` as public-facing and human-first: explain the project’s purpose, installation, and use. Keep current work, internal status, agent workflow, and detailed contributor setup out of it.
- Keep the README developer setup brief; put longer contributor/development instructions in a focused document such as `CONTRIBUTING.md`.
- If existing `todo/`, `docs/decisions/`, or similar record folders exist, route to `/skill:attendant` Markdown-migration workflow; never replace them with blank collections.
- Skip `.pi/handoff.md` when no real handoff/status content exists.

## Default outputs

- `AGENTS.md` — local agent instructions.
- `.pi/attendant.tables` — default collections configuration.
- `records/tasks/.schema.md` — canonical task tracker schema.
- `records/tasks/.template.md` — starting body for new task records.
- `records/decisions/.schema.md` — canonical durable decisions schema.
- `records/decisions/.template.md` — starting body for new decision records.
- `.gitignore` entry `.attendant/` — generated local state.
- `.pi/handoff.md` — optional agent pickup summary.
- `CHANGELOG.md`, `README.md` — human-facing docs.

## Workflow

1. Find project root.
   - Done when target repo/cwd is explicit.
2. Inventory target docs and record sources: `AGENTS.md`, `.pi/attendant.tables`, `.gitignore`, `records/`, `todo/`, `docs/decisions/`, `CHANGELOG.md`, `README.md`.
   - Done when existing, missing, and migration candidates are known.
3. Inspect high-signal files, Git status, and recent files.
   - Ground the README only in durable public facts: project purpose, installation, usage, and a concise developer setup. Put current work and internal status in Attendant records or `.pi/handoff.md`; create or extend a focused contributor document when developer setup would be more than a short section.
   - Done when purpose, stack, commands, current work, and their appropriate document homes are grounded or marked unknown.
4. If legacy record folders exist, summarize migration fit and ask whether to use migration workflow, add only non-record docs, or skip setup. Do not create default collection over a migration candidate.
   - Done when migration/merge policy is chosen.
5. If target docs/config already exist, ask whether to merge templates, add only missing pieces, or skip existing files.
   - Done when merge policy is chosen.
6. Read matching templates, adapt wording, create missing files. For new default collections, create configured directories, schemas, and body templates, add `.attendant/` ignore entry, then use `/skill:attendant` runner: `validate --no-correct` and `sync`. Run `doctor` only when setup reports a health or projection problem.
   - Done when collection setup validates or diagnostic is reported.
7. Ask only for missing facts that change human docs: project purpose, install/use commands, and concise developer setup. Keep current goal and acceptance check in agent records rather than the README.
   - Done when unknowns can safely remain `TBD`.
8. Final output: created/updated paths, skipped/migration items, TBD fields.
   - Done when user can resume cleanly.

Template map:
- `templates/AGENTS.md` → `AGENTS.md`
- `templates/.pi/attendant.tables` → `.pi/attendant.tables`
- `templates/records/tasks/.schema.md` → `records/tasks/.schema.md`
- `templates/records/tasks/.template.md` → `records/tasks/.template.md`
- `templates/records/decisions/.schema.md` → `records/decisions/.schema.md`
- `templates/records/decisions/.template.md` → `records/decisions/.template.md`
- `templates/.pi/handoff.md` → `.pi/handoff.md`
- `templates/CHANGELOG.md` → `CHANGELOG.md`
- `templates/README.md` → `README.md` (public purpose, installation, usage, brief developer setup)
