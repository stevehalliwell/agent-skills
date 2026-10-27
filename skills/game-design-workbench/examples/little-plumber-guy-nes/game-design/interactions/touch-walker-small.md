---
id: 01a16f86-7bec-7824-96b6-5caf1a0d4e2c
name: touch-walker-small
created_at: 2026-10-24T20:07:26.956Z
desc: Unprotected small hero dies from side contact.
tags: []
status: accepted
coverage: defined
nouns:
  - nouns/walker
verb: verbs/touch
context_nouns:
  - nouns/hero
  - nouns/power-state
mechanics:
  - mechanics/enemy-contact
  - mechanics/death-and-retry
context_axes:
  - axes/Contact
---
## Conditions

Unprotected small hero makes harmful side contact with walker.

## Consequences and feedback

Invoke one lethal event through death-and-retry. Do not also process a stomp for this side contact. No observed player mistake is implied by documenting the case.

```gd-rule
case lethal-side:
    context: Hero sideContacts Walker
    unknown_conditions: Harmful side contact, not valid stomp; one lethal occurrence. Simultaneous lethal deduplication required but unresolved.
    unknown_effects: One lethal event spends one life; later retry/game-end is separate death-and-retry occurrence, not simultaneous resurrection. Do not also process stomp. Walker result unspecified.
    Hero:
        noun: nouns/hero
        requires: small AND NOT protected
        becomes: dead
    Walker:
        noun: nouns/walker
    Lives:
        noun: nouns/lives
        set:
            Amount: this - 1[life]
```
