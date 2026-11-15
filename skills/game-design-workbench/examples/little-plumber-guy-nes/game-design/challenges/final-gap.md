---
id: 01a16f86-6ff6-7ad5-bcb1-6ac085978d4d
name: final-gap
created_at: 2026-10-24T20:07:23.894Z
desc: Last floor break combines timing, edge reading and landing.
tags: []
status: accepted
entry: false
next:
  - challenges/finish
requires:
  - skill-atoms/jump
  - skill-atoms/land-on-target
  - skill-atoms/manage-clock
tests_mastery:
  - skill-atoms/finish-approach
mechanics:
  - mechanics/jump-launch
  - mechanics/death-and-retry
nouns:
  - nouns/gap
---
## Goal and context

A last visible floor break separates approach from finish. Combine launch, landing and remaining-time awareness.

## Assistance and prior knowledge

Prerequisite opportunities exist earlier, but high-stair is optional and finish-approach has no prior explicit introduction. These are review prompts, not proof the design is impossible.

## Decision and assessment

Contextual assessment uses [shared vocabulary](../../../../references/decision-structure.md). Task: cross last floor break alive on approach to finish. Crossing successfully satisfies this local task; lethal falling fails it. Reaching finish post remains a separate course-success event.

Composite test combines launch/landing execution, edge reading and remaining-time awareness. Existing mastery target describes intended assessment, not observed mastery. Preparation opportunities differ by route; high-stair is optional and finish-approach has no prior explicit introduction. Neither fact alone proves player incompetence or impossible design.

Approach deliberation, remaining clock and jump-execution window are different timing facts. No numeric response window or specific dexterity difficulty is supplied. Correctly identifying response does not prove ability to perform it.

Intent check: is this intended to test a prepared combination of skills or discovery under pressure? Review route-specific support before deciding whether reaction/dexterity demands serve intended final assessment.

```decision-assessment
id: final-crossing
task: Cross last floor break alive before course finish.
context: Combine landing and remaining-time awareness; preparation varies by route and precise response windows are unknown.
evidence_status: authored-assessment
rule_cases: []
demands:
  - kind: knowledge
    intent: undecided
  - kind: execution
    intent: undecided
```

## Route

Successful crossing leads to finish. Retry state and exact jump geometry remain outside this route graph.
