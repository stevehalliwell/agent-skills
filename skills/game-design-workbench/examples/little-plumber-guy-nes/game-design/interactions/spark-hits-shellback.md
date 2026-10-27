---
id: 01a16f86-7bed-7503-976d-8a3759e0d164
name: spark-hits-shellback
created_at: 2026-10-24T20:07:26.957Z
desc: Spark defeats a walking shellback without creating a shell.
tags: []
status: accepted
coverage: defined
nouns:
  - nouns/shellback
verb: verbs/touch
context_nouns:
  - nouns/spark
mechanics:
  - mechanics/spark-impact
context_axes:
  - axes/Contact
---
## Conditions

Hero spark contacts an unretracted walking shellback.

## Consequences and feedback

Defeat enemy and consume spark; grant shellback award once. Do not leave a resting shell. Existing-shell impact is not defined by this walking-enemy case.

```gd-rule
case spark-defeat:
    context: Spark touches Target
    unknown_conditions: Hero-owned Spark contacts unretracted walking Shellback; original award once, event identity unencoded.
    unknown_effects: Remove defeated enemy, consume Spark/free slot. Do not leave resting Shell; existing-shell impact remains unknown. Owner-count association unencoded.
    Spark:
        noun: nouns/spark
        becomes: consumed
    Target:
        noun: nouns/shellback
        requires: walking
        becomes: dead
    Score:
        noun: nouns/score
        set:
            ScoreAmount: this + {control-vars/shellback-points}
```
