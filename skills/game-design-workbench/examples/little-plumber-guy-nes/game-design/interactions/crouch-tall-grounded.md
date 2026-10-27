---
id: 01a16f86-7bec-76b1-82b4-48d0b9d713e3
name: crouch-tall-grounded
created_at: 2026-10-24T20:07:26.956Z
desc: Tall grounded hero lowers collision profile without gaining crawl.
tags: []
status: accepted
coverage: defined
nouns:
  - nouns/hero
verb: verbs/crouch
context_nouns:
  - nouns/power-state
  - nouns/ground
mechanics:
  - mechanics/crouching
---
## Conditions

Hero is tall or spark, supported, and crouch is held.

## Consequences and feedback

Lower collision profile and show lowered pose. No crawling capability. Releasing with inadequate clearance is a separate unknown case, not silently resolved here.
