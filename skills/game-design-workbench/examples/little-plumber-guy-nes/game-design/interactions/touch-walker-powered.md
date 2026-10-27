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
context_axes:
  - axes/Contact
---
## Rule

### Original declaration checkpoint

Accepted authoring specimen. Native `gd rules` reads this block as declarations. Typed axes, explicit noun/axis bindings, grouped state predicates, effect names and the quantity target now resolve. Ordered context binds local participants through explicit `context_axes` without establishing actual contact. Concrete applicability, resulting quantities, preservation and composition remain pending; compiled predicates do not execute or validate gameplay meaning.

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

### Current inspection

Declaration-only limitations above describe the historical authoring checkpoint. Native `rules --case` now evaluates explicitly supplied hypotheses and simultaneous proposals; `--analyze` adds bounded comparison. No gameplay execution, scene inference or missing Walker outcome supplied. Original specimen and prose retained.

## Feedback and boundaries

Show power loss and temporary protection distinctly. Gap death and clock death remain possible; enemy-contact protection does not prevent them.

This case does not specify the walker's resulting state. `ProtectionTime` is the proposed numeric protection-duration axis, measured in seconds; its [typed declaration](../axes/ProtectionTime.md) remains proposed. `sideContacts` belongs to the proposed [Contact vocabulary](../axes/Contact.md), not a new player-input action or inferred actor state.
