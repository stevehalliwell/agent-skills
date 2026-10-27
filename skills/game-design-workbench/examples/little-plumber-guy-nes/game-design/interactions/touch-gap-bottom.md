---
id: 01a16f86-7bed-7ba0-8ba8-f064d9c38e33
name: touch-gap-bottom
created_at: 2026-10-24T20:07:26.957Z
desc: Crossing lower death boundary kills regardless of form or protection.
tags: []
status: accepted
coverage: defined
nouns:
  - nouns/hero
verb: verbs/touch
context_nouns:
  - nouns/gap
mechanics:
  - mechanics/death-and-retry
context_axes:
  - axes/Contact
---
## Conditions

Hero crosses lower death boundary of a floor gap.

## Consequences and feedback

Invoke lethal retry rule regardless of power form or enemy-contact protection. A damage window is not general invulnerability. Multiple overlapping lethal events must not spend several lives for one death.

```gd-rule
case gap-lethal:
    context: Hero crossesLowerBoundary Gap
    unknown_conditions: Cross authored floor-gap lower death boundary regardless of form/protection. One death must not repeatedly charge overlapping lethal events; deduplication unresolved.
    unknown_effects: One lethal occurrence spends one life; later retry/game-end separate in death-and-retry. Enemy protection is not general invulnerability.
    Hero:
        noun: nouns/hero
        becomes: dead
    Gap:
        noun: nouns/gap
    Lives:
        noun: nouns/lives
        set:
            Amount: this - 1[life]
```
