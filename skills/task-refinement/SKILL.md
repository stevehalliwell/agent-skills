---
name: task-refinement
description: "Scope a task, flesh out a feature, define behavior or acceptance checks, or refine how work should function when the user asks; also use when a clear task goal has vague implementation shape and multiple plausible local approaches. Establish a shared completed end state with worked examples, explicit non-goals, technical shape, and observable acceptance. Skip todo review, triage, prioritization, status checks, task selection, and routine implementation with an established approach."
---

# Task Refinement

Make the completed result concrete enough that the user can recognize what they will receive and an implementer can deliver it without choosing unresolved product behavior, then return to the calling workflow or prior work.

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
   - For a standalone invocation, say: `Task refinement start. Current work resumes after shared task shape.`
   - When called by [Refine the backlog](../refine-the-backlog/SKILL.md), retain its selected item and return context. That caller requests persisted, implementation-ready refinement; do not select another task or resume implementation here.
   - This mode may inspect project context, discuss task shape, and update the task record as user input resolves details. It never modifies implementation code.
   - Done when the selected task, persistence intent, and return context are explicit.

2. Ground in existing context.
   - When `.pi/attendant.tables` configures `tasks`, load [Attendant](../attendant/SKILL.md), run `schema`, and read the configured task schema, usage guidance, body template when present, and selected source record; use Attendant `search`/`query` to locate that record when needed. Follow usage for readiness and state transitions; resolve material ambiguity when usage is missing or contradicts the schema.
   - If no record exists, use the user's message as source.
   - Inspect related records, existing behavior, dependencies, and adjacent work as needed to ground task shape. Separate confirmed facts from assumptions and identify behavior to preserve.
   - Done when source text, selected item, and relevant constraints are known.

3. Identify missing outcome and implementation detail.
   - Check for: completed end state, worked examples, user/business value, in-scope behavior, explicit non-goals, behavior to preserve, affected code/data flows, technical approach, alternatives ruled out, acceptance checks, open questions.
   - Keep one independently reviewable outcome per task. Identify split points for oversized work and make sequencing, dependencies, opportunity cost, and speculative complexity explicit where they affect the boundary.
   - Done when gaps and any necessary split points are explicit.

4. Reflect understanding, not checklist results.
   - Separate:
     - Confirmed: explicit user requirements or existing behavior grounded in project context.
     - Proposed: agent recommendations requiring user agreement.
     - Open: unknowns that could change behavior, boundaries, or acceptance.
   - Within those groups cover:
     - What it is: the completed result, including what someone can see, do, or receive after the whole task, not merely the next implementation slice.
     - Worked examples: concrete starting conditions, action/input, and exact expected output or resulting state. Cover the main outcome and materially distinct failure, empty, or compatibility cases where relevant; use enough examples to expose choices, not a fixed quota. For internal work, show observable before/after behavior or artifacts. Label unresolved behavior as Proposed or Open rather than inventing it to complete an example.
     - What it is not: explicit non-goals and tempting adjacent work, including behavior the examples might otherwise imply.
     - Technical detail: affected files/flows/interfaces, data shape, edge cases, constraints, expected sequence.
     - Alternatives ruled out: option, why rejected, revisit trigger if useful.
     - Acceptance: observable checks tied to the promised end state and examples, with expected results and a command or manual verification where useful. A passing test command alone does not define acceptance; actual validation evidence belongs to implementation.
   - Keep broad system trade-offs in `tradeoff-review`; keep local implementation detail here.
   - Present the end state, worked examples, and exclusions to the user as a coherent explanation, not only a saved path or technical plan. Reuse already supplied requirements and answers; do not add a final approval gate for settled details.
   - Done when the user has a reviewable picture of what the task is and is not, and the recorded technical shape supports that result.

5. Resolve and record open questions.
   - Ask specific decision questions only when choices change task shape or acceptance.
   - When the user supplies or corrects requested detail, apply the persistence rule in step 6 and continue with the next unresolved material question or refinement step.
   - Treat a direct answer as agreement on the answered detail unless the user marks it tentative, asks for discussion, or a material decision remains open.
   - Keep unknown product behavior open. `TBD` records unresolved detail; it does not authorize choosing behavior.
   - Agent may recommend a default only when clearly labelled `Proposed`.
   - Done when every resolved detail is recorded and remaining uncertainty is visible.

6. Keep task record current.
   - Immediately save confirmed facts, resolved user answers, proposals, and remaining open questions to the task source record as refinement proceeds; do not restate settled detail merely to seek final confirmation.
   - Before declaring refinement complete, check the presented explanation and saved record against the end state, examples, exclusions, preserved behavior, and acceptance. Could materially different results satisfy the wording? Are any examples dependent on unagreed product behavior? Correct gaps, resolve material questions through user input, and recheck. If blocked, keep the uncertainty explicit and do not mark ready; absence of questions is not evidence of clarity.
   - When that check passes and the user or caller requested implementation-ready work, apply the ready-state transition documented in collection usage, using only schema-declared fields and values. Report a blocker when the transition is unspecified or conflicts with the current state. Do not set the implementation-active state or change unrelated task state.
   - If the user only wants discussion, do not write files.
   - Done when saved task detail reflects current shared understanding or discussion-only scope is explicit.

7. Exit refinement mode.
   - Use one explicit outcome:
     - `Task refined: <summary>. Saved: <task path, or discussion only>. Next: return to calling workflow or prior work.`
     - `Task refinement open: <specific decisions or blocker>. Saved: <task path, or discussion only>. Next: ask the next material question or resolve the blocker.`
   - Return a resolved item to its caller immediately. For an unresolved item, retain the selected item and record the material decision, blocker, or explicit pause; the caller must not advance past it.
   - Do not imply implementation authorization from refinement discussion alone.
   - Done when refinement discussion is closed or waiting on a specific user decision.

## Output shape

Show the completed end state, worked examples, and explicit exclusions before closing refinement. Include technical shape, acceptance, and confirmed/proposed/open distinctions where relevant. Routine updates may be brief; neither the user-facing explanation nor the durable record may omit required content to shorten the reply. Use the exit phrases in step 7; for discussion-only work, report `Saved: discussion only`.

### Example: from headline to shared end state

Input: `Export filtered tasks to CSV.`

Insufficient: `Add an export button and CSV generation; tests pass.` This describes work, not the agreed result.

After the user has supplied or agreed these details:

- End state: Export downloads the tasks matching the active filters in their displayed order, with columns `title,status` and filename `tasks.csv`.
- Worked example: Given displayed tasks `Fix login` (`todo`) and `Update docs` (`doing`), with `Old task` excluded by the filters, clicking Export downloads `tasks.csv` containing:

  ```csv
  title,status
  Fix login,todo
  Update docs,doing
  ```

- Empty case: With no matching tasks, Export downloads a header-only CSV containing `title,status`.
- Not included: Exporting excluded tasks, choosing columns, or changing filters or task data.
- Acceptance: Check the downloaded filename, headers, values, ordering, excluded row, empty case, and unchanged filters/task data against these examples.

These are illustrative agreed requirements, not defaults for other tasks. If filename, columns, ordering, empty behavior, or relevant CSV escaping rules remain unknown, record and resolve them rather than silently choosing them.

## Rules

- Brevity compresses wording, not required content.
- Do not begin implementation while feature boundary or technical shape is vague.
- Do not dump private checklist output to files; turn it into user-reviewable understanding first.
- Use `tradeoff-review`, not this skill, for larger design direction, project priorities, cross-feature ramifications, or architectural trade-offs.
- Do not invent product decisions silently; label assumptions.
- Prefer concrete examples and clear prose over abstract claims of readiness.
