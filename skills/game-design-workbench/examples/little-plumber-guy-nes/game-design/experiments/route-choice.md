---
id: 01a16f86-7803-74a0-a260-abf921f92717
name: route-choice
created_at: 2026-10-24T20:07:25.955Z
desc: Proposed playtest of optional route discovery and time/reward trade-offs.
tags: []
status: proposed
stage: proposed
experiment_kind: playtest
mechanics:
  - mechanics/pipe-transfer
  - mechanics/active-clock
control-vars:
  - control-vars/opening-clock
  - control-vars/clock-runs-in-cellar
dynamics:
  - dynamics/reward-detours
  - dynamics/optional-practice-bypass
  - dynamics/clock-tradeoff
aesthetics:
  - aesthetics/curiosity
  - aesthetics/manageable-tension
---
## Question

Do players discover optional cellar and flower routes, and what do they say influenced choosing or skipping them?

## Hypotheses

Reward visibility and shared time budget may affect choices. Neither preferred route nor choice probability is assumed.

## Local measures

- entered-cellar: boolean, unitless, whether declared entrance is used during observed attempt.
- route-choice-reason: enum, unitless, reward/time/safety/curiosity/other/unknown, coded from participant explanation, not inferred from path alone.
- remaining-time-at-finish: number, s, course clock when finish commits; undefined if finish is not reached or observation is lost.

## Method and expectations

Proposed observed play with optional post-attempt questions. Keep every attempted route, failure and undefined finish value. Separate prior familiarity from discovered affordance. No minimum discovery rate or preferred choice supplied; refine criteria before execution.

## Observations and results

None. No participant data or probability model exists. Opening this record does not run a test.

## Provenance and limits

Current authored route graph describes bypass opportunities but omits alignment, power and retry predicates. Behavioral meaning requires actual context; a path count alone cannot establish intention or feelings.
