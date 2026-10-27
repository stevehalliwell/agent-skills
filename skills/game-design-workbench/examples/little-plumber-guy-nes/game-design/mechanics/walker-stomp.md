---
id: 01a16f86-69b5-791a-a411-4929abf1c104
name: walker-stomp
created_at: 2026-10-24T20:07:22.293Z
desc: A descending top contact defeats a walker and rebounds the hero.
tags: []
status: accepted
nouns:
  - nouns/hero
  - nouns/walker
verbs:
  - verbs/stomp
control-vars:
  - control-vars/walker-points
resources:
  - resources/run-score
context_axes:
  - axes/Contact
---
## Rule

Descending top contact defeats walker, pays walker-points once and rebounds hero. Side contact remains harmful. Walker patrol reverses at walls but may fall off edges. No defeat-chain multiplier is assumed.

## Open design

Rebound height and simultaneous contact arbitration remain unspecified.

```gd-rule
case descending-defeat:
    context: Hero topContacts Walker
    unknown_conditions: Original descending top defeat once; event identity/deduplication unencoded.
    unknown_effects: Remove Walker, rebound Hero; height/impulse/trajectory and simultaneous arbitration unknown; geometry/later hazards active, no chain multiplier.
    Hero:
        noun: nouns/hero
        requires: falling
        becomes: rising
    Walker:
        noun: nouns/walker
        becomes: dead
    Score:
        noun: nouns/score
        set:
            ScoreAmount: this + {control-vars/walker-points}

case small-side:
    context: Hero sideContacts Walker
    unknown_conditions: Harmful side contact outside valid stomp; explicit selected enemy-contact case.
    Hero:
        noun: nouns/hero
        causes: mechanics/enemy-contact#walker-small with Hero=Hero, Target=Walker, Lives=Lives
    Walker:
        noun: nouns/walker
    Lives:
        noun: nouns/lives

case powered-side:
    context: Hero sideContacts Walker
    unknown_conditions: Harmful side contact outside valid stomp.
    Hero:
        noun: nouns/hero
        causes: mechanics/enemy-contact#walker-powered with Hero=Hero, Target=Walker, Lives=Lives
    Walker:
        noun: nouns/walker
    Lives:
        noun: nouns/lives

case protected-side:
    context: Hero sideContacts Walker
    Hero:
        noun: nouns/hero
        causes: mechanics/enemy-contact#walker-protected with Hero=Hero, Target=Walker
    Walker:
        noun: nouns/walker

case patrol:
    unknown_conditions: Walker patrol on support; wall/edge coordinates and geometry unencoded.
    unknown_effects: Reverse at walls, may fall off edges; no intelligent gap avoidance. Direction/speed/trajectory unknown.
    Walker:
        noun: nouns/walker
```
