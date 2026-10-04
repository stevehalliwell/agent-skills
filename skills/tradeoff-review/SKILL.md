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
   - Load [Attendant](../attendant/SKILL.md) when configured, run `schema`, and read the decisions collection's schema and usage guidance before querying or relying on records. Follow its approval, supersession, and revisit rules; resolve material ambiguity when usage is missing or contradicts the schema.
   - If `revisit_triggers` is declared and documented, derive likely tags from conditions under discussion, then use Attendant `query` for matching records before framing options. Otherwise use targeted `search` without inventing fields. For one normalized tag: `SELECT 'decisions' AS collection, id, name, source_path, status, revisit_triggers FROM decisions WHERE revisit_triggers LIKE :trigger_pattern`, with `trigger_pattern` set to `%"<tag>"%`.
   - Read matching records for conflicting decisions, accepted costs, guardrails, and reopen conditions.
   - Done when facts, related decisions, and material unknowns are explicit.
3. Frame decision: viable options, including defer/smallest credible path; benefits, costs, risks, affected areas, future consequences.
   - Done when user can weigh meaningful trade-offs.
4. Ask user to decide.
   - Recommendation remains proposed until confirmed.
   - Done when decision, guardrails, or blocker is explicit.
5. Record agreed decision.
   - Require configured `decisions` collection. If absent, route to `/skill:init-project` or `/skill:attendant` empty-collection workflow.
   - Apply the collection's documented agreed-decision state and revisit-trigger conventions using only declared fields. Use Attendant `create -c decisions -i <items-json>` with one `{ "name": "<safe-slug>", "fields": <declared-fields> }` item, or `update` for an existing proposal. Adapt the configured collection's `.template.md` when present; record context, choice, options, consequences, affected areas, guardrails, and concrete revisit conditions.
   - If this replaces an existing decision, follow usage guidance for preserving history and linking its replacement. Read saved records against the agreement, correct inaccuracies, and recheck; report unresolved save or schema blockers.
   - Done when saved source paths accurately reflect agreement and any replacement history is verified.
6. Exit.
   - After a verified save, say: `Design decision recorded: <path>. Decision: <choice>. Trade-off: <cost>. Next: return to prior work or request implementation.`
   - If deferred, cancelled, blocked, or unable to save, say: `Trade-off review unresolved: <reason>. Next: <required decision, record repair, or independent prior work>.` Do not resume work that depends on the unresolved choice or claim a missing record was saved. Other independent work may resume if the user chooses it.
   - Done when the review is closed with a saved decision or a specific unresolved gate.

## Rules

- Do not implement or modify code in this mode.
- Preserve rejected alternatives and rationale.
