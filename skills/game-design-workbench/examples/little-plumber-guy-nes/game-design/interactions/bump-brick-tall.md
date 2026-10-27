---
id: 01a16f86-7bec-7b31-8f8d-353d94fcd7fd
name: bump-brick-tall
created_at: 2026-10-24T20:07:26.956Z
desc: Tall or spark form breaks a brick and receives its award.
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
  - mechanics/score-awards
control-vars:
  - control-vars/brick-points
context_axes:
  - axes/Contact
---
## Conditions

Rising tall or spark hero hits brick underside.

## Consequences and feedback

Remove brick, award configured brick-points once and show break feedback. New empty space is no longer landing support. No hidden item drops.

```gd-rule
case powered-break:
    context: Hero undersideContacts Brick
    unknown_conditions: Credit original destruction once; event identity/deduplication unencoded.
    unknown_effects: Remove Brick geometry/landing support, show break feedback; no hidden item.
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
```
