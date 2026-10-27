---
id: 01a16f86-7bec-759b-b9ce-b442997cef21
name: bump-life-supply
created_at: 2026-10-24T20:07:26.956Z
desc: Authored concealed supply releases a life-cap on a valid hit.
tags: []
status: accepted
coverage: defined
nouns:
  - nouns/supply-block
verb: verbs/bump
context_nouns:
  - nouns/life-cap
mechanics:
  - mechanics/supply-release
  - mechanics/earned-life
context_axes:
  - axes/Contact
---
## Conditions

Valid rising head hit on authored concealed life supply near brick-canopy.

## Consequences and feedback

Reveal block, release life-cap and mark spent. Discovery is optional, not required for finish. The record does not claim every hidden location is encoded as geometry.

```gd-rule
case reveal-life:
    context: Hero undersideContacts Block
    unknown_conditions: Valid rising head hit on authored concealed life supply near brick-canopy; location/valid-hit predicate unencoded.
    unknown_effects: Release Life-cap, not immediate life award; reveal spent solid Block. Discovery optional; not every hidden location encoded.
    Hero:
        noun: nouns/hero
        requires: rising
    Block:
        noun: nouns/supply-block
        requires: unspent AND SupplyContent.life AND concealed
        becomes: spent, revealed
```
