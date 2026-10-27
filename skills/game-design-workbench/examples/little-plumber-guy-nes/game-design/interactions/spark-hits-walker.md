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
---
## Conditions

Hero-owned spark contacts living walker.

## Consequences and feedback

Defeat walker, credit its configured award and consume spark, freeing a live-shot slot. It cannot also award a second defeat from the same removed enemy.
