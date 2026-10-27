---
id: 01a16f86-69b6-73fb-aed4-6db9b5204968
name: score-awards
created_at: 2026-10-24T20:07:22.294Z
desc: Fixed awards reward events; no hidden chain multiplier or spending.
tags: []
status: accepted
nouns:
  - nouns/score
  - nouns/coin
  - nouns/brick
  - nouns/walker
  - nouns/shellback
control-vars:
  - control-vars/coin-points
  - control-vars/brick-points
  - control-vars/walker-points
  - control-vars/shellback-points
resources:
  - resources/run-score
---
## Rule

Apply named fixed point awards for coin credit, brick destruction and original enemy defeat/retraction. Credit each event once. Shell stops and kicks do not farm the original enemy award. Score survives retries but cannot be spent and grants no lives.

## Boundary

This is an authored feedback economy, not a player-skill or enjoyment metric.
