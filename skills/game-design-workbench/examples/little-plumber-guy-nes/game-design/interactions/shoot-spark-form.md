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

```gd-rule
case fire:
    unknown_conditions: Fresh shoot press; input/facing/spawn geometry unencoded.
    unknown_effects: Create one Hero-owned Spark in facing direction and firing feedback. Concurrent count, not persistent ammo; speed, bounce arc and range unspecified.
    Hero:
        noun: nouns/hero
        requires: spark AND LiveSparks < {control-vars/spark-capacity}
        set:
            LiveSparks: this + 1[spark]
```
