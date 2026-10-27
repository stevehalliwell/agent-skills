---
id: 01a16f86-7bec-76e7-89fd-7c141d01077f
name: bump-brick-small
created_at: 2026-10-24T20:07:26.956Z
desc: Small form bumps a brick without breaking it.
tags: []
status: accepted
coverage: defined
nouns:
  - nouns/brick
verb: verbs/bump
context_nouns:
  - nouns/hero
  - nouns/power-state
mechanics:
  - mechanics/brick-response
context_axes:
  - axes/Contact
---
## Conditions

Rising small hero hits brick underside.

## Consequences and feedback

End upward penetration and show a bump. Brick remains; no score or reward. The same target and verb has a destructive powered-form alternative.

```gd-rule
case small-bump:
    context: Hero undersideContacts Brick
    unknown_effects: End upward penetration and show bump; no score/reward. Exact collision response unconfigured.
    Hero:
        noun: nouns/hero
        requires: small AND rising
    Brick:
        noun: nouns/brick
        preserve: BrickState
```
