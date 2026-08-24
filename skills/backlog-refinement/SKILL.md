---
name: backlog-refinement
description: "Refine backlog items, clarify unready tasks, flesh out a todo, scope work, define acceptance, split an oversized item, or make backlog work implementation-ready. Process oldest unready item one at a time, retain user approval before making it ready, then continue to the next item; do not implement work or automatically change facets."
---

# Backlog refinement

Turn the oldest eligible backlog item into independently reviewable work, then continue until the backlog is exhausted or paused.

## Workflow

1. Enter and find eligible backlog. State: `Backlog refinement start. Current work resumes after backlog refinement.` Read `tasks/.schema.md`. Select oldest `status: needs-refinement` item when declared; otherwise select oldest legacy `status: todo` and `scope: draft` item when both fields and values are declared. If none exist, state: `Backlog refinement complete. Use /facets to choose a different facet or workflow.` Done when one oldest eligible item is selected or the empty-backlog exit is clear.
2. Anchor the item. Read its current record and state desired outcome, user or business value, and current status. Inspect related records, existing behavior, dependencies, and adjacent work; separate confirmed facts from assumptions. Done when the item's constraints and preserved behavior are known.
3. Shape the boundary. Define in scope, out of scope, affected flows, likely implementation path, split points, observable acceptance, edge cases, and proportionate validation. Keep one independently reviewable outcome per task. Done when an implementer can identify what changes and what does not.
4. Resolve decisions. Label recommendations and open questions. Stop for a material decision, blocked item, or explicit user pause; do not treat `TBD` as permission to choose. Done when agreement is possible or a stop condition is explicit.
5. Seek agreement. Summarize the refined item and ask the user to confirm or correct it. On approval, update only that item to `status: todo` when `needs-refinement` is declared; otherwise set legacy `scope: agreed` and retain `status: todo` when both fields and values are declared. Never implement it or set it `doing`. Done when the item is agreed, corrected for another pass, or paused.
6. Continue or exit. After agreement, return to step 1 and select the next oldest eligible item. Exit only for an empty eligible backlog, a material decision, a blocked item, or explicit user pause. On empty backlog, direct the user to `/facets`; do not change facets automatically. Done when the next item or exit condition is explicit.

## Output shape

```text
Backlog refinement: <task>

Confirmed:
- <facts and preserved behavior>

Proposed:
- <implementation shape and recommendation>

Open:
- <decisions required, or None>

Acceptance:
- <observable checks>

Need from you: confirm, correct, or choose <decision>.
```

## Rules

- Eligible backlog contains only declared unready task states: `needs-refinement`, or legacy `todo` plus `scope: draft`; process it oldest first.
- Refine one item at a time and preserve user approval before setting its declared ready state.
- Keep ready items at `status: todo` for a separate implementation workflow.
- Do not implement, prioritize, change facets, or modify task-lifecycle status definitions.
- Use a broader trade-off review for cross-feature architecture, public API, migration, security, or cost decisions.
