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

## Route

Successful crossing leads to finish. Retry state and exact jump geometry remain outside this route graph.
