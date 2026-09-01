---
name: task-lifecycle
description: "Use when Attendant tasks exists and agent needs to create, select, update, implement, review, complete, or resume tracked work. Defines task status transitions, record contents, and task-tracking invariants; operation skills own tool calls and user-flow details."
---

# Task lifecycle

Keep Attendant work records useful across sessions. Markdown records are source of truth.

## Trigger clarification

Use only when `.pi/attendant.tables` configures `tasks`. Skip for untracked small tasks and projects without Attendant.

## Lifecycle

Read `tasks/.schema.md` before applying this lifecycle. Existing project schemas are authoritative: use only their declared fields and values. This lifecycle and initialization template define recommended defaults for new projects, not a retrofit requirement.

1. Use one active item.
   - Keep one task `doing` at a time. Before switching, update or block current item.
   - `needs-refinement`: captured backlog work that needs clearer outcome, boundaries, or acceptance before it can be selected. `todo`: refined, prioritised, and ready to select. `doing`: active implementation, including intermediate slices. `review`: entire task is implementation-complete and awaits final user review/approval. `done`: approved and complete. `deferred`: intentionally paused; record reason and reconsideration trigger. `blocked`: waiting on dependency or answer. `defunct`: no longer relevant; record why. `merged`: folded into another task; link its replacement record. `willnotdo`: deliberately not to be implemented; record rationale.
   - Done when current work state is unambiguous.

2. Keep readiness honest.
   - Keep unshaped work in `needs-refinement`; do not implement it.
   - Move an item to `todo` when its outcome and acceptance are sufficiently clear and material decisions are resolved through user input.
   - When `priority` is declared, order backlog work: `critical`, `high`, `medium`, `low`, then `delay`. `delay` means deprioritised, not paused; use the schema's paused status for intentional pauses.
   - Preserve unresolved product behavior under `Open questions`; `TBD` is not permission to choose behavior.
   - Done when declared status/priority fields and authority match the task's actual state.

3. Keep record useful.
   - Title/name identifies one independently reviewable task.
   - Record desired outcome, in/out of scope, behavior to preserve, acceptance, decisions, plan, implemented work, checks, review/next slice, risks.
   - Add dependencies with `depends_on` when another work record blocks progress.
   - Keep body concise; do not dump chat history, secrets, or routine intermediate tweaks.
   - Done when future agent can resume without reconstructing task intent.

4. Preserve work cadence.
   - Before implementation: selected record was `todo`, is now `doing`, and has clear acceptance with material decisions resolved through user input.
   - During implementation: record material decisions, behavior changes, and meaningful checks.
   - Keep `doing` while task has remaining slices. Record completed slices under `Implemented so far` and next slice under `Review / next slice`; do not toggle `review` between slices.
   - Set `review` only when entire task is implementation-complete and awaits final user approval. After user approves: set `done`; retain record for search/history. Do not archive or duplicate it into a tracker file.
   - Done when record reflects actual task phase.

5. Query and hand off.
   - Use `/skill:attendant` `query` for status/field selection and `search` for text. Its normal operations prepare the projection automatically.
   - Normal Attendant data actions prepare the projection themselves. Run `validate`, `sync`, or `doctor` only for an explicitly requested diagnostic or a reported health/projection problem; resolve diagnostics before relying on record state.
   - Handoff points to records or exact queries. Each actionable next step has a work record.
   - Done when next session can locate current work directly.

## Operation ownership

- `add-todo`: bootstrap/capture/dedupe/create records.
- `pick-up`: query candidates and select active work.
- `task-refinement`: agree implementation shape.
- `wrap-up`: update task state and handoff.
- `iteration`: write one completed iteration record.
- `tradeoff-review`: durable decision records, not task mechanics.

## Rules

- Do not use a handoff as task tracker.
- Do not start `needs-refinement` work or mark work `done` before user approval.
