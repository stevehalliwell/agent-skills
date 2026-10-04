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

1. Enter iteration mode. Anchor the current task and pause point, if any. State: `Iteration mode: bundled tweaks; direct edits and no per-tweak checks unless needed for obvious breakage.` Do not update work records, status, handoff, or changelog for each tweak. Done when the tweak area and return context are known.
2. Make the direct requested edit. Read only files needed for it; load [Coding](../coding/SKILL.md) for code changes. Keep the diff narrow; do not plan, refactor, add abstractions, or ask exploratory questions unless ambiguity or risk materially changes the requested behaviour. Done when the requested tweak is applied without unrelated changes.
3. Check only for obvious breakage. Do not run tests, linters, builds, or broad audits per tweak by default. Run the fastest relevant check only when requested or needed for safety; correct any in-scope failure and recheck, or exit with a blocker. Done when obvious breakage is excluded or reported.
4. Report the delta. Name the changed file, exact change, and check state, then invite the next tweak. Done when the user can assess the change and continue or stop.
5. End cleanly. Treat `done`, `end iteration`, `iteration complete`, `return to normal`, `that works`, or a request for summary as the end of the tweak loop. Review the final diff and run one smallest credible check for the bundle when its risk warrants it; otherwise state why no check was run. Correct failures and recheck or report a blocker. When `tasks` is configured, load [Attendant](../attendant/SKILL.md), run `schema`, and read task schema and usage guidance. Update the existing task or record one bundle under its documented lifecycle and approval rules; retain active state if task work remains. Resolve material ambiguity when usage is missing or contradicts the schema. Ending the mode or asking for a summary alone is not completion approval. Summarize the bundle and resume the anchored context or normal work. Done when final checks, record state, and mode exit are explicit.

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
