---
id: 01a16f86-69b6-7fc4-917f-09fab5c850e0
name: earned-life
created_at: 2026-10-24T20:07:22.294Z
desc: Coin threshold or life-cap grants a life up to capacity.
tags: []
status: accepted
nouns:
  - nouns/coin-bank
  - nouns/life-cap
  - nouns/lives
  - nouns/hero
verbs:
  - verbs/touch
control-vars:
  - control-vars/coins-for-life
  - control-vars/life-capacity
resources:
  - resources/coin-progress
  - resources/retry-budget
---
## Rule

On coin threshold, subtract coins-for-life and add a life, clamped at life-capacity. Consume life-cap pickup for the same capped award without changing coin bank. At capacity, award is lost but token conversion/pickup consumption still occurs. No score-based life rule.

## Feedback

Show life count change when one occurs; capacity must not wrap to zero.
