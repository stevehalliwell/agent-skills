---
id: 01a16f86-69b5-76f4-96bd-5c0566ed7395
name: enemy-contact
created_at: 2026-10-24T20:07:22.293Z
desc: Unprotected side contact costs power or a life; protection blocks repeated enemy damage.
tags: []
status: accepted
nouns:
  - nouns/hero
  - nouns/walker
  - nouns/shellback
  - nouns/shell
  - nouns/power-state
verbs:
  - verbs/touch
control-vars:
  - control-vars/safety-window
resources:
  - resources/retry-budget
context_axes:
  - axes/Contact
---
## Rule

Unprotected harmful side contact with walker, shellback or sliding shell kills small form. Tall or spark instead becomes small and gains safety-window enemy protection. Protected contacts cause no further enemy damage. Gap death and clock expiry ignore this protection.

## Feedback

Show power loss and protected state distinctly. Descending top contacts use their own stomp cases, not this side-contact rule.

```gd-rule
case walker-small:
    context: Hero sideContacts Target
    unknown_conditions: Harmful side contact, not descending top stomp. One lethal occurrence; deduplication unresolved.
    unknown_effects: One death spends one life; later retry separate. Target result unspecified.
    Hero:
        noun: nouns/hero
        requires: small AND NOT protected
        becomes: dead
    Target:
        noun: nouns/walker
    Lives:
        noun: nouns/lives
        set:
            Amount: this - 1[life]

case walker-powered:
    context: Hero sideContacts Target
    unknown_conditions: Harmful side contact, not valid descending top stomp.
    unknown_effects: Show power loss/protection distinctly. Gap/Clock death still possible; target result unspecified.
    Hero:
        noun: nouns/hero
        requires: (tall OR spark) AND NOT protected
        becomes: small, protected
        set:
            ProtectionTime: {control-vars/safety-window}
    Target:
        noun: nouns/walker
    Lives:
        noun: nouns/lives
        preserve: Amount

case walker-protected:
    context: Hero sideContacts Target
    unknown_conditions: Enemy-contact protection active; descending top cases remain separate.
    unknown_effects: No further enemy damage; not gap/Clock immunity or frozen protection time. Target result unspecified.
    Hero:
        noun: nouns/hero
        requires: protected
        preserve: Life, Form
    Target:
        noun: nouns/walker

case walking-small:
    context: Hero sideContacts Target
    unknown_conditions: Harmful side contact, not valid descending stomp; one lethal occurrence, deduplication unresolved.
    unknown_effects: One death spends one life; later retry separate; target result unspecified.
    Hero:
        noun: nouns/hero
        requires: small AND NOT protected
        becomes: dead
    Target:
        noun: nouns/shellback
        requires: walking
    Lives:
        noun: nouns/lives
        set:
            Amount: this - 1[life]

case walking-powered:
    context: Hero sideContacts Target
    unknown_conditions: Harmful side contact outside valid stomp.
    unknown_effects: Show power loss/protection; gap/Clock death still possible; target result unspecified.
    Hero:
        noun: nouns/hero
        requires: (tall OR spark) AND NOT protected
        becomes: small, protected
        set:
            ProtectionTime: {control-vars/safety-window}
    Target:
        noun: nouns/shellback
        requires: walking
    Lives:
        noun: nouns/lives
        preserve: Amount

case walking-protected:
    context: Hero sideContacts Target
    unknown_conditions: Enemy protection active; top stomp cases remain separate.
    unknown_effects: No further enemy damage; gap/Clock/geometry unaffected, protection duration not frozen. Target result unspecified.
    Hero:
        noun: nouns/hero
        requires: protected
        preserve: Life, Form
    Target:
        noun: nouns/shellback
        requires: walking

case sliding-small:
    context: Hero sideContacts Target
    unknown_conditions: Harmful side contact outside valid stomp; original ownership gives no safety. One lethal occurrence; deduplication unresolved.
    unknown_effects: One life spent, later retry separate; target result unspecified.
    Hero:
        noun: nouns/hero
        requires: small AND NOT protected
        becomes: dead
    Target:
        noun: nouns/shellback
        requires: sliding
    Lives:
        noun: nouns/lives
        set:
            Amount: this - 1[life]

case sliding-powered:
    context: Hero sideContacts Target
    unknown_conditions: Harmful side contact outside valid stomp; no ownership-based safety.
    unknown_effects: Show power loss/protection; gap/Clock death still possible; target result unspecified.
    Hero:
        noun: nouns/hero
        requires: (tall OR spark) AND NOT protected
        becomes: small, protected
        set:
            ProtectionTime: {control-vars/safety-window}
    Target:
        noun: nouns/shellback
        requires: sliding
    Lives:
        noun: nouns/lives
        preserve: Amount

case sliding-protected:
    context: Hero sideContacts Target
    unknown_conditions: Enemy protection active; valid descending stomp separate.
    unknown_effects: No additional enemy damage; Clock/gaps/geometry remain active. Target result and expiry law unspecified.
    Hero:
        noun: nouns/hero
        requires: protected
        preserve: Life, Form
    Target:
        noun: nouns/shellback
        requires: sliding
```
