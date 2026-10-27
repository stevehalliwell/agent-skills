---
id: 01a16f86-7bec-7d94-97f8-b847773cdbb7
name: touch-flower-tall
created_at: 2026-10-24T20:07:26.956Z
desc: Tall hero consumes flower and becomes spark.
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

Tall hero overlaps an available flower.

## Consequences and feedback

Consume flower and become spark. Current live-shot count still gates shooting. A hero already in spark form keeps that form when collecting another flower.

```gd-rule
case collect-tall:
    context: Hero overlaps Flower
    unknown_effects: Consume Flower and gain spark capability; current live-shot count still gates shooting.
    Hero:
        noun: nouns/hero
        requires: tall
        becomes: spark
    Flower:
        noun: nouns/spark-flower
        requires: available
        becomes: consumed

case collect-already-spark:
    context: Hero overlaps Flower
    unknown_effects: Consume another Flower without further form change; live-shot cap still applies.
    Hero:
        noun: nouns/hero
        requires: spark
        preserve: Form
    Flower:
        noun: nouns/spark-flower
        becomes: consumed
```
