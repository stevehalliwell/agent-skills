---
id: 01a16f86-69b5-7866-99dc-569deba690cc
name: shell-travel
created_at: 2026-10-24T20:07:22.293Z
desc: A kick starts a hazardous sliding shell; a stomp stops it.
tags: []
status: accepted
nouns:
  - nouns/shell
  - nouns/hero
  - nouns/walker
  - nouns/shellback
  - nouns/pipe
verbs:
  - verbs/kick
  - verbs/stomp
  - verbs/touch
context_axes:
  - axes/Contact
---
## Rule

Hero side contact launches a resting shell away from hero. Sliding shell defeats a walker and continues; walls and pipes reverse it. Descending hero stomp stops it; harmful side contact uses enemy-contact. A resting shell never wakes automatically.

## Open design

Two-shell collision, shell contact with another walking shellback, and precise launch separation remain unresolved.

```gd-rule
case kick-resting:
    context: Hero sideContacts Shell
    unknown_effects: Launch away from Hero; speed/hazard separation unresolved; no extra kick button or repeat award.
    Hero:
        noun: nouns/hero
    Shell:
        noun: nouns/shellback
        requires: resting
        becomes: sliding

case walker-impact:
    context: Shell touches Walker
    unknown_effects: Defeat Walker and continue sliding; score award belongs to separate explicit score-awards/interaction declaration, not inherited or duplicated here.
    Shell:
        noun: nouns/shellback
        requires: sliding
        preserve: ShellState
    Walker:
        noun: nouns/walker
        becomes: dead

case pipe-reversal:
    context: Shell sideContacts Pipe
    unknown_effects: Reverse horizontal direction, remain sliding; speed/separation unknown.
    Shell:
        noun: nouns/shellback
        requires: sliding
        preserve: ShellState
    Pipe:
        noun: nouns/pipe

case wall-reversal:
    unknown_conditions: Sliding Shell reaches solid wall; no canonical Wall noun in this example, no guessed binding to another noun.
    unknown_effects: Reverse horizontal direction, remain sliding; magnitude/separation unconfigured.
    Shell:
        noun: nouns/shellback
        requires: sliding
        preserve: ShellState

case descending-stop:
    context: Hero topContacts Shell
    unknown_effects: Stop Shell, rebound Hero; magnitude/trajectory unknown, no repeated original-enemy award.
    Hero:
        noun: nouns/hero
        requires: falling
        becomes: rising
    Shell:
        noun: nouns/shellback
        requires: sliding
        becomes: resting

case small-side:
    context: Hero sideContacts Shell
    unknown_conditions: Harmful side contact outside valid stomp; apply enemy-contact, one lethal event; deduplication unresolved.
    unknown_effects: One life spent; later retry separate. Shell result unspecified; original ownership gives no safety.
    Hero:
        noun: nouns/hero
        requires: small AND NOT protected
        becomes: dead
    Shell:
        noun: nouns/shellback
        requires: sliding
    Lives:
        noun: nouns/lives
        set:
            Amount: this - 1[life]

case powered-side:
    context: Hero sideContacts Shell
    unknown_conditions: Harmful side contact outside valid stomp; apply enemy-contact.
    unknown_effects: Power loss/protection feedback, no life charge; Shell result and expiration law unspecified.
    Hero:
        noun: nouns/hero
        requires: (tall OR spark) AND NOT protected
        becomes: small, protected
        set:
            ProtectionTime: {control-vars/safety-window}
    Shell:
        noun: nouns/shellback
        requires: sliding
    Lives:
        noun: nouns/lives
        preserve: Amount

case protected-side:
    context: Hero sideContacts Shell
    unknown_conditions: Harmful side contact outside valid stomp; apply enemy-contact.
    unknown_effects: No additional enemy damage; gaps/Clock/geometry remain active, not frozen protection duration.
    Hero:
        noun: nouns/hero
        requires: protected
        preserve: Life, Form
    Shell:
        noun: nouns/shellback
        requires: sliding

case no-automatic-waking:
    unknown_conditions: No kick/contact occurrence triggering another authored transition; this forbids automatic waking, not other active effects.
    unknown_effects: Resting Shell never automatically re-emerges.
    Shell:
        noun: nouns/shellback
        requires: resting
        preserve: ShellState

case two-shell-unknown:
    context: First touches Second
    unknown_effects: Two-Shell collision response unresolved.
    First:
        noun: nouns/shellback
        requires: sliding
    Second:
        noun: nouns/shellback
        requires: sliding

case walking-shellback-unknown:
    context: Shell touches Other
    unknown_effects: Shell contact with walking Shellback unresolved.
    Shell:
        noun: nouns/shellback
        requires: sliding
    Other:
        noun: nouns/shellback
        requires: walking
```
