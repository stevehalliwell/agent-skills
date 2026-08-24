---
name: wrap-up
description: "Wrap up, end session, finish session, handoff, write handoff, save progress, record next steps, future pickup, prepare for next session, or user says done for now. Goal: update Attendant work records first, then write concise project-local pickup context in .pi/handoff.md."
---

# Wrap It Up

Work records hold durable task state. Handoff points next agent to them.

## Workflow

1. Enter wrap-up mode.
   - Say `Wrap-up start. Updating work records and capturing pickup state.`
   - Done when writes are expected.
2. Ground facts in repo state, user goal, changed paths, and checks actually run.
   - Done when completed work, unresolved work, and validation state are known.
3. Update or create work records.
   - If Attendant is configured, use `/skill:attendant` `sync`, find relevant records, update completed/current/future work status, priority, checks, and next slice. Create missing concrete follow-up work via `/skill:add-todo` behavior before handoff. Every actionable `Next:` must have a record.
   - If configuration is absent, do not create a tracker only for wrap-up; state that durable task state is unavailable.
   - Done when each next action has a current work record or absence is explicit.
4. Create/update `.pi/handoff.md`.
   - Use `../init-project/templates/.pi/handoff.md`.
   - `Next:` must name 1-3 actions and directly link relevant `records/tasks/*.md` paths or give an exact Attendant query. Include record source paths in `Files:`.
   - Preserve human notes; replace only marked handoff block.
   - Done when handoff is concise pointer, not duplicate tracker.
5. Exit.

```text
Wrap-up resolved:
Records updated: <paths or "Attendant not configured">
Handoff written: .pi/handoff.md
Next:
1. <action>
```

## Rules

- High-signal facts only. No chat dumps, secrets, tokens, or env values.
- Do not inspect Pi session logs unless user asks.
