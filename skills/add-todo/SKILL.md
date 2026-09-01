---
name: add-todo
description: "Add todo, capture task, create task, backlog item, track this, remember this work, defer idea, future work, or make project TODO when user wants project-local work recorded. Goal: create or update one Attendant task record without inventing task shape; leave unresolved items in needs-refinement and use task-refinement when implementation shape needs agreement."
---

# Add Todo

Capture one task. Attendant Markdown record is source of truth.

## Required read

Load before drafting:
- `../init-project/templates/.pi/attendant.tables`
- `../init-project/templates/records/tasks/.schema.md`
- `../init-project/templates/records/tasks/.template.md`

## Workflow

1. Anchor current work.
   - Say: `Todo capture start. Current work resumes after record capture.`
   - Done when pause point is explicit.

2. Find or bootstrap task collection.
   - Prefer Git root; else cwd.
   - If `.pi/attendant.tables` is absent, create `.pi/attendant.tables`, collection `.schema.md` files, and collection `.template.md` files from required templates; add `.attendant/` to `.gitignore` when missing.
   - If config exists but `tasks` is absent, use `/skill:attendant` empty-collection workflow; do not invent a second tracker.
   - Run `node <attendant-skill-dir>/scripts/attendant.mjs validate --no-correct`, then `node <attendant-skill-dir>/scripts/attendant.mjs sync`; stop on diagnostics.
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
   - New record: use `/skill:attendant` `create -c tasks -n <safe-slug> -f <declared-fields>`; then replace copied template body with content populated by known facts.
   - Existing record: edit its source path directly; preserve confirmed decisions and prior checks.
   - Done when source record accurately captures request.

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
