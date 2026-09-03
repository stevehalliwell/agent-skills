---
name: pick-up
description: "Start session pickup for humans: resume project, pick up where left off, continue previous work, review handoff, or choose next task. Reads repo state, Attendant tasks, and .pi/handoff.md, then offers short next-step choices and starts selected work."
disable-model-invocation: true
---

# Pick It Up

Reconstruct state fast. Attendant tasks are planning authority; handoff is supplemental.

## Workflow

1. Enter pickup mode.
   - Say: `Pickup start. Reading repo state, work records, and handoff before changing files.`
   - Do not modify files before selection.
   - Done when read-only boundary is clear.
2. Check repo state: `git status --short`, recent commits, relevant paths.
   - Done when recent changes and dirty files are known.
3. Inspect Attendant when configured.
   - Load `/skill:attendant` and `/skill:task-lifecycle`; read `tasks/.schema.md`. Use `/skill:attendant` `query` for `tasks` with declared status values `doing`, `todo`, `blocked`, `review`, and `needs-refinement`, ordered by updated/current relevance. Use its `search` only for targeted text.
   - For every `review`, `doing`, and `needs-refinement` task, read enough of its record to show exact name and a faithful short description, using its outcome or summary when present.
   - If config is absent, state that no work collection exists; do not create one during pickup.
   - Done when active/resumable work is known or absence is confirmed.
4. Read `.pi/handoff.md` if present.
   - Treat it as a pointer to record paths/queries and repo state. A handoff is current only when it names exactly one `Next:` action, its referenced task record exists, and its task state, changed paths, and Git state do not contradict the handoff.
   - When current, select its `Next:` action automatically; do not offer alternatives. Work records and Git state win when fresher.
   - Done when a current handoff action is selected or the handoff is stale/absent.
5. Read root status docs only if needed, stopping once context suffices.
   - Done when enough context exists to resume the handoff action or offer actions.
6. If no current handoff action was selected, ask user to select from max three concrete actions.
   - Done when user has clear choices or the handoff action is selected.
7. Record selected work before implementation.
   - If `tasks` exists, update selected record `status: doing` unless it is already `doing`; create one via `/skill:add-todo` only when selected work lacks a record and user selection makes it necessary.
   - Use the normal Attendant update operation, which prepares its projection; verify the resulting record. Run `validate`, `sync`, or `doctor` only for a reported health/projection problem.
   - Do not update handoff after selection; it is stale once used.
   - Done when selected work has an active record or absence is stated.
8. Exit and begin work.
   - Say: `Pickup resolved: <selected action>. Next: starting <action>.`
   - Done when first action starts.

## Output shape

```text
Pickup start. Reading repo state, work records, and handoff before changing files.
Recent:
- <fact>
Work state:
- In review: <exact task name> — <short description>, or None
- In progress: <exact task name> — <short description>, or None
- Needs refinement: <exact task name> — <short description>, or None
Options:
1. <action>
2. <action>
3. <action>
```

## Rules

- Read only enough context to choose next action.
- Do not inspect Pi session logs or summarize whole repo.
