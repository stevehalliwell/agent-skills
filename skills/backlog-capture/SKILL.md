---
name: backlog-capture
description: "Capture backlog ideas, todo items, future work, or deferred requests quickly with only detail needed for later refinement. Use when user wants to add, note, record, or batch-enter backlog items; skip when they want scope, prioritization, or implementation-ready tasks."
---

# Backlog capture

Record faithful draft items. Refinement happens later.

## Workflow

1. Anchor capture. Say: `Backlog capture start. Current work resumes after capture.` Identify each distinct requested item. Done when capture target is clear.
2. Check task storage and duplicates. If Pi Attendant is available and configures `tasks`, read `tasks/.schema.md`, then search matching titles or key terms. Update exact duplicates. Ask whether to merge only for materially similar items. Done when each item has one record target.
3. Record minimum useful detail. When Pi Attendant is available, create each new item with its entry-creation operation in `tasks`. Use `status: needs-refinement` when declared; otherwise use legacy `status: todo` plus `scope: draft` when both fields and values are declared. Set `priority: medium` only when declared. When no task storage is configured, ask for a destination before writing. Keep supplied outcome, customer or business value, context, and constraints; mark unknowns `TBD`. Done when later refinement can identify the idea without reconstructing chat.
4. Preserve capture boundary. Defer scope, acceptance, priority, sequencing, research, splitting, and implementation to backlog refinement unless user explicitly requests them. Done when no product decision was inferred.
5. Exit. Say: `Backlog captured: <record paths>. Next: refine an item or resume prior work.` Done when saved records and next action are clear.

## Rules

- Ask only for information required to distinguish or faithfully record an item.
- Keep replies brief.
- Record existence does not authorize implementation.
- One user request may create multiple distinct draft records.
