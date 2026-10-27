---
id: 01a16f86-7bec-7918-9352-2837e58f27e1
name: touch-moving-shell-side
created_at: 2026-10-24T20:07:26.956Z
desc: Sliding shell's side is an enemy-contact hazard.
tags: []
status: accepted
coverage: defined
nouns:
  - nouns/shell
verb: verbs/touch
context_nouns:
  - nouns/hero
mechanics:
  - mechanics/shell-travel
  - mechanics/enemy-contact
context_axes:
  - axes/Contact
---
## Conditions

Hero side contacts a sliding shell outside a valid stomp.

## Consequences and feedback

Apply enemy-contact according to form and protection. A shell launched by hero is not friendly afterward. Original ownership does not suppress its hazard.

```gd-rule
case small-side:
    context: Hero sideContacts Shell
    unknown_conditions: Harmful side contact outside valid stomp; original ownership gives no safety. One lethal occurrence; deduplication unresolved.
    unknown_effects: One life spent; later retry separate. Shell result unspecified.
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
    unknown_conditions: Outside valid stomp; apply enemy-contact; ownership does not suppress hazard.
    unknown_effects: Show power loss and temporary protection; no life charge for powered damage. Shell result and protection expiration law unspecified.
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
    unknown_conditions: Outside valid stomp during enemy-contact safety window.
    unknown_effects: No further enemy damage; Clock, gaps and geometry remain active. Do not freeze protection duration or infer Shell result.
    Hero:
        noun: nouns/hero
        requires: protected
        preserve: Life, Form
    Shell:
        noun: nouns/shellback
        requires: sliding
```
