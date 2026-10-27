---
id: 01a16f86-7bec-7575-a7be-65b35cc13e3a
name: touch-flower-small
created_at: 2026-10-24T20:07:26.956Z
desc: Small hero consumes flower and becomes tall, not spark.
tags: []
status: accepted
coverage: defined
nouns:
  - nouns/spark-flower
verb: verbs/touch
context_nouns:
  - nouns/hero
  - nouns/power-state
mechanics:
  - mechanics/spark-upgrade
context_axes:
  - axes/Contact
---
## Conditions

Small hero reaches a flower already released in the world.

## Consequences and feedback

Consume flower and become tall only. This can happen after damage between release and collection. Do not infer current release-time form from pickup-time form.

```gd-rule
case collect-small:
    context: Hero overlaps Flower
    unknown_conditions: Flower already released in world; form at collection, not release time. Same object gives one upgrade only.
    unknown_effects: Consume Flower, become tall only; no sequential second upgrade. Damage may have occurred since release.
    Hero:
        noun: nouns/hero
        requires: small
        becomes: tall
    Flower:
        noun: nouns/spark-flower
        becomes: consumed
```
