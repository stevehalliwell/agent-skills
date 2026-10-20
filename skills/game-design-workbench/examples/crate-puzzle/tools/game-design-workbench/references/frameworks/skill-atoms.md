# Skill atoms and learning chains

Pack: `skill-atoms` · version 1
Attribution: Daniel Cook, The Chemistry of Game Design (original article; 2021 repost)

## Contents

- [Scope](#scope)
- [Inputs and query support](#inputs-and-query-support)
- [Working concepts](#working-concepts)
- [Apply to selected slice](#apply-to-selected-slice)
- [Original prompts](#original-prompts)
- [Worked example](#worked-example)
- [Evidence and counterchecks](#evidence-and-counterchecks)
- [Common traps](#common-traps)
- [Missing data and evidence limits](#missing-data-and-evidence-limits)
- [Sources](#sources)

## Scope

Inspect a selected learned interaction, its prerequisite chain, authored progression rank, feedback, and challenge opportunities.

Use only when this lens is selected. Follow [review operation](../review.md); do not repair source during review.

## Inputs and query support

Roles: `skills`, `challenges`, `levels`, `mechanics`. Read actual role mapping and live schema/usage before writing.

Available recipes: `query skills`; `query routes`; `query records --collection <mapped-alias>`. These are declared-data projections, not automated framework verdicts. Inspect linked bodies for conditions and evidence.

## Working concepts

- **Learning loop:** Connect an attempted action, the result and feedback, and a revised understanding. Describe what the player can learn from the difference between intention and outcome, not just what button is pressed.
- **Skill and evidence:** A skill record describes an intended understanding or expression. Success once may come from chance, copying, or an already known strategy. Repeated explanation and transfer are possible evidence, not automatic requirements or proof.
- **Prerequisite and rank:** A prerequisite declares an AND dependency in this model. Rank is optional authored conceptual placement. Neither graph depth nor room order determines it, and a null rank must not become zero.
- **Opportunity and mastery:** A challenge can offer introduction, practice, combination, or transfer. An opportunity on a declared route does not establish that a player used it or acquired understanding.

## Apply to selected slice

1. Select one skill or challenge and read its stated understanding, expression, feedback, and evidence sought. Name the learned relationship under review; avoid turning every motor action into a separate mandatory skill.
2. Inspect immediate prerequisites and relevant declared routes. Keep AND dependencies separate from alternative challenge links. Read descriptions before deciding whether the links represent genuine conceptual needs.
3. Compare authored ranks without filling nulls or computing replacement values. A prerequisite placed later is a question about intent, not an instruction to reorder the graph.
4. Trace where the selected skill could be introduced, practised, combined, and used in a changed context. Mark undocumented opportunities and bypasses without declaring those routes impossible.
5. Return the specific loop, dependency question, and evidence gap. If an edit is explicitly requested, change only the selected authored field. Do not propagate ranks, infer mastered skills, or repair unrelated routes.

## Original prompts

1. What choice or action expresses the skill, and what feedback lets the learner update a mental model?
2. What would distinguish understanding from merely succeeding once?
3. Which prerequisites are genuinely needed together, and which are only conventional ordering?
4. Where can the skill be introduced, practised, combined, and transferred under changed conditions?
5. Does an authored rank disagree with prerequisites? Is that intentional, unresolved, or only a display concern?
6. Which declared route offers a learning opportunity, and which route bypasses it?

## Worked example

- **Authored facts:** Coordinate has rank 2 and AND prerequisites plan-push and route-switch. Route-switch has rank 3; plan-push is unranked. Independent read-switch remains a root at rank 3.
- **Interpretation:** The coordinate/route-switch ordering invites clarification. It may be intentional conceptual grouping, an unresolved ordering choice, or a mismatch; the declared numbers do not explain the reason.
- **Authorized edit example:** If the designer requests coordinate rank 3, change that field only. Preserve its identity, body, prerequisites, plan-push null, and both existing switch-chain rank values.
- **Useful next question:** What should a player understand after combining push planning and switch routing, and what feedback reveals that relationship? That learning question remains even after a rank edit.

## Evidence and counterchecks

- Distinguish completing one instance, explaining a rule, repeating a choice, and transferring understanding to a changed situation. Each supports a different claim.
- Inspect the bypass route as an opportunity question. A player might arrive with prior knowledge; a missing practice record cannot establish incapability.
- Native cycle, rank-conflict, and all-route checks are deferred. Manual tracing should state inspected nodes and routes rather than imply exhaustive graph coverage.

## Common traps

- Do not infer rank from prerequisite depth. Late roots and unranked skills are valid authored states.
- Do not treat AND prerequisites as interchangeable routes. All declared prerequisite links matter to the conceptual claim.
- Fixing one rank does not validate the learning design. Keep structural editing separate from evidence of understanding.

## Missing data and evidence limits

Keep unknown ranks unknown and late roots visible. Optional levels are not required for route discussion. No teaching requirement or learner incapability claim follows from missing practice records.

Skills use AND prerequisites; absent/null/empty arrays declare no links. Rank is nullable signed conceptual placement, not physical level or graph depth. Recipes expose links/ranks/routes; cycle, rank-conflict, and all-route opportunity checks remain manual until C99 analysis ships. Opportunity does not prove acquired mastery.

## Sources

- [The Chemistry of Game Design](https://lostgarden.com/2021/03/13/the-chemistry-of-game-design-2/)

Access checked 2026-10-19: Lostgarden article inspected, including action/rule/feedback/mental-model loop. Questions independently authored.

Guidance is independently authored. No book, proprietary deck, questionnaire, or scoring method is reproduced.
