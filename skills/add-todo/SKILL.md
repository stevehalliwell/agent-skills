---
name: add-todo
description: "Add todo, capture task, create task, backlog item, track this, remember this work, defer idea, future work, or make project TODO when user wants project-local work recorded. Goal: create or update one Attendant task record without inventing task shape; leave unresolved items in needs-refinement and use task-refinement when implementation shape needs agreement."
---

# Add Todo

Capture one task. Attendant Markdown record is source of truth.

## Required read

Load [Attendant](../attendant/SKILL.md) before collection or record operations. When `tasks` is configured, load [Task lifecycle](../task-lifecycle/SKILL.md) and read the project's configured schema before drafting. Read the [default configuration](../init-project/templates/.pi/attendant.tables), [task schema](../init-project/templates/records/tasks/.schema.md), and [task body template](../init-project/templates/records/tasks/.template.md) only when bootstrapping default task storage; existing project schemas remain authoritative.

## Workflow

1. Anchor current work.
   - Say: `Todo capture start. Current work resumes after record capture.`
   - Done when pause point is explicit.

2. Find or bootstrap task collection.
   - Prefer Git root; else cwd.
   - If `.pi/attendant.tables` is absent, inspect existing record folders first. Route legacy task or decision records to Attendant's guarded migration workflow; do not create blank default collections over existing records. For a fresh project with no migration candidates, use [Init Project](../init-project/SKILL.md) to bootstrap the default task storage while preserving existing docs.
   - If config exists but `tasks` is absent, use `/skill:attendant` empty-collection workflow; do not invent a second tracker.
   - Follow the chosen setup operation's validation contract; ordinary Attendant operations prepare the projection. Resolve reported diagnostics and recheck, or stop with the affected path and blocker.
   - Done when valid `tasks` collection exists.

3. Find duplicate records.
   - Use `/skill:attendant` `search` for title/key terms. Use its `query` command for status/priority filtering when needed.
   - If same item exists, update it. If similar item exists, ask whether to merge, link dependency, or keep separate.
   - Done when record target and duplicate handling are clear.

4. Choose capture depth.
   - Read `tasks/.schema.md` first; it is authoritative. Capture mode records smallest faithful description in its declared capture state (recommended `needs-refinement`), and uses `priority: medium` only when `priority` is declared; preserve unknowns as `TBD`.
   - Refinement mode: when user asks to make work implementation-ready, use `task-refinement`; move it to its declared ready state (recommended `todo`) when its outcome and acceptance are sufficiently clear and material decisions are resolved through user input.
   - Future/unrelated work is normal capture mode. Use `priority: delay` only when that field and value are declared and the user explicitly wants deprioritisation.
   - Done when authority and declared status/priority fields are explicit.

5. Create or update record.
   - New record: use `/skill:attendant` `create -c tasks -i <items-json>` with one `{ "name": "<safe-slug>", "fields": <declared-fields> }` item; then replace copied template body with content populated by known facts.
   - Existing record: edit its source path directly; preserve confirmed decisions and prior checks.
   - Read the saved source record and compare its fields/body with the request. Correct inaccurate or missing captured detail and recheck; report a blocker rather than claiming a failed save.
   - Done when the saved source record accurately captures the request.

6. Finish.
   - If Git repo, show diff for changed record and setup files.
   - Do not commit unless explicitly asked.

```text
Todo saved: <records/tasks/slug.md>
Status: <status>; priority: <priority, if declared>
Next: task or resume prior task
```

## Rules

- One record per run unless user asks for more.
- Record existence does not authorize implementation.
- Keep product decisions confirmed; proposals and unknowns remain open.
- No secrets, tokens, private env values, or chat dumps.
