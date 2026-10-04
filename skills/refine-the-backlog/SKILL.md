---
name: refine-the-backlog
description: "Refine the backlog, work through unready backlog items, or make the backlog implementation-ready. Select the oldest needs-refinement task and apply task-refinement one item at a time until no eligible tasks remain or refinement is blocked or paused. Use task-refinement directly for one specified task; do not implement, triage, or reprioritize work."
---

# Refine the backlog

Work through the unready task queue using the single-task refinement workflow. This skill owns selection and iteration, not task shaping.

## Workflow

1. Enter and resolve the queue.
   - State: `Backlog refinement start. Current work resumes after backlog refinement.` Retain the prior work context.
   - Inspect `.pi/attendant.tables`. When `tasks` is configured, load [Attendant](../attendant/SKILL.md), run `schema`, and read the configured task schema and usage guidance. Follow its unready/ready state meanings and backlog ordering; resolve material ambiguity when usage is missing or contradicts the schema.
   - If `tasks` is not configured, report that this workflow requires a configured task queue. Do not bootstrap storage or invent a tracker; a specified task can still use task refinement directly.
   - Done when the queue's schema and return context are known, or a storage blocker is explicit.

2. Select the oldest eligible item.
   - Use Attendant `query` to find items in the unready state documented by collection usage (the init-project default is `needs-refinement`). Use only declared fields and values; do not guess eligibility from status names.
   - For oldest-first refinement, use Attendant's documented built-in `created_at`, then record name as a tie-breaker. If collection usage requires a different order, make the conflict with this workflow explicit and resolve it before selecting; never infer age from modification time.
   - If no eligible items remain, state: `Backlog refinement complete. Next: resume prior work or choose another workflow.`
   - Done when one oldest eligible item and its source path are selected, or the empty-queue exit is clear.

3. Refine that item.
   - Load and follow [Task refinement](../task-refinement/SKILL.md) for the selected item. Pass its source path, persisted implementation-ready intent, and this backlog workflow as the return context.
   - Task refinement owns context grounding, task boundaries, split points, acceptance, material questions, immediate persistence, and the declared ready-state transition. Do not duplicate or replace its procedure here.
   - Stay on this item while it has an unresolved material decision. On a blocker or explicit user pause, preserve its recorded state and report the reason; do not skip to another item.
   - Done when task refinement returns the item ready, waiting on a specific decision, blocked, or explicitly paused.

4. Continue or exit.
   - After an item is saved ready, immediately return to step 2 and begin the next eligible item's refinement in the same turn. Do not add a confirmation gate for settled details or continuing the requested backlog workflow.
   - Exit only when no eligible items remain, an item awaits a material decision, a blocker prevents progress, or the user explicitly pauses.
   - Done when the next item is underway or an exit condition and return context are explicit.

## Rules

- Refine one item at a time, oldest first; task refinement owns all per-task shaping and readiness rules.
- Never implement, set an item to `doing`, reprioritize without request, or modify collection lifecycle rules.
- Do not treat completion of one item as completion of the requested backlog pass.
