---
id: 01a16f86-7bec-79a2-98cf-215888e2646a
name: bump-growth-supply-tall
created_at: 2026-10-24T20:07:26.956Z
desc: Growth supply releases a flower for tall or spark form.
tags: []
status: accepted
coverage: defined
nouns:
  - nouns/supply-block
verb: verbs/bump
context_nouns:
  - nouns/hero
  - nouns/power-state
mechanics:
  - mechanics/supply-release
context_axes:
  - axes/Contact
---
## Conditions

Tall or spark hero hits an unspent growth supply from below.

## Consequences and feedback

Release one stationary spark-flower above block and mark block spent. Current power determines released item; it is not an independent random drop.

```gd-rule
case release-flower:
    context: Hero undersideContacts Block
    unknown_conditions: Valid underside hit on authored unspent growth supply.
    unknown_effects: Release one stationary Spark-flower above Block; keep spent Block solid. No randomized drop; exact placement unspecified.
    Hero:
        noun: nouns/hero
        requires: tall OR spark
    Block:
        noun: nouns/supply-block
        requires: unspent AND SupplyContent.growth
        becomes: spent
```
