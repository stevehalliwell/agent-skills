---
id: 01a16f86-7bec-7901-876f-9515219da574
name: touch-walker-powered
created_at: 2026-10-24T20:07:26.956Z
desc: Unprotected powered hero becomes small and gains temporary protection.
tags: []
status: accepted
coverage: defined
nouns:
  - nouns/walker
verb: verbs/touch
context_nouns:
  - nouns/hero
  - nouns/power-state
  - nouns/lives
mechanics:
  - mechanics/enemy-contact
control-vars:
  - control-vars/safety-window
---
## Rule

Authoring-review specimen. Axis declarations, context vocabulary and the `gd-rule` reader are pending; this block is not yet executable or semantically validated.

```gd-rule
case powered-side-contact:
    context: Player sideContacts Walker

    Player:
        noun: nouns/hero
        requires: alive AND (tall OR spark) AND NOT protected
        other_axes: ANY
        becomes: small, protected
        set:
            ProtectionTime: {control-vars/safety-window}
        preserve: OTHER

    Walker:
        noun: nouns/walker
        requires: alive
        other_axes: ANY

    Lives:
        noun: nouns/lives
        requires: ANY
        preserve: ALL
```

## Feedback and boundaries

Show power loss and temporary protection distinctly. Gap death and clock death remain possible; enemy-contact protection does not prevent them.

This case does not specify the walker's resulting state. `ProtectionTime` is the proposed numeric protection-duration axis, measured in seconds; its declaration belongs in the pending axis model. `sideContacts` is the proposed ordered contact-context value, not a new player-input action.
