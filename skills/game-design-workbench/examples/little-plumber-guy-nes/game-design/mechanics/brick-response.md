---
id: 01a16f86-69b5-7693-b317-808411004621
name: brick-response
created_at: 2026-10-24T20:07:22.293Z
desc: A rising head hit bumps or breaks a brick according to power state.
tags: []
status: accepted
nouns:
  - nouns/brick
  - nouns/hero
  - nouns/power-state
verbs:
  - verbs/bump
control-vars:
  - control-vars/brick-points
resources:
  - resources/run-score
---
## Rule

A rising underside hit by small form bumps but preserves brick. Tall or spark form destroys it and credits brick-points once. Other contact remains solid collision. Ordinary bricks contain no reward.

## Feedback

A bump and actual break must look distinct. Removing geometry changes available landing support; destruction is not equivalent to collecting a pickup.
