---
name: task-refinement
description: "Refine a task when user explicitly asks to define how it should work; or task goal is clear but implementation is vague and multiple plausible local approaches exist. Produce executable technical shape: behavior, boundaries, path, edge cases, and acceptance. Skip todo review, triage, prioritization, status checks, task selection, and routine implementation with an established approach."
---

# Task Refinement

Turn task headline and desired outcome into shared executable technical shape, then return to implementation or docs.

## Trigger clarification

Use this skill when either is true:

- User explicitly asks to scope, refine, flesh out, or define a feature/task: its user-visible behavior, boundaries, edge cases, or acceptance checks.
- Task goal is clear, but source material leaves behavior, technical shape, edge cases, acceptance, or implementation sequence vague; two or more plausible local approaches exist; existing code/patterns do not clearly select one; and the choice materially affects observable behavior, acceptance, public interfaces, data, compatibility, security, or substantial rework. Do not use this skill merely because routine implementation details or file order are unknown.

A selected task record only triggers this skill when user asks to make it implementation-ready, or it has unresolved behavior/technical shape with multiple plausible approaches.

Use `tradeoff-review` instead when the main issue is larger design direction, project priorities, cross-feature ramifications, architecture, durable public API/data/schema choices, migration/reversibility risk, or trade-offs that affect existing/future features.

Skip todo review, triage, prioritization, status checks, task selection, and codebase investigation. Also skip any task—small or multi-file—with an established approach from explicit requirements or existing code patterns; routine implementation planning alone is not task refinement.

Completion: task needs an executable-path pass or skill is skipped for clear reason.

## Workflow

1. Anchor current work.
   - Say: `Task refinement start. Current work resumes after shared task shape.`
   - This mode may inspect project context, discuss task shape, and update the task record as user input resolves details. It never modifies implementation code.
   - Record confirmed facts, user answers, resolved decisions, and remaining open questions in the task Markdown as refinement proceeds. If the user requests discussion only, do not write files.
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
   - Done when another agent could implement after material decisions are resolved through user input and the current task shape and technical detail are recorded.

5. Resolve and record open questions.
   - Ask specific decision questions only when choices change task shape or acceptance.
   - When the user supplies or corrects requested detail, immediately update the task Markdown and continue with the next unresolved material question or refinement step.
   - Treat a direct answer as agreement on the answered detail unless the user marks it tentative, asks for discussion, or a material decision remains open.
   - Keep unknown product behavior open. `TBD` records unresolved detail; it does not authorize choosing behavior.
   - Agent may recommend a default only when clearly labelled `Proposed`.
   - Done when every resolved detail is recorded and remaining uncertainty is visible.

6. Keep task record current.
   - Update the relevant task source record after each resolved refinement detail; do not restate settled detail merely to seek final confirmation.
   - When no material decision remains, record the completed task shape and set its declared ready state when the user requested implementation-ready work.
   - If user only wants discussion, do not write files.
   - Done when saved task detail reflects current shared understanding or discussion-only scope is explicit.

7. Exit refinement mode.
   - Use one explicit outcome:
     - `Task refined: <summary>. Saved: <task path>. Next: continue refinement or return to prior work.`
     - `Task refinement open: <specific decisions>. Saved: <task path>. Next: ask the next material question.`
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

Saved:
- <task Markdown details updated in this turn>

Open:
- <specific material decision, or None>

Next:
- <next material question, or continued refinement>.
```

When resolved:

```text
Task refined: <summary>. Saved: <task path>. Next: return to prior work.
```

When unresolved:

```text
Task refinement open: <specific decisions>. Saved: <task path>. Next: <next material question>.
```

## Rules

- Brevity compresses wording, not required content.
- Do not begin implementation while feature boundary or technical shape is vague.
- Treat task refinement as temporary mode: enter explicitly, resolve material questions while recording progress, then exit explicitly.
- Discuss toward shared understanding while keeping the task record current; implementation remains a separate workflow.
- Do not dump private checklist output to files; turn it into user-reviewable understanding first.
- Use `tradeoff-review`, not this skill, for larger design direction, project priorities, cross-feature ramifications, or architectural trade-offs.
- Do not invent product decisions silently; label assumptions.
- Prefer fewer, concrete bullets over broad prose.
