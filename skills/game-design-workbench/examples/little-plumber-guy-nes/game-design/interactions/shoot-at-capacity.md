---
id: 01a16f86-7bed-7719-9cb7-04f18a59b840
name: shoot-at-capacity
created_at: 2026-10-24T20:07:26.957Z
desc: Shot input at live cap has no effect and is not queued.
tags: []
status: accepted
coverage: defined
nouns:
  - nouns/hero
verb: verbs/shoot
context_nouns:
  - nouns/spark
mechanics:
  - mechanics/spark-launch
---
## Conditions

Spark hero already has spark-capacity live projectiles.

## Consequences and feedback

Create nothing and queue nothing. Releasing a slot later does not retroactively fire. Zero configured capacity deliberately makes every shot request take this no-effect path.
