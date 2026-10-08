---
name: wrap-up
description: "Wrap up, end session, write a handoff, save progress for future pickup, prepare for next session, or user says done for now. Update configured Attendant task records first, then save concise project-local pickup context in .pi/handoff.md. Skip summary-only requests with no persistent handoff."
---

# Wrap It Up

Work records hold durable task state. Handoff points next agent to them.

## Trigger clarification

Use for persistent session closure or pickup context, not a chat-only summary. On explicit invocation, inspect current work and storage immediately; loading this skill alone is not completion. Wrap-up authorizes record and handoff updates, not implementation, tracker setup, commits, or publishing.

## Workflow

1. Anchor current work and resolve target project. Prefer its Git root, otherwise its working directory; resolve `.pi/` paths there. Read existing handoff and inspect `.pi/attendant.tables`.
   - Done when project, current goal, and storage are known.
2. Ground facts in repo state, user goal, changed paths, and checks actually run. Distinguish session work from unrelated existing changes. Select one grounded continuation; if work is complete with no known follow-up, use `Next: None — requested work complete.` rather than inventing work.
   - Done when completed work, unresolved work, validation state, and continuation are known.
3. Update or create work records.
   - If `.pi/attendant.tables` configures `tasks`, load [Attendant](../attendant/SKILL.md), run `schema`, and read the configured task schema and usage guidance before finding relevant records. Follow its lifecycle, approval, and handoff rules; resolve material ambiguity when usage is missing or contradicts the schema. Use its normal update operation for completed/current/future work status, priority, checks, and next slice. Create missing concrete follow-up work via [Backlog capture](../backlog-capture/SKILL.md) before handoff. When task storage is configured, the one actionable `Next:` must have a record. Run `validate`, `sync`, or `doctor` only for a reported health/projection problem.
   - Without configured `tasks`, do not create a tracker only for wrap-up; state `Attendant tasks not configured`. Preserve any known existing task links without claiming they were updated.
   - Done when relevant records reflect verified work and the actionable continuation has a record, or configuration absence or update blockers are explicit.
4. Create/update `.pi/handoff.md`.
   - Read [Handoff template](../init-project/templates/.pi/handoff.md) before writing. Use its fields inside `<!-- wrap-up:start -->` and `<!-- wrap-up:end -->` markers; omit empty optional fields.
   - `Next:` names the continuation selected in step 2. With task storage, directly link its record source path or give an exact Attendant query and include source paths in `Files:`. Without configured tasks, state that limitation without inventing a record link. Include checks actually run and material blockers in `Context:`; mark unrun checks explicitly.
   - Replace only the single complete marked block. If no markers exist, preserve existing content and append one marked block; do not silently rewrite a legacy handoff. If markers are incomplete or duplicated, stop replacement and report the conflict rather than guessing ownership.
   - Done when verified pickup facts are saved without overwriting human notes, or the write blocker is explicit.
5. Verify and exit. Reread changed records and handoff; check facts, links or query, marker boundaries, and the single continuation against repo evidence. Correct errors and recheck. Report unresolved failures instead of claiming success. End wrap-up without starting the continuation.
   - Done when saved state is verified or blockers are reported.

## Output shape

Report only verified writes; replace success lines with blockers when needed.

```text
Wrap-up resolved:
Records updated: <source paths, or "Attendant tasks not configured">
Handoff written: <project-root>/.pi/handoff.md
Next: <one continuation, or "None — requested work complete">
```

## Rules

- High-signal facts only. No chat dumps, secrets, tokens, or env values.
- Do not inspect Pi session logs unless user asks.
