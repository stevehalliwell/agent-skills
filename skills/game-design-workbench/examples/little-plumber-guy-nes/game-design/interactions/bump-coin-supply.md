---
id: 01a16f86-7bec-7afe-b2ca-c9ce45e3088c
name: bump-coin-supply
created_at: 2026-10-24T20:07:26.956Z
desc: Coin supply directly credits one coin and becomes spent.
tags: []
status: accepted
coverage: defined
nouns:
  - nouns/supply-block
verb: verbs/bump
context_nouns:
  - nouns/coin
mechanics:
  - mechanics/supply-release
  - mechanics/coin-collection
---
## Conditions

Hero hits an unspent coin supply from below.

## Consequences and feedback

Credit one coin through coin-collection and show payout; turn block spent. Visual coin emergence is feedback, not a second world coin that can pay again.
