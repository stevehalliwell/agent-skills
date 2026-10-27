---
id: 01a16f86-7bec-7b8a-ace9-96b89bf1bcf5
name: touch-coin
created_at: 2026-10-24T20:07:26.956Z
desc: Token disappears and is credited once.
tags: []
status: accepted
coverage: defined
nouns:
  - nouns/coin
verb: verbs/touch
context_nouns:
  - nouns/hero
mechanics:
  - mechanics/coin-collection
---
## Conditions

Alive hero overlaps an uncollected world coin.

## Consequences and feedback

Remove token, credit bank and score, then process threshold conversion. Update HUD. Enemy and projectile contacts do not collect coins for hero.
