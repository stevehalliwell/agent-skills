---
name: backlog-capture
description: "Add a todo, capture a task or backlog idea, track this work, remember a deferred request, record future work, or batch-enter backlog items. Save one or several faithful draft items in configured Attendant tasks, otherwise project-local Markdown, without inventing scope or acceptance. Use refinement workflows for implementation-ready task shaping; skip backlog review, prioritization, task selection, and implementation."
---

# Backlog capture

Capture one or several faithful draft items. Refinement and implementation remain separate work.

## Workflow

1. Anchor capture. State: `Backlog capture start. Current work resumes after capture.` Identify each distinct requested item and supplied outcome, value, context, constraints, or acceptance. Ask only when ambiguity prevents distinguishing or faithfully recording an item. Done when the capture targets and return context are clear.
2. Resolve storage. Prefer the target project's Git root, otherwise its working directory. Inspect `.pi/attendant.tables` for configured `tasks`.
   - With `tasks`, load [Attendant](../attendant/SKILL.md) and [Task lifecycle](../task-lifecycle/SKILL.md), then read the configured collection's schema and body template when present. Existing project fields and values are authoritative.
   - Without configured `tasks`, use the supplied Markdown destination or the project's clearly established task document; otherwise use `<project-root>/TODO.md`. Read existing content before writing. Do not bootstrap Attendant, migrate records, or create a second tracker alongside configured tasks. If a configured Attendant operation fails, report its blocker rather than silently switching storage.
   - Done when the destination and its record conventions are known.
3. Check duplicates. With Attendant, search titles and key terms; use its query operation only for relevant declared fields. With Markdown, inspect existing entries. Update exact duplicates while preserving confirmed decisions, prior checks, and current state. Ask whether to merge, link, or keep separate only for materially similar items. Done when each requested item has one record target.
4. Capture minimum useful detail.
   - Preserve supplied facts and constraints; mark unknowns `TBD`. Do not invent implementation plans, scope, acceptance, ownership, or product decisions.
   - For new Attendant records, use `status: needs-refinement` when declared; otherwise use legacy `status: todo` plus `scope: draft` when both fields and values are declared. If neither representation is supported, mark the body as draft and use only permitted metadata; ask only when a required field cannot be filled faithfully. Use supplied priority only when permitted; default to `medium` only when declared. Use `delay` only when declared and explicitly requested as deprioritization. Existing records retain their state and priority unless the user requests a change.
   - Review the planned items and source paths before a validated create batch. Run Attendant's documented `create -c tasks -i @items.json` with one or several `{ "name": "<safe-slug>", "fields": <declared-fields> }` objects. Use `update` for existing record metadata and edit its source body for captured detail.
   - With Markdown, preserve the document's conventions and unrelated content. For a new document, use one heading per item with `Status: needs-refinement`, the supplied outcome/context, and open questions. Append new entries or update exact duplicates without rewriting the whole document.
   - Done when new items are saved as faithful drafts and existing items retain their state, or save failures are explicit.
5. Verify capture. Read every saved record or Markdown entry against the request. Correct missing or inaccurate detail and recheck; report unresolved write, schema, or content blockers and distinguish saved items from failed ones. Normal Attendant operations prepare the projection; do not add routine validation/sync churn. Done when saved paths and capture fidelity are confirmed.
6. Exit or hand off. State: `Backlog captured: <paths>. Next: <resume prior work or explicitly requested refinement>.` If the user also requested shaping the captured backlog, load [Refine the backlog](../refine-the-backlog/SKILL.md); for a specific task's executable technical shape, load [Task refinement](../task-refinement/SKILL.md). Capture alone does not authorize either refinement or implementation. Done when the capture mode is closed and the authorized continuation is clear.

## Rules

- One requested item normally creates one record; a batch request may create several distinct records.
- Preserve existing task state when adding capture detail; record existence does not establish readiness or authorize implementation.
- Keep replies brief: saved paths, meaningful updates, blockers, and continuation. Show the relevant diff when it helps verify an update.
- Keep secrets, tokens, private environment values, and chat dumps out of records.
- Do not commit unless explicitly requested.
