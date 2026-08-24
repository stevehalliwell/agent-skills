---
name: iteration
description: "Iteration mode, rapid iteration, quick tweaks, tweak pass, minor adjustments, polish pass, UI polish, tune color, adjust size, font size, spacing, layout tweak, copy tweak, try another value, or bundle small changes when user wants many quick back-and-forth edits without work-record/status churn. Goal: make direct, careful local changes, ask for the next tweak, and summarize the final bundle."
---

# Iteration

Keep quick tweak loops fast: apply small explicit changes carefully, preserve local context, avoid per-tweak validation and documentation churn, then summarize once when iteration ends.

## Trigger clarification

Use this skill when any of these are true:

- User asks for rapid-fire iteration, tweak mode, polish pass, minor adjustment pass, or many small edits in one area.
- User is changing small values repeatedly: color, font size, spacing, dimensions, labels, copy, thresholds, ordering, visual state, or nearby config.
- User says to try another value, adjust it again, make it bigger/smaller/lighter/darker/tighter/looser, or similar back-and-forth tweak language.
- User wants changes bundled as one set instead of updating work records, handoff notes, changelog, or status docs per tweak.

Iteration is for low-risk changes where the direct requested change and final state matter more than planning or intermediate history. If work becomes material, multi-area, risky, or needs investigation, exit iteration and return to normal behaviour.

## Workflow

1. Enter iteration mode. State: `Iteration mode: bundled tweaks; direct edits and no per-tweak checks unless needed for obvious breakage.` Do not update work records, status, handoff, or changelog for each tweak.
2. Make the direct requested edit. Read only files needed for it. Keep the diff narrow; do not plan, refactor, add abstractions, or ask exploratory questions unless ambiguity or risk materially changes the requested behaviour.
3. Check only for obvious breakage. Do not run tests, linters, builds, or broad audits per tweak by default. Run the fastest relevant check only when the user requests it or the change cannot be safely assessed otherwise.
4. Report the delta. Name the changed file, exact change, and check state, then invite the next tweak.
5. End cleanly. Treat `done`, `end iteration`, `iteration complete`, `return to normal`, `that works`, or a request for summary as completion. Summarize the final bundle; when task storage is configured, record one completed iteration bundle. Return to normal project behaviour.

## Output shape

During iteration:

```text
Iteration mode: bundled.
Changed: <path>
Tweak: <exact small change>
Check: <not run / command and result>
Next: send the next tweak or say done.
```

On completion:

```text
Iteration complete.
Changed:
- <path> — <final bundled change>
Check: <command or not run>
Record: <records/tasks/iteration-*.md / not created: task storage unavailable>
```

## Rules

- Keep each change faithful to the direct request and narrow enough for a fast feedback loop.
- Do not let repeated tweaks create per-change tracking or validation churn.
- Escalate material behavior, scope, security, data, public API, compatibility, or uncertain risk to normal work.
- Always close iteration mode on an end-iteration signal; final summary marks return to normal behaviour.
