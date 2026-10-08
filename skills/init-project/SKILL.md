---
name: init-project
description: "Initialize project, set up an empty repo, create AGENTS.md, or bootstrap project docs and Attendant records. User-run setup that identifies whether implementation exists, interviews the user about purpose and likely tools, languages, frameworks, and formats for empty projects, then creates or carefully merges project guidance with unresolved facts left TBD."
disable-model-invocation: true
---

# Init Project

Set up high-signal project guidance plus Attendant-backed records.

## Rules

- Do not scaffold app code, install deps, or change build config unless user asks.
- Preserve existing docs; merge carefully or ask before reorganizing them.
- Adapt templates to inspected facts and user answers. Use concise, ordered, structured documentation; lead with the result and important technical facts. Omit irrelevant headings and empty placeholders; retain material unknowns as explicit `TBD` or open questions. Distinguish planned choices from installed tools or verified behavior.
- Ask one focused question at a time. Reuse facts already supplied, record each answer in working setup notes, and reassess before asking the next dependent question. Accept “unknown” or “not decided” without pressing for a choice.
- Treat `README.md` as public-facing and human-first: explain the project’s purpose, installation, and use. Keep current work, internal status, agent workflow, and detailed contributor setup out of it.
- Keep the README developer setup brief; put longer contributor/development instructions in a focused document such as `CONTRIBUTING.md`.
- If existing `todo/`, `docs/decisions/`, or similar record folders exist, route to `/skill:attendant` Markdown-migration workflow; never replace them with blank collections.
- Skip `.pi/handoff.md` when no real handoff/status content exists.

## Default outputs

- `AGENTS.md` — local agent instructions.
- `.pi/attendant.tables` — default collections configuration.
- `records/tasks/.schema.md` — canonical task tracker schema.
- `records/tasks/.usage.md` — task lifecycle, readiness, approval, and operating rules.
- `records/tasks/.template.md` — starting body for new task records.
- `records/decisions/.schema.md` — canonical durable decisions schema.
- `records/decisions/.usage.md` — decision purpose, approval, supersession, and revisit rules.
- `records/decisions/.template.md` — starting body for new decision records.
- `.gitignore` entry `.attendant/` — generated local state.
- `.pi/handoff.md` — optional agent pickup summary.
- `CHANGELOG.md`, `README.md` — human-facing docs.

## Workflow

1. Find project root and inspect it immediately, including hidden files. Use the target cwd unless another path is supplied; a Git repository is not required.
   - Done when target directory is explicit and inspected.
2. Inventory docs and record sources: `AGENTS.md`, `.pi/attendant.tables`, `.gitignore`, `records/`, `todo/`, `docs/decisions/`, `CHANGELOG.md`, `README.md`. Inspect source/assets, manifests, configuration, and Git status when available; do not treat a Git error as proof of emptiness.
   - Classify as **empty/unimplemented** when no substantive project implementation or content exists. `.git/`, editor settings, starter docs, licenses, ignore files, and empty directories alone do not make a project implemented. A manifest alone can indicate an intended stack, not a working app.
   - Code, meaningful assets, datasets, or authored content count as implementation; projects need not contain application code. Exclude generated/vendor files from this judgment. If starter files make the classification materially uncertain, ask whether they are the project or just scaffolding.
   - Done when classification, existing/missing docs, and migration candidates are grounded in inspected files.
3. For an empty/unimplemented project, follow the interview below **before generating docs**. For an implemented project, derive facts from files and ask only for missing facts that affect guidance.
   - Ground README in public purpose, installation, usage, and brief developer setup. Keep current work in records or a real handoff; put lengthy contributor instructions in a focused document.
   - Done when purpose and likely stack are captured, with unresolved details explicitly `TBD`.
4. If legacy record folders exist, summarize migration fit and ask whether to use `/skill:attendant` migration, add only non-record docs, or skip setup. Never create a default collection over a migration candidate.
   - Done when migration policy is chosen.
5. If target docs/config already exist, ask whether to merge templates, add only missing pieces, or skip existing files.
   - Done when merge policy is chosen.
6. Read matching templates and generate/adapt files using gathered facts. Keep unknown commands, paths, constraints, and acceptance details `TBD`; never invent install/test commands from a likely stack. Label unimplemented features and tentative tools as planned, not available or verified. Do not claim a release, versioning policy, or completed change without evidence; retain an unreleased changelog rather than copying the example release.
   - For new default collections, create configured directories, schemas, usage guidance, and body templates, add `.attendant/` ignore entry, then use `/skill:attendant` runner: `validate --no-correct` and `sync`. Run `doctor` only for a reported health or projection problem.
   - Task templates/usage must specify technical contracts as well as their point: code/API tasks use declarations and caller examples; refactors use before/after code or artifacts; CLI/data/UI tasks use commands, records, or interaction/result examples. Each requirement has one home. Use one final Comments section for errata and incidental notes, not a separate Notes section; omit it until populated.
   - Adapt usage guidance to confirmed project policy and keep it consistent with each schema. Preserve existing collection guidance under the chosen merge policy; do not silently retrofit default lifecycle rules. Include configured schema and usage paths in `AGENTS.md` and require reading usage before operating on records.
   - Correct supported setup diagnostics without replacing records; rerun validation and sync only after validation passes. Stop with paths and a blocker when correction needs user decisions.
   - Done when generated docs match answers/evidence and collection setup validates, or an unresolved diagnostic is reported.
7. Review generated files against interview answers and inspected facts. Replace leftover template instructions with actual text or explicit material unknowns; remove empty placeholders, irrelevant headings, repeated facts, and unsupported claims. Verify that material unknowns remain visible. Correct and recheck before reporting completion.
   - Inspect Attendant `schema` output to confirm both new collections report their usage paths and full guidance. Review lifecycle values, approval gates, and references against the schemas; schema validation does not enforce usage policy. Correct inconsistencies and recheck, or report a blocker.
   - Done when docs distinguish confirmed facts, planned choices, and unknowns without presenting an empty project as runnable, and collection usage matches the schemas and confirmed policy.
8. Report created/updated paths, skipped/migration items, and remaining TBD fields.
   - Done when user can resume cleanly.

## Empty-project interview

Walk the user through these topics in order, one focused question per turn. Skip answered topics; split a topic into follow-ups only when its answer leaves a material gap. Use the harness's structured question tool when available, with custom answers allowed; use plain chat for substantial free-text explanations.

1. **Purpose:** What is this repository for, and who will use it? Capture project name if not supplied or evident.
2. **Project shape:** What should it contain or produce (for example an app, library, CLI, game, documents, assets, or data)? Ask only if purpose did not establish this.
3. **Languages and formats:** Which languages or file/data formats might it need? “Undecided” is valid; do not default every project to a programming language.
4. **Tools and frameworks:** Which tools, runtimes, frameworks, or build systems are expected? Distinguish firm choices from possibilities; do not choose a stack on the user's behalf.
5. **Constraints:** Ask about target platforms, integrations, or special data/security requirements only when relevant and still unknown.

Capture supplied setup/use/check commands, but do not require the user to design commands for an unimplemented project. Summarize captured purpose, planned stack, and unknowns briefly, then continue generation without an extra approval gate. Remaining applicable template fields stay `TBD`; the interview does not authorize code scaffolding, dependency installation, or build configuration changes.

Template map:
- `templates/AGENTS.md` → `AGENTS.md`
- `templates/.pi/attendant.tables` → `.pi/attendant.tables`
- `templates/records/tasks/.schema.md` → `records/tasks/.schema.md`
- `templates/records/tasks/.usage.md` → `records/tasks/.usage.md`
- `templates/records/tasks/.template.md` → `records/tasks/.template.md`
- `templates/records/decisions/.schema.md` → `records/decisions/.schema.md`
- `templates/records/decisions/.usage.md` → `records/decisions/.usage.md`
- `templates/records/decisions/.template.md` → `records/decisions/.template.md`
- `templates/.pi/handoff.md` → `.pi/handoff.md`
- `templates/CHANGELOG.md` → `CHANGELOG.md`
- `templates/README.md` → `README.md` (public purpose, installation, usage, brief developer setup)
