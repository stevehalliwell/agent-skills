---
id: 01a16f86-7bec-7de9-afea-5d1dcc7b3319
name: bump-spent-block
created_at: 2026-10-24T20:07:26.956Z
desc: Spent block remains solid and pays nothing.
tags: []
status: accepted
coverage: defined
nouns:
  - nouns/spent-block
verb: verbs/bump
mechanics:
  - mechanics/solid-collision
  - mechanics/supply-release
---
## Conditions

Hero hits underside of a block already spent.

## Consequences and feedback

Block remains solid and pays nothing. Brick-breaking rule does not apply to this state. Defined no payout must not be reported as missing behavior.
