---
id: 01a16f86-7bec-78c0-ab9c-7df235df2b59
name: stand-under-low-ceiling
created_at: 2026-10-24T20:07:26.956Z
desc: Releasing crouch without standing clearance is unresolved.
tags: []
status: accepted
coverage: unknown
nouns:
  - nouns/hero
verb: verbs/crouch
context_nouns:
  - nouns/ceiling
mechanics:
  - mechanics/crouching
  - mechanics/solid-collision
---
## Conditions

Tall or spark hero releases crouch without enough standing clearance.

## Open question

Choose whether stance remains low, motion is constrained, or another safe response applies. No consequence has been adopted. Coverage is unknown, not a defined no-effect action or an exclusion.

```gd-rule
case release-without-clearance:
    unknown_conditions: Tall/spark Hero releases crouch without enough standing clearance; geometry/predicate unencoded.
    unknown_effects: No response adopted; remain low, constrain motion or another response unselected.
    Hero:
        noun: nouns/hero
        requires: tall OR spark
    Ceiling:
        noun: nouns/ceiling
```
