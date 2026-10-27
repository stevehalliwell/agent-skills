---
id: 01a16f86-69b6-7041-9787-0bceb2a13c70
name: pipe-transfer
created_at: 2026-10-24T20:07:22.294Z
desc: Declared entrance connects overworld to cellar and one-way return.
tags: []
status: accepted
nouns:
  - nouns/pipe
  - nouns/exit-pipe
  - nouns/hero
  - nouns/camera
verbs:
  - verbs/enter-pipe
resources:
  - resources/course-time
---
## Rule

Aligned down input on pipe-steps' declared entrance transfers to coin-cellar. Its exit transfers to bonus-return near shell-lane; no reverse transfer. Preserve lives, bank, score, power and remaining clock; clear transient shots and reset camera anchor. Ordinary pipes stay solid.

## Boundary

Challenge next links show route alternatives, not the alignment predicate or transfer state. No portal coordinates are stored.
