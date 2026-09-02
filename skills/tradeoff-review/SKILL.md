---
name: tradeoff-review
description: "Pause for a team-style design discussion when a choice has material trade-offs across system parts, future features, operations, compatibility, security, performance, or cost—or agent detects one before coding. Compare options, get user decision, record agreed durable decision in Attendant; do not change code."
---

# Trade-off Review

Pause before durable system decision. Agreed decisions become Attendant records.

## Trigger clarification

Use for consequential architecture, framework, API, data model, schema, migration, security, operations, or dependency choices. Skip routine/reversible local choices; use `task-refinement` for local executable task shape.

## Workflow

1. Anchor current work.
   - Say: `Trade-off review start. Current work resumes after decision record.`
   - Done when pending decision is explicit.
2. Ground discussion in existing code, records, docs, constraints, reversibility, and affected areas.
   - If Attendant `decisions` exists, derive likely revisit-trigger tags from conditions under discussion, then use `/skill:attendant` `query` for matching records before framing options. For one normalized tag: `SELECT 'decisions' AS collection, id, name, source_path, status, revisit_triggers FROM decisions WHERE revisit_triggers LIKE :trigger_pattern`, with `trigger_pattern` set to `%"<tag>"%`.
   - Read matching records for conflicting decisions, accepted costs, guardrails, and reopen conditions.
   - Done when facts, related decisions, and material unknowns are explicit.
3. Frame decision: viable options, including defer/smallest credible path; benefits, costs, risks, affected areas, future consequences.
   - Done when user can weigh meaningful trade-offs.
4. Ask user to decide.
   - Recommendation remains proposed until confirmed.
   - Done when decision, guardrails, or blocker is explicit.
5. Record agreed decision.
   - Require configured `decisions` collection. If absent, route to `/skill:init-project` or `/skill:attendant` empty-collection workflow.
   - Derive one or more stable kebab-case `revisit_triggers` from conditions that should reopen this decision; use `/skill:attendant` `create -c decisions -i <items-json>` with one `{ "name": "<safe-slug>", "fields": <declared-fields> }` item including `status: accepted`. Fill record body from `../init-project/templates/records/decisions/.template.md`, explaining each trigger alongside context, decision, options, consequences, affected areas, and guardrails.
   - Done when saved source path accurately reflects agreement.
6. Exit.
   - Say: `Design decision recorded: <path>. Decision: <choice>. Trade-off: <cost>. Next: return to prior work or request implementation.`

## Rules

- Do not implement or modify code in this mode.
- Preserve rejected alternatives and rationale.
