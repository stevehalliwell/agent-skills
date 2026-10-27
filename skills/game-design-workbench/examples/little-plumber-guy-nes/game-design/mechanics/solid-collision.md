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

```gd-rule
case ground-solid:
    unknown_conditions: Contact with solid Ground; top/side/underside geometry, leaving-support event and collision order unencoded.
    unknown_effects: Prevent penetration; top supports landing, side stops obstructed travel, underside ends upward penetration. Leaving support begins falling. No numerical correction/tolerance or moving-platform behavior specified.
    Hero:
        noun: nouns/hero
    Surface:
        noun: nouns/ground

case step-solid:
    unknown_conditions: Contact with solid Step; contact direction/geometry and collision ordering unencoded.
    unknown_effects: Prevent penetration; top supports landing, side stops obstructed travel, underside ends upward penetration. Leaving support starts falling; no numerical correction/tolerance.
    Hero:
        noun: nouns/hero
    Surface:
        noun: nouns/step

case pipe-solid:
    unknown_conditions: Contact with solid Pipe; contact direction/geometry and ordering unencoded.
    unknown_effects: Prevent penetration; top supports landing, side blocks travel, underside ends upward penetration; leaving support starts fall. Entering declared connector is separate rule, not every Pipe.
    Hero:
        noun: nouns/hero
    Surface:
        noun: nouns/pipe

case ceiling-solid:
    unknown_conditions: Contact with solid Ceiling; geometry/direction/ordering unencoded.
    unknown_effects: Prevent penetration; top landing/side stop/underside upward stop apply according to contact direction. Ceiling not necessarily reward block; leaving support falls. No collision dimensions supplied.
    Hero:
        noun: nouns/hero
    Surface:
        noun: nouns/ceiling

case intact-brick-solid:
    unknown_conditions: Contact with intact Brick; contact direction/geometry unencoded.
    unknown_effects: Prevent penetration; top supports landing, side stops travel, underside stops upward penetration and may invoke separately authored brick response. Leaving support falls; no automatic invocation from link or file order.
    Hero:
        noun: nouns/hero
    Surface:
        noun: nouns/brick
        requires: intact

case supply-solid:
    unknown_conditions: Contact with solid Supply Block, including spent view; exact contact direction/geometry unencoded.
    unknown_effects: Prevent penetration; top supports landing, side stops travel, underside stops upward penetration and may invoke authored supply response on valid hit. Spent Block remains solid; leaving support falls. No moving platforms in this course.
    Hero:
        noun: nouns/hero
    Surface:
        noun: nouns/supply-block
```
