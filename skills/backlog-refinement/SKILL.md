---
name: backlog-refinement
description: "Refine backlog items, clarify unready tasks, flesh out a todo, scope work, define acceptance, split an oversized item, or make backlog work implementation-ready. Process oldest unready item one at a time, persist each refinement as it is resolved, make it ready once material decisions are resolved, then continue; do not implement work or make unrequested workflow changes."
---

# Backlog refinement

Turn each oldest eligible backlog item into independently reviewable work, recording each resolved detail as refinement proceeds, then continue until the backlog is exhausted or paused.

## Workflow

1. Enter and find eligible backlog. State: `Backlog refinement start. Current work resumes after backlog refinement.` Read `tasks/.schema.md`. Select oldest `status: needs-refinement` item when declared; otherwise select oldest legacy `status: todo` and `scope: draft` item when both fields and values are declared. If none exist, state: `Backlog refinement complete. Choose another workflow or resume prior work.` Done when one oldest eligible item is selected or the empty-backlog exit is clear.
2. Anchor the item. Read its current record and state desired outcome, user or business value, and current status. Inspect related records, existing behavior, dependencies, and adjacent work; separate confirmed facts from assumptions. Make sequencing, scope, opportunity cost, and speculative complexity explicit. Done when the item's constraints and preserved behavior are known.
3. Shape and record the boundary. Define in scope, out of scope, affected flows, likely implementation path, split points, observable acceptance, edge cases, and proportionate validation. Immediately update the task Markdown with confirmed detail, recommendations, and open questions as each becomes clear; do not hold resolved detail for a final approval. Keep one independently reviewable outcome per task. Done when the record shows the current known task shape and an implementer can identify what changes and what does not.
4. Resolve and record decisions. Label recommendations and open questions. When the user supplies or corrects requested detail, immediately update the task Markdown with that response and continue refinement. Treat a direct answer as agreement on the answered detail unless the user marks it tentative, asks for discussion, or a material decision remains open. Ask only the next unresolved material question. Stop only for a material decision, blocked item, or explicit user pause; do not treat `TBD` as permission to choose. Done when every resolved detail is recorded and remaining uncertainty is explicit.
5. Make the item ready. When no material decision remains open, set only that item to `status: todo` when `needs-refinement` is declared; otherwise set legacy `scope: agreed` and retain `status: todo` when both fields and values are declared. Never implement it or set it `doing`. Do not restate settled detail merely to request confirmation. Done when the item is ready, blocked on a specific decision, or explicitly paused.
6. Continue or exit. After saving a ready item, immediately return to step 1, select the next oldest eligible item, and begin its refinement in the same turn. Exit only for an empty eligible backlog, a material decision, a blocked item, or explicit user pause. On empty backlog, direct the user to another workflow or prior work. Done when the next item is underway or an exit condition is explicit.

## Output shape

```text
Backlog refinement: <task>

Confirmed:
- <facts and preserved behavior>

Proposed:
- <implementation shape and recommendation>

Acceptance:
- <observable checks>

Saved:
- <details recorded in the task Markdown>

Open:
- <material decision, or None>

Next:
- <next question when an open decision exists; otherwise next eligible item>.
```

## Rules

- Eligible backlog contains only declared unready task states: `needs-refinement`, or legacy `todo` plus `scope: draft`; process it oldest first.
- Refine one item at a time; persist each resolved detail in its task Markdown before moving on.
- Keep ready items at `status: todo` for a separate implementation workflow.
- Do not implement, reprioritize without request, or modify task-lifecycle status definitions.
- Use a broader trade-off review for cross-feature architecture, public API, migration, security, or cost decisions.
