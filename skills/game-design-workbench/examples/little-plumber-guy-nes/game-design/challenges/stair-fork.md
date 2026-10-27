---
id: 01a16f86-6ff6-761c-95c1-8f119824e94b
name: stair-fork
created_at: 2026-10-24T20:07:23.894Z
desc: Choose exposed upper steps or a lower longer passage.
tags: []
status: accepted
entry: false
next:
  - challenges/high-stair
  - challenges/low-path
practises:
  - skill-atoms/move-right
  - skill-atoms/stop-at-edge
mechanics:
  - mechanics/solid-collision
  - mechanics/active-clock
nouns:
  - nouns/step
---
## Goal and context

Choose an exposed staircase of smaller supports or a lower, longer ground passage.

## Assistance and prior knowledge

Prior landings should make upper route legible; lower route preserves an easier alternative.

## Route

Branch to high-stair or low-path, both rejoining final-gap. No physics solver or measured time advantage establishes the route labels.
