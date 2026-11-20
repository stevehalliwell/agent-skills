---
id: 01a15175-3c00-7fb3-8cce-a0e792664984
name: combined-room
created_at: 2026-10-19T00:00:00.000Z
desc: Combined crate and switch challenge
tags: []
status: accepted
introduces:
  - skill-atoms/coordinate
practises: []
requires:
  - skill-atoms/plan-push
  - skill-atoms/route-switch
  - skill-atoms/coordinate
tests_mastery: []
entry: false
next: []
mechanics:
  - mechanics/grid-walking
  - mechanics/crate-pushing
interactions: []
nouns:
  - nouns/player
  - nouns/crate
verbs: []
resources: []
---
# Combined crate and switch challenge

Declared conceptual route only; no physical level map. Learning roles describe intent, not proof.

Player combines planning with switch routing. Intentional discovery may be appropriate; prior planning support is not yet documented.

## Decision and assessment

Contextual assessment uses [shared vocabulary](../../../../references/decision-structure.md). Intended task from concept: use finite push energy to position crate on switch and open exit. This is a puzzle/planning-test candidate combining rule knowledge and constrained actions, not a certified solvable room or observed player test.

[Crate-pushing](../mechanics/crate-pushing.md) owns push conditions, energy spending and unavailable pulling. Distinguish permitted action, successful push and progress toward local task. A blocked attempt and a successful but unhelpful push need not have same meaning. Without physical layout, no particular push can be declared an irreversible dead end or wrong puzzle answer.

Finding a feasible solution is not automatically optimization. Minimizing pushes/time needs an explicit goal; counting pushes becomes calculation only when comparison requires work. No deliberation deadline, reaction window or controller-specific dexterity demand is established. Energy constrains successful actions, not thinking duration.

Intent check: should this room test planning and prediction rather than reaction or controller dexterity? Concept favors thoughtful commitment over fast execution; layout, inputs and assistance must establish that fit. Introducing and requiring coordination in same room remains a support question, not a silent learning-role change.

```decision-assessment
id: crate-switch-plan
task: Position crate on switch to open exit within finite push energy.
context: Intended conceptual puzzle; no physical layout, certified solution or deliberation deadline supplied.
evidence_status: prediction
rule_cases:
  - "mechanics/crate-pushing#successful-push"
demands:
  - kind: planning
    intent: undecided
```
