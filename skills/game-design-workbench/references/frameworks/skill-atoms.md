# Skill atoms and learning chains

Pack: `skill-atoms` · version 1
Attribution: Daniel Cook, The Chemistry of Game Design (original article; 2021 repost)

## Scope

Inspect a selected learned interaction, its prerequisite chain, authored progression rank, feedback, and challenge opportunities.

Use only when this lens is selected. Follow [review operation](../review.md); do not repair source during review.

## Inputs and query support

Roles: `skills`, `challenges`, `levels`, `mechanics`. Read actual role mapping and live schema/usage before writing.

Available recipes: `query skills`; `query routes`; `query records --collection <mapped-alias>`. These are declared-data projections, not automated framework verdicts. Inspect linked bodies for conditions and evidence.

## Original prompts

1. What choice or action expresses the skill, and what feedback lets the learner update a mental model?
2. What would distinguish understanding from merely succeeding once?
3. Which prerequisites are genuinely needed together, and which are only conventional ordering?
4. Where can the skill be introduced, practised, combined, and transferred under changed conditions?
5. Does an authored rank disagree with prerequisites? Is that intentional, unresolved, or only a display concern?
6. Which declared route offers a learning opportunity, and which route bypasses it?

## Missing data and evidence limits

Keep unknown ranks unknown and late roots visible. Optional levels are not required for route discussion. No teaching requirement or learner incapability claim follows from missing practice records.

Skills use AND prerequisites; absent/null/empty arrays declare no links. Rank is nullable signed conceptual placement, not physical level or graph depth. Recipes expose links/ranks/routes; cycle, rank-conflict, and all-route opportunity checks remain manual until C99 analysis ships. Opportunity does not prove acquired mastery.

## Fixture application

Coordinate is authored rank 2 but depends on route-switch rank 3 and unranked plan-push. A requested edit to coordinate rank 3 changes only that field; descendants are not rewritten.

## Sources

- [The Chemistry of Game Design](https://lostgarden.com/2021/03/13/the-chemistry-of-game-design-2/)

Access checked 2026-10-19: Lostgarden article inspected, including action/rule/feedback/mental-model loop. Questions independently authored.

Guidance is independently authored. No book, proprietary deck, questionnaire, or scoring method is reproduced.
