---
id: 01a16f86-69b5-7693-b317-808411004621
name: brick-response
created_at: 2026-10-24T20:07:22.293Z
desc: A rising head hit bumps or breaks a brick according to power state.
tags: []
status: accepted
nouns:
  - nouns/brick
  - nouns/hero
  - nouns/power-state
verbs:
  - verbs/bump
control-vars:
  - control-vars/brick-points
resources:
  - resources/run-score
context_axes:
  - axes/Contact
---
## Rule

A rising underside hit by small form bumps but preserves brick. Tall or spark form destroys it and credits brick-points once. Other contact remains solid collision. Ordinary bricks contain no reward.

## Feedback

A bump and actual break must look distinct. Removing geometry changes available landing support; destruction is not equivalent to collecting a pickup.

```gd-rule
case small-bump:
    context: Hero undersideContacts Brick
    unknown_effects: Bump/end upward penetration; Brick remains; no reward. Geometry/collision response unconfigured.
    Hero:
        noun: nouns/hero
        requires: rising AND small
    Brick:
        noun: nouns/brick
        preserve: BrickState

case powered-break:
    context: Hero undersideContacts Brick
    unknown_conditions: Original destruction credited once; event identity/deduplication unencoded.
    unknown_effects: Remove geometry/landing support; break feedback distinct from bump, no hidden item. Not pickup collection.
    Hero:
        noun: nouns/hero
        requires: rising AND (tall OR spark)
    Brick:
        noun: nouns/brick
        becomes: destroyed
    Score:
        noun: nouns/score
        set:
            ScoreAmount: this + {control-vars/brick-points}

case other-contact-solid:
    unknown_conditions: Other contact with ordinary Brick, excluding valid destructive underside branch; exact geometry unencoded.
    unknown_effects: Solid collision, not reward collection; numerical response unspecified.
    Hero:
        noun: nouns/hero
    Brick:
        noun: nouns/brick
```
