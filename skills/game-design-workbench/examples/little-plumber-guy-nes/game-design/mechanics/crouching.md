---
id: 01a16f86-69b5-7cc5-988a-b8123ea4d313
name: crouching
created_at: 2026-10-24T20:07:22.293Z
desc: Tall forms can lower their grounded profile but cannot crawl.
tags: []
status: accepted
nouns:
  - nouns/hero
  - nouns/power-state
  - nouns/ceiling
verbs:
  - verbs/crouch
---
## Rule

Grounded tall or spark hero can lower the standing profile while crouch is held. Small form receives no reduction. Crouching does not permit crawling. Airborne input does not grant a crouch-jump capability.

## Open design

Standing up under insufficient clearance is unresolved and has an explicit unknown case. Do not assume an automatic safe correction.

```gd-rule
case lower-profile:
    unknown_conditions: Crouch held while grounded.
    unknown_effects: Lower standing collision profile; no crawl. Exact dimensions unspecified.
    Hero:
        noun: nouns/hero
        requires: (tall OR spark) AND supported
        becomes: lowered

case small-no-reduction:
    unknown_conditions: Crouch input.
    unknown_effects: No additional collision-size reduction for small form; no narrower-passage or hidden-immunity capability.
    Hero:
        noun: nouns/hero
        requires: small

case airborne-input:
    unknown_conditions: Crouch input while airborne.
    unknown_effects: No crouch-jump capability granted; other motion unspecified.
    Hero:
        noun: nouns/hero
        requires: unsupported

case release-without-clearance:
    unknown_conditions: Tall/spark Hero releases crouch without sufficient standing clearance.
    unknown_effects: Standing response unselected; no automatic safe correction.
    Hero:
        noun: nouns/hero
        requires: tall OR spark
    Ceiling:
        noun: nouns/ceiling
```
