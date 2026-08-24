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
   - Use `/skill:attendant` `query` for `tasks` where `status IN ('doing', 'todo', 'blocked', 'review')`, ordered by updated/current relevance. Use its `search` only for targeted text.
   - If config is absent, state that no work collection exists; do not create one during pickup.
   - Done when active/resumable work is known or absence is confirmed.
4. Read `.pi/handoff.md` if present.
   - Treat it as a pointer to record paths/queries and repo state. Work records and Git state win when fresher.
   - Done when handoff either informs choices or is stale/absent.
5. Read root status docs only if needed, stopping once context suffices.
   - Done when enough context exists to offer actions.
6. Ask user to select from max three concrete actions.
   - Done when user has clear choices.
7. Record selected work before implementation.
   - If `tasks` exists, update selected record `status: doing`; create one via `/skill:add-todo` only when selected work lacks a record and user selection makes it necessary.
   - Use `/skill:attendant` `validate`, then `sync`, and verify record.
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
Options:
1. <action>
2. <action>
3. <action>
```

## Rules

- Read only enough context to choose next action.
- Do not inspect Pi session logs or summarize whole repo.
