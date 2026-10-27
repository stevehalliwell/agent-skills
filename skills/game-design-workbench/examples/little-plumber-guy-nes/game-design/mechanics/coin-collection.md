---
id: 01a16f86-69b5-79c4-b2e0-8cd155e521bd
name: coin-collection
created_at: 2026-10-24T20:07:22.293Z
desc: Collecting a token updates coin bank and score exactly once.
tags: []
status: accepted
nouns:
  - nouns/coin
  - nouns/coin-bank
  - nouns/score
  - nouns/hero
verbs:
  - verbs/touch
control-vars:
  - control-vars/coin-points
resources:
  - resources/coin-progress
  - resources/run-score
---
## Rule

Hero collection removes a world coin or consumes one direct block payout, increments coin bank and adds coin-points to score. Apply earned-life conversion immediately if threshold is reached. A consumed instance cannot pay twice.

## Feedback

HUD bank and score change. Collection does not restore clock, grow hero or protect against enemies.
