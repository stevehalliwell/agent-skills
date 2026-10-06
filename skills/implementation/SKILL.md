---
name: implementation
description: "Use when implementing an agreed task, coding an approved change, fixing a scoped bug, delivering the next reviewable slice, or listing implementation-ready tasks when no focus is supplied. Trace affected code enough to avoid wrong edits, implement one slice, run proportionate validation, and report result. Do not use for unclear scope, architecture trade-offs, backlog shaping, or pre-code feasibility review."
---

# Implementation

Deliver one agreed, reviewable slice with smallest credible validation.

## Focus check

When no clear implementation focus is supplied or selected, load and follow [Find implementation-ready tasks](ready-tasks.md) instead of this implementation workflow. Done when a ready-task report recommends the highest-priority ready task, with quick wins and other factors secondary, or reports that no ready work exists.

## Required read

Load [Coding](../coding/SKILL.md) before inspecting implementation paths or editing code. This workflow covers code changes, so complete Coding's required read and apply its workflow alongside this one. Done when Coding is loaded before any code-change work begins.

## Workflow

1. Anchor work. Use the agreed request as the behavior and acceptance source. When `.pi/attendant.tables` configures `tasks`, load [Attendant](../attendant/SKILL.md), run `schema`, and read the configured task schema, usage guidance, and selected record. Follow the collection's documented readiness, active-work limits, approval gates, and transitions; start ready work in its implementation-active state and resume authorized active work without resetting it. Resolve material ambiguity when usage is missing or contradicts the schema. Do not start unready or blocked work. Without task storage, proceed from the agreed request without creating a tracker. Preserve stated behavior and decisions. Done when target behavior, tracking state, and slice boundary are known.
2. Summarize the task before starting implementation. After reading the agreed request and relevant task context, give a concise chat summary of the whole task's goal, key anti-goals, and expected result, including how it works or what it consists of. Synthesize the task in a short paragraph or a few bullets; do not paste its sections, full examples, acceptance checklist, or technical plan into chat. Keep the full end-state explanation, worked examples, boundaries, and implementation detail in the task record for deeper review; link its path when available. Distinguish the first slice from the whole-task result only when needed. This summary is not a new approval gate. Do not invent missing behavior to complete it; resolve material gaps before implementation. Select the smallest coherent slice and proceed when scope, acceptance, and approach are established. Ask only when an unresolved decision materially affects behavior, scope, security, data, public API, compatibility, irreversible cleanup, or an explicit review gate. Done when the user has seen a concise summary of the intended result and exclusions, detailed task context remains available, and one bounded slice is authorized or a necessary decision is explicit.
3. Trace before edit. Apply Coding's understanding and smallest-solution workflow. Treat untraced paths and unconfirmed assumptions as delivery risk. Inspect callers, interfaces, data flow, tests, edge cases, operational impact, and local conventions needed to avoid a wrong change. Done when changed path and preserved behavior are known.
4. Implement slice. Prefer existing patterns, standard-library behavior, and small diffs; make the smallest complete change. Pause for decisions affecting behavior, scope, security, data, public API, or compatibility. Done when slice behavior is implemented without unrelated refactoring.
5. Handle replacement. When removing or replacing behavior, identify obsolete references, tests, docs, and files. Ask user for cleanup scope before broad deletion. Done when cleanup is either agreed or explicitly deferred.
6. Validate proportionately. Run the smallest credible agreed or low-risk local check against acceptance. Correct in-scope failures and rerun affected checks; stop with a precise blocker when correction needs a material decision or unavailable capability. Report checks not run and remaining risk. Do not run broad suites by habit. Done when checks pass or unresolved failures are explicit.
7. Record and report. When tracking is configured, update the task with completed work and checks. Keep its declared active state while coherent work remains; use its review state only when the requested work is complete. Without tracking, report the outcome directly. Report completed work, non-obvious inspection path, validation, and any remaining material decision or blocker. Do not report a routine next slice before continuing it. Done when work is complete or a necessary user action is clear.

## Output shape

Before implementation begins, use a short paragraph or a few bullets, not a mandatory section dump. For example:

> Add CSV export for the currently filtered tasks, in displayed order. The download uses the agreed columns; filters and task data stay unchanged, and column selection is out of scope. Details: `records/tasks/export-filtered-tasks.md`.

Chat is the orientation summary; the task record holds the full explanation, worked examples, acceptance, and technical detail. Include only the detail needed to recognize the intended result, and offer depth through the record path rather than copying the record. Without a task record, summarize the agreed request without inventing a tracker. On resuming a task, briefly restate its goal and expected result plus remaining work; do not repeat the summary before every routine slice. Continue without waiting for confirmation unless a material decision or explicit review gate remains.

At completion or a blocker, report:

```text
Implementation: <task>

Completed:
- <slice and outcome>

Inspected:
- <non-obvious path or none>

Validation:
- <check and result>

Status:
- <complete, review, or blocker>
```

## Rules

- Implement only agreed scope.
- Continue through coherent requested work in the same turn. Stop after a completed slice only when user review is explicitly requested or a material decision or blocker requires it.
- Prefer existing code, platform features, and focused diffs.
- Treat user approval as required for material behavior, scope, security, data, public API, or compatibility decisions.
- State assumptions and unresolved risk; do not claim unrun validation.
