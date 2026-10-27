---
id: 01a16f86-69b5-7b2b-94b0-04c5d70a5457
name: spark-impact
created_at: 2026-10-24T20:07:22.293Z
desc: Sparks defeat walking enemies and vanish on walls or spent range.
tags: []
status: accepted
nouns:
  - nouns/spark
  - nouns/walker
  - nouns/shellback
  - nouns/pipe
verbs:
  - verbs/touch
control-vars:
  - control-vars/walker-points
  - control-vars/shellback-points
resources:
  - resources/run-score
---
## Rule

Hero spark defeats walker or walking shellback, grants that enemy's fixed award and is consumed. Sparked shellback leaves no resting shell. Wall contact or leaving active viewport removes a spark; floor contact bounces it. Spark versus existing shell remains unspecified.

## Feedback

Defeat and projectile removal should be visible; these rules do not imply a complete collision table.
