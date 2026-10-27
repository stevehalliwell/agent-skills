---
id: 01a16f86-69b5-71b5-b289-a17403b1fa5e
name: spark-upgrade
created_at: 2026-10-24T20:07:22.293Z
desc: Flower makes tall form spark; small form becomes tall instead.
tags: []
status: accepted
nouns:
  - nouns/spark-flower
  - nouns/hero
  - nouns/power-state
verbs:
  - verbs/touch
context_axes:
  - axes/Contact
---
## Rule

Flower pickup is consumed. Small becomes tall; tall becomes spark; spark remains spark. Upgrade persists until damage or retry rather than expiring by timer. The same object does not grant two successive upgrades in one contact.

## Feedback

Distinguish visible form from actual shooting availability; live-shot capacity still gates launch.

```gd-rule
case small-collection:
    context: Hero overlaps Flower
    unknown_conditions: Original pickup consumed once; same object cannot apply two upgrades in this occurrence.
    unknown_effects: Consume Flower for tall form only; upgrade persists until damage/retry, not timer. Live shot cap still applies; collection form not release form.
    Hero:
        noun: nouns/hero
        requires: small
        becomes: tall
    Flower:
        noun: nouns/spark-flower
        becomes: consumed

case tall-collection:
    context: Hero overlaps Flower
    unknown_conditions: Original Flower consumed once.
    unknown_effects: Consume Flower; upgrade persists until damage/retry, not timer; live shot cap still applies.
    Hero:
        noun: nouns/hero
        requires: tall
        becomes: spark
    Flower:
        noun: nouns/spark-flower
        becomes: consumed

case spark-collection:
    context: Hero overlaps Flower
    unknown_conditions: Original Flower consumed once.
    unknown_effects: Consume Flower without further form upgrade; live shot capacity still gates launch.
    Hero:
        noun: nouns/hero
        requires: spark
        preserve: Form
    Flower:
        noun: nouns/spark-flower
        becomes: consumed
```
