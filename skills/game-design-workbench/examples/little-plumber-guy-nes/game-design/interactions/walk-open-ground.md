---
id: 01a16f86-7bea-7347-90cd-c0419b164759
name: walk-open-ground
created_at: 2026-10-24T20:07:26.954Z
desc: Horizontal input advances on unobstructed support.
tags: []
status: accepted
coverage: defined
nouns:
  - nouns/hero
verb: verbs/walk
context_nouns:
  - nouns/ground
mechanics:
  - mechanics/horizontal-movement
control-vars:
  - control-vars/walk-speed
  - control-vars/run-speed
---
## Conditions

Hero has support, horizontal input and no blocking surface. Run modifier selects target speed.

## Consequences and feedback

Advance toward input direction; update facing. This is successful movement, unlike blocked side contact. Acceleration profile remains a movement-rule unknown.
