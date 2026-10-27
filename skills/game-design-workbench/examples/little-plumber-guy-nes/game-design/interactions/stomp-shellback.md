---
id: 01a16f86-7bec-7ad0-89e5-10ebff8233c9
name: stomp-shellback
created_at: 2026-10-24T20:07:26.956Z
desc: Descending top contact retracts the walking armored enemy.
tags: []
status: accepted
coverage: defined
nouns:
  - nouns/shellback
verb: verbs/stomp
context_nouns:
  - nouns/hero
mechanics:
  - mechanics/shellback-stomp
context_axes:
  - axes/Contact
---
## Conditions

Descending hero contacts a walking shellback from above.

## Consequences and feedback

Retract to stationary shell, credit original shellback award once and rebound hero. Shell remains in course and can become a later hazard.

```gd-rule
case retract:
    context: Hero topContacts Target
    unknown_conditions: Descending top contact; original Shellback award once, event uniqueness unencoded.
    unknown_effects: Retract same persistent actor to stationary Shell; rebound Hero. Exact rebound magnitude unconfigured; Shell remains and may later become hazard.
    Hero:
        noun: nouns/hero
        requires: falling
        becomes: rising
    Target:
        noun: nouns/shellback
        requires: walking
        becomes: resting
    Score:
        noun: nouns/score
        set:
            ScoreAmount: this + {control-vars/shellback-points}
```
