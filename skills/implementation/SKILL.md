---
name: implementation
description: "Use when implementing an agreed task, coding an approved change, fixing a scoped bug, or delivering the next reviewable slice. Trace affected code enough to avoid wrong edits, implement one slice, run proportionate validation, and report result. Do not use for unclear scope, architecture trade-offs, backlog shaping, or pre-code feasibility review."
---

# Implementation

Deliver one agreed, reviewable slice with smallest credible validation.

## Workflow

1. Anchor work. Read selected task record, `tasks/.schema.md`, acceptance, current slice, and affected code paths. Confirm task is `status: todo`; when schema declares `scope`, also confirm `scope: agreed`. Preserve stated behavior and decisions. Done when target behavior and slice boundary are known.
2. Establish slice. If task has no reviewable slices, write proposed slices to task plan, state first slice, and wait for user approval before editing implementation files. If slices exist, select one. Done when one bounded slice is authorized.
3. Trace before edit. Inspect callers, interfaces, data flow, tests, and local conventions needed to avoid a wrong change. Done when changed path and preserved behavior are known.
4. Implement slice. Reuse existing mechanisms; make smallest complete change. Pause for decisions affecting behavior, scope, security, data, public API, or compatibility. Done when slice behavior is implemented without unrelated refactoring.
5. Handle replacement. When removing or replacing behavior, identify obsolete references, tests, docs, and files. Ask user for cleanup scope before broad deletion. Done when cleanup is either agreed or explicitly deferred.
6. Validate proportionately. Recommend smallest credible check for change; run only agreed or low-risk local checks. Do not run broad suites by habit. Done when validation result and remaining risk are explicit.
7. Record and report. Update task with completed slice, checks, and next slice. Keep task `doing` until all slices complete; then set `review`. Report completed slice, non-obvious inspection path, validation, and next slice. Done when next action is clear.

## Output shape

```text
Implementation: <task>

Completed:
- <slice and outcome>

Inspected:
- <non-obvious path or none>

Validation:
- <check and result>

Next:
- <next slice, review, or blocker>
```

## Rules

- Implement only agreed scope.
- One reviewable slice per turn.
- Prefer existing code, platform features, and focused diffs.
- Treat user approval as required for material behavior, scope, security, data, public API, or compatibility decisions.
- State assumptions and unresolved risk; do not claim unrun validation.
