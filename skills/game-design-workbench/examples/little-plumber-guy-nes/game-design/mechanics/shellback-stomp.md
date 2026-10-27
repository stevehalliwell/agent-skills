---
id: 01a16f86-69b5-7e22-80c6-77580c4ef798
name: shellback-stomp
created_at: 2026-10-24T20:07:22.293Z
desc: Stomp retracts a walking shellback into a stationary shell.
tags: []
status: accepted
nouns:
  - nouns/hero
  - nouns/shellback
  - nouns/shell
verbs:
  - verbs/stomp
control-vars:
  - control-vars/shellback-points
resources:
  - resources/run-score
context_axes:
  - axes/Contact
---
## Rule

Descending top contact retracts walking shellback into a resting shell and rebounds hero. Credit shellback-points once for that original enemy. Later shell kicks and stops do not repeat the award. Unretracted enemy side contact is harmful.

## Feedback

Stationary shell state must contrast with a live walker and a sliding shell.

```gd-rule
case retract:
    context: Hero topContacts Target
    unknown_conditions: Descending top contact; original Shellback award once, event uniqueness unencoded.
    unknown_effects: Retract same persistent actor into stationary Shell and rebound Hero. Rebound magnitude/trajectory unknown; stationary/walking/sliding visuals must contrast.
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

case later-kick-or-stop:
    unknown_conditions: Later kick/stop of already retracted original Shellback; event identity unencoded.
    unknown_effects: No repeated original award; shell-travel owns detailed kick/stop transitions, not silently inherited.
    Target:
        noun: nouns/shellback
    Score:
        noun: nouns/score
        preserve: ScoreAmount

case unretracted-small-side:
    context: Hero sideContacts Target
    unknown_conditions: Harmful side contact, not valid stomp; original enemy-contact behavior explicitly composed.
    Hero:
        noun: nouns/hero
        causes: mechanics/enemy-contact#walking-small with Hero=Hero, Target=Target, Lives=Lives
    Target:
        noun: nouns/shellback
        requires: walking
    Lives:
        noun: nouns/lives

case unretracted-powered-side:
    context: Hero sideContacts Target
    unknown_conditions: Harmful side contact, not valid stomp.
    Hero:
        noun: nouns/hero
        causes: mechanics/enemy-contact#walking-powered with Hero=Hero, Target=Target, Lives=Lives
    Target:
        noun: nouns/shellback
        requires: walking
    Lives:
        noun: nouns/lives

case unretracted-protected-side:
    context: Hero sideContacts Target
    Hero:
        noun: nouns/hero
        causes: mechanics/enemy-contact#walking-protected with Hero=Hero, Target=Target
    Target:
        noun: nouns/shellback
        requires: walking
```
