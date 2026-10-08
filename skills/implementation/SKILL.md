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
2. Bound the slice. Establish the whole task's goal, exclusions, expected result, and acceptance from the agreed request. Keep the technical contract and representative code/artifacts in the task record when one exists; do not invent missing behavior or create a tracker. Select the smallest coherent slice. Proceed without another approval gate when scope and approach are established. Use `ask_user` only for unresolved decisions materially affecting behavior, scope, security, data, public API, compatibility, irreversible action, or an explicit review gate. Do not emit routine preambles or progress summaries; present context when needed to resolve a decision. Done when one bounded slice is authorized or a necessary decision is explicit.
3. Trace before edit. Apply Coding's understanding and smallest-solution workflow. Treat untraced paths and unconfirmed assumptions as delivery risk. Inspect callers, interfaces, data flow, tests, edge cases, operational impact, and local conventions needed to avoid a wrong change. Done when changed path and preserved behavior are known.
4. Implement slice. Prefer existing patterns, standard-library behavior, and small diffs; make the smallest complete change. Pause for decisions affecting behavior, scope, security, data, public API, or compatibility. Done when slice behavior is implemented without unrelated refactoring.
5. Handle replacement. Remove obsolete local references, tests, docs, and files within agreed scope. Use `ask_user` before broad deletion outside that scope or irreversible cleanup; leave those parts untouched while blocked. Done when in-scope cleanup is complete and broader cleanup is agreed or explicitly deferred.
6. Validate proportionately. Run the smallest credible agreed or low-risk local check against acceptance. Correct in-scope failures and rerun affected checks; stop with a precise blocker when correction needs a material decision or unavailable capability. Report checks not run and remaining risk. Do not run broad suites by habit. Done when checks pass or unresolved failures are explicit.
7. Record and report. When tracking is configured, update Plan progress and Checks evidence in place; update changed contracts in their specification section rather than appending repeated summaries. Follow collection usage for layout; omit irrelevant headings and empty placeholders. In the default task layout, one final Comments section holds errata and incidental notes, not a second specification or check log. Keep its declared active state while coherent work remains; use its review state only when the requested work is complete. Without tracking, report the outcome directly. Report completed work, non-obvious inspection path, validation, and any remaining material decision or blocker. Do not report a routine next slice before continuing it. Done when work is complete or a necessary user action is clear.

## Output shape

Before implementation begins, use a short paragraph or a few bullets, not a mandatory section dump. For example:

> Add CSV export for the currently filtered tasks, in displayed order. The download uses the agreed columns; filters and task data stay unchanged, and column selection is out of scope. Details: `records/tasks/export-filtered-tasks.md`.

Chat is the orientation summary; the task record holds the concise technical contract, code/artifact examples, acceptance, and check evidence. For technical tasks, explain the important interface or flow change and its point, not only eventual user value. Include only the detail needed to recognize the intended result, and offer depth through the record path rather than copying the record. Without a task record, summarize the agreed request without inventing a tracker. On resuming a task, briefly restate its goal and expected result plus remaining work; do not repeat the summary before every routine slice. Continue without waiting for confirmation unless a material decision or explicit review gate remains.

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
