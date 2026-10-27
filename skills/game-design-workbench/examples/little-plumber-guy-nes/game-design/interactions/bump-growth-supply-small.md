---
id: 01a16f86-7bec-7ebc-950e-e7df32792d16
name: bump-growth-supply-small
created_at: 2026-10-24T20:07:26.956Z
desc: Growth supply releases a moving cap for small form.
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
---
## Conditions

Small hero hits an unspent growth supply from below.

## Consequences and feedback

Release one moving grow-cap above the block; turn block spent. Reward can move away and be missed. Touching the released object, not bumping alone, changes hero form.
