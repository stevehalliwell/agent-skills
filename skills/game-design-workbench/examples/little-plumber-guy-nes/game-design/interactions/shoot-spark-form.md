---
id: 01a16f86-7bec-7979-9903-e8a4eef14d64
name: shoot-spark-form
created_at: 2026-10-24T20:07:26.957Z
desc: Spark hero emits one shot while under live cap.
tags: []
status: accepted
coverage: defined
nouns:
  - nouns/hero
verb: verbs/shoot
context_nouns:
  - nouns/power-state
mechanics:
  - mechanics/spark-launch
control-vars:
  - control-vars/spark-capacity
---
## Conditions

Hero is spark form and live projectile count is below spark-capacity when shoot is pressed.

## Consequences and feedback

Create one spark in facing direction and show firing feedback. Projectiles are transient entities, not ammunition deducted from a persistent pool.
