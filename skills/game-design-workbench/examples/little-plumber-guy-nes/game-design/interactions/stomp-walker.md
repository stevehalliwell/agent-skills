---
id: 01a16f86-7bec-7285-a0e2-248bd44a87b8
name: stomp-walker
created_at: 2026-10-24T20:07:26.956Z
desc: Descending top contact removes walker and rebounds hero.
tags: []
status: accepted
coverage: defined
nouns:
  - nouns/walker
verb: verbs/stomp
context_nouns:
  - nouns/hero
mechanics:
  - mechanics/walker-stomp
context_axes:
  - axes/Contact
---
## Conditions

Descending hero contacts walker from above.

## Consequences and feedback

Defeat walker, credit its fixed award and rebound hero. Geometry and later hazards remain active. Merely touching a walker's side is not this event.

```gd-rule
case descending-defeat:
    context: Hero topContacts Walker
    unknown_conditions: Descending top contact, not side contact; award once per original defeat, uniqueness unencoded.
    unknown_effects: Remove Walker and rebound Hero; rebound height/trajectory unspecified. Geometry and later hazards stay active.
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
```
