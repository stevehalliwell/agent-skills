---
id: 01a16f86-7bed-7f1e-9b7f-967f9c95ea6e
name: spark-hits-walker
created_at: 2026-10-24T20:07:26.957Z
desc: Spark defeats a walker; projectile is consumed.
tags: []
status: accepted
coverage: defined
nouns:
  - nouns/walker
verb: verbs/touch
context_nouns:
  - nouns/spark
mechanics:
  - mechanics/spark-impact
context_axes:
  - axes/Contact
---
## Conditions

Hero-owned spark contacts living walker.

## Consequences and feedback

Defeat walker, credit its configured award and consume spark, freeing a live-shot slot. It cannot also award a second defeat from the same removed enemy.

```gd-rule
case spark-defeat:
    context: Spark touches Walker
    unknown_conditions: Spark is Hero-owned; original defeat credited once; owner/instance identity and deduplication unencoded.
    unknown_effects: Remove Walker, consume Spark and free its live-shot slot; no second award from removed enemy. Owner-count association unencoded, not guessed.
    Spark:
        noun: nouns/spark
        becomes: consumed
    Walker:
        noun: nouns/walker
        requires: alive
        becomes: dead
    Score:
        noun: nouns/score
        set:
            ScoreAmount: this + {control-vars/walker-points}
```
