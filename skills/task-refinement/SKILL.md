---
name: task-refinement
description: "Refine a task when user explicitly asks to define how it should work; or task goal is clear but implementation is vague and multiple plausible local approaches exist. Produce executable technical shape: behavior, boundaries, path, edge cases, and acceptance. Skip todo review, triage, prioritization, status checks, task selection, and routine implementation with an established approach."
---

# Task Refinement

Turn task headline and desired outcome into shared executable technical shape, then return to implementation or docs.

## Trigger clarification

Use this skill when either is true:

- User explicitly asks to scope, refine, flesh out, or define a feature/task: its user-visible behavior, boundaries, edge cases, or acceptance checks.
- Task goal is clear, but source material leaves behavior, technical shape, edge cases, acceptance, or implementation sequence vague; two or more plausible local approaches exist; and existing code/patterns do not clearly select one.

A selected task record only triggers this skill when user asks to make it implementation-ready, or it has unresolved behavior/technical shape with multiple plausible approaches.

Use `tradeoff-review` instead when the main issue is larger design direction, project priorities, cross-feature ramifications, architecture, durable public API/data/schema choices, migration/reversibility risk, or trade-offs that affect existing/future features.

Skip todo review, triage, prioritization, status checks, task selection, and codebase investigation. Also skip any task—small or multi-file—with an established approach from explicit requirements or existing code patterns; routine implementation planning alone is not task refinement.

Completion: task needs an executable-path pass or skill is skipped for clear reason.

## Workflow

1. Anchor current work.
   - Say: `Task refinement start. Current work resumes after shared task shape.`
   - This mode may inspect project context, discuss task shape, and—after user agreement—update requested task records. It never modifies implementation code.
   - Do not write docs during refinement discussion unless user explicitly switches modes.
   - Done when current task pause point is explicit.

2. Ground in existing context.
   - If Attendant is configured; read selected task source path or use `/skill:attendant` `search`/`query` to locate it.
   - If no record exists, use user's message as source.
   - Done when source text and selected item are known.

3. Identify missing implementation detail.
   - Check for: feature summary, user/value, in-scope behavior, out-of-scope behavior, affected code/data flows, technical approach, alternatives ruled out, acceptance checks, open questions.
   - Done when gaps are explicit.

4. Reflect understanding, not checklist results.
   - Separate:
     - Confirmed: explicit user requirements or existing behavior grounded in project context.
     - Proposed: agent recommendations requiring user agreement.
     - Open: unknowns that could change behavior, boundaries, or acceptance.
   - Within those groups cover:
     - What it is: user-visible behavior/change.
     - What it is not: non-goals and tempting adjacent work.
     - Technical detail: affected files/flows/interfaces, data shape, edge cases, constraints, expected sequence.
     - Alternatives ruled out: option, why rejected, revisit trigger if useful.
     - Acceptance: observable check or command/manual verification.
   - Keep broad system trade-offs in `tradeoff-review`; keep local implementation detail here.
   - Done when another agent could implement after user confirms/corrects task shape and technical detail.

5. Ask user to steer, correct, or agree.
   - Ask for confirmation when understanding seems complete enough.
   - Ask specific decision questions only when choices change task shape or acceptance.
   - Keep unknown product behavior open. `TBD` records unresolved detail; it does not authorize choosing behavior.
   - Agent may recommend a default only when clearly labelled `Proposed`.
   - Done when remaining uncertainty is visible and user has clear next response path.

6. Update docs only after shared understanding.
   - If user asked to update record, first present detail block unless they explicitly asked for direct file edit.
   - After user agrees or corrects, edit relevant task source record.
   - If user only wants discussion, do not write files.
   - Done when output asks for review or includes saved paths after agreement.

7. Exit refinement mode.
   - Use one explicit outcome:
     - `Task refined: <summary>. Next: update requested docs or return to prior work.`
     - `Task refinement open: <specific decisions>. Next: waiting for user direction.`
   - Do not imply implementation authorization from refinement discussion alone.
   - Done when refinement discussion is closed or waiting on a specific user decision.

## Output shape

```text
Task refinement start. Current work resumes after shared task shape.

Task refinement: <name>

Confirmed:
- ...

Proposed:
- ...

Open:
- ...

Need from you: confirm, correct, or choose <specific decision>.
```

After agreement:

```text
Task refined: <summary>. Next: update requested docs or return to prior work.
```

If unresolved:

```text
Task refinement open: <specific decisions>. Next: waiting for user direction.
```

## Rules

- Brevity compresses wording, not required content.
- Do not begin implementation while feature boundary or technical shape is vague.
- Treat task refinement as temporary mode: enter explicitly, reach agreement, exit explicitly.
- Discuss toward shared understanding before implementing or saving docs.
- Do not dump private checklist output to files; turn it into user-reviewable understanding first.
- Use `tradeoff-review`, not this skill, for larger design direction, project priorities, cross-feature ramifications, or architectural trade-offs.
- Do not invent product decisions silently; label assumptions.
- Prefer fewer, concrete bullets over broad prose.
