---
id: 01a16f86-7bec-7d6e-b179-a06085c45807
name: crouch-small
created_at: 2026-10-24T20:07:26.956Z
desc: Small form gains no further size reduction.
tags: []
status: accepted
coverage: defined
nouns:
  - nouns/hero
verb: verbs/crouch
context_nouns:
  - nouns/power-state
mechanics:
  - mechanics/crouching
---
## Conditions

Small hero receives crouch input.

## Consequences and feedback

No additional collision-size reduction. Do not grant movement through narrower passages or hidden immunity. Cosmetic pose is optional and does not change the rule.

```gd-rule
case small-input:
    unknown_conditions: Crouch input received.
    unknown_effects: No additional collision-size reduction, narrower-passage capability or hidden immunity. Cosmetic pose optional; no dimensions supplied.
    Hero:
        noun: nouns/hero
        requires: small
```
