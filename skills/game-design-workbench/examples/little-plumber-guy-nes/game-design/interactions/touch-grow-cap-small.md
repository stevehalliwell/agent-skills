---
id: 01a16f86-7bec-7870-b537-69dd7cce6e9d
name: touch-grow-cap-small
created_at: 2026-10-24T20:07:26.956Z
desc: Small hero consumes cap and becomes tall.
tags: []
status: accepted
coverage: defined
nouns:
  - nouns/grow-cap
verb: verbs/touch
context_nouns:
  - nouns/hero
  - nouns/power-state
mechanics:
  - mechanics/growth
context_axes:
  - axes/Contact
---
## Conditions

Small hero overlaps an available grow-cap.

## Consequences and feedback

Consume pickup and become tall. No life or safety window granted. Clearance handling if growth occurs under a ceiling remains open in growth mechanic.

```gd-rule
case collect:
    context: Hero overlaps Cap
    unknown_effects: Consume pickup; no life or safety-window grant. Growth under inadequate ceiling clearance unresolved; exact geometry unconfigured.
    Hero:
        noun: nouns/hero
        requires: small
        becomes: tall
    Cap:
        noun: nouns/grow-cap
        requires: available
        becomes: consumed
```
