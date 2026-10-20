---
id: 01a15175-3c00-7115-8cde-8cb37cfe1ff1
name: crate-pushing
created_at: 2026-10-19T00:00:00.000Z
desc: Crate pushing rule
tags: []
status: accepted
nouns:
  - nouns/player
  - nouns/crate
  - nouns/wall
verbs:
  - verbs/push
controls:
  - controls/push-duration
  - controls/push-cost
resources:
  - resources/player-energy
---
# Crate pushing

A successful push requires a free destination and energy at least push-cost. Move crate one cell, then spend push-cost. Wall-blocked or insufficient-energy pushes move nothing and cost nothing. Pulling is unavailable.
