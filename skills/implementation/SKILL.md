---
name: implementation
description: "Use when implementing an agreed task, coding an approved change, fixing a scoped bug, or delivering the next reviewable slice. Trace affected code enough to avoid wrong edits, implement one slice, run proportionate validation, and report result. Do not use for unclear scope, architecture trade-offs, backlog shaping, or pre-code feasibility review."
---

# Implementation

Deliver one agreed, reviewable slice with smallest credible validation.

## Required read

Load [Coding](../coding/SKILL.md) before inspecting implementation paths or editing code. This workflow covers code changes, so complete Coding's required read and apply its workflow alongside this one. Done when Coding is loaded before any code-change work begins.

## Workflow

1. Anchor work. Read selected task record, `tasks/.schema.md`, acceptance, current slice, and affected code paths. Confirm task is `status: todo`; when schema declares `scope`, also confirm `scope: agreed`. Preserve stated behavior and decisions. Done when target behavior and slice boundary are known.
2. Establish slice. Select the smallest coherent slice. When requested behavior, acceptance, and implementation approach are established, proceed without a separate slice-approval gate. Ask only when an unresolved decision materially affects behavior, scope, security, data, public API, compatibility, irreversible cleanup, or an explicit review gate. Done when one bounded slice is authorized by the request or a necessary decision is explicit.
3. Trace before edit. Apply Coding's understanding and smallest-solution workflow. Treat untraced paths and unconfirmed assumptions as delivery risk. Inspect callers, interfaces, data flow, tests, edge cases, operational impact, and local conventions needed to avoid a wrong change. Done when changed path and preserved behavior are known.
4. Implement slice. Prefer existing patterns, standard-library behavior, and small diffs; make the smallest complete change. Pause for decisions affecting behavior, scope, security, data, public API, or compatibility. Done when slice behavior is implemented without unrelated refactoring.
5. Handle replacement. When removing or replacing behavior, identify obsolete references, tests, docs, and files. Ask user for cleanup scope before broad deletion. Done when cleanup is either agreed or explicitly deferred.
6. Validate proportionately. Recommend smallest credible check for change; run only agreed or low-risk local checks. Do not run broad suites by habit. Done when validation result and remaining risk are explicit.
7. Record and report. Update task with completed work and checks. Keep task `doing` until all requested coherent work is complete; then set `review`. Report completed work, non-obvious inspection path, validation, and any remaining material decision or blocker. Do not report a routine next slice before continuing it. Done when work is complete or a necessary user action is clear.

## Output shape

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
