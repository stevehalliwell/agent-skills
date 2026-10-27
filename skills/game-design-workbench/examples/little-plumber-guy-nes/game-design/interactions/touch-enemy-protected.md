---
id: 01a16f86-7bec-77da-b09d-4bde1553b505
name: touch-enemy-protected
created_at: 2026-10-24T20:07:26.956Z
desc: Contact during protection causes no additional enemy damage.
tags: []
status: accepted
coverage: defined
nouns:
  - nouns/walker
  - nouns/shellback
  - nouns/shell
verb: verbs/touch
context_nouns:
  - nouns/hero
mechanics:
  - mechanics/enemy-contact
context_axes:
  - axes/Contact
---
## Conditions

Hero is in enemy-contact safety window and contacts walker, shellback or sliding shell.

## Consequences and feedback

No additional enemy damage. Keep protection feedback visible until expiry. This does not disable gap death, clock spending or geometry.

```gd-rule
case protected-walker:
    context: Hero touches Target
    unknown_effects: No additional enemy damage; show protection feedback until expiry. Do not disable gaps, Clock or geometry; target result unspecified. This damage rule does not suppress separately authored stomps.
    Hero:
        noun: nouns/hero
        requires: protected
        preserve: Life, Form
    Target:
        noun: nouns/walker

case protected-walking-shellback:
    context: Hero touches Target
    unknown_effects: No additional enemy damage; feedback until expiry; gaps, Clock, geometry and separately authored stomps remain active. Target result unspecified.
    Hero:
        noun: nouns/hero
        requires: protected
        preserve: Life, Form
    Target:
        noun: nouns/shellback
        requires: walking

case protected-sliding-shell:
    context: Hero touches Target
    unknown_effects: No additional enemy damage; feedback until expiry; gaps, Clock, geometry and separately authored stomps remain active. Target result unspecified.
    Hero:
        noun: nouns/hero
        requires: protected
        preserve: Life, Form
    Target:
        noun: nouns/shellback
        requires: sliding
```
