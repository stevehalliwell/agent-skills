---
name: implementation
description: "Use when implementing an agreed task, coding an approved change, fixing a scoped bug, delivering the next reviewable slice, or listing implementation-ready tasks when no focus is supplied. Trace affected code enough to avoid wrong edits, implement one slice, run proportionate validation, and report result. Do not use for unclear scope, architecture trade-offs, backlog shaping, or pre-code feasibility review."
---

# Implementation

Deliver one agreed, reviewable slice with smallest credible validation.

## Focus check

When no clear implementation focus is supplied or selected, load and follow [Find implementation-ready tasks](ready-tasks.md) instead of this implementation workflow. Done when a ready-task report recommends the highest-priority ready task, with quick wins and other factors secondary, or reports that no ready work exists.

## Required read

Load [Coding](../coding/SKILL.md) before tracing or editing. Coding owns engineering method and verification; this skill owns acceptance, delivery, and tracking. Done when Coding is loaded.

## Workflow

1. Anchor work. Use the agreed request as the behavior and acceptance source. When `.pi/attendant.tables` configures `tasks`, load [Attendant](../attendant/SKILL.md), run `schema`, and read the configured task schema, usage guidance, and selected record. Follow the collection's documented readiness, active-work limits, approval gates, and transitions; start ready work in its implementation-active state and resume authorized active work without resetting it. Resolve material ambiguity when usage is missing or contradicts the schema. Do not start unready or blocked work. Without task storage, proceed from the agreed request without creating a tracker. Preserve stated behavior and decisions. Done when target behavior, tracking state, and slice boundary are known.
2. Bound the slice. Establish goal, exclusions, expected result, and acceptance; select the smallest coherent slice. Keep the technical contract and representative code/artifacts in the existing task record. Proceed within agreed scope; follow global decision and approval rules. Done when one bounded slice is authorized or a necessary decision is explicit.
3. Trace before edit. Follow Coding's trace-and-select workflow against this slice's acceptance and preserved behavior. Done when the change point and approach are known.
4. Implement slice. Follow Coding's change-and-cleanup workflow. For product feedback, change the implementation or artifact first; return the relevant build, test result, preview, or reproduction, not a revised report. When feedback targets the report itself, edit it directly. Done when the requested change is available for assessment.
5. Handle delivery effects. Keep task documentation and acceptance aligned with agreed changes. Leave broader cleanup untouched unless authorized under global rules. Done when delivery effects are addressed or explicitly deferred.
6. Check acceptance. Apply Coding's verification workflow, then compare the delivered change against agreed acceptance and exclusions. A passing check proves only what it exercised; name unmet or unverified conditions. Done when acceptance is supported by observed results or gaps are explicit.
7. Record and report. Update existing task progress, contracts, and check evidence at meaningful checkpoints or when requested, not after every feedback exchange. Follow collection usage for layout and lifecycle; keep work active while coherent work remains, and use review state only when requested work is complete. Without tracking, report directly. Done when completion or necessary user action is clear.

## Output shape

When orientation is needed under global guidance, state intended behavior, scope boundary, and the result the user can inspect. For example:

> Add CSV export for the currently filtered tasks, in displayed order. The download uses the agreed columns; filters and task data stay unchanged, and column selection is out of scope. Details: `records/tasks/export-filtered-tasks.md`.

Explain important interface or flow changes, not only user value. Link the existing task record for contract details rather than copying it; do not invent a tracker. When resume context is needed, state goal, expected result, and remaining work.

At completion or a blocker, lead with outcome and include relevant evidence, remaining risk, and status. Mention non-obvious inspection paths only when useful; omit empty categories.

## Rules

- Implement only agreed scope.
- Continue through coherent requested work in the same turn. Stop after a completed slice only when user review is explicitly requested or a material decision or blocker requires it.
- Keep assumptions distinct from agreed acceptance.
