---
name: task-refinement
description: "Scope a task, flesh out a feature, define behavior or acceptance checks, or refine how work should function when the user asks; also use when a clear task goal has vague implementation shape and multiple plausible local approaches. Establish a shared technical contract and completed result with code/artifact examples, boundaries, and observable acceptance. Skip todo review, triage, prioritization, status checks, task selection, and routine implementation with an established approach."
---

# Task Refinement

Make the result and technical contract concrete enough to review and implement without choosing unresolved product behavior. Return to the calling workflow afterward; do not modify implementation code.

## Trigger clarification

Use for explicit refinement requests, or when a clear goal leaves multiple plausible approaches affecting behavior, acceptance, interfaces, data, security, compatibility, or substantial rework. Unknown file order or routine implementation details alone do not qualify.

Skip task selection, status review, and implementation with an established approach. Use `tradeoff-review` for larger architecture, cross-feature direction, durable API/data choices, or migration/reversibility trade-offs.

Done: refinement is needed, or skip with a clear reason.

## Workflow

1. Anchor the task.
   - Standalone entry: `Task refinement start. Current work resumes after shared task shape.`
   - When called by [Refine the backlog](../refine-the-backlog/SKILL.md), retain its selected item, persisted implementation-ready intent, and return context. Do not select another task here.
   - Done: selected task, persistence intent, and return context are known.

2. Ground the contract.
   - If `.pi/attendant.tables` configures tasks, load [Attendant](../attendant/SKILL.md), run `schema`, and read the configured schema, usage, body template, and selected record. Locate records with `query`/`search`; follow collection readiness and transition rules. Resolve material missing/conflicting guidance rather than importing defaults.
   - Otherwise use the user's request. Inspect related code, dependencies, and decisions enough to distinguish facts from assumptions and identify preserved behavior.
   - Done: source requirements and relevant constraints are known.

3. Shape one reviewable outcome.
   - Lead with the completed result and its point. For technical tasks, specify important interfaces, code/data flow, and constraints—not just eventual user value.
   - Use task-type examples below. Add prose only for contracts artifacts cannot express. Include ownership, lifetime, capacity, failure/empty behavior, and preserved behavior where relevant.
   - State non-goals where plausible adjacent work could be inferred. Record rejected alternatives only when rationale affects delivery. Split oversized work into independent outcomes; record dependencies and sequencing where material.
   - Acceptance references the contract/examples with observable checks and expected results. A passing test command alone is not acceptance; actual evidence belongs under Checks.
   - Done: result, technical shape, examples, boundaries, and acceptance describe one deliverable.

4. Present and resolve.
   - Show the result, decisive technical example, and relevant exclusions—not only a saved path or plan. Use concise, structured documentation, not a checklist dump.
   - Label recommendations `Proposed` and material unknowns `Open`; use `Confirmed` only where distinction helps. Do not duplicate the specification under these labels.
   - Ask one material decision at a time; state a proposed default. Reuse supplied answers, which settle their detail unless tentative. `TBD` does not authorize inventing behavior or signatures. Do not add a final approval gate for settled requirements.
   - Done: user has a reviewable picture; remaining decisions are explicit.

5. Persist and check.
   - Save resolved answers immediately and continue refinement in the same turn. Update each fact in its canonical section, not an answer log. For discussion-only requests, do not write files.
   - Omit irrelevant headings and empty placeholders; keep material unknowns under Open questions. One final Comments section holds errata and incidental notes. Agreed technical choices belong in the specification, not a separate Notes section.
   - Compare presentation and record: could materially different implementations satisfy them? Do examples depend on unagreed behavior? Is a code-facing contract described only in prose? Correct gaps and recheck; keep blocked uncertainty explicit. No identified questions alone does not establish readiness.
   - If implementation-ready work was requested and checks pass, apply the collection's documented ready transition. Use only declared schema values; report unspecified/conflicting transitions. Do not set implementation-active state or alter unrelated task state.
   - Done: saved detail matches shared understanding; ready state is justified or blocker is explicit.

6. Return.
   - Resolved: `Task refined: <summary>. Saved: <path or discussion only>. Next: return to calling workflow or prior work.`
   - Unresolved: `Task refinement open: <decision/blocker>. Saved: <path or discussion only>. Next: <required decision or action>.`
   - Return resolved items immediately. Retain unresolved items; callers must not advance past them. Refinement alone does not authorize implementation.
   - Done: caller can resume or work waits on a specific decision/blocker.

## Match examples to task type

Use applicable branches, not a fixed quota. Combine types when needed. Label unsettled interfaces/behavior Proposed or Open; do not invent an API to fill a heading.

- Library/API/framework: declarations and representative caller code; storage/ownership and error handling where relevant. Show what callers write, not only what users gain.
- Refactor/internal tooling: before/after code, generated artifact, or flow; duplication removed and behavior retained. Private helper names need not be fixed.
- CLI/build: invocation, relevant stdout/stderr or generated files, exit status, and a material failure case.
- Data/configuration/storage: schema or records, read/write/transformation result, and relevant validation/persistence rules.
- UI/game: initial state, interaction/input, resulting visible state; sketches, values, or transitions where useful.
- Bug/performance: reproduction/workload, current versus expected behavior, verification/measurement method. Do not invent measured results.
- Writing/research: deliverable structure and representative passage or evidence/output where useful; do not force code.

### Example: shared generator refactor

After ordering and boundaries are agreed:

**Result/point:** test and benchmark generators share collection/emission logic, removing duplicate scanning without coupling standalone `build.h` to `base.h`.

**Artifact:** directory input `fixtures/a.h`, `fixtures/z.c`, and `fixtures/nested/b.c` produces this runner excerpt:

```c
#include "fixtures/a.h"
#include "fixtures/z.c"

/* In generated benchmark runner: */
bench_first();
bench_second();
```

**Contract:** immediate files sorted together; explicit inputs keep caller order. Separate runner emitters remain. Collection uses caller storage; exhaustion fails without growth. No recursive discovery or general parser framework.

**Acceptance:** compare fixture includes/call order; test explicit order and exhaustion; compile standalone `build.h` without `base.h`.

The artifact replaces paragraphs describing emitted C. Acceptance references the contract; it does not repeat it. Example snippets specify behavior, not permission to implement an entire API.
