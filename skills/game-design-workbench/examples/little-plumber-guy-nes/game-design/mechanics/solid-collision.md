---
id: 01a16f86-69b5-7072-83d5-638f692f1ca3
name: solid-collision
created_at: 2026-10-24T20:07:22.293Z
desc: Solid geometry prevents penetration and supports landings.
tags: []
status: accepted
nouns:
  - nouns/ground
  - nouns/step
  - nouns/pipe
  - nouns/ceiling
  - nouns/hero
verbs:
  - verbs/walk
  - verbs/jump
  - verbs/bump
---
## Rule

Ground, steps, pipes and intact blocks prevent penetration. Top contact supports a landing; side contact stops obstructed travel; underside contact ends upward penetration and may invoke a block-specific response. Leaving support starts falling.

## Open design

Hitbox dimensions, collision ordering and moving-platform behavior are not specified. There are no moving platforms in this opening course.
