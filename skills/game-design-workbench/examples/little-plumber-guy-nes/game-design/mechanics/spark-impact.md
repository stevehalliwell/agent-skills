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
context_axes:
  - axes/Contact
---
## Rule

Hero spark defeats walker or walking shellback, grants that enemy's fixed award and is consumed. Sparked shellback leaves no resting shell. Wall contact or leaving active viewport removes a spark; floor contact bounces it. Spark versus existing shell remains unspecified.

## Feedback

Defeat and projectile removal should be visible; these rules do not imply a complete collision table.

```gd-rule
case walker-defeat:
    context: Spark touches Target
    unknown_conditions: Hero-owned Spark contacts eligible Walker; original defeat once, owner/event identity unencoded.
    unknown_effects: Remove defeated enemy, consume Spark/free live slot; visible defeat/removal. Owner-count association unencoded.
    Spark:
        noun: nouns/spark
        becomes: consumed
    Target:
        noun: nouns/walker
        becomes: dead
    Score:
        noun: nouns/score
        set:
            ScoreAmount: this + {control-vars/walker-points}

case walking-shellback-defeat:
    context: Spark touches Target
    unknown_conditions: Hero-owned Spark, original eligible enemy event once; owner/event identity unencoded.
    unknown_effects: Remove enemy, consume Spark/free live slot. Do not leave resting Shell; visible defeat/removal, count association unspecified.
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

case wall-removal:
    unknown_conditions: Spark contacts wall; no canonical Wall noun or geometry supplied.
    unknown_effects: Remove Spark/free live slot; owner/count binding unencoded, no wall damage invented.
    Spark:
        noun: nouns/spark
        becomes: removed

case viewport-removal:
    unknown_conditions: Spark leaves active viewport; boundary coordinates/owner binding unencoded.
    unknown_effects: Remove Spark/free live slot; viewport exit is not guessed maximum range.
    Spark:
        noun: nouns/spark
        becomes: removed
    Camera:
        noun: nouns/camera

case floor-bounce:
    context: Spark touches Ground
    unknown_conditions: Floor contact.
    unknown_effects: Bounce Spark; no speed or arc magnitude supplied.
    Spark:
        noun: nouns/spark
    Ground:
        noun: nouns/ground

case existing-shell-unknown:
    context: Spark touches Shell
    unknown_effects: Existing-Shell response unspecified; do not reuse walking-Shellback defeat.
    Spark:
        noun: nouns/spark
    Shell:
        noun: nouns/shellback
        requires: resting OR sliding
```
