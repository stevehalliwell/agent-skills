---
id: 01a15175-3c00-7a58-803c-f7e2cfabbd6f
name: grid-walking
created_at: 2026-10-19T00:00:00.000Z
desc: Grid walking rule
tags: []
status: accepted
nouns:
  - nouns/player
  - nouns/wall
verbs:
  - verbs/walk
controls:
  - controls/tile-distance
  - controls/walk-speed
  - controls/walk-duration
resources: []
---
# Grid walking

Player walks into free adjacent cells; walls block movement. Walking remains possible at zero energy. Walk duration references canonical controls.
